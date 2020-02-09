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

#ifndef TITANIC_VIDEO_SURFACE_H
#define TITANIC_VIDEO_SURFACE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "graphics/managed_surface.h"
#include "titanic/support/font.h"
#include "titanic/support/direct_draw.h"
#include "titanic/support/movie.h"
#include "titanic/support/movie_range_info.h"
#include "titanic/support/rect.h"
#include "titanic/support/transparency_surface.h"
#include "titanic/core/list.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class CScreenManager;
class CJPEGDecode;
class CTargaDecode;

class CVideoSurface : public ListItem {
	friend class CJPEGDecode;
	friend class CTargaDecode;
private:
	static byte _palette1[32][32];
	static byte _palette2[32][32];

	/**
	 * Setup the shading palettes
	 */
	static void setupPalette(byte palette[32][32], byte val);
public:
	/**
	 * Setup statics
	 */
	static void setup() {
		setupPalette(_palette1, 0xff);
	}
private:
	/**
	 * Calculates blitting bounds
	 */
	void clipBounds(Rect &srcRect, Rect &destRect, CVideoSurface *srcSurface,
		const Rect *subRect = nullptr, const Point *destPos = nullptr);

	/**
	 * Copies a rect from a given source surface
	 */
	void blitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src);

	/**
	 * Copies a rect from a given source surface and draws it vertically flipped
	 */
	void flippedBlitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src);

	void transBlitRect(const Rect &srcRect, const Rect &destRect, CVideoSurface *src, bool flipFlag);
protected:
	static int _videoSurfaceCounter;
protected:
	CScreenManager *_screenManager;
	Graphics::ManagedSurface *_rawSurface;
	bool _pendingLoad;
	Graphics::ManagedSurface *_transparencySurface;
	DisposeAfterUse::Flag _freeTransparencySurface;
	int _videoSurfaceNum;
	bool _hasFrame;
	int _lockCount;
public:
	CMovie *_movie;
	DirectDrawSurface *_ddSurface;
	bool _fastBlitFlag;
	bool _flipVertically;
	CResourceKey _resourceKey;
	TransparencyMode _transparencyMode;
