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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/settings.h"

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/system.h"
#include "graphics/paletteman.h"

#include "ripper/detection.h"

namespace Ripper {

static const RipperSettings::Descriptor kSliderDescriptors[RipperSettings::kSliderCount] = {
	{   0, 100,  5, 100 },
	{   0, 100,  5, 100 },
	{   0, 100,  5, 100 },
	{   0, 100,  5, 100 },
	{  50, 200,  5, 100 },
	{   0, 200,  5, 100 },
	{  80, 120, -2, 100 },
	{   0, 200,  5, 100 }
};

static const char *const kVisualConfigKeys[] = {
	"ripper_brightness",
	"ripper_color",
	"ripper_contrast",
	"ripper_tint"
};

static const uint16 kDefaultActionKeys[RipperSettings::kActionKeyCount] = {
	0x2e00, 0x1100, 0x1700, 0x0077, 0x1f00,
	0x1300, 0x1800, 0x3b00, 0x1000
};

static const char *const kActionKeyConfigKeys[RipperSettings::kActionKeyCount] = {
	"ripper_action_key_0", "ripper_action_key_1", "ripper_action_key_2",
	"ripper_action_key_3", "ripper_action_key_4", "ripper_action_key_5",
	"ripper_action_key_6", "ripper_action_key_7", "ripper_action_key_8"
};

static const char *const kDemoReservedToggleConfigKeys[] = {
	"ripper_demo_option_4", "ripper_demo_option_5"
};

RipperSettings::RipperSettings(Audio::Mixer *mixer) : _mixer(mixer), _videoMode(1),
		_combatLevel(2), _puzzleLevel(2), _bufferedVideo(false), _subtitles(true),
		_toolbarHelp(true), _toolbarPermanent(false), _mouseSensitivity(100) {
	_demoReservedToggles[0] = true;
	_demoReservedToggles[1] = true;
	for (uint i = 0; i < kSliderCount; ++i)
		_values[i] = kSliderDescriptors[i].defaultValue;
	for (uint i = 0; i < kActionKeyCount; ++i)
		_actionKeys[i] = kDefaultActionKeys[i];
}

const RipperSettings::Descriptor &RipperSettings::getDescriptor(Slider slider) {
	return kSliderDescriptors[slider];
}

int RipperSettings::mixerToPercent(int volume) {
	return CLIP<int>((volume * 100 + Audio::Mixer::kMaxMixerVolume / 2) /
		Audio::Mixer::kMaxMixerVolume, 0, 100);
}

int RipperSettings::percentToMixer(int volume) {
	return CLIP<int>((volume * Audio::Mixer::kMaxMixerVolume + 50) / 100,
		0, Audio::Mixer::kMaxMixerVolume);
}

void RipperSettings::load() {
	_values[kMasterVolume] = ConfMan.hasKey("ripper_master_volume") ?
		ConfMan.getInt("ripper_master_volume") : kSliderDescriptors[kMasterVolume].defaultValue;
	_values[kAmbientVolume] = mixerToPercent(ConfMan.getInt("music_volume"));
	_values[kSfxVolume] = mixerToPercent(ConfMan.getInt("sfx_volume"));
	_values[kVideoVolume] = mixerToPercent(ConfMan.getInt("speech_volume"));

	for (uint i = kBrightness; i <= kTint; ++i) {
		const char *key = kVisualConfigKeys[i - kBrightness];
		_values[i] = ConfMan.hasKey(key) ? ConfMan.getInt(key) : kSliderDescriptors[i].defaultValue;
	}
	for (uint i = 0; i < kSliderCount; ++i)
		_values[i] = CLIP<int>(_values[i], kSliderDescriptors[i].minimum,
			kSliderDescriptors[i].maximum);

	_bufferedVideo = ConfMan.hasKey("ripper_buffered_video") ?
		ConfMan.getBool("ripper_buffered_video") : false;
	_subtitles = ConfMan.hasKey("subtitles") ? ConfMan.getBool("subtitles") : true;
	_toolbarHelp = ConfMan.hasKey("ripper_toolbar_help") ?
		ConfMan.getBool("ripper_toolbar_help") : true;
	_toolbarPermanent = ConfMan.hasKey("ripper_toolbar_permanent") ?
		ConfMan.getBool("ripper_toolbar_permanent") : false;
	for (uint i = 0; i < ARRAYSIZE(_demoReservedToggles); ++i)
		_demoReservedToggles[i] = ConfMan.hasKey(kDemoReservedToggleConfigKeys[i]) ?
			ConfMan.getBool(kDemoReservedToggleConfigKeys[i]) : true;
	_mouseSensitivity = CLIP<int>(ConfMan.hasKey("ripper_mouse_sensitivity") ?
		ConfMan.getInt("ripper_mouse_sensitivity") : 100, 0, 100);
	_videoMode = CLIP<int>(ConfMan.hasKey("ripper_video_mode") ?
		ConfMan.getInt("ripper_video_mode") : 1, 0, 3);
	_combatLevel = CLIP<int>(ConfMan.hasKey("ripper_combat_level") ?
		ConfMan.getInt("ripper_combat_level") : 2, 1, 3);
	_puzzleLevel = CLIP<int>(ConfMan.hasKey("ripper_puzzle_level") ?
		ConfMan.getInt("ripper_puzzle_level") : 2, 1, 3);
	for (uint i = 0; i < kActionKeyCount; ++i) {
		const int key = ConfMan.hasKey(kActionKeyConfigKeys[i]) ?
			ConfMan.getInt(kActionKeyConfigKeys[i]) : kDefaultActionKeys[i];
		_actionKeys[i] = CLIP<int>(key, 0, 0xffff);
	}

	applyAudioVolumes();
	debugC(1, kDebugGeneral,
		"Ripper: loaded Remote Control settings master=%d ambient=%d sfx=%d video=%d brightness=%d color=%d contrast=%d tint=%d",
		_values[kMasterVolume], _values[kAmbientVolume], _values[kSfxVolume],
		_values[kVideoVolume], _values[kBrightness], _values[kColor],
		_values[kContrast], _values[kTint]);
	debugC(1, kDebugGeneral,
		"Ripper: loaded Options Panel settings bufferedVideo=%d videoMode=%u combat=%u puzzle=%u",
		_bufferedVideo, _videoMode, _combatLevel, _puzzleLevel);
}

void RipperSettings::save() {
	ConfMan.setInt("ripper_master_volume", _values[kMasterVolume]);
	ConfMan.setInt("music_volume", percentToMixer(_values[kAmbientVolume]));
	ConfMan.setInt("sfx_volume", percentToMixer(_values[kSfxVolume]));
	ConfMan.setInt("speech_volume", percentToMixer(_values[kVideoVolume]));
	for (uint i = kBrightness; i <= kTint; ++i)
		ConfMan.setInt(kVisualConfigKeys[i - kBrightness], _values[i]);
	ConfMan.setBool("ripper_buffered_video", _bufferedVideo);
	ConfMan.setBool("subtitles", _subtitles);
	ConfMan.setBool("ripper_toolbar_help", _toolbarHelp);
	ConfMan.setBool("ripper_toolbar_permanent", _toolbarPermanent);
	for (uint i = 0; i < ARRAYSIZE(_demoReservedToggles); ++i)
		ConfMan.setBool(kDemoReservedToggleConfigKeys[i], _demoReservedToggles[i]);
	ConfMan.setInt("ripper_mouse_sensitivity", _mouseSensitivity);
	ConfMan.setInt("ripper_video_mode", _videoMode);
	ConfMan.setInt("ripper_combat_level", _combatLevel);
	ConfMan.setInt("ripper_puzzle_level", _puzzleLevel);
	for (uint i = 0; i < kActionKeyCount; ++i)
		ConfMan.setInt(kActionKeyConfigKeys[i], _actionKeys[i]);
	ConfMan.flushToDisk();
	debugC(1, kDebugGeneral,
		"Ripper: saved settings remoteControl=1 optionsPanel=1 bufferedVideo=%d videoMode=%u combat=%u puzzle=%u",
		_bufferedVideo, _videoMode, _combatLevel, _puzzleLevel);
}

void RipperSettings::resetDefaults() {
	for (uint i = 0; i < kSliderCount; ++i)
		_values[i] = kSliderDescriptors[i].defaultValue;
	applyAudioVolumes();
	debugC(2, kDebugGeneral, "Ripper: restored Remote Control defaults");
}

void RipperSettings::setValue(Slider slider, int value) {
	const Descriptor &descriptor = kSliderDescriptors[slider];
	const int clampedValue = CLIP<int>(value, descriptor.minimum, descriptor.maximum);
	if (_values[slider] == clampedValue)
		return;
	_values[slider] = clampedValue;
	if (slider <= kVideoVolume)
		applyAudioVolumes();
	debugC(2, slider <= kVideoVolume ? kDebugAudio : kDebugVideo,
		"Ripper: Remote Control slider=%u value=%d", (uint)slider, clampedValue);
}

void RipperSettings::applyAudioVolumes() {
	if (!_mixer)
		return;
	const int master = _values[kMasterVolume];
	const int ambient = percentToMixer(master * _values[kAmbientVolume] / 100);
	const int sfx = percentToMixer(master * _values[kSfxVolume] / 100);
	const int video = percentToMixer(master * _values[kVideoVolume] / 100);
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, percentToMixer(master));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ambient);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, sfx);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, video);
	debugC(3, kDebugAudio,
		"Ripper: applied audio profiles master=%d ambient=%d sfx=%d video=%d mixer=%d,%d,%d",
		master, _values[kAmbientVolume], _values[kSfxVolume], _values[kVideoVolume],
		ambient, sfx, video);
}

