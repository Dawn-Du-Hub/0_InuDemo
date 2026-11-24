
#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <queue>
#include <deque>
#include <mutex>
#include <condition_variable>
#ifdef _SHAREDATA_
#include "InuStreams.h"
#endif
namespace Inuchip{
    using namespace std;

    struct FrameData
    {
        int datatype; //1:depth, 2:rgb, 3:IR,4：PCL,5:IMU,6: Temperature,7:FishEye
        int frame_id;
        uint32_t width;  // when PCL, this means num of points
        uint32_t height; // when PCL, this is PCL format
        uint64_t timestamp;
        uint64_t datasize;
        uint64_t totalsize;
        int compressed; //0: none 1:rvl 2:jpg
        int device_id;
        uint8_t* frame_data;  // here store all data (struct data + stream data)
    };

    typedef FrameData DepthFrame;
    typedef FrameData RGBFrame;
    typedef FrameData IRFrame;
    typedef FrameData PCLFrame;
    typedef FrameData IMUFrame;
    typedef FrameData TemperaFrame;
    typedef FrameData FishEyeFrame;


    struct CommData
    {
        int commtype;   // 101: start 103:stop 105:yaml setting 107: time adjust
        uint64_t timestamp;
        uint64_t datasize; // 0 means not real comm_data
        uint64_t totalsize; 
        uint8_t* comm_data;
    };

    struct CommDataResult
    {
        bool comm_timeadjust;

    };

    struct RecBUF
    {
        uint32_t len;
        uint8_t* buf;
    };

    class InuDataPath
    {
    public:
        CommDataResult commResult;

        std::deque<DepthFrame> depth_queue;
        std::mutex m_depthFrameMutex;
        std::condition_variable m_depthCV;

        std::deque<RGBFrame> rgb_queue;
        std::mutex m_rgbFrameMutex;
        std::condition_variable m_rgbCV;

        std::deque<IRFrame> ir_queue;
        std::mutex m_irFrameMutex;
        std::condition_variable m_irCV;

        std::deque<PCLFrame> pcl_queue;
        std::mutex m_pclFrameMutex;
        std::condition_variable m_pclCV;

        std::deque<IMUFrame> imu_queue;
        std::mutex m_imuFrameMutex;
        std::condition_variable m_imuCV;

        std::deque<TemperaFrame> tempera_queue;
        std::mutex m_temperaFrameMutex;
        std::condition_variable m_temperaCV;

        std::deque<FishEyeFrame> fisheye_queue;
        std::mutex m_fisheyeFrameMutex;
        std::condition_variable m_fisheyeCV;

        std::mutex m_featureFrameMutex;
        std::condition_variable m_featureCV;

        std::mutex m_cnnFrameMutex;
        std::condition_variable m_cnnCV;


        DepthFrame getDepthFrame();
        DepthFrame getLatestDepthFrame();
        bool setDepthFrame(DepthFrame depthFrame);

        RGBFrame getRGBFrame();
        RGBFrame getLatestRGBFrame();
        bool setRGBFrame(RGBFrame rgbFrame);

        IRFrame getIRFrame();
        IRFrame getLatestIRFrame();
        bool setIRFrame(IRFrame irFrame);

        PCLFrame getPCLFrame();
        PCLFrame getLatestPCLFrame();
        bool setPCLFrame(PCLFrame pclFrame);

        IMUFrame getIMUFrame();
        IMUFrame getLatestIMUFrame();
        bool setIMUFrame(IMUFrame imuFrame);

        TemperaFrame getTemperaFrame();
        TemperaFrame getLatestTemperaFrame();
        bool setTemperaFrame(TemperaFrame temperaFrame);

        FishEyeFrame getFishEyeFrame();
        FishEyeFrame getLatestFishEyeFrame();
        bool setFishEyeFrame(FishEyeFrame fisheyeFrame);

#ifdef _SHAREDATA_
        std::shared_ptr<const InuDev::CImageFrame> idepthshareframe;
        std::shared_ptr<const InuDev::CImageFrame> irgbshareframe;
        std::shared_ptr<const InuDev::CImageFrame> ifisheyeshareframe;
        std::shared_ptr<const InuDev::CStereoImageFrame> iIRshareframe;
        std::shared_ptr<const InuDev::CImuFrame> iIMUshareframe;
        std::shared_ptr<const InuDev::CPointCloudFrame> iPCLshareframe;
        std::shared_ptr<const InuDev::CTemperaturesFrame> iTemperatureshareframe;
        std::shared_ptr<const InuDev::CFeaturesTrackingFrame> iFeatureshareframe;
        std::shared_ptr<const InuDev::CCnnAppFrame> iCNNshareframe;


        bool setDepthshareFrame(std::shared_ptr<const InuDev::CImageFrame> iframe);
        std::shared_ptr<const InuDev::CImageFrame>  getDepthshareFrame();

        bool setRGBshareFrame(std::shared_ptr<const InuDev::CImageFrame> iframe);
        std::shared_ptr<const InuDev::CImageFrame>  getRGBshareFrame();

        bool setFisheyeshareFrame(std::shared_ptr<const InuDev::CImageFrame> iframe);
        std::shared_ptr<const InuDev::CImageFrame>  getFisheyeshareFrame();

        bool setFeatureshareFrame(std::shared_ptr<const InuDev::CFeaturesTrackingFrame> iframe);
        std::shared_ptr<const InuDev::CFeaturesTrackingFrame>  getFeatureshareFrame();

        bool setIRshareFrame(std::shared_ptr<const InuDev::CStereoImageFrame> iframe);
        std::shared_ptr<const InuDev::CStereoImageFrame>  getIRshareFrame();

        bool setIMUshareFrame(std::shared_ptr<const InuDev::CImuFrame> imuframe);
        std::shared_ptr<const InuDev::CImuFrame>  getIMUshareFrame();

        bool setPCLshareFrame(std::shared_ptr<const InuDev::CPointCloudFrame> ipclframe);
        std::shared_ptr<const InuDev::CPointCloudFrame>  getPCLshareFrame();

        bool setTemperatureshareFrame(std::shared_ptr<const InuDev::CTemperaturesFrame> itemperatureframe);
        std::shared_ptr<const InuDev::CTemperaturesFrame>  getTemperatureshareFrame();

        bool setCNNshareFrame(std::shared_ptr<const InuDev::CCnnAppFrame> iCnnframe);
        std::shared_ptr<const InuDev::CCnnAppFrame> getCnnshareFrame();
#endif
    };
}
