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

#include "common/config-manager.h"
#include "common/std/algorithm.h"
#include "ags/engine/ac/display.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/font/ags_font_renderer.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/engine/ac/speech.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/top_bar_settings.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/gfx/blender.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/shared/util/string_utils.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/timer.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Shared::BitmapHelper;

struct DisplayVars {
	int linespacing = 0;   // font's line spacing
	int fulltxtheight = 0; // total height of all the text
} disp;

// Generates a textual image and returns a disposable bitmap
Bitmap *create_textual_image(const char *text, int asspch, int isThought,
							 int &xx, int &yy, int &adjustedXX, int &adjustedYY, int wii, int usingfont, int allowShrink,
							 bool &alphaChannel) {
	//
	// Configure the textual image
	//
	const bool use_speech_textwindow = (asspch < 0) && (_GP(game).options[OPT_SPEECHTYPE] >= 2);
	const bool use_thought_gui = (isThought) && (_GP(game).options[OPT_THOUGHTGUI] > 0);

	alphaChannel = false;
	int usingGui = -1;
	if (use_speech_textwindow)
		usingGui = _GP(play).speech_textwindow_gui;
	else if (use_thought_gui)
		usingGui = _GP(game).options[OPT_THOUGHTGUI];

	const int padding = get_textwindow_padding(usingGui);
	const int paddingScaled = get_fixed_pixel_size(padding);
 	// Just in case screen size is not neatly divisible by 320x200
	const int paddingDoubledScaled = get_fixed_pixel_size(padding * 2);

	// WORKAROUND: Guard Duty specifies a wii of 100,000, which is larger
	// than can be supported by ScummVM's surface classes
	wii = MIN(wii, 8000);

	// Make message copy, because ensure_text_valid_for_font() may modify it
	char todis[STD_BUFFER_SIZE];
	snprintf(todis, STD_BUFFER_SIZE - 1, "%s", text);
	ensure_text_valid_for_font(todis, usingfont);
	break_up_text_into_lines(todis, _GP(Lines), wii - 2 * padding, usingfont);
	disp.linespacing = get_font_linespacing(usingfont);
	disp.fulltxtheight = get_text_lines_surf_height(usingfont, _GP(Lines).Count());

	if (_GP(topBar).wantIt) {
		// ensure that the window is wide enough to display any top bar text
		int topBarWid = get_text_width_outlined(_GP(topBar).text, _GP(topBar).font);
		topBarWid += data_to_game_coord(_GP(play).top_bar_borderwidth + 2) * 2;
		if (_G(longestline) < topBarWid)
			_G(longestline) = topBarWid;
	}

	const Rect &ui_view = _GP(play).GetUIViewport();
	if (xx == OVR_AUTOPLACE);
	// centre text in middle of screen
	else if (yy < 0) yy = ui_view.GetHeight() / 2 - disp.fulltxtheight / 2 - padding;
	// speech, so it wants to be above the character's head
	else if (asspch > 0) {
		yy -= disp.fulltxtheight;
		if (yy < 5) yy = 5;
		yy = adjust_y_for_guis(yy);
	}

	if (_G(longestline) < wii - paddingDoubledScaled) {
		// shrink the width of the dialog box to fit the text
		int oldWid = wii;
		//if ((asspch >= 0) || (allowShrink > 0))
		// If it's not speech, or a shrink is allowed, then shrink it
		if ((asspch == 0) || (allowShrink > 0))
			wii = _G(longestline) + paddingDoubledScaled;

		// shift the dialog box right to align it, if necessary
		if ((allowShrink == 2) && (xx >= 0))
			xx += (oldWid - wii);
	}

	if (xx < -1) {
		xx = (-xx) - wii / 2;
		if (xx < 0)
			xx = 0;

		xx = adjust_x_for_guis(xx, yy);

		if (xx + wii >= ui_view.GetWidth())
			xx = (ui_view.GetWidth() - wii) - 5;
	} else if (xx < 0) xx = ui_view.GetWidth() / 2 - wii / 2;

	const int extraHeight = paddingDoubledScaled;
	color_t text_color = MakeColor(15);
	const int bmp_width = MAX(2, wii);
	const int bmp_height = MAX(2, disp.fulltxtheight + extraHeight);
	Bitmap *text_window_ds = BitmapHelper::CreateTransparentBitmap(
		bmp_width, bmp_height, _GP(game).GetColorDepth());

	// inform draw_text_window to free the old bitmap
	const bool wantFreeScreenop = true;

	//
	// Create the textual image (may also adjust some params in the process)
	//
	// may later change if usingGUI, needed to avoid changing original coordinates
	adjustedXX = xx;
	adjustedYY = yy;

	if ((strlen(todis) < 1) || (strcmp(todis, "  ") == 0) || (wii == 0));
	// if it's an empty speech line, don't draw anything
	else if (asspch) { //text_color = ds->GetCompatibleColor(12);
		int ttxleft = 0, ttxtop = paddingScaled, oriwid = wii - padding * 2;
		int drawBackground = 0;

		if (use_speech_textwindow) {
			drawBackground = 1;
		} else if (use_thought_gui) {
			// make it treat it as drawing inside a window now
			if (asspch > 0)
				asspch = -asspch;
			drawBackground = 1;
		}

		if (drawBackground) {
			draw_text_window_and_bar(&text_window_ds, wantFreeScreenop, &ttxleft, &ttxtop, &adjustedXX, &adjustedYY, &wii, &text_color, 0, usingGui);
			if (usingGui > 0) {
				alphaChannel = _GP(guis)[usingGui].HasAlphaChannel();
			}
		} else if ((ShouldAntiAliasText()) && (_GP(game).GetColorDepth() >= 24))
			alphaChannel = true;

		for (size_t ee = 0; ee < _GP(Lines).Count(); ee++) {
			//int ttxp=wii/2 - get_text_width_outlined(lines[ee], usingfont)/2;
			int ttyp = ttxtop + ee * disp.linespacing;
			// asspch < 0 means that it's inside a text box so don't
			// centre the text
			if (asspch < 0) {
				if ((usingGui >= 0) &&
				        ((_GP(game).options[OPT_SPEECHTYPE] >= 2) || (isThought)))
					text_color = text_window_ds->GetCompatibleColor(_GP(guis)[usingGui].FgColor);
				else
					text_color = text_window_ds->GetCompatibleColor(-asspch);

				wouttext_aligned(text_window_ds, ttxleft, ttyp, oriwid, usingfont, text_color, _GP(Lines)[ee].GetCStr(), _GP(play).text_align);
			} else {
				text_color = text_window_ds->GetCompatibleColor(asspch);
				wouttext_aligned(text_window_ds, ttxleft, ttyp, wii, usingfont, text_color, _GP(Lines)[ee].GetCStr(), _GP(play).speech_text_align);
			}
		}
	} else {
		int xoffs, yoffs, oriwid = wii - padding * 2;
		draw_text_window_and_bar(&text_window_ds, wantFreeScreenop, &xoffs, &yoffs, &adjustedXX, &adjustedYY, &wii, &text_color);

		if (_GP(game).options[OPT_TWCUSTOM] > 0) {
			alphaChannel = _GP(guis)[_GP(game).options[OPT_TWCUSTOM]].HasAlphaChannel();
		}

		adjust_y_coordinate_for_text(&yoffs, usingfont);

		for (size_t ee = 0; ee < _GP(Lines).Count(); ee++)
			wouttext_aligned(text_window_ds, xoffs, yoffs + ee * disp.linespacing, oriwid, usingfont, text_color, _GP(Lines)[ee].GetCStr(), _GP(play).text_align);
	}

	return text_window_ds;
}

