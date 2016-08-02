#include <memory>
#include <vector>
#include <iostream>
#include <vector>

#include "PointCloud.h"

PointCloud::PointCloud(PXCImage * mapped_color_to_depth, PXCImage * depth, PXCProjection * projection, short low_confidence, int point_cloud_resolution){

	PXCImage *depth_image = depth;
	PXCImage::ImageInfo depth_info = depth_image->QueryInfo();
	PXCImage::ImageData depth_data;
	depth_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &depth_data);
	short *dpixels = (short*)depth_data.planes[0];
	depth_image->ReleaseAccess(&depth_data);

	PXCImage::ImageInfo mapped_info = mapped_color_to_depth->QueryInfo();
	PXCImage::ImageData mapped_data;
	mapped_color_to_depth->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &mapped_data);
	pxcBYTE * mpixels = mapped_data.planes[0];
	int mpitch = mapped_data.pitches[0];
	mapped_color_to_depth->ReleaseAccess(&mapped_data);


	int res_height = depth_info.height;
	int res_width = depth_info.width;
	int number_of_elements_pushed = 0;
	for (int v = 0; v < res_height; v = v + point_cloud_resolution){
		for (int u = 0; u < res_width; u = u + point_cloud_resolution){
			int idx = v*res_width + u;
			GLfloat red = (GLfloat)(mpixels + v*mpitch)[4 * u + 0] / 255;
			GLfloat green = (GLfloat)(mpixels + v*mpitch)[4 * u + 1] / 255;
			GLfloat blue = (GLfloat)(mpixels + v*mpitch)[4 * u + 2] / 255;
			short depth_value = dpixels[idx];
			PXCPoint3DF32 xyzpoint;
			PXCPoint3DF32 uvzpoint;
			PXCColor color;
			uvzpoint.x = (pxcF32)u;
			uvzpoint.y = (pxcF32)v;
			uvzpoint.z = depth_value;
			if (depth_value < 1500 && depth_value > low_confidence){
				projection->ProjectDepthToCamera(1, &uvzpoint, &xyzpoint);
				xyzpoint.x = -xyzpoint.x / 1000;
				xyzpoint.y = -xyzpoint.y / 1000;
				xyzpoint.z = xyzpoint.z / 1000;
				color.red = red;
				color.blue = blue;
				color.green = green;
				Point p;
				p.position.x = xyzpoint.x;
				p.position.y = xyzpoint.y;
				p.position.z = xyzpoint.z;
				p.distance_from_origin = sqrt(pow(p.position.x, 2) + pow(p.position.y, 2) + pow(p.position.z, 2));
				p.color = color;
				//Normal Vector
				PXCPoint3DF32 horizontal_neighbor_uvz;
				PXCPoint3DF32 horizontal_neighbor_xyz;
				horizontal_neighbor_uvz.x = (pxcF32)u + 1;
				horizontal_neighbor_uvz.y = (pxcF32)v;
				horizontal_neighbor_uvz.z = dpixels[(v*res_width + (u + 1))];
				projection->ProjectDepthToCamera(1, &horizontal_neighbor_uvz, &horizontal_neighbor_xyz);

				PXCPoint3DF32 vertical_neighbor_uvz;
				PXCPoint3DF32 vertical_neighbor_xyz;
				vertical_neighbor_uvz.x = (pxcF32)u;
				vertical_neighbor_uvz.y = (pxcF32)v + 1;
				vertical_neighbor_uvz.z = dpixels[((v + 1)*res_width + u)];
				projection->ProjectDepthToCamera(1, &vertical_neighbor_uvz, &vertical_neighbor_xyz);

				vector3f current_point;
				current_point.x = xyzpoint.x;
				current_point.y = xyzpoint.y;
				current_point.z = xyzpoint.z;
				vector3f horizontal_point;
				horizontal_point.x = horizontal_neighbor_xyz.x;
				horizontal_point.y = horizontal_neighbor_xyz.y;
				horizontal_point.z = horizontal_neighbor_xyz.z;
				vector3f vertical_point;
				vertical_point.x = vertical_neighbor_xyz.x;
				vertical_point.y = vertical_neighbor_xyz.y;
				vertical_point.z = vertical_neighbor_xyz.z;

				vector3f horizontal_difference = vector3f::subtract(horizontal_point, current_point);
				vector3f vertical_difference = vector3f::subtract(vertical_point, current_point);

				vector3f normal_vector = vector3f::normalize(vector3f::cross(horizontal_difference, vertical_difference));
				p.normal_vector.x = normal_vector.x;
				p.normal_vector.y = normal_vector.y;
				p.normal_vector.z = normal_vector.z;

				if (p.distance_from_origin > 0.0f){
					this->points.push_back(p);
					number_of_elements_pushed++;
				}
			}
		}
	}
	this->points.resize(number_of_elements_pushed);
}

