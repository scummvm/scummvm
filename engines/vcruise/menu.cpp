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

#include "graphics/managed_surface.h"

#include "audio/mixer.h"

#include "vcruise/menu.h"
#include "vcruise/runtime.h"

namespace VCruise {

class ReahMenuPage : public MenuPage {
public:
	ReahMenuPage();

	bool run() override;
	void start() override;

protected:
	virtual void onButtonClicked(uint button, bool &outChangedState);
	virtual void onCheckboxClicked(uint button, bool &outChangedState);
	virtual void onSliderMoved(uint slider);
	virtual void eraseSlider(uint sliderIndex) const;

protected:
	enum ButtonState {
		kButtonStateDisabled,
		kButtonStateIdle,
		kButtonStateHighlighted,
		kButtonStatePressed,
	};

	enum CheckboxState {
		kCheckboxStateOff,
		kCheckboxStateOffHighlighted,
		kCheckboxStateOn,
		kCheckboxStateOnHighlighted,
	};

	enum InteractionState {
		kInteractionStateNotInteracting,

		kInteractionStateOverButton,
		kInteractionStateClickingOnButton,
		kInteractionStateClickingOffButton,

		kInteractionStateOverSlider,
		kInteractionStateDraggingSlider,

		kInteractionStateOverCheckbox,
		kInteractionStateClickingOnCheckbox,
		kInteractionStateClickingOffCheckbox,
	};

	struct Button {
		Button();
		Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenRect, const Common::Point &stateOffset, bool enabled);

		Graphics::Surface *_graphic;
		Common::Rect _graphicRect;
		Common::Rect _screenRect;
		Common::Point _stateOffset;
		bool _enabled;
	};

	struct Slider {
		Slider();
		Slider(Graphics::Surface *graphic, const Common::Rect &baseRect, int value, int maxValue);

		Graphics::Surface *_graphic;
		Common::Rect _baseRect;
		int _value;
		int _maxValue;
	};

private:
	void drawButtonInState(uint buttonIndex, ButtonState state) const;
	void drawCheckboxInState(uint buttonIndex, CheckboxState state) const;
	void drawSlider(uint sliderIndex) const;
	void drawButtonFromListInState(const Common::Array<Button> &buttonList, uint buttonIndex, int state) const;

	void handleMouseMove(const Common::Point &pt);
	void handleMouseDown(const Common::Point &pt, bool &outChangedState);
	void handleMouseUp(const Common::Point &pt, bool &outChangedState);

protected:
	Common::Array<Button> _buttons;
	Common::Array<Button> _checkboxes;
	Common::Array<Slider> _sliders;

	InteractionState _interactionState;
	uint _interactionIndex;

	Common::Point _sliderDragStart;
	int _sliderDragValue;
};

class ReahMenuBarPage : public ReahMenuPage {
public:
	explicit ReahMenuBarPage(uint page);

	void start() override final;

protected:
	enum MenuBarButtonID {
		kMenuBarButtonHelp,
		kMenuBarButtonSave,
		kMenuBarButtonLoad,
		kMenuBarButtonSound,
		kMenuBarButtonQuit,

		kMenuBarButtonReturn,
	};

	virtual void addPageContents() = 0;
	void onButtonClicked(uint button, bool &outChangedState) override;

	uint _page;
};

class ReahHelpMenuPage : public ReahMenuBarPage {
public:
	ReahHelpMenuPage();

	void addPageContents() override;
};

class ReahSoundMenuPage : public ReahMenuBarPage {
public:
	ReahSoundMenuPage();

	void addPageContents() override;

protected:
	void eraseSlider(uint sliderIndex) const override;
	void onCheckboxClicked(uint button, bool &outChangedState) override;
	void onSliderMoved(uint slider) override;

private:
	enum SoundMenuCheckbox {
		kCheckboxSound = 0,
		kCheckboxMusic,
	};

