#include "inuFE.h"

using namespace std;

namespace Inuchip
{
    InuFisheye::InuFisheye(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        FESetting = &((p_inuSensor->p_InuParams)->inuFESetting);
        p_inuDataPath = _inuDataPath;
    }

    void InuFisheye::FrameCallback(std::shared_ptr<InuDev::CImageStream> iStream,
                            std::shared_ptr<const InuDev::CImageFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;

            std::cout << "FE Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("FE Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;

            std::cout << "FE Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
#ifdef _SHAREDATA_
        p_inuDataPath->setFisheyeshareFrame(iFrame);
#else

        int image_height = iFrame->Height();
        int image_width = iFrame->Width();
        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;


        fisheye_frame.datatype = 7;
        fisheye_frame.frame_id = currentFrameIndex;
        fisheye_frame.width = image_width;
        fisheye_frame.height = image_height;
        fisheye_frame.timestamp = currentTimeStamp/1000000;
        fisheye_frame.compressed = 0;
        // // // rgb_frame.device_id = p_inuSensor->st->_device_id;
        fisheye_frame.datasize = iFrame->GetDataSize();;

        int fisheye_frame_size = sizeof(FishEyeFrame);
        fisheye_frame.totalsize = fisheye_frame_size + fisheye_frame.datasize;

        fisheye_frame.frame_data = new uint8_t[fisheye_frame.totalsize];

        memcpy(fisheye_frame.frame_data,&fisheye_frame,fisheye_frame_size);
        memcpy(fisheye_frame.frame_data + fisheye_frame_size,iFrame->GetData(), fisheye_frame.datasize);

        p_inuDataPath->setFishEyeFrame(fisheye_frame);
#endif
    }

    bool InuFisheye::StartStream()
    {
        InuDev::CInuError retCode(0);

        my_logger->info("StartFEStream start!");

        feStream = p_inuSensor->inuSensor->CreateImageStream(FESetting->channel);
        if (feStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get fisheye Stream" << std::endl;
            return false;
        }

        retCode = feStream->Init(CImageStream::eDefault);

        if (retCode != InuDev::eOK)
        {
            std::cout << "fisheye initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        retCode = feStream->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "fisheye Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        if(useCallback){
            retCode = feStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            if (retCode != InuDev::eOK)
            {
                std::cout << "fisheyeStream register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }

        my_logger->info("StartFEStream end!");

        return true;
    }

    bool InuFisheye::StopStream()
    {

        if (!feStream) { return true; }

        /**  unregister, stop,terminate should call together, so just comment the return false in each call
        */
        InuDev::CInuError retCode = feStream->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "Fisheye unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("Fisheye  unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;  
        }

        retCode = feStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Fisheye Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("Fisheye Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }

        // Finalize Fisheye  stream service
        retCode = feStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Fisheye Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("Fisheye Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }
        std::cout << "Fisheye Stream was finalized" << std::endl;
        my_logger->info("Fisheye Stream was finalized !");


        return true;
    }

    int InuFisheye::getFisheyeErrorTimes()
    {
        return error_times;
    }
}
