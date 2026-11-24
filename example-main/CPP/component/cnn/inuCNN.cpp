#include "inuCNN.h"

using namespace std;

namespace Inuchip
{
    InuCNN::InuCNN(InuSensor *_inuSensor, InuDataPath *_inuDataPath)
    {
        p_inuSensor = _inuSensor;
        cnnSetting = &((p_inuSensor->p_InuParams)->inuCNNSetting);
        p_inuDataPath = _inuDataPath;
    }

    void InuCNN::FrameCallback(std::shared_ptr<InuDev::CCnnAppStream> iStream,
                            std::shared_ptr<const InuDev::CCnnAppFrame>  iFrame,
                            InuDev::CInuError retCode)
    {
        if (retCode != InuDev::eOK)
        {
            error_times++;
            std::cout << "CNN Error in receiving frame: " << std::hex << int(retCode) << " " << std::string(retCode) << std::endl;
            my_logger->warn("CNN Error in receiving frame: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            return;
        }

        if (iFrame->Valid == false)
        {
            error_times++;
            std::cout << "CNN Frame  " << iFrame->FrameIndex << " is invalid" << std::endl;
            return;
        }
        error_times = 0;
        p_inuDataPath->setCNNshareFrame(iFrame);

    }

    bool InuCNN::StartStream()
    {
        InuDev::CInuError retCode(0);

        uint32_t outputformat = cnnSetting->type;

        InuDev::CCnnAppFrame::EOutputType ObjectDetectionType = InuDev::CCnnAppFrame::eObjectDetection;

        if ( outputformat == 0 )
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::eObjectDetection;
        }
        else if (outputformat == 1)
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::eSegmentation;
        }
        else if (outputformat == 2)
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::eClassification;
        }
        else if (outputformat == 4)
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::eObjectDetectionYoloV3;
        }
        else if (outputformat == 7)
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::eObjectDetectionYoloV7;
        }
        else if (outputformat == 11)
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::ePoseDetection;
        }
        else if (outputformat == 12)
        {
            ObjectDetectionType = InuDev::CCnnAppFrame::eHandDetection;
        }
        else
        {
            my_logger->info("Plese input correct cnn net! ");
            return false;
        }

        cnnStream = std::static_pointer_cast<InuDev::CBaseStream>(p_inuSensor->inuSensor->CreateCnnAppStream());
        if (cnnStream == nullptr)
        {
            my_logger->warn("Unexpected error, failed to get CNN Stream ");
            return false;
        }

        retCode = std::static_pointer_cast<InuDev::CCnnAppStream>(cnnStream)->Init(ObjectDetectionType);
        if (retCode != InuDev::EErrorCode::eOK)
        {
            std::cout << "CNN initiation error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;

            return false;
        }

        retCode = std::static_pointer_cast<InuDev::CCnnAppStream>(cnnStream)->Start();
        if (retCode != InuDev::eOK)
        {
            std::cout << "CNN start error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }
        retCode = std::static_pointer_cast<InuDev::CCnnAppStream>(cnnStream)->Register([this](auto stream, auto frame, auto error) { FrameCallback(stream, frame, error); });
        if (retCode != InuDev::eOK)
        {
            std::cout << "CNN register error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            return false;
        }

        return true;
    }

    bool InuCNN::StopStream()
    {
        if (!cnnStream ) { return true; }

        InuDev::CInuError retCode(0);

        retCode = std::static_pointer_cast<InuDev::CCnnAppStream>(cnnStream)->Register(nullptr);
        if (retCode != InuDev::eOK)
        {
            std::cout << "CNN unregister error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("CNN unregister error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;  
        }

        retCode = cnnStream->Stop();
        if (retCode != InuDev::eOK)
        {
            std::cout << "CNN Stop error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("CNN Stop error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }

        retCode = cnnStream->Terminate();
        if (retCode != InuDev::eOK)
        {
            std::cout << "CNN Terminate error: " << std::hex << int(retCode) << " - " << std::string(retCode) << std::endl;
            my_logger->warn("CNN Terminate error: 0x{:x} - {:s} !", int(retCode), std::string(retCode));
            // return false;
        }
        std::cout << "CNN Stream was finalized" << std::endl;
        my_logger->info("CNN Stream was finalized !");

        return true;
    }

    int InuCNN::getErrorTimes()
    {
        return error_times;
    }
}
