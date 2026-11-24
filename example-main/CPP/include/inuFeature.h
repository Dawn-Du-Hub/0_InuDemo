
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{
    struct FeaturesTrackingData {
        int keypointnumber;
        int leftnumber;
        int rightnumber;
        int descriptorsize;
        uint64_t timeStamp;
        uint32_t frameId;
        std::vector<FeaturesTracking::CProcessedData> keypoints;
    };
    class InuFeature 
    {
    public:
        InuFeature(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getFeatureErrorTimes();
        bool getFeaturesTrackingData2(FeaturesTrackingData &features);

    private:

        int useCallback = 1;

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;

        // inu_Feature_setting *FeatureSetting;
        InuSensor *p_inuSensor;
        InuDataPath* p_inuDataPath;
        
        FeaturesTrackingData features;
        std::mutex mMutexFeatures;

        std::shared_ptr<InuDev::CFeaturesTrackingStream>  FeatureStream;
        void FrameCallback(std::shared_ptr<InuDev::CFeaturesTrackingStream> iStream, std::shared_ptr<const InuDev::CFeaturesTrackingFrame> iFrame, InuDev::CInuError iError);

    private:
        //FeatureFrame feature_frame = { 0 };
        int32_t error_times = 1;
    };

}
