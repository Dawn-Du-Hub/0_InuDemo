#include "inuIMU.h"

using namespace std;
uint64_t gyro_lastTimeStamp = 0;
uint64_t accel_lastTimeStamp = 0;

namespace Inuchip
{
    InuIMU::InuIMU(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        p_inuDataPath = _inuDataPath;
    }

    void InuIMU::FrameCallback(std::shared_ptr<InuDev::CImuStream> iStream,
                            std::shared_ptr<const InuDev::CImuFrame> iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;

            std::cout << "IMU Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("IMU Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;

            std::cout << "IMU Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
#ifdef _SHAREDATA_
        #if 0
        p_inuDataPath->setIMUshareFrame(iFrame);
        #else
        float gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z;
        float gyro_fps = 0, accel_fps = 0;


        auto imu_gyro_search = iFrame->SensorsData.find(InuDev::EImuType::eGyroscope);
        auto imu_accel_search = iFrame->SensorsData.find(InuDev::EImuType::eAccelerometer);
        uint64_t currentTimeStamp = iFrame->Timestamp;
        uint32_t frameId = iFrame->FrameIndex;
        if (imu_gyro_search != iFrame->SensorsData.end())
        {
            gyro = imu_gyro_search->second;
            gyro_fps = 1000000000. / (currentTimeStamp - gyro_lastTimeStamp);
            gyro_lastTimeStamp = currentTimeStamp;
            gyro_x = gyro.X();
            gyro_y = gyro.Y();
            gyro_z = gyro.Z();
            std::cout << "gyro_x  = " << gyro_x << " gyro_y  = " << gyro_y << " gyro_z  = " << gyro_z << " fps: " << gyro_fps << std::endl;
        }

        if (imu_accel_search != iFrame->SensorsData.end())
        {
            accel = imu_accel_search->second;
            accel_fps = 1000000000. / (currentTimeStamp - accel_lastTimeStamp);
            accel_lastTimeStamp = currentTimeStamp;
            accel_x = accel.X();
            accel_y = accel.Y();
            accel_z = accel.Z();
            std::cout << "accel_x = " << accel_x << " accel_y = " << accel_y << " accel_z = " << accel_z << " fps: " << accel_fps << std::endl;
        }
        #endif

#else

        currentFrameIndex = iFrame->FrameIndex;
        currentTimeStamp = iFrame->Timestamp;


        imu_frame.datatype = 5;
        imu_frame.frame_id = currentFrameIndex;

        imu_frame.timestamp = currentTimeStamp/1000000;
        imu_frame.datasize = sizeof(float)*6;

        int imuframe_size = sizeof(IMUFrame);
        imu_frame.totalsize = imuframe_size + imu_frame.datasize;

        imu_frame.frame_data = new uint8_t[imu_frame.totalsize];

        auto imu_gyro_search = iFrame->SensorsData.find(InuDev::EImuType::eGyroscope);
        auto imu_accel_search = iFrame->SensorsData.find(InuDev::EImuType::eAccelerometer);
        if (imu_gyro_search != iFrame->SensorsData.end())
        {
            gyro = imu_gyro_search->second;
        }

        if (imu_accel_search != iFrame->SensorsData.end())
        {
            accel = imu_accel_search->second;
        }

        memcpy(imu_frame.frame_data,&imu_frame,imuframe_size);
        memcpy(imu_frame.frame_data + imuframe_size, gyro.GetElements(), sizeof(float)*3);
        memcpy(imu_frame.frame_data + imuframe_size+ sizeof(float) * 3, accel.GetElements(), sizeof(float) * 3);

        p_inuDataPath->setIMUFrame(imu_frame);
#endif  
    }

    bool InuIMU::StartStream()
    {
        InuDev::CInuError retCode(0);

        my_logger->info("StartIMUStream start!");

        imuStream = std::static_pointer_cast<InuDev::CBaseStream>(p_inuSensor->inuSensor->CreateImuStream());
        if (imuStream == nullptr)
        {
            my_logger->info("Unexpected error, failed to get IMU Stream ");
            return false;
        }

        retCode = std::static_pointer_cast<InuDev::CImuStream>(imuStream)->Init();
        if (retCode != InuDev::eOK)
        {
            std::cout << "IMU initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        retCode = std::static_pointer_cast<InuDev::CImuStream>(imuStream)->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "IMU start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        retCode = std::static_pointer_cast<InuDev::CImuStream>(imuStream)->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
        if (retCode != InuDev::eOK)
        {
            std::cout << "IMU error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        my_logger->warn("StartIMUStream end!");

        return true;
    }

    bool InuIMU::StopStream()
    {

        if (!imuStream) { return true; }

        InuDev::CInuError retCode = std::static_pointer_cast<InuDev::CImuStream>(imuStream)->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "IMU unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }

        retCode = imuStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "IMU Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }
        std::cout << "IMU acquisition stopped" << std::endl;

        retCode = imuStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "IMU Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }
        std::cout << "IMU Stream was finalized" << std::endl;
        imuStream = nullptr;

        return true;
    }

    int InuIMU::getIMUErrorTimes()
    {
        return error_times;
    }
}
