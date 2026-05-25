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

#ifndef EEM_DETECTION_H
#define EEM_DETECTION_H

#include "engines/advancedDetector.h"

namespace EEM {

#define GAMEOPTION_HIDE_HIGHLIGHT_BOXES   GUIO_GAMEOPTIONS1
#define GAMEOPTION_FIT_DIALOG_BALLOONS    GUIO_GAMEOPTIONS2
#define GAMEOPTION_SKIP_REPEATED_CASES    GUIO_GAMEOPTIONS3
#define GAMEOPTION_RESTORED_CONTENT       GUIO_GAMEOPTIONS4

enum EEMDebugChannels {
	kDebugGeneral = 1 << 0,
	kDebugScript  = 1 << 1,
	kDebugMystery = 1 << 2,
	kDebugSite    = 1 << 3,
	kDebugGfx     = 1 << 4,
	kDebugSound   = 1 << 5
};

extern const ADGameDescription gameDescriptions[];

} // End of namespace EEM

#endif
