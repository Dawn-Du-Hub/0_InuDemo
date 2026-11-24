#include "inuSensor.h"
using namespace std;

namespace Inuchip
{
    InuSensor::InuSensor()
    {
    }

    InuSensor::InuSensor(InuParams *_InuParams)
    {
        p_InuParams = _InuParams;
        st.fps = p_InuParams->inuSensorSetting.fps;
        st.resolution = p_InuParams->inuSensorSetting.resolution;
        st.sensor_name = p_InuParams->inuSensorSetting.ip_address;
        st.device_id = p_InuParams->inuSensorSetting.device_id;
    }

    void InuSensor::FrameCallback(std::shared_ptr<InuDev::CInuSensor> iSensor, InuDev::EConnectionState iConnectionState, InuDev::CInuError iError)
    {
        // if (iError == InuDev::EErrorCode::eOK)
        {
            std::cout << " connection state = " << iConnectionState;

        }

        switch(int(iConnectionState))
        {
            case  InuDev::EConnectionState::eDisconnected:
            break;
            case InuDev::EConnectionState::eServiceDisconnected:
            break;

        }
        /*
        switch(int(iError))
        {
            case InuDev::EErrorCodeExt::eUSBDisconnect:
            break;
            case InuDev::EErrorCodeExt::eStreamDisconnected:
            break;
        }*/
    }

    bool InuSensor::checkDeviceState()
    {
        InuDev::CInuError retCode(0);

        my_logger->info("checkDeviceState start!");

        if (st.sensor_name != "" || st.device_id != "")
        {
            inuSensor = InuDev::CInuSensorExt::Create(st.device_id,st.sensor_name);
        }
        else {
            inuSensor = InuDev::CInuSensorExt::Create("","");
        }
        DeviceTerminate();

        retCode = inuSensor->Connect();

        if (retCode != InuDev::eOK)
        {
            inuSensor->Terminate();
            inuSensor->Disconnect();
            // Error, probably InuService is not running 
            std::cout << "Failed to start the Inuitive Sensor." << std::endl;
            return false;
        }

        InuDev::EConnectionState sensorstate = inuSensor->GetConnectionState();
        if (sensorstate != eConnected)
        {
            inuSensor->Terminate();
            inuSensor->Disconnect();
            return false;
        }
        my_logger->info("checkDeviceState: sensor is connecting!");
        return true;
    }

