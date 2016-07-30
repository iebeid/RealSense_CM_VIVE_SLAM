#pragma once

#include "PointCloud.h"
#include "Queue.h"
#include "RealSenseUtils.h"
#include "CmUtils.h"
#include "Render2D.h"
#include "Render3D.h"
#include "FPS.h"

#include <thread>

class App{
public:
	static App& getInstance()
	{
		static App instance;
		return instance;
	}
	
	void run(int width, int height, int point_cloud_res, PXCProjection * projection,
		PXCSenseManager *sense_manager, PXCCapture::Device* device, PXCSession *session, CmDevice* cm_device, CmProgram* program);

	void terminate();

private:
	App(){};
	~App(){};

public:
	App(App const&) = delete;
	void operator=(App const&) = delete;
	
};