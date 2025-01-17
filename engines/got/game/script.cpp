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

#include "got/game/script.h"
#include "common/textconsole.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/main.h"
#include "got/game/object.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/utils/file.h"
#include "got/vars.h"
#include "got/views/dialogs/ask.h"
#include "got/views/dialogs/say.h"

namespace Got {

#define ERROR_MAX 10
#define SCR_BUFF_SIZE 5000

static const char *SCR_COMMAND[] = {
	"!@#$%", "END", "GOTO", "GOSUB", "RETURN", "FOR", "NEXT",
	"IF", "ELSE", "RUN",
	"ADDJEWELS", "ADDHEALTH", "ADDMAGIC", "ADDKEYS",
	"ADDSCORE", "SAY", "ASK", "SOUND", "PLACETILE",
	"ITEMGIVE", "ITEMTAKE", "ITEMSAY", "SETFLAG", "LTOA",
	"PAUSE", "TEXT", "EXEC", "VISIBLE", "RANDOM",
	nullptr};

static const char *INTERNAL_VARIABLE[] = {
	"@JEWELS", "@HEALTH", "@MAGIC", "@SCORE",
	"@SCREEN", "@KEYS",
	"@OW", "@GULP", "@SWISH", "@YAH", "@ELECTRIC",
	"@THUNDER", "@DOOR", "@FALL", "@ANGEL", "@WOOP",
	"@DEAD", "@BRAAPP", "@WIND", "@PUNCH", "@CLANG",
	"@EXPLODE", "@FLAG", "@ITEM", "@THORTILE",
	"@THORPOS",
	nullptr};

static const char *SCR_ERROR[] = {
	"!@#$%", "Out of Memory", "Can't Read Script",
	"Too Many Labels", "No END",
	"Syntax", "Out of Range", "Undefined Label",
	"RETURN Without GOSUB", "Nesting",
	"NEXT Without FOR",
	nullptr};

static const char *OFFENSE[] = {
	"Cussing", "Rebellion", "Kissing Your Mother Goodbye",
	"Being a Thunder God", "Door-to-Door Sales",
	"Carrying a Concealed Hammer"};

static const char *REASON[] = {
	"We heard you say 'Booger'.",
	"You look kind of rebellious.",
	"Your mother turned you in.",
	"We don't want you here.",
	"Nobody wants your sweepers.",
	"That's a dangerous weapon."};

Scripts::ScrFunction Scripts::scr_func[5] = {
	&Scripts::scr_func1,
	&Scripts::scr_func2,
	&Scripts::scr_func3,
	&Scripts::scr_func4,
	&Scripts::scr_func5,
};

Scripts *g_scripts;

void execute_script(long index, const Gfx::Pics &speakerIcon, ScriptEndFn endFn) {
	g_scripts->execute_script(index, speakerIcon, endFn);
}

Scripts::Scripts() {
	g_scripts = this;
}

Scripts::~Scripts() {
	g_scripts = nullptr;
}

void Scripts::execute_script(long index, const Gfx::Pics &speakerIcon, ScriptEndFn endFn) {
	// Firstly disable any on-screen actors
	for (int i = 0; i < MAX_ACTORS; i++)
		_G(actor[i])._show = 0;

	_endFn = endFn;
	_scrIndex = index;
	_scrPic = speakerIcon;

	Common::fill(_numVar, _numVar + 26, 0);
	Common::fill((char *)_strVar, (char *)_strVar + 81 * 26, 0);
	runScript();
}

void Scripts::runScript(bool firstTime) {
	// Clear line label buffer, line ptrs, and the gosub stack
	Common::fill((char *)_lineLabel, (char *)_lineLabel + 32 * 9, 0);
	Common::fill(_linePtr, _linePtr + 32, (char *)nullptr);
	Common::fill(_gosubStack, _gosubStack + 32, (char *)nullptr);
	_gosubPtr = 0;

	Common::fill(_forVar, _forVar + 10, 0);
	Common::fill(_forVal, _forVal + 10, 0);
	Common::fill(_forStack, _forStack + 10, (char *)nullptr);
	_forPtr = 0;

	int i = read_script_file();
	if (i != 0) {
		script_error(i);
		script_exit();
		return;
	}

	if (firstTime)
		script_entry();

	_buffPtr = _buffer;
	scriptLoop();
}

void Scripts::scriptLoop() {
	while (_paused == SCRIPT_READY) {
		if (_G(cheat) && _G(key_flag[_B]))
			break;

		int ret = get_command();
		if (ret == -1)
			break; // Ignore NO END error

		if (ret == -2) {
			script_error(5); // Syntax error
			break;
		}

		if (ret > 0) {
			ret = exec_command(ret);
			if (ret == -100) { // RUN command
				if (_buffer)
					free(_buffer);

				runScript(false);
				return;
			}
			if (!ret)
				break;
		}
	}

	if (_paused == SCRIPT_READY)
		script_exit();
}

void Scripts::script_exit() {
	if (_buffer) {
		free(_buffer);
		_buffer = nullptr;
	}

	if (_endFn)
		_endFn();
}

int Scripts::skip_colon() {
	while (*_buffPtr == 0 || *_buffPtr == ':') {
		_buffPtr++;
		if (_buffPtr > _buffEnd)
			return 0;
	}

	return 1;
}

int Scripts::get_command() {
	if (!skip_colon())
		return -1;

	int i = 0;
	while (true) {
		if (!SCR_COMMAND[i])
			break; // Lookup command

		int len = strlen(SCR_COMMAND[i]);
		if (!strncmp(_buffPtr, SCR_COMMAND[i], len)) {
			_buffPtr += len;
			return i;
		}

		i++;
	}

	if (Common::isAlpha(*_buffPtr)) {
		int ret;
		if (*(_buffPtr + 1) == '=') { // Num var assignment
			i = (*_buffPtr) - 65;
			_buffPtr += 2;
			ret = calc_value();
			if (!ret)
				return -2;

			_numVar[i] = _lValue;
			return 0;
		}

		if (*(_buffPtr + 1) == '$' && *(_buffPtr + 2) == '=') {
			i = (*_buffPtr) - 65;
			_buffPtr += 3;
			ret = calc_string(0); // String var assignment
			if (ret == 0)
				return -2;

			if (ret == -1)
				return -3;

			if (strlen(_tempS) > 80)
				return -3;

			Common::strcpy_s(_strVar[i], _tempS);
			return 0;
		}
	}

	return -2;
}

int Scripts::calc_string(int mode) {
	// if mode==1 stop at comma
	char varstr[255];
	uint varnum;

	Common::strcpy_s(varstr, "");

	if (!skip_colon())
		return 0;

strloop:
	if (*_buffPtr == '"') {
		get_str();
		if (strlen(varstr) + strlen(_tempS) < 255)
			Common::strcat_s(varstr, _tempS);
		goto nextstr;
	}
	if (Common::isAlpha(*_buffPtr)) {
		if (*(_buffPtr + 1) == '$') {
			varnum = (*_buffPtr) - 65;
			if (strlen(varstr) + strlen(_strVar[varnum]) < 255)
				Common::strcat_s(varstr, _strVar[varnum]);
			_buffPtr += 2;
			goto nextstr;
		}
	}
	return 0;

nextstr:
	if (*_buffPtr == 0 || *_buffPtr == 58) {
		_buffPtr++;
		goto strdone;
	}
	if (*_buffPtr == ',' && mode == 1)
		goto strdone;

	if (*_buffPtr == '+') {
		_buffPtr++;
		goto strloop;
	}
	return 0;

strdone:

	Common::strcpy_s(_tempS, (char *)varstr);
	return 1;
}

void Scripts::get_str() {
	_buffPtr++;
	int t = 0;

	while (1) {
		if (*_buffPtr == '"' || *_buffPtr == 0) {
			_tempS[t] = 0;
			if (*_buffPtr == '"')
				_buffPtr++;
			return;
		}

		_tempS[t++] = *_buffPtr;
		_buffPtr++;
	}
}

int Scripts::calc_value() {
	long tmpval2 = 0;
	char exptype = 1;

	while (1) {
		if (!get_next_val())
			return 0;
		switch (exptype) {
		case 0:
			tmpval2 = tmpval2 * _lTemp;
			break;
		case 1:
			tmpval2 = tmpval2 + _lTemp;
			break;
		case 2:
			tmpval2 = tmpval2 - _lTemp;
			break;
		case 3:
			if (_lTemp != 0)
				tmpval2 = tmpval2 / _lTemp;
			break;
		}

		char ch = *_buffPtr;
		switch (ch) {
		case 42:
			exptype = 0; /* multiply */
			break;
		case 43:
			exptype = 1; /* add */
			break;
		case 45:
			exptype = 2; /* minus */
			break;
		case 47:
			exptype = 3; /* divide */
			break;
		default:
			_lValue = tmpval2;
			return 1;
		}

		_buffPtr++;
	}
}

int Scripts::get_next_val() {
	char ch = *_buffPtr;
	if (ch == 0 || ch == ':')
		return 0;
	if (ch == 64)
		return get_internal_variable();

	if (Common::isAlpha(ch)) {
		_buffPtr++;
		_lTemp = _numVar[ch - 65];
		return 1;
	}

	if (strchr("0123456789-", ch)) {
		char tmpstr[25];
		int t = 0;
		tmpstr[t] = ch;
		_buffPtr++;
		t++;
		while (strchr("0123456789", *_buffPtr) && *_buffPtr != 0) {
			tmpstr[t] = *_buffPtr;
			_buffPtr++;
			t++;
		}
		tmpstr[t] = 0;
		if (t > 10)
			return 0;
		_lTemp = atol(tmpstr);
		return 1;
	}

	return 0;
}

int Scripts::get_internal_variable() {
	int i = 0;
	while (true) {
		if (!INTERNAL_VARIABLE[i])
			return 0; // Lookup internal variable
		int len = strlen(INTERNAL_VARIABLE[i]);
		if (!strncmp(_buffPtr, INTERNAL_VARIABLE[i], len)) {
			_buffPtr += len;
			break;
		}
		i++;
	}
	switch (i) {
	case 0:
		_lTemp = _G(thor_info).jewels;
		break;
	case 1:
		_lTemp = _G(thor)->_health;
		break;
	case 2:
		_lTemp = _G(thor_info).magic;
		break;
	case 3:
		_lTemp = _G(thor_info).score;
		break;
	case 4:
		_lTemp = _G(current_level);
		break;
	case 5:
		_lTemp = _G(thor_info).keys;
		break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
		_lTemp = (long)(i - 5l);
		break;
	case 22:
		if (!calc_value())
			return 0;
		i = (int)_lValue;
		if (i < 1 || i > 64)
			return 0;

		_lTemp = _G(setup)._flags[i - 1] ? 1 : 0;
		break;
	case 23:
		if (_G(thor_info).inventory & 64)
			_lTemp = _G(thor_info).object;
		else
			_lTemp = 0;
		break;
	case 24:
		_lTemp = _G(scrn).icon[(_G(thor)->_y + 8) / 16][(_G(thor)->_x + 7) / 16];
		break;
	case 25:
		_lTemp = (((_G(thor)->_y + 8) / 16) * 20) + ((_G(thor)->_x + 7) / 16);
		break;
	default:
		return 0;
	}

	return 1;
}

int Scripts::get_line(char *src, char *dst) {
	int cnt = 0;
	if (!src)
		return cnt;
	
	while (*src != 13) {
		if (*src != 10) {
			*dst = *src;
			dst++;
		}
		cnt++;
		src++;
	}

	*dst = 0;
	cnt++;

	return cnt;
}

int Scripts::read_script_file() {
	char temp_buff[255];
	char quote_flag;
	int len, p, ret, cnt;
	Common::String str;
	char tmps[255];
	char *sbuff = nullptr;
	char *sb;

	_buffer = (char *)malloc(SCR_BUFF_SIZE);
	if (!_buffer) {
		ret = 1;
		goto done;
	};
	_buffPtr = _buffer;
	Common::fill(_buffer, _buffer + SCR_BUFF_SIZE, 0);

	sbuff = (char *)malloc(25000l);
	sb = sbuff;
	if (!sb) {
		ret = 1;
		goto done;
	};

	str = Common::String::format("SPEAK%d", _G(area));
	if (res_read(str.c_str(), sb) < 0) {
		ret = 6;
		goto done;
	}

	str = Common::String::format("|%ld", _scrIndex);
	Common::strcpy_s(temp_buff, str.c_str());

	while (1) {
		cnt = get_line(sb, (char *)tmps);
		sb += cnt;
		if (!strcmp(tmps, "|EOF")) {
			ret = 2;
			goto done;
		}
		if (!strcmp(tmps, temp_buff))
			break;
	}
	_numLabels = 0;
	while (1) {
		cnt = get_line(sb, (char *)tmps);
		if (!strcmp(tmps, "|STOP")) {
			if (_buffPtr != _buffer) {
				_buffEnd = _buffPtr;
				ret = 0;
				goto done;
			}
			ret = 2;
			goto done;
		}
		sb += cnt;
		len = strlen(tmps);
		if (len < 2) {
			*_buffPtr = 0;
			_buffPtr++;
			continue;
		}
		quote_flag = 0;
		p = 0;
		for (int i = 0; i < len; i++) {
			char ch = tmps[i];
			if (ch == 34)
				quote_flag ^= 1;
			else if (ch == 13 || ch == 10) { // Check for CR
				temp_buff[p] = 0;
				break;
			} else if ((ch == 39 || ch == 96) && !quote_flag) {
				temp_buff[p] = 0;
				break;
			}
			if (!quote_flag)
				ch = toupper(ch);
			if (quote_flag || ch > 32) {
				temp_buff[p++] = ch;
			}
		}
		temp_buff[p] = 0;

		len = strlen(temp_buff);
		if (len > 0 && len < 10 && temp_buff[len - 1] == ':') { //line label
			temp_buff[len - 1] = 0;
			_linePtr[_numLabels] = _buffPtr;
			Common::strcpy_s(_lineLabel[_numLabels++], (char *)temp_buff);
			if (_numLabels > 31) {
				ret = 3;
				goto done;
			}
			*_buffPtr = 0;
			_buffPtr++;
			continue;
		}

		Common::strcpy_s(_buffPtr, SCR_BUFF_SIZE, temp_buff);
		_buffPtr += strlen(temp_buff);
		*_buffPtr = 0;
		_buffPtr++;
	}

done:
	if (sbuff)
		free(sbuff);

	return ret;
}

void Scripts::script_error(int err_num) {
	int line_num = 1;
	char *tb = _buffer;

	while (1) {
		if (*tb == 0)
			line_num++;
		if (tb >= _buffPtr)
			break;
		tb++;
	}

	if (err_num > ERROR_MAX)
		err_num = 5; // Unknown=syntax

	warning("%s in Line #%d", SCR_ERROR[err_num], line_num);
}

int Scripts::cmd_goto() {
	char s[255];

	Common::strcpy_s(s, _buffPtr);
	char *p = strchr(s, ':');
	if (p)
		*p = 0;

	for (int i = 0; i < _numLabels; i++) {
		int len = strlen(s);
		if (len == 0)
			break;
		if (!strcmp(s, _lineLabel[i])) {
			_newPtr = _linePtr[i];
			_buffPtr += len;
			return 0;
		}
	}
	return 8;
}

int Scripts::cmd_if() {
	if (!calc_value())
		return 5;
	
	long tmpval1 = _lValue;
	char exptype = *_buffPtr;
	_buffPtr++;

	char ch = *_buffPtr;
	if (ch == 60 || ch == 61 || ch == 62) {
		if (exptype == *_buffPtr)
			return 5;
		
		exptype += *_buffPtr;
		_buffPtr++;
	}
	if (!calc_value())
		return 5;

	long tmpval2 = _lValue;
	_buffPtr += 4;

	switch (exptype) {
	case 60: /* less than */
		if (tmpval1 < tmpval2)
			goto iftrue;
		goto iffalse;
	case 61: /* equal */
		if (tmpval1 == tmpval2)
			goto iftrue;
		goto iffalse;
	case 62: /* greater than */
		if (tmpval1 > tmpval2)
			goto iftrue;
		goto iffalse;
	case 121: /* less than or equal */
		if (tmpval1 <= tmpval2)
			goto iftrue;
		goto iffalse;
	case 122: /* less or greater (not equal) */
		if (tmpval1 != tmpval2)
			goto iftrue;
		goto iffalse;
	case 123: /* greater than or equal */
		if (tmpval1 >= tmpval2)
			goto iftrue;
		goto iffalse;
	default:
		return 5;
	}

iffalse:
	while (*_buffPtr != 0)
		_buffPtr++;
	while (*_buffPtr == 0)
		_buffPtr++;

	if (!strncmp(_buffPtr, "ELSE", 4))
		_buffPtr += 4;

iftrue:
	return 0;
}

int Scripts::cmd_run() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	_scrIndex = _lValue;
	return -100;
}

