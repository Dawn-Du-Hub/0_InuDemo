#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuTempera.h"

using namespace Inuchip;

bool running = true;

void showTempera(InuDataPath* p_inuDataPath)
{
    float sensor1_tempera,sensor2_tempera,pvt_tempera;
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_temperaFrameMutex);
        p_inuDataPath->m_temperaCV.wait(locker);
        TemperaFrame temperaframe = p_inuDataPath->getLatestTemperaFrame();
        locker.unlock();
        
        if (temperaframe.frame_data)
        {
            int offset = int(temperaframe.totalsize - temperaframe.datasize);
            float *data = (float*)(temperaframe.frame_data + offset);
            sensor1_tempera = *data++;
            sensor2_tempera = *data++;
            pvt_tempera = *data;
            std::cout << "sensor1_tempera  = " <<sensor1_tempera<< " sensor2_tempera  = " <<sensor2_tempera<<" pvt_tempera  = " <<pvt_tempera<<std::endl;

            delete[](temperaframe.frame_data);
            running = false;
        }
    };
}

#ifdef _SHAREDATA_
void showTempera2(InuDataPath* p_inuDataPath)
{
    float sensor1_tempera,sensor2_tempera,pvt_tempera;
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_temperaFrameMutex);
        p_inuDataPath->m_temperaCV.wait(locker);
        std::shared_ptr<const InuDev::CTemperaturesFrame> pframe = p_inuDataPath->getTemperatureshareFrame();
        locker.unlock();
        
        if (pframe)
        {
            float tempera[3];//sensor1,sensor2,pvt;

            pframe->GetTemperature((CTemperaturesFrame::ESensorTemperatureType)1,tempera[0]);
            pframe->GetTemperature((CTemperaturesFrame::ESensorTemperatureType)2,tempera[1]);
            pframe->GetTemperature(CTemperaturesFrame::ESensorTemperatureType::ePVT,tempera[2]);
            uint8_t* frame_data = new uint8_t[sizeof(float)*3];
            memcpy(frame_data,tempera, sizeof(float)*3);


            float *data = (float*)(frame_data);
            sensor1_tempera = *data++;
            sensor2_tempera = *data++;
            pvt_tempera = *data;
            std::cout << "sensorLeft_tempera  = " <<sensor1_tempera<< " sensorRight_tempera  = " <<sensor2_tempera<<" pvt_tempera  = " <<pvt_tempera<<std::endl;

            running = false;
        }
    };
}
#endif

int main(int argc, char* argv[])
{
    spdlog_init();

    InuDataPath m_InuDataPath;

    InuParams m_InuParams = InuParams();

    InuSensor m_InuSensor(&m_InuParams);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    InuTempera *p_InuTempera = new InuTempera(&m_InuSensor,&m_InuDataPath);

    p_InuTempera->StartStream();

    // if(p_InuTempera->isCallback())
    // {
#ifdef _SHAREDATA_
    std::thread show(showTempera2,&m_InuDataPath);
#else
    std::thread show(showTempera,&m_InuDataPath);
#endif
    // }
    // else{
    //     std::vector< float> tempera = p_InuTempera->getTemperature(); //sensor1,sensor2,pvt;

    //     std::cout << "sensor1_tempera  = " <<tempera[0]<< " sensor2_tempera  = " <<tempera[1]<<" pvt_tempera  = " <<tempera[2]<<std::endl;
    // }

    while (running) {
        // int choice = 1;
        // std::cout << "**********" << std::endl;
        // std::cout << "Exit: 0" << std::endl;
        // std::cout << "**********" << std::endl;
        // std::cin >> choice;
        // if (choice == 0) {
        //     break;
        // }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    };
    running = false;

//    if(p_InuTempera->isCallback())
       show.join();

    p_InuTempera->StopStream();
    m_InuSensor.DeviceTerminate();
    delete p_InuTempera;

    std::cout << "********** finish **********" << std::endl;

    return 0;
}
