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

#ifndef COMMON_RENDERMODE_H
#define COMMON_RENDERMODE_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_rendermode Render modes
 * @ingroup common
 *
 * @brief API for render modes.
 *
 * @{
 */

class String;

/**
 * List of render modes. It specifies which original graphics mode
 * to use. Some targets used postprocessing dithering routines for
 * reducing color depth of final image which let it to be rendered on
 * such low-level adapters as CGA or Hercules.
 *
 * If you are adding an option here, do not forget to create relevant GUIO
 */
enum RenderMode {
	kRenderDefault = 0,
	kRenderVGA = 1,
	kRenderEGA = 2,
	kRenderCGA = 3,
	kRenderHercG = 4,
	kRenderHercA = 5,
	kRenderAmiga = 6,
	kRenderFMTowns = 7,
	kRenderPC98_256c = 8,
	kRenderPC98_16c = 9,
	kRenderApple2GS = 10,
	kRenderAtariST = 11,
	kRenderMacintosh = 12,
	kRenderMacintoshBW = 13,
	kRenderCGAComp = 14,
	kRenderCGA_BW = 15,
	kRenderCPC = 16,
	kRenderZX = 17,
	kRenderC64 = 18,
	kRenderVGAGrey = 19,
	kRenderPC98_8c = 20,
	kRenderWin256c = 21,
	kRenderWin16c = 22
};

struct RenderModeDescription {
	const char *code;
	const char *description;
	RenderMode id;
};

extern const RenderModeDescription g_renderModes[];

/** Convert a string containing a render mode name into a RenderingMode enum value. */
extern RenderMode parseRenderMode(const String &str);
extern const char *getRenderModeCode(RenderMode id);
extern const char *getRenderModeDescription(RenderMode id);

// TODO: Rename the following to something better; also, document it
extern String renderMode2GUIO(RenderMode id);

// TODO: Rename the following to something better; also, document it
extern String allRenderModesGUIOs();

/** @} */

} // End of namespace Common

#endif