public:
	CVideoSurface(CScreenManager *screenManager);
	~CVideoSurface() override;

	/**
	 * Set the underlying surface for this video surface
	 */
	void setSurface(CScreenManager *screenManager, DirectDrawSurface *surface);

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override {
		ListItem::load(file);
	}

	/**
	 * Load the surface with the passed resource
	 */
	virtual void loadResource(const CResourceKey &key) = 0;

	/**
	 * Loads a Targa image file specified by the resource key
	 */
	virtual void loadTarga(const CResourceKey &key) = 0;

	/**
	 * Loads a JPEG image file specified by the resource key
	 */
	virtual void loadJPEG(const CResourceKey &key) = 0;

	/**
	 * Loads a Targa image file specified by the given name
	 */
	virtual void loadTarga(const CString &name) = 0;

	/**
	 * Loads a movie file specified by the resource key.
	 * @param key			Resource key for movie to load
	 * @param destroyFlag	Immediately destroy movie after decoding first frame
	 */
	virtual void loadMovie(const CResourceKey &key, bool destroyFlag = false) = 0;

	/**
	 * Lock the surface for direct access to the pixels
	 */
	virtual bool lock() = 0;

	/**
	 * Unlocks the surface after prior calls to lock()
	 */
	virtual void unlock() = 0;

	/**
	 * Returns true if an underlying raw surface has been set
	 */
	virtual bool hasSurface() = 0;

	/**
	 * Returns the width of the surface
	 */
	virtual int getWidth() = 0;

	/**
	 * Returns the height of the surface
	 */
	virtual int getHeight() = 0;

	/**
	 * Returns the pitch of the surface in bytes
	 */
	virtual int getPitch() = 0;

	/**
	 * Returns the bytes per pixel of the surface
	 */
	virtual int getBpp() = 0;

	/**
	 * Recreates the surface
	 */
	virtual void recreate(int width, int height, int bpp = 16) = 0;

	/**
	 * Resizes the surface
	 */
	virtual void resize(int width, int height, int bpp = 16) = 0;

	/**
	 * Detachs the underlying raw surface
	 */
	virtual void detachSurface() = 0;

	/**
	 * Returns the number of bytes per pixel in the surface
	 */
	virtual int getPixelDepth() = 0;

	/**
	 * Gets the pixel at the specified position within the surface
	 */
	virtual uint16 getPixel(const Common::Point &pt) = 0;


	/**
	 * Sets a pixel at a specified position within the surface
	 */
	virtual void setPixel(const Point &pt, uint pixel) = 0;

	/**
	 * Shifts the colors of the surface.. maybe greys it out?
	 */
	virtual void shiftColors() = 0;

	/**
	 * Clears the entire surface to black
	 */
	virtual void clear() = 0;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	virtual void playMovie(uint flags, CGameObject *obj) = 0;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	virtual void playMovie(uint startFrame, uint endFrame, uint flags, CGameObject *obj) = 0;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	virtual void playMovie(uint startFrame, uint endFrame, uint initialFrame, uint flags, CGameObject *obj) = 0;

	/**
	 * Stops any movie currently attached to the surface
	 */
	virtual void stopMovie() = 0;

	/**
	 * Set the current movie frame number
	 */
	virtual void setMovieFrame(uint frameNumber) = 0;

	/**
	 * Adds a movie playback event
	 */
	virtual void addMovieEvent(int eventId, CGameObject *obj) = 0;

	/**
	 * Set the movie frame rate
	 */
	virtual void setMovieFrameRate(double rate) = 0;

	/**
	 * Return any movie range info associated with the surface's movie
	 */
	virtual const CMovieRangeInfoList *getMovieRangeInfo() const = 0;

	/**
	 *
	 */
	virtual void flipVertically(bool needsLock = true) = 0;

	/**
	 * Loads the surface's resource if there's one pending
	 */
	virtual bool loadIfReady() = 0;

	/**
	 * Loads the surface data based on the currently set resource key
	 */
	virtual bool load() = 0;

	/**
	 * Does a replacement of transparent pixels on certain lines at regular
	 * intervals. This is totally weird
	 */
	virtual void transPixelate() = 0;

	/**
	 * Returns true if there's a frame to display on the video surface
	 */
	virtual bool hasFrame();

	/**
	 * Duplicates movie transparency surface
	 */
	virtual Graphics::ManagedSurface *dupMovieTransparency() const = 0;

	/**
	 * Frees the underlying surface
	 */
	virtual int freeSurface() { return 0; }

	/**
	 * Get a pointer into the underlying surface
	 */
	virtual uint16 *getBasePtr(int x, int y) = 0;

	/**
	 * Blit from another surface
	 */
	void blitFrom(const Point &destPos, CVideoSurface *src, const Rect *srcRect = nullptr);

	/**
	 * Blit from another surface
	 */
	void blitFrom(const Point &destPos, const Graphics::Surface *src);

	/**
	 * Sets a raw surface to use as a transparency mask for the surface
	 */
	void setTransparencySurface(Graphics::ManagedSurface *surface) { _transparencySurface = surface; }

	/**
	 * Get the previously set transparency mask surface
	 */
	const Graphics::Surface *getTransparencySurface() const {
		return _transparencySurface ? &_transparencySurface->rawSurface() : nullptr;
	}

	/**
	 * Get the pixels associated with the surface. Only valid when the
	 * surface has been locked for access
	 */
	uint16 *getPixels() { return (uint16 *)_rawSurface->getPixels(); }

	/**
	 * Get a reference to the underlying surface. Only valid when the surface
	 * has been locked for access
	 */
	Graphics::ManagedSurface *getRawSurface() { return _rawSurface; }

	/**
	 * Returns the transparent color
	 */
	uint getTransparencyColor();

	/**
	 * Copies a pixel, handling transparency
	 * @param destP		Dest pointer to 16-bit pixel
	 * @param srcP		Source pointer to 16-bit pixel
	 * @param alpha		Alpha (0-31). At 0, it's completely opaque,
	 *	and overwrites the dest pixel. Through to 31, which is completely
	 *	transparent, and ignores the source pixel.
	 * @param srcFormat	The source surface format
	 * @param isAlpha	If true, has alpha channel
	 */
	void copyPixel(uint16 *destP, const uint16 *srcP, byte alpha,
		const Graphics::PixelFormat &srcFormat, bool isAlpha);
};

