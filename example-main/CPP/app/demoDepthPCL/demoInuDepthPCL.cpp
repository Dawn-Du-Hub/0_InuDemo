#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 


#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/cloud_viewer.h>

using namespace cv;

using namespace Inuchip;

bool running = true;

float x0_pcl = 0;
float y0_pcl = 0;
float fx_pcl = 0;
float fy_pcl = 0;


pcl::visualization::CloudViewer *pviewer;

void pub_pcl(const cv::Mat &dep, const cv::Mat &imgWeb,bool bOrder,int rinter,int cinter,int frameID)
{
    float fScale = 0.001F;

    int rtotal = 0;
    int ctotal = 0;
    if (imgWeb.empty())
    {
        pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
        
        for (int y = 0; y < dep.rows; y+=rinter)
        {
            rtotal ++;
            for (int x = 0; x < dep.cols; x+=cinter)
            {
                if (y==0) ctotal++;

                int z = dep.at<ushort>(y,x);

                if (z < 10 || z > 65530)
                {
                    if (!bOrder)
                        continue;
                    else
                        z = 0;
                }

                int xw = z * (x - x0_pcl) /fx_pcl;
                int yw = z * (y - y0_pcl) /fy_pcl;
                cloud->points.emplace_back(pcl::PointXYZ(xw*fScale,yw*fScale,z*fScale));
            }
        }
        //     std::ostringstream fileName;
        //     fileName << "capture/PCL_" << frameID << "_" << 0 << ".ply";
        //     pcl::io::savePLYFileBinary(fileName.str(), *cloud);
        pviewer->showCloud(cloud);
    }
}


void showDepth(InuDataPath* p_inuDataPath)
{
    pviewer = new pcl::visualization::CloudViewer("Cloud Viewer");
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
            cv::Mat display,pclMatRGB;

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, (unsigned char*)(depthframe.frame_data + offset));
                depthorg.copyTo(display);

                pub_pcl(depthorg, pclMatRGB,true,1,1,depthframe.frame_id);

                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                std::string netname = "Depth" ;
                cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(netname, image_width, image_height);
                cv::imshow(netname, display);

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
    pviewer = new pcl::visualization::CloudViewer("Cloud Viewer");
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
            cv::Mat display,pclMatRGB;

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());
                depthorg.copyTo(display);

                pub_pcl(depthorg, pclMatRGB,true,1,1,p_frame->FrameIndex);

                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                std::string netname = "Depth" ;
                cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::resizeWindow(netname, image_width, image_height);
                cv::imshow(netname, display);

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

    InuDataPath m_InuDataPath;

    std::string filename = "./../../../app/cfg/Params.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();


    // if(m_InuParams.inuDepthSetting.postProcessing & 1 == 1)  //Registration
    if(m_InuParams.inuDepthSetting.registrationChannel != -1)  //Registration
    {
        x0_pcl = m_InuSensor.mOD.centerWeb[0];
        y0_pcl = m_InuSensor.mOD.centerWeb[1];
        fx_pcl = m_InuSensor.mOD.focalWeb[0];
        fy_pcl = m_InuSensor.mOD.focalWeb[1];
    }
    else{
        x0_pcl = m_InuSensor.mOD.centerDepth[0];
        y0_pcl = m_InuSensor.mOD.centerDepth[1];
        fx_pcl = m_InuSensor.mOD.focalDepth[0];
        fy_pcl = m_InuSensor.mOD.focalDepth[1];
    }

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