// Pass yy = -1 to find Y co-ord automatically
// allowShrink = 0 for none, 1 for leftwards, 2 for rightwards
// pass blocking=2 to create permanent overlay
ScreenOverlay *display_main(int xx, int yy, int wii, const char *text, int disp_type, int usingfont,
							 int asspch, int isThought, int allowShrink, bool overlayPositionFixed, bool roomlayer) {
	//
	// Prepare for the message display
	//

	// AGS 2.x: If the screen is faded out, fade in again when displaying a message box.
	if (!asspch && (_G(loaded_game_file_version) <= kGameVersion_272))
		_GP(play).screen_is_faded_out = 0;

	// if it's a normal message box and the game was being skipped,
	// ensure that the screen is up to date before the message box
	// is drawn on top of it
	// TODO: is this really necessary anymore?
	if ((_GP(play).skip_until_char_stops >= 0) && (disp_type == DISPLAYTEXT_MESSAGEBOX))
		render_graphics();

	// TODO: should this really be called regardless of message type?
	// _display_main may be called even for custom textual overlays
	EndSkippingUntilCharStops();

	if (_GP(topBar).wantIt) {
		// the top bar should behave like DisplaySpeech wrt blocking
		disp_type = DISPLAYTEXT_SPEECH;
	}

	if ((asspch > 0) && (disp_type < DISPLAYTEXT_NORMALOVERLAY)) {
		// update the all_buttons_disabled variable in advance
		// of the adjust_x/y_for_guis calls
		_GP(play).disabled_user_interface++;
		update_gui_disabled_status();
		_GP(play).disabled_user_interface--;
	}

	// remove any previous blocking texts if necessary
	if (disp_type < DISPLAYTEXT_NORMALOVERLAY)
		remove_screen_overlay(_GP(play).text_overlay_on);

	// If fast-forwarding, then skip any blocking message immediately
	if (_GP(play).fast_forward && (disp_type < DISPLAYTEXT_NORMALOVERLAY)) {
		_GP(play).SetWaitSkipResult(SKIP_AUTOTIMER);
		post_display_cleanup();
		return nullptr;
	}

	//
	// Configure and create an overlay object
	//

	int ovrtype;
	switch (disp_type) {
	case DISPLAYTEXT_SPEECH: ovrtype = OVER_TEXTSPEECH; break;
	case DISPLAYTEXT_MESSAGEBOX: ovrtype = OVER_TEXTMSG; break;
	case DISPLAYTEXT_NORMALOVERLAY: ovrtype = OVER_CUSTOM; break;
	default: ovrtype = disp_type; break; // must be precreated overlay id
	}

	int adjustedXX, adjustedYY;
	bool alphaChannel;
	Bitmap *text_window_ds = create_textual_image(text, asspch, isThought, xx, yy, adjustedXX, adjustedYY, wii, usingfont, allowShrink, alphaChannel);

	size_t nse = add_screen_overlay(roomlayer, xx, yy, ovrtype, text_window_ds, adjustedXX - xx, adjustedYY - yy, alphaChannel);
	auto *over = get_overlay(nse); // FIXME: optimize return value
	// we should not delete text_window_ds here, because it is now owned by Overlay

	// If it's a non-blocking overlay type, then we're done here
	if (disp_type >= DISPLAYTEXT_NORMALOVERLAY) {
		return over;
	}

	//
	// Wait for the blocking text to timeout or until skipped by another command
	//

	if (disp_type == DISPLAYTEXT_MESSAGEBOX) {

		int countdown = GetTextDisplayTime(text);
		int skip_setting = user_to_internal_skip_speech((SkipSpeechStyle)_GP(play).skip_display);
		// Loop until skipped
		while (true) {
			if (SHOULD_QUIT)
				return 0;

			sys_evt_process_pending();

			update_audio_system_on_game_loop();
			UpdateCursorAndDrawables();
			render_graphics();
			eAGSMouseButton mbut;
			int mwheelz;
			if (run_service_mb_controls(mbut, mwheelz) && mbut > kMouseNone) {
				check_skip_cutscene_mclick(mbut);
				if (_GP(play).fast_forward)
					break;
				if (skip_setting & SKIP_MOUSECLICK && !_GP(play).IsIgnoringInput()) {
					_GP(play).SetWaitSkipResult(SKIP_MOUSECLICK, mbut);
					break;
				}
			}
			bool do_break = false;
			while (!_GP(play).fast_forward && !do_break && ags_keyevent_ready()) {
				KeyInput ki;
				if (run_service_key_controls(ki)) {
					check_skip_cutscene_keypress(ki.Key);
					if ((skip_setting & SKIP_KEYPRESS) && !_GP(play).IsIgnoringInput() && !IsAGSServiceKey(ki.Key)) {
						_GP(play).SetWaitKeySkip(ki);
						do_break = true;
					}
				}
			}
			if (do_break)
				break;

			update_polled_stuff();

			if (_GP(play).fast_forward == 0) {
				WaitForNextFrame();
			}

			countdown--;

			// Special behavior when coupled with a voice-over
			if (_GP(play).speech_has_voice) {
				// extend life of text if the voice hasn't finished yet
				if (AudioChans::ChannelIsPlaying(SCHAN_SPEECH) && (_GP(play).fast_forward == 0)) {
					if (countdown <= 1)
						countdown = 1;
				} else  // if the voice has finished, remove the speech
					countdown = 0;
			}
			// Test for the timed auto-skip
			if ((countdown < 1) && (skip_setting & SKIP_AUTOTIMER)) {
				_GP(play).SetWaitSkipResult(SKIP_AUTOTIMER);
				_GP(play).SetIgnoreInput(_GP(play).ignore_user_input_after_text_timeout_ms);
				break;
			}
			// if skipping cutscene, don't get stuck on No Auto Remove text boxes
			if ((countdown < 1) && (_GP(play).fast_forward))
				break;
		}

		remove_screen_overlay(OVER_TEXTMSG);
		invalidate_screen();
	} else {
		/* DISPLAYTEXT_SPEECH */
		if (!overlayPositionFixed) {
			over->SetRoomRelative(true);
			VpPoint vpt = _GP(play).GetRoomViewport(0)->ScreenToRoom(over->x, over->y, false);
			over->x = vpt.first.X;
			over->y = vpt.first.Y;
		}

		GameLoopUntilNoOverlay();
	}

	//
	// Post-message cleanup
	//
	post_display_cleanup();
	return nullptr;
}

