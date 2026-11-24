#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuFE.h"
#include "inuFeature.h"
#include "inuSlam.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;

#define SAVEFILE  1

#if SAVEFILE
std::ofstream outputFile_6DOF("Inu_6DOF.csv");
std::ofstream outputFile_feature("Inu_features.csv");
#endif

std::string winname = "Feature";
InuFeature *p_InuFeature;
InuSlam *p_InuSlam;

bool running = true;
bool withLeft = true;

char name_line_T[100];
char name_line_Q[100];
void showSlam(cv::Mat& d, SlamData& slamData)
{

    CvScalar text_color = { 255, 255, 0 };
    CvScalar text_color_r = { 0, 0, 255 };
    cv::Size size;
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.5;
    int thickness = 2;
    cv::Point position;

    if(slamData.state == 128 ){
        position = cv::Point(10,20);
        cv::putText(d,
            "Track lost",
            position,
            fontFace, // Font
            fontScale, // Scale. 2.0 = 2x bigger... was 0.5
            text_color_r, //obj_class_colors[obj_ind], // Color
            thickness); // Thickness
    }
   
    position = cv::Point(10,40);
    cv::putText(d,
        name_line_T,
        position,
        fontFace, // Font
        fontScale, // Scale. 2.0 = 2x bigger... was 0.5
        text_color, //obj_class_colors[obj_ind], // Color
        thickness); // Thickness
 
    
    position = cv::Point(10,60);
    cv::putText(d,
        name_line_Q,
        position,
        fontFace, // Font
        fontScale, // Scale. 2.0 = 2x bigger... was 0.5
        text_color, //obj_class_colors[obj_ind], // Color
        thickness); // Thickness
        
    return;
}

