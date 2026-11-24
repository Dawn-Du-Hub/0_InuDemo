#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuPCL.h"


#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/cloud_viewer.h>

using namespace Inuchip;

bool running = true;
void showPCL(InuDataPath* p_inuDataPath)
{
    pcl::visualization::CloudViewer *pviewer = new pcl::visualization::CloudViewer("Cloud Viewer");
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_show(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudrgb_show(new pcl::PointCloud<pcl::PointXYZRGBA>);

    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_pclFrameMutex);
        p_inuDataPath->m_pclCV.wait(locker);
        PCLFrame pclframe = p_inuDataPath->getLatestPCLFrame();
        locker.unlock();
        int format = pclframe.height; // height is format
        uint32_t numOfPoints = pclframe.width; //width is the num of points
        int offset = int(pclframe.totalsize - pclframe.datasize);

        if (pclframe.frame_data)
        {
            if (format == 1) 
            {
                 //shortpoints
                cloud_show->points.resize(numOfPoints);
                int16_t *p3DshortData = (int16_t*) (pclframe.frame_data + offset);
                for (uint32_t i = 0; i < numOfPoints; i++)
                {
                    cloud_show->points[i].x = ((float)(p3DshortData[i*3]))/1000;
                    cloud_show->points[i].y = -((float)(p3DshortData[i*3+1]))/1000;
                    cloud_show->points[i].z = ((float)(p3DshortData[i*3+2]))/1000;
                }
                pviewer->showCloud(cloud_show);
            }
            if (format == 2) 
            {
                 //shortpoints
                cloudrgb_show->points.resize(numOfPoints);
                float* p3DData = (float*) (pclframe.frame_data + offset);
                uint8_t* p3DDataRGB = (pclframe.frame_data + offset);

                for (uint32_t i = 0; i < numOfPoints; i++)
                {
                    cloudrgb_show->points[i].x = ((float)(p3DData[i*4]))/1000;
                    cloudrgb_show->points[i].y = -((float)(p3DData[i*4+1]))/1000;
                    cloudrgb_show->points[i].z = -((float)(p3DData[i*4+2]))/1000;
                    cloudrgb_show->points[i].r = p3DDataRGB[i*16+12];
                    cloudrgb_show->points[i].g = p3DDataRGB[i*16+13];
                    cloudrgb_show->points[i].b = p3DDataRGB[i*16+14];
                    cloudrgb_show->points[i].a = p3DDataRGB[i*16+15];
                }
                pviewer->showCloud(cloudrgb_show);
            }

            delete[](pclframe.frame_data);
        }
    };

    delete pviewer;
}

#ifdef _SHAREDATA_
void showPCL2(InuDataPath* p_inuDataPath)
{
    pcl::visualization::CloudViewer *pviewer = new pcl::visualization::CloudViewer("Cloud Viewer");
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_show(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudrgb_show(new pcl::PointCloud<pcl::PointXYZRGBA>);

    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_pclFrameMutex);
        p_inuDataPath->m_pclCV.wait(locker);
        std::shared_ptr<const InuDev::CPointCloudFrame> pframe = p_inuDataPath->getPCLshareFrame();
        locker.unlock();
        int format = (InuDev::CPointCloudFrame::EFormat)pframe->GetFormat(); // height is format
        uint32_t numOfPoints = pframe->GetNumOfPoints();; //width is the num of points
        // int offset = int(pclframe.totalsize - pclframe.datasize);

        if (pframe)
        {
            // e3DPoints:0   e3DShortPoints:1    e3DPointsRGB:2
            if (format == 0)
            {
                cloud_show->points.resize(numOfPoints);
                const CPointCloudFrame::C3DPixel* p3Data = pframe->Get3DData();
                for (uint32_t i = 0; i < numOfPoints; i++)
                {
                    std::memcpy(&(cloud_show->points[i]), &(p3Data[i]), 3 * sizeof(float));
                }
                pviewer->showCloud(cloud_show);
            }
            else if (format == 1) 
            {
                 //shortpoints
                cloud_show->points.resize(numOfPoints);
                int16_t *p3DshortData = (int16_t*) (pframe->Get3DShortData());
                for (uint32_t i = 0; i < numOfPoints; i++)
                {
                    cloud_show->points[i].x = ((float)(p3DshortData[i*3]))/1000;
                    cloud_show->points[i].y = -((float)(p3DshortData[i*3+1]))/1000;
                    cloud_show->points[i].z = ((float)(p3DshortData[i*3+2]))/1000;
                }
                pviewer->showCloud(cloud_show);
            }
            else if (format == 2) 
            {
                 //shortpoints
                cloudrgb_show->points.resize(numOfPoints);
                float* p3DData = (float*) (pframe->Get3DRGBData());
                uint8_t* p3DDataRGB = (uint8_t*) (pframe->Get3DRGBData());

                for (uint32_t i = 0; i < numOfPoints; i++)
                {
                    cloudrgb_show->points[i].x = ((float)(p3DData[i*4]))/1000;
                    cloudrgb_show->points[i].y = -((float)(p3DData[i*4+1]))/1000;
                    cloudrgb_show->points[i].z = -((float)(p3DData[i*4+2]))/1000;
                    cloudrgb_show->points[i].r = p3DDataRGB[i*16+12];
                    cloudrgb_show->points[i].g = p3DDataRGB[i*16+13];
                    cloudrgb_show->points[i].b = p3DDataRGB[i*16+14];
                    cloudrgb_show->points[i].a = p3DDataRGB[i*16+15];
                }
                pviewer->showCloud(cloudrgb_show);
            }

        }
    };

    delete pviewer;
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

    // InuDepth *p_InuDepth = new InuDepth(&m_InuSensor,&m_InuDataPath);
    InuPCL *p_InuPCL = new InuPCL(&m_InuSensor,&m_InuDataPath);

    p_InuPCL->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showPCL2,&m_InuDataPath);
#else
    std::thread show(showPCL,&m_InuDataPath);
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

    p_InuPCL->StopStream();
    m_InuSensor.DeviceTerminate();
    delete p_InuPCL;

    return 0;
}
