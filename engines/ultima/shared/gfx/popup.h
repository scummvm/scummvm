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

#ifndef ULTIMA_SHARED_GFX_POPUP_H
#define ULTIMA_SHARED_GFX_POPUP_H

#include "ultima/shared/gfx/visual_item.h"

namespace Ultima {
namespace Shared {

class GameBase;

namespace Gfx {

/**
 * Base class for graphic elements that "pop up" on top of existing views. This includes things like
 * dialogs, text input, etc.
 */
class Popup : public VisualItem {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
protected:
	GameBase *_game;
	VisualItem *_parentView;
	TreeItem *_respondTo;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Popup(GameBase *game);

	/**
	 * Destructor
	 */
	~Popup() override;

	/**
	 * Show the popup
	 * @param respondTo		Element to send any response to when the popup closes.
	 *						If not provided, any response goes to the active view
	 */
	void show(TreeItem *respondTo = nullptr);

	/**
	 * Hide the popup
	 */
	virtual void hide();
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
