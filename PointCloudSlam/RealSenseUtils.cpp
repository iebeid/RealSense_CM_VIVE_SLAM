#include "CmUtils.h"

#include "RealSenseUtils.h"

PXCCapture::Device* real_sense_info(PXCCaptureManager *pCaptureManager){
	//Info
	PXCCapture::Device *device = pCaptureManager->QueryDevice();
	PXCCapture::DeviceInfo device_info = {};
	device->QueryDeviceInfo(&device_info);
	wprintf(device_info.name);
	cout << endl;
	cout << "Firmware: " << device_info.firmware[0] << "." << device_info.firmware[1] << "." << device_info.firmware[2] << "." << device_info.firmware[3] << endl;
	PXCPointF32 fov = device->QueryDepthFieldOfView();
	cout << "Depth Horizontal Field Of View: " << fov.x << endl;
	cout << "Depth Vertical Field Of View: " << fov.y << endl;
	PXCSizeI32 csize = pCaptureManager->QueryImageSize(PXCCapture::STREAM_TYPE_COLOR);
	cout << "Color Resolution: " << csize.width << " * " << csize.height << endl;
	PXCSizeI32 dsize = pCaptureManager->QueryImageSize(PXCCapture::STREAM_TYPE_DEPTH);
	cout << "Depth Resolution: " << dsize.width << " * " << dsize.height << endl;

	//Camera calibration
	cout << "Calibrating" << endl;
	device->SetDepthConfidenceThreshold(6);
	device->SetIVCAMFilterOption(5);
	device->SetIVCAMLaserPower(16);
	device->SetIVCAMMotionRangeTradeOff(16);
	device->SetIVCAMAccuracy(device->IVCAM_ACCURACY_MEDIAN);
	cout << "Depth Setting - OK - Calibrated" << endl;

	return device;
}

PXCSenseManager* init_real_sense(int width, int height, PXCProjection** projection, PXCCapture::Device** device, PXCSession** session){
	//Camera initialization
	PXCSession* current_session = PXCSession::CreateInstance();
	current_session->SetCoordinateSystem(PXCSession::CoordinateSystem::COORDINATE_SYSTEM_REAR_OPENCV);
	PXCSenseManager *sense_manager = current_session->CreateSenseManager();
	sense_manager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, width, height, 60);
	sense_manager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, width, height, 60);
	sense_manager->EnableScenePerception();
	sense_manager->Init();
	PXCCaptureManager *capture_manager = sense_manager->QueryCaptureManager();
	PXCCapture::Device* current_device = real_sense_info(capture_manager);
	PXCProjection * current_projection = current_device->CreateProjection();
	cout << "Camera Initialized" << endl;
	*device = current_device;
	*session = current_session;
	*projection = current_projection;
	return sense_manager;
}

