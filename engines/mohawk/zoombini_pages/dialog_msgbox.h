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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_MSGBOX_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_MSGBOX_H

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * Modal message box used for short notices and yes/no decisions.
 *
 * The message may come from a predefined @ref ZoombiniMsgBoxType, from a
 * caller-provided string, or from localized yes/no keys. All three
 * constructors converge on the same two-button feature state machine; the
 * selected answer is returned through @ref ZoombiniDialog::_dialogResult.
 */
class ZoombiniDialogMsgBox : public ZoombiniDialog {
public:
	/** Create a predefined message box. */
	ZoombiniDialogMsgBox(MohawkEngine_Zoombini *vm, ZoombiniMsgBoxType type);
	/** Create an informational message box with caller-provided text. */
	ZoombiniDialogMsgBox(MohawkEngine_Zoombini *vm, const Common::U32String &message);
	/** Create a message box with localized labels for the two choices. */
	ZoombiniDialogMsgBox(MohawkEngine_Zoombini *vm, const Common::U32String &message, ZoombiniText::Key yesKey, ZoombiniText::Key noKey);
	/** Release message text and button state. */
	~ZoombiniDialogMsgBox() override;

	/** Load the message body and long-button feature. */
	void loadFeatures() override;

	/** Prepare the two long buttons before rendering. */
	void longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Restore the message box background after button rendering. */
	void longButtons_onPostRender(ZmbFeature *feature);
	/** Apply the selected button action to the dialog result. */
	void longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Handle pointer activation of the yes/no buttons. */
	ZmbEventHandleResult longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard activation of the yes/no buttons. */
	ZmbEventHandleResult longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

private:
	// MapRect & MapSave data
	/** Fixed indexes of the yes and no message-box buttons. */
	enum MsgBoxDialogButtonIdx : uint32 {
		/** Accept the message-box question. */
		kMsgBoxDialogButton01_Yes = 0,
		/** Reject or cancel the message-box question. */
		kMsgBoxDialogButton02_No = 1,
	};

	/** Initialize button labels and state for the selected text keys. */
	void initButtons(ZoombiniText::Key yesKey, ZoombiniText::Key noKey);
	/** Return the message-body rectangle for the detected release. */
	Common::Rect getBodyTextRect() const;

	/** Predefined message-box kind, when one was selected. */
	ZoombiniMsgBoxType _type;
	/** Localized message key used by a predefined message box. */
	ZoombiniText::Key _msgKey;
	/** Caller-provided message text, when present. */
	Common::U32String _msgText;
};

} // End of namespace Mohawk

#endif
