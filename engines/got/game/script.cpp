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

#include "common/textconsole.h"
#include "got/game/script.h"
#include "got/game/back.h"
#include "got/game/main.h"
#include "got/game/object.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/utils/file.h"
#include "got/events.h"
#include "got/vars.h"

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
	NULL
};

static const char *INTERNAL_VARIABLE[] = {
	"@JEWELS", "@HEALTH", "@MAGIC", "@SCORE",
	"@SCREEN", "@KEYS",
	"@OW", "@GULP", "@SWISH", "@YAH", "@ELECTRIC",
	"@THUNDER", "@DOOR", "@FALL", "@ANGEL", "@WOOP",
	"@DEAD", "@BRAAPP", "@WIND", "@PUNCH", "@CLANG",
	"@EXPLODE", "@FLAG", "@ITEM", "@THORTILE",
	"@THORPOS",
	NULL
};

static const char *SCR_ERROR[] = {
	"!@#$%", "Out of Memory", "Can't Read Script",
	"Too Many Labels", "No END",
	"Syntax", "Out of Range", "Undefined Label",
	"RETURN Without GOSUB", "Nesting",
	"NEXT Without FOR",
	NULL
};

static const char *OFFENSE[] = {
	"Cussing", "Rebellion", "Kissing Your Mother Goodbye",
	"Being a Thunder God", "Door-to-Door Sales",
	"Carrying a Concealed Hammer"
};

static const char *REASON[] = {
	"We heard you say 'Booger'.",
	"You look kind of rebellious.",
	"Your mother turned you in.",
	"We don't want you here.",
	"Nobody wants your sweepers.",
	"That's a dangerous weapon."
};

Scripts::ScrFunction Scripts::scr_func[5] = {
	&Scripts::scr_func1,
	&Scripts::scr_func2,
	&Scripts::scr_func3,
	&Scripts::scr_func4,
	&Scripts::scr_func5,
};

Scripts *g_scripts;

void execute_script(long index, Gfx::Pics *pic) {
	g_scripts->execute_script(index, pic);
}

Scripts::Scripts() {
	g_scripts = this;
}

Scripts::~Scripts() {
	g_scripts = nullptr;
}

void Scripts::execute_script(long index, Gfx::Pics *pic) {
	// Firstly hide any on-screen actors
	g_events->send(GameMessage("HIDE_ACTORS"));

	scr_index = index;
	scr_pic = pic;

	Common::fill(num_var, num_var + 26, 0);
	Common::fill((char *)str_var, (char *)str_var + 81 * 26, 0);
	runScript();
}

void Scripts::runScript(bool firstTime) {
	int i;

	// Clear line label buffer, line ptrs, and the gosub stack
	Common::fill((char *)line_label, (char *)line_label + 32 * 9, 0);
	Common::fill(line_ptr, line_ptr + 32, (char *)nullptr);
	Common::fill(gosub_stack, gosub_stack + 32, (char *)nullptr);
	gosub_ptr = 0;

	Common::fill(for_var, for_var + 11, 0);
	Common::fill(for_val, for_val + 11, 0);
	Common::fill(for_stack, for_stack + 11, (char *)nullptr);
	for_ptr = 0;

	i = read_script_file();
	if (i != 0) {
		script_error(i);
		script_exit();
		return;
	}

	if (firstTime)
		script_entry();

	buff_ptr = buffer;
	scriptLoop();
}

void Scripts::scriptLoop() {
	int ret;

	while (!_paused) {
		if (_G(cheat) && _G(key_flag)[_B]) break;
		ret = get_command();
		if (ret == -1)
			break;       // Ignore NO END error
		else if (ret == -2) {
			script_error(5);       // Syntax error
			break;
		} else if (ret > 0) {
			ret = exec_command(ret);
			if (ret == -100) {         // RUN command
				if (buffer)
					free(buffer);

				runScript(false);
				return;
			}
			if (!ret)
				break;
		}
	}

	script_exit();
}

void Scripts::script_exit() {
	if (buffer)
		free(buffer);
}

int Scripts::skip_colon() {
	while (*buff_ptr == 0 || *buff_ptr == ':') {
		buff_ptr++;
		if (buff_ptr > buff_end) return 0;
	}

	return 1;
}