	enum SoundMenuSlider {
		kSliderSound = 0,
		kSliderMusic,
	};

	void applySoundVolume() const;
	void applyMusicVolume() const;

	Common::SharedPtr<Graphics::ManagedSurface> _sliderKeyGraphic;

	static const int kSoundSliderWidth = 300;
	static const int kSoundSliderY = 127;
	static const int kMusicSliderY = 275;

	bool _soundChecked;
	bool _musicChecked;
};

class ReahQuitMenuPage : public ReahMenuBarPage {
public:
	ReahQuitMenuPage();

	void addPageContents() override;
	void onButtonClicked(uint button, bool &outChangedState) override;

private:
	enum QuitMenuButton {
		kButtonYes = 6,
		kButtonNo,
	};
};

class ReahMainMenuPage : public ReahMenuPage {
public:
	void start() override;

protected:
	void onButtonClicked(uint button, bool &outChangedState) override;

private:
	enum ButtonID {
		kButtonContinue,
		kButtonNew,
		kButtonLoad,
		kButtonSound,
		kButtonCredits,
		kButtonQuit,
	};
};

ReahMenuPage::ReahMenuPage() : _interactionIndex(0), _interactionState(kInteractionStateNotInteracting), _sliderDragValue(0) {
}

bool ReahMenuPage::run() {
	bool changedState = false;

	OSEvent evt;
	while (_menuInterface->popOSEvent(evt)) {
		switch (evt.type) {
		case kOSEventTypeLButtonUp:
			handleMouseMove(evt.pos);
			handleMouseUp(evt.pos, changedState);
			if (changedState)
				return changedState;
			break;
		case kOSEventTypeLButtonDown:
			handleMouseMove(evt.pos);
			handleMouseDown(evt.pos, changedState);
			if (changedState)
				return changedState;
			break;
		case kOSEventTypeMouseMove:
			handleMouseMove(evt.pos);
			break;
		default:
			break;
		}
	}

	return false;
}

void ReahMenuPage::start() {
	for (uint buttonIndex = 0; buttonIndex < _buttons.size(); buttonIndex++)
		drawButtonInState(buttonIndex, _buttons[buttonIndex]._enabled ? kButtonStateIdle : kButtonStateDisabled);

	for (uint checkboxIndex = 0; checkboxIndex < _checkboxes.size(); checkboxIndex++)
		drawCheckboxInState(checkboxIndex, _checkboxes[checkboxIndex]._enabled ? kCheckboxStateOn : kCheckboxStateOff);

	for (uint sliderIndex = 0; sliderIndex < _sliders.size(); sliderIndex++)
		drawSlider(sliderIndex);

	Common::Point mousePoint = _menuInterface->getMouseCoordinate();
	handleMouseMove(mousePoint);
}

void ReahMenuPage::onButtonClicked(uint button, bool &outChangedState) {
	outChangedState = false;
}

void ReahMenuPage::onCheckboxClicked(uint button, bool &outChangedState) {
}

void ReahMenuPage::onSliderMoved(uint slider) {
}

void ReahMenuPage::eraseSlider(uint sliderIndex) const {
}

