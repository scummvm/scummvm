/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_BIOCHIP_RIGHT_H
#define BURIED_BIOCHIP_RIGHT_H

#include "buried/window.h"

namespace Buried {

// FIXME: Why is this here?
enum {
	CASTLE_EVIDENCE_FOOTPRINT = 1,
	CASTLE_EVIDENCE_SWORD = 2,
	MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID = 3,
	MAYAN_EVIDENCE_PHONY_BLOOD = 4,
	MAYAN_EVIDENCE_ENVIRON_CART = 5,
	CASTLE_EVIDENCE_AGENT3 = 6,
	AI_EVIDENCE_SCULPTURE = 7,
	DAVINCI_EVIDENCE_FOOTPRINT = 8,
	DAVINCI_EVIDENCE_AGENT3 = 9,
	DAVINCI_EVIDENCE_CODEX = 10,
	DAVINCI_EVIDENCE_LENS_FILTER = 11
};

class BioChipRightWindow : public Window {
public:
	BioChipRightWindow(BuriedEngine *vm, Window *parent);
	~BioChipRightWindow();

	bool changeCurrentBioChip(int bioChipID);
	bool showBioChipMainView();
	bool destroyBioChipViewWindow();
	void swapAIBioChipIfActive();
	void sceneChanged();
	void disableEvidenceCapture();
	void jumpInitiated(bool redraw);
	void jumpEnded(bool redraw);
	void toggleBioChip();
	int getCurrentBioChip() const;

	void onPaint();
	void onEnable(bool enable);
	void onLButtonUp(const Common::Point &point, uint flags);

	// These are used to enable the help and comment buttons
	// when the player finds Arthur, while he explains how
	// these two buttons are used.
	bool _forceHelp;
	bool _forceComment;

private:
	int _curBioChip;
	int _status;
	Window *_bioChipViewWindow;
	bool _jumpInProgress;
};

} // End of namespace Buried

#endif
