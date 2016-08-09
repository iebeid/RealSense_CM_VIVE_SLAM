#ifndef APP_H
#define APP_H 1
//Main app class singltone style
class App{
public:
	static App& getInstance()
	{
		static App instance;
		return instance;
	}
	//App entry point
	void run();
	//App terminate
	void terminate();

private:
	App(){};
	~App(){};

public:
	App(App const&) = delete;
	void operator=(App const&) = delete;
	
};
#endif