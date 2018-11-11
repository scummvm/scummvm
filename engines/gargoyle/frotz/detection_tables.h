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

#include "engines/advancedDetector.h"
#include "common/language.h"

namespace Gargoyle {
namespace Frotz {

/**
 * Game descriptor for ZCode games
 */
struct FrotzGameDescription {
	const char *const _gameId;
	const char *const _description;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
	const char *const _guiOptions;
};

extern const FrotzGameDescription FROTZ_GAMES[];
extern const char *const AMFV;
extern const char *const ARTHUR;
extern const char *const BALLYHOO;
extern const char *const BEYONDZORK;
extern const char *const BORDERZONE;
extern const char *const BUREAUCRACY;
extern const char *const CUTTHROATS;
extern const char *const DEADLINE;
extern const char *const ENCHANTER;
extern const char *const HHGTTG;
extern const char *const HIJINX;
extern const char *const INFIDEL;
extern const char *const JOURNEY;
extern const char *const LGOP;
extern const char *const LGOP2;
extern const char *const LURKING;
extern const char *const MINIZORK1;
extern const char *const MOONMIST;
extern const char *const NORDBERT;
extern const char *const PLANETFALL;
extern const char *const PLUNDERED;
extern const char *const SAMPLER1;
extern const char *const SAMPLER2;
extern const char *const SEASTALKER;
extern const char *const SHERLOCKRIDDLE;
extern const char *const SHOGUN;
extern const char *const SORCERER;
extern const char *const SPELLBREAKER;
extern const char *const STARCROSS;
extern const char *const STATIONFALL;
extern const char *const SUSPECT;
extern const char *const SUSPENDED;
extern const char *const TRINITY;
extern const char *const WISHBRINGER;
extern const char *const WITNESS;
extern const char *const ZORK0;
extern const char *const ZORK1;
extern const char *const ZORK2;
extern const char *const ZORK3;
extern const char *const ZTUU;

} // End of namespace Frotz
} // End of namespace Gargoyle
