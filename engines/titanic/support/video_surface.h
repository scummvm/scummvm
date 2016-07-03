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
#include "titanic/support/font.h"
#include "titanic/support/direct_draw.h"
#include "titanic/support/movie.h"
#include "titanic/support/movie_range_info.h"
#include "titanic/support/rect.h"
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
	/**
	 * Calculates blitting bounds
	 */
	void clipBounds(Rect &srcRect, Rect &destRect, CVideoSurface *srcSurface,
		const Rect *subRect = nullptr, const Point *destPos = nullptr);

	void blitRect1(const Rect &srcRect, const Rect &destRect, CVideoSurface *src);
	void blitRect2(const Rect &srcRect, const Rect &destRect, CVideoSurface *src);
protected:
	static int _videoSurfaceCounter;
protected:
	CScreenManager *_screenManager;
	Graphics::ManagedSurface *_rawSurface;
	bool _pendingLoad;
	void *_field40;
	int _field44;
	int _field48;
	int _videoSurfaceNum;
	int _field50;
	int _lockCount;
public:
	CMovie *_movie;
	DirectDrawSurface *_ddSurface;
	bool _blitFlag;
	bool _blitStyleFlag;
	CResourceKey _resourceKey;
public:
	CVideoSurface(CScreenManager *screenManager);
	virtual ~CVideoSurface();

	/**
	 * Set the underlying surface for this video surface
	 */
	void setSurface(CScreenManager *screenManager, DirectDrawSurface *surface);

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
	virtual void recreate(int width, int height) = 0;

	/**
	 * Resizes the surface
	 */
	virtual void resize(int width, int height) = 0;

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
	 * Change a pixel
	 */
	virtual void changePixel(uint16 *pixelP, uint16 *color, byte srcVal, bool remapFlag = true) = 0;

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
	virtual void playMovie(uint flags, CVideoSurface *surface) = 0;

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	virtual void playMovie(uint startFrame, uint endFrame, int v3, bool v4) = 0;

	virtual void proc35(int v1, int v2, int frameNumber, int flags, CGameObject *owner) = 0;

	/**
	 * Stops any movie currently attached to the surface
	 */
	virtual void stopMovie() = 0;

	/**
	 * Sets the movie to the specified frame number
	 */
	virtual void setMovieFrame(uint frameNumber) = 0;

	virtual void proc38(int v1, int v2) = 0;

	virtual void proc39(int v1, int v2) = 0;

	/**
	 * Return any movie range info associated with the surface's movie
	 */
	virtual const Common::List<CMovieRangeInfo *> getMovieRangeInfo() const = 0;

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

	virtual bool proc45();

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

	void set40(void *v) { _field40 = v; }

	uint16 *getPixels() { return (uint16 *)_rawSurface->getPixels(); }

	/**
	 * Returns the transparent color
	 */
	uint getTransparencyColor();
};

class OSVideoSurface : public CVideoSurface {
	friend class OSMovie;
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
		setupPalette(_palette2, 0xe0);
	}
public:
	OSVideoSurface(CScreenManager *screenManager, DirectDrawSurface *surface);
	OSVideoSurface(CScreenManager *screenManager, const CResourceKey &key, bool flag = false);

	/**
	 * Load the surface with the passed resource
	 */
	virtual void loadResource(const CResourceKey &key);

	/**
	 * Loads a Targa image file specified by the resource key
	 */
	virtual void loadTarga(const CResourceKey &key);

	/**
	 * Loads a JPEG image file specified by the resource key
	 */
	virtual void loadJPEG(const CResourceKey &key);

	/**
	 * Loads a movie file specified by the resource key.
	 * @param key			Resource key for movie to load
	 * @param destroyFlag	Immediately destroy movie after decoding first frame
	 */
	virtual void loadMovie(const CResourceKey &key, bool destroyFlag = false);

	/**
	 * Lock the surface for direct access to the pixels
	 */
	virtual bool lock();

	/**
	 * Unlocks the surface after prior calls to lock()
	 */
	virtual void unlock();

	/**
	 * Returns true if an underlying raw surface has been set
	 */
	virtual bool hasSurface();

	/**
	 * Returns the width of the surface
	 */
	virtual int getWidth();

	/**
	 * Returns the height of the surface
	 */
	virtual int getHeight();

	/**
	 * Returns the pitch of the surface in bytes
	 */
	virtual int getPitch();

	/**
	 * Returns the bytes per pixel of the surface
	 */
	virtual int getBpp();

	/**
	 * Recreates the surface with the designated size
	 */
	virtual void recreate(int width, int height);

	/**
	 * Resizes the surface
	 */
	virtual void resize(int width, int height);

	/**
	 * Detachs the underlying raw surface
	 */
	virtual void detachSurface();

	/**
	 * Returns the number of bytes per pixel in the surface
	 */
	virtual int getPixelDepth();

	/**
	 * Gets the pixel at the specified position within the surface
	 */
	virtual uint16 getPixel(const Point &pt);

	/**
	 * Sets a pixel at a specified position within the surface
	 */
	virtual void setPixel(const Point &pt, uint pixel);

	/**
	 * Change a pixel
	 */
	virtual void changePixel(uint16 *pixelP, uint16 *color, byte srcVal, bool remapFlag = true);

	/**
	 * Shifts the colors of the surface.. maybe greys it out?
	 */
	virtual void shiftColors();

	/**
	 * Clears the entire surface to black
	 */
	virtual void clear();

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	virtual void playMovie(uint flags, CVideoSurface *surface);

	/**
	 * Plays a movie, loading it from the specified _resource
	 * if not already loaded
	 */
	virtual void playMovie(uint startFrame, uint endFrame, int v3, bool v4);

	virtual void proc35(int v1, int v2, int frameNumber, int flags, CGameObject *owner);

	/**
	 * Stops any movie currently attached to the surface
	 */
	virtual void stopMovie();

	/**
	 * Sets the movie to the specified frame number
	 */
	virtual void setMovieFrame(uint frameNumber);

	virtual void proc38(int v1, int v2);

	virtual void proc39(int v1, int v2);

	/**
	 * Return any movie range info associated with the surface's movie
	 */
	virtual const Common::List<CMovieRangeInfo *> getMovieRangeInfo() const;

	/**
	 * Loads the surface's resource if there's one pending
	 */
	virtual bool loadIfReady();

	/**
	 * Loads the surface data based on the currently set resource key
	 */
	virtual bool load();

	/**
	 * Does a replacement of transparent pixels on certain lines at regular
	 * intervals. This is totally weird
	 */
	virtual void transPixelate();

	/**
	 * Frees the underlying surface
	 */
	virtual int freeSurface();

	/**
	 * Get a pointer into the underlying surface
	 */
	virtual uint16 *getBasePtr(int x, int y);
};

} // End of namespace Titanic

#endif /* TITANIC_VIDEO_SURFACE_H */