PXCImage * filter_depth(PXCImage* depth, PXCSession* pSession, int low_threshold, int high_threshold, CmDevice* pCmDev, CmProgram* program){

	PXCImage * new_depth_image;

	PXCImage *depth_image = depth;
	PXCImage::ImageInfo depth_info = depth_image->QueryInfo();
	PXCImage::ImageData depth_data;
	depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &depth_data);
	short *dpixels = (short*)depth_data.planes[0];
	int dpitch = depth_data.pitches[0] / sizeof(short);
	depth_image->ReleaseAccess(&depth_data);

	int width = depth_info.width;
	int height = depth_info.height;




	//---------------------------------------------------------------
	//UINT pitch_inputSurf = 0;
	//UINT size_inputSurf = 0;
	//pCmDev->GetSurface2DInfo(width * 3 / 4, height, CM_SURFACE_FORMAT_D16, pitch_inputSurf, size_inputSurf);
	//CmSurface2DUP*  pInputSurf = NULL;
	//short *pSysMemSrc = (short*)CM_ALIGNED_MALLOC(size_inputSurf*sizeof(short), 0x1000);
	//pCmDev->CreateSurface2DUP(width * 3 / 4, height, CM_SURFACE_FORMAT_D16, pSysMemSrc, pInputSurf);

	short * new_depth_image_manual = (short *)malloc(height*width*sizeof(short) * 1);
	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){
			short depth_value = dpixels[(y * dpitch) + x];
			if (depth_value < high_threshold && depth_value > low_threshold){
				new_depth_image_manual[(y * dpitch) + x] = depth_value;
				//pSysMemSrc[(y * dpitch) + x] = depth_value;
			}
			else{
				new_depth_image_manual[(y * dpitch) + x] = 0;
				//pSysMemSrc[(y * dpitch) + x] = 0;
			}
		}
	}

	//UINT pitch_outputSurf = 0;
	//UINT size_outputSurf = 0;
	//pCmDev->GetSurface2DInfo(width * 3 / 4, height, CM_SURFACE_FORMAT_D16, pitch_outputSurf, size_outputSurf);
	//CmSurface2DUP*  pOutputSurf = NULL;
	//short* pSysMemDst = (short*)CM_ALIGNED_MALLOC(size_outputSurf*sizeof(short), 0x1000);
	//pCmDev->CreateSurface2DUP(width * 3 / 4, height, CM_SURFACE_FORMAT_D16, pSysMemDst, pOutputSurf);

	//// Create a kernel
	//CmKernel* kernel = NULL;
	//pCmDev->CreateKernel(program, CM_KERNEL_FUNCTION(linear), kernel);

	//int threadswidth = width * 3 / 24;
	//int threadsheight = height / 6;
	//kernel->SetThreadCount(threadswidth * threadsheight);
	//CmThreadSpace* pTS = NULL;
	//pCmDev->CreateThreadSpace(threadswidth, threadsheight, pTS);

	//SurfaceIndex * index0 = NULL;
	//pInputSurf->GetIndex(index0);
	//kernel->SetKernelArg(0, sizeof(SurfaceIndex), index0);
	//SurfaceIndex * index1 = NULL;
	//pOutputSurf->GetIndex(index1);
	//kernel->SetKernelArg(1, sizeof(SurfaceIndex), index1);

	//CmQueue* pCmQueue = NULL;
	//pCmDev->CreateQueue(pCmQueue);

	//CmTask *pKernelArray = NULL;

	//pCmDev->CreateTask(pKernelArray);


	//pKernelArray->AddKernel(kernel);


	//CmEvent* e = NULL;
	//pCmQueue->Enqueue(pKernelArray, e, pTS);

	//PXCImage::ImageInfo info = {};
	//info.format = PXCImage::PIXEL_FORMAT_DEPTH;
	//info.width = depth_info.width;
	//info.height = depth_info.height;
	//new_depth_image = pSession->CreateImage(&info);
	//PXCImage::ImageData data;
	//new_depth_image->AcquireAccess(PXCImage::ACCESS_WRITE, &data);
	//memcpy(data.planes[0], pSysMemDst, size_outputSurf*sizeof(short));
	//new_depth_image->ReleaseAccess(&data);

	//pCmDev->DestroySurface2DUP(pInputSurf);
	//pCmDev->DestroySurface2DUP(pOutputSurf);
	//pCmDev->DestroyTask(pKernelArray);
	//pCmDev->DestroyThreadSpace(pTS);
	//CM_ALIGNED_FREE(pSysMemSrc);
	//CM_ALIGNED_FREE(pSysMemDst);
	//---------------------------------------------------------------

	PXCImage::ImageInfo info = {};
	info.format = PXCImage::PIXEL_FORMAT_DEPTH;
	info.width = depth_info.width;
	info.height = depth_info.height;
	new_depth_image = pSession->CreateImage(&info);
	PXCImage::ImageData data;
	new_depth_image->AcquireAccess(PXCImage::ACCESS_WRITE, &data);
	memcpy(data.planes[0], new_depth_image_manual, depth_info.height*depth_info.width*sizeof(short) * 1);
	new_depth_image->ReleaseAccess(&data);

	free(new_depth_image_manual);



	return new_depth_image;
}

PXCImage * map_color_to_depth(PXCImage * depth, PXCImage * color, short low_confidence, PXCSession *pSession){
	//Image to be returned
	PXCImage * mapped_image;

	//Color image data
	PXCImage *color_image = color;
	PXCImage::ImageInfo color_info = color_image->QueryInfo();
	PXCImage::ImageData color_data;
	color_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &color_data);
	unsigned char * cpixels = (unsigned char *)color_data.planes[0];
	int cpitch = color_data.pitches[0];
	color_image->ReleaseAccess(&color_data);

	//Depth image data
	PXCImage *depth_image = depth;
	PXCImage::ImageInfo depth_info = depth_image->QueryInfo();
	PXCImage::ImageData depth_data;
	depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &depth_data);
	short *dpixels = (short*)depth_data.planes[0];
	int dpitch = depth_data.pitches[0] / sizeof(short);
	depth_image->ReleaseAccess(&depth_data);

	unsigned char * mapped_colored_image_manual = (unsigned char *)malloc(depth_info.height*depth_info.width*sizeof(pxcBYTE) * 4);
	int h = 0;
	for (int y = 0; y < depth_info.height; y++){
		for (int x = 0; x < depth_info.width; x++){
			short depth_value = dpixels[(y * dpitch) + x];
			pxcBYTE blue, green, red, alpha;
			if (depth_value != 0){
				blue = (pxcBYTE)(cpixels + y*cpitch)[4 * x + 0];
				green = (pxcBYTE)(cpixels + y*cpitch)[4 * x + 1];
				red = (pxcBYTE)(cpixels + y*cpitch)[4 * x + 2];
				alpha = (pxcBYTE)(cpixels + y*cpitch)[4 * x + 3];
			}
			else{
				blue = 0;
				green = 0;
				red = 0;
				alpha = 0;
			}
			mapped_colored_image_manual[h] = blue;
			mapped_colored_image_manual[h + 1] = green;
			mapped_colored_image_manual[h + 2] = red;
			mapped_colored_image_manual[h + 3] = alpha;
			h = h + 4;
		}
	}

	PXCImage::ImageInfo info = {};
	info.format = PXCImage::PIXEL_FORMAT_RGB32;
	info.width = depth_info.width;
	info.height = depth_info.height;
	mapped_image = pSession->CreateImage(&info);
	PXCImage::ImageData data;
	mapped_image->AcquireAccess(PXCImage::ACCESS_WRITE, &data);
	memcpy(data.planes[0], mapped_colored_image_manual, depth_info.height*depth_info.width*sizeof(pxcBYTE) * 4);
	mapped_image->ReleaseAccess(&data);
	delete mapped_colored_image_manual;

	return mapped_image;
}