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

#include "bladerunner/ui/kia_section_settings.h"

#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/music.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/ui_check_box.h"
#include "bladerunner/ui/ui_container.h"
#include "bladerunner/ui/ui_image_picker.h"
#include "bladerunner/ui/ui_slider.h"
#include "bladerunner/ui/ui_scroll_box.h"
#include "bladerunner/ui/ui_dropdown.h"

#include "audio/mixer.h"
#include "common/keyboard.h"
#include "common/debug.h"

namespace BladeRunner {

const char *KIASectionSettings::kLeary = "LEARY";

const Color256 KIASectionSettings::kColors[] = {
	{   0,   0,   0 }, // Black - unpressed (framing rectange)
	{  16,   8,   8 },
	{  32,  24,   8 },
	{  56,  32,  16 },
	{  72,  48,  16 },
	{  88,  56,  24 }, // Mouse-over (framing rectange)
	{ 104,  72,  32 },
	{ 128,  80,  40 },
	{ 136,  96,  48 },
	{ 152, 112,  56 },
	{ 168, 128,  72 }  // Pressed (framing rectange)
};

KIASectionSettings::KIASectionSettings(BladeRunnerEngine *vm)
	: KIASectionBase(vm) {

	_uiContainer          = new UIContainer(_vm);

#if BLADERUNNER_ORIGINAL_SETTINGS
	_musicVolume          = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 160, 460, 170), 101, 0);
	_soundEffectVolume    = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 185, 460, 195), 101, 0);
	_ambientSoundVolume   = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 210, 460, 220), 101, 0);
	_speechVolume         = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 235, 460, 245), 101, 0);
	_gammaCorrection      = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 260, 460, 270), 101, 0);
#else
	_musicVolume          = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 160, 460, 170), _vm->_mixer->kMaxMixerVolume, 0);
	_soundEffectVolume    = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 185, 460, 195), _vm->_mixer->kMaxMixerVolume, 0);
	_speechVolume         = new UISlider(_vm, sliderCallback, this, Common::Rect(180, 210, 460, 220), _vm->_mixer->kMaxMixerVolume, 0);
#endif
	_subtitlesEnable                   = nullptr;

	if (_vm->_language == Common::RU_RUS) {
		// expanded click-bounding box x-axis
		_directorsCut         = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(180, 364, 436, 374), 0, false);
		if (_vm->_subtitles->isSystemActive()) {
			// moved to new line
			_subtitlesEnable  = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(276, 376, 345, 386), 0, false);
		}
	} else {
		_directorsCut         = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(180, 364, 270, 374), 0, false);
		if (_vm->_subtitles->isSystemActive()) {
			// moved further to the right to avoid overlap with 'Designer's Cut' in some language versions (ESP)
			_subtitlesEnable  = new UICheckBox(_vm, checkBoxCallback, this, Common::Rect(311, 364, 380, 374), 0, false);
		}
	}

	_selectedTextLanguageStr = "";
	_selectedTextLanguageId = -1;
	_textLanguageDropdown = nullptr;
	//
	// Section commented out for 2.2.0 release since it's not ready for production
	//
	//if (_vm->_subtitles->isSystemActive()) {
	//	// TODO initialize with default values or ConfMan() in open()?
	//	_selectedTextLanguageStr = "";
	//	_selectedTextLanguageId = -1;
	//	// Put at height on top of Music setting
	//	// This avoids interference and handling the case of when BLADERUNNER_ORIGINAL_SETTINGS is set
	//	//      (in which case the middle part of the KIA screen is filled with controls)
	//	_textLanguageDropdown = new UIDropDown(_vm,
	//	                                       dropdownSelectedCallback,
	//	                                       dropdownCancelledCallback,
	//	                                       dropdownClickedTopFrameCallback,
	//	                                       this,
	//	                                       "",
	//	                                       0,
	//	                                       136,
	//	                                       Subtitles::kMaxLanguageSelectionNum);
	//}

	_playerAgendaSelector = new UIImagePicker(_vm, 5);

	_uiContainer->add(_musicVolume);
	_uiContainer->add(_soundEffectVolume);
	_uiContainer->add(_speechVolume);
