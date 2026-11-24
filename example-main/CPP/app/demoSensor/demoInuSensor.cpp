#include "inuCommon.h"
#include "inuSensor.h"
using namespace Inuchip;

int main(int argc, char* argv[])
{
    spdlog_init();

    InuSensor m_InuSensor;

    m_InuSensor.checkDeviceState();
    return 0;
}