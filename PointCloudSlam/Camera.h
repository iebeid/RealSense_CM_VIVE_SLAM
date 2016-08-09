#ifndef CAMERA_H
#define CAMERA_H 1
// Camera class to model the rendering virtual camera and the RealSense sensor
class Camera{
public:
	Camera(){
		rotation = new float[9]{0, };
		position = new float[3]{0, };
	}
	~Camera(){ ; }
	void set_rotation(float * rotation);
	void set_position(float * position);
	float * get_rotation();
	float * get_position();

private:
	float * rotation;
	float * position;

};
#endif