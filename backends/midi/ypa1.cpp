#include "stdafx.h"
#include "sound/mpu401.h"
#include "common/engine.h"	// for warning/error/debug

#include "Pa1Lib.h"

class MidiDriver_YamahaPa1:public MidiDriver_MPU401 {
public:
	MidiDriver_YamahaPa1();
	int open();
	void close();
	void send(uint32 b);

private:
	UInt8 _midiHandle;
	Boolean _isOpen;
 };

MidiDriver_YamahaPa1::MidiDriver_YamahaPa1() {
	_isOpen = false;
	_midiHandle = 0;
}

int MidiDriver_YamahaPa1::open() {
	if (!(_isOpen = Pa1Lib_midiOpen(NULL, &_midiHandle)))
		return MERR_DEVICE_NOT_AVAILABLE;

	return 0;
}

void MidiDriver_YamahaPa1::close() {
	MidiDriver_MPU401::close();
	if (_isOpen) {
		for (UInt8 channel = 0; channel < 16; channel++) {
			Pa1Lib_midiControlChange(_midiHandle, channel, 120,0); // all sound off
			Pa1Lib_midiControlChange(_midiHandle, channel, 121,0); // reset all controller
			Pa1Lib_midiControlChange(_midiHandle, channel, 123, 0); // all notes off
		}
		Pa1Lib_midiClose(_midiHandle);
		_isOpen = false;
	}
}

void MidiDriver_YamahaPa1::send(uint32 b) {
	if (!_isOpen)
		return;

	UInt8 midiCmd[4];
	UInt8 chanID,mdCmd;

	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);
	
	chanID = (midiCmd[0] & 0x0F) ;
	mdCmd = midiCmd[0] & 0xF0;
	
	switch (mdCmd) {
		case 0x80:	// note off
			Pa1Lib_midiNoteOff(_midiHandle, chanID, midiCmd[1], 0);
			break;
	
		case 0x90:	// note on
			Pa1Lib_midiNoteOn(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;
		
		case 0xB0:	// control change
			Pa1Lib_midiControlChange(_midiHandle, chanID, midiCmd[1], midiCmd[2]);
			break;
		
		case 0xC0:	// progam change
			Pa1Lib_midiProgramChange(_midiHandle, chanID, midiCmd[1]);
			break;
		
		case 0xE0:	// pitchBend
			Pa1Lib_midiPitchBend(_midiHandle, chanID, (short)(midiCmd[1] | (midiCmd[2] << 8)));
			break;
	}
}

MidiDriver *MidiDriver_YamahaPa1_create() {
	return new MidiDriver_YamahaPa1();
}

//////////////////////////////////////////
// thread emu
static struct {
	bool active;
	int old_time;
	int sleep;

} g_thread;

static bool t_first_call = false;

int MidiDriver_MPU401::midi_driver_thread(void *param) {
	MidiDriver_MPU401 *mid = (MidiDriver_MPU401 *)param;
	int cur_time;

	if (!t_first_call)	
	{
		g_thread.active = false;
		g_thread.old_time = g_system->get_msecs();
		g_thread.sleep = 10;
		t_first_call = true;
	}

	cur_time = g_system->get_msecs();
	if (cur_time - g_thread.old_time >= g_thread.sleep)
		g_thread.active = true;

	if (g_thread.active)
	{
		cur_time = g_system->get_msecs();
		while (g_thread.old_time < cur_time) {
			g_thread.old_time += 10;
			// Don't use mid->_se_on_timer()
			// We must come in through IMuseMonitor to protect
			// against conflicts with script access to IMuse.
			if (mid->_timer_proc)
				(*(mid->_timer_proc)) (mid->_timer_param);
		}
		g_thread.active = false;
	}

	return 0;
}