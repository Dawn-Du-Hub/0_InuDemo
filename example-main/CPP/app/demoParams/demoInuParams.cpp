#include "inuCommon.h"
#include "inuParams.h"

#include "inuspdlog.h"
using namespace Inuchip;

int main(int argc, char* argv[])
{
    spdlog_init();

    std::string filename = "./../../../app/cfg/Params.yaml";
    InuParams m_InuParams(filename);


    return 0;
}