int Scripts::cmd_addjewels() {
	if (!calc_value())
		return 5;

	_buffPtr++;

	add_jewels(_lValue);
	return 0;
}

int Scripts::cmd_addhealth() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	add_health((int)_lValue);
	return 0;
}

int Scripts::cmd_addmagic() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	add_magic((int)_lValue);
	return 0;
}

int Scripts::cmd_addkeys() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	add_keys((int)_lValue);
	return 0;
}

int Scripts::cmd_addscore() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	add_score((int)_lValue);
	return 0;
}

int Scripts::cmd_say(int mode, int type) {
	int obj = 0;

	if (mode) {
		if (!calc_value())
			return 5;
		_buffPtr++;
		obj = (int)_lValue;
		if (obj < 0 || obj > 32)
			return 6;
		
		if (obj)
			obj += 10;
	}
	
	Common::fill(_G(tmp_buff), _G(tmp_buff) + TMP_SIZE, 0);
	char *p = (char *)_G(tmp_buff);

	while (calc_string(0)) {
		Common::strcpy_s(p, TMP_SIZE, _tempS);
		p += strlen(_tempS);
		*(p) = 10;
		p++;
	}
	*(p - 1) = 0;

	pause();
	Views::Dialogs::Say::show(obj, _scrPic, type);

	return 0;
}

