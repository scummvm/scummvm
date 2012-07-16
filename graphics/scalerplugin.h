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
 */

#ifndef GRAPHICS_SCALERPLUGIN_H
#define GRAPHICS_SCALERPLUGIN_H

#include "base/plugins.h"
#include "graphics/pixelformat.h"

enum SourceType {
	SRC_SCREEN = 0,
	SRC_CURSOR,
	SRC_OTHER,

	SRC_MAX
};


class ScalerPluginObject : public PluginObject {
public:

	virtual ~ScalerPluginObject() {}
	
	/**
	 * This function will be called before any scaler is used.
	 * Precomputed data should be generated here.
	 * @param format The pixel format to scale.
	 */
	virtual void initialize(Graphics::PixelFormat format) = 0;

	/**
	 * This is called when the plugin is not needed. It should clean
	 * up memory from the initialize method.
	 */
	virtual void deinitialize() {}

	virtual void scale(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
	                   uint32 dstPitch, int width, int height, int x, int y) = 0;


	/**
	 * Increase the factor of scaling.
	 * @return The new factor
	 */
	virtual uint increaseFactor() = 0;

	/**
	 * Decrease the factor of scaling.
	 * @return The new factor
	 */
	virtual uint decreaseFactor() = 0;

	virtual uint getFactor() const = 0;

	virtual const Common::Array<uint>& getFactors() const { return _factors; }

	/**
	 * Set the scaling factor.
	 * Intended to be used with GUI to set a known valid factor.
	 * Plugins should override if they require additional state changes.
	 * @param factor A valid scaling factor for the plugin
	 * @return The factor set
	 */
	virtual uint setFactor(uint factor) { return _factor = factor; }

	/**
	 * Indicates how far outside the scaling region this scaler "looks"
	 * @return The number of pixels in any direction
	 */
	virtual uint extraPixels() const = 0;

	/**
	 * Some scalers are not suitable for scaling the cursor.
	 * Blurring scalers should return false.
	 */
	virtual bool canDrawCursor() const = 0;

	/**
	 * This value will be displayed on the GUI.
	 */
	virtual const char *getPrettyName() const = 0;

	/**
	 * Computationally intense scalers can benefit from comparing new and old
	 * source images and updating only the pixels necessary. If the function
	 * returns true, this scaler prefers this method and the backend can
	 * optionally call it.
	 *
	 * @see oldSourceScale
	 * @see setSource
	 */
	virtual bool useOldSrc() const { return false; }

	/**
	 * Set the source to be used when scaling and copying to the old buffer.
	 *
	 * @param padding The number of pixels on the border (Used to prevent memory access crashes)
	 * @param type    The surface type. This source will only be used when calling oldSrcScale with the same type.
	 */
	virtual void setSource(byte *src, uint pitch, int width, int height, int padding, SourceType type) {
		// Should not be called unless overriden
		assert(0);
	}

	/**
	 * Scale using the source from setSource called with the same value as type.
	 * The source will be compared against previous frames to avoid computations
	 * on unchanged pixels.
	 *
	 * @param type    The surface type set previously with setSource
	 */
	virtual void oldSrcScale(byte *dst, uint dstPitch, SourceType type) {
		// Should not be called unless overriden
		assert(0);
	}

protected:
	uint _factor;
	Common::Array<uint> _factors;
	Graphics::PixelFormat _format;
};

/**
 * Convenience class that implements some bookkeeping for keeping track of
 * old source images.
 */
class SourceScaler : public ScalerPluginObject {

public:

	SourceScaler();

	virtual void setSource(byte *src, uint pitch, int width, int height, int padding, SourceType type);
	virtual void oldSrcScale(byte *dst, uint dstPitch, SourceType type);

protected:

	/**
	 * Scalers must implement this function. It will be called by oldSrcScale.
	 * If by comparing the src and oldsrc images it is discovered that no change
	 * is necessary, do not write a pixel.
	 */
	virtual void internScale(const uint8 *srcPtr, uint32 srcPitch,
	                         uint8 *dstPtr, uint32 dstPitch,
	                         const uint8 *oldSrcPtr, uint32 oldSrcPitch,
	                         int width, int height) = 0;

	int widths[SRC_MAX];
	int heights[SRC_MAX];
	uint paddings[SRC_MAX];
	uint pitches[SRC_MAX];

	byte *newSrcs[SRC_MAX];
	byte *oldSrcs[SRC_MAX];
};

typedef PluginSubclass<ScalerPluginObject> ScalerPlugin;

/**
 * Singleton class to manage scaler plugins
 */
class ScalerManager : public Common::Singleton<ScalerManager> {
private:
	friend class Common::Singleton<SingletonBaseType>;

public:
	const ScalerPlugin::List &getPlugins() const;

	/**
	 * Queries all scaler plugins for the maximum number of pixels they
	 * can access out of bounds. Useful for adding extra rows and columns
	 * to surfaces.
	 */
	uint getMaxExtraPixels() const;
};

/** Convenience shortcut for accessing singleton */
#define ScalerMan ScalerManager::instance()

#endif
