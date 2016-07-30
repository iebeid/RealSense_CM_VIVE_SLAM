#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <pxcsensemanager.h>

#include <openvr.h>

#include <iostream>

#include "FPS.h"

using namespace std;

GLFWwindow* window;

GLuint fboIdLeft;
GLuint textureIdLeft;
GLuint rboIdLeft;

GLuint fboIdRight;
GLuint textureIdRight;
GLuint rboIdRight;

struct line_vertex_vive
{
	unsigned char r, g, b, a;
	float x, y, z;
};

line_vertex_vive g_lineVertices_vive[] =
{
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 1.0f, 0.0f, 0.0f },
	{ 0, 255, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 0, 255, 0, 255, 0.0f, 1.0f, 0.0f },
	{ 0, 0, 255, 255, 0.0f, 0.0f, 0.0f },
	{ 0, 0, 255, 255, 0.0f, 0.0f, 1.0f }
};

GLFWwindow* setup_window(int width, int height, int pos_x, int pos_y, const char * title);

void create_frame_buffer(int frame_buffer_width, int frame_buffer_height);

// https://www.opengl.org/discussion_boards/showthread.php/184651-Loading-a-bitmap-image-to-use-it-as-a-texture-background-on-canvas-for-drawing
void render_window_ar(const GLvoid * image, int width, int height, int frame_buffer_width, int frame_buffer_height, GLFWwindow* window, float fov);

void render_vive();