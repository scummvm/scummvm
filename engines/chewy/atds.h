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

#include "chewy/resource.h"

namespace Chewy {

#define ATDS_VOC_OFFSET 20

#define AAD_MAX_PERSON 10

#define AAD_STR_START 0
#define AAD_STR_END 1
#define AAD_DATA 0
#define ATS_DATA 1
#define ADS_DATA 2
#define INV_USE_DATA 4
#define INV_IDX_DATA 5
#define INV_ATS_DATA 6

#define MAX_HANDLE 8
#define AAD_HANDLE 0
#define ATS_HANDLE 1
#define ADS_HANDLE 2
#define INV_USE_HANDLE 4
#define INV_IDX_HANDLE 5
#define INV_ATS_HANDLE 6
#define ATDS_HANDLE 7

enum DisplayMode {
	DISPLAY_NONE = -1,
	DISPLAY_TXT = 0,
	DISPLAY_VOC = 1,
	DISPLAY_ALL = 2
};

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

#define MAX_ADS_DIALOG 500
#define ADS_MAX_BL_EIN 6
#define ADS_STACK_SIZE 50

#define ADS_EXIT_BIT 1
#define ADS_SHOW_BIT 2
#define ADS_RESTART_BIT 4

struct KbdMouseInfo;

struct AdsDiaHeaders {
	int16 _nr;
};

struct AtdsVar {
	int16 _silent = 0;
	int16 *_delay = nullptr;
	int16 _diaNr = 0;

	DisplayMode _display = DISPLAY_TXT;
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
#define INV_STRC_NR 30

struct InvUse {
	int16 _objId;
	int16 _objNr;
	int16 _txtNr;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 6; }
};

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

struct AadTxtHeader {
	int16 _diaNr;
	int16 _perNr;
	int16 _aMov;
	int16 _curNr;

	bool load(const void *src);
	static constexpr int SIZE() { return 8; }
};

struct AadStrHeader {
	int16 _akPerson;
	int16 _vocNr;
};

struct AadVar {
	int16 _dialog;

	AadTxtHeader *_txtHeader;
	AadStrHeader *_strHeader;
	AadInfoArray _person;
	char *_ptr;
	int16 _strNr;
	int16 _delayCount;
	int16 _silentCount;
};

struct AdsTxtHeader {
	int16 _diaNr;
	int16 _perNr;
	int16 _aMov;
	int16 _curNr;

	bool load(const void *src);
	static constexpr int SIZE() { return 8; }
};

struct AdsVar {
	int16 _dialog;
	int16 _autoDia;
	AdsTxtHeader _txtHeader;
	AadInfoArray _person;
	char *_ptr;
	char *_blkPtr;
	int16 _strNr;
	int16 _delayCount;
	int16 _silentCount;
};

struct AdsNextBlk {
	int16 _blkNr;
	int16 _endNr;
};

struct AtsStrHeader {
	uint16 _vocNr = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 2; }
};

struct AtsTxtHeader {
	uint16 _txtNr = 0;
	int16 _aMov = 0;
	int16 _curNr = 0;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() { return 8; }
};

struct AtsVar {
	DisplayMode _display = DISPLAY_NONE;
	AtsTxtHeader _txtHeader;
	AtsStrHeader _strHeader;
	char *_ptr;
	int16 _delayCount;
	int16 _silentCount;
	int16 _txtLen;
	int16 _color;
	int16 _txtMode;
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
	int16 _width;
	int16 _lines;
	int16 _mode;

	int16 Fvorx;
	int16 FHoehe;
};

class Atdsys {
public:
	Atdsys();
	~Atdsys();

	void set_delay(int16 *delay, int16 silent);
	void setHasSpeech(bool hasSpeech);
	void updateSoundSettings();
	void set_split_win(int16 nr, int16 x, int16 y);
	SplitStringRet *split_string(SplitStringInit *ssi);
	void calc_txt_win(SplitStringInit *ssi);
	void str_null2leer(char *strStart, char *strEnd);
	char *atds_adr(const char *fname, int16 chunkStart, int16 chunkNr);
	void load_atds(int16 chunkNr, int16 mode);

