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

#include "graphics/font.h"
#include "graphics/managed_surface.h"

#include "audio/mixer.h"

#include "vcruise/menu.h"
#include "vcruise/runtime.h"

namespace VCruise {

class ReahSchizmMenuPage : public MenuPage {
public:
	explicit ReahSchizmMenuPage(bool isSchizm);

	bool run() override;
	void start() override;

protected:
	virtual void onButtonClicked(uint button, bool &outChangedState);
	virtual void onCheckboxClicked(uint button, bool &outChangedState);
	virtual void onSliderMoved(uint slider);
	virtual void onKeymappedEvent(VCruise::KeymappedEvent evt, bool &outChangedState);
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
		Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenRect, const Common::Rect &interactiveRect, const Common::Point &stateOffset, bool enabled);
		Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenRect, const Common::Rect &interactiveRect, const Common::Point &stateOffset, bool enabled, const Common::String (&states)[4]);
		Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenAndInteractiveRect, const Common::Point &stateOffset, bool enabled);
		Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenAndInteractiveRect, const Common::Point &stateOffset, bool enabled, const Common::String (&states)[4]);

		Graphics::Surface *_graphic;
		Common::Rect _graphicRect;
		Common::Rect _screenRect;
		Common::Rect _interactiveRect;
		Common::Point _stateOffset;
		bool _enabled;

		Common::String _buttonStates[4];
	};

	struct Slider {
		Slider();
		Slider(Graphics::Surface *graphic, const Common::Rect &baseRect, int value, int maxValue);

		Graphics::Surface *_graphic;
		Common::Rect _baseRect;
		int _value;
		int _maxValue;
	};

protected:
	void drawButtonInState(uint buttonIndex, ButtonState state) const;
	void drawCheckboxInState(uint buttonIndex, CheckboxState state) const;
	void drawSlider(uint sliderIndex) const;
	void drawButtonFromListInState(const Common::Array<Button> &buttonList, uint buttonIndex, int state) const;

	void handleMouseMove(const Common::Point &pt);
	void handleMouseDown(const Common::Point &pt, bool &outChangedState);
	void handleMouseUp(const Common::Point &pt, bool &outChangedState);

	Common::Array<Button> _buttons;
	Common::Array<Button> _checkboxes;
	Common::Array<Slider> _sliders;

	InteractionState _interactionState;
	uint _interactionIndex;

	Common::Point _sliderDragStart;
	int _sliderDragValue;

	bool _isSchizm;
};

class ReahMenuBarPage : public ReahSchizmMenuPage {
public:
	ReahMenuBarPage(uint page, bool isSchizm);

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
	explicit ReahHelpMenuPage(bool isSchizm);

	void addPageContents() override;
};

class ReahSoundMenuPage : public ReahMenuBarPage {
public:
	explicit ReahSoundMenuPage(bool isSchizm);

	void addPageContents() override;
	void onSettingsChanged() override;

protected:
	void eraseSlider(uint sliderIndex) const override;
	void onCheckboxClicked(uint button, bool &outChangedState) override;
	void onSliderMoved(uint slider) override;

private:
	enum SoundMenuCheckbox {
		kCheckboxSound = 0,
		kCheckboxMusic,

		kCheckboxSubtitle,
	};

	enum SoundMenuSlider {
		kSliderSound = 0,
		kSliderMusic,
	};

	void applySoundVolume() const;
	void applyMusicVolume() const;

	Common::SharedPtr<Graphics::ManagedSurface> _sliderKeyGraphic;

	static const int kSoundSliderWidth = 300;
	static const int kReahSoundSliderY = 127;
	static const int kReahMusicSliderY = 275;
	static const int kSchizmSoundSliderY = 100;
	static const int kSchizmMusicSliderY = 220;

	bool _soundChecked;
	bool _musicChecked;
	bool _subtitleChecked;
};

class ReahQuitMenuPage : public ReahMenuBarPage {
public:
	explicit ReahQuitMenuPage(bool isSchizm);

	void addPageContents() override;
	void onButtonClicked(uint button, bool &outChangedState) override;

private:
	enum QuitMenuButton {
		kButtonYes = 6,
		kButtonNo,
	};
};

class ReahPauseMenuPage : public ReahMenuBarPage {
public:
	explicit ReahPauseMenuPage(bool isSchizm);

	void addPageContents() override;

