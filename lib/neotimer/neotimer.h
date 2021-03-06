#pragma once

#define NEOTIMER_INDEFINITE -1
#define NEOTIMER_UNLIMITED -1


#include <Arduino.h>

class Neotimer{
	public:
	//Methods
	Neotimer();
	Neotimer(long _t);      //Constructor
	~Neotimer();            //Destructor
		
	void init();            //Initializations
	boolean done();         //Indicates time has elapsed
	boolean repeat(int times);
	boolean repeat(int times, long _t);
	boolean repeat();
	void repeatReset();
	boolean waiting();			// Indicates timer is started but not finished
	boolean started();			// Indicates timer has started
	void start();			//Starts a timer
	long stop();			//Stops a timer and returns elapsed time
	void restart();
	void reset();           //Resets timer to zero
	void set(long t);
	long get();
	boolean debounce(boolean signal);
	int repetitions = NEOTIMER_UNLIMITED;
	
	private:

	struct {
		unsigned long time;
		unsigned long last;
		boolean done;
		boolean started;
	} _timer;

	boolean _waiting;
};
