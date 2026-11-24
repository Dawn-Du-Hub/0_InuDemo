#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "inuRGB.h"
#include "inuCNN.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h> 

using namespace cv;

using namespace Inuchip;


bool running = true;

std::string  winname = "CNN Stream";
struct HandInfoSB
{
    CCnnAppFrame::EHandSide HandSide;
    CCnnAppFrame::EHandGesture HandGesture;
}mHandtmp;

std::array<std::array<float, 3>, 18> mKeypos;                        // one person
std::vector <std::array<std::array<float, 3>, 18>> mKeyposVec;      // many persons

std::array<std::array<uint32_t, 2>, 21> mKeyHand;
std::vector <std::array<std::array<uint32_t, 2>, 21>> mKeyHandVec;
std::vector <HandInfoSB> mHandsInfo;


static int YoloV7Skeleton[][2] = {
    {15, 13},
    {13, 11},
    {16, 14},
    {14, 12},
    {11, 12},
    {5, 11},
    {6, 12},
    {5, 6},
    {5, 7},
    {6, 8},
    {7, 9},
    {8, 10},
    {0, 1},
    {0, 2},
    {1, 3},
    {2, 4},
    {3, 5},
    {4, 6},
};

void _draw_keypoints_pose(cv::Mat& webcam_mat, std::array<std::array<float, 3>, 18>& keypoints)
{
    CvScalar kp_color = { 255, 0, 0 };
    CvScalar connect_color = { 0, 255, 0 };

    if (keypoints.size() != 18)
        return;

    for (int i = 0; i < keypoints.size(); i++) {
        if (keypoints[i][2] == 0.0)
            continue;
        cv::Point p1((uint32_t)keypoints[i][0], (uint32_t)keypoints[i][1]);
        cv::circle(webcam_mat, p1, 2, kp_color, 3);
        for (int j = 0; j < keypoints.size(); j++) {
            if (YoloV7Skeleton[j][0] == i && keypoints[YoloV7Skeleton[j][1]][2] > 0) {
                cv::Point p2((uint32_t)keypoints[YoloV7Skeleton[j][1]][0], (uint32_t)keypoints[YoloV7Skeleton[j][1]][1]);
                cv::line(webcam_mat, p1, p2, connect_color, 2);
            }
        }
    }
}

static int lines_hand[21][2] = {
    {0, 1}, {1,2}, {2,3}, {3,4},
    {0,5}, {5,6}, {6,7}, {7,8},
    {5,9}, {9, 10}, {10,11}, {11,12},
    {9, 13}, {13,14}, {14,15}, {15,16},
    {13,17}, {17,18}, {18,19}, {19, 20}, {0, 17}
};

