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

#ifndef GUI_STORAGEWIZARDDIALOG_H
#define GUI_STORAGEWIZARDDIALOG_H

#include "gui/dialog.h"
#include "common/str.h"

namespace GUI {

class CommandSender;
class EditTextWidget;
class StaticTextWidget;
class ButtonWidget;
class GraphicsWidget;

#ifdef USE_SDL_NET
enum StorageWizardDialogCommands {
	kStorageCodePassedCmd = 'SWDC'
};
#endif

class StorageWizardDialog : public Dialog {
	static const uint32 CODE_FIELDS = 8;
	uint32 _storageId;

	StaticTextWidget *_headlineWidget;
	StaticTextWidget *_navigateLineWidget;
	StaticTextWidget *_urlLineWidget;
	StaticTextWidget *_returnLine1;
	StaticTextWidget *_returnLine2;
	EditTextWidget *_codeWidget[CODE_FIELDS];
	StaticTextWidget *_messageWidget;

	GraphicsWidget *_picture;
	ButtonWidget *_openUrlWidget;
	ButtonWidget *_pasteCodeWidget;

	ButtonWidget *_cancelWidget;
	ButtonWidget *_connectWidget;

	bool _close;
#ifdef USE_SDL_NET
	bool _stopServerOnClose;
#endif

	/** Hides/shows widgets for Container to work with them correctly. */
	void containerWidgetsReflow();

	/** Return short scummvm.org URL for user to navigate to. */
	Common::String getUrl() const;

	/**
	 * Return the value corresponding to the given character.
	 *
	 * There is a value corresponding to each of 64 selected
	 * printable characters (0-9, A-Z, a-z, ? and !).
	 *
	 * When given another character, -1 is returned.
	 */
	int decodeHashchar(char c);

	/**
	 * Return whether checksum is correct.
	 *
	 * The last character of the string is treated as
	 * the checksum of all the others (decoded with
	 * decodeHashchar()).
	 *
	 * Checksum = (c[0] ^ c[1] ^ ...) % 64
	 */
	bool correctChecksum(Common::String s);

	/** The "CRC16_CCITT_FALSE" CRC-16 algorithm. */
	uint32 crc16(Common::String s);
public:
	StorageWizardDialog(uint32 storageId);

	virtual void open();
	virtual void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleTickle();
};

} // End of namespace GUI

#endif