	void onKeymappedEvent(VCruise::KeymappedEvent evt, bool &outChangedState) override;
};

class ReahSchizmMainMenuPage : public ReahSchizmMenuPage {
public:
	explicit ReahSchizmMainMenuPage(bool isSchizm);

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

ReahSchizmMenuPage::ReahSchizmMenuPage(bool isSchizm) : _interactionIndex(0), _interactionState(kInteractionStateNotInteracting), _sliderDragValue(0), _isSchizm(isSchizm) {
}

bool ReahSchizmMenuPage::run() {
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
		case kOSEventTypeKeymappedEvent:
			onKeymappedEvent(evt.keymappedEvent, changedState);
			if (changedState)
				return changedState;
			break;
		default:
			break;
		}
	}

	return false;
}

void ReahSchizmMenuPage::start() {
	for (uint buttonIndex = 0; buttonIndex < _buttons.size(); buttonIndex++)
		drawButtonInState(buttonIndex, _buttons[buttonIndex]._enabled ? kButtonStateIdle : kButtonStateDisabled);

	for (uint checkboxIndex = 0; checkboxIndex < _checkboxes.size(); checkboxIndex++)
		drawCheckboxInState(checkboxIndex, _checkboxes[checkboxIndex]._enabled ? kCheckboxStateOn : kCheckboxStateOff);

	for (uint sliderIndex = 0; sliderIndex < _sliders.size(); sliderIndex++)
		drawSlider(sliderIndex);

	Common::Point mousePoint = _menuInterface->getMouseCoordinate();
	handleMouseMove(mousePoint);
}

void ReahSchizmMenuPage::onButtonClicked(uint button, bool &outChangedState) {
	outChangedState = false;
}

void ReahSchizmMenuPage::onCheckboxClicked(uint button, bool &outChangedState) {
}

void ReahSchizmMenuPage::onSliderMoved(uint slider) {
}

void ReahSchizmMenuPage::onKeymappedEvent(VCruise::KeymappedEvent evt, bool &outChangedState) {
}

void ReahSchizmMenuPage::eraseSlider(uint sliderIndex) const {
}

