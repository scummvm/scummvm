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

#ifndef CHEWY_ATDS_H
#define CHEWY_ATDS_H

#include "common/str.h"
#include "common/str-array.h"
#include "chewy/resource.h"
#include "chewy/text.h"

namespace Chewy {

#define ATDS_VOC_OFFSET 20

#define AAD_MAX_PERSON 10

#define AAD_STR_START 0
#define AAD_STR_END 1
#define AAD_DATA 0
#define ATS_DATA 1
#define DIALOG_CLOSEUP_DATA 2	// ADS_DATA
#define INV_USE_DATA 4
#define INV_ATS_DATA 6

#define MAX_HANDLE 8
#define AAD_HANDLE 0
#define ADS_HANDLE 2

#define MAX_STR_SPLIT 10

#define SPLIT_CENTER 1
#define SPLIT_LEFT 2

#define ATDS_END_LINE 0
#define ATDS_END_BLOCK 0x0b
#define ATDS_END_ENTRY 0x0c
#define ATDS_END_TEXT 0x0d
#define ATDS_END 0x0e
#define BLOCKENDE 0x0f

#define MAX_ATS_ROOM 100

#define ATS_ACTIVE_BIT 1
#define ATS_COUNT_BIT 2
#define ATS_RESET_BIT 4
#define ATS_ACTION_BIT 8

#define MAX_ATS_STATUS 3

#define TXT_MARK_NAME 0
#define TXT_MARK_LOOK 1
#define TXT_MARK_USE 2
#define TXT_MARK_WALK 3
#define TXT_MARK_TALK 4

#define CONTROL_BYTE 0xff

#define DIALOG_CLOSEUP_MAX 6
#define DIALOG_CLOSEUP_STACK_SIZE 50

#define ADS_EXIT_BIT 1
#define ADS_SHOW_BIT 2
#define ADS_RESTART_BIT 4

struct KbdMouseInfo;
class Text;

struct AtdsVar {
	int16 _silent = 0;
	int16 _delay = 1;
	int16 _diaNr = 0;

	bool _eventsEnabled = false;
	int16 _vocNr = 0;

	void (*aad_str)(int16 diaNr, int16 strNr, int16 personNr, int16 mode) = nullptr;
};

#define IUID_IIB 0
#define IUID_SIB 1
#define IUID_REC 2
#define IUID_AUTO 3
#define IUID_DET 4
#define IUID_PLAYER 5
#define INV_USE_DEF 16

struct AadInfo {
	int16 _x;
	int16 _y;
	int16 _color;

	AadInfo() : _x(0), _y(0), _color(0) {}
	void load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 6; }
};

class AadInfoArray : public Common::Array<AadInfo> {
public:
	void load(const void *data, size_t count);
};

struct AadStrHeader {
	int16 _akPerson;
	int16 _vocNr;
};

// ADS (dialog closeup) header
struct DialogCloseupTxtHeader {
	int16 _diaNr;
	int16 _perNr;
	int16 _aMov;
	int16 _curNr;

	bool load(const void *src);
	static constexpr int SIZE() { return 8; }
};

struct DialogCloseupVariables {
	int16 _dialog;
	int16 _autoDia;
	DialogCloseupTxtHeader _txtHeader;
	AadInfoArray _person;
	char *_ptr;
	char *_blockPtr;
	int16 _strNr;
	int16 _delayCount;
	int16 _silentCount;
};

struct DialogCloseupNextBlock {
	int16 _blkNr;
	int16 _endNr;
};

struct AadVar {
	int16 _dialog;

	DialogCloseupTxtHeader *_txtHeader;
	AadStrHeader *_strHeader;
	AadInfoArray _person;
	char *_ptr;
	int16 _strNr;
	int16 _delayCount;
	int16 _silentCount;
};

struct AtsTxtHeader {
	uint16 _txtNr = 0;
	int16 _aMov = 0;
	int16 _curNr = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 8; }
};

struct AtsVar {
	AtsTxtHeader _txtHeader;
	uint16 vocNum;
	Common::String text;
	int16 _delayCount;
	int16 _silentCount;
	int16 _color;
	int16 _txtMode;
	bool shown;
};

struct SplitStringRet {
	char **_strPtr;
	int16 *_x;
	int16 _y;
	short _nr;
	int16 _next;

	SplitStringRet() {
		_strPtr = nullptr;
		_x = nullptr;
		_y = 0;
		_nr = 0;
		_next = 0;
	}
};

struct SplitStringInit {
	char *_str;

	int16 _x;
	int16 _y;
};

class Atdsys {
public:
	Atdsys();
	~Atdsys();

