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

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

void SceneScriptPS09::InitializeScene() {
	if (Game_Flag_Query(kFlagMcCoyArrested)) {
		Setup_Scene_Information(-410.0f, 0.26f, -200.0f, 512);
	} else {
		Setup_Scene_Information(-559.0f, 0.0f, -85.06f, 250);
	}

	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);

	Ambient_Sounds_Remove_All_Non_Looping_Sounds(false);
	Ambient_Sounds_Add_Looping_Sound(kSfxBRBED5X,  50, 0, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxPRISAMB1, 30, 0, 0);
	Ambient_Sounds_Add_Looping_Sound(kSfxPRISAMB3, 30, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPRISSLM1, 15, 60, 7, 10, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPRISSLM2, 25, 60, 7, 10, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(kSfxPRISSLM3, 25, 60, 7, 10, 100, 100, -101, -101, 0, 0);

	if (!Game_Flag_Query(kFlagGrigorianArrested)) {
		Actor_Put_In_Set(kActorGrigorian, kSetPS09);
		Actor_Set_At_XYZ(kActorGrigorian, -417.88f, 0.0f, -200.74f, 512);
		Game_Flag_Set(kFlagGrigorianArrested);
	}
	if (Game_Flag_Query(kFlagMcCoyArrested)) {
		Actor_Put_In_Set(kActorGrigorian, kSetFreeSlotD);
		Actor_Set_At_XYZ(kActorGrigorian, 0.0f, 0.0f, 0.0f, 512);
	}
	if (Game_Flag_Query(kFlagIzoArrested)) {
		Actor_Put_In_Set(kActorIzo, kSetPS09);
		Actor_Set_At_XYZ(kActorIzo, -476.0f, 0.2f, -225.0f, 518);
	}
	if (Game_Flag_Query(kFlagCrazylegsArrested)) { // cut feature? it is impossible to arrest crazylegs
		Actor_Put_In_Set(kActorCrazylegs, kSetPS09);
		Actor_Set_At_XYZ(kActorCrazylegs, -290.0f, 0.33f, -235.0f, 207);
	}
}

void SceneScriptPS09::SceneLoaded() {
	Obstacle_Object("OFFICE DOOR", true);
	Unobstacle_Object("OFFICE DOOR", true);
	Unclickable_Object("OFFICE DOOR");
}

