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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_INPUT_H
#define CRAB_INPUT_H

#include "crab/common_header.h"
#include "crab/inputval.h"

#define ANALOG_PRESSED 2

namespace pyrodactyl {
namespace input {
enum InputType {
	IT_NONE = -1,

	// Game related input values
	IG_UP,
	IG_DOWN,
	IG_RIGHT,
	IG_LEFT,
	IG_TALK,
	IG_MAP,
	IG_JOURNAL,
	IG_INVENTORY,
	IG_CHARACTER,
	IG_PAUSE,
	IG_QUICKSAVE,
	IG_QUICKLOAD,
	IG_ATTACK,
	IG_BLOCK,

	// UI related input values
	IU_UP,
	IU_DOWN,
	IU_RIGHT,
	IU_LEFT,
	IU_ACCEPT,
	IU_BACK,
	IU_NEXT,
	IU_PREV,
	IU_REPLY_0,
	IU_REPLY_1,
	IU_REPLY_2,
	IU_REPLY_3,
	IU_REPLY_4,
	IU_REPLY_5,
	IU_PAGE_NEXT,
	IU_PAGE_PREV,

	IT_TOTAL
};

// Constants related to menu size
const int IG_START = IG_UP, IG_SIZE = IG_BLOCK - IG_START + 1;
const int IU_START = IU_UP, IU_SIZE = IT_TOTAL - IU_START;

class InputManager {
	// The backups used to restore in case of the user pressing cancel
	InputVal backup[IT_TOTAL];

	// Load key configuration from file
	void Load(const std::string &filename);

	// The current version of the input scheme
	unsigned int version;

public:
	InputManager() {
		controller = nullptr;
		version = 0;
	}
	~InputManager() {}
	void Quit() {
		if (controller != nullptr)
			SDL_GameControllerClose(controller);
	}

	// NOTE: The lower level arrays can have buttons in common, but buttons cannot be common within these arrays
	// Ex. UI and Fight can have buttons in common, but not two keys within UI

	// Inputs used in the game
	InputVal iv[IT_TOTAL];

	// Our controller object
	SDL_GameController *controller;

	// These functions check if the value of a key matches the input values
	// Returns SDL_PRESSED for pressing a button, SDL_RELEASED for releasing a button and -1 if no input
	const int Equals(const InputType &val, const SDL_Event &Event);

	// These functions return true if key is pressed, false otherwise
	const bool State(const InputType &val);

	void CreateBackup();
	void RestoreBackup();

	// Initialize the controller if it is plugged in
	void AddController();

	// Handle plugging and unplugging of controllers on the fly
	void HandleController(const SDL_Event &Event);

	// Initialize the input system
	void Init();

	void Save();
};

// The saved key values
extern InputManager gInput;
} // End of namespace input
} // End of namespace pyrodactyl

#endif // CRAB_INPUT_H
