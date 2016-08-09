#ifndef FPS_H
#define FPS_H 1

#include <ctime>
// Frame Counter Class
class FPS{
public:
	FPS();
	~FPS();
	void start_fps_counter();
	void end_fps_counter();
	void print_fps();

private:
	//Frame counter
	int frames;
	float starttime;
	bool first;
	//Number of frame per seconds
	float nof;
	std::clock_t start;
	float timepassed;
};

#endif