void ReahMenuPage::handleMouseMove(const Common::Point &pt) {
	switch (_interactionState) {
	case kInteractionStateNotInteracting:
		for (uint buttonIndex = 0; buttonIndex < _buttons.size(); buttonIndex++) {
			const Button &button = _buttons[buttonIndex];

			if (button._enabled && button._screenRect.contains(pt)) {
				drawButtonInState(buttonIndex, kButtonStateHighlighted);

				_interactionIndex = buttonIndex;
				_interactionState = kInteractionStateOverButton;
				break;
			}
		}

		for (uint checkboxIndex = 0; checkboxIndex < _checkboxes.size(); checkboxIndex++) {
			const Button &checkbox = _checkboxes[checkboxIndex];

			if (checkbox._screenRect.contains(pt)) {
				drawCheckboxInState(checkboxIndex, checkbox._enabled ? kCheckboxStateOnHighlighted : kCheckboxStateOffHighlighted);

				_interactionIndex = checkboxIndex;
				_interactionState = kInteractionStateOverCheckbox;
				break;
			}
		}

		for (uint sliderIndex = 0; sliderIndex < _sliders.size(); sliderIndex++) {
			const Slider &slider = _sliders[sliderIndex];

			Common::Rect sliderRect = slider._baseRect;
			sliderRect.translate(slider._value, 0);

			if (sliderRect.contains(pt)) {
				_interactionIndex = sliderIndex;
				_interactionState = kInteractionStateOverSlider;
			}
		}
		break;

	case kInteractionStateOverButton: {
			const Button &button = _buttons[_interactionIndex];
			if (!button._screenRect.contains(pt)) {
				drawButtonInState(_interactionIndex, kButtonStateIdle);

				_interactionState = kInteractionStateNotInteracting;
				handleMouseMove(pt);
			}
		} break;

	case kInteractionStateClickingOnButton: {
			const Button &button = _buttons[_interactionIndex];
			if (!button._screenRect.contains(pt)) {
				drawButtonInState(_interactionIndex, kButtonStateHighlighted);

				_interactionState = kInteractionStateClickingOffButton;
			}
		} break;

	case kInteractionStateClickingOffButton: {
			const Button &button = _buttons[_interactionIndex];
			if (button._screenRect.contains(pt)) {
				drawButtonInState(_interactionIndex, kButtonStatePressed);

				_interactionState = kInteractionStateClickingOnButton;
			}
		} break;

	case kInteractionStateOverSlider: {
			const Slider &slider = _sliders[_interactionIndex];

			Common::Rect sliderRect = slider._baseRect;
			sliderRect.translate(slider._value, 0);

			if (!sliderRect.contains(pt)) {
				_interactionState = kInteractionStateNotInteracting;
				handleMouseMove(pt);
			}
		} break;

	case kInteractionStateDraggingSlider: {
			Slider &slider = _sliders[_interactionIndex];

			int newValue = _sliderDragValue + pt.x - _sliderDragStart.x;
			if (newValue < 0)
				newValue = 0;
			else if (newValue >= slider._maxValue)
				newValue = slider._maxValue;

			if (newValue != slider._value) {
				eraseSlider(_interactionIndex);
				slider._value = newValue;
				drawSlider(_interactionIndex);

				onSliderMoved(_interactionIndex);
			}
		} break;

	case kInteractionStateOverCheckbox: {
			const Button &checkbox = _checkboxes[_interactionIndex];
			if (!checkbox._screenRect.contains(pt)) {
				drawCheckboxInState(_interactionIndex, checkbox._enabled ? kCheckboxStateOn : kCheckboxStateOff);

				_interactionState = kInteractionStateNotInteracting;
				handleMouseMove(pt);
			}
		} break;

	case kInteractionStateClickingOnCheckbox: {
			const Button &checkbox = _checkboxes[_interactionIndex];
			if (!checkbox._screenRect.contains(pt)) {
				drawCheckboxInState(_interactionIndex, checkbox._enabled ? kCheckboxStateOnHighlighted : kCheckboxStateOffHighlighted);

				_interactionState = kInteractionStateClickingOffCheckbox;
			}
		} break;

	case kInteractionStateClickingOffCheckbox: {
			const Button &checkbox = _checkboxes[_interactionIndex];
			if (checkbox._screenRect.contains(pt)) {
				drawCheckboxInState(_interactionIndex, checkbox._enabled ? kCheckboxStateOffHighlighted : kCheckboxStateOnHighlighted);

				_interactionState = kInteractionStateClickingOnCheckbox;
			}
		} break;

	default:
		error("Unhandled UI state");
		break;
	}
}

