#pragma once

#include <pxcsensemanager.h>

#include <iostream>

#include "CmUtils.h"

using namespace std;

PXCCapture::Device* real_sense_info(PXCCaptureManager *pCaptureManager);

PXCSenseManager* init_real_sense(int width, int height, PXCProjection** projection, PXCCapture::Device** device, PXCSession** session);

PXCImage* map_color_to_depth(PXCImage* depth, PXCImage* color, short low_confidence, PXCSession* pSession);

PXCImage* filter_depth(PXCImage* depth, PXCSession* pSession, int low_threshold, int high_threshold, CmDevice* device, CmProgram* program);