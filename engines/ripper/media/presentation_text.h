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

#ifndef RIPPER_MEDIA_PRESENTATION_TEXT_H
#define RIPPER_MEDIA_PRESENTATION_TEXT_H

#include "common/rect.h"
#include "common/str.h"

#include "ripper/media.h"
#include "ripper/modal_dialog.h"

namespace Ripper {

class InputManager;
class RipperEngine;

Common::Rect calculatePresentationTextBounds(uint sequenceId, uint videoMode,
	bool largeMedia, int displayTop);
Common::Rect calculatePresentationContinueBounds(
	const Common::Rect &textBounds);
uint calculatePresentationTextAutoScrollLine(uint progress, uint total,
	uint maximumFirstVisible);

class PresentationTextControl {
public:
	PresentationTextControl(RipperEngine *engine, InputManager *input,
		const Common::String &text, int displayTop);

	bool isActive() const { return !_text.empty(); }
	bool isDismissed() const { return _dismissed; }
	void configureSegment(uint sequenceId, uint totalFrames,
		uint mediaWidth, uint mediaHeight, uint videoMode);
	uint16 service(uint progress);
	bool waitForDismissal();
	void applySharedPalettePatch();

private:
	bool draw(bool includeContinueControl);
	bool serviceInput(bool allowDismissal);
	void scrollTo(uint firstVisible, const char *source);

	RipperEngine *_engine;
	InputManager *_input;
	Common::String _text;
	Common::Rect _bounds;
	uint _firstVisible;
	uint _maximumFirstVisible;
	uint _visibleRows;
	uint _totalFrames;
	uint _sequenceId;
	int _displayTop;
	bool _dismissed;
	bool _continuePressed;
	bool _textColorPending;
	ModalDialogManager::TextPanelScrollControl _hoveredScrollControl;
};

class PresentationTextMediaCallback : public MediaSequenceCallback {
public:
	PresentationTextMediaCallback(RipperEngine *engine, InputManager *input,
		const Common::String &text, int displayTop, uint videoMode);

	void beginIavfSegment(uint sequenceId, uint frameCount,
		uint width, uint height) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }
	bool waitForDismissal() { return _control.waitForDismissal(); }
	bool isDismissed() const { return _control.isDismissed(); }

private:
	PresentationTextControl _control;
	uint _videoMode;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_PRESENTATION_TEXT_H
