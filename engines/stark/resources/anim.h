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

#ifndef STARK_RESOURCES_ANIM_H
#define STARK_RESOURCES_ANIM_H

#include "common/rect.h"
#include "common/str.h"

#include "engines/stark/resources/object.h"

namespace Stark {

class SkeletonAnim;
class VisualActor;
class VisualProp;
class VisualSmacker;
class Visual;
namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Direction;
class Image;
class Item;
class ItemVisual;

/**
 * Animation base class
 *
 * Animations provide a time dependent visual state to Items
 */
class Anim : public Object {
public:
	static const Type::ResourceType TYPE = Type::kAnim;

	enum SubType {
		kAnimImages   = 1,
		kAnimProp     = 2,
		kAnimVideo    = 3,
		kAnimSkeleton = 4
	};

	enum ActionUsage {
		kActionUsagePassive = 1,
		kActionUsageActive  = 2
	};

	enum UIUsage {
		kUIUsageInventory = 1,
		kUIUsageUseCursorPassive = 4,
		kUIUsageUseCursorActive = 5
	};

	enum ActorActivity {
		kActorActivityIdle       = 1,
		kActorActivityWalk       = 2,
		kActorActivityTalk       = 3,
		kActorActivityRun        = 6,
		kActorActivityIdleAction = 10
	};

	/** Anim factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Anim(Object *parent, byte subType, uint16 index, const Common::String &name);
	~Anim() override;

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;

	/** Sets the animation frame to be displayed */
	virtual void selectFrame(uint32 frameIndex);

	/** Obtain the Visual to be used to render the animation */
	virtual Visual *getVisual() = 0;

	/** Associate the animation to an Item */
	virtual void applyToItem(Item *item);

	/** Dissociate the animation from an item */
	virtual void removeFromItem(Item *item);

	/** Check is the animation is being used by an item */
	bool isInUse();

	/** Obtain the purpose of this anim */
	uint32 getActivity() const;

	/** Return the hotspot index for a point given in relative coordinates */
	virtual int getPointHotspotIndex(const Common::Point &point) const;

	/** Get the animation typical duration in milliseconds */
	virtual uint32 getDuration() const;

	/**
	 * Play the animation as an action for an item.
	 *
	 * This sets up a callback to the item for when the animation completes.
	 */
	virtual void playAsAction(ItemVisual *item);

	/** Checks if the elapsed time since the animation start is greater than a specified duration */
	virtual bool isAtTime(uint32 time) const;

	/** Get the duration in milliseconds before the animation loops ends */
	virtual uint32 getRemainingTime() const;

	/** Get the anim movement speed in units per seconds */
	virtual uint32 getMovementSpeed() const;

	/** Get the chance the animation has to play among other idle actions from the same anim hierarchy */
	virtual uint32 getIdleActionFrequency() const;

protected:
	virtual void printData() override;

	uint32 _activity;
	uint32 _currentFrame;
	uint32 _numFrames;
	int32 _refCount;
};

/**
 * Displays still images controlled by an AnimScript
 */
class AnimImages : public Anim {
public:
	AnimImages(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimImages();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;
	void saveLoad(ResourceSerializer *serializer) override;

	// Anim API
	void selectFrame(uint32 frameIndex) override;
	Visual *getVisual() override;
	int getPointHotspotIndex(const Common::Point &point) const override;

protected:
	void printData() override;

	float _field_3C;

	uint32 _currentDirection;
	Common::Array<Direction *> _directions;

	Image *_currentFrameImage;
};

class AnimProp : public Anim {
public:
	AnimProp(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimProp();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onPostRead() override;

	// Anim API
	Visual *getVisual() override;
	uint32 getMovementSpeed() const override;

protected:
	void printData() override;

	Common::String _field_3C;
	Common::Array<Common::String> _meshFilenames;
	Common::String _textureFilename;
	uint32 _movementSpeed;
	Common::String _archiveName;

	VisualProp *_visual;
};

/**
 * Displays a Smacker video
 */
class AnimVideo : public Anim {
public:
	AnimVideo(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimVideo();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onAllLoaded() override;
	void onGameLoop() override;
	void onEnginePause(bool pause) override;
	void saveLoadCurrent(ResourceSerializer *serializer) override;

	// Anim API
	Visual *getVisual() override;
	void playAsAction(ItemVisual *item) override;
	uint32 getDuration() const override;
	bool isAtTime(uint32 time) const override;

protected:
	typedef Common::Array<Common::Point> PointArray;
	typedef Common::Array<Common::Rect> RectArray;

	void printData() override;

	/** Update the position of the video for the current frame */
	void updateSmackerPosition();

	Common::String _smackerFile;
	Common::String _archiveName;

	VisualSmacker *_smacker;

	uint32 _width;
	uint32 _height;

	PointArray _positions;
	RectArray _sizes;

	int32 _frameRateOverride;
	bool _preload;
	bool _loop;

	ItemVisual *_actionItem;
};

/**
 * Animates a 3D mesh skeleton
 */
class AnimSkeleton : public Anim {
public:
	AnimSkeleton(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimSkeleton();

	// Resource API
	void readData(Formats::XRCReadStream *stream) override;
	void onPostRead() override;
	void onAllLoaded() override;
	void onGameLoop() override;
	void onExitLocation() override;
	void onPreDestroy() override;

	// Anim API
	void applyToItem(Item *item) override;
	void removeFromItem(Item *item) override;
	Visual *getVisual() override;
	uint32 getDuration() const override;
	void playAsAction(ItemVisual *item) override;
	bool isAtTime(uint32 time) const override;
	uint32 getMovementSpeed() const override;
	uint32 getIdleActionFrequency() const override;
	uint32 getRemainingTime() const override;

	/** Get the position in the animation loop in milliseconds */
	uint32 getCurrentTime() const;

protected:
	void printData() override;

	bool _castsShadow;
	Common::String _archiveName;
	Common::String _animFilename;
	bool _loop;
	uint32 _movementSpeed;
	uint32 _idleActionFrequency;

	uint32 _totalTime;
	uint32 _currentTime;

	SkeletonAnim *_seletonAnim;
	VisualActor *_visual;

	ItemVisual *_actionItem;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_H
