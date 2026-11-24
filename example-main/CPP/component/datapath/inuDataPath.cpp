#include "inuDataPath.h"

namespace Inuchip{

#define SAVED_NUMBER  10
    DepthFrame InuDataPath::getDepthFrame()
    {
        DepthFrame tmp = {0};

        if(!depth_queue.empty())
        {
            tmp =  depth_queue.front();
            depth_queue.pop_front();
        }
        return tmp;
    }

    DepthFrame InuDataPath::getLatestDepthFrame()
    {
        DepthFrame tmp = {0};

        if(!depth_queue.empty())
        {
            tmp =  depth_queue.back();
            depth_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setDepthFrame(DepthFrame depthFrame)
    {
        std::unique_lock<std::mutex> locker(m_depthFrameMutex);
        if (depth_queue.size() > SAVED_NUMBER) {
            DepthFrame tmp = { 0 };
            tmp = depth_queue.front();
            depth_queue.pop_front();
            delete[] tmp.frame_data;
        }

        depth_queue.push_back(depthFrame);
        locker.unlock();
        m_depthCV.notify_one();
        return true;
    }

    RGBFrame InuDataPath::getRGBFrame()
    {
        RGBFrame tmp = {0};
        //std::unique_lock<std::mutex> locker(m_rgbFrameMutex);

        if(!rgb_queue.empty())
        {
            tmp =  rgb_queue.front();
            rgb_queue.pop_front();
        }
        return tmp;
    }

    RGBFrame InuDataPath::getLatestRGBFrame()
    {
        RGBFrame tmp = {0};
        //std::unique_lock<std::mutex> locker(m_rgbFrameMutex);

        if(!rgb_queue.empty())
        {
            tmp =  rgb_queue.back();
            rgb_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setRGBFrame(RGBFrame rgbFrame)
    {
        std::unique_lock<std::mutex> locker(m_rgbFrameMutex);

        if (rgb_queue.size() > SAVED_NUMBER) {
            RGBFrame tmp = { 0 };
            tmp = rgb_queue.front();
            rgb_queue.pop_front();
            delete[] tmp.frame_data;
        }

        rgb_queue.push_back(rgbFrame);
        locker.unlock();
        m_rgbCV.notify_one();
        return true;
    }

    IRFrame InuDataPath::getIRFrame()
    {
        IRFrame tmp = {0};

        if(!ir_queue.empty())
        {
            tmp =  ir_queue.front();
            ir_queue.pop_front();
        }
        return tmp;
    }

    IRFrame InuDataPath::getLatestIRFrame()
    {
        IRFrame tmp = {0};

        if(!ir_queue.empty())
        {
            tmp =  ir_queue.back();
            ir_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setIRFrame(IRFrame irFrame)
    {
        std::unique_lock<std::mutex> locker(m_irFrameMutex);

        if (ir_queue.size() > SAVED_NUMBER) {
            IRFrame tmp = { 0 };
            tmp = ir_queue.front();
            ir_queue.pop_front();
            delete[] tmp.frame_data;
        }

        ir_queue.push_back(irFrame);
        locker.unlock();
        m_irCV.notify_one();
        return true;
    }


    PCLFrame InuDataPath::getPCLFrame()
    {
        PCLFrame tmp = {0};

        if(!pcl_queue.empty())
        {
            tmp =  pcl_queue.front();
            pcl_queue.pop_front();
        }
        return tmp;
    }

    PCLFrame InuDataPath::getLatestPCLFrame()
    {
        PCLFrame tmp = {0};

        if(!pcl_queue.empty())
        {
            tmp = pcl_queue.back();
            pcl_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setPCLFrame(PCLFrame pclFrame)
    {
        std::unique_lock<std::mutex> locker(m_pclFrameMutex);

        if (pcl_queue.size() > SAVED_NUMBER) {
            PCLFrame tmp = { 0 };
            tmp = pcl_queue.front();
            pcl_queue.pop_front();
            delete[] tmp.frame_data;
        }

        pcl_queue.push_back(pclFrame);
        locker.unlock();
        m_pclCV.notify_one();
        return true;
    }

    IMUFrame InuDataPath::getIMUFrame()
    {
        IMUFrame tmp = {0};

        if(!imu_queue.empty())
        {
            tmp =  imu_queue.front();
            imu_queue.pop_front();
        }
        return tmp;
    }

    IMUFrame InuDataPath::getLatestIMUFrame()
    {
        IMUFrame tmp = {0};

        if(!imu_queue.empty())
        {
            tmp = imu_queue.back();
            imu_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setIMUFrame(IMUFrame imuFrame)
    {
        std::unique_lock<std::mutex> locker(m_imuFrameMutex);

        if (imu_queue.size() > SAVED_NUMBER) {
            IMUFrame tmp = { 0 };
            tmp = imu_queue.front();
            imu_queue.pop_front();
            delete[] tmp.frame_data;
        }

        imu_queue.push_back(imuFrame);
        locker.unlock();
        m_imuCV.notify_one();
        return true;
    }

    TemperaFrame InuDataPath::getTemperaFrame()
    {
        TemperaFrame tmp = {0};

        if(!tempera_queue.empty())
        {
            tmp =  tempera_queue.front();
            tempera_queue.pop_front();
        }
        return tmp;
    }

    TemperaFrame InuDataPath::getLatestTemperaFrame()
    {
        TemperaFrame tmp = {0};

        if(!tempera_queue.empty())
        {
            tmp = tempera_queue.back();
            tempera_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setTemperaFrame(TemperaFrame temperaFrame)
    {
        std::unique_lock<std::mutex> locker(m_temperaFrameMutex);

        if (tempera_queue.size() > SAVED_NUMBER) {
            TemperaFrame tmp = { 0 };
            tmp = tempera_queue.front();
            tempera_queue.pop_front();
            delete[] tmp.frame_data;
        }

        tempera_queue.push_back(temperaFrame);
        locker.unlock();
        m_temperaCV.notify_one();
        return true;
    }

    FishEyeFrame InuDataPath::getFishEyeFrame()
    {
        FishEyeFrame tmp = {0};
        //std::unique_lock<std::mutex> locker(m_rgbFrameMutex);

        if(!fisheye_queue.empty())
        {
            tmp =  fisheye_queue.front();
            fisheye_queue.pop_front();
        }
        return tmp;
    }

    FishEyeFrame InuDataPath::getLatestFishEyeFrame()
    {
        FishEyeFrame tmp = {0};
        //std::unique_lock<std::mutex> locker(m_rgbFrameMutex);

        if(!fisheye_queue.empty())
        {
            tmp =  fisheye_queue.back();
            fisheye_queue.pop_back();
        }
        return tmp;
    }

    bool InuDataPath::setFishEyeFrame(FishEyeFrame fisheyeFrame)
    {
        std::unique_lock<std::mutex> locker(m_fisheyeFrameMutex);
        if (fisheye_queue.size() > SAVED_NUMBER) {
            FishEyeFrame tmp = { 0 };
            tmp = fisheye_queue.front();
            fisheye_queue.pop_front();
            delete[] tmp.frame_data;
        }

        fisheye_queue.push_back(fisheyeFrame);
        locker.unlock();
        m_fisheyeCV.notify_one();
        return true;
    }


#ifdef _SHAREDATA_

    bool InuDataPath::setRGBshareFrame(std::shared_ptr<const InuDev::CImageFrame> iframe)
    {
        std::unique_lock<std::mutex> locker(m_rgbFrameMutex);

        irgbshareframe = std::move(iframe);
        locker.unlock();

        m_rgbCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CImageFrame>  InuDataPath::getRGBshareFrame()
    {
        return std::move(irgbshareframe);
    }

    bool InuDataPath::setDepthshareFrame(std::shared_ptr<const InuDev::CImageFrame> iframe)
    {
        std::unique_lock<std::mutex> locker(m_depthFrameMutex);

        idepthshareframe = std::move(iframe);
        locker.unlock();

        m_depthCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CImageFrame>  InuDataPath::getDepthshareFrame()
    {
        return std::move(idepthshareframe);
    }

    bool InuDataPath::setFisheyeshareFrame(std::shared_ptr<const InuDev::CImageFrame> iframe)
    {
        std::unique_lock<std::mutex> locker(m_fisheyeFrameMutex);

        ifisheyeshareframe = std::move(iframe);
        locker.unlock();

        m_fisheyeCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CImageFrame>  InuDataPath::getFisheyeshareFrame()
    {
        return std::move(ifisheyeshareframe);
    }

    bool InuDataPath::setFeatureshareFrame(std::shared_ptr<const InuDev::CFeaturesTrackingFrame> iframe)
    {
        std::unique_lock<std::mutex> locker(m_featureFrameMutex);

        iFeatureshareframe = std::move(iframe);
        locker.unlock();

        m_featureCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CFeaturesTrackingFrame> InuDataPath::getFeatureshareFrame()
    {
        return std::move(iFeatureshareframe);
    }

    bool InuDataPath::setIRshareFrame(std::shared_ptr<const InuDev::CStereoImageFrame> iframe)
    {
        std::unique_lock<std::mutex> locker(m_irFrameMutex);

        iIRshareframe = std::move(iframe);
        locker.unlock();

        m_irCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CStereoImageFrame>  InuDataPath::getIRshareFrame()
    {
        return std::move(iIRshareframe);
    }

    bool InuDataPath::setIMUshareFrame(std::shared_ptr<const InuDev::CImuFrame> imuframe)
    {
        std::unique_lock<std::mutex> locker(m_imuFrameMutex);

        iIMUshareframe = std::move(imuframe);
        locker.unlock();

        m_imuCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CImuFrame>  InuDataPath::getIMUshareFrame()
    {
        return std::move(iIMUshareframe);
    }

    bool InuDataPath::setPCLshareFrame(std::shared_ptr<const InuDev::CPointCloudFrame> ipclframe)
    {
        std::unique_lock<std::mutex> locker(m_pclFrameMutex);

        iPCLshareframe = std::move(ipclframe);
        locker.unlock();

        m_pclCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CPointCloudFrame>  InuDataPath::getPCLshareFrame()
    {
        return std::move(iPCLshareframe);
    }

    bool InuDataPath::setTemperatureshareFrame(std::shared_ptr<const InuDev::CTemperaturesFrame> itemperatureframe)
    {
        std::unique_lock<std::mutex> locker(m_temperaFrameMutex);

        iTemperatureshareframe = std::move(itemperatureframe);
        locker.unlock();

        m_temperaCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CTemperaturesFrame>  InuDataPath::getTemperatureshareFrame()
    {
        return std::move(iTemperatureshareframe);
    }

    bool InuDataPath::setCNNshareFrame(std::shared_ptr<const InuDev::CCnnAppFrame> icnnframe)
    {
        std::unique_lock<std::mutex> locker(m_cnnFrameMutex);

        iCNNshareframe = std::move(icnnframe);
        locker.unlock();

        m_cnnCV.notify_one();

        return true;
    }

    std::shared_ptr<const InuDev::CCnnAppFrame>  InuDataPath::getCnnshareFrame()
    {
        return std::move(iCNNshareframe);
    }

#endif

}

