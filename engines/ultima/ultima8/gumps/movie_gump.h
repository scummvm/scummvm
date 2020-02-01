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

#ifndef ULTIMA8_GUMPS_MOVIEGUMP_H
#define ULTIMA8_GUMPS_MOVIEGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RawArchive;
class SKFPlayer;

class MovieGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	MovieGump();
	MovieGump(int width, int height, RawArchive *skf, bool introMusicHack = false,
	          uint32 _Flags = 0, int32 layer = LAYER_MODAL);
	virtual ~MovieGump(void);

	virtual void InitGump(Gump *newparent, bool take_focus = true);

	virtual void Close(bool no_del = false);

	virtual void run();

	// Paint the Gump
	virtual void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled);

	virtual bool OnKeyDown(int key, int mod);

	static ProcId U8MovieViewer(RawArchive *skf, bool introMusicHack = false);

	//! "play" console command
	static void ConCmd_play(const Console::ArgvType &argv);

	bool loadData(IDataSource *ids);
protected:
	virtual void saveData(ODataSource *ods);

	SKFPlayer *player;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
