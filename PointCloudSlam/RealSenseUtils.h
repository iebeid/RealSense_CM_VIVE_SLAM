#ifndef REAL_SENSE_UTILS
#define REAL_SENSE_UTILS 1

#include <pxcsensemanager.h>
#include <iostream>
#include <cm_rt.h>

PXCCapture::Device* real_sense_info(PXCCaptureManager *pCaptureManager);

PXCSenseManager* init_real_sense(int width, int height, PXCProjection** projection, PXCCapture::Device** device, PXCSession** session, PXCScenePerception** scene_perception);

PXCImage* map_color_to_depth(PXCImage* depth, PXCImage* color, PXCSession* pSession);

PXCImage* filter_depth(PXCImage * depth_image, PXCSession * pSession, int low_threshold, int high_threshold, CmKernel * kernel, CmQueue * pCmQueue, CmTask * pKernelArray, CmThreadSpace * pTS, SurfaceIndex * index0, SurfaceIndex * index1, short * pSysMemSrc, short * pSysMemDst);

#endif