	void set_delay(int16 *delay, int16 silent);
	void set_split_win(int16 nr, int16 x, int16 y);
	void split_string(SplitStringInit *ssi, SplitStringRet *ret);
	void calc_txt_win(SplitStringInit *ssi);
	void str_null2leer(char *strStart, char *strEnd);
	void load_atds(int16 chunkNr, int16 mode);

	void set_handle(int16 mode, int16 chunkStart, int16 chunkNr);
	bool start_ats(int16 txtNr, int16 txtMode, int16 color, int16 mode, int16 *vocNr);
	void stop_ats();
	bool atsShown() { return _atsv.shown; }
	void print_ats(int16 x, int16 y, int16 scrX, int16 scrY);
	int16 getControlBit(int16 txtNr, int16 bitIdx);
	void setControlBit(int16 txtNr, int16 bitIdx);
	void delControlBit(int16 txtNr, int16 bitIdx);
	void set_ats_str(int16 txtNr, int16 txtMode, int16 strNr, int16 mode);
	void set_all_ats_str(int16 txtNr, int16 strNr, int16 mode);
	int16 start_aad(int16 diaNr, bool continueWhenSpeechEnds = false);
	void stopAad();
	void print_aad(int16 scrX, int16 scrY);
	int16 aadGetStatus();
	void set_string_end_func(void (*strFunc)(int16 diaNr, int16 strNr, int16 personNr, int16 mode));
	void aad_search_dia(int16 diaNr, char **ptr);
	int16 aadGetTxtLen(char *str);

	bool startDialogCloseup(int16 diaNr);
	void stopDialogCloseup();
	char **dialogCloseupItemPtr(uint16 dialogNum, int16 blockNr, int16 *retNr);
	DialogCloseupNextBlock *dialogCloseupItemChoice(uint16 dialogNum, int16 blockNr, int16 itemNr);
	DialogCloseupNextBlock *calcNextDialogCloseupBlock(uint16 dialogNum, int16 blockNr, int16 itemNr);
	int16 getDialogCloseupStatus();
	void hideDialogCloseupItem(int16 diaNr, int16 blockNr, int16 itemNr);
	void showDialogCloseupItem(int16 diaNr, int16 blockNr, int16 itemNr);
	int16 getDialogCloseupBlock(uint16 dialogNum);
	void dialogCloseupSearchBlock(int16 blockNr, char **ptr);
	void dialogCloseupSearchItem(int16 itemNr, char **blkAdr);
	int16 startAutoDialogCloseup(char *itemAdr);

	int16 calc_inv_no_use(int16 curInv, int16 testNr);
	int8 getStereoPos(int16 x);
	void enableEvents(bool nr) {
		_atdsv._eventsEnabled = nr;
	}

	void saveAtdsStream(Common::WriteStream *stream);
	void loadAtdsStream(Common::SeekableReadStream *stream);
	uint32 getAtdsStreamSize() const;

	Common::StringArray getTextArray(uint dialogNum, uint entryNum, int type, int subEntry = -1);
	Common::String getTextEntry(uint dialogNum, uint entryNum, int type, int subEntry = -1);
	int16 getLastSpeechId();

private:
	void init();
	int16 get_delay(int16 txt_len);
	void initItemUseWith();
	
	char *_atdsMem[MAX_HANDLE] = { nullptr };
	int16 _atdsPoolOff[MAX_HANDLE] = { 0 };
	AadVar _aadv;
	AtsVar _atsv;
	DialogCloseupVariables _dialogCloseup;
	AtdsVar _atdsv;
	char *_ePtr[DIALOG_CLOSEUP_MAX] = { nullptr };
	int16 _eNr[DIALOG_CLOSEUP_MAX] = { 0 };
	DialogCloseupNextBlock _dialogCloseupNextBlock;
	uint8 _dialogCloseupStack[DIALOG_CLOSEUP_STACK_SIZE] = { 0 };
	int16 _dialogCloseupStackPtr;
	bool _continueWhenSpeechEnds = false;

	SplitStringInit _ssi[AAD_MAX_PERSON] = {
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
		{ 0, 100, 0 },
	};

	char *_splitPtr[MAX_STR_SPLIT] = { nullptr };
	int16 _splitX[MAX_STR_SPLIT] = { 0 };
	int16 _invBlockNr;
	int16 _mousePush = 0;
	int _printDelayCount1 = 0;
	DialogResource *_dialogResource;
	Text *_text;

	Common::HashMap<uint32, uint16> _itemUseWithDesc;
};

} // namespace Chewy

#endif
