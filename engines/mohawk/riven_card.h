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

class RivenHotspot;

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

	/** Draw borders for all the hotspots in the card */
	void drawHotspotRects();

	/** Enable the zip hotspots if they match to already visited locations */
	void initializeZipMode();

	/** Get the hotspot containing the specified point */
	RivenHotspot *getHotspotContainingPoint(const Common::Point &point) const;

	/** Get the hotspot with the specified name */
	RivenHotspot *getHotspotByName(const Common::String &name) const;

	/** Get the hotspot with the specified BLST id */
	RivenHotspot *getHotspotByBlstId(const uint16 blstId) const;

	/** Get all the hotspots in the card. To be used for debugging features only */
	Common::Array<RivenHotspot *> getHotspots() const;

	/** Activate a hotspot using a hotspot enable list entry */
	void activateHotspotEnableRecord(uint16 index);

private:
	void loadCardResource(uint16 id);
	void loadHotspots(uint16 id);
	void loadCardPictureList(uint16 id);
	void loadCardSoundList(uint16 id);
	void loadCardHotspotEnableList(uint16 id);

	void defaultLoadScript();

	struct HotspotEnableRecord {
		uint16 index;
		uint16 enabled;
		uint16 hotspotId;
	};

	MohawkEngine_Riven *_vm;

	// General card data
	uint16 _id;
	int16 _name;
	uint16 _zipModePlace;
	RivenScriptList _scripts;

	Common::Array<RivenHotspot *> _hotspots;

	// Resource lists
	Common::Array<Picture> _pictureList;
	Common::Array<SLSTRecord> _soundList;
	Common::Array<HotspotEnableRecord> _hotspotEnableList;
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

	/** Enable or disable the hotspot */
	void enable(bool e);

	/** Can the hotspot be interacted with? */
	bool isEnabled() const;

	/** Is the hotspot's purpose to zip to another card */
	bool isZip() const;

	/** Get the hotspot'a rect in Card coordinates */
	Common::Rect getRect() const;

	/** Does the hotspot contain the specified point? */
	bool containsPoint(const Common::Point &point) const;

	/** Override the hotspot's default rect */
	void setRect(const Common::Rect &rect);

	/** Get the default mouse cursor id to be used when hovering the hostpot */
	uint16 getMouseCursor() const;

	/** Get the hotspot's name from the current stack's name list */
	Common::String getName() const;

	/** Get the hotspot's name id */
	int16 getNameId() const;

	/** Get the hotspot's order in the view */
	uint16 getIndex() const;

	/** Get the hotspot's enable list id */
	uint16 getBlstId() const;

private:
	enum {
		kFlagZip = 1,
		kFlagEnabled = 2
	};

	void loadFromStream(Common::ReadStream *stream);

	MohawkEngine_Riven *_vm;

	uint16 _blstID;
	int16 _nameResource;
	Common::Rect _rect;
	uint16 _u0;
	uint16 _mouseCursor;
	uint16 _index;
	int16 _u1;
	uint16 _flags;
	RivenScriptList _scripts;
};

} // End of namespace Mohawk

#endif
