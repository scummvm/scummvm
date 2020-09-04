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

namespace Wintermute {

struct AchievementDescriptionList {
	const char *gameId;
	Common::AchievementsPlatform platform;
	const char *appId;
	const Common::AchievementDescription descriptions[64];
};

#define ACHIEVEMENT_SIMPLE_ENTRY(id, title, comment) {id, false, title, comment}
#define ACHIEVEMENT_NODESC_ENTRY(id, title) {id, false, title, ""}
#define ACHIEVEMENT_HIDDEN_ENTRY(id, title) {id, true, title, ""}
#define ACHIEVEMENTS_LISTEND {0,0,0,0}

static const AchievementDescriptionList achievementDescriptionList[] = {
	{
		"juliastars",
		Common::GALAXY_ACHIEVEMENTS,
		"48891696681534931",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_LAND", "Good morning Xenophon!", "You've managed to land on a planet."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_MIND", "Observant player", "You've obtained your first Mind'o'Matic."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_JUNGLE", "Explorer"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_HACKER", "Hacker", "You have hacked into all datapads."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_MEMORY", "Sweet memories", "You have recovered some of J.U.L.I.A.'s erased memory clusters."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_TRAVEL", "Traveller", "You have visited all the planets."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_LUDITE", "Luddite"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_SCIENTIST", "Real scientist", "You've analyzed every single object in the game."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_PLAT1", "Pacifist"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_PLAT2", "Science over all"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_PLAT3", "Apathy"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_FIRST", "First contact", "You encountered your first sentient extraterrestrial being."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_UPGRADE", "Constructor Jr.", "You built your first upgrade."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_MEGABOT", "Megabot", "You've fully upgraded Mobot."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_REPAIR", "Plumber", "You repaired the probe. The result is that you won't probably die."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR1", "Deadly Xir"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR2", "Xir Destroyer"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR3", "Xir the Invincible"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR4", "Jaeger"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_SCHI", "Dreamer"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_NIBIRU", "Artificial planet"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_FULLMIND", "Great mind", "You solved all Mind'o'Matics."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_COMPL", "Completist", "You have completed everything, the game had to offer."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_END1", "Homesick"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_END2", "Adventurous"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_TRAPPER", "Trapper"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_DECRYPT", "Cryptoanalyst"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_CREDITS", "Voyeur"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_SCAN", "Methodical", "You scanned all the planets."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_CORDES", "Unexpected visitor"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_START", "Untold: Hungry for more?"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_BLUE", "Untold: Blue solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_RED", "Untold: Red solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_PURPLE", "Untold: Purple solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_GREEN", "Untold: Green solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_YELLOW", "Untold: Yellow solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_WIRELESS", "Untold: Go wireless"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_BLINDER", "Untold: Blinder"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_AMPLIFIER", "Untold: Amplifier"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_FINISHED", "Untold: You know the story"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"juliastars",
		Common::STEAM_ACHIEVEMENTS,
		"257690",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_LAND", "Good morning Xenophon!", "You've managed to land on a planet."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_MIND", "Observant player", "You've obtained your first Mind'o'Matic."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_JUNGLE", "Explorer"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_HACKER", "Hacker", "You have hacked into all datapads."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_MEMORY", "Sweet memories", "You have recovered some of J.U.L.I.A.'s erased memory clusters."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_TRAVEL", "Traveller", "You have visited all the planets."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_LUDITE", "Luddite"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_SCIENTIST", "Real scientist", "You've analyzed every single object in the game."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_PLAT1", "Pacifist"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_PLAT2", "Science over all"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_PLAT3", "Apathy"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_FIRST", "First contact", "You encountered your first sentient extraterrestrial being."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_UPGRADE", "Constructor Jr.", "You built your first upgrade."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_MEGABOT", "Megabot", "You've fully upgraded Mobot."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_REPAIR", "Plumber", "You repaired the probe. The result is that you won't probably die."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR1", "Deadly Xir"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR2", "Xir Destroyer"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR3", "Xir the Invincible"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_XIR4", "Jaeger"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_SCHI", "Dreamer"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_NIBIRU", "Artificial planet"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_FULLMIND", "Great mind", "You solved all Mind'o'Matics."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_COMPL", "Completist", "You have completed everything, the game had to offer."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_END1", "Homesick"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_END2", "Adventurous"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_TRAPPER", "Trapper"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_DECRYPT", "Cryptoanalyst"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_CREDITS", "Voyeur"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHI_SCAN", "Methodical", "You scanned all the planets."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_CORDES", "Unexpected visitor"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_START", "Untold: Hungry for more?"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_BLUE", "Untold: Blue solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_RED", "Untold: Red solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_PURPLE", "Untold: Purple solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_GREEN", "Untold: Green solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_YELLOW", "Untold: Yellow solved"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_WIRELESS", "Untold: Go wireless"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_BLINDER", "Untold: Blinder"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_AMPLIFIER", "Untold: Amplifier"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHI_UNT_FINISHED", "Untold: You know the story"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"reversion1",
		Common::STEAM_ACHIEVEMENTS,
		"270570",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_INTRODUCCION", "Introduction", "Start a new game"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_HABLAR_CON_CHICA_ANTES_DE_DARLE_LA_FOTO", "Lady Killer", "Talk to the girl"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_SEDANTE", "Sweet Dreams"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_GUARDIA_ENCINTADO", "The Caterpillar"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_PALO_GUARDIA", "Big Stick", "Grab the baseball bat"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_INSISTENTE", "Nuisance guy"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_LLAVE_DEPOSITO", "GateKeeper"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_HACER_FUEGO", "Incendiary"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_DESMAYAR_GUARDIA_MATAFUEGOS", "Off down!"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ENGANCHA_SOGA", "Perfect escape"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_CHISTOSO", "Funny Man", "Read all the jokes in the graffiti"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_HABLAR_RATA", "The Piper", "Hypnotize a rat"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_TERMINAR_JUEGO", "The Escapist", "Escape from the hospital and finish the game"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_TIMING", "Marathon Runner", "Finish the game in less than 4 hours"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_TERMINAR_SIN_PISTAS", "The Riddle", "Finish the game without using the hint system"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_GANAR_2_VECES", "The perfect escapist", "Finish the game for a second time"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"reversion2",
		Common::STEAM_ACHIEVEMENTS,
		"281060",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_INTRODUCCION2", "Introduction", "Start a new game"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PABLO", "Mystery man"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_ESTACIONES_SUBTE", "Subway Maraude", "Take a stroll through the subway"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PUERTA_SECRETA", "Secret Door"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_BULLSEYE", "Bullseye", "Resolve the subway puzzle on your first attempt"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_MECANICO", "The Mechanic"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ELECTRISISTA", "The Electrician"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_CARPINTERO", "The Carpenter"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TOMB_RAIDER", "Tomb Raider"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_INFORMANTE", "The Informant"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PASTELITO_EXPLOSIVO", "Exploding Candy"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PASTELITO_SEDANTE", "Sleeping Candy"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_RECORDANDO", "Remembering"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ESPANTA_MOSCAS", "Flier Shoosh"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_ESTUDIANTE", "The Student", "Visit the house of knowledge"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ACOMODADOR", "Usher"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_BAJANDO_AGUA", "Lowering Water", "Find the right combination"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PUERTA_LABORATORIO", "The final door"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_FINAL", "Winner", "Finish the game"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_CONOCEDOR_SUBTE", "Subway Erudite", "You know all the stations by heart"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_LADRON", "Thief"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_PALA_FRAGIL", "Broken Shovel", "You can't dig with a broken shovel"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_TIMING2", "Marathon Runner", "Finish the game in less than 4 hours"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_TERMINAR_SIN_PISTAS2", "Riddle Guy", "Finish the game without using the hint system"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACHIEVEMENT_GANAR_2_VECES2", "Double Winner", "Finish the game for a second time"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"reversion3",
		Common::STEAM_ACHIEVEMENTS,
		"281080",
		{
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_INTRODUCCION3", "Introduction"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_HISTORIA_VICTORIA", "The orphan"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_CUPIDO", "Cupid"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_BORRACHO", "Drinking buddy"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_OSO_PELUCHE", "Teddy"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_HIT_THOSE_MOLES_2", "The fast and the furious"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_HIT_THOSE_MOLES_5", "Addict"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_RATON_GOMA", "The elephant"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_MONO_ATACANDO", "Don't feed the animals"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_MONO_ASUSTADO", "The planet of the apes"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_RESCATE_PABLO", "Rescuing Pablo"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_HOME_SWEET_HOME", "Home sweet home"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_LOCKSMITH", "Locksmith"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_FLUX_CAPACITOR", "Back to the future"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_VICTORIA_OCUPADA", "A very busy girl"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_FOTO_FLORENCIA", "Where there was fire"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_CRYSTAL_DISC", "Things to remember"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_CENTRAL_COMUNICACIONES_SIN_CREDENCIAL_CORRECTA", "Admission rights"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TANGO_CON_FLORENCIA", "To the rhythm of tango"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TURISTA", "Tourist"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TANGO01", "Presidential plane"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PUERTO_MADERO_FLORENCIA", "Until death do us part"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_CASA_ROSADA_MAIN_GATE", "The main gate"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_PARTES_ESTABILIZADOR", "The Pulse Stabilizer"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_NICOLAS_TIENE_TODO", "Back to the past"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_VIRUS_INSTALADO", "Phone home"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ATRAPADO", "This is going to leave a mark"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ESCAPAMOS", "A new hope"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ESCAPE_SERGIO", "No turning back"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_CHOCOLATE", "Condor Chocolate"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_ATRAPAMOS_SERGIO", "Evil always pays"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_SAN_MARTIN", "The Liberator"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TIMING_R3", "Marathon Runner"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TERMINAR_SIN_PISTAS_R3", "Riddle Guy"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_TERMINAR_JUEGO_R3", "Winner"),
			ACHIEVEMENT_HIDDEN_ENTRY("ACHIEVEMENT_GANAR_2_VECES_R3", "Double Winner"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"oknytt",
		Common::STEAM_ACHIEVEMENTS,
		"286320",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_1", "Awakening", "Finish chapter 1"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_2", "An extended hand", "Finish chapter 2"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_2_boss", "Into the darkness", "Escape the eyrie"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_3", "Beneath the surface", "Finish chapter 3"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_4", "A winding path", "Finish chapter 4"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_5", "Gate of promises", "Finish chapter 5"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_chapter_5_boss", "Dawn", "Escape the cavern"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_lore_library", "Folklorist", "Complete the lore library"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_grave", "A final resting place", "Find the nattramn's grave"),
			ACHIEVEMENT_SIMPLE_ENTRY("ach_mystery_carving", "Mystery carving", "It's a secret to everybody"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"sotv1",
		Common::STEAM_ACHIEVEMENTS,
		"286360",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_ZENMASTER", "Zen Master", "Survive a conversation with Anna Marano"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_LOOK_BIDET", "Italian memories", "Find a widespread sign of modern civilization"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TUTORIAL_DONE", "James Murphy, doctor", "Finish the tutorial by stitching Mario's wound"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CRI_STUFF", "The game is on", "Find the first clues about the Cristoforo investigation"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_HOLYPICS", "Harder path", "Solve a puzzle using the calendar at home"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_LEPRIGNANO", "Cyberspace Surfer", "Complete successfully your first online search"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SCROOGE", "Cheap James", "Corrupt the Capena church keeper"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_DRAPE", "Take that, dr. Jones", "Solve the most cryptic puzzle left by Cristoforo"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_DRUNKNUN", "James the sinner", "Get ready for hell by ruining sister Candida"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_MARANO_DIARY", "Attentive reader", "Face an adventure cliche: the diary!"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CIGAR", "Not afraid of pixel hunting", "Find something that belonged to the jeweler"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CARCHASE", "Lights off", "Follow the car to its destination without making mistakes"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_HOTSPOTTER", "Look, a bush!", "It's not pixel hunting if you have a magnifier..."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_NOLK27", "Mysterious password", "...it maybe makes sense in another game?"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SANTA", "Ho ho ho!", "Try entering from the chimney on December 24th"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_ELISA", "Audere Semper", "Elisa, we hold you in our hearts"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SILVIA", "Lots of space here...", "Bring a sexy lady back home"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_MATH_ACE", "Math Ace", "Get the calculation wrong twice"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_COMPLETION", "Thrilled face", "Complete Shadows on the Vatican Act I"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_FAST_GAMER", "Dialogue skipper", "Complete the adventure in less than two hours"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"sotv2",
		Common::STEAM_ACHIEVEMENTS,
		"378630",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_FARMER", "Very appropriate, you fight like a priest", "Get rid of James by using no wrong answers."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_LAZYKILLER", "Lazy Killer", "Open the car trunk as lazy people do."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_GOAULD", "Silvia the Goa'uld", "Try camouflaging your voice to trick a thug."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_RAJATHUGS", "Queen of the shadows", "Get rid of all the thugs guarding the Raja."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_MURDER", "Fifth Commandment", "Commit murder."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CROWBAR", "Now I am an adventure hero", "An adventure game with no crowbars? Make one!"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CHAMOMILE", "Chamomile needed", "Hit someone five times in a row just because you can."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_LADIESFIRST", "Ladies First", "Can't get enough of playing with Silvia, huh?"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TRUEDETECTIVE", "True Detective", "Complete James actions first."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_ACTIONADDICT", "Action Addict", "Complete Silvia actions first."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_COCKTAIL", "Cocktail Master", "Get the anti-hangover preparation right on first try"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_OLDSCHOOL", "Old School Adventurer", "Playing with pen and paper on your side?"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SEDUCTION", "Seduction Failure", "That's not going to work with him."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_MARIKA", "No shortcuts", "We understand you, but better avoid violence if possible!"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_DAMSEL", "To the rescue!", "Find out where Silvia is taken captive."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_PIN", "Desperate attempt", "Sorry, this is not your tv's default PIN."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_COPYPASTER", "Copy Paster", "Nice try, but wrong!"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TEAMPLAY", "Team Play", "Wasn't \"team\" the best path of the three?"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TOMBRAIDERS", "Tomb Raiders", "Complete Shadows on the Vatican Act II."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_DIALOGUELOVER", "Dialogue Lover", "Complete the adventure with dialogue skipping disabled."),
			ACHIEVEMENT_HIDDEN_ENTRY("ACH_CHEATER", "Cheater"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"thelostcrowngha",
		Common::STEAM_ACHIEVEMENTS,
		"291710",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_HEDGE_WYTCH", "Hedge Wych", "Pick all available plants and flowers in the country lane."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_GOOD_LISTENER", "Good Listener", "Listen to the story on the Harbour Cottage telephone every day or night."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_FEARLESS", "Ghostbuster", "Vanquish the train tracks ghosts on the first attempt."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_GRAVE_DWELLER", "Grave Dweller", "Defeat the Darkness on the first attempt at Northfield."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CAIRANS_FRIEND", "My Porcine Friend", "Feed Cairan the pig over 20 times."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SWOT", "Swot", "Thoroughly explore the Saxton Museum."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_FILM_BUFF", "Film Buff", "Watch the films in Saxton Museum in their entirety."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_BOOKWORM", "Bookworm", "Read the books in Saxton Museum's Library and Celtic Corner."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_DETECTIVE", "True Detective", "Study all of the photographs on the wall in the Nightmare Room."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_LOBSTER_POTTY", "Lobster Potty", "Visit the lobster in The Bear."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CAT_WATCHER", "Cat Watcher", "Provide Cat Watch with the names of 6 Saxton citizens."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CRIME_LINE", "Saxton Sherlock", "Successfully identify the Saxton Skelton."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_PUNCH_JUDY_FAN", "That's the way to do it", "Keep watching the Punch and Judy show on Saxton Shore."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_RETRO_HORROR", "Retro Horror", "Travel on the Ghost Train 5 times."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_LUCKY_DIPPER", "Lucky Dipper", "See all items in the Lucky Dip."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_ENVIRONMENTALIST", "Environmentalist", "Photograph the rare Natterjack Toad."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TWITCHER", "Bird Watcher", "Successfully photograph the Heron in Saxon Fens."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CAT_SNAPPER", "Cat Snapper", "Photograph Mr Tibbs at Ulcombe."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_WE_THREE_KINGS", "We Three Kings", "Successfully photograph the statue of the three Saxon Kings."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SAXTON_SNAPPER", "Paparazzi", "Win 1st place in the Saxton Snappers photography competition."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_EVP_MASTER", "Spirit Voices", "Collected all EVP's in the game."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_GHOST_PHOTOGRAPHER", "Phantom Photomaster", "Capture all possible Ghost Photos in the game."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_PEOPLE_PERSON", "Name Dropper", "Name all the figures seen in Ganwulfs tomb."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_ENVIRONMETER", "Ghosthunter", "Detect over 20 paranormal events in Harbour Cottage on the Environmeter."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TO_THE_FUTURE", "To The Future", "Complete the Game."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_WARNING_CURIOUS", "A Warning to the Curious", "Listen to All of Hardachre's dire warnings."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_MIRROR_MIRROR", "Mirror Mirror", "Successfully call up the ghost in the Mirror."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_IN_DEEP", "Holistic Detective", "Study the documents and photos stolen from Hadden."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CHECK_IN", "Home Sweet Home", "Find new accommodation in Saxton."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SPY_GLASS", "Spy Glass", "Use the Telescope on May Day to see All views from the Little Lighthouse."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_ISOLATION", "Intrinsic Isolation", "Call out to sea more than once in the Fens."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_UNDERCURRENTS", "Undying Undercurrents", "Fix the warning sign near the Fenland Eye."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_CHATTER_BOX", "Chatter Box", "Fully talk with Nanny Noah on Saxton Shore."),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_NARCISSIST", "Narcissist", "Look in the Harbour Cottage mirror 5 times on Day 1"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"corrosion",
		Common::STEAM_ACHIEVEMENTS,
		"349140",
		{
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_0", "Cadet"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_1", "Recruit Officer"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_2", "Police Officer"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_3", "Investigator"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_4", "Detective 3rd Grade"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_5", "Detective 2nd Grade"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_6", "Detective 1st Grade"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_7", "Specialist"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_8", "Sergeant"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_9", "Lieutenant"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_10", "Captain"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_11", "Deputy Inspector"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_12", "Inspector"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_13", "Deputy Chief"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_14", "Assistant Chief"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_15", "Chief"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_16", "Deputy Commissioner"),
			ACHIEVEMENT_NODESC_ENTRY("corrosion_achievement_17", "Commissioner"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"alphapolaris",
		Common::STEAM_ACHIEVEMENTS,
		"405780",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_POLARBEAR", "Vetenarian", "Treat a polar bear"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_DAY1", "A Day in the Arctic", "Survive the first day"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SANDWICH", "Munchies", "Observe a nourishing treat"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_NOVA_TALK", "A Shoulder to Lean on", "Check on Nova in the first evening"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_THE_END", "The End", "Finish the game"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_BRA", "True Gentleman", "Discover something intimate"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_BOMBE_PERFECT", "Chef de Cuisine", "Create a perfect Bombe Alaska"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_BOMBE_OK", "Sous-Chef", "Create an OK Bombe Alaska"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_BOMBE_BAD", "Butcher", "Create a lousy Bombe Alaska"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_BOMBE_GASOLINE", "Chef le Octane", "Create a Bombe Alaska with gasoline"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_MCGUYVER", "You are not MacGyver", "Use the multitool way too much"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_SHOOTBLANKS", "Shootin' Blanks", "Jokingly try to shoot Tully"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_FRIEND_OF_THE_YEAR", "Friend of the Year", "Try to shoot Tully"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_TRIANGULATE", "Everyday I'm Calculatin'", "Successfully use triangulation on the first try"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_WEATHERMAN", "Weatherman", "Always check the temperature first thing in the morning"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_POTTYMOUTH", "Potty Mouth", "Use parser impropriately"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_GENERATOR", "Very Strong With Machines", "Start the generator on the first try"),
			ACHIEVEMENT_SIMPLE_ENTRY("ACH_PARSER", "Parser Hero", "Make no mistakes in any of the parser puzzles"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"drdoylemotch",
		Common::STEAM_ACHIEVEMENTS,
		"574420",
		{
			ACHIEVEMENT_NODESC_ENTRY("ACT_1", "Act 1 Completed"),
			ACHIEVEMENT_NODESC_ENTRY("ACT_2", "Act 2 Completed"),
			ACHIEVEMENT_NODESC_ENTRY("ACT_3", "Act 3 Completed"),
			ACHIEVEMENT_NODESC_ENTRY("ACT_4", "Act 4 Completed"),
			ACHIEVEMENT_NODESC_ENTRY("ACT_5", "Act 5 Completed"),
			ACHIEVEMENT_NODESC_ENTRY("CHAPMANS_LODGINGS", "The Missing Tenant"),
			ACHIEVEMENT_NODESC_ENTRY("CONSTABULARY", "The Borough's Finest"),
			ACHIEVEMENT_NODESC_ENTRY("CRIME_SCENE", "An Ugly Beating"),
			ACHIEVEMENT_NODESC_ENTRY("FEATHERSTONE", "A Walk To The Shops"),
			ACHIEVEMENT_NODESC_ENTRY("GRAND_HOTEL", "Luxurious Stay"),
			ACHIEVEMENT_NODESC_ENTRY("HAT_WORKSHOP", "Revelations"),
			ACHIEVEMENT_NODESC_ENTRY("HOSPITAL", "Home Sweet Home"),
			ACHIEVEMENT_NODESC_ENTRY("PEMBERTON_EMPORIUM", "\"Fair\" Exchanges"),
			ACHIEVEMENT_NODESC_ENTRY("PRESCOTT_LANE", "Breaking & Entering"),
			ACHIEVEMENT_NODESC_ENTRY("THE_MARQUIS", "Dinner Is Served"),
			ACHIEVEMENT_NODESC_ENTRY("WHITEHAVEN_HALL", "A Grand E\"state\" Of Affairs"),
			ACHIEVEMENT_NODESC_ENTRY("DUBOIS_SMALLTALK", "The Man In Black"),
			ACHIEVEMENT_NODESC_ENTRY("FEATHERSTONE_SMALLTALK", "Suited With Enthusiasm"),
			ACHIEVEMENT_NODESC_ENTRY("HOBBS_SMALLTALK", "Gardening & Nasty Quarrels"),
			ACHIEVEMENT_NODESC_ENTRY("INSPECTOR_SMALLTALK", "Friends In High Places"),
			ACHIEVEMENT_NODESC_ENTRY("SHAW_SMALLTALK", "The Timid Suspect"),
			ACHIEVEMENT_NODESC_ENTRY("LAWSON_SMALLTALK", "A Lady's Whims"),
			ACHIEVEMENT_NODESC_ENTRY("MAID_SMALLTALK", "A Maid's Sadness"),
			ACHIEVEMENT_NODESC_ENTRY("GIBBS_SMALLTALK", "The Landlady's Avarice"),
			ACHIEVEMENT_NODESC_ENTRY("PIKE_SMALLTALK", "An Obstinate 'Old Dear'"),
			ACHIEVEMENT_NODESC_ENTRY("PEMBERTON_SMALLTALK", "Shady Dealings"),
			ACHIEVEMENT_NODESC_ENTRY("ROBERT_SMALLTALK", "Eager To Serve"),
			ACHIEVEMENT_NODESC_ENTRY("1919_POISONING_CASE", "Solved The 1919 Case"),
			ACHIEVEMENT_NODESC_ENTRY("ASHBERG_HEIST", "Solved The Ashberg Heist"),
			ACHIEVEMENT_NODESC_ENTRY("CLOCHE_HAT", "Solved The Mystery of the Cloche Hat"),
			ACHIEVEMENT_NODESC_ENTRY("BODYSLASHER", "Bodyslasher"),
			ACHIEVEMENT_NODESC_ENTRY("CHEMISTRY_101", "Chemistry 101"),
			ACHIEVEMENT_NODESC_ENTRY("BOOKWORM", "Bookworm"),
			ACHIEVEMENT_NODESC_ENTRY("TABLE_FOR_ONE", "Table for one"),
			ACHIEVEMENT_NODESC_ENTRY("COUPLES_CONSULTANT", "Couples consultant"),
			ACHIEVEMENTS_LISTEND
		}
	},

	{
		"erinmyers",
		Common::STEAM_ACHIEVEMENTS,
		"1064660",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("erin_myers_achieve_art", "Not An Art Lover", "More important things to be doing."),
			ACHIEVEMENT_SIMPLE_ENTRY("erin_myers_achieve_chalk", "Chalk It Up To Experience", "A strange obsession with the chalkboard."),
			ACHIEVEMENT_SIMPLE_ENTRY("erin_myers_achieve_coffee", "Coffee Connoisseur", "Coffee is nice. But not that coffee."),
			ACHIEVEMENT_SIMPLE_ENTRY("erin_myers_achieve_hammer", "Hammering The Point", "Don't break down."),
			ACHIEVEMENT_SIMPLE_ENTRY("erin_myers_achieve_wait", "Try Try Again", "Don't give up."),
			ACHIEVEMENTS_LISTEND
		}
	},
	
	{0, Common::UNK_ACHIEVEMENTS, 0, {ACHIEVEMENTS_LISTEND}}
};

} // End of namespace Wintermute

#undef ACHIEVEMENT_SIMPLE_ENTRY
#undef ACHIEVEMENT_NODESC_ENTRY
#undef ACHIEVEMENT_HIDDEN_ENTRY
#undef ACHIEVEMENTS_LISTEND