void draw_keypoints_hand(cv::Mat& webcam_mat, std::array<std::array<uint32_t, 2>, 21>& keypoints, CCnnAppFrame::EHandSide side, CCnnAppFrame::EHandGesture gesture)
{

    if (keypoints.size() != 21)
        return;

    /* Draw the lines */
    cv::Scalar lines_color = { 255, 0, 0 };
    cv::Scalar circle_color = { 0, 128, 255 };
    cv::Scalar box_color = { 0, 0, 255 };
    cv::Point point_from, point_to;
    vector<cv::Point> one_line;
    vector<vector<cv::Point>> lines;
    int index_from, index_to, min_y_pos;
    int line_type = 8, shift = 0, text_y_offset;
    int line_thickness, text_thickness;
    int circle_thickness, circle_radius;
    float text_scale;
    string sideStr;
    string gestureStr;
    cv::Point pt1, pt2, text_pos;
    CvScalar text_color = { 255, 255, 0 };

    if (webcam_mat.cols > 1000) {
        line_thickness = 1;
        circle_thickness = 6;
        circle_radius = 2;
        text_thickness = 3;
        text_scale = 3;
        text_pos.y = 40;
        line_thickness = 2;
        text_y_offset = 10;
    }
    else {
        line_thickness = 2;
        circle_thickness = 3;
        circle_radius = 1;
        text_thickness = 2;
        text_scale = 2;
        text_pos.y = 30;
        line_thickness = 4;
        text_y_offset = 20;
    }

    min_y_pos = 99999;
    for (int j = 0; j < keypoints.size(); j++) {
        one_line.clear();
        index_from = lines_hand[j][1];
        index_to = lines_hand[j][0];
        point_from.x = keypoints[index_from][0];
        point_from.y = keypoints[index_from][1];
        point_to.x = keypoints[index_to][0];
        point_to.y = keypoints[index_to][1];
        one_line.push_back(point_from);
        one_line.push_back(point_to);
        lines.push_back(one_line);
        if (point_to.y < min_y_pos) {
            min_y_pos = point_to.y;
        }
        if (point_from.y < min_y_pos) {
            min_y_pos = point_from.y;
        }
    }
    cv::polylines(webcam_mat, lines, false, lines_color, line_thickness);

    /* Draw the circles */
    for (int j = 0; j < keypoints.size(); j++) {
        point_to.x = keypoints[j][0];
        point_to.y = keypoints[j][1];
        cv::circle(webcam_mat, point_to, circle_radius, circle_color, circle_thickness, cv::LINE_AA);
    }

    if (side == CCnnAppFrame::EHandSide::eRightHand) {
        sideStr = "Right hand";
    }
    else {
        sideStr = "Left hand";
    }

    if (gesture != CCnnAppFrame::EHandGesture::eNoneHand) {
        switch (gesture)
        {
        case CCnnAppFrame::EHandGesture::eCloseHand:
            gestureStr = "Closed hand";
            break;
        case CCnnAppFrame::EHandGesture::eOne:
            gestureStr = "One hand";
            break;
        case CCnnAppFrame::EHandGesture::eOpenHand:
            gestureStr = "Open hand";
            break;
        case CCnnAppFrame::EHandGesture::ePointLeft:
            gestureStr = "Pointing Left";
            break;
        case CCnnAppFrame::EHandGesture::ePointRight:
            gestureStr = "Pointing Right";
            break;
        case CCnnAppFrame::EHandGesture::eThumbUp:
            gestureStr = "Thumb up";
            break;
        default:
            break;
        }
    }

    text_pos.x = keypoints[1][0];
    text_pos.y = max(min_y_pos, 20);

    cv::putText(webcam_mat,
        sideStr,
        text_pos,
        cv::FONT_HERSHEY_PLAIN,
        text_scale,
        text_color,
        text_thickness);

    text_pos.y += 50;

    cv::putText(webcam_mat,
        gestureStr,
        text_pos,
        cv::FONT_HERSHEY_PLAIN,
        text_scale,
        text_color,
        text_thickness);
}

cv::Mat mMatWeb;
std::mutex mMutexWeb;
uint64_t mTimestampWeb;

void InsertWebImage(const cv::Mat &img, uint64_t ts)
{
    unique_lock<mutex> lock(mMutexWeb);
    mMatWeb = std::move(img);
    mTimestampWeb = ts;
}

uint64_t GetWebImage(cv::Mat &img, uint64_t ts)
{
    unique_lock<mutex> lock(mMutexWeb);
    if (mMatWeb.empty())
    {
        return 0;
    }
    img = std::move(mMatWeb);
    return mTimestampWeb;
}


void showRGB(InuDataPath* p_inuDataPath)
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

                InsertWebImage(imgshow,p_frame->Timestamp);
            }
            catch (Exception e) {
                continue;
            }
        }
    };
}