void ReahMenuPage::handleMouseDown(const Common::Point &pt, bool &outChangedState) {
	switch (_interactionState) {
	case kInteractionStateNotInteracting:
	case kInteractionStateClickingOnButton:
	case kInteractionStateClickingOffButton:
	case kInteractionStateDraggingSlider:
	case kInteractionStateClickingOnCheckbox:
	case kInteractionStateClickingOffCheckbox:
		break;

	case kInteractionStateOverButton:
		drawButtonInState(_interactionIndex, kButtonStatePressed);
		_interactionState = kInteractionStateClickingOnButton;
		break;

	case kInteractionStateOverSlider:
		_interactionState = kInteractionStateDraggingSlider;
		_sliderDragStart = pt;
		_sliderDragValue = _sliders[_interactionIndex]._value;
		break;

	case kInteractionStateOverCheckbox:
		drawCheckboxInState(_interactionIndex, _checkboxes[_interactionIndex]._enabled ? kCheckboxStateOffHighlighted : kCheckboxStateOnHighlighted);
		_interactionState = kInteractionStateClickingOnCheckbox;
		break;

	default:
		break;
	}
}

void ReahMenuPage::handleMouseUp(const Common::Point &pt, bool &outChangedState) {
	switch (_interactionState) {
	case kInteractionStateNotInteracting:
	case kInteractionStateOverButton:
	case kInteractionStateOverCheckbox:
	case kInteractionStateOverSlider:
		break;

	case kInteractionStateClickingOnButton:
		drawButtonInState(_interactionIndex, kButtonStateHighlighted);
		_interactionState = kInteractionStateOverButton;

		onButtonClicked(_interactionIndex, outChangedState);
		break;

	case kInteractionStateClickingOffButton:
		drawButtonInState(_interactionIndex, kButtonStateIdle);
		_interactionState = kInteractionStateNotInteracting;
		handleMouseMove(pt);
		break;

	case kInteractionStateDraggingSlider:
		_interactionState = kInteractionStateNotInteracting;
		handleMouseMove(pt);
		break;

	case kInteractionStateClickingOnCheckbox:
		_checkboxes[_interactionIndex]._enabled = !_checkboxes[_interactionIndex]._enabled;
		drawCheckboxInState(_interactionIndex, _checkboxes[_interactionIndex]._enabled ? kCheckboxStateOnHighlighted : kCheckboxStateOffHighlighted);
		_interactionState = kInteractionStateOverCheckbox;

		onCheckboxClicked(_interactionIndex, outChangedState);
		break;

	case kInteractionStateClickingOffCheckbox:
		drawCheckboxInState(_interactionIndex, _checkboxes[_interactionIndex]._enabled ? kCheckboxStateOn : kCheckboxStateOff);
		_interactionState = kInteractionStateNotInteracting;
		handleMouseMove(pt);
		break;

	default:
		break;
	}
}

ReahMenuBarPage::ReahMenuBarPage(uint page) : _page(page) {
}

void ReahMenuBarPage::start() {
	Graphics::Surface *graphic = _menuInterface->getUIGraphic(4);

	bool menuButtonsEnabled[5] = {true, true, true, true, true};

	menuButtonsEnabled[1] = _menuInterface->canSave();
	menuButtonsEnabled[_page] = false;

	if (graphic) {
		for (int buttonIndex = 0; buttonIndex < 5; buttonIndex++) {
			Common::Rect buttonRect(128 * buttonIndex, 0, 128 * buttonIndex + 128, 44);
			_buttons.push_back(Button(graphic, buttonRect, buttonRect, Common::Point(0, 44), menuButtonsEnabled[buttonIndex]));
		}
	}

	Graphics::Surface *returnButtonGraphic = _menuInterface->getUIGraphic(9);
	if (returnButtonGraphic)
		_buttons.push_back(Button(returnButtonGraphic, Common::Rect(0, 0, 112, 44), Common::Rect(519, 423, 631, 467), Common::Point(0, 44), true));

	Graphics::Surface *lowerBarGraphic = _menuInterface->getUIGraphic(8);

	if (lowerBarGraphic) {
		_menuInterface->getMenuSurface()->blitFrom(*lowerBarGraphic, Common::Point(0, 392));
		_menuInterface->commitRect(Common::Rect(0, 392, 640, 480));
	}

	addPageContents();

	ReahMenuPage::start();
}

