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

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

struct Texture;

class PentagramMenuGump : public ModalGump {
	class PentagramMenuCallbackProcess : public Process {
		Std::string _game;
	public:
		PentagramMenuCallbackProcess(ObjId id, const Std::string &game) : Process(id), _game(game) {
			_flags |= PROC_RUNPAUSED;
		}

		void run() override {
			pout << "Gump returned: " << _result << Std::endl;
			PentagramMenuGump *menu = p_dynamic_cast<PentagramMenuGump *>(ObjectManager::get_instance()->getObject(getItemNum()));
			if (menu) menu->ProcessCallback(_game, _result);
			terminate();
		}
	};

public:
	ENABLE_RUNTIME_CLASSTYPE()

	PentagramMenuGump(int x, int y, int w, int h);
	~PentagramMenuGump() override;

	void InitGump(Gump *newparent, bool take_focus = true) override;

	void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;
	void PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled) override;

	void ChildNotify(Gump *child, uint32 message) override;

	bool OnKeyDown(int key, int mod) override;

	void run() override;

#if 0
	virtual uint16 TraceObjId(int32 mx, int32 my);

	virtual bool StartDraggingChild(Gump *gump, int32 mx, int32 my);
	virtual void DraggingChild(Gump *gump, int mx, int my);
	virtual void StopDraggingChild(Gump *gump);

	virtual Gump *OnMouseDown(int button, int32 mx, int32 my);
	virtual void OnMouseUp(int button, int32 mx, int32 my);
	virtual void OnMouseClick(int button, int32 mx, int32 my);
	virtual void OnMouseDouble(int button, int32 mx, int32 my);

	virtual void RenderSurfaceChanged();
#endif

	Texture *getCovers() const {
		return _coversImage;
	}
	Texture *getFlags() const {
		return _flagsImage;
	}

private:
	int _gameScrollPos;
	int _gameScrollTarget;
	int _gameScrollLastDelta;

	int _gameCount;

	Texture *_titleImage;
	Texture *_navbarImage;
	Texture *_coversImage;
	Texture *_flagsImage;

	void ProcessCallback(Std::string gamename, int message);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