int Scripts::cmd_ask() {
	int v = 0;
	char title[41], opt[41];
	Common::StringArray opts;

	memset(_G(tmp_buff), 0, TMP_SIZE);

	if (!skip_colon())
		return 5;

	if (Common::isAlpha(*_buffPtr)) {
		v = *_buffPtr - 65;
		_buffPtr++;
		if (*_buffPtr != ',')
			return 5;
		
		_buffPtr++;
	} else {
		return 5;
	}

	if (!calc_string(1))
		return 5;

	strncpy(title, _tempS, 41);
	title[40] = 0;

	if (*_buffPtr == ',') {
		_buffPtr++;
		if (!calc_value())
			return 5;

		_buffPtr++;
	} else {
		return 5;
	}

	_askVar = v;

	while (calc_string(0)) {
		Common::strcpy_s(opt, _tempS);
		opts.push_back(opt);

		if (opts.size() > 9)
			return 3;
	}

	// Pause the script execution, and open up an ask window.
	// Execution of the script will resume after a selection.
	pause();
	Views::Dialogs::Ask::show(title, opts);
	return 0;
}

void Scripts::pause() {
	_paused = SCRIPT_PAUSED;
}

void Scripts::resume() {
	_paused = SCRIPT_RESUMING;
}

void Scripts::setAskResponse(int option) {
	_numVar[_askVar] = option;
	resume();
}

