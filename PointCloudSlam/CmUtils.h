#ifndef CM_UTILS_H
#define CM_UTILS_H 1

#include <cm_rt.h>
//Create CM device
CmDevice* create_device();
//Load the GENX kernel program
CmProgram* load_kernel_program(CmDevice* pCmDev, const char * filename);

#endif