void showCNN(InuDataPath* p_inuDataPath)
{
    static int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    static double fontScale = 1;
    static CvScalar text_color = { 20, 250, 20 };

	cv::Mat imgWeb;


    while (running)
    {
        std::unique_lock<std::mutex> locker(p_inuDataPath->m_cnnFrameMutex);
        p_inuDataPath->m_cnnCV.wait(locker);
        std::shared_ptr<const InuDev::CCnnAppFrame> p_frame = p_inuDataPath->getCnnshareFrame();
        locker.unlock();

        if (p_frame)
        {
            uint64_t webTs = GetWebImage(imgWeb,1);
            if (imgWeb.empty()) continue;

            std::shared_ptr<std::vector<CCnnAppFrame::CDetectedPose>> tmpPoses = p_frame->GetPoseData();
            std::shared_ptr<std::vector<InuDev::CCnnAppFrame::CDetectedObject>> tmpObjects = p_frame->GetObjectData();
            std::shared_ptr<std::vector<CCnnAppFrame::CDetectedHand>> tmpHands = p_frame->GetHandsData();

            CCnnAppFrame::EOutputType format = p_frame->GetOutputType();

            switch (format)
            {
                case CCnnAppFrame::eObjectDetection:
                case CCnnAppFrame::eObjectDetectionYoloV3:
                case CCnnAppFrame::eObjectDetectionYoloV7:
                    if (tmpObjects)
                    {
                        for (std::vector<InuDev::CCnnAppFrame::CDetectedObject>::iterator it = tmpObjects->begin(); it != tmpObjects->end(); ++it)
                        {

                            cv::rectangle(imgWeb, cv::Point(it->ClosedRectTopLeft.X(), it->ClosedRectTopLeft.Y()),
                                cv::Point(it->ClosedRectTopLeft.X() + it->ClosedRectSize.X(), it->ClosedRectTopLeft.Y() + it->ClosedRectSize.Y()),
                                cv::Scalar(0, 0, 255), 2);

                            std::string  name_line = it->ClassID + " " + std::to_string(it->Confidence);

                            cv::putText(imgWeb, name_line, cv::Point(it->ClosedRectTopLeft.X(), it->ClosedRectTopLeft.Y()),
                                fontFace,
                                fontScale,
                                text_color,
                                3);
                        }
                    }
                    break;
                case CCnnAppFrame::ePoseDetection:
                    if (tmpPoses)
                    {
                        if (!tmpPoses->empty())
                        {
                            mKeyposVec.clear();
                            for (uint32_t j = 0; j < tmpPoses->size(); j++)
                            {
                                for_each(mKeypos.begin(), mKeypos.end(), [](auto& innerArray) {
                                    fill(innerArray.begin(), innerArray.end(), 0.0f);
                                    });
                                for (uint32_t i = 0; i < tmpPoses->at(j).NumberOfKeyPoints; i++)
                                {
                                    mKeypos[i][0] = tmpPoses->at(j).KeyPoints[i].P.X();
                                    mKeypos[i][1] = tmpPoses->at(j).KeyPoints[i].P.Y();
                                    mKeypos[i][2] = tmpPoses->at(j).KeyPoints[i].Conf;
                                }
                                mKeyposVec.push_back(mKeypos);
                            }
                        }

                        if (!mKeyposVec.empty()){
                            for (uint32_t i = 0; i < mKeyposVec.size(); i++){
                                _draw_keypoints_pose(imgWeb,mKeyposVec.at(i));
                            }
                            mKeyposVec.clear();
                        }
                    }
                    break;
                case CCnnAppFrame::eHandDetection:
                    if (tmpHands){

                        if (!tmpHands->empty())
                        {
                            mKeyHandVec.clear();
                            cout << "tmpHands - size: " << tmpHands->size() << endl;
                            for (uint32_t j = 0; j < tmpHands->size(); j++)
                            {
                                for_each(mKeyHand.begin(), mKeyHand.end(), [](auto& innerArray) {
                                    fill(innerArray.begin(), innerArray.end(), 0);
                                    });
                                for (uint32_t i = 0; i < (uint32_t)tmpHands->at(j).HAND_LANDMARKS_POINTS; i++)
                                {
                                    mKeyHand[i][0] = tmpHands->at(j).HandPoints[i].X();
                                    mKeyHand[i][1] = tmpHands->at(j).HandPoints[i].Y();
                                }
                                mHandtmp.HandGesture = (CCnnAppFrame::EHandGesture)tmpHands->at(j).Gesture;
                                mHandtmp.HandSide = (CCnnAppFrame::EHandSide)tmpHands->at(j).Side;

                                mHandsInfo.push_back(mHandtmp);
                                mKeyHandVec.push_back(mKeyHand);
                            }
                        }

                        if (!mKeyHandVec.empty())
                        {
                            for (uint32_t i = 0; i < mKeyHandVec.size(); i++)
                                draw_keypoints_hand(imgWeb, mKeyHandVec.at(i), mHandsInfo.at(i).HandSide, mHandsInfo.at(i).HandGesture);
                            mKeyHandVec.clear();
                            mHandsInfo.clear();
                        }
                    }
            }

            cv::namedWindow(winname, cv::WINDOW_NORMAL|cv::WINDOW_KEEPRATIO);

            int image_height = imgWeb.rows;
            int image_width = imgWeb.cols;

            cv::resizeWindow(winname, image_width , image_height );
            cv::imshow(winname, imgWeb);

            #ifdef _MSC_VER
                cv::waitKey(1);
            #endif
        }
    };
}

int main(int argc, char* argv[])
{
    spdlog_init();

    InuDataPath m_InuDataPath;

    std::string filename = "./../../../app/cfg/Params_CNN.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    InuRGB *p_InuRGB = new InuRGB(&m_InuSensor,&m_InuDataPath);
    InuCNN *p_InuCNN = new InuCNN(&m_InuSensor,&m_InuDataPath);

    p_InuRGB->StartStream();
    p_InuCNN->StartStream();

    std::thread show(showCNN,&m_InuDataPath);
    std::thread showC(showRGB,&m_InuDataPath);


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
    showC.join();

    p_InuCNN->StopStream();
    delete p_InuCNN;

    p_InuRGB->StopStream();
    delete p_InuRGB;

    m_InuSensor.DeviceTerminate();

    return 0;
}
