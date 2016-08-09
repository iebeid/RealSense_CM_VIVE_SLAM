#ifndef POINT_CLOUD_H
#define POINT_CLOUD_H 1

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#undef APIENTRY

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
	PXCColor normal_color;
	float distance_from_origin;
};

struct Render{
	GLfloat * vertices;
	GLfloat * colors;
	GLfloat * normal_colors;
};

class Transformation{
public:
	Transformation(){
		R = Matrix::eye(3);
		t = Matrix(3, 1);
	}
public:
	Matrix R;
	Matrix t;
};

class PointCloud
{
public:

	PointCloud(){ ; }
	PointCloud(vector<Point> p);
	PointCloud(PXCImage * rgb_frame, PXCImage * depth_frame, PXCImage * mapped_rgb_frame, PXCSenseManager * sense_manager, PXCProjection * projection, int depth_threshold, int point_cloud_resolution);
	~PointCloud();

	void transform(PointCloud mo, Transformation trans);
	static PointCloud transform_glm(PointCloud mo, Transformation trans);
	Render get_rendering_structures();
	Transformation align_point_cloud(CmDevice* cm_device, CmProgram* program, PointCloud mod, int number_of_points, Matrix *R, Matrix *t);
	void terminate(Render rs);

public:

	std::vector<Point> points;
	PXCScenePerception * scene_perception;
};

struct GlobalMap{
	vector<PointCloud> point_clouds;
};
#endif