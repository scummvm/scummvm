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
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/managed_surface.h"
#include "image/bmp.h"
#include "image/image_decoder.h"

#include "cryomni3d/mouse_boxes.h"
#include "cryomni3d/font_manager.h"

#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

bool CryOmni3DEngine_Versailles::showSubtitles() const {
	return ConfMan.getBool("subtitles");
}

void CryOmni3DEngine_Versailles::drawMenuTitle(Graphics::ManagedSurface *surface, byte color) {
	int offY;

	int oldFont = _fontManager.getCurrentFont();

	int titleX, titleY, subtitleX, subtitleY;
	if (getLanguage() == Common::FR_FRA ||
	        getLanguage() == Common::ES_ESP ||
	        getLanguage() == Common::KO_KOR ||
	        getLanguage() == Common::PT_BRA) {
		titleX = 144;
		titleY = 160;
		subtitleX = 305;
		subtitleY = 160;
	} else if (getLanguage() == Common::DE_DEU) {
		titleX = 122;
		titleY = 80;
		subtitleX = 283;
		subtitleY = 80;
	} else if (getLanguage() == Common::JA_JPN) {
		titleX = 144;
		titleY = 125;
		subtitleX = 144;
		subtitleY = 145;
	} else if (getLanguage() == Common::ZH_TWN) {
		titleX = 130;
		titleY = 160;
		subtitleX = 340;
		subtitleY = 160;
	} else {
		titleX = 100;
		titleY = 80;
		subtitleX = 261;
		subtitleY = 80;
	}

	_fontManager.setSurface(surface);
	_fontManager.setForeColor(color);
	_fontManager.setCurrentFont(1);
	offY = _fontManager.getFontMaxHeight();
	_fontManager.displayStr(titleX, titleY - offY, _messages[23]);
	_fontManager.setCurrentFont(3);
	offY = _fontManager.getFontMaxHeight();
	_fontManager.displayStr(subtitleX, subtitleY - offY, _messages[24]);

	if (getLanguage() == Common::FR_FRA ||
	        getLanguage() == Common::ES_ESP ||
	        getLanguage() == Common::KO_KOR ||
	        getLanguage() == Common::PT_BRA ||
	        getLanguage() == Common::ZH_TWN) {
		surface->vLine(100, 146, 172, color);
		surface->hLine(100, 172, 168, color); // minus 1 because hLine draws inclusive
	}

	_fontManager.setCurrentFont(oldFont);
}

