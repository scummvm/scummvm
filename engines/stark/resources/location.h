/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_RESOURCES_LOCATION_H
#define STARK_RESOURCES_LOCATION_H

#include "common/hashmap.h"
#include "common/rect.h"
#include "common/str.h"

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class ItemVisual;
class Layer;
class ModelItem;
class Sound;

/**
 * A location is a scene of the game
 *
 * Locations contain layers. The game engine retrieves the list of renderable
 * items from the current location.
 */
class Location : public Object {
public:
	static const Type::ResourceType TYPE = Type::kLocation;

	Location(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Location();

	// Resource API
	void onAllLoaded() override;
	void onEnterLocation() override;
	void onGameLoop() override;
	void saveLoadCurrent(ResourceSerializer *serializer) override;

	/** Does the location have a 3D layer ? */
	bool has3DLayer();

	/** Obtain a list of render entries for all the items in the location */
	Gfx::RenderEntryArray listRenderEntries();

	/** Obtain a list of render entries for all the lights in the location */
	Gfx::LightEntryArray listLightEntries();

	/** Initialize scrolling from Camera data */
	void initScroll(const Common::Point &maxScroll);

	/** Obtain the current scroll position */
	Common::Point getScrollPosition() const;

	/** Scroll the location to the specified position if possible */
	void setScrollPosition(const Common::Point &position);

	/** Smoothly scroll to a position in 2D world coordinates */
	bool scrollToCoordinateSmooth(uint32 coordinate);

	/** Immediatly scroll the character location */
	void scrollToCharacterImmediate();

	/** Replace the currently active layer */
	void goToLayer(Layer *layer);

	/**
	 * Indicate on script driven scroll is active.
	 *
	 * This means that the location should not follow the character
	 */
	void setHasActiveScroll();

	/**
	 * Stop all script driven scrolls
	 */
	void stopAllScrolls();

	/** Tell the location to scroll to follow the character */
	void startFollowingCharacter();

	/** Tell the location not to scroll to follow the character */
	void stopFollowingCharacter();

	void scrollToCoordinateImmediate(uint32 coordinate);

	/** Get an item from its character index */
	ItemVisual *getCharacterItem(int32 character) const;

	/** Register an item as a character to the location */
	void registerCharacterItem(int32 character, ItemVisual *item);

	/** Get the list of items with a 3d model present in the location */
	const Common::Array<ModelItem *> &listModelItems() const;

	/** Reset animation blending for all the items in the location */
	void resetAnimationBlending();

	/** Find a stock sound by its type in the location, the level, or the global level */
	Sound *findStockSound(uint32 stockSoundType) const;

	/** Set remaining frames to rumble on this lcation */
	void startRumble(int32 rumbleDurationRemaining);

	/** Setup fading for this location */
	void fadeInInit(int32 fadeDuration);
	void fadeOutInit(int32 fadeDuration);

	/** Setup a swaying movement for the 3d items in this location */
	void swayScene(int32 periodMs, const Math::Angle &angle, float amplitude, float offset);

	/** Setup a up / down floating movement for the 3d items in this location */
	void floatScene(int32 periodMs, float amplitude, float offset);

	/** Get the layer with a given name, return null when not found */
	Layer *getLayerByName(const Common::String &name);

	/** Get a render entry with a given name, return null when not found */
	Gfx::RenderEntry *getRenderEntryByName(const Common::String &name);

	/** Obtain the list of all the inner layers */
	Common::Array<Layer *> listLayers() { return _layers; }

	/** List all the exit positions */
	Common::Array<Common::Point> listExitPositions();

protected:
	void printData() override;

private:
	bool scrollToSmooth(const Common::Point &position, bool followCharacter);
	bool scrollToCharacter(ModelItem *item);
	Common::Point getCharacterScrollPosition(ModelItem *item);
	uint getScrollStepFollow();
	Common::Point getScrollPointFromCoordinate(uint32 coordinate) const;

	Sound *findStockSound(const Object *parent, uint32 stockSoundType) const;

	Common::Array<Layer *> _layers;
	Layer *_currentLayer;

	bool _canScroll;
	bool _hasActiveScroll;
	bool _scrollFollowCharacter;
	Common::Point _scroll;
	Common::Point _maxScroll;

	uint getScrollStep();

	typedef Common::HashMap<int32, ItemVisual *> CharacterMap;
	CharacterMap _characterItemMap;

	Common::Array<ModelItem *> _modelItems;

	int32 _rumbleDurationRemaining;

	bool _fadeOut;
	int32 _fadeDuration;
	float _fadePosition;

	int32 _swayPeriodMs;
	Math::Angle _swayAngle;
	float _swayAmplitude;
	float _swayOffset;
	float _swayPosition;

	int32 _idleActionWaitMs;

	int32 _floatPeriodMs;
	float _floatAmplitude;
	float _floatPosition;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_LOCATION_H
