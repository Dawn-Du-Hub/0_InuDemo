
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

#include "InuStreams.h"
#include "InuSensorExt.h"


using namespace InuDev;

namespace Inuchip
{
    struct poseT{
        float mQuaternion[4];
        float mTranslation[3];
    };

    struct SlamData {
        uint64_t timeStamp;
        uint32_t frameId;
        uint32_t state;
        poseT iPose;
        poseT oPose;
    };
    class InuSlam 
    {
    public:
        InuSlam(InuSensor *_inuSensor,InuDataPath *_inuDataPath);

        bool StartStream();
        bool StopStream();
        int  getSlamErrorTimes();
        bool getSlamData(SlamData &slamdata);


    private:

        int useCallback = 1;

        //staticstics
        uint64_t lastTimeStamp = 0;
        uint64_t currentTimeStamp = 0;
        uint32_t lastFrameIndex = 0;
        uint32_t currentFrameIndex = 0;

        InuSensor *p_inuSensor;
        InuDataPath* p_inuDataPath;

        SlamData _slamData;
        poseT iPose, oPose;

        std::mutex mMutexSlam;

        std::shared_ptr<InuDev::CBaseStream>  slamStream;
        void FrameCallback(std::shared_ptr<InuDev::CSlamStream> iStream, std::shared_ptr<const InuDev::CSlamFrame> iFrame, InuDev::CInuError iError);
        void MultiplyQuaternionByVec(poseT& oPose, float v[3], float newTrans[3]);
        void Inverse_pos(poseT& iPose, poseT& oPose);

    private:
        int32_t error_times = 1;

    };

}
