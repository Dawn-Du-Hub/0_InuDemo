
#pragma once

#include "inuSensor.h"
#include "inuRGB.h"
#include "inuDepth.h"
#include "inuIR.h"
namespace Inuchip{

    class InuStatus
    {
        public:
        InuSensor *p_inuSensor = nullptr;
        InuRGB    *p_inuRGB = nullptr;
        InuDepth  *p_inuDepth = nullptr;
        InuIR  *p_inuIR = nullptr;


        int Sensor_status = 0;  // 0: stop 1:start
        int RGB_status = 0;
        int Depth_status = 0;
        int IR_status = 0;


        void setSensorStatus(InuSensor* p_inuSensor,int status);
        void setRGBStatus(InuRGB *inuRGB, int status);
        void setDepthStatus(InuDepth  *inuDepth, int status);
        void setIRStatus(InuIR  *inuIR, int status);


        int getSensorStatus();
        int getRGBStatus();
        int getDepthStatus();
        int getIRStatus();

    };

    inline void InuStatus::setSensorStatus(InuSensor* inuSensor, int status){p_inuSensor = inuSensor; Sensor_status = status;};
    inline void InuStatus::setRGBStatus(InuRGB *inuRGB, int status){p_inuRGB = inuRGB; RGB_status = status;};
    inline void InuStatus::setDepthStatus(InuDepth  *inuDepth, int status){p_inuDepth = inuDepth; Depth_status = status;};
    inline void InuStatus::setIRStatus(InuIR  *inuIR, int status){p_inuIR = inuIR; IR_status = status;};


    inline int InuStatus::getSensorStatus( ){return Sensor_status;};
    inline int InuStatus::getRGBStatus( ){return RGB_status;};
    inline int InuStatus::getDepthStatus( ){return Depth_status;};
    inline int InuStatus::getIRStatus( ){return IR_status;};
}