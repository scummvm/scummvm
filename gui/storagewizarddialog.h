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
	uint32 _storageId;

	StaticTextWidget *_headlineWidget;
	StaticTextWidget *_navigateLineWidget;
	StaticTextWidget *_urlLineWidget;
	StaticTextWidget *_returnLine1;
	StaticTextWidget *_returnLine2;
	EditTextWidget *_codeWidget;
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

public:
	StorageWizardDialog(uint32 storageId);

	virtual void open();
	virtual void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleTickle();
};

} // End of namespace GUI

#endif
