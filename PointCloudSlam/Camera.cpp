#include "Camera.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <pxcsensemanager.h>
#include "Camera.h"
using namespace std;

void Camera::set_rotation(float * rotation){
	this->rotation = rotation;
}

void Camera::set_position(float * position){
	this->position = position;
}

float * Camera::get_rotation(){
	return this->rotation;
}

float * Camera::get_position(){
	return this->position;
}