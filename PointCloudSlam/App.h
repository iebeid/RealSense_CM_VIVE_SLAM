#ifndef APP_H
#define APP_H 1
#include <pxcsensemanager.h>
#include <pxcsceneperception.h>
#include <cm_rt.h>
#include <thread>
class App{
public:
	static App& getInstance()
	{
		static App instance;
		return instance;
	}
	
	void run();

	void terminate();

private:
	App(){};
	~App(){};

public:
	App(App const&) = delete;
	void operator=(App const&) = delete;
	
};
#endif