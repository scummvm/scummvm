/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_log_file.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

appLog::appLog() : time_(0)
{
}

appLog::~appLog()
{
}

appLog& appLog::default_log()
{
	static appLogFile log("qd_engine.log");
	return log;
}

const char* appLog::time_string() const
{
	static XBuffer text(1024,1);
	text.init();

#ifndef _FINAL_VERSION_
	int hrs = time_ / (1000 * 60 * 60);
	if(hrs < 10) text < "0";
	text <= hrs < ":";

	int min = (time_ % (1000 * 60 * 60)) / (1000 * 60);
	if(min < 10) text < "0";
	text <= min < ":";

	int sec = (time_ % (1000 * 60)) / 1000;
	if(sec < 10) text < "0";
	text <= sec < ":";

	int hsec = (time_ % 1000) / 10;
	if(hsec < 10) text < "0";
	text <= hsec < " ";
#endif

	return text.c_str();
}
