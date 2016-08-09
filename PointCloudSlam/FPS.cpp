#include "FPS.h"
#include <iostream>
using namespace std;

FPS::FPS(){
	frames = 0;
	starttime = 0;
	timepassed = 1;
	first = true;
	nof = 0.0f;
	start = std::clock();
}

FPS::~FPS(){

}

void FPS::start_fps_counter(){
	if (first)
	{
		frames = 0;
		starttime = timepassed;
		first = false;
	}
	frames++;
}

void FPS::end_fps_counter(){
	timepassed = (std::clock() - start) / (float)CLOCKS_PER_SEC;
	if (timepassed - starttime > 0.25 && frames > 10)
	{
		nof = (float)frames / ((float)timepassed - (float)starttime);
		starttime = timepassed;
		frames = 0;
	}
}

void FPS::print_fps(){
	cout << "Frame Rate: " << nof;
	cout.flush();
	cout << '\r';
}