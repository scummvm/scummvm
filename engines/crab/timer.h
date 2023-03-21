//=============================================================================
// Author:   Arvind
// Purpose:  Timer class
//=============================================================================
#pragma once
#include "common_header.h"

#include "loaders.h"

class Timer
{
private:
	//The clock time when the timer started
	Uint32 start_ticks;

	//The ticks stored when the timer was paused
	Uint32 paused_ticks;

	//Since most timers usually only check one value, we might as well as store it here
	Uint32 target_ticks;

	//See if we have a target loaded or set
	bool target_valid;

	//The timer status
	bool paused;
	bool started;

public:
	//Initialize variables
	Timer();
	void Target(const Uint32 &val) { target_valid = true; target_ticks = val; }

	void Load(rapidxml::xml_node<char> *node, const std::string &name, const bool &echo = true);

	//The various clock actions
	void Start();
	void Stop();
	void Pause();
	void Resume();

	//Gets the timer's time
	Uint32 Ticks();

	//Get the time remaining
	Uint32 RemainingTicks() { return target_ticks - Ticks(); }

	//Have we reached the target yet?
	bool TargetReached(const float &factor = 1.0f);

	//Checks the status of the timer
	const bool Started() { return started; }
	const bool Paused() { return paused; }
	const bool TargetValid() { return target_valid; }
};