void ReahMenuBarPage::onButtonClicked(uint button, bool &outChangedState) {
	switch (button) {
	case kMenuBarButtonHelp:
		_menuInterface->changeMenu(new ReahHelpMenuPage());
		outChangedState = true;
		break;
	case kMenuBarButtonLoad:
		outChangedState = g_engine->loadGameDialog();
		break;
	case kMenuBarButtonSave:
		g_engine->saveGameDialog();
		break;
	case kMenuBarButtonSound:
		_menuInterface->changeMenu(new ReahSoundMenuPage());
		outChangedState = true;
		break;
	case kMenuBarButtonQuit:
		_menuInterface->changeMenu(new ReahQuitMenuPage());
		outChangedState = true;
		break;

	case kMenuBarButtonReturn:
		if (_menuInterface->canSave())
			outChangedState = _menuInterface->reloadFromCheckpoint();
		else {
			_menuInterface->changeMenu(new ReahMainMenuPage());
			outChangedState = true;
		}
		break;
	default:
		break;
	}
}

void ReahMenuPage::drawButtonInState(uint buttonIndex, ButtonState state) const {
	drawButtonFromListInState(_buttons, buttonIndex, state);
}

void ReahMenuPage::drawCheckboxInState(uint buttonIndex, CheckboxState state) const {
	drawButtonFromListInState(_checkboxes, buttonIndex, state);
}

void ReahMenuPage::drawSlider(uint sliderIndex) const {
	const Slider &slider = _sliders[sliderIndex];

	Common::Point screenPoint(slider._baseRect.left + slider._value, slider._baseRect.top);

	_menuInterface->getMenuSurface()->blitFrom(*slider._graphic, screenPoint);
	_menuInterface->commitRect(Common::Rect(screenPoint.x, screenPoint.y, screenPoint.x + slider._baseRect.width(), screenPoint.y + slider._baseRect.height()));
}

void ReahMenuPage::drawButtonFromListInState(const Common::Array<Button> &buttonList, uint buttonIndex, int state) const {
	const Button &button = buttonList[buttonIndex];

	Common::Rect graphicRect = button._graphicRect;
	graphicRect.translate(button._stateOffset.x * state, button._stateOffset.y * state);

	_menuInterface->getMenuSurface()->blitFrom(*button._graphic, graphicRect, button._screenRect);
	_menuInterface->commitRect(Common::Rect(button._screenRect.left, button._screenRect.top, button._screenRect.left + graphicRect.width(), button._screenRect.top + graphicRect.height()));
}

ReahMenuPage::Button::Button() : _graphic(nullptr), _enabled(true) {
}

ReahMenuPage::Button::Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenRect, const Common::Point &stateOffset, bool enabled)
	: _graphic(graphic), _graphicRect(graphicRect), _screenRect(screenRect), _stateOffset(stateOffset), _enabled(enabled) {
}

ReahMenuPage::Slider::Slider() : _graphic(nullptr), _value(0), _maxValue(1) {
}

ReahMenuPage::Slider::Slider(Graphics::Surface *graphic, const Common::Rect &baseRect, int value, int maxValue)
	: _graphic(graphic), _baseRect(baseRect), _value(value), _maxValue(maxValue) {
	assert(_value >= 0 && _value <= maxValue);
}

ReahHelpMenuPage::ReahHelpMenuPage() : ReahMenuBarPage(kMenuBarButtonHelp) {
}

