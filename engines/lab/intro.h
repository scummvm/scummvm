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

 /*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_INTRO_H
#define LAB_INTRO_H

namespace Lab {

class Intro {
public:
	Intro(LabEngine *vm);
	~Intro();

	/**
	 * Does the introduction sequence for Labyrinth.
	 */
	void play();

private:
	/**
	 * Goes through, and responds to all the intuition messages currently in the
	 * message queue.
	 */
	void introEatMessages();

	/**
	 * Reads in a picture.
	 */
	void doPictText(const Common::String filename, bool isScreen = false);

	void nReadPict(const Common::String filename, bool playOnce = true, bool noPalChange = false, bool doBlack = false, int wait = 0);

	LabEngine *_vm;
	bool _quitIntro;
	TextFont *_font;
};

} // End of namespace Lab

#endif // LAB_INTRO_H