void display_at(int xx, int yy, int wii, const char *text) {
	EndSkippingUntilCharStops();
	// Start voice-over, if requested by the tokens in speech text
	try_auto_play_speech(text, text, _GP(play).narrator_speech);
	display_main(xx, yy, wii, text, DISPLAYTEXT_MESSAGEBOX, FONT_NORMAL, 0, 0, 0, false);

	// Stop any blocking voice-over, if was started by this function
	if (_GP(play).IsBlockingVoiceSpeech())
		stop_voice_speech();
}

void post_display_cleanup() {
	ags_clear_input_buffer();
	_GP(play).messagetime = -1;
	_GP(play).speech_in_post_state = false;
}

bool try_auto_play_speech(const char *text, const char *&replace_text, int charid) {
	int voice_num;
	const char *src = parse_voiceover_token(text, &voice_num);
	if (src == text)
		return false; // no token

	if (voice_num <= 0)
		quit("DisplaySpeech: auto-voice symbol '&' not followed by valid integer");

	replace_text = src; // skip voice tag
	if (play_voice_speech(charid, voice_num)) {
		// if Voice Only, then blank out the text
		if (_GP(play).speech_mode == kSpeech_VoiceOnly)
			replace_text = "  ";
		return true;
	}
	return false;
}

int GetTextDisplayLength(const char *text) {
	// Skip voice-over token from the length calculation if required
	if (_GP(play).unfactor_speech_from_textlength != 0)
		text = parse_voiceover_token(text, nullptr);
	return static_cast<int>(strlen(text));
}

