#include "RenderHMD.h"

GLFWwindow* setup_window(int width, int height, int pos_x, int pos_y, const char * title){
	glfwInit();
	window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowPos(window, pos_x, pos_y);
	glewExperimental = GL_TRUE;
	glewInit();

	return window;
}

void create_frame_buffer(int frame_buffer_width, int frame_buffer_height){

	glGenTextures(1, &textureIdLeft);
	glBindTexture(GL_TEXTURE_2D, textureIdLeft);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frame_buffer_width, frame_buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenFramebuffers(1, &fboIdLeft);
	glBindFramebuffer(GL_FRAMEBUFFER, fboIdLeft);
	glGenRenderbuffers(1, &rboIdLeft);
	glBindRenderbuffer(GL_RENDERBUFFER, rboIdLeft);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, frame_buffer_width, frame_buffer_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureIdLeft, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboIdLeft);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glGenTextures(1, &textureIdRight);
	glBindTexture(GL_TEXTURE_2D, textureIdRight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, frame_buffer_width, frame_buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	glGenFramebuffers(1, &fboIdRight);
	glBindFramebuffer(GL_FRAMEBUFFER, fboIdRight);
	glGenRenderbuffers(1, &rboIdRight);
	glBindRenderbuffer(GL_RENDERBUFFER, rboIdRight);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, frame_buffer_width, frame_buffer_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureIdRight, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboIdRight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

// https://www.opengl.org/discussion_boards/showthread.php/184651-Loading-a-bitmap-image-to-use-it-as-a-texture-background-on-canvas-for-drawing
void render_window_ar(const GLvoid * image, int width, int height, int frame_buffer_width, int frame_buffer_height, GLFWwindow* window, float fov)
{

	glBindFramebuffer(GL_FRAMEBUFFER, fboIdLeft);

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, frame_buffer_width, frame_buffer_height, 0.0, 0.0, 1.0);
	glDrawPixels(frame_buffer_width, frame_buffer_height, GL_BGRA, GL_UNSIGNED_BYTE, image);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	gluPerspective(fov, width / height, 0.1, 100);
	glTranslatef(0.0f, -1.0f, -5.0f);
	glRotatef(0.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
	glInterleavedArrays(GL_C4UB_V3F, 0, g_lineVertices_vive);
	glDrawArrays(GL_LINES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	const vr::Texture_t tex_1 = { reinterpret_cast<void*>(intptr_t(textureIdLeft)), vr::API_OpenGL, vr::ColorSpace_Gamma };

	vr::VRCompositor()->Submit(vr::EVREye(1), &tex_1);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboIdLeft);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glBlitFramebuffer(0, 0, frame_buffer_width, frame_buffer_height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fboIdRight);

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, frame_buffer_width, frame_buffer_height, 0.0, 0.0, 1.0);
	glDrawPixels(frame_buffer_width, frame_buffer_height, GL_BGRA, GL_UNSIGNED_BYTE, image);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	gluPerspective(fov, width / height, 0.1, 100);
	glTranslatef(0.0f, -1.0f, -5.0f);
	glRotatef(0.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
	glInterleavedArrays(GL_C4UB_V3F, 0, g_lineVertices_vive);
	glDrawArrays(GL_LINES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	const vr::Texture_t tex_2 = { reinterpret_cast<void*>(intptr_t(textureIdRight)), vr::API_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::EVREye(2), &tex_2);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboIdRight);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glBlitFramebuffer(0, 0, frame_buffer_width, frame_buffer_height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	vr::VRCompositor()->PostPresentHandoff();

	glfwSwapBuffers(window);
}

void render_vive(){
	int fb_width = 640;
	int fb_height = 480;
	GLFWwindow *ar_window = setup_window(fb_width, fb_height, 100, 100, "Augmented Reality");
	PXCSession *pSession = PXCSession::CreateInstance();
	PXCSenseManager *pSenseManager = pSession->CreateSenseManager();
	pSenseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, fb_width, fb_height, 60);
	pSenseManager->Init();
	cout << "Camera Initialized" << endl;
	PXCCapture::Device * device = pSenseManager->QueryCaptureManager()->QueryDevice();
	PXCPointF32 fov = device->QueryColorFieldOfView();
	FPS f_c;

	uint32_t recommended_fb_width = 1280;
	uint32_t recommended_fb_height = 720;
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem* hmd;
	hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
	hmd->GetRecommendedRenderTargetSize(&recommended_fb_width, &recommended_fb_height);
	const int windowHeight = 720;
	const int windowWidth = (recommended_fb_width * windowHeight) / recommended_fb_height;

	create_frame_buffer(recommended_fb_width, recommended_fb_height);

	vr::IVRCompositor* compositor = vr::VRCompositor();
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];

	while (pSenseManager->AcquireFrame(true) >= PXC_STATUS_NO_ERROR) {
		f_c.start_fps_counter();
		PXCCapture::Sample *sample = pSenseManager->QuerySample();
		PXCImage *color_image = sample->color;
		PXCImage::ImageInfo color_info = color_image->QueryInfo();
		PXCImage::ImageData color_data;
		color_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &color_data);

		//VR
		const float nearPlaneZ = -0.1f;
		const float farPlaneZ = -100.0f;
		vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
		const vr::HmdMatrix34_t& ltMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Left);
		const vr::HmdMatrix34_t& rtMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Right);
		const vr::HmdMatrix44_t& ltProj = hmd->GetProjectionMatrix(vr::Eye_Left, -nearPlaneZ, -farPlaneZ, vr::API_OpenGL);
		const vr::HmdMatrix44_t& rtProj = hmd->GetProjectionMatrix(vr::Eye_Right, -nearPlaneZ, -farPlaneZ, vr::API_OpenGL);
		//
		render_window_ar(color_data.planes[0], windowWidth, windowHeight, recommended_fb_width, recommended_fb_height, ar_window, fov.y);

		color_image->ReleaseAccess(&color_data);
		pSenseManager->ReleaseFrame();
		f_c.end_fps_counter();
		f_c.print_fps();
	}
}