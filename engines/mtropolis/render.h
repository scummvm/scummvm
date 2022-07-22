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

#ifndef MTROPOLIS_RENDER_H
#define MTROPOLIS_RENDER_H

#include "common/events.h"
#include "common/ptr.h"
#include "common/scummsys.h"

#include "graphics/pixelformat.h"

namespace Graphics {

class ManagedSurface;
struct Surface;

} // End of namespace Graphics

namespace MTropolis {

class CursorGraphic;
class Runtime;
class Project;
struct SceneTransitionEffect;

enum TextAlignment {
	kTextAlignmentLeft,
	kTextAlignmentCenter,
	kTextAlignmentRight,
};

struct TextStyleFlags {
	bool bold : 1;
	bool italic : 1;
	bool underline : 1;
	bool outline : 1;
	bool shadow : 1;
	bool condensed : 1;
	bool expanded : 1;

	TextStyleFlags();
	bool load(uint8 dataStyleFlags);
};

struct MacFontFormatting {
	MacFontFormatting();
	MacFontFormatting(uint16 mff_fontID, uint8 mff_fontFlags, uint16 mff_size);

	uint16 fontID;
	uint8 fontFlags;
	uint16 size;
};

struct MacFormattingSpan {
	MacFormattingSpan();

	uint16 spanStart;
	MacFontFormatting formatting;
};

struct WindowParameters {
	Runtime *runtime;
	int32 x;
	int32 y;
	int16 width;
	int16 height;
	const Graphics::PixelFormat format;

	WindowParameters(Runtime *wp_runtime, int32 wp_x, int32 wp_y, int16 wp_width, int16 wp_height, const Graphics::PixelFormat &wp_format);
};

class Window {
public:
	explicit Window(const WindowParameters &windowParams);
	virtual ~Window();

	int32 getX() const;
	int32 getY() const;
	int32 getWidth() const;
	int32 getHeight() const;
	void setPosition(int32 x, int32 y);
	void resizeWindow(int32 width, int32 height);	// Destroys contents

	const Common::SharedPtr<Graphics::ManagedSurface> &getSurface() const;
	const Graphics::PixelFormat &getPixelFormat() const;

	const Common::SharedPtr<CursorGraphic> &getCursorGraphic() const;
	void setCursorGraphic(const Common::SharedPtr<CursorGraphic> &cursor);

	void setStrata(int strata);
	int getStrata() const;

	// Mouse transparency = ignores mouse events
	void setMouseTransparent(bool isTransparent);
	bool isMouseTransparent() const;

	void close();
	void detachFromRuntime();

	virtual void onMouseDown(int32 x, int32 y, int mouseButton);
	virtual void onMouseMove(int32 x, int32 y);
	virtual void onMouseUp(int32 x, int32 y, int mouseButton);
	virtual void onKeyboardEvent(const Common::EventType evtType, bool repeat, const Common::KeyState &keyEvt);

protected:
	int32 _x;
	int32 _y;
	Runtime *_runtime;
	int _strata;
	bool _isMouseTransparent;

	Common::SharedPtr<Graphics::ManagedSurface> _surface;
	Common::SharedPtr<CursorGraphic> _cursor;
};

namespace Render {

uint32 resolveRGB(uint8 r, uint8 g, uint8 b, const Graphics::PixelFormat &fmt);
void renderProject(Runtime *runtime, Window *mainWindow);
void renderSceneTransition(Runtime *runtime, Window *mainWindow, const SceneTransitionEffect &effect, uint32 startTime, uint32 endTime, uint32 currentTime, const Graphics::ManagedSurface &oldFrame, const Graphics::ManagedSurface &newFrame);

void convert32To16(Graphics::Surface &destSurface, const Graphics::Surface &srcSurface);
void convert16To32(Graphics::Surface &destSurface, const Graphics::Surface &srcSurface);

} // End of namespace Render

} // End of namespace MTropolis

#endif
