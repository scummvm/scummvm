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

#include "titanic/game/parrot/parrot_lobby_link_updater.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotLobbyLinkUpdater, CParrotLobbyObject)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

/*------------------------------------------------------------------------*/

LinkUpdatorEntry::LinkUpdatorEntry() {
	Common::fill(&_vals[0], &_vals[8], 0);
}

void LinkUpdatorEntry::load(Common::SeekableReadStream *s) {
	_linkStr = readStringFromStream(s);
	for (int idx = 0; idx < 8; ++idx)
		_vals[idx] = s->readByte();
}

/*------------------------------------------------------------------------*/

void LinkUpdatorEntries::load(Common::SeekableReadStream *s, int count) {
	resize(count);
	for (int idx = 0; idx < count; ++idx)
		(*this)[idx].load(s);
}

/*------------------------------------------------------------------------*/

CParrotLobbyLinkUpdater::CParrotLobbyLinkUpdater() : CParrotLobbyObject(), _fieldBC(1) {
	Common::SeekableReadStream *s = g_vm->_filesManager->getResource("DATA/PARROT_LOBBY_LINK_UPDATOR");
	_entries[0].load(s, 7);
	_entries[1].load(s, 5);
	_entries[2].load(s, 6);
	_entries[3].load(s, 9);
	_entries[4].load(s, 1);
	delete s;
}

void CParrotLobbyLinkUpdater::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CParrotLobbyObject::save(file, indent);
}

void CParrotLobbyLinkUpdater::load(SimpleFile *file) {
	file->readNumber();
	CParrotLobbyObject::load(file);
}

bool CParrotLobbyLinkUpdater::ActMsg(CActMsg *msg) {
	if (msg->_action != "Refresh")
		return false;

	CNodeItem *node = findNode();
	LinkUpdatorEntries *entriesP;
	if (isEquals("ParrotLobbyUpdater_TOW")) {
		entriesP = &_entries[4];
	} else {
		if (node->_nodeNumber < 1 || node->_nodeNumber > 4)
			return true;
		entriesP = &_entries[node->_nodeNumber - 1];
	}
	int count = entriesP->size();

	for (CTreeItem *item = node->getFirstChild(); item; item = item->scan(node)) {
		CLinkItem *link = dynamic_cast<CLinkItem *>(item);
		if (!link || count == 0)
			continue;

		CString linkName = link->getName();
		char c = linkName.lastChar();
		if (c >= 'a' && c <= 'd')
			linkName.deleteLastChar();

		for (uint idx = 0; idx < entriesP->size(); ++idx) {
			const LinkUpdatorEntry &entry = (*entriesP)[idx];
			if (entry._linkStr == linkName) {
				int val = entry._vals[CParrotLobbyObject::_flags];
				if (val)
					linkName += (char)(0x60 + val);

				link->_name = linkName;
				break;
			}
		}
	}

	return true;
}

} // End of namespace Titanic
