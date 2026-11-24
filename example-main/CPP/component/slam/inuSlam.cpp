#include "inuSlam.h"

using namespace std;

namespace Inuchip
{
    InuSlam::InuSlam(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        p_inuDataPath = _inuDataPath;
    }

    bool InuSlam::getSlamData(SlamData &slamdata)
    {
        unique_lock<mutex> lock(mMutexSlam);
        slamdata = _slamData;
        return true;
    }

    void InuSlam::FrameCallback(std::shared_ptr<InuDev::CSlamStream> iStream,
                            std::shared_ptr<const InuDev::CSlamFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            std::cout << "SLAM Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("SLAM Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            error_times++;
            return;
        }

        if (iFrame->Valid == false)
        {
            std::cout << "SLAM Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            error_times++;
            return;
        }

        if (iFrame->mSlamState == InuDev::CSlamFrame::eTrackLost)       //eTrackOK
        {
            std::cout << "Tracking Lost!" << std::endl;
            my_logger->warn("Tracking Lost!");
            // return;
        }

        error_times = 0;
#if 0//_SHAREDATA_

#else
        std::array<float, 4> Q;
        std::array<float, 3> T;

        iFrame->ConvertPose4x4ToQuaternionTranslation(
            iFrame->mPose4x4BodyToWorld,
            Q,
            T
        );

        iPose.mQuaternion[0] = Q[0];
        iPose.mQuaternion[1] = Q[1];
        iPose.mQuaternion[2] = Q[2];
        iPose.mQuaternion[3] = Q[3];
        iPose.mTranslation[0] = T[0];
        iPose.mTranslation[1] = T[1];
        iPose.mTranslation[2] = T[2];

        Inverse_pos(iPose, oPose);
        unique_lock<mutex> lock(mMutexSlam);

        _slamData.timeStamp = iFrame->Timestamp;
        _slamData.frameId = iFrame->FrameIndex;
        _slamData.state = iFrame->mSlamState;
        _slamData.iPose = iPose;
        _slamData.oPose = oPose;

#endif
    }
    void InuSlam::Inverse_pos(poseT& iPose, poseT& oPose)
    {
        int k;
        for (k = 0; k < 3; k++)
        {
            oPose.mQuaternion[k] = -iPose.mQuaternion[k];
            oPose.mTranslation[k] = -iPose.mTranslation[k];
        }
        oPose.mQuaternion[3] = iPose.mQuaternion[3];

        float newTrans[3];
        MultiplyQuaternionByVec(oPose, oPose.mTranslation, newTrans);
        oPose.mTranslation[0] = newTrans[0];
        oPose.mTranslation[1] = newTrans[1];
        oPose.mTranslation[2] = newTrans[2];

    }

   bool InuSlam::StartStream()
    {
        my_logger->info("StartSlamStream start!");

        InuDev::CInuError retCode(0);

        slamStream = std::static_pointer_cast<InuDev::CBaseStream>(p_inuSensor->inuSensor->CreateSlamStream());

        if (slamStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get slam Stream" << std::endl;
            return false;
        }
        
        retCode = std::static_pointer_cast<InuDev::CSlamStream>(slamStream)->Init();
        if (retCode != InuDev::eOK)
        {
            std::cout << "slam initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        retCode = std::static_pointer_cast<InuDev::CSlamStream>(slamStream)->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "slam start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        if(useCallback){
            retCode = std::static_pointer_cast<InuDev::CSlamStream>(slamStream)->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            if (retCode != InuDev::eOK)
            {
                std::cout << "slam register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
            my_logger->info("StartSlamStream end!");
        }

        return true;
    }

    bool InuSlam::StopStream()
    {
        if (!slamStream) { return true; }

        InuDev::CInuError retCode = std::static_pointer_cast<InuDev::CSlamStream>(slamStream)->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "slam unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("slam unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        retCode = slamStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "slam Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("slam Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        // Finalize slam stream service
        retCode = slamStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "slam Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("slam Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        return true;
    }

    int InuSlam::getSlamErrorTimes()
    {
        return error_times;
    }

    void InuSlam::MultiplyQuaternionByVec(poseT& oPose, float v[3], float newTrans[3])
    {
        float quaternion[4];
        quaternion[0] = oPose.mQuaternion[0];
        quaternion[1] = oPose.mQuaternion[1];
        quaternion[2] = oPose.mQuaternion[2];
        quaternion[3] = oPose.mQuaternion[3];

        float uv[3];
        uv[0] = 2 * (quaternion[1] * v[2] - quaternion[2] * v[1]);
        uv[1] = 2 * (quaternion[2] * v[0] - quaternion[0] * v[2]);
        uv[2] = 2 * (quaternion[0] * v[1] - quaternion[1] * v[0]);
        newTrans[0] = v[0] + quaternion[3] * uv[0] + (quaternion[1] * uv[2] - quaternion[2] * uv[1]);
        newTrans[1] = v[1] + quaternion[3] * uv[1] + (quaternion[2] * uv[0] - quaternion[0] * uv[2]);
        newTrans[2] = v[2] + quaternion[3] * uv[2] + (quaternion[0] * uv[1] - quaternion[1] * uv[0]);
    }

}