// Calculates lipsync frame duration (or duration per character) in game loops.
// NOTE: historical formula was this:
//   loops_per_character = (((text_len / play.lipsync_speed) + 1) * fps) / text_len;
// But because of a precision loss due integer division this resulted in "jumping" values.
// The new formula uses float division, and coefficent found experimentally to make
// results match the old formula in certain key text lengths, for backwards compatibility.
int CalcLipsyncFrameDuration(int text_len, int fps) {
	return static_cast<int>((((static_cast<float>(text_len) / _GP(play).lipsync_speed) + 0.75f) * fps) / text_len);
}

int GetTextDisplayTime(const char *text, int canberel) {
	int uselen = 0;
	auto fpstimer = ::lround(get_game_fps());

	// if it's background speech, make it stay relative to game speed
	if ((canberel == 1) && (_GP(play).bgspeech_game_speed == 1))
		fpstimer = 40; // NOTE: should be a fixed constant here, not game speed value

	if (_G(source_text_length) >= 0) {
		// sync to length of original text, to make sure any animations
		// and music sync up correctly
		uselen = _G(source_text_length);
		_G(source_text_length) = -1;
	} else {
		uselen = GetTextDisplayLength(text);
	}

	if (uselen <= 0)
		return 0;

	if (_GP(play).text_speed + _GP(play).text_speed_modifier <= 0)
		quit("!Text speed is zero; unable to display text. Check your _GP(game).text_speed settings.");

	// Store how many game loops per character of text
	_G(loops_per_character) = CalcLipsyncFrameDuration(uselen, fpstimer);

	int textDisplayTimeInMS = ((uselen / (_GP(play).text_speed + _GP(play).text_speed_modifier)) + 1) * 1000;
	if (textDisplayTimeInMS < _GP(play).text_min_display_time_ms)
		textDisplayTimeInMS = _GP(play).text_min_display_time_ms;

	return (textDisplayTimeInMS * fpstimer) / 1000;
}

