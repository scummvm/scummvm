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

#ifndef ULTIMA8_GUMPS_PENTAGRAMMENUGUMP_H
#define ULTIMA8_GUMPS_PENTAGRAMMENUGUMP_H

#include "ultima8/gumps/modal_gump.h"
#include "ultima8/kernel/process.h"
#include "ultima8/kernel/object_manager.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

struct Texture;

class PentagramMenuGump : public ModalGump {
	class PentagramMenuCallbackProcess : public Process {
		std::string game;
	public:
		PentagramMenuCallbackProcess(ObjId id, std::string game_) : Process(id), game(game_) {
			flags |= PROC_RUNPAUSED;
		}

		virtual void run() {
			pout << "Gump returned: " << result << std::endl;
			PentagramMenuGump *menu = p_dynamic_cast<PentagramMenuGump *>(ObjectManager::get_instance()->getObject(getItemNum()));
			if (menu) menu->ProcessCallback(game, result);
			terminate();
		}
	};

public:
	ENABLE_RUNTIME_CLASSTYPE()

	PentagramMenuGump(int x, int y, int w, int h);
	virtual ~PentagramMenuGump();

	virtual void InitGump(Gump *newparent, bool take_focus = true);

	virtual void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled);
	virtual void PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled);

	virtual void ChildNotify(Gump *child, uint32 message);

	virtual bool OnKeyDown(int key, int mod);

	virtual void run();

#if 0
	virtual uint16 TraceObjId(int mx, int my);

	virtual bool StartDraggingChild(Gump *gump, int mx, int my);
	virtual void DraggingChild(Gump *gump, int mx, int my);
	virtual void StopDraggingChild(Gump *gump);

	virtual Gump *OnMouseDown(int button, int mx, int my);
	virtual void OnMouseUp(int button, int mx, int my);
	virtual void OnMouseClick(int button, int mx, int my);
	virtual void OnMouseDouble(int button, int mx, int my);

	virtual void RenderSurfaceChanged();
#endif

	Texture *getCovers() const {
		return coversImage;
	}
	Texture *getFlags() const {
		return flagsImage;
	}

private:

	int gameScrollPos;
	int gameScrollTarget;
	int gameScrollLastDelta;

	int gamecount;

	Texture *titleImage;
	Texture *navbarImage;
	Texture *coversImage;
	Texture *flagsImage;

	void ProcessCallback(std::string gamename, int message);

};

} // End of namespace Ultima8

#endif
