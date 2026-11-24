#include "inuFeature.h"

using namespace std;

namespace Inuchip
{
    InuFeature::InuFeature(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        p_inuDataPath = _inuDataPath;
    }

    bool InuFeature::getFeaturesTrackingData2(FeaturesTrackingData &_features)
    {
        unique_lock<mutex> lock(mMutexFeatures);
        _features = features;
        return true;
    }

    void InuFeature::FrameCallback(std::shared_ptr<InuDev::CFeaturesTrackingStream> iStream,
                            std::shared_ptr<const InuDev::CFeaturesTrackingFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            std::cout << "Feature Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("Feature Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            error_times++;
            return;
        }

        if (iFrame->Valid == false)
        {
            std::cout << "Feature Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            error_times++;
            return;
        }

        error_times = 0;
#if 0//_SHAREDATA_
        p_inuDataPath->setFeatureshareFrame(iFrame);
#else
        unique_lock<mutex> lock(mMutexFeatures);

        FeaturesTracking::CProcessedData processdata;

        features.keypointnumber = iFrame->GetKeyPointNumber();
        features.descriptorsize = iFrame->GetDescriptorSize();

        features.rightnumber = iFrame->GetKeyPointNumberRight();
        features.leftnumber = iFrame->GetKeyPointNumberLeft();
        features.timeStamp = iFrame->Timestamp;
        features.frameId = iFrame->FrameIndex;
        features.keypoints.resize(0);
        FeaturesTracking::EOutputType outputType = iFrame->GetOutputType();

        for(int i = 0; i<iFrame->GetKeyPointNumber(); i++)
        {
            if (iFrame->GetProcessedData()[i].Descriptor)
            {
                int j = 0;
                for (auto elem : iFrame->GetProcessedData()[i].Descriptor)
                {
                    processdata.Descriptor[j++] = elem;
                }
            }

            processdata.Angle = (iFrame->GetProcessedData()[i].Angle);
            processdata.Confidence = (iFrame->GetProcessedData()[i].Confidence);
            processdata.PatternSize = (iFrame->GetProcessedData()[i].PatternSize);
            processdata.UniqId = (iFrame->GetProcessedData()[i].UniqId);
            processdata.X = (iFrame->GetProcessedData()[i].X);
            processdata.Y =(iFrame->GetProcessedData()[i].Y);
            processdata.IsRightImage = (iFrame->GetProcessedData()[i].IsRightImage);
            features.keypoints.push_back(processdata);
        }
#endif
    }


   bool InuFeature::StartStream()
    {
        my_logger->info("StartfeatureStream start!");

        InuDev::CInuError retCode(0);

        FeatureStream = p_inuSensor->inuSensor->CreateFeaturesTrackingStream();

        if (FeatureStream == nullptr)
        {
            std::cout << "Unexpected error, failed to get feature Stream" << std::endl;
            return false;
        }
        
        retCode = FeatureStream->Init(InuDev::FeaturesTracking::EOutputType::eProcessed);
        if (retCode != InuDev::eOK)
        {
            std::cout << "fe initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        retCode = FeatureStream->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "Start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        if(useCallback){
            retCode = FeatureStream->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
            if (retCode != InuDev::eOK)
            {
                std::cout << "feature register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
                return false;
            }
            my_logger->info("StartFeatureStream end!");
        }

        return true;
    }

    bool InuFeature::StopStream()
    {
        if (!FeatureStream) { return true; }

        InuDev::CInuError retCode = FeatureStream->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "feature unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("feature unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        retCode = FeatureStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "feature Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("feature Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        // Finalize feature stream service
        retCode = FeatureStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "feature Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("feature Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));

            // return false;
        }

        return true;
    }

    int InuFeature::getFeatureErrorTimes()
    {
        return error_times;
    }

}