bool ShouldAntiAliasText() {
	return (_GP(game).GetColorDepth() >= 24) && (_GP(game).options[OPT_ANTIALIASFONTS] != 0 || ::AGS::g_vm->_forceTextAA);
}

void wouttextxy_AutoOutline(Bitmap *ds, size_t font, int32_t color, const char *texx, int &xxp, int &yyp) {
	const FontInfo &finfo = get_fontinfo(font);
	int const thickness = finfo.AutoOutlineThickness;
	auto const style = finfo.AutoOutlineStyle;
	if (thickness <= 0)
		return;

	// 16-bit games should use 32-bit stencils to keep anti-aliasing working
	// because 16-bit blending works correctly if there's an actual color
	// on the destination bitmap (and our intermediate bitmaps are transparent).
	int const  ds_cd = ds->GetColorDepth();
	bool const antialias = ds_cd >= 16 && _GP(game).options[OPT_ANTIALIASFONTS] != 0 && !is_bitmap_font(font);
	int const  stencil_cd = antialias ? 32 : ds_cd;
	if (antialias) // This is to make sure TTFs render proper alpha channel in 16-bit games too
		color |= makeacol32(0, 0, 0, 0xff);

	// WORKAROUND: Clifftop's Spritefont plugin returns a wrong font height for font 2 in Kathy Rain, which causes a partial outline
	// for some letters. Unfortunately fixing the value on the plugin side breaks the line spacing, so let's just correct it here.
	const int t_width = get_text_width(texx, font);
	const auto t_extent = get_font_surface_extent(font);
	const int t_height = t_extent.second - t_extent.first + ((strcmp(_GP(game).guid, "{d6795d1c-3cfe-49ec-90a1-85c313bfccaf}") == 0) && (font == 2) ? 1 : 0);

	if (t_width == 0 || t_height == 0)
		return;

	// Prepare stencils
	const int t_yoff = t_extent.first;
	Bitmap *texx_stencil, *outline_stencil;
	alloc_font_outline_buffers(font, &texx_stencil, &outline_stencil,
		t_width, t_height, stencil_cd);
	texx_stencil->ClearTransparent();
	outline_stencil->ClearTransparent();
	// Ready text stencil
	// Note we are drawing with y off, in case some font's glyphs exceed font's ascender
	wouttextxy(texx_stencil, 0, -t_yoff, font, color, texx);

	// Anti-aliased TTFs require to be alpha-blended, not blit,
	// or the alpha values will be plain copied and final image will be broken.
	void(Bitmap:: * pfn_drawstencil)(Bitmap * src, int dst_x, int dst_y);
	if (antialias) { // NOTE: we must set out blender AFTER wouttextxy, or it will be overidden
		set_argb2any_blender();
		pfn_drawstencil = &Bitmap::TransBlendBlt;
	} else {
		pfn_drawstencil = &Bitmap::MaskedBlit;
	}

	// move start of text so that the outline doesn't drop off the bitmap
	xxp += thickness;
	int const outline_y = yyp + t_yoff;
	yyp += thickness;

	// What we do here: first we paint text onto outline_stencil offsetting vertically;
	// then we paint resulting outline_stencil onto final dest offsetting horizontally.
	int largest_y_diff_reached_so_far = -1;
	for (int x_diff = thickness; x_diff >= 0; x_diff--) {
		// Integer arithmetics: In the following, we use terms k*(k + 1) to account for rounding.
		//     (k + 0.5)^2 == k*k + 2*k*0.5 + 0.5^2 == k*k + k + 0.25 ==approx. k*(k + 1)
		int y_term_limit = thickness * (thickness + 1);
		if (FontInfo::kRounded == style)
			y_term_limit -= x_diff * x_diff;

		// Extend the outline stencil to the top and bottom
		for (int y_diff = largest_y_diff_reached_so_far + 1;
			y_diff <= thickness && y_diff * y_diff <= y_term_limit;
			y_diff++) {
			(outline_stencil->*pfn_drawstencil)(texx_stencil, 0, thickness - y_diff);
			if (y_diff > 0)
				(outline_stencil->*pfn_drawstencil)(texx_stencil, 0, thickness + y_diff);
			largest_y_diff_reached_so_far = y_diff;
		}

		// Stamp the outline stencil to the left and right of the text
		(ds->*pfn_drawstencil)(outline_stencil, xxp - x_diff, outline_y);
		if (x_diff > 0)
			(ds->*pfn_drawstencil)(outline_stencil, xxp + x_diff, outline_y);
	}
}

