#pragma once

#include <cm_rt.h>

CmDevice* create_device();

CmProgram* load_kernel_program(CmDevice* pCmDev, const char * filename);
