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

#ifndef HODJNPODJ_LIBS_TYPES_H
#define HODJNPODJ_LIBS_TYPES_H

#include "common/rect.h"
#include "common/serializer.h"
#include "bagel/mfc/mfc_types.h"

namespace Bagel {
namespace HodjNPodj {

struct GAMESTRUCT {
	long lCrowns = 0;
	long lScore = 0;
	int nSkillLevel = 0;
	bool bSoundEffectsEnabled = false;
	bool bMusicEnabled = false;
	bool bPlayingMetagame = false;
	bool bPlayingHodj = false;
};
typedef GAMESTRUCT *LPGAMESTRUCT;

} // namespace HodjNPodj
} // namespace Bagel

#endif
