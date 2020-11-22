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

//
// Graphics initialization
//

#include <algorithm>
#include "core/platform.h"
#include "ac/draw.h"
#include "debug/debugger.h"
#include "debug/out.h"
#include "gfx/ali3dexception.h"
#include "gfx/bitmap.h"
#include "gfx/gfxdriverfactory.h"
#include "gfx/gfxfilter.h"
#include "gfx/graphicsdriver.h"
#include "main/config.h"
#include "main/engine_setup.h"
#include "main/graphics_mode.h"
#include "main/main_allegro.h"
#include "platform/base/agsplatformdriver.h"

namespace AGS3 {

// Don't try to figure out the window size on the mac because the port resizes itself.
#if AGS_PLATFORM_OS_MACOS || defined(ALLEGRO_SDL2) || AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID
#define USE_SIMPLE_GFX_INIT
#endif

using namespace AGS::Shared;
using namespace AGS::Engine;

extern int proper_exit;
extern AGSPlatformDriver *platform;
extern IGraphicsDriver *gfxDriver;


IGfxDriverFactory *GfxFactory = nullptr;

// Last saved fullscreen and windowed configs; they are used when switching
// between between fullscreen and windowed modes at runtime.
// If particular mode is modified, e.g. by script command, related config should be overwritten.
ActiveDisplaySetting SavedFullscreenSetting;
ActiveDisplaySetting SavedWindowedSetting;
// Current frame scaling setup
GameFrameSetup     CurFrameSetup;
// The game-to-screen transformation
PlaneScaling       GameScaling;


GameFrameSetup::GameFrameSetup()
	: ScaleDef(kFrame_IntScale)
	, ScaleFactor(1) {
}

GameFrameSetup::GameFrameSetup(FrameScaleDefinition def, int factor)
	: ScaleDef(def)
	, ScaleFactor(factor) {
}

bool GameFrameSetup::IsValid() const {
	return ScaleDef != kFrame_IntScale || ScaleFactor > 0;
}

ScreenSizeSetup::ScreenSizeSetup()
	: SizeDef(kScreenDef_MaxDisplay)
	, MatchDeviceRatio(true) {
}

DisplayModeSetup::DisplayModeSetup()
	: RefreshRate(0)
	, VSync(false)
	, Windowed(false) {
}


Size get_desktop_size() {
	Size sz;
	get_desktop_resolution(&sz.Width, &sz.Height);
	return sz;
}

Size get_max_display_size(bool windowed) {
	Size device_size = get_desktop_size();
	if (windowed)
		platform->ValidateWindowSize(device_size.Width, device_size.Height, false);
	return device_size;
}

bool create_gfx_driver(const String &gfx_driver_id) {
	GfxFactory = GetGfxDriverFactory(gfx_driver_id);
	if (!GfxFactory) {
		Debug::Printf(kDbgMsg_Error, "Failed to initialize %s graphics factory. Error: %s", gfx_driver_id.GetCStr(), get_allegro_error());
		return false;
	}
	Debug::Printf("Using graphics factory: %s", gfx_driver_id.GetCStr());
	gfxDriver = GfxFactory->GetDriver();
	if (!gfxDriver) {
		Debug::Printf(kDbgMsg_Error, "Failed to create graphics driver. Error: %s", get_allegro_error());
		return false;
	}
	Debug::Printf("Created graphics driver: %s", gfxDriver->GetDriverName());
	return true;
}

// Set requested graphics filter, or default filter if the requested one failed
bool graphics_mode_set_filter_any(const GfxFilterSetup &setup) {
	Debug::Printf("Requested gfx filter: %s", setup.UserRequest.GetCStr());
	if (!graphics_mode_set_filter(setup.ID)) {
		String def_filter = GfxFactory->GetDefaultFilterID();
		if (def_filter.CompareNoCase(setup.ID) == 0)
			return false;
		Debug::Printf(kDbgMsg_Error, "Failed to apply gfx filter: %s; will try to use factory default filter '%s' instead",
			setup.UserRequest.GetCStr(), def_filter.GetCStr());
		if (!graphics_mode_set_filter(def_filter))
			return false;
	}
	Debug::Printf("Using gfx filter: %s", GfxFactory->GetDriver()->GetGraphicsFilter()->GetInfo().Id.GetCStr());
	return true;
}

bool find_nearest_supported_mode(const IGfxModeList &modes, const Size &wanted_size, const int color_depth,
	const Size *ratio_reference, const Size *upper_bound, DisplayMode &dm, int *mode_index) {
	uint32_t wanted_ratio = 0;
	if (ratio_reference && !ratio_reference->IsNull()) {
		wanted_ratio = (ratio_reference->Height << kShift) / ratio_reference->Width;
	}

	int nearest_width = 0;
	int nearest_height = 0;
	int nearest_width_diff = 0;
	int nearest_height_diff = 0;
	DisplayMode nearest_mode;
	int nearest_mode_index = -1;
	int mode_count = modes.GetModeCount();
	for (int i = 0; i < mode_count; ++i) {
		DisplayMode mode;
		if (!modes.GetMode(i, mode)) {
			continue;
		}
		if (mode.ColorDepth != color_depth) {
			continue;
		}
		if (wanted_ratio > 0) {
			uint32_t mode_ratio = (mode.Height << kShift) / mode.Width;
			if (mode_ratio != wanted_ratio) {
				continue;
			}
		}
		if (upper_bound && (mode.Width > upper_bound->Width || mode.Height > upper_bound->Height))
			continue;
		if (mode.Width == wanted_size.Width && mode.Height == wanted_size.Height) {
			nearest_width = mode.Width;
			nearest_height = mode.Height;
			nearest_mode_index = i;
			nearest_mode = mode;
			break;
		}

		int diff_w = abs(wanted_size.Width - mode.Width);
		int diff_h = abs(wanted_size.Height - mode.Height);
		bool same_diff_w_higher = (diff_w == nearest_width_diff && nearest_width < wanted_size.Width);
		bool same_diff_h_higher = (diff_h == nearest_height_diff && nearest_height < wanted_size.Height);

		if (nearest_width == 0 ||
			((diff_w < nearest_width_diff || same_diff_w_higher) && diff_h <= nearest_height_diff) ||
			((diff_h < nearest_height_diff || same_diff_h_higher) && diff_w <= nearest_width_diff)) {
			nearest_width = mode.Width;
			nearest_width_diff = diff_w;
			nearest_height = mode.Height;
			nearest_height_diff = diff_h;
			nearest_mode = mode;
			nearest_mode_index = i;
		}
	}

	if (nearest_width > 0 && nearest_height > 0) {
		dm = nearest_mode;
		if (mode_index)
			*mode_index = nearest_mode_index;
		return true;
	}
	return false;
}

Size set_game_frame_after_screen_size(const Size &game_size, const Size screen_size, const GameFrameSetup &setup) {
	// Set game frame as native game resolution scaled by particular method
	Size frame_size;
	if (setup.ScaleDef == kFrame_MaxStretch) {
		frame_size = screen_size;
	} else if (setup.ScaleDef == kFrame_MaxProportional) {
		frame_size = ProportionalStretch(screen_size, game_size);
	} else {
		int scale;
		if (setup.ScaleDef == kFrame_MaxRound)
			scale = Math::Min((screen_size.Width / game_size.Width) << kShift,
			(screen_size.Height / game_size.Height) << kShift);
		else
			scale = convert_scaling_to_fp(setup.ScaleFactor);

		// Ensure scaling factors are sane
		if (scale <= 0)
			scale = kUnit;

		frame_size = Size((game_size.Width * scale) >> kShift, (game_size.Height * scale) >> kShift);
		// If the scaled game size appear larger than the screen,
		// use "proportional stretch" method instead
		if (frame_size.ExceedsByAny(screen_size))
			frame_size = ProportionalStretch(screen_size, game_size);
	}
	return frame_size;
}

Size precalc_screen_size(const Size &game_size, const DisplayModeSetup &dm_setup, const GameFrameSetup &frame_setup) {
	Size screen_size, frame_size;
	Size device_size = get_max_display_size(dm_setup.Windowed);

	// Set requested screen (window) size, depending on screen definition option
	ScreenSizeSetup scsz = dm_setup.ScreenSize;
	switch (scsz.SizeDef) {
	case kScreenDef_Explicit:
		// Use resolution from user config
		screen_size = scsz.Size;
		if (screen_size.IsNull()) {
			// If the configuration did not define proper screen size,
			// use the scaled game size instead
			frame_size = set_game_frame_after_screen_size(game_size, device_size, frame_setup);
			if (screen_size.Width <= 0)
				screen_size.Width = frame_size.Width;
			if (screen_size.Height <= 0)
				screen_size.Height = frame_size.Height;
		}
		break;
	case kScreenDef_ByGameScaling:
		// Use game frame (scaled game) size
		frame_size = set_game_frame_after_screen_size(game_size, device_size, frame_setup);
		screen_size = frame_size;
		break;
	case kScreenDef_MaxDisplay:
		// Set as big as current device size
		screen_size = device_size;
		break;
	}
	return screen_size;
}

// Find closest possible compatible display mode and initialize it
bool try_init_compatible_mode(const DisplayMode &dm, const bool match_device_ratio) {
	const Size &screen_size = Size(dm.Width, dm.Height);
	// Find nearest compatible mode and init that
	Debug::Printf("Attempting to find nearest supported resolution for screen size %d x %d (%d-bit) %s",
		dm.Width, dm.Height, dm.ColorDepth, dm.Windowed ? "windowed" : "fullscreen");
	const Size device_size = get_max_display_size(dm.Windowed);
	if (dm.Windowed)
		Debug::Printf("Maximal allowed window size: %d x %d", device_size.Width, device_size.Height);
	DisplayMode dm_compat = dm;

	std::unique_ptr<IGfxModeList> modes(gfxDriver->GetSupportedModeList(dm.ColorDepth));  // TODO: use unique_ptr when available

	// Windowed mode
	if (dm.Windowed) {
		// If windowed mode, make the resolution stay in the generally supported limits
		dm_compat.Width = Math::Min(dm_compat.Width, device_size.Width);
		dm_compat.Height = Math::Min(dm_compat.Height, device_size.Height);
	}
	// Fullscreen mode
	else {
		// If told to find mode with aspect ratio matching current desktop resolution, then first
		// try find matching one, and if failed then try any compatible one
		bool mode_found = false;
		if (modes.get()) {
			if (match_device_ratio)
				mode_found = find_nearest_supported_mode(*modes.get(), screen_size, dm.ColorDepth, &device_size, nullptr, dm_compat);
			if (!mode_found)
				mode_found = find_nearest_supported_mode(*modes.get(), screen_size, dm.ColorDepth, nullptr, nullptr, dm_compat);
		}
		if (!mode_found)
			Debug::Printf("Could not find compatible fullscreen mode. Will try to force-set mode requested by user and fallback to windowed mode if that fails.");
		dm_compat.Vsync = dm.Vsync;
		dm_compat.Windowed = false;
	}

	bool result = graphics_mode_set_dm(dm_compat);
	if (!result && dm.Windowed) {
		// When initializing windowed mode we could start with any random window size;
		// if that did not work, try to find nearest supported mode, as with fullscreen mode,
		// except refering to max window size as an upper bound
		if (find_nearest_supported_mode(*modes.get(), screen_size, dm.ColorDepth, nullptr, &device_size, dm_compat)) {
			dm_compat.Vsync = dm.Vsync;
			dm_compat.Windowed = true;
			result = graphics_mode_set_dm(dm_compat);
		}
	}
	return result;
}

// Try to find and initialize compatible display mode as close to given setup as possible
bool try_init_mode_using_setup(const Size &game_size, const DisplayModeSetup &dm_setup,
	const int col_depth, const GameFrameSetup &frame_setup,
	const GfxFilterSetup &filter_setup) {
	// We determine the requested size of the screen using setup options
	const Size screen_size = precalc_screen_size(game_size, dm_setup, frame_setup);
	DisplayMode dm(GraphicResolution(screen_size.Width, screen_size.Height, col_depth),
		dm_setup.Windowed, dm_setup.RefreshRate, dm_setup.VSync);
	if (!try_init_compatible_mode(dm, dm_setup.ScreenSize.SizeDef == kScreenDef_Explicit ? false : dm_setup.ScreenSize.MatchDeviceRatio))
		return false;

	// Set up native size and render frame
	if (!graphics_mode_set_native_size(game_size) || !graphics_mode_set_render_frame(frame_setup))
		return false;

	// Set up graphics filter
	if (!graphics_mode_set_filter_any(filter_setup))
		return false;
	return true;
}

void log_out_driver_modes(const int color_depth) {
	IGfxModeList *modes = gfxDriver->GetSupportedModeList(color_depth);
	if (!modes) {
		Debug::Printf(kDbgMsg_Error, "Couldn't get a list of supported resolutions for color depth = %d", color_depth);
		return;
	}
	const int mode_count = modes->GetModeCount();
	DisplayMode mode;
	String mode_str;
	for (int i = 0, in_str = 0; i < mode_count; ++i) {
		if (!modes->GetMode(i, mode) || mode.ColorDepth != color_depth)
			continue;
		mode_str.Append(String::FromFormat("%dx%d;", mode.Width, mode.Height));
		if (++in_str % 8 == 0)
			mode_str.Append("\n\t");
	}
	delete modes;

	String out_str = String::FromFormat("Supported gfx modes (%d-bit): ", color_depth);
	if (!mode_str.IsEmpty()) {
		out_str.Append("\n\t");
		out_str.Append(mode_str);
	} else
		out_str.Append("none");
	Debug::Printf(out_str);
}

// Create requested graphics driver and try to find and initialize compatible display mode as close to user setup as possible;
// if the given setup fails, gets default setup for the opposite type of mode (fullscreen/windowed) and tries that instead.
bool create_gfx_driver_and_init_mode_any(const String &gfx_driver_id, const Size &game_size, const DisplayModeSetup &dm_setup,
	const ColorDepthOption &color_depth, const GameFrameSetup &frame_setup, const GfxFilterSetup &filter_setup) {
	if (!graphics_mode_create_renderer(gfx_driver_id))
		return false;

	const int use_col_depth =
		color_depth.Forced ? color_depth.Bits : gfxDriver->GetDisplayDepthForNativeDepth(color_depth.Bits);
	// Log out supported driver modes
	log_out_driver_modes(use_col_depth);

	bool result = try_init_mode_using_setup(game_size, dm_setup, use_col_depth, frame_setup, filter_setup);
	// Try windowed mode if fullscreen failed, and vice versa
	if (!result && editor_debugging_enabled == 0) {
		// we need to clone from initial config, because not every parameter is set by graphics_mode_get_defaults()
		DisplayModeSetup dm_setup_alt = dm_setup;
		dm_setup_alt.Windowed = !dm_setup.Windowed;
		GameFrameSetup frame_setup_alt;
		graphics_mode_get_defaults(dm_setup_alt.Windowed, dm_setup_alt.ScreenSize, frame_setup_alt);
		result = try_init_mode_using_setup(game_size, dm_setup_alt, use_col_depth, frame_setup_alt, filter_setup);
	}
	return result;
}

bool simple_create_gfx_driver_and_init_mode(const String &gfx_driver_id,
	const Size &game_size,
	const DisplayModeSetup &dm_setup,
	const ColorDepthOption &color_depth,
	const GameFrameSetup &frame_setup,
	const GfxFilterSetup &filter_setup) {
	if (!graphics_mode_create_renderer(gfx_driver_id)) {
		return false;
	}

	const int col_depth = gfxDriver->GetDisplayDepthForNativeDepth(color_depth.Bits);

	DisplayMode dm(GraphicResolution(game_size.Width, game_size.Height, col_depth),
		dm_setup.Windowed, dm_setup.RefreshRate, dm_setup.VSync);

	if (!graphics_mode_set_dm(dm)) {
		return false;
	}
	if (!graphics_mode_set_native_size(game_size)) {
		return false;
	}
	if (!graphics_mode_set_render_frame(frame_setup)) {
		return false;
	}
	if (!graphics_mode_set_filter_any(filter_setup)) {
		return false;
	}

	return true;
}


void display_gfx_mode_error(const Size &game_size, const ScreenSetup &setup, const int color_depth) {
	proper_exit = 1;
	platform->FinishedUsingGraphicsMode();

	String main_error;
	ScreenSizeSetup scsz = setup.DisplayMode.ScreenSize;
	PGfxFilter filter = gfxDriver ? gfxDriver->GetGraphicsFilter() : PGfxFilter();
	Size wanted_screen;
	if (scsz.SizeDef == kScreenDef_Explicit)
		main_error.Format("There was a problem initializing graphics mode %d x %d (%d-bit), or finding nearest compatible mode, with game size %d x %d and filter '%s'.",
			scsz.Size.Width, scsz.Size.Height, color_depth, game_size.Width, game_size.Height, filter ? filter->GetInfo().Id.GetCStr() : "Undefined");
	else
		main_error.Format("There was a problem finding and/or creating valid graphics mode for game size %d x %d (%d-bit) and requested filter '%s'.",
			game_size.Width, game_size.Height, color_depth, setup.Filter.UserRequest.IsEmpty() ? "Undefined" : setup.Filter.UserRequest.GetCStr());

	platform->DisplayAlert("%s\n"
		"(Problem: '%s')\n"
		"Try to correct the problem, or seek help from the AGS homepage."
		"%s",
		main_error.GetCStr(), get_allegro_error(), platform->GetGraphicsTroubleshootingText());
}

bool graphics_mode_init_any(const Size game_size, const ScreenSetup &setup, const ColorDepthOption &color_depth) {
	// Log out display information
	Size device_size;
	if (get_desktop_resolution(&device_size.Width, &device_size.Height) == 0)
		Debug::Printf("Device display resolution: %d x %d", device_size.Width, device_size.Height);
	else
		Debug::Printf(kDbgMsg_Error, "Unable to obtain device resolution");

	const char *screen_sz_def_options[kNumScreenDef] = { "explicit", "scaling", "max" };
	ScreenSizeSetup scsz = setup.DisplayMode.ScreenSize;
	const bool ignore_device_ratio = setup.DisplayMode.Windowed || scsz.SizeDef == kScreenDef_Explicit;
	GameFrameSetup gameframe = setup.DisplayMode.Windowed ? setup.WinGameFrame : setup.FsGameFrame;
	const String scale_option = make_scaling_option(gameframe);
	Debug::Printf(kDbgMsg_Info, "Graphic settings: driver: %s, windowed: %s, screen def: %s, screen size: %d x %d, match device ratio: %s, game scale: %s",
		setup.DriverID.GetCStr(),
		setup.DisplayMode.Windowed ? "yes" : "no", screen_sz_def_options[scsz.SizeDef],
		scsz.Size.Width, scsz.Size.Height,
		ignore_device_ratio ? "ignore" : (scsz.MatchDeviceRatio ? "yes" : "no"), scale_option.GetCStr());

	// Prepare the list of available gfx factories, having the one requested by user at first place
	// TODO: make factory & driver IDs case-insensitive!
	StringV ids;
	GetGfxDriverFactoryNames(ids);
	StringV::iterator it = ids.begin();
	for (; it != ids.end(); ++it) {
		if (it->CompareNoCase(setup.DriverID) == 0) break;
	}
	if (it != ids.end())
		std::rotate(ids.begin(), it, ids.end());
	else
		Debug::Printf(kDbgMsg_Error, "Requested graphics driver '%s' not found, will try existing drivers instead", setup.DriverID.GetCStr());

	// Try to create renderer and init gfx mode, choosing one factory at a time
	bool result = false;
	for (StringV::const_iterator it = ids.begin(); it != ids.end(); ++it) {
		result =
#ifdef USE_SIMPLE_GFX_INIT
			simple_create_gfx_driver_and_init_mode
#else
			create_gfx_driver_and_init_mode_any
#endif
			(*it, game_size, setup.DisplayMode, color_depth, gameframe, setup.Filter);

		if (result)
			break;
		graphics_mode_shutdown();
	}
	// If all possibilities failed, display error message and quit
	if (!result) {
		display_gfx_mode_error(game_size, setup, color_depth.Bits);
		return false;
	}
	return true;
}

ActiveDisplaySetting graphics_mode_get_last_setting(bool windowed) {
	return windowed ? SavedWindowedSetting : SavedFullscreenSetting;
}

bool graphics_mode_update_render_frame();
void GfxDriverOnSurfaceUpdate() {
	// Resize render frame using current scaling settings
	graphics_mode_update_render_frame();
	on_coordinates_scaling_changed();
}

bool graphics_mode_create_renderer(const String &driver_id) {
	if (!create_gfx_driver(driver_id))
		return false;

	gfxDriver->SetCallbackOnInit(GfxDriverOnInitCallback);
	gfxDriver->SetCallbackOnSurfaceUpdate(GfxDriverOnSurfaceUpdate);
	// TODO: this is remains of the old code; find out if this is really
	// the best time and place to set the tint method
	gfxDriver->SetTintMethod(TintReColourise);
	return true;
}

bool graphics_mode_set_dm_any(const Size &game_size, const DisplayModeSetup &dm_setup,
	const ColorDepthOption &color_depth, const GameFrameSetup &frame_setup) {
	// We determine the requested size of the screen using setup options
	const Size screen_size = precalc_screen_size(game_size, dm_setup, frame_setup);
	DisplayMode dm(GraphicResolution(screen_size.Width, screen_size.Height, color_depth.Bits),
		dm_setup.Windowed, dm_setup.RefreshRate, dm_setup.VSync);
	return try_init_compatible_mode(dm, dm_setup.ScreenSize.MatchDeviceRatio);
}

bool graphics_mode_set_dm(const DisplayMode &dm) {
	Debug::Printf("Attempt to switch gfx mode to %d x %d (%d-bit) %s",
		dm.Width, dm.Height, dm.ColorDepth, dm.Windowed ? "windowed" : "fullscreen");

	// Tell Allegro new default bitmap color depth (must be done before set_gfx_mode)
	// TODO: this is also done inside ALSoftwareGraphicsDriver implementation; can remove one?
	set_color_depth(dm.ColorDepth);
	// TODO: this is remains of the old code; find out what it means and do we
	// need this if we are not using allegro software driver?
	if (dm.RefreshRate >= 50)
		request_refresh_rate(dm.RefreshRate);

	if (!gfxDriver->SetDisplayMode(dm, nullptr)) {
		Debug::Printf(kDbgMsg_Error, "Failed to init gfx mode. Error: %s", get_allegro_error());
		return false;
	}

	DisplayMode rdm = gfxDriver->GetDisplayMode();
	if (rdm.Windowed)
		SavedWindowedSetting.Dm = rdm;
	else
		SavedFullscreenSetting.Dm = rdm;
	Debug::Printf("Succeeded. Using gfx mode %d x %d (%d-bit) %s",
		rdm.Width, rdm.Height, rdm.ColorDepth, rdm.Windowed ? "windowed" : "fullscreen");
	return true;
}

bool graphics_mode_update_render_frame() {
	if (!gfxDriver || !gfxDriver->IsModeSet() || !gfxDriver->IsNativeSizeValid())
		return false;

	DisplayMode dm = gfxDriver->GetDisplayMode();
	Size screen_size = Size(dm.Width, dm.Height);
	Size native_size = gfxDriver->GetNativeSize();
	Size frame_size = set_game_frame_after_screen_size(native_size, screen_size, CurFrameSetup);
	Rect render_frame = CenterInRect(RectWH(screen_size), RectWH(frame_size));

	if (!gfxDriver->SetRenderFrame(render_frame)) {
		Debug::Printf(kDbgMsg_Error, "Failed to set render frame (%d, %d, %d, %d : %d x %d). Error: %s",
			render_frame.Left, render_frame.Top, render_frame.Right, render_frame.Bottom,
			render_frame.GetWidth(), render_frame.GetHeight(), get_allegro_error());
		return false;
	}

	Rect dst_rect = gfxDriver->GetRenderDestination();
	Debug::Printf("Render frame set, render dest (%d, %d, %d, %d : %d x %d)",
		dst_rect.Left, dst_rect.Top, dst_rect.Right, dst_rect.Bottom, dst_rect.GetWidth(), dst_rect.GetHeight());
	// init game scaling transformation
	GameScaling.Init(native_size, gfxDriver->GetRenderDestination());
	return true;
}

bool graphics_mode_set_native_size(const Size &native_size) {
	if (!gfxDriver || native_size.IsNull())
		return false;
	if (!gfxDriver->SetNativeSize(native_size))
		return false;
	// if render frame translation was already set, then update it with new native size
	if (gfxDriver->IsRenderFrameValid())
		graphics_mode_update_render_frame();
	return true;
}

GameFrameSetup graphics_mode_get_render_frame() {
	return CurFrameSetup;
}

bool graphics_mode_set_render_frame(const GameFrameSetup &frame_setup) {
	if (!frame_setup.IsValid())
		return false;
	CurFrameSetup = frame_setup;
	if (gfxDriver->GetDisplayMode().Windowed)
		SavedWindowedSetting.FrameSetup = frame_setup;
	else
		SavedFullscreenSetting.FrameSetup = frame_setup;
	graphics_mode_update_render_frame();
	return true;
}

bool graphics_mode_set_filter(const String &filter_id) {
	if (!GfxFactory)
		return false;

	String filter_error;
	PGfxFilter filter = GfxFactory->SetFilter(filter_id, filter_error);
	if (!filter) {
		Debug::Printf(kDbgMsg_Error, "Unable to set graphics filter '%s'. Error: %s", filter_id.GetCStr(), filter_error.GetCStr());
		return false;
	}
	Rect filter_rect = filter->GetDestination();
	Debug::Printf("Graphics filter set: '%s', filter dest (%d, %d, %d, %d : %d x %d)", filter->GetInfo().Id.GetCStr(),
		filter_rect.Left, filter_rect.Top, filter_rect.Right, filter_rect.Bottom, filter_rect.GetWidth(), filter_rect.GetHeight());
	return true;
}

void graphics_mode_shutdown() {
	if (GfxFactory)
		GfxFactory->Shutdown();
	GfxFactory = nullptr;
	gfxDriver = nullptr;

	// Tell Allegro that we are no longer in graphics mode
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}

} // namespace AGS3
