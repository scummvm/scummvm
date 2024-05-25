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
#include "common/span.h"
#include "math/vector2d.h"
#include "graphics/managed_surface.h"

#include "camera.h"
#include "common.h"

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

class Shape;

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
		Color color = kWhite,
		Math::Angle rotation = Math::Angle(),
		Math::Vector2d texMin = Math::Vector2d(0, 0),
		Math::Vector2d texMax = Math::Vector2d(1, 1)) = 0;
	virtual void end() = 0;

	static IRenderer *createOpenGLRenderer(Common::Point resolution);
};

class IDebugRenderer : public IRenderer {
public:
	virtual void debugPolygon(
		Common::Span<Math::Vector2d> points,
		Color color = kDebugRed
	) = 0;
	virtual void debugPolyline(
		Common::Span<Math::Vector2d> points,
		Color color = kDebugRed
	) = 0;

	virtual void debugShape(
		const Shape &shape,
		Color color = kDebugRed
	);
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
	uint32 _totalDuration = 0;

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
	inline uint spriteCount() const { return _spriteBases.size(); }
	inline uint frameCount() const { return _frames.size(); }
	inline uint32 frameDuration(int32 frameI) const { return _frames[frameI]._duration; }
	inline const Common::Point &frameCenter(int32 frameI) const { return _frames[frameI]._center; }
	inline uint32 totalDuration() const { return _totalDuration; }
	int32 frameAtTime(uint32 time) const;
	Common::Point imageSize(int32 imageI) const;

	void draw2D(
		int32 frameI,
		Math::Vector2d center,
		float scale,
		BlendMode blendMode,
		Color color);
	void draw3D(
		int32 frameI,
		Math::Vector3d center,
		float scale,
		BlendMode blendMode,
		Color color);
	void drawEffect(
		int32 frameI,
		Math::Vector3d center,
		Math::Vector2d tiling,
		Math::Vector2d texOffset,
		BlendMode blendMode);

private:
	int32 imageIndex(int32 frameI, int32 spriteI) const;
	Common::Rect spriteBounds(int32 frameI, int32 spriteI) const;
	Common::Rect frameBounds(int32 frameI) const;
	Common::Rect maxFrameBounds() const;
	Math::Vector2d totalFrameOffset(int32 frameI) const;
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

	inline Common::Point &center() { return _center; }
	inline int8 &order() { return _order; }
	inline int16 &scale() { return _scale; }
	inline float &depthScale() { return _depthScale; }
	inline Color &color() { return _color; }
	inline int32 &frameI() { return _frameI; }
	inline uint32 lastTime() const { return _lastTime; }
	inline Animation &animation() {
		assert(_animation != nullptr && _animation->isLoaded());
		return *_animation;
	}

	void loadResources();
	void freeResources();
	void update();
	void start(bool looping);
	void pause();
	void reset();
	void setAnimation(const Common::String &fileName, AnimationFolder folder);
	void setAnimation(Animation *animation); ///< no memory ownership is given, but for prerendering it has to be mutable
	void serializeSave(Common::Serializer &serializer);

private:
	friend class AnimationDrawRequest;
	friend class SpecialEffectDrawRequest;
	Common::ScopedPtr<Animation> _ownedAnimation;
	Animation *_animation = nullptr;
	Common::Point _center;
	int16 _scale = kBaseScale;
	int8 _order = 0;
	Color _color = kWhite;

	bool _isPaused = true,
		_isLooping = true;
	uint32 _lastTime = 0; ///< either start time or played duration at pause
	int32 _frameI = -1;
	float _depthScale = 1.0f;
};

class IDrawRequest {
public:
	IDrawRequest(int8 order);
	virtual ~IDrawRequest() = default;

	inline int8 order() const { return _order; }
	virtual void draw() = 0;

private:
	const int8 _order;
};

class AnimationDrawRequest : public IDrawRequest {
public:
	AnimationDrawRequest(
		Graphic &graphic,
		bool is3D,
		BlendMode blendMode,
		float lodBias = 0.0f);
	AnimationDrawRequest(
		Animation *animation,
		int32 frameI,
		Math::Vector2d center,
		int8 order
	);

	virtual void draw() override;

private:
	bool _is3D;
	Animation *_animation;
	int32 _frameI;
	Math::Vector3d _center;
	float _scale;
	Color _color;
	BlendMode _blendMode;
	float _lodBias;
};

class SpecialEffectDrawRequest : public IDrawRequest {
public:
	SpecialEffectDrawRequest(
		Graphic &graphic,
		Common::Point topLeft,
		Common::Point bottomRight,
		Math::Vector2d texOffset,
		BlendMode blendMode);

	virtual void draw() override;

private:
	Animation *_animation;
	int32 _frameI;
	Math::Vector3d _topLeft;
	Math::Vector2d
		_tiling,
		_texOffset;
	BlendMode _blendMode;
};

class BumpAllocator {
public:
	BumpAllocator(size_t pageSize);
	~BumpAllocator();

	template<typename T, typename... Args>
	inline T *allocate(Args&&... args) {
		return new(allocateRaw(sizeof(T), alignof(T))) T(Common::forward<Args>(args)...);
	}
	void *allocateRaw(size_t size, size_t align);
	void deallocateAll();

private:
	void allocatePage();

	const size_t _pageSize;
	size_t _pageI = 0, _used = 0;
	Common::Array<void *> _pages;
};

class DrawQueue {
public:
	DrawQueue(IRenderer *renderer);

	template<typename T, typename... Args>
	inline void add(Args&&... args) {
		addRequest(_allocator.allocate<T>(Common::forward<Args>(args)...));
	}

	void clear();
	void setLodBias(int8 orderFrom, int8 orderTo, float newLodBias);
	void draw();

private:
	void addRequest(IDrawRequest *drawRequest);

	static constexpr const uint kMaxDrawRequestsPerOrder = 50;
	IRenderer *const _renderer;
	BumpAllocator _allocator;
	IDrawRequest *_requestsPerOrder[kOrderCount][kMaxDrawRequestsPerOrder] = { 0 };
	uint8 _requestsPerOrderCount[kOrderCount] = { 0 };
	float _lodBiasPerOrder[kOrderCount] = { 0 };
};

}

#endif
