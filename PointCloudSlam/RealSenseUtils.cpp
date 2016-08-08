#include "CmUtils.h"
#include "RealSenseUtils.h"

using namespace std;

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

PXCSenseManager* init_real_sense(int width, int height, PXCProjection** projection, PXCCapture::Device** device, PXCSession** session, PXCScenePerception** scene_perception){
	//Camera initialization
	PXCSession* current_session = PXCSession::CreateInstance();
	PXCSenseManager *sense_manager = current_session->CreateSenseManager();
	sense_manager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, width, height, 60);
	sense_manager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, width, height, 60);
	sense_manager->EnableScenePerception();
	sense_manager->Init();
	PXCCaptureManager *capture_manager = sense_manager->QueryCaptureManager();
	PXCCapture::Device* current_device = real_sense_info(capture_manager);
	PXCProjection * current_projection = current_device->CreateProjection();
	PXCScenePerception* scene_perception_obj = sense_manager->QueryScenePerception();
	cout << "Camera Initialized" << endl;
	*device = current_device;
	*session = current_session;
	*projection = current_projection;
	*scene_perception = scene_perception_obj;
	return sense_manager;
}

PXCImage * filter_depth(PXCImage * depth_image, PXCSession * pSession, int low_threshold, int high_threshold, CmKernel * kernel, 
	CmQueue * pCmQueue, CmTask * pKernelArray, CmThreadSpace * pTS, SurfaceIndex * index0, SurfaceIndex * index1, short * pSysMemSrc, short * pSysMemDst){
	PXCImage::ImageInfo depth_info = depth_image->QueryInfo();
	PXCImage::ImageData depth_data;
	depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &depth_data);
	short *dpixels = (short*)depth_data.planes[0];
	int dpitch = depth_data.pitches[0] / sizeof(short);
	depth_image->ReleaseAccess(&depth_data);
	for (int y = 0; y < depth_info.height; y++){
		for (int x = 0; x < depth_info.width; x++){
			short depth_value = dpixels[(y * dpitch) + x];
			if (depth_value < high_threshold && depth_value > low_threshold){
				pSysMemSrc[(y * dpitch) + x] = depth_value;
			}
			else{
				pSysMemSrc[(y * dpitch) + x] = 0;
			}
		}
	}
	kernel->SetKernelArg(0, sizeof(SurfaceIndex), index0);
	kernel->SetKernelArg(1, sizeof(SurfaceIndex), index1);
	CmEvent* e = NULL;
	pCmQueue->Enqueue(pKernelArray, e, pTS);
	PXCImage * new_depth_image;
	PXCImage::ImageInfo info = {};
	info.format = PXCImage::PIXEL_FORMAT_DEPTH;
	info.width = depth_info.width;
	info.height = depth_info.height;
	new_depth_image = pSession->CreateImage(&info);
	PXCImage::ImageData data;
	new_depth_image->AcquireAccess(PXCImage::ACCESS_WRITE, &data);
	memcpy(data.planes[0], pSysMemDst, depth_info.height*depth_info.width*sizeof(short) * 1);
	new_depth_image->ReleaseAccess(&data);
	return new_depth_image;
}

PXCImage * map_color_to_depth(PXCImage * depth, PXCImage * color, PXCSession *pSession){
	PXCImage *color_image = color;
	PXCImage::ImageInfo color_info = color_image->QueryInfo();
	PXCImage::ImageData color_data;
	color_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &color_data);
	unsigned char * cpixels = (unsigned char *)color_data.planes[0];
	int cpitch = color_data.pitches[0];
	color_image->ReleaseAccess(&color_data);
	PXCImage *depth_image = depth;
	PXCImage::ImageInfo depth_info = depth_image->QueryInfo();
	PXCImage::ImageData depth_data;
	depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &depth_data);
	short *dpixels = (short*)depth_data.planes[0];
	int dpitch = depth_data.pitches[0] / sizeof(short);
	depth_image->ReleaseAccess(&depth_data);
	pxcBYTE * mapped_colored_image_manual = (pxcBYTE *)malloc(depth_info.height*depth_info.width*sizeof(pxcBYTE) * 4);
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
	PXCImage * mapped_image;
	PXCImage::ImageInfo info = {};
	info.format = PXCImage::PIXEL_FORMAT_RGB32;
	info.width = depth_info.width;
	info.height = depth_info.height;
	mapped_image = pSession->CreateImage(&info);
	PXCImage::ImageData data;
	mapped_image->AcquireAccess(PXCImage::ACCESS_WRITE, &data);
	memcpy(data.planes[0], mapped_colored_image_manual, depth_info.height*depth_info.width*sizeof(pxcBYTE) * 4);
	free(mapped_colored_image_manual);
	mapped_image->ReleaseAccess(&data);
	return mapped_image;
}