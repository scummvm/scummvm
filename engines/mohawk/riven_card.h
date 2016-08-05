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

#ifndef RIVEN_CARD_H
#define RIVEN_CARD_H

#include "mohawk/riven_scripts.h"
#include "mohawk/riven_sound.h"

#include "common/rect.h"
#include "common/system.h"

namespace Mohawk {

/**
 * A game view
 *
 * The names Card and Stack are legacy from the HyperCard engine used in
 * the original mac version of Myst.
 *
 * Cards contain hotspots, scripts, and resource lists.
 */
class RivenCard {
public:
	RivenCard(MohawkEngine_Riven *vm, uint16 id);
	~RivenCard();

	/**
	 * An image that can be drawn in this card
	 */
	struct Picture {
		uint16 index;
		uint16 id;
		Common::Rect rect;
	};

	/** Initialization routine used to draw a card for the first time or to refresh it */
	void open();

	/** Run one of the card's scripts */
	void runScript(uint16 scriptType);

	/** Get the id of the card in the stack */
	uint16 getId() const;

	/** Get the card's picture with the specified index */
	Picture getPicture(uint16 index) const;

	/** Draw one of the card's pictures synchronously or asynchronously */
	void drawPicture(uint16 index, bool queue = false);

	/** Play the card's ambient sounds with the specified index */
	void playSound(uint16 index, bool queue = false);

	/** Get the card's sound description with the specified index */
	SLSTRecord getSound(uint16 index) const;

private:
	void loadCardResource(uint16 id);
	void loadCardPictureList(uint16 id);
	void loadCardSoundList(uint16 id);

	void initializeZipMode();
	void defaultLoadScript();

	MohawkEngine_Riven *_vm;

	// General card data
	uint16 _id;
	int16 _name;
	uint16 _zipModePlace;
	RivenScriptList _scripts;

	// Resource lists
	Common::Array<Picture> _pictureList;
	Common::Array<SLSTRecord> _soundList;
};

/**
 * A Card Hotspot
 *
 * Hotspots are named rectangular areas of the view.
 * Hotspots can be interactive through their scripts.
 */
class RivenHotspot {
public:
	RivenHotspot(MohawkEngine_Riven *vm, Common::ReadStream *stream);

	/** Run one of the hotspot's scripts */
	void runScript(uint16 scriptType);

	uint16 blstID;
	int16 name_resource;
	uint16 index;
	Common::Rect rect;
	uint16 mouse_cursor;
	int16 zipModeHotspot;

	bool enabled; // TODO: Remove

private:
	void loadFromStream(Common::ReadStream *stream);

	MohawkEngine_Riven *_vm;

	uint16 _u0;
	int16 _u1;
	RivenScriptList _scripts;
};

} // End of namespace Mohawk

#endif
