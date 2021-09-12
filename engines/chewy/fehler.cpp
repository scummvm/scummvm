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

#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "common/system.h"
#include "chewy/fehler.h"
#include "chewy/ngstypes.h"

fehler *err;

fehler::fehler() {
	language = ENGL;

	memset(err_dat, 0, sizeof(err_dat));
	memset(err_txt, 0, sizeof(err_txt));

	modul = 0;
	user_msg = nullptr;
}

fehler::~fehler() {
}

void fehler::msg() {
	bool ok = true;

	if (language == GERMAN) {
		switch (modul) {
		case 1:
			warning(" Meldung des Grafikmoduls:");
			break;

		case 2:
			warning("Meldung des Speichermoduls:");
			break;

		case 3:
			warning("Meldung des Dateimoduls:");
			break;

		case 4:
			warning("Meldung des Mausmoduls:");
			break;

		case 5:
			warning("Meldung des Dialogmoduls:");
			break;

		default:
			warning("Modul %d und Meldung %d sind mir nicht bekannt", modul, fcode);
			ok = false;
			break;
		}
	}
	else {
		switch (modul) {
		case 1:
			warning("GRAPHICS ERROR:");
			break;

		case 2:
			warning("ERROR FROM MEMORY MANAGER:");
			break;

		case 3:
			warning("ERROR FROM FILER:");
			break;

		case 4:
			warning("ERROR FROM MOUSE-HANDLER:");
			break;

		case 5:
			warning("ERROR FROM DILOGUE:");
			break;

		default:
			warning("MODUKE %d AND MESSAGE %d NOT KNOWN", modul, fcode);
			ok = false;
			break;
		}
	}
	if (ok) {
		warning("%s", err_txt);
		if (user_msg) {
			warning("%s", user_msg);
			user_msg = nullptr;
		}
	}
}

void fehler::set_user_msg(const char *msg) {
	user_msg = msg;
}
