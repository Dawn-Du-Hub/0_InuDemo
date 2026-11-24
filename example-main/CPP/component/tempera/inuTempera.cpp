#include "inuTempera.h"

using namespace std;

namespace Inuchip
{
    InuTempera::InuTempera(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        p_inuDataPath = _inuDataPath;
    }

    void InuTempera::FrameCallback(std::shared_ptr<InuDev::CTemperaturesStream> iStream,
                            std::shared_ptr<const InuDev::CTemperaturesFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;
            std::cout << "Temperature Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("Temperature Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;
            std::cout << "Temperature Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
#ifdef _SHAREDATA_
        p_inuDataPath->setTemperatureshareFrame(iFrame);
#else
        float tempera[3];//sensor1,sensor2,pvt;

        iFrame->GetTemperature((CTemperaturesFrame::ESensorTemperatureType)1,tempera[0]);
        iFrame->GetTemperature((CTemperaturesFrame::ESensorTemperatureType)2,tempera[1]);
        iFrame->GetTemperature(CTemperaturesFrame::ESensorTemperatureType::ePVT,tempera[2]);

        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;


        tempera_frame.datatype = 6;
        tempera_frame.frame_id = currentFrameIndex;
        tempera_frame.timestamp = currentTimeStamp/1000000;
        tempera_frame.datasize = sizeof(float)*3;

        int temperae_size = sizeof(TemperaFrame);
        tempera_frame.totalsize = temperae_size + tempera_frame.datasize;

        tempera_frame.frame_data = new uint8_t[tempera_frame.totalsize];

        memcpy(tempera_frame.frame_data,&tempera_frame,temperae_size);
        memcpy(tempera_frame.frame_data + temperae_size,tempera, tempera_frame.datasize);

        p_inuDataPath->setTemperaFrame(tempera_frame);
#endif
    }

    bool InuTempera::StartStream()
    {
        temperaStream = std::static_pointer_cast<InuDev::CBaseStream>(p_inuSensor->inuSensor->CreateTemperaturesStream(InuDev::CTemperaturesFrame::ESensorTemperatureType::eAll));
        if (temperaStream == nullptr)
        {
            my_logger->info("Unexpected error, failed to get Temperature Stream ");
            return false;
        }

        InuDev::CInuError retCode(InuDev::eOK);
        retCode = std::static_pointer_cast<InuDev::CTemperaturesStream>(temperaStream)->Init();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Temperature initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        retCode = std::static_pointer_cast<InuDev::CTemperaturesStream>(temperaStream)->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Temperature start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        if(useCallback){
            // retCode = rgbStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            retCode = std::static_pointer_cast<InuDev::CTemperaturesStream>(temperaStream)->Register(std::bind(&InuTempera::FrameCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

            if (retCode != InuDev::eOK)
            {
                std::cout << "Temperature register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }
        my_logger->warn("StartTemperaturesStream end!");

        return true;
    }

    bool InuTempera::StopStream()
    {
        if (!temperaStream) { return true; }

        InuDev::CInuError retCode = std::static_pointer_cast<InuDev::CTemperaturesStream>(temperaStream)->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "Temperature unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }

        retCode = temperaStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Temperature Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }
        std::cout << "Temperature acquisition stopped" << std::endl;

        retCode = temperaStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Temperature Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }
        std::cout << "Temperature Stream was finalized" << std::endl;
        temperaStream = nullptr;

        return true;
    }

    int InuTempera::getTemperaErrorTimes()
    {
        return error_times;
    }
    std::vector<float> InuTempera::getTemperature()
    {
        std::vector<float>  vec = {0,0,0};
        std::shared_ptr<const CTemperaturesFrame> oTemperatureFrame;

        std::static_pointer_cast<InuDev::CTemperaturesStream>(temperaStream)->GetFrame(oTemperatureFrame);
        oTemperatureFrame->GetTemperature((CTemperaturesFrame::ESensorTemperatureType)1,vec[0]);
        oTemperatureFrame->GetTemperature((CTemperaturesFrame::ESensorTemperatureType)2,vec[1]);
        oTemperatureFrame->GetTemperature(CTemperaturesFrame::ESensorTemperatureType::ePVT,vec[2]);
        return vec;
    }
}