#if BLADERUNNER_ORIGINAL_SETTINGS
	_uiContainer->add(_ambientSoundVolume);
	_uiContainer->add(_gammaCorrection);
#endif
	_uiContainer->add(_directorsCut);
	if (_vm->_subtitles->isSystemActive()) {
		_uiContainer->add(_subtitlesEnable);
		// Note: Keep _textLanguageDropdown last added to _uiContainer
		// in order to be able to set it as the only active object
		// when the language selection dropdown is shown.
		//
		// commented out for 2.2.0 release since _textLanguageDropdown is not ready for production
		//
		//_uiContainer->add(_textLanguageDropdown);
	}

	_state = kStateNormal;

	_mouseX = 0;
	_mouseY = 0;

	_learyPos = 0;
}

KIASectionSettings::~KIASectionSettings() {
	delete _uiContainer;
	delete _musicVolume;
	delete _soundEffectVolume;
	delete _speechVolume;
#if BLADERUNNER_ORIGINAL_SETTINGS
	delete _ambientSoundVolume;
	delete _gammaCorrection;
#endif
	delete _directorsCut;
	if (_vm->_subtitles->isSystemActive()) {
		//
		// commented out for 2.2.0 release since _textLanguageDropdown is not ready for production
		//
		//delete _textLanguageDropdown;
		delete _subtitlesEnable;
	}
	delete _playerAgendaSelector;
}

void KIASectionSettings::open() {
	_state = kStateNormal;

	_playerAgendaSelector->resetImages();

	_playerAgendaSelector->defineImage(kPlayerAgendaPolite,     Common::Rect(180, 290, 227, 353), nullptr, nullptr, nullptr, _vm->_textOptions->getText(30));
	_playerAgendaSelector->defineImage(kPlayerAgendaNormal,     Common::Rect(238, 290, 285, 353), nullptr, nullptr, nullptr, _vm->_textOptions->getText(31));
	_playerAgendaSelector->defineImage(kPlayerAgendaSurly,      Common::Rect(296, 290, 343, 353), nullptr, nullptr, nullptr, _vm->_textOptions->getText(32));
	_playerAgendaSelector->defineImage(kPlayerAgendaErratic,    Common::Rect(354, 290, 401, 353), nullptr, nullptr, nullptr, _vm->_textOptions->getText(33));
	_playerAgendaSelector->defineImage(kPlayerAgendaUserChoice, Common::Rect(412, 290, 459, 353), nullptr, nullptr, nullptr, _vm->_textOptions->getText(34));
	initConversationChoices();
	_playerAgendaSelector->activate(mouseInCallback, nullptr, nullptr, mouseUpCallback, this);

	_directorsCut->enable();
	if (_vm->_subtitles->isSystemActive()) {
		_subtitlesEnable->enable();
		//
		// commented out for 2.2.0 release since _textLanguageDropdown is not ready for production
		//
		//_textLanguageDropdown->activate();
		populateLanguageSelection();
	}
}

void KIASectionSettings::close() {
	_playerAgendaSelector->deactivate();
	//
	// commented out for 2.2.0 release since _textLanguageDropdown is not ready for production
	//
	//if (_vm->_subtitles->isSystemActive()) {
	//	_textLanguageDropdown->deactivate();
	//}
}

