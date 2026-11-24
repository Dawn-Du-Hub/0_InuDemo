#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuRGB.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

bool running = true;
bool savergb = false;
void showRGB(InuDataPath* p_inuDataPath)
{
    while (running)
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
#ifdef _MSC_VER
                    cv::cvtColor(img, imgshow, CV_RGBA2BGR);
#else
                    cv::cvtColor(img, imgshow, CV_BGRA2RGB);
#endif
                    break;
                case eYVU422:
                    img = cv::Mat(image_height, image_width, CV_8UC2, (rgbframe.frame_data + offset));
                    cv::cvtColor(img, imgshow, CV_YUV2BGR_YUYV);
                    break;
                default:
                    printf("Got unrecognised format: %d\n", rgbframe.compressed);
                    break;
                }

                if(savergb)
                {
                    std::ostringstream RfileName;
                    RfileName << "logs/RGB_" <<image_width<<"X"<<image_height<<"_" << rgbframe.frame_id <<"_"<< rgbframe.timestamp << ".jpg";
                    savergb = false;
                    cv::imwrite(RfileName.str(), imgshow);
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
    while (running)
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
                    img = cv::Mat(image_height, image_width, CV_8UC4, p_frame->GetData());
#ifdef _MSC_VER
                    cv::cvtColor(img, imgshow, CV_BGRA2BGR);
#else
                    cv::cvtColor(img, imgshow, CV_BGRA2BGR);
#endif
                    break;
                case eRGBA:
                    img = cv::Mat(image_height, image_width, CV_8UC4, p_frame->GetData());
#ifdef _MSC_VER
                    cv::cvtColor(img, imgshow, CV_RGBA2BGR);
#else
                    cv::cvtColor(img, imgshow, CV_BGRA2RGB);
#endif
                    break;
                case eYVU422:
                    img = cv::Mat(image_height, image_width, CV_8UC2, p_frame->GetData());
                    cv::cvtColor(img, imgshow, CV_YUV2BGR_YUYV);
                break;
                default:
                    printf("Got unrecognised format: %d\n", p_frame->Format());
                    break;
                }

                if(savergb)
                {
                    std::ostringstream RfileName;
                    RfileName << "logs/RGB_" <<image_width<<"X"<<image_height<<"_" << p_frame->FrameIndex <<"_"<< p_frame->Timestamp/1000000 << ".jpg";
                    savergb = false;
                    cv::imwrite(RfileName.str(), imgshow);
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
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    InuRGB *p_InuRGB = new InuRGB(&m_InuSensor,&m_InuDataPath);

    p_InuRGB->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showRGB2,&m_InuDataPath);
#else
    std::thread show(showRGB,&m_InuDataPath);
#endif

    // comment setRGBExposeParams, if you donot want to use manual setting RGB exposure
    m_InuSensor.setRGBExposeParams();

    while (1) {
        int choice = 1;
        std::cout << "**********" << std::endl;
        std::cout << "Exit: 0" << std::endl;
        std::cout << "Save: 1" << std::endl;
        std::cout << "**********" << std::endl;
        std::cin >> choice;

        if (choice == 0) {
            break;
        }

        switch (choice)
         {
            case 1:
                savergb = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    };
    running = false;

   show.join();

    p_InuRGB->StopStream();
    delete p_InuRGB;

    m_InuSensor.DeviceTerminate();

    return 0;
}
