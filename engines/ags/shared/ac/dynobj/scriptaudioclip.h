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

#ifndef AGS_SHARED_AC_DYNOBJ_SCRIPTAUDIOCLIP_H
#define AGS_SHARED_AC_DYNOBJ_SCRIPTAUDIOCLIP_H

#include "ags/shared/util/string.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

enum AudioFileType {
	eAudioFileOGG = 1,
	eAudioFileMP3 = 2,
	eAudioFileWAV = 3,
	eAudioFileVOC = 4,
	eAudioFileMIDI = 5,
	eAudioFileMOD = 6
};

#define AUCL_BUNDLE_EXE 1
#define AUCL_BUNDLE_VOX 2

#define SCRIPTAUDIOCLIP_SCRIPTNAMELENGTH    30
#define SCRIPTAUDIOCLIP_FILENAMELENGTH      15
struct ScriptAudioClip {
	int id = 0;
	Common::String scriptName;
	Common::String fileName;
	char bundlingType = AUCL_BUNDLE_EXE;
	char type = 0;
	char fileType = eAudioFileOGG;
	char defaultRepeat = 0;
	short defaultPriority = 50;
	short defaultVolume = 100;

	void ReadFromFile(Common::Stream *in);
};

} // namespace AGS3

#endif