uint CryOmni3DEngine_Versailles::displayOptions() {
	Common::Array<int> menuEntries;
	menuEntries.push_back(26);
	menuEntries.push_back(27);
	menuEntries.push_back(28);
	menuEntries.push_back(29);
	menuEntries.push_back(48);
	menuEntries.push_back(30);
	menuEntries.push_back(32);
#if 0
	// Music on HDD setting
	menuEntries.push_back(34);
#endif
	menuEntries.push_back(25);
	menuEntries.push_back(-42);
	menuEntries.push_back(43);
	menuEntries.push_back(40);
	// 1 is for volume box
	MouseBoxes boxes(menuEntries.size() + 1);

	bool end = false;

	int drawState = 1;

	uint volumeCursorMiddleY = _sprites.getCursor(102).getHeight() / 2;
	uint volume = CLIP(ConfMan.getInt("music_volume"), 0, 256);
	uint soundVolumeY = ((283 * (256 - volume)) >> 8) + 101;
	byte volumeForeColor = 243;

	Graphics::ManagedSurface optionsSurface;
	Image::ImageDecoder *imageDecoder = loadHLZ(getFilePath(kFileTypeMenu, "option.hlz"));
	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	optionsSurface.create(bgFrame->w, bgFrame->h, bgFrame->format);

	setCursor(181);
	showMouse(true);

	uint hoveredBox = uint(-1);
	uint selectedBox = uint(-1);
	int selectedMsg = 0;
	uint volumeBox = uint(-1);
	bool resetScreen = true;
	bool forceEvents = true;

	while (!shouldAbort() && !end) {
		if (resetScreen) {
			setPalette(imageDecoder->getPalette(), imageDecoder->getPaletteStartIndex(),
			           imageDecoder->getPaletteColorCount());
			// _cursorPalette has only 248 colors as 8 last colors are for translucency
			setPalette(_cursorPalette + 240 * 3, 240, 8);

			_fontManager.setCurrentFont(3);
			_fontManager.setTransparentBackground(true);
			_fontManager.setForeColor(243);
			_fontManager.setLineHeight(14);
			_fontManager.setSpaceWidth(0);
			_fontManager.setCharSpacing(1);
			_fontManager.setSurface(&optionsSurface);
			resetScreen = false;
		}
		if (drawState > 0) {
			if (drawState == 1) {
				optionsSurface.blitFrom(*bgFrame);
			}
			drawMenuTitle(&optionsSurface, 243);
			_fontManager.setForeColor(volumeForeColor);
			_fontManager.displayStr(550, 407, _messages[39]);
			optionsSurface.vLine(544, 402, 429, volumeForeColor);
			optionsSurface.hLine(544, 429, 613, volumeForeColor); // minus 1 because hLine draws inclusive

			boxes.reset();
			uint boxId = 0;
			uint top = 195;
			uint bottom;
			uint width;

			for (Common::Array<int>::iterator it = menuEntries.begin(); it != menuEntries.end(); it++) {
				if (*it == 30 && !ConfMan.getBool("subtitles")) {
					*it = 31;
				} else if (*it == 32 && (ConfMan.getBool("mute") ||
				                         ConfMan.getBool("music_mute"))) {
					*it = 33;
				}
#if 0
				else if (*it == 34) {
					// What to do with music on HDD setting?
				}
#endif
				else if (*it == 26 && !_isPlaying) {
					*it = -26;
				} else if (*it == 29 && !_isPlaying) {
					*it = -29;
				} else if (*it == -42 && canVisit()) {
					*it = 42;
				} else if (*it == 48) {
					uint omni3D_speed = ConfMan.getInt("omni3d_speed");
					switch (omni3D_speed) {
					case 1:
						*it = 51;
						break;
					case 2:
						*it = 52;
						break;
					case 3:
						*it = 49;
						break;
					case 4:
						*it = 50;
						break;
					default:
						// If it is another value, menu text is 48 (normal speed) and there is nothing to do
						break;
					}
				}

				if (*it > 0) {
					int msgId = *it;
					bottom = top;
					top += 24;

					// Patch on the fly the text displayed
					if (_isVisiting) {
						if (msgId == 26) {
							msgId = 44;
						} else if (msgId == 29) {
							msgId = 45;
						}
					}

					width = _fontManager.getStrWidth(_messages[msgId]);
					//Common::Rect rct(144, top - 39, width + 144, bottom);
					//optionsSurface.frameRect(rct, 0);
					boxes.setupBox(boxId, 144, top - 39, width + 144, bottom);
					if (boxId == hoveredBox) {
						_fontManager.setForeColor(240);
					} else {
						_fontManager.setForeColor(243);
					}
					_fontManager.displayStr(144, top - 39, _messages[msgId]);
				}
				boxId++;
			}

			volumeBox = boxId;
			boxes.setupBox(boxId, 525, 101, 570, 401);
			optionsSurface.transBlitFrom(_sprites.getSurface(102), Common::Point(553, soundVolumeY),
			                             _sprites.getKeyColor(102));

			g_system->copyRectToScreen(optionsSurface.getPixels(), optionsSurface.pitch, 0, 0, optionsSurface.w,
			                           optionsSurface.h);
			drawState = 0;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (pollEvents() || forceEvents) { // always call pollEvents
			forceEvents = false;
			Common::Point mouse = getMousePos();
			uint boxId = 0;
			Common::Array<int>::iterator it;
			for (it = menuEntries.begin(); it != menuEntries.end(); it++) {
				if (boxes.hitTest(boxId, mouse)) {
					if (hoveredBox != boxId) {
						hoveredBox = boxId;
						drawState = 2;
					}
					// We met a hit, no need to look further
					break;
				}
				boxId++;
			}
			if (it != menuEntries.end()) {
				if (getDragStatus() == 2) {
					selectedMsg = *it;
					selectedBox = hoveredBox;
				}
			} else {
				// no menu selected, check volume
				if (boxes.hitTest(volumeBox, mouse)) {
					if (volumeForeColor != 240) {
						volumeForeColor = 240;
						drawState = 1;
					}
					if (getCurrentMouseButton() == 1) {
						if (soundVolumeY != getMousePos().y - volumeCursorMiddleY) {
							soundVolumeY = CLIP(getMousePos().y - volumeCursorMiddleY, 101u, 384u);
							drawState = 1;
							volume = CLIP(((384 - soundVolumeY) << 8) / 283, 0u, 256u);
							// Global setting
							ConfMan.setInt("music_volume", volume);
							// As we modify speech volume, let's unmute it
							// Using in-game settings resets all scummvm specific settings
							ConfMan.setBool("speech_mute", false);
							ConfMan.setInt("speech_volume", volume);
							syncSoundSettings();
						}
					} else if (getDragStatus() == 2 &&
					           !_mixer->hasActiveChannelOfType(Audio::Mixer::kMusicSoundType) &&
					           _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) > 0) {
						// Finished dragging
						_mixer->stopID(SoundIds::kOrgue);
						do {
							Common::Path orguePath(getFilePath(kFileTypeSound, "ORGUE.WAV"));
							Common::File *audioFile = new Common::File();
							if (!audioFile->open(orguePath)) {
								warning("Failed to open sound file %s", orguePath.toString(Common::Path::kNativeSeparator).c_str());
								delete audioFile;
								break;
							}

							Audio::SeekableAudioStream *audioDecoder = Audio::makeWAVStream(audioFile, DisposeAfterUse::YES);
							// We lost ownership of the audioFile just set it to nullptr and don't use it
							audioFile = nullptr;
							if (!audioDecoder) {
								break;
							}

							_mixer->playStream(Audio::Mixer::kMusicSoundType, nullptr, audioDecoder, SoundIds::kOrgue);
							// We lost ownership of the audioDecoder just set it to nullptr and don't use it
							audioDecoder = nullptr;
						} while (false);
					}
				} else {
					if (hoveredBox != uint(-1)) {
						hoveredBox = uint(-1);
						drawState = 2;
					}
					if (volumeForeColor != 243) {
						volumeForeColor = 243;
						drawState = 1;
					}
				}
			}
			if (getNextKey().keycode == Common::KEYCODE_ESCAPE && _isPlaying) {
				selectedMsg = 26;
			}
			if (selectedMsg == 27 || selectedMsg == 28 || selectedMsg == 40 || selectedMsg == 42) {
				// New game, Load game, Quit, Visit
				if (!_isPlaying || _isVisiting) {
					end = true;
				} else {
					Common::Rect rct;
					if (getLanguage() == Common::DE_DEU) {
						rct = Common::Rect(286, 433, 555, 475);
					} else if (getLanguage() == Common::ES_ESP ||
					           getLanguage() == Common::IT_ITA) {
						rct = Common::Rect(250, 420, 530, 465);
					} else if (getLanguage() == Common::JA_JPN) {
						rct = Common::Rect(245, 420, 505, 465);
					} else {
						rct = Common::Rect(235, 420, 505, 465);
					}
					end = displayYesNoBox(optionsSurface, rct, 57);
				}
				drawState = 1;
				if (!end) {
					selectedMsg = 0;
				}
			}
			if (selectedMsg == 25) {
				// Documentation area
				_docManager.handleDocArea();
				drawState = 1;
				resetScreen = true;
				forceEvents = true;
				waitMouseRelease();
				selectedMsg = 0;
			} else if (selectedMsg == 26) {
				// Continue game
				end = true;
			} else if (selectedMsg == 28) {
				Common::String saveName;
				bool wasVisiting = _isVisiting;
				_isVisiting = false;
				uint saveNumber = displayFilePicker(bgFrame, false, saveName);
				if (saveNumber == uint(-1)) {
					_isVisiting = wasVisiting;
					drawState = 1;
					selectedMsg = 0;
				} else {
					_loadedSave = saveNumber;
					end = true;
				}
				waitMouseRelease();
			} else if (selectedMsg == 42) {
				Common::String saveName;
				bool wasVisiting = _isVisiting;
				_isVisiting = true;
				uint saveNumber = displayFilePicker(bgFrame, false, saveName);
				if (saveNumber == uint(-1)) {
					_isVisiting = wasVisiting;
					drawState = 1;
					selectedMsg = 0;
				} else {
					_loadedSave = saveNumber;
					end = true;
				}
				waitMouseRelease();
			} else if (selectedMsg == 29) {
				Common::String saveName;
				uint saveNumber = displayFilePicker(bgFrame, true, saveName);
				if (saveNumber != uint(-1)) {
					saveGame(_isVisiting, saveNumber, saveName);
				}
				drawState = 1;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 30) {
				ConfMan.setBool("subtitles", false);
				drawState = 1;
				menuEntries[selectedBox] = 31;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 31) {
				ConfMan.setBool("subtitles", true);
				drawState = 1;
				menuEntries[selectedBox] = 30;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 32) {
				ConfMan.setBool("music_mute", true);
				syncSoundSettings();
				drawState = 1;
				menuEntries[selectedBox] = 33;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 33) {
				ConfMan.setBool("mute", false);
				ConfMan.setBool("music_mute", false);
				syncSoundSettings();
				drawState = 1;
				menuEntries[selectedBox] = 32;
				selectedMsg = 0;
				waitMouseRelease();
			}
#if 0
			// Music on disk settings
			else if (selectedMsg == 35) {
				drawState = 1;
				menuEntries[selectedBox] = 34;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 34) {
				drawState = 1;
				menuEntries[selectedBox] = 36;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 36) {
				drawState = 1;
				menuEntries[selectedBox] = 35;
				selectedMsg = 0;
				waitMouseRelease();
			}
#endif
			else if (selectedMsg == 39) {
				// Volume
				selectedMsg = 0;
			} else if (selectedMsg == 47) {
				// Unknown
				selectedMsg = 0;
			} else if (selectedMsg == 48) {
				ConfMan.setInt("omni3d_speed", 1);
				drawState = 1;
				menuEntries[selectedBox] = 51;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 51) {
				ConfMan.setInt("omni3d_speed", 2);
				drawState = 1;
				menuEntries[selectedBox] = 52;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 52) {
				ConfMan.setInt("omni3d_speed", 3);
				drawState = 1;
				menuEntries[selectedBox] = 49;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 49) {
				ConfMan.setInt("omni3d_speed", 4);
				drawState = 1;
				menuEntries[selectedBox] = 50;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 50) {
				ConfMan.setInt("omni3d_speed", 0);
				drawState = 1;
				menuEntries[selectedBox] = 48;
				selectedMsg = 0;
				waitMouseRelease();
			} else if (selectedMsg == 43) {
				displayCredits();
				drawState = 1;
				resetScreen = true;
				forceEvents = true;
				selectedMsg = 0;
				waitMouseRelease();
			}
		}
	}

	showMouse(false);

	if (selectedMsg == 42) {
		_abortCommand = kAbortLoadGame;
		// For return value
		selectedMsg = 28;
	} else if (selectedMsg == 28) {
		_abortCommand = kAbortLoadGame;
	} else if (selectedMsg == 40) {
		_abortCommand = kAbortQuit;
	} else if (selectedMsg == 27) {
		_abortCommand = kAbortNewGame;
		_isVisiting = false;
	} else if (shouldAbort()) {
		// Fake a quit
		selectedMsg = 40;
		// shouldAbort called earlier has already set _abortCommand
		// If GMM is called on main menu in game, return value is ignored so quit isn't important
		// If GMM is called on main menu out of game, GMM can only quit game and don't load
	}

	ConfMan.flushToDisk();
	syncOmni3DSettings();
	musicUpdate();

	delete imageDecoder;
	return selectedMsg;
}

