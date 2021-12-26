/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA8_GUMPS_CRUCREDITSGUMP_H
#define ULTIMA8_GUMPS_CRUCREDITSGUMP_H

#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class RenderedText;

/**
 * Full-screen gump for the credits roll in Crusader: No Remorse
 */
class CruCreditsGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	CruCreditsGump();
	CruCreditsGump(Common::SeekableReadStream *txtrs, Common::SeekableReadStream *bmprs,
	            uint32 flags = FLAG_PREVENT_SAVE, int32 layer = LAYER_MODAL);
	~CruCreditsGump() override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;

	void Close(bool no_del = false) override;

	void run() override;

	// Paint the Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;

protected:
	enum CredLineType {
		kCredTitle,
		kCredName
	};

	struct CredLine {
		Common::String _text;
		enum CredLineType _lineType;
	};

	struct CredScreen {
		//! The lines of text for this screen
		Common::Array<CredLine> _lines;
		//! How long to display this screen, in engine ticks
		unsigned int _delay;
	};

	//! Number of clock ticks the gump has run
	int _timer;
	//! Clock tick where the next screen should be shown
	int _nextScreenStart;
	//! Current screen number
	int _screenNo;

	//! Pre-rendered text
	Common::Array<RenderedText *> _currentLines;

	//! The starry background picture
	RenderSurface *_background;

	//! Screen text data
	Common::Array<CredScreen> _screens;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
