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

#ifndef AGOS_OBSOLETE_H
#define AGOS_OBSOLETE_H

static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"simon1acorn", "simon1", Common::kPlatformAcorn},
	{"simon1amiga", "simon1", Common::kPlatformAmiga},
	{"simon1cd32", "simon1", Common::kPlatformAmiga},
	{"simon1demo", "simon1", Common::kPlatformDOS},
	{"simon1dos", "simon1", Common::kPlatformDOS},
	{"simon1talkie", "simon1", Common::kPlatformDOS},
	{"simon1win", "simon1", Common::kPlatformWindows},
	{"simon2dos", "simon2", Common::kPlatformDOS},
	{"simon2talkie", "simon2", Common::kPlatformDOS},
	{"simon2mac", "simon2", Common::kPlatformMacintosh},
	{"simon2win", "simon2", Common::kPlatformWindows},
	{0, 0, Common::kPlatformUnknown}
};

#endif // AGOS_OBSOLETE_H