void Scripts::runIfResuming() {
	if (_paused == SCRIPT_RESUMING) {
		_paused = SCRIPT_READY;
		scriptLoop();
	}
}

int Scripts::cmd_sound() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	if (_lValue < 1 || _lValue > 16)
		return 6;
	
	play_sound((int)_lValue - 1, true);
	return 0;
}

int Scripts::cmd_settile() {
	if (!calc_value())
		return 5;
	
	_buffPtr++;
	int screen = (int)_lValue;
	if (!calc_value())
		return 5;
	
	_buffPtr++;
	int pos = (int)_lValue;
	if (!calc_value())
		return 5;
	
	int tile = (int)_lValue;

	if (screen < 0 || screen > 119)
		return 6;
	
	if (pos < 0 || pos > 239)
		return 6;
	
	if (tile < 0 || tile > 230)
		return 6;

	if (screen == _G(current_level)) {
		place_tile(pos % 20, pos / 20, tile);
	} else {
		LEVEL tmp;
		tmp.load(screen);
		tmp.icon[pos / 20][pos % 20] = tile;
		tmp.save(screen);
	}
	return 0;
}

int Scripts::cmd_itemgive() {
	if (!calc_value())
		return 5;

	_buffPtr++;
	int i = (int)_lValue;
	if (i < 1 || i > 15)
		return 6;

	_G(thor_info).inventory |= 64;
	_G(thor_info).item = 7;
	_G(thor_info).object = i;
	_G(thor_info).object_name = OBJECT_NAMES[_G(thor_info).object - 1];

	return 0;
}

