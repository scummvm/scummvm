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

#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include "engines/metaengine.h"

#include "gui/dialog.h"
#include "common/str.h"
#include "audio/mididrv.h"

#ifdef GUI_ENABLE_KEYSDIALOG
#include "gui/KeysDialog.h"
#endif

#ifdef USE_FLUIDSYNTH
#include "gui/fluidsynth-dialog.h"
#endif

#ifdef USE_LIBCURL
#include "backends/cloud/storage.h"
#endif

namespace GUI {
class LauncherDialog;

class CheckboxWidget;
class EditTextWidget;
class PopUpWidget;
class SliderWidget;
class StaticTextWidget;
class TabWidget;
class ButtonWidget;
class CommandSender;
class GuiObject;
class RadiobuttonGroup;
class RadiobuttonWidget;

class OptionsDialog : public Dialog {
	typedef Common::Array<CheckboxWidget *> CheckboxWidgetList;

public:
	OptionsDialog(const Common::String &domain, int x, int y, int w, int h);
	OptionsDialog(const Common::String &domain, const Common::String &name);
	~OptionsDialog();

	void init();

	void open();
	virtual void apply();
	void close();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	const Common::String& getDomain() const { return _domain; }

	virtual void reflowLayout();

protected:
	/** Config domain this dialog is used to edit. */
	Common::String _domain;

	ButtonWidget *_soundFontButton;
	StaticTextWidget *_soundFont;
	ButtonWidget *_soundFontClearButton;

	virtual void build();
	virtual void clean();
	void rebuild();


	void addControlControls(GuiObject *boss, const Common::String &prefix);
	void addGraphicControls(GuiObject *boss, const Common::String &prefix);
	void addShaderControls(GuiObject *boss, const Common::String &prefix);
	void addAudioControls(GuiObject *boss, const Common::String &prefix);
	void addMIDIControls(GuiObject *boss, const Common::String &prefix);
	void addMT32Controls(GuiObject *boss, const Common::String &prefix);
	void addVolumeControls(GuiObject *boss, const Common::String &prefix);
	// The default value is the launcher's non-scaled talkspeed value. When SCUMM uses the widget,
	// it uses its own scale
	void addSubtitleControls(GuiObject *boss, const Common::String &prefix, int maxSliderVal = 255);
	void addEngineControls(GuiObject *boss, const Common::String &prefix, const ExtraGuiOptions &engineOptions);

	void setGraphicSettingsState(bool enabled);
	void setAudioSettingsState(bool enabled);
	void setMIDISettingsState(bool enabled);
	void setMT32SettingsState(bool enabled);
	void setVolumeSettingsState(bool enabled);
	void setSubtitleSettingsState(bool enabled);

	virtual void setupGraphicsTab();

	bool loadMusicDeviceSetting(PopUpWidget *popup, Common::String setting, MusicType preferredType = MT_AUTO);
	void saveMusicDeviceSetting(PopUpWidget *popup, Common::String setting);

	TabWidget *_tabWidget;
	int _graphicsTabId;
	int _midiTabId;
	int _pathsTabId;

private:

	//
	// Control controls
	//
	bool _enableControlSettings;

	CheckboxWidget *_touchpadCheckbox;
	CheckboxWidget *_onscreenCheckbox;
	CheckboxWidget *_swapMenuAndBackBtnsCheckbox;

	StaticTextWidget *_kbdMouseSpeedDesc;
	SliderWidget *_kbdMouseSpeedSlider;
	StaticTextWidget *_kbdMouseSpeedLabel;
	StaticTextWidget *_joystickDeadzoneDesc;
	SliderWidget *_joystickDeadzoneSlider;
	StaticTextWidget *_joystickDeadzoneLabel;

	//
	// Graphics controls
	//
	bool _enableGraphicSettings;
	StaticTextWidget *_gfxPopUpDesc;
	PopUpWidget *_gfxPopUp;
	StaticTextWidget *_stretchPopUpDesc;
	PopUpWidget *_stretchPopUp;
	CheckboxWidget *_fullscreenCheckbox;
	CheckboxWidget *_filteringCheckbox;
	CheckboxWidget *_aspectCheckbox;
	StaticTextWidget *_renderModePopUpDesc;
	PopUpWidget *_renderModePopUp;

