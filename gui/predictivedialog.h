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

#ifndef GUI_PREDICTIVEDIALOG_H
#define GUI_PREDICTIVEDIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/stream.h"

namespace GUI {

class EditTextWidget;
class ButtonWidget;
class PicButtonWidget;

class PredictiveDialog : public GUI::Dialog {
public:
	PredictiveDialog();
	~PredictiveDialog() override;

	void reflowLayout() override;

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleKeyUp(Common::KeyState state) override;
	void handleKeyDown(Common::KeyState state) override;

	const char *getResult() const { return _predictiveResult; }

private:
	enum ButtonId {
		kButton1Act   = 0,
		kButton2Act   = 1,
		kButton3Act   = 2,
		kButton4Act   = 3,
		kButton5Act   = 4,
		kButton6Act   = 5,
		kButton7Act   = 6,
		kButton8Act   = 7,
		kButton9Act   = 8,
		kNextAct      = 9,
		kAddAct       = 10,
		kDelAct       = 11,
		kCancelAct    = 12,
		kOkAct        = 13,
		kModeAct      = 14,
		kButton0Act   = 15,
		kNoAct        = -1
	};

	enum {
		kButtonCount = kButton0Act + 1
	};

	enum {
		kRepeatDelay = 500
	};

	enum {
		kMaxLineLen = 80,
		kMaxWordLen = 24,
		kMaxWord = 50
	};

	struct Dict {
		Dict() : dictLine(nullptr), dictText(nullptr), dictActLine(nullptr),
		         dictLineCount(0), dictTextSize(0) {}
		~Dict() { free(dictText); }
		char **dictLine;
		char *dictText;
		char *dictActLine; // using only for united dict...
		int32 dictLineCount;
		int32 dictTextSize;
		Common::String nameDict;
		Common::String defaultFilename;
	};

	uint8 countWordsInString(const char *const str);
	void bringWordtoTop(char *str, int wordnum);
	void loadDictionary(Common::SeekableReadStream *in, Dict &dict);
	void loadAllDictionary(Dict &dict);
	void addWordToDict();
	void addWord(Dict &dict, const Common::String &word, const Common::String &code);
	bool searchWord(const char *const where, const Common::String &whatCode);
	int binarySearch(const char *const *const dictLine, const Common::String &code, const int dictLineCount);
	bool matchWord();
	void processButton(ButtonId active);
	void pressEditText();

	void saveUserDictToFile();

	void mergeDicts();

	void updateHighLightedButton(ButtonId active);
private:
	Dict _unitedDict;
	Dict _predictiveDict;
	Dict _userDict;

	int _mode;
	ButtonId _lastButton;

	bool _userDictHasChanged;

	int _wordNumber;
	uint8 _numMatchingWords;
	char _predictiveResult[40];

	Common::String _currentCode;
	Common::String _currentWord;
	Common::String _prefix;

	uint32 _curTime, _lastTime;
	ButtonId _lastPressedButton;
	ButtonId _curPressedButton;

	char _temp[kMaxWordLen + 1];
	int _repeatcount[kMaxWordLen];

	char *_memoryList[kMaxWord];
	int _numMemory;

	Common::String _search;

	bool _navigationWithKeys;
	bool _needRefresh;
	bool _isPressed;

private:
	EditTextWidget *_editText;
	ButtonWidget   *_button[kButtonCount];
};

} // namespace GUI

#endif
