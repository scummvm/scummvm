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

#include "lead_actor.h"
#include "../walk/walk_mgr.h"
#include "../cursor_mgr.h"
#include "engines/pink/sequences/sequencer.h"
#include "../archive.h"
#include "../page.h"
#include "../pink.h"

namespace Pink {

void LeadActor::deserialize(Archive &archive) {
    Actor::deserialize(archive);
    _cursorMgr = static_cast<CursorMgr*>(archive.readObject());
    _walkMgr = static_cast<WalkMgr*>(archive.readObject());
    _sequencer = static_cast<Sequencer*>(archive.readObject());
}

void LeadActor::setNextExecutors(Common::String &nextModule, Common::String &nextPage) {
    if (_state == Ready || _state == Moving || _state == inDialog1 || _state == Inventory || _state == PDA) {
        _state = PlayingVideo;
        _page->getGame()->setNextExecutors(nextModule, nextPage);
    }
}

void LeadActor::init(bool unk) {
    if (_state == unk_Loading){
        _state = Ready;
    }
    //TODO set actor ref to inv mgr
    Actor::init(unk);
}

} // End of namespace Pink
