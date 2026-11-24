#include "yaml-cpp/yaml.h"
#include "yaml-cpp/node/parse.h"
#include "inuParams.h"
#include "inuspdlog.h"
using namespace std;
using namespace Inuchip;

namespace YAML {
    template<>
    struct convert<inu_Sensor_setting> {
        static Node encode(const inu_Sensor_setting &rhs) {
            Node node;
            node.push_back(rhs.ip_address);
            node.push_back(rhs.device_id);
            node.push_back(rhs.fps);
            node.push_back(rhs.resolution);
            node.push_back(rhs.depthON);
            node.push_back(rhs.rgbON);

            return node;
        }

        static bool decode(const Node &node, inu_Sensor_setting &rhs) {
            rhs.ip_address = node["IP_Address"].as<std::string>();
            rhs.device_id = node["Device_ID"].as<std::string>();
            rhs.fps = node["FPS"].as<int>();
            rhs.resolution = node["Resolution"].as<int>();
            rhs.depthON = node["DepthOn"].as<bool>();
            rhs.rgbON = node["RGBOn"].as<bool>();

            return true;
        }
    };

    template<>
    struct convert<inu_Depth_setting> {
        static Node encode(const inu_Depth_setting &rhs) {
            Node node;
            node.push_back(rhs.fps);
            node.push_back(rhs.channel);
            node.push_back(rhs.resolution);
            node.push_back(rhs.outputFormat);
            node.push_back(rhs.postProcessing);
            node.push_back(rhs.registrationChannel);
            node.push_back(rhs.confidence);
            node.push_back(rhs.depthScale);
            node.push_back(rhs._autoExposure);
            node.push_back(rhs._analogGain);
            node.push_back(rhs._digitalGain);
            node.push_back(rhs._exposureTime);
            node.push_back(rhs.useROI);
            node.push_back(rhs.irTop);
            node.push_back(rhs.irLeft);
            node.push_back(rhs.irBottom);
            node.push_back(rhs.irRight);
            node.push_back(rhs._sdkBlobmode);
            node.push_back(rhs._sdkBlobMaxSize);
            node.push_back(rhs._sdkBlobMaxHeight);
            node.push_back(rhs._sdkBlobDisparityThreshold);
            node.push_back(rhs._sdkBlobDispDiffSameBlob);
            node.push_back(rhs._sdkTemporalStable);
            node.push_back(rhs._sdkTemporalRapid);

            return node;
        }

        static bool decode(const Node &node, inu_Depth_setting &rhs) {
            rhs.fps = node["FPS"].as<int>();
            rhs.channel = node["Channel"].as<int>();
            rhs.resolution = node["Resolution"].as<int>();
            rhs.outputFormat = node["OutputFormat"].as<int>();
            rhs.postProcessing = node["PostProcessing"].as<int>();
            rhs.registrationChannel = node["RegistrationChannel"].as<int>();
            rhs.confidence = node["Confidence"].as<int>();
            rhs.depthScale = node["DepthScale"].as<int>();
            rhs._autoExposure = node["Exposure"].as<bool>();
            rhs._analogGain = node["AnalogGain"].as<float>();
            rhs._digitalGain = node["DigitalGain"].as<float>();
            rhs._exposureTime = node["ExposureTime"].as<int>();
            rhs.useROI = node["ExposureUseROI"].as<bool>();
            rhs.irTop = node["ExposureTop"].as<int>();
            rhs.irLeft = node["ExposureLeft"].as<int>();
            rhs.irBottom = node["ExposureBottom"].as<int>();
            rhs.irRight = node["ExposureRight"].as<int>();
            rhs._sdkBlobmode = node["SDKBlobMode"].as<int>();
            rhs._sdkBlobMaxSize = node["SDKBlobMaxSize"].as<int>();
            rhs._sdkBlobMaxHeight = node["SDKBlobMaxHeight"].as<int>();
            rhs._sdkBlobDisparityThreshold = node["SDKBlobDisparityThreshold"].as<int>();
            rhs._sdkBlobDispDiffSameBlob = node["SDKBlobDispDiffSameBlob"].as<int>();
            rhs._sdkTemporalStable = node["SDKTemporalStable"].as<int>();
            rhs._sdkTemporalRapid = node["SDKTemporalRapid"].as<int>();

            return true;
        }
    };