int Scripts::get_command() {
	int ret, i, len;

	if (!skip_colon()) return -1;

	i = 0;
	while (1) {
		if (!SCR_COMMAND[i])
			break;           // Lookup command

		len = strlen(SCR_COMMAND[i]);
		if (!strncmp(buff_ptr, (char *)SCR_COMMAND[i], len)) {
			buff_ptr += len;
			return i;
		}

		i++;
	}

	if (Common::isAlpha(*buff_ptr)) {
		if (*(buff_ptr + 1) == '=') {           // Num var assignment
			i = (*buff_ptr) - 65;
			buff_ptr += 2;
			ret = calc_value();
			if (!ret) return -2;
			else {
				num_var[i] = lvalue; return 0;
			}
		} else if (*(buff_ptr + 1) == '$' && *(buff_ptr + 2) == '=') {
			i = (*buff_ptr) - 65;
			buff_ptr += 3;
			ret = calc_string(0);                 // String var assignment
			if (ret == 0) return -2;
			else if (ret == -1) return -3;
			if (strlen(temps) > 80) return -3;
			Common::strcpy_s(str_var[i], temps);
			return 0;
		}
	}

	return -2;
}

int Scripts::calc_string(int mode) {
	// if mode==1 stop at comma
	char varstr[255];
	char varnum;

	Common::strcpy_s(varstr, "");

	if (!skip_colon()) return 0;

strloop:
	if (*buff_ptr == '"') {
		get_str();
		if (strlen(varstr) + strlen(temps) < 255)
			Common::strcat_s(varstr, temps);
		goto nextstr;
	}
	if (Common::isAlpha(*buff_ptr)) {
		if (*(buff_ptr + 1) == '$') {
			varnum = (*buff_ptr) - 65;
			if (strlen(varstr) + strlen(str_var[varnum]) < 255)
				Common::strcat_s(varstr, str_var[varnum]);
			buff_ptr += 2;
			goto nextstr;
		}
	}
	return 0;

nextstr:
	if (*buff_ptr == 0 || *buff_ptr == 58) {
		buff_ptr++;
		goto strdone;
	}
	if (*buff_ptr == ',' && mode == 1) goto strdone;

	if (*buff_ptr == '+') {
		buff_ptr++;
		goto strloop;
	}
	return 0;

strdone:
	if (strlen(varstr) > 255) return -1;
	Common::strcpy_s(temps, (char *) varstr);
	return 1;
}

void Scripts::get_str() {
	int t;

	buff_ptr++;
	t = 0;

	while (1) {
		if (*buff_ptr == '"' || *buff_ptr == 0) {
			temps[t] = 0;
			if (*buff_ptr == '"') buff_ptr++;
			return;
		}

		temps[t++] = *buff_ptr;
		buff_ptr++;
	}
}

int Scripts::calc_value() {
	long tmpval2;
	char exptype;
	char ch;

	tmpval2 = 0;
	exptype = 1;

	while (1) {
		if (!get_next_val()) return 0;
		switch (exptype) {
		case 0:
			tmpval2 = tmpval2 * ltemp;
			break;
		case 1:
			tmpval2 = tmpval2 + ltemp;
			break;
		case 2:
			tmpval2 = tmpval2 - ltemp;
			break;
		case 3:
			if (ltemp != 0) tmpval2 = tmpval2 / ltemp;
			break;
		}

		ch = *buff_ptr;
		switch (ch) {
		case 42:
			exptype = 0;                       /* multiply */
			break;
		case 43:
			exptype = 1;                       /* add */
			break;
		case 45:
			exptype = 2;                       /* minus */
			break;
		case 47:
			exptype = 3;                       /* divide */
			break;
		default:
			lvalue = tmpval2;
			return 1;
		}

		buff_ptr++;
	}
}

