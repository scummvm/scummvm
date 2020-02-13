/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MADS_MENU_VIEWS_H
#define MADS_MENU_VIEWS_H

#include "common/scummsys.h"
#include "mads/dialogs.h"
#include "mads/game.h"
#include "mads/msurface.h"

namespace MADS {

class MADSEngine;

class MenuView: public FullScreenDialog {
protected:
	bool _breakFlag;
	bool _redrawFlag;
	Common::String _filename;

	virtual void doFrame() = 0;

	void display() override;

	/**
	* Event handler
	*/
	bool onEvent(Common::Event &event) override;
public:
	MenuView(MADSEngine *vm);

	~MenuView() override {}

	virtual void show();

	Common::String getResourceName();
};

struct TextLine {
	Common::Point _pos;
	Common::String _line;
	int _textDisplayIndex;
};

/**
 * Scrolling text view
 */
class TextView : public MenuView {
private:
	static char _resourceName[100];

	bool _animating;
	Common::Array<TextLine> _textLines;
	Common::Point _pan;
	int _panSpeed;
	MSurface _spareScreens[4];
	int _scrollCount;
	int _lineY;
	uint32 _scrollTimeout;
	int _panCountdown;
	int _translationX;
	Common::File _script;
	char _currentLine[80];
	MSurface *_spareScreen;
	Font *_font;
private:
	/**
	 * Load the text resource
	 */
	void load();

	/**
	 * Process the lines
	 */
	void processLines();

	/**
	 * Process a command from the script file
	 */
	void processCommand();

	/**
	 * Process text from the script file
	 */
	void processText();

	/**
	 * Get a parameter from line
	 */
	int getParameter(const char **paramP);

	/**
	 * Reset the game palette
	 */
	void resetPalette();
protected:
	void display() override;

	void doFrame() override;

	/**
	 * Called when the script is finished
	 */
	virtual void scriptDone();
public:
	/**
	 * Queue the given text resource for display
	 */
	static void execute(MADSEngine *vm, const Common::String &resName);

	TextView(MADSEngine *vm);

	~TextView() override;
};

enum ResyncMode { NEVER, ALWAYS, BEGINNING };

struct ResourceEntry {
	Common::String _resourceName;
	int _fx;
	bool _soundFlag;
	bool _bgFlag;
	bool _showWhiteBars;

	ResourceEntry() {}
	ResourceEntry(const Common::String &resName, int fx, bool soundFlag,
			bool bgFlag, bool showWhiteBars) {
		_resourceName = resName;
		_fx = fx;
		_soundFlag = soundFlag;
		_bgFlag = bgFlag;
		_showWhiteBars = showWhiteBars;
	}
};

struct ResIndexEntry {
	int _id;
	int _v;
	Common::String _resourceName;

	ResIndexEntry() {}
};

/**
* Animation cutscene view
*/
class AnimationView : public MenuView {
private:
	static char _resourceName[100];

	Common::File _script;
	uint32 _previousUpdate;
	Common::String _currentLine;
	bool _soundDriverLoaded;
	bool _resetPalette;
	ResyncMode _resyncMode;
	int _sfx;
	bool _soundFlag;
	bool _bgLoadFlag;
	bool _showWhiteBars;
	Common::Array<ResourceEntry> _resources;
	Common::Array<ResIndexEntry> _resIndex;
	int _v1;
	int _v2;
	int _resourceIndex;
	SceneInfo *_sceneInfo;
	Animation *_currentAnimation;
	int _manualFrameNumber;
	SpriteAsset *_manualSpriteSet;
	int _manualStartFrame, _manualEndFrame;
	int _manualFrame2;
	int _animFrameNumber;
	bool _nextCyclingActive;
private:
	void checkResource(const Common::String &resourceName);

	int scanResourceIndex(const Common::String &resourceName);

	uint _scrollFrameCtr;
private:
	void load();

	void processLines();

	void processCommand();

	int getParameter();

	void loadNextResource();

	void scroll();
protected:
	void display() override;

	void doFrame() override;

	bool onEvent(Common::Event &event) override;

	virtual void scriptDone();
public:
	/**
	* Queue the given text resource for display
	*/
	static void execute(MADSEngine *vm, const Common::String &resName);

	AnimationView(MADSEngine *vm);

	~AnimationView() override;
};

} // End of namespace MADS

#endif /* MADS_MENU_VIEWS_H */
