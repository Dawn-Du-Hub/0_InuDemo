#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"
#include "inuRGB.h"


#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

bool running = true;
bool savergb = false;
bool savedepth = false;


float xCenter,yCenter,fx,fy;
InuSensor *p_inuSensor = nullptr;

#define SHOWINFO 1


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

}
#endif
void showDepth(InuDataPath* p_inuDataPath)
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


                if(savedepth)
                {
                    // std::ostringstream rawDepthfileName;
                    // rawDepthfileName << "logs/DEPTH_RAW_" << image_width << "X" << image_height << "_" << depthframe.frame_id << "_" << depthframe.timestamp << ".raw";
                    // std::ofstream outputFile(rawDepthfileName.str(), std::ios::binary);
                    // outputFile.write(reinterpret_cast<const char*>(depthorg.data), depthframe.datasize);
                    // outputFile.flush();

                    std::ostringstream TiffileName;
                    TiffileName << "logs/Depth_" <<image_width<<"X"<<image_height<<"_" << depthframe.frame_id << "_" << depthframe.timestamp << ".tif";
                    cv::imwrite(TiffileName.str(), depthorg);
                    savedepth = false;
                }
                std::string netname = "Depth" ;//+ _name;// std::to_string(tmp->device_id);// _name;
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
            cv::Mat display;

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());
                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT


                if(savedepth)
                {
                    // std::ostringstream rawDepthfileName;
                    // rawDepthfileName << "logs/DEPTH_RAW_" << image_width << "X" << image_height << "_" << depthframe.frame_id << "_" << depthframe.timestamp << ".raw";
                    // std::ofstream outputFile(rawDepthfileName.str(), std::ios::binary);
                    // outputFile.write(reinterpret_cast<const char*>(depthorg.data), depthframe.datasize);
                    // outputFile.flush();

                    std::ostringstream TiffileName;
                    TiffileName << "logs/Depth_" <<image_width<<"X"<<image_height<<"_" << p_frame->FrameIndex << "_" << p_frame->Timestamp << ".tif";
                    cv::imwrite(TiffileName.str(), depthorg);
                    savedepth = false;
                }
                std::string netname = "Depth" ;//+ _name;// std::to_string(tmp->device_id);// _name;
                cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                ///////////////////////////////////////////////////////////////////////////////////////
                    #if SHOWINFO
                    cv::setMouseCallback(netname, on_mouse_callback, (void*)(&diff));
                    showDepthValue(display, cv::Point(diff[2], diff[3]), depthorg.at<unsigned short>(diff[3], diff[2]), depthorg.at<unsigned short>(diff[3], diff[2]));
                    // showFPS(display,fps);
                    #endif
                ///////////////////////////////////////////////////////////////////////////////////////

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
                default:
                    printf("Got unrecognised format: %d\n", p_frame->Format());
                    break;
                }

                if(savergb)
                {
                    std::ostringstream RfileName;
                    RfileName << "logs/RGB_" <<image_width<<"X"<<image_height<<"_" << p_frame->FrameIndex <<"_"<< p_frame->Timestamp << ".jpg";
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

    
#ifdef _SHAREDATA_
    std::thread showD(showDepth2,&m_InuDataPath);
    std::thread showC(showRGB2,&m_InuDataPath);
#else
    std::thread showD(showDepth,&m_InuDataPath);
    std::thread showC(showRGB,&m_InuDataPath);
#endif
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