uint RipperSettings::getFilledTickCount(Slider slider) const {
	const Descriptor &descriptor = kSliderDescriptors[slider];
	const int range = descriptor.maximum - descriptor.minimum;
	if (range <= 0)
		return 0;
	const int distance = descriptor.step < 0 ? descriptor.maximum - _values[slider] :
		_values[slider] - descriptor.minimum;
	return CLIP<int>(distance * 10 / range, 0, 10);
}

void RipperSettings::applyVideoPalette(byte *palette, uint colorCount, bool rememberSource) {
	if (!palette || colorCount == 0)
		return;
	if (rememberSource) {
		_videoPaletteSource.resize(colorCount * 3);
		memcpy(_videoPaletteSource.data(), palette, colorCount * 3);
	}
	if (_values[kBrightness] == 100 && _values[kColor] == 100 &&
		_values[kContrast] == 100 && _values[kTint] == 100)
		return;

	const int brightness = _values[kBrightness];
	const int color = _values[kColor];
	const int contrast = _values[kContrast];
	const int tint = _values[kTint];
	const int tintDistance = ABS(tint - 100);
	for (uint i = 0; i < colorCount; ++i) {
		const int sourceRed = palette[i * 3] >> 2;
		const int sourceGreen = palette[i * 3 + 1] >> 2;
		const int sourceBlue = palette[i * 3 + 2] >> 2;
		int redTint = 0;
		int greenTint = 0;
		int blueTint = 0;
		if (tint < 100) {
			redTint = sourceGreen * tintDistance / 100 - sourceRed * tintDistance / 100;
			greenTint = (sourceBlue * tintDistance - sourceGreen * tintDistance) / 100;
			blueTint = -(sourceBlue * tintDistance / 100);
		} else if (tint > 100) {
			redTint = -(sourceRed * tintDistance / 100);
			greenTint = (sourceRed * tintDistance - sourceGreen * tintDistance) / 100;
			blueTint = (sourceGreen * tintDistance - sourceBlue * tintDistance) / 100;
		}

		int red = 63 - (63 - sourceRed) * contrast / 100 - sourceRed +
			brightness * sourceRed / 100 + redTint;
		int green = 63 - (63 - sourceGreen) * contrast / 100 - sourceGreen +
			brightness * sourceGreen / 100 + greenTint;
		int blue = 63 - (63 - sourceBlue) * contrast / 100 - sourceBlue +
			brightness * sourceBlue / 100 + blueTint;
		const int luma = (blue * 11 + green * 59 + red * 30) / 100;
		const int lumaContribution = (100 - color) * luma;
		red = CLIP<int>((color * red + lumaContribution) / 100, 0, 63);
		green = CLIP<int>((color * green + lumaContribution) / 100, 0, 63);
		blue = CLIP<int>((color * blue + lumaContribution) / 100, 0, 63);
		palette[i * 3] = (red << 2) | (red >> 4);
		palette[i * 3 + 1] = (green << 2) | (green >> 4);
		palette[i * 3 + 2] = (blue << 2) | (blue >> 4);
	}
	debugC(3, kDebugVideo,
		"Ripper: applied video palette settings colors=%u brightness=%d color=%d contrast=%d tint=%d",
		colorCount, brightness, color, contrast, tint);
}