// Draw an outline if requested, then draw the text on top
void wouttext_outline(Shared::Bitmap *ds, int xxp, int yyp, int font, color_t text_color, const char *texx) {
	size_t const text_font = static_cast<size_t>(font);
	// Draw outline (a backdrop) if requested
	color_t const outline_color = ds->GetCompatibleColor(_GP(play).speech_text_shadow);
	int const outline_font = get_font_outline(font);
	if (outline_font >= 0)
		wouttextxy(ds, xxp, yyp, static_cast<size_t>(outline_font), outline_color, texx);
	else if (outline_font == FONT_OUTLINE_AUTO)
		wouttextxy_AutoOutline(ds, text_font, outline_color, texx, xxp, yyp);
	else
		; // no outline

	// Draw text on top
	wouttextxy(ds, xxp, yyp, text_font, text_color, texx);
}

void wouttext_aligned(Bitmap *ds, int usexp, int yy, int oriwid, int usingfont, color_t text_color, const char *text, HorAlignment align) {

	if (align & kMAlignHCenter)
		usexp = usexp + (oriwid / 2) - (get_text_width_outlined(text, usingfont) / 2);
	else if (align & kMAlignRight)
		usexp = usexp + (oriwid - get_text_width_outlined(text, usingfont));

	wouttext_outline(ds, usexp, yy, usingfont, text_color, text);
}

int get_font_outline_padding(int font) {
	if (get_font_outline(font) == FONT_OUTLINE_AUTO) {
		// scaled up bitmap font, push outline further out
		if (is_bitmap_font(font) && get_font_scaling_mul(font) > 1)
			return get_fixed_pixel_size(2); // FIXME: should be 2 + get_fixed_pixel_size(2)?
		// otherwise, just push outline by 1 pixel
		else
			return 2;
	}
	return 0;
}

void do_corner(Bitmap *ds, int sprn, int x, int y, int offx, int offy) {
	if (sprn < 0) return;
	if (!_GP(spriteset).DoesSpriteExist(sprn)) {
		sprn = 0;
	}

	x = x + offx * _GP(game).SpriteInfos[sprn].Width;
	y = y + offy * _GP(game).SpriteInfos[sprn].Height;
	draw_gui_sprite_v330(ds, sprn, x, y);
}

int get_but_pic(GUIMain *guo, int indx) {
	int butid = guo->GetControlID(indx);
	return butid >= 0 ? _GP(guibuts)[butid].GetNormalImage() : 0;
}

