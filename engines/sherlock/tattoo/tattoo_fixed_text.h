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

#ifndef SHERLOCK_TATTOO_FIXED_TEXT_H
#define SHERLOCK_TATTOO_FIXED_TEXT_H

#include "sherlock/fixed_text.h"

namespace Sherlock {

namespace Tattoo {

enum FixedTextId {
	kFixedText_Inv1,
	kFixedText_InvDesc1,
	kFixedText_Inv2,
	kFixedText_InvDesc2,
	kFixedText_Inv3,
	kFixedText_InvDesc3,
	kFixedText_Inv4,
	kFixedText_InvDesc4,
	kFixedText_Inv5,
	kFixedText_InvDesc5,
	kFixedText_Inv6,
	kFixedText_InvDesc6,
	kFixedText_Inv7,
	kFixedText_InvDesc7,
	kFixedText_Inv8,
	kFixedText_InvDesc8,
	kFixedText_Open,
	kFixedText_Look,
	kFixedText_Talk,
	kFixedText_Use,
	kFixedText_Journal,
	kFixedText_Inventory,
	kFixedText_Options,
	kFixedText_Solve,
	kFixedText_With,
	kFixedText_NoEffect,
	kFixedText_NothingToSay,
	kFixedText_PickedUp,

	kFixedText_Page,
	kFixedText_CloseJournal,
	kFixedText_SearchJournal,
	kFixedText_SaveJournal,
	kFixedText_AbortSearch,
	kFixedText_SearchBackwards,
	kFixedText_SearchForwards,
	kFixedText_TextNotFound,

	kFixedText_DartsPlayerHolmes,
	kFixedText_DartsPlayerJock,
	kFixedText_DartsBull,
	kFixedText_DartsCurrentRound,
	kFixedText_DartsCurrentTotalPoints,
	kFixedText_DartsCurrentDart,
	kFixedText_DartsStartPressKey1,
	kFixedText_DartsStartPressKey2,
	kFixedText_DartsPressKey,
	kFixedText_DartsGameOver,
	kFixedText_DartsBusted,
	kFixedText_DartsWins,
	kFixedText_DartsScoredPoint,
	kFixedText_DartsScoredPoints,
	kFixedText_DartsHitSingle,
	kFixedText_DartsHitDouble,
	kFixedText_DartsHitTriple,
	kFixedText_DartsHitSingleBullseye,
	kFixedText_DartsHitDoubleBullseye,
	kFixedText_DartsHitTripleBullseye,

