
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{
    class InuIR 
    {
    public:
        InuIR(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getIRErrorTimes();

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

        std::shared_ptr<InuDev::CStereoImageStream>  stereoStream;

        void FrameCallback(std::shared_ptr<InuDev::CStereoImageStream> iStream, std::shared_ptr<const InuDev::CStereoImageFrame> iFrame, InuDev::CInuError iError);

        private:
        IRFrame ir_frame = {0};
        int32_t error_times = 0;
    };
}