    bool  InuSensor::DeviceInit()
    {
        InuDev::CInuError retCode(0);

        InuDev::CHwInformation mHwInfo;
        InuDev::CDeviceParamsExt params;
        InuDev::CDpeParams dpeparams;
        std::vector<CDpeParams> vecDpeParamsInit;
        std::vector<CDpeParams> vecDpeParamsStart;
        std::map<CEntityVersion::EEntitiesID, CEntityVersion> oVersion;
        params.FPS = st.fps;
        if (st.resolution == 1)
        {
            params.SensorRes = InuDev::eBinning;
        }
        else if (st.resolution == 2)
        {
            params.SensorRes = InuDev::eVerticalBinning;
        }
        else if (st.resolution == 3)
        {
            params.SensorRes = InuDev::eFull;
        }

        // my_logger->info("DeviceInit start!");

        retCode = inuSensor->Init(mHwInfo, vecDpeParamsInit, params);
        if (retCode != InuDev::eOK)
        {
            std::cout << "Failed to connect to Inuitive Sensor. Error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // my_logger->warn("Failed to connect to Inuitive Sensor. error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            return false;
        }

        retCode = inuSensor->GetVersion(oVersion);
        if (retCode != InuDev::eOK)
        {
            std::cout << "Failed to get SN. Error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // my_logger->warn("Failed to get SN. error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            return false;
        }
        std::cout << "Serial Number: " << oVersion[CEntityVersion::eSerialNumber].VersionName << std::endl;
        st.serial = oVersion[CEntityVersion::eSerialNumber].VersionName;
        st.FwVersion = oVersion[CEntityVersion::eFWVersion].VersionName;
        st.usb_type = oVersion[CEntityVersion::EEntitiesID::eUSBSpeed].VersionNum;
        st.CalibrationVersion = oVersion[CEntityVersion::EEntitiesID::eCalibrationVersion].VersionName;
        st.BootfixVersion = oVersion[CEntityVersion::EEntitiesID::eBootfixVersion].VersionName;

        st.module = "R132";

        if((st.serial).find("BBA")!= string::npos)
        {
            st.module = "R132";
        }
        else if((st.serial).find("ABA") != string::npos || (st.serial).find("JHT") != string::npos)
        {
            st.module = "C158";
        }
        else if((st.serial).find("CAA") != string::npos)
        {
            st.module = "R130";
        }
        else if((st.serial).find("ABB") != string::npos )
        {
            st.module = "C158Tracking";
        }


        // Start acquiring frames - it must be call before starting acquiring any type of frames (depth, video, head, etc.)
        std::map<uint32_t, CChannelSize> oChannelsSize;
        CStartDeviceParamsExt ioStartParams;
        ioStartParams.VecDpeParams = vecDpeParamsStart;

        ESensorResolution depthResolution,rgbResolution;
        if (p_InuParams->inuDepthSetting.resolution == 1)
        {
            depthResolution = InuDev::eBinning;
        }
        else if (p_InuParams->inuDepthSetting.resolution == 2)
        {
            depthResolution = InuDev::eVerticalBinning;
        }
        else if (p_InuParams->inuDepthSetting.resolution == 3)
        {
            depthResolution = InuDev::eFull;
        }

        if (p_InuParams->inuRGBSetting.resolution == 1)
        {
            rgbResolution = InuDev::eBinning;
        }
        else if (p_InuParams->inuRGBSetting.resolution == 2)
        {
            rgbResolution = InuDev::eVerticalBinning;
        }
        else if (p_InuParams->inuRGBSetting.resolution == 3)
        {
            rgbResolution = InuDev::eFull;
        }
 #if 1
        for (auto channel : mHwInfo.GetChannels()) {
            switch (channel.second.ChannelType) {
            case eDepthChannel:
                ioStartParams.ChannelControlParam[channel.second.ChannelId].SensorRes = depthResolution;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].FPS = p_InuParams->inuDepthSetting.fps;
                    if(p_InuParams->inuDepthSetting.registrationChannel != -1){
                        ioStartParams.ChannelControlParam[channel.second.ChannelId].ActivateRegisteredDepth = true;
                        ioStartParams.ChannelControlParam[channel.second.ChannelId].RegisteredDepthChannelID = p_InuParams->inuDepthSetting.registrationChannel;
                    }
                break;
            case eDisparityChannel:
                ioStartParams.ChannelControlParam[channel.second.ChannelId].SensorRes = depthResolution;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].FPS = p_InuParams->inuDepthSetting.fps;
                    if(p_InuParams->inuDepthSetting.registrationChannel != -1){
                        ioStartParams.ChannelControlParam[channel.second.ChannelId].ActivateRegisteredDepth = true;
                        ioStartParams.ChannelControlParam[channel.second.ChannelId].RegisteredDepthChannelID = p_InuParams->inuDepthSetting.registrationChannel;
                    }
                break;
            case eGeneralCameraChannel:
                ioStartParams.ChannelControlParam[channel.second.ChannelId].SensorRes = rgbResolution;//InuDev::eFull;//params.SensorRes;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].FPS = p_InuParams->inuRGBSetting.fps;
                break;
            case eTrackingChannel:
                ioStartParams.ChannelControlParam[channel.second.ChannelId].SensorRes = InuDev::eFull;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].FPS = 30;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].InterleaveMode = InuDev::eInterleave;
                break;
            case eFeaturesTrackingChannel:
                break;
            case eStereoChannel:
                ioStartParams.ChannelControlParam[channel.second.ChannelId].SensorRes = params.SensorRes;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].FPS = st.fps;
                ioStartParams.ChannelControlParam[channel.second.ChannelId].InterleaveMode = InuDev::eInterleave;
                break;
            default:
                break;
            };
        }