uint CryOmni3DEngine_Versailles::displayYesNoBox(Graphics::ManagedSurface &surface,
		const Common::Rect &position, uint msg_id) {
	uint confirmWidth = _fontManager.getStrWidth(_messages[53]);
	uint cancelWidth = _fontManager.getStrWidth(_messages[54]);
	uint oldFont = _fontManager.getCurrentFont();

	_fontManager.setSurface(&surface);
	_fontManager.setForeColor(240);
	_fontManager.setLineHeight(20);
	surface.frameRect(position, 243);

	_fontManager.setupBlock(Common::Rect(position.left + 5, position.top + 5, position.right - 5,
	                                     position.bottom - 5));
	_fontManager.setCurrentFont(5);
	_fontManager.displayBlockText(_messages[msg_id]);
	_fontManager.setCurrentFont(3);

	MouseBoxes boxes(2);
	boxes.setupBox(1, position.left + 5, position.bottom - 15, position.left + confirmWidth,
	               position.bottom, &_messages[53]);
	boxes.setupBox(0, position.right - cancelWidth - 5, position.bottom - 15, position.right,
	               position.bottom, &_messages[54]);

	bool end = false;
	bool redraw = true;
	uint result = uint(-1);

	while (!shouldAbort() && (!end || redraw)) {
		if (redraw) {
			for (uint boxId = 0; boxId < 2; boxId++) {
				if (boxId == result) {
					_fontManager.setForeColor(240);
				} else {
					_fontManager.setForeColor(243);
				}
				boxes.display(boxId, _fontManager);
			}
			redraw = false;

			g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, surface.w, surface.h);
		}
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (pollEvents()) {
			Common::Point mouse = getMousePos();
			uint hit_result = uint(-1);
			if (boxes.hitTest(1, mouse)) {
				hit_result = 1;
			} else if (boxes.hitTest(0, mouse)) {
				hit_result = 0;
			}
			if (!end && hit_result != result) {
				result = hit_result;
				redraw = true;
			}
			if ((getCurrentMouseButton() == 1) && (result != uint(-1))) {
				end = true;
			}
			Common::KeyCode keyPressed = getNextKey().keycode;
			if (keyPressed == Common::KEYCODE_ESCAPE) {
				result = 0;
				redraw = true;
				end = true;
			} else if (keyPressed == Common::KEYCODE_RETURN) {
				result = 1;
				redraw = true;
				end = true;
			}
		}
	}
	_fontManager.setCurrentFont(oldFont);
	return result;
}

