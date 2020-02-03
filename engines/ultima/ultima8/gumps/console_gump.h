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

#ifndef ULTIMA8_GUMPS_CONSOLEGUMP_H
#define ULTIMA8_GUMPS_CONSOLEGUMP_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ConsoleGump : public Gump {
	enum ConsoleScrollState {
		NORMAL_DISPLAY,
		WAITING_TO_HIDE,
		SCROLLING_TO_HIDE,
		NOTIFY_OVERLAY,
		WAITING_TO_SHOW,
		SCROLLING_TO_SHOW
	};

	ConsoleScrollState  scroll_state;
	uint32 scroll_frame;

public:
	ENABLE_RUNTIME_CLASSTYPE()

	ConsoleGump();
	ConsoleGump(int x, int y, int w, int h);
	virtual ~ConsoleGump();

	void ToggleConsole();
	void ShowConsole();
	void HideConsole();
	bool ConsoleIsVisible();

	// ConsoleGump doesn't have any 'effective' area
	virtual bool PointOnGump(int mx, int my) override {
		return false;
	}

	virtual void run() override;

	virtual void PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;

	virtual void RenderSurfaceChanged() override;

	virtual bool        OnTextInput(int unicode) override;
	virtual void        OnFocus(bool /*gain*/) override;
	virtual bool        OnKeyDown(int key, int mod) override;

	static void         ConCmd_toggle(const Console::ArgvType &argv);   //!< "ConsoleGump::toggle" console command

	bool    loadData(IDataSource *ids, uint32 version);

protected:
	virtual void saveData(ODataSource *ods) override;

private:
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
