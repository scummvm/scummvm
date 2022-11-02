/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/biochip_view.h"
#include "buried/buried.h"
#include "buried/fbcdata.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/livetext.h"
#include "buried/resources.h"
#include "buried/scene_view.h"

#include "common/error.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

BioChipMainViewWindow::BioChipMainViewWindow(BuriedEngine *vm, Window *parent, int currentBioChipID) : Window(vm, parent) {
	_rect = Common::Rect(0, 0, 432, 189);
	_bioChipDisplayWindow = createBioChipSpecificViewWindow(currentBioChipID);
	_currentBioChipID = currentBioChipID;

	if (_bioChipDisplayWindow)
		_bioChipDisplayWindow->showWindow(kWindowShow);

	_oldCursor = _vm->_gfx->setCursor(kCursorArrow);
}

BioChipMainViewWindow::~BioChipMainViewWindow() {
	delete _bioChipDisplayWindow;
}

bool BioChipMainViewWindow::onSetCursor(uint message) {
	_vm->_gfx->setCursor((Cursor)_oldCursor);
	return true;
}

bool BioChipMainViewWindow::changeCurrentBioChip(int newBioChipID) {
	delete _bioChipDisplayWindow;
	_currentBioChipID = newBioChipID;
	_bioChipDisplayWindow = createBioChipSpecificViewWindow(newBioChipID);

	if (_bioChipDisplayWindow)
		_bioChipDisplayWindow->showWindow(kWindowShow);

	return true;
}

enum {
	REGION_NONE = 0
};

enum {
	REGION_BRIEFING = 1,
	REGION_JUMP = 2,
	REGION_MAYAN = 3,
	REGION_CASTLE = 4,
	REGION_DAVINCI = 5,
	REGION_SPACE_STATION = 6
};

class JumpBiochipViewWindow : public Window {
public:
	JumpBiochipViewWindow(BuriedEngine *vm, Window *parent);

	void onPaint() override;
	void onLButtonDown(const Common::Point &point, uint flags) override;
	void onLButtonUp(const Common::Point &point, uint flags) override;
	void onMouseMove(const Common::Point &point, uint flags) override;

private:
	Common::Rect _missionBriefing;
	Common::Rect _jumpButton;
	Common::Rect _mayanZone;
	Common::Rect _castle;
	Common::Rect _daVinci;
	Common::Rect _spaceStation;
	AVIFrames _stillFrames;
	int _curSelection;

	int _curState;

	static const byte _briefingNavData[4][2];
	int _curRegion;
	int _curBriefingPage;

	bool _currentMissionReviewed;
};

const byte JumpBiochipViewWindow::_briefingNavData[4][2] = {
	{ 2, 2 },
	{ 4, 3 },
	{ 7, 2 },
	{ 9, 4 }
};

JumpBiochipViewWindow::JumpBiochipViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_missionBriefing = Common::Rect(306, 24, 422, 74);
	_jumpButton = Common::Rect(306, 84, 422, 134);
	_mayanZone = Common::Rect(9, 48, 272, 74);
	_castle = Common::Rect(9, 78, 296, 104);
	_daVinci = Common::Rect(9, 108, 284, 134);
	_spaceStation = Common::Rect(9, 138, 284, 164);
	_curRegion = REGION_NONE;
	_curSelection = -1;
	_curState = 0;
	_curBriefingPage = 0;
	_currentMissionReviewed = false;

	_rect = Common::Rect(0, 0, 432, 189);

	if (!_stillFrames.open(_vm->getFilePath(IDS_BC_JUMP_VIEW_FILENAME)))
		error("Failed to open jump view video");
}

void JumpBiochipViewWindow::onPaint() {
	Common::Rect absoluteRect = getAbsoluteRect();

	if (_curState == 0) {
		const Graphics::Surface *frame = _stillFrames.getFrame(0);
		_vm->_gfx->blit(frame, absoluteRect.left, absoluteRect.top);

		if (_curSelection >= 0) {
			frame = _stillFrames.getFrame(1);

			Common::Rect highlightRect(11, _curSelection * 30 + 50, 11 + 23, _curSelection * 30 + 50 + 23);
			_vm->_gfx->blit(frame, highlightRect, makeAbsoluteRect(highlightRect));
			_vm->_gfx->blit(frame, Common::Rect(306, 24, 306 + 116, 24 + 50), makeAbsoluteRect(Common::Rect(306, 24, 306 + 116, 24 + 50)));

			if (_currentMissionReviewed)
				_vm->_gfx->blit(frame, Common::Rect(306, 84, 306 + 116, 84 + 50), makeAbsoluteRect(Common::Rect(306, 84, 306 + 116, 84 + 50)));
		}
	} else {
		const Graphics::Surface *frame = _stillFrames.getFrame(_briefingNavData[_curSelection][0] + _curBriefingPage);
		_vm->_gfx->blit(frame, absoluteRect.left, absoluteRect.top);
	}
}

void JumpBiochipViewWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_curState == 0) {
		if (_jumpButton.contains(point) && _curSelection >= 0) {
			if (_currentMissionReviewed) {
				if (((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().faHeardAgentFigure == 1)
					_curRegion = REGION_JUMP;
				else
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_MBT_JUMP_LOCKOUT_TEXT));
			} else {
				((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_JUMP_BC_REVIEW_MISSION_TEXT_A + _curSelection));
				return;
			}
		} else if (_missionBriefing.contains(point) && _curSelection >= 0) {
			_curRegion = REGION_BRIEFING;
		} else if (_mayanZone.contains(point)) {
			_curRegion = REGION_MAYAN;
			_curSelection = 0;
		} else if (_castle.contains(point)) {
			_curRegion = REGION_CASTLE;
			_curSelection = 1;
		} else if (_daVinci.contains(point)) {
			_curRegion = REGION_DAVINCI;
			_curSelection = 2;
		} else if (_spaceStation.contains(point)) {
			_curRegion = REGION_SPACE_STATION;
			_curSelection = 3;
		}

		invalidateWindow(false);
	}
}

void JumpBiochipViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_curState == 0) {
		switch (_curRegion) {
		case REGION_BRIEFING:
			if (_missionBriefing.contains(point)) {
				_currentMissionReviewed = true;
				_curState = 1;
				_curBriefingPage = 0;
				invalidateWindow(false);

				switch (_curSelection) {
				case 0:
					((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpMayanBriefing = 1;
					break;
				case 1:
					((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpCastleBriefing = 1;
					break;
				case 2:
					((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpDaVinciBriefing = 1;
					break;
				case 3:
					((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpStationBriefing = 1;
					break;
				}
			}
			break;
		case REGION_JUMP:
			if (_jumpButton.contains(point)) {
				TempCursorChange cursorChange(kCursorWait);

				SceneViewWindow *sceneViewWindow = (SceneViewWindow *)getParent()->getParent();
				int curSelection = _curSelection;

				((GameUIWindow *)getParent()->getParent()->getParent())->_bioChipRightWindow->destroyBioChipViewWindow();
				sceneViewWindow->timeSuitJump(curSelection);
				return;
			}
			break;
		case REGION_MAYAN:
			if (_mayanZone.contains(point)) {
				if (((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpMayanBriefing == 1) {
					_currentMissionReviewed = true;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText("");
				} else {
					_currentMissionReviewed = false;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_JUMP_BC_REVIEW_MISSION_TEXT_A));
				}

				_curSelection = 0;
				invalidateWindow(false);
			}
			break;
		case REGION_CASTLE:
			if (_castle.contains(point)) {
				if (((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpCastleBriefing == 1) {
					_currentMissionReviewed = true;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText("");
				} else {
					_currentMissionReviewed = false;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_JUMP_BC_REVIEW_MISSION_TEXT_B));
				}

				_curSelection = 1;
				invalidateWindow(false);
			}
			break;
		case REGION_DAVINCI:
			if (_daVinci.contains(point)) {
				if (((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpDaVinciBriefing == 1) {
					_currentMissionReviewed = true;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText("");
				} else {
					_currentMissionReviewed = false;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_JUMP_BC_REVIEW_MISSION_TEXT_C));
				}

				_curSelection = 2;
				invalidateWindow(false);
			}
			break;
		case REGION_SPACE_STATION:
			if (_spaceStation.contains(point)) {
				if (((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().genJumpStationBriefing == 1) {
					_currentMissionReviewed = true;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText("");
				} else {
					_currentMissionReviewed = false;
					((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_JUMP_BC_REVIEW_MISSION_TEXT_D));
				}

				_curSelection = 3;
				invalidateWindow(false);
			}
			break;
		}

		_curRegion = REGION_NONE;
		invalidateWindow(false);
	} else {
		// Browsing the mission review pages
		Common::Rect returnRect(343, 157, 427, 185);
		Common::Rect nextPage(230, 25, 270, 43);
		Common::Rect prevPage(182, 25, 222, 43);

		if (returnRect.contains(point)) {
			_curState = 0;
			invalidateWindow(false);
		} else if (prevPage.contains(point)) {
			if (_curBriefingPage > 0) {
				_curBriefingPage--;
				invalidateWindow(false);
			}
		} else if (nextPage.contains(point)) {
			if (_curBriefingPage < _briefingNavData[_curSelection][1] - 1) {
				_curBriefingPage++;
				invalidateWindow(false);
			}
		}
	}
}

void JumpBiochipViewWindow::onMouseMove(const Common::Point &point, uint flags) {
	if (_curState == 0 && _curRegion > 0) {
		switch (_curRegion) {
		case REGION_BRIEFING:
			if (!_missionBriefing.contains(point))
				_curRegion = REGION_NONE;
			break;
		case REGION_JUMP:
			if (!_jumpButton.contains(point))
				_curRegion = REGION_NONE;
			break;
		case REGION_MAYAN:
		case REGION_CASTLE:
		case REGION_DAVINCI:
		case REGION_SPACE_STATION: {
			int newRegion = REGION_NONE;

			if (_mayanZone.contains(point)) {
				newRegion = REGION_MAYAN;
				_curSelection = 0;
			} else if (_castle.contains(point)) {
				newRegion = REGION_CASTLE;
				_curSelection = 1;
			} else if (_daVinci.contains(point)) {
				newRegion = REGION_DAVINCI;
				_curSelection = 2;
			} else if (_spaceStation.contains(point)) {
				newRegion = REGION_SPACE_STATION;
				_curSelection = 3;
			}

			if (newRegion != REGION_NONE && _curRegion != newRegion) {
				_curRegion = newRegion;
				invalidateWindow(false);
			}
			break;
		}
		}
	}
}

class EvidenceBioChipViewWindow : public Window {
public:
	EvidenceBioChipViewWindow(BuriedEngine *vm, Window *parent);
	~EvidenceBioChipViewWindow();

	void onPaint() override;
	void onLButtonUp(const Common::Point &point, uint flags) override;

private:
	Common::Rect _evidence[6];
	Common::Rect _pageButton;
	AVIFrames _stillFrames;
	AVIFrames _evidenceFrames;
	int _status;
	int _pageIndex;
	Graphics::Surface _preBuffer;

	bool rebuildMainPrebuffer();
};

EvidenceBioChipViewWindow::EvidenceBioChipViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_evidence[0] = Common::Rect(14, 49, 96, 83);
	_evidence[1] = Common::Rect(14, 91, 96, 125);
	_evidence[2] = Common::Rect(14, 133, 96, 167);
	_evidence[3] = Common::Rect(225, 20, 307, 54);
	_evidence[4] = Common::Rect(225, 61, 307, 95);
	_evidence[5] = Common::Rect(225, 103, 307, 137);
	_pageButton = Common::Rect(336, 156, 420, 185);
	_status = 0;
	_pageIndex = 0;

	_rect = Common::Rect(0, 0, 432, 189);

	_preBuffer.create(432, 189, g_system->getScreenFormat());

	if (!_stillFrames.open(_vm->getFilePath(IDS_BC_EVIDENCE_VIEW_FILENAME)))
		error("Failed to open evidence still frames video");

	if (!_evidenceFrames.open(_vm->getFilePath(IDS_BC_EVIDENCE_ITEMS_FILENAME)))
		error("Failed to open evidence frames video");

	// Build the initial page
	rebuildMainPrebuffer();

	// Display some live text if no evidence is present
	if (((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().evcapNumCaptured == 0)
		((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_MBT_EVIDENCE_NONE_ACQUIRED));
}

EvidenceBioChipViewWindow::~EvidenceBioChipViewWindow() {
	_preBuffer.free();
}

void EvidenceBioChipViewWindow::onPaint() {
	Common::Rect absoluteRect = getAbsoluteRect();

	if (_status == 0) {
		_vm->_gfx->blit(&_preBuffer, absoluteRect.left, absoluteRect.top);
	} else {
		const Graphics::Surface *frame = _stillFrames.getFrame(_status + 2);
		_vm->_gfx->blit(frame, absoluteRect.left, absoluteRect.top);
	}
}

void EvidenceBioChipViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_status == 0) {
		// Get the number of items currently captured
		int itemCount = ((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().evcapNumCaptured;

		// Loop through the evidence piece regions, determining if we have another page to go to
		for (int i = 0; i < 6; i++) {
			if (_evidence[i].contains(point) && (_pageIndex * 6 + i) < itemCount) {
				_status = ((SceneViewWindow *)getParent()->getParent())->getNumberFromGlobalFlagTable(_pageIndex * 6 + i);
				invalidateWindow(false);
				((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText(_vm->getString(IDS_EC_DESC_TEXT_A + _status - 1), false);

				// Give scores for research
				if (_status + 2 == 3)
					((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().scoreResearchCastleFootprint = 1;
				else if (_status + 2 == 10)
					((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().scoreResearchDaVinciFootprint = 1;
			}
		}

		// Did we click on the next page button and is it valid?
		if (_pageButton.contains(point)) {
			if (itemCount > 6) {
				if (_pageIndex * 6 < itemCount - 6) {
					_pageIndex++;
				} else {
					_pageIndex = 0;
				}

				rebuildMainPrebuffer();
				invalidateWindow(false);
			}
		}
	} else {
		// Return to the last evidence page displayed
		_status = 0;
		invalidateWindow(false);
		((GameUIWindow *)getParent()->getParent()->getParent())->_liveTextWindow->updateLiveText("");
	}
}

bool EvidenceBioChipViewWindow::rebuildMainPrebuffer() {
	int itemCount = ((SceneViewWindow *)getParent()->getParent())->getGlobalFlags().evcapNumCaptured;
	int frameIndex = (itemCount > 6) ? 1 : 0;
	const Graphics::Surface *frame = _stillFrames.getFrame(frameIndex);
	_vm->_gfx->crossBlit(&_preBuffer, 0, 0, 432, 189, frame, 0, 0);

	for (int i = 0; i < 6; i++) {
		if ((_pageIndex * 6 + i) < itemCount) {
			frameIndex = ((SceneViewWindow *)getParent()->getParent())->getNumberFromGlobalFlagTable(_pageIndex * 6 + i) - 1;
			frame = _evidenceFrames.getFrame(frameIndex);

			if (frame) {
				byte transparentColor = _vm->isTrueColor() ? 255 : 248;
				_vm->_gfx->opaqueTransparentBlit(&_preBuffer, _evidence[i].left, _evidence[i].top, 203, 34, frame, 2, 2, 0, transparentColor, transparentColor, transparentColor);
			}
		}
	}

	return true;
}

enum {
	REGION_SAVE = 1,
	REGION_RESTORE = 2,
	REGION_PAUSE = 3,
	REGION_QUIT = 4,
	REGION_FLICKER = 5,
	REGION_TRANSITION_SPEED = 6
};

class InterfaceBioChipViewWindow : public Window {
public:
	InterfaceBioChipViewWindow(BuriedEngine *vm, Window *parent);
	~InterfaceBioChipViewWindow();

	void onPaint() override;
	void onLButtonDown(const Common::Point &point, uint flags) override;
	void onLButtonUp(const Common::Point &point, uint flags) override;
	void onMouseMove(const Common::Point &point, uint flags) override;

private:
	Common::Rect _save;
	Common::Rect _pause;
	Common::Rect _restore;
	Common::Rect _quit;
	Common::Rect _flicker;
	Common::Rect _transitionSpeed;

	int _curRegion;
	int _transLocation;
	int _soundLocation;

	Graphics::Surface *_background;
	Graphics::Surface *_cycleCheck;
	Graphics::Surface *_caret;
	PauseToken _pauseToken;
};

InterfaceBioChipViewWindow::InterfaceBioChipViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_pauseToken = _vm->pauseEngine();
	_save = Common::Rect(192, 37, 300, 74);
	_pause = Common::Rect(192, 84, 300, 121);
	_restore = Common::Rect(313, 37, 421, 74);
	_quit = Common::Rect(313, 84, 421, 121);
	_flicker = Common::Rect(14, 146, 164, 166);

	// For whatever reason, the Spanish version uses the demo coordinates.
	_transitionSpeed = (_vm->getLanguage() == Common::ES_ESP || _vm->isDemo()) ? Common::Rect(14, 117, 179, 140) : Common::Rect(14, 100, 125, 140);

	_curRegion = REGION_NONE;
	_soundLocation = 0;
	_transLocation = _vm->getTransitionSpeed() * 50;

	_rect = Common::Rect(0, 0, 432, 189);

	_background = _vm->_gfx->getBitmap(_vm->isDemo() ? IDB_BCM_INTERFACE_NORMAL : IDB_BCV_INTERFACE_MAIN);
	_cycleCheck = _vm->_gfx->getBitmap(_vm->isDemo() ? IDB_BCM_INTERFACE_SELECTED : IDB_BCV_INTERFACE_CHECK);
	_caret = _vm->_gfx->getBitmap(_vm->isDemo() ? IDB_BCM_INTERFACE_SLIDER : IDB_BCV_INTERFACE_HANDLE);
}

InterfaceBioChipViewWindow::~InterfaceBioChipViewWindow() {
	_background->free();
	delete _background;

	_cycleCheck->free();
	delete _cycleCheck;

	_caret->free();
	delete _caret;
}

void InterfaceBioChipViewWindow::onPaint() {
	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(_background, absoluteRect.left, absoluteRect.top);

	if (((SceneViewWindow *)getParent()->getParent())->getCyclingStatus())
		_vm->_gfx->blit(_cycleCheck, absoluteRect.left + 13, absoluteRect.top + 144);

	if (_caret) {
		if (_vm->isDemo()) {
			_vm->_gfx->opaqueTransparentBlit(_vm->_gfx->getScreen(), absoluteRect.left + _transLocation + 12, absoluteRect.top + 112, 20, 35, _caret, 0, 0, 0, 255, 255, 255);
		} else {
			// For whatever reason, the Spanish version has to be different.
			int dy = (_vm->getLanguage() == Common::ES_ESP) ? 115 : 97;
			_vm->_gfx->opaqueTransparentBlit(_vm->_gfx->getScreen(), absoluteRect.left + _transLocation + 14, absoluteRect.top + dy, 15, 30, _caret, 0, 0, 0, 248, _vm->isTrueColor() ? 252 : 248, 248);
		}
	}
}

void InterfaceBioChipViewWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_save.contains(point))
		_curRegion = REGION_SAVE;
	else if (_restore.contains(point))
		_curRegion = REGION_RESTORE;
	else if (_pause.contains(point))
		_curRegion = REGION_PAUSE;
	else if (_quit.contains(point))
		_curRegion = REGION_QUIT;
	else if (_flicker.contains(point))
		_curRegion = REGION_FLICKER;
	else if (_transitionSpeed.contains(point))
		_curRegion = REGION_TRANSITION_SPEED;
}

void InterfaceBioChipViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	switch (_curRegion) {
	case REGION_SAVE:
		_vm->handleSaveDialog();
		break;
	case REGION_RESTORE:
		_vm->handleRestoreDialog();
		break;
	case REGION_QUIT:
		if (_vm->runQuitDialog()) {
			((FrameWindow *)_vm->_mainWindow)->showMainMenu();
			return;
		}
		break;
	case REGION_PAUSE:
		_vm->pauseGame();
		break;
	case REGION_FLICKER:
		if (_flicker.contains(point)) {
			if (((SceneViewWindow *)(_parent->getParent()))->getCyclingStatus()) {
				((SceneViewWindow *)(_parent->getParent()))->enableCycling(false);
				invalidateRect(_flicker, false);
			} else {
				((SceneViewWindow *)(_parent->getParent()))->enableCycling(true);
				invalidateRect(_flicker, false);
			}
		}
		break;
	case REGION_TRANSITION_SPEED:
		_transLocation = CLIP<int>(point.x - 14, 0, 150);

		if ((_transLocation % 50) > 25)
			_transLocation = (_transLocation / 50 + 1) * 50;
		else
			_transLocation = _transLocation / 50 * 50;

		_vm->setTransitionSpeed(_transLocation / 50);

		invalidateWindow(false);
		break;
	}

	_curRegion = REGION_NONE;
}

void InterfaceBioChipViewWindow::onMouseMove(const Common::Point &point, uint flags) {
	if (_curRegion == REGION_TRANSITION_SPEED) {
		int newPos = CLIP<int>(point.x - 14, 0, 150);

		if ((newPos % 50) > 25)
			newPos = (newPos / 50 + 1) * 50;
		else
			newPos = newPos / 50 * 50;

		if (_transLocation != newPos) {
			_transLocation = newPos;
			invalidateWindow(false);
		}
	}
}

class FilesBioChipViewWindow : public Window {
public:
	FilesBioChipViewWindow(BuriedEngine *vm, Window *parent);

	void onPaint() override;
	void onLButtonUp(const Common::Point &point, uint flags) override;

private:
	int _curPage;
	AVIFrames _stillFrames;
	Common::Array<FilesPage> _navData;
};

FilesBioChipViewWindow::FilesBioChipViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curPage = 0;
	_rect = Common::Rect(0, 0, 432, 189);

	Common::SeekableReadStream *fbcStream = _vm->getFileBCData(IDBD_BC_VIEW_DATA);
	assert(fbcStream);

	fbcStream->skip(2); // Page count

	while (fbcStream->pos() < fbcStream->size()) {
		FilesPage page;
		page.pageID = fbcStream->readSint16LE();
		page.returnPageIndex = fbcStream->readSint16LE();
		page.prevButtonPageIndex = fbcStream->readSint16LE();
		page.nextButtonPageIndex = fbcStream->readSint16LE();

		for (int i = 0; i < 6; i++) {
			page.hotspots[i].left = fbcStream->readSint16LE();
			page.hotspots[i].top = fbcStream->readSint16LE();
			page.hotspots[i].right = fbcStream->readSint16LE();
			page.hotspots[i].bottom = fbcStream->readSint16LE();
			page.hotspots[i].pageIndex = fbcStream->readSint16LE();
		}

		_navData.push_back(page);
	}

	delete fbcStream;

	if (!_stillFrames.open(_vm->getFilePath(IDS_BC_FILES_VIEW_FILENAME)))
		error("Failed to open files biochip video");
}

void FilesBioChipViewWindow::onPaint() {
	const Graphics::Surface *frame = _stillFrames.getFrame(_curPage);
	assert(frame);

	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(frame, absoluteRect.left, absoluteRect.top);
}

void FilesBioChipViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_curPage < 0 || _curPage >= (int)_navData.size())
		return;

	const FilesPage &page = _navData[_curPage];

	Common::Rect returnButton(343, 157, 427, 185);
	Common::Rect previous(193, 25, 241, 43);
	Common::Rect next(253, 25, 301, 43);

	if (page.returnPageIndex >= 0 && returnButton.contains(point)) {
		_curPage = page.returnPageIndex;
		invalidateWindow(false);
		return;
	}

	if (page.nextButtonPageIndex >= 0 && next.contains(point)) {
		_curPage = page.nextButtonPageIndex;
		invalidateWindow(false);

		if (_curPage == 6)
			((SceneViewWindow *)(_parent->getParent()))->getGlobalFlags().scoreResearchBCJumpsuit = 1;

		return;
	}

	if (page.prevButtonPageIndex >= 0 && previous.contains(point)) {
		_curPage = page.prevButtonPageIndex;
		invalidateWindow(false);
		return;
	}

	for (int i = 0; i < 6; i++) {
		if (page.hotspots[i].pageIndex >= 0 && Common::Rect(page.hotspots[i].left, page.hotspots[i].top, page.hotspots[i].right, page.hotspots[i].bottom).contains(point)) {
			_curPage = page.hotspots[i].pageIndex;
			invalidateWindow(false);

			if (_curPage == 21)
				((SceneViewWindow *)(_parent->getParent()))->getGlobalFlags().scoreResearchMichelle = 1;
			else if (_curPage == 31)
				((SceneViewWindow *)(_parent->getParent()))->getGlobalFlags().scoreResearchMichelleBkg = 1;

			return;
		}
	}
}

Window *BioChipMainViewWindow::createBioChipSpecificViewWindow(int bioChipID) {
	switch (bioChipID) {
	case kItemBioChipInterface:
		return new InterfaceBioChipViewWindow(_vm, this);
	case kItemBioChipJump:
		return new JumpBiochipViewWindow(_vm, this);
	case kItemBioChipEvidence:
		return new EvidenceBioChipViewWindow(_vm, this);
	case kItemBioChipFiles:
		return new FilesBioChipViewWindow(_vm, this);
	}

	// No entry for this BioChip
	return nullptr;
}

} // End of namespace Buried
