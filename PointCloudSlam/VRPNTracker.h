#include <vrpn_Text.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_Connection.h>

using namespace std;


// your tracker class must inherit from the vrpn_Tracker class
class myTracker : public vrpn_Tracker
{
public:
	myTracker(vrpn_Connection *c = 0);
	virtual ~myTracker() {};

	virtual void mainloop();

protected:
	struct timeval _timestamp;
};

myTracker::myTracker(vrpn_Connection *c /*= 0 */) :
vrpn_Tracker("Tracker0", c)
{
}

float x, y, z;
float qx, qy, qz, qw;

void
myTracker::mainloop()
{
	vrpn_gettimeofday(&_timestamp, NULL);

	vrpn_Tracker::timestamp = _timestamp;

	
	x = 0, y = 0, z = 0, qx = 0, qy = 0, qz = 0, qw = 1;
	
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;

	d_quat[0] = qx;
	d_quat[1] = qy;
	d_quat[2] = qz;
	d_quat[3] = qw;

	char msgbuf[1000];

	d_sensor = 0;

	int  len = vrpn_Tracker::encode_to(msgbuf);

	if (d_connection->pack_message(len, _timestamp, position_m_id, d_sender_id, msgbuf,
		vrpn_CONNECTION_LOW_LATENCY))
	{
		fprintf(stderr, "can't write message: tossing\n");
	}

	server_mainloop();
}