void draw_button_background(Bitmap *ds, int xx1, int yy1, int xx2, int yy2, GUIMain *iep) {
	color_t draw_color;
	if (iep == nullptr) {  // standard window
		draw_color = ds->GetCompatibleColor(15);
		ds->FillRect(Rect(xx1, yy1, xx2, yy2), draw_color);
		draw_color = ds->GetCompatibleColor(16);
		ds->DrawRect(Rect(xx1, yy1, xx2, yy2), draw_color);
	} else {
		if (_G(loaded_game_file_version) < kGameVersion_262) {
			// In pre-2.62 color 0 should be treated as "black" instead of "transparent";
			// this was an unintended effect in older versions (see 2.62 changelog fixes).
			if (iep->BgColor == 0)
				iep->BgColor = 16;
		}

		if (iep->BgColor >= 0) draw_color = ds->GetCompatibleColor(iep->BgColor);
		else draw_color = ds->GetCompatibleColor(0); // black backrgnd behind picture

		if (iep->BgColor > 0)
			ds->FillRect(Rect(xx1, yy1, xx2, yy2), draw_color);

		const int leftRightWidth = _GP(game).SpriteInfos[get_but_pic(iep, 4)].Width;
		const int topBottomHeight = _GP(game).SpriteInfos[get_but_pic(iep, 6)].Height;
		// GUI middle space
		if (iep->BgImage > 0) {
			{
				// offset the background image and clip it so that it is drawn
				// such that the border graphics can have a transparent outside
				// edge
				int bgoffsx = xx1 - leftRightWidth / 2;
				int bgoffsy = yy1 - topBottomHeight / 2;
				ds->SetClip(Rect(bgoffsx, bgoffsy, xx2 + leftRightWidth / 2, yy2 + topBottomHeight / 2));
				int bgfinishx = xx2;
				int bgfinishy = yy2;
				int bgoffsyStart = bgoffsy;
				while (bgoffsx <= bgfinishx) {
					bgoffsy = bgoffsyStart;
					while (bgoffsy <= bgfinishy) {
						draw_gui_sprite_v330(ds, iep->BgImage, bgoffsx, bgoffsy);
						bgoffsy += _GP(game).SpriteInfos[iep->BgImage].Height;
					}
					bgoffsx += _GP(game).SpriteInfos[iep->BgImage].Width;
				}
				// return to normal clipping rectangle
				ds->ResetClip();
			}
		}
		// Vertical borders
		ds->SetClip(Rect(xx1 - leftRightWidth, yy1, xx2 + 1 + leftRightWidth, yy2));
		for (int uu = yy1; uu <= yy2; uu += _GP(game).SpriteInfos[get_but_pic(iep, 4)].Height) {
			do_corner(ds, get_but_pic(iep, 4), xx1, uu, -1, 0);    // left side
			do_corner(ds, get_but_pic(iep, 5), xx2 + 1, uu, 0, 0); // right side
		}
		// Horizontal borders
		ds->SetClip(Rect(xx1, yy1 - topBottomHeight, xx2, yy2 + 1 + topBottomHeight));
		for (int uu = xx1; uu <= xx2; uu += _GP(game).SpriteInfos[get_but_pic(iep, 6)].Width) {
			do_corner(ds, get_but_pic(iep, 6), uu, yy1, 0, -1);    // top side
			do_corner(ds, get_but_pic(iep, 7), uu, yy2 + 1, 0, 0); // bottom side
		}
		ds->ResetClip();
		// Four corners
		do_corner(ds, get_but_pic(iep, 0), xx1, yy1, -1, -1);       // top left
		do_corner(ds, get_but_pic(iep, 1), xx1, yy2 + 1, -1, 0);    // bottom left
		do_corner(ds, get_but_pic(iep, 2), xx2 + 1, yy1, 0, -1);    // top right
		do_corner(ds, get_but_pic(iep, 3), xx2 + 1, yy2 + 1, 0, 0); // bottom right
	}
}

// Calculate the width that the left and right border of the textwindow
// GUI take up
int get_textwindow_border_width(int twgui) {
	if (twgui < 0)
		return 0;

	if (!_GP(guis)[twgui].IsTextWindow())
		quit("!GUI set as text window but is not actually a text window GUI");

	int borwid = _GP(game).SpriteInfos[get_but_pic(&_GP(guis)[twgui], 4)].Width +
	             _GP(game).SpriteInfos[get_but_pic(&_GP(guis)[twgui], 5)].Width;

	return borwid;
}

// get the hegiht of the text window's top border
int get_textwindow_top_border_height(int twgui) {
	if (twgui < 0)
		return 0;

	if (!_GP(guis)[twgui].IsTextWindow())
		quit("!GUI set as text window but is not actually a text window GUI");

	return _GP(game).SpriteInfos[get_but_pic(&_GP(guis)[twgui], 6)].Height;
}

// Get the padding for a text window
// -1 for the game's custom text window
int get_textwindow_padding(int ifnum) {
	int result;

	if (ifnum < 0)
		ifnum = _GP(game).options[OPT_TWCUSTOM];
	if (ifnum > 0 && ifnum < _GP(game).numgui)
		result = _GP(guis)[ifnum].Padding;
	else
		result = TEXTWINDOW_PADDING_DEFAULT;

	return result;
}

