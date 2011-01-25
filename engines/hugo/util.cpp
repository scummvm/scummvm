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
#include "hugo/text.h"

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
	static char buffer[kMaxStrLength + 1];          // Format text into this

	if (!s)
		return 0;                                   // NULL strings catered for

	if (s[0] == '\0')
		return 0;

	if (strlen(s) > kMaxStrLength - 100) {          // Test length
		warning("String too long: '%s'", s);
		return 0;
	}

	va_list marker;
	va_start(marker, s);
	vsprintf(buffer, s, marker);                    // Format string into buffer
	va_end(marker);

	if (buffer[0] == '\0')
		return 0;

	switch(dismiss) {
	case kBoxAny:
	case kBoxOk: {
		GUI::MessageDialog dialog(buffer, "OK");
		dialog.runModal();
		break;
		}
	case kBoxYesNo: {
		GUI::MessageDialog dialog(buffer, "YES", "NO");
		if (dialog.runModal() == GUI::kMessageOK)
			return buffer;
		return 0;
		break;
		}
	case kBoxPrompt: {
		// TODO: Some boxes (i.e. the combination code for the shed), needs to return an input.
		warning("Box: unhandled BOX_PROMPT");
		int boxTime = strlen(buffer) * 30;
		GUI::TimedMessageDialog dialog(buffer, MAX(1500, boxTime));
		dialog.runModal();
		break;
		}
	default:
		error("Unknown BOX Type %d", dismiss);
	}

	return buffer;
}

/**
 * Print options for user when dead
 */
void Utils::gameOverMsg(void) {
	Utils::Box(kBoxOk, "%s", HugoEngine::get()._text->getTextUtil(kGameOver));
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
