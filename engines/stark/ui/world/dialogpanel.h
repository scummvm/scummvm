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

#include "common/scummsys.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/array.h"
#include "common/rect.h"

namespace Stark {

class VisualImageXMG;
class VisualText;
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

protected:
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onRender() override;

private:
	void updateSubtitleVisual();
	void updateDialogOptions();
	void clearOptions();
	int getHoveredOption(const Common::Point &pos);
	void scrollOptions(int increment);
	void renderOptions();
	void renderScrollArrows() const;

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

	uint32 _firstVisibleOption;
	Common::Array<ClickText*> _options;

	static const uint32 _aprilColor = 0xFF00C0FF;
	static const uint32 _otherColor = 0xFF4040FF;
	static const uint32 _optionsTop = 4;
	static const uint32 _optionsLeft = 30;
	static const uint32 _optionsHeight = 71;
};

} // End of namespace Stark

#endif // STARK_UI_DIALOG_PANEL_H
