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

#ifndef DGDS_DIALOG_H
#define DGDS_DIALOG_H

#include "common/stream.h"
#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/serializer.h"

#include "dgds/dgds_rect.h"

namespace Graphics {
class ManagedSurface;
}

namespace Dgds {

class DgdsFont;
class SceneOp;

enum DialogFlags {
	kDlgFlagNone     =         0,
	kDlgFlagFlatBg   =         1,
	kDlgFlagLeftJust =         2,
	kDlgFlagLo4      =         4,
	kDlgFlagLo8      =         8,
	kDlgFlagLo80     =      0x80,
	kDlgFlagHiFinished = 0x10000,
	kDlgFlagHi2      =   0x20000,
	kDlgFlagHi4      =   0x40000,
	kDlgFlagRedrawSelectedActionChanged      =   0x80000,
	kDlgFlagHi10     =  0x100000,
	kDlgFlagHi20     =  0x200000,
	kDlgFlagHi40     =  0x400000,
	kDlgFlagVisible  =  0x800000,
	kDlgFlagOpening  = 0x1000000,
};

enum DialogFrameType {
	kDlgFramePlain = 1,
	kDlgFrameBorder = 2,
	kDlgFrameThought = 3,
	kDlgFrameRounded = 4
};

enum DialogDrawStage {
	kDlgDrawStageForeground = 0,
	kDlgDrawStageBackground = 1,
	kDlgDrawFindSelectionPointXY = 2,
	kDlgDrawFindSelectionTxtOffset = 3,
};

struct DialogAction {
	// The game initializes str offsets to pointers, but let's be a bit nicer.
	uint16 strStart; /// The start of the clickable text for this action
	uint16 strEnd;	 /// End of clickable text for this action
	//byte unk[8]; /* Not initialized in loader */
	Common::Array<SceneOp> sceneOpList;  /// ops to run when this is selected

	Common::String dump(const Common::String &indent) const;
};

class DialogState {
public:
	DialogState() : _hideTime(0), _lastMouseX(0), _lastMouseY(0), _charWidth(0),
			_charHeight(0), _strMouseLoc(0), _selectedAction(nullptr) {}
	uint _hideTime;
	DgdsRect _loc;
	int _lastMouseX;
	int _lastMouseY;
	uint16 _charWidth;
	uint16 _charHeight;
	int _strMouseLoc;
	struct DialogAction *_selectedAction;

	Common::String dump(const Common::String &indent) const;
	Common::Error syncState(Common::Serializer &s);
};

class Dialog {
public:
	Dialog();
	uint16 _num;
	uint16 _fileNum; // HOC onward
	DgdsRect _rect;
	uint16 _bgColor;
	uint16 _fontColor;
	uint16 _selectionBgCol;
	uint16 _selectonFontCol;
	uint16 _fontSize;
	DialogFlags _flags;
	DialogFrameType _frameType;
	uint16 _time;
	uint16 _nextDialogFileNum; // HOC onward, always set 0 in dragon.
	uint16 _nextDialogDlgNum;
	uint16 _talkDataNum; // Willy onward, always set 0 in dragon and HoC
	uint16 _talkDataHeadNum; // Willy onward, always set 0 in dragon and HoC
	Common::Array<DialogAction> _action;
	Common::String _str;

	Common::SharedPtr<DialogState> _state;

 	void draw(Graphics::ManagedSurface *dst, DialogDrawStage stage);
	void setFlag(DialogFlags flg);
	void clearFlag(DialogFlags flg);
	void flipFlag(DialogFlags flg);
	bool hasFlag(DialogFlags flg) const;
	void updateSelectedAction(int delta);
	struct DialogAction *pickAction(bool isClosing, bool isForceClose);
	Common::String dump(const Common::String &indent) const;
	void clear();

	Common::Error syncState(Common::Serializer &s);

private:
	void drawType1(Graphics::ManagedSurface *dst, DialogDrawStage stage);
	void drawType2(Graphics::ManagedSurface *dst, DialogDrawStage stage);
	void drawType3(Graphics::ManagedSurface *dst, DialogDrawStage stage);
	void drawType4(Graphics::ManagedSurface *dst, DialogDrawStage stage);

	void drawType2BackgroundDragon(Graphics::ManagedSurface *dst, const Common::String &title);
	void drawType2BackgroundChina(Graphics::ManagedSurface *dst, const Common::String &title);
	void drawType2BackgroundBeamish(Graphics::ManagedSurface *dst, const Common::String &title);

	void drawFindSelectionXY();
	void drawFindSelectionTxtOffset();
	void drawForeground(Graphics::ManagedSurface *dst, uint16 fontcol, const Common::String &txt);

	const DgdsFont *getDlgTextFont() const;

	static int _lastSelectedDialogItemNum;
	static Dialog *_lastDialogSelectionChangedFor;
};



} // end namespace Dgds

#endif // DGDS_DIALOG_H
