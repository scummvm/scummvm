#ifndef STARK_UI_MENU_MAIN_MENU_H
#define STARK_UI_MENU_MAIN_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"

namespace Stark {

/**
 * The main menu of the game when it is opened
 */
class MainMenuScreen : public StaticLocationScreen {
public:
	MainMenuScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~MainMenuScreen();

	// StaticLocationScreen API
	void open();

private:
	template<uint N>
	void helpTextHandler(StaticLocationWidget &widget, const Common::Point &mousePos);
};

} // End of namespace Stark

#endif // STARK_UI_MENU_MAIN_MENU_H
