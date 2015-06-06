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

namespace Sherlock {

namespace Tattoo {

const char PORTRAITS[TATTOO_MAX_PEOPLE][5] = { 
	{ "HOLM" },		//  Sherlock Holmes
	{ "WATS" },		//  Dr. Watson
	{ "HUDS" },		//  Mrs. Hudson
	{ "FORB" },		//  Stanley Forbes
	{ "MYCR" },		//  Mycroft Holmes
	{ "WIGG" },		//  Wiggins
	{ "BURN" },		//  Police Constable Burns
	{ "TRIM" },		//  Augustus Trimble
	{ "DALE" },		//  Police Constable Daley
	{ "MATR" },		//  Matron
	{ "GRAC" },		//  Sister Grace
	{ "MCCA" },		//  Preston McCabe
	{ "COLL" },		//  Bob Colleran
	{ "JONA" },		//  Jonas Rigby
	{ "ROAC" },		//  Police Constable Roach
	{ "DEWA" },		//  James Dewar
	{ "JERE" },		//  Sergeant Jeremy Duncan
	{ "GREG" },		//  Inspector Gregson
	{ "LEST" },		//  Inspector Lestrade
	{ "NEED" },		//  Jesse Needhem
	{ "FLEM" },		//  Arthur Fleming
	{ "PRAT" },		//  Mr. Thomas Pratt
	{ "TILL" },		//  Mathilda (Tillie) Mason
	{ "RUSS" },		//  Adrian Russell
	{ "WHIT" },		//  Eldridge Whitney
	{ "HEPP" },		//  Hepplethwaite
	{ "HORA" },		//  Horace Silverbridge
	{ "SHER" },		//  Old Sherman
	{ "VERN" },		//  Maxwell Verner
	{ "REDD" },		//  Millicent Redding
	{ "VIRG" },		//  Virgil Silverbridge
	{ "GEOR" },		//  George O'Keeffe
	{ "LAWT" },		//  Lord Denys Lawton
	{ "JENK" },		//  Jenkins
	{ "JOCK" },		//  Jock Mahoney
	{ "BART" },		//  Bartender
	{ "LADY" },		//  Lady Cordelia Lockridge
	{ "PETT" },		//  Pettigrew
	{ "FANS" },		//  Sir Avery Fanshawe
	{ "HODG" },		//  Hodgkins
	{ "WILB" },		//  Wilbur "Birdy" Heywood
	{ "JACO" },		//  Jacob Farthington
	{ "BLED" },		//  Philip Bledsoe
	{ "FOWL" },		//  Sidney Fowler
	{ "PROF" },		//  Professor Theodore Totman
	{ "ROSE" },		//  Rose Hinchem
	{ "TALL" },		//  Tallboy
	{ "STIT" },		//  Ethlebert "Stitch" Rumsey
	{ "FREE" },		//  Charles Freedman
	{ "HEMM" },		//  Nigel Hemmings
	{ "CART" },		//  Fairfax Carter
	{ "WILH" },		//  Wilhelm II
	{ "WACH" },		//  Wachthund
	{ "WILS" },		//  Jonathan Wilson
	{ "DAVE" },		//  David Lloyd-Jones
	{ "HARG" },		//  Edward Hargrove
	{ "MORI" },		//  Professor James Moriarty
	{ "LASC" },		//  The Lascar
	{ "PARR" },		//  Parrot
	{ "SCAR" },		//  Vincent Scarrett
	{ "ALEX" },		//  Alexandra
	{ "QUEE" },		//  Queen Victoria
	{ "JOHN" },		//  John Brown
	{ "PAT1" },		//  Patient #1
	{ "PAT2" },		//  Patient #2
	{ "PATR" },		//  Patron
	{ "QUEN" },		//  Queen Victoria
	{ "WITE" },		//  Patient in White
	{ "LUSH" },		//  Lush
	{ "DRNK" },		//  Drunk
	{ "PROS" },		//  Prostitute
	{ "MUDL" },		//  Mudlark
	{ "GRIN" },		//  Grinder
	{ "BOUN" },		//  Bouncer
	{ "RATC" },		//  Agnes Ratchet
	{ "ALOY" },		//  Aloysius Ratchet
	{ "REAL" },		//  Real Estate Agent
	{ "CAND" },		//  Candy Clerk
	{ "BEAD" },		//  Beadle
	{ "PRUS" },		//  Prussian
	{ "ROWB" },		//  Mrs. Rowbottom
	{ "MSLJ" },		//  Miss Lloyd-Jones
	{ "TPAT" },		//  Tavern patron
	{ "USER" },		//  User
	{ "TOBY" },		//  Toby
	{ "STAT" },		//  Stationer
	{ "CLRK" },		//  Law Clerk
	{ "CLER" },		//  Ministry Clerk
	{ "BATH" },		//  Bather
	{ "MAID" },		//  Maid
	{ "LADF" },		//  Lady Fanshawe
	{ "SIDN" },		//  Sidney Ratchet
	{ "BOYO" },		//  Boy
	{ "PTR2" },		//  Second Patron
	{ "BRIT" },		//  Constable Brit
	{ "DROV" }		//  Wagon Driver
};

const char *const FRENCH_NAMES[TATTOO_MAX_PEOPLE] = {
	"Sherlock Holmes",
	"Dr. Watson",
	"Mme. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Sergent Burns",
	"Augustus Trimble",
	"Sergent Daley",
	"Infirmi?re chef",
	"Mme. Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Sergent Roach",
	"James Dewar",
	"Sergent Jeremy Duncan",
	"Inspecteur Gregson",
	"Inspecteur Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"M. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Serveur",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Professeur Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Wilhelm II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"Le Lascar",
	"Oiseau",
	"Vincent Scarrett",
	"Alexandra",
	"Queen Victoria",
	"John Brown",
	"Patient",
	"Patient",
	"Client",
	"Queen Victoria",
	"Patient en blanc",
	"Ivrogne",
	"Ivrogne",
	"Belle femme",
	"Mudlark",
	"Broyeur",
	"Videur",
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Immobilier",
	"Gar?on",
	"Beadle",
	"Prussian",
	"Mme. Rowbottom",
	"Mme Lloyd-Jones",
	"Tavern Client",
	"User",
	"Toby",
	"Papeterie",
	"Law Clerc",
	"Ministry Employ?",
	"Clint du thermes",
	"Bonne",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Gar?on",
	"Client",
	"Sergent Brit",
	"Wagon Driver"
};

const char *const ENGLISH_NAMES[TATTOO_MAX_PEOPLE] = {
	"Sherlock Holmes",
	"Dr. Watson",
	"Mrs. Hudson",
	"Stanley Forbes",
	"Mycroft Holmes",
	"Wiggins",
	"Police Constable Burns",
	"Augustus Trimble",
	"Police Constable Daley",
	"Matron",
	"Sister Grace",
	"Preston McCabe",
	"Bob Colleran",
	"Jonas Rigby",
	"Police Constable Roach",
	"James Dewar",
	"Sergeant Jeremy Duncan",
	"Inspector Gregson",
	"Inspector Lestrade",
	"Jesse Needhem",
	"Arthur Fleming",
	"Mr. Thomas Pratt",
	"Mathilda (Tillie) Mason",
	"Adrian Russell",
	"Eldridge Whitney",
	"Hepplethwaite",
	"Horace Silverbridge",
	"Old Sherman",
	"Maxwell Verner",
	"Millicent Redding",
	"Virgil Silverbridge",
	"George O'Keeffe",
	"Lord Denys Lawton",
	"Jenkins",
	"Jock Mahoney",
	"Bartender",
	"Lady Cordelia Lockridge",
	"Pettigrew",
	"Sir Avery Fanshawe",
	"Hodgkins",
	"Wilbur \"Birdy\" Heywood",
	"Jacob Farthington",
	"Philip Bledsoe",
	"Sidney Fowler",
	"Professor Theodore Totman",
	"Rose Hinchem",
	"Tallboy",
	"Ethlebert \"Stitch\" Rumsey",
	"Charles Freedman",
	"Nigel Hemmings",
	"Fairfax Carter",
	"Wilhelm II",
	"Wachthund",
	"Jonathan Wilson",
	"David Lloyd-Jones",
	"Edward Hargrove",
	"Misteray",
	"The Lascar",
	"Parrot",
	"Vincent Scarrett",
	"Alexandra",
	"Queen Victoria",
	"John Brown",
	"A Patient",
	"A Patient",
	"Patron",
	"Queen Victoria",
	"Patient in white",
	"Lush",
	"Drunk",
	"Prostitute",
	"Mudlark",
	"Grinder",
	"Bouncer",
	"Agnes Ratchet",
	"Aloysius Ratchet",
	"Real Estate Agent",
	"Candy Clerk",
	"Beadle",
	"Prussian",
	"Mrs. Rowbottom",
	"Miss Lloyd-Jones",
	"Tavern patron",
	"User",
	"Toby",
	"Stationer",
	"Law Clerk",
	"Ministry Clerk",
	"Bather",
	"Maid",
	"Lady Fanshawe",
	"Sidney Ratchet",
	"Boy",
	"Patron",
	"Constable Brit",
	"Wagon Driver"
};


} // End of namespace Tattoo

} // End of namespace Sherlock
