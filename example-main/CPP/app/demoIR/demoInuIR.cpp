#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuIR.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

bool running = true;
bool withLeft = true;
double ir_scale = 1.0;
float ir_beta = 10;
std::string winname = "IR"; 

InuSensor *p_inuSensor = nullptr;

void showIR(InuDataPath* p_inuDataPath)
{
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_irFrameMutex);
        p_inuDataPath->m_irCV.wait(locker);
        IRFrame irframe = p_inuDataPath->getLatestIRFrame();
        locker.unlock();
        
        if (irframe.frame_data)
        {
            int image_height = irframe.height;
            int image_width = irframe.width;
            int offset = int(irframe.totalsize - irframe.datasize*2);

            cv::Mat  imgshow;

            cv::Mat LeftIR(image_height, image_width, CV_16UC1, (unsigned char*)(irframe.frame_data + offset));
            cv::Mat RightIR(image_height, image_width, CV_16UC1, (unsigned char*)(irframe.frame_data + offset + irframe.datasize));


            LeftIR.convertTo(LeftIR, CV_8UC1, ir_scale, ir_beta);
            RightIR.convertTo(RightIR, CV_8UC1, ir_scale, ir_beta);

            int show_width = image_width;
            if(withLeft){
                cv::hconcat(LeftIR, RightIR, imgshow);
                show_width += show_width;
            } 
            else
                imgshow = RightIR;

            cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            cv::resizeWindow(winname, show_width, image_height);
            cv::imshow(winname, imgshow);
            cv::waitKey(1);

            delete[](irframe.frame_data);
        }
    };
}

#ifdef _SHAREDATA_
void showIR2(InuDataPath* p_inuDataPath)
{
    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_irFrameMutex);
        p_inuDataPath->m_irCV.wait(locker);
        std::shared_ptr<const InuDev::CStereoImageFrame> p_frame = p_inuDataPath->getIRshareFrame();
        locker.unlock();
        
        if (p_frame)
        {
            int image_height = p_frame->GetRightFrame()->Height();
            int image_width = p_frame->GetRightFrame()->Width();

            cv::Mat  imgshow;

            cv::Mat LeftIR(image_height, image_width, CV_16UC1, p_frame->GetLeftFrame()->GetData());
            cv::Mat RightIR(image_height, image_width, CV_16UC1, p_frame->GetRightFrame()->GetData());

            LeftIR.convertTo(LeftIR, CV_8UC1, ir_scale, ir_beta);
            RightIR.convertTo(RightIR, CV_8UC1, ir_scale, ir_beta);

            if (p_inuSensor->p_InuParams->inuDepthSetting.useROI) {
                cv::Point topleft(p_inuSensor->p_InuParams->inuDepthSetting.irLeft, p_inuSensor->p_InuParams->inuDepthSetting.irTop);
                cv::Point bottomRight(p_inuSensor->p_InuParams->inuDepthSetting.irRight, p_inuSensor->p_InuParams->inuDepthSetting.irBottom);

                cv::rectangle(RightIR, topleft, bottomRight,cv::Scalar(255, 255, 255), 2);
                cv::rectangle(LeftIR, topleft, bottomRight,cv::Scalar(255, 255, 255), 2);
            }

            int show_width = image_width;
            if(withLeft){
                cv::hconcat(LeftIR, RightIR, imgshow);
                show_width += show_width;
            } 
            else
                imgshow = RightIR;

            cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            cv::resizeWindow(winname, show_width, image_height);
            cv::imshow(winname, imgshow);
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
    p_inuSensor = &(m_InuSensor);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    InuIR *p_InuIR = new InuIR(&m_InuSensor,&m_InuDataPath);

    p_InuIR->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showIR2,&m_InuDataPath);
#else
    std::thread show(showIR,&m_InuDataPath);
#endif

    m_InuSensor.setIRExposeParams(); //beacuse SDK 4.36 has bug when setting the Exposure, just comment it temporarily

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
   p_InuIR->StopStream();
   m_InuSensor.DeviceTerminate();
   delete p_InuIR;

    return 0;

}