class OSVideoSurface : public CVideoSurface {
	friend class OSMovie;
public:
	OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface);
	OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool flag = false);
	~OSVideoSurface() override;

	/**
	 * Load the surface with the passed resource
	 */
	void loadResource(const CResourceKey &key) override;

	/**
	 * Loads a Targa image file specified by the resource key
	 */
	void loadTarga(const CResourceKey &key) override;

	/**
	 * Loads a JPEG image file specified by the resource key
	 */
	void loadJPEG(const CResourceKey &key) override;

	/**
	 * Loads a Targa image file specified by the given name
	 */
	void loadTarga(const CString &name) override;

	/**
	 * Loads a movie file specified by the resource key.
	 * @param key			Resource key for movie to load
	 * @param destroyFlag	Immediately destroy movie after decoding first frame
	 */
	void loadMovie(const CResourceKey &key, bool destroyFlag = false) override;

	/**
	 * Lock the surface for direct access to the pixels
	 */
	bool lock() override;

	/**
	 * Unlocks the surface after prior calls to lock()
	 */
	void unlock() override;

	/**
	 * Returns true if an underlying raw surface has been set
	 */
	bool hasSurface() override;

	/**
	 * Returns the width of the surface
	 */
	int getWidth() override;

	/**
	 * Returns the height of the surface
	 */
	int getHeight() override;

	/**
	 * Returns the pitch of the surface in bytes
	 */
	int getPitch() override;

	/**
	 * Returns the bytes per pixel of the surface
	 */
	int getBpp() override;

	/**
	 * Recreates the surface with the designated size
	 */
	void recreate(int width, int height, int bpp = 16) override;

	/**
	 * Resizes the surface
	 */
	void resize(int width, int height, int bpp = 16) override;

	/**
	 * Detachs the underlying raw surface
	 */
	void detachSurface() override;

	/**
	 * Returns the number of bytes per pixel in the surface
	 */
	int getPixelDepth() override;

	/**
	 * Gets the pixel at the specified position within the surface
	 */
	uint16 getPixel(const Point &pt) override;

	/**
	 * Sets a pixel at a specified position within the surface
	 */
	void setPixel(const Point &pt, uint pixel) override;

	/**
	 * Shifts the colors of the surface.. maybe greys it out?
	 */
	void shiftColors() override;

	/**
	 * Clears the entire surface to black
	 */
	void clear() override;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	void playMovie(uint flags, CGameObject *obj) override;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	void playMovie(uint startFrame, uint endFrame, uint flags, CGameObject *obj) override;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	void playMovie(uint startFrame, uint endFrame, uint initialFrame, uint flags, CGameObject *obj) override;

	/**
	 * Stops any movie currently attached to the surface
	 */
	void stopMovie() override;

	/**
	 * Sets the movie to the specified frame number
	 */
	void setMovieFrame(uint frameNumber) override;

	/**
	 * Adds a movie playback event
	 */
	void addMovieEvent(int frameNumber, CGameObject *obj) override;

	/**
	 * Set the movie frame rate
	 */
	void setMovieFrameRate(double rate) override;

	/**
	 * Return any movie range info associated with the surface's movie
	 */
	const CMovieRangeInfoList *getMovieRangeInfo() const override;

	/**
	 *
	 */
	void flipVertically(bool needsLock = true) override;

	/**
	 * Loads the surface's resource if there's one pending
	 */
	bool loadIfReady() override;

	/**
	 * Loads the surface data based on the currently set resource key
	 */
	bool load() override;

	/**
	 * Does a replacement of transparent pixels on certain lines at regular
	 * intervals. This is totally weird
	 */
	void transPixelate() override;

	/**
	 * Duplicates movie transparency surface
	 */
	Graphics::ManagedSurface *dupMovieTransparency() const override;

	/**
	 * Frees the underlying surface
	 */
	int freeSurface() override;

	/**
	 * Get a pointer into the underlying surface
	 */
	uint16 *getBasePtr(int x, int y) override;
};

} // End of namespace Titanic

#endif /* TITANIC_VIDEO_SURFACE_H */
