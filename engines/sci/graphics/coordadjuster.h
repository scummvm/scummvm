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

#ifndef SCI_GRAPHICS_COORDADJUSTER_H
#define SCI_GRAPHICS_COORDADJUSTER_H

#include "common/list.h"
#include "common/array.h"

namespace Sci {

class GfxPorts;

/**
 * CoordAdjuster class, does coordinate adjustment as need by various functions
 *  most of the time sci32 doesn't do any coordinate adjustment at all
 *  sci16 does a lot of port adjustment on given coordinates
 */
class GfxCoordAdjuster {
public:
	GfxCoordAdjuster();
	virtual ~GfxCoordAdjuster() { }

	virtual void kernelGlobalToLocal(int16 &x, int16 &y, reg_t planeObject = NULL_REG) { }
	virtual void kernelLocalToGlobal(int16 &x, int16 &y, reg_t planeObject = NULL_REG) { }

	virtual Common::Rect onControl(Common::Rect rect) { return rect; }
	virtual void setCursorPos(Common::Point &pos) { }
	virtual void moveCursor(Common::Point &pos) { }

	virtual void setScriptsResolution(uint16 width, uint16 height) { }
	virtual void fromScriptToDisplay(int16 &y, int16 &x) { }
	virtual void fromDisplayToScript(int16 &y, int16 &x) { }

	virtual Common::Rect pictureGetDisplayArea() { return Common::Rect(0, 0); }
private:
};

class GfxCoordAdjuster16 : public GfxCoordAdjuster {
public:
	GfxCoordAdjuster16(GfxPorts *ports);
	~GfxCoordAdjuster16();

	void kernelGlobalToLocal(int16 &x, int16 &y, reg_t planeObject = NULL_REG) override;
	void kernelLocalToGlobal(int16 &x, int16 &y, reg_t planeObject = NULL_REG) override;

	Common::Rect onControl(Common::Rect rect) override;
	void setCursorPos(Common::Point &pos) override;
	void moveCursor(Common::Point &pos) override;

	Common::Rect pictureGetDisplayArea() override;

private:
	GfxPorts *_ports;
};

#ifdef ENABLE_SCI32
class GfxCoordAdjuster32 : public GfxCoordAdjuster {
public:
	GfxCoordAdjuster32(SegManager *segMan);
	~GfxCoordAdjuster32();

	void kernelGlobalToLocal(int16 &x, int16 &y, reg_t planeObject = NULL_REG) override;
	void kernelLocalToGlobal(int16 &x, int16 &y, reg_t planeObject = NULL_REG) override;

	void setScriptsResolution(uint16 width, uint16 height) override;
	void fromScriptToDisplay(int16 &y, int16 &x) override;
	void fromDisplayToScript(int16 &y, int16 &x) override;

	void pictureSetDisplayArea(Common::Rect displayArea);
	Common::Rect pictureGetDisplayArea() override;

private:
	SegManager *_segMan;

	Common::Rect _pictureDisplayArea;

	uint16 _scriptsRunningWidth;
	uint16 _scriptsRunningHeight;
};
#endif

} // End of namespace Sci

#endif
