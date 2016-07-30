#pragma once


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <pxcsensemanager.h>
#include "Matrix.h"

#define PI 3.14159265358

using namespace std;

class Camera{
public:
	Camera(int w, int h) :width(w), height(h){
		rotation = new float[9]{0, };
		position = new float[3]{0, };
	}
	~Camera(){ ; }
	void set_rotation(float * rotation);
	void set_position(float * position);
	float * get_rotation();
	float * get_position();
	float * Camera::get_rot_matrix(float angle_x, float angle_y, float angle_z);

private:
	int width;
	int height;
	float * rotation;
	float * position;

};
