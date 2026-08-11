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

#ifndef AGS_ENGINE_MAIN_GRAPHICS_MODE_H
#define AGS_ENGINE_MAIN_GRAPHICS_MODE_H

#include "ags/engine/gfx/gfx_defines.h"
#include "ags/shared/util/geometry.h"
#include "ags/shared/util/scaling.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using AGS::Shared::String;
using AGS::Engine::GraphicResolution;
using AGS::Engine::DisplayMode;
using AGS::Engine::WindowMode;

Size get_desktop_size();

namespace AGS {
namespace Engine {
class IGfxModeList;
} // namespace Engine
} // namespace AGS

bool find_nearest_supported_mode(const AGS::Engine::IGfxModeList &modes, const Size &wanted_size,
                                 const int color_depth, const Size *ratio_reference, const Size *upper_bound,
                                 AGS::Engine::DisplayMode &dm, int *mode_index = nullptr);

// Filter configuration
struct GfxFilterSetup {
	String ID;          // internal filter ID
	String UserRequest; // filter name, requested by user
};

// Defines how game frame is scaled inside a larger window
enum FrameScaleDef {
	kFrame_Undefined = -1,
	kFrame_Round,        // max round (integer) scaling factor
	kFrame_Stretch,      // resize to maximal possible inside the display box
	kFrame_Proportional, // same as stretch, but keep game's aspect ratio
	kNumFrameScaleDef
};

// Configuration that is used to determine the size and style of the window
struct WindowSetup
{
    AGS3::Size           Size;      // explicit screen metrics
    int                  Scale = 0; // explicit game scale factor
    WindowMode           Mode = AGS::Engine::kWnd_Windowed; // window mode

    WindowSetup() = default;
    WindowSetup(const AGS3::Size &sz, WindowMode mode = AGS::Engine::kWnd_Windowed)
        : Size(sz), Scale(0), Mode(mode) {}
    WindowSetup(int scale, WindowMode mode = AGS::Engine::kWnd_Windowed)
        : Scale(scale), Mode(mode) {}
    WindowSetup(WindowMode mode) : Scale(0), Mode(mode) {}
};

// Additional parameters for the display mode setup
struct DisplaySetupEx {
	int                  RefreshRate = 0;  // gfx mode refresh rate
	bool                 VSync = false;    // vertical sync
};

// Full graphics configuration, contains graphics driver selection,
// alternate settings for windowed and fullscreen modes and gfx filter setup.
struct DisplayModeSetup {
	String               DriverID;      // graphics driver ID

	// Definitions for the fullscreen and windowed modes and scaling methods.
	// When the initial display mode is set, corresponding scaling method from this pair is used.
	// The second method is meant to be saved and used if display mode is switched at runtime.
	WindowSetup          FsSetup;       // definition of the fullscreen mode
	WindowSetup          WinSetup;      // definition of the windowed mode
	FrameScaleDef        FsGameFrame =  // how the game frame should be scaled/positioned in fullscreen mode
		kFrame_Undefined;
	FrameScaleDef        WinGameFrame = // how the game frame should be scaled/positioned in windowed mode
		kFrame_Undefined;

	bool                 Windowed = false; // initial mode
	DisplaySetupEx       Params;

	GfxFilterSetup       Filter;        // graphics filter definition
};

// Display mode color depth variants suggested for the use
struct ColorDepthOption {
	int     Bits;   // color depth value in bits
	bool    Forced; // whether the depth should be forced, or driver's recommendation used

	ColorDepthOption() : Bits(0), Forced(false) {
	}
	ColorDepthOption(int bits, bool forced = false) : Bits(bits), Forced(forced) {
	}
};

// ActiveDisplaySetting struct merges DisplayMode and FrameScaleDef,
// which is useful if you need to save active settings and reapply them later.
struct ActiveDisplaySetting {
	DisplayMode	  Dm;
	FrameScaleDef Frame = kFrame_Undefined;
	int			  DisplayIndex = -1;
};

// Initializes any possible gfx mode, using user config as a recommendation;
// may try all available renderers and modes before succeeding (or failing)
bool graphics_mode_init_any(const GraphicResolution &game_res, const DisplayModeSetup &setup, const ColorDepthOption &color_depth);
// Return last saved display mode of the given kind
ActiveDisplaySetting graphics_mode_get_last_setting(bool windowed);
// Creates graphics driver of given id
bool graphics_mode_create_renderer(const String &driver_id);
// Try to find and initialize compatible display mode as close to given setup as possible
bool graphics_mode_set_dm_any(const Size &game_size, const WindowSetup &ws,
	const ColorDepthOption &color_depth,
	const FrameScaleDef frame, const DisplaySetupEx &params);
// Set the display mode with given parameters
bool graphics_mode_set_dm(const AGS::Engine::DisplayMode &dm);
// Set the native image size
bool graphics_mode_set_native_res(const GraphicResolution &native_res);
// Get current render frame setup
FrameScaleDef graphics_mode_get_render_frame();
// Set the render frame position inside the window
bool graphics_mode_set_render_frame(const FrameScaleDef &frame_setup);
// Set requested graphics filter, or default filter if the requested one failed
bool graphics_mode_set_filter_any(const GfxFilterSetup &setup);
// Set the scaling filter with given ID
bool graphics_mode_set_filter(const String &filter_id);
// Update graphic renderer and render frame when window size changes
void graphics_mode_on_window_changed(const Size &sz);
// Releases current graphic mode and shuts down renderer
void graphics_mode_shutdown();

} // namespace AGS3

#endif