void ReahSchizmMenuPage::handleMouseMove(const Common::Point &pt) {
	switch (_interactionState) {
	case kInteractionStateNotInteracting:
		for (uint buttonIndex = 0; buttonIndex < _buttons.size(); buttonIndex++) {
			const Button &button = _buttons[buttonIndex];

			if (button._enabled && button._interactiveRect.contains(pt)) {
				drawButtonInState(buttonIndex, kButtonStateHighlighted);

				_interactionIndex = buttonIndex;
				_interactionState = kInteractionStateOverButton;
				break;
			}
		}

		for (uint checkboxIndex = 0; checkboxIndex < _checkboxes.size(); checkboxIndex++) {
			const Button &checkbox = _checkboxes[checkboxIndex];

			if (checkbox._interactiveRect.contains(pt)) {
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
			if (!button._interactiveRect.contains(pt)) {
				drawButtonInState(_interactionIndex, kButtonStateIdle);

				_interactionState = kInteractionStateNotInteracting;
				handleMouseMove(pt);
			}
		} break;

	case kInteractionStateClickingOnButton: {
			const Button &button = _buttons[_interactionIndex];
			if (!button._interactiveRect.contains(pt)) {
				drawButtonInState(_interactionIndex, kButtonStateHighlighted);

				_interactionState = kInteractionStateClickingOffButton;
			}
		} break;

	case kInteractionStateClickingOffButton: {
			const Button &button = _buttons[_interactionIndex];
			if (button._interactiveRect.contains(pt)) {
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
			if (!checkbox._interactiveRect.contains(pt)) {
				drawCheckboxInState(_interactionIndex, checkbox._enabled ? kCheckboxStateOn : kCheckboxStateOff);

				_interactionState = kInteractionStateNotInteracting;
				handleMouseMove(pt);
			}
		} break;

	case kInteractionStateClickingOnCheckbox: {
			const Button &checkbox = _checkboxes[_interactionIndex];
			if (!checkbox._interactiveRect.contains(pt)) {
				drawCheckboxInState(_interactionIndex, checkbox._enabled ? kCheckboxStateOnHighlighted : kCheckboxStateOffHighlighted);

				_interactionState = kInteractionStateClickingOffCheckbox;
			}
		} break;

	case kInteractionStateClickingOffCheckbox: {
			const Button &checkbox = _checkboxes[_interactionIndex];
			if (checkbox._interactiveRect.contains(pt)) {
				drawCheckboxInState(_interactionIndex, checkbox._enabled ? kCheckboxStateOffHighlighted : kCheckboxStateOnHighlighted);

				_interactionState = kInteractionStateClickingOnCheckbox;
			}
		} break;

	default:
		error("Unhandled UI state");
		break;
	}
}

void ReahSchizmMenuPage::handleMouseDown(const Common::Point &pt, bool &outChangedState) {
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

void ReahSchizmMenuPage::handleMouseUp(const Common::Point &pt, bool &outChangedState) {
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

ReahMenuBarPage::ReahMenuBarPage(uint page, bool isSchizm) : ReahSchizmMenuPage(isSchizm), _page(page) {
}

void ReahMenuBarPage::start() {
	Graphics::Surface *graphic = _menuInterface->getUIGraphic(4);

	bool menuButtonsEnabled[5] = {true, true, true, true, true};

	menuButtonsEnabled[1] = _menuInterface->canSave();
	if (_page < 5)
		menuButtonsEnabled[_page] = false;

	if (graphic) {
		for (int buttonIndex = 0; buttonIndex < 5; buttonIndex++) {
			Common::String states[4];

			for (int sti = 0; sti < 4; sti++)
				states[sti] = Common::String::format("szData004_%02i", buttonIndex + sti * 5 + 1);

			Common::Rect buttonRect(128 * buttonIndex, 0, 128 * buttonIndex + 128, 44);
			_buttons.push_back(Button(graphic, buttonRect, buttonRect, Common::Point(0, 44), menuButtonsEnabled[buttonIndex], states));
		}
	}

	Graphics::Surface *returnButtonGraphic = _menuInterface->getUIGraphic(9);
	if (returnButtonGraphic) {
		Common::String states[4];
		for (int i = 0; i < 4; i++)
			states[i] = Common::String::format("szData009_%02i", i + 1);

		Common::Rect screenRect = Common::Rect(519, 423, 631, 467);
		Common::Rect interactiveRect = screenRect;

		if (_isSchizm)
			interactiveRect.bottom = interactiveRect.top + 32;

		_buttons.push_back(Button(returnButtonGraphic, Common::Rect(0, 0, 112, 44), screenRect, interactiveRect, Common::Point(0, 44), true, states));
	}

	Graphics::Surface *lowerBarGraphic = _menuInterface->getUIGraphic(8);

	if (lowerBarGraphic) {
		_menuInterface->getMenuSurface()->blitFrom(*lowerBarGraphic, Common::Point(0, 392));
		_menuInterface->commitRect(Common::Rect(0, 392, 640, 480));
	}

	addPageContents();

	ReahSchizmMenuPage::start();
}

void ReahMenuBarPage::onButtonClicked(uint button, bool &outChangedState) {
	switch (button) {
	case kMenuBarButtonHelp:
		_menuInterface->changeMenu(new ReahHelpMenuPage(_isSchizm));
		outChangedState = true;
		break;
	case kMenuBarButtonLoad:
		outChangedState = g_engine->loadGameDialog();
		break;
	case kMenuBarButtonSave:
		g_engine->saveGameDialog();
		break;
	case kMenuBarButtonSound:
		_menuInterface->changeMenu(new ReahSoundMenuPage(_isSchizm));
		outChangedState = true;
		break;
	case kMenuBarButtonQuit:
		if (_isSchizm && !_menuInterface->isInGame())
			_menuInterface->changeMenu(new ReahSchizmMainMenuPage(_isSchizm));
		else
			_menuInterface->changeMenu(new ReahQuitMenuPage(_isSchizm));
		outChangedState = true;
		break;

	case kMenuBarButtonReturn:
		if (_menuInterface->canSave())
			outChangedState = _menuInterface->reloadFromCheckpoint();
		else {
			_menuInterface->changeMenu(new ReahSchizmMainMenuPage(_isSchizm));
			outChangedState = true;
		}
		break;
	default:
		break;
	}
}

void ReahSchizmMenuPage::drawButtonInState(uint buttonIndex, ButtonState state) const {
	drawButtonFromListInState(_buttons, buttonIndex, state);
}

void ReahSchizmMenuPage::drawCheckboxInState(uint buttonIndex, CheckboxState state) const {
	drawButtonFromListInState(_checkboxes, buttonIndex, state);
}

void ReahSchizmMenuPage::drawSlider(uint sliderIndex) const {
	const Slider &slider = _sliders[sliderIndex];

	Common::Point screenPoint(slider._baseRect.left + slider._value, slider._baseRect.top);

	_menuInterface->getMenuSurface()->blitFrom(*slider._graphic, screenPoint);
	_menuInterface->commitRect(Common::Rect(screenPoint.x, screenPoint.y, screenPoint.x + slider._baseRect.width(), screenPoint.y + slider._baseRect.height()));
}

void ReahSchizmMenuPage::drawButtonFromListInState(const Common::Array<Button> &buttonList, uint buttonIndex, int state) const {
	const Button &button = buttonList[buttonIndex];

	Common::Rect graphicRect = button._graphicRect;
	graphicRect.translate(button._stateOffset.x * state, button._stateOffset.y * state);

	Graphics::ManagedSurface *menuSurf = _menuInterface->getMenuSurface();
	menuSurf->blitFrom(*button._graphic, graphicRect, button._screenRect);

	_menuInterface->drawLabel(menuSurf, button._buttonStates[state], button._screenRect);

	_menuInterface->commitRect(Common::Rect(button._screenRect.left, button._screenRect.top, button._screenRect.left + graphicRect.width(), button._screenRect.top + graphicRect.height()));
}

ReahSchizmMenuPage::Button::Button() : _graphic(nullptr), _enabled(true) {
}

ReahSchizmMenuPage::Button::Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenRect, const Common::Rect &interactiveRect, const Common::Point &stateOffset, bool enabled)
	: _graphic(graphic), _graphicRect(graphicRect), _screenRect(screenRect), _interactiveRect(interactiveRect), _stateOffset(stateOffset), _enabled(enabled) {
}

ReahSchizmMenuPage::Button::Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenRect, const Common::Rect &interactiveRect, const Common::Point &stateOffset, bool enabled, const Common::String (&states)[4])
	: _graphic(graphic), _graphicRect(graphicRect), _screenRect(screenRect), _interactiveRect(interactiveRect), _stateOffset(stateOffset), _enabled(enabled) {
	for (int i = 0; i < 4; i++)
		this->_buttonStates[i] = states[i];
}

ReahSchizmMenuPage::Button::Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenAndInteractiveRect, const Common::Point &stateOffset, bool enabled)
	: _graphic(graphic), _graphicRect(graphicRect), _screenRect(screenAndInteractiveRect), _interactiveRect(screenAndInteractiveRect), _stateOffset(stateOffset), _enabled(enabled) {
}

ReahSchizmMenuPage::Button::Button(Graphics::Surface *graphic, const Common::Rect &graphicRect, const Common::Rect &screenAndInteractiveRect, const Common::Point &stateOffset, bool enabled, const Common::String(&states)[4])
	: _graphic(graphic), _graphicRect(graphicRect), _screenRect(screenAndInteractiveRect), _interactiveRect(screenAndInteractiveRect), _stateOffset(stateOffset), _enabled(enabled) {
	for (int i = 0; i < 4; i++)
		this->_buttonStates[i] = states[i];
}


ReahSchizmMenuPage::Slider::Slider() : _graphic(nullptr), _value(0), _maxValue(1) {
}

ReahSchizmMenuPage::Slider::Slider(Graphics::Surface *graphic, const Common::Rect &baseRect, int value, int maxValue)
	: _graphic(graphic), _baseRect(baseRect), _value(value), _maxValue(maxValue) {
	assert(_value >= 0 && _value <= maxValue);
}

ReahHelpMenuPage::ReahHelpMenuPage(bool isSchizm) : ReahMenuBarPage(kMenuBarButtonHelp, isSchizm) {
}

void ReahHelpMenuPage::addPageContents() {
	Graphics::ManagedSurface *menuSurf = _menuInterface->getMenuSurface();

	Graphics::Surface *helpBG = _menuInterface->getUIGraphic(12);

	if (helpBG) {
		menuSurf->blitFrom(*helpBG, Common::Point(0, 44));
		_menuInterface->commitRect(Common::Rect(0, 44, helpBG->w, 44 + helpBG->h));
	}

	if (_isSchizm) {
		for (int htX = 0; htX < 2; htX++) {
			for (int htY = 0; htY < 6; htY++) {
				Common::String labelID = Common::String::format("szData012_%02i", htX * 6 + htY + 2);

				Common::Point topLeft = Common::Point(htX * 280 + 60, htY * 25 + 140);
				Common::Rect rect(topLeft.x, topLeft.y, topLeft.x + 280, topLeft.y + 25);

				_menuInterface->drawLabel(menuSurf, labelID, rect);

				_menuInterface->commitRect(rect);
			}
		}

		Common::Rect titleRect(240, 80, 400, 124);

		_menuInterface->drawLabel(menuSurf, "szData012_01", titleRect);

		_menuInterface->commitRect(titleRect);
	}
}

ReahSoundMenuPage::ReahSoundMenuPage(bool isSchizm) : ReahMenuBarPage(kMenuBarButtonSound, isSchizm), _soundChecked(false), _musicChecked(false), _subtitleChecked(false) {
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

	bool musicMute = false;
	if (ConfMan.hasKey("vcruise_mute_music"))
		musicMute = ConfMan.getBool("vcruise_mute_music");

	bool soundMute = false;
	if (ConfMan.hasKey("vcruise_mute_sound"))
		soundMute = ConfMan.getBool("vcruise_mute_sound");

	_soundChecked = !soundMute;
	_musicChecked = !musicMute;

	Graphics::Surface *soundGraphics = _menuInterface->getUIGraphic(17);
	if (soundGraphics) {
		Common::Point checkboxSize = _isSchizm ? Common::Point(200, 44) : Common::Point(112, 44);

		Common::Rect checkboxGraphicRects[4];

		for (int i = 0; i < 4; i++)
			checkboxGraphicRects[i] = Common::Rect(checkboxSize.x * i, 0, checkboxSize.x * (i + 1), checkboxSize.y);

		// Schizm has a blank spot where a "Louder" checkbox was
		const Common::Rect &musicGraphicRect = _isSchizm ? checkboxGraphicRects[2] : checkboxGraphicRects[1];

		Common::Point soundCheckboxScreenCoord = _isSchizm ? Common::Point(50, 100) : Common::Point(77, 90);
		Common::Point musicCheckboxScreenCoord = _isSchizm ? Common::Point(50, 220) : Common::Point(77, 231);
		Common::Point subtitleCheckboxScreenCoord = Common::Point(50, 280);

		Common::Rect soundCheckboxScreenRect = Common::Rect(soundCheckboxScreenCoord.x, soundCheckboxScreenCoord.y, soundCheckboxScreenCoord.x + checkboxSize.x, soundCheckboxScreenCoord.y + checkboxSize.y);
		Common::Rect musicCheckboxScreenRect = Common::Rect(musicCheckboxScreenCoord.x, musicCheckboxScreenCoord.y, musicCheckboxScreenCoord.x + checkboxSize.x, musicCheckboxScreenCoord.y + checkboxSize.y);
		Common::Rect subtitleCheckboxScreenRect = Common::Rect(subtitleCheckboxScreenCoord.x, subtitleCheckboxScreenCoord.y, subtitleCheckboxScreenCoord.x + checkboxSize.x, subtitleCheckboxScreenCoord.y + checkboxSize.y);

		Common::String checkStates[3][4];

		const int baseStates[3] = {1, 3, 4};
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 4; j++)
				checkStates[i][j] = Common::String::format("szData017_%02i", static_cast<int>(baseStates[i] + j * 4));

		_checkboxes.push_back(Button(soundGraphics, checkboxGraphicRects[0], soundCheckboxScreenRect, Common::Point(0, checkboxSize.y), _soundChecked, checkStates[0]));
		_checkboxes.push_back(Button(soundGraphics, musicGraphicRect, musicCheckboxScreenRect, Common::Point(0, checkboxSize.y), _musicChecked, checkStates[1]));

		if (_isSchizm)
			_checkboxes.push_back(Button(soundGraphics, checkboxGraphicRects[3], subtitleCheckboxScreenRect, Common::Point(0, checkboxSize.y), _subtitleChecked, checkStates[2]));

		Common::Point sliderSize(40, 60);

		_sliderKeyGraphic.reset(new Graphics::ManagedSurface(sliderSize.x, sliderSize.y, Graphics::createPixelFormat<8888>()));

		Graphics::PixelFormat srcFormat = soundGraphics->format;
		Graphics::PixelFormat dstFormat = _sliderKeyGraphic->format;

		int sliderGraphicLeftX = (_isSchizm ? 800 : 224);

		for (int y = 0; y < sliderSize.y; y++) {
			for (int x = 0; x < sliderSize.x; x++) {
				uint32 maskColor = soundGraphics->getPixel(sliderGraphicLeftX + x, y + 60);

				byte r = 0;
				byte g = 0;
				byte b = 0;
				srcFormat.colorToRGB(maskColor, r, g, b);

				uint32 dstColor = 0;
				if (r > 128) {
					dstColor = dstFormat.ARGBToColor(0, 0, 0, 0);
				} else {
					uint32 srcColor = soundGraphics->getPixel(sliderGraphicLeftX + x, y);
					srcFormat.colorToRGB(srcColor, r, g, b);
					dstColor = dstFormat.ARGBToColor(255, r, g, b);
				}

				_sliderKeyGraphic->setPixel(x, y, dstColor);
			}
		}

		int sliderScreenLeftX = (_isSchizm ? 250 : 236);

		int soundSliderY = (_isSchizm ? kSchizmSoundSliderY : kReahSoundSliderY);
		int musicSliderY = (_isSchizm ? kSchizmMusicSliderY : kReahMusicSliderY);

		_sliders.push_back(Slider(_sliderKeyGraphic->surfacePtr(), Common::Rect(sliderScreenLeftX, soundSliderY, sliderScreenLeftX + 40, soundSliderY + 60), sndVol * kSoundSliderWidth / Audio::Mixer::kMaxMixerVolume, kSoundSliderWidth));
		_sliders.push_back(Slider(_sliderKeyGraphic->surfacePtr(), Common::Rect(sliderScreenLeftX, musicSliderY, sliderScreenLeftX + 40, musicSliderY + 60), musVol * kSoundSliderWidth / Audio::Mixer::kMaxMixerVolume, kSoundSliderWidth));
	}
}