int Scripts::cmd_itemtake() {
	delete_object();
	return 0;
}

int Scripts::cmd_setflag() {
	if (!calc_value())
		return 5;

	int i = (int)_lValue;
	if (i < 1 || i > 64)
		return 6;

	_G(setup)._flags[i - 1] = true;

	return 0;
}

int Scripts::cmd_ltoa() {
	int sv;

	if (!calc_value())
		return 5;
	_buffPtr++;

	if (Common::isAlpha(*_buffPtr)) {
		if (*(_buffPtr + 1) == '$') {
			sv = (*_buffPtr) - 65;
			_buffPtr += 2;
		} else {
			return 5;
		}
	} else {
		return 5;
	}

	Common::String str = Common::String::format("%ld", _lValue);
	Common::strcpy_s(_strVar[sv], str.c_str());
	return 0;
}

int Scripts::cmd_pause() {
	if (!calc_value())
		return 5;
	
	_buffPtr++;
	if (_lValue < 1 || _lValue > 65535l)
		return 6;

	Got::pause((int)_lValue);
	return 0;
}

int Scripts::cmd_visible() {
	if (!calc_value())
		return 5;
	
	_buffPtr++;
	if (_lValue < 1 || _lValue > 16)
		return 6;

	actor_visible((int)_lValue);
	return 0;
}

