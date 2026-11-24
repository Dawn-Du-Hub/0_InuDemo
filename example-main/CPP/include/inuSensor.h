
#pragma once

#include "inuCommon.h"
#include "InuSensorExt.h"
#include "InuError.h"
#include "inuParams.h"

using namespace InuDev;

namespace Inuchip
{
    class RGBExposParams
    {
        public:
        bool auto_exposure = false;  //0:false 2:true
        int analog_gain = 0;
        int digital_gain = 85;
        int exposure_time = 300; //us
    };
    class Settings
    {
        public:
        std::string device_id;              // for net or multimodule
        int _device_id;
        int fps = 10;
        int resolution = 0;                 // 0:binning  1:vbinning 2:full

        //time sync
        bool time_sync = false;

        int IRWithLeft = 1;
        int IRuseCallback = 1;
        int RGBuseCallback = 1;

        int FEWithLeft = 2;
        int FEWithFeatures = 2;

        // std::string rgb_ppe = "None";       // 0: none 1: regi
        std::string sensor_name ;           // for net or multimodule, do not use anymore

        std::string depth_ppe = "None";
        std::string depth_format = "eDepth";

        int SaveStream = 0;                 // 0: pic  2: steam
        int startSaveStream = 0;            // 0: stop 1: recoding

        // PCL setting
        std::string pclFrom;                // Depth  Orignal Orignal-RGB
        bool pclUseFiltered = false;
        int useCloudView = 0;               // 0: do not use cloudView to display PCL 2:use
        int showCloud = 2;                  // 0: do not show 2: show cloud
        float pclAxisRate = 1.;             // 1 - m, 10 - dm,100 - dm,1000 - m
        int PCLwithRGB = 0;
        int virtualWall = 0;                // mm

        std::string serial;
        std::string module;
        std::string FwVersion;
        int usb_type;
        std::string CalibrationVersion;
        std::string BootfixVersion;

        float temperature;
        // std::map<uint32_t, CChannelSize> *pChannelsSize=nullptr;
        std::map<uint32_t, CChannelSize> pChannelsSize;//=nullptr;

    };

    class SensorCOptData{
        public:
            /*********** Internal Right Camera Parameters ***************/
            float focalDepth[2];
            float centerDepth[2];

            // float inversefocalDepth[2];

            float baselineDepth;

            /*********** Internal Web Camera Parameters ***************/
            float focalWeb[2];
            float centerWeb[2];
            float uvWeb[2];

            /****************Web Distortion parameters****************************/
            float KdWeb[5];
            // cv::Size sizeWeb;

            /*********** ROTATION Right to Web ***************/
            // cv::Mat rotRightWeb;
            float rotAngle[3];

            float WebcamTranslate[3];
    };

    class InuSensor
    {
        public:
        InuSensor();
        InuSensor(InuParams *_InuParams);
        Settings st; 
        InuParams *p_InuParams = nullptr;
        CCalibrationData oOpticalData;
        SensorCOptData mOD;

        std::shared_ptr<InuDev::CInuSensorExt> inuSensor;

        bool save_rgb = false;
        bool save_ir = false;
        bool save_pcl = false;
        bool save_depth = false;
        bool save_fe = false;


        bool checkDeviceState();
        bool DeviceInit();
        bool DeviceTerminate();
        bool getSensorTemperature(float& temperature);
        bool getRGBExposeParams();
        bool setRGBExposeParams();
        bool getIRExposeParams();
        bool setIRExposeParams();
        bool setProjector(EProjectorLevel iLevel = eHigh, EProjectors iProjectorID = ePatterns);

        private:
        void FrameCallback(std::shared_ptr<InuDev::CInuSensor> iSensor, InuDev::EConnectionState eConnectState, InuDev::CInuError iError);
        void LoadOD();

    };
}