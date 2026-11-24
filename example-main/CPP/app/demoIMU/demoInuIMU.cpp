#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuIMU.h"

using namespace Inuchip;

bool running = true;
bool withLeft = true;
double scale = 1.0;
float beta = 10;
std::string winname = "IR"; 

void showIMU(InuDataPath* p_inuDataPath)
{
    float gyro_x,gyro_y,gyro_z,accel_x,accel_y,accel_z;
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_imuFrameMutex);
        p_inuDataPath->m_imuCV.wait(locker);
        IMUFrame imuframe = p_inuDataPath->getLatestIMUFrame();
        locker.unlock();
        
        if (imuframe.frame_data)
        {
            int offset = int(imuframe.totalsize - imuframe.datasize);
            float *data = (float*)(imuframe.frame_data + offset);
            gyro_x = *data++;
            gyro_y = *data++;
            gyro_z = *data++;
            accel_x = *data++;
            accel_y = *data++;
            accel_z = *data;
            std::cout << "gyro_x  = " <<gyro_x<< " gyro_y  = " <<gyro_y<<" gyro_z  = " <<gyro_z<<std::endl;
            std::cout << "accel_x = " <<accel_x<< " accel_y = " <<accel_y<<" accel_z = " <<accel_z<<std::endl;\

            delete[](imuframe.frame_data);
        }
    };
}

#ifdef _SHAREDATA_
void showIMU2(InuDataPath* p_inuDataPath)
{
    float gyro_x, gyro_y, gyro_z, accel_x, accel_y, accel_z;
    float gyro_fps = 0, accel_fps=0;
    uint64_t gyro_lastTimeStamp = 0;
    uint64_t accel_lastTimeStamp = 0;
    uint64_t currentTimeStamp = 0;

    InuDev::CPoint3D gyro;
    InuDev::CPoint3D accel;
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_imuFrameMutex);
        p_inuDataPath->m_imuCV.wait(locker);
        std::shared_ptr<const InuDev::CImuFrame> p_frame = p_inuDataPath->getIMUshareFrame();
        locker.unlock();
        
        auto imu_gyro_search = p_frame->SensorsData.find(InuDev::EImuType::eGyroscope);
        auto imu_accel_search = p_frame->SensorsData.find(InuDev::EImuType::eAccelerometer);
        currentTimeStamp = p_frame->Timestamp;
        uint32_t frameId = p_frame->FrameIndex;

        if (imu_accel_search != p_frame->SensorsData.end())
        {
            accel = imu_accel_search->second;
            accel_fps = 1000000000. / (currentTimeStamp - accel_lastTimeStamp);
            accel_lastTimeStamp = currentTimeStamp;
            accel_x = accel.X();
            accel_y = accel.Y();
            accel_z = accel.Z();
            std::cout << "accel_x = " << accel_x << " accel_y = " << accel_y << " accel_z = " << accel_z << " fps: " << accel_fps << std::endl;
        }
        if (imu_gyro_search != p_frame->SensorsData.end())
        {
            gyro = imu_gyro_search->second;
            gyro_fps = 1000000000./(currentTimeStamp - gyro_lastTimeStamp);
            gyro_lastTimeStamp = currentTimeStamp;
            gyro_x = gyro.X();
            gyro_y = gyro.Y();
            gyro_z = gyro.Z();
            std::cout << "gyro_x  = " << gyro_x << " gyro_y  = " << gyro_y << " gyro_z  = " << gyro_z << " fps: " << gyro_fps << std::endl;
        }
    };
}
#endif
// 由于IMU的帧率较高，demoInuIMU 中有时会造成帧率错乱，因此把显示直接放到了 inuIMU.cpp 的回调中
// 请根据自身电脑性能实测确定放到合适的地方显示
// IMU CPP
int main(int argc, char* argv[])
{
    spdlog_init();

    InuDataPath m_InuDataPath;

    std::string filename = "./../../../app/cfg/Params.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    InuIMU *p_InuIMU = new InuIMU(&m_InuSensor,&m_InuDataPath);

    p_InuIMU->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showIMU2,&m_InuDataPath);
#else
    std::thread show(showIMU,&m_InuDataPath);
#endif

    while (1) {
        int choice = 1;
        std::cout << "**********" << std::endl;
        std::cout << "Exit: 0" << std::endl;
        std::cout << "**********" << std::endl;
        std::cin >> choice;
        if (choice == 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    };
    running = false;

    show.join();
    p_InuIMU->StopStream();
    m_InuSensor.DeviceTerminate();
    delete p_InuIMU;

    return 0;

}
