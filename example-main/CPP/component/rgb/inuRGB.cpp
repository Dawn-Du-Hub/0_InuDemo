#include "inuRGB.h"

using namespace std;

namespace Inuchip
{
    InuRGB::InuRGB(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        rgbSetting = &((p_inuSensor->p_InuParams)->inuRGBSetting);
        p_inuDataPath = _inuDataPath;
    }

    void InuRGB::FrameCallback(std::shared_ptr<InuDev::CImageStream> iStream,
                            std::shared_ptr<const InuDev::CImageFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;
            std::cout << "RGB Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("RGB Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;
            std::cout << "RGB Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
#ifdef _SHAREDATA_
        p_inuDataPath->setRGBshareFrame(iFrame);
#else

        int image_height = iFrame->Height();
        int image_width = iFrame->Width();
        int datasize = iFrame->GetDataSize();
        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;


        rgb_frame.datatype = 2;
        rgb_frame.frame_id = currentFrameIndex;
        rgb_frame.width = image_width;
        rgb_frame.height = image_height;
        rgb_frame.timestamp = currentTimeStamp/1000000;
        rgb_frame.compressed = iFrame->Format();
        // rgb_frame.device_id = p_inuSensor->st->_device_id;
        rgb_frame.datasize = datasize;

        int rgbframe_size = sizeof(RGBFrame);
        rgb_frame.totalsize = rgbframe_size + rgb_frame.datasize;

        //if (rgb_frame.frame_data)
          //  delete[]  (rgb_frame.frame_data);

        rgb_frame.frame_data = new uint8_t[rgb_frame.totalsize];

        memcpy(rgb_frame.frame_data,&rgb_frame,rgbframe_size);
        memcpy(rgb_frame.frame_data + rgbframe_size,iFrame->GetData(), rgb_frame.datasize);


        p_inuDataPath->setRGBFrame(rgb_frame);
#endif
    }

    bool InuRGB::_StartStream()
    {
        CImageStream::EOutputFormat   outforamt;
        InuDev::CInuError retCode(0);

        uint32_t register_channel = rgbSetting->registrationChannel;
        if (-1 == rgbSetting->registrationChannel)
        {
            register_channel = DEFAULT_CHANNEL_ID;
        }

        if (0 == rgbSetting->outputFormat) {// eDefault
            outforamt = CImageStream::EOutputFormat::eDefault;
        }
        else if (1 == rgbSetting->outputFormat) {// eRaw
            outforamt = CImageStream::EOutputFormat::eRaw;
        }
        else if (2 == rgbSetting->outputFormat) {//eBGRA
            outforamt = CImageStream::EOutputFormat::eBGRA;
        }
        else if (3 == rgbSetting->outputFormat) {//eRGBA
            outforamt = CImageStream::EOutputFormat::eRGBA;
        }
        else {//eBGR
            outforamt = CImageStream::EOutputFormat::eBGR;
        }


        ppe = CImageStream::EPostProcessing::eNone;

        if (0 == rgbSetting->postProcessing)
        {
            ppe = CImageStream::EPostProcessing::eNone;
        }
        else if(4 == rgbSetting->postProcessing)
        {
            ppe = CImageStream::EPostProcessing::eGammaCorrect;
        }

#ifndef _GE432_
        if (-1 != rgbSetting->registrationChannel)
        {
            ppe = CImageStream::EPostProcessing::eRegistered;
        }
#endif

#ifdef _GE432_
        if(-1 != rgbSetting->registrationChannel){
            rgbRegisteredStream = p_inuSensor->inuSensor->CreateImageRegisteredStream(rgbSetting->channel,register_channel);
            if (rgbRegisteredStream == nullptr)
            {
                std::cout << "Unexpected error, failed to get RGB register Stream" << std::endl;
                return false;
            }
            retCode = rgbRegisteredStream->Init(outforamt, ppe,CDepthProperties::EPostProcessing::eDefaultPP);
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
            retCode = rgbRegisteredStream->Start();
            if (retCode != InuDev::eOK)
            {
                std::cout << "Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }
        else{
            rgbStream = p_inuSensor->inuSensor->CreateImageStream(rgbSetting->channel);
            if (rgbStream == nullptr)
            {
                std::cout << "Unexpected error, failed to get RGB Stream" << std::endl;
                return false;
            }
            retCode = rgbStream->Init(outforamt, ppe);
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
            retCode = rgbStream->Start();
            if (retCode != InuDev::eOK)
            {
                std::cout << "Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }
        return true;

#else
        rgbStream = p_inuSensor->inuSensor->CreateImageStreamExt(rgbSetting->channel);
        if (rgbStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get RGB Stream" << std::endl;
            return false;
        }

        retCode = rgbStream->Init(outforamt, ppe, register_channel);

        if (retCode != InuDev::eOK)
        {
            std::cout << "RGB initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        retCode = rgbStream->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        return true;
#endif
    }

    bool InuRGB::StartStream()
    {
        InuDev::CInuError retCode(0);

        module_name = p_inuSensor->st.module;

        if (!(_StartStream()))
        {
            std::cout << "Call StartRGBStream error! " << std::endl;
            return false;
        }

        useCallback  = p_inuSensor->st.RGBuseCallback;

        if(useCallback){
#ifdef _GE432_
            if(-1 != rgbSetting->registrationChannel){
                retCode = rgbRegisteredStream->Register(std::bind(&InuRGB::FrameCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            }else{
                retCode = rgbStream->Register(std::bind(&InuRGB::FrameCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            }
#else            
            // retCode = rgbStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            retCode = rgbStream->Register(std::bind(&InuRGB::FrameCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
#endif
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }

        return true;
    }

    bool InuRGB::StopStream()
    {
#ifdef _GE432_
        if (!rgbStream && !rgbRegisteredStream) { return true; }

        InuDev::CInuError retCode(0);

        if(-1 != rgbSetting->registrationChannel){

            retCode = rgbRegisteredStream->Register(nullptr);
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                my_logger->warn("RGB unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
                // return false;  
            }

            retCode = rgbRegisteredStream->Stop();
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                my_logger->warn("RGB Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
                // return false;
            }

            // Finalize depth stream service
            retCode = rgbRegisteredStream->Terminate();
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                my_logger->warn("RGB Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
                // return false;
            }
            std::cout << "RGB Stream was finalized" << std::endl;
            my_logger->info("RGB Stream was finalized !");

        }else{

            retCode = rgbStream->Register(nullptr);
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                my_logger->warn("RGB unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
                // return false;  
            }

            retCode = rgbStream->Stop();
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                my_logger->warn("RGB Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
                // return false;
            }

            // Finalize depth stream service
            retCode = rgbStream->Terminate();
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                my_logger->warn("RGB Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
                // return false;
            }
            std::cout << "RGB Stream was finalized" << std::endl;
            my_logger->info("RGB Stream was finalized !");
        }
#else
        if (!rgbStream) { return true; }

        /**  unregister, stop,terminate should call together, so just comment the return false in each call
        */
        InuDev::CInuError retCode = rgbStream->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "RGB unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("RGB unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;  
        }

        retCode = rgbStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "RGB Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("RGB Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }

        // Finalize depth stream service
        retCode = rgbStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "RGB Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("RGB Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }
        std::cout << "RGB Stream was finalized" << std::endl;
        my_logger->info("RGB Stream was finalized !");
#endif

        return true;
    }

    int InuRGB::getRGBErrorTimes()
    {
        return error_times;
    }
}
