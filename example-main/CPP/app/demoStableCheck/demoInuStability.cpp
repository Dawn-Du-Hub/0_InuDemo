#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"
#include "inuStatus.h"
#include "inuRGB.h"

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>


#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

bool depthRunning = true;
bool rgbRunning = true;

bool monitorRunning = true;

InuStatus g_InuStatus;

int ser;
#define BAUDRATE B9600
#define DEVFILE  "/dev/ttyUSB0"
uint8_t beepoff[4] = {0xA0,0x02,0x00,0xA2};
uint8_t beepon[4] = {0xA0,0x02,0x01,0xA3};
uint8_t poweroff[4] = {0xA0,0x01,0x00,0xA1}; 
uint8_t poweron[4] = {0xA0,0x01,0x01,0xA2}; 


int restarttime = 0;
int reconnecttime = 0;
int errorrestarttime = 0;

int openserial()
{
    int fd = 0;
    fd = open(DEVFILE, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("Can't Open Serial Port");
        return -1;
    }

    if(fcntl(fd,F_SETFL,0) < 0){
        perror("fcntl failed");
        return -2;
    }

    printf("Serial Port Opened!\n");
    fflush(stdout);

    //Serial Configuration
    struct termios serial;
    serial.c_iflag = 0;
    serial.c_oflag = 0;
    serial.c_lflag = 0;
    serial.c_cflag = 0;

    serial.c_cc[VMIN]  = 1;		// 最小读1字节
    serial.c_cc[VTIME] = 50;	// 5秒超时

    serial.c_cflag = BAUDRATE | CS8 | CREAD;

    if (tcsetattr(fd, TCSANOW, &serial) < 0) {
        perror("Error Setting configuration\n");
        return -3;
    }

    printf("Serial Port Configged!\n");
    fflush(stdout);

    tcflush(fd, TCIOFLUSH);

    return fd;
}

int send_data(int fd, uint8_t *send_buf){
    ssize_t ret;
    int len = 4;
    ret = write(fd,send_buf,len);

    if (ret == -1){
        printf ("write device  error\n");
        return -1;
    }
    return 1;
}

void OnlineMonitor(InuStatus* p_inuStatus)
{
    while(monitorRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if(1 == p_inuStatus->getSensorStatus())
        {
            InuDev::CInuSensor::ESensorState currState = p_inuStatus->p_inuSensor->inuSensor->GetState();
            InuDev::EConnectionState sensorConnectstate = p_inuStatus->p_inuSensor->inuSensor->GetConnectionState();

            if(sensorConnectstate == InuDev::EConnectionState::eDisconnected ||
               currState == InuDev::CInuSensor::ESensorState::eServiceNotConnected ||
               p_inuStatus->p_inuDepth->getDepthErrorTimes() > 10 ||
               p_inuStatus->p_inuRGB->getRGBErrorTimes() > 10)
            {
                ///////////////////////////////////////////////////////////////////////////////////
                if( (1 == p_inuStatus->getDepthStatus()) && p_inuStatus->p_inuDepth != nullptr ){
                    p_inuStatus->p_inuDepth->StopStream();
                }

                if( (1 == p_inuStatus->getRGBStatus()) && p_inuStatus->p_inuRGB != nullptr ){
                    p_inuStatus->p_inuRGB->StopStream();
                }

                p_inuStatus->p_inuSensor->DeviceTerminate();
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));

                while(!(p_inuStatus->p_inuSensor->checkDeviceState()) && reconnecttime< 50)
                {
                    reconnecttime++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                };

                if(reconnecttime == 50)
                {
                    while(1){
                        send_data(ser,beepon);
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                        send_data(ser,beepoff);
                        std::this_thread::sleep_for(std::chrono::seconds(3));

                        my_logger->info("error alarm!");
                    }
                }
                reconnecttime = 0;


                errorrestarttime++;
                std::cout << "Error Restart time:"<< errorrestarttime << std::endl;
                my_logger->warn("Error Restart time:{}",errorrestarttime);
                
                
                p_inuStatus->p_inuSensor->DeviceInit();

                if( 1 == p_inuStatus->getDepthStatus())
                {
                    p_inuStatus->p_inuDepth->StartStream();
                }
                if( 1 == p_inuStatus->getRGBStatus())
                {
                    p_inuStatus->p_inuRGB->StartStream();
                }
                ///////////////////////////////////////////////////////////////////////////////////

               // send_data(ser,beepon);
               // std::this_thread::sleep_for(std::chrono::seconds(3));
               // send_data(ser,beepoff);
                
               // my_logger->info("error alarm!");
            }
            else if(
                p_inuStatus->p_inuDepth->getDepthErrorTimes() == 0 &&
                p_inuStatus->p_inuRGB->getRGBErrorTimes() == 0)
            {
                send_data(ser,poweron);
                std::this_thread::sleep_for(std::chrono::seconds(10));

                send_data(ser,poweroff);
                std::this_thread::sleep_for(std::chrono::seconds(10));

                restarttime++;
                std::cout << "Restart time:"<< restarttime << std::endl;
                my_logger->warn("Restart time:{}",restarttime);

                ///////////////////////////////////////////////////////////////////////////////////
                if( (1 == p_inuStatus->getDepthStatus()) && p_inuStatus->p_inuDepth != nullptr ){
                    p_inuStatus->p_inuDepth->StopStream();
                }

                if( (1 == p_inuStatus->getRGBStatus()) && p_inuStatus->p_inuRGB != nullptr ){
                    p_inuStatus->p_inuRGB->StopStream();
                }

                p_inuStatus->p_inuSensor->DeviceTerminate();
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));

                while(!(p_inuStatus->p_inuSensor->checkDeviceState()))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                };

                p_inuStatus->p_inuSensor->DeviceInit();

                if( 1 == p_inuStatus->getDepthStatus())
                {
                    p_inuStatus->p_inuDepth->StartStream();
                }
                if( 1 == p_inuStatus->getRGBStatus())
                {
                    p_inuStatus->p_inuRGB->StartStream();
                }
                ///////////////////////////////////////////////////////////////////////////////////
            }
        }
    }
}

