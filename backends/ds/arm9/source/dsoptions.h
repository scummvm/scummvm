/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 Neil Millstone
 * Copyright (C) 2006 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#ifndef _DSOPTIONS_H_
#define _DSOPTIONS_H_

#include "stdafx.h"

#include "common/scummsys.h"
#include "common/str.h"

#include "gui/object.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "scumm/dialogs.h"

namespace DS {

class DSOptionsDialog : public GUI::Dialog {

public:
	DSOptionsDialog();
	~DSOptionsDialog();
	
protected:
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);


	GUI::SliderWidget* _touchX;
	GUI::SliderWidget* _touchY;
	GUI::CheckboxWidget* _leftHandedCheckbox;
	GUI::CheckboxWidget* _unscaledCheckbox;
	GUI::CheckboxWidget* _twoHundredPercentCheckbox;
	GUI::CheckboxWidget* _indyFightCheckbox;
	GUI::CheckboxWidget* _highQualityAudioCheckbox;
	GUI::CheckboxWidget* _disablePowerOff;
#ifdef DS_SCUMM_BUILD
	Scumm::SaveLoadChooser* _delDialog;
#endif

};

extern void showOptionsDialog();
extern void setOptions();

}

#endif
