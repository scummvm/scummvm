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

#ifndef TITANIC_PARROT_LOBBY_LINK_UPDATER_H
#define TITANIC_PARROT_LOBBY_LINK_UPDATER_H

#include "common/stream.h"
#include "titanic/game/parrot/parrot_lobby_object.h"

namespace Titanic {

struct LinkUpdatorEntry {
	CString _linkStr;
	int _vals[8];

	LinkUpdatorEntry();
	void load(Common::SeekableReadStream *s);
};

class LinkUpdatorEntries : public Common::Array<LinkUpdatorEntry> {
public:
	void load(Common::SeekableReadStream *s, int count);
};

class CParrotLobbyLinkUpdater : public CParrotLobbyObject {
	DECLARE_MESSAGE_MAP;
	bool ActMsg(CActMsg *msg);
private:
	LinkUpdatorEntries _entries[5];
public:
	int _fieldBC;
public:
	CLASSDEF;
	CParrotLobbyLinkUpdater();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_PARROT_LOBBY_LINK_UPDATER_H */
