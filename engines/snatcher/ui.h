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


#ifndef SNATCHER_UI_H
#define SNATCHER_UI_H


#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
class SeekableWriteStream;
} // namespace Common

namespace Snatcher {

class CmdQueue;
class GraphicsEngine;
class ResourcePointer;
class ScriptArray;
class UI {
public:
	UI(GraphicsEngine *gfx, CmdQueue *que, ResourcePointer *scd);
	~UI();

	void setControllerConfig(uint8 cfg) { _controllerCfg = cfg; }
	void setScriptTextResource(const uint8 *textResource) { _scriptTextResource = textResource; }

	bool displayDialog(int sceneInfo, int sceneTextOffset, uint16 inputFlags);

	bool drawVerbs();
	bool verbsTabInputPrompt(uint16 inputFlags);

	void setScriptVerbsArray(const ScriptArray *verbsArray) { _scriptVerbsArray = verbsArray; }
	void setScriptSentenceArray(const ScriptArray *sentenceArray) { _scriptSentenceArray = sentenceArray; }

	int8 getSelectedVerb() const { return (int8)_selectedVerb; }
	void resetVerbSelection() { _prevSelectedVerb = 0; }
	void setVerbsTabLayout(uint16 layout);
	void setInterpreterMode(uint16 mode);

	void loadState(Common::SeekableReadStream *in);
	void saveState(Common::SeekableWriteStream *out);

private:
	void printDialogStringHead();
	void printDialogStringBody();
	bool checkStringPrintProgress();
	bool waitWithCursorAnim(uint16 inputFlags);

	uint8 _textColor;
	uint8 _textY;
	uint8 _textY2;
	//uint8 _makestrbt1;
	uint8 _sceneId;
	uint8 _textLineBreak;
	uint8 _textLineEnd;
	uint16 _sceneTextOffsCur;
	uint16 _sceneInfo;
	uint16 _sceneTextOffset;
	uint16 _sceneTextOffsStart;
	uint8 _waitCursorFrame;
	uint8 _waitCursorAnimDelay;
	uint8 *_dialogTextBuffer;

	const uint8 *_scriptTextResource;
	uint8 _controllerCfg;
	int16 _progress;

private:
	uint8 drawVerb(uint8 id);
	void drawHiliteVerb(uint8 id);
	bool drawUnderscoreString();
	void drawUnderscoreCursor();
	void verbSelect2setResult();
	void verbsTabHandleInput(uint16 inputFlags);
	uint8 verbsTabCheckInputFlags(uint16 inputFlags);
	bool verbsTabMoveHilite(int &pos);
	void adjustHilitePos();
	void verbTabSwapPage();
	void verbSelect();
	bool verbSelect2();
	void virtKeybBackspace();
	void virtKeybStart();
	bool verbSelect2checkSelection(uint16 val);
	void virtKeybPrint();

	uint16 _verbsInterpreterMode;
	uint8 _lastVerbFirstPage;
	uint8 _numVerbsFirstPage;
	uint8 _numVerbsLastPage;
	uint8 _lastVerbLastPage;
	uint8 _prevSelectedVerb;
	uint8 _numVerbsMax;
	uint8 _una_rr_hi;
	uint8 _una_rr_lo;
	uint8 _verbsTabCurPage;
	uint8 _transit_02;
	int16 _transDW1;
	int16 _transDW2;
	int16 _drawVerbsWd1;
	uint8 _textInputMarginLeft;
	int16 _textInputColumnCur;
	int16 _textInputColumnMax;
	uint8 _textInputCursorState;
	uint8 _textInputCursorBlinkCnt;
	uint8 _selectedVerb;
	uint8 _vkeybPosTotal;
	uint8 _vkeybColumnWidth;
	uint8 _verbsTabOffsX;
	uint8 _verbsTabOffsY;

	int8 _lastHiliteVerb;
	int8 _hiliteVerb;
	uint8 _lastVerbDrawn;
	uint8 *_verbTextBuffer;
	uint8 *_underscoreStr;
	uint8 *_textInputStr;
	const uint8 *_verbsTabLayoutMap;
	uint16 _verbsTabLayout;
	int16 _hilitePosX;
	int16 _hilitePosY;
	bool _vkeybVisible;
	const ScriptArray *_scriptVerbsArray;
	const ScriptArray *_scriptSentenceArray;
	int16 _progress2;

private:
	GraphicsEngine *_gfx;
	CmdQueue *_que;
	ResourcePointer *_scd;
};

} // End of namespace Snatcher

#endif // SNATCHER_UI_H
