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
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/ButtonHandler.h"

#include "hpl1/penumbra-overture/Credits.h"
#include "hpl1/penumbra-overture/DeathMenu.h"
#include "hpl1/penumbra-overture/DemoEndText.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/IntroStory.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/MapLoadText.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/NumericalPanel.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PreMenu.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

#include "hpl1/penumbra-overture/MainMenu.h"

struct cButtonHandlerAction {
	const char *msName;
	const char *msType;
	int mlVal;
	bool mbConfig;
};

static const char *const gsLastPlayerAction = "GlowStick";
static constexpr cButtonHandlerAction gvDefaultActions[] = {
	{"Forward", "Keyboard", eKey_w, true},
	{"Backward", "Keyboard", eKey_s, true},
	{"Left", "Keyboard", eKey_a, true},
	{"Right", "Keyboard", eKey_d, true},

	{"LeanLeft", "Keyboard", eKey_q, true},
	{"LeanRight", "Keyboard", eKey_e, true},

	{"Run", "Keyboard", eKey_LSHIFT, true},
	{"Jump", "Keyboard", eKey_SPACE, true},
	{"Crouch", "Keyboard", eKey_LCTRL, true},

	{"InteractMode", "Keyboard", eKey_r, true},
	{"LookMode", "MouseButton", eMButton_Middle, true},

	{"Holster", "Keyboard", eKey_x, true},

	{"Examine", "MouseButton", eMButton_Right, true},
	{"Interact", "MouseButton", eMButton_Left, true},

	{"Inventory", "Keyboard", eKey_TAB, true},
	{"NoteBook", "Keyboard", eKey_n, true},
	{"PersonalNotes", "Keyboard", eKey_p, true},

	{"WheelUp", "MouseButton", eMButton_WheelUp, true},
	{"WheelDown", "MouseButton", eMButton_WheelDown, true},

	{"Flashlight", "Keyboard", eKey_f, true},
	{"GlowStick", "Keyboard", eKey_g, true},

	{"Escape", "Keyboard", eKey_ESCAPE, false},
	{"Enter", "Keyboard", eKey_RETURN, false},
	{"MouseClick", "MouseButton", eMButton_Left, false},
	{"MouseClickRight", "MouseButton", eMButton_Right, false},

	{"RightClick", "MouseButton", eMButton_Right, false},
	{"LeftClick", "MouseButton", eMButton_Left, false},

	{"One", "Keyboard", eKey_1, false},
	{"Two", "Keyboard", eKey_2, false},
	{"Three", "Keyboard", eKey_3, false},
	{"Four", "Keyboard", eKey_4, false},
	{"Five", "Keyboard", eKey_5, false},
	{"Six", "Keyboard", eKey_6, false},
	{"Seven", "Keyboard", eKey_7, false},
	{"Eight", "Keyboard", eKey_8, false},
	{"Nine", "Keyboard", eKey_9, false},

	// Debug:
	{"ResetGame", "Keyboard", eKey_F1, false},
	{"SaveGame", "Keyboard", eKey_F4, false},
	{"LoadGame", "Keyboard", eKey_F5, false},
#ifdef __APPLE__
	{"QuitGame", "Keyboard", eKeyModifier_META | eKey_q, false},
#endif
	{"LockInput", "Keyboard", eKey_k, false},
	{"Screenshot", "Keyboard", eKey_F12, false},

	//{"Hit","Keyboard",eKey_h,false},
	//{"Log","Keyboard",eKey_l,false},
	//{"Taunt","Keyboard",eKey_t,false},
	{"PrintLog", "Keyboard", eKey_l, false},

	{"", "", 0, false}};

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cButtonHandler::cButtonHandler(cInit *apInit) : iUpdateable("ButtonHandler") {
	mpInit = apInit;

	mpInput = mpInit->mpGame->GetInput();
	mpLowLevelGraphics = mpInit->mpGame->GetGraphics()->GetLowLevel();

	mState = eButtonHandlerState_Game;

	mlNumOfActions = 0;

	// INIT ALL ACTIONS USED
	const cButtonHandlerAction *pBHAction = &gvDefaultActions[0];
	while (pBHAction->msName[0] != '\0') {
		tString sName = pBHAction->msName;
		tString sType = mpInit->mpConfig->GetString("Keys", sName + "_Type", pBHAction->msType);
		tString sVal = mpInit->mpConfig->GetString("Keys", sName + "_Val", cString::ToString(pBHAction->mlVal));

		iAction *pAction = ActionFromTypeAndVal(sName, sType, sVal);
		if (pAction) {
			mpInput->AddAction(pAction);
		} else {
			Warning("Couldn't create action from '%s' and %d\n", pBHAction->msType,
					pBHAction->mlVal);
		}

		++pBHAction;
		++mlNumOfActions;
	}

	// LOAD SETTINGS
	mfMouseSensitivity = mpInit->mpConfig->GetFloat("Controls", "MouseSensitivity", 1.0f);
	mbInvertMouseY = mpInit->mpConfig->GetBool("Controls", "InvertMouseY", false);
	mbToggleCrouch = mpInit->mpConfig->GetBool("Controls", "ToggleCrouch", true);
}

