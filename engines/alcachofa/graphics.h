/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/ptr.h"
#include "common/stream.h"
#include "common/serializer.h"
#include "common/rect.h"
#include "math/vector2d.h"
#include "graphics/managed_surface.h"

namespace Alcachofa {

/**
 * Because this gets confusing fast, here in tabular form
 *
 * | BlendMode     | SrcColor                         | SrcAlpha  | SrcBlend | DstBlend     |
 * |:-------------:|:---------------------------------|:----------|:---------|:-------------|
 * | AdditiveAlpha | (1 - TintAlpha) * TexColor       | TexAlpha  | One      | 1 - SrcAlpha |
 * | Additive      | (1 - TintAlpha) * TexColor       | TexAlpha  | One      | One          |
 * | Multiply      | (1 - TintAlpha) * TexColor       | TexAlpha  | DstColor | One          |
 * | Alpha         | TexColor                         | TintAlpha | SrcAlpha | 1 - SrcAlpha |
 * | Tinted        | TintColor * TintAlpha * TexColor | TexAlpha  | One      | 1 - SrcAlpha |
 *
 */
enum class BlendMode {
	AdditiveAlpha,
	Additive,
	Multiply,
	Alpha,
	Tinted
};

enum class CursorType {
	Normal,
	LookAt,
	Use,
	GoTo,
	LeaveUp,
	LeaveRight,
	LeaveDown,
	LeaveLeft
};

enum class Direction {
	Up,
	Down,
	Left,
	Right
};

constexpr const int32 kDirectionCount = 4;

struct Color {
	uint8 b, g, r, a;
};

class ITexture {
public:
	ITexture(Common::Point size);
	virtual ~ITexture() = default;

	virtual void update(const Graphics::ManagedSurface &surface) = 0;

	inline const Common::Point &size() const { return _size; }

private:
	Common::Point _size;
};

class IRenderer {
public:
	virtual ~IRenderer() = default;

	virtual Common::ScopedPtr<ITexture> createTexture(int32 w, int32 h, bool withMipmaps = true) = 0;

	virtual void begin() = 0;
	virtual void setTexture(const ITexture *texture) = 0;
	virtual void setBlendMode(BlendMode blendMode) = 0;
	virtual void setLodBias(float lodBias) = 0;
	virtual void quad(
		Math::Vector2d center,
		Math::Vector2d size,
		Color color = { 255, 255, 255, 255 },
		Math::Angle rotation = Math::Angle(),
		Math::Vector2d texMin = Math::Vector2d(0, 0),
		Math::Vector2d texMax = Math::Vector2d(1, 1)) = 0;
	virtual void end() = 0;

	static IRenderer *createOpenGLRenderer(Common::Point resolution);
};

enum class AnimationFolder {
	Animations,
	Masks,
	Backgrounds
};

struct AnimationFrame {
	Common::Point
		_center, ///< the center is used for more than just drawing the animation frame
		_offset; ///< the offset is only used for drawing the animation frame
	uint32 _duration;
};

/**
 * An animation contains one or more sprites which change their position and image during playback.
 *
 * Internally there is a single list of images. Every sprite ID is mapped to an index
 * (via _spriteIndexMapping) which points to:
 *   1. The fixed image base for that sprite
 *   2. The image offset for that sprite for the current frame
 * Image indices are unfortunately one-based
 *
 * As fonts are handled very differently they are split into a second class
 */
class AnimationBase {
protected:
	AnimationBase(Common::String fileName, AnimationFolder folder = AnimationFolder::Animations);
	~AnimationBase();

	void load();
	void freeImages();
	Graphics::ManagedSurface *readImage(Common::SeekableReadStream &stream) const;

	static constexpr const uint kMaxSpriteIDs = 256;
	Common::String _fileName;
	AnimationFolder _folder;
	bool _isLoaded = false;

	int32 _spriteIndexMapping[kMaxSpriteIDs] = { -1 };
	Common::Array<uint32>
		_spriteOffsets, ///< index offset per sprite and animation frame
		_spriteBases; ///< base index per sprite
	Common::Array<AnimationFrame> _frames;
	Common::Array<Graphics::ManagedSurface *> _images; ///< will contain nullptr for fake images
	Common::Array<Common::Point> _imageOffsets;
};

/**
 * Animations prerenders its sprites into a single texture for a set frame.
 * This prerendering can be customized with a alpha to be premultiplied
 */
class Animation : private AnimationBase {
public:
	Animation(Common::String fileName, AnimationFolder folder = AnimationFolder::Animations);

	void load();
	using AnimationBase::freeImages;

	inline bool isLoaded() const { return _isLoaded; }
	inline uint frameCount() const { return _frames.size(); }
	inline uint spriteCount() const { return _spriteBases.size(); }
	inline uint32 frameDuration(int32 frameI) const { return _frames[frameI]._duration; }

	void draw2D(
		int32 frameI,
		Math::Vector2d center,
		float scale,
		Math::Angle rotation,
		BlendMode blendMode,
		Color color);

private:
	int32 imageIndex(int32 frameI, int32 spriteI) const;
	Common::Rect spriteBounds(int32 frameI, int32 spriteI) const;
	Common::Rect frameBounds(int32 frameI) const;
	Common::Rect maxFrameBounds() const;
	void prerenderFrame(int32 frameI);

	int32_t _renderedFrameI = -1;
	uint8 _premultiplyAlpha = 100; ///< in percent [0-100] not [0-255]

	Graphics::ManagedSurface _renderedSurface;
	Common::ScopedPtr<ITexture> _renderedTexture;
};

class Font : private AnimationBase {

};


class Graphic {
public:
	Graphic();
	Graphic(Common::ReadStream &stream);

	inline int8 order() const { return _order; }

	void start(bool looping);
	void stop();
	void serializeSave(Common::Serializer &serializer);

public:
	Common::SharedPtr<Animation> _animation;
	Common::Point _center;
	int16 _scale = 300;
	int8 _order = 0;

private:
	bool _isPaused = true,
		_isLooping = true;
	uint32 _lastTime = 0;
	float _camAcceleration = 1.0f;
};

}

#endif
