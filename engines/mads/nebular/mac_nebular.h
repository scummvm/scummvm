/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADS_NEBULAR_MAC_NEBULAR_H
#define MADS_NEBULAR_MAC_NEBULAR_H

#include "common/rect.h"
#include "common/str.h"
#include "graphics/managed_surface.h"
#include "mads/core/font.h"
#include "mads/core/general.h"

namespace Common {
struct Event;
}

namespace MADS {
namespace RexNebular {

class MacResourceProvider;
class MacNebularMenu;
class RexNebularEngine;

enum MacNebularDisplaySize {
	kMacNebularDisplay100 = 0,
	kMacNebularDisplay150 = 1,
	kMacNebularDisplay200 = 2
};

// Returns true when the Macintosh palette replaced the DOS defaults.
bool setMacintoshMessageColors(int primaryR, int primaryG, int primaryB,
	int secondaryR, int secondaryG, int secondaryB);

class MacNebular {
private:
	RexNebularEngine &_engine;
	MacResourceProvider *_resources = nullptr;
	MacNebularMenu *_menus = nullptr;
	Graphics::ManagedSurface _output;
	Graphics::ManagedSurface _popup;
	Common::Rect _popupRect;
	RGBcolor _palette[256];
	bool _useOriginalMenus;
	int _displaySize;
	bool _hideMenuBar;
	bool _preferencesAtStartup;
	bool _showPreferencesAtStartup;
	bool _startupPreferencesReady = false;
	bool _storyLocked;
	Common::String _storyPassword;
	bool _fullFrameActive = false;
	bool _aboutActive = false;
	bool _gameplayHandoffPending = false;
	bool _gameplayHandoffEffectSeen = false;
	bool _popupActive = false;
	int _popupAskLine = -1;
	int _popupAskX = 0;
	int _popupAskY = 0;
	bool _layoutLogged = false;
	uint32 _lastMacintoshSoundTick = (uint32)-1;
	uint32 _macintoshSoundPausedAt = 0;
	uint32 _macintoshSoundPausedTicks = 0;
	bool _macintoshSoundPaused = false;

	int getSceneWidth() const;
	int getSceneHeight() const;
	int getSceneX() const;
	int getSceneY() const;
	int getInterfaceY() const;
	void showAbout();
public:
	explicit MacNebular(RexNebularEngine &engine);
	~MacNebular();

	void initGraphics();
	bool initResources();
	void applyGameSettings();
	void selectDifficulty();
	int selectResumeSlot();
	bool usesOriginalMenus() const { return _useOriginalMenus; }
	int getDisplaySize() const { return _displaySize; }
	bool getHideMenuBar() const { return _hideMenuBar; }
	bool getPreferencesAtStartup() const { return _preferencesAtStartup; }
	Common::String getApplicationVersion() const;
	bool getStoryLocked() const { return _storyLocked; }
	bool verifyStoryPassword(const Common::String &password) const;
	void setDisplaySize(int displaySize, bool persist);
	void setHideMenuBar(bool hide, bool persist);
	void setPreferencesAtStartup(bool show, bool persist);
	void setStoryLocked(bool locked, const Common::String &password);
	void setFullFrameActive(bool active);
	bool isFullFrameActive() const { return _fullFrameActive; }
	void setOuterMenuActive(bool active);
	void notifyOuterMenuFrameReady();
	Common::Point screenToGame(const Common::Point &point) const;
	Common::Point gameToScreen(const Common::Point &point) const;
	bool handleMacEvent(Common::Event &event);
	void serviceUI();
	void serviceSound();
	void setPalette(const RGBcolor *palette, int firstColor, int numColors);
	void getPalette(RGBcolor *palette, int firstColor, int numColors) const;
	void presentScreen(int shakeOffset);
	void showPopup();
	int editPopup(char *target, int maxLength);
	int runCopyProtectionDialog(const Common::String &title,
		const Common::String &subtitle, const Common::String &prompt,
		char *target, int maxLength);
	void hidePopup();
	int getTextWidth(FontPtr font, const char *text, int spacing) const;
	bool drawText(FontPtr font, Buffer *target, const char *text, int x,
		int y, int color, int spacing) const;
};

} // namespace RexNebular
} // namespace MADS

#endif
