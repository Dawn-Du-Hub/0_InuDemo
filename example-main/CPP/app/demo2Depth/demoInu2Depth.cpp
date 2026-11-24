#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

bool running = true;
void showDepth1(InuDataPath* p_inuDataPath)
{
    while (running)
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

                std::string netname = "Depth1" ;//+ _name;// std::to_string(tmp->device_id);// _name;
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
void showDepth1Shard_ptr(InuDataPath* p_inuDataPath)
{
    while (running)
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

                std::string netname = "Depth1" ;//+ _name;// std::to_string(tmp->device_id);// _name;
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

void showDepth2(InuDataPath* p_inuDataPath)
{
    while (running)
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

                std::string netname = "Depth2" ;//+ _name;// std::to_string(tmp->device_id);// _name;
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
void showDepth2Shared_ptr(InuDataPath* p_inuDataPath)
{
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getDepthshareFrame();
        locker.unlock();

        if (p_frame){
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            cv::Mat display;

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());
                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                std::string netname = "Depth2" ;//+ _name;// std::to_string(tmp->device_id);// _name;
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

int main(int argc, char* argv[])
{
    spdlog_init();

    InuDataPath m1_InuDataPath;

    std::string filename = "./../../../app/cfg/Params1.yaml";
    InuParams m1_InuParams(filename);

    InuSensor m1_InuSensor(&m1_InuParams);

    while(!m1_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m1_InuSensor.DeviceInit();

    InuDepth *p1_InuDepth = new InuDepth(&m1_InuSensor,&m1_InuDataPath);

    p1_InuDepth->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showDepth1Shard_ptr,&m1_InuDataPath);
#else
    std::thread show(showDepth1,&m1_InuDataPath);
#endif

    InuDataPath m2_InuDataPath;

    std::string filename2 = "./../../../app/cfg/Params2.yaml";
    InuParams m2_InuParams(filename2);

    InuSensor m2_InuSensor(&m2_InuParams);

    while(!m2_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m2_InuSensor.DeviceInit();

    InuDepth *p2_InuDepth = new InuDepth(&m2_InuSensor,&m2_InuDataPath);

    p2_InuDepth->StartStream();

#ifdef _SHAREDATA_
    std::thread show2(showDepth2Shared_ptr,&m2_InuDataPath);
#else
    std::thread show2(showDepth2,&m2_InuDataPath);
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
    show2.join();

    p1_InuDepth->StopStream();
    m1_InuSensor.DeviceTerminate();
    delete p1_InuDepth;

    p2_InuDepth->StopStream();
    m2_InuSensor.DeviceTerminate();
    delete p2_InuDepth;
    
    return 0;
}
