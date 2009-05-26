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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_SFX_SFX_PLAYER_REALTIME_H
#define SCI_SFX_SFX_PLAYER_REALTIME_H

#include "sci/sfx/player.h"

namespace Sci {

class RealtimePlayer : public SfxPlayer {
public:
	RealtimePlayer();

	virtual Common::Error init(ResourceManager *resmgr, int expected_latency);
	virtual Common::Error add_iterator(SongIterator *it, uint32 start_time);
	virtual Common::Error stop();
	virtual Common::Error iterator_message(const SongIterator::Message &msg);
	virtual Common::Error pause();
	virtual Common::Error resume();
	virtual Common::Error exit();
	virtual void maintenance();
	virtual void tell_synth(int buf_nr, byte *buf);
};

} // End of namespace Sci

#endif