void KIASectionSettings::draw(Graphics::Surface &surface) {
#if BLADERUNNER_ORIGINAL_SETTINGS
	_musicVolume->setValue(_vm->_music->getVolume());
	_soundEffectVolume->setValue(_vm->_audioPlayer->getVolume());
	_ambientSoundVolume->setValue(_vm->_ambientSounds->getVolume());
	_speechVolume->setValue(_vm->_audioSpeech->getVolume());
	_gammaCorrection->setValue(100.0f);
#else
	_musicVolume->setValue(_vm->_mixer->getVolumeForSoundType(_vm->_mixer->kMusicSoundType));
	_soundEffectVolume->setValue(_vm->_mixer->getVolumeForSoundType(_vm->_mixer->kSFXSoundType));
	_speechVolume->setValue(_vm->_mixer->getVolumeForSoundType(_vm->_mixer->kSpeechSoundType));
#endif

	_directorsCut->setChecked(_vm->_gameFlags->query(kFlagDirectorsCut));

	if (_vm->_subtitles->isSystemActive()) {
		_subtitlesEnable->setChecked(_vm->isSubtitlesEnabled());
	}

	const char *textConversationChoices = _vm->_textOptions->getText(0);
	const char *textMusic = _vm->_textOptions->getText(2);
	const char *textSoundEffects = _vm->_textOptions->getText(3);
	const char *textSpeech = _vm->_textOptions->getText(5);
	const char *textSoft = _vm->_textOptions->getText(10);
	const char *textLoud = _vm->_textOptions->getText(11);
	const char *textDesignersCut = _vm->_textOptions->getText(18);
#if BLADERUNNER_ORIGINAL_SETTINGS
	const char *textAmbientSound = _vm->_textOptions->getText(4);
	const char *textGammaCorrection = _vm->_textOptions->getText(7);
	const char *textDark = _vm->_textOptions->getText(14);
	const char *textLight = _vm->_textOptions->getText(15);
#endif

	int posConversationChoices = 320 - _vm->_mainFont->getStringWidth(textConversationChoices) / 2;
	int posMusic = 320 - _vm->_mainFont->getStringWidth(textMusic) / 2;
	int posSoundEffects = 320 - _vm->_mainFont->getStringWidth(textSoundEffects) / 2;
	int posSpeech = 320 - _vm->_mainFont->getStringWidth(textSpeech) / 2;
	int posSoft = 178 - _vm->_mainFont->getStringWidth(textSoft);
#if BLADERUNNER_ORIGINAL_SETTINGS
	int posAmbientSound = 320 - _vm->_mainFont->getStringWidth(textAmbientSound) / 2;
	int posGammaCorrection = 320 - _vm->_mainFont->getStringWidth(textGammaCorrection) / 2;
	int posDark = 178 - _vm->_mainFont->getStringWidth(textDark);
#endif

	_playerAgendaSelector->draw(surface);

	_vm->_mainFont->drawString(&surface, textConversationChoices, posConversationChoices, 280, surface.w, surface.format.RGBToColor(232, 208, 136));

	_vm->_mainFont->drawString(&surface, textMusic, posMusic, 150, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, textSoft, posSoft, 161, surface.w, surface.format.RGBToColor(216, 184, 112));
	_vm->_mainFont->drawString(&surface, textLoud, 462, 161, surface.w, surface.format.RGBToColor(216, 184, 112));

	_vm->_mainFont->drawString(&surface, textSoundEffects, posSoundEffects, 175, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, textSoft, posSoft, 186, surface.w, surface.format.RGBToColor(216, 184, 112));
	_vm->_mainFont->drawString(&surface, textLoud, 462, 186, surface.w, surface.format.RGBToColor(216, 184, 112));

#if BLADERUNNER_ORIGINAL_SETTINGS
	_vm->_mainFont->drawString(&surface, textAmbientSound, posAmbientSound, 200, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, textSoft, posSoft, 211, surface.w, surface.format.RGBToColor(216, 184, 112));
	_vm->_mainFont->drawString(&surface, textLoud, 462, 211, surface.w, surface.format.RGBToColor(216, 184, 112));

	_vm->_mainFont->drawString(&surface, textSpeech, posSpeech, 225, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, textSoft, posSoft, 236, surface.w, surface.format.RGBToColor(216, 184, 112));
	_vm->_mainFont->drawString(&surface, textLoud, 462, 236, surface.w, surface.format.RGBToColor(216, 184, 112));

	_vm->_mainFont->drawString(&surface, textGammaCorrection, posGammaCorrection, 250, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, textDark, posDark, 261, surface.w, surface.format.RGBToColor(216, 184, 112));
	_vm->_mainFont->drawString(&surface, textLight, 462, 261, surface.w, surface.format.RGBToColor(216, 184, 112));
#else
	_vm->_mainFont->drawString(&surface, textSpeech, posSpeech, 200, surface.w, surface.format.RGBToColor(232, 208, 136));
	_vm->_mainFont->drawString(&surface, textSoft, posSoft, 211, surface.w, surface.format.RGBToColor(216, 184, 112));
	_vm->_mainFont->drawString(&surface, textLoud, 462, 211, surface.w, surface.format.RGBToColor(216, 184, 112));
#endif

	_vm->_mainFont->drawString(&surface, textDesignersCut, 192, 365, surface.w, surface.format.RGBToColor(232, 208, 136));

	if (_vm->_subtitles->isSystemActive()) {
		// Allow this to be loading as an extra text item in the resource for text options
		const char *subtitlesTranslation      = nullptr;
		const char *languageSelectTranslation = nullptr;
		switch (_vm->_language) {
		case Common::EN_ANY:
		default:
			subtitlesTranslation      = "Subtitles";
			languageSelectTranslation = "Text Language:";
			break;
		case Common::DE_DEU:
			subtitlesTranslation = "Untertitel";
			languageSelectTranslation = "Text Language:"; // TODO DEUTCH translation
			break;
		case Common::FR_FRA:
			subtitlesTranslation = "Sous-titres";
			languageSelectTranslation = "Text Language:"; // TODO FRENCH translation
			break;
		case Common::IT_ITA:
			subtitlesTranslation = "Sottotitoli";
			languageSelectTranslation = "Text Language:"; // TODO ITALIAN translation
			break;
		case Common::ES_ESP:
			// the spanish text must have accented í
			subtitlesTranslation = "Subt\xa1tulos";
			languageSelectTranslation = "Text Language:"; // TODO SPANISH translation
			break;
		case Common::RU_RUS:
			// субтитры
			if (_vm->_russianCP1251) {
				// Patched translation by Siberian Studio is using Windows-1251 encoding
				subtitlesTranslation = "\xf1\xf3\xe1\xf2\xe8\xf2\xf0\xfb";
				languageSelectTranslation = "Text Language:"; // TODO RUSSIAN translation + proper characters for encoding
			} else {
				// Original release uses custom encoding
				subtitlesTranslation = "CE,NBNHS";
				languageSelectTranslation = "Text Language:"; // TODO RUSSIAN translation + proper characters for this encoding
			}
			break;
		}
		// +2 to the max of original index of textOptions which is 41
		const char *textSubtitles  = strcmp(_vm->_textOptions->getText(42), "") == 0 ? subtitlesTranslation : _vm->_textOptions->getText(42);
		//const char *textLanguageSelect = strcmp(_vm->_textOptions->getText(43), "") == 0 ? languageSelectTranslation : _vm->_textOptions->getText(43);

		if (_vm->_language == Common::RU_RUS) {
			// special case for Russian version, put the option in a new line to avoid overlap
			_vm->_mainFont->drawString(&surface, textSubtitles, 288, 376, surface.w, surface.format.RGBToColor(232, 208, 136));
		} else {
			// moved further to the right to avoid overlap with 'Designer's Cut' in some language versions (ESP)
			_vm->_mainFont->drawString(&surface, textSubtitles, 323, 365, surface.w, surface.format.RGBToColor(232, 208, 136));
		}

		// Vertical Align with "Soft" label
		//
		// commented out for 2.2.0 release since _textLanguageDropdown is not ready for production
		//
		//_textLanguageDropdown->setControlLeft(posSoft);
		//_textLanguageDropdown->setLabelStr(textLanguageSelect);
	}

	// Draw uiContainer contained objects after drawing the text on the section for music, sound, speech etc.
	_uiContainer->draw(surface);

	_playerAgendaSelector->drawTooltip(surface, _mouseX, _mouseY);
}

