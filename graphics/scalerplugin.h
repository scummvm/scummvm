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

	ScalerPluginObject();

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
	virtual int increaseFactor() = 0;

	/**
	 * Decrease the factor of scaling.
	 * @return The new factor
	 */
	virtual int decreaseFactor() = 0;

	virtual int getFactor() = 0;

	/**
	 * Indicates how far outside the scaling region this scaler "looks"
	 * @return The number of pixels in any direction
	 */
	virtual int extraPixels() = 0;

	/**
	 * Some scalers are not suitable for scaling the cursor.
	 * Blurring scalers should return false.
	 */
	virtual bool canDrawCursor() = 0;

	// temporary HACK
	virtual void disableScaling();

	virtual void enableScaling();

protected:
	int _factor;
	bool _doScale; // < temporary
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
};

/** Convenience shortcut for accessing singleton */
#define ScalerMan ScalerManager::instance()

#endif
