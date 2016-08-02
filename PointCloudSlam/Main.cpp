#include <iostream>
#include "CmUtils.h"
#include "RealSenseUtils.h"
#include "App.h"

using namespace std;

int main(int argc, char argv[]) {
	cout << "Initialize CM" << endl;
	CmDevice* cm_device = create_device();
	CmProgram* program = load_kernel_program(cm_device, "slam_genx.isa");
	int width = 320;
	int height = 240;
	cout << "Initialize RealSense" << endl;
	PXCProjection * projection = NULL;
	PXCCapture::Device* device = NULL;
	PXCSession *session = NULL;
	PXCSenseManager *sense_manager = init_real_sense(width, height, &projection, &device, &session);
	
	App::getInstance().run(width, height, 4, projection,
		sense_manager, device, session, cm_device, program);

	return 0;
}