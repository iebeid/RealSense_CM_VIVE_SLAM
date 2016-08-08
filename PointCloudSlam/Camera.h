#ifndef CAMERA_H
#define CAMERA_H 1
#include <glm\glm.hpp>
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
	static float * get_rotation_matrix_glm(float angle_x, float angle_y, float angle_z);
	static float * get_rotation_matrix(float angle_x, float angle_y, float angle_z);

private:
	int width;
	int height;
	float * rotation;
	float * position;

};
#endif