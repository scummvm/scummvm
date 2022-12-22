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

#ifndef GRIM_DETECTION_H
#define GRIM_DETECTION_H

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"grimdemo", "grim", Common::kPlatformWindows},
	{nullptr, nullptr, Common::kPlatformUnknown}
};

namespace Grim {

enum GrimGameType {
	GType_GRIM,
	GType_MONKEY4
};

struct GrimGameDescription {
	ADGameDescription desc;
	GrimGameType gameType;
};

#define GAMEOPTION_LOAD_DATAUSR GUIO_GAMEOPTIONS1
#define GAMEOPTION_SHOW_FPS GUIO_GAMEOPTIONS2

#define GUI_OPTIONS_GRIME GUIO2(GAMEOPTION_LOAD_DATAUSR, GAMEOPTION_SHOW_FPS)

} // End of namespace Grim

#endif // GRIM_DETECTION_H
