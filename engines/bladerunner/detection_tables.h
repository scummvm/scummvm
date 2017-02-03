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

#ifndef BLADERUNNER_DETECTION_TABLES_H
#define BLADERUNNER_DETECTION_TABLES_H

namespace BladeRunner {

static const ADGameDescription gameDescriptions[] = {
	// BladeRunner
	{
		"bladerunner",
		0,
		{
			{"STARTUP.MIX", 0, "5643b53306ca7764cf1ec7b79c9630a3", 2312374},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	
	// BladeRunner (German)
	{
		"bladerunner",
		0,
		{
			{"STARTUP.MIX", 0, "57d674ed860148a530b7f4957cbe65ec", 2314301},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

} // End of namespace BladeRunner

#endif
