#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"
#include "inuRGB.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

#ifndef _SHAREDATA_
#error "Please define _SHAREDATA_ !!!"
#endif

#define SHOWINFO 1

using namespace cv;
using namespace Inuchip;

bool running = true;
bool savergb = false;
bool savedepth = false;

float xCenter,yCenter,fx,fy;
InuSensor *p_inuSensor = nullptr;

std::list<std::pair<uint64_t, cv::Mat>> depth_frames_cache;
std::mutex depth_cache_mutex;

#if SHOWINFO

bool showTimeDiff(cv::Mat& d,uint64_t rgbtime, uint64_t depthtime)
{
    static char name_line[100];

    cv::Point position;
    uint64_t _showrgbtime = rgbtime/1000000;
    uint64_t _showdepthtime = depthtime/1000000;

    CvScalar point_color = { 250, 250, 20 };
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    const double fontScale = 0.5;

    sprintf(name_line, "rgb:%llu | depth:%llu | diff: %d ms", _showrgbtime,_showdepthtime, _showrgbtime - _showdepthtime);
    position = cv::Point(10,20);

    cv::putText(d,
    name_line,
    position,
    fontFace, // Font
    fontScale, // Scale. 2.0 = 2x bigger... was 0.5
    point_color, //obj_class_colors[obj_ind], // Color
    2); // Thickness

    return true;
}

bool showFPS(cv::Mat& d,float fps)
{
    static char name_line[40];
    uint64_t currentDeviceTime;
    cv::Point position;

    p_inuSensor->inuSensor->GetDeviceTime(currentDeviceTime);
    std::time_t cur_time = currentDeviceTime/1000000;   //ms

    std::stringstream  ss_time;
    ss_time << std::put_time(std::localtime(&cur_time), "%Y-%m-%d-%H-%M-%S");

    CvScalar point_color = { 250, 250, 20 };
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    const double fontScale = 0.5;

    sprintf(name_line, "fps:%.1f | ", fps);
    std::strcat(name_line,  ss_time.str().c_str());
    position = cv::Point(10,20);

    cv::putText(d,
    name_line,
    position,
    fontFace, // Font
    fontScale, // Scale. 2.0 = 2x bigger... was 0.5
    point_color, //obj_class_colors[obj_ind], // Color
    2); // Thickness

    return true;
}

int diff[5] = { 0,0,0,0,0 };        //{win_width_diff,win_height_diff,point_x,point_y,mode}

bool showDepthValue(cv::Mat& d, cv::Point p, float z, unsigned short disparity)
{
	char name_line[100];
    CvScalar text_color = { 255, 255, 0 };
    cv::Size size;
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.5;
    int thickness = 2;

    float x3d = ((float)p.x - xCenter)*z/fx;
    float y3d = ((float)p.y - yCenter)*z/fy;

    sprintf(name_line, "(%d, %d) (%0.2f %0.2f %0.2f) mm %#06x", p.x, p.y, x3d,y3d,z, disparity);

    size = cv::getTextSize(name_line, fontFace, fontScale, thickness, 0);

    if (p.x + size.width > d.cols) {
        p.x -= p.x + size.width - d.cols;
    }

    if (p.y + size.height > d.rows) {
        p.y -= p.y + size.height - d.rows;
    }

    cv::Point circlePoint(xCenter, yCenter);
    CvScalar circle_color = { 0, 0, 255 };
    cv::circle(d,circlePoint,3,circle_color,cv::FILLED);

    cv::putText(d,
        name_line,
        p,
        fontFace, // Font
        fontScale, // Scale. 2.0 = 2x bigger... was 0.5
        text_color, //obj_class_colors[obj_ind], // Color
        2); // Thickness
    return true;
}

std::mutex mMutexMouseCallBack;

void on_mouse_callback(int event, int x, int y, int flags, void* ustc)
{
    unique_lock<mutex> lock(mMutexMouseCallBack);

	if (cv::EVENT_MOUSEMOVE == event )
	{
		((int*)ustc)[2] = x;
		((int*)ustc)[3] = y;
	}

}
#endif