bool RipperSettings::restoreVideoPalette() {
	if (_videoPaletteSource.size() != 256 * 3)
		return false;
	byte palette[256 * 3];
	memcpy(palette, _videoPaletteSource.data(), sizeof(palette));
	// The saved source already includes the shared interface palette patch made
	// immediately before applyVideoPalette installed the scene palette.
	applyVideoPalette(palette, 256, false);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	return true;
}

uint16 RipperSettings::getActionKey(uint index) const {
	return index < kActionKeyCount ? _actionKeys[index] : 0;
}

bool RipperSettings::getDemoToggle(DemoToggle toggle) const {
	switch (toggle) {
	case kDemoSubtitles:
		return _subtitles;
	case kDemoBufferedVideo:
		return _bufferedVideo;
	case kDemoToolbarHelp:
		return _toolbarHelp;
	case kDemoToolbarPermanent:
		return _toolbarPermanent;
	case kDemoReservedToggle4:
	case kDemoReservedToggle5:
		return _demoReservedToggles[toggle - kDemoReservedToggle4];
	default:
		return false;
	}
}

int RipperSettings::getDemoSlider(DemoSlider slider) const {
	switch (slider) {
	case kDemoSoundVolume:
		return _values[kMasterVolume];
	case kDemoMusicVolume:
		return _values[kAmbientVolume];
	case kDemoMouseSensitivity:
		return _mouseSensitivity;
	default:
		return 0;
	}
}