void showFisheye(InuDataPath* p_inuDataPath)
{
    uint64_t lastTimeStamp = 0;
    uint64_t currentTimeStamp = 0;
    float fps = 0;

    // my_logger->info("SLAM Info: timestamp x y z q_x q_y q_z q_w");

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

            FeaturesTrackingData featureTrackingData;
            p_InuFeature->getFeaturesTrackingData2(featureTrackingData);
            
            cv::Point pt;
            CvScalar point_color = { 0, 0, 250 };  // b,g,r

            for(auto elem : featureTrackingData.keypoints)
            {
                if (elem.IsRightImage == 0){
                    pt.x = elem.X;
                    pt.y = elem.Y;
                }
                else {
                    pt.x = elem.X+ image_width/2;
                    pt.y = elem.Y;
                }

                if (elem.Confidence > 100) {
                    point_color = { 0,250,0 };
                }
                if (elem.Confidence > 200) {
                    point_color = { 250,0,0 };
                }

                cv::circle(img,pt,1,point_color,cv::FILLED);
            }

            if(!withLeft)
            {
                cv::Rect m_select;
                m_select = cv::Rect(image_width / 2, 0, image_width / 2, image_height);
                img = img(m_select);
                image_width = image_width / 2;
            }

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

    // my_logger->info("SLAM Info: timestamp x y z q_x q_y q_z q_w");

    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_fisheyeFrameMutex);
        p_inuDataPath->m_fisheyeCV.wait(locker);
        std::shared_ptr<const InuDev::CImageFrame> p_frame = p_inuDataPath->getFisheyeshareFrame();
        locker.unlock();

        currentTimeStamp = p_frame->Timestamp;
        fps = 1000./(currentTimeStamp - lastTimeStamp);
        lastTimeStamp = currentTimeStamp;
        int index = p_frame->FrameIndex;

        if (p_frame)
        {
            int image_height = p_frame->Height();
            int image_width = p_frame->Width();
            cv::Mat img;

            img = cv::Mat(image_height, image_width, CV_8UC4, p_frame->GetData());

            FeaturesTrackingData featureTrackingData;
            p_InuFeature->getFeaturesTrackingData2(featureTrackingData);
            SlamData slamData;
            p_InuSlam->getSlamData(slamData);

            sprintf(name_line_T, "T: X=%+8.3f Y=%+8.3f Z=%+8.3f", 
                                    slamData.iPose.mTranslation[0], slamData.iPose.mTranslation[1], slamData.iPose.mTranslation[2]);
            sprintf(name_line_Q, "Q: X=%+8.3f Y=%+8.3f Z=%+8.3f W=%+8.3f", 
                                    slamData.iPose.mQuaternion[0], slamData.iPose.mQuaternion[1], slamData.iPose.mQuaternion[2], slamData.iPose.mQuaternion[3]);

            // my_logger->info("{} {} {} {} {} {} {} {}",slamData.timeStamp, 
            //     slamData.iPose.mTranslation[0], slamData.iPose.mTranslation[1], slamData.iPose.mTranslation[2],
            //     slamData.iPose.mQuaternion[0], slamData.iPose.mQuaternion[1], slamData.iPose.mQuaternion[2], slamData.iPose.mQuaternion[3]);

#if SAVEFILE

            outputFile_6DOF<<slamData.timeStamp<<","<<slamData.frameId<<","<<
                slamData.iPose.mTranslation[0]<<","<<
                slamData.iPose.mTranslation[1]<<","<<
                slamData.iPose.mTranslation[2]<<","<<
                slamData.iPose.mQuaternion[0]<<","<<
                slamData.iPose.mQuaternion[1]<<","<<
                slamData.iPose.mQuaternion[2]<<","<<
                slamData.iPose.mQuaternion[3]<<"\n";

            for(auto elem:featureTrackingData.keypoints){
                outputFile_feature<< featureTrackingData.timeStamp<<","<<featureTrackingData.frameId<<"," << featureTrackingData.keypointnumber<<","<< featureTrackingData.leftnumber<<","<< featureTrackingData.rightnumber<<","<< 
                elem.UniqId<<","<<elem.Confidence<<","<<elem.IsRightImage<<","<<elem.PatternSize<<","<< elem.X<<","<<elem.Y<<","<<elem.Angle<<","<<
                elem.Descriptor[0]<<","<<elem.Descriptor[1]<<","<<elem.Descriptor[2]<<","<<elem.Descriptor[3]<<","<<
                elem.Descriptor[4]<<","<<elem.Descriptor[5]<< ","<<elem.Descriptor[6]<<","<<elem.Descriptor[7]<< "," <<
                elem.Descriptor[8]<<","<<elem.Descriptor[9]<< ","<<elem.Descriptor[10]<<","<<elem.Descriptor[11]<< "," <<
                elem.Descriptor[12]<<","<<elem.Descriptor[13]<< ","<<elem.Descriptor[14]<<","<<elem.Descriptor[15]<<"\n";
            }

            outputFile_6DOF.flush();
            outputFile_feature.flush();

#endif

            cv::Point pt;
            CvScalar point_color = { 0, 0, 250 };  // b,g,r

            for(auto elem : featureTrackingData.keypoints)
            {
                if (elem.IsRightImage == 0){
                    pt.x = elem.X;
                    pt.y = elem.Y;
                }
                else {
                    pt.x = elem.X+ image_width/2;
                    pt.y = elem.Y;
                }

                if (elem.Confidence > 100) {
                    point_color = { 0,250,0 };
                }
                if (elem.Confidence > 200) {
                    point_color = { 250,0,0 };
                }

                cv::circle(img,pt,1,point_color,cv::FILLED);
            }

            if(!withLeft)
            {
                cv::Rect m_select;
                m_select = cv::Rect(image_width / 2, 0, image_width / 2, image_height);
                img = img(m_select);
                image_width = image_width / 2;
            }
            showSlam(img,slamData);

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

#if SAVEFILE
    if (!outputFile_6DOF.is_open()) {
        std::cerr << "error in creat 6DOF file!\n";
        return 1;
    }
    if (!outputFile_feature.is_open()) {
        std::cerr << "error in creat feature file!\n";
        return 1;
    }
    outputFile_6DOF<<"TimeStamp,"<<"ID,"<<"X,"<<"Y,"<<"Z,"<<"Q_X,"<<"Q_Y,"<<"Q_Z,"<<"Q_W\n";
    outputFile_feature<<"TimeStamp,"<<"ID,"<<"Total KP,"<<"Left KP,"<<"Right KP,"<<"Uniq ID,"<<"confidence,"<<
                        "Is Right,"<<"Pattern Size,"<<"X,"<<"Y,"<<"Angle,"<<
                        "Desc1,"<<"Desc2,"<<""<<"Desc3,"<<"Desc4,"<<
                        "Desc5,"<<"Desc6,"<<""<<"Desc7,"<<"Desc8,"<<
                        "Desc9,"<<"Desc10,"<<""<<"Desc11,"<<"Desc12,"<<
                        "Desc13,"<<"Desc14,"<<""<<"Desc15,"<<"Desc16\n";
#endif

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

    p_InuFeature = new InuFeature(&m_InuSensor,&m_InuDataPath);
    p_InuFeature->StartStream();

    p_InuSlam = new InuSlam(&m_InuSensor,&m_InuDataPath);
    p_InuSlam->StartStream();

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

#if SAVEFILE
    outputFile_6DOF.flush();
    outputFile_6DOF.close();
    outputFile_feature.flush();
    outputFile_feature.close();
#endif

    running = false;

    show.join();
    p_InuFeature->StopStream();
    p_InuFisheye->StopStream();
    p_InuSlam->StopStream();
    m_InuSensor.DeviceTerminate();
    delete p_InuFisheye;
    delete p_InuFeature;
    delete p_InuSlam;

    return 0;
}
