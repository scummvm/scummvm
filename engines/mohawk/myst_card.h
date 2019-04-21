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

#ifndef MYST_CARD_H
#define MYST_CARD_H

#include "common/rect.h"

#include "mohawk/myst.h"

namespace Mohawk {

/**
 * A game view
 *
 * The names Card and Stack are legacy from the HyperCard engine used in
 * the original mac version.
 *
 * Cards contain resources (hotspots), images, sounds, and cursor hints.
 */
class MystCard {
public:
	MystCard(MohawkEngine_Myst *vm, uint16 id);
	~MystCard();

	/** Get the id of the card */
	uint16 getId() const;

	/** Initialization routine used to draw a card for the first time */
	void enter();

	/** Run the card's leave scripts */
	void leave();

	/** Get a card resource (hotspot) by its index in the resource list */
	template<class T>
	T *getResource(uint index);

	/** The list of resources in the card */
	Common::Array<MystArea *> _resources;

	/** Enable or disable a card resource */
	void setResourceEnabled(uint16 resourceIndex, bool enable);

	/** Update the card's active resource according to the mouse position */
	void updateActiveResource(const Common::Point &mouse);

	/** Set the card's currently clicked resource to the currently active resource */
	MystArea *forceUpdateClickedResource(const Common::Point &mouse);

	/** Clear the card's currently clicked resource
	 *
	 * Used to indicate the mouse up event handler should not be called.
	 * (Possibly because it was already called by the gameplay logic
	 */
	void resetClickedResource();

	/**
	 * Get the mouse cursor that should be used when hovering the currently active resource
	 *
	 * -1 means that no specific cursor is defined for the active resource, or that there is no
	 * currently active resource. In that case the default main cursor should be used by the caller.
	 */
	int16 getActiveResourceCursor();

	/**
	 * Call the resource event handlers to account for the new specified input
	 *
	 * For example call the mouse down event handler for the currently active resource
	 * if the mouse is clicked and there was no clicked resource previously.
	 */
	void updateResourcesForInput(const Common::Point &mouse, bool mouseClicked, bool mouseMoved);

	/** Is there a currently clicked resource */
	bool isDraggingResource() const;

	/** Retrieve the id of the background image to use when drawing the card */
	uint16 getBackgroundImageId();

	/** Draw the card's background image to the backbuffer */
	void drawBackground();

	/** Draw the card's image resources to the backbuffer */
	void drawResourceImages();

	/** Draw debug rectangles around the card's resources */
	void drawResourceRects();

	/** Redraw the card's resources that are affected by the specified variable */
	void redrawArea(uint16 var, bool updateScreen = true);

private:
	// View flags
	enum {
		kMystZipDestination = (1 << 0)
	};

	struct MystCursorHint {
		uint16 id;
		int16 cursor;

		MystCondition variableHint;
	};

	MohawkEngine_Myst *_vm;

	// General card data
	uint16 _id;
	uint16 _flags;

	// Image Data
	Common::Array<MystCondition> _conditionalImages;
	uint16 _mainImage;

	// Sound Data
	MystSoundBlock _soundBlock;

	// Script Resources
	enum ScriptResourceType {
		kResourceImage = 1,
		kResourceSound = 2,
		kResourceSwitch = 3,
		kResourceImageNoCache = 4,
		kResourceSoundNoCache = 5
	};

	struct ScriptResource {
		ScriptResourceType type;
		uint16 id;
		uint16 switchVar;
		ScriptResourceType switchResourceType;
		Common::Array<int16> switchResourceIds;
	};
	Common::Array<ScriptResource> _scriptResources;

	uint16 _resourceListId;
	uint16 _hintResourceId;
	uint16 _initScriptId;
	uint16 _exitScriptId;

	Common::Array<MystCursorHint> _cursorHints;

	/** Area of type kMystAreaHover being hovered by the mouse, if any */
	MystAreaHover *_hoverResource;

	/** Active area being hovered by the mouse, if any */
	MystArea *_activeResource;

	/** Active area being clicked on / dragged, if any */
	MystArea *_clickedResource;

	void loadView();
	void loadResources();
	void loadCursorHints();

	void runInitScript();
	void runExitScript();
};

template<class T>
T *MystCard::getResource(uint index) {
	T *resource = dynamic_cast<T *>(_resources[index]);

	if (!resource) {
		error("View resource '%d' has unexpected type", index);
	}

	return resource;
}

} // End of namespace Mohawk

#endif
