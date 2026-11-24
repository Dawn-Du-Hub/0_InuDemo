
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{
    class InuFisheye
    {
    public:
        InuFisheye (InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getFisheyeErrorTimes();

    private:

        int useCallback = 1;

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;
        float fps = 0;
        uint64_t monitorSeconds = 0;

        inu_FE_setting *FESetting;
        InuSensor *p_inuSensor;
        InuDataPath *p_inuDataPath;

        std::shared_ptr<InuDev::CImageStream>  feStream;

        void FrameCallback(std::shared_ptr<InuDev::CImageStream> iStream, std::shared_ptr<const InuDev::CImageFrame> iFrame, InuDev::CInuError iError);

        private:
        FishEyeFrame  fisheye_frame = {0};
        int32_t error_times = 0;
    };
}