bool SceneScriptPS09::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS09::ClickedOnActor(int actorId) {
	if (actorId == kActorGrigorian) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -381.11f, 0.0f, -135.55f, 0, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorGrigorian, true);
			Actor_Face_Actor(kActorGrigorian, kActorMcCoy, true);

			if (!Game_Flag_Query(kFlagPS09GrigorianTalk1)) {
				Actor_Says(kActorGrigorian, 0, 12);
				Actor_Says(kActorMcCoy, 4235, 18);
				Actor_Says(kActorGrigorian, 10, 13);
				Game_Flag_Set(kFlagPS09GrigorianTalk1);
				return true;
			}

			if ( Game_Flag_Query(kFlagPS09GrigorianTalk1)
			 && !Game_Flag_Query(kFlagPS09GrigorianTalk2)
			 && !Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewA)
			 && !Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)
			 && !Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB2)
			) {
				Actor_Says(kActorMcCoy, 4245, 14);
				Actor_Says(kActorGrigorian, 20, 14);
				Game_Flag_Set(kFlagPS09GrigorianTalk2);
				return true;
			}

			if (!Game_Flag_Query(kFlagPS09GrigorianDialogue)
			 &&  Game_Flag_Query(kFlagPS09GrigorianTalk1)
			 &&  (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewA)
			  ||  Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)
			  ||  Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB2)
			  ||  Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)
			 )
			) {
				Game_Flag_Set(kFlagPS09GrigorianDialogue);
				Actor_Says(kActorMcCoy, 4240, 13);
				Actor_Says(kActorGrigorian, 550, 15);
				Actor_Says(kActorGrigorian, 480, 16);
				dialogueWithGrigorian();
				return true;
			}

			if (Game_Flag_Query(kFlagGrigorianDislikeMcCoy)) {
				Actor_Says(kActorMcCoy, 4270, 18);
				Actor_Says(kActorGrigorian, 30, 14);
				Actor_Says(kActorGrigorian, 40, 13);
				return true;
			}

			if (Game_Flag_Query(kFlagPS09GrigorianDialogue)
			 && Game_Flag_Query(kFlagPS09GrigorianTalk1)
			 && (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewA)
			  || Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)
			  || Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote)
			 )
			) {
				dialogueWithGrigorian();
				return true;
			}

			Actor_Says(kActorMcCoy, 4270, 18);
			Actor_Says(kActorGrigorian, 30, 14);
			Actor_Says(kActorGrigorian, 40, 13);
			return true;
		}
	}

	if (actorId == kActorIzo) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -473.0f, 0.2f, -133.0f, 12, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorIzo, true);
			Actor_Face_Actor(kActorIzo, kActorMcCoy, true);

			if (!Game_Flag_Query(kFlagPS09IzoTalk1)) {
				Actor_Says(kActorMcCoy, 4200, 14);
				Actor_Says(kActorIzo, 570, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4205, 18);
				Game_Flag_Set(kFlagPS09IzoTalk1);
				return true;
			}

			if ( Game_Flag_Query(kFlagPS09IzoTalk1)
			 && !Game_Flag_Query(kFlagPS09IzoTalk2)
			) {
				Actor_Says(kActorMcCoy, 4210, 18);
				Actor_Says(kActorIzo, 580, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4215, 14);
				Actor_Says(kActorIzo, 590, kAnimationModeTalk);
				Actor_Says(kActorIzo, 600, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4220, 18);
				Actor_Says(kActorIzo, 610, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4225, 19);
				Actor_Says(kActorIzo, 620, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4230, 14);
				Game_Flag_Set(kFlagPS09IzoTalk2);
				return true;
			}

			Actor_Says(kActorMcCoy, 4200, 13);
			return true;
		}
	}

	if (actorId == kActorCrazylegs) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -295.0f, 0.34f, -193.0f, 12, true, false, false)) {
			Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);

			if (!Game_Flag_Query(kFlagPS09CrazylegsTalk1)) {
				Actor_Says(kActorMcCoy, 4415, 18);
				Actor_Says(kActorCrazylegs, 1090, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4420, 18);
				Game_Flag_Set(kFlagPS09CrazylegsTalk1);
			}

			if ( Game_Flag_Query(kFlagPS09CrazylegsTalk1)
			 &&  Game_Flag_Query(kFlagGrigorianArrested)
			 && !Game_Flag_Query(kFlagPS09CrazylegsGrigorianTalk)
			) {
				Actor_Face_Actor(kActorGrigorian, kActorCrazylegs, true);
				Actor_Says(kActorGrigorian, 420, 14);
				Actor_Face_Actor(kActorCrazylegs, kActorGrigorian, true);
				Actor_Says(kActorCrazylegs, 1120, kAnimationModeTalk);
				Actor_Face_Actor(kActorMcCoy, kActorGrigorian, true);
				Actor_Says(kActorMcCoy, 4435, 14);
				Actor_Says(kActorGrigorian, 430, 16);
				Actor_Says(kActorCrazylegs, 1130, kAnimationModeTalk);
				Game_Flag_Set(kFlagPS09CrazylegsGrigorianTalk);
				return true;
			}

			if ( Game_Flag_Query(kFlagPS09CrazylegsTalk1)
			 && !Game_Flag_Query(kFlagGrigorianArrested)
			 && !Game_Flag_Query(kFlagPS09CrazylegsTalk2)) {
				Actor_Says(kActorMcCoy, 4425, 18);
				Actor_Says(kActorCrazylegs, 1100, kAnimationModeTalk);
				Actor_Says(kActorMcCoy, 4430, 19);
				Actor_Says(kActorCrazylegs, 1110, kAnimationModeTalk);
				Game_Flag_Set(kFlagPS09CrazylegsTalk2);
				return true;
			}

			Actor_Says(kActorMcCoy, 4425, 18);
			Actor_Says(kActorCrazylegs, 1160, kAnimationModeTalk);
			return true;
		}
	}
	return false;
}