int Scripts::get_next_val() {
	char ch;
	char tmpstr[25];
	int t;

	ch = *buff_ptr;
	if (ch == 0 || ch == ':') return 0;
	if (ch == 64) return get_internal_variable();

	if (Common::isAlpha(ch)) {
		buff_ptr++;
		ltemp = num_var[ch - 65];
		return 1;
	}

	t = 0;
	if (strchr("0123456789-", ch)) {
		tmpstr[0] = ch;
		t++;
		buff_ptr++;
		while (strchr("0123456789", *buff_ptr) && *buff_ptr != 0) {
			tmpstr[t] = *buff_ptr;
			buff_ptr++;
			t++;
		}
		tmpstr[t] = 0;
		if (t > 10) return 0;
		ltemp = atol(tmpstr);
		return 1;
	}

	return 0;
}

int Scripts::get_internal_variable() {
	int i, len;
	char b;
	byte *sp;

	i = 0;
	while (1) {
		if (!INTERNAL_VARIABLE[i]) return 0;         // Lookup internal variable
		len = strlen(INTERNAL_VARIABLE[i]);
		if (!strncmp(buff_ptr, INTERNAL_VARIABLE[i], len)) {
			buff_ptr += len;
			break;
		}
		i++;
	}
	switch (i) {
	case 0:
		ltemp = _G(thor_info).jewels;
		break;
	case 1:
		ltemp = _G(thor)->health;
		break;
	case 2:
		ltemp = _G(thor_info).magic;
		break;
	case 3:
		ltemp = _G(thor_info).score;
		break;
	case 4:
		ltemp = _G(current_level);
		break;
	case 5:
		ltemp = _G(thor_info).keys;
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
		ltemp = (long)(i - 5l);
		break;
	case 22:
		if (!calc_value())
			return 0;
		i = (int)lvalue;
		if (i < 1 || i>64)
			return 0;

		sp = (byte *)&_G(setup);
		sp += (i / 8);
		b = 1;
		b = b << (i % 8);
		if (*sp & b)
			ltemp = 1;
		else
			ltemp = 0;
		break;
	case 23:
		if (_G(thor_info).inventory & 64) ltemp = _G(thor_info).object;
		else ltemp = 0;
		break;
	case 24:
		ltemp = _G(scrn).icon[(_G(thor)->y + 8) / 16][(_G(thor)->x + 7) / 16];
		break;
	case 25:
		ltemp = (((_G(thor)->y + 8) / 16) * 20) + ((_G(thor)->x + 7) / 16);
		break;
	default:
		return 0;
	}

	return 1;
}

int Scripts::get_line(char *src, char *dst) {
	int cnt;

	cnt = 0;
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
	src++;

	return cnt;
}

int Scripts::read_script_file() {
	char temp_buff[255];
	char quote_flag;
	int  i, len, p, ret, cnt;
	char ch;
	char *sb;
	char *sbuff = nullptr;
	Common::String str;
	char tmps[255];

	buffer = (char *)malloc(SCR_BUFF_SIZE);
	if (!buffer) {
		ret = 1; goto done;
	};
	buff_ptr = buffer;
	Common::fill(buffer, buffer + SCR_BUFF_SIZE, 0);

	sbuff = (char *)malloc(25000l);
	if (!sbuff) {
		ret = 1; goto done;
	};
	sb = sbuff;

	str = Common::String::format("SPEAK%d", _G(area));
	if (res_read(str.c_str(), sb) < 0) {
		ret = 6; goto done;
	}

	str = Common::String::format("|%d", scr_index);
	Common::strcpy_s(temp_buff, str.c_str());

	while (1) {
		cnt = get_line(sb, (char *)tmps);
		sb += cnt;
		if (!strcmp(tmps, "|EOF")) {
			ret = 2; goto done;
		}
		if (!strcmp(tmps, temp_buff)) break;
	}
	num_labels = 0;
	while (1) {
		cnt = get_line(sb, (char *)tmps);
		if (!strcmp(tmps, "|STOP")) {
			if (buff_ptr != buffer) {
				buff_end = buff_ptr; ret = 0; goto done;
			}
			ret = 2;
			goto done;
		}
		sb += cnt;
		len = strlen(tmps);
		if (len < 2) {
			*buff_ptr = 0;
			buff_ptr++;
			continue;
		}
		quote_flag = 0;
		p = 0;
		for (i = 0; i < len; i++) {
			ch = tmps[i];
			if (ch == 34) quote_flag ^= 1;
			else if (ch == 13 || ch == 10) {  //check for CR
				temp_buff[p] = 0;
				break;
			} else if ((ch == 39 || ch == 96) && !quote_flag) {
				temp_buff[p] = 0;
				break;
			}
			if (!quote_flag) ch = toupper(ch);
			if (quote_flag || ch > 32) {
				temp_buff[p++] = ch;
			}
		}
		temp_buff[p] = 0;

		len = strlen(temp_buff);
		if (len < 10 && temp_buff[len - 1] == ':') {       //line label
			temp_buff[len - 1] = 0;
			line_ptr[num_labels] = buff_ptr;
			Common::strcpy_s(line_label[num_labels++], (char *) temp_buff);
			if (num_labels > 31) {
				ret = 3; goto done;
			}
			*buff_ptr = 0;
			buff_ptr++;
			continue;
		}

		Common::strcpy_s(buff_ptr, SCR_BUFF_SIZE, temp_buff);
		buff_ptr += strlen(temp_buff);
		*buff_ptr = 0;
		buff_ptr++;
	}

done:
	if (sbuff)
		free(sbuff);

	return ret;
}

