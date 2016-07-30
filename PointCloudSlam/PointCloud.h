#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#undef APIENTRY

#include <memory>
#include <vector>
#include <iostream>
#include <vector>

#include <pxcsensemanager.h>

#include "Matrix.h"
#include "CmUtils.h"

using namespace std;

struct PXCColor{
	float blue;
	float green;
	float red;
};

struct Point
{
	vector3f position;
	vector3f normal_vector;
	PXCColor color;
	float distance_from_origin;
};

struct Render{
	GLfloat * vertices;
	GLfloat * colors;
};

struct Transformation{
	Matrix R;
	Matrix t;
};

class PointCloud
{
public:

	PointCloud(){ ; }
	PointCloud(vector<Point> p);
	PointCloud(PXCImage * mapped_color_to_depth, PXCImage * depth, PXCProjection * projection, short low_confidence, int point_cloud_resolution);
	~PointCloud(){ ; }

	void transform(PointCloud mo, Transformation trans);
	Render get_rendering_structures();
	Transformation align_point_cloud(CmDevice* cm_device, CmProgram* program, PointCloud mod, int number_of_points, Matrix *R, Matrix *t);

public:

	std::vector<Point> points;
};

struct GlobalMap{
	vector<PointCloud> point_clouds;
};