void ReahHelpMenuPage::addPageContents() {
	Graphics::Surface *helpBG = _menuInterface->getUIGraphic(12);
	if (helpBG) {
		_menuInterface->getMenuSurface()->blitFrom(*helpBG, Common::Point(0, 44));
		_menuInterface->commitRect(Common::Rect(0, 44, helpBG->w, 44 + helpBG->h));
	}
}

ReahSoundMenuPage::ReahSoundMenuPage() : ReahMenuBarPage(kMenuBarButtonSound), _soundChecked(false), _musicChecked(false) {
}

void ReahSoundMenuPage::addPageContents() {
	Graphics::Surface *soundBG = _menuInterface->getUIGraphic(16);
	if (soundBG) {
		_menuInterface->getMenuSurface()->blitFrom(*soundBG, Common::Point(0, 44));
		_menuInterface->commitRect(Common::Rect(0, 44, soundBG->w, 44 + soundBG->h));
	}

	int sndVol = 0;
	if (ConfMan.hasKey("sfx_volume"))
		sndVol = ConfMan.getInt("sfx_volume");

	int musVol = 0;
	if (ConfMan.hasKey("music_volume"))
		musVol = ConfMan.getInt("music_volume");

	_soundChecked = (sndVol != 0);
	_musicChecked = (musVol != 0);

	// Set defaults so clicking the checkbox does something
	if (sndVol == 0)
		sndVol = 3 * Audio::Mixer::kMaxMixerVolume / 4;

	if (musVol == 0)
		musVol = 3 * Audio::Mixer::kMaxMixerVolume / 4;

	Graphics::Surface *soundGraphics = _menuInterface->getUIGraphic(17);
	if (soundGraphics) {
		_checkboxes.push_back(Button(soundGraphics, Common::Rect(0, 0, 112, 44), Common::Rect(77, 90, 77 + 112, 90 + 44), Common::Point(0, 44), _soundChecked));
		_checkboxes.push_back(Button(soundGraphics, Common::Rect(112, 0, 224, 44), Common::Rect(77, 231, 77 + 112, 231 + 44), Common::Point(0, 44), _musicChecked));

		Common::Point sliderSize(40, 60);

		_sliderKeyGraphic.reset(new Graphics::ManagedSurface(sliderSize.x, sliderSize.y, Graphics::createPixelFormat<8888>()));

		Graphics::PixelFormat srcFormat = soundGraphics->format;
		Graphics::PixelFormat dstFormat = _sliderKeyGraphic->format;

		for (int y = 0; y < sliderSize.y; y++) {
			for (int x = 0; x < sliderSize.x; x++) {
				uint32 maskColor = soundGraphics->getPixel(224 + x, y + 60);

				byte r = 0;
				byte g = 0;
				byte b = 0;
				srcFormat.colorToRGB(maskColor, r, g, b);

				uint32 dstColor = 0;
				if (r > 128) {
					dstColor = dstFormat.ARGBToColor(0, 0, 0, 0);
				} else {
					uint32 srcColor = soundGraphics->getPixel(224 + x, y);
					srcFormat.colorToRGB(srcColor, r, g, b);
					dstColor = dstFormat.ARGBToColor(255, r, g, b);
				}

				_sliderKeyGraphic->setPixel(x, y, dstColor);
			}
		}

		_sliders.push_back(Slider(_sliderKeyGraphic->surfacePtr(), Common::Rect(236, kSoundSliderY, 236 + 40, kSoundSliderY + 60), sndVol * kSoundSliderWidth / Audio::Mixer::kMaxMixerVolume, kSoundSliderWidth));
		_sliders.push_back(Slider(_sliderKeyGraphic->surfacePtr(), Common::Rect(236, kMusicSliderY, 236 + 40, kMusicSliderY + 60), musVol * kSoundSliderWidth / Audio::Mixer::kMaxMixerVolume, kSoundSliderWidth));
	}
}