int Scripts::cmd_random() {
	int v;

	if (Common::isAlpha(*_buffPtr)) {
		v = *_buffPtr - 65;
		_buffPtr++;
		if (*_buffPtr != ',')
			return 5;
		
		_buffPtr++;
	} else {
		return 5;
	}

	if (!calc_value())
		return 5;
	_buffPtr++;
	int r = (int)_lValue;
	if (r < 1 || r > 1000)
		return 6;

	_numVar[v] = g_events->getRandomNumber(r - 1);
	return 0;
}

void Scripts::scr_func1() {
	play_sound(FALL, true);

	_G(new_level) = 109;
	_G(new_level_tile) = 215;
	_G(thor)->_x = (_G(new_level_tile) % 20) * 16;
	_G(thor)->_y = ((_G(new_level_tile) / 20) * 16) - 2;

	_G(thor)->_lastX[0] = _G(thor)->_x;
	_G(thor)->_lastX[1] = _G(thor)->_x;
	_G(thor)->_lastY[0] = _G(thor)->_y;
	_G(thor)->_lastY[1] = _G(thor)->_y;
	_G(thor)->_show = 2;
}

void Scripts::scr_func2() {
	int r = g_events->getRandomNumber(5);
	Common::strcpy_s(_strVar[0], 81, OFFENSE[r]);
	Common::strcpy_s(_strVar[1], 81, REASON[r]);
}

void Scripts::scr_func3() {
	int p = (((_G(thor)->_y + 8) / 16) * 20) + ((_G(thor)->_x + 7) / 16);
	int y = p / 20;
	int x = p % 20;

	if (y < 0 || x < 0 || y > 11) {
		play_sound(BRAAPP, true);
		_G(key_flag[key_magic]) = false;
		return;
	}
	if (_G(scrn).icon[y][x] < 174 || _G(scrn).icon[y][x] > 178) {
		play_sound(BRAAPP, true);
		_G(key_flag[key_magic]) = false;
		return;
	}

	_numVar[0] = 1;
	play_sound(WOOP, true);
	if (_G(current_level) == 106 && p == 69) {
		place_tile(x, y, 220);
		_G(key_flag[key_magic]) = false;
		return;
	}

	_G(key_flag[key_magic]) = false;
	place_tile(x, y, 191);

	if ((g_events->getRandomNumber(99)) < 25 ||
		(_G(current_level) == 13 && p == 150 && !_G(setup).f26 && _G(setup).f28)) {
		if (!_G(object_map[p]) && _G(scrn).icon[y][x] >= 140) { // nothing there and solid
			int o = g_events->getRandomNumber(1, 5);
			if (_G(current_level) == 13 && p == 150 && !_G(setup).f26 && _G(setup).f28)
				o = 20;

			_G(object_map[p]) = o;
			_G(object_index[p]) = 31; // actor is 3-15
		}
	}
}

