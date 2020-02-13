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

#ifndef ULTIMA_ULTIMA1_GFX_VIEW_CHAR_GEN_H
#define ULTIMA_ULTIMA1_GFX_VIEW_CHAR_GEN_H

#include "ultima/shared/gfx/visual_container.h"
#include "ultima/shared/gfx/text_input.h"
#include "ultima/shared/core/party.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

#define ATTRIBUTE_COUNT 6

using Shared::CKeypressMsg;
using Shared::CShowMsg;
using Shared::CHideMsg;
using Shared::CTextInputMsg;

/**
 * This class implements the character generation view
 */
class ViewCharacterGeneration : public Shared::Gfx::VisualItem {
	DECLARE_MESSAGE_MAP;
	bool KeypressMsg(CKeypressMsg &msg);
	bool ShowMsg(CShowMsg &msg);
	bool HideMsg(CHideMsg &msg);
	bool TextInputMsg(CTextInputMsg &msg);
private:
	enum Flag {
		FLAG_FRAME = 1, FLAG_POINTS = 2, FLAG_ATTRIBUTES = 4, FLAG_ATTR_POINTERS = 8, FLAG_HELP = 16,
		FLAG_RACE = 32, FLAG_SEX = 64, FLAG_CLASS = 128, FLAG_NAME = 256, FLAG_SAVE = 512,
		FLAG_INITIAL = FLAG_FRAME | FLAG_POINTS | FLAG_ATTRIBUTES | FLAG_ATTR_POINTERS | FLAG_HELP
	};
	uint _flags;
	Shared::Character *_character;
	int _pointsRemaining;
	int _selectedAttribute;
	uint *_attributes[ATTRIBUTE_COUNT];
	Shared::Gfx::TextInput *_textInput;
private:
	/**
	 * Set state within the view
	 */
	void setMode(uint flags);

	/**
	 * Draw the outer frame for the view
	 */
	void drawFrame(Shared::Gfx::VisualSurface &s);

	/**
	 * Draw points remaining
	 */
	void drawPointsRemaining(Shared::Gfx::VisualSurface &s);

	/**
	 * Draw the attribute list
	 */
	void drawAttributes(Shared::Gfx::VisualSurface &s);

	/**
	 * Draw the pointers to the currently selected attribute
	 */
	void drawAttributePointers(Shared::Gfx::VisualSurface &s);

	/**
	 * Draw the help text
	 */
	void drawHelp(Shared::Gfx::VisualSurface &s);

	/**
	 * Draw the race selection
	 */
	void drawRace(Shared::Gfx::VisualSurface &s);

	/**
	 * Draw the sex selection
	 */
	 void drawSex(Shared::Gfx::VisualSurface &s);

	 /**
	  * Draw the class selection
	  */
	 void drawClass(Shared::Gfx::VisualSurface &s);

	 /**
	  * Draw the name entry
	  */
	 void drawName(Shared::Gfx::VisualSurface &s);

	 /**
	  * Draw the save prompt
	  */
	 void drawSave(Shared::Gfx::VisualSurface &s);

	 /**
	  * Set the character's race
	  */
	 void setRace(int raceNum);

	 /**
	  * Set the character's sex
	  */
	 void setSex(int sexNum);

	 /**
	  * Set the character's class
	  */
	 void setClass(int classNum);

	 /**
	  * Set final properties and save the game
	  */
	 bool save();
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	ViewCharacterGeneration(Shared::TreeItem *parent = nullptr);
	
	/**
	 * Destructor
	 */
	~ViewCharacterGeneration() override;

	/**
	 * Draw the game screen
	 */
	void draw() override;
};

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