uint CryOmni3DEngine_Versailles::displayFilePicker(const Graphics::Surface *bgFrame,
		bool saveMode, Common::String &saveName) {
	bool autoName = (_messages.size() >= 148);

	Graphics::ManagedSurface surface(bgFrame->w, bgFrame->h, bgFrame->format);
	surface.blitFrom(*bgFrame);

	drawMenuTitle(&surface, 243);

	int subtitleId;
	if (_isVisiting) {
		subtitleId = saveMode ? 45 : 46;
	} else {
		subtitleId = saveMode ? 29 : 28;
	}
	_fontManager.displayStr(164, 214, _messages[subtitleId]);

	// Draw an empty screen before we list saves
	showMouse(false);
	g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, surface.w, surface.h);
	g_system->updateScreen();

	Common::Array<Common::String> savesList;
	int nextSaveNum;
	getSavesList(_isVisiting, savesList, nextSaveNum);
	Common::String saveNameBackup;

	showMouse(true);

	MouseBoxes boxes(10); // 6 files + Yes/No/Up/Down buttons

	// Yes/No buttons
	const Common::String &okMsg = _messages[53];
	uint okWidth = _fontManager.getStrWidth(okMsg);
	boxes.setupBox(6, 246, 430, 246 + okWidth, 450, &okMsg);
	const Common::String &cancelMsg = _messages[54];
	uint cancelWidth = _fontManager.getStrWidth(cancelMsg);
	boxes.setupBox(7, 146, 430, 146 + cancelWidth, 450, &cancelMsg);

	// Up/Down buttons
	boxes.setupBox(8, 428, 320, 448, 340);
	boxes.setupBox(9, 428, 360, 448, 380);
	surface.transBlitFrom(_sprites.getSurface(162), Common::Point(428, 320), _sprites.getKeyColor(162));
	surface.transBlitFrom(_sprites.getSurface(185), Common::Point(428, 360), _sprites.getKeyColor(185));

	setCursor(181);

	uint fileListOffset = CLIP(ConfMan.getInt(_isVisiting ? "visits_list_off" :
	                           "saves_list_off"), 0, 100 - 6);

	uint boxHovered = uint(-1);
	uint boxSelected = uint(-1);

	bool textCursorState = false;
	uint textCursorNextState = 0;
	uint textCursorPos = uint(-1);

	bool autoRepeatInhibit = false;
	uint autoRepeatDelay = 250;
	uint autoRepeatEndInhibit = 0;

	bool finished = false;
	bool filesListChanged = true;
	bool redraw = false;
	while (!finished) {
		if (filesListChanged || redraw) {
			if (filesListChanged) {
				for (uint file = 0, fileY = 280; file < 6; file++, fileY += 20) {
					boxes.setupBox(file, 146, fileY, 408, fileY + 14, &savesList[file + fileListOffset]);
				}
				// Redraw background as file list changed
				surface.blitFrom(*bgFrame, Common::Rect(116, 280, 408, 400), Common::Point(116, 280));
				filesListChanged = false;
			}
			// Don't redraw the scroll buttons
			for (uint box = 0; box < 8; box++) {
				if (box == boxSelected) {
					// Selected
					_fontManager.setForeColor(240);
				} else if (box == 6 && boxSelected == uint(-1)) {
					// Ok and no file selected
					_fontManager.setForeColor(245);
				} else if (box == boxHovered) {
					// Hovered
					_fontManager.setForeColor(241);
				} else {
					// Other cases
					_fontManager.setForeColor(243);
				}

				if (box == boxSelected && saveMode) {
					Common::Rect boxRct = boxes.getBoxRect(box);
					boxRct.top -= 2;
					surface.blitFrom(*bgFrame, boxRct, Common::Point(boxRct.left, boxRct.top));
					boxRct.top += 2;
					if (textCursorState) {
						surface.vLine(textCursorPos, boxRct.top, boxRct.top + 11, 240);
					}
				}
				boxes.display(box, _fontManager);
				if (box < 6) {
					// Draw line below
					surface.hLine(116, 280 + box * 20 + 15, 407, 243); // minus 1 because hLine draws inclusive

					// Display file number
					_fontManager.displayInt(126, 280 + box * 20, fileListOffset + box + 1);
				}
			}
			redraw = false;
			g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, surface.w, surface.h);
		}

		g_system->updateScreen();
		g_system->delayMillis(10);

		pollEvents();
		Common::KeyState key = getNextKey();
		uint mousePressed = getCurrentMouseButton();

		if (!mousePressed) {
			bool boxFound = false;
			// Don't handle scroll arrows hovering
			for (uint box = 0; box < 8; box++) {
				if (boxes.hitTest(box, getMousePos())) {
					boxFound = true;
					if (boxHovered != box) {
						boxHovered = box;
						redraw = true;
					}
				}
			}
			if (!boxFound && boxHovered != uint(-1)) {
				boxHovered = uint(-1);
				redraw = true;
			}
		}
		if (key == Common::KEYCODE_RETURN || (mousePressed == 1 && boxHovered == 6)) {
			// OK
			if (boxSelected != uint(-1)) {
				Common::String &selectedSaveName = savesList[boxSelected + fileListOffset];
				if (!selectedSaveName.size()) {
					selectedSaveName = _messages[56]; // No name
				}
				redraw = true;
				finished = true;
			}
		} else if (mousePressed == 1) {
			if (boxHovered == 7) {
				// Cancel
				boxSelected = uint(-1);
				finished = true;
			} else if (boxHovered != uint(-1) && boxHovered != boxSelected) {
				// This can only be a file
				bool existingSave = (savesList[boxHovered + fileListOffset] != _messages[55]);
				// Don't allow to save on slot 0 when visiting to avoid problems with original visit save
				bool validSave = !(_isVisiting && saveMode && boxSelected == 0);
				if ((saveMode || existingSave) && validSave) {
					// Restore old name
					if (saveMode && boxSelected != uint(-1)) {
						savesList[boxSelected + fileListOffset] = saveNameBackup;
						filesListChanged = true;
					}
					boxSelected = boxHovered;
					// Backup new one
					saveNameBackup = savesList[boxSelected + fileListOffset];
					if (saveMode) {
						if (!existingSave) {
							// Not an existing save clear free name
							savesList[boxSelected + fileListOffset] = "";
						}
						if (autoName) {
							// Apply autoname to text
							if (_currentLevel < 8) {
								savesList[boxSelected + fileListOffset] = Common::String::format(_messages[146].c_str(),
								        _currentLevel);
							} else {
								savesList[boxSelected + fileListOffset] = _messages[147];
							}
							savesList[boxSelected + fileListOffset] += Common::String::format(" - %d", nextSaveNum);
						}
					}
					redraw = true;
				}
			}
		}
		if (boxSelected != uint(-1) && saveMode && !autoName) {
			if (key.keycode != Common::KEYCODE_INVALID) {
				// Reference means we edit in place
				Common::String &selectedSaveName = savesList[boxSelected + fileListOffset];
				if (key == Common::KEYCODE_BACKSPACE && selectedSaveName.size() > 0) {
					selectedSaveName.deleteLastChar();
					textCursorNextState = 0;
					redraw = true;
				} else if (key.ascii >= 32 && key.ascii < 256 && selectedSaveName.size() < 20) {
					selectedSaveName += key.ascii;
					textCursorNextState = 0;
					redraw = true;
				}
			}
			if (g_system->getMillis() > textCursorNextState) {
				textCursorNextState = g_system->getMillis() + 200; // Blink at 200ms period
				uint width = _fontManager.getStrWidth(savesList[boxSelected + fileListOffset]);
				Common::Rect boxRct = boxes.getBoxRect(boxSelected);
				textCursorPos = boxRct.left + width;
				textCursorState = !textCursorState;
				redraw = true;
			}
		}
		if (!autoRepeatInhibit) {
			bool autoRepeatTrigger = false;
			uint oldFileListOffset = fileListOffset;
			if (mousePressed) {
				if (boxes.hitTest(8, getMousePos()) && fileListOffset > 0) {
					fileListOffset--;
					autoRepeatTrigger = true;
				} else if (boxes.hitTest(9, getMousePos()) && fileListOffset < 99 - 6) {
					fileListOffset++;
					autoRepeatTrigger = true;
				}
			} else if (key == Common::KEYCODE_UP) {
				if (fileListOffset > 0) {
					fileListOffset--;
					autoRepeatTrigger = true;
				}
			} else if (key == Common::KEYCODE_DOWN) {
				if (fileListOffset < 99 - 6) {
					fileListOffset++;
					autoRepeatTrigger = true;
				}
			} else if (key == Common::KEYCODE_PAGEUP) {
				if (fileListOffset > 6) {
					fileListOffset -= 6;
				} else {
					fileListOffset = 0;
				}
			} else if (key == Common::KEYCODE_PAGEDOWN) {
				if (fileListOffset < 99 - 6 - 6) {
					fileListOffset += 6;
				} else {
					fileListOffset = 99 - 6;
				}
			}
			if (autoRepeatTrigger) {
				// Restore old name
				if (saveMode && boxSelected != uint(-1)) {
					savesList[boxSelected + oldFileListOffset] = saveNameBackup;
				}
				boxHovered = uint(-1);
				boxSelected = uint(-1);
				autoRepeatInhibit = true;
				autoRepeatEndInhibit = g_system->getMillis() + autoRepeatDelay;
				filesListChanged = true;
			}
			if (shouldAbort()) {
				return uint(-1);
			}
		}
		if (autoRepeatInhibit && g_system->getMillis() > autoRepeatEndInhibit) {
			autoRepeatInhibit = false;
			autoRepeatDelay = 60; // Next rounds will wait 60ms after first one
		}
		if (!mousePressed && key == Common::KEYCODE_INVALID) {
			// Nothing was clicked or pressed: set back autoRepeatDelay to 250ms
			autoRepeatDelay = 250;
		}
	}
	if (boxSelected != uint(-1)) {
		if (autoName) {
			saveName = Common::String::format("AUTO%04d", nextSaveNum);
		} else {
			saveName = savesList[boxSelected + fileListOffset];
		}
		ConfMan.setInt(_isVisiting ? "visits_list_off" : "saves_list_off", fileListOffset);
		return boxSelected + fileListOffset + 1;
	} else {
		return uint(-1);
	}
}

