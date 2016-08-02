#ifndef APP_H
#define APP_H 1
#include <pxcsensemanager.h>
#include <pxcsceneperception.h>
#include <cm_rt.h>
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
#endif