void Scripts::script_error(int err_num) {
	int line_num;
	char *tb;

	line_num = 1;
	tb = buffer;

	while (1) {
		if (*tb == 0) line_num++;
		if (tb >= buff_ptr) break;
		tb++;
	}

	if (err_num > ERROR_MAX)
		err_num = 5;       // unknown=syntax

	warning("%s in Line #%d", SCR_ERROR[err_num], line_num);
}

int Scripts::cmd_goto() {
	int i, len;
	char s[255];
	char *p;

	Common::strcpy_s(s, buff_ptr);
	p = strchr(s, ':');
	if (p) *p = 0;

	for (i = 0; i < num_labels; i++) {
		len = strlen(s);
		if (len == 0) break;
		if (!strcmp(s, line_label[i])) {
			new_ptr = line_ptr[i];
			buff_ptr += len;
			return 0;
		}
	}
	return 8;
}

int Scripts::cmd_if() {
	long tmpval1, tmpval2;
	char exptype, ch;

	if (!calc_value()) return 5;
	tmpval1 = lvalue;
	exptype = *buff_ptr;
	buff_ptr++;

	ch = *buff_ptr;
	if (ch == 60 || ch == 61 || ch == 62) {
		if (exptype == *buff_ptr) return 5;
		exptype += *buff_ptr;
		buff_ptr++;
	}
	if (!calc_value()) return 5;
	tmpval2 = lvalue;
	buff_ptr += 4;
	switch (exptype) {
	case 60:                              /* less than */
		if (tmpval1 < tmpval2) goto iftrue;
		goto iffalse;
	case 61:                              /* equal */
		if (tmpval1 == tmpval2) goto iftrue;
		goto iffalse;
	case 62:                              /* greater than */
		if (tmpval1 > tmpval2) goto iftrue;
		goto iffalse;
	case 121:                              /* less than or equal */
		if (tmpval1 <= tmpval2) goto iftrue;
		goto iffalse;
	case 122:                              /* less or greater (not equal) */
		if (tmpval1 != tmpval2) goto iftrue;
		goto iffalse;
	case 123:                              /* greater than or equal */
		if (tmpval1 >= tmpval2) goto iftrue;
		goto iffalse;
	default:
		return 5;
	}

iffalse:
	while (*buff_ptr != 0) buff_ptr++;
	while (*buff_ptr == 0) buff_ptr++;

	if (!strncmp(buff_ptr, "ELSE", 4)) buff_ptr += 4;

iftrue:
	return 0;
}

int Scripts::cmd_run() {
	if (!calc_value()) return 5;
	buff_ptr++;
	scr_index = lvalue;
	return -100;
}

int Scripts::cmd_addjewels() {
	if (!calc_value())
		return 5;

	buff_ptr++;

	add_jewels(lvalue);
	return 0;
}

int Scripts::cmd_addhealth() {
	if (!calc_value()) return 5;
	buff_ptr++;
	add_health((int)lvalue);
	return 0;
}

