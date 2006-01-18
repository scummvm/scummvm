/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef CELAUNCHERDIALOG
#define CELAUNCHERDIALOG

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "gui/launcher.h"

class CELauncherDialog : public GUI::LauncherDialog {
public:
	CELauncherDialog(GameDetector &detector);
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
protected:
	void addGame();
	void addCandidate(String &path, DetectedGameList &candidates);
	void automaticScanDirectory(const FilesystemNode &node);
};

typedef GUI::LauncherDialog GUILauncherDialog;

#endif
