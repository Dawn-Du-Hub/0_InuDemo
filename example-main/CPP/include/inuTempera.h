
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"
#include <vector>

using namespace InuDev;

namespace Inuchip
{

    class InuTempera 
    {
    public:
        InuTempera(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getTemperaErrorTimes();
        bool isCallback();
        std::vector<float> getTemperature();
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
        InuDataPath* p_inuDataPath;

        std::shared_ptr<InuDev::CBaseStream>  temperaStream;
        void FrameCallback(std::shared_ptr<InuDev::CTemperaturesStream> iStream, std::shared_ptr<const InuDev::CTemperaturesFrame> iFrame, InuDev::CInuError iError);

        private:

        TemperaFrame tempera_frame = {0};
        int32_t error_times = 1;

    };
    inline bool InuTempera::isCallback() { if (useCallback) { return true; } else { return false; } };
}

