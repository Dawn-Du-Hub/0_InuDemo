
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "InuStreams.h"
#include "InuSensorExt.h"
#include "inuDepthFilter.h"
using namespace InuDev;

namespace Inuchip
{
    class InuDepth 
    {
    public:
        InuDepth(InuSensor *_inuSensor,InuDataPath *_inuDataPath);
        ~InuDepth();

        bool StartStream();
        bool StopStream();
        int  getDepthErrorTimes();
        
    private:

        int useCallback = 1;

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;

        inu_Depth_setting *depthSetting;
        InuSensor *p_inuSensor;
        InuDataPath* p_inuDataPath;
        InuDepthFilter* p_inuDepthFilter;

        std::shared_ptr<InuDev::CDepthStream> depthStream;
        void FrameCallback(std::shared_ptr<InuDev::CDepthStream> iStream, std::shared_ptr<const InuDev::CImageFrame> iFrame, InuDev::CInuError iError);

    private:
        DepthFrame depth_frame = { 0 };
        int32_t error_times = 1;
    };
}