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

#include "sherlock/tattoo/tattoo_resources.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"

namespace Sherlock {

namespace Tattoo {

const PeopleData PEOPLE_DATA[TATTOO_MAX_PEOPLE] = {
	{ "HOLM", kFixedText_People_SherlockHolmes },
	{ "WATS", kFixedText_People_DrWatson },
	{ "HUDS", kFixedText_People_MrsHudson },
	{ "FORB", kFixedText_People_StanleyForbes },
	{ "MYCR", kFixedText_People_MycroftHolmes },
	{ "WIGG", kFixedText_People_Wiggins },
	{ "BURN", kFixedText_People_PoliceConstableBurns },
	{ "TRIM", kFixedText_People_AugustusTrimble },
	{ "DALE", kFixedText_People_PoliceConstableDaley },
	{ "MATR", kFixedText_People_Matron },
	{ "GRAC", kFixedText_People_SisterGrace },
	{ "MCCA", kFixedText_People_PrestonMcCabe },
	{ "COLL", kFixedText_People_BobColleran },
	{ "JONA", kFixedText_People_JonasRigby },
	{ "ROAC", kFixedText_People_PoliceConstableRoach },
	{ "DEWA", kFixedText_People_JamesDewar },
	{ "JERE", kFixedText_People_SergeantJeremyDuncan },
	{ "GREG", kFixedText_People_InspectorGregson },
	{ "LEST", kFixedText_People_InspectorLestrade },
	{ "NEED", kFixedText_People_JesseNeedhem },
	{ "FLEM", kFixedText_People_ArthurFleming },
	{ "PRAT", kFixedText_People_MrThomasPratt },
	{ "TILL", kFixedText_People_MathildaTillieMason },
	{ "RUSS", kFixedText_People_AdrianRussell },
	{ "WHIT", kFixedText_People_EldridgeWhitney },
	{ "HEPP", kFixedText_People_Hepplethwaite },
	{ "HORA", kFixedText_People_HoraceSilverbridge },
	{ "SHER", kFixedText_People_OldSherman },
	{ "VERN", kFixedText_People_MaxwellVerner },
	{ "REDD", kFixedText_People_MillicentRedding },
	{ "VIRG", kFixedText_People_VirgilSilverbridge },
	{ "GEOR", kFixedText_People_GeorgeOKeeffe },
	{ "LAWT", kFixedText_People_LordDenysLawton },
	{ "JENK", kFixedText_People_Jenkins },
	{ "JOCK", kFixedText_People_JockMahoney },
	{ "BART", kFixedText_People_Bartender },
	{ "LADY", kFixedText_People_LadyCordeliaLockridge },
	{ "PETT", kFixedText_People_Pettigrew },
	{ "FANS", kFixedText_People_SirAveryFanshawe },
	{ "HODG", kFixedText_People_Hodgkins },
	{ "WILB", kFixedText_People_WilburBirdyHeywood },
	{ "JACO", kFixedText_People_JacobFarthington },
	{ "BLED", kFixedText_People_PhilipBledsoe },
	{ "FOWL", kFixedText_People_SidneyFowler },
	{ "PROF", kFixedText_People_ProfessorTheodoreTotman },
	{ "ROSE", kFixedText_People_RoseHinchem },
	{ "TALL", kFixedText_People_Tallboy },
	{ "STIT", kFixedText_People_EthlebertStitchRumsey },
	{ "FREE", kFixedText_People_CharlesFreedman },
	{ "HEMM", kFixedText_People_NigelHemmings },
	{ "CART", kFixedText_People_FairfaxCarter },
	{ "WILH", kFixedText_People_WilhelmII },
	{ "WACH", kFixedText_People_Wachthund },
	{ "WILS", kFixedText_People_JonathanWilson },
	{ "DAVE", kFixedText_People_DavidLloydJones },
	{ "HARG", kFixedText_People_EdwardHargrove },
	{ "MORI", kFixedText_People_Misteray },
	{ "LASC", kFixedText_People_TheLascar },
	{ "PARR", kFixedText_People_Parrot },
	{ "SCAR", kFixedText_People_VincentScarrett },
	{ "ALEX", kFixedText_People_Alexandra },
	{ "QUEE", kFixedText_People_QueenVictoria },
	{ "JOHN", kFixedText_People_JohnBrown },
	{ "PAT1", kFixedText_People_APatient1 },
	{ "PAT2", kFixedText_People_APatient2 },
	{ "PATR", kFixedText_People_Patron },
	{ "QUEN", kFixedText_People_QueenVictoria },
	{ "WITE", kFixedText_People_PatientInWhite },
	{ "LUSH", kFixedText_People_Lush },
	{ "DRNK", kFixedText_People_Drunk },
	{ "PROS", kFixedText_People_Prostitute },
	{ "MUDL", kFixedText_People_Mudlark },
	{ "GRIN", kFixedText_People_Grinder },
	{ "BOUN", kFixedText_People_Bouncer },
	{ "RATC", kFixedText_People_AgnesRatchet },
	{ "ALOY", kFixedText_People_AloysiusRatchet },
	{ "REAL", kFixedText_People_RealEstateAgent },
	{ "CAND", kFixedText_People_CandyClerk },
	{ "BEAD", kFixedText_People_Beadle },
	{ "PRUS", kFixedText_People_Prussian },
	{ "ROWB", kFixedText_People_MrsRowbottom },
	{ "MSLJ", kFixedText_People_MissLloydJones },
	{ "TPAT", kFixedText_People_TavernPatron },
	{ "USER", kFixedText_People_User },
	{ "TOBY", kFixedText_People_Toby },
	{ "STAT", kFixedText_People_Stationer },
	{ "CLRK", kFixedText_People_LawClerk },
	{ "CLER", kFixedText_People_MinistryClerk },
	{ "BATH", kFixedText_People_Bather },
	{ "MAID", kFixedText_People_Maid },
	{ "LADF", kFixedText_People_LadyFanshawe },
	{ "SIDN", kFixedText_People_SidneyRatchet },
	{ "BOYO", kFixedText_People_Boy },
	{ "PTR2", kFixedText_People_Patron2 },
	{ "BRIT", kFixedText_People_ConstableBrit },
	{ "DROV", kFixedText_People_WagonDriver }
};

} // End of namespace Tattoo

} // End of namespace Sherlock
