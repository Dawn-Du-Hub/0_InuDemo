
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "InuStreams.h"
#include "InuSensorExt.h"

using namespace InuDev;

namespace Inuchip
{
    class InuPCL
    {
    public:
        InuPCL(InuSensor *_inuSensor,InuDataPath *_inuDataPath);
        ~InuPCL();

        bool StartStream();
        bool StopStream();
        int  getPCLErrorTimes();


    private:
        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;
        float fps = 0;
        uint64_t monitorSeconds = 0;

        InuSensor *p_inuSensor;
        InuDataPath* p_inuDataPath;

        std::shared_ptr<InuDev::CPointCloudStream>  pclStream;

        void FrameCallback(std::shared_ptr<InuDev::CPointCloudStream> iStream, std::shared_ptr<const InuDev::CPointCloudFrame> iFrame, InuDev::CInuError iError);

    private:
        PCLFrame pcl_frame = { 0 };
        int32_t error_times = 0;

    };
}