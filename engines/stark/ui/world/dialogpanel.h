/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_UI_DIALOG_PANEL_H
#define STARK_UI_DIALOG_PANEL_H

#include "engines/stark/ui/window.h"

#include "engines/stark/visual/text.h"

#include "common/scummsys.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/array.h"
#include "common/rect.h"

namespace Stark {

class VisualImageXMG;
class ClickText;

namespace Resources {
class Speech;
}

class DialogPanel : public Window {
public:
	DialogPanel(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~DialogPanel();

	/** Abort the currently playing dialog */
	void reset();

	/** The screen resolution changed, rebuild the text textures accordingly */
	void onScreenChanged();

	/** Scroll up and down the panel */
	void scrollUp();
	void scrollDown();

	/** Select the next or previous option */
	void focusNextOption();
	void focusPrevOption();

	/** Select the focused option */
	void selectFocusedOption();

	/** Select an option by index */
	void selectOption(uint index);

protected:
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onRightClick(const Common::Point &pos) override;
	void onGameLoop() override;
	void onRender() override;

private:
	void updateSubtitleVisual();
	void clearSubtitleVisual();
	void updateDialogOptions();
	void clearOptions();
	void renderOptions();
	void renderScrollArrows() const;

	void updateFirstVisibleOption();
	void updateLastVisibleOption();

	VisualImageXMG *_passiveBackGroundTexture;
	VisualImageXMG *_activeBackGroundTexture;
	VisualImageXMG *_scrollUpArrowImage;
	VisualImageXMG *_scrollDownArrowImage;
	VisualImageXMG *_dialogOptionBullet;
	VisualText *_subtitleVisual;

	bool _scrollUpArrowVisible;
	bool _scrollDownArrowVisible;
	Common::Rect _scrollUpArrowRect;
	Common::Rect _scrollDownArrowRect;

	Resources::Speech *_currentSpeech;
	void abortCurrentSpeech();

	uint32 _firstVisibleOption, _lastVisibleOption;
	uint32 _focusedOption;
	Common::Array<ClickText*> _options;
	bool _acceptIdleMousePos;

	static const Color _aprilColor;
	static const Color _otherColor;
	static const uint32 _optionsTop = 4;
	static const uint32 _optionsLeft = 30;
	static const uint32 _optionsHeight = 80;
};

} // End of namespace Stark

#endif // STARK_UI_DIALOG_PANEL_H
