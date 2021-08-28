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

#include "common/config-manager.h"
#include "ags/lib/std/algorithm.h"
#include "ags/lib/std/math.h"
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

struct DisplayVars {
	int lineheight;    // font's height of single line
	int linespacing;   // font's line spacing
	int fulltxtheight; // total height of all the text
} disp;

// Pass yy = -1 to find Y co-ord automatically
// allowShrink = 0 for none, 1 for leftwards, 2 for rightwards
// pass blocking=2 to create permanent overlay
int _display_main(int xx, int yy, int wii, const char *text, int disp_type, int usingfont, int asspch, int isThought, int allowShrink, bool overlayPositionFixed) {
	const bool use_speech_textwindow = (asspch < 0) && (_GP(game).options[OPT_SPEECHTYPE] >= 2);
	const bool use_thought_gui = (isThought) && (_GP(game).options[OPT_THOUGHTGUI] > 0);

	bool alphaChannel = false;
	char todis[STD_BUFFER_SIZE];
	snprintf(todis, STD_BUFFER_SIZE - 1, "%s", text);
	int usingGui = -1;
	if (use_speech_textwindow)
		usingGui = _GP(play).speech_textwindow_gui;
	else if (use_thought_gui)
		usingGui = _GP(game).options[OPT_THOUGHTGUI];

	int padding = get_textwindow_padding(usingGui);
	int paddingScaled = get_fixed_pixel_size(padding);
	int paddingDoubledScaled = get_fixed_pixel_size(padding * 2); // Just in case screen size does is not neatly divisible by 320x200

	// FIXME: Fixes the display of the F1 help dialog in La Croix Pan,
	// since it was previously incorrectly wrapping on the 's' at the end
	// of the 'Cursors' word. May be due to minor differences in width calcs
	if (padding == 3 && ConfMan.get("gameid") == "lacroixpan")
		padding = 0;

	// WORKAROUND: Guard Duty specifies a wii of 100,000, which is larger
	// than can be supported by ScummVM's surface classes
	wii = MIN(wii, 8000);

	ensure_text_valid_for_font(todis, usingfont);
	break_up_text_into_lines(todis, _GP(Lines), wii - 2 * padding, usingfont);
	disp.lineheight = getfontheight_outlined(usingfont);
	disp.linespacing = getfontspacing_outlined(usingfont);
	disp.fulltxtheight = getheightoflines(usingfont, _GP(Lines).Count());

	// AGS 2.x: If the screen is faded out, fade in again when displaying a message box.
	if (!asspch && (_G(loaded_game_file_version) <= kGameVersion_272))
		_GP(play).screen_is_faded_out = 0;

	// if it's a normal message box and the game was being skipped,
	// ensure that the screen is up to date before the message box
	// is drawn on top of it
	// TODO: is this really necessary anymore?
	if ((_GP(play).skip_until_char_stops >= 0) && (disp_type == DISPLAYTEXT_MESSAGEBOX))
		render_graphics();

	EndSkippingUntilCharStops();

	if (_GP(topBar).wantIt) {
		// ensure that the window is wide enough to display
		// any top bar text
		int topBarWid = wgettextwidth_compensate(_GP(topBar).text, _GP(topBar).font);
		topBarWid += data_to_game_coord(_GP(play).top_bar_borderwidth + 2) * 2;
		if (_G(longestline) < topBarWid)
			_G(longestline) = topBarWid;
		// the top bar should behave like DisplaySpeech wrt blocking
		disp_type = DISPLAYTEXT_SPEECH;
	}

	if (asspch > 0) {
		// update the all_buttons_disabled variable in advance
		// of the adjust_x/y_for_guis calls
		_GP(play).disabled_user_interface++;
		update_gui_disabled_status();
		_GP(play).disabled_user_interface--;
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

	int extraHeight = paddingDoubledScaled;
	color_t text_color = MakeColor(15);
	if (disp_type < DISPLAYTEXT_NORMALOVERLAY)
		remove_screen_overlay(_GP(play).text_overlay_on); // remove any previous blocking texts

	const int bmp_width = std::max(2, wii);
	const int bmp_height = std::max(2, disp.fulltxtheight + extraHeight);
	Bitmap *text_window_ds = BitmapHelper::CreateTransparentBitmap(
		bmp_width, bmp_height, _GP(game).GetColorDepth());

	// inform draw_text_window to free the old bitmap
	const bool wantFreeScreenop = true;

	//
	// Creating displayed graphic
	//
	// may later change if usingGUI, needed to avoid changing original coordinates
	int adjustedXX = xx;
	int adjustedYY = yy;

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
			//int ttxp=wii/2 - wgettextwidth_compensate(lines[ee], usingfont)/2;
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
				//wouttext_outline(ttxp,ttyp,usingfont,lines[ee]);
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

	int ovrtype;
	switch (disp_type) {
	case DISPLAYTEXT_SPEECH: ovrtype = OVER_TEXTSPEECH; break;
	case DISPLAYTEXT_MESSAGEBOX: ovrtype = OVER_TEXTMSG; break;
	case DISPLAYTEXT_NORMALOVERLAY: ovrtype = OVER_CUSTOM; break;
	default: ovrtype = disp_type; break; // must be precreated overlay id
	}

	int nse = add_screen_overlay(xx, yy, ovrtype, text_window_ds, adjustedXX - xx, adjustedYY - yy, alphaChannel);
	// we should not delete text_window_ds here, because it is now owned by Overlay

	if (disp_type >= DISPLAYTEXT_NORMALOVERLAY) {
		return _G(screenover)[nse].type;
	}

	//
	// Wait for the blocking text to timeout or until skipped by another command
	//
	if (disp_type == DISPLAYTEXT_MESSAGEBOX) {
		// If fast-forwarding, then skip immediately
		if (_GP(play).fast_forward) {
			remove_screen_overlay(OVER_TEXTMSG);
			_GP(play).SetWaitSkipResult(SKIP_AUTOTIMER);
			_GP(play).messagetime = -1;
			return 0;
		}

		if (!_GP(play).mouse_cursor_hidden)
			ags_domouse(DOMOUSE_ENABLE);
		int countdown = GetTextDisplayTime(todis);
		int skip_setting = user_to_internal_skip_speech((SkipSpeechStyle)_GP(play).skip_display);
		// Loop until skipped
		while (true) {
			if (SHOULD_QUIT)
				return 0;

			sys_evt_process_pending();

			update_audio_system_on_game_loop();
			render_graphics();
			int mbut, mwheelz;
			if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0) {
				check_skip_cutscene_mclick(mbut);
				if (_GP(play).fast_forward)
					break;
				if (skip_setting & SKIP_MOUSECLICK && !_GP(play).IsIgnoringInput()) {
					_GP(play).SetWaitSkipResult(SKIP_MOUSECLICK, mbut);
					break;
				}
			}
			KeyInput kp;
			if (run_service_key_controls(kp)) {
				check_skip_cutscene_keypress(kp.Key);
				if (_GP(play).fast_forward)
					break;
				if ((skip_setting & SKIP_KEYPRESS) && !_GP(play).IsIgnoringInput()) {
					_GP(play).SetWaitSkipResult(SKIP_KEYPRESS, kp.Key);
					break;
				}
			}

			update_polled_stuff_if_runtime();

			if (_GP(play).fast_forward == 0) {
				WaitForNextFrame();
			}

			countdown--;

			// Special behavior when coupled with a voice-over
			if (_GP(play).speech_has_voice) {
				// extend life of text if the voice hasn't finished yet
				if (channel_is_playing(SCHAN_SPEECH) && (_GP(play).fast_forward == 0)) {
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
		if (!_GP(play).mouse_cursor_hidden)
			ags_domouse(DOMOUSE_DISABLE);
		remove_screen_overlay(OVER_TEXTMSG);
		invalidate_screen();
	} else {
		/* DISPLAYTEXT_SPEECH */
		// if the speech does not time out, but we are skipping a cutscene,
		// allow it to time out
		if ((_GP(play).messagetime < 0) && (_GP(play).fast_forward))
			_GP(play).messagetime = 2;

		if (!overlayPositionFixed) {
			_G(screenover)[nse].positionRelativeToScreen = false;
			VpPoint vpt = _GP(play).GetRoomViewport(0)->ScreenToRoom(_G(screenover)[nse].x, _G(screenover)[nse].y, false);
			_G(screenover)[nse].x = vpt.first.X;
			_G(screenover)[nse].y = vpt.first.Y;
		}

		GameLoopUntilNoOverlay();
	}

	_GP(play).messagetime = -1;
	return 0;
}

void _display_at(int xx, int yy, int wii, const char *text, int disp_type, int asspch, int isThought, int allowShrink, bool overlayPositionFixed) {
	int usingfont = FONT_NORMAL;
	if (asspch) usingfont = FONT_SPEECH;
	// TODO: _display_at may be called from _displayspeech, which can start
	// and finalize voice speech on its own. Find out if we really need to
	// keep track of this and not just stop voice regardless.
	bool need_stop_speech = false;

	EndSkippingUntilCharStops();

	if (try_auto_play_speech(text, text, _GP(play).narrator_speech, true)) {// TODO: is there any need for this flag?
		need_stop_speech = true;
	}
	_display_main(xx, yy, wii, text, disp_type, usingfont, asspch, isThought, allowShrink, overlayPositionFixed);

	if (need_stop_speech)
		stop_voice_speech();
}

bool try_auto_play_speech(const char *text, const char *&replace_text, int charid, bool blocking) {
	const char *src = text;
	if (src[0] != '&')
		return false;

	int sndid = atoi(&src[1]);
	while ((src[0] != ' ') & (src[0] != 0)) src++;
	if (src[0] == ' ') src++;
	if (sndid <= 0)
		quit("DisplaySpeech: auto-voice symbol '&' not followed by valid integer");

	replace_text = src; // skip voice tag
	if (play_voice_speech(charid, sndid)) {
		// if Voice Only, then blank out the text
		if (_GP(play).want_speech == 2)
			replace_text = "  ";
		return true;
	}
	return false;
}

int GetTextDisplayLength(const char *text) {
	int len = (int)strlen(text);
	if ((text[0] == '&') && (_GP(play).unfactor_speech_from_textlength != 0)) {
		// if there's an "&12 text" type line, remove "&12 " from the source length
		size_t j = 0;
		while ((text[j] != ' ') && (text[j] != 0))
			j++;
		j++;
		len -= j;
	}
	return len;
}

int GetTextDisplayTime(const char *text, int canberel) {
	int uselen = 0;
	auto fpstimer = ::lround(get_current_fps());

	// if it's background speech, make it stay relative to game speed
	if ((canberel == 1) && (_GP(play).bgspeech_game_speed == 1))
		fpstimer = 40;

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
	// This is calculated using a hard-coded 15 for the text speed,
	// so that it's always the same no matter how fast the user
	// can read.
	_G(loops_per_character) = (((uselen / _GP(play).lipsync_speed) + 1) * fpstimer) / uselen;

	int textDisplayTimeInMS = ((uselen / (_GP(play).text_speed + _GP(play).text_speed_modifier)) + 1) * 1000;
	if (textDisplayTimeInMS < _GP(play).text_min_display_time_ms)
		textDisplayTimeInMS = _GP(play).text_min_display_time_ms;

	return (textDisplayTimeInMS * fpstimer) / 1000;
}

bool ShouldAntiAliasText() {
	return (_GP(game).options[OPT_ANTIALIASFONTS] != 0 || ::AGS::g_vm->_forceTextAA);
}

void wouttext_outline(Shared::Bitmap *ds, int xxp, int yyp, int usingfont, color_t text_color, const char *texx) {
	color_t outline_color = ds->GetCompatibleColor(_GP(play).speech_text_shadow);
	if (get_font_outline(usingfont) >= 0) {
		// MACPORT FIX 9/6/5: cast
		wouttextxy(ds, xxp, yyp, (int)get_font_outline(usingfont), outline_color, texx);
	} else if (get_font_outline(usingfont) == FONT_OUTLINE_AUTO) {
		int outlineDist = 1;

		if (is_bitmap_font(usingfont) && get_font_scaling_mul(usingfont) > 1) {
			// if it's a scaled up bitmap font, move the outline out more
			outlineDist = get_fixed_pixel_size(1);
		}

		// move the text over so that it's still within the bounding rect
		xxp += outlineDist;
		yyp += outlineDist;

		wouttextxy(ds, xxp - outlineDist, yyp, usingfont, outline_color, texx);
		wouttextxy(ds, xxp + outlineDist, yyp, usingfont, outline_color, texx);
		wouttextxy(ds, xxp, yyp + outlineDist, usingfont, outline_color, texx);
		wouttextxy(ds, xxp, yyp - outlineDist, usingfont, outline_color, texx);
		wouttextxy(ds, xxp - outlineDist, yyp - outlineDist, usingfont, outline_color, texx);
		wouttextxy(ds, xxp - outlineDist, yyp + outlineDist, usingfont, outline_color, texx);
		wouttextxy(ds, xxp + outlineDist, yyp + outlineDist, usingfont, outline_color, texx);
		wouttextxy(ds, xxp + outlineDist, yyp - outlineDist, usingfont, outline_color, texx);
	}

	wouttextxy(ds, xxp, yyp, usingfont, text_color, texx);
}

void wouttext_aligned(Bitmap *ds, int usexp, int yy, int oriwid, int usingfont, color_t text_color, const char *text, HorAlignment align) {

	if (align & kMAlignHCenter)
		usexp = usexp + (oriwid / 2) - (wgettextwidth_compensate(text, usingfont) / 2);
	else if (align & kMAlignRight)
		usexp = usexp + (oriwid - wgettextwidth_compensate(text, usingfont));

	wouttext_outline(ds, usexp, yy, usingfont, text_color, text);
}

int get_outline_adjustment(int font) {
	// automatic outline fonts are 2 pixels taller
	if (get_font_outline(font) == FONT_OUTLINE_AUTO) {
		// scaled up bitmap font, push outline further out
		if (is_bitmap_font(font) && get_font_scaling_mul(font) > 1)
			return get_fixed_pixel_size(2);
		// otherwise, just push outline by 1 pixel
		else
			return 2;
	}
	return 0;
}

int getfontheight_outlined(int font) {
	return getfontheight(font) + get_outline_adjustment(font);
}

int getfontspacing_outlined(int font) {
	return use_default_linespacing(font) ?
	       getfontheight_outlined(font) :
	       getfontlinespacing(font);
}

int getfontlinegap(int font) {
	return getfontspacing_outlined(font) - getfontheight_outlined(font);
}

int getheightoflines(int font, int numlines) {
	return getfontspacing_outlined(font) * (numlines - 1) + getfontheight_outlined(font);
}

int wgettextwidth_compensate(const char *tex, int font) {
	int wdof = wgettextwidth(tex, font);

	if (get_font_outline(font) == FONT_OUTLINE_AUTO) {
		// scaled up SCI font, push outline further out
		if (is_bitmap_font(font) && get_font_scaling_mul(font) > 1)
			wdof += get_fixed_pixel_size(2);
		// otherwise, just push outline by 1 pixel
		else
			wdof += get_fixed_pixel_size(1);
	}

	return wdof;
}

void do_corner(Bitmap *ds, int sprn, int x, int y, int offx, int offy) {
	if (sprn < 0) return;
	if (_GP(spriteset)[sprn] == nullptr) {
		sprn = 0;
	}

	x = x + offx * _GP(game).SpriteInfos[sprn].Width;
	y = y + offy * _GP(game).SpriteInfos[sprn].Height;
	draw_gui_sprite_v330(ds, sprn, x, y);
}

int get_but_pic(GUIMain *guo, int indx) {
	int butid = guo->GetControlID(indx);
	return butid >= 0 ? _GP(guibuts)[butid].Image : 0;
}

void draw_button_background(Bitmap *ds, int xx1, int yy1, int xx2, int yy2, GUIMain *iep) {
	color_t draw_color;
	if (iep == nullptr) {  // standard window
		draw_color = ds->GetCompatibleColor(15);
		ds->FillRect(Rect(xx1, yy1, xx2, yy2), draw_color);
		draw_color = ds->GetCompatibleColor(16);
		ds->DrawRect(Rect(xx1, yy1, xx2, yy2), draw_color);
		/*    draw_color = ds->GetCompatibleColor(opts.tws.backcol); ds->FillRect(Rect(xx1,yy1,xx2,yy2);
		draw_color = ds->GetCompatibleColor(opts.tws.ds->GetTextColor()); ds->DrawRect(Rect(xx1+1,yy1+1,xx2-1,yy2-1);*/
	} else {
		if (_G(loaded_game_file_version) < kGameVersion_262) { // < 2.62
			// Color 0 wrongly shows as transparent instead of black
			// From the changelog of 2.62:
			//  - Fixed text windows getting a black background if colour 0 was
			//    specified, rather than being transparent.
			if (iep->BgColor == 0)
				iep->BgColor = 16;
		}

		if (iep->BgColor >= 0) draw_color = ds->GetCompatibleColor(iep->BgColor);
		else draw_color = ds->GetCompatibleColor(0); // black backrgnd behind picture

		if (iep->BgColor > 0)
			ds->FillRect(Rect(xx1, yy1, xx2, yy2), draw_color);

		int leftRightWidth = _GP(game).SpriteInfos[get_but_pic(iep, 4)].Width;
		int topBottomHeight = _GP(game).SpriteInfos[get_but_pic(iep, 6)].Height;
		if (iep->BgImage > 0) {
			if ((_G(loaded_game_file_version) <= kGameVersion_272) // 2.xx
			        && (_GP(spriteset)[iep->BgImage]->GetWidth() == 1)
			        && (_GP(spriteset)[iep->BgImage]->GetHeight() == 1)
			        && (*((const unsigned int *)_GP(spriteset)[iep->BgImage]->GetData()) == 0x00FF00FF)) {
				// Don't draw fully transparent dummy GUI backgrounds
			} else {
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
		int uu;
		for (uu = yy1; uu <= yy2; uu += _GP(game).SpriteInfos[get_but_pic(iep, 4)].Height) {
			do_corner(ds, get_but_pic(iep, 4), xx1, uu, -1, 0);   // left side
			do_corner(ds, get_but_pic(iep, 5), xx2 + 1, uu, 0, 0);  // right side
		}
		for (uu = xx1; uu <= xx2; uu += _GP(game).SpriteInfos[get_but_pic(iep, 6)].Width) {
			do_corner(ds, get_but_pic(iep, 6), uu, yy1, 0, -1);  // top side
			do_corner(ds, get_but_pic(iep, 7), uu, yy2 + 1, 0, 0); // bottom side
		}
		do_corner(ds, get_but_pic(iep, 0), xx1, yy1, -1, -1);  // top left
		do_corner(ds, get_but_pic(iep, 1), xx1, yy2 + 1, -1, 0);  // bottom left
		do_corner(ds, get_but_pic(iep, 2), xx2 + 1, yy1, 0, -1);  //  top right
		do_corner(ds, get_but_pic(iep, 3), xx2 + 1, yy2 + 1, 0, 0);  // bottom right
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
		int textx = (ds->GetWidth() / 2) - wgettextwidth_compensate(_GP(topBar).text, _GP(topBar).font) / 2;
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
