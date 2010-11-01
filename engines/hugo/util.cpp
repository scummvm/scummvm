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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"
#include "gui/message.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/util.h"
#include "hugo/sound.h"

namespace Hugo {

/**
 * Returns index (0 to 7) of first 1 in supplied byte, or 8 if not found
 */
int Utils::firstBit(byte data) {
	if (!data)
		return 8;

	int i;
	for (i = 0; i < 8; i++) {
		if ((data << i) & 0x80)
			break;
	}

	return i;
}

/**
 * Returns index (0 to 7) of last 1 in supplied byte, or 8 if not found
 */
int Utils::lastBit(byte data) {
	if (!data)
		return 8;

	int i;
	for (i = 7; i >= 0; i--) {
		if ((data << i) & 0x80)
			break;
	}

	return i;
}

/**
 * Reverse the bit order in supplied byte
 */
void Utils::reverseByte(byte *data) {
	byte maskIn = 0x80;
	byte maskOut = 0x01;
	byte result = 0;

	for (byte i = 0; i < 8; i++, maskIn >>= 1, maskOut <<= 1) {
		if (*data & maskIn)
			result |= maskOut;
	}

	*data = result;
}

char *Utils::Box(box_t dismiss, const char *s, ...) {
	static char buffer[MAX_STRLEN + 1];             // Format text into this

	if (!s)
		return 0;                                   // NULL strings catered for

	if (s[0] == '\0')
		return 0;

	if (strlen(s) > MAX_STRLEN - 100) {             // Test length
		warning("String too big: '%s'", s);
		return 0;
	}

	va_list marker;
	va_start(marker, s);
	vsprintf(buffer, s, marker);                    // Format string into buffer
	va_end(marker);

	if (buffer[0] == '\0')
		return 0;

	switch(dismiss) {
	case BOX_ANY:
	case BOX_OK: {
		GUI::MessageDialog dialog(buffer, "OK");
		dialog.runModal();
		break;
		}
	case BOX_YESNO: {
		GUI::MessageDialog dialog(buffer, "YES", "NO");
		if (dialog.runModal() == GUI::kMessageOK)
			return buffer;
		return 0;
		break;
		}
	case BOX_PROMPT:
		warning("Box: unhandled BOX_PROMPT");
		int boxTime = strlen(buffer) * 30;
		GUI::TimedMessageDialog dialog(buffer, MAX(1500, boxTime));
		dialog.runModal();
	// TODO: Some boxes (i.e. the combination code for the shed), needs to return an input.
	}

	return buffer;
}

/**
 * Fatal error handler.  Reset environment, print error and exit
 * Arguments are same as printf
 */
void Utils::Error(int error_type, const char *format, ...) {
	char buffer[ERRLEN + 1];
	bool fatal = true;                              // Fatal error, else continue

	switch (error_type) {
	case FILE_ERR:
		strcpy(buffer, HugoEngine::get()._textUtil[kErr1]);
		break;
	case WRITE_ERR:
		strcpy(buffer, HugoEngine::get()._textUtil[kErr2]);
		fatal = false;                              // Allow continuation
		break;
	case PCCH_ERR:
		strcpy(buffer, HugoEngine::get()._textUtil[kErr3]);
		break;
	case HEAP_ERR:
		strcpy(buffer, HugoEngine::get()._textUtil[kErr4]);
		break;
	case SOUND_ERR:
		strcpy(buffer, HugoEngine::get()._textUtil[kErr5]);
		break;
	default:
		strcpy(buffer, HugoEngine::get()._textUtil[kErr6]);
		break;
	}

	if (fatal)
		HugoEngine::get().shutdown();                                   // Restore any devices before exit

	va_list marker;
	va_start(marker, format);
	vsnprintf(&buffer[strlen(buffer)], ERRLEN - strlen(buffer), format, marker);
	va_end(marker);
	//MessageBeep(MB_ICONEXCLAMATION);
	//MessageBox(hwnd, buffer, "HugoWin Error", MB_OK | MB_ICONEXCLAMATION);
	warning("Hugo Error: %s", buffer);

	if (fatal)
		exit(1);
}

/**
 * Print options for user when dead
 */
void Utils::gameOverMsg(void) {
	//MessageBox(hwnd, gameoverstring, "Be more careful next time!", MB_OK | MB_ICONINFORMATION);
	warning("STUB: Gameover_msg(): %s", HugoEngine::get()._textUtil[kGameOver]);
}

char *Utils::strlwr(char *buffer) {
	char *result = buffer;

	while (*buffer != '\0') {
		if (isupper(*buffer))
			*buffer = tolower(*buffer);
		buffer++;
	}

	return result;
}

} // End of namespace Hugo