	//
	// Shader controls
	//
	bool _enableShaderSettings;
	StaticTextWidget *_shaderPopUpDesc;
	PopUpWidget *_shaderPopUp;

	//
	// Audio controls
	//
	bool _enableAudioSettings;
	StaticTextWidget *_midiPopUpDesc;
	PopUpWidget *_midiPopUp;
	StaticTextWidget *_oplPopUpDesc;
	PopUpWidget *_oplPopUp;

	StaticTextWidget *_mt32DevicePopUpDesc;
	PopUpWidget *_mt32DevicePopUp;
	StaticTextWidget *_gmDevicePopUpDesc;
	PopUpWidget *_gmDevicePopUp;

	//
	// MIDI controls
	//
	bool _enableMIDISettings;
	CheckboxWidget *_multiMidiCheckbox;
	StaticTextWidget *_midiGainDesc;
	SliderWidget *_midiGainSlider;
	StaticTextWidget *_midiGainLabel;

	//
	// MT-32 controls
	//
	bool _enableMT32Settings;
	CheckboxWidget *_mt32Checkbox;
	CheckboxWidget *_enableGSCheckbox;

	//
	// Subtitle controls
	//
	int getSubtitleMode(bool subtitles, bool speech_mute);
	bool _enableSubtitleSettings;
	StaticTextWidget *_subToggleDesc;
	RadiobuttonGroup *_subToggleGroup;
	RadiobuttonWidget *_subToggleSubOnly;
	RadiobuttonWidget *_subToggleSpeechOnly;
	RadiobuttonWidget *_subToggleSubBoth;
	static const char *_subModeDesc[];
	static const char *_lowresSubModeDesc[];
	StaticTextWidget *_subSpeedDesc;
	SliderWidget *_subSpeedSlider;
	StaticTextWidget *_subSpeedLabel;

	//
	// Volume controls
	//
	void updateMusicVolume(const int newValue) const;
	void updateSfxVolume(const int newValue) const;
	void updateSpeechVolume(const int newValue) const;
	bool _enableVolumeSettings;

	StaticTextWidget *_musicVolumeDesc;
	SliderWidget *_musicVolumeSlider;
	StaticTextWidget *_musicVolumeLabel;

	StaticTextWidget *_sfxVolumeDesc;
	SliderWidget *_sfxVolumeSlider;
	StaticTextWidget *_sfxVolumeLabel;

	StaticTextWidget *_speechVolumeDesc;
	SliderWidget *_speechVolumeSlider;
	StaticTextWidget *_speechVolumeLabel;

	CheckboxWidget *_muteCheckbox;

protected:
	//
	// Game GUI options
	//
	Common::String _guioptions;
	Common::String _guioptionsString;

	//
	// Engine-specific controls
	//
	CheckboxWidgetList _engineCheckboxes;
};


class GlobalOptionsDialog : public OptionsDialog, public CommandSender {
public:
	GlobalOptionsDialog(LauncherDialog *launcher);
	~GlobalOptionsDialog();

	virtual void apply();
	void close();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	void handleTickle();

	virtual void reflowLayout();

protected:
	virtual void build();
	virtual void clean();

	Common::String _newTheme;
	LauncherDialog *_launcher;
#ifdef GUI_ENABLE_KEYSDIALOG
	KeysDialog *_keysDialog;
#endif
#ifdef USE_FLUIDSYNTH
	FluidSynthSettingsDialog *_fluidSynthSettingsDialog;
#endif
	StaticTextWidget *_savePath;
	ButtonWidget	 *_savePathClearButton;
	StaticTextWidget *_themePath;
	ButtonWidget	 *_themePathClearButton;
	StaticTextWidget *_extraPath;
	ButtonWidget	 *_extraPathClearButton;
#ifdef DYNAMIC_MODULES
	StaticTextWidget *_pluginsPath;
#endif