#else
        ioStartParams.ChannelControlParam[0].SensorRes = params.SensorRes;
        ioStartParams.ChannelControlParam[0].FPS = st.fps;
        ioStartParams.ChannelControlParam[0].InterleaveMode = InuDev::eInterleave;

        if (st.module == "C158" ||  st.module == "C158Tracking")
        {
            ioStartParams.ChannelControlParam[2].SensorRes = InuDev::eFull;   // must set full, or fe will crash
            ioStartParams.ChannelControlParam[2].FPS = 30;//st->fps;
            ioStartParams.ChannelControlParam[2].InterleaveMode = InuDev::eInterleave;
        }
        ioStartParams.ChannelControlParam[3].SensorRes = depthResolution;
        ioStartParams.ChannelControlParam[3].FPS = p_InuParams->inuDepthSetting.fps;
        ioStartParams.ChannelControlParam[5].SensorRes = depthResolution;//params.SensorRes;  InuDev::eFull
        ioStartParams.ChannelControlParam[5].FPS = p_InuParams->inuDepthSetting.fps;

        ioStartParams.ChannelControlParam[4].SensorRes = rgbResolution;
        ioStartParams.ChannelControlParam[4].FPS = p_InuParams->inuRGBSetting.fps;
        ioStartParams.ChannelControlParam[9].SensorRes = rgbResolution;
        ioStartParams.ChannelControlParam[9].FPS = p_InuParams->inuRGBSetting.fps;
        if (p_InuParams->inuDepthSetting.registrationChannel != -1) {
            
            ioStartParams.ChannelControlParam[p_InuParams->inuDepthSetting.channel].ActivateRegisteredDepth = true;
            ioStartParams.ChannelControlParam[p_InuParams->inuDepthSetting.channel].RegisteredDepthChannelID = p_InuParams->inuDepthSetting.registrationChannel;
        }
