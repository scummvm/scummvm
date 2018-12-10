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

#include "glk/tads/tads2/tads2.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

TADS2::TADS2(OSystem *syst, const GlkGameDescription &gameDesc) : OS(syst, gameDesc) {
	cmap_init_default();
}

void TADS2::runGame(Common::SeekableReadStream *gameFile) {
	errcxdef errctx;
	errctx.errcxlgc = &errctx;
	errctx.errcxfp = nullptr;
	errctx.errcxofs = 0;
	errctx.errcxappctx = this;

	/* copyright-date-string */
#ifdef T2_COPYRIGHT_NOTICE
	trdptf("%s - A %s TADS %s Interpreter.\n",
		G_tads_oem_app_name, G_tads_oem_display_mode,
		TADS_RUNTIME_VERSION);
	trdptf("%sopyright (c) 1993, 2012 by Michael J. Roberts.\n",
		G_tads_oem_copyright_prefix ? "TADS c" : "C");
	trdptf("%s\n", G_tads_oem_author);
#endif

	trdmain1(&errctx);

	// pause before exiting if the OS desires it
	os_expause();
}

void TADS2::trdmain1(errcxdef *errctx) {

}

void TADS2::trdptf(const char *fmt, ...) {
	va_list va;

	// format the string */
	va_start(va, fmt);
	Common::String msg = Common::String::vformat(fmt, va);
	va_end(va);

	// print the formatted buffer
	os_printz(msg);
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