void ReahSoundMenuPage::onSettingsChanged() {
	int sndVol = 0;
	if (ConfMan.hasKey("sfx_volume"))
		sndVol = ConfMan.getInt("sfx_volume");

	int musVol = 0;
	if (ConfMan.hasKey("music_volume"))
		musVol = ConfMan.getInt("music_volume");

	bool musicMute = false;
	if (ConfMan.hasKey("vcruise_mute_music"))
		musicMute = ConfMan.getBool("vcruise_mute_music");

	bool soundMute = false;
	if (ConfMan.hasKey("vcruise_mute_sound"))
		soundMute = ConfMan.getBool("vcruise_mute_sound");

	_soundChecked = !soundMute;
	_musicChecked = !musicMute;
	_subtitleChecked = true;

	eraseSlider(kSliderSound);
	eraseSlider(kSliderMusic);

	_sliders[kSliderSound]._value = sndVol * kSoundSliderWidth / Audio::Mixer::kMaxMixerVolume;
	_sliders[kSliderMusic]._value = musVol * kSoundSliderWidth / Audio::Mixer::kMaxMixerVolume;

	drawSlider(kSliderSound);
	drawSlider(kSliderMusic);

	// Release any active interactions with the checkboxes
	if ((_interactionState == kInteractionStateClickingOnCheckbox || _interactionState == kInteractionStateClickingOffCheckbox)
		&& (_interactionIndex == kCheckboxMusic || _interactionIndex == kCheckboxSound)) {
		_interactionState = kInteractionStateNotInteracting;
	}

	drawCheckboxInState(kCheckboxSound, _soundChecked ? kCheckboxStateOn : kCheckboxStateOff);
	drawCheckboxInState(kCheckboxMusic, _musicChecked ? kCheckboxStateOn : kCheckboxStateOff);

	if (_isSchizm)
		drawCheckboxInState(kCheckboxSubtitle, _subtitleChecked ? kCheckboxStateOn : kCheckboxStateOff);
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
	if (button == kCheckboxSubtitle) {
		_subtitleChecked = _checkboxes[button]._enabled;
		//applySubtitles();
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
	int vol = _sliders[kSliderSound]._value * Audio::Mixer::kMaxMixerVolume / _sliders[kSliderSound]._maxValue;

	ConfMan.setInt("sfx_volume", vol, ConfMan.getActiveDomainName());
	ConfMan.setBool("vcruise_mute_sound", !_soundChecked, ConfMan.getActiveDomainName());

	g_engine->syncSoundSettings();
}

void ReahSoundMenuPage::applyMusicVolume() const {
	int vol = _sliders[kSliderMusic]._value * Audio::Mixer::kMaxMixerVolume / _sliders[kSliderMusic]._maxValue;

	ConfMan.setInt("music_volume", vol, ConfMan.getActiveDomainName());
	ConfMan.setBool("vcruise_mute_music", !_musicChecked, ConfMan.getActiveDomainName());

	// Try to avoid changing music volume right before stopping music to avoid an audio pop
	if (!_musicChecked)
		_menuInterface->setMusicMute(true);

	g_engine->syncSoundSettings();

	if (_musicChecked)
		_menuInterface->setMusicMute(false);
}

ReahQuitMenuPage::ReahQuitMenuPage(bool isSchizm) : ReahMenuBarPage(kMenuBarButtonQuit, isSchizm) {
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
		int noButtonY = _isSchizm ? 246 : 248;

		Common::String states[2][4];

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 4; j++)
				states[i][j] = Common::String::format("szData015_%02i", i + j * 4 + 3);

		_buttons.push_back(Button(buttonsGraphic, Common::Rect(224, 0, 336, 44), Common::Rect(174, 246, 286, 290), Common::Point(0, 44), true, states[0]));
		_buttons.push_back(Button(buttonsGraphic, Common::Rect(336, 0, 448, 44), Common::Rect(351, noButtonY, 463, noButtonY + 44), Common::Point(0, 44), true, states[1]));
	}

	_menuInterface->commitRect(Common::Rect(0, 44, 640, 392));

	// Disable the "Return" button since the "No" button is functionally the same (and Reah does this)
	_buttons[kMenuBarButtonReturn]._enabled = false;

	if (_isSchizm) {
		Common::Rect promptRect = Common::Rect(82, 174, 558, 215);
		_menuInterface->drawLabel(menuSurf, "szData014_04", promptRect);

		_menuInterface->commitRect(promptRect);
	}
}