int Scripts::cmd_addmagic() {
	if (!calc_value()) return 5;
	buff_ptr++;
	add_magic((int)lvalue);
	return 0;
}

int Scripts::cmd_addkeys() {

	if (!calc_value()) return 5;
	buff_ptr++;
	add_keys((int)lvalue);
	return 0;
}

int Scripts::cmd_addscore() {

	if (!calc_value()) return 5;
	buff_ptr++;
	add_score((int)lvalue);
	return 0;
}

int Scripts::cmd_say(int mode, int type) {
#ifdef TODO
	char *p;
	int obj;

	if (mode) {
		if (!calc_value()) return 5;
		buff_ptr++;
		obj = (int)lvalue;
		if (obj < 0 || obj>32) return 6;
		if (obj) obj += 10;
	} else obj = 0;

	Common::fill(_G(tmp_buff), _G(tmp_buff) + TMP_SIZE, 0);
	p = (char *)_G(tmp_buff);

	while (calc_string(0)) {
		Common::strcpy_s(p, TMP_SIZE, temps);
		p += strlen(temps);
		*(p) = 10;
		p++;
	}
	*(p - 1) = 0;

	display_speech(obj, (char *)scr_pic, type);
	d_restore();
	return 0;
#else
	error("TODO: cmd_say");
#endif
}

