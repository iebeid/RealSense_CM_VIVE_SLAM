//#ifndef RENDER_HMD_H
//#define RENDER_HMD_H 1
//#include <pxcsensemanager.h>
//#include <pxcsceneperception.h>
//void setup_vr();
//void render_vr(PXCSenseManager * sense_manager);
//void terminate_vr();
//#endif


#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <pxcsensemanager.h>

#include <openvr.h>

#include <iostream>

#include "FPS.h"

using namespace std;



GLFWwindow* setup_vr(int width, int height, int pos_x, int pos_y, const char * title);

void create_frame_buffer(int frame_buffer_width, int frame_buffer_height);

// https://www.opengl.org/discussion_boards/showthread.php/184651-Loading-a-bitmap-image-to-use-it-as-a-texture-background-on-canvas-for-drawing
void render_window_ar(const GLvoid * image, int width, int height, int frame_buffer_width, int frame_buffer_height, GLFWwindow* window, float fov);

void render_vive(const GLvoid * image);