void ReahQuitMenuPage::onButtonClicked(uint button, bool &outChangedState) {
	ReahMenuBarPage::onButtonClicked(button, outChangedState);

	if (button == kButtonYes) {
		if (_isSchizm && _menuInterface->isInGame())
			_menuInterface->quitToMenu();
		else
			_menuInterface->quitGame();

		outChangedState = true;
	} else if (button == kButtonNo)
		onButtonClicked(kMenuBarButtonReturn, outChangedState);
}

ReahPauseMenuPage::ReahPauseMenuPage(bool isSchizm) : ReahMenuBarPage(static_cast<uint>(-1), isSchizm) {
}

void ReahPauseMenuPage::addPageContents() {
	Graphics::Surface *pauseGraphic = _menuInterface->getUIGraphic(20);

	Graphics::ManagedSurface *menuSurf = _menuInterface->getMenuSurface();

	uint32 blackColor = menuSurf->format.RGBToColor(0, 0, 0);

	menuSurf->fillRect(Common::Rect(0, 44, 640, 392), blackColor);

	if (pauseGraphic)
		menuSurf->blitFrom(*pauseGraphic, Common::Point(164, 186));

	if (_isSchizm) {
		Common::Rect labelRect1 = Common::Rect(164, 192, 476, 216);
		Common::Rect labelRect2 = Common::Rect(164, 216, 476, 240);

		_menuInterface->drawLabel(menuSurf, "szData020_01", labelRect1);
		_menuInterface->drawLabel(menuSurf, "szData020_02", labelRect2);
	}

	_menuInterface->commitRect(Common::Rect(0, 44, 640, 392));
}

