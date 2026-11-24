#include "inuPCL.h"

using namespace std;

namespace Inuchip
{
    InuPCL::InuPCL(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        p_inuDataPath = _inuDataPath;
    }
    InuPCL::~InuPCL()
    {
    }

    void InuPCL::FrameCallback(std::shared_ptr<InuDev::CPointCloudStream> iStream,
                            std::shared_ptr<const InuDev::CPointCloudFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;

            std::cout << "Pcl Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("PCL Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;

            std::cout << "Pcl Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
#ifdef _SHAREDATA_
        p_inuDataPath->setPCLshareFrame(iFrame);
#else

        InuDev::CPointCloudFrame::EFormat format = (InuDev::CPointCloudFrame::EFormat)iFrame->GetFormat();
        if (format != InuDev::CPointCloudFrame::EFormat::e3DPoints && format != InuDev::CPointCloudFrame::EFormat::e3DShortPoints && format != InuDev::CPointCloudFrame::EFormat::e3DPointsRGB)
        {
            std::cout <<"Pcl Frame's format (" << format << ") is not ePointCloud";
            return;
        }

        //const CPointCloudFrame::CPoint3DShortPixel* p3DshortData;
        // const CPointCloudFrame::C3DRGBPixel* pData;

        uint32_t numOfPoints = iFrame->GetNumOfPoints();
        uint32_t bytesPerPoint = iFrame->BytesPerPoint();
        uint32_t datasize;// = iFrame->BufferSize();

        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;

        pcl_frame.datatype = 4;
        pcl_frame.frame_id = currentFrameIndex;
        pcl_frame.width = numOfPoints;
        pcl_frame.height = format;
        pcl_frame.timestamp = currentTimeStamp/1000000;
        // depth_frame.compressed = 0;
        // // rgb_frame.device_id = p_inuSensor->st->_device_id;

        int pclframe_size = sizeof(PCLFrame);

        switch (format)
        {
            case InuDev::CPointCloudFrame::EFormat::e3DPoints:
                datasize = numOfPoints*3*sizeof(float);
                pcl_frame.datasize = datasize;
                pcl_frame.totalsize = pclframe_size + pcl_frame.datasize;
                pcl_frame.frame_data = new uint8_t[pcl_frame.totalsize];
                memcpy(pcl_frame.frame_data + pclframe_size,iFrame->Get3DData(), datasize);
                break;
            case InuDev::CPointCloudFrame::EFormat::e3DShortPoints:
                datasize = numOfPoints*3*sizeof(short);
                pcl_frame.datasize = datasize;
                pcl_frame.totalsize = pclframe_size + pcl_frame.datasize;
                pcl_frame.frame_data = new uint8_t[pcl_frame.totalsize];
                //p3DshortData = iFrame->Get3DShortData();
                memcpy(pcl_frame.frame_data + pclframe_size,iFrame->Get3DShortData(), datasize);
                break;
            case InuDev::CPointCloudFrame::EFormat::e3DPointsRGB:
                datasize = numOfPoints*(3*sizeof(float)+4);
                pcl_frame.datasize = datasize;
                pcl_frame.totalsize = pclframe_size + pcl_frame.datasize;
                pcl_frame.frame_data = new uint8_t[pcl_frame.totalsize];
                // pData = iFrame->Get3DRGBData();
                memcpy(pcl_frame.frame_data + pclframe_size,iFrame->Get3DRGBData(), datasize);
                break;
        }

        memcpy(pcl_frame.frame_data,&pcl_frame,pclframe_size);

        p_inuDataPath->setPCLFrame(pcl_frame);
#endif
    }

    bool InuPCL::StartStream()
    {
        my_logger->info("StartPclStream start!");

        InuDev::CInuError retCode(0);

        pclStream = p_inuSensor->inuSensor->CreatePointCloudStream(); 
        if (pclStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get PCL Stream" << std::endl;
            return false;
        }

        if(!(p_inuSensor->p_InuParams->inuPCLSetting.outputFormat))
        {
            retCode = pclStream->Init();
        }
        else{
#ifdef _GE432_
            retCode = pclStream->Init(p_inuSensor->p_InuParams->inuPCLSetting.registrationChannel); 
#else            
            retCode = pclStream->Init(InuDev::CPointCloudStream::eRGBRegistation,p_inuSensor->p_InuParams->inuPCLSetting.registrationChannel);
#endif
        }

        if (retCode != InuDev::eOK)
        {
            std::cout << "PCL initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        std::cout << "PCL Stream is initialized" << std::endl;
        my_logger->info("PCL Stream is initialized ");


        retCode = pclStream->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "PCL Stream Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        std::cout << "PCL Frames acquisition started" << std::endl;

        retCode = pclStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
        if (retCode != InuDev::eOK)
        {
            std::cout << "PCL register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        my_logger->info("StartPclStream ok!");

        return true;
    }

    bool InuPCL::StopStream()
    {
        if (!pclStream) { return true; }

        InuDev::CInuError retCode = pclStream->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "PCL Stream unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }

        // Stop frames acquisition (standby mode)
        retCode = pclStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "PCL Stream Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }
        std::cout << "PCL Stream frames acquisition stopped" << std::endl;

        retCode = pclStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "PCL Stream Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }

        return true;
    }

    int InuPCL::getPCLErrorTimes()
    {
        return error_times;
    }
}
