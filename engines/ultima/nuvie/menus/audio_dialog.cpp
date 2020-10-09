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


#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_types.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_text.h"
#include "ultima/nuvie/gui/gui_text_toggle_button.h"
#include "ultima/nuvie/gui/gui_callback.h"
#include "ultima/nuvie/gui/gui_area.h"
#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/menus/audio_dialog.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/nuvie.h"

namespace Ultima {
namespace Nuvie {

#define AD_WIDTH 292
#define AD_HEIGHT 166

AudioDialog::AudioDialog(GUI_CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - AD_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - AD_HEIGHT) / 2,
	             AD_WIDTH, AD_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE) {
	callback_object = callback;
	init();
	grab_focus();
}

bool AudioDialog::init() {
	int height = 12;
	int colX[] = { 213, 243 };
	int textX[] = { 9, 19, 29 };
	int buttonY = 9;
	uint8 textY = 11;
	uint8 row_h = 13;
	b_index_num = -1;
	last_index = 0;

	GUI_Widget *widget;
	GUI_Font *font = GUI::get_gui()->get_font();

	widget = (GUI_Widget *) new GUI_Text(textX[0], textY, 0, 0, 0, "Audio:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[1], textY += row_h, 0, 0, 0, "Enable music:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[2], textY += row_h, 0, 0, 0, "Music volume:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[2], textY += row_h, 0, 0, 0, "Combat changes music:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[2], textY += row_h, 0, 0, 0, "Vehicle changes music:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[2], textY += row_h, 0, 0, 0, "Conversations stop music:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[2], textY += row_h, 0, 0, 0, "Stop music on group change:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[1], textY += row_h, 0, 0, 0, "Enable sfx:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(textX[2], textY += row_h, 0, 0, 0, "Sfx volume:", font);
	AddWidget(widget);
	bool use_speech_b = (Game::get_game()->get_game_type() == NUVIE_GAME_U6 && has_fmtowns_support(Game::get_game()->get_config()));
	if (use_speech_b) {
		widget = (GUI_Widget *) new GUI_Text(textX[1], textY += row_h, 0, 0, 0, "Enable speech:", font);
		AddWidget(widget);
	}
	char musicBuff[5], sfxBuff[5];
	int sfxVol_selection, musicVol_selection, num_of_sfxVol, num_of_musicVol;
	SoundManager *sm = Game::get_game()->get_sound_manager();
	const char *const enabled_text[] = { "Disabled", "Enabled" };
	const char *const yes_no_text[] = { "no", "yes" };

	uint8 music_percent = round(sm->get_music_volume() / 2.55); // round needed for 10%, 30%, etc.
	sprintf(musicBuff, "%u%%", music_percent);
	const char *const musicVol_text[] = { "0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%", musicBuff };

	if (music_percent % 10 == 0) {
		num_of_musicVol = 11;
		musicVol_selection = music_percent / 10;
	} else {
		num_of_musicVol = 12;
		musicVol_selection = 11;
	}

	uint8 sfx_percent = round(sm->get_sfx_volume() / 2.55); // round needed for 10%, 30%, etc.
	sprintf(sfxBuff, "%u%%", sfx_percent);
	const char *const sfxVol_text[] = { "0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%", sfxBuff };

	if (sfx_percent % 10 == 0) {
		num_of_sfxVol = 11;
		sfxVol_selection = sfx_percent / 10;
	} else {
		num_of_sfxVol = 12;
		sfxVol_selection = 11;
	}

	audio_button = new GUI_TextToggleButton(this, colX[0], buttonY, 70, height, enabled_text, 2, sm->is_audio_enabled(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(audio_button);
	button_index[last_index] = audio_button;

	music_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, sm->is_music_enabled(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(music_button);
	button_index[last_index += 1] = music_button;

	musicVol_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, musicVol_text, num_of_musicVol, musicVol_selection, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(musicVol_button);
	button_index[last_index += 1] = musicVol_button;

	Party *party = Game::get_game()->get_party();
	combat_b = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, party->combat_changes_music, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(combat_b);
	button_index[last_index += 1] = combat_b;

	vehicle_b = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, party->vehicles_change_music, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(vehicle_b);
	button_index[last_index += 1] = vehicle_b;

	bool stop_converse = Game::get_game()->get_converse()->conversations_stop_music;
	converse_b = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, stop_converse, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(converse_b);
	button_index[last_index += 1] = converse_b;

	group_b = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, sm->stop_music_on_group_change, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(group_b);
	button_index[last_index += 1] = group_b;

	sfx_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, sm->is_sfx_enabled(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(sfx_button);
	button_index[last_index += 1] = sfx_button;

	sfxVol_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, sfxVol_text, num_of_sfxVol, sfxVol_selection, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(sfxVol_button);
	button_index[last_index += 1] = sfxVol_button;

	if (use_speech_b) {
		speech_b = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, 40, height, yes_no_text, 2, sm->is_speech_enabled(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
		AddWidget(speech_b);
		button_index[last_index += 1] = speech_b;
	} else
		speech_b = NULL;
	cancel_button = new GUI_Button(this, 80, AD_HEIGHT - 20, 54, height, "Cancel", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(cancel_button);
	button_index[last_index += 1] = cancel_button;

	save_button = new GUI_Button(this, 151, AD_HEIGHT - 20, 60, height, "Save", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(save_button);
	button_index[last_index += 1] = save_button;

	return true;
}

AudioDialog::~AudioDialog() {
}

GUI_status AudioDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(0, this, this);
	return GUI_YUM;
}

GUI_status AudioDialog::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case NORTH_KEY:
	case WEST_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num <= 0)
			b_index_num = last_index;
		else
			b_index_num = b_index_num - 1;
		button_index[b_index_num]->set_highlighted(true);
		break;
	case SOUTH_KEY:
	case EAST_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num == last_index)
			b_index_num = 0;
		else
			b_index_num += 1;
		button_index[b_index_num]->set_highlighted(true);
		break;
	case DO_ACTION_KEY:
		if (b_index_num != -1) return button_index[b_index_num]->Activate_button();
		break;
	case CANCEL_ACTION_KEY:
		return close_dialog();
	default:
		keybinder->handle_always_available_keys(a);
		break;
	}
	return GUI_YUM;
}

GUI_status AudioDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (caller == cancel_button) {
		return close_dialog();
	} else if (caller == save_button) {
		Configuration *config = Game::get_game()->get_config();
		SoundManager *sm = Game::get_game()->get_sound_manager();

		int music_selection = musicVol_button->GetSelection();
		if (music_selection != 11) {
			uint8 musicVol = music_selection * 25.5;
			sm->set_music_volume(musicVol);
			if (sm->get_m_pCurrentSong() != NULL)
				sm->get_m_pCurrentSong()->SetVolume(musicVol);
			config->set("config/music_volume", musicVol);
		}

		int sfx_selection = sfxVol_button->GetSelection();
		if (sfx_selection != 11) {
			uint8 sfxVol = sfx_selection * 25.5;
			sm->set_sfx_volume(sfxVol);
// probably need to update sfx volume if we have background sfx implemented
			config->set("config/sfx_volume", sfxVol);
		}

		if ((bool)music_button->GetSelection() != sm->is_music_enabled())
			sm->set_music_enabled(music_button->GetSelection());
		config->set("config/music_mute", !music_button->GetSelection());
		if ((bool)sfx_button->GetSelection() != sm->is_sfx_enabled())
			sm->set_sfx_enabled(sfx_button->GetSelection());

		Party *party = Game::get_game()->get_party();
		party->combat_changes_music = combat_b->GetSelection();
		config->set("config/audio/combat_changes_music", combat_b->GetSelection() ? "yes" : "no");

		party->vehicles_change_music = vehicle_b->GetSelection();
		config->set("config/audio/vehicles_change_music", vehicle_b->GetSelection() ? "yes" : "no");

		Game::get_game()->get_converse()->conversations_stop_music = converse_b->GetSelection();
		config->set("config/audio/conversations_stop_music", converse_b->GetSelection() ? "yes" : "no");

		sm->stop_music_on_group_change = group_b->GetSelection();
		config->set("config/audio/stop_music_on_group_change", group_b->GetSelection() ? "yes" : "no");

		config->set("config/sfx_mute", !sfx_button->GetSelection());
		if ((bool)audio_button->GetSelection() != sm->is_audio_enabled())
			sm->set_audio_enabled(audio_button->GetSelection());
		config->set("config/mute", !audio_button->GetSelection());

		if (speech_b) {
			bool speech_enabled = speech_b->GetSelection() ? true : false;
			config->set("config/speech_mute", !speech_b->GetSelection());
			if (speech_enabled != sm->is_speech_enabled())
				sm->set_speech_enabled(speech_enabled);
		}
		config->write();
		return close_dialog();
	}

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