	kFixedText_Apply,
	kFixedText_Water,
	kFixedText_Heat,
	kFixedText_LoadGame,
	kFixedText_SaveGame,
	kFixedText_Music,
	kFixedText_SoundEffects,
	kFixedText_Voices,
	kFixedText_TextWindows,
	kFixedText_TransparentMenus,
	kFixedText_ChangeFont,
	kFixedText_Off,
	kFixedText_On,
	kFixedText_Quit,
	kFixedText_AreYouSureYou,
	kFixedText_WishToQuit,
	kFixedText_Yes,
	kFixedText_No,
	kFixedText_EnterPassword,
	kFixedText_CorrectPassword,
	kFixedText_WatsonsJournal,
	kFixedText_JournalSaved,
	// SH2: People names
	kFixedText_People_SherlockHolmes,
	kFixedText_People_DrWatson,
	kFixedText_People_MrsHudson,
	kFixedText_People_StanleyForbes,
	kFixedText_People_MycroftHolmes,
	kFixedText_People_Wiggins,
	kFixedText_People_PoliceConstableBurns,
	kFixedText_People_AugustusTrimble,
	kFixedText_People_PoliceConstableDaley,
	kFixedText_People_Matron,
	kFixedText_People_SisterGrace,
	kFixedText_People_PrestonMcCabe,
	kFixedText_People_BobColleran,
	kFixedText_People_JonasRigby,
	kFixedText_People_PoliceConstableRoach,
	kFixedText_People_JamesDewar,
	kFixedText_People_SergeantJeremyDuncan,
	kFixedText_People_InspectorGregson,
	kFixedText_People_InspectorLestrade,
	kFixedText_People_JesseNeedhem,
	kFixedText_People_ArthurFleming,
	kFixedText_People_MrThomasPratt,
	kFixedText_People_MathildaTillieMason,
	kFixedText_People_AdrianRussell,
	kFixedText_People_EldridgeWhitney,
	kFixedText_People_Hepplethwaite,
	kFixedText_People_HoraceSilverbridge,
	kFixedText_People_OldSherman,
	kFixedText_People_MaxwellVerner,
	kFixedText_People_MillicentRedding,
	kFixedText_People_VirgilSilverbridge,
	kFixedText_People_GeorgeOKeeffe,
	kFixedText_People_LordDenysLawton,
	kFixedText_People_Jenkins,
	kFixedText_People_JockMahoney,
	kFixedText_People_Bartender,
	kFixedText_People_LadyCordeliaLockridge,
	kFixedText_People_Pettigrew,
	kFixedText_People_SirAveryFanshawe,
	kFixedText_People_Hodgkins,
	kFixedText_People_WilburBirdyHeywood,
	kFixedText_People_JacobFarthington,
	kFixedText_People_PhilipBledsoe,
	kFixedText_People_SidneyFowler,
	kFixedText_People_ProfessorTheodoreTotman,
	kFixedText_People_RoseHinchem,
	kFixedText_People_Tallboy,
	kFixedText_People_EthlebertStitchRumsey,
	kFixedText_People_CharlesFreedman,
	kFixedText_People_NigelHemmings,
	kFixedText_People_FairfaxCarter,
	kFixedText_People_WilhelmII,
	kFixedText_People_Wachthund,
	kFixedText_People_JonathanWilson,
	kFixedText_People_DavidLloydJones,
	kFixedText_People_EdwardHargrove,
	kFixedText_People_Misteray,
	kFixedText_People_TheLascar,
	kFixedText_People_Parrot,
	kFixedText_People_VincentScarrett,
	kFixedText_People_Alexandra,
	kFixedText_People_QueenVictoria,
	kFixedText_People_JohnBrown,
	kFixedText_People_APatient1,
	kFixedText_People_APatient2,
	kFixedText_People_Patron,
	kFixedText_People_QueenVictoria2,
	kFixedText_People_PatientInWhite,
	kFixedText_People_Lush,
	kFixedText_People_Drunk,
	kFixedText_People_Prostitute,
	kFixedText_People_Mudlark,
	kFixedText_People_Grinder,
	kFixedText_People_Bouncer,
	kFixedText_People_AgnesRatchet,
	kFixedText_People_AloysiusRatchet,
	kFixedText_People_RealEstateAgent,
	kFixedText_People_CandyClerk,
	kFixedText_People_Beadle,
	kFixedText_People_Prussian,
	kFixedText_People_MrsRowbottom,
	kFixedText_People_MissLloydJones,
	kFixedText_People_TavernPatron,
	kFixedText_People_User,
	kFixedText_People_Toby,
	kFixedText_People_Stationer,
	kFixedText_People_LawClerk,
	kFixedText_People_MinistryClerk,
	kFixedText_People_Bather,
	kFixedText_People_Maid,
	kFixedText_People_LadyFanshawe,
	kFixedText_People_SidneyRatchet,
	kFixedText_People_Boy,
	kFixedText_People_Patron2,
	kFixedText_People_ConstableBrit,
	kFixedText_People_WagonDriver
};

struct FixedTextLanguageEntry {
	Common::Language language;
	const char *const *fixedTextArray;
};

class TattooFixedText: public FixedText {
private:
	const FixedTextLanguageEntry *_curLanguageEntry;
public:
	TattooFixedText(SherlockEngine *vm);
	~TattooFixedText() override {}

	/**
	 * Gets text
	 */
	const char *getText(int fixedTextId) override;

	/**
	 * Get action message
	 */
	const Common::String getActionMessage(FixedTextActionId actionId, int messageIndex) override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
