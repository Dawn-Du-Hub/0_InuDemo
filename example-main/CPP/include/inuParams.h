
#pragma once 
#include <string>
namespace Inuchip{

    struct inu_Sensor_setting{
        std::string ip_address;
        std::string device_id;
        int fps;
        int resolution;
        bool depthON;
        bool rgbON;
    };

    struct inu_Depth_setting{
        int fps;
        int channel;
        int resolution;
        int outputFormat;
        int postProcessing;
        int registrationChannel;
        int confidence;
        int depthScale;
        //exposure
        bool _autoExposure;
        float _analogGain;
        float _digitalGain;
        int _exposureTime; //us
        bool useROI;
        int irTop;
        int irLeft;
        int irBottom;
        int irRight;
        //blob
        int _sdkBlobmode;
        int _sdkBlobMaxSize;
        int _sdkBlobMaxHeight;
        int _sdkBlobDisparityThreshold;
        int _sdkBlobDispDiffSameBlob;
        //temporal
        int _sdkTemporalStable;
        int _sdkTemporalRapid;
    };

    struct inu_RGB_setting{
        int fps;
        int channel;
        int resolution;
        int outputFormat;
        int postProcessing;
        int registrationChannel;
        bool _autoExposure;
        float _analogGain;
        float _digitalGain;
        int _exposureTime; //us 
    };

    struct inu_PCL_setting{
        int outputFormat;
        int registrationChannel;
    };

    struct inu_FE_setting{
        int channel;
    };

    struct inu_CNN_setting {
        int type;
    };

    class InuParams
    {
        public:
        inu_Sensor_setting  inuSensorSetting;
        inu_Depth_setting   inuDepthSetting;
        inu_RGB_setting     inuRGBSetting;
        inu_PCL_setting     inuPCLSetting;
        inu_FE_setting      inuFESetting;
        inu_CNN_setting     inuCNNSetting;


        InuParams(std::string yamlfile);
        InuParams();

        private:
        bool loadParams(std::string yamlfile);

    };
}