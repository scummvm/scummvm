/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "app_log_file.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

appLogFile::appLogFile(const char* fname) : file_(NULL)
{
	if(fname)
		file_name_ = fname;
}

appLogFile::~appLogFile()
{
	close();
}

bool appLogFile::open(const char* fname)
{
	close();

	if(fname)
		file_name_ = fname;

	file_ = new XStream(file_name_.c_str(),XS_OUT);
	return true;
}

bool appLogFile::close()
{
	if(file_){
		file_ -> close();
		delete file_;
		file_ = NULL;
	}
	return true;
}

appLog& appLogFile::operator << (const char* str)
{
#ifdef __APP_LOG_ENABLE__
	if(is_enabled() && str) *file() < str;
#endif
	return *this;
}

appLog& appLogFile::operator << (int data)
{
#ifdef __APP_LOG_ENABLE__
	if(is_enabled()) *file() <= data;
#endif
	return *this;
}

appLog& appLogFile::operator << (unsigned int data)
{
#ifdef __APP_LOG_ENABLE__
	if(is_enabled()) *file() <= data;
#endif
	return *this;
}

appLog& appLogFile::operator << (float data)
{
#ifdef __APP_LOG_ENABLE__
	if(is_enabled()) *file() <= data;
#endif
	return *this;
}

