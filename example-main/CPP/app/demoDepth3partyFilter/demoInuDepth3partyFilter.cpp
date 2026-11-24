#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"

#include "space_filter.h"
#include "temporal_filter.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;


std::string displayName = "Depth";
std::string displayFilterName = "Depth Filter";

float xCenter, yCenter, fx, fy;
#define SHOWINFO 0
//////////////////////////////////////////////////////////////////////

InuSensor *p_inuSensor = nullptr;

#if SHOWINFO
bool showFPS(cv::Mat& d,float fps)
{
    static char name_line[40];
    uint64_t currentDeviceTime;
    cv::Point position;

    p_inuSensor->inuSensor->GetDeviceTime(currentDeviceTime);
    std::time_t cur_time = currentDeviceTime/1000000;   //us

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

	if (cv::EVENT_MOUSEMOVE == event )
	{
		((int*)ustc)[2] = x;
		((int*)ustc)[3] = y;
	}
}
#endif
//////////////////////////////////////////////////////////////////////

bool running = true;
void showDepth(InuDataPath* p_inuDataPath)
{
    uint64_t lastTimeStamp = 0;
    uint64_t currentTimeStamp = 0;
    float fps = 0;
    Space_Filter sf;
    TemporalFast mTf;

    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        DepthFrame depthframe = p_inuDataPath->getLatestDepthFrame();
        locker.unlock();

        currentTimeStamp = depthframe.timestamp;
        fps = 1000./(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;

        if (depthframe.frame_data)
        {
            int image_height = depthframe.height;
            int image_width = depthframe.width;
            int offset = int(depthframe.totalsize - depthframe.datasize);
            cv::Mat display,depthFilter;

            cv::Mat depthorg(image_height, image_width, CV_16UC1, (unsigned char*)(depthframe.frame_data + offset));
            depthorg.copyTo(display);

            ///////////////////////////////////////////////////////////////////////////////////////
            // here use filter function
            depthorg.copyTo(depthFilter);

            // outlier filter
            depthFilter = sf.process_outlier_remove(depthFilter,0.1);

            // hole filter
            depthFilter = sf.process_hole_fill(depthFilter,25);

            // temporalfast filter
            depthFilter = mTf.process_frame(depthFilter);

            depthFilter.convertTo(depthFilter, CV_8UC1, 0.1/*scale*/, 0);
            applyColorMap(depthFilter, depthFilter, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT
            cv::namedWindow(displayFilterName, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            cv::resizeWindow(displayFilterName, image_width, image_height);
            cv::imshow(displayFilterName, depthFilter);
            ///////////////////////////////////////////////////////////////////////////////////////



            display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
            applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

            cv::namedWindow(displayName, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

            ///////////////////////////////////////////////////////////////////////////////////////
            #if SHOWINFO
            cv::setMouseCallback(displayName, on_mouse_callback, (void*)(&diff));
            showDepthValue(display, cv::Point(diff[2], diff[3]), depthorg.at<unsigned short>(diff[3], diff[2]), depthorg.at<unsigned short>(diff[3], diff[2]));
            showFPS(display,fps);
            #endif
            ///////////////////////////////////////////////////////////////////////////////////////

            cv::resizeWindow(displayName, image_width, image_height);
            cv::imshow(displayName, display);

            cv::waitKey(1);
            delete[](depthframe.frame_data);
        }
    };
}

#ifdef _SHAREDATA_
void showDepth2(InuDataPath* p_inuDataPath)
{
    uint64_t lastTimeStamp = 0;
    uint64_t currentTimeStamp = 0;
    float fps = 0;
    Space_Filter sf;
    TemporalFast mTf;

    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getDepthshareFrame();
        locker.unlock();

        currentTimeStamp = p_frame->Timestamp/1000000;
        fps = 1000./(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;

        if (p_frame)
        {
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            cv::Mat display,depthFilter;

            cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());
            depthorg.copyTo(display);

            ///////////////////////////////////////////////////////////////////////////////////////
            // here use filter function
            depthorg.copyTo(depthFilter);

            // outlier filter
            depthFilter = sf.process_outlier_remove(depthFilter,0.1);

            // hole filter
            depthFilter = sf.process_hole_fill(depthFilter,25);

            // temporalfast filter
            depthFilter = mTf.process_frame(depthFilter);

            depthFilter.convertTo(depthFilter, CV_8UC1, 0.1/*scale*/, 0);
            applyColorMap(depthFilter, depthFilter, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT
            cv::namedWindow(displayFilterName, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
            cv::resizeWindow(displayFilterName, image_width, image_height);
            cv::imshow(displayFilterName, depthFilter);
            ///////////////////////////////////////////////////////////////////////////////////////



            display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
            applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

            cv::namedWindow(displayName, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

            ///////////////////////////////////////////////////////////////////////////////////////
            #if SHOWINFO
            cv::setMouseCallback(displayName, on_mouse_callback, (void*)(&diff));
            showDepthValue(display, cv::Point(diff[2], diff[3]), depthorg.at<unsigned short>(diff[3], diff[2]), depthorg.at<unsigned short>(diff[3], diff[2]));
            showFPS(display,fps);
            #endif
            ///////////////////////////////////////////////////////////////////////////////////////

            cv::resizeWindow(displayName, image_width, image_height);
            cv::imshow(displayName, display);

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

    fx = m_InuSensor.mOD.focalDepth[0];
    fy = m_InuSensor.mOD.focalDepth[1];
    xCenter = m_InuSensor.mOD.centerDepth[0];
    yCenter = m_InuSensor.mOD.centerDepth[1];

    InuDepth *p_InuDepth = new InuDepth(&m_InuSensor,&m_InuDataPath);

    p_InuDepth->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showDepth2,&m_InuDataPath);
#else
    std::thread show(showDepth,&m_InuDataPath);
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

    p_InuDepth->StopStream();
    m_InuSensor.DeviceTerminate();
    delete p_InuDepth;

    return 0;
}