void KIASectionSettings::handleKeyDown(const Common::KeyState &kbd) {
	if (_state == kStateNormal) {
		if (toupper(kbd.ascii) != kLeary[_learyPos]) {
			_learyPos = 0;
		}

		if (toupper(kbd.ascii) == kLeary[_learyPos]) {
			++_learyPos;
			if (!kLeary[_learyPos]) {
				_vm->_settings->setLearyMode(!_vm->_settings->getLearyMode());
				_learyPos = 0;
				initConversationChoices();
			}
		}
	}
}

void KIASectionSettings::handleMouseMove(int mouseX, int mouseY) {
	_uiContainer->handleMouseMove(mouseX, mouseY);

	_mouseX = mouseX;
	_mouseY = mouseY;

	_playerAgendaSelector->handleMouseAction(mouseX, mouseY, false, false, false);
}

void KIASectionSettings::handleMouseDown(bool mainButton) {
	if (mainButton) {
		_uiContainer->handleMouseDown(false);

		_playerAgendaSelector->handleMouseAction(_mouseX, _mouseY, true, false, false);
	}
}

void KIASectionSettings::handleMouseUp(bool mainButton) {
	if (mainButton) {
		_uiContainer->handleMouseUp(false);

		_playerAgendaSelector->handleMouseAction(_mouseX, _mouseY, false, true, false);
	}
}

