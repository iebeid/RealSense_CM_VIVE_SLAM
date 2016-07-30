#pragma once


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;

enum FORMAT { DEPTH, COLOR, DEPTH32, CONFIDENCE };

GLFWwindow* setup_window_frame(int width, int height, int pos_x, int pos_y, const char * title);

void render_window(const GLvoid * image, int width, int height, GLFWwindow* window, FORMAT f);

void terminate_window(GLFWwindow* window);