void ReahPauseMenuPage::onKeymappedEvent(VCruise::KeymappedEvent evt, bool &outChangedState) {
	if (evt == VCruise::kKeymappedEventPause) {
		if (_menuInterface->canSave())
			outChangedState = _menuInterface->reloadFromCheckpoint();
		return;
	}
}

ReahSchizmMainMenuPage::ReahSchizmMainMenuPage(bool isSchizm) : ReahSchizmMenuPage(isSchizm) {
}

void ReahSchizmMainMenuPage::start() {
	Graphics::Surface *bgGraphic = _menuInterface->getUIGraphic(0);

	Graphics::ManagedSurface *menuSurf = _menuInterface->getMenuSurface();

	if (bgGraphic) {
		menuSurf->blitFrom(*bgGraphic, Common::Point(0, 0));
	}

	_menuInterface->commitRect(Common::Rect(0, 0, 640, 480));

	Graphics::Surface *buttonGraphic = _menuInterface->getUIGraphic(1);

	Common::Point buttonSize;

	Common::Point buttonCoords[6];
	Common::String buttonStates[6][4];

	if (_isSchizm) {
		buttonCoords[0] = Common::Point(240, 52);
		buttonCoords[1] = Common::Point(181, 123);
		buttonCoords[2] = Common::Point(307, 157);
		buttonCoords[3] = Common::Point(179, 232);
		buttonCoords[4] = Common::Point(298, 296);
		buttonCoords[5] = Common::Point(373, 395);

		buttonSize = Common::Point(150, 40);

		for (int i = 0; i < 6; i++) {
			int index = i;
			if (i == 5)
				index = 6;

			buttonStates[i][0] = Common::String::format("szData001_%02i", static_cast<int>(index + 1));
			buttonStates[i][1] = Common::String::format("szData001_%02i", static_cast<int>(index + 8));
			buttonStates[i][2] = Common::String::format("szData001_%02i", static_cast<int>(index + 15));
			buttonStates[i][3] = Common::String::format("szData001_%02i", static_cast<int>(index + 22));
		}

	} else {
		const int buttonTopYs[6] = {66, 119, 171, 224, 277, 330};
		for (int i = 0; i < 6; i++)
			buttonCoords[i] = Common::Point(492, buttonTopYs[i]);

		buttonSize = Common::Point(112, 44);
	}

	for (int i = 0; i < 6; i++) {
		bool isEnabled = true;
		if (i == kButtonContinue)
			isEnabled = _menuInterface->hasDefaultSave();
		else if (i == kButtonLoad)
			isEnabled = _menuInterface->hasAnySave();

		int coordScale = i;

		// Skip uninstall button
		if (_isSchizm && i == 5)
			coordScale = 6;

		Common::Rect graphicRect(0, coordScale * buttonSize.y, buttonSize.x, (coordScale + 1) * buttonSize.y);
		Common::Rect screenRect(buttonCoords[i].x, buttonCoords[i].y, buttonCoords[i].x + buttonSize.x, buttonCoords[i].y + buttonSize.y);
		Common::Rect interactiveRect(buttonCoords[i].x, buttonCoords[i].y, buttonCoords[i].x + buttonSize.x, buttonCoords[i].y + 34);

		_buttons.push_back(Button(buttonGraphic, graphicRect, screenRect, interactiveRect, Common::Point(buttonSize.x, 0), isEnabled, buttonStates[i]));
	}

	if (_isSchizm) {
		Common::Rect copyrightRect = Common::Rect(6, 456, 308, 480);

		_menuInterface->drawLabel(menuSurf, "szData000_01", copyrightRect);
		_menuInterface->commitRect(copyrightRect);
	}

	ReahSchizmMenuPage::start();
}

void ReahSchizmMainMenuPage::onButtonClicked(uint button, bool &outChangedState) {
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
		_menuInterface->changeMenu(new ReahSoundMenuPage(_isSchizm));
		outChangedState = true;
		break;

	case kButtonCredits:
		_menuInterface->goToCredits();
		outChangedState = true;
		break;

	case kButtonQuit:
		if (_isSchizm)
			_menuInterface->quitGame();
		else
			_menuInterface->changeMenu(new ReahQuitMenuPage(_isSchizm));

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

void MenuPage::onSettingsChanged() {
}

bool MenuPage::run() {
	return false;
}

MenuPage *createMenuMain(bool isSchizm) {
	return new ReahSchizmMainMenuPage(isSchizm);
}

MenuPage *createMenuQuit(bool isSchizm) {
	return new ReahQuitMenuPage(isSchizm);
}

MenuPage *createMenuHelp(bool isSchizm) {
	return new ReahHelpMenuPage(isSchizm);
}

MenuPage *createMenuSound(bool isSchizm) {
	return new ReahSoundMenuPage(isSchizm);
}

MenuPage *createMenuPause(bool isSchizm) {
	return new ReahPauseMenuPage(isSchizm);
}

} // End of namespace VCruise
