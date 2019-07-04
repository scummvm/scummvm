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

#include "glk/alan3/alan3.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/acode.h"

namespace Glk {
namespace Alan3 {

winid_t glkMainWin;
winid_t glkStatusWin;

void glkio_printf(const char *fmt, ...) {
	// If there's a savegame being loaded from the launcher, ignore any text out
	if (g_vm->_saveSlot != -1)
		return;

	va_list argp;
	va_start(argp, fmt);
	if (glkMainWin) {
		char buf[1024]; /* FIXME: buf size should be foolproof */
		vsprintf(buf, fmt, argp);
		g_vm->glk_put_string(buf);
	} else {
		// assume stdio is available in this case only
		Common::String str = Common::String::vformat(fmt, argp);
		warning(fmt, argp);
	}

	va_end(argp);
}

void showImage(int image, int align) {
	uint ecode;

	if ((g_vm->glk_gestalt(gestalt_Graphics, 0) == 1) &&
		(g_vm->glk_gestalt(gestalt_DrawImage, wintype_TextBuffer) == 1)) {
		g_vm->glk_window_flow_break(glkMainWin);
		printf("\n");
		ecode = g_vm->glk_image_draw(glkMainWin, image, imagealign_MarginLeft, 0);
		(void)ecode;
	}
}

void playSound(int sound) {
#ifdef GLK_MODULE_SOUND
	static schanid_t soundChannel = NULL;

	if (g_vm->glk_gestalt(gestalt_Sound, 0) == 1) {
		if (soundChannel == NULL)
			soundChannel = g_vm->glk_schannel_create(0);
		if (soundChannel != NULL) {
			g_vm->glk_schannel_stop(soundChannel);
			(void)g_vm->glk_schannel_play(soundChannel, sound);
		}
	}
#endif
}

void setStyle(int style) {
	switch (style) {
	case NORMAL_STYLE:
		g_vm->glk_set_style(style_Normal);
		break;
	case EMPHASIZED_STYLE:
		g_vm->glk_set_style(style_Emphasized);
		break;
	case PREFORMATTED_STYLE:
		g_vm->glk_set_style(style_Preformatted);
		break;
	case ALERT_STYLE:
		g_vm->glk_set_style(style_Alert);
		break;
	case QUOTE_STYLE:
		g_vm->glk_set_style(style_BlockQuote);
		break;
	}
}

} // End of namespace Alan3
} // End of namespace Glk