//-----------------------------------------------------------------------

cButtonHandler::~cButtonHandler(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cButtonHandler::ChangeState(eButtonHandlerState aState) {
	mState = aState;
}

//-----------------------------------------------------------------------

void cButtonHandler::OnStart() {
	mpPlayer = mpInit->mpPlayer;
}

//-----------------------------------------------------------------------

void cButtonHandler::OnPostSceneDraw() {
	if (mpInit->mpGame->GetGraphics()->GetRenderer3D()->GetDebugFlags() & eRendererDebugFlag_LogRendering) {
		Log("-------------- STOP RENDERING LOG ------------------------\n");
		mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetDebugFlags(0);
	}
}

void cButtonHandler::Update(float afTimeStep) {
	static bool bLockState = true;
	///////////////////////////////////
	// GLOBAL Key Strokes
	///////////////////////////////////
	if (mpInput->BecameTriggerd("QuitGame")) {
		mpInit->mpGame->Exit();
	}
	if (mpInput->BecameTriggerd("Screenshot")) {
		int lCount = 1;
		tString sFileName = "screenshot000.bmp";
		while (FileExists(cString::To16Char(sFileName))) {
			sFileName = "screenshot";
			if (lCount < 10)
				sFileName += "00";
			else if (lCount < 100)
				sFileName += "0";
			sFileName += cString::ToString(lCount);
			sFileName += ".bmp";
			++lCount;
		}

		mpInit->mpGame->GetGraphics()->GetLowLevel()->SaveScreenToBMP(sFileName);
	}
	if (mpInit->_allowInputUnlock && mpInput->BecameTriggerd("LockInput")) {
		bLockState = !bLockState;
		mpInit->mpGame->GetInput()->GetLowLevel()->LockInput(bLockState);
	}
	///////////////////////////////////
	// DEMO END TEXT
	///////////////////////////////////
	if (mState == eButtonHandlerState_DemoEndText) {
		if (mpInput->BecameTriggerd("Escape"))
			mpInit->mpDemoEndText->OnButtonDown();
		if (mpInput->BecameTriggerd("LeftClick"))
			mpInit->mpDemoEndText->OnMouseDown(eMButton_Left);
		if (mpInput->BecameTriggerd("RightClick"))
			mpInit->mpDemoEndText->OnMouseDown(eMButton_Right);
	}
	///////////////////////////////////
	// CREDITS STATE
	///////////////////////////////////
	else if (mState == eButtonHandlerState_Credits) {
		if (mpInput->BecameTriggerd("Escape"))
			mpInit->mpCredits->OnButtonDown();
		if (mpInput->BecameTriggerd("LeftClick"))
			mpInit->mpCredits->OnMouseDown(eMButton_Left);
		if (mpInput->BecameTriggerd("RightClick"))
			mpInit->mpCredits->OnMouseDown(eMButton_Right);
	}
	///////////////////////////////////
	// PRE MENU STATE
	///////////////////////////////////
	else if (mState == eButtonHandlerState_PreMenu) {
		if (mpInput->BecameTriggerd("Escape"))
			mpInit->mpPreMenu->OnButtonDown();
		if (mpInput->BecameTriggerd("LeftClick"))
			mpInit->mpPreMenu->OnMouseDown(eMButton_Left);
		if (mpInput->BecameTriggerd("RightClick"))
			mpInit->mpPreMenu->OnMouseDown(eMButton_Right);
	}
	///////////////////////////////////
	// MAP LOAD TEXT STATE
	///////////////////////////////////
	else if (mState == eButtonHandlerState_MapLoadText) {
		if (mpInput->BecameTriggerd("Escape") ||
			mpInput->BecameTriggerd("RightClick") ||
			mpInput->BecameTriggerd("LeftClick")) {
			mpInit->mpMapLoadText->SetActive(false);
		}
	}
	///////////////////////////////////
	// MAIN MENU BUTTON STATE
	///////////////////////////////////
	else if (mState == eButtonHandlerState_MainMenu) {
		if (mpInput->BecameTriggerd("Escape")) {
			mpInit->mpMainMenu->Exit();
		}

		if (mpInput->BecameTriggerd("RightClick")) {
			mpInit->mpMainMenu->OnMouseDown(eMButton_Right);
			mpInput->BecameTriggerd("Examine");
		}
		if (mpInput->WasTriggerd("RightClick")) {
			mpInit->mpMainMenu->OnMouseUp(eMButton_Right);
		}
		if (mpInput->DoubleTriggerd("RightClick", 0.15f)) {
			mpInit->mpMainMenu->OnMouseDoubleClick(eMButton_Right);
		}

		if (mpInput->BecameTriggerd("LeftClick")) {
			mpInit->mpMainMenu->OnMouseDown(eMButton_Left);
			mpInput->BecameTriggerd("Interact");
		}
		if (mpInput->WasTriggerd("LeftClick")) {
			mpInit->mpMainMenu->OnMouseUp(eMButton_Left);
		}
		if (mpInput->DoubleTriggerd("LeftClick", 0.15f)) {
			mpInit->mpMainMenu->OnMouseDoubleClick(eMButton_Left);
		}
		/// Mouse Movement
		cVector2f vRel = mpInput->GetMouse()->GetRelPosition();
		mpInit->mpMainMenu->AddMousePos(vRel * mfMouseSensitivity);

	}
	///////////////////////////////////
	// INTRO BUTTON STATE
	///////////////////////////////////
	else if (mState == eButtonHandlerState_Intro) {
		if (mpInput->BecameTriggerd("Escape")) {
			mpInit->mpIntroStory->Exit();
		}
	}
	///////////////////////////////////
	// GAME BUTTON STATE
	///////////////////////////////////
	else if (mState == eButtonHandlerState_Game) {
		///////////////////////////////////////
		// Global ////////////////////
		/*if(mpInput->BecameTriggerd("ResetGame"))
		{
			mpInit->ResetGame(true);
			mpInit->mpMapHandler->Load(	mpInit->msStartMap,mpInit->msStartLink);
		}*/
		if (mpInit->mbAllowQuickSave) {
			if (mpInput->BecameTriggerd("SaveGame")) {
				mpInit->mpSaveHandler->AutoSave(_W("auto"), 5);
			}
			if (mpInput->BecameTriggerd("LoadGame")) {
				mpInit->mpSaveHandler->AutoLoad(_W("auto"));
			}
		}
		if (mpInput->BecameTriggerd("PrintLog")) {
			Log("-------------- START RENDERING LOG ------------------------\n");
			mpInit->mpGame->GetGraphics()->GetRenderer3D()->SetDebugFlags(eRendererDebugFlag_LogRendering);
		}
		// Check if no jump is pressed always.
		bool bPlayerStateIsActive = false;
		///////////////////////////////////////
		// Death menu ////////////////////
		if (mpInit->mpDeathMenu->IsActive()) {
			if (mpInput->BecameTriggerd("Escape")) {
				mpInit->mpGame->GetUpdater()->Reset();
				mpInit->mpMainMenu->SetActive(true);
			}

			if (mpInput->BecameTriggerd("RightClick")) {
				mpInit->mpDeathMenu->OnMouseDown(eMButton_Right);
				mpInput->BecameTriggerd("Examine");
			}

			if (mpInput->BecameTriggerd("LeftClick")) {
				mpInit->mpDeathMenu->OnMouseDown(eMButton_Left);
				mpInput->BecameTriggerd("Interact");
			}
			if (mpInput->WasTriggerd("LeftClick")) {
				mpInit->mpDeathMenu->OnMouseUp(eMButton_Left);
			}
			/// Mouse Movement
			cVector2f vRel = mpInput->GetMouse()->GetRelPosition();
			mpInit->mpDeathMenu->AddMousePos(vRel * mfMouseSensitivity);
		}
		///////////////////////////////////////
		// Death ////////////////////
		else if (mpPlayer->IsDead()) {
			if (mpInput->BecameTriggerd("Escape")) {
				mpInit->mpMainMenu->SetActive(true);
			}
		}
		///////////////////////////////////////
		// Numerical panel ////////////////////
		else if (mpInit->mpNumericalPanel->IsActive()) {
			if (mpInput->BecameTriggerd("Inventory") || mpInput->BecameTriggerd("Escape")) {
				mpInit->mpNumericalPanel->OnExit();
			}
			if (mpInput->BecameTriggerd("RightClick")) {
				mpInit->mpNumericalPanel->OnExit();
			}

			if (mpInput->BecameTriggerd("LeftClick")) {
				mpInit->mpNumericalPanel->OnMouseDown(eMButton_Left);
				mpInput->BecameTriggerd("Interact");
			}
			if (mpInput->WasTriggerd("LeftClick")) {
				mpInit->mpNumericalPanel->OnMouseUp(eMButton_Left);
			}
			/// Mouse Movement
			cVector2f vRel = mpInput->GetMouse()->GetRelPosition();
			mpInit->mpNumericalPanel->AddMousePos(vRel * mfMouseSensitivity);
		}
		///////////////////////////////////////
		// Notebook ////////////////////
		else if (mpInit->mpNotebook->IsActive()) {
			if (mpInput->BecameTriggerd("Inventory") || mpInput->BecameTriggerd("Escape")) {
				mpInit->mpNotebook->OnExit();
			}

			if (mpInput->BecameTriggerd("LeftClick")) {
				mpInit->mpNotebook->OnMouseDown(eMButton_Left);

				mpInput->BecameTriggerd("Interact");
			}

			if (mpInput->BecameTriggerd("NoteBook")) {
				mpInit->mpNotebook->OnExit();
			}
			if (mpInput->BecameTriggerd("PersonalNotes")) {
				cStateMachine *pStateMachine = mpInit->mpNotebook->GetStateMachine();
				if (pStateMachine->CurrentState()->GetId() == eNotebookState_TaskList) {
					pStateMachine->ChangeState(eNotebookState_Front);
					mpInit->mpNotebook->OnExit();
				} else {
					pStateMachine->ChangeState(eNotebookState_TaskList);
				}
			}
			/// Mouse Movement
			cVector2f vRel = mpInput->GetMouse()->GetRelPosition();
			mpInit->mpNotebook->AddMousePos(vRel * mfMouseSensitivity);
		}
		///////////////////////////////////////
		// Inventory ////////////////////
		else if (mpInit->mpInventory->IsActive()) {
			////////////////////////////
			// Normal Input
			if (mpInput->BecameTriggerd("Inventory") || mpInput->BecameTriggerd("Escape")) {
				mpInit->mpInventory->OnInventoryDown();
			}

			if (mpInput->BecameTriggerd("LeftClick")) {
				mpInit->mpInventory->OnMouseDown(eMButton_Left);

				mpInput->BecameTriggerd("Interact");
			}

			if (mpInput->DoubleTriggerd("LeftClick", 0.2f)) {
				mpInit->mpInventory->OnDoubleClick(eMButton_Left);
			}
			if (mpInput->WasTriggerd("LeftClick")) {
				mpInit->mpInventory->OnMouseUp(eMButton_Left);
			}

			if (mpInput->BecameTriggerd("RightClick")) {
				mpInit->mpInventory->OnMouseDown(eMButton_Right);

				mpInput->BecameTriggerd("Examine");
			}
			if (mpInput->WasTriggerd("RightClick")) {
				mpInit->mpInventory->OnMouseUp(eMButton_Right);
			}

			//////////////////////////////
			// Short cut keys
			if (mpInput->BecameTriggerd("One"))
				mpInit->mpInventory->OnShortcutDown(0);
			if (mpInput->BecameTriggerd("Two"))
				mpInit->mpInventory->OnShortcutDown(1);
			if (mpInput->BecameTriggerd("Three"))
				mpInit->mpInventory->OnShortcutDown(2);
			if (mpInput->BecameTriggerd("Four"))
				mpInit->mpInventory->OnShortcutDown(3);
			if (mpInput->BecameTriggerd("Five"))
				mpInit->mpInventory->OnShortcutDown(4);
			if (mpInput->BecameTriggerd("Six"))
				mpInit->mpInventory->OnShortcutDown(5);
			if (mpInput->BecameTriggerd("Seven"))
				mpInit->mpInventory->OnShortcutDown(6);
			if (mpInput->BecameTriggerd("Eight"))
				mpInit->mpInventory->OnShortcutDown(7);
			if (mpInput->BecameTriggerd("Nine"))
				mpInit->mpInventory->OnShortcutDown(8);
			/// Mouse Movement
			cVector2f vRel = mpInput->GetMouse()->GetRelPosition();
			mpInit->mpInventory->AddMousePos(vRel * mfMouseSensitivity);
		} else {
			bPlayerStateIsActive = true;

			if (mpInput->BecameTriggerd("Escape")) {
				mpInit->mpMainMenu->SetActive(true);
			}
			if (mpInput->BecameTriggerd("Hit")) {
				mpInit->mpPlayer->Damage(20, ePlayerDamageType_BloodSplash);
			}
			/*if(mpInput->BecameTriggerd("Log"))
			{
				mpInit->mpGame->GetPhysics()->SetDebugLog(!mpInit->mpGame->GetPhysics()->GetDebugLog());
			}*/
			if (mpInput->BecameTriggerd("Taunt")) {
				for (int i = 0; i < 10; ++i)
					mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("gui_notebook_add_note1",
																		   false, 0.01f);
				/*cVector3f vPos = mpInit->mpPlayer->GetCharacterBody()->GetPosition();
				cSoundEntity *pSound = mpInit->mpGame->GetScene()->GetWorld3D()->CreateSoundEntity("Taunt","interact_homer",true);
				if(pSound)
				{
					pSound->SetPosition(vPos);
				}*/
			}

			if (mpPlayer->IsActive() || mpPlayer->GetState() == ePlayerState_Message) {
				if (mpPlayer->IsActive()) {
					if (mpInput->BecameTriggerd("Inventory")) {
						mpPlayer->StartInventory();
					}

					if (mpInput->BecameTriggerd("NoteBook")) {
						mpInit->mpNotebook->SetActive(true);
					}
					if (mpInput->BecameTriggerd("PersonalNotes")) {
						mpInit->mpNotebook->SetActive(true);
						mpInit->mpNotebook->GetStateMachine()->ChangeState(eNotebookState_TaskList);
					}

					if (mpInput->BecameTriggerd("Flashlight")) {
						mpPlayer->StartFlashLightButton();
					}

					if (mpInput->BecameTriggerd("GlowStick")) {
						mpPlayer->StartGlowStickButton();
					}

					///////////////////////////////////////
					// Player Movement ////////////////////
					if (mpInput->IsTriggerd("Forward")) {
						mpPlayer->MoveForwards(1, afTimeStep);
					} else if (mpInput->IsTriggerd("Backward")) {
						mpPlayer->MoveForwards(-1, afTimeStep);
					} else {
						mpPlayer->MoveForwards(0, afTimeStep);
					}

					if (mpInput->IsTriggerd("Left")) {
						mpPlayer->MoveSideways(-1, afTimeStep);
					} else if (mpInput->IsTriggerd("Right")) {
						mpPlayer->MoveSideways(1, afTimeStep);
					} else {
						mpPlayer->MoveSideways(0, afTimeStep);
					}

					if (mpInput->IsTriggerd("LeanLeft")) {
						mpPlayer->Lean(-1, afTimeStep);
					} else if (mpInput->IsTriggerd("LeanRight")) {
						mpPlayer->Lean(1, afTimeStep);
					}

					if (mpInput->BecameTriggerd("Jump")) {
						mpPlayer->Jump();
					}
					if (mpInput->IsTriggerd("Jump")) {
						mpPlayer->SetJumpButtonDown(true);
					}

					if (mpInput->BecameTriggerd("Run")) {
						mpPlayer->StartRun();
					}
					if (mpInput->WasTriggerd("Run")) {
						mpPlayer->StopRun();
					}

					if (mpInput->BecameTriggerd("Crouch")) {
						mpPlayer->StartCrouch();
					}
					if (GetToggleCrouch()) {
						if (mpInput->WasTriggerd("Crouch"))
							mpPlayer->StopCrouch();
					} else {
						if (mpInput->IsTriggerd("Crouch") == false)
							mpPlayer->StopCrouch();
					}

					if (mpInput->BecameTriggerd("InteractMode")) {
						mpPlayer->StartInteractMode();
					}
					cVector2f vRel = mpInput->GetMouse()->GetRelPosition();
					vRel /= mpLowLevelGraphics->GetVirtualSize();

					mpPlayer->AddYaw(vRel.x * mfMouseSensitivity);
					mpPlayer->AddPitch(vRel.y * mfMouseSensitivity);
				}

				///////////////////////////////////////
				// Player Interaction /////////////////
				if (mpInput->BecameTriggerd("Interact")) {
					mpPlayer->StartInteract();
					mpInput->BecameTriggerd("LeftClick");
				}
				if (mpInput->WasTriggerd("Interact")) {
					mpPlayer->StopInteract();
				}
				if (mpInput->BecameTriggerd("Examine")) {
					mpPlayer->StartExamine();
				}
				if (mpInput->WasTriggerd("Examine")) {
					mpPlayer->StopExamine();
				}
				if (mpInput->BecameTriggerd("Holster")) {
					mpPlayer->StartHolster();
				}

				if (mpPlayer->IsActive()) {
					if (mpInput->BecameTriggerd("One"))
						mpPlayer->StartInventoryShortCut(0);
					if (mpInput->BecameTriggerd("Two"))
						mpPlayer->StartInventoryShortCut(1);
					if (mpInput->BecameTriggerd("Three"))
						mpPlayer->StartInventoryShortCut(2);
					if (mpInput->BecameTriggerd("Four"))
						mpPlayer->StartInventoryShortCut(3);
					if (mpInput->BecameTriggerd("Five"))
						mpPlayer->StartInventoryShortCut(4);
					if (mpInput->BecameTriggerd("Six"))
						mpPlayer->StartInventoryShortCut(5);
					if (mpInput->BecameTriggerd("Seven"))
						mpPlayer->StartInventoryShortCut(6);
					if (mpInput->BecameTriggerd("Eight"))
						mpPlayer->StartInventoryShortCut(7);
					if (mpInput->BecameTriggerd("Nine"))
						mpPlayer->StartInventoryShortCut(8);
				}
			}
		}
		if (mpInput->IsTriggerd("Jump") == false || bPlayerStateIsActive == false) {
			mpPlayer->SetJumpButtonDown(false);
		}
	}
}

//-----------------------------------------------------------------------

void cButtonHandler::Reset() {
}

//-----------------------------------------------------------------------

void cButtonHandler::OnExit() {
	// SAVE SETTINGS
	Log("  Saving to config\n");
	mpInit->mpConfig->SetFloat("Controls", "MouseSensitivity", mfMouseSensitivity);
	mpInit->mpConfig->SetBool("Controls", "InvertMouseY", mbInvertMouseY);
	mpInit->mpConfig->SetBool("Controls", "ToggleCrouch", mbToggleCrouch);

	// SAVE KEYS
	Log("  Saving keys\n");
	for (int i = 0; i < mlNumOfActions; ++i) {
		// Log(" Action %s\n",gvDefaultActions[i].msName.c_str());

		iAction *pAction = mpInput->GetAction(gvDefaultActions[i].msName);
		tString sType = "", sVal = "";
		TypeAndValFromAction(pAction, &sType, &sVal);

		// Log(" type %s val: %s\n",sType.c_str(),sVal.c_str());

		mpInit->mpConfig->SetString("Keys", gvDefaultActions[i].msName + Common::String("_Type"), sType);
		mpInit->mpConfig->SetString("Keys", gvDefaultActions[i].msName + Common::String("_Val"), sVal);
	}
}

//-----------------------------------------------------------------------

void cButtonHandler::SetDefaultKeys() {
	const cButtonHandlerAction *pBHAction = &gvDefaultActions[0];
	while (pBHAction->msName[0] != '\0') {
		tString sName = pBHAction->msName;
		tString sType = pBHAction->msType;
		tString sVal = cString::ToString(pBHAction->mlVal);

		iAction *pAction = ActionFromTypeAndVal(sName, sType, sVal);

		if (pAction) {
			mpInput->DestroyAction(sName);
			mpInput->AddAction(pAction);
		} else {
			Warning("Couldn't create action from '%s' and %d\n", pBHAction->msType,
					pBHAction->mlVal);
		}

		++pBHAction;
	}
}

//-----------------------------------------------------------------------

tString cButtonHandler::GetActionName(const tString &asInputName, const tString &asSkipAction) {
	const cButtonHandlerAction *pBHAction = &gvDefaultActions[0];
	while (pBHAction->msName[0] != '\0') {
		tString sName = pBHAction->msName;
		tString sType = pBHAction->msType;
		tString sVal = cString::ToString(pBHAction->mlVal);

		iAction *pAction = mpInput->GetAction(sName);

		if (asSkipAction != sName && pAction && pAction->GetInputName() == asInputName)
			return sName;

		// If at last player action, skip the rest.
		if (sName == gsLastPlayerAction)
			return "";

		++pBHAction;
	}

	return "";
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

iAction *cButtonHandler::ActionFromTypeAndVal(const tString &asName, const tString &asType, const tString &asVal) {
	// Log("Action %s from %s\n",asName.c_str(),asType.c_str());

	if (asType == "Keyboard") {
		return hplNew(cActionKeyboard, (asName, mpInit->mpGame->GetInput(), (eKey)cString::ToInt(asVal.c_str(), 0)));
	} else if (asType == "MouseButton" || asType == "HapticDeviceButton") {
		return hplNew(cActionMouseButton, (asName, mpInit->mpGame->GetInput(), (eMButton)cString::ToInt(asVal.c_str(), 0)));
	}

	return NULL;
}

//-----------------------------------------------------------------------

void cButtonHandler::TypeAndValFromAction(iAction *apAction, tString *apType, tString *apVal) {
	if (apAction) {
		*apType = apAction->GetInputType();

		if (apAction->GetInputType() == "Keyboard") {
			cActionKeyboard *pKeyAction = static_cast<cActionKeyboard *>(apAction);
			*apVal = cString::ToString((int)pKeyAction->GetKey() | (int)pKeyAction->GetModifier());
		} else if (apAction->GetInputType() == "MouseButton" ||
				   apAction->GetInputType() == "HapticDeviceButton") {
			cActionMouseButton *pMouseAction = static_cast<cActionMouseButton *>(apAction);
			*apVal = cString::ToString((int)pMouseAction->GetButton());
		}
	} else {
		*apVal = "";
		*apType = "";
	}
}

//-----------------------------------------------------------------------
