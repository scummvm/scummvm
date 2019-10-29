#include "common/scummsys.h"

// In this file, the TYPO tag is used when a typo is fixed in a piece of text, or when
// text is changed to match the audio.

namespace StarTrek {

extern const char *const g_gameStrings[] = {
	NULL,
	"",
	"Dialog error",
	"Animation error",

	"Capt. Kirk",
	"Mr. Spock",
	"Dr. McCoy",
	"Lt. Uhura",
	"Mr. Scott",
	"Mr. Sulu",
	"Mr. Chekov",

	"Ensign Everts",
	"Prel. Angiven",
	"Sign",
	"Klingon",
	"Brother Kandrey",
	"Brother Stephen",
	"Brother Chub",
	"Brother Roberts",
	"Brother Grisnash",
	"Nauian",
	"Ship's Computer",

	"Lt. Christensen",
	"Crewman Simpson",
	"Elasi Guard",
	"Elasi Clansman",
	"Elasi Cereth",
	"Masada Crewman",

	"Lt. Ferris",
	"Computer",
	"Dr. Marcus",
	"Dr. Cheever",
	"Preax",

	"Lt. Buchert",
	"Harry Mudd",

	"Lt. Stragey",
	"Quetzecoatl",
	"Tlaoxac",

	"Ensign Bennie",
	"Vlict",
	"Klingon Guard",
	"Entity",
	"Bialbi",
	"A Voice Speaks",
	"Light of War",
	"Light of Knowledge",
	"Light of Travel",

	"Ensign Mosher",

	"Ensign Kije",
	"*Capt. Patterson*",
	"*Commander Andrade*",
	"Elasi Captain",
	"Elasi Weapons Master",
	"Elasi Crewman",
	"Brittany Marata",

	"#BRID\\BRIDU146#Nothing to report, Captain.",

	"#GENE\\G_014#This person's dead, Jim.",
	"#GENE\\G_024#Fascinating.",
	"#GENE\\G_043#Ouch! Watch it with that thing!",

	"#GENE\\GENER004#Game Over",

	"#COMP\\COMPA180#Orbit Stabilized. Warning Cancelled.",
	"#COMP\\COMPA185#Please select an impedence.",
	"#COMP\\COMPA186#Ammonia",
	"#COMP\\COMPA187#Di-hydrogen Oxide",
	"#COMP\\COMPA189#Nitrous Oxide",
	"#COMP\\COMPA190#Oroborus Virus",
	"#COMP\\COMPA191#Exit Database",
	"#COMP\\COMPA193#TLTDH Gas",
	"#COMP\\COMPU181#Warning. Orbital Decay is now irreversible. Abandon Ship. Abandon Ship.",
	"#COMP\\COMPU182#Warning. Orbital decay detected. Warning.",
	"#COMP\\COMPU186#Ammonia: a colorless pungent compound NH3, a common byproduct of metabolism in carbon-based lifeforms. Liquid or gaseous at 1 atm. Compounds widely used in agricultural, medical and industrial applications.",
	"#COMP\\COMPU187#Di-hydrogen Oxide: a colorless, tasteless, nonflammable compound HO- water. Liquid, solid, or gaseous at 1 atm. Temperature scales defined in most humanoid cultures by state-shift of pure water.",
	"#COMP\\COMPU188#Dr. Marcus' Log, Supplemental: Continued testing of the Oroborus Virus confirms its potential harmful effects on Romulan/Vulcan humanoids. As soon as the spill has been cleaned up, we will sterilize both labs and purge the circulation system to make sure no traces of the virus will remain viable. Then all research files must be erased -- we must leave no chance that this terrible accident will ever be repeated.",
	"#COMP\\COMPU189#Nitrous Oxide: a colorless nonflammable compound NO, gaseous at 1 atm and temperatures within the human norm. Early anesthetic among humans, in whom it produces laughter, feelings of exhiliration, euphoria; sometimes leading to unconsciousness.",
	"#COMP\\COMPU190#Oroborus virus: Atypical growth patterns for L-Type virus. Opportunistic pneumococcal mimic affecting Romulo-Vulcan genotype. Etiology: contact or airborne vector with alveoli microcollapse involvement immediately upon exposure. Tissue dehydration follows pneumal flooding. Mortality: 0.6 in 1.0 stardates, 1.0 within 2.0 stardates Treatment: none. Named for the mythic worldsnake that swallows its own tail.",
	"#COMP\\COMPU192#Please select subject file...",
	"#COMP\\COMPU193#TLTDH gas: The chemical compound tantalum bi-lithium thalo-dihydroxide. Colorless, odorless, nonflammable gaseous at 1 atm and temperatures within the human norm. Early anesthetic among Vulcans and Romulans, in whom it produces laughter, feelings of exhiliration, euphoria; sometimes leading to unconsciousness. In post-industrial/pre-spaceflight era, a social problem evolved when crude TLTDH became popular to \"cook up\" from non-conductive tantalo-lithial compounds commonly used as electrical insulation.",

	"#DEM0\\DEM0N009#Spock raises an eyebrow.",	// kept because it's used as an enhancement in mudd1
	"All mining equipment use this road.",
	"#DEM3\\DEM3_019#He's dead, Jim...",
	"#DEM3\\DEM3_A32#Aieeee!",

	"#TUG2\\TUG2J003#I recommend extreme caution, Captain. We must be ready to expect anything.",
	"Snip...snip...snip.",

	"#LOV3\\LOV3NA08#With a hiss, the Romulan Laughing Gas billows down the vent. Things are strangely quiet below.",
	"#LOV3\\LOV3NA09#With a hiss, the Romulan Laughing Gas billows down the vent. You hear the muffled sounds through the vent of hearty Romulan laughter, followed by the dull thud of bodies hitting the deck.",
	"#LOV3\\LOV3NA20#This is a service access panel, permitting used-up or worn materials to be replaced.",
	"#LOV3\\LOV3NA21#This is a vent shaft that leads to the lower level.",
	"#LOV3\\LOV3NA22#This is an engineering access panel, allowing repairs to be made to the interior wiring in the equipment.",
	"#LOV3\\LOV3NA23#This is the engineering center for the ARK7 space station.",
	"#LOV3\\LOV3NJ32#You attach the antigrav unit to the gas tank. It can be moved freely.",
	"(Raises an eyebrow)", // NOTE: no corresponding audio

	"#MUD0\\MUD0_018#Life support down to 25%, Captain.",
	"#MUD0\\MUD0_019#Life support down to 50%, Captain.",
	"#MUD0\\MUD0_020#Life support down to 75%, Captain.",
	"#MUD1\\MUD1N014#This is much too heavy to lift.",
	"#MUD2\\MUD2_040#You look troubled, Captain.",
	"#MUD4\\MUD4_018#Kirk to Enterprise ... Kirk to Enterprise.", // TYPO: used in MUD4 and LOVE mission; the text was different in LOVE, not matching with audio.
	"#MUD4\\MUD4_019#Later, Mr. Scott.",
	"#MUD4\\MUD4_023#No, I need to have a word with Harry Mudd before we go.",
	"#MUD4\\MUD4_A29#Very well, bring us home, Mr. Scott.",
	
	"#FEA1\\FEA1_A46#There is not enough support, Captain.",

	"#TRI1\\TRI1_J00#Why do I have the feeling that I'm about to have a bad day?",
	"#TRI1\\TRI1U080#Affirmative, sir.",
	"Zzzt! Sptttz! Zzzt! Tttt!",
	"Clunk.",
	"#SFX\\QUIET#Zzzzzzzzzzzmmmm.",
	"#TRI4\\TRI4_076#You humans have an excellent imagination, but a poor grasp of reality.",

	"#SIN3\\SIN3_008#All readings are normal. The structure seems to be protecting us from the cosmic rays.",
	"#SIN3\\SIN3_012#Can't say I like the decor.",
	"Laser Setting: 001",
	"Laser Setting: 010",
	"Laser Setting: 100",
	"cancel",
	"#SIN4\\SIN4U83B#Captain, we'll help you all we can.",

	"#VEN0\\VEN0_011#Yes, Scotty. Just don't take too long. Kirk out.",
	"#VEN0\\VEN0_016#They're dead, Jim. All of them.", // TYPO (for only some rooms this text appears in)
	"#VEN0\\VEN0N016#This man is dead.",
	"#VEN1\\VEN1_004#I'd like to meet the people responsible for this and give them a piece of my mind.",
	"#VEN2\\VEN2_028#Yes, Uhura, Just wanted a status report. Kirk out.",
	"#VEN2\\VEN2_066#They are jamming all frequencies, Captain. We will not be able to contact the Enterprise.",
	"#VEN2\\VEN2_098#What are you stalling for? Lower your shields.",
	"#VEN2\\VEN2_SHI#Shields",
	"#VEN2\\VEN2_TRA#Transporter Circuits",
	"#VEN2\\VEN2_WEA#Weapons",
	"#VEN2\\VEN2U093#Enterprise here. We're still tracking the distress call. Are you all right, Sir?",
	"Hail Elasi",
	"Hail Enterprise",
	"cancel",
	"#VEN4\\VEN4_016#There's nothing more I can do.", // TYPO (in some rooms, this has "Jim..." at the start of the text, though not in VENG4 itself)
	"#VEN4\\VEN4_017#I believe nothing needs to be said, Captain.",
	"#VEN4\\VEN4N007#The woman in this sickbed is alive, barely.",
	"#VEN4\\VEN4N010#This is an empty hypodermic injector.",
	"#VEN4\\VEN4N014#Yet another crewman of the Republic whose life was unfairly abbreviated.",
	"#VEN5\\VEN5_R19#Is there a question, Captain?",
	"#VEN5\\VEN5_R20#I will monitor the situation, Mr. Spock. I can use the distraction.",
	"#VEN5\\VEN5N002#A resolute Mr. Spock.",
	"#VEN5\\VEN5N004#Dr. McCoy avoids the glances of his crewmates.",
	"#VEN5\\VEN5N007#This door leads to main engineering.",

	"#sfx\\spokcoff#cough... cough...",
	"#SFX\\STATICU1#Ent... neu ... trans...",
};

} // End of namespace StarTrek
