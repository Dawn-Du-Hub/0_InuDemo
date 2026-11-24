#include "inuIR.h"

using namespace std;

namespace Inuchip
{
    InuIR::InuIR(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        p_inuDataPath = _inuDataPath;
    }

    void InuIR::FrameCallback(std::shared_ptr<InuDev::CStereoImageStream> iStream,
                            std::shared_ptr<const InuDev::CStereoImageFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;

            std::cout << "Stereo Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("Stereo Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;

            std::cout << "Stereo Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
#ifdef _SHAREDATA_
        p_inuDataPath->setIRshareFrame(iFrame);
#else

        int image_height = iFrame->GetRightFrame()->Height();
        int image_width = iFrame->GetRightFrame()-> Width();
        int rightdatasize = iFrame->GetRightFrame()->GetDataSize();
        // int limage_height = iFrame->GetLeftFrame()->Height();
        // int limage_width = iFrame->GetLeftFrame()->Width();
        // int leftdatasize = iFrame->GetLeftFrame()->GetDataSize();
        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;


        ir_frame.datatype = 3;
        ir_frame.frame_id = currentFrameIndex;
        ir_frame.width = image_width;
        ir_frame.height = image_height;
        ir_frame.timestamp = currentTimeStamp/1000000;
        // rgb_frame.compressed = iFrame->Format();
        // // rgb_frame.device_id = p_inuSensor->st->_device_id;
        ir_frame.datasize = rightdatasize;

        int irframe_size = sizeof(IRFrame);
        ir_frame.totalsize = irframe_size + ir_frame.datasize*2;

        ir_frame.frame_data = new uint8_t[ir_frame.totalsize];

        memcpy(ir_frame.frame_data,&ir_frame,irframe_size);
        memcpy(ir_frame.frame_data + irframe_size,iFrame->GetLeftFrame()->GetData(), ir_frame.datasize);
        memcpy(ir_frame.frame_data + irframe_size + ir_frame.datasize,iFrame->GetRightFrame()->GetData(), ir_frame.datasize);

        p_inuDataPath->setIRFrame(ir_frame);
#endif
    }

    bool InuIR::StartStream()
    {
        InuDev::CInuError retCode(0);

        my_logger->info("StartIRStream start!");

        stereoStream = p_inuSensor->inuSensor->CreateStereoImageStream(); //0,3
        if (stereoStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get Stereo Stream" << std::endl;
            return false;
        }

        retCode = stereoStream->Init(CStereoImageStream::eRaw);

        if (retCode != InuDev::eOK)
        {
            std::cout << "Stereo initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        retCode = stereoStream->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "StereoStream Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        useCallback  = p_inuSensor->st.IRuseCallback;

        if(useCallback){
            retCode = stereoStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            if (retCode != InuDev::eOK)
            {
                std::cout << "StereoStream register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }

        my_logger->info("StartIRStream end!");

        return true;
    }

    bool InuIR::StopStream()
    {

        if (!stereoStream) { return true; }

        /**  unregister, stop,terminate should call together, so just comment the return false in each call
        */
        InuDev::CInuError retCode = stereoStream->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "IR unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("IR unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;  
        }

        retCode = stereoStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "IR Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("IR Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }

        // Finalize depth stream service
        retCode = stereoStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "IR Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("IR Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }
        std::cout << "IR Stream was finalized" << std::endl;
        my_logger->info("IR Stream was finalized !");


        return true;
    }

    int InuIR::getIRErrorTimes()
    {
        return error_times;
    }
}
