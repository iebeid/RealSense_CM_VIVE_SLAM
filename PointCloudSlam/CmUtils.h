#ifndef CM_UTILS_H
#define CM_UTILS_H 1

#include <cm_rt.h>

CmDevice* create_device();

CmProgram* load_kernel_program(CmDevice* pCmDev, const char * filename);

#endif