void draw_text_window(Bitmap **text_window_ds, bool should_free_ds,
                      int *xins, int *yins, int *xx, int *yy, int *wii, color_t *set_text_color, int ovrheight, int ifnum) {
	assert(text_window_ds);
	Bitmap *ds = *text_window_ds;
	if (ifnum < 0)
		ifnum = _GP(game).options[OPT_TWCUSTOM];

	if (ifnum <= 0) {
		if (ovrheight)
			quit("!Cannot use QFG4 style options without custom text window");
		draw_button_background(ds, 0, 0, ds->GetWidth() - 1, ds->GetHeight() - 1, nullptr);
		if (set_text_color)
			*set_text_color = ds->GetCompatibleColor(16);
		xins[0] = 3;
		yins[0] = 3;
	} else {
		if (ifnum >= _GP(game).numgui)
			quitprintf("!Invalid GUI %d specified as text window (total GUIs: %d)", ifnum, _GP(game).numgui);
		if (!_GP(guis)[ifnum].IsTextWindow())
			quit("!GUI set as text window but is not actually a text window GUI");

		int tbnum = get_but_pic(&_GP(guis)[ifnum], 0);

		wii[0] += get_textwindow_border_width(ifnum);
		xx[0] -= _GP(game).SpriteInfos[tbnum].Width;
		yy[0] -= _GP(game).SpriteInfos[tbnum].Height;
		if (ovrheight == 0)
			ovrheight = disp.fulltxtheight;

		if (should_free_ds)
			delete *text_window_ds;
		int padding = get_textwindow_padding(ifnum);
		*text_window_ds = BitmapHelper::CreateTransparentBitmap(wii[0], ovrheight + (padding * 2) + _GP(game).SpriteInfos[tbnum].Height * 2, _GP(game).GetColorDepth());
		ds = *text_window_ds;
		int xoffs = _GP(game).SpriteInfos[tbnum].Width, yoffs = _GP(game).SpriteInfos[tbnum].Height;
		draw_button_background(ds, xoffs, yoffs, (ds->GetWidth() - xoffs) - 1, (ds->GetHeight() - yoffs) - 1, &_GP(guis)[ifnum]);
		if (set_text_color)
			*set_text_color = ds->GetCompatibleColor(_GP(guis)[ifnum].FgColor);
		xins[0] = xoffs + padding;
		yins[0] = yoffs + padding;
	}
}

void draw_text_window_and_bar(Bitmap **text_window_ds, bool should_free_ds,
                              int *xins, int *yins, int *xx, int *yy, int *wii, color_t *set_text_color, int ovrheight, int ifnum) {
	assert(text_window_ds);
	draw_text_window(text_window_ds, should_free_ds, xins, yins, xx, yy, wii, set_text_color, ovrheight, ifnum);

	if ((_GP(topBar).wantIt) && (text_window_ds && *text_window_ds)) {
		// top bar on the dialog window with character's name
		// create an enlarged window, then free the old one
		Bitmap *ds = *text_window_ds;
		Bitmap *newScreenop = BitmapHelper::CreateBitmap(ds->GetWidth(), ds->GetHeight() + _GP(topBar).height, _GP(game).GetColorDepth());
		newScreenop->Blit(ds, 0, 0, 0, _GP(topBar).height, ds->GetWidth(), ds->GetHeight());
		delete *text_window_ds;
		*text_window_ds = newScreenop;
		ds = *text_window_ds;

		// draw the top bar
		color_t draw_color = ds->GetCompatibleColor(_GP(play).top_bar_backcolor);
		ds->FillRect(Rect(0, 0, ds->GetWidth() - 1, _GP(topBar).height - 1), draw_color);
		if (_GP(play).top_bar_backcolor != _GP(play).top_bar_bordercolor) {
			// draw the border
			draw_color = ds->GetCompatibleColor(_GP(play).top_bar_bordercolor);
			for (int j = 0; j < data_to_game_coord(_GP(play).top_bar_borderwidth); j++)
				ds->DrawRect(Rect(j, j, ds->GetWidth() - (j + 1), _GP(topBar).height - (j + 1)), draw_color);
		}

		// draw the text
		int textx = (ds->GetWidth() / 2) - get_text_width_outlined(_GP(topBar).text, _GP(topBar).font) / 2;
		color_t text_color = ds->GetCompatibleColor(_GP(play).top_bar_textcolor);
		wouttext_outline(ds, textx, _GP(play).top_bar_borderwidth + get_fixed_pixel_size(1), _GP(topBar).font, text_color, _GP(topBar).text);

		// don't draw it next time
		_GP(topBar).wantIt = 0;
		// adjust the text Y position
		yins[0] += _GP(topBar).height;
	} else if (_GP(topBar).wantIt)
		_GP(topBar).wantIt = 0;
}

} // namespace AGS3
