#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuDepth.h"

#include "inudepthquality.h"

using namespace cv;
using namespace Inuchip;

bool show_depth_running = true;
bool show_pcl_running = true;

struct Params{
    //
    double distance;    //GD
    float range1 = 0.001;  //m
    float range2 = 0.002;
    float range3 = 0.003;
    float range4 = 0.004;
    // sensors
    float x0_pcl = 0;
    float y0_pcl = 0;
    float fx_pcl = 0;
    float fy_pcl = 0;
    //depth
    int image_height;
    int image_width;
    int ROI_x1 = 0;
    int ROI_y1 = 0;
    int ROI_x2 = 0;
    int ROI_y2 = 0;
    // pcl
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;              // orignal cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr planeCloud;         // orignal cloud
    double theta_x;                                         // angle of x axis
    double theta_y;
    double theta_z;
    // plane coefficient
    double x_factor;                                            // coefficients of the plane equation
    double y_factor;
    double z_factor;
    double b_factor;
    //result
    double meanDist;
    double zAccuracy;
    double RMS;
    double totalPoints;                                     // number of cloudpoint
    double totalpixel;                                      // number of image pixel
    double fillrate;
    int littleRange1;
    int betweenRange1A2;
    int betweenRange2A3;
    int betweenRange3A4;
    int bigthan4;
};

Params params;
std::ostringstream fileName;
bool save_pointcloud = false;
bool show_result = false;
int diff[5] = { 0 }; // rectangle of ROI
std::mutex mMutexPCL;
std::mutex mMutexMouseCallBack;
int firstX = 0;
int firstY = 0;
int clickStart = 0;

void on_mouse_callback(int event, int x, int y, int flags, void* ustc)
{
    unique_lock<mutex> lock(mMutexMouseCallBack);

    if (cv::EVENT_MOUSEMOVE == event && clickStart)
    {
        ((int*)ustc)[2] = x;
        ((int*)ustc)[3] = y;
    }

    if (cv::EVENT_LBUTTONDOWN == event)
    {
        clickStart = 1;
        ((int*)ustc)[4] = 0xAAA;

        ((int*)ustc)[0] = x;
        ((int*)ustc)[1] = y;
        ((int*)ustc)[2] = x;
        ((int*)ustc)[3] = y;
    }
    if (cv::EVENT_LBUTTONUP == event)
    {
        clickStart = 0;
    }

    if (cv::EVENT_RBUTTONDOWN == event)
    {
        ((int*)ustc)[4] = 0xBBB;
    }
}

void drawRectangle(cv::Mat& d,cv::Point pointStart,cv::Point pointEnd)
{
    cv::rectangle(d, pointStart, pointEnd, Scalar(0, 255, 0), 2);
}

void clearScreen() 
{
    #ifdef _WIN32
        system("cls");  // Windows
    #else
        system("clear");  // macOS 和 Linux
    #endif
}

void pub_pcl(const cv::Mat &dep, const cv::Mat &imgWeb,bool bOrder,int rinter,int cinter,int frameID)
{
    float fScale = 0.001F;

    int rtotal = 0;
    int ctotal = 0;
    unique_lock<mutex> lock(mMutexPCL);

    if (imgWeb.empty())
    {
        params.cloud->clear();
        int y0,x0,rows,cols ;
        if(params.ROI_x1 == 0 && params.ROI_x2 == 0 && params.ROI_y1 == 0 && params.ROI_y2 == 0)
        {
            y0 = 0,x0 = 0,rows = dep.rows,cols = dep.cols;
        }
        else{
            y0 = params.ROI_y1, x0 = params.ROI_x1, rows = params.ROI_y2, cols = params.ROI_x2;
        }

        for (int y=y0; y < rows; y+=rinter)
        {
            rtotal ++;
            for (int x = x0; x < cols; x+=cinter){
                if (y==0) ctotal++;

                int z = dep.at<ushort>(y,x);

                if (z < 10 || z > 65530){
                    if (!bOrder)
                        continue;
                    else
                        z = 0;
                }

                int xw = z * (x - params.x0_pcl) /params.fx_pcl;
                int yw = z * (y - params.y0_pcl) /params.fy_pcl;
                params.cloud->points.emplace_back(pcl::PointXYZ(xw*fScale,yw*fScale,z*fScale));
            }
        }

        params.cloud->width = params.cloud->size();
        params.cloud->height = 1;
        params.cloud->is_dense = false;

        if(save_pointcloud){
            //fileName << "saved.ply";
            pcl::io::savePLYFileBinary(fileName.str(), *params.cloud);
            save_pointcloud = false;
        }
    }
}

