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
class VisualSmacker;
class Visual;
namespace Formats {
class XRCReadStream;
}

namespace Resources {

class Direction;
class Image;
class Item;

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
		kAnimSub2     = 2,
		kAnimVideo    = 3,
		kAnimSkeleton = 4
	};

	/** Anim factory */
	static Object *construct(Object *parent, byte subType, uint16 index, const Common::String &name);

	Anim(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Anim();

	// Resource API
	virtual void readData(Formats::XRCReadStream *stream) override;

	/** Sets the animation frame to be displayed */
	virtual void selectFrame(uint32 frameIndex);

	/** Obtain the Visual to be used to render the animation */
	virtual Visual *getVisual();

	/** Associate the animation to an Item */
	virtual void applyToItem(Item *item);

	/** Dissociate the animation from an item */
	virtual void removeFromItem(Item *item);

	/** Check is the animation is being used by an item */
	bool isInUse();

protected:
	virtual void printData() override;

	uint32 _field_30;
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

	// Anim API
	void selectFrame(uint32 frameIndex) override;
	Visual *getVisual() override;

protected:
	void printData() override;

	float _field_3C;

	uint32 _currentDirection;
	Common::Array<Direction *> _directions;

	Image *_currentFrameImage;
};

class AnimSub2 : public Anim {
public:
	AnimSub2(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~AnimSub2();
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

	// Anim API
	Visual *getVisual() override;

protected:
	typedef Common::Array<Common::Point> PointArray;
	typedef Common::Array<Common::Rect> RectArray;

	void printData() override;

	Common::String _smackerFile;
	Common::String _archiveName;

	VisualSmacker *_smacker;

	uint32 _width;
	uint32 _height;

	PointArray _positions;
	RectArray _sizes;

	int32 _field_4C;
	uint32 _field_50;
	uint32 _field_7C;
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

	// Anim API
	void applyToItem(Item *item) override;
	void removeFromItem(Item *item) override;
	Visual *getVisual() override;

protected:
	void printData() override;

	bool _castsShadow;
	Common::String _archiveName;
	Common::String _animFilename;
	uint32 _field_48;
	uint32 _field_4C;
	uint32 _field_6C;

	uint32 _totalTime;
	uint32 _currentTime;

	SkeletonAnim *_seletonAnim;
	VisualActor *_visual;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_ANIM_H
