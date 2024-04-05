#include "xglobal.h"
#include <my_stl.h>
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <string>
#include "Xutil.h"
#include "Console.h"

#pragma warning(disable : 4995)


Singleton<Console> console;

Console& Console::instance()
{
	return Singleton<Console>::instance();
}

Console::Console()
: file_(0)
, visible_(false)
, inited_(false)
, popupLevel_(LEVEL_CRITICAL)
{
	init();
}

bool Console::init()
{
#ifdef _FINAL_VERSION_
	return false;
#else
	inited_ = true;
	return true;
#endif
}

Console::~Console()
{
	for(Listeners::iterator it = listeners_.begin(); it != listeners_.end(); ++it)
		(*it)->detach();
	if(file_.isOpen())
		file_.close();
	inited_ = false;
}

void Console::popupMessage(const Message& message)
{
	// MTG();
	XBuffer buf;
	buf < levelText(message.level());
	if(message.author() != 0 && message.author()[0] != '\0')
		buf < " [" < message.author() < "]";
	buf < ":\n\n";
	buf < "   " < message.message();
	if(message.location().file_){
		buf < "\n\n";
		buf < "at: " < message.location().function_ < ", " < message.location().file_ < ": " <= message.location().line_ < " line";
	}

	UINT icon = MB_ICONERROR;
	switch(message.level()){
	case LEVEL_INFO:
		icon = MB_ICONINFORMATION;
		break;
	case LEVEL_WARNING:
		icon = MB_ICONEXCLAMATION;
		break;
	case LEVEL_ERROR:
		icon = MB_ICONERROR;
		break;
	case LEVEL_CRITICAL:
		icon = MB_ICONERROR;
		break;
	}
	MessageBox(0, static_cast<const char*>(buf), levelText(message.level()), MB_OK | icon);
}

void Console::writeToListeners(const Message& message)
{
    for(Listeners::iterator it = listeners_.begin(); it != listeners_.end(); ++it){
        ConsoleListener* listener = *it;
        xassert(listener);
        listener->writeMessage(message);
    }
}

void Console::quant()
{
	// MTL();
}

void Console::graphQuant()
{
	// MTG();

	if(!queue_.messages_.empty()){
		int count = 0;
		{
			MTAuto lock(queueLock_);
			if(count = queue_.messages_.size()){
				std::copy(queue_.messages_.begin(), queue_.messages_.end(),
						  back_inserter(messages_.messages_));
				queue_.messages_.clear();
			}
		}
		if(count){
			Messages::iterator it = messages_.messages_.begin();
			std::advance(it, messages_.messages_.size() - count);

			for(; it != messages_.messages_.end(); ++it){
				flushMessage(*it);
			}
		}
	}
}

void Console::flushMessage(const Message& message)
{
	if(message.level() >= popupLevel_)
		popupMessage(message);
	writeToListeners(message);
	if(inited_)
		writeToFile(message, false);
}

void Console::writeToFile(const Message& message, bool flush)
{
	if(openLogFile()){
		file_ < levelText(message.level());
		file_ < " [" < message.author();
		if(message.location().file_)
			file_ < ", " < message.location().function_ < ", " < message.location().file_ < ": " <= message.location().line_ < " line";
		file_ < "]:\n    " < message.message() < "\n";
		if(flush)
			file_.flush();
	}
}

void Console::write(Level level, const char* author, const char* text, Location location)
{
	if(isAvailable()){
		Message message(level, author ? author : "", text, location);
		MTAuto lock(queueLock_);
		queue_.messages_.push_back(message);
	}
}

void Console::unregisterListener(ConsoleListener* listener)
{
	Listeners::iterator it = std::find(listeners_.begin(), listeners_.end(), listener);
	if(it != listeners_.end())
		listeners_.erase(it);
	else
		xassert(0 && "Unable to unregister console listener!");
}

bool Console::registerListener(ConsoleListener* listener)
{
	Listeners::iterator it = std::find(listeners_.begin(), listeners_.end(), listener);
	if(it == listeners_.end()){
		listeners_.push_back(listener);
		listener->init(this);
		return true;
	}
	else
		return false;
}

bool Console::openLogFile()
{
	if(logFileOpened_)
		return true;

	modulePath_ = __argv[0];
	std::string::size_type begin = modulePath_.rfind("\\");
	std::string::size_type end = modulePath_.rfind(".");
	if(begin == std::string::npos)
		begin = 0;
	if(end == std::string::npos)
		end = modulePath_.end() - modulePath_.begin();
	moduleName_ = std::string(modulePath_.begin() + begin + 1,  modulePath_.begin() + end);

	for(int i = 0; i < 10; i++){
		XBuffer buf;
		buf < moduleName() < "_console";
		if(i)
			buf <= i;
		buf < ".log";
		if(file_.open(buf, XS_OUT)){
			Console::Message message(LEVEL_INFO, "Console", XBuffer() < "Module Name: " < modulePath(), Location());
			MTAuto lock(queueLock_);
			messages_.messages_.insert(messages_.messages_.begin(), message);
			++messages_.unflushedCount_;
			logFileOpened_ = true;
			break; 
		}
	}

	return logFileOpened_;
}