#endif


        std::cout << "inuSensor->Start..." << std::endl;

        retCode = inuSensor->Start(oChannelsSize, ioStartParams);
        if (retCode != InuDev::eOK)
        {
            std::cout << "Failed to start to Inuitive Sensor." << std::endl;
            // my_logger->warn("Failed to start to Inuitive Sensor. error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            return false;
        }
        std::cout << "Sensor is started" << std::endl;

        std::cout << "Print Channel Size" << std::endl;
        st.pChannelsSize.clear();
        for (auto cs : oChannelsSize)
        {
            std::cout << "\t" << "Channel " << cs.first << std::endl;
            std::cout << "\t" << "Channel " << cs.second << std::endl;
            std::cout << std::endl;
            st.pChannelsSize.insert(pair<uint32_t, CChannelSize>( cs.first, cs.second));
        }
        //st->pChannelsSize.insert(oChannelsSize.begin(),oChannelsSize.end());


        LoadOD();

        // inuSensor->Register([this](auto stream,auto iConnectionState,auto error){FrameCallback(inuSensor,iConnectionState,error);});
        my_logger->info("DeviceInit end!");

        return true;
    }

    bool InuSensor::getSensorTemperature(float& temperature)
    {
        // GetSensorTemperature is removed from SDK4.30

        // inuSensor->GetSensorTemperature(CInuSensor::ESensorTemperatureType::eSensor1,temperature);

        return true;
    }

    bool InuSensor::DeviceTerminate()
    {
        InuDev::CInuError  retCode = inuSensor->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Failed to stop CInuSensor, error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }
        std::cout << "Sensor was stopped" << std::endl;

        // Turn off the sensor
        retCode = inuSensor->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Failed to terminate CInuSensor, error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            // return false;
        }

        retCode = inuSensor->Disconnect();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Failed to disconnect InuService, error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
        }

        return true;
    }

    void InuSensor::LoadOD()
    {
        int iSen = 0;

        inuSensor->GetCalibrationData(oOpticalData);


        mOD.focalDepth[0] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.FocalLength[0];
        mOD.focalDepth[1] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.FocalLength[1];

        mOD.centerDepth[0] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.OpticalCenter[0];
        mOD.centerDepth[1] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.OpticalCenter[1];
        mOD.baselineDepth = oOpticalData.Baselines[std::pair<int,int>(0,1)];


        inuSensor->GetCalibrationData(oOpticalData,4);

        //
        iSen = 2;
        if(std::stoi((st.CalibrationVersion).substr(2,2)) < 20){

            mOD.focalWeb[0] = oOpticalData.Sensors[iSen].RealCamera.Intrinsic.FocalLength[0];
            mOD.focalWeb[1] = oOpticalData.Sensors[iSen].RealCamera.Intrinsic.FocalLength[1];
            mOD.centerWeb[0] = oOpticalData.Sensors[iSen].RealCamera.Intrinsic.OpticalCenter[0];
            mOD.centerWeb[1] = oOpticalData.Sensors[iSen].RealCamera.Intrinsic.OpticalCenter[1];

            for (int i = 0; i < oOpticalData.Sensors[iSen].RealCamera.Intrinsic.LensDistortion.size(); i++ )
                mOD.KdWeb[i] = oOpticalData.Sensors[iSen].RealCamera.Intrinsic.LensDistortion[i];
            
            mOD.rotAngle[0] = oOpticalData.Sensors[iSen].RealCamera.Extrinsic.Rotation[0];
            mOD.rotAngle[1] = oOpticalData.Sensors[iSen].RealCamera.Extrinsic.Rotation[1];
            mOD.rotAngle[2] = oOpticalData.Sensors[iSen].RealCamera.Extrinsic.Rotation[2];

            mOD.WebcamTranslate[0] = oOpticalData.Sensors[iSen].RealCamera.Extrinsic.Translation[0];
            mOD.WebcamTranslate[1] = oOpticalData.Sensors[iSen].RealCamera.Extrinsic.Translation[1];
            mOD.WebcamTranslate[2] = oOpticalData.Sensors[iSen].RealCamera.Extrinsic.Translation[2];
        }else{

            mOD.focalWeb[0] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.FocalLength[0];
            mOD.focalWeb[1] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.FocalLength[1];
            mOD.centerWeb[0] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.OpticalCenter[0];
            mOD.centerWeb[1] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.OpticalCenter[1];

            for (int i = 0; i < oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.LensDistortion.size(); i++ )
                mOD.KdWeb[i] = oOpticalData.Sensors[iSen].VirtualCamera.Intrinsic.LensDistortion[i];
            
            mOD.rotAngle[0] = oOpticalData.Sensors[iSen].VirtualCamera.Extrinsic.Rotation[0];
            mOD.rotAngle[1] = oOpticalData.Sensors[iSen].VirtualCamera.Extrinsic.Rotation[1];
            mOD.rotAngle[2] = oOpticalData.Sensors[iSen].VirtualCamera.Extrinsic.Rotation[2];

            mOD.WebcamTranslate[0] = oOpticalData.Sensors[iSen].VirtualCamera.Extrinsic.Translation[0];
            mOD.WebcamTranslate[1] = oOpticalData.Sensors[iSen].VirtualCamera.Extrinsic.Translation[1];
            mOD.WebcamTranslate[2] = oOpticalData.Sensors[iSen].VirtualCamera.Extrinsic.Translation[2];
        }

        // mOD.Init();

    }

    bool InuSensor::setIRExposeParams()
    {
        InuDev::CSensorControlParams IRsensorControlParams;

        IRsensorControlParams.DigitalGain = p_InuParams->inuDepthSetting._digitalGain;           //85
        IRsensorControlParams.ExposureTime = p_InuParams->inuDepthSetting._exposureTime;         //300
        IRsensorControlParams.AnalogGain = p_InuParams->inuDepthSetting._analogGain;             //0
        
        IRsensorControlParams.AutoControl = false;
        if(p_InuParams->inuDepthSetting._autoExposure)
            IRsensorControlParams.AutoControl = true;

        if(p_InuParams->inuDepthSetting.useROI){
            IRsensorControlParams.Params.UseROI = true;
            IRsensorControlParams.Params.ROITopLeft.X() = p_InuParams->inuDepthSetting.irLeft;
            IRsensorControlParams.Params.ROITopLeft.Y() = p_InuParams->inuDepthSetting.irTop;
            IRsensorControlParams.Params.ROIBottomRight.X() = p_InuParams->inuDepthSetting.irRight;
            IRsensorControlParams.Params.ROIBottomRight.Y() = p_InuParams->inuDepthSetting.irBottom;
        }else{
            IRsensorControlParams.Params.UseROI = false;
            IRsensorControlParams.Params.ROITopLeft.X() = 0;
            IRsensorControlParams.Params.ROITopLeft.Y() = 0;
            IRsensorControlParams.Params.ROIBottomRight.X() = 0;
            IRsensorControlParams.Params.ROIBottomRight.Y() = 0;
        }

        InuDev::CInuError retCode = inuSensor->SetSensorControlParams(IRsensorControlParams, 0);
        
        if (retCode != InuDev::eOK)
        {
            std::cout << "IR setExposeParams error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        return true;
    }

    bool InuSensor::getIRExposeParams()
    {
        InuDev::CSensorControlParams IRsensorControlParams;

        InuDev::CInuError retCode = inuSensor->GetSensorControlParams(IRsensorControlParams, 0);
        
        if (retCode != InuDev::eOK)
        {
            std::cout << "IR getExposeParams error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        p_InuParams->inuDepthSetting._autoExposure = false;
        if(IRsensorControlParams.AutoControl == true)
            p_InuParams->inuDepthSetting._autoExposure = true;

        p_InuParams->inuDepthSetting._analogGain = IRsensorControlParams.AnalogGain;
        p_InuParams->inuDepthSetting._digitalGain = IRsensorControlParams.DigitalGain;
        p_InuParams->inuDepthSetting._exposureTime = IRsensorControlParams.ExposureTime;

        return true;
    }

    bool InuSensor::setRGBExposeParams()
    {
        InuDev::CSensorControlParams RGBsensorControlParams;
        RGBsensorControlParams.DigitalGain = p_InuParams->inuRGBSetting._digitalGain;           //85
        RGBsensorControlParams.ExposureTime = p_InuParams->inuRGBSetting._exposureTime;         //300
        RGBsensorControlParams.AnalogGain = p_InuParams->inuRGBSetting._analogGain;             //0

        InuDev::CSensorI2CRegisterParams  i2cparams;
        i2cparams.mI2CNumber = 2;
        i2cparams.mDeviceAddress = 0x78; 
        i2cparams.mDataSize = 1;
        i2cparams.mRegAddress = 0x3503;  // total time 
        i2cparams.mRegAddressSize = 2;

        inuSensor->GetI2CRegister(i2cparams);

        if(p_InuParams->inuRGBSetting._autoExposure)
        {
            if(i2cparams.mData != 0)  //auto
            {
                i2cparams.mData = 0;
                inuSensor->SetI2CRegister(i2cparams);
            }

            // AutoControl cannot control the RGB exposure, but IR can

            // RGBsensorControlParams.AutoControl = true;
            // InuDev::CInuError retCode = inuSensor->SetSensorControlParams(RGBsensorControlParams, 2);
            // if (retCode != InuDev::eOK)
            // {
            //     std::cout << "RGB SetSensorControlParams error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            //     return false;
            // }
        }else{
            //if(i2cparams.mData != 3)
            {
                i2cparams.mData = 3;
                inuSensor->SetI2CRegister(i2cparams);
            }

            RGBsensorControlParams.AutoControl = false;
            InuDev::CInuError retCode = inuSensor->SetSensorControlParams(RGBsensorControlParams, 2);
            
            if (retCode != InuDev::eOK)
            {
                std::cout << "RGB SetSensorControlParams error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
        }

        return true;
    }

    bool InuSensor::getRGBExposeParams()
    {
        InuDev::CSensorControlParams RGBsensorControlParams;
        InuDev::CInuError retCode = inuSensor->GetSensorControlParams(RGBsensorControlParams, 2);

        p_InuParams->inuRGBSetting._digitalGain = RGBsensorControlParams.DigitalGain;
        p_InuParams->inuRGBSetting._exposureTime = RGBsensorControlParams.ExposureTime;
        p_InuParams->inuRGBSetting._analogGain = RGBsensorControlParams.AnalogGain;

        InuDev::CSensorI2CRegisterParams  i2cparams;
        i2cparams.mI2CNumber = 2;
        i2cparams.mDeviceAddress = 0x78; 
        i2cparams.mDataSize = 1;
        i2cparams.mRegAddress = 0x3503;  // total time 
        i2cparams.mRegAddressSize = 2;

        inuSensor->GetI2CRegister(i2cparams);

        // if(RGBsensorControlParams.AutoControl == false)
        if(i2cparams.mData == 3)   // manual
        {
            p_InuParams->inuRGBSetting._autoExposure = false;
        }

        else if(i2cparams.mData == 0){  //auto
            p_InuParams->inuRGBSetting._autoExposure = true;
        }
        return true;
    }

    bool InuSensor::setProjector(EProjectorLevel iLevel, EProjectors iProjectorID)
    {
        inuSensor->SetProjectorLevel(iLevel,iProjectorID);
        return true;
    }
}