bool SceneScriptPS09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -559.15f, 0.0f, -85.06f, 0, true, false, false)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(kSetPS02, kScenePS02);
			Game_Flag_Reset(kFlagPS09Entered);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS09::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS09::SceneFrameAdvanced(int frame) {
	if (frame == 1
	 || frame == 15
	 || frame == 20
	 || frame == 31
	 || frame == 33
	 || frame == 35
	 || frame == 52
	 || frame == 54
	) {
		Sound_Play(kSfxNEON7, Random_Query(50, 33), 10, 10, 50);
	}
	//return true;
}

void SceneScriptPS09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS09::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagMcCoyArrested)) {
		Player_Loses_Control();
		Delay(2000);
		Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
		//return true;
		return;
	}
	if (!Game_Flag_Query(kFlagPS09Entered)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -491.15f, 0.0f, -73.06f, 0, false, false, false);
		Player_Gains_Control();
		Game_Flag_Set(kFlagPS09Entered);
	}
	if (Game_Flag_Query(kFlagPS02toPS09)) {
		Game_Flag_Reset(kFlagPS02toPS09);
		//return true;
		return;
	}
	//return false;
}

void SceneScriptPS09::PlayerWalkedOut() {
}

void SceneScriptPS09::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS09::dialogueWithGrigorian() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewA)
	 || Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)
	 || Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB2)
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(170,  5, 5, 3); // PROTEST
		DM_Add_To_List_Never_Repeat_Once_Selected(180, -1, 5, 5); // CARS
		if ((_vm->_cutContent
		     && (!Game_Flag_Query(kFlagPS09GrigorianVKChosen)
		         && (!Actor_Clue_Query(kActorMcCoy, kClueVKGrigorianHuman) && !Actor_Clue_Query(kActorMcCoy, kClueVKGrigorianReplicant))))
		    || !_vm->_cutContent
		) {
			if (_vm->_cutContent) {
				Dialogue_Menu_Clear_Never_Repeat_Was_Selected_Flag(200);
			}
			DM_Add_To_List_Never_Repeat_Once_Selected(200, -1, 3, 6); // VOIGT-KAMPFF
		}
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueGrigoriansNote) // cut feature? it is impossible to obtain this clue
	 && (Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewA)
	  || Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB1)
	  || Actor_Clue_Query(kActorMcCoy, kClueGrigorianInterviewB2)
	 )
	) {
		DM_Add_To_List_Never_Repeat_Once_Selected(190, 5, 6, -1); // NOTE
	}
#if BLADERUNNER_ORIGINAL_BUGS
	Dialogue_Menu_Add_To_List(210); // DONE // A bug? why not Dialogue_Menu_Add_DONE_To_List?
#else
	Dialogue_Menu_Add_DONE_To_List(210); // DONE