const MsgBoxParameters CryOmni3DEngine_Versailles::kWarpMsgBoxParameters = {
	9, 241, 22, 2, 1, 36, 18, 20, 10, 5
};

const MsgBoxParameters CryOmni3DEngine_Versailles::kFixedimageMsgBoxParameters = {
	3, 241, 22, 2, 1, 40, 20, 20, 10, 3
};

void CryOmni3DEngine_Versailles::displayMessageBox(const MsgBoxParameters &params,
		const Graphics::Surface *surface, const Common::String &msg, const Common::Point &position,
		const Common::Functor0<void> &callback) {
	Graphics::ManagedSurface dstSurface;
	dstSurface.create(surface->w, surface->h, surface->format);
	dstSurface.blitFrom(*surface);

	_fontManager.setSurface(&dstSurface);
	_fontManager.setCurrentFont(params.font);
	_fontManager.setTransparentBackground(true);
	_fontManager.setForeColor(params.foreColor);
	_fontManager.setLineHeight(params.lineHeight);
	_fontManager.setSpaceWidth(params.spaceWidth);
	_fontManager.setCharSpacing(params.charSpacing);

	uint width = params.initialWidth;
	uint height = params.initialHeight;
	uint lineCount = 0;
	Common::Point pt = position;
	Common::Rect rct;

	bool notEnough = true;
	bool tooLarge = false;

	while (notEnough && !tooLarge) {
		width += params.incrementWidth;
		height += params.incrementHeight;
		rct = Common::Rect::center(pt.x, pt.y, width, height);
		if (rct.left < 10) {
			rct.left = 10;
			if (pt.x < 320) {
				pt.x += 10;
			}
		}
		if (rct.right >= 630) {
			rct.right = 630;
			if (pt.x > 320) {
				pt.x -= 10;
			}
		}
		if (rct.top <= 10) {
			rct.top = 10;
			if (pt.y < 240) {
				pt.y += 10;
			}
		}
		if (rct.bottom >= 470) {
			rct.bottom = 470;
			if (pt.y > 235) { // sic.
				pt.y -= 10;
			}
		}
		if (rct.left == 10 && rct.top == 10 && rct.right == 630 && rct.bottom == 470) {
			tooLarge = true;
		}
		lineCount = _fontManager.getLinesCount(msg, rct.width() - 12);
		if (lineCount && lineCount * _fontManager.lineHeight() + 18 < (uint)rct.height()) {
			notEnough = false;
		}
	}
	rct.setHeight(lineCount * _fontManager.lineHeight() + 12);
	if (rct.bottom > 479) {
		rct.bottom = 479;
	}

	Graphics::Surface subSurface = dstSurface.getSubArea(rct);
	makeTranslucent(subSurface, surface->getSubArea(rct));
	rct.grow(-6);
	_fontManager.setupBlock(rct);
	_fontManager.displayBlockText(msg);

	drawCountdown(&dstSurface);

	g_system->copyRectToScreen(dstSurface.getPixels(), dstSurface.pitch, 0, 0,
	                           dstSurface.w, dstSurface.h);

	waitMouseRelease();
	uint disappearTime = g_system->getMillis() + msg.size() * params.timeoutChar * 10;
	bool finished = false;
	while (!finished) {
		callback();

		if (g_system->getMillis() > disappearTime) {
			finished = true;
		}
		if (getCurrentMouseButton() == 1) {
			finished = true;
		}
	}

	// Restore image
	g_system->copyRectToScreen(surface->getPixels(), surface->pitch, 0, 0, surface->w, surface->h);
}

