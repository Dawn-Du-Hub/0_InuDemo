#include "inuCommon.h"
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"

using namespace Inuchip;

//////////////////////////////////////////////////////////////////////

InuSensor *p_inuSensor = nullptr;

//////////////////////////////////////////////////////////////////////

void DynamicCalibrationCallBack(CInuCalibration::EDynamicCalibrationStatus iStatus, CInuError iError) 
{ 
    switch (iStatus)  
    { 
        case CInuCalibration::eNotStarted: 
            std::cout << "DynamicCalibrationCallBack: Not Sarted" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationStarted: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Started" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationRecordingFrames: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration RecordingFrames" << std::endl; 
        break; 
        case CInuCalibration::eTestCalibrationEvaluatingExistingCalibrationAndRecordedImages: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Evaluating Existing Calibration And Recorded Images" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationCompletedSuccessfully: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Completed Successfully" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationCompletedCalibrationIsRequired: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Calibration Is Required" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailed: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedSceneNotGoodForCalibration: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed Scene Not Good For Calibration" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedToCreateRecordingsDirectory: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed To Create Recordings Directory" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedToStartIrStream: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed To Start Ir Stream" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedToStartDepthStream: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed To Start Depth Stream" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedToRecordStreams: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed To Record Streams" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedToStopIrStream: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed To Stop Ir Stream" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationFailedToStopDepthStream: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Failed To Stop Depth Stream" << std::endl; 
            break; 
        case CInuCalibration::eTestCalibrationCalibrationDirectoryWasntFound: 
            std::cout << "DynamicCalibrationCallBack: Test Calibration Calibration Directory Wasnt Found" << std::endl; 
            break; 
        case CInuCalibration::eGenerateCalibrationStarted: 
            std::cout << "DynamicCalibrationCallBack: Dsr Started" << std::endl; 
            break; 
        case CInuCalibration::eGenerateCalibrationProcessing: 
            std::cout << "DynamicCalibrationCallBack: Dsr Processing" << std::endl; 
            break; 
        case CInuCalibration::eGenerateCalibrationCompletedSuccessfully: 
            std::cout << "DynamicCalibrationCallBack: Dsr Completed Successfully" << std::endl; 
            break; 
        case CInuCalibration::eGenerateCalibrationFailed: 
            std::cout << "DynamicCalibrationCallBack: Dsr Failed" << std::endl; 
            break; 
        default: 
            std::cout << "DynamicCalibrationCallBack: Unknown status" << std::endl; 
            break; 
    } 
} 

int main(int argc, char* argv[])
{
    spdlog_init();

    InuDataPath m_InuDataPath;

    std::string filename = "./../../../app/cfg/Params_autoCalib.yaml";
    InuParams m_InuParams(filename);

    InuSensor m_InuSensor(&m_InuParams);

    p_inuSensor = &(m_InuSensor);

    while(!m_InuSensor.checkDeviceState())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_InuSensor.DeviceInit();

    int action = -1;
    InuDev::CInuError status = InuDev::eOK; 
    #ifdef _MSC_VER
    //where to create the test data and new calib folder 
    std::string sFolder = "C:\\Program Files\\Inuitive\\InuDev\\config\\6";  
    //which folder to take as new calibration 
    std::string sFolderApply = "C:\\Program Files\\Inuitive\\InuDev\\config\\6\\BBA12391007"; 
    #else
    std::string sFolder = "/home/inu/Desktop/test/1"; 
    std::string sFolderApply = "/home/inu/Desktop/test/1/BBA12391007"; 
    #endif 
    std::string sCode = "FlashInu123!"; 

    while(1){

        std::cout << "Select action to perform: 1=TestCalibration\n" 
        "                          2=GenerateCalibration\n" 
        "                          3=ApplyCalibration\n" 
        "                          4=EraseCalibration\n" 
        "                          5=FlashCalibration\n"
        "                          0=Exit\n"; 

        std::cin >> action;

        if(action == 0){break;}

        switch (action) 
        { 
            case 1: // TestCalibration 
            std::cout << "Performing TestCalibration." << std::endl; 
            status = p_inuSensor->inuSensor->TestCalibration(DynamicCalibrationCallBack, sFolder); 
            break; 
            
            case 2: // GenerateCalibration 
            std::cout << "Performing GenerateCalibration." << std::endl; 
            status = p_inuSensor->inuSensor->GenerateCalibration(DynamicCalibrationCallBack, sFolder); 
            break; 
            
            case 3: // ApplyCalibration 
            std::cout << "Performing ApplyCalibration." << std::endl; 
            status = p_inuSensor->inuSensor->ApplyCalibration(sFolderApply); 
            break; 
            
            case 4: // EraseCalibration 
            std::cout << "Performing EraseCalibration." << std::endl; 
            status = p_inuSensor->inuSensor->EraseCalibration(sCode); 
            break; 
            
            case 5: // FlashCalibration 
            std::cout << "Performing FlashCalibration." << std::endl; 
            status = p_inuSensor->inuSensor->FlashCalibration(std::string(), sCode); 
            break; 
            
            default: 
            std::cout << "Unknown calibration action." << std::endl; 
            break; 
        } 
    }


    m_InuSensor.DeviceTerminate();

    return 0;
}
