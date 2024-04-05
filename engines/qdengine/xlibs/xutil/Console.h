#ifndef __CONSOLE_H_INCLUDED__
#define __CONSOLE_H_INCLUDED__
#include "Handle.h"

#include <list>
#include <string>

#include "MTSection.h"

// Консоль используется для вывода сообщений.
// Желательно использовать через следующие макросы.
// Пример: kdError("Andrey", "Очень серьезная ошибка..!!!");
#ifndef _FINAL_VERSION_
# define kdCritical(name,message) Console::instance().write(Console::LEVEL_CRITICAL, name, message, Console::Location(__FUNCSIG__, __FILE__, __LINE__))
# define kdError(name,message)	  Console::instance().write(Console::LEVEL_ERROR, name, message, Console::Location(__FUNCSIG__, __FILE__, __LINE__))
# define kdWarning(name,message)  Console::instance().write(Console::LEVEL_WARNING, name, message, Console::Location(__FUNCSIG__, __FILE__, __LINE__))
# define kdMessage(name,message)  Console::instance().write(Console::LEVEL_INFO, name, message, Console::Location(__FUNCSIG__, __FILE__, __LINE__))
#else
# define kdCritical(name,message) {}
# define kdError(name,message) {}
# define kdWarning(name,message) {}
# define kdMessage(name,message) {}
#endif

class ConsoleListener;
class Console{
public:
	enum Level{
		LEVEL_INFO = 0,
		LEVEL_WARNING,
		LEVEL_ERROR,
		LEVEL_CRITICAL,
		NUM_LEVELS
	};

    static const char* levelText(Level level){
        switch(level){
        case LEVEL_INFO:    return "INFO";
        case LEVEL_WARNING: return "WARNING";
        case LEVEL_ERROR:   return "ERROR";
		case LEVEL_CRITICAL:   return "CRITICAL ERROR";
        default:
			xassert(0 && "invalid level value");
			return "";
        };
    }

	struct Location{
		Location(const char* function = 0, const char* file = 0, int line = 0)
		: function_(function)
		, file_(file)
		, line_(line)
		{}
		const char* function_;
		const char* file_;
		int line_;
	};

    enum { MAX_MESSAGES = 10000 };

	class Message{
	public:
		Message(Level level = LEVEL_INFO, const char* author = 0, const char* message = "", Location location = Location())
		: level_(level)
		, author_(author)
		, message_(message)
		, location_(location)
		{
		}
		const char* message() const{ return message_.c_str(); }
		const char* author() const{ return author_.c_str(); }
		Level level() const{ return level_; }
		const Location& location() const{ return location_; }
	protected:
		Level level_;
        Location location_;
		std::string message_;
		std::string author_;
	};
    typedef std::list<Console::Message> Messages;

	Console();
	~Console();
	static Console& instance();

	void quant();
	void graphQuant();

	void write(Level level, const char* author, const char* text, Location location); // Пользуйтесь макросами kd*, товарищи
	
	bool isAvailable() const{
		return inited_ && messages_.messages_.size() < MAX_MESSAGES;
	}

	void setPopupLevel(Level level) { popupLevel_ = level; }
	const Messages& messages() const{ return messages_.messages_; }

	const char* moduleName() const { return moduleName_.c_str(); }
	const char* modulePath() const { return modulePath_.c_str(); }

	// вызывать только из потока, гле были создан listener-ы (в частности для ConsoleWindow)
	void writeToListeners(const Message& message); 
	bool registerListener(ConsoleListener* listener);
	void unregisterListener(ConsoleListener* listener);
protected:
	struct MessageQueue{
		MessageQueue()
		: unflushedCount_(0)
		{}

		Messages messages_;
		int unflushedCount_;
	};

	bool init();
	bool openLogFile();
    void writeToListeners(const Message& message, bool logic);
	void writeToFile(const Message& message, bool flush = false);
	void popupMessage(const Message& message);
	void flushMessage(const Message&);

    typedef std::list<ConsoleListener*> Listeners;
	Listeners listeners_;

	MessageQueue messages_;

	MessageQueue queue_;
	MTSection queueLock_;

	XStream file_;

	int  visible_;
	bool inited_;
	bool logFileOpened_;
	Level popupLevel_;

	std::string moduleName_;
	std::string modulePath_;
};


class ConsoleListener{
public:
    virtual void writeMessage(const Console::Message&) = 0;
	virtual void init(Console* console) = 0;
	virtual void detach() = 0;
};

extern Singleton<Console> console;

#endif
