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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#ifdef INCLUDE_OLDGFX
#include "sci/gfx/gfx_gui.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxVisual
#endif
#include "sci/gfx/menubar.h"
#include "sci/gui/gui.h"
#include "sci/gui/gui_cursor.h"

namespace Sci {

reg_t kAddMenu(EngineState *s, int argc, reg_t *argv) {
	Common::String title = s->_segMan->getString(argv[0]);
	Common::String content = s->_segMan->getString(argv[1]);

	s->_gui->menuAdd(title, content, argv[1]);
	return s->r_acc;
}


reg_t kSetMenu(EngineState *s, int argc, reg_t *argv) {
	uint16 menuId = argv[0].toUint16() >> 8;
	uint16 itemId = argv[0].toUint16() & 0xFF;
	uint16 attributeId;
	int argPos = 1;

	while (argPos < argc) {
		attributeId = argv[argPos].toUint16();
		if ((argPos + 1) >= argc)
			error("Too few parameters for kSetMenu");
		s->_gui->menuSet(menuId, itemId, attributeId, argv[argPos + 1]);
		argPos += 2;
	}
	return s->r_acc;
}

reg_t kGetMenu(EngineState *s, int argc, reg_t *argv) {
	uint16 menuId = argv[0].toUint16() >> 8;
	uint16 itemId = argv[0].toUint16() & 0xFF;
	uint16 attributeId = argv[1].toUint16();

	return s->_gui->menuGet(menuId, itemId, attributeId);
}


reg_t kDrawStatus(EngineState *s, int argc, reg_t *argv) {
	reg_t textReference = argv[0];
	Common::String text;
	int16 colorPen = (argc > 1) ? argv[1].toSint16() : 0;
	int16 colorBack = (argc > 2) ? argv[2].toSint16() : s->resMan->isVGA() ? 255 : 15;

	if (!textReference.isNull()) {
		// Sometimes this is called without giving text, if thats the case dont process it
		text = s->_segMan->getString(textReference);

		s->_gui->drawStatus(s->strSplit(text.c_str(), NULL).c_str(), colorPen, colorBack);
	}
	return s->r_acc;
}

reg_t kDrawMenuBar(EngineState *s, int argc, reg_t *argv) {
	bool clear = argv[0].isNull() ? true : false;

	s->_gui->drawMenuBar(clear);
	return s->r_acc;
}

reg_t kMenuSelect(EngineState *s, int argc, reg_t *argv) {
	reg_t eventObject = argv[0];
	//bool pauseSound = argc > 1 ? (argv[1].isNull() ? false : true) : false;

	// TODO: pauseSound implementation
	return s->_gui->menuSelect(eventObject);
}

} // End of namespace Sci