#endif // BLADERUNNER_ORIGINAL_BUGS

	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 170: // PROTEST
		Actor_Says(kActorMcCoy, 4270, 13);
		Actor_Says(kActorMcCoy, 4250, 18);
		Actor_Says(kActorGrigorian, 50, 13);
		Actor_Says(kActorMcCoy, 4275, 18);
		Actor_Says(kActorMcCoy, 4280, 19);
		if (Game_Flag_Query(kFlagIzoIsReplicant)) {
			Actor_Says(kActorGrigorian, 60, 14);
			Actor_Says(kActorMcCoy, 4285, 13);
			Actor_Says(kActorGrigorian, 70, 12);
			Actor_Says(kActorMcCoy, 4290, 13);
			Actor_Says(kActorGrigorian, 80, 13);
			Actor_Says(kActorGrigorian, 90, 13);
			Actor_Says(kActorMcCoy, 4295, 18);
			Actor_Says(kActorGrigorian, 110, 14);
			Actor_Says(kActorMcCoy, 4300, 17);
			return;
		}
		if (!Game_Flag_Query(kFlagIzoIsReplicant)) {
			Actor_Says(kActorGrigorian, 130, 15);
			Actor_Says(kActorGrigorian, 140, 13);
			Actor_Says(kActorMcCoy, 4305, 13);
			Actor_Says(kActorGrigorian, 150, 14);
			Actor_Says(kActorGrigorian, 160, 12);
			Actor_Says(kActorMcCoy, 4310, 13);
			Actor_Says(kActorGrigorian, 170, 15);
			Actor_Says(kActorGrigorian, 180, 16);
			Actor_Says(kActorMcCoy, 4315, 18);
			if (_vm->_cutContent
			    && (_vm->_language == Common::ES_ESP
			        || _vm->_language == Common::IT_ITA)
			) {
				// In ESP and ITA versions:
				// Quote 190 is the full quote.
				// "Everything and anything. Guns so new that even the police had hardly used them I heard."
				// Quote 200 is muted in ESP and becomes redundant in ITA
				Actor_Says(kActorGrigorian, 190, kAnimationModeTalk);
				// TODO: When mixing ESP or ITA voiceover with subtitles from other languages,
				//       those subtitles would have to be merged with the 190 quote
				//       in order to show up in the ESP amd ITA version
				//       Also, the spoken quote appears in ESP version only in restored content mode!
			} else if (_vm->_cutContent) {
				// TODO is RUS version covered by this case?
				// In ENG, FRA and DEU versions:
				// Quote 190 is the first half:
				// "Everything and anything."
				// and quote 200 is the second half:
				// "Guns so new that even the police had hardly used them I heard."
				Actor_Says(kActorGrigorian, 190, kAnimationModeTalk);
				Actor_Says(kActorGrigorian, 200, 13);
			} else {
				// vanilla version (non-restored content)
				// This plays only the second half of the full quote in ENG, FRA, DEU and ITA versions
#if BLADERUNNER_ORIGINAL_BUGS
				Actor_Says(kActorGrigorian, 200, 13);
#else
				// Quote 200 is muted in the ESP version
				if (_vm->_language != Common::ES_ESP) {
					Actor_Says(kActorGrigorian, 200, 13);
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
			return;
		}
		break;

	case 180: // CARS
		Actor_Says(kActorMcCoy, 4270, 18);
		Actor_Says(kActorMcCoy, 4255, kAnimationModeTalk);
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorGrigorian, 210, 12);
		Actor_Says(kActorGrigorian, 220, 13);
		Actor_Says(kActorGrigorian, 230, 14);
#else
		if (_vm->_language != Common::ES_ESP) {
			Actor_Says(kActorGrigorian, 210, 12);
			Actor_Says(kActorGrigorian, 220, 13);
			// pause (after the quote is spoken) is set to 0.0f here
			// Grigorian is interrupted by McCoy here, so there shouldn't be any pause after his quote
			Actor_Says_With_Pause(kActorGrigorian, 230, 0.0f, 14);
		} else {
			// In ESP version, quote 210 contains the full quote,
			// and quotes 220 and 230 are muted.
			// The pause (after the quote is spoken) is set to 0.0f here, because Grigorian is interrupted in the end
			Actor_Says_With_Pause(kActorGrigorian, 210, 0.0f, 12);
			// we skip the muted quotes for the ESP version
			// TODO: When mixing ESP voiceover with subtitles from other languages,
			//       those subtitles would have to be merged with the previous full quote (210)
			//       in order to show up in the ESP version
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorMcCoy, 4320, 14);
		if (_vm->_cutContent) {
			// "What else do you guys do besides wave signs"
			Actor_Says(kActorMcCoy, 4325, kAnimationModeTalk);
		}
		Actor_Says(kActorGrigorian, 240, 16);
		Actor_Says(kActorGrigorian, 250, 15);
		Actor_Says(kActorMcCoy, 4330, 13);
		Actor_Says(kActorGrigorian, 260, 13);
		Actor_Says(kActorGrigorian, 270, 12);
		if (_vm->_cutContent) {
			// In the ESP version quote 280 is muted. The previous quote (270) already contains the full quote.
			// Essentially, the full quote is not removed content in the vanilla ESP version
			// TODO: When mixing ESP voiceover with subtitles from other languages,
			//       those subtitles would have to be merged with the previous full quote (270)
			//       in order to show up in the ESP version
			if (_vm->_language != Common::ES_ESP) {
				// "A way for the slaves to escape"
				Actor_Says(kActorGrigorian, 280, 12);
			}
		}
		Actor_Says(kActorMcCoy, 4335, 18);
		Actor_Says(kActorGrigorian, 290, 15);
		Actor_Says(kActorMcCoy, 4340, 13);
		Actor_Modify_Friendliness_To_Other(kActorGrigorian, kActorMcCoy, -5);
		if (Game_Flag_Query(kFlagCrazylegsArrested)) {
			Actor_Says(kActorGrigorian, 300, 12);
			Actor_Face_Actor(kActorCrazylegs, kActorGrigorian, true);
			Actor_Says(kActorCrazylegs, 1010, kAnimationModeTalk);
			Actor_Face_Actor(kActorGrigorian, kActorCrazylegs, true);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorGrigorian, 310, 16);
#else
			// Grigorian is interrupted here
			Actor_Says_With_Pause(kActorGrigorian, 310, 0.0f, 16);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
			Actor_Says(kActorMcCoy, 4345, 14);
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
			Actor_Says(kActorCrazylegs, 1020, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4350, 18);
			Actor_Says(kActorCrazylegs, 1030, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4355, 19);
			Actor_Says(kActorCrazylegs, 1040, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4360, 16);
			Actor_Says(kActorMcCoy, 4365, 14);
			Actor_Says(kActorCrazylegs, 1050, kAnimationModeTalk);
			Actor_Says(kActorCrazylegs, 1060, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4370, 14);
			Actor_Says(kActorCrazylegs, 1070, kAnimationModeTalk);
			Actor_Says(kActorCrazylegs, 1080, kAnimationModeTalk);
		} else {
			Actor_Says(kActorGrigorian, 320, 13);
			if (_vm->_cutContent) {
				// In the ESP version quote 330 is muted. The previous quote (320) already contains the full quote.
				// Essentially, the full quote is not removed content in the vanilla ESP version
				// TODO: When mixing ESP voiceover with subtitles from other languages,
				//       those subtitles would have to be merged with the previous full quote (320)
				//       in order to show up in the ESP version
				if (_vm->_language != Common::ES_ESP) {
					// "Friends with access to vehicles"
					Actor_Says(kActorGrigorian, 330, kAnimationModeTalk);
				}
			}
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorGrigorian, 340, 14);
			Actor_Says(kActorGrigorian, 350, 12);
