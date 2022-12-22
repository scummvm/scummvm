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

#ifndef SWORD1_OBSOLETE_H
#define SWORD1_OBSOLETE_H

static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"sword1demo", "sword1", Common::kPlatformWindows},
	{"sword1mac", "sword1", Common::kPlatformMacintosh},
	{"sword1macdemo", "sword1", Common::kPlatformMacintosh},
	{"sword1psx", "sword1", Common::kPlatformPSX},
	{"sword1psxdemo", "sword1", Common::kPlatformPSX},
	{0, 0, Common::kPlatformUnknown}
};

#endif // SWORD1_OBSOLETE_H
