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

#ifndef ULTIMA_ULTIMA1_GFX_VIEW_MAP_H
#define ULTIMA_ULTIMA1_GFX_VIEW_MAP_H

#include "ultima/shared/gfx/visual_container.h"
#include "ultima/shared/gfx/bitmap.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

using Shared::CShowMsg;
using Shared::CKeypressMsg;
using Shared::CFrameMsg;

/**
 * This class implements the title screen, which shows the copyright screens and Ultima castle screen,
 * as well as the main menu
 */
class ViewTitle : public Shared::Gfx::VisualItem {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
	bool KeypressMsg(CKeypressMsg &msg);
	bool FrameMsg(CFrameMsg &msg);
private:
	Graphics::ManagedSurface _logo, _castle;
	Graphics::ManagedSurface _flags[3];
	enum TitleMode { TITLEMODE_COPYRIGHT, TITLEMODE_PRESENTS, TITLEMODE_CASTLE, TITLEMODE_TRADEMARKS, TITLEMODE_MAIN_MENU };
	TitleMode _mode;
	uint32 _expiryTime;
	int _counter;
private:
	/**
	 * Shows the initial copyright screen
	 */
	void drawCopyrightView();

	/**
	 * Draws the presents view
	 */
	void drawPresentsView();

	/**
	 * Draws the castle view
	 */
	void drawCastleView();

	/**
	 * Animates the castle flags
	 */
	void drawCastleFlag(Shared::Gfx::VisualSurface &s, int xp);

	/**
	 * Draws the trademarks view
	 */
	void drawTrademarksView();

	/**
	 * Draws the main menu
	 */
	void drawMainMenu();

	/**
	 * Sets up the palette for the castle view
	 */
	void setCastlePalette();

	/**
	 * Sets up the palette for the title views
	 */
	void setTitlePalette();

	/**
	 * Sets the current mode (display) within the title
	 */
	void setMode(TitleMode mode);
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	ViewTitle(Shared::TreeItem *parent = nullptr);

	/**
	 * Draw the game screen
	 */
	void draw() override;
};

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