#else
			if (_vm->_language != Common::ES_ESP) {
				Actor_Says(kActorGrigorian, 340, 14);
				Actor_Says_With_Pause(kActorGrigorian, 350, 0.0f, 12);
			} else {
				// quote 350 is muted in ESP version. The quote 340 contains the full quote
				// TODO: When mixing ESP voiceover with subtitles from other languages,
				//       those subtitles would have to be merged with the previous full quote (340)
				//       in order to show up in the ESP version
				Actor_Says_With_Pause(kActorGrigorian, 340, 0.0f, 14);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 4375, 18);
		}
		break;

	case 190: // NOTE
		Actor_Says(kActorMcCoy, 4270, 18);
		Actor_Says(kActorMcCoy, 4260, kAnimationModeTalk);
		Actor_Says(kActorGrigorian, 360, 16);
		Actor_Says(kActorMcCoy, 4380, 19);
		Actor_Says(kActorMcCoy, 4385, 19);
		Actor_Says(kActorGrigorian, 370, 13);
		Actor_Says(kActorMcCoy, 4390, 19);
		Actor_Says(kActorMcCoy, 4395, 18);
		Actor_Says(kActorGrigorian, 380, 14);
		Actor_Says(kActorGrigorian, 390, 12);
		Actor_Modify_Friendliness_To_Other(kActorGrigorian, kActorMcCoy, -5);
		break;

	case 200: // VOIGT-KAMPFF
		if (_vm->_cutContent) {
			Game_Flag_Set(kFlagPS09GrigorianVKChosen);
		}
		Actor_Says(kActorMcCoy, 4265, 14);
		Actor_Says(kActorGrigorian, 400, 13);
		Actor_Says(kActorMcCoy, 4400, 13);
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorGrigorian, 410, 16);
#else
		// Grigorian is interrupted here, so pause has to be 0.0f
		Actor_Says_With_Pause(kActorGrigorian, 410, 0.0f, 16);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorMcCoy, 4405, 14);
		Actor_Says(kActorMcCoy, 4410, 15);
		Voight_Kampff_Activate(kActorGrigorian, 20);
		Actor_Modify_Friendliness_To_Other(kActorGrigorian, kActorMcCoy, -10);
		break;

	case 210: // DONE
		Actor_Says(kActorMcCoy, 8600, 18);
		Actor_Says(kActorGrigorian, 20, 15);
		break;
	}
}

} // End of namespace BladeRunner