	//
	// Misc controls
	//
	StaticTextWidget *_curTheme;
	StaticTextWidget *_rendererPopUpDesc;
	PopUpWidget *_rendererPopUp;
	StaticTextWidget *_autosavePeriodPopUpDesc;
	PopUpWidget *_autosavePeriodPopUp;
	StaticTextWidget *_guiLanguagePopUpDesc;
	PopUpWidget *_guiLanguagePopUp;
	CheckboxWidget *_guiLanguageUseGameLanguageCheckbox;
	CheckboxWidget *_useSystemDialogsCheckbox;


#ifdef USE_UPDATES
	StaticTextWidget *_updatesPopUpDesc;
	PopUpWidget *_updatesPopUp;
#endif

#ifdef USE_CLOUD
#ifdef USE_LIBCURL
	//
	// Cloud controls
	//
	uint32 _selectedStorageIndex;
	StaticTextWidget *_storagePopUpDesc;
	PopUpWidget      *_storagePopUp;
	StaticTextWidget *_storageDisabledHint;
	ButtonWidget	 *_storageEnableButton;
	StaticTextWidget *_storageUsernameDesc;
	StaticTextWidget *_storageUsername;
	StaticTextWidget *_storageUsedSpaceDesc;
	StaticTextWidget *_storageUsedSpace;
	StaticTextWidget *_storageSyncHint;
	StaticTextWidget *_storageLastSyncDesc;
	StaticTextWidget *_storageLastSync;
	ButtonWidget	 *_storageSyncSavesButton;
	StaticTextWidget *_storageDownloadHint;
	ButtonWidget	 *_storageDownloadButton;
	StaticTextWidget *_storageDisconnectHint;
	ButtonWidget	 *_storageDisconnectButton;

	bool _connectingStorage;
	StaticTextWidget *_storageWizardNotConnectedHint;
	StaticTextWidget *_storageWizardOpenLinkHint;
	StaticTextWidget *_storageWizardLink;
	StaticTextWidget *_storageWizardCodeHint;
	EditTextWidget   *_storageWizardCodeBox;
	ButtonWidget	 *_storageWizardPasteButton;
	ButtonWidget	 *_storageWizardConnectButton;
	StaticTextWidget *_storageWizardConnectionStatusHint;
	bool _redrawCloudTab;

	void addCloudControls(GuiObject *boss, const Common::String &prefix, bool lowres);
	void setupCloudTab();
	void shiftWidget(Widget *widget, const char *widgetName, int32 xOffset, int32 yOffset);

	void storageConnectionCallback(Networking::ErrorResponse response);
	void storageSavesSyncedCallback(Cloud::Storage::BoolResponse response);
	void storageErrorCallback(Networking::ErrorResponse response);
#endif // USE_LIBCURL

#ifdef USE_SDL_NET
	//
	// LAN controls
	//
	ButtonWidget	 *_runServerButton;
	StaticTextWidget *_serverInfoLabel;
	ButtonWidget	 *_rootPathButton;
	StaticTextWidget *_rootPath;
	ButtonWidget	 *_rootPathClearButton;
	StaticTextWidget *_serverPortDesc;
	EditTextWidget   *_serverPort;
	ButtonWidget	 *_serverPortClearButton;
	StaticTextWidget *_featureDescriptionLine1;
	StaticTextWidget *_featureDescriptionLine2;
	bool _serverWasRunning;

	void addNetworkControls(GuiObject *boss, const Common::String &prefix, bool lowres);
	void reflowNetworkTabLayout();
#endif // USE_SDL_NET

#endif // USE_CLOUD
	//
	// Accessibility controls
	//
#ifdef USE_TTS
	bool _enableTTS;
	CheckboxWidget *_ttsCheckbox;
	PopUpWidget *_ttsVoiceSelectionPopUp;
#endif
};

} // End of namespace GUI

#endif