PointCloud::PointCloud(vector<Point> p){
	this->points = p;
	memcpy(&this->points[0], &p[0], p.size()*sizeof(Point));
}

Render PointCloud::get_rendering_structures(){
	Render rs;
	rs.vertices = new GLfloat[this->points.size() * 3];
	rs.colors = new GLfloat[this->points.size() * 3];
	int t = 0;
	for (int j = 0; j < this->points.size(); j++)
	{
		Point p = this->points[j];
		rs.vertices[t] = p.position.x;
		rs.vertices[t + 1] = p.position.y;
		rs.vertices[t + 2] = p.position.z;
		rs.colors[t] = p.color.blue;
		rs.colors[t + 1] = p.color.green;
		rs.colors[t + 2] = p.color.red;
		t = t + 3;
	}
	return rs;
}

void PointCloud::transform(PointCloud mo, Transformation trans){
	Matrix translation(3, 1);
	translation.val[0][0] = (float)trans.t.val[0][0];
	translation.val[1][0] = (float)trans.t.val[0][1];
	translation.val[2][0] = (float)trans.t.val[0][2];

	Matrix rot_mat(3, 3);
	rot_mat.val[0][0] = trans.R.val[0][0];
	rot_mat.val[0][1] = trans.R.val[1][0];
	rot_mat.val[0][2] = trans.R.val[2][0];
	rot_mat.val[1][0] = trans.R.val[0][1];
	rot_mat.val[1][1] = trans.R.val[1][1];
	rot_mat.val[1][2] = trans.R.val[2][1];
	rot_mat.val[2][0] = trans.R.val[0][2];
	rot_mat.val[2][1] = trans.R.val[1][2];
	rot_mat.val[2][2] = trans.R.val[2][2];

	for (int i = 0; i < mo.points.size(); i++){
		Point p;
		Matrix v(3, 1);
		v.val[0][0] = mo.points[i].position.x;
		v.val[1][0] = mo.points[i].position.y;
		v.val[2][0] = mo.points[i].position.z;
		Matrix n(3, 1);
		n.val[0][0] = mo.points[i].normal_vector.x;
		n.val[1][0] = mo.points[i].normal_vector.y;
		n.val[2][0] = mo.points[i].normal_vector.z;
		PXCColor c = mo.points[i].color;
		Matrix new_vertex_vector = rot_mat * v + translation;
		Matrix new_normal_vector = rot_mat * n + translation;
		p.position.x = (float)new_vertex_vector.val[0][0];
		p.position.y = (float)new_vertex_vector.val[1][0];
		p.position.z = (float)new_vertex_vector.val[2][0];
		p.normal_vector.x = (float)new_normal_vector.val[0][0];
		p.normal_vector.y = (float)new_normal_vector.val[1][0];
		p.normal_vector.z = (float)new_normal_vector.val[2][0];
		p.color.red = c.red;
		p.color.green = c.green;
		p.color.blue = c.blue;
		p.distance_from_origin = sqrt(pow(p.position.x, 2) + pow(p.position.y, 2) + pow(p.position.z, 2));
		this->points.push_back(p);
	}
}

Transformation PointCloud::align_point_cloud(CmDevice* cm_device, CmProgram* program, PointCloud mod, int number_of_points, Matrix *R, Matrix *t){
	PointCloud current(this->points);
	

	Transformation f;
	f.R = *R;
	f.t = *t;

	return f;
}