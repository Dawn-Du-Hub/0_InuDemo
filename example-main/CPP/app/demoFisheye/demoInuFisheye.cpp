#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuFE.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

std::string winname = "Fisheye ";

bool running = true;
void showFisheye(InuDataPath* p_inuDataPath)
{
    uint64_t lastTimeStamp = 0;
    uint64_t currentTimeStamp = 0;
    float fps = 0;
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_fisheyeFrameMutex);
        p_inuDataPath->m_fisheyeCV.wait(locker);
        FishEyeFrame fisheyeframe = p_inuDataPath->getLatestFishEyeFrame();
        locker.unlock();

        currentTimeStamp = fisheyeframe.timestamp;
        fps = 1000./(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;


        if (fisheyeframe.frame_data)
        {
            int image_height = fisheyeframe.height;
            int image_width = fisheyeframe.width;
            int offset = int(fisheyeframe.totalsize - fisheyeframe.datasize);
            
            cv::Mat img;
            
            img = cv::Mat(image_height, image_width, CV_8UC4, (unsigned char*)(fisheyeframe.frame_data + offset));

            cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            cv::resizeWindow(winname, image_width, image_height);
            cv::imshow(winname, img);
            cv::waitKey(1);
            delete[](fisheyeframe.frame_data);
        }
    };
}

#ifdef _SHAREDATA_
void showFisheye2(InuDataPath* p_inuDataPath)
{
    uint64_t lastTimeStamp = 0;
    uint64_t currentTimeStamp = 0;
    float fps = 0;
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_fisheyeFrameMutex);
        p_inuDataPath->m_fisheyeCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getFisheyeshareFrame();
        locker.unlock();

        currentTimeStamp = p_frame->Timestamp;
        fps = 1000./(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;

        if (p_frame)
        {
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            
            cv::Mat img;
            
            img = cv::Mat(image_height, image_width, CV_8UC4, p_frame->GetData());

            cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            cv::resizeWindow(winname, image_width, image_height);
            cv::imshow(winname, img);
            cv::waitKey(1);
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
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    InuFisheye *p_InuFisheye = new InuFisheye(&m_InuSensor,&m_InuDataPath);
    p_InuFisheye->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showFisheye2,&m_InuDataPath);
#else
    std::thread show(showFisheye,&m_InuDataPath);
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

    p_InuFisheye->StopStream();
    m_InuSensor.DeviceTerminate();
    delete p_InuFisheye;

    return 0;
}