void fitPlane()
{
    //RANSAC
    pcl::SampleConsensusModelPlane<pcl::PointXYZ>::Ptr model_plane(new pcl::SampleConsensusModelPlane<pcl::PointXYZ>(params.cloud));
    pcl::RandomSampleConsensus<pcl::PointXYZ> ransac(model_plane);
    ransac.setDistanceThreshold(0.01);
    ransac.computeModel();

    std::vector<int> inliers;
    ransac.getInliers(inliers);

    //module 
    Eigen::VectorXf _params,coefficient;
    ransac.getModelCoefficients(_params);
    model_plane->optimizeModelCoefficients(inliers, _params,coefficient);

    params.x_factor = coefficient[0];
    params.y_factor = coefficient[1];
    params.z_factor = coefficient[2];
    params.b_factor = coefficient[3];

    float fit_norm = 1;// sqrt(x_factor * x_factor + y_factor * y_factor + z_factor * z_factor);

    params.theta_x = acos(params.x_factor / fit_norm) * (180.0 / M_PI)-90;
    if (params.theta_x >= 90) { params.theta_x -= 180; }
    params.theta_y = acos(params.y_factor / fit_norm) * (180.0 / M_PI)-90;
    if (params.theta_y >= 90) { params.theta_y -= 180; }
    params.theta_z = acos(params.z_factor / fit_norm) * (180.0 / M_PI);
    //if (params.theta_z >= 90) { params.theta_z -= 90; }
}

void generatePlaneCloud()
{
    float startx = 0.8;
    float starty = 0.7;
    float pointstep = 0.02;// 0.01;

    params.planeCloud->clear();

    for (float x = -startx; x <= startx; x += pointstep)
    {
        for (float y = -starty; y <= starty; y += pointstep)
        {
            float z = -(params.x_factor * x + params.y_factor * y + params.b_factor) / params.z_factor;
            params.planeCloud->points.push_back(pcl::PointXYZ(x, y, z));
        }
    }
    params.planeCloud->width = params.planeCloud->size();
    params.planeCloud->height = 1;
    params.planeCloud->is_dense = false;

    if(save_pointcloud){
        pcl::io::savePLYFileBinary("plane.ply", *params.planeCloud);
    }
}

void showResult()
{
    double sum = 0;
    int totalPoints = params.cloud->points.size();
    double squaresSum = 0;

    std::vector<float> deviations;
    deviations.resize(totalPoints);

    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
    for (int i = 0; i < totalPoints; i++)
    {
        sum += params.cloud->points[i].z;

        deviations[i] = abs(params.x_factor * params.cloud->points[i].x + params.y_factor * params.cloud->points[i].y + params.z_factor * params.cloud->points[i].z + params.b_factor) ;
        
        squaresSum += pow(deviations[i], 2);

        if (deviations[i] <= params.range1) {
            sum1++;
        }
        else if (deviations[i] > params.range1 && deviations[i] <= params.range2) {
            sum2++;
        }
        else if (deviations[i] > params.range2 && deviations[i] <= params.range3) {
            sum3++;
        }
        else if (deviations[i] > params.range3 && deviations[i] <= params.range4) {
            sum4++;
        }
        else {
            sum5++;
        }
    }
    params.littleRange1 = sum1;
    params.betweenRange1A2 = sum2;
    params.betweenRange2A3 = sum3;
    params.betweenRange3A4 = sum4;
    params.bigthan4 = sum5;

    params.meanDist = sum / totalPoints;
    params.zAccuracy = abs(params.distance - params.meanDist);
    params.RMS = sqrt(squaresSum / totalPoints);
    double totalpixel = ((params.ROI_x2 - params.ROI_x1 ) * (params.ROI_y2 - params.ROI_y1 ));
    params.totalPoints = totalPoints;
    params.totalpixel = totalpixel;
    params.fillrate = (double(totalPoints)) / totalpixel;

    return;
}