void ReahSoundMenuPage::eraseSlider(uint sliderIndex) const {
	Graphics::Surface *soundBG = _menuInterface->getUIGraphic(16);

	if (soundBG) {
		Common::Rect sliderRect = _sliders[sliderIndex]._baseRect;
		sliderRect.translate(_sliders[sliderIndex]._value, 0);

		Common::Rect backgroundSourceRect = sliderRect;
		backgroundSourceRect.translate(0, -44);

		_menuInterface->getMenuSurface()->blitFrom(*soundBG, backgroundSourceRect, Common::Point(sliderRect.left, sliderRect.top));
		_menuInterface->commitRect(sliderRect);
	}
}

void ReahSoundMenuPage::onCheckboxClicked(uint button, bool &outChangedState) {
	if (button == kCheckboxSound) {
		_soundChecked = _checkboxes[button]._enabled;
		applySoundVolume();
	}
	if (button == kCheckboxMusic) {
		_musicChecked = _checkboxes[button]._enabled;
		applyMusicVolume();
	}

	outChangedState = false;
}

void ReahSoundMenuPage::onSliderMoved(uint slider) {
	if (slider == kSliderSound && _soundChecked)
		applySoundVolume();

	if (slider == kSliderMusic && _musicChecked)
		applyMusicVolume();
}

void ReahSoundMenuPage::applySoundVolume() const {
	int vol = 0;

	if (_soundChecked)
		vol = _sliders[kSliderSound]._value * Audio::Mixer::kMaxMixerVolume / _sliders[kSliderSound]._maxValue;

	ConfMan.setInt("sfx_volume", vol, ConfMan.getActiveDomainName());

	if (g_engine->_mixer)
		g_engine->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol);
}

void ReahSoundMenuPage::applyMusicVolume() const {
	int vol = 0;

	if (_musicChecked)
		vol = _sliders[kSliderMusic]._value * Audio::Mixer::kMaxMixerVolume / _sliders[kSliderMusic]._maxValue;

	ConfMan.setInt("music_volume", vol, ConfMan.getActiveDomainName());

	if (g_engine->_mixer)
		g_engine->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol);
}

ReahQuitMenuPage::ReahQuitMenuPage() : ReahMenuBarPage(kMenuBarButtonQuit) {
}

void ReahQuitMenuPage::addPageContents() {
	Graphics::ManagedSurface *menuSurf = _menuInterface->getMenuSurface();
	menuSurf->fillRect(Common::Rect(0, 44, 640, 392), menuSurf->format.RGBToColor(0, 0, 0));

	Graphics::Surface *borderGraphic = _menuInterface->getUIGraphic(10);

	if (borderGraphic) {
		Graphics::PixelFormat borderGraphicFmt = borderGraphic->format;
		Graphics::PixelFormat menuSurfFmt = menuSurf->format;
		byte r = 0;
		byte g = 0;
		byte b = 0;

		const int xOffsets[2] = {0, 640 - 16};

		for (int y = 0; y < borderGraphic->h; y++) {
			for (int x = 0; x < 16; x++) {
				uint32 pixels[2] = { borderGraphic->getPixel(x, y), borderGraphic->getPixel(x + 16, y) };
				int intensities[2] = {(16 - x) * 32, (x + 1) * 32};
				for (int i = 0; i < 2; i++) {
					borderGraphicFmt.colorToRGB(pixels[i], r, g, b);

					int intensity = intensities[i];
					if (intensity < 256) {
						r = (r * intensity) >> 8;
						g = (g * intensity) >> 8;
						b = (b * intensity) >> 8;
					}

					menuSurf->setPixel(x + xOffsets[i], y + 44, menuSurfFmt.RGBToColor(r, g, b));
				}
			}
		}
	}

	Graphics::Surface *windowGraphic = _menuInterface->getUIGraphic(13);

	if (windowGraphic)
		menuSurf->blitFrom(*windowGraphic, Common::Point(82, 114));

	Graphics::Surface *textGraphic = _menuInterface->getUIGraphic(14);

	if (textGraphic)
		menuSurf->blitFrom(*textGraphic, Common::Rect(0, 72, textGraphic->w, textGraphic->h), Common::Point(82, 174));

	Graphics::Surface *buttonsGraphic = _menuInterface->getUIGraphic(15);

	if (buttonsGraphic) {
		_buttons.push_back(Button(buttonsGraphic, Common::Rect(224, 0, 336, 44), Common::Rect(174, 246, 286, 290), Common::Point(0, 44), true));
		_buttons.push_back(Button(buttonsGraphic, Common::Rect(336, 0, 448, 44), Common::Rect(351, 248, 463, 292), Common::Point(0, 44), true));
	}

	_menuInterface->commitRect(Common::Rect(0, 44, 640, 392));

	// Disable the "Return" button since the "No" button is functionally the same (and Reah does this)
	_buttons[kMenuBarButtonReturn]._enabled = false;
}

