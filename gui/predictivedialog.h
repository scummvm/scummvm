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
 */

#ifndef GLOBAL_DIALOGS_H
#define GLOBAL_DIALOGS_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/stream.h"

namespace GUI {

class EditTextWidget;
class ButtonWidget;
class PicButtonWidget;

enum ButtonId {
	kBtn1Act   = 0,
	kBtn2Act   = 1,
	kBtn3Act   = 2,
	kBtn4Act   = 3,
	kBtn5Act   = 4,
	kBtn6Act   = 5,
	kBtn7Act   = 6,
	kBtn8Act   = 7,
	kBtn9Act   = 8,
	kNextAct   = 9,
	kAddAct    = 10,
	kDelAct    = 11,
	kCancelAct = 12,
	kOkAct     = 13,
	kModeAct   = 14,
	kBtn0Act   = 15,
	kNoAct     = -1
};

enum {
	kRepeatDelay = 500
};

enum {
	MAXLINELEN = 80,
	MAXWORDLEN = 24,
	MAXWORD = 50
};

class PredictiveDialog : public GUI::Dialog {
	typedef Common::String String;

public:
	PredictiveDialog();
	~PredictiveDialog();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(Common::KeyState state);
	virtual void handleTickle();

	char * getResult() { return _predictiveResult; }
private:
	struct Dict {
		char **dictLine;
		char *dictText;
		char *dictActLine; // using only for united dict...
		int32 dictLineCount;
		int32 dictTextSize;
		String nameDict;
		String fnameDict;
	};

	uint8 countWordsInString(char *str);
	void bringWordtoTop(char *str, int wordnum);
	void loadDictionary(Common::SeekableReadStream *in, Dict &dict);
	void loadAllDictionary(Dict &dict);
	void addWordToDict();
	void addWord(Dict &dict, const String &word, const String &code);
	bool searchWord(char *where, const String &whatCode);
	int binarySearch(char **dictLine, const String &code, int dictLineCount);
	bool matchWord();
	void processBtnActive(ButtonId active);
	void pressEditText();

	void saveUserDictToFile();

	void mergeDicts();
private:
	Dict _unitedDict;
	Dict _predictiveDict;
	Dict _userDict;

	int _mode;
	ButtonId _lastbutton;

	bool _userDictHasChanged;

	int _wordNumber;
	uint8 _numMatchingWords;
	char _predictiveResult[40];

	String _currentCode;
	String _currentWord;
	String _prefix;

	uint32 _curTime, _lastTime;
	ButtonId _lastPressBtn;

	char _temp[MAXWORDLEN + 1];
	int _repeatcount[MAXWORDLEN];

	char *_memoryList[MAXWORD];
	int _numMemory;

	String _search;

	bool _navigationwithkeys;
private:
	EditTextWidget *_edittext;
	ButtonWidget   *_modebutton;
	ButtonWidget   *_delbtn;
	ButtonWidget   *_addBtn;
};

} // namespace GUI

#endif
