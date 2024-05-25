#include "stdafx.h"
#include "Core/CCore.h"

extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;

int main(int argc, char *argv[])
{
    CCore l_core(argc, argv);
    l_core.Launch();
    return 0;
}