void CryOmni3DEngine_Versailles::displayMessageBoxWarp(const Common::String &message) {
	Common::Point mousePos = getMousePos();
	mousePos += Common::Point(0, 32);
	if (mousePos.x > 639) {
		mousePos.x = 639;
	}
	if (mousePos.y > 479) {
		mousePos.y = 479;
	}
	displayMessageBox(kWarpMsgBoxParameters, _omni3dMan.getSurface(), message, mousePos,
	                  Common::Functor0Mem<void, CryOmni3DEngine_Versailles>(this,
	                          &CryOmni3DEngine_Versailles::warpMsgBoxCB));
}

void CryOmni3DEngine_Versailles::displayCredits() {
	waitMouseRelease();

	Graphics::ManagedSurface creditsSurface;
	Image::ImageDecoder *imageDecoder = loadHLZ(getFilePath(kFileTypeMenu, "credits.hlz"));
	if (!imageDecoder) {
		return;
	}

	const Graphics::Surface *bgFrame = imageDecoder->getSurface();

	byte palette[256 * 3];
	memset(palette, 0, 256 * 3);
	// getPalette returns the first color not index 0
	memcpy(palette + 3 * imageDecoder->getPaletteStartIndex(), imageDecoder->getPalette(),
	       3 * imageDecoder->getPaletteColorCount());
	copySubPalette(palette, _cursorPalette, 240, 8);

	creditsSurface.create(bgFrame->w, bgFrame->h, bgFrame->format);

	_fontManager.setCurrentFont(3);
	_fontManager.setTransparentBackground(true);
	_fontManager.setForeColor(243);
	_fontManager.setLineHeight(14);
	_fontManager.setSpaceWidth(0);
	_fontManager.setCharSpacing(1);
	_fontManager.setSurface(&creditsSurface);

	Common::Path creditsPath(getFilePath(kFileTypeText, _localizedFilenames[LocalizedFilenames::kCredits]));
	Common::File creditsFile;
	if (!creditsFile.open(creditsPath)) {
		warning("Failed to open credits file: %s",
		        _localizedFilenames[LocalizedFilenames::kCredits].c_str());
		delete imageDecoder;
		return;
	}

	showMouse(false);

	char line[256];
	bool end = false;
	bool calculatedScreen = false;
	uint lineHeight = 20;
	uint currentY = 0;
	int32 fileOffset = 0;
	bool skipScreen = false;

	while (!end && creditsFile.readLine(line, ARRAYSIZE(line))) {
		// Remove line ending
		line[strlen(line) - 1] = '\0';
		if (!strncmp(line, "###", 3)) {
			// Prefix for commands
			if (!strncmp(line + 3, "ECRAN", 5)) {
				// ECRAN command
				if (calculatedScreen) {
					g_system->copyRectToScreen(creditsSurface.getPixels(), creditsSurface.pitch, 0, 0,
					                           creditsSurface.w, creditsSurface.h);
					if (skipScreen) {
						// Just display palette
						setPalette(palette, 0, 256);
					} else {
						fadeInPalette(palette);
					}
					skipScreen = false;
					// Wait
					uint endScreenTime = g_system->getMillis() + 6000;
					while (g_system->getMillis() < endScreenTime && !skipScreen) {
						g_system->updateScreen();
						g_system->delayMillis(10);

						if (pollEvents()) {
							if (getCurrentMouseButton() == 1) {
								skipScreen = true;
							}
							Common::KeyCode kc = getNextKey().keycode;
							while (kc != Common::KEYCODE_INVALID) {
								if (kc == Common::KEYCODE_SPACE) {
									skipScreen = true;
									break;
								} else if (kc == Common::KEYCODE_ESCAPE) {
									skipScreen = true;
									end = true;
									break;
								}
								kc = getNextKey().keycode;
							}
							clearKeys();
						}
						if (shouldAbort()) {
							skipScreen = true;
							end = true;
						}
					}
					if (!skipScreen) {
						fadeOutPalette();
						fillSurface(0);
					}
					currentY = 0;
					fileOffset = creditsFile.pos();
					calculatedScreen = false;
				} else {
					// We just finished calculated all lines, roll back and display them
					creditsFile.seek(fileOffset, SEEK_SET);
					calculatedScreen = true;
					if (currentY <= 480 - lineHeight) {
						// Center in screen
						currentY = (480 - lineHeight) / 2 - currentY / 2;
					} else {
						currentY = 3;
					}
					creditsSurface.blitFrom(*bgFrame);
				}
			} else if (!strcmp(line + 3, "T0")) {
				_fontManager.setCurrentFont(1);
				lineHeight = _fontManager.getFontMaxHeight() + 10;
			} else if (!strcmp(line + 3, "T1")) {
				_fontManager.setCurrentFont(2);
				lineHeight = _fontManager.getFontMaxHeight() + 10;
			} else if (!strcmp(line + 3, "T2")) {
				_fontManager.setCurrentFont(4);
				lineHeight = _fontManager.getFontMaxHeight() + 10;
			} else if (!strcmp(line + 3, "T3")) {
				_fontManager.setCurrentFont(2);
				lineHeight = _fontManager.getFontMaxHeight() + 10;
			} else if (!strcmp(line + 3, "T4")) {
				_fontManager.setCurrentFont(5);
				lineHeight = _fontManager.getFontMaxHeight() + 10;
			} else if (!strcmp(line + 3, "T5")) {
				_fontManager.setCurrentFont(6);
				lineHeight = _fontManager.getFontMaxHeight() + 10;
			} else {
				warning("Unknown ### command : %s", line + 3);
			}
		} else {
			// Text
			if (calculatedScreen) {
				uint width = _fontManager.getStrWidth(line);
				// Center around 315
				_fontManager.displayStr(315 - width / 2, currentY, line);
			}
			currentY += lineHeight;
		}
	}
	showMouse(true);

	delete imageDecoder;
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
