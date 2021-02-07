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

class ScalerPluginObject : public PluginObject {
public:

	virtual ~ScalerPluginObject() {}

	/**
	 * This function will be called before any scaler is used.
	 * Precomputed data should be generated here.
	 * @param format The pixel format to scale.
	 */
	virtual void initialize(const Graphics::PixelFormat &format) = 0;

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

	virtual const Common::Array<uint> &getFactors() const { return _factors; }

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
	 * optionally use it.
	 *
	 * @see enableSource
	 * @see setSource
	 */
	virtual bool useOldSource() const { return false; }

	/**
	 * Set the source to be used when scaling and copying to the old buffer.
	 *
	 * @param padding The number of pixels on the border (Used to prevent memory access crashes)
	 * @param type    The surface type. This source will only be used when calling oldSrcScale with the same type.
	 */
	virtual void setSource(const byte *src, uint pitch, int width, int height, int padding) {
		// Should not be called unless overriden
		assert(0);
	}

	/**
	 * Enable or disable the old Source functionality. It is initially
	 * disabled. When disabled, the old source data is preserved until re-enabled.
	 *
	 * Useful for scaling a different surface (e.g. the cursor).
	 */
	virtual void enableSource(bool enable) {
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
	virtual ~SourceScaler();

	virtual void scale(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
	                   uint32 dstPitch, int width, int height, int x, int y);

	virtual void setSource(const byte *src, uint pitch, int width, int height, int padding);

	virtual void enableSource(bool enable) { _enable = enable; }

protected:

	/**
	 * Scalers must implement this function. It will be called by oldSrcScale.
	 * If by comparing the src and oldsrc images it is discovered that no change
	 * is necessary, do not write a pixel.
	 *
	 * If oldSrcPtr is NULL, do not read from it. Scale every pixel.
	 */
	virtual void internScale(const uint8 *srcPtr, uint32 srcPitch,
	                         uint8 *dstPtr, uint32 dstPitch,
	                         const uint8 *oldSrcPtr, uint32 oldSrcPitch,
	                         int width, int height) = 0;


private:

	int _padding;
	bool _enable;
	byte *_oldSrc;
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