void showDepth(InuDataPath* p_inuDataPath)
{
    while (depthRunning)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        DepthFrame depthframe = p_inuDataPath->getLatestDepthFrame();
        locker.unlock();


        if (depthframe.frame_data)
        {
            int image_height = depthframe.height;
            int image_width = depthframe.width;
            int offset = int(depthframe.totalsize - depthframe.datasize);
            cv::Mat display;

            cv::Mat depthorg(image_height, image_width, CV_16UC1, (unsigned char*)(depthframe.frame_data + offset));
            depthorg.copyTo(display);

            display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
            applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

            std::string netname = "Depth from Net " ;//+ _name;// std::to_string(tmp->device_id);// _name;
            // cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            // cv::resizeWindow(netname, image_width, image_height);
            // cv::imshow(netname, display);  //? imshow will causes memory leak! stop then start will memory leak, but if comment imshow, will not?

            // cv::waitKey(1);

            std::cout << "depth OK!" << std::endl;
            my_logger->warn("depth OK!");

            delete[](depthframe.frame_data);
        }
    };
}
#ifdef _SHAREDATA_
void showDepth2(InuDataPath* p_inuDataPath)
{
    while (depthRunning)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getDepthshareFrame();
        locker.unlock();


        if (p_frame)
        {
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            cv::Mat display;

            cv::Mat depthorg(image_height, image_width, CV_16UC1, (unsigned char*)(p_frame->GetData()));
            depthorg.copyTo(display);

            display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
            applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

            std::string netname = "Depth from Net " ;//+ _name;// std::to_string(tmp->device_id);// _name;
            // cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            // cv::resizeWindow(netname, image_width, image_height);
            // cv::imshow(netname, display);  //? imshow will causes memory leak! stop then start will memory leak, but if comment imshow, will not?

            // cv::waitKey(1);

            std::cout << "depth OK!" << std::endl;
            my_logger->warn("depth OK!");

        }
    };
}
#endif