void RipperSettings::setBufferedVideo(bool enabled) {
	_bufferedVideo = enabled;
	debugC(2, kDebugGeneral,
		"Ripper: Options Panel bufferedVideo=%d", _bufferedVideo);
}

void RipperSettings::setVideoMode(uint mode) {
	_videoMode = MIN<uint>(mode, 3);
	debugC(2, kDebugGeneral,
		"Ripper: Options Panel videoMode=%u", _videoMode);
}

void RipperSettings::setCombatLevel(uint level) {
	_combatLevel = CLIP<uint>(level, 1, 3);
	debugC(2, kDebugGeneral,
		"Ripper: Options Panel combatLevel=%u", _combatLevel);
}

void RipperSettings::setPuzzleLevel(uint level) {
	_puzzleLevel = CLIP<uint>(level, 1, 3);
	debugC(2, kDebugGeneral,
		"Ripper: Options Panel puzzleLevel=%u", _puzzleLevel);
}

void RipperSettings::setActionKey(uint index, uint16 key) {
	if (index >= kActionKeyCount || key == 0 || _actionKeys[index] == key)
		return;
	_actionKeys[index] = key;
	debugC(2, kDebugInput,
		"Ripper: Options Panel actionKeySlot=%u command=0x%04x", index, key);
}

void RipperSettings::setDemoToggle(DemoToggle toggle, bool enabled) {
	switch (toggle) {
	case kDemoSubtitles:
		_subtitles = enabled;
		break;
	case kDemoBufferedVideo:
		setBufferedVideo(enabled);
		break;
	case kDemoToolbarHelp:
		_toolbarHelp = enabled;
		break;
	case kDemoToolbarPermanent:
		_toolbarPermanent = enabled;
		break;
	case kDemoReservedToggle4:
	case kDemoReservedToggle5:
		_demoReservedToggles[toggle - kDemoReservedToggle4] = enabled;
		break;
	default:
		return;
	}
	debugC(2, kDebugGeneral,
		"Ripper: demo Options toggle=%u enabled=%d", (uint)toggle, enabled);
}

void RipperSettings::setDemoSlider(DemoSlider slider, int value) {
	value = CLIP<int>(value, 0, 100);
	switch (slider) {
	case kDemoSoundVolume:
		setValue(kMasterVolume, value);
		break;
	case kDemoMusicVolume:
		setValue(kAmbientVolume, value);
		break;
	case kDemoMouseSensitivity:
		_mouseSensitivity = value;
		break;
	default:
		return;
	}
	debugC(2, slider == kDemoMouseSensitivity ? kDebugInput : kDebugAudio,
		"Ripper: demo Options slider=%u value=%d", (uint)slider, value);
}

} // End of namespace Ripper
