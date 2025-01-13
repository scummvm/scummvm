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

#ifndef GOT_VIEWS_DIALOGS_SELECT_OPTION_H
#define GOT_VIEWS_DIALOGS_SELECT_OPTION_H

#include "got/views/dialogs/dialog.h"

namespace Got {
namespace Views {
namespace Dialogs {

// Commonly used options across multiple dialogs
extern const char *ON_OFF[];
extern const char *YES_NO[];

class SelectOption : public Dialog {
private:
	Common::String _title;
	Common::StringArray _options;
	int _hammerFrame = 0;
	int _smackCtr = 0;

protected:
	int _selectedItem = 0;

	void setContent(const Common::String &title,
					const Common::StringArray &options);
	virtual void closed() {}
	virtual void selected() {}

public:
	SelectOption(const Common::String &name, const char *title, const char *options[]);
	SelectOption(const Common::String &name);
	virtual ~SelectOption() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Dialogs
} // namespace Views
} // namespace Got

#endif