void KIASectionSettings::handleMouseScroll(int direction) {
	if (_vm->_subtitles->isSystemActive() && _state == kStateLanguageSelect) {
		_uiContainer->handleMouseScroll(direction);
	}
}

void KIASectionSettings::sliderCallback(void *callbackData, void *source) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;

#if BLADERUNNER_ORIGINAL_SETTINGS
	if (source == self->_musicVolume) {
		self->_vm->_music->setVolume(self->_musicVolume->_value);
		self->_vm->_music->playSample();
	} else if (source == self->_soundEffectVolume) {
		self->_vm->_audioPlayer->setVolume(self->_soundEffectVolume->_value);
		self->_vm->_audioPlayer->playSample();
	} else if (source == self->_ambientSoundVolume) {
		self->_vm->_ambientSounds->setVolume(self->_ambientSoundVolume->_value);
		self->_vm->_ambientSounds->playSample();
	} else if (source == self->_speechVolume) {
		self->_vm->_audioSpeech->setVolume(self->_speechVolume->_value);
		self->_vm->_audioSpeech->playSample();
	} else if (source == self->_gammaCorrection) {
		// TODO: gamma, should we support it?
		// gamma = self->_gammaCorrection._value / 100.0f;
		// settings::setGamma(&Settings, gamma);
		// colorFormat = DirectDrawSurfaces_get_colorFormat();
		// Palette_fill(colorFormat);
		// Palette_copy(Palette);
		// kia::resume(KIA);
	}
#else
	if (source == self->_musicVolume) {
		ConfMan.setInt("music_volume", self->_musicVolume->_value);
		self->_vm->syncSoundSettings();
		self->_vm->_music->playSample();
	} else if (source == self->_soundEffectVolume) {
		ConfMan.setInt("sfx_volume", self->_soundEffectVolume->_value);
		self->_vm->syncSoundSettings();
		self->_vm->_audioPlayer->playSample();
	} else if (source == self->_speechVolume) {
		ConfMan.setInt("speech_volume", self->_speechVolume->_value);
		self->_vm->syncSoundSettings();
		self->_vm->_audioSpeech->playSample();
	}
#endif
}

void KIASectionSettings::checkBoxCallback(void *callbackData, void *source) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;
	if (source == self->_directorsCut) {
		if (self->_directorsCut->_isChecked) {
			self->_vm->_gameFlags->set(kFlagDirectorsCut);
		} else {
			self->_vm->_gameFlags->reset(kFlagDirectorsCut);
		}
	} else if (self->_vm->_subtitles->isSystemActive() && source == self->_subtitlesEnable) {
		self->_vm->setSubtitlesEnabled(self->_subtitlesEnable->_isChecked);
	}
}

void KIASectionSettings::mouseInCallback(int buttonId, void *callbackData) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;
	self->_vm->_audioPlayer->playAud(self->_vm->_gameInfo->getSfxTrack(kSfxTEXT3), 100, 0, 0, 50, 0);
}

void KIASectionSettings::mouseUpCallback(int buttonId, void *callbackData) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;
	self->onButtonPressed(buttonId);
}

void KIASectionSettings::onButtonPressed(int buttonId) {
	switch (buttonId) {
	case kPlayerAgendaPolite:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, -30, -30, 50, 0);
		_vm->_settings->setPlayerAgenda(kPlayerAgendaPolite);
		initConversationChoices();
		break;
	case kPlayerAgendaNormal:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, -15, -15, 50, 0);
		_vm->_settings->setPlayerAgenda(kPlayerAgendaNormal);
		initConversationChoices();
		break;
	case kPlayerAgendaSurly:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, 0, 0, 50, 0);
		_vm->_settings->setPlayerAgenda(kPlayerAgendaSurly);
		initConversationChoices();
		break;
	case kPlayerAgendaErratic:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, 15, 15, 50, 0);
		_vm->_settings->setPlayerAgenda(kPlayerAgendaErratic);
		initConversationChoices();
		break;
	case kPlayerAgendaUserChoice:
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxELECBP1), 90, 30, 30, 50, 0);
		_vm->_settings->setPlayerAgenda(kPlayerAgendaUserChoice);
		initConversationChoices();
		break;
	default:
		return;
	}
}

