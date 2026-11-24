
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{
    class InuCNN 
    {
    public:

        InuCNN(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getErrorTimes();
    private:

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;
        float fps = 0;
        uint64_t monitorSeconds = 0;

        inu_CNN_setting *cnnSetting;
        InuSensor *p_inuSensor;
        InuDataPath* p_inuDataPath;

        std::shared_ptr<InuDev::CBaseStream>  cnnStream;

        void FrameCallback(std::shared_ptr<InuDev::CCnnAppStream> iStream, 
                           std::shared_ptr<const InuDev::CCnnAppFrame> iFrame, InuDev::CInuError iError);

    private:

        int32_t error_times = 1;

    };
}