int Scripts::cmd_ask() {
#ifdef TODO
	int i, v, p;
	char title[41];
	char *op[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
	char opts[10][41];

	memset(_G(tmp_buff), 0, TMP_SIZE);
	memset(opts, 0, 10 * 41);

	if (!skip_colon()) return 5;

	if (Common::isAlpha(*buff_ptr)) {
		v = *buff_ptr - 65;
		buff_ptr++;
		if (*buff_ptr != ',') return 5;
		buff_ptr++;
	} else return 5;

	if (!calc_string(1)) return 5;

	strncpy(title, temps, 41);
	title[40] = 0;

	if (*buff_ptr == ',') {
		buff_ptr++;
		if (!calc_value()) return 5;
		buff_ptr++;
		p = (int)lvalue;
	} else return 5;

	i = 0;
	while (calc_string(0)) {
		strncpy((char *) opts[i], temps, 41);
		opts[i][40] = 0;
		op[i] = opts[i];
		i++;
		if (i > 9) return 3;
	}
	if (p > i) p = 0;
	num_var[v] = select_option(op, title, p - 1);
	d_restore();
	return 0;
#else
	error("TODO: cmd_ask");
#endif
}

int Scripts::cmd_sound() {
	if (!calc_value()) return 5;
	buff_ptr++;
	if (lvalue < 1 || lvalue>16) return 6;
	play_sound((int)lvalue - 1, 1);
	return 0;
}

int Scripts::cmd_settile() {
	int screen, pos, tile;
	LEVEL *lvl;

	if (!calc_value()) return 5;
	buff_ptr++;
	screen = (int)lvalue;
	if (!calc_value()) return 5;
	buff_ptr++;
	pos = (int)lvalue;
	if (!calc_value()) return 5;
	tile = (int)lvalue;
	if (screen < 0 || screen>119) return 6;
	if (pos < 0 || pos>239) return 6;
	if (tile < 0 || tile>230) return 6;
	if (screen == _G(current_level)) {
		place_tile(pos % 20, pos / 20, tile);
	} else {
		lvl = (LEVEL *) (_G(sd_data) + (screen * 512));
		lvl->icon[pos / 20][pos % 20] = tile;
	}
	return 0;
}

int Scripts::cmd_itemgive() {
#ifdef TODO
	int i;

	if (!calc_value()) return 5;
	buff_ptr++;
	i = (int)lvalue;
	if (i < 1 || i > 15) return 6;

	_G(thor_info).inventory |= 64;
	_G(thor_info).item = 7;
	_G(thor_info).object = i;
	display_item();
	_G(thor_info).object_name = object_names[_G(thor_info).object - 1];
	return 0;
#else
	error("TODO: cmd_itemgive");
#endif
}

int Scripts::cmd_itemtake() {
	delete_object();
	return 0;
}

int Scripts::cmd_setflag() {
	int i;
	char b;
	byte *sp;

	if (!calc_value())
		return 5;
	i = (int)lvalue;
	if (i < 1 || i>64)
		return 6;

	sp = (byte *)&_G(setup);
	sp += (i / 8);
	b = 1;
	b = b << (i % 8);
	*sp |= b;
	return 0;
}

int Scripts::cmd_ltoa() {
	int sv;
	char str[21];

	if (!calc_value()) return 5;
	buff_ptr++;

	if (Common::isAlpha(*buff_ptr)) {
		if (*(buff_ptr + 1) == '$') {
			sv = (*buff_ptr) - 65;
			buff_ptr += 2;
		} else return 5;
	} else return 5;

	ltoa(lvalue, str, 10);
	Common::strcpy_s(str_var[sv], (char *) str);
	return 0;
}

int Scripts::cmd_pause() {
	if (!calc_value())
		return 5;
	buff_ptr++;
	if (lvalue < 1 || lvalue>65535l)
		return 6;

	Got::pause((int)lvalue);
	return 0;
}

int Scripts::cmd_visible() {
	if (!calc_value()) return 5;
	buff_ptr++;
	if (lvalue < 1 || lvalue>16) return 6;

	actor_visible((int)lvalue);
	return 0;
}

int Scripts::cmd_random() {
	int v, r;

	if (Common::isAlpha(*buff_ptr)) {
		v = *buff_ptr - 65;
		buff_ptr++;
		if (*buff_ptr != ',') return 5;
		buff_ptr++;
	} else return 5;

	if (!calc_value()) return 5;
	buff_ptr++;
	r = (int)lvalue;
	if (r < 1 || r>1000) return 6;

	num_var[v] = g_events->getRandomNumber(r - 1);
	return 0;
}

void Scripts::scr_func1() {
	play_sound(FALL, 1);
	if (_G(key_flag)[_FOUR]) return;
	_G(new_level) = 109;
	_G(new_level_tile) = 215;
	_G(thor)->x = (_G(new_level_tile) % 20) * 16;
	_G(thor)->y = ((_G(new_level_tile) / 20) * 16) - 2;

	_G(thor)->last_x[0] = _G(thor)->x;
	_G(thor)->last_x[1] = _G(thor)->x;
	_G(thor)->last_y[0] = _G(thor)->y;
	_G(thor)->last_y[1] = _G(thor)->y;
	_G(thor)->show = 2;
}

void Scripts::scr_func2() {
	int r;

	r = g_events->getRandomNumber(5);
	Common::strcpy_s(str_var[0], 81, OFFENSE[r]);
	Common::strcpy_s(str_var[1], 81, REASON[r]);
}

void Scripts::scr_func3() {
	int p, x, y, o;

	p = (((_G(thor)->y + 8) / 16) * 20) + ((_G(thor)->x + 7) / 16);
	y = p / 20;
	x = p % 20;
	if (y < 0 || x < 0 || y>11 || x>19) {
		play_sound(BRAAPP, 1);
#if 0
		while (_G(key_flag)[key_magic])
			rotate_pal();
#endif
		return;
	}
	if (_G(scrn).icon[y][x] < 174 || _G(scrn).icon[y][x]>178) {
		play_sound(BRAAPP, 1);
#if 0
		while (_G(key_flag)[key_magic]) rotate_pal();
#endif
		return;
	}
	num_var[0] = 1;
	play_sound(WOOP, 1);
	if (_G(current_level) == 106 && p == 69) {
		place_tile(x, y, 220);
#if 0
		while (_G(key_flag)[key_magic]) rotate_pal();
#endif
		return;
	}
#if 0
	while (_G(key_flag)[key_magic]) rotate_pal();
#endif
	place_tile(x, y, 191);
	if ((g_events->getRandomNumber(99)) < 25 ||
		(_G(current_level) == 13 && p == 150 && !_G(setup).f26 && _G(setup).f28)) {
		if (!_G(object_map)[p] && _G(scrn).icon[y][x] >= 140) {  // nothing there and solid
			o = g_events->getRandomNumber(1, 5);
			if (_G(current_level) == 13 && p == 150 && !_G(setup).f26 && _G(setup).f28)
				o = 20;

			_G(object_map)[p] = o;
			_G(object_index)[p] = 31;  //actor is 3-15
			x = (p % 20) * 16;
			y = (p / 20) * 16;
#ifdef TODO
			xfput(x, y, PAGE2, (char *) objects[o - 1]);
			xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, draw_page, 320, 320);
			xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, display_page, 320, 320);
			pause(30);
#else
			error("TODO: display/wait");
#endif
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
	_G(actor)[3].i1 = 16;
}

int Scripts::cmd_exec() {
	if (!calc_value()) return 5;
	buff_ptr++;
	if (lvalue < 1 || lvalue>10)
		return 6;

	(this->*scr_func[lvalue - 1])();
	return 0;
}

int Scripts::exec_command(int num) {
	int ret;
	char ch;

	ret = 0;
	switch (num) {
	case 1:                          // end
		return 0;
	case 2:                          // goto
		ret = cmd_goto();
		if (!ret) buff_ptr = new_ptr;
		break;
	case 3:                          // gosub
		ret = cmd_goto();
		if (!ret) {
			gosub_ptr++;
			if (gosub_ptr > 31) {
				ret = 10; break;
			}
			gosub_stack[gosub_ptr] = buff_ptr;
			buff_ptr = new_ptr;
		}
		break;
	case 4:                         //return
		if (!gosub_ptr) {
			ret = 9; break;
		}
		buff_ptr = gosub_stack[gosub_ptr--];
		break;
	case 5:                         //for
		for_ptr++;
		if (for_ptr > 10) {
			ret = 10; break;
		}
		ch = *buff_ptr;
		if (!Common::isAlpha(ch)) {
			ret = 5; break;
		}
		ch -= 65;
		for_var[for_ptr] = ch;
		buff_ptr += 2;
		if (!calc_value()) {
			ret = 5; break;
		}
		num_var[for_var[for_ptr]] = lvalue;
		buff_ptr += 2;
		if (!calc_value()) {
			ret = 5; break;
		}
		for_val[for_ptr] = lvalue;
		for_stack[for_ptr] = buff_ptr;
		break;
	case 6:                         //next
		if (!for_ptr) {
			ret = 11; break;
		}
		num_var[for_var[for_ptr]] = num_var[for_var[for_ptr]] + 1;
		if (num_var[for_var[for_ptr]] <= for_val[for_ptr])
			buff_ptr = for_stack[for_ptr];
		else for_ptr--;
		break;
	case 7:                         // if
		ret = cmd_if();
		break;
	case 8:                         // else
		while (*buff_ptr != 0) buff_ptr++;
		break;
	case 9:                         // run
		ret = cmd_run();
		if (ret < 0) return -100;
		break;
	case 10:                        // addjewels
		ret = cmd_addjewels();
		break;
	case 11:                        // addhealth
		ret = cmd_addhealth();
		break;
	case 12:                        // addmagic
		ret = cmd_addmagic();
		break;
	case 13:                        // addkeys
		ret = cmd_addkeys();
		break;
	case 14:                        // addscore
		ret = cmd_addscore();
		break;
	case 15:                        // say
		ret = cmd_say(0, 1);
		break;
	case 16:                        // ask
		ret = cmd_ask();
		break;
	case 17:                        // sound
		ret = cmd_sound();
		break;
	case 18:                        // settile
		ret = cmd_settile();
		break;
	case 19:                        // itemgive
		ret = cmd_itemgive();
		break;
	case 20:                        // itemtake
		ret = cmd_itemtake();
		break;
	case 21:                        // itemsay
		ret = cmd_say(1, 1);
		break;
	case 22:                        // setflag
		ret = cmd_setflag();
		break;
	case 23:                        // ltoa
		ret = cmd_ltoa();
		break;
	case 24:                        // pause
		ret = cmd_pause();
		break;
	case 25:                        // text
		ret = cmd_say(0, 0);
		break;
	case 26:                        // exec
		ret = cmd_exec();
		break;
	case 27:                        // visible
		ret = cmd_visible();
		break;
	case 28:                        // random
		ret = cmd_random();
		break;
	default:
		ret = 5;
	}
	if (ret > 0) {
		script_error(ret); return 0;
	}
	return 1;
}

} // namespace Got
