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

#ifndef ULTIMA8_GUMPS_CREDITSGUMP_H
#define ULTIMA8_GUMPS_CREDITSGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderedText;

/**
 * Full-screen gump for the credits roll in U8
 */
class CreditsGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	CreditsGump();
	CreditsGump(const Std::string &text, int parskip = 24,
	            uint32 flags = 0, int32 layer = LAYER_MODAL);
	~CreditsGump() override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	// Set a configuration option to true when user watches entire sequence
	void SetFlagWhenFinished(Std::string configKey) {
		_configKey = configKey;
	}

	void Close(bool no_del = false) override;

	void run() override;

	// Paint the Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;

protected:

	void extractLine(Std::string &text, char &modifier, Std::string &line);

	Std::string _text;
	int _parSkip;

	enum CreditsState {
		CS_PLAYING,
		CS_FINISHING,
		CS_CLOSING
	} _state;

	int _timer;

	RenderedText *_title;
	RenderedText *_nextTitle;
	int _nextTitleSurf;

	RenderSurface *_scroll[4];
	int _scrollHeight[4];
	int _currentSurface;
	int _currentY;

	Std::string _configKey;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
