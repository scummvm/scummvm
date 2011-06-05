#ifndef TASMRECOVER_DREAMGEN_STUBS_H__
#define TASMRECOVER_DREAMGEN_STUBS_H__

#	include "runtime.h"

namespace dreamgen {

	struct Data : public Segment {
		Data();
	};


	typedef RegisterContext<Data> Context;

	void __dispatch_call(Context &context, unsigned addr);
	void __start(Context &context);

	void randomnumber(Context &context);
	void quickquit(Context &context);
	void quickquit2(Context &context);
	void keyboardread(Context &context);
	void resetkeyboard(Context &context);
	void setkeyboardint(Context &context);
	void readfromfile(Context &context);
	void closefile(Context &context);
	void openforsave(Context &context);
	void openfilenocheck(Context &context);
	void openfile(Context &context);
	void createfile(Context &context);
	void dontloadseg(Context &context);
	void mousecall(Context &context);
	void setmouse(Context &context);
	void gettime(Context &context);
	void allocatemem(Context &context);
	void deallocatemem(Context &context);
	void removeemm(Context &context);
	void setupemm(Context &context);
	void pitinterupt(Context &context);
	void getridofpit(Context &context);
	void setuppit(Context &context);
	void startdmablock(Context &context);
	void dmaend(Context &context);
	void restoreems(Context &context);
	void saveems(Context &context);
	void bothchannels(Context &context);
	void channel1only(Context &context);
	void channel0only(Context &context);
	void out22c(Context &context);
	void soundend(Context &context);
	void interupttest(Context &context);
	void disablesoundint(Context &context);
	void enablesoundint(Context &context);
	void checksoundint(Context &context);
	void setsoundoff(Context &context);
	void soundstartup(Context &context);
	void loadsecondsample(Context &context);
	void loadsample(Context &context);
	void loadspeech(Context &context);
	void scanfornames(Context &context);
	void saveseg(Context &context);
	void loadseg(Context &context);
	void loadposition(Context &context);
	void saveposition(Context &context);
	void error(Context &context);
	void generalerror(Context &context);
	void dosreturn(Context &context);
	void set16colpalette(Context &context);
	void mode640x480(Context &context);
	void showgroup(Context &context);
	void fadedos(Context &context);
	void doshake(Context &context);
	void vsync(Context &context);
	void setmode(Context &context);
	void readoneblock(Context &context);
	void showpcx(Context &context);

}

#endif