void KIASectionSettings::initConversationChoices() {
	for (int i = 0; i < 5; ++i) {
		const Shape *shape = nullptr;
		if (_vm->_settings->getPlayerAgenda() == i) {
			if (i == kPlayerAgendaUserChoice) {
				shape = _vm->_kia->_shapes->get(122);
			} else if (_vm->_settings->getLearyMode()) {
				shape = _vm->_kia->_shapes->get(106 + i);
			} else {
				shape = _vm->_kia->_shapes->get(114 + i);
			}
		} else {
			if (i == kPlayerAgendaUserChoice) {
				shape = _vm->_kia->_shapes->get(123);
			} else if (_vm->_settings->getLearyMode()) {
				shape = _vm->_kia->_shapes->get(110 + i);
			} else {
				shape = _vm->_kia->_shapes->get(118 + i);
			}
		}

		_playerAgendaSelector->setImageShapeUp(i, shape);
		_playerAgendaSelector->setImageShapeHovered(i, shape);
		_playerAgendaSelector->setImageShapeDown(i, shape);
	}
}

void KIASectionSettings::populateLanguageSelection() {
	if (_textLanguageDropdown != nullptr) {
		_textLanguageDropdown->clearLines();
		_textLanguageDropdown->addLine("English v7 [ENG] (SCUMMVM)", 1);
		_textLanguageDropdown->addLine("French v7 [FRA] (Kwama57)", 2);
		_textLanguageDropdown->addLine("Spanish v7 [ESP] (Victor G. Fraile & GeekOb)", 3);
		_textLanguageDropdown->addLine("Greek v1 [ENG] (Praetorian)", 4);
		_textLanguageDropdown->addLine("Hebrew v1 [ENG] (Rzil)", 5);
		_textLanguageDropdown->addLine("Chinese v0 [ENG] (*)", 6);
		_textLanguageDropdown->addLine("Russian v1 [ENG] (*)", 7);
		_textLanguageDropdown->addLine("Italian v0 [ITA] (*)", 8);
		_textLanguageDropdown->addLine("Deutsch v0 [DEU] (*)", 9);
	}
}

void KIASectionSettings::changeState(State state) {
	_state = state;
	if (state != kStateNormal) {
		_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(kSfxSPNBEEP7), 90, 0, 0, 50, 0);
	}
}

void KIASectionSettings::dropdownSelectedCallback(void *callbackData, void *source, int lineData, int mouseButton) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;

	if (source == self->_textLanguageDropdown && lineData >= 0) {
		self->_selectedTextLanguageStr = self->_textLanguageDropdown->getLineSelectedStr();
		self->_selectedTextLanguageId = lineData;
		self->showTextSelectionDropdown(false);
	}
}

void KIASectionSettings::dropdownCancelledCallback(void *callbackData, void *source) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;
	if (source == self->_textLanguageDropdown) {
		self->showTextSelectionDropdown(false);
	}
}

void KIASectionSettings::dropdownClickedTopFrameCallback(void *callbackData, void *source) {
	KIASectionSettings *self = (KIASectionSettings *)callbackData;

	if (source == self->_textLanguageDropdown) {
		self->showTextSelectionDropdown(!self->_textLanguageDropdown->isDropDownMenuExpanded());
	}
}

void KIASectionSettings::showTextSelectionDropdown(bool showToggle) {
	if (showToggle) {
		// scrollable area will be expanded
		populateLanguageSelection();
		_uiContainer->setHandleSpecificNumOfTopLayers(1);
		changeState(kStateLanguageSelect);
	} else {
		// scrollable area will be collapsed
		_uiContainer->setHandleSpecificNumOfTopLayers(0);
		changeState(kStateNormal);
	}
}

} // End of namespace BladeRunner