void Scripts::scr_func4() {
	_G(thunder_flag) = 60;
}

void Scripts::scr_func5() {
	_G(scrn).actor_loc[0] -= 2;
	_G(scrn).actor_loc[1] -= 2;
	_G(scrn).actor_loc[2] -= 2;
	_G(scrn).actor_loc[3] -= 2;
	_G(actor[3])._i1 = 16;
}

int Scripts::cmd_exec() {
	if (!calc_value())
		return 5;
	_buffPtr++;
	if (_lValue < 1 || _lValue > 10)
		return 6;

	if (_lValue > 5) {
		error("cmd_exec - unhandled lValue %d", _lValue);
	}
	
	(this->*scr_func[_lValue - 1])();
	return 0;
}

int Scripts::exec_command(int num) {
	char ch;

	int ret = 0;
	switch (num) {
	case 1: // end
		return 0;
	case 2: // goto
		ret = cmd_goto();
		if (!ret)
			_buffPtr = _newPtr;
		break;
	case 3: // gosub
		ret = cmd_goto();
		if (!ret) {
			_gosubPtr++;
			if (_gosubPtr > 31) {
				ret = 10;
				break;
			}
			_gosubStack[_gosubPtr] = _buffPtr;
			_buffPtr = _newPtr;
		}
		break;
	case 4: // return
		if (!_gosubPtr) {
			ret = 9;
			break;
		}
		_buffPtr = _gosubStack[_gosubPtr--];
		break;
	case 5: // for
		_forPtr++;
		if (_forPtr > 10) {
			ret = 10;
			break;
		}
		ch = *_buffPtr;
		if (!Common::isAlpha(ch)) {
			ret = 5;
			break;
		}
		ch -= 65;
		_forVar[_forPtr] = ch;
		_buffPtr += 2;
		if (!calc_value()) {
			ret = 5;
			break;
		}
		_numVar[_forVar[_forPtr]] = _lValue;
		_buffPtr += 2;
		if (!calc_value()) {
			ret = 5;
			break;
		}
		_forVal[_forPtr] = _lValue;
		_forStack[_forPtr] = _buffPtr;
		break;
	case 6: // next
		if (!_forPtr) {
			ret = 11;
			break;
		}
		_numVar[_forVar[_forPtr]] = _numVar[_forVar[_forPtr]] + 1;
		if (_numVar[_forVar[_forPtr]] <= _forVal[_forPtr])
			_buffPtr = _forStack[_forPtr];
		else
			_forPtr--;
		break;
	case 7: // if
		ret = cmd_if();
		break;
	case 8: // else
		while (*_buffPtr != 0)
			_buffPtr++;
		break;
	case 9: // run
		ret = cmd_run();
		if (ret < 0)
			return -100;
		break;
	case 10: // addjewels
		ret = cmd_addjewels();
		break;
	case 11: // addhealth
		ret = cmd_addhealth();
		break;
	case 12: // addmagic
		ret = cmd_addmagic();
		break;
	case 13: // addkeys
		ret = cmd_addkeys();
		break;
	case 14: // addscore
		ret = cmd_addscore();
		break;
	case 15: // say
		ret = cmd_say(0, 1);
		break;
	case 16: // ask
		ret = cmd_ask();
		break;
	case 17: // sound
		ret = cmd_sound();
		break;
	case 18: // settile
		ret = cmd_settile();
		break;
	case 19: // itemgive
		ret = cmd_itemgive();
		break;
	case 20: // itemtake
		ret = cmd_itemtake();
		break;
	case 21: // itemsay
		ret = cmd_say(1, 1);
		break;
	case 22: // setflag
		ret = cmd_setflag();
		break;
	case 23: // ltoa
		ret = cmd_ltoa();
		break;
	case 24: // pause
		ret = cmd_pause();
		break;
	case 25: // text
		ret = cmd_say(0, 0);
		break;
	case 26: // exec
		ret = cmd_exec();
		break;
	case 27: // visible
		ret = cmd_visible();
		break;
	case 28: // random
		ret = cmd_random();
		break;
	default:
		ret = 5;
	}

	if (ret > 0) {
		script_error(ret);
		return 0;
	}

	return 1;
}

} // namespace Got