void showDepth2(InuDataPath* p_inuDataPath)
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

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());

                std::lock_guard<std::mutex> lock(depth_cache_mutex);
                if (depth_frames_cache.size() > 5) {
                    depth_frames_cache.pop_front();
                }
                depth_frames_cache.emplace_back(p_frame->Timestamp, depthorg);

            }catch(Exception  e){
                continue;
            }
        }
    };
}

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
            uint64_t rgb_timestamp = p_frame->Timestamp;
            uint64_t best_match_timestamp = 0;
            cv::Mat best_match_depth;
            int64_t min_time_diff = INT64_MAX;

            {
                std::lock_guard<std::mutex> lock(depth_cache_mutex);

                for (auto it = depth_frames_cache.begin(); it != depth_frames_cache.end(); ) {
                    int64_t time_diff = static_cast<int64_t>(rgb_timestamp) - static_cast<int64_t>(it->first);
                    int64_t abs_time_diff = std::abs(time_diff);

                    // if time diff is big than 0.5s, remove the depth from the cache
                    if (abs_time_diff > 500000000) {
                        it = depth_frames_cache.erase(it);
                        continue;
                    }

                    // find the best match depth
                    if (abs_time_diff < min_time_diff) {
                        min_time_diff = abs_time_diff;
                        best_match_timestamp = it->first;
                        best_match_depth = it->second.clone();
                    }

                    ++it;
                }
            }

            if (min_time_diff > 150000000) {
                continue;
            }

            // find the best match depth, just show it

            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            cv::Mat imgrgb;

            try {

                cv::Mat img;

                switch (p_frame->Format())
                {
                case eRGB:
                    img = cv::Mat(image_height, image_width, CV_8UC3, p_frame->GetData());
                    imgrgb = img;
                    break;
                case eBGR:
                    img = cv::Mat(image_height, image_width, CV_8UC3, p_frame->GetData());
                    imgrgb = img;
                    break;
                case eBGRA:
                    img = cv::Mat(image_height, image_width, CV_8UC4, p_frame->GetData());
#ifdef _MSC_VER
                    cv::cvtColor(img, imgrgb, CV_BGRA2BGR);
#else
                    cv::cvtColor(img, imgrgb, CV_BGRA2BGR);
#endif
                    break;
                case eRGBA:
                    img = cv::Mat(image_height, image_width, CV_8UC4, p_frame->GetData());
#ifdef _MSC_VER
                    cv::cvtColor(img, imgrgb, CV_RGBA2BGR);
#else
                    cv::cvtColor(img, imgrgb, CV_BGRA2RGB);
#endif
                    break;
                case eYVU422:
                    img = cv::Mat(image_height, image_width, CV_8UC2, p_frame->GetData());
                    cv::cvtColor(img, imgrgb, CV_YUV2BGR_YUYV);
                default:
                    printf("Got unrecognised format: %d\n", p_frame->Format());
                    break;
                }

                showTimeDiff(imgrgb,rgb_timestamp,best_match_timestamp);

                std::string winname = "RGB";

                cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(winname, image_width, image_height);
                cv::imshow(winname, imgrgb);

                cv::Mat imgdepth;

                best_match_depth.convertTo(imgdepth,CV_8UC1,0.015,0);
                applyColorMap(imgdepth, imgdepth, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                winname = "Depth";

                cv::namedWindow(winname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(winname, image_width, image_height);
                cv::imshow(winname, imgdepth);

                if(savergb)
                {
                    std::ostringstream RfileName;
                    RfileName << "logs/RGB_" <<image_width<<"X"<<image_height<<"_" << p_frame->FrameIndex <<"_"<< p_frame->Timestamp << ".jpg";
                    savergb = false;
                    cv::imwrite(RfileName.str(), imgrgb);
                }
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

int main(int argc, char* argv[])
{
    spdlog_init();

    InuDataPath m_InuDataPath;

    std::string filename = "./../../../app/cfg/Params_D2C.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);
    p_inuSensor = &(m_InuSensor);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    // if D2C, should use web lens param
    fx = m_InuSensor.mOD.focalWeb[0];
    fy = m_InuSensor.mOD.focalWeb[1];

    xCenter = m_InuSensor.mOD.centerWeb[0];
    yCenter = m_InuSensor.mOD.centerWeb[1];

    InuDepth *p_InuDepth = new InuDepth(&m_InuSensor,&m_InuDataPath);
    InuRGB *p_InuRGB = new InuRGB(&m_InuSensor,&m_InuDataPath);

    p_InuDepth->StartStream();
    p_InuRGB->StartStream();

    
    std::thread showD(showDepth2,&m_InuDataPath);
    std::thread showC(showRGB2,&m_InuDataPath);

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
                savedepth = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    };
    running = false;

    showD.join();
    showC.join();

    p_InuDepth->StopStream();
    delete p_InuDepth;

    p_InuRGB->StopStream();
    delete p_InuRGB;

    m_InuSensor.DeviceTerminate();

    return 0;
}
