#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"
#include "inuStatus.h"
#include "inuRGB.h"


#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

bool depthRunning = true;
bool rgbRunning = true;

bool monitorRunning = true;

InuStatus g_InuStatus;


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
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
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

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, (unsigned char*)(depthframe.frame_data + offset));
                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                std::string netname = "Depth from Net " ;//+ _name;// std::to_string(tmp->device_id);// _name;
                cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(netname, image_width, image_height);
                cv::imshow(netname, display);  //? imshow will causes memory leak! stop then start will memory leak, but if comment imshow, will not?

                cv::waitKey(1);
            }catch(Exception  e){
                continue;
            }
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

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());
                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                std::string netname = "Depth from Net " ;//+ _name;// std::to_string(tmp->device_id);// _name;
                cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(netname, image_width, image_height);
                cv::imshow(netname, display);  //? imshow will causes memory leak! stop then start will memory leak, but if comment imshow, will not?

                cv::waitKey(1);
            }catch(Exception  e){
                continue;
            }

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

            try {

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

                cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(winname, image_width, image_height);
                cv::imshow(winname, imgshow);

//#ifdef _MSC_VER
                cv::waitKey(1);
//#endif
                }
            catch (Exception e) {
                continue;
            }
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

            try {

                cv::Mat img, imgshow;

                switch (p_frame->Format())
                {
                case eRGB:
                    img = cv::Mat(image_height, image_width, CV_8UC3, p_frame->GetData());
                    imgshow = img;
                    break;
                case eBGR:
                    img = cv::Mat(image_height, image_width, CV_8UC3, p_frame->GetData());
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

                cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(winname, image_width, image_height);
                cv::imshow(winname, imgshow);

//#ifdef _MSC_VER
                cv::waitKey(1);
//#endif
                }
            catch (Exception e) {
                continue;
            }
        }
    };
}
#endif

int main(int argc, char* argv[])
{
    spdlog_init();

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
