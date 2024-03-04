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
#include "bagel/api/smacker.h"

namespace Bagel {

Smack *SmackOpen(const char *filename, int param1, int param2) {
	error("TODO: SmackOpen");
}

void SmackToBuffer(Smack *Smack, uint32 Unknown1, uint32 Unknown2,
	uint32 Stride, uint32 FrameHeightInPixels, char *OutBuffer, uint32 Flags) {
	error("TODO: SmackToBuffer");
}

void SmackDoFrame(Smack *smack) {
	error("TODO: SmackDoFrame");
}

void SmackNextFrame(Smack *smack) {
	error("TODO: SmackNextFrame");
}

} // namespace Bagel