void ReahQuitMenuPage::onButtonClicked(uint button, bool &outChangedState) {
	ReahMenuBarPage::onButtonClicked(button, outChangedState);

	if (button == kButtonYes)
		_menuInterface->quitGame();
	else if (button == kButtonNo)
		onButtonClicked(kMenuBarButtonReturn, outChangedState);
}

void ReahMainMenuPage::start() {
	Graphics::Surface *bgGraphic = _menuInterface->getUIGraphic(0);

	Graphics::ManagedSurface *menuSurf = _menuInterface->getMenuSurface();

	if (bgGraphic) {
		menuSurf->blitFrom(*bgGraphic, Common::Point(0, 0));
	}

	_menuInterface->commitRect(Common::Rect(0, 0, 640, 480));

	Graphics::Surface *buttonGraphic = _menuInterface->getUIGraphic(1);

	Common::Point buttonStateOffset = Common::Point(112, 0);
	Common::Point buttonTopLeft = Common::Point(492, 66);
	const int buttonTopYs[6] = {66, 119, 171, 224, 277, 330};

	for (int i = 0; i < 6; i++) {
		bool isEnabled = true;
		if (i == kButtonContinue)
			isEnabled = _menuInterface->hasDefaultSave();

		_buttons.push_back(Button(buttonGraphic, Common::Rect(0, i * 44, 112, i * 44 + 44), Common::Rect(492, buttonTopYs[i], 492 + 112, buttonTopYs[i] + 44), Common::Point(112, 0), isEnabled));
	}

	ReahMenuPage::start();
}

void ReahMainMenuPage::onButtonClicked(uint button, bool &outChangedState) {
	switch (button) {
	case kButtonContinue: {
			Common::Error loadError = g_engine->loadGameState(g_engine->getAutosaveSlot());
			outChangedState = (loadError.getCode() == Common::kNoError);
		} break;

	case kButtonNew:
		_menuInterface->restartGame();
		outChangedState = true;
		break;

	case kButtonLoad:
		outChangedState = g_engine->loadGameDialog();
		break;

	case kButtonSound:
		_menuInterface->changeMenu(new ReahSoundMenuPage());
		outChangedState = true;
		break;

	case kButtonCredits:
		_menuInterface->goToCredits();
		outChangedState = true;
		break;

	case kButtonQuit:
		_menuInterface->changeMenu(new ReahQuitMenuPage());
		outChangedState = true;
		break;
	}
}

MenuInterface::~MenuInterface() {
}

MenuPage::MenuPage() : _menuInterface(nullptr) {
}

MenuPage::~MenuPage() {
}

void MenuPage::init(const MenuInterface *menuInterface) {
	_menuInterface = menuInterface;
}

void MenuPage::start() {
}

bool MenuPage::run() {
	return false;
}

MenuPage *createMenuReahMain() {
	return new ReahMainMenuPage();
}

} // End of namespace VCruise
