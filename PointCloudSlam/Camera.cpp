
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <pxcsensemanager.h>

#include "Matrix.h"

#include "Camera.h"

#define PI 3.14159265358

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

float * Camera::get_rotation_matrix_glm(float angle_x, float angle_y, float angle_z){
	float * rot = new float[9];
	glm::mat3 rotation_matrix = glm::mat3();
	glm::mat3 rotation_x(1, 0, 0, 0, cos((angle_x*PI) / 180), -sin((angle_x*PI) / 180), 0, sin((angle_x*PI) / 180), cos((angle_x*PI) / 180));
	glm::mat3 rotation_y(cos((angle_y*PI) / 180), 0, sin((angle_y*PI) / 180), 0, 1, 0, -sin((angle_y*PI) / 180), 0, cos((angle_y*PI) / 180));
	glm::mat3 rotation_z(cos((angle_z*PI) / 180), -sin((angle_z*PI) / 180), 0, sin((angle_z*PI) / 180), cos((angle_z*PI) / 180), 0, 0, 0, 1);
	rotation_matrix = rotation_z * rotation_y * rotation_x;
	rot[0] = rotation_matrix[0][0];
	rot[1] = rotation_matrix[1][0];
	rot[2] = rotation_matrix[2][0];
	rot[3] = rotation_matrix[0][1];
	rot[4] = rotation_matrix[2][2];
	rot[5] = rotation_matrix[2][1];
	rot[6] = rotation_matrix[0][2];
	rot[7] = rotation_matrix[1][2];
	rot[8] = rotation_matrix[1][1];
	return rot;
}

float * Camera::get_rotation_matrix(float angle_x, float angle_y, float angle_z){
	float * rot = new float[9];

	Matrix rotation_x(3, 3);
	rotation_x.val[0][0] = 1;
	rotation_x.val[0][1] = 0;
	rotation_x.val[0][2] = 0;
	rotation_x.val[1][0] = 0;
	rotation_x.val[1][1] = cos((angle_x*PI) / 180);
	rotation_x.val[1][2] = -sin((angle_x*PI) / 180);
	rotation_x.val[2][0] = 0;
	rotation_x.val[2][1] = sin((angle_x*PI) / 180);
	rotation_x.val[2][2] = cos((angle_x*PI) / 180);

	Matrix rotation_y(3, 3);
	rotation_y.val[0][0] = cos((angle_y*PI) / 180);
	rotation_y.val[0][1] = 0;
	rotation_y.val[0][2] = sin((angle_y*PI) / 180);
	rotation_y.val[1][0] = 0;
	rotation_y.val[1][1] = 1;
	rotation_y.val[1][2] = 0;
	rotation_y.val[2][0] = -sin((angle_y*PI) / 180);
	rotation_y.val[2][1] = 0;
	rotation_y.val[2][2] = cos((angle_y*PI) / 180);

	Matrix rotation_z(3, 3);
	rotation_z.val[0][0] = cos((angle_z*PI) / 180);
	rotation_z.val[0][1] = -sin((angle_z*PI) / 180);
	rotation_z.val[0][2] = 0;
	rotation_z.val[1][0] = sin((angle_z*PI) / 180);
	rotation_z.val[1][1] = cos((angle_z*PI) / 180);
	rotation_z.val[1][2] = 0;
	rotation_z.val[2][0] = 0;
	rotation_z.val[2][1] = 0;
	rotation_z.val[2][2] = 1;

	Matrix rotation_matrix = rotation_z * rotation_y * rotation_x;
	rot[0] = (float)rotation_matrix.val[0][0];
	rot[1] = (float)rotation_matrix.val[0][1];
	rot[2] = (float)rotation_matrix.val[0][2];
	rot[3] = (float)rotation_matrix.val[1][0];
	rot[4] = (float)rotation_matrix.val[1][1];
	rot[5] = (float)rotation_matrix.val[1][2];
	rot[6] = (float)rotation_matrix.val[2][0];
	rot[7] = (float)rotation_matrix.val[2][1];
	rot[8] = (float)rotation_matrix.val[2][2];
	return rot;
}