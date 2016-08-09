#ifndef VRPN_H
#define VRPN_H 1
#include <vrpn_Text.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_Connection.h>

class myTracker : public vrpn_Tracker
{
public:
	myTracker(vrpn_Connection *c = 0);
	virtual ~myTracker() {};

	virtual void mainloop();

protected:
	struct timeval _timestamp;
	float x, y, z, qx, qy, qz, qw;
};
#endif