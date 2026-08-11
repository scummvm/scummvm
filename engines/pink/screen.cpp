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

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/paletteman.h"
#include "graphics/fonts/ttf.h"

#include "pink/pink.h"
#include "pink/cel_decoder.h"
#include "pink/screen.h"
#include "pink/objects/actions/action_sound.h"
#include "pink/objects/actions/action_cel.h"
#include "pink/objects/actions/action_text.h"
#include "pink/objects/actors/actor.h"

#include "graphics/macgui/macmenu.h"

namespace Pink {

enum {
	kMenuHighLevel = -1,
	kMenuAbout = 0,
	kMenuGame = 1,
	kMenuBOK = 2,
	kMenuOnline = 3,
	kMenuHelp = 4
};

enum {
	kMenuActionAbout,
	kMenuActionNewGame,
	kMenuActionOpenSavedGame,
	kMenuActionSaveGame,
	kMenuActionSaveGameAs,
	kMenuActionSongs,
	kMenuActionSoundPreferences,
	kMenuActionPause,
	kMenuActionExit
};

/* Currently Unused
static const Graphics::MacMenuData menuSubItems[] = {
	{ kMenuHighLevel, "Game",	0, 0, false },
	{ kMenuHighLevel, "Book of Knowledge",	0, 0, false },
	{ kMenuHighLevel, "Online",	0, 0, false },
	{ kMenuHighLevel, "Help",	0, 0, false },

	{ kMenuGame, "New Game",	kMenuActionNewGame, 'N', false },
	{ kMenuGame, "Open Saved Game...",	kMenuActionOpenSavedGame, 'O', false },
	{ kMenuGame, "Save Game",	kMenuActionSaveGame, 'S', false },
	{ kMenuGame, "Save Game As...",	kMenuActionSaveGameAs, 0, false },
	{ kMenuGame, NULL,			0, 0, false },
	{ kMenuGame, "Songs",	kMenuActionSongs, 0, false },
	{ kMenuGame, NULL,			0, 0, false },
	{ kMenuGame, "Sound Preferences...",	kMenuActionSoundPreferences, 0, false },
	{ kMenuGame, NULL,			0, 0, false },
	{ kMenuGame, "Pause",	kMenuActionPause, 'P', false },
	{ kMenuGame, "Exit",	kMenuActionExit, 'N', false },

	{ 0, NULL,			0, 0, false }
};
*/

static void redrawCallback(void *ref) {
	Screen *dir = (Screen *)ref;

	if (dir->getWndManager().isMenuActive()) {
		dir->addDirtyRect(Common::Rect(0, 0, 640, 480));
		dir->draw(false);
	}
}

Screen::Screen(PinkEngine *vm)
	: _surface(640, 480), _textRendered(false) {
	uint32 wmMode = Graphics::kWMModeNoDesktop | Graphics::kWMModeAutohideMenu
		| Graphics::kWMModalMenuMode | Graphics::kWMModeForceBuiltinFonts
		| Graphics::kWMModeUnicode	 | Graphics::kWMModeWin95;

	if (vm->getLanguage() == Common::HE_ISR) // We do not have Hebrew font in MS fonts :(
		wmMode |= Graphics::kWMModeForceMacFontsInWin95;

	_wm = new Graphics::MacWindowManager(wmMode);

	_wm->setScreen(&_surface);
	_wm->setMenuHotzone(Common::Rect(0, 0, 640, 23));
	_wm->setMenuDelay(250);
	_wm->setEngineRedrawCallback(this, redrawCallback);

	_textFont = nullptr;
	_textFontCleanup = true;
#ifdef USE_FREETYPE2
	if (vm->getLanguage() == Common::HE_ISR) {
		_textFont = _wm->_fontMan->getFont(Graphics::MacFont(Graphics::kMacFontSystem, 12, Graphics::kMacFontRegular));
		_textFontCleanup = false;
	} else {
		_textFont = Graphics::loadTTFFontFromArchive("system.ttf", 16);
	}
#endif

	if (!_textFont) {
		_textFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		warning("Director: falling back to built-in font");

		_textFontCleanup = false;
	}
}

Screen::~Screen() {
	delete _wm;

	if (_textFontCleanup)
		delete _textFont;
}

void Screen::update() {
	if (_wm->isMenuActive()) {
		_wm->draw();
		g_system->updateScreen();
		return;
	}

	for (uint i = 0; i < _sounds.size(); ++i) {
		_sounds[i]->update();
	}

	for (uint i = 0; i < _sprites.size(); ++i) {
		if (_sprites[i]->needsUpdate())
			_sprites[i]->update();
	}

	draw();

	_wm->draw();
}

bool Screen::processEvent(Common::Event &event) {
	return _wm->processEvent(event);
}

void Screen::setPalette(const byte *palette) {
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	_wm->passPalette(palette, 256);
}

void Screen::addTextAction(ActionText *txt) {
	_textActions.push_back(txt);
	_textRendered = false;
}

void Screen::removeTextAction(ActionText *action) {
	for (uint i = 0; i < _textActions.size(); ++i) {
		if (_textActions[i] == action) {
			_textActions.remove_at(i);
			break;
		}
	}
}

void Screen::addTextWindow(Graphics::MacTextWindow *window) {
	_textWindows.push_back(window);
}

void Screen::removeTextWindow(Graphics::MacTextWindow *window) {
	for (uint i = 0; i < _textWindows.size(); i++) {
		if (_textWindows[i] == window) {
			_textWindows.remove_at(i);
			break;
		}
	}
}

void Screen::addSprite(ActionCEL *sprite) {
	_sprites.push_back(sprite);
	int i;
	for (i = _sprites.size() - 1; i > 0 ; --i) {
		if (sprite->getZ() < _sprites[i - 1]->getZ())
			_sprites[i] = _sprites[i - 1];
		else
			break;
	}
	_sprites[i] = sprite;
}

void Screen::removeSprite(ActionCEL *sprite) {
	for (uint i = 0; i < _sprites.size(); ++i) {
		if (sprite == _sprites[i]) {
			_sprites.remove_at(i);
			break;
		}
	}
	_dirtyRects.push_back(sprite->getBounds());
}

void Screen::removeSound(ActionSound *sound) {
	for (uint i = 0; i < _sounds.size(); ++i) {
		if (_sounds[i] == sound)
			_sounds.remove_at(i);
	}
}

void Screen::clear() {
	_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	_sprites.resize(0);
	draw();
}

void Screen::pause(bool pause_) {
	if (!pause_) {
		_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	}
	for (uint i = 0; i < _sprites.size() ; ++i) {
		_sprites[i]->pause(pause_);
	}
}

bool Screen::isMenuActive() {
	return _wm != nullptr && _wm->isMenuActive();
}

void Screen::saveStage() {
	_savedSprites = _sprites;
	clear();
}

void Screen::loadStage() {
	assert(_sprites.empty());
	_dirtyRects.push_back(Common::Rect(0, 0, 640, 480));
	_sprites = _savedSprites;
	_savedSprites.clear();
}

Actor *Screen::getActorByPoint(Common::Point point) {
	for (int i = _sprites.size() - 1; i >= 0; --i) {
		if (_sprites[i]->getActor()->isCursor())
			continue;
		CelDecoder *decoder = _sprites[i]->getDecoder();
		const Graphics::Surface *frame = decoder->getCurrentFrame();
		const Common::Rect &rect = _sprites[i]->getBounds();
		if (rect.contains(point)) {
			byte spritePixel = *(const byte *)frame->getBasePtr(point.x - rect.left, point.y - rect.top);
			if (spritePixel != decoder->getTransparentColourIndex())
				return _sprites[i]->getActor();
		}
	}

	return nullptr;
}

void Screen::draw(bool blit) {
	if (!_dirtyRects.empty() || !_textRendered) {
		mergeDirtyRects();

		for (uint i = 0; i < _dirtyRects.size(); ++i) {
			drawRect(_dirtyRects[i]);
		}

		if (!_textRendered) {
			_textRendered = true;
			for (uint i = 0; i < _textActions.size(); ++i) {
				_textActions[i]->draw(&_surface);
			}
		}

		_dirtyRects.resize(0);

		if (blit)
			_surface.update();
	} else
		g_system->updateScreen();
}

void Screen::mergeDirtyRects() {
	Common::Array<Common::Rect>::iterator rOuter, rInner;
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {
			if ((*rOuter).intersects(*rInner)) {
				// These two rectangles overlap, so merge them
				rOuter->extend(*rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

void Screen::addDirtyRect(const Common::Rect &rect) {
	_dirtyRects.push_back(rect);
}

void Screen::addDirtyRects(ActionCEL *sprite) {
	const Common::Rect spriteRect = sprite->getBounds();
	const Common::List<Common::Rect> *dirtyRects = sprite->getDecoder()->getDirtyRects();
	if (dirtyRects->size() > 100) {
		_dirtyRects.push_back(spriteRect);
	} else {
		for (Common::List<Common::Rect>::const_iterator it = dirtyRects->begin(); it != dirtyRects->end(); ++it) {
			Common::Rect dirtyRect = *it;
			dirtyRect.translate(spriteRect.left, spriteRect.top);
			_dirtyRects.push_back(dirtyRect);
		}
	}
	sprite->getDecoder()->clearDirtyRects();
}

void Screen::drawRect(const Common::Rect &rect) {
	_surface.fillRect(rect, 0);
	for (uint i = 0; i < _sprites.size(); ++i) {
		const Common::Rect &spriteRect = _sprites[i]->getBounds();
		Common::Rect interRect = rect.findIntersectingRect(spriteRect);
		if (interRect.isEmpty())
			continue;

		Common::Rect srcRect(interRect);
		srcRect.translate(-spriteRect.left, -spriteRect.top);
		_surface.transBlitFrom(*_sprites[i]->getDecoder()->getCurrentFrame(), srcRect, interRect, _sprites[i]->getDecoder()->getTransparentColourIndex());
	}

	// check the intersection with action text
	for (uint i = 0; i < _textActions.size(); i++) {
		const Common::Rect &textActionRect = _textActions[i]->getBound();
		Common::Rect interRect = rect.findIntersectingRect(textActionRect);
		if (interRect.isEmpty())
			continue;
		_textActions[i]->draw(&_surface);
	}

	// check the intersection with mactextwindow
	for (uint i = 0; i < _textWindows.size(); i++) {
		const Common::Rect &textWindowRect = _textWindows[i]->getDimensions();
		Common::Rect interRect = rect.findIntersectingRect(textWindowRect);
		if (interRect.isEmpty())
			continue;
		_textWindows[i]->draw(_wm->_screen, true);
	}
}

} // End of namespace Pink
