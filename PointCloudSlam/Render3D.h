#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <time.h>

#include <iostream>
#include <cmath>

#include "Camera.h"
#include "PointCloud.h"

using namespace std;

#define PI 3.14159265358

GLFWwindow* initWindow(const int resX, const int resY, int pos_x, int pos_y, const char * title);

void idle(GLFWwindow* window);

void show(GLFWwindow* window, int n, GLfloat *vertices, GLfloat *colors, Camera cam);

void show_map(GLFWwindow* window, int n, GLfloat *vertices, GLfloat *colors);

void show_points(GLFWwindow* window, GlobalMap s, Camera camera, Camera realsense);

int terminate_point_renderer(GLFWwindow* window);