void showPCL(InuDataPath* p_inuDataPath)
{
    pcl::visualization::PCLVisualizer viewer("viewer");

    viewer.setBackgroundColor(0,0,0);
    float pclUnit = 1.0;
    float theta = M_PI ;
    Eigen::Affine3f transform_x = Eigen::Affine3f::Identity();
    transform_x.rotate(Eigen::AngleAxisf(theta, Eigen::Vector3f::UnitX()));	
    viewer.addCoordinateSystem(pclUnit,transform_x);

    float theta_y = M_PI ;
    Eigen::Affine3f transform_y = Eigen::Affine3f::Identity();
    transform_y.rotate(Eigen::AngleAxisf(theta_y, Eigen::Vector3f::UnitY()));	

    float theta_z = M_PI;
    Eigen::Affine3f transform_z = Eigen::Affine3f::Identity();
    transform_z.rotate(Eigen::AngleAxisf(theta_z, Eigen::Vector3f::UnitZ()));

    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_show(new pcl::PointCloud<pcl::PointXYZ>);

    viewer.addPointCloud(cloud_show, "show cloud");
    viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 1.0, 0.0, 0.0, "show cloud"); // 红色
    viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "show cloud");

    viewer.addPointCloud(params.planeCloud, "plane cloud");
    viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, .0, 1.0, 0.0, "plane cloud"); // 红色
    viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "plane cloud");

    char name_line[100];

    sprintf(name_line, "Angle with x-asix: %.1f \n ", params.theta_x);
    viewer.addText(name_line, 10, 80, 16, 1.0, 1.0, 1.0, "x_asix_id");

    sprintf(name_line, "Angle with y-asix: %.1f \n ", params.theta_y);
    viewer.addText(name_line, 10, 60, 16, 1.0, 1.0, 1.0, "y_asix_id");

    sprintf(name_line, "Angle with z-asix: %.1f \n ", params.theta_z);
    viewer.addText(name_line, 10, 40, 16, 1.0, 1.0, 1.0, "z_asix_id");

    //sprintf(name_line, "mean distance:%.1f \n ", params.meanDist);
    //viewer.addText(name_line, 10, 200, 16, 1.0, 1.0, 1.0, "meanDist_id");

    while (show_pcl_running)
    {
        unique_lock<mutex> lock(mMutexPCL);
        // if (!(pcl::io::loadPLYFile(fileName.str(), *params.cloud)))
        // {
        //     std::cout << "load from file!" << std::endl;
        // }
        fitPlane();
        generatePlaneCloud();

        if (show_result)
        {
            showResult();

            sprintf(name_line, "RMS: %.4f %%\n ", 100*params.RMS);
            viewer.addText(name_line, 10, 240, 16, 1.0, 1.0, 1.0, "RMS_id");

            sprintf(name_line, "fillRate(%d/%d): %.2f %%\n ", int(params.totalPoints),int(params.totalpixel),params.fillrate*100);
            viewer.addText(name_line, 10, 220, 16, 1.0, 1.0, 1.0, "fillrate_id");

            sprintf(name_line, "mean distance: %.4f \n ", params.meanDist);
            viewer.addText(name_line, 10, 200, 16, 1.0, 1.0, 1.0, "meanDist_id");

            sprintf(name_line, "[0 ~ %.1f mm] : %d (%.1f %%)\n ", params.range1*1000,params.littleRange1,100*params.littleRange1/params.totalPoints);
            viewer.addText(name_line, 10, 180, 16, 1.0, 1.0, 1.0, "Distbetween01_id");

            sprintf(name_line, "[%.1f ~ %.1f mm] : %d (%.1f %%)\n ", params.range1*1000, params.range2*1000,params.betweenRange1A2,100*params.betweenRange1A2/params.totalPoints);
            viewer.addText(name_line, 10, 160, 16, 1.0, 1.0, 1.0, "Distbetween12_id");

            sprintf(name_line, "[%.1f ~ %.1f mm] : %d (%.1f %%)\n ", params.range2*1000, params.range3*1000,params.betweenRange2A3,100*params.betweenRange2A3/params.totalPoints);
            viewer.addText(name_line, 10, 140, 16, 1.0, 1.0, 1.0, "Distbetween23_id");

            sprintf(name_line, "[%.1f ~ %.1f mm] : %d (%.1f %%)\n ", params.range3*1000, params.range4*1000,params.betweenRange3A4,100*params.betweenRange3A4/params.totalPoints);
            viewer.addText(name_line, 10, 120, 16, 1.0, 1.0, 1.0, "Distbetween34_id");

            sprintf(name_line, "[%.1f ~  mm] : %d (%.1f %%)\n ", params.range4*1000, params.bigthan4,100*params.bigthan4/params.totalPoints);
            viewer.addText(name_line, 10, 100, 16, 1.0, 1.0, 1.0, "Distbetween5-_id");


            show_result = false;
        }
        pcl::transformPointCloud(*params.cloud,*cloud_show,transform_x);
        pcl::transformPointCloud(*params.planeCloud,*params.planeCloud,transform_x);

        sprintf(name_line, "Angle with x-asix: %.1f \n ", params.theta_x);
        viewer.updateText(name_line, 10, 80, 16, 1.0, 1.0, 1.0, "x_asix_id");

        sprintf(name_line, "Angle with y-asix: %.1f \n ", params.theta_y);
        viewer.updateText(name_line, 10, 60, 16, 1.0, 1.0, 1.0, "y_asix_id");

        sprintf(name_line, "Angle with z-asix: %.1f \n ", params.theta_z);
        viewer.updateText(name_line, 10, 40, 16, 1.0, 1.0, 1.0, "z_asix_id");

       // viewer.addText3D("3D Text", pcl::PointXYZ(1.0, 1.0, 1.0), 0.1, 1.0, 1.0, 1.0, "3d_text_id");

        viewer.updatePointCloud<pcl::PointXYZ>(cloud_show, "show cloud");
        viewer.updatePointCloud<pcl::PointXYZ>(params.planeCloud, "plane cloud");

        lock.unlock();
        viewer.spinOnce(100);
    };
    return;
}

