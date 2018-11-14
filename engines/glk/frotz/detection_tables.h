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
extern const char *const AMFV_DESC;
extern const char *const ARTHUR_DESC;
extern const char *const BALLYHOO_DESC;
extern const char *const BEYONDZORK_DESC;
extern const char *const BORDERZONE_DESC;
extern const char *const BUREAUCRACY_DESC;
extern const char *const CUTTHROATS_DESC;
extern const char *const DEADLINE_DESC;
extern const char *const ENCHANTER_DESC;
extern const char *const HHGTTG_DESC;
extern const char *const HIJINX_DESC;
extern const char *const INFIDEL_DESC;
extern const char *const JOURNEY_DESC;
extern const char *const LGOP_DESC;
extern const char *const LGOP2_DESC;
extern const char *const LURKING_DESC;
extern const char *const MINIZORK1_DESC;
extern const char *const MOONMIST_DESC;
extern const char *const NORDBERT_DESC;
extern const char *const PLANETFALL_DESC;
extern const char *const PLUNDERED_DESC;
extern const char *const SAMPLER1_DESC;
extern const char *const SAMPLER2_DESC;
extern const char *const SEASTALKER_DESC;
extern const char *const SHERLOCKRIDDLE_DESC;
extern const char *const SHOGUN_DESC;
extern const char *const SORCERER_DESC;
extern const char *const SPELLBREAKER_DESC;
extern const char *const STARCROSS_DESC;
extern const char *const STATIONFALL_DESC;
extern const char *const SUSPECT_DESC;
extern const char *const SUSPENDED_DESC;
extern const char *const TRINITY_DESC;
extern const char *const WISHBRINGER_DESC;
extern const char *const WITNESS_DESC;
extern const char *const ZORK0_DESC;
extern const char *const ZORK1_DESC;
extern const char *const ZORK2_DESC;
extern const char *const ZORK3_DESC;
extern const char *const ZTUU_DESC;

} // End of namespace Frotz
} // End of namespace Gargoyle
