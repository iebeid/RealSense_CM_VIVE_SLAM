#pragma once
#include <ctime>
#include <iostream>

class FPS{
public:
	FPS();
	~FPS();
	void start_fps_counter();
	void end_fps_counter();
	void print_fps();

private:
	int frames;
	float starttime;
	bool first;
	float nof;
	std::clock_t start;
	float timepassed;
};