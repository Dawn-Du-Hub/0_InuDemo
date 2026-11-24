
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{
    class InuIMU
    {
    public:
        InuIMU(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getIMUErrorTimes();

    private:

        int useCallback = 1;

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;
        float fps = 0;
        uint64_t monitorSeconds = 0;

        InuSensor *p_inuSensor;
        InuDataPath *p_inuDataPath;

        InuDev::CPoint3D gyro;
        InuDev::CPoint3D accel;

        std::shared_ptr<InuDev::CBaseStream>  imuStream;

        void FrameCallback(std::shared_ptr<InuDev::CImuStream> iStream, std::shared_ptr<const InuDev::CImuFrame> iFrame, InuDev::CInuError iError);

        private:
        IMUFrame imu_frame = {0};
        int32_t error_times = 0;
    };
}

