#ifndef __CONSOLE_WINDOW_H_INCLUDED__
#define __CONSOLE_WINDOW_H_INCLUDED__

#include "Console.h"
#include "Handle.h"

class PopupMenu;
class Archive;
class ConsoleWindow : public ConsoleListener{
public:
    typedef std::vector<std::string> Groups;

	struct Options{
		Options():showLevel_(0){}
		void serialize(Archive& ar);
		Groups hiddenGroups_;
		int showLevel_;
		bool visible_;
	};

    ConsoleWindow();
	~ConsoleWindow();

	static ConsoleWindow& instance(){
		static ConsoleWindow theOnlyOne;
		return theOnlyOne;
	}


    static const char* className() { return "VistaEngineConsole"; }

	bool isVisible() const;
    void show(bool show);
	void setShowLevel(Console::Level level);
	Console::Level showLevel() const;

	const Options& options() const{ return options_; }
	void setOptions(const Options& options, bool changeVisibleState = true) {
		options_ = options;
		refreshEdit();
		if(changeVisibleState)
			show(options_.visible_);
	}

    typedef std::vector<ConsoleWindow*> Instances;
	static Instances& instances(){
		static Instances theOnlyOne;
		return theOnlyOne;
	}
protected:
	void init(Console* console);
	void detach();
	void writeMessage(const Console::Message& msg);
private:
    int  onCreate();
    bool onClose();
    int  onCommand(WPARAM wParam, LPARAM lParam);
    void onDestroy();
    void onPaint(HDC dc);
	void onSize();
	void onRButtonDown(int x, int y, UINT button);
	void onNotify(int id, NMHDR* nmhdr);

    void onFilterGroup(std::string group);
    void onFilterGroupAll();
    
    static LRESULT CALLBACK WindowProc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam);
	void registerWindowClass(HINSTANCE instance);
    bool isMessageVisible(const Console::Message& message) const;
    bool isGroupHidden(const char* groupName) const;
	
	bool allGroupsVisible() const;
	void hideGroup(const char* groupName);
	void unhideGroup(const char* groupName);
	void addGroup(const char* groupName);
	Groups& hiddenGroups() { return options_.hiddenGroups_; }
	const Groups& hiddenGroups() const { return options_.hiddenGroups_; }

    void printMessageToEdit(const Console::Message& msg);
	void syncEditToConsole();
	void refreshEdit();
	void clearMessages();
    
	bool createEdit();
	bool create(HINSTANCE instance);
    void createFonts();
    void freeFonts();

    
	Options options_;
    Groups groups_;
    int numMessagesInEdit_;
	
	Console* console_;
	PtrHandle<PopupMenu> popupMenu_;

    HMODULE richEdModule_;
    HFONT defaultFont_;
    HFONT boldFont_;

    HWND wnd_;
    HWND edit_;
};

#endif
