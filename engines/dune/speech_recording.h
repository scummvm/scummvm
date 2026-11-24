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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DUNE_SPEECH_RECORDING_H
#define DUNE_SPEECH_RECORDING_H

#include "common/scummsys.h"

namespace Dune {

/** A6EE/A6F1/A701: choose the recording before A8BC formats its filename. */
inline uint16 speechRecordingNumber(uint16 request, uint16 speakerBase,
		uint16 eventBase, bool report, bool overlay) {
	const uint16 line = request & ~0x0c00;
	if (report)
		return (uint16)(line - eventBase + 999);
	return overlay ? line : (uint16)(line - speakerBase);
}

} // End of namespace Dune

#endif
