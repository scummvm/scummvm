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

#include "ac/dynobj/scriptaudioclip.h"
#include "util/stream.h"

using namespace AGS::Common;

void ScriptAudioClip::ReadFromFile(Stream *in) {
	id = in->ReadInt32();
	scriptName.ReadCount(in, SCRIPTAUDIOCLIP_SCRIPTNAMELENGTH);
	fileName.ReadCount(in, SCRIPTAUDIOCLIP_FILENAMELENGTH);
	bundlingType = in->ReadInt8();
	type = in->ReadInt8();
	fileType = in->ReadInt8();
	defaultRepeat = in->ReadInt8();
	defaultPriority = in->ReadInt16();
	defaultVolume = in->ReadInt16();
	in->ReadInt32(); // reserved
}