void showRGB(InuDataPath* p_inuDataPath)
{
    while (rgbRunning)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_rgbFrameMutex);
        p_inuDataPath->m_rgbCV.wait(locker);
        RGBFrame rgbframe = p_inuDataPath->getLatestRGBFrame();
        locker.unlock();
        
        if (rgbframe.frame_data)
        {
            int image_height = rgbframe.height;
            int image_width = rgbframe.width;
            int offset = int(rgbframe.totalsize - rgbframe.datasize);


            cv::Mat img, imgshow;

            switch (rgbframe.compressed)
            {
            case eRGB:
                img = cv::Mat(image_height, image_width, CV_8UC3, (rgbframe.frame_data + offset));
                imgshow = img;
                break;
            case eBGR:
                img = cv::Mat(image_height, image_width, CV_8UC3, (rgbframe.frame_data + offset));
                imgshow = img;
                break;
            case eBGRA:
                img = cv::Mat(image_height, image_width, CV_8UC4, (uint8_t*)(rgbframe.frame_data + offset));
#ifdef _MSC_VER
                cv::cvtColor(img, imgshow, CV_BGRA2BGR);
#else
                cv::cvtColor(img, imgshow, CV_BGRA2BGR);
#endif
                break;
            case eRGBA:
                img = cv::Mat(image_height, image_width, CV_8UC4, (rgbframe.frame_data + offset));
                cv::cvtColor(img, imgshow, CV_RGBA2RGB);
                break;
            default:
                printf("Got unrecognised format: %d\n", rgbframe.compressed);
                break;
            }

            std::string winname = "RGB"; //std::to_string(tmp->device_id); //_name;

            // cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            // cv::resizeWindow(winname, image_width, image_height);
            // cv::imshow(winname, imgshow);

            // cv::waitKey(1);

            std::cout << "RGB OK!" << std::endl;
            my_logger->warn("RGB OK!");

            delete[](rgbframe.frame_data);
        }
    };
}

#ifdef _SHAREDATA_
void showRGB2(InuDataPath* p_inuDataPath)
{
    while (rgbRunning)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_rgbFrameMutex);
        p_inuDataPath->m_rgbCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getRGBshareFrame();
        locker.unlock();
        
        if (p_frame)
        {
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();


            cv::Mat img, imgshow;

            switch (p_frame->Format())
            {
            case eRGB:
                img = cv::Mat(image_height, image_width, CV_8UC3, (p_frame->GetData()));
                imgshow = img;
                break;
            case eBGR:
                img = cv::Mat(image_height, image_width, CV_8UC3, (p_frame->GetData()));
                imgshow = img;
                break;
            case eBGRA:
                img = cv::Mat(image_height, image_width, CV_8UC4, (uint8_t*)(p_frame->GetData()));
#ifdef _MSC_VER
                cv::cvtColor(img, imgshow, CV_BGRA2BGR);
#else
                cv::cvtColor(img, imgshow, CV_BGRA2BGR);
#endif
                break;
            case eRGBA:
                img = cv::Mat(image_height, image_width, CV_8UC4, (p_frame->GetData()));
                cv::cvtColor(img, imgshow, CV_RGBA2RGB);
                break;
            default:
                printf("Got unrecognised format: %d\n", p_frame->Format());
                break;
            }

            std::string winname = "RGB"; //std::to_string(tmp->device_id); //_name;

            // cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            // cv::resizeWindow(winname, image_width, image_height);
            // cv::imshow(winname, imgshow);

            // cv::waitKey(1);

            std::cout << "RGB OK!" << std::endl;
            my_logger->warn("RGB OK!");

        }
    };
}
#endif

int main(int argc, char* argv[])
{
    spdlog_init();

    ser = openserial();
    if(ser < 0)
        return -1;

    InuDataPath m_InuDataPath;

    std::string filename = "./../../../app/cfg/Params.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };

    m_InuSensor.DeviceInit();
    g_InuStatus.setSensorStatus(&m_InuSensor,1);

    InuDepth *p_InuDepth = new InuDepth(&m_InuSensor,&m_InuDataPath);
    p_InuDepth->StartStream();
    g_InuStatus.setDepthStatus(p_InuDepth,1);

    InuRGB *p_InuRGB = new InuRGB(&m_InuSensor,&m_InuDataPath);
    p_InuRGB->StartStream();
    g_InuStatus.setRGBStatus(p_InuRGB,1);

#ifdef _SHAREDATA_
    std::thread showD(showDepth2,&m_InuDataPath);
#else
    std::thread showD(showDepth,&m_InuDataPath);
#endif

#ifdef _SHAREDATA_
    std::thread showC(showRGB2,&m_InuDataPath);
#else
    std::thread showC(showRGB,&m_InuDataPath);
#endif
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::thread thMonitor(OnlineMonitor,&g_InuStatus);

    while (1) {
        int choice = 1;
        // std::cout << "**********" << std::endl;
        // std::cout << "Exit: 0" << std::endl;
        // std::cout << "**********" << std::endl;
        // std::cin >> choice;
        // if (choice == 0) {
        //     break;
        // }
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::seconds(2));

    };
    depthRunning = false;
    rgbRunning = false;
    monitorRunning = false;

    showD.join();
    showC.join();
    thMonitor.join();

    p_InuDepth->StopStream();
    delete p_InuDepth;

    p_InuRGB->StopStream();
    delete p_InuRGB;

    m_InuSensor.DeviceTerminate();

    return 0;
}
