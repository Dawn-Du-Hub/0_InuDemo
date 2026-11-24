
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{

    class InuRGB 
    {
    public:
        InuRGB(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getRGBErrorTimes();
    private:

        int useCallback = 1;

        std::string module_name = "R132";
        CImageStream::EPostProcessing ppe;

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;
        float fps = 0;
        uint64_t monitorSeconds = 0;

        inu_RGB_setting *rgbSetting;
        InuSensor *p_inuSensor;
        InuDataPath* p_inuDataPath;
#ifdef _GE432_
        std::shared_ptr<InuDev::CImageStream> rgbStream;
        std::shared_ptr<InuDev::CImageRegisteredStream> rgbRegisteredStream;
#else
        std::shared_ptr<InuDev::CImageStreamExt>  rgbStream;
#endif

        void FrameCallback(std::shared_ptr<InuDev::CImageStream> iStream, std::shared_ptr<const InuDev::CImageFrame> iFrame, InuDev::CInuError iError);
        bool _StartStream();

        private:

        RGBFrame rgb_frame = {0};
        int32_t error_times = 1;

    };
}

