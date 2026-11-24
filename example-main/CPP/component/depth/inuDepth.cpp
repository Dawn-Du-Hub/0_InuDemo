#include "inuDepth.h"

using namespace std;

namespace Inuchip
{
    InuDepth::InuDepth(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        depthSetting = &((p_inuSensor->p_InuParams)->inuDepthSetting);
        p_inuDataPath = _inuDataPath;
        p_inuDepthFilter = new InuDepthFilter(_inuSensor);
        p_inuDepthFilter->setDepthScale(depthSetting->depthScale);
        p_inuDepthFilter->setConfidence(depthSetting->confidence);

        p_inuDepthFilter->depthParams->blob_maxsize_sdk = depthSetting->_sdkBlobMaxSize;
        p_inuDepthFilter->depthParams->blob_maxheight_sdk = depthSetting->_sdkBlobMaxHeight;
        p_inuDepthFilter->depthParams->blob_threshold_sdk = depthSetting->_sdkBlobDisparityThreshold;
        p_inuDepthFilter->depthParams->blob_sameblob_sdk = depthSetting->_sdkBlobDispDiffSameBlob;
        p_inuDepthFilter->depthParams->blob_modeblob_sdk = depthSetting->_sdkBlobmode;

        p_inuDepthFilter->depthParams->temporal_stable_sdk = depthSetting->_sdkTemporalStable;
        p_inuDepthFilter->depthParams->temporal_rapid_sdk = depthSetting->_sdkTemporalRapid;

    }
    InuDepth::~InuDepth()
    {
        delete p_inuDepthFilter;
    }

    void InuDepth::FrameCallback(std::shared_ptr<InuDev::CDepthStream> iStream,
                            std::shared_ptr<const InuDev::CImageFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            std::cout << "Depth Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("Depth Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            error_times++;
            return;
        }

        if (iFrame->Valid == false)
        {
            std::cout << "Depth Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            error_times++;
            return;
        }

        error_times = 0;
        p_inuDepthFilter->_setDistAndScale();
        p_inuDepthFilter->_setConfidence();
        p_inuDepthFilter->_setSDKRuntimefilter();

#ifdef _SHAREDATA_
        p_inuDataPath->setDepthshareFrame(iFrame);
#else

        int image_height = iFrame->Height();
        int image_width = iFrame->Width();
        int datasize = iFrame->GetDataSize();
        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;


        depth_frame.datatype = 1;
        depth_frame.frame_id = currentFrameIndex;
        depth_frame.width = image_width;
        depth_frame.height = image_height;
        depth_frame.timestamp = currentTimeStamp/1000000;
        depth_frame.compressed = 0;
        // rgb_frame.device_id = p_inuSensor->st->_device_id;
        depth_frame.datasize = datasize;

        int depthframe_size = sizeof(DepthFrame);
        depth_frame.totalsize = depthframe_size + depth_frame.datasize;

        // if (depth_frame.frame_data)
        //     delete[]  (depth_frame.frame_data);

        depth_frame.frame_data = new uint8_t[depth_frame.totalsize];

        memcpy(depth_frame.frame_data,&depth_frame,depthframe_size);
        memcpy(depth_frame.frame_data + depthframe_size,iFrame->GetData(), depth_frame.datasize);

        p_inuDataPath->setDepthFrame(depth_frame);
#endif
    }

    bool InuDepth::StartStream()
    {
        my_logger->info("StartDepthStream start!");

        InuDev::CInuError retCode(0);

        depthStream = p_inuSensor->inuSensor->CreateDepthStream(depthSetting->channel);

        if (depthStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get Depth Stream" << std::endl;
            return false;
        }

        p_inuDepthFilter->depthStream = depthStream;

        uint32_t tmpPostProcessing = depthSetting->postProcessing;

        if (depthSetting->outputFormat == CDepthStream::EOutputFormat::eRaw)
        {
            
            // if output format is raw, none of postprocess can be used
            #ifdef _GE432_
            retCode = depthStream->Init(CDepthStream::EOutputFormat::eRaw, CDepthStream::EPostProcessing::eNone | (CDepthStream::EPostProcessing)tmpPostProcessing);
            #else
            retCode = depthStream->Init(CDepthStream::EOutputFormat::eRaw, CDepthStream::EPostProcessing::eNone | tmpPostProcessing, InuDev::DEFAULT_CHANNEL_ID);
            #endif
        }

        else if (tmpPostProcessing == CDepthStream::EPostProcessing::eNone)
        {
            #ifdef _GE432_
            retCode = depthStream->Init(CDepthStream::EOutputFormat::eDefault, CDepthStream::EPostProcessing::eNone); //eDefaultPP  eNone
            #else
            retCode = depthStream->Init(CDepthStream::EOutputFormat::eDefault, CDepthStream::EPostProcessing::eNone, InuDev::DEFAULT_CHANNEL_ID); //eDefaultPP  eNone
            #endif
        }

        else if (tmpPostProcessing == CDepthStream::EPostProcessing::eDefaultPP)
        {
            retCode = depthStream->Init();
            // setSDKfilterDefault();
        }
        else
        {
            #ifdef _GE432_
            retCode = depthStream->Init(CDepthStream::EOutputFormat::eDefault, (CDepthStream::EPostProcessing)tmpPostProcessing);
            #else
            retCode = depthStream->Init(CDepthStream::EOutputFormat::eDefault, (unsigned int)tmpPostProcessing, InuDev::DEFAULT_CHANNEL_ID);
            #endif

            if (tmpPostProcessing & InuDev::CDepthStream::EPostProcessing::eHoleFill)
            {
                p_inuDepthFilter->_setSDKHoleFill();
            }

            if (tmpPostProcessing & InuDev::CDepthStream::EPostProcessing::eOutlierRemove)
            {
                p_inuDepthFilter->_setSDKOutlierRemove();
            }

            if (tmpPostProcessing & InuDev::CDepthStream::EPostProcessing::eStaticTemporal)
            {
                // setSDKTemporalFileter();
            }

            if (tmpPostProcessing & InuDev::CDepthStream::EPostProcessing::eBlob)
            {
                p_inuDepthFilter->_setSDKBlob();
            }

            if (tmpPostProcessing & InuDev::CDepthStream::EPostProcessing::eTemporal)
            {
                p_inuDepthFilter->_setSDKTemporalFilter();
            }
        }

        if (retCode != InuDev::eOK)
        {
            std::cout << "Depth initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        
        // Start depth frames acquisition (operation mode)
        retCode = depthStream->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        if(useCallback){
            retCode = depthStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
            my_logger->info("StartDepthStream end!");
        }

        return true;
    }

    bool InuDepth::StopStream()
    {
        if (!depthStream) { return true; }

        InuDev::CInuError retCode = depthStream->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "Depth unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("Depth unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        retCode = depthStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Depth Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("Depth Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        // Finalize depth stream service
        retCode = depthStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Depth Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("Depth Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        return true;
    }

    int InuDepth::getDepthErrorTimes()
    {
        return error_times;
    }

}