	Common::Stream *pool_handle(const char *fname);
	void set_handle(const char *fname, int16 mode, Common::Stream *handle, int16 chunkStart, int16 chunkNr);
	void open_handle(const char *fname, int16 mode);
	void close_handle(int16 mode);
	void crypt(char *txt, uint32 size);
	void init_ats_mode(int16 mode, uint8 *atsHeader);
	DisplayMode start_ats(int16 txtNr, int16 txtMode, int16 color, int16 mode, int16 *vocNr);
	void stop_ats();
	DisplayMode &ats_get_status();
	void print_ats(int16 x, int16 y, int16 scrX, int16 scrY);
	int16 getControlBit(int16 txtNr, int16 bitIdx, int16 mode);
	void setControlBit(int16 txtNr, int16 bitIdx, int16 mode);
	void delControlBit(int16 txtNr, int16 bitIdx, int16 mode);
	void set_ats_str(int16 txtNr, int16 txtMode, int16 strNr, int16 mode);
	void set_ats_str(int16 txtNr, int16 strNr, int16 mode);
	int16 get_ats_str(int16 txtNr, int16 txtMode, int16 mode);
	char *ats_get_txt(int16 txtNr, int16 txtMode, int16 *retNr, int16 mode);
	char *ats_search_block(int16 txtMode, char *txtAdr);
	void ats_search_nr(int16 txtNr, char **str);
	void ats_search_str(int16 *nr, uint8 *status, uint8 controlByte, char **str);
	void set_ats_mem(int16 mode);
	int16 start_aad(int16 diaNr);
	void stopAad();
	void print_aad(int16 scrX, int16 scrY);
	int16 aadGetStatus();
	void set_string_end_func(void (*strFunc)(int16 diaNr, int16 strNr, int16 personNr, int16 mode));
	void aad_search_dia(int16 diaNr, char **ptr);
	int16 aad_get_zeilen(char *str, int16 *txtLen);
	bool ads_start(int16 diaNr);
	void stop_ads();
	char **ads_item_ptr(uint16 dialogNum, int16 blockNr, int16 *retNr);
	AdsNextBlk *ads_item_choice(uint16 dialogNum, int16 blockNr, int16 itemNr);
	AdsNextBlk *calc_next_block(uint16 dialogNum, int16 blockNr, int16 itemNr);
	int16 ads_get_status();
	void hide_item(int16 diaNr, int16 blockNr, int16 itemNr);
	void show_item(int16 diaNr, int16 blockNr, int16 itemNr);
	int16 return_block(uint16 dialogNum);
	void ads_search_block(int16 blockNr, char **ptr);
	void ads_search_item(int16 itemNr, char **blkAdr);
	int16 start_ads_auto_dia(char *itemAdr);
	int16 calc_inv_no_use(int16 curInv, int16 testNr, int16 mode);
	int16 getStereoPos(int16 x);
	void enableEvents(bool nr) {
		_atdsv._eventsEnabled = nr;
	}
	int getAtdDisplay() const {
		return _atdsv._display;	
	}

	void saveAtdsStream(Common::WriteStream *stream);
	void loadAtdsStream(Common::SeekableReadStream *stream);
	uint32 getAtdsStreamSize() const;

private:
	int16 get_delay(int16 txt_len);

	Common::Stream *_atdsHandle[MAX_HANDLE] = { nullptr };
	char *_atdsMem[MAX_HANDLE] = { nullptr };
	int16 _atdsPoolOff[MAX_HANDLE] = { 0 };
	char *_atsMem = nullptr;
	uint8 *_ats_sheader = nullptr;
	uint8 *_ats_st_header[4];
	char *_atds_id_ptr = nullptr;
	AadVar _aadv;
	AtsVar _atsv;
	AdsVar _adsv;
	AtdsVar _atdsv;
	char *_ePtr[ADS_MAX_BL_EIN] = { nullptr };
	int16 _eNr[ADS_MAX_BL_EIN] = { 0 };
	AdsNextBlk _adsnb;
	uint8 _adsStack[ADS_STACK_SIZE] = { 0 };
	int16 _adsStackPtr;
	SplitStringRet _ssret;
	SplitStringRet *_ssr = nullptr;

	SplitStringInit _ssi[AAD_MAX_PERSON] = {
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
		{ 0, 100, 0, 200, 4, SPLIT_CENTER, 8, 8 },
	};

	char *_splitPtr[MAX_STR_SPLIT] = { nullptr };
	int16 _splitX[MAX_STR_SPLIT] = { 0 };
	int16 _invBlockNr;
	char *_invUseMem = nullptr;
	int16 _tmpDelay;
	bool _hasSpeech = false;
	int16 _mousePush = 0;
	int _printDelayCount1 = 0;
	DialogResource *_dialogResource;
};

} // namespace Chewy

#endif
