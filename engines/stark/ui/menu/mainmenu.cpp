#include "engines/stark/ui/menu/mainmenu.h"

namespace Stark {

MainMenuScreen::MainMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
        StaticLocationScreen(gfx, cursor, "MainMenuLocation", Screen::kScreenMainMenu) {
}

MainMenuScreen::~MainMenuScreen() {
}

void MainMenuScreen::open() {
    StaticLocationScreen::open();

    //TODO: Implement each handler
    _widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));
    
    _widgets.push_back(new StaticLocationWidget(
			"NewGame",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
    
    _widgets.push_back(new StaticLocationWidget(
			"Continue",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
    
    _widgets.push_back(new StaticLocationWidget(
			"Options",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
    
    _widgets.push_back(new StaticLocationWidget(
			"Box",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
    
    _widgets.push_back(new StaticLocationWidget(
			"Quit",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
    
    _widgets.push_back(new StaticLocationWidget(
			"OptionHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
    
    _widgets.push_back(new StaticLocationWidget(
			"BeginHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
    
    _widgets.push_back(new StaticLocationWidget(
			"ContinueHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
    
    _widgets.push_back(new StaticLocationWidget(
			"BoxHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
    
    _widgets.push_back(new StaticLocationWidget(
			"QuitHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
    
    _widgets.push_back(new StaticLocationWidget(
			"Credits",
			nullptr,
			nullptr));
	_widgets.back()->setupSounds(0, 1);
    
    _widgets.push_back(new StaticLocationWidget(
			"CreditHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
    
    _widgets.push_back(new StaticLocationWidget(
			"VERSION INFO",
			nullptr,
			nullptr));
    
    _widgets.push_back(new StaticLocationWidget(
			"VERSION INFO REALLY",
			nullptr,
			nullptr));
}

}