void showDepth(InuDataPath* p_inuDataPath)
{
    while (show_depth_running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        DepthFrame depthframe = p_inuDataPath->getLatestDepthFrame();
        locker.unlock();

        if (depthframe.frame_data)
        {
            int image_height = depthframe.height;
            int image_width = depthframe.width;
            params.image_height = image_height;
            params.image_width = image_width;
            int offset = int(depthframe.totalsize - depthframe.datasize);
            cv::Mat display,pclMatRGB;

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, (unsigned char*)(depthframe.frame_data + offset));
                depthorg.copyTo(display);

                pub_pcl(depthorg, pclMatRGB,false,1,1,depthframe.frame_id);
                #if 1
                    depthorg.copyTo(display);

                    display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                    applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                    std::string netname = "Depth" ;
                    cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                    cv::setMouseCallback(netname, on_mouse_callback, (void*)(&diff));

                    cv::resizeWindow(netname, image_width, image_height);
                    cv::imshow(netname, display);

                    cv::waitKey(1);
                #endif
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
    while (show_depth_running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_depthFrameMutex);
        p_inuDataPath->m_depthCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getDepthshareFrame();
        locker.unlock();

        if (p_frame)
        {
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            params.image_height = image_height;
            params.image_width = image_width;
            cv::Mat display,pclMatRGB;

            try {
                cv::Mat depthorg(image_height, image_width, CV_16UC1, p_frame->GetData());
                depthorg.copyTo(display);

                display.convertTo(display, CV_8UC1, 0.1/*scale*/, 0);
                applyColorMap(display, display, cv::COLORMAP_JET); //COLORMAP_RAINBOW   COLORMAP_HOT

                std::string netname = "Depth" ;
                cv::namedWindow(netname, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
                cv::setMouseCallback(netname, on_mouse_callback, (void*)(&diff));
                if (diff[4] == 0xAAA ){
                    drawRectangle(display,cv::Point(diff[0],diff[1]),cv::Point(diff[2],diff[3]));
                }

                if (diff[4] == 0xBBB ){
                    params.ROI_x1 = (diff[0]<diff[2])?diff[0]:diff[2];
                    params.ROI_x2 = (diff[0]>diff[2])?diff[0]:diff[2];
                    params.ROI_y1 = (diff[1]<diff[3])?diff[1]:diff[3];
                    params.ROI_y2 = (diff[1]>diff[3])?diff[1]:diff[3];
                    show_depth_running = false;
                    save_pointcloud = true;
                }

                pub_pcl(depthorg, pclMatRGB,false,1,1,p_frame->FrameIndex);

                cv::resizeWindow(netname, image_width, image_height);
                cv::imshow(netname, display);

                cv::waitKey(1);
            }
            catch(Exception  e){
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

    std::string filename = "./../../../../app/cfg/Params.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);

    params.cloud = pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>());
    params.planeCloud = pcl::PointCloud<pcl::PointXYZ>::Ptr (new pcl::PointCloud<pcl::PointXYZ>());
    fileName << "saved.ply";

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    if(m_InuParams.inuDepthSetting.postProcessing & 1 == 1)  //Registration
    {
        params.x0_pcl = m_InuSensor.mOD.centerWeb[0];
        params.y0_pcl = m_InuSensor.mOD.centerWeb[1];
        params.fx_pcl = m_InuSensor.mOD.focalWeb[0];
        params.fy_pcl = m_InuSensor.mOD.focalWeb[1];
    }
    else{
        params.x0_pcl = m_InuSensor.mOD.centerDepth[0];
        params.y0_pcl = m_InuSensor.mOD.centerDepth[1];
        params.fx_pcl = m_InuSensor.mOD.focalDepth[0];
        params.fy_pcl = m_InuSensor.mOD.focalDepth[1];
    }

    InuDepth *p_InuDepth = new InuDepth(&m_InuSensor,&m_InuDataPath);

    p_InuDepth->StartStream();

#ifdef _SHAREDATA_
    std::thread show(showDepth2,&m_InuDataPath);
    std::thread showpcl(showPCL,&m_InuDataPath);
#else
    std::thread show(showDepth,&m_InuDataPath);
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    clearScreen();

    while (1) {
        std::string choice;
        std::cout << "******************************" << std::endl;
        std::cout << "q: Exit" << std::endl;
        std::cout << "s: save pointcloud!" << std::endl;
        std::cout << "c: check pointcloud!"<<std::endl;
        std::cout << "******************************" << std::endl;
        std::cin >> choice;
        if (choice == "q"){
            break;
        }else if(choice == "s"){
            save_pointcloud = true;
            clearScreen();
        }
        else if(choice == "c"){
        //    pclanalysis  m_pclanalysis;
        //    m_pclanalysis.on_calc();
        //    m_pclanalysis.on_showPCL();
            show_result = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    show_depth_running = false;
    show_pcl_running = false;

    show.join();
    showpcl.join();

    p_InuDepth->StopStream();
    m_InuSensor.DeviceTerminate();

    delete p_InuDepth;

    return 0;
}