    template<>
    struct convert<inu_RGB_setting> {
        static Node encode(const inu_RGB_setting &rhs) {
            Node node;
            node.push_back(rhs.fps);
            node.push_back(rhs.channel);
            node.push_back(rhs.resolution);
            node.push_back(rhs.outputFormat);
            node.push_back(rhs.postProcessing);
            node.push_back(rhs.registrationChannel);
            node.push_back(rhs._autoExposure);
            node.push_back(rhs._analogGain);
            node.push_back(rhs._digitalGain);
            node.push_back(rhs._exposureTime);

            return node;
        }

        static bool decode(const Node &node, inu_RGB_setting &rhs) {
            rhs.fps = node["FPS"].as<int>();
            rhs.channel = node["Channel"].as<int>();
            rhs.resolution = node["Resolution"].as<int>();
            rhs.outputFormat = node["OutputFormat"].as<int>();
            rhs.postProcessing = node["PostProcessing"].as<int>();
            rhs.registrationChannel = node["RegistrationChannel"].as<int>();
            rhs._autoExposure = node["Exposure"].as<bool>();
            rhs._analogGain = node["AnalogGain"].as<float>();
            rhs._digitalGain = node["DigitalGain"].as<float>();
            rhs._exposureTime = node["ExposureTime"].as<int>();

            return true;
        }
    };

    template<>
    struct convert<inu_PCL_setting> {
        static Node encode(const inu_PCL_setting &rhs) {
            Node node;
            node.push_back(rhs.outputFormat);
            node.push_back(rhs.registrationChannel);

            return node;
        }

        static bool decode(const Node &node, inu_PCL_setting &rhs) {
            rhs.outputFormat = node["OutputFormat"].as<int>();
            rhs.registrationChannel = node["RegistrationChannel"].as<int>();
            return true;
        }
    };

    template<>
    struct convert<inu_FE_setting> {
        static Node encode(const inu_FE_setting &rhs) {
            Node node;
            node.push_back(rhs.channel);

            return node;
        }

        static bool decode(const Node &node, inu_FE_setting &rhs) {
            rhs.channel = node["Channel"].as<int>();
            return true;
        }
    };

    template<>
    struct convert<inu_CNN_setting> {
        static Node encode(const inu_CNN_setting &rhs) {
            Node node;
            node.push_back(rhs.type);

            return node;
        }

        static bool decode(const Node &node, inu_CNN_setting &rhs) {
            rhs.type = node["OutputFormat"].as<int>();
            return true;
        }
    };
}

namespace Inuchip{
    InuParams::InuParams(std::string yamlfile)
    {
        loadParams(yamlfile);
    }

    InuParams::InuParams()
    {
        loadParams("");
    }

    bool InuParams::loadParams(std::string yamlfile)
    {
        if(yamlfile != "")
        {
            YAML::Node config = YAML::LoadFile(yamlfile);

            if(config["inu_Sensor_setting"])
            {
                inuSensorSetting = config["inu_Sensor_setting"].as<inu_Sensor_setting>();
            }
            if (config["inu_Depth_setting"])
            {
                inuDepthSetting = config["inu_Depth_setting"].as<inu_Depth_setting>();
            }
            if (config["inu_RGB_setting"])
            {
                inuRGBSetting = config["inu_RGB_setting"].as<inu_RGB_setting>();
            }
            if (config["inu_PCL_setting"])
            {
                inuPCLSetting = config["inu_PCL_setting"].as<inu_PCL_setting>();
            }
            if (config["inu_FE_setting"])
            {
                inuFESetting = config["inu_FE_setting"].as<inu_FE_setting>();
            }
            if (config["inu_CNN_setting"])
            {
                inuCNNSetting = config["inu_CNN_setting"].as<inu_CNN_setting>();
            }
        }else{
            inuSensorSetting.fps = 15;
            inuSensorSetting.ip_address = "";
            inuSensorSetting.device_id = "";
            inuSensorSetting.resolution = 1;

            inuDepthSetting.fps = 15;
            inuDepthSetting.resolution = 1;
            inuDepthSetting.registrationChannel = -1;

            inuRGBSetting.fps = 15;
            inuRGBSetting.resolution = 1;

        }

        my_logger->info("loadParams finish!");

        return true;
    }
}





