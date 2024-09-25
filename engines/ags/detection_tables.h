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

namespace AGS {

const PlainGameDescriptor GAME_NAMES[] = {
	{ "ags", "Adventure Game Studio Game" },
	{ "ags-scan", "AGS Game Scanner" },

	// Pre-2.5 games that aren't supported by the current AGS engine
	{ "6da", "The 6 Day Assassin" },
	{ "aaronsepicjourney", "Aaron's Epic Journey" },
	{ "abstract", "Abstract" },
	{ "aceduswell", "Ace Duswell: Where's The Ace?" },
	{ "achristmastale", "A Christmas Tale" },
	{ "adayinthefuture", "A Day In The Future" },
	{ "adventuregame", "Adventure Game" },
	{ "adventurenoir", "Adventure Noir" },
	{ "agsbgeditor", "AGS Background Editor" },
	{ "agsdraw", "AGS Draw" },
	{ "aliengame", "Alien Game" },
	{ "andybigadv", "Andy Penis' BIG Adventure" },
	{ "anotherrpg", "Another RPG" },
	{ "asapadventure", "ASAP Adventure" },
	{ "bertthenewsreader", "Bert the Newsreader" },
	{ "billybobwildnight", "Billy Bob's Wild Night" },
	{ "blackjack", "AGS Blackjack" },
	{ "blastoff", "Blastoff!" },
	{ "bluecupontherun", "Bluecup on the Run" },
	{ "bobsquest2", "Bob's Quest 2: The quest for the AGS Blue cup award" },
	{ "bookofspells1", "The Book of Spells (A funtasy Adventure) - Chapter 1" },
	{ "bookofspells2", "The Book of Spells (A funtasy Adventure) - Chapter 2" },
	{ "bookofspells3", "The Book of Spells (A funtasy Adventure) - Chapter 3" },
	{ "calsoon", "Calsoon: The Tork, the Witch and the Calsoon" },
	{ "candy", "I Want Candy" },
	{ "captainmuchly", "Captain Muchly Drinks Bleach" },
	{ "carverisland1", "The Secret of Carver Island" },
	{ "carverisland2", "Carver Island 2: Mrs. Rodriguez's Revenge" },
	{ "castlequest", "Castle Quest" },
	{ "cda", "Captain Disaster" },
	{ "chef", "The Chef" },
	{ "coffeebreak", "A Cup Of Coffee is Worth a Thousand Bathroom Breaks" },
	{ "commanderkeenron", "Commander Keen Enters Reality-on-the-Norm" },
	{ "compensation", "Compensation" },
	{ "cosmos", "Cosmos" },
	{ "crimetime", "Crime Time" },
	{ "cutlass", "Cutlass" },
	{ "damnthatspill", "Clyde Remolen in: Damn that Spill!" },
	{ "darksects", "Dark Sects" },
	{ "davyjonescestmort", "Davy Jones c'est Mort" },
	{ "davyjonesisback", "Davy Jones is Back" },
	{ "deadquest", "Dead Quest" },
	{ "deepbright", "Deepbright" },
	{ "defenderofron", "Defender of Reality-on-the-Norm" },
	{ "defsrpgdemo", "Def's RPG Demo" },
	{ "demonslayer1", "DemonSlayer 1: Bert the Super Demon Slayer Guy" },
	{ "demonslayer2", "DemonSlayer 2: DemonSlayer vs. Vegetable Vampires" },
	{ "demonslayer3", "DemonSlayer 3: Hotel and Alley" },
	{ "demonslayer4", "DemonSlayer 4" },
	{ "dirkchafberg", "Dirk Chafberg" },
	{ "dogkennel", "The Dog Kennel" },
	{ "domjohnson", "Dom Johnson" },
	{ "earwigisangry", "Earwig Is Angry!" },
	{ "easterbunny", "The Easter Bunny's Splendiferous Adventure" },
	{ "edgeofreality", "Edge of Reality" },
	{ "elburro", "El Burro" },
	{ "enchantedlake", "The Enchanted Lake" },
	{ "equallyinsane", "Jake Rabbit in: E.I. - Equally Insane" },
	{ "erniesbigadventure1", "Ernie's Big Adventure" },
	{ "erniesbigadventure2", "Ernie's Big Adventure II" },
	{ "everythingeuro", "Everything Euro / Alles Euro" },
	{ "exile", "Exile" },
	{ "existance", "Existance" },
	{ "exodus", "Exodus" },
	{ "eyesofthejadesphinx", "Eyes of the Jade Sphinx" },
	{ "faddevil", "Fad of Devil's Hash" },
	{ "fatman", "Fatman" },
	{ "firewall", "Firewall" },
	{ "flies", "Flies" },
	{ "floyd", "Floyd SB the Company Man" },
	{ "fowlplay", "Reality-on-the-Norm 2 and 1/2: Fowl Play" },
	{ "gaeafallen", "Gaea Fallen" },
	{ "gorthor", "Gorthor of the Cave People" },
	{ "grandfathertreasure", "Grandfather's Treasure" },
	{ "grannyzombiekiller", "Granny Zombiekiller in: Mittens Murder Mystery" },
	{ "gravediggerjoe", "Grave Digger Joe" },
	{ "gregsmountainousadventure", "Greg's Mountainous Adventure" },
	{ "greysgreytadv", "Mr. Grey's Greyt Adventure" },
	{ "helmsuntitled", "Helm's Untitled Game" },
	{ "henkkaquest", "Henkka: Mini Quest" },
	{ "hermit", "Night of the Hermit" },
	{ "hiphendrix", "Highly Hip Hendrix" },
	{ "hookymcp", "Hooky McPegleg, Pirate Postman" },
	{ "housequest", "Joe Jenkins in: House Quest" },
	{ "huckleburyhippie", "Hucklebury Hippie" },
	{ "hugoags", "Hugo's House of Horrors AGS Remake" },
	{ "ijuntitled", "Ian James' Untitled Game" },
	{ "inadream", "In a Dream" },
	{ "indygoldmedallion", "Indiana Jones and The Gold Medallion" },
	{ "indyguybrush", "Indiana Jones - Alla ricerca di Guybrush Threepwood" },
	{ "interactivefiction", "Interactive Fiction" },
	{ "intergalacticlife", "Intergalactic Life" },
	{ "invasionspacealiens", "Invasion of the Space Aliens Who Are Not From Earth But From Some Other Planet" },
	{ "invisiblemachine", "The Invisible Machine" },
	{ "islandquest", "Island Quest" },
	{ "ispy", "I Spy" },
	{ "jamesbondage", "James Bondage" },
	{ "javajo", "Java Jo's Koffee Stand!" },
	{ "jinglebells", "Jingle Bells" },
	{ "kittensandcacti", "Kittens and Cacti" },
	{ "larryvales1", "Larry Vales: Traffic Division" },
	{ "larryvales2", "Larry Vales II: Dead Girls are Easy" },
	{ "larryvales3", "Larry Vales III: Time Heals All 'Burns" },
	{ "lassiandrogerventure", "Lassi and Roger Venture a Bit" },
	{ "lassiandrogermeetgod", "Lassi and Roger Meet God" },
	{ "lassiquest1", "Lassi's Quest I" },
	{ "lassiquest1remake", "Lassi's Quest I Remake" },
	{ "lassiquest2", "Lassi's Quest II" },
	{ "leisuresuitlarry4", "Leisure Suit Larry 4" },
	{ "limpyghost", "Limpy Ghost" },
	{ "littlejonnyevil", "Little Jonny Evil" },
	{ "littlewillydarts", "Little Willy's Darts Game" },
	{ "littlewillyshotgun", "Little Willy's Shotgun Fun Zone!" },
	{ "loop", "Loop" },
	{ "lorryloader", "Lorry Loader" },
	{ "losttape", "The Lost Tape" },
	{ "losttreasureron", "The Lost Treasure of Reality-on-the-Norm" },
	{ "loumarlou", "The Adventures of Lou Marlou" },
	{ "ludwig", "VonLudwig" },
	{ "lukelandrunner", "Luke Landrunner" },
	{ "lunchtimeofthedamned", "Lunchtime Of The Damned" },
	{ "lupoinutile", "Lupo Inutile" },
	{ "m0ds_eve", "M0ds's Unmade Games: EVE" },
	{ "m0ds_lassi", "M0ds's Unmade Games: LASSI" },
	{ "m0ds_modplay", "M0ds's Unmade Games: MODPLAY" },
	{ "m0ds_runner", "M0ds's Unmade Games: RUNNER" },
	{ "m0ds_test", "M0ds's Unmade Games: TEST" },
	{ "m0ds_thedome", "M0ds's Unmade Games: THEDOME" },
	{ "m0ds_thunder", "M0ds's Unmade Games: THUNDER" },
	{ "magsceremony2001jul", "July 2001 MAGS Ceremony" },
	{ "magsceremony2001jun", "June 2001 MAGS Ceremony" },
	{ "magsceremony2002jul", "July 2002 MAGS Ceremony" },
	{ "martychonks", "Marty Chonks in: Dances With Camels" },
	{ "maxdark", "Max the Dark Hero" },
	{ "meninbrown", "Men in Brown" },
	{ "miagsremake", "Monkey Island AGS Remake" },
	{ "midtownshootout", "Mid-Town Shoot-Out" },
	{ "migaiden", "Monkey Island Gaiden" },
	{ "mikasdream", "Mika's Surreal Dream" },
	{ "mojoisland", "Mojo Island" },
	{ "momsquest", "Mom's Quest" },
	{ "monkeypeninsula", "The Treasure of Monkey Peninsula" },
	{ "monkeyplank", "Monkey Plank" },
	{ "moosewars", "Moose Wars: Desire For More Cows" },
	{ "mousetcd", "The Mouse That Couldn't Die" },
	{ "murder", "Murder" },
	{ "murderfishhotel", "Philip Illinilings and the Murder at Fish Hotel" },
	{ "nicholaswolfe1", "Nicholas Wolfe part I: Framed" },
	{ "nightoftheplumber", "Night of the Plumber" },
	{ "nightwatchron", "Nightwatch (2001)" },
	{ "nihilism", "Nihilism" },
	{ "ninjaquest", "Ninja Quest" },
	{ "nokq", "No King Quest!" },
	{ "nomedon", "Nomedon Inc." },
	{ "notanothersq", "Not Another Space Quest" },
	{ "novomestro", "Novo Mestro" },
	{ "odysseus", "The Trials of Odysseus Kent" },
	{ "oldparkquest", "Old Park Quest" },
	{ "orbblanc", "Orb Blanc: Ball of the Universe" },
	{ "pantaloon", "Quirk Pantaloon" },
	{ "paranormality", "Lee & Perrin: Paranormality" },
	{ "pd", "P.D." },
	{ "permanentdaylight", "Permanent Daylight" },
	{ "perpetrator", "Perpetrator" },
	{ "pezzobertrum", "Pezzo Bertrum and the Band That Wouldn't Die" },
	{ "piratefry1", "Pirate Fry and Volcano Island" },
	{ "pixelypete", "Pixely Pete" },
	{ "pizzaquest", "Pizza Quest" },
	{ "pointblank", "Point Blank" },
	{ "pornquest", "Porn Quest" },
	{ "postmanonlydiesonce", "The Postman Only Dies Once" },
	{ "preachersjump", "Preachers Can't Jump" },
	{ "projectevilspy", "Project Evilspy: FenderQ Meets Jeeforce for Lunch" },
	{ "pyramid", "Pyramid" },
	{ "qfc", "Quest For Colours" },
	{ "qfg412", "Quest for Glory 4 1/2" },
	{ "raymondskeys", "Raymond's Keys: A Tragedy" },
	{ "redflagg", "Red Flagg in: Don't Call Me Blue!" },
	{ "retardsgame", "The Retards Game" },
	{ "returnofdvs", "Return of Die Vie Ess" },
	{ "richardlonghurst", "Richard Longhurst and the Box That Ate Time" },
	{ "ripp", "Revenge in Parelly Point" },
	{ "robblanc1", "Rob Blanc I: Better Days of a Defender of the Universe" },
	{ "robblanc2", "Rob Blanc II: Planet of the Pasteurised Pestilence" },
	{ "robblanc3", "Rob Blanc III: The Temporal Terrorists" },
	{ "rodekill", "Rode Kill: A Day In the Life" },
	{ "rodequest", "Rode Quest" },
	{ "ronbeachparty", "Reality-on-the-Norm Beach Party" },
	{ "ronxmas2002", "Reality-on-the-Norm Xmas 2002" },
	{ "samthepiratemonkey", "Sam The Pirate Monkey" },
	{ "short", "Short" },
	{ "slackerquest", "Slacker Quest" },
	{ "sma1", "Second Moon Adventure - Part I: Night" },
	{ "snailquest1", "Snail Quest" },
	{ "snailquest2", "Snail Quest 2" },
	{ "snailquest3", "Snail Quest 3" },
	{ "sol", "SOL" },
	{ "southpark", "South Park" },
	{ "sovietunionstrikesback", "The Soviet Union Strikes Back" },
	{ "space", "Space" },
	{ "spacemail", "Space Mail" },
	{ "startreksnw", "Star Trek Explorer: Strange New World" },
	{ "startrektng", "Star Trek: The Next Generation" },
	{ "stickmen", "Stickmen" },
	{ "superdisk", "Superdisk" },
	{ "taleofroland", "Tale of Roland" },
	{ "teamwork", "Teamwork Tutorial" },
	{ "testicle", "Day of the Testicle" },
	{ "thecrownofgold", "The Crown of Gold" },
	{ "theinexperiencedassassin", "Petrakov: The Inexperienced Assassin" },
	{ "theisland", "The Island" },
	{ "thepigion", "The Pigion" },
	{ "thetower", "The Tower" },
	{ "thevestibule", "The Vestibule" },
	{ "thewarp", "The Warp" },
	{ "tinygreen", "Tiny Green Piece of Crap" },
	{ "tomeoflegend", "Tome of Legend" },
	{ "tommato", "Tom Mato's Grand Wing-Ding" },
	{ "tullesworld1", "Tulle's World 1: The Roving of Candale" },
	{ "tullesworld3", "Tulle's World 3: The Glorious Realm of Thendor" },
	{ "tvquest", "TV Quest" },
	{ "ultimerr", "Ultimerr XXII: The Gems of Anthrax" },
	{ "ultimerr2", "Ultimerr XXXIV: The Island of Fartfase" },
	{ "universalequalizer", "The Universal Equalizer" },
	{ "unkhour_anhour", "Unnamed Hour Game - ANHOUR" },
	{ "unkhour_jv", "Unnamed Hour Game - JV" },
	{ "unkhour_onehour", "Unnamed Hour Game - ONEHOUR" },
	{ "unkhour_shorty", "Unnamed Hour Game - SHORTY" },
	{ "vengeanceofthechicken", "Vengeance of the Chicken" },
	{ "waitkey", "WaitKey();" },
	{ "wambus", "The Wambus" },
	{ "whokilledkennyrogers", "Who killed Kenny Rogers?" },
	{ "xenoreturns", "Xenophobe Returns" },
	{ "zakagsremake", "Zak McKracken and the Alien Mindbenders AGS Remake" },


	// Commercial games
	{ "3geeks", "3 GEEKS" },
	{ "7metresaway", "7 Meters Away. Oscar and the Cell of the Soul" },
	{ "aboardtheadventure","Aboard the Adventure" },
	{ "abscission", "Abscission" },
	{ "alemmo", "Al Emmo And The Lost Dutchman's Mine" },
	{ "alemmoanozira", "Al Emmo's Postcards from Anozira" },
	{ "alum", "Alum" },
	{ "apotheosis", "The Apotheosis Project" },
	{ "ashinaredwitch", "Ashina: The Red Witch" },
	{ "astroloco", "Astroloco: Worst Contact" },
	{ "atotk", "A Tale Of Two Kingdoms" },
	{ "azazelxmas", "Azazel's Christmas Fable" },
	{ "avcsurvivalguide", "AVC's Survival Guide / Guía de supervivencia en CAV" },
	{ "beekeeperpicnic", "The Beekeeper's Picnic" },
	{ "beer", "Beer!" },
	{ "beyondowlsgard","Beyond the Edge of Owlsgard" },
	{ "bizarreearthquake", "Bizarre Earthquake" },
	{ "blackfriday2", "Autumn of Death: Black Friday II" },
	{ "blackwell1", "The Blackwell Legacy" },
	{ "blackwell2", "Blackwell Unbound" },
	{ "blackwell3", "The Blackwell Convergence" },
	{ "blackwell4", "The Blackwell Deception" },
	{ "blackwell5", "The Blackwell Epiphany" },
	{ "brownieadv1", "Brownie's Adventure" },
	{ "captaindisaster", "Captain Disaster in: Death Has A Million Stomping Boots" },
	{ "captaindisasterriskara", "Captain Disaster and The Two Worlds of Riskara" },
	{ "carnivalags", "Carnival" },
	{ "castleagony", "Castle Agony" },
	{ "castledornstein", "Castle Dornstein" },
	{ "charnelhousetrilogy", "The Charnel House Trilogy" },
	{ "chronicleofinnsmouth", "Chronicle of Innsmouth" },
	{ "conspirocracy", "Conspirocracy" },
	{ "cosmosquest3", "Cosmos Quest III" },
	{ "cosmosquest4", "Cosmos Quest IV" },
	{ "crimsondiamond","The Crimson Diamond" },
	{ "crystalshardadventurebundle", "Crystal Shard Adventure Bundle" },
	{ "danewguys2", "Da New Guys: Day of the Jackass" },
	{ "detectivebhdino", "Detective Boiled-Hard - Case File: Death of the Space Dino Hunter" },
	{ "detectivegallo", "Detective Gallo" },
	{ "diamondsintherough", "Diamonds in the Rough" },
	{ "docapocalypse", "Doc Apocalypse" },
	{ "downfall2009", "Downfall (2009)" },
	{ "downfall2016", "Downfall (2016)" },
	{ "dreamswitchhouse","Dreams in the Witch House" },
	{ "dustbowl", "Dustbowl" },
	{ "englishhaunting", "An English Haunting" },
	{ "excavationhb", "The Excavation of Hob's Barrow" },
	{ "falconcity", "Falcon City" },
	{ "feriadarles", "Feria d'Arles" },
	{ "footballgame", "Football Game" },
	{ "geminirue", "Gemini Rue" },
	{ "ghostdream", "Ghostdream" },
	{ "ghostman", "Ghostman: The Council Calamity" },
	{ "gigant", "Gigant" },
	{ "gobliiins5-1", "Gobliiins5 - L'Invasion des Morglotons Part1/Demo" },
	{ "gobliiins5-2", "Gobliiins5 - L'Invasion des Morglotons Part2" },
	{ "gobliiins5-3", "Gobliiins5 - L'Invasion des Morglotons Part3" },
	{ "gobliiins5-4", "Gobliiins5 - L'Invasion des Morglotons Part4" },
	{ "goldenwake", "A Golden Wake" },
	{ "graceward", "Graceward" },
	{ "grandmabadass", "GrandMa Badass" },
	{ "guardduty", "Guard Duty" },
	{ "hauntingcliffhouse", "Haunting At Cliffhouse" },
	{ "herdiscoming", "Herd is Coming" },
	{ "incantamentum", "Incantamentum" },
	{ "insectophobiaep1", "Insectophobia: Episode 1" },
	{ "jeffreyspaceact1", "Jeffrey In Space - Act I" },
	{ "jorry", "JORRY" },
	{ "justignorethem", "Just Ignore Them" },
	{ "justignorethembrea1", "Just Ignore Them: Brea's Story Tape 1" },
	{ "kathyrain", "Kathy Rain" },
	{ "killyourself", "Kill Yourself" },
	{ "lamplightcity", "Lamplight City" },
	{ "lancelothangover", "Lancelot's Hangover" },
	{ "larrylotter", "Larry Lotter and the Test of Time / Warthogs" },
	{ "lastdayadolf","The Last Day of Adolf" },
	{ "lastpirateadventure","The Last Pirate Adventure: Drake's Treasure" },
	{ "leewardep1","Leeward - Episode 1" },
	{ "legendofhand", "Legend of Hand" },
	{ "legendofskye", "The Legend of Skye" },
	{ "mage", "Mage's Initiation: Reign of the Elements" },
	{ "maggieapartment", "Maggie's Apartment" },
	{ "magretfdb", "Magret & FaceDeBouc" },
	{ "melonhead", "Melon Head" },
	{ "metaldead", "Metal Dead" },
	{ "mountainsofmadness", "Chronicle of Innsmouth: Mountains of Madness" },
	{ "murdercases", "Murder Cases" },
	{ "mybigsister", "My Big Sister" },
	{ "mybigsisterrm", "My Big Sister: Remastered" },
	{ "nellycootalot-hd", "Nelly Cootalot: Spoonbeaks Ahoy! HD" },
	{ "neofeud", "Neofeud" },
	{ "nightmareframes", "Nightmare Frames" },
	{ "odissea", "Odissea - An Almost True Story" },
	{ "oldskies", "Old Skies" },
	{ "oott", "Order of the Thorne: The King's Challenge" },
	{ "perfecttides", "Perfect Tides" },
	{ "perfecttidess2s", "Perfect Tides: Station to Station" },
	{ "perfidiouspetrolstation", "The Perfidious Petrol Station" },
	{ "personalrocket", "Personal Rocket" },
	{ "phantomfellows", "The Phantom Fellows" },
	{ "phoenixtales", "Phoenix Tales" },
	{ "piratethemepark", "Pirate Theme Park: A Short Adventure" },
	{ "primordia", "Primordia" },
	{ "projectorface", "Projector Face" },
	{ "puzzlebots", "Puzzle Bots" },
	{ "qfi", "Quest for Infamy" },
	{ "quantumnauts", "Quantumnauts" },
	{ "quantumnauts2", "Quantumnauts 2" },
	{ "redbow", "Red Bow" },
	{ "resonance", "Resonance" },
	{ "richardandalice", "Richard & Alice" },
	{ "rnrneverdies", "Rock 'n' Roll Will Never Die!" },
	{ "roguestate", "Rogue State" },
	{ "rosewater", "Rosewater" },
	{ "samaritan", "The Samaritan Paradox" },
	{ "shardlight", "Shardlight" },
	{ "shivah", "The Shivah" },
	{ "shivahkosher", "The Shivah: Kosher Edition" },
	{ "shoaly", "Shoaly You Can't Be Serious!" },
	{ "signalecho", "Signal & Echo: Iris is Missing" },
	{ "sisterssecret", "Sisters' Secret" },
	{ "slazakloss", "The Tragic Loss of M. Slazak" },
	{ "smallsister", "Small Sister" },
	{ "snowproblem", "Snow Problem" },
	{ "spaceraven", "Space raven quest - Tiny planet" },
	{ "sphonx", "Sphonx" },
	{ "starshipquasar", "Starship Quasar" },
	{ "stayingalive", "The Ancient Art of Staying Alive" },
	{ "stellarmessep1", "Stellar Mess - Episode 1: The Princess Conundrum" },
	{ "stormwater", "Stormwater" },
	{ "strangeland", "Strangeland" },
	{ "sulifallenharmony", "Suli Fallen Harmony" },
	{ "sumatra", "Sumatra: Fate of Yandi" },
	{ "superegoch12", "Superego - Chapter 1+2" },
	{ "superegoch3", "Superego - Chapter 3" },
	{ "superjazzman", "Super Jazz Man" },
	{ "symploke1", "Symploke: Legend of Gustavo Bueno (Chapter 1)" },
	{ "symploke2", "Symploke: Legend of Gustavo Bueno (Chapter 2)" },
	{ "symploke3", "Symploke: Legend of Gustavo Bueno (Chapter 3)" },
	{ "tales", "Tales" },
	{ "technobabylon", "Technobabylon" },
	{ "theadventuresoffatman", "The Adventures of Fatman" },
	{ "theadventuresoftheblackhawk", "The Adventures of The Black Hawk" },
	{ "theantidote", "The Antidote" },
	{ "thebeardinthemirror", "The Beard in the Mirror" },
	{ "thecastle", "The Castle" },
	{ "thecatlady", "The Cat Lady" },
	{ "theropods", "Theropods" },
	{ "thesecretofhuttongrammarschoolvga", "The Secret of Hutton Grammar School VGA" },
	{ "thesecretsofjesus", "The Secrets of Jesus" },
	{ "theterribleoldman", "The Terrible Old Man" },
	{ "timegentlemenplease", "Time Gentlemen, Please!" },
	{ "unavowed", "Unavowed" },
	{ "unlikelyprometheus", "The Unlikely Prometheus" },
	{ "untilihaveyou", "Until I Have You" },
	{ "waitingfortheloop", "Waiting For the Loop" },
	{ "welcometosunnymunarvagir", "Welcome to Sunny Munarvagir" },
	{ "whispersofamachine", "Whispers of a Machine" },
	{ "wingman", "WingMan" },
	{ "wolfterritory", "Wolf Territory" },
	{ "yetilastescape", "The Last Escape of Yeti" },
	{ "zidjourney", "Zid Journey" },
	{ "zniwadventure", "Zniw Adventure" },

	// Commercial Italian games
	{ "onironauta", "Onironauta" },

	// AGDI games. They get their own grouping because they're just that awesome
	{ "kq1agdi", "King's Quest I: Quest for the Crown Remake" },
	{ "kq2agdi", "King's Quest II: Romancing the Stones Remake" },
	{ "kq3agdi", "King's Quest III Redux: To Heir is Human" },
	{ "qfg2agdi", "Quest for Glory II: Trial by Fire Remake" },

	// Infamous Adventures games. Likewise
	{ "sq2fg", "Space Quest for Glory" },
	{ "sq2vga", "Space Quest II - Vohaul's Revenge VGA Remake" },
	{ "kq3vga", "King's Quest III: To Heir is Human VGA Remake" },

	// Free post-2.5 games that are likely supported by the AGS engine
	{ "10waysfromsunday", "10 Ways from Sunday" },
	{ "11-11-11", "11-11-11" },
	{ "12hoursslave", "12 Hours a Slave" },
	{ "15minutes", "15 Minutes" },
	{ "1dayamosquito", "1 day a mosquito" },
	{ "1000hrpg", "The 1000 Hour RPG" },
	{ "2000ways", "2000 Ways To Play Badmington" },
	{ "2034acaftercanada1", "2034 A.C. (After Canada)" },
	{ "2034acaftercanada2", "2034 A.C. (After Canada) II" },
	{ "2080wallcity", "2080 Wall City Lockdown" },
	{ "24hourgame", "The 24 Hour Game" },
	{ "24hours", "24 Hours" },
	{ "30minutes", "30 minutes" },
	{ "3dayspandora", "3 Days to Pandora" },
	{ "3minfart", "3 Minutes a Fart" },
	{ "3pigsandawolf", "Three Little Pigs and a Wolf" },
	{ "46memorylane", "46 Memory Lane" },
	{ "4lungboy", "4-Lung Boy" },
	{ "4ofclubs", "4 of Clubs" },
	{ "5daysastranger", "5 Days A Stranger" },
	{ "5dragons", "5 Dragons" },
	{ "5oclocklock", "5-O'clock Lock" },
	{ "616way", "616 Mock Orange Way" },
	{ "6174solitaire", "6174 Solitaire" },
	{ "6dayblah", "6 day blah blah vampire goold!" },
	{ "6daysasacrifice", "6 Days A Sacrifice" },
	{ "6mornings", "6mornings" },
	{ "7daysaskeptic", "7 Days A Skeptic" },
	{ "99species", "99 Species" },
	{ "9hourstodawn", "9 Hours to Dawn" },
	{ "9monthsin", "9 Months In" },
	{ "aazor", "Aazor: Life of a Demon - Part I: The Beginning" },
	{ "abducted10mins", "Abducted: 10 Minutes!!!" },
	{ "abduction", "Abduction" },
	{ "abettermousetrap", "A Better Mouse Trap" },
	{ "ablemabel", "Able Mabel gets a Job" },
	{ "abominationobtainer", "Abomination Obtainer" },
	{ "absent", "Absent" },
	{ "absentpart1", "Absent - Part I: Innocent Until Proven Guilty" },
	{ "absin", "Absin - An Interactive Mystery" },
	{ "absurdistan", "Absurdistan" },
	{ "acaixa", "A caixa" },
	{ "acatsnight1", "A Cat's Night" },
	{ "acatsnight2", "A Cat's Night 2 - Orazio Goes To Town" },
	{ "access", "_Access" },
	{ "aceking", "Ace King" },
	{ "acequest", "Ace Quest" },
	{ "acharchronicles", "Achar Chronicles: Oblitus" },
	{ "achristmasblunder", "A Christmas Blunder" },
	{ "achristmasghost", "A Christmas Ghost" },
	{ "achristmasnightmare", "A Christmas Nightmare" },
	{ "achristmaswish", "A Christmas Wish" },
	{ "achtungfranz", "Achtung Franz: Quest For Wine" },
	{ "aciddd", "AciDDD" },
	{ "acjadventure", "A Christopher Jones Adventure" },
	{ "acureforthecommoncold", "A Cure for the Common Cold" },
	{ "acuriouspastime", "A Curious Pastime" },
	{ "acurioussilence", "A Curious Silence" },
	{ "adalinepicnic", "Adaline and the Moon Picnic" },
	{ "adateinthepark", "A Date in the Park" },
	{ "adaywithmichael", "A Day with Michael" },
	{ "advallinthegame", "Adventure: All In The Game" },
	{ "advbunnybunnyman", "Adventures of Bunny Bunnyman / Pupu Pupulaisen Seikkailut" },
	{ "adventureisland", "Adventure Island" },
	{ "adventurequest", "Adventure Quest" },
	{ "adventurequest2", "Adventure Quest 2 - Winterlong" },
	{ "adventuresofjoshanddyan", "The Adventures of Josh and Dyan" },
	{ "adventuresofmaxfaxepisode1", "Adventures of Max Fax (episode 1)" },
	{ "adventuretheinsidejob", "Adventure: The Inside Job" },
	{ "adventureworld", "Adventure's World" },
	{ "advwelcome", "Adventure: Welcome to the Genre" },
	{ "ael", "Ael" },
	{ "aerinde", "Aerinde" },
	{ "aeronuts", "AeroNuts" },
	{ "aeternaduel", "Aeterna Duel" },
	{ "affairoftheweirdo", "Affair of the Weirdo" },
	{ "affection", "Affection" },
	{ "afragmentofher", "A Fragment of Her" },
	{ "afriendindeed", "A Friend Indeed ..." },
	{ "afrojones", "Afro Jones" },
	{ "afterashadow", "After a Shadow" },
	{ "agentbee", "Agent Bee" },
	{ "agenthudson", "Agent Hudson" },
	{ "agenttrinityepisode0theultimatum", "Agent Trinity - Episode 0 - The Ultimatum" },
	{ "aggghost", "A G-G-Ghost!" },
	{ "agitprop", "Agitprop: The Game" },
	{ "agnosticchicken", "The Agnostic Chicken: The Quest For The Bottle Opener" },
	{ "agsawards2008", "AGS Awards Ceremony 2008" },
	{ "agsawards2016", "AGS Awards Ceremony 2016" },
	{ "agsawards2017", "AGS Awards Ceremony 2017" },
	{ "agsawards2020", "AGS Awards Ceremony 2020" },
	{ "agsawards2022", "AGS Awards Ceremony 2022" },
	{ "agsawards2023", "AGS Awards Ceremony 2023" },
	{ "agsawardsbp", "AGS Awards Backstage Pass" },
	{ "agscamerastd", "AGS Cameras Tech Demo" },
	{ "agschess", "AGS Chess" },
	{ "agscolosseum", "AGS Colosseum" },
	{ "agsdarts", "AGS 180 Darts" },
	{ "agsdarts2", "AGS 180 Darts 2" },
	{ "agsfightgame2009", "Ahmet's AGS Fight Game 2009" },
	{ "agsfightgameremix", "Ahmet's AGS Fight Game Remix" },
	{ "agsfootballer", "AGS Footballer" },
	{ "agsinvaders", "AGS-Invaders" },
	{ "agsjukebox", "AGS JukeBox" },
	{ "agslife", "AGSLife" },
	{ "agsmagus", "AGS Magus" },
	{ "agsmastermind", "AGS Mastermind" },
	{ "agsmittensshooter", "AGS Mittens Shooter" },
	{ "agsmoduletester", "AGS Module Tester" },
	{ "agstechsupport", "AGS Tech Support Game" },
	{ "agswerewolf", "AGS Werewolf" },
	{ "agsyahtzee", "AGS Yahtzee" },
	{ "agsyahtzee2", "AGS Yahtzee 2" },
	{ "agunshotinroom37", "A Gunshot in Room 37" },
	{ "ahamoth", "Ahamoth" },
	{ "ahomieadventure", "A Homie Adventure" },
	{ "aidaschristmas", "Aida's Strange Christmas" },
	{ "aidashalloween", "Aida's Bizarre Halloween" },
	{ "ainthegoffantabulousw", "Adventures in the Galaxy of Fantabulous Wonderment" },
	{ "airwave", "~airwave~ - I Fought the Law, And the Law One" },
	{ "alandlordsdream", "A Landlord's Dream" },
	{ "alansaveschristmas", "Alan saves Christmas (and most of the world too)" },
	{ "aliceinwonderlandis", "Lewis Carroll's Alice In Wonderland - an interactive storybook" },
	{ "alienattack", "Alien Attack" },
	{ "aliencarniage", "Alien Carniage" },
	{ "aliencowrampage", "Alien Cow Rampage: Orion Needs Your Milk!" },
	{ "alienescapade", "Alien Escapade" },
	{ "alienprison", "Alien Prison Escape" },
	{ "alienpuzzleinvasion", "Alien Puzzle Invasion" },
	{ "alienrapeescape", "Alien Rape Escape" },
	{ "alienspaceman", "The Alien / The Spaceman" },
	{ "alienthreat", "Alien Threat" },
	{ "alientimezone", "Alien Time Zone" },
	{ "alienvspredator", "Alien vs. Predator" },
	{ "allgonesoon", "All Gone Soon" },
	{ "allgonesoon2", "All Gone Soon 2" },
	{ "allhallowseve", "All Hallows' Eve" },
	{ "alloweisland", "Al Lowe's Secret Island" },
	{ "allpigs", "All Pigs Deserve To Burn In Hell" },
	{ "allthewaydown", "All The Way Down" },
	{ "alluminum", "P.I. Al Luminum: Haunted House" },
	{ "almostblue", "Almost Blue" },
	{ "aloneinthenight", "Alone in the Night" },
	{ "alphabeta", "Alphabeta: A Room Full of Words" },
	{ "alphablock", "AlphaBlock" },
	{ "alphadog", "Alpha Dog" },
	{ "alphax", "Alpha X" },
	{ "alphaxsmash", "Alpha X - Project: Smash" },
	{ "alquest1", "Al-Quest 1" },
	{ "alyssaescape", "Alyssa's Escape" },
	{ "alysvsthephantomfelinefoe", "Alys vs. The Phantom Feline Foe" },
	{ "amagicstone", "A magic stone" },
	{ "america2000", "America 2000" },
	{ "amotospuf", "Amoto's Puf" },
	{ "amongthorns", "Among Thorns" },
	{ "amtag", "AMTAG: another medieval themed adventure game" },
	{ "amused", "A-Mused" },
	{ "analienswork", "An Alien's Work Is Never Done..." },
	{ "ancientaliens", "Ancient Aliens - The Roots of Sound" },
	{ "anentertainingspeech", "An Entertaining Speech" },
	{ "aneternityreflecting", "An Eternity, Reflecting" },
	{ "anig", "An Instagame" },
	{ "anightinberry", "A night in Berry / Une nuit en Berry" },
	{ "anightmareonduckburg", "A Nightmare on Duckburg" },
	{ "anightthatwouldntend", "A Night That Wouldn't End" },
	{ "anighttoremember", "A Night to Remember" },
	{ "animalagency", "Animal Agency" },
	{ "animalcruelty", "Animal Cruelty" },
	{ "anna", "anna" },
	{ "annaquest", "Anna's Quest Vol. 1: Winfriede's Tower" },
	{ "annieandroidautomatedaffection", "Annie Android: Automated Affection" },
	{ "annielondonberry", "Annie Londonderry" },
	{ "annoyingquest", "Annoying Quest" },  // aka Uhhr
	{ "anoffer", "An offer you cannot refuse" },
	{ "anophtos", "ANOPHTOS" },
	{ "anotherheaven", "Another Heaven" },
	{ "anotherhero", "Another Hero" },
	{ "anothermuseum", "Another Museum" },
	{ "anotherwayout", "Another Way Out" },
	{ "anthonysessay", "Anthony's Essay" },
	{ "anticipatingmurder", "Anticipating Murder" },
	{ "antiheroes", "Anti-Heroes" },
	{ "anxiousawakening", "An Anxious Awakening" },
	{ "ap0", "AP0" },
	{ "apiratestale", "A pirates tale" },
	{ "aplaceinthesun", "A Place In The Sun" },
	{ "aplacewithoutfrontier", "A Place Without Frontier" },
	{ "apocalypsemeow1", "Apocalypse Meow Chapter 1 - Gatitos: The Paws of Fate" },
	{ "apocalypsevel", "Apocalypse: Vel" },
	{ "applefarm", "Apple Farm" },
	{ "appointmentwithdeath", "Appointment With Death" },
	{ "apprentice", "Apprentice" },
	{ "apprentice2", "Apprentice II: The Knight's Move" },
	{ "aprofoundjourney", "A Profound Journey" },
	{ "apunkwithwheels", "A punk with wheels" },
	{ "aractaur", "Aractaur" },
	{ "araindogstory", "A Raindog Story" },
	{ "archeos", "Archeos" },
	{ "archivesinsondable", "Les Archives de l'Insondable" },
	{ "ardensvale", "Arden's Vale" },
	{ "ardentfever", "Ardent Fever" },
	{ "arewethereyet", "Are we there yet?" },
	{ "arewethereyet2", "Are we there yet? 2" },
	{ "arjunaz78", "Arjunaz78 @ AGS" },
	{ "aroomwithoutyou", "A Room Without You In It" },
	{ "armageddonmargaret", "Armageddon Margaret" },
	{ "artisan", "Artisan" },
	{ "artofdying", "The Art of Dying" },
	{ "asecondface", "A Second Face: The Eye of Geltz is Watching You" },
	{ "ashpines", "Ash Pines" },
	{ "ashortnightmare", "A Short Nightmare" },
	{ "asimplefix", "A Simple Fix" },
	{ "asledmundo", "ASL: Edmundo with music" },
	{ "asotc", "A Stain on the Company" },
	{ "asporia", "Asporia: Hidden Threat" },
	{ "asterix", "Asterix and the Roman Underground" },
	{ "astranded", "Astranded (Astro-Stranded)" },
	{ "astron", "Astron" },
	{ "asuspiciousdate", "A Suspicious Date" },
	{ "aswinsdream", "Aswin's Dream" },
	{ "ataintedtreat", "A Tainted Treat" },
	{ "ataleinthezoo", "A Tale in the Zoo" },
	{ "ataleofbetrayal", "A Tale Of Betrayal" },
	{ "atapi", "Atapi" },
	{ "athingaboutnothingness", "A thing about nothingness" },
	{ "atotkjukebox", "A Tale of Two Kingdoms Jukebox" },
	{ "atreatandsometricks", "A Treat and Some Tricks" },
	{ "attackgame", "Attack Game" },
	{ "atthecafe", "At the Café" },
	{ "audioquest", "Audio Quest" },
	{ "augustlonging", "The long August longing" },
	{ "aunaturel", "Au Naturel" },
	{ "authorsim", "Super Author Simulator 2023" },
	{ "automation", "Automation" },
	{ "averyspecialdog", "A very special dog" },
	{ "awakener", "Awakener" },
	{ "awakening", "Awakening - Part 1: Escape" },
	{ "awakeningofthesphinx", "Awakening of the Sphinx" },
	{ "awalkindatomb", "A walk in da tomb" },
	{ "awalkinthepark", "A Walk in the Park" },
	{ "awayinatower", "Away in a Tower" },
	{ "awesmoequest", "Awesmoe Quest" },
	{ "awesomequest1", "Awesome Quest 1" },
	{ "awkward", "Ok...Now this is awkward!" },
	{ "awomanforallseasons", "A Woman for All Seasons" },
	{ "axmasevetale", "A Xmas Eve Tale" },
	{ "bachdead", "Bach's Dead" },
	{ "bachelorstory", "Bachelor Story" },
	{ "backdoorman", "Back Door Man" },
	{ "backlot", "Backlot: Adventure Antics" },
	{ "badbunker", "Bad Bunker" },
	{ "badluck", "Bad Luck" },
	{ "bakeoffitalia", "Bake Off Italia - The Graphic Adventure" },
	{ "baldysadventure", "Baldy's Adventure" },
	{ "balloonface", "Balloon Face" },
	{ "balls", "Balls" },
	{ "baltazarthefamiliar", "Baltazar the Familiar" },
	{ "bananaman", "Banana Man" },
	{ "bananaracer", "Banana Racer" },
	{ "barahir", "Barahir's Adventure: Askar's Castle" },
	{ "barelyfloating", "Barely Floating" },
	{ "barhoppers", "BarHoppers" },
	{ "barndilemma", "Barn Dilemma" },
	{ "barnrunner0", "Barn Runner 0: The Case of the Runaway Robot" },
	{ "barnrunner1p1", "Barn Runner 1: The Armageddon Eclair (Part 1)" },
	{ "barnrunner1p2", "Barn Runner 1: The Armageddon Eclair (Part 2)" },
	{ "barnrunner3", "Barn Runner 3: Don't Jerk The Trigger of Love" },
	{ "barnrunner4", "Barn Runner 4: The Prick Who Came In From the Cold" },
	{ "barnrunner5p1", "Barn Runner 5: The Forever Friday (Part 1)" },
	{ "barnrunner5p2", "Barn Runner 5: The Forever Friday (Part 2)" },
	{ "barnrunner5p3", "Barn Runner 5: The Forever Friday (Part 3)" },
	{ "barnrunner5p5", "Barn Runner 5: The Forever Friday (Part 5)" },
	{ "barnrunnerbake1", "Barn Runner Bake Sale 1: The Rich Dame Who Cut The Cheese" },
	{ "barnrunnerfashion", "Barn Runner: Fashionably Late" },
	{ "barnrunnerhall1", "Barn Runner Halloween 1: Fully Automatic Mojo" },
	{ "barnrunnervalentine1", "Barn Runner Valentine 1: Pucker Factor" },
	{ "barnrunnervn1", "Barn Runner Visual Novel 1: The Mayor's New Dress" },
	{ "barnrunnerxmas0", "Barn Runner Xmas 0: Christmas Soup" },
	{ "barnrunnerxmas2", "Barn Runner Xmas 2: Wreck The Halls" },
	{ "barrier", "Barrier" },
	{ "barrunner", "Ed Watts: Bar Runner" },
	{ "bartolomeo", "Bartolomeo, misled by circumstances, learns that appearances can be deceptive" },
	{ "bartsquestfortv", "Bart's Quest For TV" },
	{ "basedon", "Basedon - The Game" },
	{ "basementwoods", "Basement in the Woods" },
	{ "basketqust", "Interesting Basket Qust" },  // not a typo
	{ "battlewarriorsrt", "Battle Warriors: Rovendale Tactics" },
	{ "bbcscreensaver", "Background Blitz Collection Screensaver" },
	{ "bcremake", "Black Cauldron Remake" },
	{ "beachcomber", "Beachcomber" },
	{ "beacon", "Beacon" },
	{ "bear", "Bear Story" },
	{ "beardbeverage", "Beard Beverage" },
	{ "beardychin", "Old Woman Beardychin and the Scruffedy Bumtious" },
	{ "bearinvenice", "Bear in Venice" },
	{ "beasts", "Beasts" },
	{ "beatthebuzzer", "Beat the Buzzer" },
	{ "beautiesandbeasts", "Beauties and Beasts" },
	{ "becalmed", "Becalmed" },
	{ "beepboop", "Beep Boop" },
	{ "beforethedarkcrystal", "Before the Dark Crystal" },
	{ "beforethedarkcrystal2", "Before the Dark Crystal II" },
	{ "beforeww2", "Before WW2" },
	{ "bellyofthebeast", "Belly of the Beast" },
	{ "belowzero", "Below Zero" },
	{ "belusebiusarrival", "Belusebius Arrival" },
	{ "benchandlerpi", "Ben Chandler: Paranormal Investigator!" },
	{ "bentheredanthat", "Ben There, Dan That!" },
	{ "beprepared", "Be Prepared / Bodi Pripravljen" },
	{ "berrybattalion", "The Berry Battalion" },
	{ "berthabuttsboogie", "Bertha Butt's Boogie" },
	{ "besieged", "BESIEGED, Or: How to Get Out of A Castle... Without Being Catapulted" },
	{ "bestbuddiescorp", "Best Buddies in Corporate Espionage" },
	{ "bestowersofeternity", "Bestowers of Eternity - Part One" },
	{ "betrayal", "Betrayal" },
	{ "betweenpillars", "Between the Pillars of Creation" },
	{ "beyondeternity1", "Beyond Eternity - episode 1: The West College Disappearances" },
	{ "beyondhorizon", "Beyond Horizon" },
	{ "beyondreality", "Beyond Reality" },
	{ "beyondterror", "Beyond Terror" },
	{ "beyondthedoor", "Beyond the door" },
	{ "beyondthehorizon", "Beyond the horizon" },
	{ "bicschristmastale", "Bic's Christmas Tale" },
	{ "bigbadwolf3lilpiggies", "The Big Bad Wolf & Three little pigs" },
	{ "bigblue", "Big Blue World Domination" },
	{ "bigfoot", "Bigfoot" },
	{ "bigglesonmars", "Biggles On Mars" },
	{ "bigtroubleinlittleimola", "Big Trouble in Little Imola" },
	{ "billdebill", "Bill DeBill" },
	{ "billybstar", "Billy B. Star and the Lapaset Travel Quest" },
	{ "billyboysimportantwinelottery", "Billy Boy's Important Wine Lottery" },
	{ "billygoatsgruff", "Billy Goats Gruff" },
	{ "billymasterswasright", "Billy Masters Was Right" },
	{ "billythekid", "The New Adventures Of Billy The Kid" },
	{ "bioluminescence", "Bioluminescence" },
	{ "bird", "bird" },
	{ "birdsandbees", "Of birds and bees" },
	{ "birdybirdy", "Birdy Birdy" },
	{ "bitstream", "Bitstream" },
	{ "bittersweet", "Bittersweet" },
	{ "biwa", "Biwa of Blood" },
	{ "bjcase1", "Ben Jordan P.I. Case 1: In Search of the Skunk-Ape" },
	{ "bjcase2", "Ben Jordan P.I. Case 2: The Lost Galleon of the Salton Sea" },
	{ "bjcase3", "Ben Jordan P.I. Case 3: The Sorceress of Smailholm" },
	{ "bjcase4", "Ben Jordan P.I. Case 4: Horror at Number 50" },
	{ "bjcase5", "Ben Jordan P.I. Case 5: Land of the Rising Dead" },
	{ "bjcase6", "Ben Jordan P.I. Case 6: Scourge of the Sea People" },
	{ "bjcase7", "Ben Jordan P.I. Case 7: The Cardinal Sins" },
	{ "bjcase8", "Ben Jordan P.I. Case 8: Relics of the Past" },
	{ "bjcults", "Ben Jordan and the dark cults" },
	{ "bjpww1", "Ben Jordan P.W.W. Case 1: Wrath of the Skunk Ape" },
	{ "blackbirdpie", "Blackbird Pie" },
	{ "blackbirdstrikesback", "The Blackbird Strikes Back" },
	{ "blackfriday", "Black Friday" },
	{ "blackhandgang", "The Adventures of the Black Hand Gang" },
	{ "blackmailinbrooklyn", "Blackmail in Brooklyn" },
	{ "blackmorph", "Black Morph" },
	{ "blacksect1", "Black Sect Remake" },
	{ "blacksect2", "Black Sect 2: The Cursed Crypt - Remake" },
	{ "blackudder", "Blackudder: To Doubloon or not to Doubloon" },
	{ "bladespassion", "Blades of Passion: An Oceanspirit Dennis Adventure" },
	{ "blazeos", "Blaze From Outer Space" },
	{ "blindsweeper", "BlindSweeper" },
	{ "blindtosiberia", "Blind to Siberia" },
	{ "blitheep1", "Blithe - Episode 1: The quiet town" },
	{ "blobward", "The Blob Game" }, // aka Blobward
	{ "blockz", "Blockz - A Slider Puzzle Game" },
	{ "bloodedfields", "Blooded Fields" },
	{ "bloodyscream", "Bloody Scream" },
	{ "bluelobe", "Blue Lobe Inc." },
	{ "bluemoon", "Blue Moon" },
	{ "boardquest", "Board Quest" },
	{ "bob", "Bob" },
	{ "bobbycopper", "Bobby Copper" },
	{ "bobescapes", "Bob Escapes / Bob En Cavale" },
	{ "bobgoeshome", "Bob Goes Home" },
	{ "bobgoeshomedeluxe", "Bob Goes Home Deluxe" },
	{ "bobsquest1", "Bob's Quest" },
	{ "bobsquest2024", "Bob's Quest (2024)" },
	{ "bogsadventure", "Bog's Adventures in the Underworld" },
	{ "bogsadventureineasy3d", "Bog's Adventure in Easy3D" },
	{ "bohemianyard", "Bohemian Yard" },
	{ "boilerroom", "Boiler Room Blues" },
	{ "boltaction", "Bolt Action" },
	{ "bone", "Bone" },
	{ "bonest", "Beneath Our Next Establishment" },
	{ "boogiebum", "Boogie Bum's Roger Quest" },
	{ "bookofdestiny", "Book of Destiny" },
	{ "bookofknowledge", "The Books of Knowledge" },
	{ "bookofspells4", "The Book of Spells (A funtasy Adventure) - Chapter 4" },
	{ "bookofspellscomplete", "The Book of Spells (A funtasy Adventure)" },
	{ "bookunfinished", "The Book Unfinished" },
	{ "bovinebyproduct", "Bovine By-Product" },
	{ "bowanddork", "Bow and Dork" },
	{ "bowserquirkquest", "Bowser Quirk Quest" },
	{ "box", "Box" },
	{ "boxfight", "Boxfight for AGS" },
	{ "boxland", "Think Outside the Boxland" },
	{ "boyindahood", "Boy in da hood: Give me the money!" },
	{ "bradbradsonkeyquest", "Brad Bradson: Key Quest" },
	{ "braquagegringotts", "Braquage à Gringotts" },
	{ "breakage", "Breakage" },
	{ "breakdown", "Breakdown" },
	{ "breakfastont1", "Breakfast on Trappist-1" },
	{ "breakingcharacter", "Breaking Character" },
	{ "breakofdawn", "At the Break of Dawn" },
	{ "brexitman", "Brexit-Man in Airportland: The Rebrexiting" },
	{ "brinescene", "Brine Scene Finvestigation" },
	{ "brokenwindows1", "Broken Windows - Chapter 1" },
	{ "brokenwindows2", "Broken Windows - Chapter 2" },
	{ "brokenwindows3", "Broken Windows - Chapter 3" },
	{ "brokenwindows4", "Broken Windows - Chapter 4" },
	{ "brotherswreckers", "Brothers & Wreckers" },
	{ "brotherswreckersep7", "Brothers & Wreckers - Episode 7" },
	{ "brucequest", "Bruce Quest: The Secrets of the Outback" },
	{ "bruises", "Bruises" },
	{ "bsg78", "BSG78 - Unexpected at the Rising Star" },
	{ "bubblewrap", "Bubble Wrap Popping Simulator 2013" },
	{ "bubsybobcat", "Bubsy The Bobcat In Rip Van Bubsy Starring Bubsy" },
	{ "buccaneer", "Buccaneer" },
	{ "buddiestuniverse", "Buddiest Universe - Day Off" },
	{ "buddybrick", "Buddy Brick" },
	{ "buhf", "Buhf" },
	{ "bullettrain", "Bullet Train" },
	{ "bunawantsbeer", "Buna Wants Beer" },
	{ "bunnynightout", "Bunny's Night Out" },
	{ "bunnyquest", "Bunny Quest" },
	{ "burrow", "Burrow" },
	{ "burymeinthesand", "Bury Me in the Sand" },
	{ "bustinthebastille", "Bustin' the Bastille" },
	{ "butcherstanys", "Butcher Stanys" },
	{ "butcherstanys2", "Butcher Stanys II: Stanys Meets Marilyn Manson" },
	{ "buttercup", "Buttercup's Gone Missing!" },
	{ "byohero", "BYO-Hero" },
	{ "bytebetrayal", "Byte-Sized Betrayal" },
	{ "bythenumbers", "By the Numbers" },
	{ "bytheswordconspiracy", "By the Sword: Conspiracy" },
	{ "byzantine", "Byzantine" },
	{ "cabbagequest", "Cabbage Quest" },
	{ "cabbagesandkings", "Cabbages and Kings" },
	{ "cabintrouble", "Cabin Trouble" },
	{ "cachoquest", "Cacho Quest" },
	{ "cadaverheart", "Cadaver Heart" },
	{ "calebsdrunkenadventure", "Caleb's Drunken Adventure" },
	{ "calequest", "Cale Quest" },
	{ "callmegeorge1", "Call Me George, Chapter 1: Prophecy" },
	{ "calsoon2", "Calsoon 2: The Legend of the Looncalsoon" },
	{ "calvin", "Calvin" },
	{ "camp1", "Camp 1" },
	{ "campaigndaphnewhite", "The Campaign of Daphne White" },
	{ "cancer", "Cancer" },
	{ "candlecove", "Return to Candle Cove" },
	{ "candyforest", "The Candy Forest" },
	{ "capricorn", "Tropic of Capricorn" },
	{ "captaincringe", "Chronicles of Captain Cringe" },
	{ "captaindisastermoon", "Captain Disaster In: The Dark Side of the Moon" },
	{ "captaindownes", "Captain Downes and the Pirate Princess" },
	{ "captainhook", "Captain Hook and the Lost Girl" },
	{ "captainskull", "The Astonishing Captain Skull" },
	{ "carnivalshadows", "Carnival of Shadows" },
	{ "carrotbobinzxspeccyworld", "Carrot Bob in ZX Spectrum world" },
	{ "cartenstein", "Cartenstein" },
	{ "cartlife", "Cart Life" },
	{ "casablancathedayafter", "Casablanca, The Day After" },
	{ "casenoir", "Case Noir" },
	{ "caseofthefestivalfilcher", "The Case of the Festival Filcher" },
	{ "caseofthemuffindiver", "The case of The Muffin Diver" },
	{ "cassandra", "Cassandra" },
	{ "castleescapech1", "Castle Escape - Chapter 1" },
	{ "castleoffire", "Castle of Fire" },
	{ "catacombic", "Catacombic" },
	{ "catapault", "CATapault" },
	{ "catking", "Long Live the Cat King" },
	{ "cauche", "Cauchemarionto" },
	{ "caverns", "Caverns" },
	{ "caveofavarice", "Cave of Avarice" },
	{ "cayannepepper", "Cayanne Pepper" },
	{ "cedricandtherevolution", "Cedric and the Revolution" },
	{ "cedricshooter", "Cedric Shooter" },
	{ "ceelo", "Cee-Lo!" },
	{ "celestialcatastrophe", "Danny Sexbang in Celestial Catastrophe" },
	{ "celticchaosep1", "Celtic Chaos Episode 1: Cold mead" },
	{ "celticchaosep2", "Celtic Chaos Episode 2: Fishermen's fiends" },
	{ "cgascreensaver", "CGA Games Screensaver" },
	{ "chaelle1", "Chaëlle chapitre 1 : Mission Dragon" },
	{ "chaelle2", "Chaëlle chapitre 2 : Chimérie" },
	{ "chaelle3", "Chaëlle chapitre 3 : La Bataille de Zahrasie" },
	{ "chalkman", "Chalkman" },
	{ "chalksquest", "Chalk's Quest" },
	{ "challengetentacle", "Challenge of the Tentacle" },
	{ "chaluul", "Chaluul's Curse Remake" },
	{ "chanceofthedead", "Chance Of The Dead" },
	{ "charamba1", "Charamba, Chapter 1: Bitches in the Desert" },
	{ "charamba2", "Charamba, Chapter 2: Hallowe'en" },
	{ "charliefoxtrot", "Charlie Foxtrot and The Galaxy of Tomorrow" },
	{ "chasingrobot", "Chasing Robot" },
	{ "chatroom", "Chatroom" },
	{ "cheerfulscience", "Cheerful Science" },
	{ "cheetahs", "Cheetahs Are Meant To Be Fast" },
	{ "chekken", "cheKKen" },
	{ "cherrysquest", "Cherry's Quest For Coffee" },
	{ "chessboard", "ChessBoard" },
	{ "chezapa", "Chez Apa" },
	{ "chickchaser", "Chick Chaser" },
	{ "chicken", "Chicken" },
	{ "chickenfraction", "Chicken Fraction" },
	{ "chickenvsroad", "Chicken vs. Road" },
	{ "chinesecheckers", "Chinese Checkers" },
	{ "chlorinde", "White Bear Beauty Chlorinde and the Paranormal Parrot" },
	{ "chmelnica", "The Hop Garden / Chmelnica" },
	{ "chocofrogs", "ChocoFrogs" },
	{ "chongoadv", "Chongo's China Adventure" },
	{ "chriscolumbus", "Christopher Columbus is an Idiot" },
	{ "christmas42", "Christmas42" },
	{ "christmasgamearcade", "CGA: Christmas Game Arcade" },
	{ "christmashunt", "Christmas Hunt" },
	{ "christmaspresent", "Search for the Christmas Present - Remake" },
	{ "christmasquest", "Christmas Quest" },
	{ "christmasquest2", "Christmas Quest 2: The Yuletide Flows In" },
	{ "christmasquest3", "Christmas Quest 3: Santa's Little Help Desk" },
	{ "chtonic", "Chtonic" },
	{ "chuckisland", "Chuck and the Island of Giants" },
	{ "cirquedezale", "Cirque de Zale" },
	{ "city", "City" },
	{ "cityofthieves", "City of Thieves: Rescue Sandy" },
	{ "citythatdrowned", "The City That Drowned" },
	{ "claire", "Claire" },
	{ "clarinette", "Clarinette" },
	{ "classnotes", "Class Notes" },
	{ "clik", "Clik" },
	{ "clipgoestotown", "Clip goes to town" },
	{ "clockworklabyrinth", "The Clockwork Labyrinth" },
	{ "clotildesoffritti", "Clotilde Soffritti in: Never Buy a Used Spaceship" },
	{ "clotildesoffritti2", "Clotilde Soffritti in: Never Double Park your Spaceship" },
	{ "clownatthecircus", "Clown at the Circus" },
	{ "clownshow", "Clown Show" },
	{ "clubmidget", "Club Midget" },
	{ "clubofevil", "Club of Evil" },
	{ "coderbattle", "CODERBATTLE - quest for the whole game -" },
	{ "coelldeckaflight", "Coell Decka Flight" },
	{ "coffinvalley", "Coffin Valley" },
	{ "coinopafternoon", "Coin-Operated Afternoon" },
	{ "coinrush", "Coin Rush" },
	{ "coinrush2", "Coin Rush 2" },
	{ "colaskunk", "Cola Skunkette: A Bit of Cleanup" },
	{ "coldhandreef", "The Cold Hand Reef" },
	{ "coldmeat", "Cold Meat" },
	{ "coldstorage", "Cold Storage" },
	{ "colinsimpson", "Colin Simpson Leaves Employment" },
	{ "colonelcarver", "Colonel Carver's Carnival Curios" },
	{ "colourclash", "Colour Clash" },
	{ "colourwise", "ColourWise" },
	{ "colourwiseleveleditor", "ColourWise - Level Editor" },
	{ "columbuslander", "Columbus Lander" },
	{ "comedyquest", "Comedy Quest" },
	{ "cometcollision", "Comet Collision" },
	{ "comiclassic", "The Curse of Monkey Island: Classic Edition" },
	{ "comicsquest", "Comics Quest" },
	{ "commandoadv", "Commando: The Adventure Game" },
	{ "comradecitizenpart1", "Comrade Citizen - Part I" },
	{ "concurrence", "Concurrence" },
	{ "confessionsofacatburglar", "Confessions Of A Cat Burglar" },
	{ "confinement", "Confinement" },
	{ "coniferskunks", "Conifer Skunks: Osvald's Surprise" },
	{ "conspiracybelowzero", "Conspiracy: Below-Zero" },
	{ "conspiracyofsongo", "Conspiracy of Songo" },
	{ "constancethebarbarian", "Constance the Barbarian" },
	{ "contact", "Contact" },
	{ "content", "Content" },
	{ "contrapasso", "Commissar's Contrapasso" },
	{ "cop", "Cop" },
	{ "corneliuschristmas", "Cornelius Cat in: How the Cat Saved Christmas" },
	{ "corneliuspest", "Cornelius Cat in: The Uncontrollable Pest" },
	{ "cornersshiny", "Corner's Shiny (2013 edition)" },
	{ "corpoconnections", "Corpo Connections" },
	{ "cosmodyssey", "Cosmodyssey: Jammy Edition" },
	{ "cosmospuzzle", "Cosmos Puzzle" },
	{ "cosmosquest1", "Cosmos Quest I" },
	{ "cosmosquest2", "Cosmos Quest II" },
	{ "cosmotrinket", "Cosmo's Trinket Hunt" },
	{ "cougarisland", "The Secret of Cougar Island" },
	{ "cougarsquestforfreedom", "Cougar's Quest for Freedom" },
	{ "counterfeit", "Counterfeit" },
	{ "coupdecup", "Coup de Cup" },
	{ "coyote1", "Coyote Episode I: The Mexican" },
	{ "crackwell1", "The Crackwell Legacy" },
	{ "crackwell2", "Crackwell Unhinged" },
	{ "craftofevil", "Craft Of Evil" },
	{ "crankosaurus", "Crankosaurus Prime and the Blue Crystal Pursuit" },
	{ "crankosaurusff", "Crankosaurus Prime and the Blue Crystal Pursuit: The Fangs of Fortune" },
	{ "crashcourse", "Crash Course" },
	{ "crashedalien", "The Story Of The Alien That Crashed" },
	{ "crashevadedestroy", "Crash! Evade! Destroy!" },
	{ "crave", "Crave" },
	{ "crepefields", "Crepe Fields: A Scare Among Crows" },
	{ "crimezone", "Crime Zone" },
	{ "criminalist", "Criminalist" },
	{ "crimmsson", "Crimm's Son" },
	{ "crossstitch", "Cross Stitch Casper" },
	{ "crowandfoxy", "Crow and Foxy / Le Corbeau et la Renarde" },
	{ "crowcawled", "A Crow Cawled Raven" },
	{ "cryo", "Cryo" },
	{ "crypt", "Crypt" },
	{ "cryptic", "Cryptic" },
	{ "crystalquest", "Crystal Quest" },
	{ "crystalstory", "A Little Christmas Trial: Crystal's Story" },
	{ "csihunt1", "CSI Hunt 1" },
	{ "cspb", "Cake & Smurphy Puzzle Battle!" },
	{ "cullfield", "The Cullfield Investigation" },  // aka Detective Ian Cullfield
	{ "cursevampire", "The Curse of the Vampire" },
	{ "cutman", "Cutman" },
	{ "cyberjack", "cyberJACK" },
	{ "cybermemory", "Cybermemory Not Found" },
	{ "cyberpunk97ep1", "Cyberpunk '97 - Episode 1" },
	{ "daceyinthedark", "Dacey in the Dark: Prelude" },
	{ "dada", "Dada: Stagnation in Blue" },
	{ "dadaxmas", "Dada Christmas Special: Stagnation in Red and White" },
	{ "daggerhell", "DAGGERHELL" },
	{ "dakota", "Dakota" },
	{ "daleks", "Daleks" },
	{ "dalesfilmquest", "Dale's Film Quest" },
	{ "damsel", "Damsel - Chapter 1: Stress on the Tress" },
	{ "danbeard", "Dan Finds His Beard" },
	{ "dancetilyoudrop", "Dance Til' You Drop!" },
	{ "danewguys", "Da New Guys" },
	{ "dangermouse", "Danger Mouse" },
	{ "dangerousderek", "Dangerous Derek's Livestream" },
	{ "dangerouslandsrt2", "Dangerous Lands: Rovendale Tactics 2" },
	{ "dannydreadisoncall", "Danny Dread is On Call" },
	{ "dantesday", "Dante's Day" },
	{ "darkblack", "Dark Black" },
	{ "darkdenim", "Darkness and Denim" },
	{ "darkforce", "DarkForce: Peace Among Nations" },
	{ "darkofnight", "Dark of Night" },
	{ "darkroom", "Dark Room" },
	{ "darktimesmerrychristmas", "Dark Times (Merry Christmas)" },
	{ "darum", "Darum" },
	{ "davegeneric", "Dave Generic" },
	{ "davidcriatividade", "David A Procura Da Criatividade" },
	{ "davidletterman", "David Letterman - the Video Game" },
	{ "davyjonesspellbook", "Davy Jones' Spellbook" },
	{ "dawnswonderedatagesend", "Dawns Wondered: At Age's End" },
	{ "dayofthefish", "Day of the Fish" },
	{ "ddddd", "DDDDD: The Draft Drifter Who Dashed Doctor Dunno" },
	{ "ddr", "Dennis Dennis Revolution" },
	{ "deaddimension", "Dead Dimension" },
	{ "deadgods", "Dead Gods: Haizara 2" },
	{ "deadhand", "Dead Hand" },
	{ "deadinspace", "Dead in Space" },
	{ "deadlyconsequences", "Deadly Consequences!" },
	{ "deadmanpoliticalparty", "Dead Man's Political Party" },
	{ "deadofwinter", "Dead of Winter" },
	{ "deadphones", "Dead Phones" },
	{ "deadpixels", "Dead Pixels" },
	{ "deadroom", "Dead Room" },
	{ "deadsilence", "Dead Silence" },
	{ "deadstar", "Dead Star" },
	{ "deadtoilet", "Deadman Toilet" },
	{ "dearrgh", "DeARRGH!-dre & Shaqushia" },
	{ "deathandtransfiguration", "Death and Transfiguration" },
	{ "deathasitis", "Death as it Is" },
	{ "deathep1", "Death - Episode One: The scythe of unlimited power" },
	{ "deathofanangel", "Death of an Angel" },
	{ "deathofdavyjonesscenario", "Yet Another Death of Davy Jones Scenario" },
	{ "deathonstage", "Death on Stage" },
	{ "deathsdoor", "Death's Door" },
	{ "deathsquest", "Deat(h)'s Quest" },
	{ "deathworeendlessfeathersdisk1", "Death Wore Endless Feathers Disk 1" },
	{ "deckhex", "DeckHex" },
	{ "deephope", "Deep Hope" },
	{ "deeplakemall", "Deep Lake Mall" },
	{ "deepspacemission", "Deep Space Mission: Are we alone?" },
	{ "deflus", "Deflus" },
	{ "dehaunt", "Dehaunt" },
	{ "deity", "Deity" },
	{ "delerium", "Delerium" },
	{ "dellamorte", "Dellamorte Dellamore" },
	{ "demogame", "JWB Games Demo Game" },
	{ "demonday", "Demon Day" },
	{ "demonicdollhouse", "The Demonic Dollhouse" },
	{ "demonslayer5", "Demon Slayer 5" },
	{ "demonstohell", "All Demons Must Go To Hell" },
	{ "depressomatic", "Depressomatic" },  // aka FSi's Crap Game
	{ "derrekquest", "Derrek Quest I: Lost in the desert" },
	{ "derverschwundenehusky", "Der verschwundene Husky" },
	{ "desertminer", "Desert Miner" },
	{ "desmond", "Desmond: The 'Thing' from another world!" },
	{ "desolate", "Desolate" },
	{ "detectivebhmini", "Detective Boiled-Hard Mini Case" },
	{ "detectiveobriced", "Detective O.Briced" },
	{ "detectiverizal", "Detective Rizal and the Jaded Ruby" },
	{ "detention", "Detention!" },
	{ "deusexmachina", "Deus Ex Machina" },
	{ "devilgotwoman", "The Devil Got My Woman" },
	{ "devochkaquest", "Devochka Quest" },
	{ "dexter", "Dexter Morning Routine" },
	{ "dgsearchbatteries", "Another DG game: the search of the batteries" },
	{ "dicklarenzo", "Dick LaRenzo: Secret Agent!" },
	{ "diemaskennyarlathoteps", "Die Masken Nyarlathoteps" },
	{ "digilawyer", "DigiLawyer" },
	{ "dimetrodon", "Day of the Dimetrodon" },
	{ "dinnerforpigeons", "Dinner for Pigeons" },
	{ "dirandiouskroken", "Dirandious Kroken" },
	{ "disappearancetime", "007 ¾: Disappearance Time" },
	{ "discoflow", "DISCO FLOW" },
	{ "disgust", "Disgust" },
	{ "dislocation", "Dislocation" },
	{ "disquiet", "Disquiet" },
	{ "distancenoobject", "Distance no object" },
	{ "djdeceased", "D. Jones: Deceased" },
	{ "dlistdiva", "D-List Diva" },
	{ "dobutsu", "Dobutsu no Costco" },
	{ "doctormaze", "The Amazing Doctor Maze" },
	{ "doctormuttonchop", "Doctor Muttonchop" },
	{ "doctorwho", "Doctor Who: Time Snare" },
	{ "doctorzoo", "The Everyday Adventures of the Evil Dr. Zoo" },
	{ "dogescape", "Dog Escape" },
	{ "dolldream", "A doll in a dream" },
	{ "dollshouse", "dolls house" },
	{ "dollshouseinsanity", "dolls house: INSANITY" },
	{ "domesticgoddess", "Little Mrs. Domestic Goddess" },
	{ "dommep1", "Day of Maniac Mansion - Ep. 1: Les aventuriers des grottes perdues" },
	{ "donalddowell", "Donald Dowell and the Ghost of Barker Manor" },
	{ "donkeybas", "Donkey.Bas AGS Remake" },
	{ "donnaavengerofblood", "Donna: Avenger of Blood" },
	{ "donniedarko", "Donnie Darko - The Adventure Game" },
	{ "donspillacyconspiracyquest", "Don Spillacy's Conspiracy Quest" },
	{ "dontdrinkthepink", "Don't Drink the Pink" },
	{ "dontgiveupcat", "Don't Give Up the Cat" },
	{ "donthedweebdancedilemma", "Don the Dweeb: Dance Dilemma" },
	{ "donticeyourcool", "Don't Ice Your Cool" },
	{ "dontlook", "Don't Look!" },
	{ "dontpush", "Don't Push The Button" },
	{ "dontworrybaby", "Don't Worry Baby" },
	{ "dontworryillbringthebeer", "Don't Worry, I'll Bring The Beer!" },
	{ "doomgraphicadventure", "Doom: A Very Graphic Adventure" },
	{ "doors", "Doors" },
	{ "doses", "Doses" },
	{ "doth", "Day of the Hamster" },
	{ "dots", "Day of the Sandwich" },
	{ "dovadulasburn", "DoVaDuLa'S BuRn" },
	{ "draconis", "Draconis" },
	{ "draculahousemystery", "Dracula's Mid-Century-Modern House of Mystery" },
	{ "draculator2", "Draculator II: Byte of the Draculator" },
	{ "dragonorb", "Dragon Orb" },
	{ "dragonscale", "Dragonscale: The Encounter" },
	{ "dragonsfang", "Dragon's Fang" },
	{ "dragonslayers", "The Dragon Slayers" },
	{ "dragontales", "Dragon Tales" },
	{ "dragoonjourney", "Dragoon's Journey" },
	{ "drawnflame", "Drawn to the Flame" },
	{ "drchuckles", "Dr. Chuckles' Miniature World Of Madness" },
	{ "dreadmacfarlane", "Dread Mac Farlane" },
	{ "dreadmacfarlane2", "Dread Mac Farlane - Part 2" },
	{ "dreadmacfarlaneapprentie", "Dread Mac Farlane, apprentie pirate" },
	{ "dreadmacfarlanefils", "Dread Mac Farlane - Le Fils du Pirate" },
	{ "dreadmacfarlanev2ep1", "Dread Mac Farlane V2 (Remake) - Episode 1" },
	{ "dreadmacfarlanev2ep2", "Dread Mac Farlane V2 (Remake) - Episode 2" },
	{ "dreadmacfarlanev2ep3", "Dread Mac Farlane V2 (Remake) - Episode 3" },
	{ "dreadmacfarlanev2ep4", "Dread Mac Farlane V2 (Remake) - Episode 4" },
	{ "dreadmacfarlanev2ep5", "Dread Mac Farlane V2 (Remake) - Episode 5" },
	{ "dreadmacfarlanev2ep6", "Dread Mac Farlane V2 (Remake) - Episode 6" },
	{ "dreadmacfarlanev2ep7", "Dread Mac Farlane V2 (Remake) - Episode 7" },
	{ "dreadmacfarlanev2ep8", "Dread Mac Farlane V2 (Remake) - Episode 8" },
	{ "dreadmacfarlanev2ep9", "Dread Mac Farlane V2 (Remake) - Episode 9" },
	{ "dreadmacfarlanev2ep10", "Dread Mac Farlane V2 (Remake) - Episode 10" },
	{ "dreadmacfarlanev2ep11", "Dread Mac Farlane V2 (Remake) - Episode 11" },
	{ "dreamagine", "Dreamagine" },
	{ "dreamcatadv", "Dreamcat Adventure: Beyond Dream and Time" },  // Jenseits von Traum und Zeit
	{ "dreamdiary", "Dream Diary Quest" },
	{ "dreamer", "Little Dreamer" },
	{ "dreamscapess", "Dreamscape: Silent Serenade" },
	{ "dreamsofwintermass", "Dreams of Winter Mass" },
	{ "dreamwithindream", "The Dream within A Dream" },
	{ "dreamychristmas", "Create your own adventure game: Your dreamy Christmas" },
	{ "dressedforafight", "Dressed for a Fight Out" },
	{ "drevil", "Dr. Evil" },
	{ "driftwalker", "Driftwalker" },
	{ "drillkiller", "DRILL KILLER" },
	{ "drlutztimetravelmachine", "Dr.Lutz Time Travel Machine" },
	{ "drmoby", "Dr. Moby: Meal Adventure" },
	{ "droi0", "DROI0" },  // aka DROID3
	{ "drugsandalians", "drugs and alians" },
	{ "drunkfredcell", "Drunk Fred in the Cell" },
	{ "dumbassdrivers", "Dumbass Drivers!" },
	{ "dungeonhands", "Dungeon Hands" },
	{ "duskhunters", "Dusk Hunters" },
	{ "dusttowater", "Dust to Water" },
	{ "dutyandbeyond", "Duty and Beyond" },
	{ "dutyfirst", "Duty First" },
	{ "duzzquest", "DuzzQuest: An Egotistic Adventure" },
	{ "duzzquest2", "DuzzQuest2" },
	{ "dysmaton", "Dysmaton" },
	{ "earlbobby1", "Earl Bobby is looking for his Shoes" },
	{ "earlbobby2", "Earl Bobby is looking for his Balls" },
	{ "earlbobby3", "Earl Bobby is looking for a Loo" },
	{ "earlmansinthebreakout", "Earl Mansin: The Breakout" },
	{ "earthad", "Earth AD: Low Estate" },
	{ "earthlingpriorities", "Earthling Priorities" },
	{ "earthstory", "Earth Story" },
	{ "easterencounter", "Easter Encounter" },
	{ "easterinron", "Easter in Reality-on-the-Norm" },
	{ "easterislanddefender", "Easter Island Defender" },
	{ "echidna", "Echidna Chwest" },
	{ "echoesinstatic", "Echoes in the Static" },
	{ "echoesofterra", "Echoes of Terra" },
	{ "echoesofthepast", "Echoes of the Past" },
	{ "echointheclouds", "Echo in the Clouds" },
	{ "edmund", "Edmund and the potato" },
	{ "eerieblue", "Eerie Blue" },
	{ "egoplanetapes", "Ego in Planet of the Apes" },
	{ "egress", "Egress - The Test of STS-417" },
	{ "eight", "8" },
	{ "eikos1", "La Légende d'Eikos chapitre 1 : Le Seigneur des Loups" },
	{ "eikos2", "La Légende d'Eikos chapitre 2 : Réveil dans la Nuit" },
	{ "electrokit", "Maaikes Elektrokit" },
	{ "elegantmurdermystery", "The Elegant Murder Mystery" },
	{ "elevation", "Elevation" },
	{ "elevatorriseabyss", "The Elevator - Rise from the Abyss!" },
	{ "elfer", "Elfer" },
	{ "elfmotorsinc", "Elf Motors Inc." },
	{ "elforescuecraby", "ELFO: Rescue Craby" },
	{ "elfthe4elements", "ELF: The 4 elements" },
	{ "elfthedarkness", "ELF: And soon the darkness..." },
	{ "eliminationbyimprovisation", "Elimination by Improvisation" },
	{ "elmowagon", "Elmo's Wagon Conflict" },
	{ "elvves", "Elvves" },
	{ "emeraldeyes", "Emerald Eyes" },
	{ "emilyenough", "Emily Enough: Imprisoned" },
	{ "emmaroide", "Emma Roide" },
	{ "empty", "Empty" },
	{ "emptymindblankfate", "Empty Mind - Blank Fate" },
	{ "encounters", "Encounters of the Closest Kind" },
	{ "endacopia", "Endacopia" },
	{ "endlessloop", "Endless Loop" },
	{ "enoworld", "Enoworld" },
	{ "enqueteolonnes", "Enquête au pays des Olonnes" },
	{ "enterthestory", "Enter The Story" },
	{ "entrapment", "Entrapment" },
	{ "entrapped", "Entrapped" },
	{ "epsteinislandp1", "The Mystery of Epstein Island - Part 1: Day" },
	{ "epsteinislandp2", "The Mystery of Epstein Island - Part 2: Sunset" },
	{ "equilibrium", "Equilibrium - Out of Time" },
	{ "erictheanteater", "Eric the Anteater" },
	{ "ericmegalith", "Eric And The Apostolic Megalith" },
	{ "erkrealestate", "Erk: Adventures in Stone Age Real Estate" },
	{ "errand", "Errand" },
	{ "escape", "Escape" },
	{ "escaped", "Escaped" },
	{ "escaped2", "Escaped 2" },
	{ "escapefromasmallroom", "Escape From a Small Room 1: The walls are closing in" },
	{ "escapefromevergreenforest", "Escape From Evergreen Forest" },
	{ "escapefromlurrilous", "Escape from Lurrilous" },
	{ "escapefromterrorbay", "Escape From Terror Bay" },
	{ "escapefromthechaoticcity", "Escape From The Chaotic City" },
	{ "escapefromthegarage", "ESCAPE from the garage" },
	{ "escapefromthesalemmoons", "Escape From The Salem Moons" },
	{ "escapefromthezombiecity", "Escape From The Zombie City" },
	{ "escapeledgeoctagon", "Escape The Ledge: Octagon" },
	{ "escapethebarn", "Escape the Barn" },
	{ "escapethebrothel", "Escape The Brothel" },
	{ "escapetheship", "Escape the ship" },
	{ "escapetocivilization", "Escape to Civilization" },
	{ "esper", "ESPER: The Town on the Edge of Darkness" },
	{ "essence", "Of the Essence" },
	{ "essenceofimagination", "Essence of Imagination" },
	{ "essia", "Essia" },
	{ "etaac", "Electronic Tax Administration Advisory Committee" },
	{ "etaitungeek", "Il était un Geek" },
	{ "eternalchrysalis", "Eternal Chrysalis" },
	{ "eternallyus", "Eternally Us" },
	{ "eternaltorpor", "Eternal Torpor / Sopor Eterno" },
	{ "eventtimer", "Event Timer" },
	{ "everlight", "Everlight Forest" },
	{ "evil", "Evil" },
	{ "evilenterprises", "Evil Enterprises" },
	{ "evillodge", "Evillodge: The Criminal Adventure Game" },
	{ "exclamation", "!" },
	{ "exit", "EXIT" },
	{ "exmachina", "Ex Machina" },
	{ "explorationa", "Exploration A" },
	{ "exposedreality", "Exposed Reality" },
	{ "everythingm", "Everything that Begins with an M" },
	{ "fadingshades", "Fading Shades" },
	{ "fakethemoonlanding", "Fake the moon landing" },
	{ "fall", "Fall" },
	{ "fallenangel", "Fallen Angel" },
	{ "fallenhero", "Fallen Hero" },
	{ "fallensoldier", "Fallen soldier" },
	{ "fallingdark", "Falling Dark" },
	{ "fallingdark2", "Falling Dark 2: Relapse" },
	{ "fallingskywards", "Falling Skywards" },
	{ "familieherfurth", "Der neue Wahnsinn der Familie Herfurth" },
	{ "fanbots", "Fanbots" },
	{ "fantasymotus", "Fantasy Motus" },
	{ "farcorners1", "The Far Corners of the World: Chapter 1 - The Book, the Box and the Key" },
	{ "farnowhere", "FAR NOWHERE" },
	{ "fashiongirl", "Fashion Girl" },
	{ "fasmo", "Fasmo!" },
	{ "fasmogoeswest", "Fasmo 2: Fasmo Goes West" },
	{ "fayeking", "Faye King: Jungle Jeopardy" },
	{ "fbiquest", "FBI Quest" },
	{ "fearaphobia", "Fearaphobia" },
	{ "featherweight", "Featherweight" },
	{ "femspray", "FemSpray" },
	{ "fengshuitv", "Feng Shui And The Art Of TV Reception" },
	{ "ferra", "Neurolision: Ferra" },
	{ "ferragosto", "Elia, Ilaria & the Kids Having a Good Time at the Sea (or at least they try)" },
	{ "feuersturm1", "Feuersturm - Kapitel 1: Zurück in die welt" },
	{ "feuersturm2", "Feuersturm - Kapitel 2: Der unheimliche Zug" },
	{ "feuersturm3", "Feuersturm - Kapitel 3: Wo Der Wald Beginnt..." },
	{ "fhaloness", "Fhaloness" },
	{ "fibbersummer", "Fibber's Summer" },
	{ "fifa2004", "FIFA International Football 2004" },
	{ "fightforlife", "Fight for Life" },
	{ "finger", "Finger of suspicion" },
	{ "fireflystory3d", "Firefly story 3D" },
	{ "firstdrop", "1st Drop" },
	{ "firststitch", "The Tapestry - Prologue: The First Stitch" },
	{ "fistsofmurder", "Fists of Murder" },
	{ "fixer", "Fixer" },
	{ "fixumdude", "Fixumdude's 3D Printing Adventure" },
	{ "flamebarrels", "Flame Barrels" },
	{ "flashbax", "Flashbax" },
	{ "flashdown", "Flash Down" },
	{ "flashmccoy", "Flash McCoy" },
	{ "flightrobots", "Flight from the Robots" },
	{ "flightrobotsch2", "Flight from the Robots - Chapter 2" },
	{ "floatyrog", "Floaty Rog'" },
	{ "flophouse", "Flophouse Hijinks" },
	{ "flowergirl", "Flower Girl" },
	{ "flukie", "Flukie" },
	{ "fluxworld", "Flux World" },
	{ "flyingthinker", "Flying Thinker" },
	{ "flypaper", "Fly Paper" },
	{ "focality", "Focality" },
	{ "foggydawn", "Foggy Dawn" },
	{ "foggynotions", "Foggy Notions" },
	{ "foodwars", "Food Wars" },
	{ "foolaround", "Fool Around" },
	{ "forcemajeureiithezone", "Force majeure II: The Zone" },
	{ "forest", "Forest" },
	{ "forestdweller", "Forest Dweller 3D" },
	{ "forfrogssake", "FOR FROGS SAKE! GET THE FROG OUT!" },
	{ "forge", "Forge: Chapter One" },
	{ "forgerecap", "Forge: Loom Recap" },
	{ "forgettendeath", "Forgetten Death" },
	{ "forms", "Forms" },
	{ "fortressofwonders", "Fortress of Wonders" },
	{ "foundations", "Foundations" },
	{ "fountainofyouth", "Indiana Jones and the Fountain of Youth" },
	{ "four", "4" },
	{ "foyfighter", "Indiana Jones and the Fountain of Youth: Arcade Fighter" },
	{ "fragile", "Fragile" },
	{ "framed", "Framed!" },
	{ "frameonthewall", "The Frame on the Wall" },
	{ "frankenpooper", "Frankenpooper" },
	{ "frankfurter1", "The Adventures of Stanley 'Frankfurter' Jones" },
	{ "frankfurter2ch1", "Frank Further: The Further Adventures of Stanley 'Frankfurter' Jones - Chapter I" },
	{ "frankmalone", "The Return of Frank Malone" },
	{ "frankstallone", "Frank Stallone: The Driver and Mob Enforcer" },
	{ "frankthefarmhandpart1", "Frank the Farmhand - Part 1: The Big Escape" },
	{ "frankthefarmhandpart2", "Frank the Farmhand - Part 2: The Secret of Guija" },
	{ "franticfranko", "Frantic Franko: A Bergzwerg Gone Berserk" },
	{ "frasiercraneseattlerampage", "Frasier Crane: Seattle Rampage" },
	{ "freakchic", "Freak Chic" },
	{ "fredandbarneymeetthefuture", "Fred and Barney meet the future" },
	{ "fribbeldib", "Fribbeldib" },
	{ "fridgefollies", "Fridge Follies" },
	{ "frightfest", "FrightFest: Dracula vs. Frankenstein vs. The Mummy" },
	{ "fritz", "Fritz" },
	{ "frogisland", "Frog Island" },
	{ "frozenkingdom", "Frozen Kingdom" },
	{ "fsis666", "FSi's 666 Days a Sotona" },
	{ "fsis1000000quest", "FSi's $1000000 Quest!" },
	{ "fsisachequest", "FSi's Ache Quest" },
	{ "fsisalienation", "FSi's Alienation!" },
	{ "fsiscotmattcehotsvd", "FSi's CotMATtCEHotSVD" },
	{ "fsismhcfhr", "FSi's MHCFHR!" },
	{ "fsismountain", "FSi's Neighbour Mountain" },
	{ "fsispowercowfromuranus", "FSi's PowerCow From Uranus!" },
	{ "fsisunbearable", "FSi's Unbearable" },
	{ "fsivacuum", "FSi's Vacuum Cleaner Gone Mad" },
	{ "fuguestate", "The Man From Fugue State" },
	{ "fulkramick", "Fulkramick's Dreamting: An Interactive Adventure" },
	{ "funnyboneyard", "The Funny Boneyard" },
	{ "funsunmishaps", "Fun, Sun & Mishaps" },
	{ "funwithnumbers", "Fun With Numbers" },
	{ "fuoriora", "Fuori Ora" },
	{ "furballs1", "Fur Balls 1 - evil be thy name: Bundles of mayhem!" },
	{ "furryfather", "Sorrow of a Furry Father" },
	{ "futuramatrivia", "Futurama: Who Said That?" },
	{ "futurecity3000", "Future City 3000" },
	{ "fycorpse", "Fuck You I'm a Corpse" },  // aka Dead Dave
	{ "gabyking", "The Young Gabriel King Chronicles" },
	{ "galaxiaquest", "Galaxia Quest" },
	{ "galaxyquest", "Galaxy Quest: The Arkainian Artifact - Part I" },
	{ "gallowsvillage", "Gallows Village" },
	{ "gamequest", "Game Quest" },
	{ "gamesgalore", "Games Galore!" },
	{ "garbheileach", "The Secret of Garbh Eileach" },
	{ "gardnerheist", "The Gardner's Heist" },
	{ "garfieldlasagna", "Garfield: Attack of the Mutant Lasagna" },
	{ "gassesuittollis3", "Gasse Suit Tollis 3: Looking for Love" },
	{ "gatewayremake", "Gateway Remake" },
	{ "gaygreg", "Gay Greg is Grounded" },
	{ "gemcollector", "Gem Collector" },
	{ "genbu", "Genbu's Favour" },
	{ "geometricshapes1circleboy", "Geometric Shapes 1: Circleboy" },
	{ "gesundheit", "Gesundheit!" },
	{ "getawayfrompluto", "Get away from PLUTO" },
	{ "getfood", "Get food" },
	{ "getsomesleep", "Get Some Sleep or You'll Die!" },
	{ "getsquirty", "Get Squirty!" },
	{ "gettingpicture", "Getting the Picture" },
	{ "ghormakhour", "Ghormak Hour Game - Al Lowe" },
	{ "ghostcatchers", "GhostCatchers" },
	{ "ghostland", "Ghostland" },
	{ "ghoststories", "Ghost Stories" },
	{ "ghostv", "Ghost V" },
	{ "ghostvirus", "Ghost Virus Invasion" },
	{ "ghostvoyage", "Ghost Voyage" },
	{ "giftingspirit", "Gifting Spirit" },
	{ "girlandrabbit", "Educating Adventures of Girl and Rabbit" },
	{ "girlinthehouse", "The Girl in the House" },
	{ "gladiatorquest", "Gladiator Quest" },
	{ "glitchquest", "Glitch Quest" },
	{ "globalistagenda", "My Dear Globalist Agenda" },
	{ "gloriouswolf2", "Glorious Wolf 2 - The Queens Gambit" },
	{ "gnomeshomebrewingadventure", "Gnome's Homebrewing Adventure" },
	{ "gnrblex", "GNRBLEX" },
	{ "goatburn", "Goat Burn" },
	{ "goatherd", "Goat Herd and the Gods" },
	{ "gobyworld", "Gobyworld" },
	{ "goinghome", "Going Home" },
	{ "goldreddragon", "Gold of the Red Dragon" },
	{ "goneboatfishin", "Gone Boat Fishin'" },
	{ "gonefishin", "Gone fishin'" },
	{ "gonemyangel", "Gone, My Angel, Gone" },
	{ "gonorth", "Go North" },
	{ "gonorth2", "Go North 2" },
	{ "goodgod", "Good God!" },
	{ "goodmorningmrgingerbread", "Good Morning, Mr. Gingerbread!" },
	{ "goodsantabadsanta", "Good Santa, Bad Santa" },
	{ "goontang", "Goontang Chackalaka" },
	{ "goosequest1", "Goose Quest 1" },
	{ "gotalight", "Got a Light?" },
	{ "gpslostadventure", "G.P.'s Lost Adventure" },
	{ "grandadvest", "Grandad and The Quest for The Holey Vest" },
	{ "grandkitchenescape", "Grand Kitchen Escape" },
	{ "granville1", "The Granville Chronicles - Part 1: The Rebellion Begins" },
	{ "graveyard", "Graveyard" },
	{ "gravitytestgame", "Gravity's Test Game" },
	{ "gray", "Gray" },
	{ "graydale", "Graydale" },
	{ "greateststory", "The Greatest Story Ever Told" },
	{ "greenback", "Greenback - Prologue" },
	{ "greenred", "The Green & Red" },
	{ "greglinda", "The Exciting Space Adventures of Greg and Linda" },
	{ "greycity", "Greycity" },
	{ "grizzlygooseofgosse", "Grizzly Goose of Gosse" },
	{ "groundhog", "Groundhog" },
	{ "grr", "Grr! Bearly Sane" },
	{ "guardiansofgold", "Guardians of Gold" },
	{ "gunther", "Gunther Abstrauer" },
	{ "guyhookcrook", "Guy: by hook or by crook" },
	{ "guyredplanet", "Guy who landed on weird red planet" },
	{ "guyslug", "Guy Slug: Private Eye" },
	{ "guyver1d", "Guyver 1D" },
	{ "guyverquest1", "Guyver Quest I: Sho Adventure" },
	{ "guyverquest2", "Guyver Quest II: Cronos" },
	{ "gwendarkly", "Gwen Darkly: A Speechless Case" },
	{ "hack", "Hack" },
	{ "hackenslashisland", "The Mystery of Hackenslash Island" },
	{ "halloween", "Hansens Halloween" },
	{ "halloweenguest", "Halloween with an unexpected Guest" },
	{ "halloweenhorror", "Halloween Horror" },
	{ "halloweenparty", "The Halloween Party" },
	{ "hallwayofadventures", "Hallway of Adventures" },
	{ "hamresanden2", "The Hamresanden Chronicles II: The Black Prism" },
	{ "hamster", "No one touches my Hamster" },
	{ "hamsterisland", "The Secret of Hamster Island" },
	{ "hangon", "Hang On" },
	{ "happyduckieadventure", "Happy Duckie Adventure" },
	{ "happyface", "^_^" },
	{ "hardspace", "Hard Space: Conquest of the Gayliks!" },
	{ "harrycaine", "Harry Caine UNRATED" },
	{ "harrypotterrpg", "Harry Potter RPG" },
	{ "harrys21stbirthday", "Harry's 21st Birthday" },
	{ "harryshopshock", "Harry and the Locked Crocs Shop Shock" },
	{ "hauntedcastle", "Haunted Castle" },
	{ "hauntingsmanor", "Hauntings of Mystery Manor" },
	{ "haven1", "Haven - Episode 1" },
	{ "hawkeye", "Hawk Eye Quandaries" },
	{ "hawkmanor", "Terror Within Hawk Manor" },
	{ "headbangerheaven", "Headbanger's Heaven - A Rock & Roll Adventure" },
	{ "headoverheels", "Head over Heels" },
	{ "heartland", "Heartland" },
	{ "heartofabraxas", "Heart of Abraxas" },
	{ "heartpart", "A Heart between Parts" },
	{ "heatwave", "Heatwave" },
	{ "heavenhell", "Heaven, Hell and the Neitherworld" },
	{ "heavymetalnannulf", "Heavy Metal Nannulf: The Strange Stage" },
	{ "hecamethroughthedoor", "He Came Through the Door" },
	{ "heed", "Heed" },
	{ "helloneighbor", "Hello Neighbor!" },
	{ "hellotaxi", "Hello! Taxi!" },
	{ "hellsatans", "Hell's Satans" },
	{ "hellspuppy", "Hell's Puppy" },
	{ "helpthegame", "HELP! the game" },
	{ "helycia", "Hélycia" },
	{ "hendrixisland", "HENdRIX' Island" },
	{ "henkstroemlostincellar", "Henk Stroem in: Lost In Cellar" },
	{ "henman", "Hen Man: Origins" },
	{ "henrysmith", "Henry Smith and the Looters of Peru" },
	{ "herby", "Herby" },
	{ "herenosirens", "Here Be NO Sirens" },
	{ "heroeswyrdale", "Heroes of Wyrdale" },
	{ "heroinesquest", "Heroine's Quest: The Herald of Ragnarok" },
	{ "heroquestbeuk", "HeroQuestBeuk" },
	{ "hesgonehistorical", "He's Gone Historical" },
	{ "hewatches", "He Watches" },
	{ "hhgtgtowelday", "H2G2: Towel Day" },
	{ "hiddenmessages", "Hidden Messages" },
	{ "hiddenplains", "Hidden Plains" },
	{ "hiddentreasureryansfortune", "Hidden Treasure: Ryan's Fortune" },
	{ "hide", "Hide" },
	{ "highlandspirit", "Highland Spirit" },
	{ "him", "Him" },
	{ "hitchhikersguidetothegalaxyremake", "The Hitchhiker's Guide to the Galaxy Remake" },
	{ "hitthefreak", "Hit the Freak" },
	{ "hiyah", "HiYah!" },
	{ "hjarta", "Hjarta" },
	{ "hlobb", "The Historical League of Bouncy Boxing" },
	{ "hoik", "Hero of Infamous Kingdoms" },
	{ "holocaustmuseum", "The Holocaust Museum" },
	{ "homesweetron", "Home Sweet Reality-on-the-Norm" },
	{ "homunculus", "Homunculus" },
	{ "honksadventure", "Honk's Adventure" },
	{ "hood", "Hood" },
	{ "hookhook", "Hook's hook!" },
	{ "hope", "Hope" },
	{ "hopelessness", "Hopelessness" },
	{ "hoppinghomeward", "Hopping Homeward" },
	{ "horrorhospital", "Horror Hospital" },
	{ "horrornaturally", "Well... Horror, Naturally" },
	{ "horseparkdeluxe", "Horse Park DeLuxe" },
	{ "horseparkfantasy", "Horse Park Fantasy" },
	{ "hotelhansen", "Hotel Hansen" },
	{ "hotelhijinks", "Hotel Hijinks" },
	{ "housedesade", "House of de Sade" },
	{ "houseofhorror", "House of Horror" },
	{ "housequest2", "House Quest 2" },
	{ "howmany", "How many..." },
	{ "howtheyfoundsilence", "How They Found Silence" },
	{ "hpunk", "H_PUNK_//" },
	{ "hubris", "Hubris - A Popular Pub Pastime" },
	{ "hueshaunt", "Hues of the Haunt" },
	{ "hugglestrip", "Huggles Goes On A Trip!" },
	{ "humbleescape", "The Improbable Humble Escape" },
	{ "hungry", "Hungry" },
	{ "hungryworm", "Hungry Worm" },
	{ "huongjiaoping", "Huong Jiao Ping" },
	{ "hurray", "Hurray" },  // aka Preeee
	{ "huxzadventure", "Huxz Adventure" },
	{ "hybrid", "Hybrid" },
	{ "hydeandseek", "Hyde and Seek" },
	{ "hydrate", "HYDRATE" },
	{ "hypnotoad", "Hypnotoad" },
	{ "iamjason", "IAMJASON" },
	{ "icantsleep", "I Can't Sleep in Silence - It's Always Darkest" },
	{ "icbm", "ICBM" },
	{ "icecreammystery", "The Ice Cream Mystery" },
	{ "iceintheair", "Ice in the Air" },
	{ "icestationzero", "Ice Station Zero" },
	{ "id", "iD" },
	{ "iforgot", "I Forgot..." },
	{ "iggrok", "Instagame: Grok" },
	{ "igspaceadventure", "Instagame: Space Adventure" },
	{ "igspaceadventurer", "Instagame: Space Adventurer" },
	{ "igspaceoddities", "Instagame: Space Oddities - Zoip's Escape!" },
	{ "igstrangeplanet", "Instagame: The Strange Planet" },
	{ "iiispy", "III-Spy" },
	{ "iisstabbings", "From Hell's Hart, I is Stabbings: An Oceanspirit Dennis Tale" },
	{ "illuminationdiminishing", "Illumination Diminishing" },
	{ "illuminum", "ill-uminum" },
	{ "illusion", "Illusion" },
	{ "imfree", "I'm free / Soy libre" },
	{ "imnotcrazyrightthecell", "I'm not crazy, right? - The Cell" },
	{ "imnotcrazyrightthewell", "I'm not crazy, right? - The Well" },
	{ "imonlysleeping", "I'm Only Sleeping" },
	{ "impostersyndrome", "Imposter Syndrome" },
	{ "imstillhere", "I'm still Here" },
	{ "inbloom", "In Bloom" },
	{ "inconvenience", "inconvenience" },
	{ "indyatp", "Indiana Jones: ATP-fr Test" },
	{ "indyaventuriers", "Indiana Jones et les aventuriers de l'arche perdue" },
	{ "indyberceau", "Indiana Jones et le Berceau de l'Hiver" },
	{ "indybones", "Indy Bones: The Book of the Gods" },
	{ "indycomingofage", "Indiana Jones - Coming of Age" },
	{ "indycrownofsolomon", "Indiana Jones and the Crown of Solomon" },
	{ "indycrystalcursor", "Indiana Jones and the Window of the Crystal Cursor" },
	{ "indyeyedestiny", "Indiana Jones and the Eye of Destiny" },
	{ "indygoldofgenghiskhan", "Indiana Jones and the Gold of Genghis Khan" },
	{ "indynouvelan", "Indiana Jones et les aventuriers du nouvel an" },
	{ "indypassageofsaints", "Indiana Jones and the Passage of Saints" },
	{ "indyrelicoftheviking", "Indiana Jones and the relic of the Viking" },
	{ "indyroyaume", "Indiana Jones et le Royaume des Voeux" },
	{ "indysecretchamber", "Indiana Jones and the Secret Chamber of Schloss Brunwald" },
	{ "indysevencities", "Indiana Jones and the Seven Cities of Gold" },
	{ "indianarodent", "Indiana Rodent: Raiders of the Lost Cheese" },
	{ "ineedawee", "I need a Wee!" },
	{ "infantrydivision", "Infantry Division 1338" },
	{ "infectionep1", "Infection - Episode I: The Ship" },
	{ "infectionep2", "Infection - Episode II: The Station" },
	{ "inferno", "Inferno: the demo of a rotten bastard" },
	{ "infimum", "INFIMUM" },
	{ "infinitemonkeys", "Infinite Monkeys" },
	{ "infinitybit", "Infinity Bit" },
	{ "inlimbo", "In Limbo" },
	{ "innersanctum", "Inner Sanctum" },
	{ "inourmidst", "In Our Midst" },
	{ "insanebert", "Insane Bert" },
	{ "insidemonkeyisland", "Inside Monkey Island" },
	{ "insidemonkeyislandch2", "Inside Monkey Island: 2nd chapter" },
	{ "insidemonkeyislandch3", "Inside Monkey Island: 3rd chapter" },
	{ "insidemonkeyislandch4", "Inside Monkey Island: 4th chapter" },
	{ "inspectorgismoe", "Inspector Gismoe" },
	{ "integerbattleship", "Integer Battleship" },
	{ "interdimensionalcs", "Interdimensional Community Service" },
	{ "intergalacticspacepancake", "Intergalactic Space Pancake!" },
	{ "interstellarborders", "Interstellar Borders" },
	{ "interstellarinterruption", "Interstellar Interruption" },
	{ "intestinator", "Intestinator!" },
	{ "intothelight", "Into The Light" },
	{ "intraworld", "Intra-World" },
	{ "intraworld2", "Intra-World 2" },
	{ "invasiondeathorbs", "Invasion of the Floating Death-Orbs" },
	{ "inversion", "Inversion" },
	{ "invincibleisland", "Invincible Island Remake" },
	{ "ioawn4t", "If On A Winter's Night, Four Travelers" },
	{ "irentedaboat", "I Rented a Boat" },
	{ "irishcoffee", "Irish Coffee" },
	{ "isaacodyssey", "Isaac's Odyssey" },
	{ "iskraigra", "Iskra Igra" },
	{ "isnkill", "ISN: Kill!" },
	{ "isometrichouse", "The Isometric House" },
	{ "isoproject", "The Wambus 0: ISO Project" },
	{ "isos", "I.S.O.S." },
	{ "ispy2", "I Spy II" },
	{ "itsabugslife", "It's a Bugs Life" },
	{ "itsjustarongame", "It's Just a Reality-on-the-Norm Game" },
	{ "iwalkedapath", "I Walked a Path" },
	{ "iwantanidentity", "I want an Identity / Quiero una Identidad" },
	{ "iwantout", "I Want Out!" },
	{ "iwanttodie", "I want to die / Quiero Morir" },
	{ "iwanttodieremake", "I want to die / Quiero Morir Remake" },
	{ "iwwhiiwwhitomirotpgthegame", "IWWHIIWWHITOMIROTPG: The Game!" },
	{ "jack", "JACK" },
	{ "jacktrasheaterch1", "Jack Trasheater - Chapter I: Pain House! / Jack Trasheater e la Casa del Dolore" },
	{ "jacktrekker", "Jack Trekker - Somewhere in Egypt" },
	{ "jacob", "Jacob" },
	{ "jacobvacut", "Jacob - VertigoAddict's cut" },
	{ "jacquelinewhitecurseofthemummies", "Jacqueline White - Curse of the Mummies" },
	{ "jacquelinewhitereddesert", "Jacqueline White - Bad Trouble in the Red Desert" },
	{ "jailhouse", "Jail House Breakdown" },
	{ "jakelastjourney", "Jake's Very Last Journey" },
	{ "jamesbond", "James Bond" },
	{ "jamesinneverland", "James in Neverland" },
	{ "jamesperis", "James Peris es el agente 00,5" },
	{ "jamesperis2", "James Peris 2" },
	{ "jardimfadas", "O Jardim das Fadas" },
	{ "jasongoldenapple", "Jason and the Golden Apple" },
	{ "javelincatch", "Javelin Catch" },
	{ "jetpacksam", "Jetpack Sam" },
	{ "jezioro", "Jezioro Pingwinie" },
	{ "jimbobabducted", "Jim Bob Joe Brown Gets Abducted" },
	{ "jimmsquest3", "Jimm's Quest III: Lesko's Revenge" },
	{ "jimmygoodlove", "Jimmy Goodlove: Serial Killer Fugitive" },
	{ "jimmysday", "Jimmy's Day" },
	{ "jimmythetroublemaker", "Jimmy The Troublemaker" },
	{ "jimmythetroublemaker2", "Jimmy The Troublemaker - Part 2" },
	{ "jimsonjazz", "Jimson and the Jazz Crabs" },
	{ "joeshorriblehell", "Joe's Horrible Hell" },
	{ "joesmiserablelife", "Joe's Miserable Life" },
	{ "johnharris", "John Harris and the Treasure of the Pharaoh" },
	{ "johnjebediahgun", "John Jebediah Gun and Sepheret Island" },
	{ "johnlosthiskeyep1", "John Lost His Key - Episode 1" },
	{ "johnlosthiskeyep2", "John Lost His Key - Episode 2" },
	{ "johnnyrocket", "Johnny Rockett Adventure" },
	{ "johnsavedchristmas", "The Day that John Saved Christmas" },
	{ "johnsinclair", "John Sinclair - Voodoo in London" },
	{ "jokequest", "Joke Quest" },
	{ "jonahsplace", "Jonah's Place" },
	{ "jonathanadv", "Jonathan's Adventures" },
	{ "jonnyfeces", "Jonny and the sweet fragrance of feces" },
	{ "jonstickman", "Jon Stickman" },
	{ "journey", "Journey" },
	{ "journeyhome", "Journey Home" },
	{ "journeyhell", "Journey to Hell" },
	{ "jugglequest", "Roger's Juggle Quest" },
	{ "juliusdangerous1", "Julius Dangerous .. and the space invaders" },
	{ "juliusdangerous2", "Julius Dangerous 2" },
	{ "jumpinjack", "Jumpin' Jack: the bean powered kid" },
	{ "jumpinjones", "Jumpin' Jones in: The Big Switch-Off" },
	{ "jumpjackflash", "Jump! Jack! Flash!" },
	{ "june20th", "June 20th" },
	{ "justanotherpointnclickadventure", "Just Another Point n Click Adventure" },
	{ "justyouandme", "Just You And Me" },
	{ "kada", "Ka and Da" },
	{ "kanjigakusei", "Kanji Gakusei" },
	{ "karelianninja", "Karelian Ninja" },
	{ "kartquest", "Kart-Quest" },
	{ "kata", "KATA" },
	{ "katurachroniques", "Les Chroniques de Katura" },
	{ "katurachroniquesrpg", "Le Jeu de Rôles des Chroniques de Katura" },
	{ "katuracolonisation", "Katura Colonisation" },
	{ "katuralchimie", "KaturAlchimie" },
	{ "katurapuzzleadv", "Katura Puzzle Adventure" },
	{ "katurapuzzlearenas", "Katura Puzzle Arenas" },
	{ "katurarpg", "Katura RPG" },
	{ "keptosh1", "Keptosh I: The Search for junc" },
	{ "keyboardmadness", "Keyboard Madness" },
	{ "keysofagamespace", "Keys of a gamespace. An expressive game" },
	{ "kidnapped", "Kidnapped" },
	{ "kidnapperescape", "Kidnapper Escape" },
	{ "kikme", "Kikme Quest" },
	{ "killereye", "Killer Floating Eye" },
	{ "killjoseda", "Kill Joseda" },
	{ "killmenow", "Kill Me Now" },
	{ "kingdomlegend1", "Kingdom Legend" },
	{ "kingdomlegend2", "Kingdom Legend 2" },
	{ "kingofrock", "The King of Rock in: Riding the Wild Wind" },
	{ "kingrobert", "King Robert's Quest: The Frustratingly Early Access Adventure" },
	{ "kingsleyroad", "Kingsley Road, 1980" },
	{ "kingsquestfororgy", "King's Quest for Orgy" },  // aka "Quest for Orgy: So You Thought She Is a Virgin?!?"
	{ "kinkyisland", "Kinky Island" },
	{ "kirja", "Kirjastotäti" },
	{ "kiselyova", "Kiselyova Unleashed!" },
	{ "kittenadv", "Kitten Adventures" },
	{ "kittyquest", "Kitty Quest" },
	{ "klopoty", "Kłopoty Mirmiła - Rozdział 1" },
	{ "knightpursuit", "A Knight's Pursuit" },
	{ "knightquestforgoldenring", "Knight Quest for the Golden Ring" },
	{ "knightsquest3", "Knight's Quest III - Tides of Merania" },
	{ "knightsquest4", "Knight's Quest IV - Here Today, Gone to Yesterday" },
	{ "knightsquire", "KnightSquire" },
	{ "knobblycrook", "The Knobbly Crook" },
	{ "knobblycrookch1", "The Knobbly Crook - Chapter 1: The Horse You Sailed In On" },
	{ "knorrig", "Knorrig the Gifted Troublemaker" },
	{ "koddurova", "Kod Durova" },
	{ "koffeekrisis", "Koffee Krisis" },
	{ "kongbaghdad", "Kong Over Baghdad" },
	{ "korinsmines", "Korin's Mines" },
	{ "koscheitheimmortal", "Koschei The Immortal - The Beginning" },
	{ "kq3plus", "King's Quest III+" },
	{ "kq4retold", "King's Quest IV: The Perils of Rosella Retold" },
	{ "kq9vga", "King's Quest IX: The Silver Lining VGA" },
	{ "kristmaskrisis", "Kristmas Krisis" },
	{ "ksauna", "Karjalan Sauna" },
	{ "ktx1", "KTX-1" },
	{ "kubik", "KubiK" },
	{ "kumastory", "Kuma Story" },
	{ "kynigos", "Kynigos" },
	{ "laboite", "La boite" },
	{ "labratescape", "Lab Rat Escape" },
	{ "labratmaze", "Lab Rat Maze!" },
	{ "labyrinth", "Labyrinth" },
	{ "lacarbonara", "La Carbonara" },
	{ "lacicuta", "La Cicuta" },
	{ "lacolonia", "La Colonia" },
	{ "lacroixpan", "La Croix Pan" },
	{ "lagrancastanya", "La Gran Castanya" },
	{ "lagrandeplaine", "La Grande Plaine" },
	{ "lallaveyfabianshones", "La Llave y Fabian Shones" },
	{ "lamaleta", "La Maleta" },
	{ "lambslaughter", "Lamb to the slaughter" },
	{ "lancethepenguin", "Lance The Penguin" },
	{ "laodiseadelfracaso2", "La Odisea del Fracaso II" },
	{ "laportenoire", "AAaaah!! La porte noire" },
	{ "lasol", "Submerged - LaSol" },
	{ "lastbus", "The Last Bus" },
	{ "lastclown", "Last Clown Standing" },
	{ "lasthope", "Last Hope" },
	{ "lastlightpost", "The Last Light Post" },
	{ "lastnfurious", "Last'n'Furious" },
	{ "lastnovax", "The Last No-Vax" },
	{ "lastone", "The Last One" },
	{ "lastorder", "Last Order" },
	{ "latarta1", "La Tarta - Una Aventura en Casa de Kelly" },
	{ "latarta2", "La Tarta II - La Ciudad y el Cielo" },
	{ "latarta3", "La Tarta III - El Circo de la Alegria" },
	{ "latelastnite", "Late Last Nite" },
	{ "laundryday", "Laundry Day" },
	{ "lavablava", "Lava Blava" },
	{ "lazaruswantspants", "Lazarus Wants Pants" },
	{ "lazytownthenewkid", "LazyTown: The New Kid" },
	{ "lechuckstories", "LeChuck Stories" },
	{ "legacyicecream", "The Legacy of Icecream Man" },
	{ "legendofrovendale", "Legend of Rovendale" },
	{ "legendofseththebard", "The Legend of Seth the Bard" },
	{ "legendofskystones", "Legend Of Sky Stones" },
	{ "legendsofmardaram", "Legends of Mardaram" },
	{ "leisuresuitlarry2", "Leisure Suit Larry 2 - Point and Click Remake" },
	{ "leisuresuitlarrylil", "Leisure Suit Larry: Lost in Love!" },
	{ "lelac", "Le Lac" },
	{ "lemasabachthani", "Lema Sabachthani" },
	{ "leogravedigger", "Leo the Gravedigger's Adventure" },
	{ "leopoldkettle", "The Surprisingly Short Adventure of Leopold Kettle" },
	{ "lesangimmortels", "Le Sang des Immortels" },
	{ "lesmiserables", "Les Miserables" },
	{ "lessthanthree", "<3 (Less Than Three)" },
	{ "letscook", "Let's Cook - School of Cooking with Koala" },
	{ "letteraamorosa", "Lettera Amorosa" },
	{ "libremotus", "Libre Motus" },
	{ "lichdom", "Lichdom - \"Where did I put that...\"" },
	{ "life", "Life" },
	{ "lifeboatstoryofcedrick", "Lifeboat: The Story of Cedrick" },
	{ "lifeinabox", "Life in a Box" },
	{ "lifeofdduck", "Life of D. Duck" },
	{ "lifeofdduck2", "Life of D. Duck II" },
	{ "lifeworthlosing", "A Life Worth Losing" },
	{ "liftreasureofthetanones", "Lif and the Treasure of the Tanones" },
	{ "lightcycles", "AGS Cycles" },
	{ "lightningmaster", "Lightning Master" },
	{ "likeadream", "Like A Dream" },
	{ "likeafox", "Like a Fox!" },
	{ "lillywizard", "Lilly the Wonder Wizard" },
	{ "limeylizardwastewizard", "Limey Lizard: Waste Wizard!" },
	{ "linegame", "Line Game" },
	{ "linkattus", "The Devious and Daring Commando Raid of Linkattus" },
	{ "linnprotector", "Linn the Protector and the Seven Daughters of Ran" },
	{ "lionsden", "The Lion's Den" },
	{ "littlegirlinunderland", "Little Girl in Underland" },
	{ "littleleonardo", "Little Leonardo" },
	{ "littlesimulatedpeople", "Little Simulated People" },
	{ "livingnightmare", "Living Nightmare" },
	{ "livingnightmareendlessdream", "Living Nightmare: Endless Dreams" },
	{ "livingnightmarefreedom", "Living Nightmare: Freedom" },
	{ "loathesome", "The Loathesome Man" },
	{ "lockedin", "Locked In" },
	{ "lockedout", "Locked Out" },
	{ "lockeescape", "Locke's Escape" },
	{ "loftusandtheskycap", "Loftus and the Sky Cap" },
	{ "lonecase1", "Lone Case: Locomotive Breath" },
	{ "lonecase2", "Lone Case 2: Scars" },
	{ "lonecase3", "Lone Case 3: Showdown" },
	{ "lonecase4", "Lone Case 4: Epitaph" },
	{ "lonelynight", "Lonely Night" },
	{ "longdistancecoughing", "Long Distance Coughing" },
	{ "longestwinter", "The Longest Winter" },
	{ "longevitygene", "The Longevity Gene" },
	{ "longexpectedfriday", "Long expected Friday" },
	{ "longtooth", "Long in the Tooth" },
	{ "lonkeyisland", "The Tale of Lonkey Island" },
	{ "lookatmech1", "Look at me. Look at my face. Does it look like I care about school? - Chapter 1" },
	{ "lookingfordread", "Looking For Dread Mac Farlane / A la recherche de Dread Mac Farlane" },
	{ "loomiireturnoftheswans", "Loom II: Return of the Swans" },
	{ "loonyisland", "The Mystery of Loony Island" },
	{ "lordbytes1ep1", "The Lord of the Bytes - Chapter 1 Ep. 1" },
	{ "lordbytes1ep2", "The Lord of the Bytes - Chapter 1 Ep. 2" },
	{ "lordbytes1ep3", "The Lord of the Bytes - Chapter 1 Ep. 3" },
	{ "lordbytes1ep4", "The Lord of the Bytes - Chapter 1 Ep. 4" },
	{ "lordbytes1ep5", "The Lord of the Bytes - Chapter 1 Ep. 5" },
	{ "lordbytes1ep6", "The Lord of the Bytes - Chapter 1 Ep. 6" },
	{ "lordbytes1ep7", "The Lord of the Bytes - Chapter 1 Ep. 7" },
	{ "lordbytes1ep8", "The Lord of the Bytes - Chapter 1 Ep. 8" },
	{ "lordbytes1ep9", "The Lord of the Bytes - Chapter 1 Ep. 9" },
	{ "lordbytes1ep10", "The Lord of the Bytes - Chapter 1 Ep. 10" },
	{ "lordbytes1ep11", "The Lord of the Bytes - Chapter 1 Ep. 11" },
	{ "lordbytes1ep12", "The Lord of the Bytes - Chapter 1 Ep. 12" },
	{ "lordbytes1ep13", "The Lord of the Bytes - Chapter 1 Ep. 13" },
	{ "lordoflight", "Lord of Light" },
	{ "lorrylen", "Lorry Len" },
	{ "losjovenesdelaguerra", "Los Jovenes De La Guerra" },
	{ "losno", "Ladies of Sorrow: Night One" },
	{ "lostanswers", "Lost Answers" },
	{ "lostdollar", "Max Griff in: The Lost Dollar" },
	{ "lostfound", "Lost & Found" },
	{ "lostinparadise", "Lost In Paradise" },
	{ "lostinthenightmare", "Lost In The Nightmare" },
	{ "lostinthenightmare2", "Lost In The Nightmare 2: Unforgettable Memories" },
	{ "lostinthewoods", "Lost In The Woods" },
	{ "lotto", "Lottó" },
	{ "lowequest", "Lowe Quest" },
	{ "lucasmaniac", "Lucas Maniac!" },
	{ "lucasmendoza", "Lucas Mendoza, Amateur Detective: The Searchers of The Beginning" },
	{ "lucidlucy", "LUCID LUCY" },
	{ "lucylavender", "The Amazing Adventures of Lucy Lavender" },
	{ "lukesexistentialnightmare", "Luke's Existential Nightmare" },
	{ "lunarlander", "Lunar Lander" },
	{ "lutherinhood", "Luther in the Hood" },
	{ "lydia", "Lydia" },
	{ "lydianellreno", "Lydia and the Mystery of Nellreno Manor" },
	{ "lygophilous", "Lygophilous" },
	{ "machinesdreams", "Machines Have Lucid Dreams" },
	{ "madjack", "Mad Jack: Breakdown Shakedown" },
	{ "maelstrom", "Maelstrom Obscura - Case 1: The Legend of the Loch Ness Monster" },
	{ "mafaldawest", "Mafalda: The Strange and Unusual life of Mafalda West, Part 1" },
	{ "mafiosooverkill", "Mafioso Over Kill" },
	{ "magentaspacech1", "Magenta: IN SPACE (Chapter 1)" },
	{ "magic8ball", "Magic-8-Ball" },
	{ "magicalwhatevergirl", "Magical Whatever Girl Rocks Out In The Stone Age" },
	{ "magicballoffortune", "Magic Ball of Fortune" },
	{ "magicowl", "Magic Owl" },
	{ "magnicidio", "Arborea: Magnicidio en la Corte" },
	{ "magnumpm", "Magnum, P.M" },
	{ "magsic", "Magsic" },
	{ "magsic2", "Magsic II" },
	{ "majixs", "Majixs" },
	{ "majorbummerdude", "Major Bummer Dude: Lassi Quest Reality-on-the-Norm" },
	{ "mammamia", "Mamma Mia! Winter Ice Cream Mayhem" },
	{ "manamatch", "Mana Match" },
	{ "manboy", "Man Boy vs. Doctor Sock" },
	{ "mangivingup", "Man Giving Up" },
	{ "maniacapartment", "Maniac Apartment" },
	{ "maniacland", "Alice in Maniacland" },
	{ "maniacmansiondeluxe", "Maniac Mansion Deluxe" },
	{ "maniacmansionds", "Maniac Mansion DS" },
	{ "maniacmansiondott", "Maniac Mansion - DOTT-style Remake" },
	{ "maniacmetalheadmania", "Maniac Metalhead Mania" },
	{ "maniacmetalheadmania2", "Maniac Metalhead Mania II: The Heavy Metal Rescue Mission" },
	{ "manoli", "Manoli: Aprendiz de Hechicera" },
	{ "manvsfish", "Man vs. Fish" },
	{ "mardsrevenge", "Mard's Personal Little Revenge" },
	{ "martyausdemall", "Marty aus dem All" },
	{ "martyroftime", "A Martyr Of Time" },
	{ "mash", "M*A*S*H*: The Point n' Click Adventure" },
	{ "masked", "Masked" },
	{ "masquerade", "Masquerade at the Con" },
	{ "mastersofsound", "Masters of Sound" },
	{ "matildacurse", "Matilda and the Curse of King Stephen" },
	{ "matttothefuture", "Matt to the Future" },
	{ "maverickgunn", "Maverick Gunn and the Eye of Oggun" },
	{ "maxandmaggie", "The Chronicles of Max and Maggie" },
	{ "maxfury", "Max Fury" },
	{ "maxparade", "Max's Parade" },
	{ "mayak", "Mayak: Lost Way" },
	{ "mechanismo", "Mechanismo" },
	{ "medicaltheoriesofdrkur", "The Medical Theories of Dr. Kur" },
	{ "megacorp", "Megacorp Redux" },
	{ "megocannibaljungle", "Me Go Cannibal Jungle!" },
	{ "megostore", "Me Go Store!" },
	{ "megostore2", "Me Go Store II: Me Go Away!" },
	{ "megostore3", "Me Go 2008!" },
	{ "melrin1", "Melrin: The Disciple Ordeal" },
	{ "melrin2", "Melrin: The Pendant Quest" },
	{ "melrin3", "Melrin: The Dragon Menace" },
	{ "memochi", "Memochi - Dust" },
	{ "memoriae", "Memoriae" },
	{ "memoriesfade", "Memories Fade" },
	{ "memoriesofasnake", "Memories of a Snake / Memoires d'un Serpent" },
	{ "memory", "This Game Might Improve Your Memory" },
	{ "menial", "Menial - A Utopian Bagel Simulator" },
	{ "meninhats", "Men In Hats: Attack of the Evangelists" },
	{ "merrychristmas", "Merry Christmas, Alfred Robbins" },
	{ "meshumba", "Meshumba's Reckoning" },
	{ "messedupmothergoose", "Messed-Up Mother Goose - DELUXE MY ASS: ENHANCED" },
	{ "messgoblins", "Mess Goblins" },
	{ "meta", "META" },
	{ "metaphobia", "Metaphobia" },
	{ "meteorhead1", "Meteorhead: Recycled" },
	{ "meteorhead2", "Meteorhead II: Rückkehr eines Idioten" },
	{ "meteorhead3", "Meteorhead III: Spiel mir das 1337 vom Tod" },
	{ "meteorhead4", "Meteorhead IV: Forgotten Memories" },
	{ "meteorhead5", "Meteorhead V: Die Universalkraft" },
	{ "meteorhead6", "Meteorhead VI: Verbannt ins Exühl" },
	{ "meteorhead7", "Meteorhead VII: Meteorhead Man und Peter Pommes retten die Fast Food-Industrie" },
	{ "meteorhead8", "Meteorhead VIII: Rettet Meteorhead Man" },
	{ "meteorhead9", "Meteorhead IX: Razors Mutation" },
	{ "meteorhead10", "Meteorhead X: Eine Frage der Ausrede" },
	{ "meteorhead11", "Meteorhead XI: Die Klauung des güldenen Headgars" },
	{ "meteorhead13", "Meteorhead XIII: Universale Veränderungen" },
	{ "meteorhead14", "Meteorhead XIV: A Road Movie" },
	{ "meteorheadns3", "Meteorhead - The New Series - Episode III: Die unverträgliche Dreistigkeit des Schweins" },
	{ "meteorheadns4", "Meteorhead - The New Series - Episode IV: Die Qual des Wals" },
	{ "meteortale", "Meteor: A Tale About Earth's End" },
	{ "metrocity", "METRO CITY: Night Shift" },
	{ "mi", "Mi" },
	{ "mi0daementia", "Monkey Island 0 - Daementia" },
	{ "mi0navidad", "Monkey Island 0 - Navidad" },
	{ "mi12", "Monkey Island 1/2" },
	{ "mi25escape", "Monkey Island 2.5 - Escape From Big Whoop" },
	{ "mi25parque", "Monkey Island 2.5 - El parque de las Pesadillas" },
	{ "mi4test", "Monkey Island 4 Test Project" },
	{ "mi5thereturnoflechuck", "Monkey Island 5: The Return of LeChuck" },
	{ "mibaddaytobedead", "Monkey Island: Bad Day to be Dead" },
	{ "micarnivalofthedamned", "Monkey Island: Carnival of the Damned" },
	{ "mickeymauserpart1", "Mickey Mauser - Part 1: The Wrath of the Rat" },
	{ "micv", "Monkey Island - Carnaval Vudú - Parte 1: La Busqueda" },
	{ "midasheist", "The Midas Heist" },
	{ "midnightmadness", "Midnight Madness at Moon Forest" },
	{ "midnightsquadron", "Midnight Squadron" },
	{ "miguybrushson", "Monkey Island - Guybrush's Son" },
	{ "miillusion", "Monkey Island Illusion" },
	{ "mikasdream2", "Mika's Surreal Dream II: The Dream Comes True!?" },
	{ "mikelechey", "Mike Lechey and the Forgotten Race" },
	{ "mikesroom", "Mike's Room" },
	{ "milkshake", "Milkshake" },
	{ "mimv", "Monkey Island: La Maldicion Vudu" },
	{ "mindboggler", "MINDBOGGLER" },
	{ "mindrape", "Mind Rape: Duress" },
	{ "mindseye", "Mind's Eye" },
	{ "minewadv", "Monkey Island - The new adventure" },
	{ "minifeg", "Minifeg: The Search" },
	{ "minorminion", "Minor Minion" },
	{ "miprision", "The Secret of Monkey Island - Prision Demo" },
	{ "mirevealed", "The Secret of Monkey Island Revealed" },
	{ "misc", "Misc" },
	{ "missinginaction", "Missing In Action" },
	{ "missingsincemidnight", "Missing since Midnight" },
	{ "missionfutura", "Mission Futura: The Mindless Menace / Mission Zukunftia" },
	{ "missionfutura2", "Mission Futura II: Der Zorn des Klon / Mission Zukunftia II" },
	{ "misspingu", "Miss Pingu" },
	{ "mistdelaescueladearte", "El Misterio de la Escuela de Arte" },
	{ "misterybigwhoop", "The Mistery of Big Whoop" },
	{ "mistook", "The Man Who Mistook His Wife For a Hat" },
	{ "mivl", "Monkey Island: Largo's Revenge / La Venganza de Largo" },
	{ "mivoodoo", "The Voodoo of Monkey Island" },
	{ "mmm1", "Maniac Mansion Mania Ep. 001: Sibling Love / Geschwisterliebe" },
	{ "mmm2", "Maniac Mansion Mania Ep. 002: Commotion / Unruhen" },
	{ "mmm3", "Maniac Mansion Mania Ep. 003: Stubenarrest" },
	{ "mmm4", "Maniac Mansion Mania Ep. 004: Mimikry der Emotionen" },
	{ "mmm5", "Maniac Mansion Mania Ep. 005: Rhythmen zum Reinbeißen" },
	{ "mmm6", "Maniac Mansion Mania Ep. 006: Er is' weg" },
	{ "mmm7", "Maniac Mansion Mania Ep. 007: Right said Fred!?!" },
	{ "mmm8", "Maniac Mansion Mania Ep. 008: The Rebuff / Die Abfuhr" },
	{ "mmm9", "Maniac Mansion Mania Ep. 009: Radioactive / Radioaktiv" },
	{ "mmm10", "Maniac Mansion Mania Ep. 010: Tales of the Weird Ed" },
	{ "mmm11", "Maniac Mansion Mania Ep. 011: Ein haariger Ausflug" },
	{ "mmm12", "Maniac Mansion Mania Ep. 012: Serien-Special: GIGA Mansion" },
	{ "mmm13", "Maniac Mansion Mania Ep. 013: Nur geträumt" },
	{ "mmm14", "Maniac Mansion Mania Ep. 014: Ed's Rache für den Hamsterbraten" },
	{ "mmm15", "Maniac Mansion Mania Ep. 015: Place Machine / Ortmaschine" },
	{ "mmm16", "Maniac Mansion Mania Ep. 016: Meteor Family - The Return of the Meteor" },
	{ "mmm17", "Maniac Mansion Mania Ep. 017: Das Labor" },
	{ "mmm18", "Maniac Mansion Mania Ep. 018: Shit Happens!" },
	{ "mmm21", "Maniac Mansion Mania Ep. 021: Rettet Kanal 13!" },
	{ "mmm22", "Maniac Mansion Mania Ep. 022: Presserummel" },
	{ "mmm23", "Maniac Mansion Mania Ep. 023: Das verflixte Geschenk" },
	{ "mmm24", "Maniac Mansion Mania Ep. 024: Time Machine" },
	{ "mmm26", "Maniac Mansion Mania Ep. 026: Zeitenwende" },
	{ "mmm27", "Maniac Mansion Mania Ep. 027: Hamsternator" },
	{ "mmm28", "Maniac Mansion Mania Ep. 028: Time Machine 2" },
	{ "mmm29", "Maniac Mansion Mania Ep. 029: Flucht des Meteoriten" },
	{ "mmm30", "Maniac Mansion Mania Ep. 030: Memories of Zak" },
	{ "mmm31", "Maniac Mansion Mania Ep. 031: Britney's Quest / Britneys Suche" },
	{ "mmm32", "Maniac Mansion Mania Ep. 032: The Secret of Maniac Mansion" },
	{ "mmm33", "Maniac Mansion Mania Ep. 033: Carry on Smiley!" },
	{ "mmm34", "Maniac Mansion Mania Ep. 034: Helden des Tages" },
	{ "mmm35", "Maniac Mansion Mania Ep. 035: Weggebeamt" },
	{ "mmm36", "Maniac Mansion Mania Ep. 036: Der Liebesbrief" },
	{ "mmm37", "Maniac Mansion Mania Ep. 037: Date with Dave / Verabredung mit Dave" },
	{ "mmm38", "Maniac Mansion Mania Ep. 038: Rescue Mission" },
	{ "mmm39", "Maniac Mansion Mania Ep. 039: Erinnerungen" },
	{ "mmm40", "Maniac Mansion Mania Ep. 040: Trapped in the cellar / Verflixte Türen" },
	{ "mmm41", "Maniac Mansion Mania Ep. 041: The new Youth Protection Law / Das neue Jugendschutzgesetz" },
	{ "mmm42", "Maniac Mansion Mania Ep. 042: Die furchtbaren Pläne des Psycho Bernie" },
	{ "mmm43", "Maniac Mansion Mania Ep. 043: Vorsicht, Edna kommt!" },
	{ "mmm44", "Maniac Mansion Mania Ep. 044: Razors grosser Auftritt" },
	{ "mmm45", "Maniac Mansion Mania Ep. 045: Maniac Monday" },
	{ "mmm46", "Maniac Mansion Mania Ep. 046: Verschollen" },
	{ "mmm47", "Maniac Mansion Mania Ep. 047: Maniac Ostern" },
	{ "mmm48", "Maniac Mansion Mania Ep. 048: Wendy und das Buch des Todes" },
	{ "mmm49", "Maniac Mansion Mania Ep. 049: Clouso's grösster Coup" },
	{ "mmm50", "Maniac Mansion Mania Ep. 050: The Date 3 / Das Date 3" },
	{ "mmm51", "Maniac Mansion Mania Ep. 051: Place Machine II / Ortmaschine II" },
	{ "mmm52", "Maniac Mansion Mania Ep. 052: Ein irrer Tag eines Ronvillers" },
	{ "mmm53", "Maniac Mansion Mania Ep. 053: The Klaus strikes back / Der Klaus schlägt zurück" },
	{ "mmm54", "Maniac Mansion Mania Ep. 054: CSI:RONVILLE" },
	{ "mmm55", "Maniac Mansion Mania Ep. 055: Hamsters of the mysterious man: Chapter of dream" },
	{ "mmm56", "Maniac Mansion Mania Ep. 056: Grotten-Urlaub" },
	{ "mmm57", "Maniac Mansion Mania Ep. 057: Ein geheimnisvoller Ausflug" },
	{ "mmm58", "Maniac Mansion Mania Ep. 058: The People's Court" },
	{ "mmm59", "Maniac Mansion Mania Ep. 059: Computerliebe" },
	{ "mmm60", "Maniac Mansion Mania Ep. 060: Jahrmarkt der Verdammten" },
	{ "mmm61", "Maniac Mansion Mania Ep. 061: Bernard's Room" },
	{ "mmm63", "Maniac Mansion Mania Ep. 063: Books - Deals unter Nachbarn" },
	{ "mmm64", "Maniac Mansion Mania Ep. 064: Baranoia" },
	{ "mmm65", "Maniac Mansion Mania Ep. 065: Save Smiley!" },
	{ "mmm66", "Maniac Mansion Mania Ep. 066: Hoagies neuer Freund" },
	{ "mmm66akt3", "Maniac Mansion Mania Ep. 066: Hoagies neuer Freund - Akt 3: Der verbotene Akt" },
	{ "mmm67", "Maniac Mansion Mania Ep. 067: Die Schöne und das Biest" },
	{ "mmm68", "Maniac Mansion Mania Ep. 068: MaMMa ante Portas" },
	{ "mmm69", "Maniac Mansion Mania Ep. 069: Samstag" },
	{ "mmm70", "Maniac Mansion Mania Ep. 070: Maniac Mansion Begins" },
	{ "mmm71", "Maniac Mansion Mania Ep. 071: Neue Abenteuer auf Terra" },
	{ "mmm72", "Maniac Mansion Mania Ep. 072: Mindbending TV" },
	{ "mmm73", "Maniac Mansion Mania Ep. 073: Even a broken Clock" },
	{ "mmm74", "Maniac Mansion Mania Ep. 074: Ted Edison and the Curse of King RootenTooten" },
	{ "mmm75", "Maniac Mansion Mania Ep. 075: Das Necronomicon" },
	{ "mmm76", "Maniac Mansion Mania Ep. 076: The Bernoulli-Show" },
	{ "mmm77", "Maniac Mansion Mania Ep. 077: A Matter of Some Gravity / Schwer verquer" },
	{ "mmm78", "Maniac Mansion Mania Ep. 078: Dumm geholfen" },
	{ "mmm79", "Maniac Mansion Mania Ep. 079: Der Wunsch-O-Mat" },
	{ "mmm80", "Maniac Mansion Mania Ep. 080: Spiel des Lebens" },
	{ "mmm81", "Maniac Mansion Mania Ep. 081: Bernard bricht aus!" },
	{ "mmm82", "Maniac Mansion Mania Ep. 082: Freundin mit Hindernissen" },
	{ "mmm83", "Maniac Mansion Mania Ep. 083: Chasing Hoagie" },
	{ "mmm84", "Maniac Mansion Mania Ep. 084: Surf'n'Ronville" },
	{ "mmm85", "Maniac Mansion Mania Ep. 085: Freundin mit Hindernissen - Part 2" },
	{ "mmm86", "Maniac Mansion Mania Ep. 086: Bernard bekommt Besuch" },
	{ "mmm87", "Maniac Mansion Mania Ep. 087: Das Geheimnis des Blakes Hotel von Ronville" },
	{ "mmm88", "Maniac Mansion Mania Ep. 088: Der alltägliche Wahnsinn" },
	{ "mmm89", "Maniac Mansion Mania Ep. 089: Der Weltrettungsalgorithmus - Die neue Version" },
	{ "mmm90", "Maniac Mansion Mania Ep. 090: Packing the suitcase / Kofferpacken" },
	{ "mmm91", "Maniac Mansion Mania Ep. 091: Fels in der Brandung" },
	{ "mmm93", "Maniac Mansion Mania Ep. 093: Murder at the Moonshine Mansion" },
	{ "mmm94", "Maniac Mansion Mania Ep. 094: Maniac on the Mississippi" },
	{ "mmm95", "Maniac Mansion Mania Ep. 095: Britney's Escape / Britney's Flucht" },
	{ "mmm96", "Maniac Mansion Mania Ep. 096: Dave's Home Odyssey" },
	{ "mmm97", "Maniac Mansion Mania Ep. 097: Tollhaus-Weihnachten" },
	{ "mmm98", "Maniac Mansion Mania Ep. 098: Maniac Apartment" },
	{ "mmm99", "Maniac Mansion Mania Ep. 099: Die Premiere" },
	{ "mmm100", "Maniac Mansion Mania Ep. 100: Money Mansion" },
	{ "mmm101", "Maniac Mansion Mania Ep. 101: With Other Eyes / Mit anderen Augen" },
	{ "mmmatman", "Maniac Mansion Mania: @-Man - The Dork Knight" },
	{ "mmmd1", "Maniac Dungeon Raum 01: The great Escape from Maniac Dungeon" },
	{ "mmmd2", "Maniac Dungeon Raum 02: Der Bunker" },
	{ "mmmd3", "Maniac Dungeon Raum 03: Der Verbindungstunnel" },
	{ "mmmd4", "Maniac Dungeon Raum 04: Die Grotte" },
	{ "mmmd5", "Maniac Dungeon Raum 05: Tief unter der Erde" },
	{ "mmmd6", "Maniac Dungeon Raum 06: The Race" },
	{ "mmmd7", "Maniac Dungeon Raum 07: Das unheimliche Wesen aus einem fremden Raum" },
	{ "mmmd8", "Maniac Dungeon Raum 08: Die Brücke" },
	{ "mmmd9", "Maniac Dungeon Raum 09: Und dann gab's wirklich, echt überhaupt gar keins mehr, ehrlich!" },
	{ "mmmd10", "Maniac Dungeon Raum 10: Die neue Gefahr" },
	{ "mmmd11", "Maniac Dungeon Raum 11: Die vergrabene Gruft" },
	{ "mmmd12", "Maniac Dungeon Raum 12: Die Holozelle" },
	{ "mmmd13", "Maniac Dungeon Raum 13: Der Trockenraum" },
	{ "mmmd14", "Maniac Dungeon Raum 14: Vor der Tür" },
	{ "mmmd15", "Maniac Dungeon Raum 15: Die unterirdische Kulturstätte" },
	{ "mmmd16", "Maniac Dungeon Raum 16: Die U-Bahn-Station" },
	{ "mmmd17", "Maniac Dungeon Raum 17: Wahnsinnsfahrt im Geisterzug" },
	{ "mmmd18", "Maniac Dungeon Raum 18: Die perfekte Welle" },
	{ "mmmd19", "Maniac Dungeon Raum 19: Weihnachts-Edition" },
	{ "mmmd20", "Maniac Dungeon Raum 20: Die Pharaonen-Grabkammer" },
	{ "mmmd21", "Maniac Dungeon Raum 21: Durch Raum (21) und Zeit" },
	{ "mmmdasexperiment", "Maniac Mansion Mania: Das Experiment" },
	{ "mmmdieeroberung", "Maniac Mansion Mania: Die Eroberung" },
	{ "mmmeaster2010", "Maniac Mansion Mania Easter: Ostereiersuche 2010" },
	{ "mmmeaster2011", "Maniac Mansion Mania Easter: Ostereiersuche 2011" },
	{ "mmmeaster2018", "Maniac Mansion Mania Easter: Ostereiersuche 2018" },
	{ "mmmedgar1", "The Edgar Award Show - Staffel 1" },
	{ "mmmedgar2", "The Edgar Award Show - Staffel 2" },
	{ "mmmedgar2s2", "The Edgar Award Show - Staffel 2 Show II" },
	{ "mmmedgar3", "The Edgar Award Show - Staffel 3" },
	{ "mmmedgar4", "The Edgar Award Show - Staffel 4" },
	{ "mmmedgar5", "The Edgar Award Show - Staffel 5" },
	{ "mmmedgar6", "The Edgar Award Show - Staffel 6" },
	{ "mmmedgar7", "The Edgar Award Show - Staffel 7" },
	{ "mmmedgar8", "The Edgar Award Show - Staffel 8" },
	{ "mmmedgar9", "The Edgar Award Show - Staffel 9" },
	{ "mmmhollywood", "Maniac Mansion Mania Hollywood: Ronville Viper" },
	{ "mmmhw1", "Maniac Mansion Mania Halloween 05-1" },
	{ "mmmhw2", "Maniac Mansion Mania Halloween 05-2: Escape from Maniac Mansion" },
	{ "mmmhw3", "Maniac Mansion Mania Halloween 05-3: Day of the Dead" },
	{ "mmmhw4", "Maniac Mansion Mania Halloween 05-4: MMM-Horror" },
	{ "mmmhw5", "Maniac Mansion Mania Halloween 05-5: Redrum" },
	{ "mmmhw6", "Maniac Mansion Mania Halloween 06-1: The Curse of Maniac Mansion" },
	{ "mmmhw7", "Maniac Mansion Mania Halloween 06-2: Hä? Ich glaub' ich spinne!" },
	{ "mmmhw8", "Maniac Mansion Mania Halloween 10-1: Das Monster aus der Racoon Lagoon" },
	{ "mmmhw9", "Maniac Mansion Mania Halloween 10-2: TMMCMFH" },
	{ "mmmhw10", "Maniac Mansion Mania Halloween 10-3: Und Bernard läutet zur Geisterstunde" },
	{ "mmmhw11", "Maniac Mansion Mania Halloween 10-4: Bad and Mad" },
	{ "mmmlagrande", "Maniac Mansion Mania: LaGrande Hotel" },
	{ "mmmmm1", "Maniac Mansion Mania Mini Masterpieces Ep. 1: Eds Paket" },
	{ "mmmmm2", "Maniac Mansion Mania Mini Masterpieces Ep. 2: Wendy im finsteren Keller" },
	{ "mmmmm3", "Maniac Mansion Mania Mini Masterpieces Ep. 3: Harrys verwegener Tag" },
	{ "mmmmm4", "Maniac Mansion Mania Mini Masterpieces Ep. 4: Berthold's Return / Bertholds Rueckkehr" },
	{ "mmmmm5", "Maniac Mansion Mania Mini Masterpieces Ep. 5: Marcys Bestimmung" },
	{ "mmmmovie1", "Maniac Mansion Mania Movies: Doktor in da House" },
	{ "mmmmovie2", "Maniac Mansion Mania Movies: Doktor in da House II" },
	{ "mmmmovie3", "Maniac Mansion Mania Movies: Kochen mit Fred und Ed" },
	{ "mmmmovie4", "Maniac Mansion Mania Movies: The new president" },
	{ "mmmmovie5", "Maniac Mansion Mania Movies: Doktor in da House III" },
	{ "mmmmovie6", "Maniac Mansion Mania Movies: Just Maniac Mansion Mania" },
	{ "mmmmovie7", "Maniac Mansion Mania Movies: 5th Maniac Birthday - Der Film" },
	{ "mmmmovie8", "Maniac Mansion Mania Movies: Dinner for One" },
	{ "mmmpolicemolest", "Maniac Mansion Mania: Police Molest" },
	{ "mmmtrash1", "Maniac Mansion Mania Trash: Frühjahrsputz" },
	{ "mmmtrash2", "Maniac Mansion Mania Trash: Herrschaft der Futanaris" },
	{ "mmmtrash3", "Maniac Mansion Mania Trash: Herrschaft der Futanaris 2" },
	{ "mmmtrash4", "Maniac Mansion Mania Trash: Herrschaft der Futanaris 3" },
	{ "mmmtrash5", "Maniac Mansion Mania Trash: Herrschaft der Futanaris 4" },
	{ "mmmtrash6", "Maniac Mansion Mania Trash: NoGUI" },
	{ "mmmtrash7", "Maniac Mansion Mania Trash: Iasons Testepisode" },
	{ "mmmtrash8", "Maniac Mansion Mania Trash: MS: F**king Vista!" },
	{ "mmmtrash9", "Maniac Mansion Mania Trash: Rettet Sandy" },
	{ "mmmtrash10", "Maniac Mansion Mania Trash: Wink Smiley sucht den MMM-Millionär" },
	{ "mmmtrash11", "Maniac Mansion Mania Trash: Jagd auf rotes Tentakel" },
	{ "mmmtrash12", "Maniac Mansion Mania Trash: Run Hoagie Run" },
	{ "mmmtrash13", "Maniac Mansion Mania Trash: Bernard muss mahl" },
	{ "mmmtrash14", "Maniac Mansion Mania Trash: Bernard muss 2 mahl" },
	{ "mmmtrash15", "Maniac Mansion Mania Trash: Wink Smiley sucht den MMMMillionär - Vol. II" },
	{ "mmmtrash16", "Maniac Mansion Mania Trash: Wink Smiley sucht den Trashmillionär" },
	{ "mmmtrash17", "Maniac Mansion Mania Trash: Wink Smiley sucht den MMMM - Bloody Halloween Version" },
	{ "mmmtrash18", "Maniac Mansion Mania Trash: Bernard muss 3 mahl" },
	{ "mmmtrash19", "Maniac Mansion Mania Trash: Bernard muss 4 mahl" },
	{ "mmmtrash20", "Maniac Mansion Mania Trash: Britney muss mahl" },
	{ "mmmtrash21", "Maniac Mansion Mania Trash: Smash The Tentacle" },
	{ "mmmtrash22", "Maniac Mansion Mania Trash: Eine Traumepisode" },
	{ "mmmwayne", "Maniac Mansion Mania: Wayne Shyster - Ace Attorney" },
	{ "mmmxmas2008", "Maniac Mansion Mania Christmas 2008: A Christmas Odyssey" },
	{ "mmmxmas2015", "Maniac Mansion Mania Christmas 2015: Three Days Before Christmas" },
	{ "mobileangel", "Mobile Angel" },
	{ "moncul", "Mon Cul!" },
	{ "moneycab", "Money Cab" },
	{ "monkeyklon", "A Clone of Monkey Island" },
	{ "monkeymountain", "Monkey Mountain" },
	{ "monkeystothemoon", "Monkeys to the Moon" },
	{ "monkeywrench", "Monkey Wrench Issue 1: Paying The Bills" },
	{ "monsterfromthehountedhill", "Monster from the hounted hill" },
	{ "monsterwantsin", "Monster Wants In" },
	{ "montykomodo", "Monty the Komodo dragon" },
	{ "montyonthenorm", "Monty on the Norm" },
	{ "montypythonmansion", "Monty Python's Makeout Mansion" },
	{ "moonlightmoggy", "Moonlight Moggy" },
	{ "moorlandsonata", "Moorland Sonata" },
	{ "mooserage", "Moose Rage - part 1" },
	{ "mooserage2", "Moose Rage - part 2" },
	{ "mops", "The Memory Obfuscation of Patrick Sei" },
	{ "mordy1", "Mordy: On Holiday" },
	{ "mordy2", "Mordy 2: The Mirror of Truth" },
	{ "moremonkeys", "More Monkeys" },
	{ "morganale1", "Morgan Ale, Case 1: Professor D" },
	{ "morningshift", "Morning Shift" },
	{ "morphine", "Morphine" },
	{ "mort", "MORT: Manageably OK Response Team" },
	{ "mortifer", "Nous, les Mortifer" },
	{ "motlpaa", "MOTLPAA" },
	{ "mourirenmer", "Mourir en Mer" },
	{ "mouseoverslept", "The Mouse Who Overslept" },
	{ "moustachequest", "Moustache Quest" },
	{ "mrbeareng", "Mr. Bear Teaches English" },
	{ "mrchocolate", "The House of Mr. Chocolate" },
	{ "mrdangerscontest", "Mr. Danger's Contest" },
	{ "mrfrisby", "Mr. Frisby Saves Xhristmas" },
	{ "mrtijerakis", "Mr. Tijerakis" },
	{ "mudlarks", "Mudlarks" },
	{ "murderdog4", "Murder Dog IV: Trial of the Murder Dog" },
	{ "murdergrisly", "Murder most grisly!!" },
	{ "murderinawheel", "Murder in a Wheel / Mord im Laufrad" },
	{ "murderinminnesota", "A Murder Mystery In Minnesota" },
	{ "murderinthemansion", "Murder in the Mansion" },
	{ "murderofadrianelkwood", "The Murder of Adrian Elkwood" },
	{ "murderstarlight", "Murder on the Starlight Express" },
	{ "murphyssalvage1", "Murphy's Salvage - Mission 01: Just My Luck" },
	{ "murranchronicles1", "Murran Chronicles - Episode 1: Jersey Devil" },
	{ "murranchronicles2", "Murran Chronicles - Episode 2: Talons of Terror" },
	{ "murranchronicles3", "Murran Chronicles - Episode 3: Lifedrinker of Landsdowne" },
	{ "mushroomman", "The Mushroom Man" },
	{ "musicmixer", "Boyd's Music Mixer" },
	{ "mutagen", "Mutagen" },
	{ "muuyeeb", "MuuYeeb the Ghost" },
	{ "myburdentokeep", "My Burden to Keep" },
	{ "mycroftsim", "Mycroft Holmes Simulator 1895" },
	{ "myfathersecret", "My Father's Secret" },
	{ "myfirstbigadv", "My First Big Adventure" },
	{ "myfirstquest", "My First Quest" },
	{ "mysiblings", "My Siblings, The Stones" },
	{ "mysterioushouse", "Mysterious House" },
	{ "mysterymeat", "Mystery Meat Mayhem" },
	{ "mysticseer", "Mystic Seer" },
	{ "mythicalgambitflawlessfatality", "Mythical Gambit: Flawless Fatality" },
	{ "nadir", "Nadir" },
	{ "nakedfear", "Naked Fear" },
	{ "nanobots", "Nanobots" },
	{ "nauticell", "Nauticell" },
	{ "necroquest", "Necroquest - The Inheritance, Chapter 1" },
	{ "nedysadventure", "Nedy's Adventure: The curse of Vera" },
	{ "nefasto", "Nefasto's Misadventure: Meeting Noeroze" },
	{ "neiroalice", "NeiroAlice" },
	{ "nekusnewtrip", "Neku's new trip" },
	{ "nellycootalot", "Nelly Cootalot: Spoonbeaks Ahoy!" },
	{ "neonwilderness", "Neon Wilderness" },
	{ "neosaires2070", "Neos Aires 2070: Sunday Comes First" },
	{ "nerdyquest", "Nerdy Quest" },
	{ "nesquest", "NES Quest" },
	{ "neveralonehl", "Never Alone Hotline" },
	{ "neverlandmonopoly", "Dread Mac Farlane: Neverland Monopoly" },
	{ "newcity", "New City" },
	{ "newkidgottasteal", "New Kid Gotta Steal Somethin'" },
	{ "news", "N.orth E.ast W.est S.outh" },
	{ "nexttoevil", "Next to Evil" },
	{ "nickitandrun", "Al Gurbish in... Nick It & Run!!!" },
	{ "niemandsland", "Das Niemandsland" },
	{ "nightandday", "Night and Day" },
	{ "nighthag", "Night Hag Syndrome" },
	{ "nightmare", "Nightmare" },
	{ "nightoftheravingfeminist", "Night of the Raving Feminist" },
	{ "nightofthetesticle", "Night of the Testicle" },
	{ "nightowl", "The Night Owl Murder" },
	{ "nightwatch", "NightWatch (2008)" },
	{ "nightwitch", "Night Witch" },
	{ "nightwitches", "Night Witches: Women of the clouds" },
	{ "nightwork", "Nightwork" },
	{ "ninatonnerre", "Nina Tonnerre" },
	{ "noactionjackson", "No-Action Jackson" },
	{ "noahsquest", "Noah's Quest" },
	{ "nobodycares", "Nobody Cares" },
	{ "nocturnedream", "Nocturne and the Unseen Dream" },
	{ "nodriver", "Welcome to Nod River" },
	{ "nofear", "Where No Fear Was" },
	{ "noiamspartacus", "No, I Am Spartacus!" },
	{ "noirbois", "Noir Bois: The Prequel Prelude Saga" },
	{ "noisymountain", "Noisy Mountain" },
	{ "nomonkeysbanana", "No Monkey's Banana" },
	{ "norbisquest", "Norbi's quest" },
	{ "norbisquest15", "Norbi's quest 1,5" },
	{ "norbisquest2", "Norbi's quest 2" },
	{ "norbiwinterspecial", "Norbi winter special" },
	{ "normalday", "Normal Day" },
	{ "normancooks", "Norman Cooks in: \"Search for the Don\"" },
	{ "norserunereader", "Norse Rune Reader" },
	{ "nosferatu", "Nosferatu: Big City" },
	{ "nostalgik", "NOSTALGIK" },
	{ "nosunrise", "There Ain't No Sunrise?" },
	{ "notebookdetective", "Notebook Detective" },
	{ "notetoself", "Note to Self" },
	{ "notexplainable", "not the explainable" },
	{ "notfine", "Not Fine" },
	{ "notmyron", "Not My Ron" },
	{ "notravellerreturns", "No Traveller Returns" },
	{ "noughtscrosses", "Noughts & Crosses" },
	{ "nsfware", "NSFWare" },
	{ "ntgtfoi", "NTGTFOI - The Game" },
	{ "nukemdukem", "NukemDukem Forever" },
	{ "numberrescue", "Number Rescue" },
	{ "oakleygame", "Oakley's Game" },
	{ "obesebob", "Treasure of Obese Bob" },
	{ "objectdreams", "The Object of All Dreams" },
	{ "objectroom", "Object Room" },
	{ "obsession", "Obsession" },
	{ "odottamaton", "Odot Tamat On" },
	{ "odow", "One Dungeon One Week" },
	{ "odr3", "Outrage Dreamer Roads 3" },
	{ "odr4", "Outrage Dreamer Roads 4" },
	{ "officeolympics", "Office Olympics XP" },
	{ "officereturned", "Office of the Returned" },
	{ "officeshenanigans", "Office Shenanigans" },
	{ "offtheclock", "Off The Clock" },
	{ "ohdulieber", "Oh du lieber Augustin" },
	{ "ohitschristmas", "Oh, it's Christmas" },
	{ "ohnonotagain", "Oh no not again!" },
	{ "oldmansea", "The Old Man and the Sea" },
	{ "olol", "Olol" },  // aka Custard's 1 hour game
	{ "omfgparadox", "OMFGPARADOX!" },
	{ "omnipotenttarot", "Omnipotent Tarot" },
	{ "onceuponacrime", "Once Upon A Crime" },
	{ "onceuponatime", "Once Upon A Time" },
	{ "onceuponatime70s", "Once upon a time in the '70s" },
	{ "onceuponatimekeel", "Once Upon a Time in Keel" },
	{ "one", "One" },
	{ "oneofthem", "One of them" },
	{ "onemorefathom", "One More Fathom" },
	{ "oneofakind", "One of A Kind: a divine comedy of mistakes" },
	{ "onerainyday", "One Rainy Day" },
	{ "oneroom", "One Room" },
	{ "oneroomonecheese", "One Room, One Cheese" },
	{ "onespytoomany", "One spy too many" },
	{ "onethatremains", "The One That Remains" },
	{ "oneweekoneroom", "One Week, One Room" },
	{ "onleavingthebuilding", "On Leaving The Building" },
	{ "onlythegooddieyoung", "Only The Good Die Young" },
	{ "openquest", "OpenQuest" },
	{ "operationforklift", "Operation: FORKLIFT" },
	{ "operationnovi", "Operation Novi" },
	{ "operationreddice", "Operation: Red Dice" },
	{ "operationsavebluecup", "Operation Save Blue Cup" },
	{ "orangeman", "The Orange man" },
	{ "osd2", "Oceanspirit Dennis 2" },
	{ "osd3d", "Oceanspirit Dennis 3D" },
	{ "osdanish", "Oceanspirit Danish" },
	{ "osdarayofhope", "Oceanspirit Dennis: A Ray of Hope" },
	{ "osdarchipelago", "Oceanspirit Dennis - Archipelago Adventures 1" },
	{ "osddeath", "The Death of Oceanspirit Dennis" },
	{ "osdenise", "Oceanspirit Denise: Heroics In The Nick Of Time!" },
	{ "osdfamous", "Oceanspirit Dennis Sorta 99% FAMOUS" },
	{ "osdholiday", "Oceanspirit Dennis's Holiday Havoc!" },
	{ "osdlastboss", "Oceanspirit Dennis - Last Boss" },
	{ "osdlgs", "Oceanspirit Dennis: Leather Gear Smooth" },
	{ "osdlol", "LOL, Oceanspirit Dennis" },
	{ "osdlostworld", "Oceanspirit Dennis: The Lost World" },
	{ "osdmancake", "A Taste of Man Cake" },
	{ "osdmightypirate", "Oceanspirit Dennis: Mighty Pirate" },
	{ "osdmightyviking", "Oceanspirit Dennis: Mighty Viking" },
	{ "osdmoby", "Oceanspirit Dennis Moby GearWhaleDX" },
	{ "osdninja", "Oceanspirit Dennis: The Naked & The Ninja" },
	{ "osdocd", "Oceanspirit Dennis: The Full Name Of This Game Won't Fit In The Subject Line!!1" },
	{ "osdpoop", "Oceanspirit Dennis: Pirates on the Poopdeck!" },
	{ "osdprincess", "Oceanspirit Dennis: The Terror of the Ice Princess" },
	{ "osdrots", "Oceanspirit Dennis: Rise of the Spirit" },
	{ "osdrpg", "Oceanspirit Dennis: The RPG" },
	{ "osdscourge", "Oceanspirit Dennis: Scourge of the Underworld" },
	{ "osdscourgehd", "Oceanspirit Dennis: Scourge of the Underworld HD" },
	{ "osdsearch", "The Search for Oceanspirit Dennis" },
	{ "osdshindig", "The Shindig of Dennis" },
	{ "osdsquares", "Oceanspirit Dennis: [][][][][][][][][]" },
	{ "osdsweet", "Oceanspirit Dennis: Sweet Revenge" },
	{ "osdtextual", "Oceanspirit Dennis Gets Textual" },
	{ "osdvaginity", "Oceanspirit Dennis LOSERS HIS VAGINITY" },
	{ "osdvscloud", "Oceanspirit Dennis vs. Cloud Strife" },
	{ "osdvsron", "Oceanspirit Dennis vs. Reality-on-the-Norm" },
	{ "osdwetspot", "Oceanspirit Dennis: The Wet Spot" },
	{ "osher", "Boundless Osher / Osher Bli Gvulot" },
	{ "ossuarium", "Ossuarium" },
	{ "otakurivals", "Otaku Rivals" },
	{ "otherworlds", "Other Worlds" },
	{ "otisbuildsafire", "Otis Builds a Fire" },
	{ "ourfinesthour", "Our Finest Hour" },
	{ "ouroborossacrifice", "Ouroboros: The Sacrifice" },
	{ "ourobouros", "I am Ourobouros" },
	{ "outbreak", "Outbreak" },
	{ "outbreakwarehouse", "Outbreak: The Warehouse" },
	{ "outlawgold", "Outlaw's Gold" },
	{ "outofgas", "Out of Gas" },
	{ "outscore", "Outscore" },
	{ "outtajuice", "Outta' Juice" },
	{ "overroger", "Over Roger" },
	{ "overtheedge", "The Journey Down: Over the Edge" },
	{ "owlhunt", "Owl Hunt" },
	{ "owlsquest", "Owl's Quest: Every Owl has its Day" },
	{ "ozorwell1", "Oz Orwell and the Crawling Chaos" },
	{ "ozorwell2", "Oz Orwell and the Exorcist" },
	{ "palacin", "Palacin" },
	{ "palettequest", "Palette Quest" },
	{ "palettequest2", "Palette Quest 2.0" },
	{ "pandainspace", "Panda In Space" },
	{ "pandor", "Pandor" },
	{ "paperkubik", "Lost in the Tomb / Paper-Kubik" },
	{ "paperplanes", "Paper Planes" },
	{ "paradiselost", "Larrywilco in: Paradise Lost" },
	{ "paradiseroad", "Paradise Road" },
	{ "parameciumcomplex", "The Paramecium Complex" },
	{ "paramnesia", "Paramnesia" },
	{ "paranoid", "Paranoid!" },
	{ "paranormalinvestigation", "Paranormal Investigation" },
	{ "party", "Party" },
	{ "patchesspazcat", "The Misadventures of Patches the Spazcat" },
	{ "patchwork", "Patchwork" },
	{ "pathskinwalker", "Path of the Skinwalker" },
	{ "paulainwonderland", "Paula in Wonderland" },
	{ "paulmooseinspaceworld", "Paul Moose In Space World" },
	{ "paulquest", "Paul Quest Gold Edition" },
	{ "paulromano", "Paul Romano - Recollection 1" },
	{ "peakvalley1", "Mysteries of Peak Valley 1 - The Lost Sonata" },
	{ "peakvalley2", "Mysteries of Peak Valley 2 - The White Lady" },
	{ "pendek", "Pendek" },
	{ "pengaobcster", "Penga & Obcster's Christmish Adventure" },
	{ "penguincy", "Penguincy" },
	{ "pennispong", "Pennis: The Ultimate in Pong!" },
	{ "pepeadventure", "Pepe's Little Adventure" },
	{ "perceptions", "Perceptions" },
	{ "perelman", "Perelman" },
	{ "perilsofpoom", "Perils of Poom" },
	{ "persephonediary", "Persephone's Diary: Secrets of the South" },
	{ "pesterquest", "Pester Quest: From n00b to newb" },
	{ "petshopincident", "The Pet Shop Incident" },
	{ "petalrose", "Petals Around the Rose" },
	{ "pharmacistjones", "Pharmacist Jones" },
	{ "philadvch1", "Phil's Adventure - Chapitre 1" },
	{ "philococoa", "Philosophers Like Hot Cocoa" },
	{ "phoenix1", "Phoenix1" },
	{ "phonoi", "Phonoi" },
	{ "pickpocketrpg", "Pickpocket RPG" },
	{ "pie", "Pie!" },
	{ "piginapoke", "Pig in a poke" },
	{ "pilotlight", "Pilot Light" },
	{ "pimpinonparakuss", "Pimpin On Parakuss IV" },
	{ "pinkcult", "Rex and Sissi in Pink Cult" },
	{ "pinksky", "Pink Sky" },
	{ "pinkyalien", "Pinky the Silly Alien" },
	{ "piratefry2", "Pirate Fry 2: The Hand of Anturus" },
	{ "piratefry3", "Pirate Fry 3: The Isle of the Dead" },
	{ "pirates", "Pirates!" },
	{ "piratescaribbean", "Pirates of Monkey Island of the Caribbean" },
	{ "piratess", "Piratess - Season of the Silver Moon" },
	{ "piss", "PISS" },
	{ "pixel", "PIXEL" },
	{ "pixelhunt2005", "Pixel Hunt (2005)" },
	{ "pixelhunt2007", "Pixel Hunt (2007)" },
	{ "pixia", "PiXiA: Rainbow of Havoc" },
	{ "pixxxelhunter", "Pixxxelhunter: The Epic" },
	{ "pizzacalls", "Pizza Calls" },
	{ "pizzanostra", "Pizza Nostra" },
	{ "plan10frommypants", "Plan 10 from MY PANTS!!!" },
	{ "planetxmas", "Planet Xmas" },
	{ "planm", "Plan M" },
	{ "plantsimulator", "Plant Simulator" },
	{ "plasticmines", "Plastic Mines" },
	{ "platformerius", "Platformerius - The Ninja Incident!" },
	{ "platformhorde", "Platform Horde" },
	{ "playitagain", "Play It Again: An Improv Point And Click Adventure" },
	{ "playoflife", "The Play of Life" },
	{ "playxylo", "Play Xylophone & Singing Cat" },
	{ "pledgequest1", "Pledge Quest I: The SpaceVenture Adventure" },
	{ "pledgequest2", "Pledge Quest II: Noodle Shop of Horrors" },
	{ "pleurghburgdarkages", "Pleurghburg: Dark Ages" },
	{ "plumberboy", "PlumberBoy" },
	{ "pmquestions", "Prime Minister's Questions: The Game" },
	{ "pmuvchvt", "PMUVCHVT" },
	{ "pocketfluff", "PocketFluff! Yay!" },
	{ "pocketquest", "Pocket Quest" },
	{ "poddwellers", "Pod Dwellers" },
	{ "poeng", "POEng'n'Klik" },
	{ "politicalenemy", "Political Enemy" },
	{ "politicallyyours", "Politically Yours" },
	{ "pompadourpete", "Pompadour Pete On The Road To Goblin Keep" },
	{ "ponderabilia", "Ponderabilia" },
	{ "pong", "PONG" },
	{ "postcardadv", "Postcard Adventure" },
	{ "postmansquest", "Postman's Quest - Not Rain nor Sleet nor ARMAGEDDON" },
	{ "potatohead", "Potato Head: Sweet Attack!" },
	{ "potionmagique", "La Potion Magique" },
	{ "potionmaster", "The Potion Master" },
	{ "poudlardmotus", "Poudlard Motus" },
	{ "pouvoir", "pouvoir" },
	{ "powernap", "Power Nap" },
	{ "powerunlimited", "Power Unlimited Boardgame" },
	{ "pq4sciish", "Police Quest IV SCI(ish)" },
	{ "pqtadventure", "Adventures of PQT" },
	{ "practicescript", "Practice Script: The 1-Room Puzzler" },
	{ "predatorspreyforplants", "Predators Prey For Plants" },
	{ "preludetoadventure", "A Prelude to an Adventure" },
	{ "preptime", "Prep Time" },
	{ "priderelatives", "Pride & Relatives" },
	{ "primordiaconduit", "Primordia: The Conduit Cut" },
	{ "princedickless", "Prince Dickless" },
	{ "princeoflordenp1", "The Lost Prince Of Lorden - Part 1" },
	{ "princessandallthekingdom", "The Princess and all the Kingdom" },
	{ "princessmarian1", "The Marvellous Adventures of Princess Marian" },
	{ "princessmarian2", "The Marvellous Adventures of Princess Marian part II: Mother's Day" },
	{ "princessmarian3", "The Marvellous Adventures of Princess Marian part III: Happy Birthday Little Flower" },
	{ "princessmarian4", "The Dark Cave: Adventures of Princess Marian part IV" },
	{ "princessmarian5", "Alba the Explorer: Princess Marian part V" },
	{ "princessmarian6", "Princess Marian and the Fountain of Unicorns (Princess Marian part VI)" },
	{ "princessmarian7", "C.U.T.E. (Princess Marian VII)" },
	{ "princessmarian8", "Princess Marian VIII: Snow Fight!!!" },
	{ "princessmarian9", "Princess Marian and The Phantom of the Ballet (Princess Marian IX)" },
	{ "princessmarian10", "The Adventures of Princess Marian part X: Magic Hat" },
	{ "princessmarian11", "Princess Marian XI: Light Re-leaf" },
	{ "princessmarianspigeonpinger", "Princess Marian's Pigeon Pinger" },
	{ "principlesofevil", "Principles of Evil" },
	{ "principlesofevil2", "Principles of Evil II: Misery Loves Company" },
	{ "prisonbreakout", "Prison Breakout" },
	{ "privatedetective", "Private Detective" },
	{ "procrastinator", "Procrastinator" },
	{ "prodigal0", "Prodigal 0" },
	{ "prodigal", "Prodigal" },
	{ "prodigalshooter", "Prodigal Shooter" },
	{ "profneely", "Professor Neely And The Death Ray Of Doom" },
	{ "projectdevonia", "Project Devonia: \'Manticore\'" },
	{ "projectevilspy2", "Project Evilspy II" },
	{ "projectlazarus", "Project Lazarus" },
	{ "projectmadness", "Project Madness" },
	{ "proofoffiction", "Proof of Fiction" },
	{ "proposal", "Proposal" },
	{ "prototypical", "Prototypical" },
	{ "providence", "Providence" },
	{ "proxecto", "Proxecto Percebe" },
	{ "psychicsam", "Psychic Sam and the Postal Service of Doom" },
	{ "psychofb", "Psycho Flashback" },
	{ "psychopomp", "Psychopomp" },
	{ "pubmasterquest", "Pub Master Quest" },
	{ "pubmasterquest2", "Pub Master Quest II - Shogin Crystal" },
	{ "pubmasterquestlegends", "Pub Master Quest Legends" },
	{ "pubmasterquestspirits", "Pub Master Quest Wandering Spirits" },
	{ "puddypenguin", "Puddy Penguin" },
	{ "purgatorio", "Purgatorio" },
	{ "purityofthesurf", "Purity of the Surf" },
	{ "purposeretired", "Purpose : Retired" },
	{ "pussiehunt", "Pussie Hunt" },
	{ "puttputtstew", "Putt-Putt Saves The Stew" },
	{ "puzzlepumice", "Puzzle of the Pulchritudinous Pumice" },
	{ "pxenophobe", "Project Xenophobe" },
	{ "qfheroes", "Quest for Heroes" },
	{ "questblackdiamond", "The Quest for the Black Diamond" },
	{ "questfighter", "Quest Fighter" },
	{ "questfighter2", "Quest Fighter II" },
	{ "questforcinema", "Quest For Cinema" },
	{ "questforjesus", "Quest for Jesus" },
	{ "questfororgy", "Quest for Orgy I: So You Want to be a Porn Star" },
	{ "questforthebluecup", "Quest for the Blue Cup" },
	{ "questforyeti", "Quest for Yeti" },
	{ "questforyrolg", "Quest for Yrolg" },
	{ "quietgame", "The Quiet Game" },
	{ "quimbyquestanewdope", "Quimby Quest I: A New Dope" },
	{ "quiteannoying", "The Quite Annoying League" },
	{ "raastaja", "Raastaja" },
	{ "raataja", "Raataja" },
	{ "rabbiteyemotel", "An Evening At The RabbitEye Motel" },
	{ "rabbithill", "Rabbit Hill" },
	{ "race", "R.ACE" },
	{ "racingmanager", "AS Racing Manager" },
	{ "racist", "Racist" },
	{ "rackham", "Rackham" },
	{ "rainblood", "Rain of Blood" },
	{ "rainbowskunk", "Rainbow Skunk Prism and the Vermin-Truder" },
	{ "rainbowtube", "Billy And Desmond's Fantastic Amazing Rainbow Tube" },
	{ "rainerlesadv1", "Reinerles Abentuer - Episode 1: Das Renovierungs-Desaster" },
	{ "rainsnow", "Rain & Snow - The Bouncer" },
	{ "ralphtheraven", "Ralph the Raven" },
	{ "ramghost", "RAM Ghost" },
	{ "rampitup", "Ramp It Up" },
	{ "ramsesporter", "Ramses Porter and the Relay for Love" },
	{ "rango", "Rango" },
	{ "rapsqlud", "RapSqLud" },
	{ "rapstar15", "Rapstar 1.5" },
	{ "ratchannel", "Rat Channel" },
	{ "ratpackcastle", "The Rat Pack Do Cheesy Castle" },
	{ "ratplaying", "Rat Playing Game" },
	{ "ravench1", "Raven - Chapter 1: The Commands of Eurybia" },
	{ "rayandtheguitar", "Ray and the Guitar" },
	{ "raybexter", "Ray Bexter" },
	{ "raysrods", "Ray's Rods" },
	{ "razorsinthenight", "Razors in the Night" },
	{ "rcpd", "Robot City Police Department" },
	{ "reactor09", "Reactor 09" },
	{ "readyeddie", "READY starring Ready Eddie" },
	{ "reagentorange", "Re-Agent Orange" },
	{ "realitycheck1", "Reality Check" },
	{ "realitycheck2", "Reality Check 2" },
	{ "realitycheck3", "Reality Check 3" },
	{ "realitycheck4", "Reality Check 4" },
	{ "realityinthenorm", "Reality-in-the-Norm" },
	{ "recess", "Recess" },
	{ "recess2", "Recess 2" },
	{ "recollection", "Recollection" },
	{ "redbeardsavesron", "Red Beard Saves Reality-on-the-Norm" },
	{ "reddwarf", "Red Dwarf" },
	{ "redflaggredux", "Red Flagg in: Don't Call me Blue! REDUX" },
	{ "redhotoverdrive", "Red Hot Overdrive" },
	{ "redpantsep1", "The Adventures of Redpants: The Princess and the Beanstalk" },
	{ "redpantsep2", "Redpants Meets Robinson Clauseau" },
	{ "reefriversquestforekoban", "Reef Rivers: Quest for Ekoban" },
	{ "rein", "rein" },
	{ "remotelyinteresting", "(not even) Remotely Interesting" },
	{ "rend", "The Tapestry - Chapter 2: Rend" },
	{ "renuncio", "Renuncio!" },
	{ "renuncio2", "Renuncio: Parte 2 - El Escape" },
	{ "reonquestep1", "Reon Quest Ep. I: The Mysterious Aluminatti Society" },
	{ "reonquestep2", "Reon Quest Ep. II: Escape From Bully Island" },
	{ "repossessor", "The Repossessor" },
	{ "requiem", "Requiem" },
	{ "researchreservations", "Research Reservations" },
	{ "researchresident", "Research Resident" },
	{ "retaliation", "Reality-on-the-Norm 13:13 Retaliation" },
	{ "retroron", "Retroron" },
	{ "returnjourney", "Return Journey" },
	{ "returntocivilization", "Return To Civilization" },
	{ "revelation", "Revelation" },
	{ "revenants", "Revenants" },
	{ "rickyquest", "Ricky Quest" },
	{ "righteouscityp1", "Righteous City - part I" },
	{ "righteouscityp2", "Righteous City - part II" },
	{ "ripperjack", "Ripper Jack" },
	{ "rnbquest", "R'n'B Quest" },
	{ "roadbrollywood", "Road to Brollywood" },
	{ "roadofdestiny", "Road of Destiny" },
	{ "roadracer", "Road Racer" },
	{ "roastmothergoose", "Roast Mother Goose" },
	{ "robbingtheprincess", "Robbing The Princess" },
	{ "robertredford1", "Robert Redford Saves The Day Ep. 1: Getting There" },
	{ "robertredford2", "Robert Redford Saves The Day Ep. 2: The Pit and the Pendulum" },
	{ "robertredford3", "Robert Redford Saves The Day Ep. 3: Who Framed Roger Redford" },
	{ "robertredford3ds", "Robert Redford Saves The Day Ep. 3: Who Framed Roger Redford (Dream Sequence)" },
	{ "robmassacreofchainsawness", "Rob: The Massacre of Chainsawness" },
	{ "robolution", "Robolution" },
	{ "roboquest", "Robo Quest" },
	{ "roboquest2009", "ROBO-QUEST 2009" },
	{ "robotragedy", "Robotragedy" },
	{ "robotragedy2", "Robotragedy 2 - Countdown To Doomsday" },
	{ "robotsdream", "Do Robots Dream of Ice Cream?" },
	{ "roccioquest", "Roccio Quest" },
	{ "rockabillykid", "Rockabilly Kid" },
	{ "rockatruestory", "Rock - A True Story" },
	{ "rockburgerstreehouses", "Doc.Death in: Rock Burgers & Tree Houses" },
	{ "rockpaperscissors", "AGS Rock, Paper, Scissors" },
	{ "rockpaperscissors2", "Rock, Paper, Scissors! Reboot" },
	{ "rockrockrock", "Rock Rock Rock" },
	{ "rocktravis", "Rock Travis - Camilla's case" },
	{ "rockyroams", "Rocky roams" },
	{ "rodequest2", "Rode Quest 2: The Sweet Smell of Stenchie" },
	{ "rogered", "Rogered" },
	{ "rogerlameadv", "Roger's Lame Adventure" },
	{ "rogeroddsp1", "Roger Against The Odds - Part 1: Trapped in the lab" },
	{ "rogerquest", "Roger's Quest" },
	{ "rogertreasure", "Roger Finds Treasure" },
	{ "rogue", "Rogue" },
	{ "romanian", "Romanian Beginner" },
	{ "ron5thanniversary", "Reality-on-the-Norm 5th Anniversary Trailer" },
	{ "ronbeforethelegacy", "Reality-on-the-Norm: Before the Legacy" },
	{ "ronendgame", "End Game" },
	{ "rongyptian", "RoNgyptian" },
	{ "ronoutbreak", "The Outbreak (Day of Comet)" },
	{ "ronsixteen", "Sixteen" },
	{ "rontimeouttrailer", "Time Out Trailer" },
	{ "rootofallevil", "Root of All Evil" },
	{ "rosauradocelestial", "Rosaura Docelestial: Rescue from Despair" },
	{ "rosellahelm", "Rosella Wilson Meets Helm" },
	{ "rossnoble", "Ross Noble Arena" },
	{ "rotla", "Riders of the Lost Ark" },
	{ "roughdiamond", "Rough Diamond" },
	{ "rowengoestowork", "Rowen Goes To Work" },
	{ "rtmi", "Return to Monkey Island" },
	{ "rudeawakening", "Rude Awakening" },
	{ "rufusstory", "Rufus' Story" },
	{ "runaway", "RunAway" },
	{ "runestones", "Runestones!" },
	{ "ruptquest", "Rupt Quest" },
	{ "ryansdayout", "Ryan's Day Out" },
	{ "sabotagenb", "Sabotage on Noegato-Bas" },
	{ "saddsonissein", "Saddson Issein" },
	{ "sagemonkey", "Great Sage Monkey" },
	{ "saladecosmique", "Salade Cosmique" },
	{ "salazarsevilplan", "Salazar's Evil Plan" },
	{ "salt", "Salt" },
	{ "samarkand", "Samarkand" },
	{ "sammysperm", "Sammy Sperm" },
	{ "sammysquest", "Sammy's Quest" },
	{ "sandiknievel", "Sandi Knievel: Stunt Rider" },
	{ "sandmen", "Sandmen" },
	{ "santaclausdown", "Santa Claus Down" },
	{ "santaflight", "Santa Claus in A Flight To Remember" },
	{ "santaorphanage", "Santa and the orphanage" },
	{ "santaquest", "Santa Quest" },
	{ "santassidekick", "Santa's Sidekick" },
	{ "santasstolensleigh", "Santa's Stolen Sleigh" },
	{ "sargasso", "Sargasso" },
	{ "sarmanyanha", "The Manuscripts of Sarmanyanha" },
	{ "satanquest", "Satan Quest" },
	{ "satchsquest", "Satch's Quest" },
	{ "saturdaynightlone", "Saturday Night is the Loneliest Night of the Week" },
	{ "saturdaynightlone2", "Saturday Night is the Loneliest Night of the Week 2" },
	{ "saturdayschool", "Saturday School" },
	{ "saturdaysymbiosis", "Saturday Night Symbiosis" },
	{ "saw", "Saw" },
	{ "saw2", "Saw II" },
	{ "sawn1", "Sawn 1: Pain is just the start!" },
	{ "scalestraining", "Scales Training Game" },
	{ "scarecrowquest", "Scarecrow and his quest for a brain" },
	{ "scaredstiff", "Scared Stiff" },
	{ "scarehouse", "Scare House" },
	{ "scarymaze", "Scary Maze Halloween 2016" },
	{ "scenario5b", "Scenario 5B" },
	{ "schnelltrial", "The Trial of the Schnellersparrow" },
	{ "schoollout", "Schoollout" },
	{ "science", "Science!" },
	{ "sciencesfaer", "Les Sciences du Faër" },
	{ "scnidersom", "Scnidersom" },
	{ "scotchanimation", "Scotch's Animation Demo" },
	{ "scramschool", "Scram School 3 Challenge 17: \"Petezah Time III: Petezah in Space\"" },
	{ "scrapmanager", "S.A.M. - Scrap Allocation Manager" },
	{ "scratchingaway", "Scratching Away" },
	{ "screamingsouls", "Screaming Souls" },
	{ "scumpub", "Scum Pub" },
	{ "scyllacharybdis", "Scylla and Charybdis: A Grecian Ship From Olympus U" },
	{ "scytheisland", "Scythe Island" },
	{ "searchforsanity", "Search for Sanity" },
	{ "searchofmarina", "In search of Marina" },
	{ "seashells", "Seashells" },
	{ "seasongreetings2002", "Season Greetings 2002" },
	{ "secondplace", "Second Place is for Losers" },
	{ "secondstime", "Seconds Time's the Charm" },
	{ "secretorbs", "The Second Secret of the Orbs" },
	{ "secretquestremake", "Secret Quest Remake" },
	{ "secrets", "Secrets" },
	{ "seed", "Seed" },
	{ "seekye", "Seek, and ye shall find" },
	{ "self", "Self" },
	{ "senpaigetaway", "Senpai Getaway" },
	{ "sepulchre", "Sepulchre" },
	{ "serina", "Serina's Transylvanian Trip" },
	{ "serum", "Serum" },
	{ "sesari", "Sesari" },
	{ "sevendoors", "The Seven Doors" },
	{ "sevgilim","Sevgilim Olur musun?" },
	{ "shadesofgrey", "Shades of Grey" },
	{ "shadesofgreye", "Shades of Greye" },
	{ "shadowgate", "Shadowgate Remake" },
	{ "shadowsofron", "Shadows of Reality-on-the-Norm" },
	{ "shadowsoftheempire", "Star Wars: Shadows of the Empire - Graphic Adventure" },
	{ "shailadusithlenquete", "Shai-la du Sith : l'Enquete / The Investigation" },
	{ "shailaofthesith", "Shai-la of the Sith / Shai-la du Sith" },
	{ "shailaofthesithv2ep1", "Shai-la of the Sith V2 (Remake) - Episode 1" },
	{ "shailaofthesithv2ep2", "Shai-la of the Sith V2 (Remake) - Episode 2" },
	{ "shailaofthesithv2ep3", "Shai-la of the Sith V2 (Remake) - Episode 3" },
	{ "shailaofthesithv2ep4", "Shai-la of the Sith V2 (Remake) - Episode 4" },
	{ "shailaofthesithv2ep5", "Shai-la of the Sith V2 (Remake) - Episode 5" },
	{ "shamrockhg", "Shamrock High Graduation" },
	{ "shapeshift", "Shapeshift for Cheese" },
	{ "shapevillage", "Shape Village" },
	{ "shardsofgod", "Shards of God" },
	{ "sharethis", "Share This With Your Friends" },
	{ "sharkysthree", "Sharky's 3: The Heist" },
	{ "shawshank", "The Shawshank Redemption" },
	{ "sheepquest", "Sheep Quest" },
	{ "sheetart", "Sheet: The art of Art" },
	{ "shemwood", "Shemwood Plains" },
	{ "sherlock", "Sherlock - The Dark Arts" },
	{ "sherwood", "Sherwood" },
	{ "shifter", "Shifter" },
	{ "shifters", "Shifters" },
	{ "shiftersboxoutsidein", "Shifter's Box - Outside In" },
	{ "shittyquest", "Shitty Quest" },
	{ "shootabducted", "Shoot, I Got Abducted!" },
	{ "shootinggame", "The Shooting Game" },
	{ "shootmyvalentine", "Shoot My Valentine" },
	{ "shortcut", "ShortCut" },
	{ "shrivel", "Shrivel" },
	{ "shunday", "Shunday" },
	{ "sierraquest1", "Sierra Quest 1: Roberta in Love" },
	{ "signalloss", "Signal Loss" },
	{ "silentknight1", "Silent Knight - Chapter One: The Mediocre Escape" },
	{ "silentknight2", "Silent Knight - Chapter Two: The Conscience of the King" },
	{ "silentnightcreche", "Silent Night Creche" },
	{ "simonsjourney", "Simon's Journey" },
	{ "simonthesorcerer3", "Simon the Sorcerer 3" },
	{ "simonthesorcerersbrother", "Simon, The Sorcerer's Brother" },
	{ "simppmdee", "Simppmdee" },
	{ "simshogwarts", "Sims Hogwarts" },
	{ "sinbad", "Sinbad - The Island of Korkus" },
	{ "sinfluencer", "Sinfluencer" },
	{ "sinking", "Sinking" },
	{ "siren", "Siren" },
	{ "sisterhelper", "Sister's Little Helper" },
	{ "sisterrevenge", "Revenge of the Nagging Sister" },
	{ "sisyphus", "Sisyphus Reborn" },
	{ "skippysavestheday", "Skippy Saves The Day" },
	{ "skumring", "Skumring" },
	{ "skyadventure", "Sky Adventure" },
	{ "skyfall", "Skyfall" },
	{ "skytowerrescue", "Skytower Rescue" },
	{ "slaythedragon", "Slay the Dragon" },
	{ "slaythedragon2", "Slay the Dragon II" },
	{ "slaythedragon3", "Slay the Dragon III" },
	{ "sleepingbeauty", "Sleeping Beauty" },
	{ "sleepingcastle", "The Sleeping Castle" },
	{ "sleepyisland", "The Sleepy Island" },
	{ "sleuth2020remake", "SLEUTH Remake" },
	{ "sleuthch1", "Sleuth - Chapter One" },
	{ "slickadv", "Slick Adventures: The Warehouse" },
	{ "slidersquest", "Sliders Quest" },
	{ "slimequestforpizza", "Slime Quest for Pizza" },
	{ "slothforseasons", "A Sloth For Both Seasons" },
	{ "slugprincess", "Slug Princess" },
	{ "slvsaus", "SL vs. AUS 07" },
	{ "sma2", "Second Moon Adventure - Part II: White Moon or Red Moon" },
	{ "sma3", "Second Moon Adventure - Part III: Rest In Peace" },
	{ "sma4", "Second Moon Adventure - Part IV: Abendmond" },
	{ "sma5", "Second Moon Adventure - Part V: Next Evolution" },
	{ "sma6", "Second Moon Adventure - Part VI: Mysterious Time Travel" },
	{ "sma7", "Second Moon Adventure - Part VII: Ocean Life" },
	{ "sma8", "Second Moon Adventure - Part VIII: Ungewisse Herkunft" },
	{ "sma9", "Second Moon Adventure - Part IX: Zwei Welten in Einer" },
	{ "sma10", "Second Moon Adventure - Part X: Walder Hier Und Dort" },
	{ "sma11", "Second Moon Adventure - Part XI: Jahrtausendwende" },
	{ "smasher", "Smasher" },
	{ "smgilbert", "Sam & Max - Il caso Gilbert" },
	{ "smileordie", "Smile or Die" },
	{ "smileysquest", "Smiley's Quest" },
	{ "smileysquest2", "Smiley's Quest 2" },
	{ "smokinweed", "Smokin' Weed" },
	{ "smoothhide", "Smooth Hide" },
	{ "snake", "Snake" },
	{ "snakesofavalon", "Snakes of Avalon" },
	{ "snakesonaplane", "Snakes on a plane!" },
	{ "snipermotherland", "Sniper and spotter serving the motherland" },
	{ "sniperpatriotic", "Sniper and spotter being patriotic" },
	{ "snipertower", "Sniper and spotter climbing a tower" },
	{ "snowmaneatcarrot", "May the Best Snowman Eat a Carrot" },
	{ "snowqueen", "The Snow Queen: After the Apocalypse" },
	{ "snowtheadventure", "SNOW - The Adventure Game" },
	{ "sockstoday", "Should Have Worn Socks Today" },
	{ "solitude", "Solitude" },
	{ "somethingnovel", "Something Novel" },
	{ "somethingpipes", "There's Something in the Pipes" },
	{ "somewhere", "Somewhere" },
	{ "somnamulizer", "Somnamulizer: A Tale From Olympus U" },
	{ "songanimals", "Song Animals" },
	{ "sonicandfriendsinclubhouse", "Sonic and friends in: Club House" },
	{ "sophiamcgrath", "Sophia McGrath and the Strange Invitation" },
	{ "sorenquest", "Soren's Quest" },
	{ "sosk", "Secrets of Sultan Kanuni" },
	{ "soulsquest", "Souls Quest" },
	{ "sovietunterzoegersdorf1", "Soviet Unterzögersdorf: Sector I" },
	{ "sovietunterzoegersdorf2", "Soviet Unterzögersdorf: Sector II" },
	{ "spacebirdmissile", "Space Bird Missile Cats" },
	{ "spacecatvirus", "Space Cat vs. Virus" },
	{ "spacecross", "Space Cross: The BSG-Team" },
	{ "spacefreakers", "Space Freakers" },
	{ "spacegremlin", "Space Gremlin" },
	{ "spacehunter", "Space Hunter" },
	{ "spacelynxes", "Space Lynxes" },
	{ "spacemadness", "Space Madness" },
	{ "spacemanspace", "Spaceman in Space" },
	{ "spacemaze", "Space Maze" },
	{ "spacemisadventures", "Space Misadventures" },
	{ "spacepirates", "Space Pirates" },
	{ "spacepoolalpha", "Space Pool Alpha" },
	{ "spacerangersep46", "Space Rangers S.O.B. Ep. 46: The Devil Within" },
	{ "spacerangersep52", "Space Rangers S.O.B. Ep. 52: The Redemption of Grisli Adams" },
	{ "spacesim", "Space Sim - AGS 3D" },
	{ "spacetrash", "Space Trash" },
	{ "spacetunneler", "Space Tunneler" },
	{ "spacewarep1", "Space War - Episode 1: The Crystal" },
	{ "spacewarep2", "Space War - Episode 2: Curien Strikes Back" },
	{ "speedbuggy", "Speed Buggy: Manifold Destiny" },
	{ "spellbound", "Spellbound: A Clive Mandrake Adventure" },
	{ "spia", "Spia" },
	{ "spidertrek", "Spider Trek" },
	{ "spilakassinn", "Spilakassinn - The Slot Machine" },
	{ "spiritboard", "Midnight Spirit Board" },
	{ "spitboy", "Spit Boy" },
	{ "splendorsolis", "Splendor Solis" },
	{ "splinter", "Splinter" },
	{ "splitfighters", "Split Fighters" },
	{ "splurt", "Splurt in Wasted Town" },
	{ "spmachinima1", "SP Machinima - Episode 1" },
	{ "spmachinima2", "SP Machinima - Episode 2" },
	{ "spooks", "Spooks" },
	{ "spoonsiii", "Spoons III - The Unauthorized Edition" },
	{ "spottd_boyd", "Spot The Difference (2003)" },
	{ "spottd_captaind", "Spot The Difference (2015)" },
	{ "sproutsofevil", "Sprouts of Evil" },
	{ "sq2svga", "Space Quest II SVGA Remake" },
	{ "sq3vga", "Space Quest III VGA Preview" },
	{ "sq45", "Space Quest IV.5 - Roger Wilco And The Voyage Home" },
	{ "sq55", "Space Quest 5.5: Save Captain Roger" },
	{ "sq7m1", "Space Quest 7 Mania Ep. 1" },
	{ "sq7m2", "Space Quest 7 Mania Ep. 2" },
	{ "sqdote", "Space Quest Minus 1: Decisions of the Elders" },
	{ "sqdote2", "Space Quest: A Son of Xenon" },
	{ "sqinc", "Space Quest: Incinerations" },
	{ "sqkubik", "SQ Kubik" },
	{ "sqm11", "Space Quest Mania 1x1" },
	{ "sqm12", "Space Quest Mania 1x2" },
	{ "sqm13", "Space Quest Mania 1x3" },
	{ "sqm14", "Space Quest Mania 1x4" },
	{ "sqm15", "Space Quest Mania 1x5" },
	{ "sqm16", "Space Quest Mania 1x6" },
	{ "sqos", "SQ Online Strike" },
	{ "sqpotim", "Space Quest: Planet of the Incredible Menace" },
	{ "sqvn", "Space Quest Visual Novel" },
	{ "sqvsb", "Space Quest: Vohaul Strikes Back" },
	{ "sram2", "SRAM 2 - Cinomeh's Revenge" },
	{ "sstrek25", "Super Star Trek meets 25th Anniversary" },
	{ "stablepeteandthejoust", "Stable Pete and the Joust" },
	{ "stairquest", "Stair Quest" },
	{ "stanamespiepisode1", "Stan Ames, Private Eye - Episode 1: Murder Incorporated" },
	{ "standupaliens", "Standup for Aliens" },
	{ "stansrevenge", "Stan's Revenge" },
	{ "starfreakers", "StarFreakers" },
	{ "stargateadv", "Stargate Adventure" },
	{ "stargateatlantis", "Stargate: The City of Atlantis" },
	{ "stargatequizz", "Stargate Quizz" },
	{ "stargatesgc", "Stargate SGC" },
	{ "stargatesolitaire", "Stargate Solitaire" },
	{ "starlitgrave", "Starlit Grave" },
	{ "starshipcaramba", "Starship Caramba" },
	{ "starshiphindenburg", "Last Flight of the Starship Hindenburg" },
	{ "starshiplight", "Starship Light" },
	{ "starshipposeidon", "Starship Poseidon - Part 1" },
	{ "startrekgalactique", "Star Trek - Aventure Galactique" },
	{ "startrekgloriousep1", "Star Trek: Glorious Wolf - Episode 1" },
	{ "startrekgloriousep2", "Star Trek: Glorious Wolf - Episode 2" },
	{ "startrekgloriousremake", "Star Trek: Glorious Wolf Remake" },
	{ "startrekmansion", "Star Trek - Back To The Mansion" },
	{ "startrekmission", "Star Trek Mission" },
	{ "startreknewton", "Star Trek Newton - Part One: Anomaly" },
	{ "startropy", "Startropy" },
	{ "stateofmind", "State of Mind" },
	{ "steamedhams", "Steamed Hams: The Graphic Adventure" },
	{ "steamsquares", "Steamsquares" },
	{ "steamtrek", "Steam Trek: Clueless around Risa" },
	{ "stediddyip1employment", "Stediddy in IP1 - Employment" },
	{ "steeplejack", "Steeplejack" },
	{ "stephenkingcars", "Stephen King DodgeEmCars" },
	{ "stevequest", "Steve Quest" },
	{ "stickamhour1", "Stickam Hour Game - Downfallen Superhero" },
	{ "stickamhour2", "Stickam Hour Game - Pirates" },
	{ "stickemup", "Stick 'em Up" },
	{ "stickmanquest", "Stick Man Quest" },
	{ "stickmanrpg", "Stickman RPG" },
	{ "stickmenterrorists", "Stickmen in the land of the terrorists" },
	{ "stickycrimsonhouse", "Sticky the Stick Figure Part 1: The Crimson House" },
	{ "stolenmoustache", "The Case of the Stolen Moustache" },
	{ "stormpuncher", "Not My First Apocalypse: Stormpuncher" },
	{ "stranded", "Stranded" },
	{ "strangerbynight", "Stranger by Night" },
	{ "strangerinstickworld", "Stranger in Stickworld" },
	{ "strangerthings", "Stranger Things" },
	{ "strawmanaugment", "The Strawman Augment" },
	{ "strawmanbomb", "Strawman Bombing Disposal" },
	{ "stuckathome", "Stuck at Home" },
	{ "stuckinamuddle", "Stuck in a Muddle with You" },
	{ "stuckonyou", "Stuck on you" },
	{ "studiomediocre", "Studio Mediocre: The Case of the Bedeaded Dramatist" },
	{ "stygiophobia", "Stygiophobia Another Day" },
	{ "subatomic", "subAtomic" },
	{ "subterra", "Subterra" },
	{ "subwaycrisis", "Subway Crisis" },
	{ "suenanfantasmas", "¿Sueñan Los Fantasmas?" },
	{ "summerwoes", "Summer Woes" },
	{ "sunriseparadise", "Sunrise Paradise" },
	{ "supaevil", "Supa-EviL" },
	{ "superegoch0", "Superego - Chapter 0" },
	{ "supergirl", "Supergirl in: We Don't Need Another Hero" },
	{ "superhammerquest", "Super Hama Queste" },
	{ "supernaturaltt", "Supernatural: The Terror Trio" },
	{ "superpitstopracing", "Super Pitstop Racing" },
	{ "suppaninja", "Suppa Ninja" },
	{ "surreality", "Surreality" },
	{ "suspiciousmind", "Suspicious Mind" },
	{ "svengordan1", "Sven Gordan, Paranormal Parody - Case 1: In search of Big Apes" },
	{ "sweetmythery", "Sweet Mythery" },
	{ "sweetnightmares", "Sweet Nightmares" },
	{ "swordremake", "Sword - The First Remake" },
	{ "sydneyfindsemployment", "Sydney Finds Employment" },
	{ "sydneytreadsthecatwalk", "Sydney Treads the Catwalk" },
	{ "symbiosis", "Symbiosis" },
	{ "syncroutine", "sync::routine" },
	{ "tablarawls", "La Tabla de Rawls" },
	{ "tabletsoftibet", "The Sacred Tablets of Tibet" },
	{ "tactician", "Tactician: Civil War" },
	{ "taleofalegacy", "A Tale of a Legacy / En busca de un legado" },
	{ "talesfromtheroad", "Tales From the Road" },
	{ "talesofchickenry", "Tales of Chickenry" },
	{ "talesofinfimum", "Tales of INFIMUM" },
	{ "talesofjayvin", "Tales of Jayvin" },
	{ "talesofotubania", "Tales of Otubania" },
	{ "taoch1ep1", "Tao - Chapter 1: Le Grand Tournoi (Ep. 1)" },
	{ "taoch1ep2", "Tao - Chapter 1: Le Grand Tournoi (Ep. 2)" },
	{ "taoch1ep3", "Tao - Chapter 1: Le Grand Tournoi (Ep. 3)" },
	{ "taoch1ep4", "Tao - Chapter 1: Le Grand Tournoi (Ep. 4)" },
	{ "taoch2ep1", "Tao - Chapter 2: L'Université Faërique (Ep. 1)" },
	{ "taoch2ep2", "Tao - Chapter 2: L'Université Faërique (Ep. 2)" },
	{ "taoch2ep3", "Tao - Chapter 2: L'Université Faërique (Ep. 3)" },
	{ "taoch2ep4", "Tao - Chapter 2: L'Université Faërique (Ep. 4)" },
	{ "taoch3", "Tao - Chapter 3: Le Dernier Combat" },
	{ "taospacetime", "Tao Through Space and Time" },
	{ "tarthenia", "Tarthenia" },
	{ "tasteblood", "Taste the blood of darkness" },
	{ "tattooroom", "The Tattoo Room" },
	{ "taverntales", "Tales From A Forgotten Tavern" },
	{ "teafortwo", "Tea for Two - A Detective Logan Case" },
	{ "technobabylonp3", "Technobabylon - Part 3: In Nuntius Veritas" },
	{ "tedmcbinky", "Ted McBinky and his Steam Engine" },
	{ "teethglass", "Teeth of Glass" },
	{ "tehhorror", "Teh Horror!" },
	{ "templeofspheres", "Indiana Jones and the Temple of Spheres" },
	{ "tenhumstombpart1", "Tenhum's Tomb Part 1" },
	{ "tenweeks", "Ten Weeks" },
	{ "terrorofthevampire", "Terror of the Vampire!" },
	{ "teruteru", "Teru Teru Bozu" },
	{ "tetricity", "TetriCity" },
	{ "textparsergame", "Text Parser Game" },
	{ "tfg-bttf", "The Fan Game - Back to the Future - Interactive Adaptation" },
	{ "tfg-bttf3", "The Fan Game - Back to the Future Part III: Timeline Of Monkey Island" },
	{ "tfg-bttf4", "The Fan Game - Back to the Future Part IV: The Multitasking Crystal" },
	{ "tfg-bttf5", "The Fan Game - Back to the Future Part V: Multidimensional Space-Timelines" },
	{ "tfg-bttfwat", "The Fan Game - I'll Be Back to the Future With A Terminator" },
	{ "tfg-mi", "The Fan Game - Ghostbusters and The Secret of Monkey Island" },
	{ "tfg-goonies", "The Fan Game - The Goonies" },
	{ "tfg-pixel", "The Fan Game - The Pixel Has You - Disk 1" },
	{ "tfg-somi", "The Fan Game - The Secret of Monkey Island - RECODED" },
	{ "tfg-ssgw", "The Fan Game - Saint Seiya - Galaxian Wars" },
	{ "tfg-zak2", "The Fan Game - Zak McKracken: A Mansion, a Meteor and the Alien Mindbenders" },
	{ "tftoz1", "Tales From The Outer Zone: Cyborg Seppuku" },
	{ "tftoz2", "Tales From The Outer Zone: The Goat Crone" },
	{ "tftoz3", "Tales From The Outer Zone: Fleshworms" },
	{ "tftoz4", "Tales From The Outer Zone: The Construction" },
	{ "tharsheblows", "Thar she blows!" },
	{ "thatcrazyworld", "That Crazy World" },
	{ "thatdamndog", "THAT DAMN DOG!" },
	{ "thatday", "That Day" },
	{ "the30minutewar", "Duke Stanley, National Hero in: The 30 Minute War" },
	{ "the4thwall", "The 4th Wall" },
	{ "the7thsense", "The 7th Sense" },
	{ "theabtyoncase", "Tales from the Eureka Cluster : The Abtyon Case" },
	{ "theaddict", "The Addict" },
	{ "theadventureofthehero", "The Adventure of the Hero" },
	{ "theadventuresofturquoise", "The Adventures of Turquoise MacDonald" },
	{ "theagencyp1", "The Agency: Part One - The Conspiracy" },
	{ "theartifact", "The Artifact" },
	{ "theaspiroxcase", "The Aspirox Case / L'Affaire Aspirox" },
	{ "theassassin", "The Assassin" },
	{ "theawakening", "The Awakening" },
	{ "thebar", "The Bar" },
	{ "thebasement", "The Basement" },
	{ "theblock", "The Block" },
	{ "thebrokenbrain", "The Broken Brain" },
	{ "thebum", "The Bum" },
	{ "thebunker", "The Bunker" },
	{ "theburgerflipper", "The Burger Flipper" },
	{ "theburgomaster", "The Burgomaster" },
	{ "thebutler", "The butler didn't do it" },
	{ "thecabin", "The Cabin" },
	{ "thecadaversynod", "The cadaver synod" },
	{ "thecan", "The Can" },
	{ "thecatcase", "The Cat Case" },
	{ "thecatsman", "The Man Who Liked Cats" },
	{ "thecell2005", "The Cell (2005)" },
	{ "thecell2017", "The Cell (2017)" },
	{ "thechronomancers", "The Chronomancers" },
	{ "thechrysalis", "The Chrysalis" },
	{ "thecityadv", "The City Adventure" },
	{ "thecomingofage", "The Coming of Age - a Lorna Bains whodunit" },
	{ "thecondemned", "The Condemned" },
	{ "thecradle", "The Cradle" },
	{ "thecrazedchicken", "The Crazed Chicken" },
	{ "thecrystalball", "The Crystal Ball" },
	{ "thecube", "The Cube" },
	{ "thecurseoflife", "The Curse of Life" },
	{ "thedarkplague", "The Dark Plague" },
	{ "thedarktrial", "The Dark Trial" },
	{ "thedayafter", "The day after you went away" },
	{ "thedaynothinghappened", "The Day Nothing Happened" },
	{ "thedayofdarkness", "The Day Of Darkness" },
	{ "thedeathoflukesimpson", "The Death of Luke Simpson" },
	{ "thedecorcist", "The Decorcist" },
	{ "thedeed", "The Deed" },
	{ "thedelivery", "The Delivery" },
	{ "thedevilsshroudpart1", "The Devil's Shroud - Part I" },
	{ "thedevilsshroudpart2", "The Devil's Shroud - Part II" },
	{ "thedevilsshroudpart3", "The Devil's Shroud - Part III" },
	{ "thedevilsshroudpart4", "The Devil's Shroud - Part IV" },
	{ "thedevilsteeth", "The Devil's Teeth" },
	{ "thedigitalspell", "The Digital Spell" },
	{ "thedisgracedprince", "The disgraced prince" },
	{ "thedistantdoor", "The Distant Door" },
	{ "thedollhousech1", "The Dollhouse - Chapter 1: Plaything" },
	{ "thedome", "The Dome" },
	{ "thedreamjobep1", "The Dream Job, ep. 1" },
	{ "thedusseldorfconspiracy", "Jessica Plunkenstein and the Düsseldorf Conspiracy" },
	{ "thedwarvendaggerofblitz", "The Dwarven Dagger of Blitz" },
	{ "thedwarvendaggerofblitzch2", "The Dwarven Dagger of Blitz - Chapter 2" },
	{ "theelevator", "The Elevator" },
	{ "theendlessnight", "The Endless Night" },
	{ "theenergizer", "The Energizer" },
	{ "theepicadventures", "The epic adventures of Barry" },
	{ "theerrand", "The Errand" },
	{ "theeternalnight", "The Eternal Night" },
	{ "theeverbeginningtale", "The Ever-Beginning Tale" },
	{ "theexecutionofanneboleyn", "The Execution of Anne Boleyn" },
	{ "theexperimentp1", "The Experiment - Part 1: The Laboratory" },
	{ "theexperimentp2", "The Experiment - Part 2: Menacing Darkness" },
	{ "thefaketrix", "The Faketrix" },
	{ "thefamilytreasure", "The Family Treasure" },
	{ "thefarmep1", "The Farm - Episode One" },
	{ "thefarmmags", "The Farm" },
	{ "theficklehandsoffate", "The Fickle Hands of Fate" },
	{ "thefind", "The Find" },
	{ "thefront", "The Front" },
	{ "thefrozenshore", "The Frozen Shore" },
	{ "thefurtheststation", "The Furthest Station" },
	{ "thegardenofhades", "The Garden of Hades" },
	{ "thegift", "The Gift" },
	{ "theglasssplinters", "The Glass Splinters" },
	{ "thegourmet", "The Gourmet" },
	{ "thegreatcasserolecaper", "The Great Casserole Caper" },
	{ "thegreatstrokeoff", "The Great Stroke-Off!" },
	{ "thegreenroom", "The Greenroom - An Escape Room Adventure" },
	{ "thegruglegends", "The Grug Legends" },
	{ "theguilteternal", "The Guilt Eternal" },
	{ "thehamlet", "The Hamlet" },
	{ "thehat", "The Hat" },
	{ "thehauntedhouse", "The Haunted House" },
	{ "theheist", "The Heist" },
	{ "thehobbitriseofthedragonking", "The Hobbit: Rise of the Dragon King" },
	{ "thehousethatatemysoul", "The house that ate my soul" },
	{ "thehousewithoutwindows", "The House Without Windows" },
	{ "thehuntforgoldbeard", "The hunt for Goldbeard" },
	{ "thehuntforshaunbinda", "The hunt for Shaun Binda" },
	{ "theinfinitystring", "The Infinity String" },
	{ "theiraqiparadox", "The Iraqi Paradox" },
	{ "thejackyard", "The Jackyard" },
	{ "thejimihendrixcase", "The Jimi Hendrix Case" },
	{ "thejourneyhomep1", "The Journey Home - Part One" },
	{ "thejourneyofiesir", "The Journey of Iesir" },
	{ "thelastharvest", "The Last Harvest" },
	{ "thelastsupperawhodunnit", "The Last Supper, A Whodunnit" },
	{ "thelighthouse", "The Lighthouse" },
	{ "thelightningspell", "The Lightning Spell" },
	{ "theloneloser", "The Lone Loser" },
	{ "theloneplanet", "The Lone Planet" },
	{ "thelongtrip", "The Long Trip" },
	{ "thelurkinghorror", "The Lurking Horror" },
	{ "themajesticconspiracy", "The Majestic Conspiracy" },
	{ "themaninthehat", "The Man in the Hat" },
	{ "themanisee", "The Man That Only I Can See" },
	{ "themarionette", "The Marionette" },
	{ "themccarthychroniclesep1", "The McCarthy Chronicles - Episode 1" },
	{ "themcreedcase", "The McReed Case" },
	{ "themind", "The Mind" },
	{ "themissinghat", "G. Ramirez and The Missing Hat" },
	{ "themuseum", "The Museum" },
	{ "themysteriesofstiegomoor1", "The Mysteries of Stiego Moor - Volume One: The Wicker Man" },
	{ "themysteryofhauntedhollow", "The Mystery of Haunted Hollow" },
	{ "thenetherworld", "The Netherworld" },
	{ "thenextcurse", "The Next Curse" },
	{ "thenorthcrown", "The North Crown" },
	{ "theoccultist1", "The Occultist - Old Growth" },
	{ "theoccultist2", "The Occultist - Bridge Under Troubled Water" },
	{ "theoffice", "The Office" },
	{ "theoracle", "The Oracle" },
	{ "theoscillation", "The Oscillation" },
	{ "theoven", "The Oven" },
	{ "thepaparazziprince", "The Paparazzi Prince; and the Quest for Headlines" },
	{ "thepark", "The Park" },
	{ "theparrotsnatchers", "The Parrot Snatchers" },
	{ "thepassenger", "The Passenger" },
	{ "thepathpumpkin", "The Path of the Pumpkin" },
	{ "thepenthouse", "The Penthouse" },
	{ "theperfectmurder", "The Perfect Murder" },
	{ "thephantominheritance", "The Phantom Inheritance" },
	{ "thequestfortheholysalsa", "The Quest for the Holy Salsa" },
	{ "thequesttozooloo", "The Quest To Zooloo" },
	{ "therail", "The Rail" },
	{ "theratpack", "The Rat Pack" },
	{ "there", "There" },
	{ "thereaper", "The Reaper" },
	{ "therebirth", "The Rebirth" },
	{ "therent", "The Rent / L'Affitto" },
	{ "theroadtomurder", "Laura Bow in: The Road to Murder" },
	{ "theroadtrip", "The Road Trip" },
	{ "therobolovers", "The Robolovers" },
	{ "therotaryclub", "The Rotary Club" },
	{ "theschool", "The School" },
	{ "thesearch", "The Search" },
	{ "thesecretofchunkysalsa", "The Secret of Chunky Salsa" },
	{ "thesecretofgoldenriver", "Fynewrylst Memories: The Secret of Golden River" },
	{ "thesecretofhuttongrammarschool", "The Secret of Hutton Church of England Grammar School" },
	{ "thesecretofmaikeisland", "The Secret of Maike Island" },
	{ "thesecretofmountmonkey", "The Secret of Mount Monkey" },
	{ "thesecretplanch1", "The Secret Plan - Chapter 1: Running to stand still" },
	{ "theshadowavenger", "The Shadow Avenger: It's personal!" },
	{ "theshaft", "The Shaft" },
	{ "theshortestjourney", "The Shortest Journey" },
	{ "thesmallestpoints", "The Smallest Points" },
	{ "thesnaplock", "The Snaplock" },
	{ "thespiderweb", "The Spider's Web" },
	{ "thespoons", "The Spoons" },
	{ "thestarryskyaboveme", "The Starry Sky Above Me" },
	{ "thestinker", "The Stinker" },
	{ "thesummoned", "The Summoned" },
	{ "thesundownmystery", "The Sundown mystery" },
	{ "thesurvivors", "The Survivors / Les Survivants" },
	{ "thesylph", "The Colorless Sylph" },
	{ "thethickening", "Reality-on-the-Norm 13:13^2 The Thickening" },
	{ "thetombofthemoon", "The Tomb of the Moon" },
	{ "thetowerpuzzles", "The Tower" },
	{ "thetrap", "The Trap - A Darcy Muldoon Adventure" },
	{ "thetreasuredmedallion", "The Treasured Medallion" },
	{ "thetreasureoflochinch", "The Treasure of Loch Inch" },
	{ "theuncertaintymachine", "The Uncertainty Machine" },
	{ "theunicated", "The Unicated" },
	{ "theupliftmofopartyplan", "MI5 Bob: The Uplift Mofo Party Plan" },
	{ "thevacuum", "The Vacuum" },
	{ "thevillage", "The Village" },
	{ "thevirus", "The Virus" },
	{ "thevisitor1", "The Visitor" },
	{ "thevisitor2", "The Visitor 2" },
	{ "thevisitor3", "The Visitor 3" },
	{ "thevoid", "The Void" },
	{ "thewhitecanvas", "The White Canvas / Lienzo en Blanco" },
	{ "thewife", "The wife who wasn't there" },
	{ "thewill", "The Will" },
	{ "theworm", "The Worm" },
	{ "thinker", "Thinker" },
	{ "thiscitynight", "This City at Night" },
	{ "thisgame", "This Game" },
	{ "thisoddfeeling", "This Odd Feeling" },
	{ "thistall", "You must be this tall" },
	{ "thisway", "This is the Way" },
	{ "threeguyswalkintoheaven", "Three Guys Walk Into Heaven" },
	{ "threepwoodnightmare", "Threepwood's Nightmare" },
	{ "throwme", "Throw Me in the River" },
	{ "thrymly", "Thrymly Disguised" },
	{ "tigerhawk", "Tiger Hawk Squadron" },
	{ "tijdtripper", "TijdTripper" },
	{ "tilepuzzle", "Tile Puzzle" },
	{ "tillcows", "'Til Cows Tear us Apart" },
	{ "tiltor", "TiLTOR" },
	{ "timeoutjoint", "Time Out of Joint" },
	{ "timeparadox", "TIME-PARADOX 2009" },
	{ "timeparadoxdan", "Time Paradox" },  // Dan's Time Paradox Hourgame
	{ "timeparadoxenk", "Enkerro's Time Paradox" },
	{ "timequest1", "Time Quest" },
	{ "timequest2", "Time Quest 2" },
	{ "timesinkofchronos", "Timesink of Chronos" },
	{ "timestone", "Time Stone" },
	{ "timetrial", "Time Trial" },
	{ "timothylande", "Timothy Lande" },
	{ "tinysoccer", "Tiny Soccer Manager Stories" },
	{ "tirnanog", "Tir Na Nog" },
	{ "toffeetrouble", "Toffee Trouble in Creamville" },
	{ "tomandjimi", "Tom and Jimi in Blowout!" },
	{ "tomateundescanso", "Tómate un Descanso" },
	{ "tombhunter", "Tomb Hunter: Ramitupem" },
	{ "tomeslayne", "Tomes: Layne's Discovery" },
	{ "tomhanksaway", "Tom Hanks Away: Wilson's Revenge" },
	{ "tomten", "Tomten" },
	{ "tonyspaghetti", "Tony's Spaghetti" },
	{ "tooblivion", "Who sent me to Oblivion" },
	{ "toomanykittens", "Too Many Kittens" },
	{ "topus", "You rock, Topus!" },
	{ "totheend", "To the end of the way" },
	{ "tothemom", "To The Mountains Of Madness" },
	{ "tots", "T.O.T.S. : Trick-Or-Treat Simulator" },
	{ "totti1", "Totti's missing claw" },
	{ "totti2", "Good Evening Totti" },
	{ "tradeforward", "Trade it forward" },
	{ "trancepacific", "Trance-Pacific" },
	{ "trappedbalcony", "Trapped On The Balcony" },
	{ "trappedhell1", "Trapped with Hell" },  // AKA Rich Psycho
	{ "trappedhell2", "Trapped with Hell: Cruising" },
	{ "trappedinabuilding", "Trapped in a building" },
	{ "trapodroz", "Fantastyczna podróż Tytusa, Romka i A'tomka" },
	{ "trashquest", "Trash Quest" },
	{ "travelers", "TRAVELERS" },
	{ "tresacordes", "Tres Acordes" },
	{ "trevordaisoninouterspace", "Trevor Daison in outer space - Chapter One: Trouble on Percicron IV" },
	{ "trexmusclesam1", "T-Rex and Muscle Sam: Big Trouble in Smashing Potatoes Federation" },
	{ "trexmusclesam2", "T-Rex and Muscle Sam: A New Kickstart!" },
	{ "trianglehead", "Triangle Head's Adventure" },
	{ "trickers", "Trickers" },
	{ "trilbysnotes", "Trilby's Notes" },
	{ "trilbytheartoftheft", "Trilby: The Art Of Theft" },
	{ "trippy", "Trippy" },
	{ "trivialpassyouit", "Trivial Pass You It" },
	{ "trollsong", "Troll Song - Verse One: Completely Stoned" },
	{ "troopers", "Troopers" },
	{ "tropicjim", "Tropic Jim's Sweet Island Adventure" },
	{ "troublecorner", "Trouble on the Corner of Sugar & Spice" },
	{ "troublekingdom", "Trouble Kingdom" },
	{ "troublingteleportation", "Troubling Teleportation" },
	{ "trumpsbed", "Shit in Trump's Bed" },
	{ "truthmatter", "The Truth of the Matter" },
	{ "tsomi2", "The Secret of Monkey Island 2" },
	{ "tunnelvision", "Tunnel Vision" },
	{ "turtlesintime", "Turtles Ninja in Time" },
	{ "tvabroder", "Two Brothers / Två bröder" },
	{ "tvquest2015", "Tv Quest" },
	{ "twelvethirteense", "Twelve Thirteen - Special Edition" },
	{ "twelvethirteenep1", "Twelve Thirteen - Episode 1" },
	{ "twelvethirteenep2", "Twelve Thirteen - Episode 2" },
	{ "twelvethirteenep3", "Twelve Thirteen - Episode 3" },
	{ "twentiesflappersvsthemummy", "Twenties Flappers vs. The Mummy" },
	{ "twoghosts", "Two ghosts of that house: the missing toy" },
	{ "twoofakind", "Two of a Kind" },
	{ "ugalembrace", "Ugal's Embrace" },
	{ "uglyfiles", "The Ugly Files" },
	{ "ulitsadimitrova", "Ulitsa Dimitrova" },
	{ "ultimalatinvii", "Ultima Latin VII" },
	{ "unbound", "Unbound" },
	{ "uncontrollable", "UNCONTROLLABLE" },
	{ "underthebed", "The Weird Thing Under The Bed" },
	{ "underwateradv", "Underwater Adventures" },
	{ "underworld", "The Underworld" },
	{ "undyep1", "Hey Der, Undy! Episode 1: De King of Pong" },
	{ "undyep2", "Hey Der, Undy! Episode 2: Undy-neath the Christmas Tree" },
	{ "unexpectedguest", "Unexpected Guest" },
	{ "unexpectedquest", "Unexpected Quest" },
	{ "unfair", "Unfair" },
	{ "unfinished", "Unfinished" },
	{ "unfinishedbusiness", "Unfinished Business" },
	{ "unfinishedtales", "Unfinished Tales / Cuentos Inconclusos" },
	{ "unfoldingspider", "The Unfolding Spider" },
	{ "unganeedsmumba", "UNGA needs MUMBA" },
	{ "unintelligentdesign", "Unintelligent Design" },
	{ "unkhour_apple", "Unnamed Hour Game - APPLE" },
	{ "unkhour_nmhour1", "Unnamed Hour Game - NMHOUR1" },
	{ "unlabeledtape", "The Unlabeled Videotape" },
	{ "unlicensedkill", "Unlicensed to Kill" },
	{ "unprintablemagenta", "The unprintable Magenta" },
	{ "unprofe", "Un Profe" },
	{ "unraveling", "The Tapestry - Chapter 1: The Unraveling" },
	{ "unscripted", "Unscripted" },
	{ "unsolvedmystery", "Bob Smith and the Unsolved Case of Mystery" },
	{ "untildawn", "Until Dawn Tomorrow" },
	{ "untilfurthernotice", "Stories of the unexpected: 'Until further notice'" },
	{ "updatequest", "Update Quest" },
	{ "upducted", "Upducted" },
	{ "urbanwitchstory", "Urban Witch Story" },
	{ "uydearmyfriendsremake", "Urusei Yatsura: Dear My Friends Remake" },
	{ "urgentquest", "Urgent Quest" },
	{ "uropa", "U-ropa" },
	{ "utopiaoftyrant", "Utopia of Tyrant" },
	{ "vacantbodies", "Vacant Bodies" },
	{ "vacationquestthing", "Vacation Quest... Thing" },
	{ "valhallaextinction", "Valhalla Extinction" },
	{ "valis", "Valis" },
	{ "vankairbreak", "van K. Airbreak" },
	{ "vectorvendetta", "Vector Vendetta" },
	{ "vegetablepatch", "Vegetable Patch Extreem Turbo" },
	{ "vegetablepatch2", "Vegetable Patch Extreem Turbo 2" },
	{ "veggietales3d", "Veggie Tales 3D" },
	{ "venator", "Venator" },
	{ "vertigo", "Vertigo" },
	{ "verybadtaste", "Very Bad Taste: Dels don't hide!" },
	{ "veteranshootout", "Veteran shootout" },
	{ "vexationisland", "Vexation Island - The Game" },
	{ "vicwreckleshalloweencostume", "Vic Wreckle's Halloween Costume" },
	{ "vikingguardsman", "The Viking Guardsman" },
	{ "villard", "Villard Must Fall!" },
	{ "virmachina", "VIRMACHINA" },
	{ "virtualpiano", "Virtual Piano" },
	{ "vivarium", "Vivarium" },
	{ "vizita", "Vizita" },
	{ "vorezkor", "The VorezKor Hack" },
	{ "voxinrama", "Vox in Rama" },
	{ "voyagesfuturo", "Les Voyages de Futuro" },
	{ "vohaulsrevenge2", "Space Quest XII: Vohaul's Revenge II" },
	{ "voodoodave", "Voodoo Dave and the Tablecloth Mystery" },
	{ "vrrontour", "VR Reality-on-the-Norm Tour " },
	{ "wagesofdarkness", "The Wages of Darkness" },
	{ "waguogambo", "Wagu Ogambo" },
	{ "waitingboyfriend", "Waiting For Your Boyfriend Simulator" },
	{ "wakeup", "Wake Up" },
	{ "walkcyclegen", "SSH's Walkcycle Generator" },
	{ "wallardgromoid", "Wallard & Gromoid in: A Planned Day Out" },
	{ "walle", "WALL-E Fan Adventure" },
	{ "wallyweasel", "The Wacky World of Wally Weasel" },
	{ "waltersasteroid", "Walter's Asteroid" },
	{ "warningfragilech1", "Warning: Fragile! - Chapter I: Operation Blindhawk" },
	{ "warptile", "WarpTile" },
	{ "warstars", "War Stars" },
	{ "washedashore", "Washed Ashore" },
	{ "waskocleaner", "Wasko the Cleaner" },
	{ "wasted", "Wasted" },
	{ "waterquest", "Water Quest" },
	{ "wduprodigal", "World of Darkness Unbound - Prodigal" },
	{ "wearevectors", "We Are Vectors" },
	{ "weathered", "Weathered" },
	{ "weepinglilium", "Weeping Lilium" },
	{ "wegotlost", "I Think We Got Lost" },
	{ "weightloss", "Narcoleptic Weight Loss Expert" },
	{ "welcometodarklake", "Welcome to Dark Lake" },
	{ "wellmeetagain", "We'll meet again..." },
	{ "wellwellwell", "Well, well, well, what have we here?" },
	{ "wendywhedon1", "Wendy Whedon chapitre 1 : Le Ranch Mortensen" },
	{ "wendywhedon2", "Wendy Whedon chapitre 2 : Les Rejets de la Nature" },
	{ "wendywhedon3", "Wendy Whedon chapitre 3 : Le Kasshinkston Express" },
	{ "wereweever", "Were We Ever?" },
	{ "westroot", "West of the Root" },
	{ "wet", "Wet" },
	{ "whacksoul", "Whack-A-Soul" },
	{ "whackunilin", "Whack-A-Unilin" },
	{ "whataclown", "What a Clown!" },
	{ "whathappened", "What the F--K happened!?" },
	{ "whatisthat", "What is that thing" },
	{ "whatlinusbruckmansees", "What Linus Bruckman Sees When His Eyes Are Closed" },
	{ "whatsinthesafe", "What's In the Safe?" },
	{ "whatspunkyfound", "What Spunky Found" },
	{ "whattimeisit", "Artie Salomon: What time is it?" },
	{ "wheeloftorture", "Wheel of Torture" },
	{ "whentheworldcalls", "When the world calls..." },
	{ "whentimestops", "Reef Rivers in: When Time Stops" },
	{ "where", "Where?" },
	{ "wherebedragons", "Where Be Dragons?" },
	{ "wheredidhumansgo", "Where Did the Humans Go?" },
	{ "wheredidsamgo", "Where did Sam go?" },
	{ "wheresmhatma", "Where's M' Hat Ma?" },
	{ "whichwitch", "Which Witch" },
	{ "whiskeyweather", "Whiskey Weather" },
	{ "whitemoredeadep1", "White is more DEAD - Episode I" },
	{ "whitemoredeadep2", "White is more DEAD - Episode II" },
	{ "whitemoredeadep3", "White is more DEAD - Episode III" },
	{ "whodunit", "Whodunit" },
	{ "whokilledbambi", "Who Killed Bambi?" },
	{ "whokilledwho", "Who Killed Who?" },
	{ "whopper", "What a whopper!" },
	{ "whoseturn", "Whose turn is it anyway?" },
	{ "whowantstoliveagain", "James Bond - Who wants to live again?" },
	{ "whowantstoliveforever", "Who wants to live forever?" },
	{ "whywrongface", "Why the Wrong Face?" },
	{ "wickedwitchwest", "The Wicked Witch of the West" },
	{ "wilfred2088", "Wilfred: 2088" },
	{ "williamsnightmare", "William's Nightmare" },
	{ "willowhouse", "Willow House" },
	{ "winamillion", "Win a Million!" },
	{ "winnersdontdodrugs", "Winners Don't Do Drugs" },
	{ "winterrose", "The Winter Rose" },
	{ "wisp", "Wisp" },
	{ "witchcuisine", "How to: Witchcraft Cuisine" },
	{ "witchlullaby", "The Witch's Lullaby" },
	{ "witchnight", "Witch Night" },
	{ "witchron", "Witch!" },
	{ "witchwayout", "Witch Way Out" },
	{ "witchwizardcup", "The Witch, the Wizard and the Blue Cup" },
	{ "witchywoo", "Witchy Woo" },
	{ "withoutaprayer", "Without a Prayer" },
	{ "witness", "Witness!" },
	{ "wizardhangover", "Wizard Hangover ExtreamZz!" },
	{ "wizardhood", "Wizardhood" },
	{ "wolfcountry", "Wolf Country" },
	{ "woo", "WOO: World of Our Own" },
	{ "woof", "Woof!" },
	{ "woolyrockbottom", "Wooly Rockbottom and the Quest for the Golden Beard of Thor!" },
	{ "worldisweird", "The World is Weird" },
	{ "worldoftoto", "The World of Toto" },
	{ "worldofwarcraftquest", "World of Warcraft: The IMBA Quest" },
	{ "worldscollide", "Worlds Literally Collide" },
	{ "wormholech1", "Wormhole - Capítulo 1" },
	{ "wrathofthesolonoids", "Doctor Who: Wrath of the Solonoids" },
	{ "wrecked", "Wrecked" },
	{ "wretcher", "Wretcher" },
	{ "writersblocks", "Writer's Blocks" },
	{ "wrongchannel", "Wrong Channel" },
	{ "wronggame", "The Wrong Game" },
	{ "wulffenstein", "Wulffenstein – Im Kreuzfeuer der Reporter" },
	{ "y", "Y" },
	{ "yago", "Yago, the Coquerrestrial / Yago, el Coquerrestre" },
	{ "yipquest", "Yip Quest" },
	{ "yoda", "Yoda" },
	{ "yourgrace", "Your Grace" },
	{ "yourlate", "9 Minutes! Your late!" },
	{ "zak2", "The New Adventures of Zak McKracken" },
	{ "zakrepixeled", "Zak McKracken and the Alien Mindbenders - repixeled" },
	{ "zakseamonster", "Zak McKracken and the Lonely Sea Monster" },
	{ "zakthemaniac", "Zak the Maniac - An Interactive Music Video" },
	{ "zankrobot", "Zank, Robot Boy Wonder" },  // Matt Wilson's Hour Game
	{ "zapandy", "Zap & Andy: Get Outta Hell" },
	{ "zedquest", "Zed's Quest" },
	{ "zeke", "Zeke and the Island of Barentsev" },
	{ "zempquest", "Zemp's Birthday Kuest" },
	{ "ziggyshorts", "Ziggy Shorts: The Milkshake of Doom" },
	{ "zippermeteor", "The Zipper Meteor" },
	{ "zogmoonbuckle", "Zog Moonbuckle: The Mysterious Planet" },
	{ "zombieattack", "Zombie Attack" },
	{ "zombiefish", "Night of the Zombie Fish!" },
	{ "zombienation", "Zombie Nation" },
	{ "zombietown", "Zombie Town" },
	{ "zombietrainep2", "Zombie Train Beyond Earth: Episode 2" },
	{ "zooreal", "ZooReal" },
	{ "zugzwang", "Zugzwang" },

	// RuCOMM games and demos (Russian Only)

	// RuCOMM #1. The Best Man
	{ "evildead4", "Evil Dead 4" },
	{ "goodman", "Good Man" },
	{ "tbdiy", "The BAD day in year" },

	// RuCOMM #2. Laboratory / Hotel between worlds
	{ "einsteinmachine", "Einstein Machine" },

	// RuCOMM #3. Santa Claus Summit on Canaries
	{ "prisonersofice", "Prisoners of Ice" },
	{ "snowyadventures", "Snowy Adventures" },


	// AGS 3.6.1 and 4.0 games. not currently supported

	{ "apfelman", "Apfelmännchen" },
	{ "breakfastrequiem", "Breakfast Requiem" },
	{ "brianeggswoods", "Brian Eggs Is Lost In The Woods" },
	{ "bubbygod", "Bubby Kills God" },
	{ "castleescapech2", "Castle Escape - Chapter 2" },
	{ "dreadmacfarlaneisep1", "Dread Mac Farlane Interactive Story - Episode 1" },
	{ "dreadmacfarlaneisep2", "Dread Mac Farlane Interactive Story - Episode 2" },
	{ "federicoreturns", "Federico Returns" },
	{ "flowproblem", "Flow Problem" },
	{ "gloriouswolfcomicsep1", "Glorious Wolf - Interactive Musical Comics Ep. 1" },
	{ "gloriouswolfcomicsep2", "Glorious Wolf - Interactive Musical Comics Ep. 2" },
	{ "hoopoeinferno", "Hoopoe's Inferno" },
	{ "hydrahead", "Hydra's Head" },
	{ "hydrahead2", "Hydra's Head 2" },
	{ "hydrahead3", "Hydra's Head 3" },
	{ "indip", "In Dip" },
	{ "interviewwrong", "Interview Gone Wrong" },
	{ "jakoquest", "JaKo Quest" },
	{ "jaro", "Jaro" },
	{ "lonelyspaces", "Lonely Spaces" },
	{ "mathildedream", "Mathilde's Dream" },
	{ "metrocityresistance", "METRO CITY: Resistance" },
	{ "paintedhills", "Painted Hills" },
	{ "sarimento", "Sarimento on Hyperborean Island" },
	{ "simon1text", "Simon The Sorcerer 1 Text Adventure" },
	{ "stewlasmoras", "The Stew of Las Moras" },
	{ "stopags", "Stop" },
	{ "strangerutopia", "Stranger in Utopia" },
	{ "takes2tangle", "It Takes Two to Tangle" },
	{ "thezooags", "The Zoo" },
	{ "useitall", "Use It All (Or Forget Your Ball)" },
	{ "zalgohs", "Zalgo Hotstar in The Trailer" },

	{ 0, 0 }
};

const char *const PRE_25 = "Pre 2.5";

#define DETECTION_ENTRY_GUIO(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, GUIO, ADGF, FLAGS) \
	{{ ID, PLATFORM, AD_ENTRY1s(FILENAME, MD5, SIZE), LANG, \
	Common::kPlatformUnknown, ADGF, GUIO }, FLAGS }

#define DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, ADGF, FLAGS) \
	DETECTION_ENTRY_GUIO(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, GUIO1(GUIO_NOLANG), ADGF, FLAGS)


#define PRE_25_ENTRY(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Pre 2.5", ADGF_UNSTABLE, 0)

#define PRE_25_ENTRY_EN(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Pre 2.5", ADGF_UNSTABLE, 0)

#define PRE_25_ENTRY_LANG(ID, FILENAME, MD5, SIZE, LANG) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, "Pre 2.5", ADGF_UNSTABLE, 0)

#define PRE_25_DEMO_ENTRY(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Pre 2.5 Demo", ADGF_UNSTABLE | ADGF_DEMO, 0)

#define PRE_25_DEMO_ENTRY_EN(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Pre 2.5 Demo", ADGF_UNSTABLE | ADGF_DEMO, 0)

#define PRE_25_DEMO_ENTRY_LANG(ID, FILENAME, MD5, SIZE, LANG) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, "Pre 2.5 Demo", ADGF_UNSTABLE | ADGF_DEMO, 0)

#define UNSUPPORTED_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, ADGF_UNSTABLE, 0)

#define UNSUPPORTED_DEMO_ENTRY(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Demo", ADGF_UNSTABLE | ADGF_DEMO, 0)

#define UNSUPPORTED_GAME_ENTRY(ID, FILENAME, MD5, SIZE) \
	UNSUPPORTED_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, 0)


#define TESTING_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, ADGF_TESTING, 0)

#define STABLE_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, ADGF_NO_FLAGS, 0)

#define DEMO_ENTRY(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Demo", ADGF_DEMO, 0)

#define DEMO_ENTRY_EN(ID, FILENAME, MD5, SIZE) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Demo", ADGF_DEMO, 0)

#define DEMO_ENTRY_LANG(ID, FILENAME, MD5, SIZE, LANG) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, "Demo", ADGF_DEMO, 0)

#define GAME_ENTRY(ID, FILENAME, MD5, SIZE) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, 0)

#define GAME_ENTRY_EN(ID, FILENAME, MD5, SIZE) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, 0)

#define GAME_ENTRY_EN_PLATFORM(ID, FILENAME, MD5, SIZE, PLATFORM) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, PLATFORM)

#define GAME_ENTRY_LANG(ID, FILENAME, MD5, SIZE, LANG) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, LANG, 0)

#define GAME_ENTRY_LANG_PLATFORM(ID, FILENAME, MD5, SIZE, LANG, PLATFORM) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM)

#define GAME_ENTRY_STEAM(ID, FILENAME, MD5, SIZE) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Steam")

#define GAME_ENTRY_EN_STEAM(ID, FILENAME, MD5, SIZE) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Steam")

#define GAME_ENTRY_GOG(ID, FILENAME, MD5, SIZE) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "GOG.com")

#define GAME_ENTRY_EN_GOG(ID, FILENAME, MD5, SIZE) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "GOG.com")

#define GAME_ENTRY_PLATFORM(ID, FILENAME, MD5, SIZE, PLATFORM) \
	STABLE_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, PLATFORM)

#define STABLE_ENTRY_PLUGIN(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, FLAGS) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, ADGF_NO_FLAGS, FLAGS)

#define STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, GUIO, FLAGS) \
	DETECTION_ENTRY_GUIO(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, GUIO, ADGF_NO_FLAGS, FLAGS)

#define DEMO_ENTRY_PLUGIN(ID, FILENAME, MD5, SIZE, FLAGS) \
	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Demo", ADGF_DEMO, FLAGS)

#define GAME_ENTRY_PLUGIN_STEAM(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Steam", PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_STEAM_EN(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Steam", PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_GOG(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "GOG.com", PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_GOG_EN(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "GOG.com", PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_STEAM_NOLAUNCHLOAD(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_STEAM_EN_NOLAUNCHLOAD(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_GOG_NOLAUNCHLOAD(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::UNK_LANG, "GOG.com", GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), PLUGIN_ARR)

#define GAME_ENTRY_PLUGIN_GOG_EN_NOLAUNCHLOAD(ID, FILENAME, MD5, SIZE, PLUGIN_ARR) \
	STABLE_ENTRY_PLUGIN_GUIO(ID, FILENAME, MD5, SIZE, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), 0)


const AGSGameDescription GAME_DESCRIPTIONS[] = {
	// Pre-2.5 games that aren't supported by the current AGS engine
	PRE_25_DEMO_ENTRY_EN("achristmastale", "tale.exe", "094135f05cf14fc3903e0d3697911a4e", 1484122),
	PRE_25_DEMO_ENTRY_EN("adventuregame", "adventure.exe", "221637e5d62e2ca3cc146846ab0b2e49", 5533207),
	PRE_25_DEMO_ENTRY_EN("anotherrpg", "zombie.exe", "fc5f54dcfc82d3b991f670490a316958", 2697235),
	PRE_25_DEMO_ENTRY_EN("bobsquest2", "bq2.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 2540006),
	PRE_25_DEMO_ENTRY_EN("calsoon2", "Trailer.exe", "97cefa57276834521ee5ef54e33911a5", 2968873),
	PRE_25_DEMO_ENTRY_EN("castlequest", "castle.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 1494036),  // DOS
	PRE_25_DEMO_ENTRY_EN("castlequest", "castle.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 1792104),  // Windows
	PRE_25_DEMO_ENTRY_EN("cosmos", "ac2game.dat", "73e5b8276f3d0a2ab7077c03c24fb16e", 993845),
	PRE_25_DEMO_ENTRY_EN("damnthatspill", "SPILL.exe", "6026f2bae1609882ae6f19f4de293786", 1903003),
	PRE_25_DEMO_ENTRY_EN("deepbright", "tc.exe", "8f80c91d160e333ab7f6be5208ea0533", 3022557),
	PRE_25_DEMO_ENTRY_EN("defsrpgdemo", "GAMEE.exe", "fc5f54dcfc82d3b991f670490a316958", 3952616),
	PRE_25_DEMO_ENTRY_EN("dirkchafberg", "ac2game.dat", "be7bec4a433beb5085184b64119351c8", 1906822),
	PRE_25_DEMO_ENTRY_EN("domjohnson", "TIMMY.exe", "89f304332b7bd02ed62f754a3b53f129", 6413032),
	PRE_25_DEMO_ENTRY_EN("edgeofreality", "EDGEOF.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 7428333),
	PRE_25_DEMO_ENTRY_EN("elburro", "burro.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 1858196),
	PRE_25_DEMO_ENTRY_EN("elburro", "burro.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 2209309),
	PRE_25_DEMO_ENTRY_EN("enchantedlake", "The Enchanted Lake.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 1101257),
	PRE_25_DEMO_ENTRY_EN("equallyinsane", "EIDemo.exe", "deff766b7e64dd84edf20666a278fa1d", 1641841),
	PRE_25_DEMO_ENTRY_EN("exodus", "EXODUS.exe", "8e3cad1439a1d3336e721b1918a20704", 5107192),
	PRE_25_DEMO_ENTRY_EN("eyesofthejadesphinx", "jade.exe", "f2fe94ab604612e4595f3c79b0245529", 10763660),
	PRE_25_DEMO_ENTRY_EN("fatman", "FATMAN1.exe", "07f9921784124d9e09f39bb831e06131", 800643),
	PRE_25_DEMO_ENTRY_EN("firewall", "firewall demo.exe", "2fd8ca69f236ae1ad46edab4ba26a33d", 2839412),
	DETECTION_ENTRY("fountainofyouth", "FOY.exe", "04eedea9846d380d6d9a120f657daa43", 1206580, Common::EN_ANY, "Pre 2.5 Preview", ADGF_UNSTABLE | ADGF_DEMO, 0),
	DETECTION_ENTRY("fountainofyouth", "rollinfoy.exe", "19f4045333d9c823a5439d0447d55985", 3454107, Common::EN_ANY, "Pre 2.5 Rolling Demo", ADGF_UNSTABLE | ADGF_DEMO, 0),
	PRE_25_DEMO_ENTRY_EN("gorthor", "fart.exe", "fc5f54dcfc82d3b991f670490a316958", 1138006),
	PRE_25_DEMO_ENTRY_EN("grandfathertreasure", "GRANDPA.EXE", "9ef5bffe7b85377751c25af806345794", 2295917),
	PRE_25_DEMO_ENTRY_EN("gravediggerjoe", "dig1.exe", "426e34f40e0dc5285af3fb5fc32a220e", 8060324),
	PRE_25_DEMO_ENTRY_EN("helmsuntitled", "EXAMPLE.EXE", "9ef5bffe7b85377751c25af806345794", 1920895),
	PRE_25_DEMO_ENTRY_EN("huckleburyhippie", "HippieDemo.exe", "07f9921784124d9e09f39bb831e06131", 823186),
	PRE_25_DEMO_ENTRY_EN("ijuntitled", "PI.exe", "80bdce9a1052e896c7cba6a4334cecce", 3562650),
	PRE_25_DEMO_ENTRY_EN("inadream", "INADREAM.exe", "1bccd2edef19abc99e9683519d80c0e0", 2257400),
	PRE_25_DEMO_ENTRY_EN("indygoldmedallion", "INDIANA.exe", "d2422dcc22c969efd07b6c60eccadf54", 1038563),
	PRE_25_DEMO_ENTRY_LANG("indyguybrush", "IndyDemo.exe", "04eedea9846d380d6d9a120f657daa43", 2535029, Common::IT_ITA),
	PRE_25_DEMO_ENTRY_EN("invisiblemachine", "IMDEM.EXE", "07f9921784124d9e09f39bb831e06131", 3628276),
	PRE_25_DEMO_ENTRY_EN("islandquest", "IslandQ.exe", "fc5f54dcfc82d3b991f670490a316958", 2089405),
	DETECTION_ENTRY("kinkyisland", "kinky.exe", "04eedea9846d380d6d9a120f657daa43", 2281415, Common::EN_ANY, "Pre 2.5 Rolling Demo", ADGF_UNSTABLE | ADGF_DEMO, 0),  //Win
	DETECTION_ENTRY("kinkyisland", "kinky.exe", "19f4045333d9c823a5439d0447d55985", 5437534, Common::EN_ANY, "Pre 2.5 Rolling Demo", ADGF_UNSTABLE | ADGF_DEMO, 0),  //DOS
	PRE_25_DEMO_ENTRY_EN("larryvales3", "burns.exe", "094135f05cf14fc3903e0d3697911a4e", 2120885),
	PRE_25_DEMO_ENTRY_EN("lassiquest1remake", "LQ1R.exe", "fc5f54dcfc82d3b991f670490a316958", 1081373),
	PRE_25_DEMO_ENTRY_EN("lassiquest2", "las2demo.exe", "8e3cad1439a1d3336e721b1918a20704", 743443),
	PRE_25_DEMO_ENTRY_EN("leisuresuitlarry4", "Larry.exe", "be48a6b0b81a71d34a41930623c065f1", 4462886),  // DOS
	PRE_25_DEMO_ENTRY_EN("leisuresuitlarry4", "lsl4.exe", "80bdce9a1052e896c7cba6a4334cecce", 2062057),  // Windows
	PRE_25_DEMO_ENTRY_EN("limpyghost", "CLAYBORN.exe", "e016cb68731d0e38fe97905dbf0d5b36", 656144),
	PRE_25_DEMO_ENTRY_EN("loop", "LOOP.exe", "e016cb68731d0e38fe97905dbf0d5b36", 991339),
	PRE_25_DEMO_ENTRY_EN("loumarlou", "LOUMARLOU.exe", "c692e831b687b4822787ce9628b8de4a", 40216261),
	PRE_25_DEMO_ENTRY_EN("lukelandrunner", "LUKE.exe", "3e5a8cdd3d286ac92a79cc216e744723", 1757651),
	PRE_25_DEMO_ENTRY_LANG("lupoinutile", "lupo.exe", "19f4045333d9c823a5439d0447d55985", 1856459, Common::IT_ITA),
	PRE_25_DEMO_ENTRY_EN("m0ds_eve", "EVE.exe", "094135f05cf14fc3903e0d3697911a4e", 10788236),
	PRE_25_DEMO_ENTRY_EN("m0ds_lassi", "LASSI.exe", "094135f05cf14fc3903e0d3697911a4e", 622049),
	PRE_25_DEMO_ENTRY_EN("m0ds_modplay", "MODPLAY.exe", "7f5c957798b7ae603f3f10f54b31bc7c", 743262),
	PRE_25_DEMO_ENTRY_EN("m0ds_runner", "RUNNER.exe", "19f4045333d9c823a5439d0447d55985", 932032),
	PRE_25_DEMO_ENTRY_EN("m0ds_test", "TEST.exe", "094135f05cf14fc3903e0d3697911a4e", 1388516),
	PRE_25_DEMO_ENTRY_EN("m0ds_thedome", "THEDOME.exe", "7f5c957798b7ae603f3f10f54b31bc7c", 2152875),
	PRE_25_DEMO_ENTRY_EN("m0ds_thunder", "THUNDER.exe", "094135f05cf14fc3903e0d3697911a4e", 1678834),
	PRE_25_DEMO_ENTRY_EN("maxdark", "MAX.exe", "89f304332b7bd02ed62f754a3b53f129", 907125),
	PRE_25_DEMO_ENTRY_EN("miagsremake", "MIDEMO.exe", "ed778afb4f46c3f2a70d330532a83a2f", 1078492),
	PRE_25_DEMO_ENTRY_LANG("migaiden", "MIGAIDEN.exe", "8e3cad1439a1d3336e721b1918a20704", 969470, Common::DE_DEU),
	PRE_25_DEMO_ENTRY_EN("mojoisland", "Mojo Island.exe", "ed778afb4f46c3f2a70d330532a83a2f", 770123),
	PRE_25_DEMO_ENTRY_EN("nicholaswolfe1", "nw1demo.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 1539596),
	PRE_25_DEMO_ENTRY_EN("pantaloon", "PANTALOON.exe", "07f9921784124d9e09f39bb831e06131", 931079),
	PRE_25_DEMO_ENTRY_EN("paranormality", "ac2game.dat", "e44c0a1c2d9e99d8eed3828a9e4469ba", 515098),
	PRE_25_DEMO_ENTRY_EN("pd", "pd.exe", "ecd0793124fbc9b89c6d11162e3b5851", 3853394),  //v1.02
	PRE_25_DEMO_ENTRY_EN("pd", "pd.exe", "2fd8ca69f236ae1ad46edab4ba26a33d", 3822708),  //v1.01
	PRE_25_DEMO_ENTRY_EN("perilsofpoom", "Poom.exe", "04eedea9846d380d6d9a120f657daa43", 2056276),
	PRE_25_DEMO_ENTRY_EN("pezzobertrum", "PBdemo.exe", "9027912819f3a319ed9de0fd855310c3", 5014645),
	PRE_25_DEMO_ENTRY_EN("piratefry2", "FRY2DEMO.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 3549487),  // DOS
	PRE_25_DEMO_ENTRY_EN("piratefry2", "FRY2DEMO.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 3847555),  // Windows
	PRE_25_DEMO_ENTRY_EN("pointblank", "future.exe", "385a593828b1edb194e984ce55cda65e", 1379714),
	PRE_25_DEMO_ENTRY_EN("preachersjump", "CHURCH.exe", "385a593828b1edb194e984ce55cda65e", 1052695),
	PRE_25_DEMO_ENTRY_EN("projectevilspy", "Project Evilspy.exe", "0e4b0d6c14177a94218619b8c8e60a15", 1820214),
	PRE_25_DEMO_ENTRY_EN("ripp", "ripp.exe", "426e34f40e0dc5285af3fb5fc32a220e", 12252886),
	PRE_25_DEMO_ENTRY_EN("ronbeachparty", "BEACH.exe", "04eedea9846d380d6d9a120f657daa43", 3623974),
	PRE_25_DEMO_ENTRY_EN("ronxmas2002", "XMAS.exe", "9027912819f3a319ed9de0fd855310c3", 4056155),
	PRE_25_DEMO_ENTRY_EN("searchforsanity", "sfs.exe", "00dce98d598e9b7ad3c6674bfd4880d9", 8685593),  // DOS
	PRE_25_DEMO_ENTRY_EN("searchforsanity", "sfs.exe", "426e34f40e0dc5285af3fb5fc32a220e", 8914338),  // Win (AGS 2.4)
	PRE_25_DEMO_ENTRY_EN("southpark", "SP.exe", "b3821d77bd15dc0e986d90ba5c4a3cbd", 1163688),
	PRE_25_DEMO_ENTRY_EN("teamwork", "teamwork.exe", "538274077115c6d8b4a0927dd3cceeac", 1096149),
	PRE_25_DEMO_ENTRY_EN("testicle", "TESTICLE.exe", "094135f05cf14fc3903e0d3697911a4e", 948186),
	PRE_25_DEMO_ENTRY_EN("theinexperiencedassassin", "assassin.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 2659741),
	PRE_25_DEMO_ENTRY_EN("theuncertaintymachine", "DemoMac.exe", "426e34f40e0dc5285af3fb5fc32a220e", 6790193),
	PRE_25_DEMO_ENTRY_EN("thevestibule", "README.exe", "4a6d908fb154757893f059eade3a559c", 1540766),
	PRE_25_DEMO_ENTRY_EN("thevestibule", "README.exe", "07f9921784124d9e09f39bb831e06131", 1541355),
	PRE_25_DEMO_ENTRY_EN("tomeoflegend", "tomeoflegend.exe", "5e6be55318d92af3887be72dadd7af37", 5134790),
	PRE_25_DEMO_ENTRY_EN("tommato", "tom mato's grand wing-ding.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 2735158),
	PRE_25_DEMO_ENTRY_EN("ultimerr2", "ULT2.exe", "385a593828b1edb194e984ce55cda65e", 714551),
	PRE_25_DEMO_ENTRY_EN("waitkey", "ac2game.dat", "8ddf3744922101e33305dfcd06e3b682", 445197),
	PRE_25_DEMO_ENTRY_EN("wambus", "DOOR.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 1257218),
	PRE_25_DEMO_ENTRY_EN("whattimeisit", "WTII.exe", "8b21668ca462b0b6b35df43c5902b074", 1826322),  // 2001-08-01
	PRE_25_DEMO_ENTRY_EN("whattimeisit", "ac2game.dat", "e016cb68731d0e38fe97905dbf0d5b36", 4451529),  // 2002-02-03
	PRE_25_DEMO_ENTRY_LANG("zakagsremake", "zak.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 2342816, Common::DE_DEU),
	PRE_25_DEMO_ENTRY_EN("zakagsremake", "zak.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 2337860),
	PRE_25_ENTRY_EN("6da", "6da.exe", "e016cb68731d0e38fe97905dbf0d5b36", 1422049),  // DOS
	PRE_25_ENTRY_EN("6da", "6da.exe", "9027912819f3a319ed9de0fd855310c3", 1608073),  // Win
	PRE_25_ENTRY_EN("aaronsepicjourney", "aaron.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 1788404),
	PRE_25_ENTRY_EN("abstract", "ABSTRACT.exe", "8f80c91d160e333ab7f6be5208ea0533", 985390),
	PRE_25_ENTRY_EN("aceduswell", "ace.exe", "be48a6b0b81a71d34a41930623c065f1", 3103822),
	PRE_25_ENTRY_EN("adayinthefuture", "space.exe", "ecd0793124fbc9b89c6d11162e3b5851", 4120328),
	PRE_25_ENTRY_EN("adventurenoir", "NOIR.exe", "094135f05cf14fc3903e0d3697911a4e", 815434),
	PRE_25_ENTRY_EN("agsbgeditor", "AGSBGEDI.exe", "a9fbf05df479ca7e7dbd67d9bbd149d9", 1322728),
	PRE_25_ENTRY_EN("agsdraw", "AGSdraw1.0.exe", "9027912819f3a319ed9de0fd855310c3", 791944),
	PRE_25_ENTRY_EN("aliengame", "platform.exe", "f4585823c1b4ce97d78c8acb433bec52", 2680383),
	PRE_25_ENTRY_EN("andybigadv", "ANDY.exe", "094135f05cf14fc3903e0d3697911a4e", 629445),
	PRE_25_ENTRY_EN("asapadventure", "asap.exe", "8f80c91d160e333ab7f6be5208ea0533", 1405072),
	PRE_25_ENTRY_EN("barnrunner1p1", "eclair 1.exe", "9fa0358760f1f1bffddd080532f586f0", 1864283),
	PRE_25_ENTRY_EN("barnrunner1p2", "eclair 2.exe", "9fa0358760f1f1bffddd080532f586f0", 5644093),
	PRE_25_ENTRY_EN("bertthenewsreader", "bert.exe", "80bdce9a1052e896c7cba6a4334cecce", 2814934),
	PRE_25_ENTRY_EN("billybobwildnight", "TheWildNight.exe", "426e34f40e0dc5285af3fb5fc32a220e", 1925403),
	PRE_25_ENTRY_EN("blackjack", "blackjack.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 855024),
	PRE_25_ENTRY_EN("blackjack", "blackjack.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 1153092),
	PRE_25_ENTRY_EN("blastoff", "BlastOff.exe", "8f80c91d160e333ab7f6be5208ea0533", 2043203),
	PRE_25_ENTRY_EN("blastoff", "BlastOff-Win.exe", "727a30f9244441ad57a76086f4faa779", 2409275),
	PRE_25_ENTRY_EN("bluecupontherun", "ac2game.dat", "c290455f00f630c8a52c7eceb7c663eb", 1995708),
	PRE_25_ENTRY_EN("bobsquest1", "BQ1 2.1.exe", "80bdce9a1052e896c7cba6a4334cecce", 3096485),  // TRAC #14244
	PRE_25_ENTRY_EN("bookofspells1", "ac2game.dat", "fe66cb08bcffd094c159cd4ee72bacd3", 3442073),
	PRE_25_ENTRY_EN("bookofspells2", "ac2game.dat", "9df87a8e5bbcc7206b001c0b8316b7f9", 3263169),
	PRE_25_ENTRY_EN("bookofspells3", "ac2game.dat", "9f0181393bdceb2c0bbdb06634714023", 4806518),
	PRE_25_ENTRY_EN("cabbagesandkings", "FAL.exe", "9027912819f3a319ed9de0fd855310c3", 2836668),  // v1.0
	PRE_25_ENTRY_EN("calsoon", "calsoon.exe", "016de00bd796a8f4af6217eab480a5e5", 2565040),
	PRE_25_ENTRY_EN("calsoon", "calsoon.exe", "5477f4ed8f860427d1492548b677073c", 2865508),
	PRE_25_ENTRY_EN("calsoon2", "looncalsoon.exe", "fc5f54dcfc82d3b991f670490a316958", 16015278),
	PRE_25_ENTRY_EN("candy", "CANDY.exe", "094135f05cf14fc3903e0d3697911a4e", 857769),
	PRE_25_ENTRY_EN("candy", "CANDY.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 1155013),
	PRE_25_ENTRY_EN("captainmuchly", "bleach.exe", "e016cb68731d0e38fe97905dbf0d5b36", 988805),  // DOS
	PRE_25_ENTRY_EN("captainmuchly", "bleach.exe", "9027912819f3a319ed9de0fd855310c3", 1174829),  // Win
	PRE_25_ENTRY_EN("carverisland1", "secret.exe", "8f80c91d160e333ab7f6be5208ea0533", 3116071),
	PRE_25_ENTRY_EN("carverisland2", "carver2.exe", "ed778afb4f46c3f2a70d330532a83a2f", 6265594),
	PRE_25_ENTRY_EN("cda", "cda.exe", "094135f05cf14fc3903e0d3697911a4e", 637675),
	PRE_25_ENTRY_EN("chef", "CHEF.exe", "04eedea9846d380d6d9a120f657daa43", 11556768),
	PRE_25_ENTRY_EN("coffeebreak", "COFFEEBREAK.exe", "f2fe94ab604612e4595f3c79b0245529", 1061375),
	PRE_25_ENTRY_EN("commanderkeenron", "KEENRON.exe", "9027912819f3a319ed9de0fd855310c3", 2061720),
	PRE_25_ENTRY_EN("compensation", "comp.exe", "094135f05cf14fc3903e0d3697911a4e", 2054366),
	PRE_25_ENTRY_EN("crimetime", "crimetim.exe", "be48a6b0b81a71d34a41930623c065f1", 1303660),
	PRE_25_ENTRY_EN("cutlass", "ac2game.dat", "ba6eb93c31cd78a18448ddea70836c5e", 985175),
	PRE_25_ENTRY_EN("darksects", "ac2game.dat", "b2923f9cdad3b11898a6b5c5b014c5db", 9886156),
	PRE_25_ENTRY_EN("davyjonescestmort", "RONDDJ.exe", "c90241861523a2ed0f2f6e68fb0f0661", 2456690),
	PRE_25_ENTRY_EN("davyjonesisback", "RONDJR.exe", "9027912819f3a319ed9de0fd855310c3", 2817715),
	PRE_25_ENTRY_EN("deadquest", "DEADQUEST.exe", "dd69243e3cc9e955215e0d556301b58e", 1391000),
	PRE_25_ENTRY_EN("defenderofron", "SUPRPHIL.EXE", "9027912819f3a319ed9de0fd855310c3", 4387452),
	PRE_25_ENTRY_EN("demonslayer1", "mags.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 3820347),
	PRE_25_ENTRY_EN("demonslayer2", "bert.exe", "0c057c5e6df6f45772286986ab7b7a5b", 1726766),
	PRE_25_ENTRY_EN("demonslayer3", "tiler.exe", "426e34f40e0dc5285af3fb5fc32a220e", 2586532),
	PRE_25_ENTRY_EN("demonslayer4", "dem four.exe", "426e34f40e0dc5285af3fb5fc32a220e", 5110674),
	PRE_25_ENTRY_EN("dogkennel", "KENNEL.EXE", "094135f05cf14fc3903e0d3697911a4e", 1204666),
	PRE_25_ENTRY_EN("earwigisangry", "earwig.exe", "04eedea9846d380d6d9a120f657daa43", 2371856),
	PRE_25_ENTRY_EN("easterbunny", "EASTER.exe", "04eedea9846d380d6d9a120f657daa43", 967448),
	PRE_25_ENTRY_EN("edgeofreality", "edgeof.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 46706875),
	PRE_25_ENTRY_EN("erniesbigadventure1", "magsjune.exe", "fc5f54dcfc82d3b991f670490a316958", 8814849),
	PRE_25_ENTRY_EN("erniesbigadventure2", "magsjuly.exe", "2fd8ca69f236ae1ad46edab4ba26a33d", 6647578),
	PRE_25_ENTRY_LANG("everythingeuro", "EURO.EXE", "8e3cad1439a1d3336e721b1918a20704", 1923920, Common::DE_DEU),  // DOS Deu
	PRE_25_ENTRY_LANG("everythingeuro", "EURO.EXE", "8e3cad1439a1d3336e721b1918a20704", 1918371, Common::EN_ANY),  // DOS Eng
	PRE_25_ENTRY_EN("exile", "exile.exe", "aad0a09714fab4de51e5488da48fd5d4", 13421451),
	PRE_25_ENTRY_EN("existance", "ac2game.dat", "21dbb6216639ad5817de893385c2e5b0", 420912),
	PRE_25_ENTRY_EN("faddevil", "FAD.EXE", "094135f05cf14fc3903e0d3697911a4e", 1730090),
	PRE_25_ENTRY_EN("flies", "fly.exe", "f4585823c1b4ce97d78c8acb433bec52", 1125570),
	PRE_25_ENTRY_EN("floyd", "floyd.exe", "9ef5bffe7b85377751c25af806345794", 5477588),
	PRE_25_ENTRY_EN("fowlplay", "RON2HALF.EXE", "727a30f9244441ad57a76086f4faa779", 2416233),
	PRE_25_ENTRY_EN("gaeafallen", "gaea_fallen.exe", "80bdce9a1052e896c7cba6a4334cecce", 11273205),
	PRE_25_ENTRY_EN("grannyzombiekiller", "mags.exe", "0c057c5e6df6f45772286986ab7b7a5b", 12855495),
	PRE_25_ENTRY_EN("gregsmountainousadventure", "mags.exe", "80a17966fc547849d43646acf89de296", 2112993),
	PRE_25_ENTRY_EN("greysgreytadv", "greywin.exe", "80bdce9a1052e896c7cba6a4334cecce", 1283447),  // Windows
	PRE_25_ENTRY_EN("greysgreytadv", "grey.exe", "d2c2a45bb459890c0be598155348a4a5", 1069667),  // DOS
	PRE_25_ENTRY_EN("henkkaquest", "Henkka.exe", "04eedea9846d380d6d9a120f657daa43", 1843411),
	PRE_25_ENTRY_EN("hermit", "hermit.exe", "4689069dd6b241e38311d0586e610a8d", 13995403),
	PRE_25_ENTRY_EN("hermit", "ac2game.dat", "4689069dd6b241e38311d0586e610a8d", 13995403),
	PRE_25_ENTRY_EN("hiphendrix", "hendrix.exe", "094135f05cf14fc3903e0d3697911a4e", 711944),
	PRE_25_ENTRY_EN("hookymcp", "HMPP.exe", "e59a532ee3d6686fdcabf498e180b635", 3812053),
	PRE_25_ENTRY_EN("housequest", "HouseQuest.exe", "6988ee3c2dd1cda50d49a12317801c08", 2897790),
	PRE_25_ENTRY_EN("hugoags", "HUGO.EXE", "e59a532ee3d6686fdcabf498e180b635", 3785097),
	PRE_25_ENTRY_EN("indysecretchamber", "indy.exe", "094135f05cf14fc3903e0d3697911a4e", 1545150),
	PRE_25_ENTRY_EN("interactivefiction", "IF.exe", "0e4b0d6c14177a94218619b8c8e60a15", 1326886),
	PRE_25_ENTRY_EN("intergalacticlife", "INTERLIFE.exe", "9027912819f3a319ed9de0fd855310c3", 2226509),
	PRE_25_ENTRY_EN("invasionspacealiens", "reality.exe", "426e34f40e0dc5285af3fb5fc32a220e", 2001118),
	PRE_25_ENTRY_EN("islandquest", "IslandQ.exe", "80bdce9a1052e896c7cba6a4334cecce", 5220070),
	PRE_25_ENTRY_EN("ispy", "SPY.exe", "8f80c91d160e333ab7f6be5208ea0533", 2864744), //v2
	PRE_25_ENTRY_EN("ispy", "ac2game.dat", "f93fcb017856b28d8ee509e58d91a838", 2509569), //v3
	PRE_25_ENTRY_EN("ispy2", "ISPY2.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 3028482), //v1
	PRE_25_ENTRY_EN("jamesbondage", "jbdos.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 2056737),
	PRE_25_ENTRY_EN("javajo", "Java Jo's Koffee Stand!.exe", "0e4b0d6c14177a94218619b8c8e60a15", 1623899),
	PRE_25_ENTRY_EN("jinglebells", "jinglebells.exe", "385a593828b1edb194e984ce55cda65e", 1620588),
	PRE_25_ENTRY_EN("kidnapped", "ac2game.dat", "27daca01ccbbbaf02bf6b4b85d5990b4", 1205017),
	PRE_25_ENTRY_EN("kittensandcacti", "KAC.exe", "9027912819f3a319ed9de0fd855310c3", 5739433),
	PRE_25_ENTRY_EN("larryvales1", "larryvtd.exe", "610b7a3d1fd90f24d2218aa26b29d8ca", 3129645),
	PRE_25_ENTRY_EN("larryvales2", "dead.exe", "be48a6b0b81a71d34a41930623c065f1", 3946993),
	PRE_25_ENTRY_EN("lassiandrogerventure", "lassi.exe", "ed778afb4f46c3f2a70d330532a83a2f", 3681914),
	PRE_25_ENTRY_EN("lassiandrogermeetgod", "mtg.exe", "ed778afb4f46c3f2a70d330532a83a2f", 1618544),
	PRE_25_ENTRY_LANG("lassiquest1", "lassi.exe", "c391c6676099032440b206189babe76e", 1700368, Common::FI_FIN),  // original
	PRE_25_ENTRY_EN("lassiquest1", "lassi.exe", "c391c6676099032440b206189babe76e", 1765672),  // updated
	PRE_25_ENTRY_EN("littlejonnyevil", "lje.exe", "e93037e8efc7abc19b8978903ef5b409", 2133182),
	PRE_25_ENTRY_EN("littlewillydarts", "dart.exe", "ed778afb4f46c3f2a70d330532a83a2f", 736848),
	PRE_25_ENTRY_EN("littlewillyshotgun", "ac2game.dat", "239b11ab644222c67d981494766a3c25", 254128),
	PRE_25_ENTRY_EN("lorryloader", "ac2game.ags", "ff62dfe53850eda9fb66ab01a3a51667", 810764),
	PRE_25_ENTRY_EN("losttape", "town.exe", "0e4b0d6c14177a94218619b8c8e60a15", 1492822),
	PRE_25_ENTRY_EN("losttreasureron", "LTRON.exe", "e59a532ee3d6686fdcabf498e180b635", 2274645),
	PRE_25_ENTRY_EN("ludwig", "ludwig.exe", "727a30f9244441ad57a76086f4faa779", 3658869),
	PRE_25_ENTRY_EN("lunchtimeofthedamned", "reality.exe", "dd69243e3cc9e955215e0d556301b58e", 1357955),
	PRE_25_ENTRY_EN("magsceremony2001jul", "MAGS002.exe", "094135f05cf14fc3903e0d3697911a4e", 1821625),
	PRE_25_ENTRY_EN("magsceremony2001jun", "MAGS001.exe", "094135f05cf14fc3903e0d3697911a4e", 1698160),
	PRE_25_ENTRY_EN("magsceremony2002jul", "July.exe", "80a17966fc547849d43646acf89de296", 1621234),
	PRE_25_ENTRY_EN("martychonks", "marty.exe", "88cf59aad15ca331ab0f854e16c84df3", 3107976),
	PRE_25_ENTRY_EN("meninbrown", "0112.men in brown.exe", "426e34f40e0dc5285af3fb5fc32a220e", 2072392),
	PRE_25_ENTRY_EN("meninbrown", "MIB.exe", "426e34f40e0dc5285af3fb5fc32a220e", 2072392),
	PRE_25_ENTRY_EN("midtownshootout", "mtsowin.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 1076035),  // Windows
	PRE_25_ENTRY_EN("midtownshootout", "MTSODOS.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 777967),  // DOS
	PRE_25_ENTRY_EN("mikasdream", "MIKASDREAM.exe", "9027912819f3a319ed9de0fd855310c3", 5858215),
	PRE_25_ENTRY_EN("momsquest", "mom's quest.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 3173896),
	PRE_25_ENTRY_EN("monkeypeninsula", "PENINSULA.exe", "f4585823c1b4ce97d78c8acb433bec52", 2068383),
	PRE_25_ENTRY_EN("monkeyplank", "plank.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 815948),
	PRE_25_ENTRY_EN("montyonthenorm", "Monty.exe", "89f304332b7bd02ed62f754a3b53f129", 1663257),  // original DOS version
	PRE_25_ENTRY_EN("moosewars", "moose.exe", "f4585823c1b4ce97d78c8acb433bec52", 1448684),
	PRE_25_ENTRY_EN("mousetcd", "ac2game.dat", "f339dc194b241a59d046f4ab80ba09bf", 456911),
	PRE_25_ENTRY_EN("murder", "murder.exe", "221637e5d62e2ca3cc146846ab0b2e49", 935799),
	PRE_25_ENTRY_EN("murderfishhotel", "CONTEST.exe", "6026f2bae1609882ae6f19f4de293786", 1198112),
	PRE_25_ENTRY_EN("nightoftheplumber", "night of the plumber.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 1703896),
	PRE_25_ENTRY_EN("nightwatchron", "NIGHTWATCH.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 2616586),  // Beta2
	PRE_25_ENTRY_EN("nightwatchron", "NIGHTWAT.EXE", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 2616625),  // Beta2A
	PRE_25_ENTRY_EN("nihilism", "RONcm.exe", "c90241861523a2ed0f2f6e68fb0f0661", 1853822),
	PRE_25_ENTRY_EN("ninjaquest", "ac2game.dat", "6fc054fd5d69975566a1aba1985b3972", 769820),
	PRE_25_ENTRY_EN("nokq", "ac2game.dat", "02bc49b8b55459af996c9dd62088931d", 743091),
	PRE_25_ENTRY_EN("nomedon", "NOMEDON.exe", "094135f05cf14fc3903e0d3697911a4e", 4408173),  //v1.2
	PRE_25_ENTRY_EN("notanothersq", "NASQ.exe", "04eedea9846d380d6d9a120f657daa43", 1268632),
	PRE_25_ENTRY_EN("novomestro", "novo.exe", "07f9921784124d9e09f39bb831e06131", 1122507),
	PRE_25_ENTRY_EN("odysseus", "odysseus.exe", "426e34f40e0dc5285af3fb5fc32a220e", 2799113), // Windows
	PRE_25_ENTRY_EN("odysseus", "odysseus.exe", "00dce98d598e9b7ad3c6674bfd4880d9", 2570368), // DOS
	PRE_25_ENTRY_EN("oldparkquest", "OLDPARK.exe", "385a593828b1edb194e984ce55cda65e", 621159),
	PRE_25_ENTRY_EN("orbblanc", "ORB.exe", "7f5c957798b7ae603f3f10f54b31bc7c", 756888),
	PRE_25_ENTRY_EN("paradiselost", "larywilc.exe", "426e34f40e0dc5285af3fb5fc32a220e", 8982962),  //v2.01
	PRE_25_ENTRY_EN("paranormalinvestigation", "RONPI.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 2994116),  // original DOS version
	PRE_25_ENTRY_EN("permanentdaylight", "daylight.exe", "07f9921784124d9e09f39bb831e06131", 1799958),
	PRE_25_ENTRY_EN("perpetrator", "Perpetrator.exe", "56ef979be112e122e24d0cc8caea4ea4", 960677),  // v1.0
	PRE_25_ENTRY_EN("perpetrator", "ac2game.dat", "56ef979be112e122e24d0cc8caea4ea4", 994303),  // v1.1
	PRE_25_ENTRY_EN("piratefry1", "fry.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 4164041),  // Windows
	PRE_25_ENTRY_EN("piratefry1", "fry.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 3865973),  // DOS
	PRE_25_ENTRY_EN("pixelypete", "ac2game.dat", "10ed446ceafda3607301507952fdd749", 502917),
	PRE_25_ENTRY_EN("pizzaquest", "PIZZAQ.EXE", "27bc15f136b0ac5ad4bfa466251dff7a", 2246198),  // DOS
	PRE_25_ENTRY_EN("pizzaquest", "pqwin.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 2500089),  // Windows
	PRE_25_ENTRY_EN("pornquest", "porn.exe", "04eedea9846d380d6d9a120f657daa43", 1012323),
	PRE_25_ENTRY_EN("postmanonlydiesonce", "Postman.exe", "fc5f54dcfc82d3b991f670490a316958", 7187159),  // Windows
	PRE_25_ENTRY_EN("postmanonlydiesonce", "Post.exe", "f2fe94ab604612e4595f3c79b0245529", 6726601),  // DOS
	PRE_25_ENTRY_EN("projectevilspy", "evilspy.exe", "97d700529f5cc826f230c27acf81adfd", 1240074),
	PRE_25_ENTRY_EN("pyramid", "PYRAMID.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 970566),
	PRE_25_ENTRY_EN("qfc", "qfc.exe", "04eedea9846d380d6d9a120f657daa43", 2038696),
	PRE_25_ENTRY_EN("qfg412", "qfg.exe", "8b21668ca462b0b6b35df43c5902b074", 26674790),
	PRE_25_ENTRY_EN("qfg412", "qfg.exe", "e016cb68731d0e38fe97905dbf0d5b36", 26768799),  // DOS
	PRE_25_ENTRY_EN("qfg412", "qfg.exe", "9027912819f3a319ed9de0fd855310c3", 26954823),  // Win
	PRE_25_ENTRY_EN("raymondskeys", "keys.exe", "e016cb68731d0e38fe97905dbf0d5b36", 1032178),
	PRE_25_ENTRY_EN("redflagg", "red.exe", "be48a6b0b81a71d34a41930623c065f1", 1101194),
	PRE_25_ENTRY_EN("repossessor", "repossessor.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 2906801),  // original DOS version
	PRE_25_ENTRY_EN("retardsgame", "RETARDSGAME.exe", "094135f05cf14fc3903e0d3697911a4e", 1323585),
	PRE_25_ENTRY_EN("returnofdvs", "reality.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 2703414),
	PRE_25_ENTRY_EN("richardlonghurst", "THEBOX.exe", "9ef5bffe7b85377751c25af806345794", 7636145),  // DOS
	PRE_25_ENTRY_EN("richardlonghurst", "rlbat-win.exe", "04eedea9846d380d6d9a120f657daa43", 7935723),  // Win
	PRE_25_ENTRY_EN("ripp", "ripp.exe", "426e34f40e0dc5285af3fb5fc32a220e", 10489586),
	PRE_25_ENTRY_EN("robblanc1", "ac2game.dat", "29c2ced2f2e6ad764e4249b4e4c45bba", 920415),
	PRE_25_ENTRY_EN("robblanc1", "rb.exe", "426e34f40e0dc5285af3fb5fc32a220e", 1804361), // v1.2
	PRE_25_ENTRY_EN("robblanc2", "ac2game.dat", "dd6c52e5a6e9b70efef4654769f11c69", 2056386),
	PRE_25_ENTRY_EN("robblanc2", "rb2.exe", "426e34f40e0dc5285af3fb5fc32a220e", 2973290), // v1.2
	PRE_25_ENTRY_EN("robblanc3", "ac2game.dat", "8f8264de3c1bd91e26b84fe37fb5e53e", 2828959),
	PRE_25_ENTRY_EN("robblanc3", "rb3.exe", "426e34f40e0dc5285af3fb5fc32a220e", 3783355), // v1.2
	PRE_25_ENTRY_EN("robertredford1", "GAME.exe", "8f80c91d160e333ab7f6be5208ea0533", 6329845), // DOS
	PRE_25_ENTRY_EN("robertredford2", "ROBERT2.exe", "8f80c91d160e333ab7f6be5208ea0533", 12771788), // DOS
	PRE_25_ENTRY_EN("rodekill", "rodekill.exe", "07f9921784124d9e09f39bb831e06131", 14336787),  // DOS
	PRE_25_ENTRY_EN("rodekill", "rodekill.exe", "511cde5d104f09bea0b0288fd523dd8a", 14703194),  // Windows v1.0
	PRE_25_ENTRY_EN("rodekill", "rodekill.exe", "72f3c950b4d9d14580a11db885a63310", 11995954),  // Windows v1.7
	PRE_25_ENTRY_EN("rodequest", "rodequest1.exe", "72f3c950b4d9d14580a11db885a63310", 1196458),
	PRE_25_ENTRY_EN("rodequest", "rodequest1.exe", "72f3c950b4d9d14580a11db885a63310", 1196250),  //v1.0
	PRE_25_ENTRY_EN("rodequest", "rodequest1.exe", "72f3c950b4d9d14580a11db885a63310", 1196321),  //v1.01
	PRE_25_ENTRY_EN("samthepiratemonkey", "monkey.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 1401414),
	PRE_25_ENTRY_EN("short", "SHORT.exe", "094135f05cf14fc3903e0d3697911a4e", 536698),
	PRE_25_ENTRY_LANG("sma1", "moonsdt.exe", "9027912819f3a319ed9de0fd855310c3", 1276725, Common::DE_DEU),
	PRE_25_ENTRY_EN("slackerquest", "ac2game.dat", "e0998f2d2e14a55aae2291fdfab1ce7d", 1306492),
	PRE_25_ENTRY_EN("snailquest1", "snailquest.exe", "dd69243e3cc9e955215e0d556301b58e", 1095860),
	PRE_25_ENTRY_EN("snailquest2", "sq2.exe", "1bccd2edef19abc99e9683519d80c0e0", 955614),
	PRE_25_ENTRY_EN("snailquest3", "sq3.exe", "1bccd2edef19abc99e9683519d80c0e0", 1501892),
	PRE_25_ENTRY_EN("sol", "sol.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 4702799),
	PRE_25_ENTRY_EN("sovietunionstrikesback", "ac2game.dat", "565953ecb1a69a31439ff33b00c539bb", 1062670),
	PRE_25_ENTRY_EN("space", "space.exe", "094135f05cf14fc3903e0d3697911a4e", 3790026),
	PRE_25_ENTRY_EN("spacemail", "WinVersion.exe", "9027912819f3a319ed9de0fd855310c3", 1148443),  // Windows
	PRE_25_ENTRY_EN("spacemail", "SPACEMAIL.exe", "e016cb68731d0e38fe97905dbf0d5b36", 839512),  // DOS
	PRE_25_ENTRY_EN("spacemail", "SPACEMAIL.exe", "e016cb68731d0e38fe97905dbf0d5b36", 962419),  // DOS
	PRE_25_ENTRY_EN("startreksnw", "STX.exe", "9ef5bffe7b85377751c25af806345794", 1060462),
	PRE_25_ENTRY_EN("startrektng", "STTNG.exe", "9ef5bffe7b85377751c25af806345794", 3615127),
	PRE_25_ENTRY_EN("stickmen", "stickmen.exe", "094135f05cf14fc3903e0d3697911a4e", 2145142),
	PRE_25_ENTRY_EN("superdisk", "superdisk.exe", "80bdce9a1052e896c7cba6a4334cecce", 1304065),
	PRE_25_ENTRY_EN("taleofroland", "ROLAND.exe", "c1c7f4363f8c19e760293bc045f4e95a", 8820751),
	PRE_25_ENTRY_EN("thecrownofgold", "the crown of gold.exe", "e407143be000e44f113ba5ff1fbd17f9", 1971515),
	PRE_25_ENTRY("theisland", "island.exe", "e93037e8efc7abc19b8978903ef5b409", 1814801),  // Eng-Fin
	PRE_25_ENTRY_EN("thejourneyhomep1", "ac2game.dat", "7296c35543b93890a21482e5d3c3713c", 7538441),
	PRE_25_ENTRY_EN("thepigion", "THEPIGION.exe", "e016cb68731d0e38fe97905dbf0d5b36", 967410),
	PRE_25_ENTRY_EN("thetower", "thetower.exe", "9027912819f3a319ed9de0fd855310c3", 3431385),
	PRE_25_ENTRY_EN("thewarp", "warp.exe", "9027912819f3a319ed9de0fd855310c3", 881957),
	PRE_25_ENTRY_EN("tinygreen", "GREEN.exe", "094135f05cf14fc3903e0d3697911a4e", 1404323),
	PRE_25_ENTRY_EN("tullesworld1", "candale.exe", "1c21bdb52bcafcafe988b30fd6bb4126", 3936603),
	PRE_25_ENTRY_EN("tullesworld3", "ac2game.dat", "b9c2ad76574c08bbcfd56eb1b49d2cd3", 8580708),
	PRE_25_ENTRY_EN("tvquest", "mags.exe", "fc5f54dcfc82d3b991f670490a316958", 1318019),
	PRE_25_ENTRY_EN("ultimerr", "ultimerr.exe", "89f304332b7bd02ed62f754a3b53f129", 1512380),
	PRE_25_ENTRY_EN("underworld", "UNDERWORLD.exe", "2fd8ca69f236ae1ad46edab4ba26a33d", 14661243),
	PRE_25_ENTRY_EN("universalequalizer", "RON - The Universal Equaliser.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 1167105),
	PRE_25_ENTRY_EN("unkhour_anhour", "ANHOUR.exe", "385a593828b1edb194e984ce55cda65e", 585951),
	PRE_25_ENTRY_EN("unkhour_jv", "JV.exe", "1f0edcb7fa3f8e2b1dd60fa6c0bce60f", 585662),
	PRE_25_ENTRY_EN("unkhour_onehour", "ONEHOUR.exe", "b89bb4336e79969e99f9aa39d6184a12", 733939),  // DOS
	PRE_25_ENTRY_EN("unkhour_onehour", "ONEHOUR-win.exe", "d47d977ec267f62491db97fbaac80f44", 925987),  // Windows
	PRE_25_ENTRY_EN("unkhour_shorty", "SHORTY.exe", "e016cb68731d0e38fe97905dbf0d5b36", 658022),
	PRE_25_ENTRY_EN("vengeanceofthechicken", "chicken.exe", "6026f2bae1609882ae6f19f4de293786", 1808436),
	PRE_25_ENTRY_EN("whokilledkennyrogers", "mags.exe", "ed778afb4f46c3f2a70d330532a83a2f", 1240103),
	PRE_25_ENTRY_EN("xenoreturns", "XENOR.EXE", "8f80c91d160e333ab7f6be5208ea0533", 2329463),

	// Post 2.5 games that aren't currently supported
	// uses unsupported agslua plugin
	UNSUPPORTED_GAME_ENTRY("allthewaydown", "deepuns3.exe", "6c0b5468657c22d62201e3769cd8d2b3", 29830067),
	UNSUPPORTED_GAME_ENTRY("barelyfloating", "Barely Floating.exe", "60fbd60394e3616920325eab6eef567c", 1283262682),
	UNSUPPORTED_GAME_ENTRY("salt", "salt.exe", "652b25e0a5017c737382b7ebb9edad4e", 10247572),
	// Commercial game that likely uses custom extensions. Not only does it seem
	// to use AGSController plugin built-in, even when I hooked up AGSController for
	// the stand-alone AGS 3.5 interpreter, it hangs just like in ScummVM
	UNSUPPORTED_GAME_ENTRY("untilihaveyou", "until i have you.exe", "cda1d7e36993dd55ba5513c1c43e5b2b", 1072879555),
	// 2.55 game, but uses a plugin agsflashlight. Also, even with AGS interpreter
	// hacked to specify plugin, errors on loading room107 saying that
	// "room animations are no longer supported"
	UNSUPPORTED_GAME_ENTRY("zak2", "Zak2.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 8686711),
	UNSUPPORTED_GAME_ENTRY("zak2", "clk:fanadv_zak2.exe:Zak2.exe", "A:e88fd6a23a5e498d7b0d50e3bb914085", 8686711),
	UNSUPPORTED_GAME_ENTRY("zak2", "Zak2.exe", "0b7529a76f38283d6e850b8d56526fc1", 9205143),

	// AGS 3.6.1 games
	UNSUPPORTED_GAME_ENTRY("apfelman", "Apfelmaennchen.ags", "fd215dc93055c1123a4cb9cd7cfb3661", 1277361),
	UNSUPPORTED_GAME_ENTRY("breakfastrequiem", "Breakfast Requiem.ags", "31eb9d9dab1a820b110f1a864482a58c", 21634105),  // MAGS
	UNSUPPORTED_GAME_ENTRY("breakfastrequiem", "Breakfast Requiem.ags", "9a3d632075dd0f50b28d3a4e0321999a", 21634549),
	UNSUPPORTED_GAME_ENTRY("brianeggswoods", "Brian Eggs Woods.ags", "8b23975e2fdf0a5f1124230ca3219016", 11428377),
	UNSUPPORTED_GAME_ENTRY("bubbygod", "Bubby's adventure.ags", "4466d8b8cbc0f20904f4d7989c2ed5d7", 69402382),
	UNSUPPORTED_GAME_ENTRY("cadaverheart", "Cadaver_Heart.ags", "291f0d36af8b5ea71370e25775ca4068", 3815846),
	UNSUPPORTED_GAME_ENTRY("draculahousemystery", "Mystery House.ags", "a6638e1e37f404b1abf2fc51c03077c8", 23184317),  // Win
	UNSUPPORTED_GAME_ENTRY("federicoreturns", "Federico Returns.exe", "031470656455b56887aa57a3cff9e016", 35338182),
	UNSUPPORTED_GAME_ENTRY("flowproblem", "Flow Problem.ags", "4e82d48102ea7ca1d72d675db69ddec3", 177517),  // v1.01
	UNSUPPORTED_GAME_ENTRY("hoopoeinferno", "Hoopoe.ags", "5e41c68e68d347f9090da1b1cb57108e", 13502829),
	UNSUPPORTED_GAME_ENTRY("hydrahead", "Hydra.ags", "6283040a165e21b0c2dc4faa020a1861", 185123953),
	UNSUPPORTED_GAME_ENTRY("hydrahead2", "HH2.ags", "540e3621845b004129a6247a8a7e8d68", 405115833),
	UNSUPPORTED_GAME_ENTRY("hydrahead3", "Hydra3.ags", "bdc6fb9b2f42f4f3f552cf2c005e2e33", 387364845),
	UNSUPPORTED_GAME_ENTRY("indip", "Indip.exe", "fa3e393b05ea5d17eabc9c51064097a0", 12611892),
	UNSUPPORTED_GAME_ENTRY("interviewwrong", "Interview_Gone_Wrong.ags", "6d567f9ca477178518194a09c416f520", 57684453),
	UNSUPPORTED_GAME_ENTRY("jaro", "Jaro.ags", "73ea767bec41a9e0bb7dc16ce74b35e4", 20333293),  // Polish
	UNSUPPORTED_GAME_ENTRY("jakoquest", "JakoQuest.exe", "0b66b6f4745d0d997d2d07402d2fa7af", 9243132),
	UNSUPPORTED_GAME_ENTRY("jakoquest", "JakoQuest.ags", "30e193ae11033abafd33825595e449a5", 6103016),
	UNSUPPORTED_GAME_ENTRY("lonelyspaces", "LonelySpaces.ags", "0639cde13b999293a1b90d99e17ca49e", 17823673),  // v1
	UNSUPPORTED_GAME_ENTRY("mathildedream", "MathildeDream.ags", "561152408953c84fb7092f35e52b7b3c", 2605085),
	UNSUPPORTED_DEMO_ENTRY("metrocityresistance", "METRO CITY Resistance.ags", "f10254c9af0948cc26bef04ec0d6eb74", 1455799507),
	UNSUPPORTED_DEMO_ENTRY("metrocityresistance", "METRO CITY Resistance.ags", "b38976aa18024acd3a356102a75217da", 1454430246),
	UNSUPPORTED_DEMO_ENTRY("phantomfellows", "TPF.exe", "ed567fcba6d4f0020464f106a9e3817d", 2843553102),
	UNSUPPORTED_DEMO_ENTRY("phantomfellows", "TPF.exe", "59651b6757f7322d7c3775565d021b3a", 2882837211),
	UNSUPPORTED_DEMO_ENTRY("phantomfellows", "TPF.ags", "91fb3711824240b12ced92e50af85df4", 2840396090),
	UNSUPPORTED_DEMO_ENTRY("phantomfellows", "TPF.ags", "ebe481bbfcb6045be57ed9772cd2faaa", 2879679687),
	UNSUPPORTED_GAME_ENTRY("sarimento", "SarimentoAGS.ags", "cf2e483168c275c25697dc5c97be1df4", 45530253),
	UNSUPPORTED_DEMO_ENTRY("simon1text", "Simon the Sorcerer 1 Demo Text Adventure.ags", "c245ac2645a81e3497e9911d695f5645", 2054771),
	UNSUPPORTED_GAME_ENTRY("spacemisadventures", "Space Misadventures DX.exe", "e01325356fc4666f13e90d7cb8dfdbd6", 8167901),  // DX, windows
	UNSUPPORTED_GAME_ENTRY("spacemisadventures", "Space Misadventures DX.ags", "5cf9a3c00a70f70976961a8ea01fd249", 5134793),  // DX, linux
	UNSUPPORTED_GAME_ENTRY("sstrek25", "startrek.exe", "9952a3da8575c86fc35ffc02bafba147", 12793368), // v1.13b Win
	UNSUPPORTED_GAME_ENTRY("sstrek25", "startrek.ags", "d339b6f11561d0b74e90c475d8f4d5c5", 9605882), // v1.13a Linux/Mac
	UNSUPPORTED_GAME_ENTRY("stewlasmoras", "Stew.ags", "b3b5f1ce9819edb24eeb1dce759a2177", 7645078),
	UNSUPPORTED_GAME_ENTRY("stewlasmoras", "Stew.ags", "3abadfacf5722459f8217afb1e1e90f3", 7606074),  // v1.0.1
	UNSUPPORTED_GAME_ENTRY("stopags", "Stop.ags", "f5f834d8c625c114c97c18c921dffa2f", 296423546),
	UNSUPPORTED_GAME_ENTRY("strangerutopia", "StrangerInUtopia.exe", "5c3a31d27beb519dfe291ea27ead1371", 61286148), // Win
	UNSUPPORTED_GAME_ENTRY("strangerutopia", "StrangerInUtopia.exe", "5c3a31d27beb519dfe291ea27ead1371", 61904777),
	UNSUPPORTED_GAME_ENTRY("strangerutopia", "StrangerInUtopia.exe", "5c3a31d27beb519dfe291ea27ead1371", 62289972), // Win Fr
	UNSUPPORTED_GAME_ENTRY("strangerutopia", "StrangerInUtopia.ags", "ca1710839f34ef715bc61d883f4df47c", 58034416), // Linux
	UNSUPPORTED_GAME_ENTRY("strangerutopia", "StrangerInUtopia.ags", "7343e94d0afe0d24d49f0451cf6c56dd", 58653045),
	UNSUPPORTED_GAME_ENTRY("takes2tangle", "IT4.ags", "763959969411fd0d991d0938b28c383e", 298209870),
	UNSUPPORTED_GAME_ENTRY("thezooags", "The Zoo.exe", "031470656455b56887aa57a3cff9e016", 5221113),
	UNSUPPORTED_GAME_ENTRY("thezooags", "The Zoo.ags", "f2693962a476d108249433b555e87e7a", 2085605),
	UNSUPPORTED_GAME_ENTRY("zalgohs", "ZTT.ags", "4ab5683ccc5a075cd336493239678138", 17650653),


	// AGS 4 games
	UNSUPPORTED_GAME_ENTRY("achristmasnightmare", "xmasnightmare.exe", "53db6ba4864b17a28b8176d92459c7bc", 49689828),  // v1.3 Windows En-Es-It
	UNSUPPORTED_GAME_ENTRY("castleescapech2", "CastleEscapeChapter2.exe", "124753417c6ccda01c93f9935eb5a87d", 4527841),  // Windows
	UNSUPPORTED_GAME_ENTRY("castleescapech2", "CastleEscapeChapter2.ags", "3a1136db0175eb81b499bc4c755ecbee", 1549005),  // Linux
	UNSUPPORTED_GAME_ENTRY("dreadmacfarlaneisep1", "Dread Mac Farlane 1.ags", "f9d45767a3cf610b6fe777da54c5234d", 468102549),
	UNSUPPORTED_GAME_ENTRY("dreadmacfarlaneisep1", "Dread Mac Farlane 1_English.ags", "68999f3a425f0da1d558a72a8bff7f26", 468102549),
	UNSUPPORTED_GAME_ENTRY("dreadmacfarlaneisep2", "Dread Mac Farlane 2.ags", "df836f1cd98fdbf959d49701e47e2207", 296604541),
	UNSUPPORTED_GAME_ENTRY("dreadmacfarlaneisep2", "Dread Mac Farlane 2 Eng.ags", "58fd48707a515f2157d17b7709e69e5e", 296604545),
	UNSUPPORTED_GAME_ENTRY("gloriouswolfcomicsep1", "Glorious Wolf.ags", "578f34ce108225b7b03091b424655f6e", 1592100251),
	UNSUPPORTED_GAME_ENTRY("gloriouswolfcomicsep2", "Glorious Wolf 2.ags", "e22aacb6016af866f7b241db47a290f4", 1283086429),
	UNSUPPORTED_GAME_ENTRY("gloriouswolfcomicsep2", "Glorious Wolf 2.ags", "92d78970f88789f12275970126aa83f2", 1279399200),
	UNSUPPORTED_GAME_ENTRY("paintedhills", "Painted Hills.ags", "95becb7b83df4e176aa307223d0b7181", 32864460),
	UNSUPPORTED_GAME_ENTRY("useitall", "mags2402.exe", "41cd9288896f33d2003f3f3a3a208482", 5212519),  // Win
	UNSUPPORTED_GAME_ENTRY("useitall", "mags2402.ags", "bf6501e529633843043b94643bcb8348", 2219859),  // Linux

	// Commercial games
	GAME_ENTRY_STEAM("3geeks", "3GEEKS-Adventure.exe", "7ddb9e776648faed5a51170d087074e9", 597467679), // En-Fr
	GAME_ENTRY("7metresaway", "7 Meters away.exe", "5e5cdce3a9549bca34fb3b0e52074977", 691572532),  // itch.io Eng-Esp 1.3
	GAME_ENTRY("7metresaway", "7 Meters away.exe", "5e5cdce3a9549bca34fb3b0e52074977", 691675737),  // itch.io Eng-Esp 1.4
	GAME_ENTRY("7metresaway", "7 Meters away.exe", "5e5cdce3a9549bca34fb3b0e52074977", 691675944),  // itch.io Eng-Esp 1.5
	GAME_ENTRY_EN_STEAM("abscission", "Abscission.exe", "7633f0919378749cebf6dacba61ca0e1", 350299379),
	GAME_ENTRY_EN_STEAM("abscission", "Abscission.exe", "7633f0919378749cebf6dacba61ca0e1", 350300404),
	GAME_ENTRY_EN_STEAM("alemmo", "al-emmo.exe", "ff22fd118f18eca884dc7ed2a5be3877", 19045178),
	GAME_ENTRY_EN("alemmo", "al_emmo.exe", "bb9e32ee92bb0996df5fea50e88d686a", 19036716), // 4.1, Desura
	GAME_ENTRY_EN("alemmo", "al_emmo.exe", "d14295053c672c253460c8a7179b2bba", 19054865),
	GAME_ENTRY_EN("alemmo", "al_emmo.exe", "4139d665622b2e3c5a31e90fc530fc74", 22442059),
	GAME_ENTRY_EN("alemmo", "al_emmo.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 19181537),
	GAME_ENTRY_LANG("alemmo", "al_emmo.exe", "1b437e4d969480e6e8bccad2873701d0", 21201908, Common::DE_DEU), // German DVD
	GAME_ENTRY_LANG("alemmo", "Al_EmmoSPA.exe", "a6608b37277f1220a1aecc6c18102a04", 2479104, Common::ES_ESP), // Fanmade Spanish translation
	GAME_ENTRY_EN("alemmoanozira", "postcards from anozira.exe", "f5f73b35e809d9e01a2cff32abeffadf", 5567476), // 3.0, Desura
	GAME_ENTRY_EN_STEAM("alemmoanozira", "Postcards_from_Anozira.exe", "a33691e4a84e2645b4cbe1f681511798", 5567690),
	GAME_ENTRY_EN("alemmoanozira", "postcards from anozira.exe", "893f31faa33f1219b316894a0cbe7b27", 174781776),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("alum", "alum.exe", "6d2f8e80c5f2372b705fdd4cc32f3579", 429203260, 0), // Windows
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("alum", "Alum.exe", "6d2f8e80c5f2372b705fdd4cc32f3579", 429197066, 0), // Linux & Mac
	GAME_ENTRY_EN("alum", "alum.exe", "6d2f8e80c5f2372b705fdd4cc32f3579", 428902430),
	GAME_ENTRY_PLATFORM("atotk", "atotk.exe", "68d4f3488a9dcec74584651c0e29e439", 5078719, "Steam/Deluxe"),
	GAME_ENTRY_PLATFORM("atotk", "atotk.ags", "68d4f3488a9dcec74584651c0e29e439", 5078866, "Steam/Deluxe"), // 2.04 Eng-Hun
	GAME_ENTRY_PLATFORM("atotk", "ATOTK.ags", "e6964f210afb6a6e967434707e63f12c", 62648678, "Deluxe"), // 2.0.6 Eng-Hun-Ger (Steam/Zoom)
	GAME_ENTRY_PLATFORM("atotk", "ATOTK.ags", "427e0c98848d2afd7bc8534b481927e9", 62648886, "GOG.com/Deluxe"), // 2.0.6 Eng-Hun-Ger
	GAME_ENTRY("apotheosis", "the apotheosis project.exe", "0cab252e992e5da65bdbf3648c2b70df", 1144087889), // 1.0
	GAME_ENTRY_STEAM("apotheosis", "the apotheosis project.exe", "0115a64ddec9396108f32da31d761ecb", 1144677502),  // Eng-Ita
	GAME_ENTRY_STEAM("apotheosis", "The Apotheosis Project.exe", "4c0c28d58ebd53823fc0248e5b777c52", 1144826071),
	GAME_ENTRY_STEAM("apotheosis", "The Apotheosis Project.ags", "0d9ae5a4ffd3e3e968c6addb5d524954", 1142362222), // Linux
	DETECTION_ENTRY_GUIO("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 51054188, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // Original (rollback 0.0) Multilang
	DETECTION_ENTRY_GUIO("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 51056669, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.1 (rollback 0.1)
	DETECTION_ENTRY_GUIO("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 51089445, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.2
	DETECTION_ENTRY_GUIO("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 51063519, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.3 updated
	DETECTION_ENTRY_GUIO("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 54145006, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v2.0? (rollback 0.2)
	DETECTION_ENTRY_GUIO("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 54144779, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v2.0 updated
	GAME_ENTRY_EN_STEAM("astroloco", "astroloco1.exe", "c71919e3b0cc415ef725cf1a9818a515", 42894919),
	GAME_ENTRY_EN("astroloco", "astroloco1.exe", "06e70a826fde73b3f86f974885d31abe", 42492070),
	GAME_ENTRY("avcsurvivalguide", "CAV.exe", "42656cf777d07d4d35e978b233bb7aa8", 11001785),  // Eng-Esp itch.io 1.0
	GAME_ENTRY_EN_STEAM("azazelxmas", "Azazels_Christmas_Fable.exe", "46fcfdc2aa113c05f3ba95ad356e1a63", 408594796),
	GAME_ENTRY_EN_STEAM("beer", "beer!.exe", "6f201fd7a19869c85f49c7c471d0479a", 5055091), // Windows
	GAME_ENTRY_EN_STEAM("beer", "beer!.ags", "4a751c43af8699aabdb0b9ebcc2024d3", 2145485), // Eng Win v1.5
	GAME_ENTRY_EN_STEAM("beer", "Beer!.ags", "6f201fd7a19869c85f49c7c471d0479a", 5042748), // Linux
	GAME_ENTRY_EN_GOG("beer", "Beer!.ags", "b8a3e5c5284a0f7a8d62e2fa259c7fe7", 2144118), // v1.5
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("beyondowlsgard", "Owlsgard.exe", "0f647ddfd86c3dad2d68055fab21f091", 556521058, 0), // Eng-Deu v1.1
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("beyondowlsgard", "Owlsgard.exe", "0f647ddfd86c3dad2d68055fab21f091", 556512803, 0), // Eng-Deu Win
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("beyondowlsgard", "Owlsgard.exe", "0f647ddfd86c3dad2d68055fab21f091", 561707606, 0), // En-De-Fr-Es Win v1.2
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("beyondowlsgard", "Owlsgard.ags", "ec06f55f20c49b5cb2df6c9e16440aeb", 552945167, 0), // Eng-Deu Linux
	GAME_ENTRY_EN_STEAM("bizarreearthquake", "bizarre.exe", "b142b43c146c25443a1d155d441a6a81", 6370092),
	GAME_ENTRY_EN("blackfriday2", "Autumn of Death - Black Friday II.exe", "5275912e6a5266b64b8cf282ccbf6385", 33128099),  // Windows
	GAME_ENTRY_EN("blackfriday2", "Autumn of Death - Black Friday II.ags", "8261bc29effd192533a72943237c5c8f", 30551187),  // Linux
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "blackwell1.exe", "605e124cb7e0b56841c471e2d641c224", 18822697, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67314713, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67316743, 0),  // v2.2.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67331239, 0),  // v2.2.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67332268, 0),  // v2.2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67332730, 0),  // v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67337164, 0),  // v2.4a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48439487, 0),  // v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48443257, 0),  // v3.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48445669, 0),  // v3.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48450307, 0),  // v3.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48452795, 0),  // v3.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48454110, 0),  // v3.4b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48457210, 0),  // v3.5
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48460291, 0),  // v3.5a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48491927, 0),  // v3.6/7
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "9f7e0d47caa495c98d8ea781fb52be49", 45246635, 0),  // Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "504d537cc8af0b027ac7c2474a364ff9", 45250405, 0),  // Linux v3.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "7bcf1414a4196709cb58975772e09ff2", 45252817, 0),  // Linux v3.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "e256db7d5e726e3b522cfb5bae9c4ed1", 45257455, 0),  // Linux v3.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "221844b84fc047e9f5d4ed33ded98bc9", 45259943, 0),  // Linux v3.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "dd5b95bdc060b1a88911cf8f678173e6", 45261258, 0),  // Linux v3.4b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "b197730f059f9537efa5811da10ffa61", 45264358, 0),  // Linux v3.5
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "164fb92c9efc0ff928cc6ba82fd554fb", 45267439, 0),  // Linux v3.5a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "2bf7d1146061cd8a02b80802ca269977", 45299075, 0),  // Linux v3.6/7
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "7865a17a36ff5a1844cd5359071eb04b", 64183795, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "5b63e0d2bc117cc9d32b88bfb5de167c", 64199320, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "6d17d5534a5c77a419117975053a1e90", 64199782, 0), // Mac v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "7769be646afbdce6909e568a0d9bc857", 64204216, 0), // Mac v2.4a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "9f7e0d47caa495c98d8ea781fb52be49", 45246635, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "504d537cc8af0b027ac7c2474a364ff9", 45250405, 0), // Mac v3.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "e256db7d5e726e3b522cfb5bae9c4ed1", 45257455, 0), // Mac v3.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "221844b84fc047e9f5d4ed33ded98bc9", 45259943, 0), // Mac v3.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "dd5b95bdc060b1a88911cf8f678173e6", 45261258, 0), // Mac v3.4b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "b197730f059f9537efa5811da10ffa61", 45264358, 0), // Mac v3.5
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "164fb92c9efc0ff928cc6ba82fd554fb", 45267439, 0), // Mac v3.5a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "2bf7d1146061cd8a02b80802ca269977", 45299075, 0), // Mac v3.6
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "blackwell1.exe", "605e124cb7e0b56841c471e2d641c224", 18824597, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "blackwell1.exe", "605e124cb7e0b56841c471e2d641c224", 19757071, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "blackwell1.exe", "e5a75b86a0ea8143e1784261f5f52e83", 67241529, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "92af7315126c6da1e9e0c641cd9df200", 67336951, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "ff5b05b81909683fcd57580c58a81d87", 64200209, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux & Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "1051d3fa462b1e0cc8973a8775128307", 66679994, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "033a766f419e57c6ecec4e8dbae9a16a", 45257250, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac 3.3
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "0965b59f01d15cd341efaa7130fab41e", 45259738, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac 3.4
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "ab9d20216445fc247089bc797441e9be", 45261053, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac 3.4b
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "9788ea2eb3bbccc950ed8cb8589c23b0", 45264153, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac 3.5
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "595e214d2a2aa94d8c301f90e3a783aa", 45267234, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac 3.5a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "ac2game.dat", "698f09fd8c63da0e437e7ae773c8bfda", 45298870, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac 3.6
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48439274, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48443044, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48443052, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.1 updated
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48445464, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.2
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48450102, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.3
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48452590, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.4
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48453905, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.4b
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48457005, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.5
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48460086, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.5a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.exe", "ffc42a6ea2562dc1fb91c03a2d0c5cbe", 48491722, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win 3.6/7
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "670f3eaa22243dc1742964b697108ee8", 45246422, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "ea3ea2ce1958205810eafa9f826c4dd6", 45250192, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "1f6a8615f1da54cac5c98672a2341819", 45252612, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.2
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "033a766f419e57c6ecec4e8dbae9a16a", 45257250, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.3
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "0965b59f01d15cd341efaa7130fab41e", 45259738, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.4
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "ab9d20216445fc247089bc797441e9be", 45261053, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.4b
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "9788ea2eb3bbccc950ed8cb8589c23b0", 45264153, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.5
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "595e214d2a2aa94d8c301f90e3a783aa", 45267234, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.5a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "698f09fd8c63da0e437e7ae773c8bfda", 45298870, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux 3.6/7
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell1", "Blackwell Legacy.ags", "99f4ff1080509c856be96ba55e644380", 45277507, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Steamdeck
	GAME_ENTRY_EN("blackwell1", "blackwell1.exe", "605e124cb7e0b56841c471e2d641c224", 18824247), // Humble Bundle (Windows)
	GAME_ENTRY_EN("blackwell1", "ac2game.dat", "a81e47e6077f7049706ee59356435286", 18798215), // Big Fish Games (Windows)
	GAME_ENTRY_EN("blackwell1", "ac2game.dat", "ca6edbac96264b2adf73ef10d5a0348d", 18800040), // v1.4 Wadjet Eye store (Win)
	GAME_ENTRY_EN("blackwell1", "ac2game.dat", "605e124cb7e0b56841c471e2d641c224", 68665467), // Android
	GAME_ENTRY_EN("blackwell1", "ac2game.dat", "4668e05ef2a94267b0d571e715413302", 64200367), // Humble Bundle (Linux)
	GAME_ENTRY_EN("blackwell1", "ac2game.dat", "7fe0cac6736427d67822bca3d4bb30cb", 66675482),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "5c3a940514d91431e8e1c372018851ca", 14493753, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "8e3a3a985acc65b2a5c32fab0a998286", 60754659, 0), // v2.2.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "8e3a3a985acc65b2a5c32fab0a998286", 60767448, 0), // v2.2.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "8e3a3a985acc65b2a5c32fab0a998286", 60767900, 0), // v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "8e3a3a985acc65b2a5c32fab0a998286", 60768044, 0), // v2.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41517519, 0), // v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41518704, 0), // v3.0c
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41521377, 0), // v3.0d
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41526213, 0), // v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "67d4c87f7005084587f3c072be7279e5", 38329275, 0), // Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "610d1a1a025809083761c3236a527889", 38330460, 0), // Linux v3.0c
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "68586a9bdf0ba899540f09d9990f26fc", 38333133, 0), // Linux v3.0d
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "66a36785d7ab52f3d5de0e1813d8b2c4", 38337969, 0), // Linux v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "82f8bf2635ea1b5bfb2e8693fa883f89", 57638596, 0), // Mac
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "c787c663b92cb3596a8e7219f0f9bb25", 57639048, 0), // Mac v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "53b931909fc86ede449eb371e5396c36", 57639192, 0), // Mac v2.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "67d4c87f7005084587f3c072be7279e5", 38329275, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "610d1a1a025809083761c3236a527889", 38330460, 0), // Mac v3.0c
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "68586a9bdf0ba899540f09d9990f26fc", 38333133, 0), // Mac v3.0d
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "66a36785d7ab52f3d5de0e1813d8b2c4", 38337969, 0), // Mac v3.1a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "5c3a940514d91431e8e1c372018851ca", 14496128, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Win
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "5c3a940514d91431e8e1c372018851ca", 14469500, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "00edc7b69ae377f6093ac567fd901849", 15683333, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "8e3a3a985acc65b2a5c32fab0a998286", 60767841, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41517316, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41517324, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41518509, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41521182, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.exe", "0c77a306c0604c46e8c3bdb1002e453b", 41526018, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "87c0681f4eebafddc60533f799456d53", 57672335, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux & Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "7dc1aa012f45b6b7a86bd63b59f84009", 38330265, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "b73664ebe89fda8cfcb1c9e071bb53cc", 38332938, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "ac2game.dat", "ae0e575ddddfa5a7dc53ffb74612737f", 38337774, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "e77c03d5033a498f918e7fa872f53309", 38329072, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "1b531ccc144a878710db13606c3afe1b", 38329080, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "7dc1aa012f45b6b7a86bd63b59f84009", 38330265, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "b73664ebe89fda8cfcb1c9e071bb53cc", 38332938, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "unbound.ags", "ae0e575ddddfa5a7dc53ffb74612737f", 38337774, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell2", "Unbound.ags", "8851175e98d9d5867d4799f3955e563f", 38385837, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Steamdeck
	GAME_ENTRY_EN("blackwell2", "unbound.exe", "5c3a940514d91431e8e1c372018851ca", 14495742), // Humble Bundle (Windows)
	GAME_ENTRY_EN("blackwell2", "ac2game.dat", "c9dc789649c7f9231407c776f6829497", 13340806), // Big Fish Games (Windows)
	GAME_ENTRY_EN("blackwell2", "ac2game.dat", "5c3a940514d91431e8e1c372018851ca", 69452991), // Android
	GAME_ENTRY_EN("blackwell2", "ac2game.dat", "e28670a676a3e4e78eef76b9573e490a", 57670660), // Humble Bundle (Linux)
	GAME_ENTRY_EN("blackwell2", "ac2game.dat", "5c3a940514d91431e8e1c372018851ca", 59483140),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "2260c1a21aba7ac00baf0100d4ca54f1", 172575801, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "1fbaa55037f7f1c8c5e59b2dacc994e2", 173960270, 0),  // v2.2.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "1fbaa55037f7f1c8c5e59b2dacc994e2", 173960749, 0),  // v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "1fbaa55037f7f1c8c5e59b2dacc994e2", 173960748, 0),  // v2.3b Win-Linux
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104791861, 0),  // v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104791880, 0),  // v3.0b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104795348, 0),  // v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.ags", "c36dd508e650e798ccd4673df6f61a8f", 101541153, 0),  // Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.ags", "cba47b92a794f69d9dfde32e0b09e733", 101541172, 0),  // Linux v3.0b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "convergence.ags", "0e5941423ec7d2d34be9dd0fce38b5da", 101544640, 0),  // Linux v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "0b99002018dcf6d3db95925006da01bb", 170769466, 0), // Mac
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "0b6f9ba727ce934a515dcff9af9c3b9e", 170769945, 0), // Mac v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "613c49405b24b367a961a7e14a3849d0", 170769944, 0), // Mac v2.3b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "c36dd508e650e798ccd4673df6f61a8f", 101541153, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "cba47b92a794f69d9dfde32e0b09e733", 101541172, 0), // Mac v3.0b
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "0e5941423ec7d2d34be9dd0fce38b5da", 101544640, 0), // Mac v3.1a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "2260c1a21aba7ac00baf0100d4ca54f1", 172578803, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "2260c1a21aba7ac00baf0100d4ca54f1", 171969680, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "f261c2505f98503a1ec9f1dff7f13bda", 173873229, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "1fbaa55037f7f1c8c5e59b2dacc994e2", 173960270, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "1fbaa55037f7f1c8c5e59b2dacc994e2", 173960518, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win-Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104791631, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104791639, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104791658, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.exe", "7335d87ffcafea6c9d833a4accb2a986", 104795126, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.ags", "6aef643a63ffb0ab0bf2df290c19a971", 101540923, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.ags", "b857e7009df631cb1eee4ce7ba93878d", 101540950, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "convergence.ags", "a3f8b25ef46d0f76113307e2c56b682c", 101544418, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "2844e3823d74652d282b8173bbde2b8b", 170703726, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux & Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "0b99002018dcf6d3db95925006da01bb", 170769466, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "b857e7009df631cb1eee4ce7ba93878d", 101540950, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "ac2game.dat", "a3f8b25ef46d0f76113307e2c56b682c", 101544418, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell3", "Convergence.ags", "a20d1e625f1832f2282e7f595c1de70a", 101579536, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Steamdeck
	GAME_ENTRY_EN("blackwell3", "convergence.exe", "2260c1a21aba7ac00baf0100d4ca54f1", 172578032), // Humble Bundle (Windows, v1.1)
	GAME_ENTRY_EN("blackwell3", "ac2game.dat", "b177ae79e72dfacad8c3af0a64d80395", 171379482), // Big Fish Games (Windows)
	GAME_ENTRY_EN("blackwell3", "ac2game.dat", "c894f6005f479ba8f12e87223920aa6a", 170700456), // Humble Bundle (Linux)
	GAME_ENTRY_EN("blackwell3", "ac2game.dat", "2260c1a21aba7ac00baf0100d4ca54f1", 173301005), // Android
	GAME_ENTRY_EN("blackwell3", "ac2game.dat", "2260c1a21aba7ac00baf0100d4ca54f1", 172574888),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "b3b192cf20a2f7666ddea3410dbd87cc", 303459336, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "1c0bf79d9720d0a81219e8778fb7e231", 304040703, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "8f7335b030b38b610637b3033859f828", 305293936, 0),  //v2.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "8f7335b030b38b610637b3033859f828", 305294420, 0),  //v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219793409, 0),  //v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219796992, 0),  //v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219796990, 0),  //v3.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219797080, 0),  //v3.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.ags", "3c999468155295d3e0809c04b2e53cff", 216579565, 0),  //Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.ags", "1f174c44bfa6725ceb6b80aa9cc69d7e", 216583148, 0),  //Linux v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.ags", "ff5e37e618eb9ed86c4a40de806e9a4a", 216583146, 0),  //Linux v3.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "deception.ags", "836738be511b4854c2e22b8519aa27fa", 216583236, 0),  //Linux v3.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "agsgame.dat", "1c0bf79d9720d0a81219e8778fb7e231", 304040703, 0), // Linux
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "05a3a5ff709a5764d2b4a5dc301107d6", 302140480, 0), // Mac
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "3c999468155295d3e0809c04b2e53cff", 216579565, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "e08b56307a597c1384e45b7e6e25d14c", 216582921, 0), // Mac v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "ff5e37e618eb9ed86c4a40de806e9a4a", 216583146, 0), // Mac v3.2
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "b3b192cf20a2f7666ddea3410dbd87cc", 304076138, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "499c3261a1f73c5b78125beeca1c2d08", 305215631, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "8f7335b030b38b610637b3033859f828", 305293936, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219793174, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219793182, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219796765, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219796763, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.exe", "9cca5df9331a98e910cce4d5b4ea24f0", 219796853, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "8492a5cf8a91044a8505bc62bc1e9dfc", 216579330, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "1f174c44bfa6725ceb6b80aa9cc69d7e", 216583148, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "ac2game.dat", "fa0be2343abc9833940630b40dbc0042", 216582919, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.ags", "8492a5cf8a91044a8505bc62bc1e9dfc", 216579330, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.ags", "e08b56307a597c1384e45b7e6e25d14c", 216582921, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.ags", "fa0be2343abc9833940630b40dbc0042", 216582919, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell4", "deception.ags", "bfa4de81ea9cf6654a399a92f7148e32", 216583009, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE), // Linux
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "c1cddd6fcdbcd030beda9f10d4e4270a", 281849897, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "c3f7a995bbea7ce4ba7a2a97995c677e", 283092455, 0),  //v2.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "c3f7a995bbea7ce4ba7a2a97995c677e", 283092983, 0),  //v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "ee9f05e549e1890a66fa29f174514d83", 247075915, 0),  //v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "ee9f05e549e1890a66fa29f174514d83", 247079377, 0),  //v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.ags", "de7d49da1a1c5e0cd5b01a3c3770921d", 243851319, 0),  //Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "epiphany.ags", "de7d49da1a1c5e0cd5b01a3c3770921d", 243854781, 0),  //Linux v3.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "agsgame.dat", "c1cddd6fcdbcd030beda9f10d4e4270a", 281849897, 0), // Linux
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "ac2game.dat", "8acaa20eab5589cdc2fd81ef3d55eff3", 279928291, 0), // Mac v2.3
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "ac2game.dat", "de7d49da1a1c5e0cd5b01a3c3770921d", 243851319, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("blackwell5", "ac2game.dat", "de7d49da1a1c5e0cd5b01a3c3770921d", 243854781, 0), // Mac v3.1a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "c1cddd6fcdbcd030beda9f10d4e4270a", 281856724, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "764f20abb335b94ab8ec6a4ef6db01ea", 283020359, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "c3f7a995bbea7ce4ba7a2a97995c677e", 283092455, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "ee9f05e549e1890a66fa29f174514d83", 247075636, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "ee9f05e549e1890a66fa29f174514d83", 247075644, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.exe", "ee9f05e549e1890a66fa29f174514d83", 247079106, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "ac2game.dat", "de7d49da1a1c5e0cd5b01a3c3770921d", 243851040, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "ac2game.dat", "de7d49da1a1c5e0cd5b01a3c3770921d", 243854510, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.ags", "de7d49da1a1c5e0cd5b01a3c3770921d", 243851040, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("blackwell5", "epiphany.ags", "de7d49da1a1c5e0cd5b01a3c3770921d", 243854510, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux
	GAME_ENTRY_EN_GOG("brownieadv1", "Brownie1.exe", "3c8d2feaba74ad4978316824c903c046", 140161903),
	GAME_ENTRY_EN_STEAM("captaindisaster", "cd-dhamsb-1-0-1.exe", "e2d290f8f21c6a83a9e8c5f3a0425b5e", 150665897),
	DETECTION_ENTRY_GUIO("castleagony", "Agony.exe", "387ff720e746ae46e93f463fd58d77a4", 21017019, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("charnelhousetrilogy", "cht.ags", "55c782c9de5a09157ea6aafac90b9cc8", 339257135, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("charnelhousetrilogy", "cht.exe", "29c49db0805500ec14964eb897fc7b02", 341425110, 0),
	GAME_ENTRY_EN("charnelhousetrilogy", "cht.exe", "9ba3d9fbb098a26d7293ad3161c0f270", 341411868),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("chronicleofinnsmouth", "chronicleofinnsmouth.exe", "f2e3fe96788b72ef2bf9429c58716099", 552972692, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("chronicleofinnsmouth", "chronicleofinnsmouth.exe", "cb7ab3394128c52293417eae5dacf365", 552728591, 0),  // En-It
	GAME_ENTRY_EN("conspirocracy", "Conspirocracy.exe", "e137b66b22b1d8dbe4fb78e54f2b3de1", 49080280),  // Fireflower Games
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("contact", "Contact.exe", "b16cdffccf0a9488a2d623ced3b6dfdd", 26395439, 0),  // Eng-Kor
	GAME_ENTRY_STEAM("content", "Content.exe", "1e950496692a009ea163eb276702e008", 29813674),  // Eng-Kor
	GAME_ENTRY_EN("cosmosquest3", "cq3.exe", "18b284c22010850f79bc5c20054a70c4", 8674790),
	GAME_ENTRY_EN("cosmosquest4", "cq4.exe", "e3962995a70923a8d5a8f1cf8f932eee", 17062325),
	GAME_ENTRY_EN_STEAM("crimsondiamond", "The Crimson Diamond - Full Game.exe", "38a49a28c0af599633006c09fe6c2f72", 159585457), // Final Release Win
	GAME_ENTRY_EN_STEAM("crimsondiamond", "game.ags", "38a49a28c0af599633006c09fe6c2f72", 159579164), // Final Release Mac
	GAME_ENTRY_EN_STEAM("crimsondiamond", "The Crimson Diamond - Full Game.exe", "38a49a28c0af599633006c09fe6c2f72", 159589239), // itch.io 1.02b
	GAME_ENTRY_EN_GOG("crimsondiamond", "The Crimson Diamond - Full Game.exe", "38a49a28c0af599633006c09fe6c2f72", 159920906), // 1.07b
	GAME_ENTRY_STEAM("crystalshardadventurebundle", "bundle.exe", "1c45d5c239d9eeef61e283983b6e6851", 4645713),  // Multilang
	GAME_ENTRY_STEAM("crystalshardadventurebundle", "bundle.ags", "9b7610e4330a7aa2118bc3bdb8d2ce7e", 1935844),  // Multilang
	GAME_ENTRY_EN("danewguys2", "jackass.exe", "029aab0fef5e9fff7b95ddf5d79d7718", 23932842), // Humble Store
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("detectivebhdino", "Detective Boiled-Hard  Case File - Death of the Space Dino Hunter.exe", "4d5d25446373d4df71c6bda77a15230a", 49618396, 0),
	GAME_ENTRY_STEAM("detectivegallo", "dgbuild.exe", "2e0678b5642c64e057022a79742743fb", 90740554),  // Multilang
	GAME_ENTRY_GOG("detectivegallo", "dgbuild.exe", "2e0678b5642c64e057022a79742743fb", 90740772),
	GAME_ENTRY("detectivegallo", "dgbuild.exe", "2e0678b5642c64e057022a79742743fb", 90713626), // Big Box Limited Edition DVD
	GAME_ENTRY_EN("diamondsintherough", "Diamonds.exe", "14c995189266969d538b31d1df451d3f", 180079079),
	GAME_ENTRY_STEAM("docapocalypse", "doc_apocalypse.exe", "1a7dfeb7797720111f9e40b5cbfea4c4", 47961989),
	GAME_ENTRY("docapocalypse", "doc_apocalypse.exe", "1a7dfeb7797720111f9e40b5cbfea4c4", 47959435),
	GAME_ENTRY("docapocalypse", "doc_apocalypse.exe", "c03b77cb9fff52a1fdd1135b6fec150f", 802648268),  // itch.io  Eng-Ita
	GAME_ENTRY_EN_STEAM("downfall2009", "downfall.exe", "aabdafae8b57dfc48fdf158a72326c23", 183357927),
	GAME_ENTRY_EN_STEAM("downfall2009", "downfall.exe", "aabdafae8b57dfc48fdf158a72326c23", 279342949), // 1.4
	GAME_ENTRY_LANG("downfall2009", "downfall.exe", "10212624b951711c4b44f5cb9eecbf0a", 143360, Common::RU_RUS), // GOG
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("downfall2016", "downfall.exe", "08b1340f3528feeebce1ecc59cc17907", 1995812750, 0),  // Multilang
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("downfall2016", "downfall.exe", "08b1340f3528feeebce1ecc59cc17907", 224368590, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("downfall2016", "Downfall.ags", "7c87b99ce309a46085e40ac1a2b20e75", 224024207, 0), // Linux
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484767783, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.01
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484767190, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.02
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484783215, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.04
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484795246, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.05
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484837277, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.06
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484844189, Common::EN_ANY, "GOG.com", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.07
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484795038, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.05
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 484843981, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.07
	DETECTION_ENTRY_GUIO("dreamswitchhouse", "Dreams in the Witch House.exe", "160d78e898924f5cb1347b13746cc935", 489032242, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // v1.08 beta
	DETECTION_ENTRY_GUIO("dustbowl", "dustbowl.exe", "aa349d52fd620cf9642935cd5bdec5d8", 63365026, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	GAME_ENTRY_EN("dustbowl", "dustbowl.exe", "aa349d52fd620cf9642935cd5bdec5d8", 82185295),
	DETECTION_ENTRY_GUIO("englishhaunting", "An English Haunting.exe", "de2b4205a56fce1b199a692a8b65e161", 987485394, Common::UNK_LANG, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, 0), // itch.io Eng-Esp
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("englishhaunting", "An English Haunting.exe", "de2b4205a56fce1b199a692a8b65e161", 987491517, 0),
	DETECTION_ENTRY_GUIO("excavationhb", "TEOHB.exe", "f176b46bc89e227f745dae9878171676", 566323169, Common::EN_ANY, "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, 0), // Win
	DETECTION_ENTRY_GUIO("excavationhb", "ac2game.dat", "e5553f7c45d26d5fbc8b376a859bb87c", 563281442, Common::EN_ANY, "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP), // Mac
	DETECTION_ENTRY_GUIO("excavationhb", "ac2game.dat", "e5553f7c45d26d5fbc8b376a859bb87c", 563283917, Common::EN_ANY, "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP), // Mac
	DETECTION_ENTRY_GUIO("excavationhb", "TEOHB.exe", "f176b46bc89e227f745dae9878171676", 566320586, Common::EN_ANY, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),
	DETECTION_ENTRY_GUIO("excavationhb", "TEOHB.exe", "f176b46bc89e227f745dae9878171676", 566323443, Common::EN_ANY, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),  // Win 1.05
	DETECTION_ENTRY_GUIO("excavationhb", "ac2game.dat", "e5553f7c45d26d5fbc8b376a859bb87c", 563284191, Common::EN_ANY, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP), // Mac
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("falconcity", "game.exe", "e816b31cfe3512c2ec24ac0bc6cfc605", 584191058, 0),  // Eng-Hun-Chi
	GAME_ENTRY_EN_STEAM("feriadarles", "feria d'arles.exe", "6a3291595263debd129e1e2064baeea5", 275649462),
	GAME_ENTRY_EN_STEAM("feriadarles", "ac2game.dat", "6a3291595263debd129e1e2064baeea5", 275640157), // Mac
	GAME_ENTRY_EN("feriadarles", "feria d'arles.exe", "6a3291595263debd129e1e2064baeea5", 275659086),
	GAME_ENTRY_STEAM("footballgame", "Football Game.exe", "76703ac67835bdbfde5b378a147c8ba2", 164951859),  // En-Fr-De-Es
	GAME_ENTRY("footballgame", "football game.exe", "76703ac67835bdbfde5b378a147c8ba2", 164951859),
	GAME_ENTRY("footballgame", "football game.exe", "8283bcdb6ec07ec4a4040ef34215ec96", 164960522),
	GAME_ENTRY_PLATFORM("footballgame", "AGSProject.ags", "cf44ba9de782613a948c7ba98710d8f8", 162531547, "Switch"),  // Nintendo Switch
	GAME_ENTRY("geminirue", "GeminiRue.exe", "0bdfb2139abaae5ae2dc85f1aacfd3f6", 61906750), // v1.1 CD Release
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "f3c0c7d3892bdd8963e8ce017f73de08", 61986506, 0),
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini_rue_pc.exe", "f49a61ea46feb86f89de3c136ad809ff", 73412249, 0),  // Win v1.03
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "5f747ea1c5f624d3716926fe1ca5135d", 73450387, 0),  // Win v2.0
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "5f747ea1c5f624d3716926fe1ca5135d", 73486515, 0),  // Win v2.1
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "5f747ea1c5f624d3716926fe1ca5135d", 73505075, 0),  // Win v2.2
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "5f747ea1c5f624d3716926fe1ca5135d", 73541907, 0),  // Win, Linux v2.3
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "3b7e8fdf626c2f8e6538669cdab91c0e", 61401378, 0),  // Win v3.0
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "gemini rue.exe", "3b7e8fdf626c2f8e6538669cdab91c0e", 61438210, 0),  // Win v3.0.1
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "ac2game.dat", "056239de34dfa52cfe1b645eee4eacb7", 70356639, 0), // Mac v2.1
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "ac2game.dat", "056239de34dfa52cfe1b645eee4eacb7", 70375199, 0), // Mac v2.2
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "ac2game.dat", "056239de34dfa52cfe1b645eee4eacb7", 70412031, 0), // Mac v2.3
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "ac2game.dat", "ab6c0c2d89adb2d529131190a657be6a", 58212622, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "ac2game.dat", "ab6c0c2d89adb2d529131190a657be6a", 58249454, 0), // Mac v3.0.1
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "agsgame.dat", "f3c0c7d3892bdd8963e8ce017f73de08", 62069353, 0), // Linux
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "Gemini Rue.ags", "ab6c0c2d89adb2d529131190a657be6a", 58212622, 0),  // Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("geminirue", "Gemini Rue.ags", "ab6c0c2d89adb2d529131190a657be6a", 58249454, 0),  // Linux v3.0.1
	GAME_ENTRY("geminirue", "gemini rue.exe", "0bdfb2139abaae5ae2dc85f1aacfd3f6", 61926695),
	DETECTION_ENTRY("geminirue", "gemini rue.exe", "0bdfb2139abaae5ae2dc85f1aacfd3f6", 64650419, Common::DE_DEU, nullptr, ADGF_NO_FLAGS, GAMEFLAG_FORCE_AA), // German retail
	GAME_ENTRY_STEAM("geminirue", "gemini_rue_pc.exe", "83362d0d2c1d4909bfbd85c04c95bde2", 72960932),
	GAME_ENTRY_STEAM("geminirue", "gemini rue.exe", "5f747ea1c5f624d3716926fe1ca5135d", 73541625), // Win v2.2
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("geminirue", "gemini rue.exe", "3b7e8fdf626c2f8e6538669cdab91c0e", 61437936, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win v3.0
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("geminirue", "ac2game.dat", "ab6c0c2d89adb2d529131190a657be6a", 58249180, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("geminirue", "agsgame.dat", "f3c0c7d3892bdd8963e8ce017f73de08", 62059297, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("geminirue", "Gemini Rue.ags", "ab6c0c2d89adb2d529131190a657be6a", 58249180, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  //Linux v3.0
	GAME_ENTRY("geminirue", "gemini_rue_pc.exe", "e8f1d07a6b363e9cc80dac5367f1b4ba", 72860463), // Humble Bundle
	GAME_ENTRY("geminirue", "ac2game.dat", "f3c0c7d3892bdd8963e8ce017f73de08", 62852566), // Android
	GAME_ENTRY("geminirue", "ac2game.dat", "e8f1d07a6b363e9cc80dac5367f1b4ba", 72836785), // MacOS, Humble Bundle
	GAME_ENTRY_EN_STEAM("ghostdream", "ghostdream.exe", "05594881531d62e4575545f3c8fd2576", 480856325),
	GAME_ENTRY_EN_STEAM("ghostman", "Rebut And Ghostman.exe", "0241777c2537fc5d077c05cde10bfa9f", 10312749),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("gigant", "Gigant.exe", "96abeef1dfc1bf2317d4b2f998e8a459", 77692758, 0),  // Eng-Kor
	// Original release 2023-05-02
	GAME_ENTRY("gobliiins5-1", "Gobliiins5-Part1.exe", "t:684f59952b168c26b45556d1560bb590", 174840675),
	GAME_ENTRY("gobliiins5-2", "Gobliiins5-Part2.exe", "t:5f753c3b1df583397cf016d3bcf4bf64", 144263518),
	GAME_ENTRY("gobliiins5-3", "Gobliiins5-Part3.exe", "t:a6d99497f108311308e812b5ba904d95", 176338687),
	GAME_ENTRY("gobliiins5-4", "Gobliiins5-Part4.exe", "t:1f2401479d4104539d54a4d11a713044", 352399933),
	// Bugfix release  2023-05-08
	GAME_ENTRY("gobliiins5-1", "Gobliiins5-Part1.exe", "t:684f59952b168c26b45556d1560bb590", 174840596),
	GAME_ENTRY("gobliiins5-2", "Gobliiins5-Part2.exe", "t:5f753c3b1df583397cf016d3bcf4bf64", 144263538),
	GAME_ENTRY("gobliiins5-3", "Gobliiins5-Part3.exe", "t:a6d99497f108311308e812b5ba904d95", 176341869),
	GAME_ENTRY("gobliiins5-4", "Gobliiins5-Part4.exe", "t:1f2401479d4104539d54a4d11a713044", 352414577),
	// Bugfix release  2023-05-14
	GAME_ENTRY("gobliiins5-3", "Gobliiins5-Part3.exe", "t:a6d99497f108311308e812b5ba904d95", 176342992),
	// Bugfix release  2023-07-10
	GAME_ENTRY("gobliiins5-1", "Gobliiins5-Part1.ags", "41513db1bd8870e43426b9e94bb26ad1", 171731727),
	GAME_ENTRY("gobliiins5-2", "Gobliiins5-Part2.ags", "9c1582a2901b8478b838f0b098fa7624", 141154654),
	GAME_ENTRY("gobliiins5-3", "Gobliiins5-Part3.ags", "b90879328ece625cb99b40bd447b4fef", 173235765),
	GAME_ENTRY("gobliiins5-4", "Gobliiins5-Part4.ags", "6cbe31cdb90c190465a340db7a748f57", 349305694),
	// Bugfix release 2023-11-18
	GAME_ENTRY("gobliiins5-1", "Gobliiins5-Part1.ags", "41513db1bd8870e43426b9e94bb26ad1", 171731573),
	GAME_ENTRY("gobliiins5-3", "Gobliiins5-Part3.ags", "b90879328ece625cb99b40bd447b4fef", 173236111),
	// Steam release 2023-07-20
	GAME_ENTRY_STEAM("gobliiins5-1", "Gobliiins5-Part1.ags", "41513db1bd8870e43426b9e94bb26ad1", 171800863),
	// Steam release 2023-11-16
	GAME_ENTRY_STEAM("gobliiins5-1", "Gobliiins5-Part1.ags", "41513db1bd8870e43426b9e94bb26ad1", 171800709),
	GAME_ENTRY_EN_STEAM("goldenwake", "a-golden-wake.exe", "dbe281d93c914899886e77c09c3c65ec", 130844360), // Linux & Mac
	GAME_ENTRY_EN_STEAM("goldenwake", "a-golden-wake.exe", "e7080df9729aae2a0097b752198f3cca", 130846774),
	GAME_ENTRY_EN_STEAM("goldenwake", "a-golden-wake.exe", "409b71f7b73f30bef5098c6449505c1a", 130929070),  // Windows
	GAME_ENTRY_EN_GOG("goldenwake", "a-golden-wake.exe", "7056895c888d6cf3f0120d2edfeb7f7b", 130912596),
	GAME_ENTRY_EN_GOG("goldenwake", "a-golden-wake.exe", "418a9aaa044bfc5b7a79ba96fe679b60", 130844832),
	GAME_ENTRY_EN("goldenwake", "a-golden-wake.exe", "e80586fdc2db32f65658b235d8cbc159", 129814364), // Humble Bundle
	GAME_ENTRY_EN("goldenwake", "a-golden-wake.exe", "e80586fdc2db32f65658b235d8cbc159", 129814642),
	GAME_ENTRY_EN("goldenwake", "ac2game.dat", "bb3b4630c44a51e3a23892563121c2c8", 128018480), // Linux Humble Bundle
	GAME_ENTRY_EN("goldenwake", "ac2game.dat", "d27edc5b7eee382501dfcfea044dfc90", 128018546),
	GAME_ENTRY_EN_STEAM("graceward", "Graceward.exe", "0564de07d3fd5c16e6947a647061913c", 292555926),
	GAME_ENTRY_EN_STEAM("graceward", "Graceward.exe", "0564de07d3fd5c16e6947a647061913c", 292551446), // v1.3
	GAME_ENTRY_STEAM("grandmabadass", "GrandmaBadass-bundle1.exe", "a66d7de0e58d3f3a09522745c4d523d9", 739736114),  // En-Fr-De-Es-It
	GAME_ENTRY_STEAM("grandmabadass", "Grandma Badass.exe", "a678098aa762ed53476e245d5e4ad7b4", 2125449858),  // En-Fr-De-Es-It
	GAME_ENTRY_STEAM("guardduty", "guardduty.exe", "98d09f79129a5f96b6622661e2a7bc64", 706020335),  // En-Fr-De-Es
	GAME_ENTRY_GOG("guardduty", "guardduty.exe", "98d09f79129a5f96b6622661e2a7bc64", 706020743),
	GAME_ENTRY_PLATFORM("guardduty", "AGSProject.ags", "a7745eea8e5b81209688b0e57922deb9", 703433226, "Switch"), // Nintendo Switch
	GAME_ENTRY_EN_STEAM("hauntingcliffhouse", "Cliffhouse.exe", "615e73fc1874e92d60a1996c2330ea36", 452123357),
	GAME_ENTRY_EN_STEAM("herdiscoming", "herd.exe", "78dd4ca028ee0156b6a093d6d780aa65", 7321217),
	GAME_ENTRY_STEAM("insectophobiaep1", "insect.exe", "b142b43c146c25443a1d155d441a6a81", 16950265),  // Multilang
	GAME_ENTRY_EN("jeffreyspaceact1", "Jeffrey In Space - Act I.exe", "b333d52330cd0c544cec0d7d8064b547", 59811630),  // itch.io
	DETECTION_ENTRY_GUIO("jorry", "JORRY.000", "811e23ba3aa030f039b2264ee239e2b4", 7731041, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // En-Fr
	GAME_ENTRY_STEAM("justignorethem", "justignorethem.exe", "182e930ace9d62a22d09779cac72e597", 94918926),  // Multilang
	GAME_ENTRY_PLATFORM("justignorethem", "justignorethemremake.exe", "7a0b9391c01ad13e5107b7763084a993", 187186542, "Remake/Steam"),
	GAME_ENTRY_PLATFORM("justignorethem", "justignorethemremake.exe", "182e930ace9d62a22d09779cac72e597", 94656598, "Rollback/Steam"),
	GAME_ENTRY_STEAM("justignorethembrea1", "Brea Story.exe", "475fc0e2d0e912c22b7cc28972e04496", 51014959),
	GAME_ENTRY_STEAM("justignorethembrea1", "Brea Story.exe", "475fc0e2d0e912c22b7cc28972e04496", 51016264),  // Multilang
	GAME_ENTRY_STEAM("justignorethembrea1", "Brea Story.exe", "475fc0e2d0e912c22b7cc28972e04496", 51016109),  // Multilang Dev
	GAME_ENTRY_STEAM("justignorethembrea1", "Brea Story.exe", "86f68403cfcc2efc8e3498f15ffaddf6", 50964953),  // Multilang Savefix
	DETECTION_ENTRY_GUIO("kathyrain", "kathyrain.exe", "434e24a12ba3cfb07d7b4b2f0e0bb1bf", 197487159, Common::UNK_LANG , "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),  // Multilang
	DETECTION_ENTRY_GUIO("kathyrain", "kathyrain.exe", "d2b1ba151c3d209b50331127032f96f6", 197489719, Common::UNK_LANG , "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),
	DETECTION_ENTRY_GUIO("kathyrain", "kathyrain.exe", "4bec9449bb53b0eea78cd5818b870f43", 197485875, Common::UNK_LANG , "Amazon Games", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),
	GAME_ENTRY_STEAM("killyourself", "ky.exe", "4e0d0aca04563b21a47e8a0cc56605fc", 18567658),  // En-De-Es-It-Nl
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("lamplightcity", "lamplight city.exe", "0cb636e2235e2b7cff4ece5898653e18", 981897178, 0), // Version 1.10  En-De
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("lamplightcity", "lamplight city.ags", "70a52d0ba7e5975df40aef460142f770", 979393622, 0), // Linux
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("lamplightcity", "lamplight city.exe", "4b18403fd1d150321f50cff9232d7b28", 982468239, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("lamplightcity", "lamplight city.ags", "69936bd2263616bb0bd64fa6b4fe5a8d", 979409133, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("lamplightcity", "ac2game.dat", "0cb636e2235e2b7cff4ece5898653e18", 981897178, 0), // Mac
	GAME_ENTRY_EN_STEAM("lancelothangover", "Lancelot.exe", "72df0ed89f7359b1466b0ea51f1f8ed4", 22506826),
	GAME_ENTRY("larrylotter", "Warthogs.exe", "9c49b6fa0460f36d6e7558281f142683", 12448793),  // Multilang
	GAME_ENTRY("larrylotter", "warthogs.exe", "cbe62572ed082a3063a49d594612ac0b", 12397280), // v1.7
	GAME_ENTRY_STEAM("larrylotter", "larry lotter.ags", "2802b69370de199a1f8d47889a197c14", 47202232),
	GAME_ENTRY_STEAM("larrylotter", "larry lotter.ags", "04d8c81bd74d2bfb91f08500b3bb63bb", 56911916),
	GAME_ENTRY_EN_STEAM("lastdayadolf", "thelastdayofadolf.ags", "6ed2d17ac0fcdb9da877daa89d4c9d30", 24559188),
	GAME_ENTRY_EN_STEAM("lastpirateadventure", "AGSGames.exe", "7971a7c02d414dc8cb33b6ec36080b91", 687320039),
	GAME_ENTRY("leewardep1", "LeeWard.exe", "886b68d435856de8fe38b65cabf38273", 1885509300),  // itch.io Eng-Esp
	GAME_ENTRY_EN_STEAM("legendofhand", "legend of hand.exe", "fc478dd7564c908615c9366398d995c8", 75797285),
	GAME_ENTRY_EN("legendofhand", "legend of hand.exe", "fc478dd7564c908615c9366398d995c8", 75797155),
	GAME_ENTRY_EN("legendofhand", "legend of hand.exe", "e07a475bcf14bc75c016724186f222ac", 75971657),  // itch.io v2.01
	GAME_ENTRY_GOG("legendofskye", "LegendSkye.exe", "8d1ff95c16500befbdc72260d461d73f", 72093654),  // Win 1.3.4
	GAME_ENTRY_GOG("legendofskye", "LegendSkye.ags", "d5316b4bf0f8620e9851ab333480cee9", 68984770),  // Linux 1.3.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("mage", "ac2game.dat", "2e822f554994f36e0c62da2acda874da", 30492258, 0), // Mac
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("mage", "mages_initiation.exe", "2e822f554994f36e0c62da2acda874da", 30492089, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("mage", "mages_initiation.exe", "2e822f554994f36e0c62da2acda874da", 30492087, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("mage", "ac2game.dat", "2e822f554994f36e0c62da2acda874da", 30492087, 0), // Mac
	GAME_ENTRY_EN_STEAM("maggieapartment", "maggie.exe", "60619c2e5e8d4dac3eda3f932bd7aad8", 1244635713),
	GAME_ENTRY_EN_STEAM("metaldead", "metaldead.exe", "91996c5379e82787b68e84ab67d9672f", 1504754877),
	GAME_ENTRY_EN("metaldead", "metaldead.exe", "3b0874c12fa643efa5fcdc43f647c871", 1501335400),
	GAME_ENTRY_STEAM("mountainsofmadness", "MountainsOfMadness.exe", "95822b7957b8f239d3216b9209cfb1f5", 337894184),  // En-De-It
	GAME_ENTRY_STEAM("mountainsofmadness", "MountainsOfMadness.exe", "95822b7957b8f239d3216b9209cfb1f5", 337897388),  // En-De-Es-It
	DETECTION_ENTRY_GUIO("mybigsister", "My Big Sister.exe", "4dc50ca8b5d2c762fe86a528b09973cf", 147257266, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // En-Fr-De-Es-Pt
	DETECTION_ENTRY_GUIO("mybigsister", "My Big Sister.exe", "4dc50ca8b5d2c762fe86a528b09973cf", 147082912, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // rollback
	DETECTION_ENTRY_GUIO("mybigsister", "My Big Sister.exe", "4dc50ca8b5d2c762fe86a528b09973cf", 147248623, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // rollback - latest
	GAME_ENTRY_PLATFORM("mybigsister", "AGSProject.ags", "5dccc941089cedbb7d84cc890528b5e4", 144835477, "Switch"),  // Nintendo Switch
	DETECTION_ENTRY_GUIO("mybigsisterrm", "MBSRemastered.exe", "e08c78e2591f196a2f42e5911062e56e", 156797059, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("mybigsisterrm", "MBSRemastered.exe", "e08c78e2591f196a2f42e5911062e56e", 157043019, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // Multilang (rollback 0.1)
	DETECTION_ENTRY_GUIO("mybigsisterrm", "MBSRemastered.exe", "e08c78e2591f196a2f42e5911062e56e", 157045932, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // rollback 0.2
	DETECTION_ENTRY_GUIO("mybigsisterrm", "MBSRemastered.exe", "e08c78e2591f196a2f42e5911062e56e", 157046186, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("mybigsisterrm", "MBSRemastered.exe", "e08c78e2591f196a2f42e5911062e56e", 157048203, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("mybigsisterrm", "MBSRemastered.exe", "e08c78e2591f196a2f42e5911062e56e", 157056576, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	GAME_ENTRY_STEAM("nellycootalot-hd", "Nelly.exe", "521aecdb5343c8d8f1c1000c4c2fa468", 11069524),  // Win En-Fr-De-Es-Pl
	GAME_ENTRY_STEAM("nellycootalot-hd", "Nelly.exe", "28f22ae60e2f3524665c1d5be60a0bf3", 11069220),
	GAME_ENTRY_STEAM("nellycootalot-hd", "Nelly.exe", "ad0aa2a72c107a9782c4ca3358d0db0f", 11032356),
	GAME_ENTRY_STEAM("nellycootalot-hd", "Nelly.ags", "bc290b10ac1d90b127eaa5cbe098bd31", 8573092),  // Linux En-Fr-De-Es-Pl
	GAME_ENTRY_STEAM("nellycootalot-hd", "ac2game.dat", "914d76b051867892f78883a2ff6be6ea", 11060916),  // Mac En-Fr-De-Es-Pl
	GAME_ENTRY_EN_STEAM("neofeud", "neofeud.exe", "6e861b1f476ff7cdf036082abb271329", 2078740517),
	GAME_ENTRY_EN("neofeud", "neofeud.exe", "6e861b1f476ff7cdf036082abb271329", 2078743375),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("nightmareframes", "Nightmare Frames.exe", "aec3371f8e8572efe5b0f601fff7913e", 2697791318, 0),  // Eng-Esp 1.0
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("nightmareframes", "Nightmare Frames.exe", "55a84a70f80cba8dd2373ea04c2b04ce", 2697804016, 0),
	DETECTION_ENTRY_GUIO("nightmareframes", "Nightmare Frames.exe", "55a84a70f80cba8dd2373ea04c2b04ce", 2697804015, Common::UNK_LANG, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, 0), // itch.io Eng-Esp
	GAME_ENTRY("odissea", "ODISSEA.exe", "5bb86de9245242046c309711ff02dad3", 717438956),  // itch.io  Eng-Ita
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("oott", "oott-tkc.exe", "11c2421258465cba4bd773c49d918ee3", 467834855, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("oott", "oott-tkc.exe", "11c2421258465cba4bd773c49d918ee3", 467845009, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("oott", "agsgame.dat", "ca8414acc5dc5687891db2d8ac5a0e76", 465662744, 0), // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("oott", "oott-tkc.exe", "11c2421258465cba4bd773c49d918ee3", 467844896, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("oott", "agsgame.dat", "801b35fe313a432e44671ab04178b679", 465670839, 0), // Linux
	GAME_ENTRY("ozorwell1", "OZORWELL.exe", "7036a7f21cfe585bc890093b4e004d52", 588631118), // itch.io  Eng-Ita
	GAME_ENTRY("ozorwell2", "OZ2.exe", "99a9a592b2ffe51a93b5f18b927441b0", 1028373987), // itch.io  Eng-Ita
	GAME_ENTRY_EN_STEAM("perfecttides", "perfecttides.exe", "63f8a60cc094996bd070e68cb3c4722c", 1201068548),  // Win v1.0.2
	GAME_ENTRY_EN_STEAM("perfecttides", "perfecttides.exe", "7a2ea9a4824badcacb5c779ff98b380f", 1243795250),  // Win v2.0
	GAME_ENTRY_EN_STEAM("perfecttides", "perfecttides.exe", "7a2ea9a4824badcacb5c779ff98b380f", 1243797801),  // Win v2.0.1
	GAME_ENTRY_EN_STEAM("perfecttides", "perfecttides.exe", "ccae5c3eb855450efe79830f4fa2855f", 1243799719),  // Win v2.0.1 hotfix
	GAME_ENTRY_EN_STEAM("perfecttides", "perfecttides.exe", "ccae5c3eb855450efe79830f4fa2855f", 1243799760),  // Win v2.0.1 28/11/23 update
	GAME_ENTRY_EN_STEAM("perfecttides", "game.ags", "63f8a60cc094996bd070e68cb3c4722c", 1201066930),  // Mac
	GAME_ENTRY_EN_STEAM("perfecttides", "game.ags", "63f8a60cc094996bd070e68cb3c4722c", 1201068548),  // Mac v1.0.2
	GAME_ENTRY_EN_STEAM("perfecttides", "game.ags", "163f1abe335f3c3eb8bc68131a2ba6c6", 1240690225),  // Mac v2.0.1
	GAME_ENTRY("perfidiouspetrolstation", "NancyATPPS.exe", "be858e83311d4f1b49e05b933b92f0a9", 28085287),  // Eng-Deu
	GAME_ENTRY_EN_STEAM("personalrocket", "PersonalRocket.exe", "7db052bc30700d1f30f5330f5814f519", 76071166),
	GAME_ENTRY_EN_STEAM("phoenixtales", "phoenixtales1.exe", "4e8d400018aa356e0e8a09dff6e4f4e2", 3922955),
	GAME_ENTRY_EN("piratethemepark", "Pirate Theme Park.exe", "f7308b375ff8aebe23e909add49dbd4d", 31368047),  // itch.io
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.exe", "22313e59c3233001488c26f18c80cc08", 973495830, 0),
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.exe", "22313e59c3233001488c26f18c80cc08", 973511911, 0),
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.exe", "f2edc9c3161f1f538df9b4c59fc89e24", 978377890, 0), // En-Fr-De-Es
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "Primordia.exe", "8f717a5a14ceda815292ce4065348afb", 979602678, 0), // v2.0.0.2, with Ita translation
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 979603175, 0), // v2.6
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 979604026, 0), // v3
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 981119316, 0), // v3.0a
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341456, 0), // v3.5a
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341644, 0), // v3.5b
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341856, 0), // v3.7
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866342835, 0), // v3.8
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866344821, 0), // v3.9
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866348102, 0), // v4.0
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "agsgame.dat", "22313e59c3233001488c26f18c80cc08", 973495830, 0), // Linux
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "ac2game.dat", "7755b4df8706de91f31ddeee98bd20e9", 976474850, 0), // Mac
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "ac2game.dat", "7755b4df8706de91f31ddeee98bd20e9", 976475347, 0), // Mac v2.7a
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "ac2game.dat", "7755b4df8706de91f31ddeee98bd20e9", 976476198, 0), // Mac v3
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 977991488, 0), // Mac v3.0a
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866341456, 0), // Mac v3.5a
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866342835, 0), // Mac v3.9
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.exe", "f2edc9c3161f1f538df9b4c59fc89e24", 978377182, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 979602678, 0),  // with Ita translation
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 979603766, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 981119056, 0),  // Hun-Tur translation release
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341196, 0),  // v3.6
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341384, 0),  // v3.6 updated
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341392, 0),  // v3.6 updated2
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866341604, 0),  // v3.7
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866342583, 0),  // v3.8
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866344569, 0),  // v3.9
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "primordia.ags", "fe4f46407ac736cf0e6badf96cef4ee5", 866347850, 0),  // v4.0
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "ac2game.dat", "7755b4df8706de91f31ddeee98bd20e9", 976475938, 0),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866341384, 0),  // Mac v3.6 updated
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866341392, 0),  // Mac v3.6 updated2
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866341604, 0),  // Mac v3.7
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866342583, 0),  // Mac v3.8
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("primordia", "ac2game.dat", "fe4f46407ac736cf0e6badf96cef4ee5", 866344569, 0),  // Mac v3.9
	GAME_ENTRY("primordia", "primordia.exe", "22313e59c3233001488c26f18c80cc08", 973154021), // DVD version
	GAME_ENTRY("primordia", "primordia.exe", "8f717a5a14ceda815292ce4065348afb", 978722743),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("projectorface", "Projector Face.exe", "d26528c4933649f147c6c4338bcb9e0a", 42610502, 0),  // En-De-It
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("puzzlebots", "ac2game.dat", "34b49df9cf6eadb5c3587b3921d5b72f", 787776664, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("qfi", "qfi.exe", "0702df6e67ef87fd3c51d09303803126", 534847265, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("qfi", "qfi.exe", "32b36aebe0729c9360bc10dcddc0653c", 538562096, 0),  // v1.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("qfi", "agsgame.dat", "2a0ba29e479ca5aa8c6b4233f030e78f", 536391599, 0),  // Linux v1.1.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("qfi", "qfi.exe", "32b36aebe0729c9360bc10dcddc0653c", 538681872, 0),  // v1.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("qfi", "agsgame.dat", "2a0ba29e479ca5aa8c6b4233f030e78f", 536513114, 0), // Linux
	GAME_ENTRY_EN("qfi", "qfi.exe", "32b36aebe0729c9360bc10dcddc0653c", 538547262),
	DETECTION_ENTRY_GUIO("qfi", "qfi.exe", "946f453df05714c65256ad315dc6cc37", 538574225, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, 0), // itch.io 1.2
	GAME_ENTRY_EN("qfi", "agsgame.dat", "9efb669ada4b685f9a5a91ce161424a7", 532694795), // Linux Groupees
	GAME_ENTRY_EN("qfi", "qfi.exe", "8a7bc4f9ef0e6b0a5b01d3e4d5d5ee7d", 534881961),
	GAME_ENTRY_EN("qfi", "qfi.exe", "32b36aebe0729c9360bc10dcddc0653c", 538547259),
	GAME_ENTRY_EN_PLATFORM("qfi", "AGSProject.ags", "80ba5f7583f86e45dabfb0c49cb60771", 531571799, "Switch"),  // Nintendo Switch
	GAME_ENTRY("quantumnauts", "QUANTUMNAUTS.exe", "78b3f99175ce10a60d70b7c70ab560af", 460832399),  // itch.io Eng-Ita
	GAME_ENTRY("quantumnauts2", "QN2.exe", "b00c1257945b1e553776b564dd03cb8d", 740268419),  // itch.io Eng-Ita
	DETECTION_ENTRY_GUIO("redbow", "Red Bow.exe", "eefa4092bead6512f40e60ce1e20b1e0", 41826704, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // Multilanguage
	DETECTION_ENTRY_GUIO("redbow", "Red Bow.exe", "eefa4092bead6512f40e60ce1e20b1e0", 41818778, Common::UNK_LANG, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // rollback
	DETECTION_ENTRY_GUIO("redbow", "AGSProject.ags", "fb399121adc843b2aea24728349ff32a", 49109581, Common::UNK_LANG, "Switch", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // Nintendo Switch
	GAME_ENTRY_EN_GOG("resonance", "resonance.exe", "2e635c22bcbf0ed3d46f1bcde71812d4", 849404957),
	GAME_ENTRY_EN_GOG("resonance", "resonance.exe", "6cc23ce7cdf62de776c6b8ddb6b8a7ff", 850642889),  // v2.2
	GAME_ENTRY_EN_GOG("resonance", "resonance.exe", "6cc23ce7cdf62de776c6b8ddb6b8a7ff", 850643390),  // v2.3
	GAME_ENTRY_EN_GOG("resonance", "resonance.exe", "b6ce764bf83d025c7c0aa50ceb006e09", 732933098),  // v3.0
	GAME_ENTRY_EN_GOG("resonance", "ac2game.dat", "1dd9f78fbd1947fdf66a820928baf8a4", 847433642), // Mac v2.3
	GAME_ENTRY_EN_GOG("resonance", "ac2game.dat", "920f99ac1e32f07ccde5c960aac52d8c", 729663446), // Mac v3.0
	GAME_ENTRY_EN_GOG("resonance", "agsgame.dat", "2e635c22bcbf0ed3d46f1bcde71812d4", 849404957), // Linux
	GAME_ENTRY_EN_GOG("resonance", "resonance.ags", "920f99ac1e32f07ccde5c960aac52d8c", 729663446), // Linux 3.0
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "resonance.exe", "2e635c22bcbf0ed3d46f1bcde71812d4", 849410915, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win original
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "resonance.exe", "6cc23ce7cdf62de776c6b8ddb6b8a7ff", 850642889, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win-Linux
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "resonance.exe", "b6ce764bf83d025c7c0aa50ceb006e09", 732932834, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win 3.0
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "resonance.exe", "b6ce764bf83d025c7c0aa50ceb006e09", 732932842, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win 3.0 updated
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "resonance.ags", "920f99ac1e32f07ccde5c960aac52d8c", 729663182, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux 3.0
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "ac2game.dat", "1dd9f78fbd1947fdf66a820928baf8a4", 847433141, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN("resonance", "ac2game.dat", "920f99ac1e32f07ccde5c960aac52d8c", 729663182, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac 3.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("richardandalice", "richardandalice.exe", "1023d348f67ba958f78ed62d029565c7", 109164768, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("richardandalice", "richardandalice.exe", "f47d11397e2059a25b11057ea6cf3409", 108264256, 0),
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("richardandalice", "richardandalice.exe", "9a7e38a1810e6a1839601fb5c6bd42e6", 108602238, 0), // Desura - GOG v1.0
	GAME_ENTRY_EN_STEAM("rnrneverdies", "Rock 'n' Roll Will Never Die.exe", "28456c6a3a38874b833651c4433e22b4", 37722884),
	GAME_ENTRY_EN_STEAM("rnrneverdies", "Rock 'n' Roll Will Never Die.exe", "28456c6a3a38874b833651c4433e22b4", 37725830),
	GAME_ENTRY_EN_STEAM("rnrneverdies", "Rock 'n' Roll Will Never Die.exe", "28456c6a3a38874b833651c4433e22b4", 37729146),
	DETECTION_ENTRY_GUIO("roguestate", "roguestate.exe", "981a1c4d5a64b8ebe300f9769acf0fe5", 895486573, Common::EN_ANY, "Steam", GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	GAME_ENTRY_GOG("samaritan", "samaritan.exe", "79dd677433bbb10d4970367eb68b243c", 78070237), // Groupees, v1.1 - GOG
	GAME_ENTRY_STEAM("samaritan", "samaritan.exe", "5fb404a4a7c9cd1449dfdd3d23fa0250", 78753952),  // En-Fr-De-Es-It
	GAME_ENTRY_STEAM("samaritan", "samaritan.exe", "6a3b669b7d34ec9356a665afe0c2c003", 78438375),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "ee801fba52d252249677a9170bd2db96", 642157876, 0),  // v1.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "cc447656f408ac32e8443274c3f70c67", 643459667, 0),  // v2.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "cc447656f408ac32e8443274c3f70c67", 643461648, 0),  // v2.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391409822, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391409830, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391410092, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391410784, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "Shardlight.ags", "df94d3c1cec961c89894c676b000e7bc", 388236426, 0),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "Shardlight.ags", "df94d3c1cec961c89894c676b000e7bc", 388236696, 0),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "Shardlight.ags", "df94d3c1cec961c89894c676b000e7bc", 388237388, 0),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 640348156, 0),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 388236426, 0),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 388236696, 0),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 388237388, 0),  // Mac
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "cc447656f408ac32e8443274c3f70c67", 643461930, 0), // Win v2.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391410104, 0), // Win v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391410366, 0), // Win v3.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "shardlight.exe", "400682c45fb6c0754ef545b96e7efc1d", 391411058, 0), // Win v3.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 640346706, 0), // Mac v2.3a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 640348438, 0), // Mac v2.4
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 388236708, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 388236970, 0), // Mac v3.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "ac2game.dat", "df94d3c1cec961c89894c676b000e7bc", 388237662, 0), // Mac v3.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "Shardlight.ags", "df94d3c1cec961c89894c676b000e7bc", 388236708, 0), // Linux v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "Shardlight.ags", "df94d3c1cec961c89894c676b000e7bc", 388236970, 0), // Linux v3.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shardlight", "Shardlight.ags", "df94d3c1cec961c89894c676b000e7bc", 388237662, 0), // Linux v3.2
	GAME_ENTRY_EN("shardlight", "shardlight.exe", "ee801fba52d252249677a9170bd2db96", 642151325),
	GAME_ENTRY_EN("shardlight", "shardlight.exe", "ee801fba52d252249677a9170bd2db96", 642142591),
	GAME_ENTRY_EN("shivah", "shivah.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 19542815), // Official website
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "0aaf5445a3544a631d6e7dd4561fc7ae", 32323040, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win Orig
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "adf5d1e8de8b1292e98ef59ce89ab42c", 33845144, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win/Linux v2.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "b4b91ed6682c0aa7f1c78dbb6373f7eb", 21135800, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win v3.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "b4b91ed6682c0aa7f1c78dbb6373f7eb", 21135808, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Win v3.0 updated
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "ac2game.dat", "bdc618b23fc279938f05f8ac058a938b", 32727114, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac Orig
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "ac2game.dat", "2ffd6c204c78d9c5f7b22b5469f8f0e7", 30657924, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "ac2game.dat", "4f21d15d7bba261823d078cf0418d04f", 17889188, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "ac2game.dat", "0aaf5445a3544a631d6e7dd4561fc7ae", 32323040, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux Orig
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("shivahkosher", "Shivah.ags", "4f21d15d7bba261823d078cf0418d04f", 17889188, GAMEFLAG_PLUGINS_AGSTEAM_WADJETEYE),  // Linux v3.0
	GAME_ENTRY_EN("shivahkosher", "ac2game.dat", "0aaf5445a3544a631d6e7dd4561fc7ae", 32339699), // Android
	GAME_ENTRY_EN("shivahkosher", "ac2game.dat", "ec0019b528dd1e9bcb4264967c4a3a31", 30446404), // Linux Humble Bundle
	GAME_ENTRY_EN("shivahkosher", "ac2game.dat", "bdc618b23fc279938f05f8ac058a938b", 32723739), // MacOS
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "0aaf5445a3544a631d6e7dd4561fc7ae", 32319665, 0), // Windows GOG, Humble Bundle
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "adf5d1e8de8b1292e98ef59ce89ab42c", 33845596, 0), // Windows v2.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shivahkosher", "shivah.exe", "b4b91ed6682c0aa7f1c78dbb6373f7eb", 21136003, 0), // Windows v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shivahkosher", "ac2game.dat", "021b85e94ba52ed77b6b4841ffb50c43", 30658376, 0), // macOS v2.1a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shivahkosher", "ac2game.dat", "75d706c07c1e8beb688a4968d1b8e6a4", 17889391, 0), // macOS v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("shivahkosher", "Shivah.ags", "75d706c07c1e8beb688a4968d1b8e6a4", 17889391, 0), // Linux v3.0
	GAME_ENTRY_EN_PLATFORM("shoaly", "SYCBS.exe", "fae163b58c16e194688727d0903684be", 4302275, "Deluxe"),  // Windows
	GAME_ENTRY_EN_PLATFORM("shoaly", "SYCBS.ags", "9ad5572708bd5b2077bf227494fd2679", 1198511, "Deluxe"),  // Linux
	GAME_ENTRY_EN("sisterssecret", "SistersSecret.exe", "f8d06c339ca2af5160b662c7a19572d6", 18659398),  // v1.0.0a
	GAME_ENTRY_EN("sisterssecret", "SistersSecret.exe", "f8d06c339ca2af5160b662c7a19572d6", 18660074),  // v1.0.1
	GAME_ENTRY_STEAM("smallsister", "sister.exe", "78dd4ca028ee0156b6a093d6d780aa65", 18534526),  // Eng-Tur
	GAME_ENTRY_EN_STEAM("snowproblem", "Snow Problem.exe", "6689ccca6f9b2c8398352e9b772ff411", 9919914),
	GAME_ENTRY_EN_STEAM("snowproblem", "Snow Problem.exe", "fae163b58c16e194688727d0903684be", 9839621),
	GAME_ENTRY_EN_STEAM("spaceraven", "Tiny Planet.exe", "c6154d27a773ebf98a9be181a840d146", 29803571),
	GAME_ENTRY_STEAM("starshipquasar", "starship quasar.ags", "0ea8150e15a4f4d0f3e82c231a4659f2", 10415956),  // Multilang
	GAME_ENTRY_STEAM("starshipquasar", "starship quasar.ags", "0ea8150e15a4f4d0f3e82c231a4659f2", 10415936),  // Multilang
	GAME_ENTRY_STEAM("starshipquasar", "starship quasar.ags", "86e0e39c86afe0edd4fba90eea19d85e", 21972705),  // Multilang
	GAME_ENTRY("starshipquasar", "Starship Quasar.exe", "0ea8150e15a4f4d0f3e82c231a4659f2", 10415936),  // Multilang itch.io
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("stayingalive", "Staying Alive.exe", "fae163b58c16e194688727d0903684be", 17380838, 0),
	GAME_ENTRY_STEAM("stellarmessep1", "StellarMessTPC.exe", "a409703089eebbcfa13f0a22f6fb71ed", 18068440),  // Eng-Esp
	GAME_ENTRY_STEAM("stellarmessep1", "StellarMessTPC.exe", "a409703089eebbcfa13f0a22f6fb71ed", 18077040),  // Eng-Esp v1.4
	DETECTION_ENTRY_GUIO("strangeland", "Strangeland.exe", "c5978d50a9b7ee1c8a50a731847d8504", 3186614879, Common::EN_ANY, nullptr, GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("strangeland", "Strangeland.exe", "c5978d50a9b7ee1c8a50a731847d8504", 3186488616, Common::EN_ANY, nullptr, GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "151b0391a650d3c81c2758deaa5852cd", 3183277137, Common::EN_ANY, nullptr, GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("strangeland", "Strangeland.exe", "c5978d50a9b7ee1c8a50a731847d8504", 3186989657, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Win v2.7
	DETECTION_ENTRY_GUIO("strangeland", "Strangeland.exe", "c5978d50a9b7ee1c8a50a731847d8504", 3187004936, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Win v3.0
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "511e08ad31c4020a9ca01b6f7ce1365f", 3009634736, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Mac
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "7e5abc5202098bd00ddef999854eb9ab", 3183368017, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Mac
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "e4b9f0aa26c1115143ad483c99bc9383", 3183880236, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Mac v2.5a
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "86e8b93b8231d6c571669f1621561a21", 3183886917, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Mac v2.7
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "c0d0009485795a4ff0cf9dbe5ad82a2f", 3183902196, Common::UNK_LANG, "GOG.com", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Mac v3?
	DETECTION_ENTRY_GUIO("strangeland", "Strangeland.exe", "c5978d50a9b7ee1c8a50a731847d8504", 3186428869, Common::UNK_LANG, "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Win 2.3
	DETECTION_ENTRY_GUIO("strangeland", "Strangeland.exe", "c5978d50a9b7ee1c8a50a731847d8504", 3187004654, Common::UNK_LANG, "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Win-Linux v3.0
	DETECTION_ENTRY_GUIO("strangeland", "ac2game.dat", "e037ab3414dec971b90715c7dbb56f4c", 3183901914, Common::UNK_LANG, "Steam", GUIO3(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0), // Mac
	GAME_ENTRY("sulifallenharmony", "Suli Fallen Harmony.exe", "8a58836830dce896e4366f57791ab6cc", 35900870),  // v1.4 itch.io En-Fr
	GAME_ENTRY_STEAM("sumatra", "sumatra fate of yandi.exe", "57c868b1a81c0335ab60970292cd79d8", 170088886),  // En-Fr-De-Es
	GAME_ENTRY_STEAM("sumatra", "sumatra fate of yandi.exe", "b4407ac542f316456f549fc8a60dccae", 170634789),
	GAME_ENTRY_GOG("sumatra", "sumatra fate of yandi.exe", "b4407ac542f316456f549fc8a60dccae", 170715680),  // v1.2
	GAME_ENTRY_PLATFORM("sumatra", "AGSProject.ags", "35c924b7e6751756f368dadeb3771f83", 167630075, "Switch"),  // Nintendo Switch
	DETECTION_ENTRY_GUIO("superegoch12", "Superego - Chapter 1+2 (ENG).exe", "faeaa85c173b559be75f55a3433e1fd2", 346120240, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),  // itch.io
	DETECTION_ENTRY_GUIO("superegoch12", "Superego - Cap. 1+2.exe", "2b5c2ecd5dbeafcd1591a6d743cf8adc", 346121328, Common::ES_ESP, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	DETECTION_ENTRY_GUIO("superegoch3", "Superego - Cap. 3.exe", "9e42b731efe7dceff095628372a173cc", 308682202, Common::ES_ESP, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, 0),
	GAME_ENTRY_EN("superjazzman", "sjm.exe", "0710e2ec71042617f565c01824f0cf3c", 10841689), // Official website
	GAME_ENTRY_LANG("symploke1", "Symploke.exe", "ff20c8c9dda8379607db87e7726909c6", 29996616, Common::ES_ESP), // Videojuegos Fermin website (Spanish)
	GAME_ENTRY_EN("symploke1", "symploke1.exe", "ff20c8c9dda8379607db87e7726909c6", 57363019), // Steam (English), same as Videojuegos Fermin website (English)
	GAME_ENTRY_LANG_PLATFORM("symploke1", "symploke1.exe", "ff20c8c9dda8379607db87e7726909c6", 29996469, Common::ES_ESP, "Steam"), // Steam (Spanish)
	GAME_ENTRY_EN_STEAM("symploke2", "symploke2.exe", "ff20c8c9dda8379607db87e7726909c6", 184247323), // Steam (English)
	GAME_ENTRY_LANG_PLATFORM("symploke2", "symploke2.exe", "ff20c8c9dda8379607db87e7726909c6", 124539110, Common::ES_ESP, "Steam"), // Steam (Spanish)
	GAME_ENTRY_EN_STEAM("symploke3", "symploke3.exe", "c7acc0fba53cb6a656c9cbc788868a2d", 284040264), // Steam (English)
	GAME_ENTRY_LANG_PLATFORM("symploke3", "symploke3.exe", "c7acc0fba53cb6a656c9cbc788868a2d", 284098408, Common::ES_ESP, "Steam"), // Steam (Spanish)
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("tales", "tales.exe", "7b5842e8954354ef6e21dc1837073182", 150969855, 0),  // En-Fr-De-Es-It-Ru
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "e523535e730f313e1ed9b19b1a9dd228", 447900356, 0),  // Win Orig
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448795857, 0),  // Win v2.2
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448823986, 0),  // Win v2.2 updated
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448823978, 0),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448825176, 0),  // Win v2.5
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448836470, 0),  // Win/Linux v2.5a
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "6475b8eb253b8d992a052869de4632cb", 420107350, 0),  // Win v3.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "6475b8eb253b8d992a052869de4632cb", 420114105, 0),  // Win v3.0.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "6475b8eb253b8d992a052869de4632cb", 420114113, 0),  // Win v3.0.1 updated
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "6475b8eb253b8d992a052869de4632cb", 420115055, 0),  // Win v3.0.5
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.ags", "9d48667020cf3e3612a753934b16cc04", 416922690, 0),  // Linux v3.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.ags", "9d48667020cf3e3612a753934b16cc04", 416929445, 0),  // Linux v3.0.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "technobabylon.ags", "9d48667020cf3e3612a753934b16cc04", 416930395, 0),  // Linux v3.0.5
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "570e69be27d3fa94b50f2779100e3fed", 445700420, 0),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "9d48667020cf3e3612a753934b16cc04", 416929445, 0),  // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "9d48667020cf3e3612a753934b16cc04", 416930395, 0),  // Mac v3.0.5
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448796372, 0), // Win v2.2
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448825442, 0), // Win v2.5
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "83cd1ad574bcfffe1b11504a32402b1e", 448836736, 0), // Win v2.5a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "6475b8eb253b8d992a052869de4632cb", 420107616, 0), // Win v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "f2332e54784086e5a2f249c1867897df", 420097475, 0), // Win v3.0.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.exe", "6475b8eb253b8d992a052869de4632cb", 420115313, 0), // Win v3.0.5
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "570e69be27d3fa94b50f2779100e3fed", 445671616, 0), // Mac v2.2a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "570e69be27d3fa94b50f2779100e3fed", 445711980, 0), // Mac v2.5a
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "9d48667020cf3e3612a753934b16cc04", 416922956, 0), // Mac v3.0
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "ac2game.dat", "9d48667020cf3e3612a753934b16cc04", 416929711, 0), // Mac v3.0.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.ags", "9d48667020cf3e3612a753934b16cc04", 416929711, 0), // Linux v3.0.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("technobabylon", "technobabylon.ags", "9d48667020cf3e3612a753934b16cc04", 416930653, 0), // Linux v3.0.5
	GAME_ENTRY_EN("technobabylon", "technobabylon.exe", "e523535e730f313e1ed9b19b1a9dd228", 447894701),  // Humble Bundle
	GAME_ENTRY_EN_STEAM("theadventuresoffatman", "fatman.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 44449205),
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "275e81b2ba532d12399b6329fa484ed4", 658932547), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "6cf383309d1e937d5e44b6e41789d3d0", 658933667), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "e75216cf3d23c452e2b0689d9fc78c56", 658936316), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "2122f502b77ac24c3e8ffdbdeaa5eae1", 658936978), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "1a8a63f2ced9c70755a7b615d85e31e5", 658942614), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "0b16e505197f0bfcc167dd828afac192", 658957465), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "0eeac94dd1aa801eb0e7dddd6a53ff36", 658959289), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "9bfaffc81b49093dda477098212245c1", 658963264), // En-Es
	GAME_ENTRY_STEAM("theadventuresoftheblackhawk", "The Adventures of The Black Hawk.ags", "675f43ff9490172433eb905fb3e53a0a", 658966635), // En-Es
	GAME_ENTRY_EN("theantidote", "The Antidote.exe", "4017d881af6467d6aad9183e6e38a261", 325819947),
	GAME_ENTRY_EN_STEAM("thebeardinthemirror", "the beard in the mirror.exe", "4d4b6d6e560cf32f440c39f8d3896da5", 40637501),
	GAME_ENTRY_STEAM("thecastle", "maniac.exe", "3b96c2143d927f005715be73344e456c", 22362337),  // En-Es-It
	GAME_ENTRY_STEAM("thecastle", "maniac.exe", "f39cba038b386d28fbcf869556c090c1", 23317965),  // En-De-Es-It
	GAME_ENTRY_STEAM("thecastle", "maniac.ags", "9d689b7feb29cd29bcb17ad5a3c94f3e", 20208057),  // En-De-Es-It
	GAME_ENTRY_STEAM("thecastle", "maniac.exe", "f39cba038b386d28fbcf869556c090c1", 23324413),  // En-De-Es-It / Improved (De) translation
	GAME_ENTRY_STEAM("thecastle", "maniac.ags", "a61420fa39d9772a0d43b41e5db31725", 20214505),  // En-De-Es-It / Improved (De) translation
	GAME_ENTRY_GOG("thecatlady", "thecatlady.exe", "0ea2b985dc5d7d27cb7c1b7da5eedea0", 1072316702),  // Multilang
	GAME_ENTRY_STEAM("thecatlady", "thecatlady.exe", "0ea2b985dc5d7d27cb7c1b7da5eedea0", 53988582),
	GAME_ENTRY("thecatlady", "Cat Lady.exe", "92320e20e3d4c70a94d89e2f797e65c1", 1061000756), // DVD
	GAME_ENTRY_EN("thesecretofhuttongrammarschoolvga", "The Secret of Hutton Grammar School VGA.ags", "8d699b21ab64b28c6b2c245c01c822fc", 64971355),
	GAME_ENTRY_EN_STEAM("thesecretofhuttongrammarschoolvga", "The Secret of Hutton Grammar School VGA.exe", "2321081ba51d1869ebe11b9ba126911b", 67463317),  // Steam Win
	GAME_ENTRY_STEAM("thesecretsofjesus", "the secrets of jesus.exe", "2e17ab52586ad34b5e597c20a59f60a0", 1577637692),  // En-De
	GAME_ENTRY_EN_STEAM("theterribleoldman", "The Terrible Old Man.exe", "b3a10d7d3b5a728330b028ec28ff2d05", 103852030),  //v1.2 MAGS port
	GAME_ENTRY_EN_STEAM("theterribleoldman", "The Terrible OId Man.exe", "e07a475bcf14bc75c016724186f222ac", 369769127),
	GAME_ENTRY_EN_STEAM("theterribleoldman", "The Terrible OId Man.exe", "b84c9741f4165953e46b5d9472f20dec", 370277066),
	GAME_ENTRY_EN_STEAM("timegentlemenplease", "tgp.exe", "86a5359bac7c88f0dfa060478800dd61", 74077811),
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "b1ff7d96667707daf4266975cea2bf90", 1755457364, 0), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "140570a663877cb81e3656b4f29c63f6", 1752922600, 0), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "140570a663877cb81e3656b4f29c63f6", 1752927680, 0), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "f311000c476689a6e77d25f002f412db", 1617818900, 0), // Mac
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "f311000c476689a6e77d25f002f412db", 1617819275, 0), // Mac v1.1.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "f311000c476689a6e77d25f002f412db", 1617819875, 0), // Mac v1.2.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "b1ff7d96667707daf4266975cea2bf90", 1755451248, 0), // Windows/Linux
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756036604, 0), // Windows/Linux v0.9.0 (GOG v1.2)
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756040045, 0), // Windows?/Linux v1.0.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756041684, 0), // Windows v1.0.2
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617816607, 0), // Windows/Linux v1.1.0
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617818775, 0), // Windows/Linux v1.1.0 updated
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617819275, 0), // Windows/Linux v1.1.1
	GAME_ENTRY_PLUGIN_STEAM_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617819875, 0), // Windows/Linux v1.2.1
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "b1ff7d96667707daf4266975cea2bf90", 1755451248, 0), // Windows/Linux
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756036604, 0), // Windows/Linux v0.9.0 (GOG v1.2)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756037170, 0), // Windows/Linux v0.9.0 (GOG v1.3)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756040362, 0), // Windows/Linux v1.0.0 (GOG v1.31)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756041833, 0), // Windows/Linux v1.0.1 (GOG v1.32)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "Unavowed.exe", "52c411caa3bfb65788ed8768ceaa0c30", 1756042001, 0), // Windows/Linux v1.0.2 (GOG v1.4)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617816924, 0), // Windows/Linux v1.1.0 (GOG v2.0)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617819092, 0), // Windows/Linux v1.1.0 updated (GOG v2.0.1)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617819592, 0), // Windows/Linux v1.1.1 (GOG v2.0.2)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "unavowed.ags", "f311000c476689a6e77d25f002f412db", 1617820192, 0), // Windows/Linux v1.2.1 (GOG v2.1)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "f311000c476689a6e77d25f002f412db", 1617819592, 0), // Mac v1.1.1 (GOG v2.0.2)
	GAME_ENTRY_PLUGIN_GOG_EN_NOAUTOSAVE("unavowed", "ac2game.dat", "f311000c476689a6e77d25f002f412db", 1617820192, 0), // Mac v1.2.1 (GOG v2.1)
	GAME_ENTRY_EN_STEAM("unlikelyprometheus", "The Unlikely Prometheus.exe", "a370a1db43a1d376d09e43469abba1d1", 129124983),
	GAME_ENTRY_EN_STEAM("untilihaveyou", "until i have you.exe", "cda1d7e36993dd55ba5513c1c43e5b2b", 1089857773), // Steam
	GAME_ENTRY_EN_STEAM("waitingfortheloop", "waitingfortheloop.exe", "0241777c2537fc5d077c05cde10bfa9f", 51472537),
	GAME_ENTRY_EN("waitingfortheloop", "waitingfortheloop.exe", "0241777c2537fc5d077c05cde10bfa9f", 51273604),
	GAME_ENTRY("welcometosunnymunarvagir", "alpha4.ags", "392dbdd0697ae32af4cfe5212f9213c5", 23000263),
	DETECTION_ENTRY_GUIO("whispersofamachine", "whispers.exe", "b4962a0a9c9c33954e185a137125f527", 159084291, Common::UNK_LANG, "Steam", GUIO4(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVE_THUMBNAIL, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),  // Multilang
	DETECTION_ENTRY_GUIO("whispersofamachine", "whispers.exe", "b8416ff5242d6540980f922f03a01a5f", 159085573, Common::UNK_LANG, "GOG.com", GUIO4(GUIO_NOLANG, GUIO_NOLAUNCHLOAD, GAMEOPTION_NO_SAVE_THUMBNAIL, GAMEOPTION_NO_SAVELOAD), ADGF_NO_FLAGS, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP),
	GAME_ENTRY_EN_STEAM("wolfterritory", "wolf.exe", "78dd4ca028ee0156b6a093d6d780aa65", 3957156),
	GAME_ENTRY_EN_STEAM("yetilastescape", "Big-run.exe", "1e003cdad70709b5bd3d0d957f637e58", 31372723),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100036465, 0),  // Win Eng-Pol
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100040004, 0),
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("zniwadventure", "ctgame.ags", "8f52737244a78a7d1e6acace8ced67a5", 97017764, 0),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100065720, 0),  // Win Eng-Pol
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100046802, 0),
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("zniwadventure", "ac2game.dat", "8a2d48ee8d92bad3c5cacd8b883c5871", 100067853, 0), // macOS
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100067853, 0), // 1.3.4 Win
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100067140, 0), // 1.3.4.1 Win
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("zniwadventure", "ctgame.ags", "8f52737244a78a7d1e6acace8ced67a5", 97019184, 0), // 1.3.4 Linux
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("zniwadventure", "ac2game.dat", "8a2d48ee8d92bad3c5cacd8b883c5871", 100067140, 0), // 1.3.4 macOS
	GAME_ENTRY("zniwadventure", "ctgame.exe", "8a2d48ee8d92bad3c5cacd8b883c5871", 100010235),
	// Commercial Italian games
	GAME_ENTRY_LANG("onironauta", "Onironauta.exe", "5c8c0e3edae9b4fad276c136b2b48ce8", 69917110, Common::IT_ITA),

	// AGDI games. They get their own grouping because they're just that awesome
	PRE_25_ENTRY_EN("kq1agdi", "kqvga.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 34298679),  // 1.0
	PRE_25_ENTRY_EN("kq1agdi", "kqvga.exe", "73f87b30f84e9c34ab09ec8dd7ae109d", 34922340),  // 2.0
	GAME_ENTRY("kq1agdi", "kqvga.exe", "888e2976e6659963af15df151b846540", 27083389),  // 3.0 Multilanguage
	GAME_ENTRY_EN("kq1agdi", "kq1vga.exe", "4e590490776aea10db84db4a92e1c1bb", 8226850),  // 4.0
	GAME_ENTRY_EN("kq1agdi", "kq1vga.exe", "688f1807c9d8df26fc0f174dc756054e", 8278611),  // 4.1c
	UNSUPPORTED_GAME_ENTRY("kq1agdi", "kq1vga.ags", "4d8b4afa7d2d9ff3e3523b0798d6581d", 6967052),  // 4.2 RC1
	GAME_ENTRY_EN("kq2agdi", "kq2vga.exe", "308d35bc34e9df29d8acce615593e3e7", 64947213),  // 1.0
	GAME_ENTRY_EN("kq2agdi", "kq2vga.exe", "3ee3a0166357ac37836f8908a371f2dc", 65743584),  // 2.0
	GAME_ENTRY_EN("kq2agdi", "kq2vga.exe", "43e6294f8bfbba8a3f754fa742353845", 12507938),  // 3.0?
	GAME_ENTRY_EN("kq2agdi", "kq2vga.exe", "40cfb7563df7dacf6530b19289a4745b", 12563246),  // 3.1
	GAME_ENTRY_EN("kq2agdi", "kq2vga.exe", "40cfb7563df7dacf6530b19289a4745b", 12572770),  // 3.1b
	GAME_ENTRY_EN("kq2agdi", "kq2vga.exe", "40cfb7563df7dacf6530b19289a4745b", 12574643),  // 3.1c
	GAME_ENTRY_LANG("kq2agdi", "Kq2vgaEsp.exe", "f964e5cb4d339432dd0302d67eb11105", 2490368, Common::ES_ESP), // Spanish translation
	GAME_ENTRY_EN("kq3agdi", "kq3redux.exe", "4c2ea3f7b4974509c59546ca8761b040", 11890197),  // 1.0
	GAME_ENTRY_EN("kq3agdi", "kq3redux.exe", "e569fb2ceabdc4a1609348c23ebc0821", 11986266),  // 1.1
	GAME_ENTRY_EN("qfg2agdi", "qfg2vga.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 20470902),  // 1.0
	GAME_ENTRY_EN("qfg2agdi", "qfg2vga.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 20523688),  // 1.1
	GAME_ENTRY_EN("qfg2agdi", "qfg2vga.exe", "582e26533cf784011c7565e89905d3c4", 18224373),  // 2.0

	// Infamous Adventures games. Likewise
	GAME_ENTRY_EN("kq3vga", "kq3.exe", "f120690b506dd63cd7d1112ea6af2f77", 4844298),  // 1.0
	GAME_ENTRY_EN("kq3vga", "KQ3.exe", "f120690b506dd63cd7d1112ea6af2f77", 5883843),  // 2.0 - itch.io Windows
	GAME_ENTRY_EN("kq3vga", "ac2game.dat", "f120690b506dd63cd7d1112ea6af2f77", 5883843), // 2.0 - itch.io Mac
	GAME_ENTRY_EN("sq2fg", "sq2fg.exe", "a524cbb1c51589903c4043b98917f1d9", 5329030),
	PRE_25_DEMO_ENTRY_EN("sq2vga", "sq2vgademo.exe", "bf5f207aff574ca3612d40d1213d8668", 4007953),
	GAME_ENTRY_EN("sq2vga", "sq2vga.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 106094482),  // 1.0
	GAME_ENTRY_EN("sq2vga", "sq2vga.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 117200044),  // 1.1
	GAME_ENTRY_EN("sq2vga", "sq2vga_v2.exe", "28a946e8a278814362613f8600375438", 177645020),  // 2.0 Win
	GAME_ENTRY_EN("sq2vga", "ac2game.dat", "28a946e8a278814362613f8600375438", 177645020),  // 2.0 Mac
	GAME_ENTRY_EN("sq2vga", "SQ2VGA_V2.ags", "b3026b025c640a5c6164274034a6bab9", 175145932), // 2.0 Linux

	// Free post-2.5 games that are likely supported by the AGS engine
	DEMO_ENTRY_EN("24hourgame", "24 Hour Game.exe", "06a03fe35791b0578068ab1873455463", 2209262),
	DEMO_ENTRY("3geeks", "3GEEKS_Demo.exe", "7ddb9e776648faed5a51170d087074e9", 512546851), // itch.io  En-Fr
	DEMO_ENTRY_EN("3dayspandora", "3_Days.ags", "654d58a2bc09c516f2ce69a1c2f88ae3", 1120924),
	DEMO_ENTRY_EN("99species", "99Species.exe", "615e73fc1874e92d60a1996c2330ea36", 23870894),
	DEMO_ENTRY_EN("aazor", "aazor.exe", "465f972675db2da6040518221af5b0ba", 4955082),
	DEMO_ENTRY_EN("aazor", "aazor.exe", "465f972675db2da6040518221af5b0ba", 4956212),
	DEMO_ENTRY_EN("aboardtheadventure", "Aboard.exe", "3c3f8f6c6fdcc519464477d3627b60f6", 125789038), // Voice: En-Es(LA), Subs: De-En-Es(La)-Pt(Br)
	DEMO_ENTRY_EN("aboardtheadventure", "Aboard.exe", "3c3f8f6c6fdcc519464477d3627b60f6", 125813308),
	DEMO_ENTRY_EN("abscission", "Abscission.exe", "418a9aaa044bfc5b7a79ba96fe679b60", 54341372),  // v1.0.3
	DEMO_ENTRY_EN("abscission", "Abscission Demo.exe", "418a9aaa044bfc5b7a79ba96fe679b60", 311033029),  // Steam
	DEMO_ENTRY_EN("abscission", "Abscission Demo.exe", "7633f0919378749cebf6dacba61ca0e1", 313386788),  // Steam
	DEMO_ENTRY_EN("absurdistan", "Absurdistan - Demo.exe", "4588012d3077d6b6791fce8cd4b15885", 35712363),  // v1.3
	DEMO_ENTRY_EN("acaixa", "A caixa.exe", "f434ba41ed68ef8d5224254027aad1f4", 48866212),
	DEMO_ENTRY_EN("acuriouspastime", "Curious.exe", "3e46b32a00fd4e6b9f68754f854695c5", 27346489),
	DEMO_ENTRY_EN("adventurequest", "begin.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1584015),
	DEMO_ENTRY_EN("adventurequest2", "adventure.exe", "f120690b506dd63cd7d1112ea6af2f77", 4603131),
	DEMO_ENTRY_EN("afrojones", "afrojones.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 4739799),
	DEMO_ENTRY_EN("agscolosseum", "AGS coliseum.exe", "6d65fa76ae212c9bbfa868698f47e921", 86449242),
	DEMO_ENTRY_EN("agsfootballer", "AgsFootballer.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 11169151),
	DEMO_ENTRY_EN("agsfootballer", "AgsFootballer.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 11169923),  //v1.0
	DEMO_ENTRY_EN("agsmoduletester", "v1.0 source.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 2020344),
	DEMO_ENTRY_EN("alemmo", "al-emmo.exe", "9661b29821fdc7f93d286f25c195fc22", 8932837), // Steam
	DEMO_ENTRY_EN("alemmo", "emmo_demo.exe", "2fb17c4382f2f54ef6d040b6493fec4b", 8943213), // Official website
	DEMO_ENTRY_EN("allhallowseve", "hallows.exe", "a9d8eb8963b4d3b8ed4f7e74edcb6ccd", 8904536),
	DEMO_ENTRY_EN("alphax", "alphax.exe", "06a03fe35791b0578068ab1873455463", 17879795),
	DEMO_ENTRY_EN("alum", "Alum Demo.exe", "6d2f8e80c5f2372b705fdd4cc32f3579", 236891240),
	DEMO_ENTRY_EN("annaquest", "AnnasQuest-DEMO.exe", "9257c91ee6cda3796d8af6856cf883bb", 69313413),
	DEMO_ENTRY_EN("anentertainingspeech", "An Entertaining Speech.exe", "a8971dcd9a97994a0b8fe9160ebad777", 171300529),
	DEMO_ENTRY_EN("animalcruelty", "Animal Cruelty Demo 1.2.exe", "495d45fb8adfd49690ae3b97921feec6", 41830978),  // v1.2
	DEMO_ENTRY_EN("anotherhero", "Another Hero - First Look Demo.exe", "7ddb9e776648faed5a51170d087074e9", 27404506),
	DEMO_ENTRY("apotheosis", "the apotheosis project.exe", "c52c9fe85328d90f0cd12e526ba585d4", 612169084),  // Screen7 demo En-It
	DEMO_ENTRY_EN("ashinaredwitch", "ATRW.exe", "8355ade0422da6e8655f71470ff51483", 44543604),  // Itch.io
	DEMO_ENTRY("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 51054188),  	// Itch.io demo/prologue
	DEMO_ENTRY_EN("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 48844189),  // Steam prologue
	DEMO_ENTRY("ashinaredwitch", "ATRW.exe", "7a24f662d686135f73c844231a5287bc", 51055599),  	// Steam prologue En-De-Pt-Ru-Tur
	DEMO_ENTRY_EN("ashinaredwitch", "Ashina.exe", "8355ade0422da6e8655f71470ff51483", 140043612),  // Steam
	DEMO_ENTRY_EN("ashinaredwitch", "Ashina.exe", "f34240ec041be3a6707a575fd28c8a03", 129558944),  // Steam
	DEMO_ENTRY_EN("ashpines", "Ash Pines Demo.exe", "981e60a9be515bb56c634856462abbc7", 90323249),
	DEMO_ENTRY_EN("ashpines", "Ash Pines Demo.exe", "0226eb0caac02b8bda8cf6e8fabf8915", 90560316),
	DEMO_ENTRY_EN("astroloco", "astroloco1.exe", "06e70a826fde73b3f86f974885d31abe", 19576101),
	DEMO_ENTRY("avcsurvivalguide", "CAV.exe", "42656cf777d07d4d35e978b233bb7aa8", 9012381),  //Eng-Esp Win/Mac
	DEMO_ENTRY_EN("awakeningofthesphinx", "awksphinx.exe", "0500aacb6c176d47ac0f8158f055db83", 7507867),
	DEMO_ENTRY_EN("bachelorstory", "Bachelor Story.exe", "a524cbb1c51589903c4043b98917f1d9", 7436465),
	DEMO_ENTRY_EN("bananaman", "banana man demo.exe", "615e806856b7730afadf1fea9a756b70", 53524952),
	DEMO_ENTRY("battlewarriorsrt", "battle.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16314318),  //En-Ru
	DEMO_ENTRY_EN("beekeeperpicnic", "beekeeperspicnic.exe", "1177e20f6a84ded5fad2a5d79fff8cb5", 190595833),
	DEMO_ENTRY_EN("beekeeperpicnic", "beekeeperspicnic.ags", "8ad3471b1f914371dae6a24d031ce8bf", 187427045),
	DEMO_ENTRY_EN("beforeww2", "beforeww.exe", "4d17844029d8910fbaae1bdc99e250f2", 9759616),
	DEMO_ENTRY_EN("beyondowlsgard", "Owlsgard Demo.exe", "ef70606d7030e9f6fa7c39f6437391c1", 783490949),
	DEMO_ENTRY_EN("beyondowlsgard", "Owlsgard Demo.ags", "7c5ebe409626b03744ac569970917a68", 766792869), // Linux
	DEMO_ENTRY_LANG("beyondowlsgard", "Owlsgard Demo.exe", "ef70606d7030e9f6fa7c39f6437391c1", 769684661, Common::DE_DEU),
	DEMO_ENTRY_LANG("beyondowlsgard", "Owlsgard Demo.ags", "718eb3e5534d254ebf0647c6dee66475", 780599157, Common::DE_DEU), // Linux
	DEMO_ENTRY_EN("bizarreearthquake", "bizarre.exe", "7e7c92d697b3cc9610d86854a00e517b", 29387739),
	DEMO_ENTRY_EN("blackmorph", "liechi.exe", "46859c6f77bdb311266daa589561fa6b", 14476030),  // MAGS
	DEMO_ENTRY("blackmorph", "Black Morph.exe", "cd37b1dbdff2c7092e05f5abed2f5dee", 16333092),  // v1.0 En-Fr
	DEMO_ENTRY_EN("blackwell1", "blackwell demo.exe", "2eeca6bae968dc61b5ed36561efc0fca", 53342880),
	DEMO_ENTRY_EN("blackwell2", "unbound_demo.exe", "6ee842f73649ced615c44d4eb303687c", 30057537),
	DEMO_ENTRY_EN("blackwell3", "convergence.exe", "856a6e0e22d2cb4a45d5bbc7245ce5e8", 29935120),
	DEMO_ENTRY_EN("blackwell4", "deception.exe", "b3b192cf20a2f7666ddea3410dbd87cc", 260533048),
	DEMO_ENTRY_EN("blackwell5", "epiphany.exe", "c1cddd6fcdbcd030beda9f10d4e4270a", 283994688),
	DEMO_ENTRY_EN("blockz", "Blockz.exe", "e007e8f70fecd73999aaf53847c5a833", 31799091),
	DEMO_ENTRY_EN("bluemoon", "blue moon.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 14845997),
	DEMO_ENTRY_EN("bookofdestiny", "SG.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1198451),
	DEMO_ENTRY_EN("bowanddork", "BowAndDork.exe", "7db052bc30700d1f30f5330f5814f519", 17121243),  // Windows
	DEMO_ENTRY_EN("bowanddork", "BowAndDork.ags", "8728011855017e243bb95682d2898004", 14086087),  // Linux
	DEMO_ENTRY_EN("brotherswreckers", "Brothers & Wreckers.exe", "f24c533ce89a2566c157b871f87a4ce4", 42831964),  // v1.0
	DEMO_ENTRY_EN("brotherswreckers", "Brothers & Wreckers.exe", "24da4d1b11cb22967dd0689aa6a3dbb4", 45934052),  // v1.0.3
	DEMO_ENTRY_EN("brownieadv1", "Brownie1.exe", "3c8d2feaba74ad4978316824c903c046", 140162800),
	DEMO_ENTRY_EN("bytheswordconspiracy", "bts.exe", "186ae6430b892c6f6945f05287f77de9", 40633887),
	DEMO_ENTRY_EN("bytheswordconspiracy", "bts.exe", "7dc7f61f79ba7a77d4ef8168bfd3d173", 60246329),
	DEMO_ENTRY_EN("byzantine", "byza.exe", "39d7a558298a9f1d40c1f415daf9bb74", 3708632),
	DEMO_ENTRY_EN("byzantine", "byza.exe", "ecc8eaa38fe3adea61ffc525d2ce5c0e", 3660405),
	DEMO_ENTRY_EN("calvin", "CalvinENG.exe", "7a3096ac0237cb6aa8e1718e28caf039", 85555229),
	DEMO_ENTRY_LANG("calvin", "Calvin.exe", "7a3096ac0237cb6aa8e1718e28caf039", 85557724, Common::FR_FRA),
	DEMO_ENTRY_EN("captaindisaster", "cd-dhamsb-demo-1-3-0.exe", "9d991dd1f9e7fee653d3a9bb2546f968", 153646768),
	DEMO_ENTRY_EN("captaindisasterriskara", "Captain Disaster and The Two Worlds of Riskara.exe", "6689ccca6f9b2c8398352e9b772ff411", 157630000),
	DEMO_ENTRY_EN("carnivalags", "Carnival.ags", "0a0ce51f708cd532d3f2f6fdfded8f32", 39307700),  // itch.io 1.0.0
	DEMO_ENTRY_EN("carnivalags", "Carnival.ags", "f5518304ced1e9db254bc80cf6aa8fa7", 41023763),  // itch.io 1.1.0
	DEMO_ENTRY_EN("carnivalags", "Carnival.ags", "9e0e6315c269b4452954dabd69def773", 80331480),  // itch.io 1.1.1
	DEMO_ENTRY("captainhook", "CaptainHookAndTheLostGirl - DEMO.exe", "776a62db4387dc68be92ef9933399fd5", 8731139),  // Windows En-Fr
	DEMO_ENTRY("captainhook", "CaptainHookAndTheLostGirl - DEMO.ags", "c0ce7476116c0a81e1d8f01ccf87d79a", 5624815),  // Linux En-Fr
	DEMO_ENTRY_EN("castledornstein", "Castle Dornstein Demo.exe", "4141c41c06c91ec8ab9fffafbc26df67", 53098143),
	DEMO_ENTRY_EN("ceelo", "Cee-Lo!.exe", "a409703089eebbcfa13f0a22f6fb71ed", 32731930),
	DEMO_ENTRY_EN("charnelhousetrilogy", "sepulchredemo.exe", "9ba3d9fbb098a26d7293ad3161c0f270", 139434513), // Steam
	DEMO_ENTRY("chronicleofinnsmouth", "ChronicleOfInnsmouthTechDemo1.1.exe", "b5c41e08919834b549ca350a28f18f87", 43595582),  //En-It
	DETECTION_ENTRY("chriscolumbus", "ChrisColumbus.exe", "615e73fc1874e92d60a1996c2330ea36", 14817026, Common::EN_ANY, "Act 1 Demo", ADGF_DEMO, 0),
	DETECTION_ENTRY("chriscolumbus", "ChrisColumbus.exe", "615e73fc1874e92d60a1996c2330ea36", 15681362, Common::EN_ANY, "Acts 1-2 (partial) Demo", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("chtonic", "Chtonic.exe", "0564de07d3fd5c16e6947a647061913c", 139463704),
	DEMO_ENTRY_EN("chtonic", "Chtonic.ags", "94e738e81223e25b6365a133831317be", 136431108),
	DEMO_ENTRY_EN("chtonic", "Chtonic.exe", "0564de07d3fd5c16e6947a647061913c", 142242256),
	DEMO_ENTRY_LANG("chuckisland", "nylon.ags", "eca3413b0c91e101af46c5f5f55e8a13", 16815791, Common::FR_FRA),
	DEMO_ENTRY_LANG("chuckisland", "nylon.ags", "0979a62d3274519bd158b775009c70f6", 17305307, Common::FR_FRA),
	DEMO_ENTRY_EN("city", "City.exe", "3e62eafed0fc365821b7afdbf1aec3d8", 18660151),
	DEMO_ENTRY_EN("citythatdrowned", "CityThatDrowned.exe", "27bedc55ec093b78c0ca59cd15ffb472", 6528707),
	DEMO_ENTRY_EN("clownatthecircus", "circus.exe", "f120690b506dd63cd7d1112ea6af2f77", 1073547),
	DEMO_ENTRY_EN("clubmidget", "midget2.exe", "4d17844029d8910fbaae1bdc99e250f2", 1593937),
	DEMO_ENTRY_EN("coelldeckaflight", "coell deca.exe", "a524cbb1c51589903c4043b98917f1d9", 5497572),
	DEMO_ENTRY_EN("comiclassic", "CMIOE.exe", "9beac9a36669407526f1c34018e443d4", 29284874),
	DEMO_ENTRY("comiclassic", "CMIOE.exe", "c361f4d93e1a802ea7908a1b95d81757", 36990562),  //Eng-Ita
	DEMO_ENTRY_EN("cosmosquest1", "tfas1.exe", "0710e2ec71042617f565c01824f0cf3c", 4571793),
	DEMO_ENTRY_EN("cougarisland", "LV69.exe", "63f8a60cc094996bd070e68cb3c4722c", 17187670),
	DEMO_ENTRY_EN("crimmsson", "Crimm's Son.exe", "618d7dce9631229b4579340b964c6810", 9039508),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "38a49a28c0af599633006c09fe6c2f72", 147061686),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "69414fa4aa2cc5414a38cc166d44338b", 51640494),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "69414fa4aa2cc5414a38cc166d44338b", 53471704),  // v10
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "69414fa4aa2cc5414a38cc166d44338b", 53472267),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 53489221),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 53489518),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 59100513),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 59894296),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 59901863),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 60144256),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 60147146),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 78606556),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO.exe", "69414fa4aa2cc5414a38cc166d44338b", 78610191),
	DEMO_ENTRY_EN("crimsondiamond", "TheCrimsonDiamond_demo_v16.exe", "69414fa4aa2cc5414a38cc166d44338b", 129741433),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "69414fa4aa2cc5414a38cc166d44338b", 129741433),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "69414fa4aa2cc5414a38cc166d44338b", 129741431),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo.exe", "69414fa4aa2cc5414a38cc166d44338b", 129741340),
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond DEMO v17.exe", "6f4d5442c166d215017764b119af9ffa", 235426199),  // Win v17
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo v18.exe", "38a49a28c0af599633006c09fe6c2f72", 131455545),  // Win v18
	DEMO_ENTRY_EN("crimsondiamond", "The Crimson Diamond Demo v19.exe", "38a49a28c0af599633006c09fe6c2f72", 131456250),  // Win v19
	DEMO_ENTRY_EN("crimsondiamond", "ac2game.dat", "69414fa4aa2cc5414a38cc166d44338b", 60147146),  // macOS
	DEMO_ENTRY_EN("crimsondiamond", "ac2game.dat", "69414fa4aa2cc5414a38cc166d44338b", 78610191),  // macOS
	DEMO_ENTRY_EN("crimsondiamond", "ac2game.dat", "69414fa4aa2cc5414a38cc166d44338b", 129741431),  // macOS
	DEMO_ENTRY_EN("cybermemory", "CybermemoryNotFound.exe", "7971a7c02d414dc8cb33b6ec36080b91", 3851249),  // Windows
	DEMO_ENTRY_EN("cybermemory", "CybermemoryNotFound.ags", "49416a0d45607a9a352fec6788ed712f", 1354209),  // Linux
	DEMO_ENTRY_EN("cyberpunk97ep1", "zone - Copy.exe", "78dd4ca028ee0156b6a093d6d780aa65", 717217747),
	DEMO_ENTRY_EN("cyberpunk97ep1", "zone - Copy.exe", "78dd4ca028ee0156b6a093d6d780aa65", 1014330648),
	DEMO_ENTRY_EN("danewguys2", "dng2 demo.exe", "0d2e8dc76f9ac381d204a3ac763b0bac", 5235302),
	DEMO_ENTRY_EN("dangermouse", "Danger Mouse.exe", "eed666c68be394737de478a52210dde4", 247422806),
	DEMO_ENTRY_EN("dawnswonderedatagesend", "myGame.exe", "75f4c7f66b1be60af5b2d65f617b91a7", 243406385),
	DEMO_ENTRY_EN("deflus", "Deflusdemo.exe", "0710e2ec71042617f565c01824f0cf3c", 2939316),
	DEMO_ENTRY_EN("demonicdollhouse", "DemonHunter.exe", "923a79b80d267f21dfdf723e8a013752", 26616797),
	DEMO_ENTRY("detectivegallo", "dgbuild.exe", "5f2e683b0d5e66e47f5800322982171e", 466252165),  // En-It
	DEMO_ENTRY_EN("detectiverizal", "Detective Rizal and the Jaded Ruby.exe", "615e73fc1874e92d60a1996c2330ea36", 28411427),
	DEMO_ENTRY_EN("diamondsintherough", "diamonds demo.exe", "14c995189266969d538b31d1df451d3f", 2439365),
	DEMO_ENTRY_EN("doctorwho", "Doctor Who.exe", "06a03fe35791b0578068ab1873455463", 50728727),
	DEMO_ENTRY_EN("doses", "PN.exe", "615e73fc1874e92d60a1996c2330ea36", 3891599),  // v1.0
	DEMO_ENTRY_EN("doses", "PN.exe", "615e73fc1874e92d60a1996c2330ea36", 3945217),  // v1.2
	DEMO_ENTRY_EN("downfall2016", "downfall_demo.exe", "7c87b99ce309a46085e40ac1a2b20e75", 97968746), // Steam
	DEMO_ENTRY("dragonsfang", "DraciZub_hra.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 27463584),  // Eng-Cze
	DEMO_ENTRY_EN("dreamswitchhouse", "Dreams in the Witch House Demo.exe", "b7996387269db9b8a17bc358eaf37464", 508137287), // 1.03
	DEMO_ENTRY_EN("dreamswitchhouse", "Dreams in the Witch House Demo.exe", "08f3fe1967f45209a939d9c36365b06a", 508229620), // 1.04
	DEMO_ENTRY_EN("dreamswitchhouse", "Dreams in the Witch House Demo.exe", "08f3fe1967f45209a939d9c36365b06a", 508230379), // 1.05
	DEMO_ENTRY_EN("drevil", "drevil.exe", "0394af1c29e1060fcdbacf2a3dd9b231", 1371668),
	DEMO_ENTRY_EN("dumbassdrivers", "dumbass.exe", "f120690b506dd63cd7d1112ea6af2f77", 40580988),
	DEMO_ENTRY_EN("dustbowl", "Dustbowl - DEMO.exe", "b9f4b6df0ab98d27d1bf8b874b8d12b6", 15285362),
	DEMO_ENTRY_EN("dusttowater", "Dust to Water.exe", "0e32c4b3380e286dc0cea8550f1c045e", 70459846),
	DEMO_ENTRY_EN("dysmaton", "Dysmaton.exe", "6e861b1f476ff7cdf036082abb271329", 140513956),
	DEMO_ENTRY_EN("dysmaton", "Dysmaton.ags", "1e0cda052a371ebf6aa0bd62c0e754b9", 138033300),
	DEMO_ENTRY_EN("earlbobby3", "demo.exe", "0e32c4b3380e286dc0cea8550f1c045e", 4221725),
	DEMO_ENTRY_EN("echointheclouds", "Echo in the Clouds.exe", "2321081ba51d1869ebe11b9ba126911b", 57717828),
	DEMO_ENTRY_EN("eerieblue", "EerieBlue.exe", "bacdba3c759a861e899e6b0adcbb8bd5", 96605122),
	DEMO_ENTRY_EN("eerieblue", "EerieBlue.exe", "bacdba3c759a861e899e6b0adcbb8bd5", 96647570),
	DEMO_ENTRY("emmaroide", "emma roide.exe", "aefd91a131817036d224fe030e406c6e", 24919443),  // En-Fr
	DEMO_ENTRY_EN("endacopia", "Endacopia.exe", "67dcfaf952fcc9451cafd2c47cab159b", 89224364),  // Win
	DEMO_ENTRY_EN("endacopia", "Endacopia.ags", "b80e86f0ec4d745d1e8503918b47dfe4", 86090392),
	DEMO_ENTRY_EN("endacopia", "game.ags", "67dcfaf952fcc9451cafd2c47cab159b", 89224364),  // Mac
	DEMO_ENTRY("englishhaunting", "An English Haunting DEMO.exe", "5528c91a1e289389553f97b50ef67bcd", 245477194),  // Eng-Esp
	DEMO_ENTRY_EN("enterthestory", "Enter The Story.exe", "0514661a4ba6772cf0db0cf839fc7003", 19589742),
	DEMO_ENTRY_EN("ericmegalith", "eric.exe", "06a03fe35791b0578068ab1873455463", 83259322),
	DEMO_ENTRY_EN("everlight", "Everlight.exe", "0500aacb6c176d47ac0f8158f055db83", 17597037),
	DEMO_ENTRY_EN("everythingm", "TinyMika.exe", "66adc3431f0626a2c6e80f31a21f0981", 3529496),
	DEMO_ENTRY_EN("ferra", "Ferra.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3679129),
	DEMO_ENTRY_EN("fixer", "Fixer Demo.exe", "6f8872f30dd49392ba50bbbed14054c4", 7931881),
	DEMO_ENTRY_EN("flashbax", "demo flashbax.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 5527034),
	DEMO_ENTRY_EN("forcemajeureiithezone", "thezonedemo.exe", "b63d26c17bf292017f9a708ae9dc38ca", 18414473),
	DEMO_ENTRY_EN("forfrogssake", "Frog2.exe", "0241777c2537fc5d077c05cde10bfa9f", 6723366),
	DEMO_ENTRY_EN("forge", "Theforge.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 11424539),
	DEMO_ENTRY_LANG("forgettendeath", "ForgettenDeath.exe", "06a03fe35791b0578068ab1873455463", 19427130, Common::TR_TUR),  // Turkish
	DETECTION_ENTRY("fountainofyouth", "FOY.exe", "06a03fe35791b0578068ab1873455463", 4137644, Common::EN_ANY, "Intro 2003", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("fountainofyouth", "foydemo.exe", "f120690b506dd63cd7d1112ea6af2f77", 14372003),  // v1.0
	DEMO_ENTRY_EN("fountainofyouth", "foydemo1.1.exe", "f120690b506dd63cd7d1112ea6af2f77", 14372003),  // v1.0 (misnamed file)
	DEMO_ENTRY_EN("fountainofyouth", "foydemo.exe", "f120690b506dd63cd7d1112ea6af2f77", 14371419),  // v1.1
	DEMO_ENTRY_LANG("fountainofyouth", "FoYDemo_Dutch.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 15295028, Common::NL_NLD),  // v1.1
	DEMO_ENTRY_EN("fountainofyouth", "foydemo.exe", "5f141d6719b7b748087a7c05be13a7f6", 18766682),  // v1.2 (TRAC #14410)
	DEMO_ENTRY_EN("fountainofyouth", "foydemo.exe", "5f141d6719b7b748087a7c05be13a7f6", 19770554),  // v1.2 r1
	DEMO_ENTRY_EN("fountainofyouth", "foydemo.exe", "f541f879f479bf430bb0f6d8bb151d5d", 19770548),  // v1.2 r2-r3
	DEMO_ENTRY_LANG("fountainofyouth", "FOYDemoDutch.exe", "f25752043e2a77ec07bca0c6818af701", 20666760, Common::NL_NLD),  // v1.2 r1
	DEMO_ENTRY_LANG("fountainofyouth", "FOYDemoDutch.exe", "f25752043e2a77ec07bca0c6818af701", 20737160, Common::NL_NLD),  // v1.2 r2-r3
	DEMO_ENTRY_EN("foyfighter", "Arcade_Fighting_Demo.exe", "6a55492a65c52cf323dd5324749e46de", 14647406),
	DEMO_ENTRY_EN("frankmalone", "Malone.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4237095),
	DEMO_ENTRY_EN("franticfranko", "steelh1.exe", "2cb9c81a20282b55f69343ef95cbb63d", 37086267),
	DEMO_ENTRY_EN("funnyboneyard", "the_funny_boneyard.exe", "78dd4ca028ee0156b6a093d6d780aa65", 63460160),
	DEMO_ENTRY_EN("gamesgalore", "gamesgalore.exe", "f120690b506dd63cd7d1112ea6af2f77", 7620552),
	DEMO_ENTRY_EN("gassesuittollis3", "gst3_demo.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 4270243),
	DEMO_ENTRY_EN("geminirue", "gemini rue demo.exe", "978eab85a41b8fc089b719af47dfbb79", 61839853),
	DEMO_ENTRY_EN("gesundheit", "gesundheit.exe", "5554b9e0df6241d25c9a070708e54478", 49666357),
	DEMO_ENTRY_EN("goldenwake", "a-golden-wake.exe", "e80586fdc2db32f65658b235d8cbc159", 54256115),
	DEMO_ENTRY_EN("goosequest1", "goose.exe", "0710e2ec71042617f565c01824f0cf3c", 25194434),
	DEMO_ENTRY_EN("ghostdream", "ghostdream.exe", "05594881531d62e4575545f3c8fd2576", 225600520),
	DEMO_ENTRY_EN("graceward", "Red_Black_Brown.exe", "0564de07d3fd5c16e6947a647061913c", 167543007),  //itch.io
	DEMO_ENTRY_EN("graceward", "Graceward.exe", "0564de07d3fd5c16e6947a647061913c", 210565575),  //itch.io 1.15
	DEMO_ENTRY_EN("graceward", "Graceward.exe", "0564de07d3fd5c16e6947a647061913c", 226256533),  //itch.io 1.2
	DETECTION_ENTRY("graceward", "Graceward.exe", "0564de07d3fd5c16e6947a647061913c", 292537766, Common::EN_ANY, "Complete Edition Demo", ADGF_DEMO, 0),  //itch.io CE
	DETECTION_ENTRY("graceward", "Graceward.exe", "0564de07d3fd5c16e6947a647061913c", 292501437, Common::EN_ANY, "Complete Edition Demo", ADGF_DEMO, 0),  //itch.io CE
	DETECTION_ENTRY("grandmabadass", "grandmaDEMO.exe", "636250e131f51c7a97989992cc97cf02", 525503237, Common::UNK_LANG, "Prologue Demo", ADGF_DEMO, 0), // Multi
	DEMO_ENTRY("grandmabadass", "grandmaDEMO.exe", "65917d4eba3b2e3cf2befc06340c1f53", 1512973969),  // Multi
	DEMO_ENTRY_EN("granville1", "GVILLE1.exe", "c1451c6631e1bf4de6d1ed3760ca1dc9", 2340624),
	DEMO_ENTRY_LANG("graydale", "JimmyFox.exe", "d25c3cb8a42c5565634dfef76b3cf83e", 2562458, Common::RU_RUS),
	DEMO_ENTRY_EN("greenback", "Greenback.exe", "b80b5892fc0291add209a0d3de6dedc5", 10490547),  // v1.0.6a
	DEMO_ENTRY_EN("greenback", "Greenback.exe", "b80b5892fc0291add209a0d3de6dedc5", 10492342),  // v1.0.6b
	DEMO_ENTRY("greycity", "graycity - demo no music.exe", "0241777c2537fc5d077c05cde10bfa9f", 3856026),  // Eng-Esp
	DEMO_ENTRY_EN("gwendarkly", "MAGSGwenVersion.exe", "ac764efb63859d435e03f04f46f2b1f0", 10141568),
	DEMO_ENTRY_EN("harrycaine", "Harry Caine Extreme Weather Reporter.exe", "82da2565c456dcfb265ded6fe3189c0b", 24158833),
	DEMO_ENTRY_EN("hellsatans", "Hell's Satans.exe", "0710e2ec71042617f565c01824f0cf3c", 2622899),
	DEMO_ENTRY_EN("herdiscoming", "herd.exe", "78dd4ca028ee0156b6a093d6d780aa65", 5664594), // Steam
	DEMO_ENTRY_EN("hewatches", "MansionAGS.exe", "74dc062c5f68f3d70da911f2db5737b6", 10728553),
	DEMO_ENTRY_EN("hiddenmessages", "RotNHM.ags", "86fda37a195616bc14eca3b9ac1a4116", 34402785),
	DEMO_ENTRY_EN("hood", "hood.exe", "97d700529f5cc826f230c27acf81adfd", 1575198),
	DEMO_ENTRY_EN("hopelessness", "TBAD.exe", "0564de07d3fd5c16e6947a647061913c", 13433436),
	DEMO_ENTRY_EN("horrorhospital", "HHH.exe", "3aba76b46b4493c5541a2a17e67fdd93", 48089904),
	DEMO_ENTRY_EN("houseofhorror", "The House of Horror.exe", "c20fee88ad37d2d21837eee9d34f2dbe", 10120651),
	DEMO_ENTRY_EN("hpunk", "H_punk.exe", "7971a7c02d414dc8cb33b6ec36080b91", 37544988),
	DEMO_ENTRY_EN("hpunk", "H_punk.ags", "a6d34e62d2584acb00cea1a45d88c2b5", 35047948),
	DEMO_ENTRY_EN("huxzadventure", "Huxsadv.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2053897),
	DEMO_ENTRY_EN("illuminum", "ill-uminum.exe", "60d4148b5813a547b0538513e9706f9c", 206930524),
	DEMO_ENTRY_EN("incantamentum", "INCANTAMENTUM.exe", "889b7abe9e3b5eccbf9188822340d506", 196455383),  // Steam Game Festival: Autumn Edition Demo
	DEMO_ENTRY_EN("incantamentum", "INCANTAMENTUM.exe", "889b7abe9e3b5eccbf9188822340d506", 211349826),  // The Big Adventure Event Demo
	DEMO_ENTRY_EN("incantamentum", "INCANTAMENTUM.exe", "889b7abe9e3b5eccbf9188822340d506", 299412317),  // Steam Next Fest Demo
	DEMO_ENTRY_EN("incantamentum", "INCANTAMENTUM.exe", "889b7abe9e3b5eccbf9188822340d506", 293619654),  // Steam Next Fest Demo
	DEMO_ENTRY_EN("incantamentum", "INCANTAMENTUM.exe", "889b7abe9e3b5eccbf9188822340d506", 332439302),
	DEMO_ENTRY_LANG("indyatp", "indy.exe", "0500aacb6c176d47ac0f8158f055db83", 2299202, Common::FR_FRA),
	DEMO_ENTRY_LANG("indyaventuriers", "ark_demo.exe", "06a03fe35791b0578068ab1873455463", 2837800, Common::FR_FRA),
	DETECTION_ENTRY("indycrownofsolomon", "rolllc2.exe", "0710e2ec71042617f565c01824f0cf3c", 5901086, Common::EN_ANY, "Rolling Demo", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("indycrownofsolomon", "IndyCoS.exe", "f120690b506dd63cd7d1112ea6af2f77", 5483592),
	DEMO_ENTRY_LANG("indycrownofsolomon", "cosdemo2.1-it.exe", "f120690b506dd63cd7d1112ea6af2f77", 5772985, Common::IT_ITA),  //Italian
	DEMO_ENTRY_LANG("indycrownofsolomon", "CoSdemo2.1.exe", "f120690b506dd63cd7d1112ea6af2f77", 6178327, Common::DE_DEU),  //German
	DEMO_ENTRY_LANG("indycrownofsolomon", "CoSdemo2.1.exe", "f120690b506dd63cd7d1112ea6af2f77", 6069990, Common::ES_ESP),  //Spanish
	DEMO_ENTRY_LANG("indycrownofsolomon", "CoSdemo2.1.exe", "06a03fe35791b0578068ab1873455463", 6903169, Common::FI_FIN),  //Finnish
	DEMO_ENTRY_EN("indycrystalcursor", "cursoid.exe", "06a03fe35791b0578068ab1873455463", 12136127),
	DEMO_ENTRY_EN("indyeyedestiny", "Trap.exe", "618d7dce9631229b4579340b964c6810", 3619133),
	DEMO_ENTRY_EN("indysevencities", "Seven Cities.exe", "c9cd3e2cdd295c33e62cfa97b14461e9", 12561314),  //v1.2.1.1
	DEMO_ENTRY("indysevencities", "Seven Cities.exe", "be858e83311d4f1b49e05b933b92f0a9", 12592498),  // Eng-Ita
	DETECTION_ENTRY("indysevencities", "mirrors_demo.exe", "615e73fc1874e92d60a1996c2330ea36", 3363553, Common::UNK_LANG, "Mirrors Module Demo v1", ADGF_DEMO, 0),  //En-It
	DETECTION_ENTRY("indysevencities", "mirrors_demo_2.0.exe", "354e3016d64ac3378478d3d52ef0460d", 4301300, Common::UNK_LANG, "Mirrors Module Demo v2", ADGF_DEMO, 0),  //En-It
	DETECTION_ENTRY("indysevencities", "IJ7Cities-Mirrors.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 4637313, Common::UNK_LANG, "Mirrors Module Demo v2", ADGF_DEMO, 0),  //En-It
	DETECTION_ENTRY("indysevencities", "Map_noDynamicSprites.exe", "615e73fc1874e92d60a1996c2330ea36", 3540387, Common::UNK_LANG, "Indy Map Module Demo", ADGF_DEMO, 0),  //En-It
	DETECTION_ENTRY("indysevencities", "IJ7Cities-TextArea.exe", "635852726a70679997f9834619678845", 3633773, Common::UNK_LANG, "Text Area Module Demo", ADGF_DEMO, 0),  //En-It
	DETECTION_ENTRY("indysevencities", "IJ7Cities-Icicles.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 3798425, Common::UNK_LANG, "Icicles Module Demo", ADGF_DEMO, 0),  //En-It
	DEMO_ENTRY_EN("infectionep2", "Infection_Ep_II_AGS.exe", "a524cbb1c51589903c4043b98917f1d9", 23691417),
	DEMO_ENTRY_EN("inferno", "inf.exe", "97d700529f5cc826f230c27acf81adfd", 8783105),
	DEMO_ENTRY_LANG("insidemonkeyislandch4", "IMI4.exe", "18b284c22010850f79bc5c20054a70c4", 8187753, Common::IT_ITA),
	DEMO_ENTRY_EN("intergalacticspacepancake", "space.exe", "4d17844029d8910fbaae1bdc99e250f2", 1993173),
	DEMO_ENTRY_EN("intraworld", "1 Intra World.exe", "0241777c2537fc5d077c05cde10bfa9f", 40044701),
	DEMO_ENTRY_EN("isometrichouse", "thehouse.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1129697),
	DEMO_ENTRY_EN("jacquelinewhitecurseofthemummies", "CurseOfTheMummies.exe", "e9bdea51db49da499c69e8c21a821a7f", 193723250),
	DEMO_ENTRY("jamesperis2", "james2.exe", "06a03fe35791b0578068ab1873455463", 35766505),  // Eng-Esp
	DEMO_ENTRY_EN("jimmsquest3", "JQ3.EXE", "0b7529a76f38283d6e850b8d56526fc1", 4930906),
	DEMO_ENTRY_EN("jimmygoodlove", "JimmyGoodlove.exe", "a524cbb1c51589903c4043b98917f1d9", 12332801),
	DEMO_ENTRY_EN("jimmythetroublemaker2", "JTT2.exe", "25976a689b0f4d73eac69b1728377ecb", 111689377),
	DEMO_ENTRY_LANG("johnsinclair", "John Sinclair - Voodoo in London.exe", "d72e72697a755c7de395b0f6c8cbbf0d", 56795991, Common::DE_DEU),
	DEMO_ENTRY_EN("jonstickman", "J.S.exe", "a2230d4ac3c39431b51c9bb20929932c", 9570622),
	DEMO_ENTRY("jorry", "jorry demo.exe", "fe5f7dc7785b335aec72a2a834629bad", 124667001),  // En-Fr
	DEMO_ENTRY("justignorethem", "justignorethem.exe", "7a3096ac0237cb6aa8e1718e28caf039", 98544330),  //itch.io - Steam Multilang
	DEMO_ENTRY("justignorethembrea1", "Brea Story - DEMO.exe", "9c9aa254cbaf1e591f57af413da13f12", 47416849),
	DEMO_ENTRY_LANG("karelianninja", "Karelian ninja.exe", "25976a689b0f4d73eac69b1728377ecb", 14407656, Common::RU_RUS),
	DEMO_ENTRY_PLUGIN("kathyrain", "kathyrain.exe", "434e24a12ba3cfb07d7b4b2f0e0bb1bf", 197487685, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP), // Steam En-Fr-De-Es-It-Pol
	DEMO_ENTRY_PLUGIN("kathyrain", "kathyrain.exe", "d2b1ba151c3d209b50331127032f96f6", 197490245, GAMEFLAG_PLUGINS_AGSSPRITEFONT_CLIFFTOP), // GOG
	DEMO_ENTRY_EN("kingrobert", "King Robert's Quest.exe", "0241777c2537fc5d077c05cde10bfa9f", 4847870),
	DEMO_ENTRY_EN("kinkyisland", "kinky.exe", "f120690b506dd63cd7d1112ea6af2f77", 10628479),
	DEMO_ENTRY_LANG("klopoty", "Demo.exe", "792c0a0eaeba1a8846cb7b1af1e3266e", 8707667, Common::PL_POL),
	DEMO_ENTRY_LANG("klopoty", "Demo.ags", "4457c889d1f6ed721a3297c9a5e48285", 5670975, Common::PL_POL),
	DEMO_ENTRY_EN("kq3plus", "Intro.ags", "06a03fe35791b0578068ab1873455463", 4866555),
	DEMO_ENTRY_EN("kq9vga", "KQ9DEMO1.exe", "06a03fe35791b0578068ab1873455463", 2635933),
	DEMO_ENTRY_EN("kumastory", "Kuma Story.exe", "54e966a013d104bf23603c780438d089", 2893472),
	DEMO_ENTRY("lacolonia", "thehomestead.ags", "990a907fae9dae241bcfdf245224fe09", 45322084),  // itch.io client Eng-Ita
	DEMO_ENTRY_EN("laportenoire", "proute.exe", "06a03fe35791b0578068ab1873455463", 17087125),
	DEMO_ENTRY_EN("lastpirateadventure", "DrakeTreasureDemo.exe", "7971a7c02d414dc8cb33b6ec36080b91", 554164470),
	DEMO_ENTRY_EN("lastpirateadventure", "DrakeTreasureDemo.exe", "7971a7c02d414dc8cb33b6ec36080b91", 554166474),
	DEMO_ENTRY_LANG("lastpirateadventure", "DrakeTreasureDemo.exe", "7971a7c02d414dc8cb33b6ec36080b91", 554449052, Common::ES_ESP),
	DEMO_ENTRY_LANG("lastpirateadventure", "DrakeTreasureDemo.exe", "7971a7c02d414dc8cb33b6ec36080b91", 554451160, Common::ES_ESP),
	DEMO_ENTRY_EN("lazytownthenewkid", "newkiddemo1.2.exe", "9cb3c8dc7a8ab9c44815955696be2677", 15515508),
	DEMO_ENTRY_EN("legendofhand", "legend of hand.exe", "fc478dd7564c908615c9366398d995c8", 75795600),
	DEMO_ENTRY("legendofskye", "LegendSkye.exe", "8d1ff95c16500befbdc72260d461d73f", 71949528),  // Win Eng-Esp
	DEMO_ENTRY("legendofskye", "LegendSkye.exe", "8d1ff95c16500befbdc72260d461d73f", 72090450),  // Win En-Fr-De-Es
	DEMO_ENTRY("legendofskye", "LegendSkye.exe", "8d1ff95c16500befbdc72260d461d73f", 72090942),  // Win En-Fr-De-Es
	DEMO_ENTRY("legendofskye", "LegendSkye.exe", "8d1ff95c16500befbdc72260d461d73f", 72091136),  // Win En-Fr-De-Es
	DEMO_ENTRY("legendofskye", "LegendSkye.ags", "df2ca0c97d229119edbde64322629cdb", 68840644),  // Linux Eng-Esp
	DEMO_ENTRY("legendofskye", "LegendSkye.ags", "65ab826c0660ae17f28f9e7d024f8f2f", 68981566),  // Linux En-Fr-De-Es
	DEMO_ENTRY("legendofskye", "LegendSkye.ags", "6bdda57dcf7c322663a5aaf99128191c", 68982058),  // Linux En-Fr-De-Es
	DEMO_ENTRY("legendofskye", "LegendSkye.ags", "19b763d61e04f9fc042e5b54a05f4d9d", 68982252),  // Linux En-Fr-De-Es
	DEMO_ENTRY_EN("leisuresuitlarrylil", "LSL.exe", "34cf71d28e1e9c55934f624969011c7e", 18440862),
	DEMO_ENTRY_EN("littlesimulatedpeople", "LSP.exe", "9444eb2427a9fc090dde9ab8330a149f", 2622652),
	DEMO_ENTRY_EN("longevitygene", "Longevity.exe", "3d40063da244931d67726a2d9600f1e8", 63748082),
	DEMO_ENTRY_EN("longexpectedfriday", "LEFRIDAY.exe", "28f82e420b82d07651b68114f90223c8", 7595812),
	DEMO_ENTRY_EN("loomiireturnoftheswans", "loom2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9761097),
	DEMO_ENTRY_EN("loonyisland", "Loony Island.exe", "726b4b32e701495c689e2e65f888e0be", 7296475),
	DEMO_ENTRY_EN("lostinparadise", "2paradise.exe", "06a03fe35791b0578068ab1873455463", 37279978),
	DEMO_ENTRY("lostinthenightmare2", "litn_umtechdemo.exe", "f120690b506dd63cd7d1112ea6af2f77", 3507831),  // Eng-Tur
	DEMO_ENTRY("lucasmendoza", "Lucas Mendoza - Detective Amateur DEMO v1.0.exe", "211fb66118926f6df948a857715a299c", 17956030),  // Eng-Esp
	DEMO_ENTRY("lucasmendoza", "Lucas Mendoza - Detective Amateur DEMO v1.0.ags", "1bdac796b6fbcbcfbbe2671965384f99", 14914218),  // Eng-Esp
	DEMO_ENTRY_EN("maelstrom", "Maelstrom Obscura.exe", "b45a11f5a7d53a105bb6cf5927d05086", 1186254833),
	DEMO_ENTRY_EN("mage", "Mages Demo.exe", "82da2565c456dcfb265ded6fe3189c0b", 6638570),
	DEMO_ENTRY("magretfdb", "Magret & FaceDeBouc.exe", "13084274a40527f7879e888856f42442", 507880580),  // En-Fr-De
	DEMO_ENTRY("magretfdb", "Magret_and_facedebouc_DEMO.exe", "ab550f1ae84387e40b8f5ce37136e8d9", 979062133),  // En-Fr-De
	DEMO_ENTRY("magretfdb", "Magret_and_facedebouc_DEMO.exe", "48570b588ecb3fd0325beff5852e94c3", 1758937958),  // En-Fr-De-Es-It-Pt
	DETECTION_ENTRY("maniacmansiondott", "Dott-Villa.exe", "06a03fe35791b0578068ab1873455463", 6536886, Common::DE_DEU, "Kitchen Tech Demo", ADGF_DEMO, 0),
	DETECTION_ENTRY("maniacmansiondott", "dott-tech.exe", "06a03fe35791b0578068ab1873455463", 6488944, Common::DE_DEU, "Laboratory Tech Demo", ADGF_DEMO, 0),
	DEMO_ENTRY_LANG("maniacmansionds", "spyvsspy.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5380409, Common::DE_DEU),
	DEMO_ENTRY_LANG("manoli", "Manoli - Apendiz de Hechicera.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 24460422, Common::ES_ESP),
	DEMO_ENTRY_EN("mash", "mash demo.ags", "af4276d98c9f71dba02658fe95e49b75", 2565641),
	DEMO_ENTRY_EN("mash", "mash demo.exe", "7825fa980fcd058caadaf6d3377b1f78", 5601309),
	DEMO_ENTRY_EN("matttothefuture", "MTTF.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 5560975),
	DEMO_ENTRY("maxfury", "Max Fury .exe", "7db052bc30700d1f30f5330f5814f519", 97498868),  // Windows Eng-Swe
	DEMO_ENTRY("maxfury", "Max Fury .ags", "7af5f159cea8989e05012410cb91020e", 94463712),  // Linux Eng-Swe
	DEMO_ENTRY_EN("melonhead", "MelonHeadDemo.exe", "a26301c89d49476d6b31dd0b8bcb9475", 45078108),
	DEMO_ENTRY_EN("melonhead", "MelonHeadDemo.ags", "00da7b5c5c1f77b020be2f6ae77ba5d3", 42007624),
	DEMO_ENTRY_EN("metaldead", "MetalDead Demo.exe", "3b0874c12fa643efa5fcdc43f647c871", 332921484),
	DEMO_ENTRY_EN("meteortale", "meteor.exe", "72ea6e2b8c5b9a19fc5ac89f6910841a", 16644183),
	DEMO_ENTRY_EN("meteortale", "data", "72ea6e2b8c5b9a19fc5ac89f6910841a", 16644183),
	DEMO_ENTRY_EN("menial", "Menial (demo).exe", "7825fa980fcd058caadaf6d3377b1f78", 18283827),
	DEMO_ENTRY_EN("menial", "Menial (demo).ags", "c828826cc17fda61464ca7cc3d3a7fc0", 15248159),
	DEMO_ENTRY_EN("mi0daementia", "Monkey Island - Zero.exe", "a524cbb1c51589903c4043b98917f1d9", 6561007),
	DEMO_ENTRY("mi0navidad", "Navidad.exe", "735dab7116efa2dfff49ea57e5259223", 2321151),  // Eng-Esp
	DEMO_ENTRY("mi0navidad", "MI Zero - Navidad.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 3642721),  // En-Es-It
	DEMO_ENTRY_LANG("mi12", "Monkey Island.exe", "0500aacb6c176d47ac0f8158f055db83", 3394895, Common::DE_DEU),
	DEMO_ENTRY_LANG("mi25escape", "MI 2.5.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6592143, Common::IT_ITA),
	DEMO_ENTRY_LANG("mi25parque", "Golden Studios.exe", "c6c49c00986ca36933959fa8afa6678b", 6553367, Common::ES_ESP),
	DEMO_ENTRY("mi25parque", "MI2.5 El parque de las Pesadillas.exe", "0b629941253ad4b140bf26a69a170f7a", 7816500),  // Es-It
	DEMO_ENTRY_EN("mi4test", "ags-test3.exe", "0241777c2537fc5d077c05cde10bfa9f", 64061277),
	DEMO_ENTRY_EN("mi5thereturnoflechuck", "demo.exe", "0500aacb6c176d47ac0f8158f055db83", 6235270),
	DEMO_ENTRY_EN("mibaddaytobedead", "Monkey.exe", "f120690b506dd63cd7d1112ea6af2f77", 2117238),
	DEMO_ENTRY_LANG("micv", "MI.exe", "d90febc33f641a5f14c485f8ef29fb33", 3371895, Common::ES_ESP),
	DEMO_ENTRY_LANG("migaiden", "MI Gaiden.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 3513977, Common::IT_ITA),
	DETECTION_ENTRY("miguybrushson", "MIGStrailer.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1870826, Common::IT_ITA, "Trailer", ADGF_DEMO, 0),
	DEMO_ENTRY_LANG("miguybrushson", "MIGSdemo.exe", "0710e2ec71042617f565c01824f0cf3c", 1270524, Common::IT_ITA),
	DEMO_ENTRY_EN("mikelechey", "Mike Lechey.exe", "06a03fe35791b0578068ab1873455463", 2349277),
	DEMO_ENTRY_LANG("mimv", "MI.exe", "c0c6dafd33236a308bd0b87678326994", 1872382, Common::ES_ESP),
	DEMO_ENTRY_EN("minewadv", "Lars MI game.exe", "7f59c1439943d984312dfb08083b13c9", 1869768),
	DEMO_ENTRY_LANG("miprision", "Prision Demo.exe", "6ccc4b7febad420898db72f0c0190875", 3444329, Common::ES_ESP),  //v1
	DEMO_ENTRY_LANG("miprision", "Prision Demo V0.2.exe", "81b72a0500164c15396238a19a8314a4", 3369056, Common::ES_ESP),  //v2
	DEMO_ENTRY("miprision", "Prision.exe", "c3b72132686c4a102b3365d380c8ae3a", 5179581),  // v3 Es-It
	DEMO_ENTRY_EN("mirevealed", "MI2.5.exe", "0710e2ec71042617f565c01824f0cf3c", 2454801),
	DEMO_ENTRY_LANG("misterybigwhoop", "monkyisland-pjt.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2132048, Common::IT_ITA),
	DEMO_ENTRY("mivl", "MI.exe", "23a7b5a6119e82a9750302492b3f2856", 1927604),  // Eng-Esp
	DEMO_ENTRY_LANG("mivoodoo", "MI-WIP.exe", "dd8586ebefd5c457e29b6b9845a576ea", 4306745, Common::DE_DEU),
	DETECTION_ENTRY("mmmatman", "@Man.exe", "3f8a67aaf05f089ecc64d15a0473dfe6", 9679818, Common::DE_DEU, "Trailer", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmm51", "K_ mmmgui_271.exe", "f120690b506dd63cd7d1112ea6af2f77", 7461469, Common::UNK_LANG, "Cut content 1", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmm51", "K_ mmmgui_271.exe", "f120690b506dd63cd7d1112ea6af2f77", 4293149, Common::UNK_LANG, "Cut content 2", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmm51", "[ZENSIERT].exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1013035, Common::UNK_LANG, "Cut content 3", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmm51", "Purpur4.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2423591, Common::UNK_LANG, "Cut content 4", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmm51", "mmmgui_262.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8747304, Common::UNK_LANG, "Cut content 5", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmm51", "PurpusSE.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7934670, Common::UNK_LANG, "Cut content 6", ADGF_DEMO, 0),
	DEMO_ENTRY_LANG("mmm78", "Trailer.exe", "18b284c22010850f79bc5c20054a70c4", 10149256, Common::DE_DEU),
	DEMO_ENTRY_LANG("mmm93", "Moonshine.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6341979, Common::DE_DEU),
	DEMO_ENTRY_LANG("mmmdasexperiment", "DAS EXPERIMENT.exe", "f120690b506dd63cd7d1112ea6af2f77", 4567668, Common::DE_DEU),
	DEMO_ENTRY_LANG("mmmdieeroberung", "brushs-mmm.exe", "c16204dc8aa338e3199b2c62da0b33f4", 4977432, Common::DE_DEU),
	DEMO_ENTRY_LANG("mmmlagrande", "Dave2.exe", "4d7d2addcde045dae6e0363a43f9acad", 7536308, Common::DE_DEU),
	DETECTION_ENTRY("mmmpolicemolest", "PQ.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5556795, Common::DE_DEU, "Trailer 1", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmmpolicemolest", "Trailer.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6917102, Common::DE_DEU, "Trailer 2", ADGF_DEMO, 0),
	DETECTION_ENTRY("mmmwayne", "Police_Molest-SP.exe", "bb59de174d70797d774dec76a171352d", 6740315, Common::DE_DEU, "Trailer", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("monkeywrench", "monkeywrench.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 27454116),
	DEMO_ENTRY_EN("morphine", "Morphine.exe", "0a04523bb2e6fe2b4eb8eee70217ac16", 113435250),
	DEMO_ENTRY("murdercases", "Murder Cases Demo.exe", "e01f39b38593e3f871a803908daa20e7", 431399292),
	DEMO_ENTRY("murdercases", "Murder Cases Demo.exe", "e01f39b38593e3f871a803908daa20e7", 431401071),
	DEMO_ENTRY("murdercases", "Murder Cases Demo.exe", "e01f39b38593e3f871a803908daa20e7", 669629768),
	DEMO_ENTRY_EN("murderinminnesota", "Game.exe", "495d45fb8adfd49690ae3b97921feec6", 14820679),  //v1 Windows
	DEMO_ENTRY_EN("murderinminnesota", "Game.ags", "b4ed9f317081139a925d1b3a0260c1ad", 12340023),  //v1 Linux
	DEMO_ENTRY_EN("murderinminnesota", "Game.exe", "495d45fb8adfd49690ae3b97921feec6", 15258174),  //v2 Windows
	DEMO_ENTRY_EN("murderinminnesota", "Game.ags", "0b73e2b659b1dc266b3155e4d0f624cf", 12777518),  //v2 Linux
	DEMO_ENTRY_EN("murderstarlight", "SherwellHolmes.exe", "0241777c2537fc5d077c05cde10bfa9f", 3917663),
	DEMO_ENTRY_EN("mybigsister", "my big sister.exe", "00a2ae796698e35fa6eb59343973e9ae", 50806260),
	DEMO_ENTRY_EN("mybigsister", "my big sister.exe", "4dc50ca8b5d2c762fe86a528b09973cf", 90464651),
	DEMO_ENTRY_EN("mythicalgambitflawlessfatality", "MGFF.exe", "4887d3dca056a0772741b42f66e018fb", 59711760),
	DEMO_ENTRY_EN("mythicalgambitflawlessfatality", "MGFF.exe", "4887d3dca056a0772741b42f66e018fb", 59710890),
	DEMO_ENTRY_EN("nefasto", "NefAdventure.exe", "1f9e4ca7abc1833fd7a674842567df0c", 101421026),  // v2.01
	DEMO_ENTRY_EN("neofeud", "neofeud demo.exe", "6e861b1f476ff7cdf036082abb271329", 1886913453),
	DEMO_ENTRY_EN("neofeud", "neofeud.exe", "6e861b1f476ff7cdf036082abb271329", 2078743282), // 1.3 itch.io
	DEMO_ENTRY_EN("neofeud", "Neofeud_LINUX_TEST_9_2_17.ags", "a23f0c48d8128bb7045ab8685665a464", 2076260110), // itch.io Linux
	DEMO_ENTRY_EN("nightmare", "B&B.exe", "465f972675db2da6040518221af5b0ba", 1521168),
	DEMO_ENTRY("nightmareframes", "Nightmare Frames Prologue.exe", "046f9ddd1ebe44b01417ab1f55dd0f45", 894156266),  // Steam/itch.io Eng-Esp
	DEMO_ENTRY("nightmareframes", "Nightmare Frames Prologue.exe", "1965ea84b883d67c781a7e9dc3427e6a", 894702132),  // Steam/itch.io Eng-Esp
	DEMO_ENTRY_EN("nightoftheravingfeminist", "Prueba1.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 7015243),
	DEMO_ENTRY_EN("nocturnedream", "Nocturne SHAREWARE EMULATOR.exe", "48f8924122df461dffa8232af153f9cd", 294829702),
	DEMO_ENTRY_EN("nocturnedream", "Nocturne SHAREWARE EMULATOR.exe", "48f8924122df461dffa8232af153f9cd", 294832172),
	DEMO_ENTRY_EN("noirbois", "Noir Bois Prelude Episode 1.exe", "4d5d25446373d4df71c6bda77a15230a", 5167731),
	DEMO_ENTRY_EN("notravellerreturns", "Test.exe", "f120690b506dd63cd7d1112ea6af2f77", 19656975),
	DEMO_ENTRY_EN("nostalgik", "NOLGASTIK.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 143891265),
	DEMO_ENTRY_EN("objectdreams", "The Object of Dreams.exe", "87f211ec30ea7f1d2ed6d223c3a06108", 25604789),
	DEMO_ENTRY_EN("oldskies", "OldSkies.exe", "0bdccc3f90f28de88de050672565cb5a", 1577321264), // Steam Windows
	DEMO_ENTRY_EN("oldskies", "OldSkies.exe", "820aa7e78063aa8aaf5e7f495472089a", 1580456994), // Steam Windows v0.1.7.9
	DEMO_ENTRY_EN("oldskies", "ac2game.dat", "008b1273459fa04425423c48ef655385", 1573980025), // Steam Mac
	DEMO_ENTRY_EN("oott", "demo_tkc.exe", "54da0ce99f3e8b556db6d236c8ef76ba", 415137576), // Steam
	DEMO_ENTRY_EN("osdlgs", "OSD - Leather Gear Smooth.exe", "82da2565c456dcfb265ded6fe3189c0b", 114550556),
	DEMO_ENTRY_EN("osdrots", "Rise of the spirit.exe", "415faaf03c892f5a8ce2c136a8fa3478", 17049026),
	DEMO_ENTRY_EN("outscore", "Outscore.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 31265367),
	DEMO_ENTRY_EN("outscore", "Outscore.exe", "dbe281d93c914899886e77c09c3c65ec", 39079910),  // v1.0.3
	DEMO_ENTRY_EN("paradiseroad", "Paradise Road Demo.ags", "240c2034ea6d40ad9762906da7193ed1", 179263331),
	DEMO_ENTRY_EN("paramnesia", "Paramnesia.exe", "7b665533f5d1fe08284fd9dbff395b06", 20233668),
	DEMO_ENTRY_EN("patchesspazcat", "CatGame.exe", "0736c98683a5ee6179edd886ca674909", 45161300),
	DEMO_ENTRY_EN("pengaobcster", "PAO.exe", "f45c6354507e154938bdb0f55f9f9d7a", 294757192),
	DEMO_ENTRY_EN("perfecttidess2s", "s2s.exe", "95ffa47057b9906869a04cf95d264629", 531651451),
	DEMO_ENTRY_EN("perfecttidess2s", "game.ags", "1be40c752a39ae37d7a9f5420139ea14", 528389246), // Mac Steam demo
	DEMO_ENTRY_EN("persephonediary", "royce.exe", "a524cbb1c51589903c4043b98917f1d9", 4972848),
	DEMO_ENTRY_EN("personalrocket", "personalrocket-demo.exe", "7971a7c02d414dc8cb33b6ec36080b91", 25732293),  // Windows
	DEMO_ENTRY_EN("personalrocket", "PersonalRocket-DemoEng.exe", "0564de07d3fd5c16e6947a647061913c", 30780417),  // Windows
	DEMO_ENTRY_EN("personalrocket", "PersonalRocket-DemoEng.ags", "16a219bf49194b5da735c846f01fc6c9", 27747821),  // Linux
	DEMO_ENTRY_LANG("personalrocket", "Personal Rocket - Demo Esp. Latino.exe", "0564de07d3fd5c16e6947a647061913c", 26248104, Common::ES_ESP),  // Windows Esp
	DEMO_ENTRY_LANG("personalrocket", "Personal Rocket - Demo Esp. Latino.ags", "86f92203d2bbfc9d89f4bb9130fcf15e", 23215508, Common::ES_ESP),  // Linux Esp
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 96534651),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 96536204),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 96799674),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 96799833),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 96800964),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 430357136),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 505116629),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "71961f24fd94e30e61167d7b0720cd35", 3262982892),
	DEMO_ENTRY_EN("phantomfellows", "TPF.exe", "6675152efd834c75aa65ea35ad7ed0d6", 3263463494),
	DEMO_ENTRY_EN("phantomfellows", "TPF.ags", "9e8ca56e0a85a5d75ae4f6fd1f6e3e21", 502058945),  // Linux
	DEMO_ENTRY_EN("phonoi", "phonee.ags", "111794f6fe829a478dc9a7d4cea239c8", 237652019),  // v0.2
	DEMO_ENTRY_EN("platformerius", "platformerius.exe", "a3ad5fa6463c0116a2ac8986841860e0", 1512479),
	DEMO_ENTRY("politicalenemy", "Political Enemy.exe", "387ff720e746ae46e93f463fd58d77a4", 205108968),  // Eng-Ita
	DEMO_ENTRY_EN("pq4sciish", "PQ4 Remake Demo.exe", "0564de07d3fd5c16e6947a647061913c", 6203078),
	DEMO_ENTRY_EN("primordia", "primordia.exe", "22313e59c3233001488c26f18c80cc08", 727898212),
	DEMO_ENTRY("projectdevonia", "PDM.ags", "7515c1b5f6e98027ee9fa146312609e1", 29295402),
	DEMO_ENTRY_EN("prototypical", "Proto.exe", "06a03fe35791b0578068ab1873455463", 48507319),
	DETECTION_ENTRY("proxecto", "proxec.exe", "465f972675db2da6040518221af5b0ba", 2636209, Common::UNK_LANG, "Demo/Galician", ADGF_DEMO, 0),
	DETECTION_ENTRY("proxecto", "ac2game.ags", "03f06112d270c9108ca618ab7b3bd735", 2116513, Common::UNK_LANG, "Demo/Galician", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("pubmasterquest", "pub master quest [demo].exe", "e1676318c8a040fcf508b817013dc8fe", 23431689),
	DEMO_ENTRY_EN("puzzlebots", "puzzlebots_demo.exe", "34b49df9cf6eadb5c3587b3921d5b72f", 354138961),
	DEMO_ENTRY_EN("qfheroes", "Bad neighbour.exe", "06a03fe35791b0578068ab1873455463", 9672173),
	DEMO_ENTRY("quantumnauts", "qndemo.exe", "aeb2dd29e5ff839cb3ee86cf3e87e3ca", 134237367),  // Eng-Ita
	DEMO_ENTRY_EN("qfi", "InfamyDemo2.exe", "06a03fe35791b0578068ab1873455463", 8121980),
	DEMO_ENTRY_EN("qfi", "qfi_demo2.exe", "9e7eb04bb1652e188e6f2633eed89408", 82314927),
	DEMO_ENTRY_EN("qfi", "QFI_DEMO.exe", "5913a8caa5969f9d781eca41f2b876fc", 47150193),
	DEMO_ENTRY_EN("rainblood", "Rain of Blood.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 45617879),
	DEMO_ENTRY_EN("ratpackcastle", "The Rat Pack Do Cheesy Castle.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 336087676),
	DEMO_ENTRY_EN("realitycheck4", "Reality Check 4.exe", "82da2565c456dcfb265ded6fe3189c0b", 16426064),
	DEMO_ENTRY_EN("recollection", "RECOLLECTION.exe", "06a03fe35791b0578068ab1873455463", 5806558),
	DEMO_ENTRY_EN("recollection", "MAGS0907.exe", "06a03fe35791b0578068ab1873455463", 5806558),
	DEMO_ENTRY_EN("redbow", "Red Bow.exe", "2971fd82315ce4df9d8287def34b4713", 21338542), // itch.io
	DEMO_ENTRY_EN("reddwarf", "Red Dwarf.exe", "06a03fe35791b0578068ab1873455463", 2812268),
	DEMO_ENTRY_EN("resonance", "resonance_demo.exe", "2e635c22bcbf0ed3d46f1bcde71812d4", 473386791),
	DEMO_ENTRY_EN("retaliation", "Ron Retaliation.exe", "0500aacb6c176d47ac0f8158f055db83", 3135809),
	DEMO_ENTRY_EN("retroron", "retroron.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 3762985),
	DEMO_ENTRY_EN("revenants", "Horror.exe", "88cf59aad15ca331ab0f854e16c84df3", 8887972),
	DEMO_ENTRY_EN("richardandalice", "richardandalicedemo.exe", "1a88ec06b59a6b56ef6d7c489bd71008", 86273262),
	DEMO_ENTRY_EN("rnrneverdies", "Rock 'n' Roll Will Never Die DEMO.exe", "bd5bf52feb42580afbcf4d5650208617", 14046564),
	DEMO_ENTRY_EN("rongyptian", "Walk Like a RoNgyptian.exe", "2ca80bd50763378b72cd1e1cf25afac3", 18481800),
	DETECTION_ENTRY("rongyptian", "Walk Like a RoNgyptian.exe", "2ca80bd50763378b72cd1e1cf25afac3", 24139016, Common::EN_ANY, "Music Video", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("rontimeouttrailer", "ronTOintro.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 2804752),
	DEMO_ENTRY_EN("rontimeouttrailer", "ronTOintro.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 2804573),
	DEMO_ENTRY_EN("ron5thanniversary", "5thpromo.exe", "f120690b506dd63cd7d1112ea6af2f77", 1620446),
	DEMO_ENTRY_EN("rosewater", "Rosewater.exe", "0b5e8f44bd102fd4f799e904de96f621", 459525303),
	DETECTION_ENTRY("rotla", "ROTLA DEMO V1.0 ENG.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 7788105, Common::EN_ANY, "Tanis Demo", ADGF_DEMO, 0),
	DETECTION_ENTRY("rotla", "ROTLA.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 7789189, Common::ES_ESP, "Tanis Demo", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("rotla", "ROTLA.exe", "82da2565c456dcfb265ded6fe3189c0b", 12126185),  // Eng-Esp
	DEMO_ENTRY_EN("roughdiamond", "game.exe", "06a03fe35791b0578068ab1873455463", 4419666),
	DEMO_ENTRY_LANG("rtmi", "RMI.exe", "465f972675db2da6040518221af5b0ba", 26481971, Common::IT_ITA),  //v1
	DEMO_ENTRY_LANG("rtmi", "RMI v2.0.exe", "b8df5e3b23c15292f7f37f1c06fa78a2", 32304300, Common::IT_ITA),  //v2
	DETECTION_ENTRY("samaritan", "samaritan.exe", "615e73fc1874e92d60a1996c2330ea36", 8714007, Common::EN_ANY, "Prototype Demo", ADGF_DEMO, 0),
	DEMO_ENTRY_EN("samaritan", "samaritan.exe", "615e73fc1874e92d60a1996c2330ea36", 58948145),  // Screen7 website
	DEMO_ENTRY_EN("screamingsouls", "Scremn.exe", "d3bc2cf3a0f72f0fbe37d9edbd5ebec3", 1800422),
	DEMO_ENTRY_EN("searchforsanity", "sfs.exe", "308d35bc34e9df29d8acce615593e3e7", 9097147),  // Win (AGS 2.5)
	DEMO_ENTRY("shadowsoftheempire", "sote_td.exe", "b8cd640b45c5a79c33c7a8a8fe32ebe2", 63246884),  //v1.0  En-It
	DEMO_ENTRY("shadowsoftheempire", "sote_td.exe", "b8cd640b45c5a79c33c7a8a8fe32ebe2", 63299246),  //v1.1  En-De-It
	DEMO_ENTRY_EN("shardlight", "shardlight demo.exe", "ee801fba52d252249677a9170bd2db96", 642405010), // Steam
	DEMO_ENTRY_EN("shardlight", "shardlight demo.exe", "ee801fba52d252249677a9170bd2db96", 642366257),
	DEMO_ENTRY_EN("shardsofgod", "Shards of God.ags", "b181201165d078ac786fa474ef8c14fd", 36754512),
	DEMO_ENTRY_EN("shivah", "shivahdemo.exe", "6e3d6225dee662ff6450a3bfa942773b", 20897850),
	DEMO_ENTRY_EN("shivahkosher", "shivah-demo.exe", "0aaf5445a3544a631d6e7dd4561fc7ae", 23987292),
	DEMO_ENTRY_EN("signalecho", "Signal & Echo - Iris is Missing (demo).exe", "f9e3e0193fbc975daa3a7c95edf04e0a", 39177267),
	DEMO_ENTRY_EN("signalecho", "Signal & Echo - Iris is Missing (demo).exe", "f9e3e0193fbc975daa3a7c95edf04e0a", 39173586),
	DEMO_ENTRY_EN("simonthesorcerer3", "simon3.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8189928),
	DEMO_ENTRY_EN("simonthesorcerersbrother", "stsb-trailer.exe", "06a03fe35791b0578068ab1873455463", 2283838),
	DEMO_ENTRY_EN("skytowerrescue", "STR.exe", "2b7ae26333ba15529a8bbbdb49605133", 8630945),
	DEMO_ENTRY_EN("slazakloss", "Slazak.exe", "7633f0919378749cebf6dacba61ca0e1", 58494797),  // itch.io
	DEMO_ENTRY_EN("slazakloss", "Slazakd.exe", "b5478748a11747ecd3185a9c76781eff", 58121492),  // Steam
	DEMO_ENTRY_EN("smallsister", "sister.exe", "78dd4ca028ee0156b6a093d6d780aa65", 18528035),
	DEMO_ENTRY_EN("smgilbert", "s&m2_demo_ENG.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 4141303),
	DEMO_ENTRY_LANG("smgilbert", "s&m2_demo.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 4268861, Common::IT_ITA),
	DEMO_ENTRY_EN("smileordie", "SmileOrDie.exe", "06a03fe35791b0578068ab1873455463", 12740162),
	DEMO_ENTRY_EN("snowproblem", "Snow Problem Demo V3.exe", "fae163b58c16e194688727d0903684be", 5047803),  //v3
	DEMO_ENTRY_EN("snowproblem", "Snow Problem.exe", "fae163b58c16e194688727d0903684be", 5139172),  //v4
	DEMO_ENTRY_LANG("spacefreakers", "Space Freakers v2.exe", "7a3096ac0237cb6aa8e1718e28caf039", 64527091, Common::FR_FRA),
	DEMO_ENTRY_EN("spacepirates", "Space Pirates.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 3006210),
	DEMO_ENTRY_EN("spacesim", "space_3d.exe", "8538afa638531020f79df88aec0fb797", 1667537),
	DEMO_ENTRY("spacetrash", "SpaceTrash.exe", "615e73fc1874e92d60a1996c2330ea36", 7532576),  // En-Fr
	DEMO_ENTRY_EN("sphonx", "Sphonx.exe", "7e4ab16deb62ffe46b9da2229672845c", 289696634),  // Windows  Tech Demo (older)
	DEMO_ENTRY_EN("sphonx", "Sphonx.ags", "943c562d8a7d1c0266e9ae4aa9d955c7", 286649190),  // Linux    Tech Demo (older)
	DEMO_ENTRY("sphonx", "Sphonx.exe", "7e4ab16deb62ffe46b9da2229672845c", 289700610),     // Windows  Tech Demo Eng/Deu
	DEMO_ENTRY("sphonx", "Sphonx.ags", "d51d6d1c979b7571c81b45f681be58da", 286653166),     // Linux    Tech Demo Eng/Deu
	DEMO_ENTRY_EN("sphonx", "Sphonx - Demo.exe", "7e4ab16deb62ffe46b9da2229672845c", 279952170),  // Windows  Demo
	DEMO_ENTRY_EN("sphonx", "Sphonx - Demo.exe", "80f839db9d4ee450fff2eaefcf0fb9df", 280677499),  // Windows  Demo
	DEMO_ENTRY_EN("sphonx", "Sphonx - Demo.ags", "47fe5f46944eba084fcbf812ffe8304f", 276904726),  // Linux    Demo
	DEMO_ENTRY_EN("sphonx", "Sphonx - Demo.ags", "7dd66185d0df199783d2f0f893e2c5f6", 277551719),  // Linux    Demo
	DEMO_ENTRY_EN("splendorsolis", "Splendor Solis #1.ags", "dd87d9b18bd1824d10829610c37aba4c", 3186720),
	DEMO_ENTRY_EN("splendorsolis", "Splendor Solis #1.exe", "03741a77615d6ae1bf2cfb9e7dc8d347", 6294580),
	DEMO_ENTRY_EN("spoonsiii", "spoonsIII.exe", "2ca80bd50763378b72cd1e1cf25afac3", 7316512),
	DEMO_ENTRY_EN("sq2svga", "sq2svga.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3245872),
	DEMO_ENTRY_EN("sq3vga", "sq3vgademo.exe", "64fcaf7da0b257ea831f89c54be0ad72", 3630019),
	DEMO_ENTRY("sq55", "sq5.5.exe", "465f972675db2da6040518221af5b0ba", 16342443),  // Eng-Rus
	DEMO_ENTRY_EN("sqos", "SQOS v04.exe", "82da2565c456dcfb265ded6fe3189c0b", 2621410),
	DEMO_ENTRY_EN("sram2", "sram2.exe", "e3a33d139d90f2e695292a618753b8a5", 296499943),  // Windows
	DEMO_ENTRY_EN("sram2", "sram2.ags", "f55362aed016ba06b3488b7162199853", 293852887),  // Linux
	DEMO_ENTRY_EN("stablepeteandthejoust", "StablePete.exe", "b142b43c146c25443a1d155d441a6a81", 30046740),  // v1.0
	DEMO_ENTRY_EN("stablepeteandthejoust", "StablePete.exe", "b142b43c146c25443a1d155d441a6a81", 30048075),  // v1.1
	DEMO_ENTRY("stargateatlantis", "Atlantis.exe", "3b5285594848a90298056cfeda4b2074", 13706734),  // En-Fr
	DEMO_ENTRY_EN("startrekmansion", "ST_BTTM.exe", "615e73fc1874e92d60a1996c2330ea36", 42877388),
	DEMO_ENTRY_EN("startropy", "Startropy.exe", "86cc8cd9b7443b68a374ad5d002c2945", 333959797),
	DEMO_ENTRY("stellarmessep1", "StellarMessShortDemo.exe", "a409703089eebbcfa13f0a22f6fb71ed", 8067581),  // En-Fr-De-Es
	DEMO_ENTRY("stellarmessep1", "StellarMessDemo.exe", "a409703089eebbcfa13f0a22f6fb71ed", 8713074),  // En-Es
	DEMO_ENTRY_EN("stickmanrpg", "Stickman RPG.exe", "465f972675db2da6040518221af5b0ba", 2030693),
	DEMO_ENTRY_EN("stormwater", "Stormwater.ags", "0a27416e82f7e740fc0715c1e54d70d7", 255136655),
	DEMO_ENTRY_EN("stormwater", "Stormwater.ags", "5d7ca223367b9b24cb251a4123605cd4", 255141618),  // v1.01
	DEMO_ENTRY_EN("stormwater", "Stormwater.ags", "4181bf294827dd75d63454ca1bde72ca", 313534600),  // v1.1
	DEMO_ENTRY_EN("stormwater", "Stormwater.ags", "7ef4e7b811998c1174cbb1082e4ef0f5", 332578704),  // v1.2
	DEMO_ENTRY_EN("stuckathome", "shtrl1.exe", "0500aacb6c176d47ac0f8158f055db83", 819147),
	DEMO_ENTRY_EN("subterra", "SUBTERRA.exe", "fd3ecd8289bebadbf775fe8a13b9c5d7", 17981991),
	DEMO_ENTRY("sulifallenharmony", "Suli Fallen Harmony - Demo.exe", "8a58836830dce896e4366f57791ab6cc", 8822781), // En-Fr
	DEMO_ENTRY("sulifallenharmony", "Suli Fallen Harmony - Demo.exe", "8a58836830dce896e4366f57791ab6cc", 8826816), // itch.io En-Fr
	DEMO_ENTRY_EN("superjazzman", "sjmdemo.exe", "0710e2ec71042617f565c01824f0cf3c", 5214882),
	DEMO_ENTRY_EN("supernaturaltt", "sam&dean.exe", "06a03fe35791b0578068ab1873455463", 6276225),
	DEMO_ENTRY("tales", "tales.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 68615667),  // Tech demo
	DEMO_ENTRY("tales", "tales.exe", "4f6c7ec127e8b0ce077abb357903612f", 112930689), // Eng-Ita Steam
	DEMO_ENTRY("tales", "tales.exe", "4f6c7ec127e8b0ce077abb357903612f", 112930701), // Eng-Ita Official website
	DEMO_ENTRY_EN("technobabylon", "technobabylon.exe", "e523535e730f313e1ed9b19b1a9dd228", 595787927),
	DEMO_ENTRY_EN("technobabylon", "technobabylon.exe", "e523535e730f313e1ed9b19b1a9dd228", 59578792),
	DEMO_ENTRY_EN("thatday", "ThatDay.exe", "b142b43c146c25443a1d155d441a6a81", 9534366),
	DEMO_ENTRY_EN("theartifact", "artifact-demo.exe", "465f972675db2da6040518221af5b0ba", 161590301),
	DEMO_ENTRY_EN("thecatlady", "Cat Lady.exe", "92320e20e3d4c70a94d89e2f797e65c1", 752683721),  // v1.1 Screen7
	DEMO_ENTRY("thedarktrial", "dark trial demo.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 36442694),  // Eng-Hun
	DEMO_ENTRY_EN("theeternalnight", "Hotel.exe", "615e73fc1874e92d60a1996c2330ea36", 29586297),
	DEMO_ENTRY_EN("thejourneyofiesir", "The Journey of Iesir.exe", "376a3f162c7940d990325c53edc20fed", 70444514),
	DEMO_ENTRY("theloneloser", "demo (english).exe", "0500aacb6c176d47ac0f8158f055db83", 6082095), //Eng-Ita
	DEMO_ENTRY_EN("themajesticconspiracy", "majestic.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 17929647),
	DEMO_ENTRY_EN("theoccultist2", "Occultist2-BUTW.exe", "b781cc4ab0fbe647615fafa4830ec308", 8571116),  // Windows
	DEMO_ENTRY_EN("theoccultist2", "Occultist2-BUTW.ags", "50198f2880d4f8334a74051fa5591192", 5404376),  // Linux
	DEMO_ENTRY_EN("theperfectmurder", "Tamz.exe", "9cf51833e787cc919837d9a8bd8fc14c", 4527709),
	DEMO_ENTRY_EN("theropods", "Theropods - Demo.exe", "bd4f8ebbab2893ccd54ea0e7ca4334a7", 445439739),  // Steam
	DEMO_ENTRY_EN("theropods", "theropods-demo.exe", "e4e47802c282a743fb269794e8bf0fe9", 445252923),  // Gamejolt
	DEMO_ENTRY_EN("theschool", "MOTPSTE.exe", "89a94326c8afd9e0234e269bd7330130", 20009231),
	DEMO_ENTRY_EN("thesearch", "xn--srts.exe-n4aa", "465f972675db2da6040518221af5b0ba", 5465638),
	DEMO_ENTRY("thesecretsofjesus", "the secrets of jesus.exe", "2e17ab52586ad34b5e597c20a59f60a0", 841450335), // Steam/itch.io Windows En-De
	DEMO_ENTRY("thesecretsofjesus", "The secrets of Jesus.ags", "ed790525ab59cfc727a35582731de9c1", 838622031), // itch.io Linux
	DEMO_ENTRY_LANG("thesecretofmountmonkey", "Mmonk.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6386620, Common::DE_DEU),  //v2
	DEMO_ENTRY_LANG("thesecretofmountmonkey", "Mmonk.exe", "0710e2ec71042617f565c01824f0cf3c", 3309022, Common::DE_DEU),  //v3
	DEMO_ENTRY_LANG("threepwoodnightmare", "Threepwood's Nightmare.exe", "06a03fe35791b0578068ab1873455463", 15225289, Common::IT_ITA),
	DEMO_ENTRY_EN("timegentlemenplease", "tgp.exe", "86a5359bac7c88f0dfa060478800dd61", 29686006),
	DEMO_ENTRY_EN("timetrial", "Time Trial.exe", "01823d511cc00f4de6fd920eb543c6e7", 18285829),
	DEMO_ENTRY_LANG("trapodroz", "Tytus.exe", "792c0a0eaeba1a8846cb7b1af1e3266e", 9082265, Common::PL_POL),
	DEMO_ENTRY_LANG("trapodroz", "Tytus.ags", "faf3a0941b78987e59339e550ed6c7b6", 6045573, Common::PL_POL),
	DEMO_ENTRY("trexmusclesam2", "T-REX and Muscle Sam A new KickStart.exe", "5f4c73f40a7e3eb4c90792cc58e38ca1", 837184670),  // Eng-Ita
	DEMO_ENTRY_EN("trollsong", "Troll Song Verse One Demo.exe", "d25c3cb8a42c5565634dfef76b3cf83e", 3504405),
	DEMO_ENTRY_EN("troopers", "Troopers.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6569716),
	DEMO_ENTRY_EN("troublecorner", "artfight2020.exe", "4d5d25446373d4df71c6bda77a15230a", 317113070),
	DEMO_ENTRY_EN("troublecorner", "artfight2020.exe", "4d5d25446373d4df71c6bda77a15230a", 413763264),
	DEMO_ENTRY_LANG("tsomi2", "The Secret of Monkey Island 2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5075308, Common::ES_ESP),
	DEMO_ENTRY_LANG("ultimalatinvii", "UltimaVIILatin1.0.exe", "63f8a60cc094996bd070e68cb3c4722c", 83843416, Common::ES_ESP),  // v1.00 demo/beta
	GAME_ENTRY_LANG_PLATFORM("ultimalatinvii", "UltimaLauncher.exe", "63f8a60cc094996bd070e68cb3c4722c", 9065961, Common::ES_ESP, "Launcher"),
	DEMO_ENTRY_EN("untilihaveyou", "until i have you.exe", "cda1d7e36993dd55ba5513c1c43e5b2b", 457842687),
	DEMO_ENTRY_EN("untilihaveyou", "Until I Have You Demo.exe", "cda1d7e36993dd55ba5513c1c43e5b2b", 479749741),
	DEMO_ENTRY_EN("updatequest", "U-Quest.exe", "06a03fe35791b0578068ab1873455463", 1776804),
	DEMO_ENTRY_EN("valis", "valis 0.70.exe", "0710e2ec71042617f565c01824f0cf3c", 4348394),
	DEMO_ENTRY_LANG("villard", "VMF_DEMO 2.0.1.exe", "534f936a0fac0c6f72b6ce6aaa6c9ce2", 653406938, Common::IT_ITA),
	DEMO_ENTRY_LANG("vohaulsrevenge2", "SQ XII.exe", "465f972675db2da6040518221af5b0ba", 17313307, Common::RU_RUS),
	DEMO_ENTRY_EN("vorezkor", "The VOREZKOR Hack.exe", "f3c6c85fd477033f539b95515fdf8520", 119247356),  // v0.21
	DEMO_ENTRY_EN("vorezkor", "The VOREZKOR Hack.ags", "4b834efb8e9691b22eb010590e12a259", 116135400),
	DEMO_ENTRY_EN("vorezkor", "The VOREZKOR Hack.exe", "f3c6c85fd477033f539b95515fdf8520", 184231232),  // v0.83
	DEMO_ENTRY_EN("wallyweasel", "wallydemo.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 12579444),
	DEMO_ENTRY_EN("waitingfortheloop", "WaitingForTheLoopDemo.exe", "0241777c2537fc5d077c05cde10bfa9f", 48581712),
	DEMO_ENTRY_EN("welcometodarklake", "mystery.exe", "18b284c22010850f79bc5c20054a70c4", 5438912),
	DEMO_ENTRY_EN("whattimeisit", "Wtii-demo.exe", "13a143130096b8d6baae1a7ccca52ac4", 4044338),  // 2002-09-09
	DEMO_ENTRY_EN("whiskeyweather", "sword-denz-wjam2013.exe", "19467230b9a39aea9f711e83ccf4c984", 14336391),
	DEMO_ENTRY_EN("wingman", "wingman.exe", "e98b37edea8765aa9afe6d1434767d68", 540112243),
	DEMO_ENTRY_EN("withoutaprayer", "prayerDemo.exe", "28f82e420b82d07651b68114f90223c8", 8745149),
	DEMO_ENTRY_EN("wolfcountry", "MYST.exe", "3128b9f90e2f954ba704414ae854d10b", 4633153),
	DEMO_ENTRY_EN("wretcher", "Wretcher.exe", "80076b2aad3170f5781c6a8a69a457fe", 20299824),
	DEMO_ENTRY_EN("writersblocks", "writersblocks.exe", "2cb9c81a20282b55f69343ef95cbb63d", 16118235),
	DEMO_ENTRY_EN("zakseamonster", "ZMALSM.exe", "82da2565c456dcfb265ded6fe3189c0b", 31849022),
	DEMO_ENTRY_EN("zankrobot", "mwhour.exe", "4d17844029d8910fbaae1bdc99e250f2", 3819086),
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 137547310),  // 2021-05-17
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 182647940),  // 2022-01-30 (carnival test)
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 204101878),  // 2022-02-26 (strongman test)
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 209576608),  // 2022-10-27
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 212459343),  // 2022-11-12
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 228487627),  // 2023-01-01
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 232891359),  // 2023-01-31
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 256734571),  // 2023-02-28
	DEMO_ENTRY_EN("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 408106212),  // 2024-07-22
	DETECTION_ENTRY("zapandy", "ZapNAndy.exe", "17009da9820f5aa86d0588023d497db8", 206168650, Common::EN_ANY, "Theater Mode Test", ADGF_DEMO, 0),  //En-It
	DEMO_ENTRY("zidjourney", "ctgame2.ags", "b00498666dd31c33f1df366a88503e01", 61183594),  // Eng-Pol Win/Linux  v1.0.1 2022-10-17
	DEMO_ENTRY("zidjourney", "ac2game.dat", "c354124b98ec4b1407ce5852f4d151c3", 64801406),  // Eng-Pol Mac  v1.0.1 2022-10-17
	DEMO_ENTRY("zidjourney", "ctgame2.exe", "c354124b98ec4b1407ce5852f4d151c3", 64808438),  // Eng-Pol Win  v1.0.2 2022-10-22
	DEMO_ENTRY("zidjourney", "ctgame2.ags", "47b1302e78e89712caa9691135d8a5a7", 61190626),  // Eng-Pol Linux  v1.0.2 2022-10-22
	DEMO_ENTRY("zidjourney", "ac2game.dat", "c354124b98ec4b1407ce5852f4d151c3", 64808438),  // Eng-Pol Mac   v1.0.2 2022-10-22
	DEMO_ENTRY("zniwadventure", "ctgame.exe", "b47f0434e11532ec79c330b44ee05130", 10641139),  // Eng-Pol
	DEMO_ENTRY_EN("zombieattack", "zademo.exe", "82da2565c456dcfb265ded6fe3189c0b", 20958555),

	GAME_ENTRY_EN("10waysfromsunday", "10waysfromsunday.exe", "495d45fb8adfd49690ae3b97921feec6", 11362765),
	GAME_ENTRY_EN("10waysfromsunday", "10waysfromsunday.exe", "495d45fb8adfd49690ae3b97921feec6", 11362850),
	GAME_ENTRY_EN("11-11-11", "APOCA.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 286278821),  // v1.1.1
	GAME_ENTRY_EN("12hoursslave", "12hoursaslave.exe", "495d45fb8adfd49690ae3b97921feec6", 42637805),
	GAME_ENTRY_EN("15minutes", "15 minutes.exe", "615e73fc1874e92d60a1996c2330ea36", 24136635),  // v1.01
	GAME_ENTRY_EN_PLATFORM("15minutes", "15 minutes.exe", "615e73fc1874e92d60a1996c2330ea36", 24124810, "MAGS"),  // v1.0
	GAME_ENTRY_EN("30minutes", "30minutes.exe", "18f5fd85de78efca16c7bafce54e3f63", 17930417),
	GAME_ENTRY_EN("3minfart", "3DAF.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 745733),
	GAME_ENTRY_EN("1dayamosquito", "mosquito.exe", "465f972675db2da6040518221af5b0ba", 2178983),
	GAME_ENTRY_EN("1000hrpg", "1000Hour.exe", "0b7529a76f38283d6e850b8d56526fc1", 833958),
	GAME_ENTRY_EN("2000ways", "hourgame.exe", "4d17844029d8910fbaae1bdc99e250f2", 1707319),
	GAME_ENTRY_EN("2034acaftercanada1", "MAGS_01_13.exe", "1280ba7c269a68a9505871516319db0c", 14123278),
	GAME_ENTRY_EN("2034acaftercanada2", "2034 ac ii.exe", "1280ba7c269a68a9505871516319db0c", 35207006),
	GAME_ENTRY_EN("2080wallcity", "2080-WCL.ags", "2e45352cdcfce13975ab7aaa1d77b3b4", 44515224),
	GAME_ENTRY_EN("24hours", "24.exe", "f120690b506dd63cd7d1112ea6af2f77", 1932370),
	GAME_ENTRY_EN("3pigsandawolf", "3piggiesalpha.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 14181120),
	GAME_ENTRY_EN("3pigsandawolf", "three little pigs and a wolf.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 14180634),
	GAME_ENTRY_EN("46memorylane", "diyu.exe", "e3962995a70923a8d5a8f1cf8f932eee", 66686277),
	GAME_ENTRY_EN("4lungboy", "4LungBoy.exe", "615e73fc1874e92d60a1996c2330ea36", 91014705),
	GAME_ENTRY_EN("4ofclubs", "4ofclubs.exe", "06a03fe35791b0578068ab1873455463", 5909169),
	GAME_ENTRY_EN("5daysastranger", "5days.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3904094),
	GAME_ENTRY_EN("5daysastranger", "5days.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 4440143), // v1.3
	GAME_ENTRY_EN_PLATFORM("5daysastranger", "5days.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4614351, "Special Edition"),
	GAME_ENTRY_PLATFORM("5daysastranger", "5daysorig.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4593115, "Alt/Multilanguage"),
	GAME_ENTRY_EN("5dragons", "5drag.exe", "e80586fdc2db32f65658b235d8cbc159", 4741670),
	GAME_ENTRY_EN("5oclocklock", "dadgame.exe", "3018c5443291aec823bc63342ce4c58b", 6073887),
	GAME_ENTRY_EN("616way", "616mockorangeway.exe", "aca1e820d633b0da0162ce128e29ccec", 59521950),
	GAME_ENTRY_EN("6174solitaire", "Game6174.exe", "01534b6a57fcdb1a57486f5c24120124", 5863226),  // Windows
	GAME_ENTRY_EN("6174solitaire", "Game6174.ags", "fedabeeb5a70acafd4f46b73238c9ec6", 2401062),  // Linux
	GAME_ENTRY_EN("6dayblah", "6dayblah.exe", "4d17844029d8910fbaae1bdc99e250f2", 1980724),
	GAME_ENTRY_EN("6daysasacrifice", "6das.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7993899),
	GAME_ENTRY_EN_PLATFORM("6daysasacrifice", "6das se.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8173314, "Special Edition"),
	GAME_ENTRY_EN("6mornings", "project2.exe", "e7dac058b9bc0b42d489e474c2ddec84", 11595240),
	GAME_ENTRY_EN("7daysaskeptic", "7days.exe", "465f972675db2da6040518221af5b0ba", 4691024),
	GAME_ENTRY_EN("7daysaskeptic", "7days.exe", "465f972675db2da6040518221af5b0ba", 4693374),
	GAME_ENTRY_EN_PLATFORM("7daysaskeptic", "7days.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4863356, "Special Edition"),
	GAME_ENTRY_EN("9hourstodawn", "9h2d.exe", "94d1b64c658b4731141e4482c88f0d79", 110935816),
	GAME_ENTRY_EN("9monthsin", "9 Months In.exe", "7407eea3a8a88cca77c59f0a3a400f36", 33372910),
	GAME_ENTRY_EN("abducted10mins", "10 minutes.exe", "465f972675db2da6040518221af5b0ba", 2687034),
	GAME_ENTRY_EN("abducted10mins", "10 minutes.exe", "465f972675db2da6040518221af5b0ba", 2688871),
	GAME_ENTRY_EN("abduction", "Abduction.exe", "a524cbb1c51589903c4043b98917f1d9", 144332695),  // v1
	GAME_ENTRY_EN("abduction", "Abduction.exe", "a524cbb1c51589903c4043b98917f1d9", 144408290),  // v3
	GAME_ENTRY_EN("abettermousetrap", "trap.exe", "0b7529a76f38283d6e850b8d56526fc1", 4735739),
	GAME_ENTRY_EN("ablemabel", "Able Mabel.exe", "618d7dce9631229b4579340b964c6810", 16175891),
	GAME_ENTRY_EN("abominationobtainer", "ABOMINATION OBTAINER.exe", "b4c0b8c907a1785b3612863610f1210c", 8582210),
	GAME_ENTRY_EN("absent", "absent.exe", "aabdafae8b57dfc48fdf158a72326c23", 39263755),  // v1.1
	GAME_ENTRY_EN("absent", "absent.exe", "aabdafae8b57dfc48fdf158a72326c23", 39284149),  // v1.21
	GAME_ENTRY_EN("absentpart1", "absent - part i.exe", "34ca36e3948aae8527dd0e90f0861a75", 31421924),
	GAME_ENTRY_EN("absin", "black_project.exe", "b142b43c146c25443a1d155d441a6a81", 8787461),
	GAME_ENTRY_EN("absurdistan", "Absurdistan.exe", "4588012d3077d6b6791fce8cd4b15885", 84568958),
	GAME_ENTRY("acatsnight1", "A Cat's Night.exe", "615e73fc1874e92d60a1996c2330ea36", 73906461),  //En-Fr-It
	GAME_ENTRY("acatsnight2", "A Cat's Night 2.exe", "1bc32c4f5753b1ccbc60b2735b89bd4c", 226162327),  //En-It
	GAME_ENTRY_EN("access", "access.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 1816218),
	GAME_ENTRY_EN("aceking", "ags.exe", "f982756f0e2c2fbeca15d199b9851559", 338249917),
	GAME_ENTRY_EN("acequest", "newgame.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 4124225),
	GAME_ENTRY_EN("acharchronicles", "Oblitus.exe", "e37052201ffaea41ff1784f59ce47163", 55523181),
	GAME_ENTRY_EN("achristmasblunder", "blunder.exe", "35c11dc3b5f73b290489503b0a7c89e5", 1974915),
	GAME_ENTRY_EN("achristmasghost", "A Christmas Ghost.exe", "7971a7c02d414dc8cb33b6ec36080b91", 54309690),
	GAME_ENTRY("achristmasnightmare", "xmasnightmare.ags", "3e9ca3f7786ca2c19e1718060726d44a", 46691215),  // v1.3 Linux En-Es-It
	GAME_ENTRY_EN("achristmaswish", "A Christmas Wish.exe", "01d0e6bd812abaa307bcb10fc2193416", 3452454),
	GAME_ENTRY_EN("achtungfranz", "Franz.exe", "949f7440e3692b7366c2029979dee9a0", 1322363),
	GAME_ENTRY_EN("aciddd", "acidddd.exe", "06a03fe35791b0578068ab1873455463", 1858394),
	GAME_ENTRY_EN("acjadventure", "CJADVENTURE.exe", "06a03fe35791b0578068ab1873455463", 1653029),
	GAME_ENTRY("acureforthecommoncold", "game.exe", "06a03fe35791b0578068ab1873455463", 5247960),  //En-It
	GAME_ENTRY_EN("acuriouspastime", "Curious.exe", "4cdf7cf9047ca83b3221a95664ee36d2", 485278752),
	GAME_ENTRY_EN("acurioussilence", "silence.exe", "495d45fb8adfd49690ae3b97921feec6", 8039714),
	GAME_ENTRY_EN("adalinepicnic", "adalinemoongame.exe", "18331bc5c4b4ea63dd649de7f7649522", 70885909),
	GAME_ENTRY_EN_PLATFORM("adateinthepark", "a date in the park.exe", "59fe2aa2cc67589f11707ddc5a7f01cc", 47324098, "itch.io/Steam"),
	GAME_ENTRY_EN("adaywithmichael", "adwm.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 65283966),
	GAME_ENTRY_EN("advallinthegame", "all in the game.exe", "bb59de174d70797d774dec76a171352d", 843215726),
	GAME_ENTRY("advbunnybunnyman", "Game.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 13431315),  //En-Fi
	GAME_ENTRY("advbunnybunnyman", "Pupupeli.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 13453697),  //En-Fi
	GAME_ENTRY_EN("adventureisland", "adisland.exe", "6d2f8e80c5f2372b705fdd4cc32f3579", 51032657),  // v3
	GAME_ENTRY_EN("adventuresofjoshanddyan", "joshanddyan.exe", "90413e9ae57e222f8913b09d2bc847bc", 3053444),
	GAME_ENTRY("adventuresofmaxfaxepisode1", "maxfax1.exe", "f95aa7fd4c60ae0c64ab2bca085d0d15", 111272141),  // Multi
	GAME_ENTRY_EN("adventuretheinsidejob", "thalia.exe", "615e806856b7730afadf1fea9a756b70", 305012295),
	GAME_ENTRY_LANG("adventureworld", "Adventure.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 3978252, Common::ES_ESP),
	GAME_ENTRY_EN("advwelcome", "welcome to the genre.exe", "7b312aca49376e8a92bc31c612dfe81d", 31339728),
	GAME_ENTRY_LANG_PLATFORM("ael", "Ael.exe", "2321081ba51d1869ebe11b9ba126911b", 12053043, Common::FR_FRA, "Prototype"),
	GAME_ENTRY_LANG_PLATFORM("ael", "Ael.ags", "442acbc3aa15ec7a2d43dcbc4bab4ee7", 9568291, Common::FR_FRA, "Prototype"),
	GAME_ENTRY_EN("aerinde", "aerinde.exe", "3d40063da244931d67726a2d9600f1e8", 31700697),
	GAME_ENTRY_EN_PLATFORM("aeronuts", "aeronuts.exe", "e446d58cf60bf13e32d0edd470065c7d", 10984662, "MAGS"),
	GAME_ENTRY("aeronuts", "aeronuts.exe", "820ac8a9776518d8510404c2deaf6328", 11469798),  //v1.2 Eng-Esp
	GAME_ENTRY_EN("aeternaduel", "AeternaDuel.exe", "256752c9a97b4780fc5e6f3239c8cdf1", 4932240),
	GAME_ENTRY_EN_PLATFORM("affairoftheweirdo", "max_griff_invg.exe", "acf67190f218f04bcafec14c0864c220", 5279933, "v1.0.0"),
	GAME_ENTRY_EN_PLATFORM("affairoftheweirdo", "max_griff_invg.exe", "acf67190f218f04bcafec14c0864c220", 5153491, "v1.0.1"),
	GAME_ENTRY_EN("affection", "SpaceGame.exe", "222a92ee46da1ef87e0108b50a5cae1c", 5115955),
	GAME_ENTRY_EN("affection", "SpaceGame.ags", "e106af6a6e74deba28138baaa17b7792", 2527267),
	GAME_ENTRY_EN("afragmentofher", "afragmentofher_v0.2.1.0.exe", "618d7dce9631229b4579340b964c6810", 8400475),
	GAME_ENTRY_EN("afriendindeed", "thehouse.exe", "3128b9f90e2f954ba704414ae854d10b", 6197624),
	GAME_ENTRY_EN("afterashadow", "after a shadow.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6157553),
	GAME_ENTRY_EN("agentbee", "BEE.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 45542087),
	GAME_ENTRY_EN("agenthudson", "ah.exe", "c7916b82f00c94013a3f7706b4d333c6", 15479216),
	GAME_ENTRY_EN("agenttrinityepisode0theultimatum", "theultimatum_03.exe", "615e73fc1874e92d60a1996c2330ea36", 3777519),
	GAME_ENTRY_EN("aggghost", "verbcoin.exe", "88d4158acfc9db9299e514979f289ced", 23937962),
	GAME_ENTRY_EN("agitprop", "agitprop.exe", "a524cbb1c51589903c4043b98917f1d9", 12834513),
	GAME_ENTRY_EN("agnosticchicken", "chicken.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 2473154),
	GAME_ENTRY_EN("agsawards2008", "aa2008.exe", "2615d67c2234f387e0ffd1a375476be0", 15151764),
	GAME_ENTRY_EN("agsawards2016", "AGS Awards 2016.exe", "6e861b1f476ff7cdf036082abb271329", 389520873),
	GAME_ENTRY_EN("agsawards2017", "AGS Awards 2017.exe", "78dd4ca028ee0156b6a093d6d780aa65", 799003462),
	GAME_ENTRY_EN("agsawards2020", "AGS Awards 2020.exe", "1466cb5cd32c5ce4a4dbc6e4975f8991", 346110549),
	GAME_ENTRY_EN("agsawards2020", "AGS Awards 2020.exe", "1466cb5cd32c5ce4a4dbc6e4975f8991", 346114803),
	GAME_ENTRY_EN("agsawards2022", "AGS Awards 2022.exe", "e0c91ac3e4bbfad799b958afb5878d32", 926641116),  // Win
	GAME_ENTRY_EN("agsawards2022", "AGS Awards 2022.ags", "2abf435d01917e3a598fc370dfe43af0", 923130246),  // Linux
	GAME_ENTRY_EN("agsawards2022", "ac2game.dat", "2abf435d01917e3a598fc370dfe43af0", 923130246),  // Mac
	GAME_ENTRY_EN("agsawards2023", "AGS Awards 2023.exe", "ab317f18d05b816d9f35556519f9c4aa", 612673895),  // Win
	GAME_ENTRY_EN("agsawards2023", "AGS Awards 2023.ags", "e2125e49d2dc4ed54fb68f63eff52ce8", 609169747),  // Linux
	GAME_ENTRY_EN("agsawards2023", "ac2game.dat", "e2125e49d2dc4ed54fb68f63eff52ce8", 609169747),  // Mac
	GAME_ENTRY_EN("agsawardsbp", "AGS Awards Backstage Pass.exe", "82da2565c456dcfb265ded6fe3189c0b", 6458787),
	GAME_ENTRY_EN("agscamerastd", "camdemo.exe", "17009da9820f5aa86d0588023d497db8", 26012159),  // Windows
	GAME_ENTRY_EN("agscamerastd", "camdemo.ags", "4e6e999d090581aea259ee10428d5628", 22977515),  // Linux
	GAME_ENTRY_EN("agschess", "chess.exe", "0710e2ec71042617f565c01824f0cf3c", 988512),
	GAME_ENTRY_EN("agschess", "chess.exe", "f120690b506dd63cd7d1112ea6af2f77", 1198934),
	GAME_ENTRY_EN_PLATFORM("agschess", "AGSMembersChess.exe", "0710e2ec71042617f565c01824f0cf3c", 1042885, "Community Version"),
	GAME_ENTRY_EN("agsdarts", "ags darts.exe", "434c43a5e1ba2a11c1bde723ffeae719", 47771575),
	GAME_ENTRY_EN("agsdarts2", "AGS 180 Darts 2.exe", "96bdc335c417f8e15e1361df894e3ced", 276660650),
	GAME_ENTRY_EN("agsfightgame2009", "Fight Game.exe", "463f79e5db4013d1b3be647edd7e338d", 1871790),
	GAME_ENTRY_EN("agsfightgame2009", "Fight Game.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3551992),
	GAME_ENTRY_EN("agsfightgameremix", "fight.exe", "f120690b506dd63cd7d1112ea6af2f77", 2469641),
	GAME_ENTRY_EN("agsinvaders", "ags-invaders.exe", "f120690b506dd63cd7d1112ea6af2f77", 1394435),
	GAME_ENTRY_EN("agsjukebox", "jukebox2.exe", "a7aef57e360306c9377164f38d317ccb", 2914973),
	GAME_ENTRY_EN("agslife", "AGSLife.exe", "f2be2a4ef91e99bd902536da5f3290c7", 9608578),
	GAME_ENTRY_EN("agsmagus", "AGS Wizard.exe", "434c43a5e1ba2a11c1bde723ffeae719", 21177588),
	GAME_ENTRY_EN("agsmastermind", "AGS Mastermind.exe", "519c0d37ab893d95f5add495355e460c", 30196465),
	GAME_ENTRY_EN("agsmittensshooter", "clex.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1381575),
	GAME_ENTRY_EN("agstechsupport", "agsgame.exe", "88cf59aad15ca331ab0f854e16c84df3", 1229972),
	GAME_ENTRY_EN("agswerewolf", "mags0722.exe", "0e4ddc9893796a9f39395d0e0220a37b", 6214215),
	GAME_ENTRY_EN("agswerewolf", "mags0722.ags", "0f36bc83671ce002addf7bfa2d748747", 3073587),
	GAME_ENTRY_EN("agsyahtzee", "ags yathzee.exe", "434c43a5e1ba2a11c1bde723ffeae719", 37295758),
	GAME_ENTRY_EN("agsyahtzee", "ags yathzee.exe", "434c43a5e1ba2a11c1bde723ffeae719", 39185090),  // v2.0
	GAME_ENTRY_EN("agsyahtzee2", "ags yahtzee 2.exe", "9e995c04d8642d6182d492c54a90b188", 71217276),
	GAME_ENTRY_EN("agunshotinroom37", "agir37.exe", "f120690b506dd63cd7d1112ea6af2f77", 1451303),
	GAME_ENTRY_EN("ahamoth", "Achamoth.exe", "3747e39ad7a65c16dbd285252087c42e", 189362193),
	GAME_ENTRY("ahomieadventure", "AHA_20150820_00_src.exe", "0241777c2537fc5d077c05cde10bfa9f", 3902636),  // Eng-Ita
	GAME_ENTRY_EN("aidaschristmas", "Aidas Strange Christmas.exe", "9cb3c8dc7a8ab9c44815955696be2677", 15568945),
	GAME_ENTRY_EN("aidashalloween", "Aida's Bizarre Halloween.exe", "18f5fd85de78efca16c7bafce54e3f63", 23337412),
	GAME_ENTRY_EN("ainthegoffantabulousw", "gfw.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7991208),
	GAME_ENTRY_EN("ainthegoffantabulousw", "gfw.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7991393),
	GAME_ENTRY_EN("airwave", "~airwave~.exe", "18456f28d9bf843b087e80072c85beca", 22521544),
	GAME_ENTRY_EN_PLATFORM("alandlordsdream", "Game.exe", "0241777c2537fc5d077c05cde10bfa9f", 6212265, "MAGS"),  // v1.1
	GAME_ENTRY("alandlordsdream", "Game.exe", "9cd9563150a69bc1d4eacda0ff3557f8", 7197801),  // v1.2 En-Es-It
	GAME_ENTRY("alandlordsdream", "Game.exe", "7db052bc30700d1f30f5330f5814f519", 8060425),  // v1.4?
	GAME_ENTRY("alandlordsdream", "Game.exe", "63f8a60cc094996bd070e68cb3c4722c", 8132910),  // v1.4.1 En-Fr-Es-Ita
	GAME_ENTRY_EN("alansaveschristmas", "alancmas.exe", "3f76f9c4249bdc06794c13b098229c90", 39312224),
	GAME_ENTRY_EN("alansaveschristmas", "alancmas.exe", "9db90e244d6f3476191d3d4de189509d", 39295833),
	GAME_ENTRY("aliceinwonderlandis", "wonderland.exe", "8fa5ae6cee9a4dfa72bbfaaad4135046", 85478687),  //Multi
	GAME_ENTRY_EN("alienattack", "alien attack.exe", "06a03fe35791b0578068ab1873455463", 11045476),
	GAME_ENTRY_EN("aliencarniage", "AlienCarni.exe", "06a03fe35791b0578068ab1873455463", 22301801),
	GAME_ENTRY_EN("aliencowrampage", "alien.exe", "57dc38c78f323a5a45e486ee7fff767f", 15639495),
	GAME_ENTRY_EN("alienescapade", "Alien Escapade.exe", "4fb72c890984548ed6782063f2230942", 30092272),
	GAME_ENTRY_EN("alienprison", "Alien prison escape.exe", "2f2bc0b9c539d20529c0e343315d5f65", 8510852),  // itch.io 1.1
	GAME_ENTRY("alienpuzzleinvasion", "alien.exe", "a524cbb1c51589903c4043b98917f1d9", 10552264),  // En-Fr
	GAME_ENTRY_EN("alienrapeescape", "a.r.e.exe", "39d7a558298a9f1d40c1f415daf9bb74", 4802707),
	GAME_ENTRY_EN("alienspaceman", "AGS1.exe", "8aff96231ca059cd61fe5ce68b83f50f", 18075265),
	GAME_ENTRY_EN("alienspaceman", "AGS1.ags", "3a758042f7c37c1fad714c6bdb383b72", 15588465),  // itch.io client
	GAME_ENTRY_EN("alienthreat", "alien threat.exe", "f120690b506dd63cd7d1112ea6af2f77", 6341266),
	GAME_ENTRY_EN("alientimezone", "atz.exe", "0710e2ec71042617f565c01824f0cf3c", 2910487),
	GAME_ENTRY_EN("alientimezone", "atz.exe", "0710e2ec71042617f565c01824f0cf3c", 2911858),
	GAME_ENTRY_EN("alienvspredator", "AVP.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1433228),
	GAME_ENTRY_EN("allgonesoon", "All_Gone_Soon.exe", "618d7dce9631229b4579340b964c6810", 8858991),
	GAME_ENTRY_EN("allgonesoon2", "AGS2.exe", "618d7dce9631229b4579340b964c6810", 11140718),
	GAME_ENTRY_EN("alloweisland", "asland.exe", "88cf59aad15ca331ab0f854e16c84df3", 1281427),
	GAME_ENTRY_EN("allpigs", "All pigs deserve to burn in hell.exe", "973f6b65820ca1f4e19704a49be99d76", 20836147),
	GAME_ENTRY_EN("alluminum", "mags_october.exe", "f120690b506dd63cd7d1112ea6af2f77", 1772481),
	GAME_ENTRY("almostblue", "Almost Blue.exe", "2cb5f4d0914d1b1f0638c65da6689050", 231366019),  // Eng-Ita
	GAME_ENTRY_EN("aloneinthenight", "alone.exe", "0710e2ec71042617f565c01824f0cf3c", 9501343),
	GAME_ENTRY_EN_PLATFORM("alphabeta", "Alphabeta.exe", "5992f37daae6ea70654e9544e62ebb97", 2601798, "Unfinished"),
	GAME_ENTRY("alphablock", "AlphaBlock.exe", "962449147c1646ee07d4022c79c6cd8b", 14360875),  // Windows  Eng-Swe
	GAME_ENTRY("alphablock", "AlphaBlock.ags", "7eb3b39b6fb6a5dfcaa058ab371a22f2", 11327255),  // Linux  Eng-Swe
	GAME_ENTRY_EN("alphadog", "alpha_dog.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3329253),
	GAME_ENTRY_EN("alphaxsmash", "project smash.exe", "06a03fe35791b0578068ab1873455463", 25377719),
	GAME_ENTRY_EN("alquest1", "alquest.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 20154310),
	GAME_ENTRY("alyssaescape", "Alyssa.ags", "24b8335774a50d5200f99437c4cde5b4", 7441237),  // Eng-Deu
	GAME_ENTRY_EN("alysvsthephantomfelinefoe", "alys.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 22323495),
	GAME_ENTRY_EN("amagicstone", "a magic stone.exe", "465f972675db2da6040518221af5b0ba", 5478520),
	GAME_ENTRY_EN("america2000", "America2000.exe", "7ddb9e776648faed5a51170d087074e9", 4136265),
	GAME_ENTRY_EN("amotospuf", "amotos.exe", "06a03fe35791b0578068ab1873455463", 10771879),
	GAME_ENTRY_EN("amotospuf", "amotos.exe", "06a03fe35791b0578068ab1873455463", 10226456),
	GAME_ENTRY_EN("amongthorns", "Among Thorns.exe", "b5c41e08919834b549ca350a28f18f87", 64621203),
	GAME_ENTRY_EN_PLATFORM("amongthorns", "Among Thorns.exe", "b5c41e08919834b549ca350a28f18f87", 64412147, "MAGS"),
	GAME_ENTRY_EN("amtag", "amtag.exe", "06a03fe35791b0578068ab1873455463", 8755912),
	GAME_ENTRY_EN("amused", "TheMeanTime.exe", "615e73fc1874e92d60a1996c2330ea36", 5113261),
	GAME_ENTRY_EN("analienswork", "AWIND.exe", "0710e2ec71042617f565c01824f0cf3c", 4460224),
	GAME_ENTRY_EN("ancientaliens", "AAliens.exe", "82da2565c456dcfb265ded6fe3189c0b", 49750239),
	GAME_ENTRY_EN("aneternityreflecting", "An Eternity, Reflecting.exe", "78dd4ca028ee0156b6a093d6d780aa65", 53731093),
	GAME_ENTRY_EN("anig", "Instagame.exe", "0500aacb6c176d47ac0f8158f055db83", 1329918),
	GAME_ENTRY_PLATFORM("anightinberry", "Nuit en Berry.exe", "06a03fe35791b0578068ab1873455463", 336951778, "Original"),  //2016 release En-Fr
	GAME_ENTRY_PLATFORM("anightinberry", "Nuit en Berry.exe", "d5225f13aa4356e9524fb0de9375cecc", 744946585, "Longer Version"),  //v2.0.5 2019 release En-Fr
	GAME_ENTRY_EN("anightmareonduckburg", "ANoD.exe", "c9cd3e2cdd295c33e62cfa97b14461e9", 91549100),
	GAME_ENTRY_EN("anightthatwouldntend", "Game.exe", "0241777c2537fc5d077c05cde10bfa9f", 14559632),
	GAME_ENTRY_EN("anighttoremember", "the trials.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 9643993),
	GAME_ENTRY_EN("animalagency", "AnimalAgency.exe", "7db052bc30700d1f30f5330f5814f519", 613308708),  // Win
	GAME_ENTRY_EN("animalagency", "AnimalAgency.ags", "39bab349175b4b7fec0a6b13b701d6eb", 610273552),  // Linux
	GAME_ENTRY_EN("animalagency", "game.ags", "39bab349175b4b7fec0a6b13b701d6eb", 610273552),  // Mac
	GAME_ENTRY_EN("anna", "anna.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 42337759),  // v1.0
	GAME_ENTRY_EN("anna", "anna.exe", "548d8cf1c544c85da3e78b7aa65a6d41", 16572928),
	GAME_ENTRY_EN("annieandroidautomatedaffection", "annie source.exe", "fc17e9b3ab53f6b4841e2a4af5c782ff", 5015270),
	GAME_ENTRY_EN("annielondonberry", "ng14.ags", "39da8acddd4bb49982c3939ebbfce4d3", 172562551),  // itch.io client
	GAME_ENTRY_EN("annoyingquest", "uhhr.exe", "0b7529a76f38283d6e850b8d56526fc1", 571857),
	GAME_ENTRY_EN("anoffer", "An offer you cannot refuse.exe", "17009da9820f5aa86d0588023d497db8", 217468062),
	GAME_ENTRY_LANG("anoffer", "Ponuka ktora sa neodmieta.exe", "17009da9820f5aa86d0588023d497db8", 217460167, Common::SK_SVK),  //Slovak
	GAME_ENTRY_EN("anophtos", "anophtos.ags", "635417bd0481d8a410bf4055807cc959", 85743735),
	GAME_ENTRY_EN("anotherheaven", "anotherheaven.exe", "1b9f13d430bb15bf30d0fd044358db68", 7144273),  // Win
	GAME_ENTRY_EN("anotherheaven", "anotherheaven.ags", "063efba5833869fe3c103684f54501bd", 4646721),  // Linux
	GAME_ENTRY_PLATFORM("anothermuseum", "Another Museum.exe", "aa3a99c8b61a8144c8add91776b9b034", 71073433, "MAGS"),  // Windows Eng-Esp
	GAME_ENTRY_PLATFORM("anothermuseum", "Another Museum.exe", "4308fc11a6564fecdd07aad435db9c3a", 71226691, "MAGS"),  // Windows Eng-Esp
	GAME_ENTRY_PLATFORM("anothermuseum", "Another Museum.ags", "0061e366527684fc95f566c498bbc9da", 68102348, "MAGS"),  // Linux Eng-Esp
	GAME_ENTRY("anothermuseum", "Another Museum.exe", "4308fc11a6564fecdd07aad435db9c3a", 71243854),  // Windows Eng-Esp
	GAME_ENTRY("anothermuseum", "Another Museum.exe", "aa3a99c8b61a8144c8add91776b9b034", 71073435),  // Windows Eng-Esp
	GAME_ENTRY("anothermuseum", "Another Museum.ags", "c21271e254f5f390306235a41eede490", 68119098),  // Linux Eng-Esp
	GAME_ENTRY("anothermuseum", "Another Museum.ags", "6ca0d0aa8f8fcf7512a3fb6081dc7acb", 67947653),  // Linux Eng-Esp
	GAME_ENTRY_EN("anotherwayout", "1week.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7870567),
	GAME_ENTRY_EN("anthonysessay", "copy of school.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 11033611),
	GAME_ENTRY_EN("anticipatingmurder", "ags.exe", "f120690b506dd63cd7d1112ea6af2f77", 19907137),
	GAME_ENTRY_EN("antiheroes", "antiheroes.exe", "f120690b506dd63cd7d1112ea6af2f77", 3984580),
	GAME_ENTRY_EN("antiheroes", "antiheroes.exe", "f120690b506dd63cd7d1112ea6af2f77", 3982762),
	GAME_ENTRY_EN("anxiousawakening", "An Anxious Awakening.exe", "0241777c2537fc5d077c05cde10bfa9f", 3991332),
	GAME_ENTRY_EN_PLATFORM("ap0", "AP0.exe", "038971ea42647e75965266a93e9a1a4b", 8426041, "Prototype"),
	GAME_ENTRY_EN("apiratestale", "a_pirates_tale.exe", "06a03fe35791b0578068ab1873455463", 13952670),
	GAME_ENTRY_EN("aplaceinthesun", "inthesun.exe", "7a3096ac0237cb6aa8e1718e28caf039", 40359648),
	GAME_ENTRY_EN("aplacewithoutfrontier", "Frontier.exe", "9cb3c8dc7a8ab9c44815955696be2677", 17733652),
	GAME_ENTRY_EN("apocalypsemeow1", "CaK3.exe", "97d700529f5cc826f230c27acf81adfd", 6044911),
	GAME_ENTRY_EN("apocalypsevel", "MAGSGAME.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 5025809),
	GAME_ENTRY_EN("applefarm", "Apple farm.exe", "82da2565c456dcfb265ded6fe3189c0b", 5483907),
	GAME_ENTRY_EN_PLATFORM("applefarm", "Apple farm.exe", "82da2565c456dcfb265ded6fe3189c0b", 5484984, "OROW"),
	GAME_ENTRY_EN("appointmentwithdeath", "APD.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1321938),
	GAME_ENTRY_EN("apprentice", "app.exe", "ecc8eaa38fe3adea61ffc525d2ce5c0e", 14110306),
	GAME_ENTRY_PLATFORM("apprentice", "app.exe", "45ab4f29031b50c8d01d10a269f77ff5", 17488568, "Deluxe"),  // Multi
	GAME_ENTRY_PLATFORM("apprentice", "app.exe", "45ab4f29031b50c8d01d10a269f77ff5", 17488604, "Deluxe"),  // Multi
	GAME_ENTRY_EN("apprentice2", "app2.exe", "465f972675db2da6040518221af5b0ba", 34158083),
	GAME_ENTRY_EN("apprentice2", "app2.exe", "465f972675db2da6040518221af5b0ba", 34159191),
	GAME_ENTRY_EN("aprofoundjourney", "apj.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 920131),
	GAME_ENTRY_EN("apunkwithwheels", "A punk with wheels.exe", "d90cd956022235ed9b272fb0b2ab5792", 17924043),
	GAME_ENTRY_EN_PLATFORM("aractaur", "aractaur.exe", "aecd482222ff54206e43a029b5f0b170", 5318963, "MAGS"),
	GAME_ENTRY_EN("aractaur", "aractaur.exe", "6b4ceb9e327ac99479c08d825461f4cb", 5543430),
	GAME_ENTRY_EN("araindogstory", "Raindog.exe", "09cf8b451781575fa3ba1a0e31f5fc66", 96888573),
	GAME_ENTRY_EN_PLATFORM("archeos", "archeos.exe", "2ff048659aaefd20d342db6428a5f1a0", 6659974, "AdvJam Build"),  // v1.5 Win
	GAME_ENTRY_EN_PLATFORM("archeos", "archeos.ags", "327fd4159f67fb3d10ec83407945cb86", 4216182, "AdvJam Build"),  // v1.5 Linux
	GAME_ENTRY_EN("archeos", "archeos.exe", "2ff048659aaefd20d342db6428a5f1a0", 6661410),  // v1.7 Win
	GAME_ENTRY_EN("archeos", "archeos.ags", "050b3e5c5486fb6b977712cb4e4d6058", 4217618),  // v1.7 Linux
	GAME_ENTRY_LANG_PLATFORM("archivesinsondable", "protoArchives01.exe", "5f6db89bead4f55f32d0a4bd81df6dd3", 20596943, Common::FR_FRA, "Prototype"),
	GAME_ENTRY_EN("ardensvale", "Arden's Vale.exe", "588df1c52f2ecdc6af866cd06b9810e1", 20097804),
	GAME_ENTRY_EN("ardentfever", "Ardent Fever.exe", "d32f0fb244a8f815e7e87a78aa5dcee3", 3979543),
	GAME_ENTRY_EN("arewethereyet", "arewethereyet.exe", "f120690b506dd63cd7d1112ea6af2f77", 1054672),
	GAME_ENTRY_EN("arewethereyet2", "arewethereyet2.exe", "f120690b506dd63cd7d1112ea6af2f77", 949631),
	GAME_ENTRY("arjunaz78", "Arjunaz78.exe", "2748aefd266d40d9a14088cb77293549", 11368111),  // En-Malay?
	GAME_ENTRY_EN("aroomwithoutyou", "aroomwithoutyou.ags", "34ba844fe761f8a9da2cf1076c30d847", 27163303),
	GAME_ENTRY_EN("aroomwithoutyou", "aroomwithoutyou.exe", "7ddb9e776648faed5a51170d087074e9", 29650615),
	GAME_ENTRY_EN("armageddonmargaret", "am_game.exe", "06a03fe35791b0578068ab1873455463", 3640885),
	GAME_ENTRY_EN("armageddonmargaret", "armageddon margaret.exe", "3128b9f90e2f954ba704414ae854d10b", 2951568),
	GAME_ENTRY_EN("armageddonmargaret", "mags.exe", "3128b9f90e2f954ba704414ae854d10b", 2951568),
	GAME_ENTRY_EN("artisan", "Artisan.ags", "1baad8bab89fc5ca19abeaf06ae8067c", 10245228),
	GAME_ENTRY_EN("artofdying", "die.exe", "89df481678b2ddc40ecc9f83caa76b89", 4804446),
	GAME_ENTRY("asecondface", "eye of geltz.exe", "0e32c4b3380e286dc0cea8550f1c045e", 7061019),  //Multi
	GAME_ENTRY("asecondface", "eye of geltz.exe", "5bda06bea6f5e620d5f3f1ca75dd6da4", 7092197),
	GAME_ENTRY("asecondface", "eye of geltz.exe", "50bc7406920eda1fd882c209060ae1f8", 556196897),  // v1.7 Itch.io Multi
	GAME_ENTRY_EN("ashortnightmare", "a_short_nightmare.exe", "b142b43c146c25443a1d155d441a6a81", 94221930),
	GAME_ENTRY_EN("asimplefix", "A simple fix.exe", "615e73fc1874e92d60a1996c2330ea36", 27488921),
	GAME_ENTRY_EN("asledmundo", "asl.exe", "6e3d6225dee662ff6450a3bfa942773b", 2749868),
	GAME_ENTRY_EN("asotc", "A Stain on the Company.ags", "2def04d9c498cb1f121bcfa2a3fe2b8f", 12916626),  // itch.io 0.9
	GAME_ENTRY("asporia", "rpg.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 7094164),  //Eng-Tur
	GAME_ENTRY_EN("asterix", "asterix.exe", "06a03fe35791b0578068ab1873455463", 62405430),
	GAME_ENTRY_EN("astranded", "astranded.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1845395),
	GAME_ENTRY_EN("astron", "astron.exe", "a0fd918609b9d139e4076fa7a0052ae1", 71101771),  // Windows
	GAME_ENTRY_EN("astron", "astron.ags", "1ba214946cd17373ae8dbc2c4fbc1f7f", 68563771),  // Linux
	GAME_ENTRY_EN("asuspiciousdate", "MAGS May.exe", "39d9d0f826b7510c850c55c109230cb4", 4686146),
	GAME_ENTRY("aswinsdream", "agsgame.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 14384181),  // Eng-Indonesian
	GAME_ENTRY_EN("ataintedtreat", "A Tainted Treat.exe", "615e73fc1874e92d60a1996c2330ea36", 5737926),
	GAME_ENTRY_EN("ataleinthezoo", "MAGS-Jan.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16729569),
	GAME_ENTRY_EN("ataleofbetrayal", "mags.exe", "465f972675db2da6040518221af5b0ba", 11071496),
	GAME_ENTRY_EN("atapi", "atapi.exe", "dd8586ebefd5c457e29b6b9845a576ea", 34114381),
	GAME_ENTRY_EN("athingaboutnothingness", "LD26.exe", "615e73fc1874e92d60a1996c2330ea36", 5654082),
	GAME_ENTRY_EN("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42740200),
	GAME_ENTRY_EN("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42763765),  // v1.2
	GAME_ENTRY("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42785860),  // Eng-Hun
	GAME_ENTRY("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42872046),
	GAME_ENTRY("atotk", "atotk.exe", "68d4f3488a9dcec74584651c0e29e439", 5078309),
	GAME_ENTRY_EN("atotkjukebox", "jukebox.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 1631992),
	GAME_ENTRY_EN("atotkjukebox", "jukebox.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 1631583),
	GAME_ENTRY_EN("atreatandsometricks", "a treat and some tricks.exe", "495d45fb8adfd49690ae3b97921feec6", 33708250),
	GAME_ENTRY_EN("atreatandsometricks", "a treat and some tricks.exe", "495d45fb8adfd49690ae3b97921feec6", 33716134),  //v1.3 Win
	GAME_ENTRY_EN("atreatandsometricks", "A Treat and Some Tricks.ags", "31980820af7cda2624b3f0be37438644", 31235478),  //v1.3 Linux
	GAME_ENTRY_EN("attackgame", "AttackGame.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 2052994),
	GAME_ENTRY_EN("atthecafe", "At the cafe.exe", "615e73fc1874e92d60a1996c2330ea36", 3192558),  // Windows
	GAME_ENTRY_EN("atthecafe", "At the cafe.ags", "c598263c0d04b74ba4f888287ceefaca", 1407438),  // Linux
	GAME_ENTRY_EN("audioquest", "Audio Quest.exe", "06a03fe35791b0578068ab1873455463", 1638472),
	GAME_ENTRY_EN("augustlonging", "The long August longing.exe", "5db720fe7ddec76ff9829c5b21905201", 18038972),
	GAME_ENTRY_EN("aunaturel", "au naturel.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8499426),
	GAME_ENTRY_EN("authorsim", "AuthorSim2023.ags", "4f2894049816e5c03648bc01da6728a8", 3386519),
	GAME_ENTRY_EN("authorsim", "AuthorSim2023.ags", "7cd8a7566f4be2600a3843770c448482", 3493740),  // v1.2
	GAME_ENTRY_EN("automation", "automation.exe", "c16204dc8aa338e3199b2c62da0b33f4", 3943320),
	GAME_ENTRY_EN_PLATFORM("automation", "automation.exe", "c16204dc8aa338e3199b2c62da0b33f4", 3942321, "OROW"),
	GAME_ENTRY("averyspecialdog", "dog.exe", "465f972675db2da6040518221af5b0ba", 2120544),  // En-De
	GAME_ENTRY("averyspecialdog", "dog.exe", "465f972675db2da6040518221af5b0ba", 2120915),
	GAME_ENTRY("awakener", "awakener.exe", "3e62eafed0fc365821b7afdbf1aec3d8", 8896076),  // En-Rus
	GAME_ENTRY_EN("awakening", "eob.exe", "a524cbb1c51589903c4043b98917f1d9", 21527657),
	GAME_ENTRY_EN("awalkindatomb", "full game.exe", "0710e2ec71042617f565c01824f0cf3c", 1312738),
	GAME_ENTRY_EN("awalkinthepark", "sag_mag_04_08.exe", "9cb3c8dc7a8ab9c44815955696be2677", 2908542),
	GAME_ENTRY_EN("awayinatower", "SoringCleaning.exe", "615e73fc1874e92d60a1996c2330ea36", 6231313),
	GAME_ENTRY_EN("awesmoequest", "Awesmoe.exe", "06a03fe35791b0578068ab1873455463", 1440230),
	GAME_ENTRY_EN("awesomequest1", "aq1.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 3989764),
	GAME_ENTRY_EN("awkward", "Tiny World.exe", "615e73fc1874e92d60a1996c2330ea36", 6497772),
	GAME_ENTRY_EN("awomanforallseasons", "A Woman For All Seasons.exe", "77c325a29d4b45fe9bb38761fb17118b", 4171441),
	GAME_ENTRY_EN_PLATFORM("awomanforallseasons", "Wizard.exe", "6ce6bc68684702e455acb729b6b2da95", 3814262, "OROW"),
	GAME_ENTRY_EN("axmasevetale", "mags.exe", "71ca0d6c1c699595f28a2125948d4a84", 6489870),
	GAME_ENTRY_EN("bachdead", "Bachy.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 1992849),
	GAME_ENTRY_EN("backdoorman", "hustler.exe", "2c26669a627da8e1e5159319b78ad1ce", 9558000),
	GAME_ENTRY_EN("backlot", "BackRemake.exe", "a524cbb1c51589903c4043b98917f1d9", 4252039),
	GAME_ENTRY_EN("badbunker", "Bad Bunker.exe", "b216ee957dd677023e02d900281a02d6", 3457312),
	GAME_ENTRY_EN("badluck", "bad luck.exe", "f120690b506dd63cd7d1112ea6af2f77", 10618466),
	GAME_ENTRY_EN_PLATFORM("badluck", "bad luck.exe", "f120690b506dd63cd7d1112ea6af2f77", 10592756, "OROW"),
	GAME_ENTRY("bakeoffitalia", "bakeoff.exe", "39b1e8b82bd8e4dc1f0e54ce100e0fa5", 9128728),  // v0.9.7 Windows En-It-Por
	GAME_ENTRY("bakeoffitalia", "bakeoff.ags", "974fa0136dbc927485a1a1a085e26369", 5973764),  // v0.9.7 Linux En-It-Por
	GAME_ENTRY_EN("baldysadventure", "baldy.exe", "ea0d3284542db629f36cb6fc785e07bc", 179358984),
	GAME_ENTRY_EN("baltazarthefamiliar", "baltazar.exe", "9cb3c8dc7a8ab9c44815955696be2677", 2867294),
	GAME_ENTRY_EN_PLATFORM("baltazarthefamiliar", "baltazar.exe", "9cb3c8dc7a8ab9c44815955696be2677", 2749185, "OROW"),
	GAME_ENTRY_EN("balloonface", "hg.exe", "0710e2ec71042617f565c01824f0cf3c", 2189438),
	GAME_ENTRY_EN("balls", "Balls.exe", "0710e2ec71042617f565c01824f0cf3c", 765814),
	GAME_ENTRY_EN("bananaracer", "bananaracer.exe", "e93f9dfa8405f1ca9f881d160ab31dc2", 10452233),
	GAME_ENTRY_EN("bananaracer", "bananaracer.exe", "e93f9dfa8405f1ca9f881d160ab31dc2", 10454173),
	GAME_ENTRY_EN("barahir", "Barahir.exe", "5677656386de765e72b7400f1e631eed", 40520601),  // v1.2 Windows
	GAME_ENTRY_EN("barahir", "Barahir.ags", "07560dece6654ae594efd210975be588", 37382021),  // v1.2 Linux
	GAME_ENTRY_EN("barhoppers", "BarHoppers.exe", "f18443f98fd61d2c655e76a17f7da905", 22590227),
	GAME_ENTRY_EN("barndilemma", "woh.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 2432485),
	GAME_ENTRY_EN("barnrunner0", "Mini Game 1.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 6419740),
	GAME_ENTRY_EN("barnrunner1p1", "eclair 1.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 13202017),
	GAME_ENTRY_EN("barnrunner1p1", "eclair 1.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 13202019),  // v2.1
	GAME_ENTRY_EN("barnrunner1p2", "eclair 2.exe", "465f972675db2da6040518221af5b0ba", 34193929),
	GAME_ENTRY_EN("barnrunner1p2", "eclair 2.exe", "465f972675db2da6040518221af5b0ba", 34193931),  // v2.1
	GAME_ENTRY_EN("barnrunner3", "Mini Game 3.exe", "465f972675db2da6040518221af5b0ba", 3455484),
	GAME_ENTRY_EN("barnrunner3", "Barn Runner 3.exe", "465f972675db2da6040518221af5b0ba", 3452579),  // v2.0
	GAME_ENTRY_EN("barnrunner4", "Barn Runner 4.exe", "465f972675db2da6040518221af5b0ba", 4954262),
	GAME_ENTRY_EN("barnrunner4", "Mini Game 4.exe", "465f972675db2da6040518221af5b0ba", 4954264),
	GAME_ENTRY_EN("barnrunner5p1", "barn runner 5-1.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 104073543),
	GAME_ENTRY_EN("barnrunner5p1", "barn runner 5-1.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 104071015),  // v1.1
	GAME_ENTRY_EN("barnrunner5p2", "barn runner 5-2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 200879890),
	GAME_ENTRY_EN("barnrunner5p2", "barn runner 5-2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 200696384),  // v1.2
	GAME_ENTRY_EN("barnrunner5p3", "Barn Runner 5-3.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 236158866),
	GAME_ENTRY_EN("barnrunner5p3", "Barn Runner 5-3.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 236159803),  // v1.2
	GAME_ENTRY_EN("barnrunner5p5", "Barn Runner 5-5.exe", "beb4cc3da02c78321e80af92b834938c", 293046932),
	GAME_ENTRY_EN("barnrunner5p5", "Barn Runner 5-5.exe", "beb4cc3da02c78321e80af92b834938c", 293051908),  // v1.0
	GAME_ENTRY_EN("barnrunnerbake1", "BR Bake Sale 1.exe", "18b284c22010850f79bc5c20054a70c4", 29716432),
	GAME_ENTRY_EN("barnrunnerfashion", "MAGS 04-24.exe", "615e806856b7730afadf1fea9a756b70", 18323428),
	GAME_ENTRY_EN("barnrunnerfashion", "MAGS 04-24.exe", "a21322447ef1d4efc1b6838c246b6f80", 19542970),
	GAME_ENTRY_EN("barnrunnerhall1", "BR Halloween 1.exe", "18b284c22010850f79bc5c20054a70c4", 52243988),
	GAME_ENTRY_EN("barnrunnerhall1", "BR Halloween 1.exe", "18b284c22010850f79bc5c20054a70c4", 52252562),  //v1.3
	GAME_ENTRY_EN("barnrunnervalentine1", "BR Valentine 1.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 101774857),
	GAME_ENTRY_EN("barnrunnervn1", "BR VN1.exe", "809418706c429cee5d88e8d483c906cc", 26857313),
	GAME_ENTRY_EN("barnrunnerxmas0", "BR Xmas 0.exe", "615e806856b7730afadf1fea9a756b70", 13689876),
	GAME_ENTRY_EN("barnrunnerxmas2", "xmas 2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 31770973),
	GAME_ENTRY_EN("barrier", "Barrier.exe", "615e73fc1874e92d60a1996c2330ea36", 19111101),
	GAME_ENTRY_EN_PLATFORM("barrier", "Barrier.exe", "615e73fc1874e92d60a1996c2330ea36", 19094978, "OROW"),
	GAME_ENTRY_EN_PLATFORM("barrunner", "Bar Runner.exe", "615e73fc1874e92d60a1996c2330ea36", 3634028, "MAGS"),
	GAME_ENTRY_EN("barrunner", "Bar Runner.exe", "615e73fc1874e92d60a1996c2330ea36", 3642405),  //v1.1
	GAME_ENTRY_EN("bartolomeo", "Bartolomeo.exe", "f604f7f3a12da5d3bcf7a7814a14e43b", 42261079),
	GAME_ENTRY_EN("bartsquestfortv", "simpsons.exe", "0500aacb6c176d47ac0f8158f055db83", 794013),
	GAME_ENTRY_EN("bartsquestfortv", "simpsons.exe", "0500aacb6c176d47ac0f8158f055db83", 794469),
	GAME_ENTRY_EN("basedon", "basedon.exe", "0500aacb6c176d47ac0f8158f055db83", 5775388),
	GAME_ENTRY_EN("basementwoods", "BasementInTheWoods.exe", "7971a7c02d414dc8cb33b6ec36080b91", 4490579),
	GAME_ENTRY_EN("basementwoods", "BasementInTheWoods.ags", "21e9a8e97bc43f3200bfc5db32c0867f", 1993539),
	GAME_ENTRY_EN("basketqust", "eeeeeeeeeeeeeeeee.exe", "9cf51833e787cc919837d9a8bd8fc14c", 874080),
	GAME_ENTRY_EN("bbcscreensaver", "BBC-Screensaver.exe", "0e6d6f3c19f5ca250b7b7ee03cdb2083", 131195371),
	GAME_ENTRY_EN("bcremake", "bc.exe", "2e58934bbe98335f33c20a23a6c6135a", 8208487),  // v1.2
	GAME_ENTRY_EN("bcremake", "bc.exe", "0710e2ec71042617f565c01824f0cf3c", 7683255),  // v1.3
	GAME_ENTRY_LANG("bcremake", "bc.exe", "ef3f57fbb5693c23f5815a421bdb7f4e", 7909416, Common::Language::RU_RUS),
	GAME_ENTRY_EN("beachcomber", "SLHUgame.exe", "7971a7c02d414dc8cb33b6ec36080b91", 44356753),
	GAME_ENTRY_EN("beacon", "beacon.exe", "af0d268193a9220891e983d03141ec58", 24671086),
	GAME_ENTRY_EN("bear", "bear.exe", "97020973a2a635fe28eb0ba4bdfaa70b", 3258662),  //v1.3
	GAME_ENTRY_EN("beardbeverage", "Beard.exe", "618d7dce9631229b4579340b964c6810", 7081492),
	GAME_ENTRY_EN("beardychin", "beardychin.exe", "9d228b6578aa40ee302991f8a8935c7d", 119562726),
	GAME_ENTRY_EN("bearinvenice", "Bear in Venice.exe", "7d2c5cddcac88662b24165b3a0dd77c9", 20615211),  // Windows
	GAME_ENTRY_EN("bearinvenice", "Bear in Venice.ags", "a6f33e002cd25795398f543caeec817f", 18068507),  // Linux
	GAME_ENTRY("beasts", "beasts.exe", "0500aacb6c176d47ac0f8158f055db83", 1295435),  //En-Fr
	GAME_ENTRY_EN_PLATFORM("beatthebuzzer", "Beat the Buzzer.exe", "4d5d25446373d4df71c6bda77a15230a", 149278067, "GameJam Build"),  // v1.0
	GAME_ENTRY_EN("beatthebuzzer", "Beat the Buzzer.exe", "4d5d25446373d4df71c6bda77a15230a", 227179876),  // v1.1 Post-Jam
	GAME_ENTRY_EN_PLATFORM("beautiesandbeasts", "beautiesandbeasts.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 6506966, "OROW"),  //v1.00
	GAME_ENTRY_EN("beautiesandbeasts", "beautiesandbeasts.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 6507202),  //v1.01
	GAME_ENTRY_EN("becalmed", "Becalmed.ags", "6ade71688ef8df9f0c6b093f3105b5c9", 3377343),
	GAME_ENTRY_EN("beepboop", "Beep Boop.exe", "b781cc4ab0fbe647615fafa4830ec308", 5230657),
	GAME_ENTRY_EN("beepboop", "Beep Boop.ags", "d0598090532ce80925e08c05424d8f6b", 2063917),
	GAME_ENTRY("beforethedarkcrystal", "DarkCrystal.exe", "f120690b506dd63cd7d1112ea6af2f77", 15194282),  // En-Fr
	GAME_ENTRY("beforethedarkcrystal2", "Before the Dark Crystal II.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 59569723),  // En-Fr
	GAME_ENTRY_EN("bellyofthebeast", "Belly of the beast.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 22461502),  // Windows
	GAME_ENTRY_EN("bellyofthebeast", "Belly of the beast.ags", "5b086e038c7bf4eada977f52f1d22cd7", 19425322),  // Linux
	GAME_ENTRY_EN("belowzero", "Below Zero.exe", "06a03fe35791b0578068ab1873455463", 2506599),
	GAME_ENTRY_EN("belusebiusarrival", "RoNXXL.exe", "465f972675db2da6040518221af5b0ba", 9426141),
	GAME_ENTRY_EN("belusebiusarrival", "ronextra.exe", "465f972675db2da6040518221af5b0ba", 9958019),
	GAME_ENTRY_EN("benchandlerpi", "BCPI.exe", "9c912b25b6768af44931d7cb40d1dac4", 5815085),
	GAME_ENTRY_EN("bentheredanthat", "btdt.exe", "90413e9ae57e222f8913b09d2bc847bc", 46342499),
	GAME_ENTRY_EN_PLATFORM("bentheredanthat", "btdt.exe", "90413e9ae57e222f8913b09d2bc847bc", 89521873, "Special Edition/Steam"),
	GAME_ENTRY("beprepared", "skavti1.exe", "615e73fc1874e92d60a1996c2330ea36", 23113815),  // Eng-Slo
	GAME_ENTRY_EN("berrybattalion", "Berry.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 1702881),
	GAME_ENTRY_EN("berthabuttsboogie", "berthabutt.exe", "a524cbb1c51589903c4043b98917f1d9", 5684614),
	GAME_ENTRY("besieged", "besieged.exe", "615e806856b7730afadf1fea9a756b70", 11679795),  // En-Fr
	GAME_ENTRY_EN("bestbuddiescorp", "Best Buddies in Corporate Espionage.exe", "c9b5fba30b71684f97ed3df0262cf02f", 944278993),
	GAME_ENTRY_EN("bestowersofeternity", "eternity.exe", "0b66a68c2c8aabe78c80e30b8b82acef", 16187539),
	GAME_ENTRY_EN("betrayal", "OROW.exe", "615e73fc1874e92d60a1996c2330ea36", 3240170),
	GAME_ENTRY_EN("betweenpillars", "Pillars.exe", "04706182ca048f6506d6342f420ae501", 395512813),
	GAME_ENTRY_EN("beyondeternity1", "Eternity1.exe", "5871918713de85ee4bae331ca3284184", 26381630),
	GAME_ENTRY_EN("beyondhorizon", "Beyond Horizon.exe", "338fa79960d40689063af31c671b8729", 173615276),
	GAME_ENTRY_EN("beyondreality", "br.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 10879320),
	GAME_ENTRY_EN("beyondterror", "beyondterror.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8219797),
	GAME_ENTRY_EN("beyondthedoor", "btd.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 6450527),
	GAME_ENTRY("beyondthehorizon", "horizon.exe", "4d7d2addcde045dae6e0363a43f9acad", 15708563),  //En-De
	GAME_ENTRY_EN("bicschristmastale", "AGS - MAGS 1208.exe", "01d0e6bd812abaa307bcb10fc2193416", 2509865),
	GAME_ENTRY_EN("bigbadwolf3lilpiggies", "big bad wolf.exe", "06a03fe35791b0578068ab1873455463", 5812024),
	GAME_ENTRY_EN("bigblue", "WorldDom.exe", "615e73fc1874e92d60a1996c2330ea36", 10079085),
	GAME_ENTRY_EN("bigfoot", "Game.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 97902481),
	GAME_ENTRY_EN("bigglesonmars", "biggles on mars.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 29048931),
	GAME_ENTRY("bigtroubleinlittleimola", "adventure.exe", "495d45fb8adfd49690ae3b97921feec6", 711456988),  // En-It
	GAME_ENTRY_LANG("billdebill", "bdb.exe", "0b7529a76f38283d6e850b8d56526fc1", 9087701, Common::CS_CZE),
	GAME_ENTRY_EN("billybstar", "Billy B Star.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1756026),
	GAME_ENTRY_EN("billyboysimportantwinelottery", "billyboy.exe", "5881d6b88386317dc9d67524a14b11d4", 4872317),
	GAME_ENTRY_EN("billygoatsgruff", "billygoatsgruff.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 5949489),
	GAME_ENTRY("billymasterswasright", "Billy Masters Was Right.exe", "6fe5526eb7dc5b8fe82fb447a4701af7", 44901409),  // En-Es-De
	GAME_ENTRY("billymasterswasright", "Billy Masters Was Right.exe", "30327f199fa3765a9d854813d65cc680", 47726867),  // Win En-Es-De-It
	GAME_ENTRY("billymasterswasright", "Billy Masters Was Right.ags", "811fd3c025c00359150d35e5f830540b", 45057908),  // Linux En-Es-De-It
	GAME_ENTRY("billymasterswasright", "Billy Masters Was Right.exe", "30327f199fa3765a9d854813d65cc680", 47727073),  // Win En-Es-De-It
	GAME_ENTRY_EN("billythekid", "The New Adventures Of Billy The Kid.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 9233573),
	GAME_ENTRY_EN("bioluminescence", "MAGS May.exe", "615e73fc1874e92d60a1996c2330ea36", 12887295),
	GAME_ENTRY_EN("bird", "bird.exe", "2c1fd0f6fb167d5e69080d8c0946a2dc", 110011869),  // Windows
	GAME_ENTRY_EN("bird", "bird.ags", "dfa991c129b488afcf54b957cd8d5710", 107520461),  // Linux
	GAME_ENTRY_EN("birdsandbees", "birdy.exe", "06a03fe35791b0578068ab1873455463", 4250415),
	GAME_ENTRY_EN("birdybirdy", "BirdyBirdy.exe", "f907e03f753f87a08d67ee20719654c2", 21144151),  //  Windows
	GAME_ENTRY_EN("birdybirdy", "BirdyBirdy.ags", "ec6416ed1649d7a829fe6f31be40498a", 8890619),  //Linux
	GAME_ENTRY_EN("bitstream", "Bitstream.exe", "e8617c9bf3fce67aa8da088c6daa4152", 610992645),
	GAME_ENTRY_EN("bittersweet", "Bittersweet.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 42348607),  // v1.02 itch.io
	GAME_ENTRY_EN("biwa", "biwa.exe", "0500aacb6c176d47ac0f8158f055db83", 6109499),
	GAME_ENTRY_EN("bjcase1", "bj1.exe", "4b1378721f4f066f75102f3dca809224", 5678910),
	GAME_ENTRY_EN_PLATFORM("bjcase1", "bj1deluxe.exe", "8275249f4ffbc28c1f10ad09a2786814", 8293704, "Deluxe"),
	GAME_ENTRY_EN("bjcase2", "bj2.exe", "465f972675db2da6040518221af5b0ba", 6318491),
	GAME_ENTRY_EN_PLATFORM("bjcase2", "BJ2Deluxe.exe", "18b284c22010850f79bc5c20054a70c4", 7055507, "Deluxe"),
	GAME_ENTRY_EN("bjcase3", "bj3.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 10199495),
	GAME_ENTRY_EN("bjcase3", "bj3.exe", "61f113e2e52e0dd27aeb438a55dd9b8c", 10841883),
	GAME_ENTRY_EN("bjcase4", "bj4.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 15303392),
	GAME_ENTRY_EN("bjcase5", "bj5.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12745806),
	GAME_ENTRY_EN("bjcase5", "bj5.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 13681827),
	GAME_ENTRY_EN("bjcase6", "bj6.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 18281493),
	GAME_ENTRY_EN("bjcase7", "bj7.exe", "18b284c22010850f79bc5c20054a70c4", 13714066),
	GAME_ENTRY_EN("bjcase8", "BJ8.exe", "98df40b0885b4e5f90f1093987f1f56c", 28334295),
	GAME_ENTRY_EN("bjcults", "the darck colts.exe", "615e73fc1874e92d60a1996c2330ea36", 5977335),
	GAME_ENTRY_EN("bjpww1", "BJWW.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3538560),
	GAME_ENTRY_EN("bjpww1", "BJWW.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3538617),
	GAME_ENTRY_EN("blackbirdpie", "Blackbird.ags", "a17b768f21841090ac4ff66e96744e3a", 3470140),
	GAME_ENTRY_EN("blackbirdstrikesback", "MAGSSept2013.exe", "615e73fc1874e92d60a1996c2330ea36", 4349965),
	GAME_ENTRY_EN("blackfriday", "Black Friday.exe", "6fbfa4a25c909d8c2c1ee06da6b828a3", 7282327),  // Windows v1.0.3
	GAME_ENTRY_EN("blackfriday", "Black Friday.ags", "84ab43eb263d80f844fa5d5910a5512d", 4759175),  // Linux v1.0.3
	GAME_ENTRY_EN("blackhandgang", "The Black Hand Gang.exe", "82da2565c456dcfb265ded6fe3189c0b", 107269914),
	GAME_ENTRY_EN("blackmailinbrooklyn", "brooklyn.exe", "3192c36199d2b0784f866b28da7106d8", 4913695),
	GAME_ENTRY_EN("blacksect1", "Black Sect Remake.exe", "615e73fc1874e92d60a1996c2330ea36", 34503092),
	GAME_ENTRY_EN("blacksect2", "BS2-TCC.ags", "37ffa36feb915ed44cc94e2cec5b67ed", 19706314),
	GAME_ENTRY_EN("blacksect2", "BS2-TCC.exe", "b96aaa8eae38a02f0c8c5262ec0a5057", 22765022),
	GAME_ENTRY_EN("blackudder", "blackudder.exe", "338fa79960d40689063af31c671b8729", 87695958),
	GAME_ENTRY_EN("bladespassion", "blades_of_passion.exe", "3f29c1146aff735c61362fc5162f42e4", 13267106),
	GAME_ENTRY_EN("blazeos", "Ludum Dare 42.ags", "c0d10e40ad4ae03ea3f38f6fd679549a", 6864287),  // Linux
	GAME_ENTRY_EN("blazeos", "Ludum Dare 42.exe", "8aff96231ca059cd61fe5ce68b83f50f", 9351087),  // Windows
	GAME_ENTRY_EN("blindsweeper", "BlindSweeper.exe", "495d45fb8adfd49690ae3b97921feec6", 3437862),
	GAME_ENTRY_EN("blindtosiberia", "blindtosiberia.exe", "e8985d9ffbfa1eda77f2eb8d1331944a", 5899304),
	GAME_ENTRY("blitheep1", "Blithe.exe", "4049012409f119dd39cdba7ed1be17d2", 259848131),  // Eng-Esp-Ita
	GAME_ENTRY_EN("blobward", "BlobGame.exe", "0736c98683a5ee6179edd886ca674909", 27207020),
	GAME_ENTRY_EN("bloodedfields", "Murder.exe", "48038a476d99d23499229a28ac4490cd", 4034796),  // Windows
	GAME_ENTRY_EN("bloodedfields", "Murder.ags", "d3cbbba3cb0b6f0a6c2a7f6665c85e71", 936431),  // Linux
	GAME_ENTRY_EN_PLATFORM("bloodedfields", "Clock.ags", "817d6fbba647f2f5dfcd91ebd4fff17c", 611037, "Beta"),  // Linux beta?
	GAME_ENTRY_EN("bloodyscream", "HorrorGame.exe", "57520fbba5b69ee9da8d04b78cb5babb", 8971158),
	GAME_ENTRY_EN("bluelobe", "Blue Lobe Inc.exe", "4d4b6d6e560cf32f440c39f8d3896da5", 34413450),
	GAME_ENTRY_EN("boardquest", "board quest.exe", "465f972675db2da6040518221af5b0ba", 3862096),
	GAME_ENTRY_EN("boardquest", "board quest.exe", "465f972675db2da6040518221af5b0ba", 3930325),
	GAME_ENTRY_EN("bob", "bob.exe", "0b7529a76f38283d6e850b8d56526fc1", 2874144),
	GAME_ENTRY_EN("bobbycopper", "Bobby Copper.exe", "0710e2ec71042617f565c01824f0cf3c", 1305658),
	GAME_ENTRY("bobescapes", "bob en cavale.exe", "06a03fe35791b0578068ab1873455463", 43264937),  // En-Fr
	GAME_ENTRY_EN("bobgoeshome", "tut.exe", "0710e2ec71042617f565c01824f0cf3c", 1480234),
	GAME_ENTRY_EN("bobgoeshomedeluxe", "bobdeluxe.exe", "0710e2ec71042617f565c01824f0cf3c", 1555913),
	GAME_ENTRY_EN("bobsquest1", "bq19.exe", "97d700529f5cc826f230c27acf81adfd", 2767487),
	GAME_ENTRY_EN("bobsquest2024", "MyNewGame.ags", "203c20dbd01e9f609a366f3cc2fe16b3", 9608322),
	GAME_ENTRY_EN_PLATFORM("bobsquest2024", "Bob's Quest.ags", "fd837adf06717f72830c3d604668cb39", 10828265, "Final"),
	GAME_ENTRY_EN_PLATFORM("bobsquest2024", "Bob's Quest (Test).ags", "24b2d02eab4a03b003b31f8cdd6c3f5d", 12485837, "Re-edit"),
	GAME_ENTRY_EN("bogsadventure", "bogsadventure.exe", "25f919423520b921a041ec854e3a0217", 58789948),
	GAME_ENTRY_EN("bogsadventureineasy3d", "Easy3D.exe", "06a03fe35791b0578068ab1873455463", 13361265),
	GAME_ENTRY_EN("bogsadventureineasy3d", "Easy3D.exe", "70e7ff59a40888b62d4b36076cd51ed1", 14307918),
	GAME_ENTRY_EN("bohemianyard", "by.exe", "bdc14551f03d02b714a949be792a016b", 39643461),
	GAME_ENTRY_EN("boilerroom", "brb.exe", "27343924ddad3be0b97bdcaa71858b1b", 4299483),
	GAME_ENTRY_EN("boltaction", "BAction.exe", "7435dda89567847b6f6ae5211cbdffde", 91472413),
	GAME_ENTRY_EN("bone", "Bone.exe", "eaa4f4b14a9fb5bb59aca188d765b41e", 4079499),
	GAME_ENTRY_EN_PLATFORM("bonest", "Bone Jam 2023.ags", "4b3937728e31fd73ccf5085779effa57", 18176891, "Prototype"),
	GAME_ENTRY_EN("boogiebum", "Feb16Mags.exe", "2ca80bd50763378b72cd1e1cf25afac3", 4664112),
	GAME_ENTRY_EN("boogiebum", "BoogieBum.exe", "2ca80bd50763378b72cd1e1cf25afac3", 4882195),
	GAME_ENTRY_EN("bookofknowledge", "BOK.ags", "f66dbb686ac38f308fb32156f70d1ec2", 4584402),
	GAME_ENTRY_EN("bookofknowledge", "BOK.ags", "d7b9d8923134101b0f1275e139805f63", 4587372),
	GAME_ENTRY_EN("bookofspells4", "BOS4.exe", "db4d303868b86e28a8eca1a4181ea288", 4537869),
	GAME_ENTRY_EN_PLATFORM("bookofspellscomplete", "bos2.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 5724377, "Beta"),
	GAME_ENTRY_EN("bookunfinished", "bookunfinished.exe", "9cb3c8dc7a8ab9c44815955696be2677", 4662400),
	GAME_ENTRY_EN("bovinebyproduct", "cow.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 134670383),
	GAME_ENTRY_EN("bowserquirkquest", "bowser quirk quest.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3046623),
	GAME_ENTRY_EN("box", "box.exe", "0b7529a76f38283d6e850b8d56526fc1", 748638),
	GAME_ENTRY("boxfight", "boxen.exe", "3128b9f90e2f954ba704414ae854d10b", 3132938),  //En-De
	GAME_ENTRY_EN("boxland", "OROW 2008.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 5988324),
	GAME_ENTRY_EN("boyindahood", "boy in da hood.exe", "afe40dc1416dd51e896ee0444d799f07", 12260759),
	GAME_ENTRY_EN("bradbradsonkeyquest", "badness.exe", "0500aacb6c176d47ac0f8158f055db83", 1190580),
	GAME_ENTRY_LANG("braquagegringotts", "xn--Braquage  Grin.exe-nrb", "06a03fe35791b0578068ab1873455463", 86953263, Common::FR_FRA),
	GAME_ENTRY_EN("breakage", "Breakage.exe", "973f6b65820ca1f4e19704a49be99d76", 727818639),
	GAME_ENTRY_EN("breakdown", "breakdown.exe", "710ac163c281a5a539ffe2386264b990", 5273352),
	GAME_ENTRY_EN("breakfastont1", "BoT1.exe", "f08d8d658578a479c72a472380e2c324", 48866312),  // Windows
	GAME_ENTRY_EN("breakfastont1", "BoT1.ags", "8fb32c0f7598562ee747b762c1ffb9e9", 45764084),  // Linux
	GAME_ENTRY_EN("breakingcharacter", "Breaking character.exe", "615e73fc1874e92d60a1996c2330ea36", 23803957),
	GAME_ENTRY_EN("breakofdawn", "At the Break of Dawn.exe", "03741a77615d6ae1bf2cfb9e7dc8d347", 16533234),
	GAME_ENTRY_EN("brexitman", "Brexit-Man.exe", "7ddb9e776648faed5a51170d087074e9", 23004330),  // Win v1.1.1
	GAME_ENTRY_EN("brexitman", "ac2game.dat", "7ddb9e776648faed5a51170d087074e9", 23004330),  // Mac
	GAME_ENTRY_EN("brexitman", "Brexit-Man.ags", "a9066441a4680c289cee851e945b4088", 20517018),  // Linux
	GAME_ENTRY_EN("brinescene", "Brine Scene Finvestigation.exe", "387ff720e746ae46e93f463fd58d77a4", 523863817),
	GAME_ENTRY_EN("brokenwindows1", "Police.exe", "7ddb9e776648faed5a51170d087074e9", 16971333),
	GAME_ENTRY_EN("brokenwindows2", "BW2.exe", "0564de07d3fd5c16e6947a647061913c", 16458558),
	GAME_ENTRY_EN("brokenwindows3", "BW3.exe", "0564de07d3fd5c16e6947a647061913c", 16926857),
	GAME_ENTRY_EN("brokenwindows4", "BW4.exe", "0564de07d3fd5c16e6947a647061913c", 15020205),
	GAME_ENTRY_EN_PLATFORM("brotherswreckers", "MAGS game 11-2019.exe", "8aff96231ca059cd61fe5ce68b83f50f", 11632335, "MAGS"),
	GAME_ENTRY_EN("brotherswreckersep7", "Brothers & Wreckers.exe", "f24c533ce89a2566c157b871f87a4ce4", 19524598),
	GAME_ENTRY_EN("brucequest", "bruce.exe", "f18443f98fd61d2c655e76a17f7da905", 30055288),
	GAME_ENTRY_EN("bruises", "Bruises.exe", "1b9191cfa0fab42776cbaf7b02768a05", 8050720),  //Windows
	GAME_ENTRY_EN("bruises", "Bruises.ags", "95cf2968736e75a32c2f35921887cec0", 5562384),  //Linux
	GAME_ENTRY_EN_PLATFORM("bruises", "Bruises.exe", "45ab64eb8ef455f82fa27fae73ca0785", 8058295, "Post-Jam Fixes"),  //Windows
	GAME_ENTRY_EN_PLATFORM("bruises", "Bruises.ags", "0fda16c094a6d1deafd8dd27aa98c287", 5567399, "Post-Jam Fixes"),  //Linux
	GAME_ENTRY("bsg78", "bsg-e01.exe", "cf1ff01dfb8261f791ac95a7f5f05c1c", 401029955),  // Eng-Tur
	GAME_ENTRY("bubblewrap", "bubble wrap popping simulator 2013.exe", "24275d3b085f0e6f3834311994eb1018", 3166059),  //Multilang
	GAME_ENTRY_EN("bubsybobcat", "rip van bubsy.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 52424482),
	GAME_ENTRY_EN("buccaneer", "buccaneer.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 1576850),
	GAME_ENTRY_EN("buddiestuniverse", "Day off.exe", "7825fa980fcd058caadaf6d3377b1f78", 4655441),
	GAME_ENTRY_EN("buddiestuniverse", "Day off.ags", "ab4fccb762c0757faffdd6e9d9462712", 1619773),
	GAME_ENTRY_EN("buddybrick", "novshag.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 1661270),
	GAME_ENTRY_LANG("buhf", "Buhf.exe", "2fb2ff7aa681aa7f1ecddaf1963a4e2f", 56198893, Common::RU_RUS),
	GAME_ENTRY_EN("bullettrain", "bullet train.exe", "f120690b506dd63cd7d1112ea6af2f77", 17383747),
	GAME_ENTRY_EN("bunawantsbeer", "bwb.exe", "c02022408287355175a601fd5ed1c66d", 2285202),
	GAME_ENTRY_EN("bunnynightout", "BunnysBigNightOut.exe", "63f8a60cc094996bd070e68cb3c4722c", 7195765),
	GAME_ENTRY_EN("bunnyquest", "bunnyquest.exe", "28f82e420b82d07651b68114f90223c8", 1154928),
	GAME_ENTRY_LANG_PLATFORM("burrow", "Burrow.exe", "7971a7c02d414dc8cb33b6ec36080b91", 93329300, Common::PT_BRA, "Prototype"),  // Windows
	GAME_ENTRY_LANG_PLATFORM("burrow", "Burrow.ags", "8280fbbf00abb0d18009a0eadc4eb466", 90832260, Common::PT_BRA, "Prototype"),  // Linux
	GAME_ENTRY_EN("burymeinthesand", "buryme.exe", "f10516e88ec858700804ee69d041aead", 24252498),
	GAME_ENTRY("bustinthebastille", "Bastille.exe", "42f8802095baac3b5ed134635673cead", 722888946), // 1.05 En-Fr
	GAME_ENTRY_EN_PLATFORM("bustinthebastille", "MAGS june 2017.exe", "e3962995a70923a8d5a8f1cf8f932eee", 304129558, "MAGS"),  //Windows
	GAME_ENTRY_EN_PLATFORM("bustinthebastille", "MAGS june 2017.ags", "066517c5d7d44f58bf522de602b58427", 301648902, "MAGS"),  //Linux
	GAME_ENTRY_EN("butcherstanys", "stanys.exe", "97d700529f5cc826f230c27acf81adfd", 1404933),
	GAME_ENTRY_EN("butcherstanys2", "stanys.exe", "97d700529f5cc826f230c27acf81adfd", 1489182),
	GAME_ENTRY_EN("buttercup", "Buttercups gone missing!.exe", "4d17844029d8910fbaae1bdc99e250f2", 2854591),
	GAME_ENTRY_EN("byohero", "BYO-Hero.exe", "f0dd8a7e2c0e033d8f928e65aa54b10f", 163626763),
	GAME_ENTRY_EN("bytebetrayal", "Byte-Sized Betrayal.exe", "bf0c8894a91378e7772dbb0a930b188d", 167768171),
	GAME_ENTRY_EN("bythenumbers", "By the Numbers.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 15781843),
	GAME_ENTRY_EN("cabbagequest", "Cabbagequest.exe", "06a03fe35791b0578068ab1873455463", 4916170),
	GAME_ENTRY_EN("cabbagesandkings", "fal.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3611642),  // v2.0
	GAME_ENTRY_LANG("cabintrouble", "Cabin trouble.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9968308, Common::DE_DEU),
	GAME_ENTRY_LANG("cachoquest", "CQ.exe", "06a03fe35791b0578068ab1873455463", 3166350, Common::ES_ESP),
	GAME_ENTRY_LANG_PLATFORM("cadaverheart", "Cadaver_Heart.ags", "94589ed820dbc041429159d8b1a9f55f", 3814577, Common::FR_FRA, "Unfinished"),  // itch.io
	GAME_ENTRY_EN("calebsdrunkenadventure", "calebdru.exe", "0b7529a76f38283d6e850b8d56526fc1", 15484923),
	GAME_ENTRY_EN("calequest", "CQ1.exe", "dbebe4ccfe277d5d040ddb0b83265508", 361215392),  // v1.0
	GAME_ENTRY_EN("callmegeorge1", "Call Me George.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3872581),
	GAME_ENTRY_EN("calsoon2", "looncalsoon.exe", "97d700529f5cc826f230c27acf81adfd", 18981033),
	GAME_ENTRY_EN("capricorn", "capricorn.exe", "06a03fe35791b0578068ab1873455463", 4817076),
	GAME_ENTRY_EN("camp1", "Camp1.exe", "d0c1d3be980d97e592ffe16b71a3b9f9", 34131784),
	GAME_ENTRY_EN("campaigndaphnewhite", "The Campaigne of Daphne White.exe", "7446ce302043dac2acc8cd5cc7cc8f68", 703443005),
	GAME_ENTRY_EN("campaigndaphnewhite", "The Campaigne of Daphne White.ags", "2ffa994c7198ed96f4425fae6932071a", 700961325),
	GAME_ENTRY_EN("cancer", "cancer.exe", "9cf51833e787cc919837d9a8bd8fc14c", 632811),
	GAME_ENTRY_EN("candlecove", "CandleCove.exe", "615e73fc1874e92d60a1996c2330ea36", 3143609),
	GAME_ENTRY_EN("candyforest", "CandyForest.exe", "7a3096ac0237cb6aa8e1718e28caf039", 3959806),
	GAME_ENTRY_EN("captaincringe", "captain_cringe.exe", "dd8586ebefd5c457e29b6b9845a576ea", 2631464),
	GAME_ENTRY_EN("captaindisastermoon", "CDiTDSotM.exe", "134528bfbb398455d06fa1dc1db5ca85", 44284638),
	GAME_ENTRY_EN_STEAM("captaindisastermoon", "CDiTDSotM.exe", "ca313f3c6ade23c16e764f21940b0863", 44604440),
	GAME_ENTRY("captaindownes", "Captain Downes.exe", "78dd4ca028ee0156b6a093d6d780aa65", 32770340),  // Windows Eng-Swe
	GAME_ENTRY("captaindownes", "Captain Downes.ags", "01c5914ae83d39dab345f2f70dd3e4ee", 30285588),  // Linux Eng-Swe
	GAME_ENTRY("captaindownes", "ac2game.dat", "01c5914ae83d39dab345f2f70dd3e4ee", 30285588),  // Mac Eng-Swe
	GAME_ENTRY("captainhook", "CaptainHookAndTheLostGirl.exe", "776a62db4387dc68be92ef9933399fd5", 8731939),  // Win En-Fr
	GAME_ENTRY("captainhook", "CaptainHookAndTheLostGirl.ags", "f057e9513fd792aa6b60c6e0555b3577", 5625615),  // Linux En-Fr
	GAME_ENTRY_EN("captainskull", "The Astonishing Captain Skull.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 21319808),
	GAME_ENTRY_EN("carrotbobinzxspeccyworld", "carrotbob.exe", "949f7440e3692b7366c2029979dee9a0", 914509),
	GAME_ENTRY_EN("carnivalshadows", "CarnivalOfShadows.ags", "b88ce232cf50d0e3fe7d58947ff908fb", 27140624),
	GAME_ENTRY_EN("cartenstein", "Ludum Dare 41.ags", "8c15c837060a65decd61d3a99c78a15c", 1655510),  // Linux
	GAME_ENTRY_EN("cartenstein", "Ludum Dare 41.exe", "78dd4ca028ee0156b6a093d6d780aa65", 4140262),  // Windows
	GAME_ENTRY_EN("cartlife", "cartlife.exe", "7c8d67dd922fa543a1b1b76f9e1cc7c2", 21045074),
	GAME_ENTRY_EN("casablancathedayafter", "casablanca.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 4238105),
	GAME_ENTRY_EN("casenoir", "case noir.exe", "fd3ecd8289bebadbf775fe8a13b9c5d7", 15407193),
	GAME_ENTRY_EN("casenoir", "case noir.exe", "2ca80bd50763378b72cd1e1cf25afac3", 15401399),
	GAME_ENTRY_EN("caseofthefestivalfilcher", "Sidekick.exe", "615e73fc1874e92d60a1996c2330ea36", 5578607),
	GAME_ENTRY_EN("caseofthemuffindiver", "TheMuffinDiver.exe", "0241777c2537fc5d077c05cde10bfa9f", 5344954),
	GAME_ENTRY_EN("cassandra", "GPCv16.exe", "615e73fc1874e92d60a1996c2330ea36", 46787711),
	GAME_ENTRY_EN("castleescapech1", "CastleEscapeAdventure.exe", "3ab9922a2ab7c5875af4c9794b33e012", 4611747),  // Windows
	GAME_ENTRY_EN("castleescapech1", "CastleEscapeAdventure.ags", "32201bc4a4bb0e678837e8b798094cb5", 1507983),  // Linux
	GAME_ENTRY_EN("castleoffire", "castle of fire.exe", "b6f0726bd5776abee0d452b8f1073850", 14800915),
	GAME_ENTRY_EN("catacombic", "catacombic.exe", "057d1aa29f6fadd83209268efcfb35a3", 7317791),
	GAME_ENTRY_EN("catapault", "catapault.exe", "290afe0bac54418822f15175e474731d", 8286661),
	GAME_ENTRY_EN("catking", "pcjaloise.exe", "0241777c2537fc5d077c05cde10bfa9f", 652032211),
	GAME_ENTRY_EN("cauche", "Cauchemarionto.exe", "5f4c694e6dc897dc417fabd8ce0844e1", 5608334),
	GAME_ENTRY_EN("caverns", "Caverns.exe", "c9cc759756e1f24783029948da533ccd", 20800343),
	GAME_ENTRY_EN("caveofavarice", "CaveOfAvarice.exe", "f907e03f753f87a08d67ee20719654c2", 103572644),  // Windows
	GAME_ENTRY_EN("caveofavarice", "CaveOfAvarice.ags", "0cc8aacba64fb6e53944d1ad37efac70", 100600464),  // Linux
	GAME_ENTRY_EN("cayannepepper", "cpepper.exe", "06a03fe35791b0578068ab1873455463", 16117141),
	GAME_ENTRY_EN_PLATFORM("cedricandtherevolution", "elves!.exe", "b216ee957dd677023e02d900281a02d6", 10112961, "MAGS"),  // v1.0
	GAME_ENTRY_EN_PLATFORM("cedricandtherevolution", "elves!.exe", "b216ee957dd677023e02d900281a02d6", 10114667, "MAGS"),  // v1.1
	GAME_ENTRY_EN("cedricandtherevolution", "elves!.exe", "b216ee957dd677023e02d900281a02d6", 10205755),  // v1.1c
	GAME_ENTRY_EN("cedricshooter", "CedricShooter.exe", "236d705f7a02fa1f44f734fb9eb054a5", 7101976),  // Windows
	GAME_ENTRY_EN("cedricshooter", "CedricShooter.ags", "b44a79cd3b24a270f276f2b014e0b239", 4601352),  // Linux
	GAME_ENTRY_EN("celestialcatastrophe", "CelestialCatastrophe.exe", "495d45fb8adfd49690ae3b97921feec6", 5164082),  // Win v1.0
	GAME_ENTRY_EN("celestialcatastrophe", "CelestialCatastrophe.ags", "bd01dfd542715abbfe14578cf62d370d", 2683426),  // Linux
	GAME_ENTRY_EN("celticchaosep1", "CC1.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 18627665),
	GAME_ENTRY_EN("celticchaosep2", "cc2.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 17463014),
	GAME_ENTRY_EN("cgascreensaver", "CGASAVER12.exe", "ba86f02ab89fb54164e226bdcbd1d5e7", 387228637),
	GAME_ENTRY_LANG("chaelle1", "chaelle.exe", "776a62db4387dc68be92ef9933399fd5", 2020837287, Common::FR_FRA),
	GAME_ENTRY("chaelle1", "chaelle.exe", "2e8d5f27d491676f4825881974e4327c", 2019480416),  // En-Fr
	GAME_ENTRY_LANG("chaelle2", "chaelle.exe", "776a62db4387dc68be92ef9933399fd5", 979823699, Common::FR_FRA),
	GAME_ENTRY("chaelle2", "chaelle.exe", "2e8d5f27d491676f4825881974e4327c", 978560344),  // En-Fr
	GAME_ENTRY_LANG("chaelle3", "chaelle 3.exe", "776a62db4387dc68be92ef9933399fd5", 1123683160, Common::FR_FRA),
	GAME_ENTRY("chaelle3", "chaelle 3.exe", "2e8d5f27d491676f4825881974e4327c", 1122406157),  // En-Fr
	GAME_ENTRY_EN("chalkman", "Chalkman.exe", "d93777b4ac9542258ab4013ad14b271e", 102049536),
	GAME_ENTRY_EN("chalksquest", "chalk.exe", "0710e2ec71042617f565c01824f0cf3c", 5138686),
	GAME_ENTRY_EN("chalksquest", "chalk.exe", "0710e2ec71042617f565c01824f0cf3c", 5138049),
	GAME_ENTRY("challengetentacle", "Challenge of the Tentacle.exe", "44d2715f4d78a639a588e94cf19b5821", 489037337),  // Win En-De
	GAME_ENTRY("challengetentacle", "Challenge of the Tentacle.ags", "b24cdc761ea244ed460f32329627cc45", 485577733),  // Linux En-De
	GAME_ENTRY("challengetentacle", "Challenge of the Tentacle.exe", "44d2715f4d78a639a588e94cf19b5821", 489037341),  // Win En-De v1.0.2
	GAME_ENTRY("challengetentacle", "Challenge of the Tentacle.ags", "deed6ad8466f2cdc31fe84112a2f949d", 485577737),  // Linux En-De v1.0.2
	GAME_ENTRY_EN("chaluul", "Chaluuls_Curse.exe", "cb8f06d6a64aff7382fed5846f62ccdc", 52242488),  // Win/Mac
	GAME_ENTRY_EN("chaluul", "Chaluuls_Curse.ags", "3d246c0271974ddbd4ada442066846eb", 49056804),  // Linux
	GAME_ENTRY_EN("chanceofthedead", "chance.exe", "8418b150c267e4f1d462b9a5d60e507d", 5597569),
	GAME_ENTRY_EN("charamba1", "Charamba.exe", "354e3016d64ac3378478d3d52ef0460d", 7468361),
	GAME_ENTRY_EN("charamba2", "CharambaII.exe", "354e3016d64ac3378478d3d52ef0460d", 4975542),
	GAME_ENTRY_EN("charliefoxtrot", "foxtrot.exe", "0500aacb6c176d47ac0f8158f055db83", 48626762),
	GAME_ENTRY_EN("chasingrobot", "1st game.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 4687601),
	GAME_ENTRY_EN("chatroom", "chatroom.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 6760748),
	GAME_ENTRY_EN_PLATFORM("chatroom", "chatroom.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 6055942, "OROW"),
	GAME_ENTRY_EN("cheerfulscience", "cheerfulscience.exe", "06a03fe35791b0578068ab1873455463", 7740040),
	GAME_ENTRY_EN("cheetahs", "Cheetah.exe", "467643a8f3484ce5f384979773245b7c", 102544802),  // GainJam entry
	GAME_ENTRY_EN_PLATFORM("cheetahs", "Cheetah.exe", "bf02e43fecd1f66bfa7ee2901cb6c6b4", 125589192, "post-GainJam fix"),
	GAME_ENTRY_EN("chekken", "chekken.exe", "bde175c0c4d87a59a7a082be595d08da", 13477393),
	GAME_ENTRY_EN("cherrysquest", "Coffee.exe", "1b0541ad9e6b6d3ceed71f6757f275c0", 9090885),
	GAME_ENTRY_EN("chessboard", "ChessBoard.exe", "82da2565c456dcfb265ded6fe3189c0b", 2638884),
	GAME_ENTRY_EN("chezapa", "chezapa.exe", "9cf51833e787cc919837d9a8bd8fc14c", 2870327),
	GAME_ENTRY_EN("chickchaser", "amy.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7967935),
	GAME_ENTRY_EN("chickchaser", "amy.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7968024),
	GAME_ENTRY_EN("chicken", "chicken.exe", "f120690b506dd63cd7d1112ea6af2f77", 1652888),
	GAME_ENTRY_EN("chickenfraction", "chicken.exe", "0500aacb6c176d47ac0f8158f055db83", 3021315),
	GAME_ENTRY_EN("chickenvsroad", "chicken.exe", "f120690b506dd63cd7d1112ea6af2f77", 30627567),  //v2.71
	GAME_ENTRY_EN("chinesecheckers", "chinesecheckers.exe", "06a03fe35791b0578068ab1873455463", 3391826),
	GAME_ENTRY_EN("chlorinde", "ChlorindeMAGS.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 10118602),
	GAME_ENTRY("chmelnica", "CHMELNICA.exe", "0b7529a76f38283d6e850b8d56526fc1", 15145197),  // Eng-Deu-Svk
	GAME_ENTRY_LANG("chocofrogs", "Chocofrogs.exe", "615e73fc1874e92d60a1996c2330ea36", 6612789, Common::FR_FRA),
	GAME_ENTRY_EN("chongoadv", "ChinaGame.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 31148695),
	GAME_ENTRY_EN("christmas42", "christmas_42.exe", "97f74f77ff127215fc44f35958fd2d55", 1546010),
	GAME_ENTRY_EN("christmasgamearcade", "Xmas_2015.exe", "b5c41e08919834b549ca350a28f18f87", 7363985),
	GAME_ENTRY_EN("christmashunt", "MyXmasGame.exe", "0241777c2537fc5d077c05cde10bfa9f", 3170875),
	GAME_ENTRY_EN("christmaspresent", "christmas.exe", "3128b9f90e2f954ba704414ae854d10b", 5644948),
	GAME_ENTRY_EN("christmasquest", "christmas quest.exe", "f120690b506dd63cd7d1112ea6af2f77", 14042816),
	GAME_ENTRY_EN("christmasquest2", "christmasquest2.exe", "06a03fe35791b0578068ab1873455463", 22683950),
	GAME_ENTRY_EN("christmasquest3", "cq3.exe", "f0a7712890942b9155193cc4488a07bc", 50854625),
	GAME_ENTRY_EN("cirquedezale", "cirque.exe", "3128b9f90e2f954ba704414ae854d10b", 8556710),  // v1.2
	GAME_ENTRY_EN("cirquedezale", "cirque.exe", "3128b9f90e2f954ba704414ae854d10b", 8547131),  // v1.4
	GAME_ENTRY_EN("cityofthieves", "City of Thieves.exe", "74dc062c5f68f3d70da911f2db5737b6", 54527105),
	GAME_ENTRY_EN("claire", "claire.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 2781456),
	GAME_ENTRY_EN("clarinette", "Clarinette.exe", "0eb179eb70b64b7a0d3da3585430d209", 14022754),
	GAME_ENTRY_LANG("classnotes", "classnotes.exe", "0710e2ec71042617f565c01824f0cf3c", 5706497, Common::ES_ESP),
	GAME_ENTRY("classnotes", "classnotes.exe", "0710e2ec71042617f565c01824f0cf3c", 5706836),  // v1.1 En-Es
	GAME_ENTRY_EN_PLATFORM("clik", "Clik.exe", "0564de07d3fd5c16e6947a647061913c", 4043815, "Prototype"),
	GAME_ENTRY_EN_PLATFORM("clik", "Clik.ags", "fb8c540550446567046e6664a9a1174b", 1011219, "Prototype"),
	GAME_ENTRY_EN_PLATFORM("clik", "ClikBeta.exe", "0564de07d3fd5c16e6947a647061913c", 4395443, "Beta 0.1"),
	GAME_ENTRY_EN_PLATFORM("clik", "ClikVersion.ags", "615479125542862ac715536b90a9e800", 1362847, "Beta 0.1"),
	GAME_ENTRY_EN_PLATFORM("clik", "ClikVersion.exe", "0564de07d3fd5c16e6947a647061913c", 4528016, "Beta 0.1.1"),
	GAME_ENTRY_EN_PLATFORM("clik", "ClikVersion.ags", "a4397e15322d1198c4d86929cb644b31", 1495420, "Beta 0.1.1"),
	GAME_ENTRY_EN_PLATFORM("clik", "ClikVersion.exe", "0564de07d3fd5c16e6947a647061913c", 4625906, "Beta 0.1.2"),
	GAME_ENTRY_EN_PLATFORM("clik", "ClikVersion.ags", "18eab6dd984ff24688fbd7f9c238903b", 1593310, "Beta 0.1.2"),
	GAME_ENTRY_EN("clik", "ClikVersion.exe", "0564de07d3fd5c16e6947a647061913c", 5246623),
	GAME_ENTRY_EN("clik", "ClikVersion.ags", "7f60934b9ad0e2bdf82e4d755056bf24", 2214027),
	GAME_ENTRY_EN("clipgoestotown", "clip goes to town.exe", "0710e2ec71042617f565c01824f0cf3c", 1690928),
	GAME_ENTRY_EN("clockworklabyrinth", "clockwork.exe", "c15b2eb56afb3a330474b0fd124a3c9d", 638025412),  //v1.1
	GAME_ENTRY_EN("clotildesoffritti", "Clotilde astronave.exe", "286551ded3eedc428451e506e29196c9", 368664276),
	GAME_ENTRY_EN("clotildesoffritti", "Clotilde astronave.exe", "286551ded3eedc428451e506e29196c9", 368848300),
	GAME_ENTRY_EN("clotildesoffritti2", "Clotilde 2.exe", "6b1455146ef5f67b4ff83cc60646b964", 107838654),
	GAME_ENTRY_EN("clotildesoffritti2", "Clotilde 2.exe", "6b1455146ef5f67b4ff83cc60646b964", 108688904),
	GAME_ENTRY_EN("clownshow", "p_Verb Coin.ags", "00e0805674e65f2ac8b6c192cf79e7c8", 15952118),
	GAME_ENTRY_EN("clubofevil", "club of evil.exe", "65f53f81071dab6b3ab8363e4c76d12e", 11838011),
	GAME_ENTRY_LANG("coderbattle", "coderbattle.exe", "8bae5a42f382b38d516a7f8c8f686376", 23992763, Common::DE_DEU),
	GAME_ENTRY_EN("coffinvalley", "CoffinValley.exe", "f560521a26ca8b76956d283dc4b31091", 98909190),  // v1.0.4
	GAME_ENTRY_EN("coinopafternoon", "Let's Play Something.exe", "615e73fc1874e92d60a1996c2330ea36", 1225364411),  //English
	GAME_ENTRY_LANG("coinopafternoon", "Let's Play Something.exe", "e80586fdc2db32f65658b235d8cbc159", 900630574, Common::FR_FRA),  //French
	GAME_ENTRY_EN("coinrush", "CoinRush.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1035234),
	GAME_ENTRY_EN("coinrush2", "coinrsh2.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1260438),
	GAME_ENTRY_EN("colaskunk", "MAGSAug2018.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 17792401),
	GAME_ENTRY_EN("coldhandreef", "WMAGS_0815.exe", "2ca80bd50763378b72cd1e1cf25afac3", 117129193),
	GAME_ENTRY("coldmeat", "Cold Meat.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 41382750),  // En-De
	GAME_ENTRY_EN("coldstorage", "cold storage.exe", "06a03fe35791b0578068ab1873455463", 2438370),
	GAME_ENTRY_EN("coldstorage", "coldstorage.exe", "f8e8f781a4a95c2bfea5f54b085df550", 3632485),
	GAME_ENTRY_EN("colinsimpson", "office.exe", "06a03fe35791b0578068ab1873455463", 11781006),
	GAME_ENTRY_EN_PLATFORM("colonelcarver", "Colonel Carver.exe", "7ddb9e776648faed5a51170d087074e9", 7002643, "MAGS"),
	GAME_ENTRY_EN("colonelcarver", "Colonel Carver.exe", "7ddb9e776648faed5a51170d087074e9", 9297683),
	GAME_ENTRY_EN("colourclash", "colourclash.exe", "0241777c2537fc5d077c05cde10bfa9f", 2803703),
	GAME_ENTRY_EN("colourwise", "colourwise.exe", "473f0e2fd72b747cef62d91090ab74c7", 24461117),
	GAME_ENTRY_EN("colourwiseleveleditor", "colourwise - Level Editor.exe", "3e09048ec0763a90ffa4eaeeb8f96df5", 3365519),
	GAME_ENTRY_EN("columbuslander", "columbus lander.exe", "779421cd8ad5268e2efdb26e04608db7", 48744943),
	GAME_ENTRY_EN("comedyquest", "Comedy Quest.exe", "e46016dff70fa37c1e9f24eec9ae55a0", 22351775),
	GAME_ENTRY_EN_STEAM("comedyquest", "Comedy Quest.exe", "7a0123d8f6575e9954a18125456515f4", 22381783),
	GAME_ENTRY_EN_PLATFORM("cometcollision", "CometCollision.exe", "7c6e063343fc2ec2bfffc93a1bbd6cfe", 6027272, "MAGS"),
	GAME_ENTRY_EN("cometcollision", "CometCollision.exe", "5b916edf70413f20906ceb9c05ac65b9", 6035658),  // v2-v3
	GAME_ENTRY_LANG("comicsquest", "Comics Quest.exe", "93ff64eb8c917a738ca7d3da67ff7509", 6577187, Common::CS_CZE),
	GAME_ENTRY_EN("commandoadv", "commando.exe", "0736c98683a5ee6179edd886ca674909", 6225721),
	GAME_ENTRY_EN("comradecitizenpart1", "citizen.exe", "06a03fe35791b0578068ab1873455463", 2174301),
	GAME_ENTRY_EN("concurrence", "Concurrence.exe", "27343924ddad3be0b97bdcaa71858b1b", 57168217),  //v1.1
	GAME_ENTRY_EN("concurrence", "Concurrence.exe", "27343924ddad3be0b97bdcaa71858b1b", 58196965),  //v1.2
	GAME_ENTRY_EN("confessionsofacatburglar", "cat.exe", "0500aacb6c176d47ac0f8158f055db83", 1328541),
	GAME_ENTRY_LANG("confinement", "Confinement.exe", "0564de07d3fd5c16e6947a647061913c", 182447106, Common::FR_FRA),
	GAME_ENTRY_LANG("confinement", "Confinement.ags", "082d23121b2c790301c42bd882b60620", 179414510, Common::FR_FRA),
	GAME_ENTRY_EN("coniferskunks", "ConiferSkunksMAGS.exe", "7ddb9e776648faed5a51170d087074e9", 16898660),
	GAME_ENTRY_EN("conspiracybelowzero", "conspiracy.exe", "338fa79960d40689063af31c671b8729", 395111008),
	GAME_ENTRY_EN("conspiracyofsongo", "songo.exe", "934a3d245739d7ac66c021f8409c1044", 31526905),
	GAME_ENTRY_EN("constancethebarbarian", "ctb.exe", "0500aacb6c176d47ac0f8158f055db83", 1150440),
	GAME_ENTRY_EN("contact", "Contact.exe", "c40014c84e8d2d081154105ba4361849", 25017442),  // Windows Eng
	GAME_ENTRY_LANG("contact", "Contact_Kor.exe", "b16cdffccf0a9488a2d623ced3b6dfdd", 26246494, Common::KO_KOR),  // Windows Korean
	GAME_ENTRY_EN("content", "Content.exe", "9878f9ded585f7191df2eebc01867c6e", 29828265),  // Windows Eng
	GAME_ENTRY_LANG("content", "Content_Kor.exe", "9878f9ded585f7191df2eebc01867c6e", 30127981, Common::KO_KOR),  // Windows Korean
	GAME_ENTRY_EN("content", "Content.ags", "7a0e6b41e951aedd4385c10c15513835", 26690197),  // Linux Eng
	GAME_ENTRY("content", "Content.exe", "1e950496692a009ea163eb276702e008", 29814856),  // Windows Eng-Kor
	GAME_ENTRY("contrapasso", "Commissar's Contrapasso.exe", "164da3f495c1fff9695893ec6622f4eb", 440822669),  // Win Eng-Swe
	GAME_ENTRY("contrapasso", "Commissar's Contrapasso.ags", "3e99c4265fbfcd89f04fc85d367289cd", 438328189),  // Linux Eng-Swe
	GAME_ENTRY("contrapasso", "ac2game.dat", "3e99c4265fbfcd89f04fc85d367289cd", 438328189),  // Mac Eng-Swe
	GAME_ENTRY_EN("cop", "COP.exe", "465f972675db2da6040518221af5b0ba", 6439577),
	GAME_ENTRY("corneliuschristmas", "CorneliusCatChristmas.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 18946254),  // En-De
	GAME_ENTRY_EN("corneliuspest", "CorneliusCat.exe", "b5c41e08919834b549ca350a28f18f87", 26410221),
	GAME_ENTRY_EN_PLATFORM("corneliuspest", "CorneliusCat.exe", "b5c41e08919834b549ca350a28f18f87", 26410225, "OROW"),
	GAME_ENTRY_EN("cornersshiny", "Corner's Shiny.exe", "74dc062c5f68f3d70da911f2db5737b6", 2245296),
	GAME_ENTRY_EN("corpoconnections", "CorpoConnections.ags", "3aaa9b607682b173dc73811ee86fa135", 1655403),
	GAME_ENTRY_EN("cosmodyssey", "KosmoGameJam.exe", "495d45fb8adfd49690ae3b97921feec6", 663963764),
	GAME_ENTRY_EN("cosmospuzzle", "space.exe", "0500aacb6c176d47ac0f8158f055db83", 3591766),
	GAME_ENTRY_EN("cosmosquest1", "tfas1.exe", "ff3d6e4edfca8b4f4f1c6cbf8e2781a6", 16653211),
	GAME_ENTRY_EN("cosmosquest2", "tfas2-32bit.exe", "ff3d6e4edfca8b4f4f1c6cbf8e2781a6", 118740291),
	GAME_ENTRY_EN("cosmotrinket", "Cosmo's Trinket Hunt.ags", "55b0e88908e0fc7363a9910063117620", 489140),
	GAME_ENTRY_EN("cougarsquestforfreedom", "chra.exe", "0500aacb6c176d47ac0f8158f055db83", 12654043),
	GAME_ENTRY_EN("cougarsquestforfreedom", "chra.exe", "0500aacb6c176d47ac0f8158f055db83", 12590916),  //v1.0
	GAME_ENTRY_EN("cougarsquestforfreedom", "cqff.exe", "0500aacb6c176d47ac0f8158f055db83", 12590916),
	GAME_ENTRY_EN("counterfeit", "counterfeit.exe", "ef1645ccd3d16691ec3908c91f340c34", 2232297),
	GAME_ENTRY_EN("coupdecup", "newgame.exe", "949f7440e3692b7366c2029979dee9a0", 9635719),
	GAME_ENTRY_EN("coyote1", "coyote.exe", "a524cbb1c51589903c4043b98917f1d9", 33124533),
	GAME_ENTRY_EN("crackwell1", "Crackwell.exe", "06a03fe35791b0578068ab1873455463", 2990976),
	GAME_ENTRY_EN("crackwell2", "Crackwell2.exe", "06a03fe35791b0578068ab1873455463", 7979141),
	GAME_ENTRY_EN("craftofevil", "craft of evil.exe", "4c1d9a74c4acf6771aab4be704bf0797", 22409329),
	GAME_ENTRY_EN("crankosaurus", "Crankosaur.exe", "00ca3a1eb345f1c3b0a88657b4087aa6", 37732797),
	GAME_ENTRY_EN("crankosaurusff", "Crankosaur.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 60705925),
	GAME_ENTRY_EN("crashcourse", "Crash course.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3354483),
	GAME_ENTRY_EN("crashedalien", "Crashed.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2470885),
	GAME_ENTRY_EN("crashevadedestroy", "ced.exe", "06a03fe35791b0578068ab1873455463", 1938399),
	GAME_ENTRY_EN_PLATFORM("crave", "crave.exe", "28f82e420b82d07651b68114f90223c8", 196145919, "MAGS"),  //v1.0
	GAME_ENTRY_EN_PLATFORM("crave", "crave.exe", "28f82e420b82d07651b68114f90223c8", 205580299, "MAGS"),  //v1.01
	GAME_ENTRY_EN_PLATFORM("crave", "crave.exe", "28f82e420b82d07651b68114f90223c8", 205750825, "MAGS"),  //v1.02
	GAME_ENTRY_EN_PLATFORM("crave", "crave.exe", "28f82e420b82d07651b68114f90223c8", 205890180, "MAGS"),  //v1.04
	GAME_ENTRY_EN("crave", "crave.exe", "7a436b9a2e8e80c8aaaeed09ef9e3d3d", 59212467),  //v1.09
	GAME_ENTRY_EN("crave", "crave.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 59222727),  //v1.09 author site
	GAME_ENTRY_EN("crepefields", "Crepe Fields.exe", "0e4ce1f719c288c65227f6073186bed7", 5928778),  // Windows
	GAME_ENTRY_EN("crepefields", "Crepe Fields.ags", "c59c4cf5d4bb0ba6a6c5fe63b740c577", 3430714),  // Linux
	GAME_ENTRY_EN("crepefields", "Crepe Fields.exe", "65d6e8fd7ed5d98e2c645c8af0cc18b2", 6023505),  // Windows 2.0
	GAME_ENTRY_EN("crepefields", "Crepe Fields.ags", "ecbc925e896e51efa3c421d00c03bb8c", 3520321),  // Linux 2.0
	GAME_ENTRY_EN("crimezone", "LD48.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 3786245),
	GAME_ENTRY_EN("criminalist", "Onromm.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1388001),
	GAME_ENTRY_EN("crossstitch", "Cross Stitch Casper.exe", "d44551532361c1eeec9b167de35515d1", 123801381),
	GAME_ENTRY_EN("crowandfoxy", "LeCorbeauetlaRenarde.exe", "615e73fc1874e92d60a1996c2330ea36", 15444218),
	GAME_ENTRY_EN("crowcawled", "A Crow Cawled Raven.exe", "478e594ff7af5068537a90b6e4ddeea6", 9379693),  // Windows
	GAME_ENTRY_EN("crowcawled", "A Crow Cawled Raven.ags", "38f6a8ae22aeaebc2513a8448adbd9e7", 6339929),  // Linux
	GAME_ENTRY_EN("cryo", "cryo.exe", "615e73fc1874e92d60a1996c2330ea36", 27992619),
	GAME_ENTRY_EN("crypt", "crypt.exe", "0500aacb6c176d47ac0f8158f055db83", 2089059),
	GAME_ENTRY_EN("cryptic", "cryptic.exe", "d9143b143b011d7bd8726c1bb5dabc59", 6027654),
	GAME_ENTRY_EN("crystalquest", "MarchMAGS.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2284362),
	GAME_ENTRY_LANG("crystalstory", "LittleChristmasTrial.exe", "55efec16c51fcbe3748009434fa56251", 10627427, Common::DE_DEU),
	GAME_ENTRY_EN("csihunt1", "csi hunt 1.exe", "06a03fe35791b0578068ab1873455463", 2190662),
	GAME_ENTRY_EN("cspb", "cspb.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 77296215),
	GAME_ENTRY_EN_PLATFORM("cullfield", "Detective Ian.exe", "7a3096ac0237cb6aa8e1718e28caf039", 7501660, "Prototype"),
	GAME_ENTRY_EN("cursevampire", "MAGS_MARZO.exe", "06a03fe35791b0578068ab1873455463", 6973036),
	GAME_ENTRY_EN("cutman", "cutman.exe", "0564de07d3fd5c16e6947a647061913c", 7962851),
	GAME_ENTRY_EN("cyberjack", "brewton24.exe", "e3962995a70923a8d5a8f1cf8f932eee", 3439339),
	GAME_ENTRY_EN("daceyinthedark", "Darkness.exe", "27343924ddad3be0b97bdcaa71858b1b", 6353848),
	GAME_ENTRY_EN("dada", "Dada.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3992871),
	GAME_ENTRY_EN("dadaxmas", "DadaXmas.exe", "2986195aec1d206f8e9c080b3232b25e", 13170134),
	GAME_ENTRY_EN("daggerhell", "DaggerHell.exe", "7971a7c02d414dc8cb33b6ec36080b91", 3614181),
	GAME_ENTRY_EN("daggerhell", "DaggerHell.ags", "5c833e0d8c484c904732ee1d14ebcafb", 1117141),
	GAME_ENTRY_EN("dakota", "Dakota.exe", "615e73fc1874e92d60a1996c2330ea36", 12745994),
	GAME_ENTRY_EN("daleks", "Daleks.exe", "0241777c2537fc5d077c05cde10bfa9f", 2409634),
	GAME_ENTRY_EN("dalesfilmquest", "dale.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2557975),
	GAME_ENTRY_EN("damsel", "Damsel.exe", "809418706c429cee5d88e8d483c906cc", 77101995),
	GAME_ENTRY_EN("danbeard", "DansBeard.exe", "f120690b506dd63cd7d1112ea6af2f77", 1350747),
	GAME_ENTRY_EN("dancetilyoudrop", "DanceTilYouDrop.exe", "0710e2ec71042617f565c01824f0cf3c", 4682843),
	GAME_ENTRY_EN_PLATFORM("dancetilyoudrop", "DanceTilYouDrop.exe", "0710e2ec71042617f565c01824f0cf3c", 4822399, "EX"),
	GAME_ENTRY_EN("danewguys", "DaNewGuys.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 12115013),
	GAME_ENTRY_EN("danewguys", "DaNewGuys.exe", "97020973a2a635fe28eb0ba4bdfaa70b", 11998791),
	GAME_ENTRY_EN("dangerousderek", "DangerousDerek.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 7416843),
	GAME_ENTRY("dangerouslandsrt2", "DL_RT2.exe", "0710e2ec71042617f565c01824f0cf3c", 16730647),  // v1.0 En-Ru
	GAME_ENTRY("dangerouslandsrt2", "dlrt2.exe", "0710e2ec71042617f565c01824f0cf3c", 21636362),  // v1.5 En-Ru
	GAME_ENTRY_EN("dannydreadisoncall", "On Call.exe", "ef480ccb0831e452e55456e0ca24c761", 8787649),
	GAME_ENTRY("dannydreadisoncall", "On Call.exe", "ef480ccb0831e452e55456e0ca24c761", 8791050),  //En-It
	GAME_ENTRY_EN("dantesday", "Dante's Day.exe", "0500aacb6c176d47ac0f8158f055db83", 7504000),
	GAME_ENTRY_EN("darkblack", "DarkBlack.exe", "e80586fdc2db32f65658b235d8cbc159", 8714374),
	GAME_ENTRY_EN("darkdenim", "MAGS_10_22.exe", "d38b8cc442c00a319560244f54e10b85", 12662135),
	GAME_ENTRY_EN("darkforce", "DarkForce.exe", "338fa79960d40689063af31c671b8729", 276608794),
	GAME_ENTRY_EN("darkofnight", "D_o_N.exe", "3f7bb944e107f883d183f047d9d7f698", 10261961),
	GAME_ENTRY_EN("darkroom", "darkroom.exe", "88cf59aad15ca331ab0f854e16c84df3", 1274900),
	GAME_ENTRY_EN("darktimesmerrychristmas", "Dark Times (Merry Christmas).exe", "1b9f13d430bb15bf30d0fd044358db68", 49043355),
	GAME_ENTRY_EN("darum", "afterlife.exe", "0241777c2537fc5d077c05cde10bfa9f", 4082712),
	GAME_ENTRY_EN("darum", "Darum.exe", "0241777c2537fc5d077c05cde10bfa9f", 4082708),
	GAME_ENTRY_LANG("davidcriatividade", "New game.exe", "7ddb9e776648faed5a51170d087074e9", 128362958, Common::PT_BRA),
	GAME_ENTRY_LANG("davidcriatividade", "New game.ags", "7fbc53d2e568e9a04a41ca2aad444f68", 125875646, Common::PT_BRA),
	GAME_ENTRY_EN("davegeneric", "generic.exe", "a524cbb1c51589903c4043b98917f1d9", 2449830),
	GAME_ENTRY_EN("davidletterman", "letterman.exe", "615e806856b7730afadf1fea9a756b70", 17019706),
	GAME_ENTRY_EN("davyjonesspellbook", "dj_sbook.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1095440),
	GAME_ENTRY_EN("dayofthefish", "Rivet.exe", "06a03fe35791b0578068ab1873455463", 2265379),
	GAME_ENTRY("ddddd", "DDDDD.exe", "f3f788c1e3a7b0b40e24e453fa1b9440", 21718220),  // Eng-Deu
	GAME_ENTRY_EN("ddr", "OSD Revolution.ags", "4b5143c2e43527c7228e0596f67b2064", 2796626),
	GAME_ENTRY_EN("ddr", "OSD Revolution.exe", "4b5143c2e43527c7228e0596f67b2064", 2796626),
	GAME_ENTRY_LANG("deaddimension", "Dead Dimension.exe", "a62c0cd6a3d7547e04c856b25bf74f60", 425596857, Common::ES_ESP),
	GAME_ENTRY_EN("deadgods", "DEAD GODS.exe", "82da2565c456dcfb265ded6fe3189c0b", 11562861),
	GAME_ENTRY_EN("deadhand", "Dead Hand.exe", "87ddba03ee2498d0df16f3e6fa1c630a", 31344599),  // v1.0
	GAME_ENTRY_EN("deadhand", "Dead Hand.exe", "87ddba03ee2498d0df16f3e6fa1c630a", 31470562),  // v1.1
	GAME_ENTRY_EN("deadinspace", "DIP.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 19166495),
	GAME_ENTRY_EN("deadlyconsequences", "Deadly Consequences.exe", "173f00e582eebd082d66f1291702b373", 173003853),
	GAME_ENTRY_EN("deadmanpoliticalparty", "DMP~1.P.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 6032550),
	GAME_ENTRY_EN("deadofwinter", "DoW.exe", "615e806856b7730afadf1fea9a756b70", 7055437),
	GAME_ENTRY("deadphones", "Dead Phones.ags", "e3d552d039190ba726e87e33a8e1803b", 23643920),  // En-De
	GAME_ENTRY("deadphones", "Dead Phones.ags", "4e020fddbc4b134a4555ee2fdf99a65d", 23643965),  // En-De v1.1
	GAME_ENTRY("deadphones", "Dead Phones.ags", "83f01cf9c2e4d22c46b67aac9b8d6e35", 23644134),  // En-De v1.3
	GAME_ENTRY("deadphones", "Dead Phones.ags", "acc52371b95f5c314edeba87a0bd541c", 24362372),  // En-De
	GAME_ENTRY("deadphones", "Dead Phones.ags", "0f84a139fa49977177b4f614050d532d", 24362476),  // En-De final
	GAME_ENTRY_EN("deadpixels", "Dead Pixels.exe", "06a03fe35791b0578068ab1873455463", 4018372),
	GAME_ENTRY_EN("deadroom", "Dead room.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 871409),
	GAME_ENTRY_EN("deadsilence", "Game.exe", "5c5d4680def6954c0cd22e82dc07d4d4", 3370359),
	GAME_ENTRY_EN("deadstar", "DeadStar.exe", "0500aacb6c176d47ac0f8158f055db83", 1974362),
	GAME_ENTRY_EN("deadtoilet", "HourDead.exe", "88cf59aad15ca331ab0f854e16c84df3", 1319168),
	GAME_ENTRY_EN("dearrgh", "DeARRGH!-dre & Shaqushia.exe", "7f8068849f77f2d7e7da162be7dbc67d", 3254681),
	GAME_ENTRY_EN("deathandtransfiguration", "death.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 11103314),
	GAME_ENTRY_EN("deathasitis", "death.exe", "a524cbb1c51589903c4043b98917f1d9", 4001615),
	GAME_ENTRY_EN("deathep1", "Death.exe", "4ffc2285a82023294aee3d41181e7177", 120667199),
	GAME_ENTRY_EN("deathofanangel", "Angel.exe", "f120690b506dd63cd7d1112ea6af2f77", 3906130),
	GAME_ENTRY_EN("deathofdavyjonesscenario", "yadodjs.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1229074),
	GAME_ENTRY_EN_PLATFORM("deathonstage", "DeathOnStage.exe", "a524cbb1c51589903c4043b98917f1d9", 2932690, "MAGS"),  //v2
	GAME_ENTRY_EN("deathonstage", "DeathOnStage.exe", "a524cbb1c51589903c4043b98917f1d9", 3131683),  //v5
	GAME_ENTRY_EN("deathsdoor", "Death's Door.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 31447213),
	GAME_ENTRY_EN("deathsquest", "HEHU!.exe", "949f7440e3692b7366c2029979dee9a0", 991809),
	GAME_ENTRY_EN("deathworeendlessfeathersdisk1", "Cyberpunk.exe", "2aaa2609117a34adf666ea546e6dc000", 12620944),
	GAME_ENTRY_EN("deckhex", "DeckHex.ags", "f83dddc650f81536028f6fd9fd81180b", 17562316),  // Linux
	GAME_ENTRY_EN("deckhex", "DeckHex.exe", "dbe9bab672130b9a84925cb6da64a3db", 20670688),  // Windows
	GAME_ENTRY_EN("deephope", "DeepHope.exe", "615e73fc1874e92d60a1996c2330ea36", 5039793),
	GAME_ENTRY_EN("deeplakemall", "mallcop.exe", "c95ed40591a20cd5879d933e629b0ee3", 12976694),  // Windows
	GAME_ENTRY_EN("deeplakemall", "mallcop.ags", "ef42e6de3128eb41dfed09cc63084c83", 10488358),  // Linux
	GAME_ENTRY_EN("deepspacemission", "DSM.exe", "173f00e582eebd082d66f1291702b373", 340115162),
	GAME_ENTRY_EN("dehaunt", "dehaunt.exe", "06a03fe35791b0578068ab1873455463", 3166435),
	GAME_ENTRY_EN("deity", "Deity.exe", "371425d66b0d5dceedf1039fbde2656a", 3799508),
	GAME_ENTRY_EN("delerium", "DLM.exe", "8c515b7c2355248233f7f8f802239cd7", 28448274),
	GAME_ENTRY_EN("dellamorte", "Dellamorte Dellamore  EN.exe", "1f9faa8df8971772624d4607b5737fc0", 49731776),
	GAME_ENTRY_LANG("dellamorte", "Dellamorte Dellamore.exe", "1f9faa8df8971772624d4607b5737fc0", 26128592, Common::FR_FRA),
	GAME_ENTRY_EN("demogame", "Demo Game.exe", "f120690b506dd63cd7d1112ea6af2f77", 3296215),
	GAME_ENTRY_EN("demonday", "Testgame.exe", "0b7529a76f38283d6e850b8d56526fc1", 6749329),
	GAME_ENTRY_EN("demonslayer5", "Dem5.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2346338),
	GAME_ENTRY_EN("demonstohell", "demons.exe", "0564de07d3fd5c16e6947a647061913c", 131418369),
	GAME_ENTRY_EN("depressomatic", "cartoon.exe", "06a03fe35791b0578068ab1873455463", 1988014),
	GAME_ENTRY("derrekquest", "dq1.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 9664668),   // Eng-Rus
	GAME_ENTRY_LANG("derverschwundenehusky", "Husky.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 51932091, Common::DE_DEU),
	GAME_ENTRY_EN("desertminer", "DesertMiner.exe", "615e73fc1874e92d60a1996c2330ea36", 7348008),
	GAME_ENTRY_EN("desmond", "Desmond.exe", "c1feaa2d7845fbe03d9791e4cae121b3", 9021681),  // Windows
	GAME_ENTRY_EN("desmond", "Desmond.ags", "7284c404d82513e2981db5134899f154", 5990621),  // Linux
	GAME_ENTRY_EN("desolate", "Desolate.exe", "5e6155cb09d4d4d59c6998fdafa0e17d", 39349967),  // Windows
	GAME_ENTRY_EN("desolate", "Desolate.ags", "ff0cfe9bf8322574250d1241e94b1bc9", 36237499),  // Linux
	GAME_ENTRY_EN("detectivebhmini", "Detective Boiled-Hard Mini Case.exe", "7971a7c02d414dc8cb33b6ec36080b91", 23715735),
	GAME_ENTRY_EN("detectiveobriced", "OBriced.exe", "3ab9922a2ab7c5875af4c9794b33e012", 92518862),
	GAME_ENTRY_EN("detectiveobriced", "OBriced.exe", "dbe9bab672130b9a84925cb6da64a3db", 379971970),
	GAME_ENTRY_EN("detectiveobriced", "OBriced.ags", "f2a2dded9282b8acab30dcc0a7c4750e", 376863598),
	GAME_ENTRY_EN("detention", "little.exe", "c16204dc8aa338e3199b2c62da0b33f4", 2361991),
	GAME_ENTRY_EN("detention", "little.exe", "c16204dc8aa338e3199b2c62da0b33f4", 2362176),
	GAME_ENTRY_EN("deusexmachina", "Deus.exe", "06a03fe35791b0578068ab1873455463", 1556578),
	GAME_ENTRY_EN("devilgotwoman", "The Devil Got My Woman.exe", "82da2565c456dcfb265ded6fe3189c0b", 12855515),
	GAME_ENTRY_EN("devochkaquest", "DevochkaQuest.exe", "84faea68bf7277610c2229be7b3e74af", 32769660),
	GAME_ENTRY_EN("dexter", "DexterIntro.exe", "2ca80bd50763378b72cd1e1cf25afac3", 4630186),
	GAME_ENTRY_EN("dgsearchbatteries", "dgsfb.exe", "88cf59aad15ca331ab0f854e16c84df3", 1664209),  // English
	GAME_ENTRY_LANG("dgsearchbatteries", "dgf.exe", "88cf59aad15ca331ab0f854e16c84df3", 1619269, Common::FR_FRA),  //French
	GAME_ENTRY_EN("dicklarenzo", "LaRenzo.exe", "0500aacb6c176d47ac0f8158f055db83", 36844867),
	GAME_ENTRY("diemaskennyarlathoteps", "ags_masken.exe", "74dc062c5f68f3d70da911f2db5737b6", 129219473),
	GAME_ENTRY_EN("digilawyer", "DigiLawyer.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1686006),
	GAME_ENTRY_EN("dimetrodon", "Day of the Dimetrodon.exe", "9632da1faae3ef66d7ea9dd60cc19440", 28727620),
	GAME_ENTRY_EN("dimetrodon", "Day of the Dimetrodon.ags", "758c54f5431641ba68ecab7ae54b0480", 25685296),
	GAME_ENTRY_EN("dinnerforpigeons", "DinnerForPigeons.exe", "0de24019f4bf96e6a7add17110514609", 27207204),
	GAME_ENTRY_EN("dirandiouskroken", "Dirandious Kroken.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 12227072),  // Windows
	GAME_ENTRY_EN("dirandiouskroken", "Dirandious Kroken.ags", "53f94c587c9728e342a0f23b1f025907", 9190892),  // Linux
	GAME_ENTRY_EN("disappearancetime", "007.exe", "35452a12003d51118f7c2ccc45ff52ca", 2838918),  // v1
	GAME_ENTRY_EN("disappearancetime", "007.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2879333),  // v2
	GAME_ENTRY_EN_PLATFORM("disappearancetime", "007Plus.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3847344, "2009 Release"),  // v3
	GAME_ENTRY_EN("discoflow", "xn--[  ].exe-uyg4fzbk", "465f972675db2da6040518221af5b0ba", 8346277),
	GAME_ENTRY_EN("disgust", "Disgust.exe", "495d45fb8adfd49690ae3b97921feec6", 8780836),  // Windows
	GAME_ENTRY_EN("disgust", "Disgust.ags", "05346dafd4ff4893660447cf75b6f542", 6300180),  // Linux
	GAME_ENTRY_EN("dislocation", "dislocation.exe", "aecd482222ff54206e43a029b5f0b170", 168869859),
	GAME_ENTRY_EN_PLATFORM("dislocation", "dislocation.exe", "b5c41e08919834b549ca350a28f18f87", 108929984, "OROW"),
	GAME_ENTRY_EN_PLATFORM("dislocation", "dislocation.ags", "235b356963f58570445e10d1306a690c", 106668464, "OROW"),
	GAME_ENTRY_EN("disquiet", "Disquiet.exe", "f90f5f612ed5879addfdd8634d093333", 9624101),
	GAME_ENTRY_EN("distancenoobject", "Distance no object.exe", "5cca0f8af961d2bc1c6e475437d3b23b", 98011325),  // Windows
	GAME_ENTRY_EN("distancenoobject", "Distance no object.ags", "60b1978b591c4ffcb63e81e69d425343", 94977705),  // Linux
	GAME_ENTRY_EN("djdeceased", "RON.exe", "0710e2ec71042617f565c01824f0cf3c", 1175250),
	GAME_ENTRY_EN("dlistdiva", "D-List Diva.exe", "aed48ef1dd9615ab3df8504b277e8d45", 8330523),
	GAME_ENTRY_EN("dlistdiva", "D-List Diva.exe", "aed48ef1dd9615ab3df8504b277e8d45", 8338409),
	GAME_ENTRY_EN("dobutsu", "Dobutsu no Costco.exe", "b781cc4ab0fbe647615fafa4830ec308", 29725433),
	GAME_ENTRY_EN("doctormaze", "Maze.exe", "825ccacaabe2a3b0cd48d6f8deb42d72", 43934617),
	GAME_ENTRY_EN("doctormuttonchop", "muttonchop.exe", "57e261dd3bb45761af4a002775e45710", 5781714),
	GAME_ENTRY_EN("doctormuttonchop", "muttonchop.exe", "57e261dd3bb45761af4a002775e45710", 5847364),
	GAME_ENTRY_EN("doctorzoo", "drzoo.exe", "06a03fe35791b0578068ab1873455463", 2592578),
	GAME_ENTRY_EN_PLATFORM("dogescape", "dog.exe", "0710e2ec71042617f565c01824f0cf3c", 1508802, "Unfinished"),
	GAME_ENTRY_EN_PLATFORM("dogescape", "mags_aug.exe", "0710e2ec71042617f565c01824f0cf3c", 1508802, "Unfinished"),
	GAME_ENTRY_EN("dolldream", "a doll in a dream.ags", "902f40c021fa97eccea8c08dea3c88b1", 149508062),
	GAME_ENTRY_EN("dollshouse", "DOLLS house.exe", "0241777c2537fc5d077c05cde10bfa9f", 37582183),
	GAME_ENTRY_EN("dollshouseinsanity", "dollshouse insanity.exe", "0241777c2537fc5d077c05cde10bfa9f", 52632190),
	GAME_ENTRY_EN("domesticgoddess", "Little Mrs. Domestic Goddess.exe", "345685aaec400c69a1e6d5e18a63850c", 43153023),
	GAME_ENTRY_LANG("dommep1", "DOMM.exe", "06a03fe35791b0578068ab1873455463", 6026756, Common::FR_FRA),
	GAME_ENTRY_EN("donalddowell", "Donald Dowell.exe", "74dc062c5f68f3d70da911f2db5737b6", 647518160),  // English
	GAME_ENTRY_LANG("donalddowell", "Donald Dowell.exe", "74dc062c5f68f3d70da911f2db5737b6", 596996103, Common::IT_ITA),  //Italian
	GAME_ENTRY_EN("donkeybas", "DonkeyBas.exe", "06a03fe35791b0578068ab1873455463", 1944795),
	GAME_ENTRY_EN("donnaavengerofblood", "DONNA.exe", "74aad8dfd71ed2ae9574a60be7610c43", 275096403),
	GAME_ENTRY_EN("donniedarko", "Darko.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1820048),
	GAME_ENTRY_EN("donspillacyconspiracyquest", "DSAQ.exe", "f120690b506dd63cd7d1112ea6af2f77", 2066918),
	GAME_ENTRY_EN("dontdrinkthepink", "pink.exe", "82da2565c456dcfb265ded6fe3189c0b", 5447689),
	GAME_ENTRY_EN("dontgiveupcat", "dont-give-up-the-cat.exe", "194307a8f17bceb166bfdeb7cc1aa2ed", 17691275),  // Windows
	GAME_ENTRY_EN("dontgiveupcat", "dont-give-up-the-cat.ags", "13c0cfd2fb3bb868d0db49e56b45daa5", 14517367),  // Linux
	GAME_ENTRY_EN("donthedweebdancedilemma", "Don the Dweeb.exe", "f120690b506dd63cd7d1112ea6af2f77", 2278453),
	GAME_ENTRY_EN("donticeyourcool", "Don't Ice Your Cool.exe", "2321081ba51d1869ebe11b9ba126911b", 5979469),
	GAME_ENTRY_EN("dontlook", "Don't Look.exe", "25976a689b0f4d73eac69b1728377ecb", 4623669),
	GAME_ENTRY_EN("dontpush", "DontPush.exe", "897e7a83055ef58603d39b1412d40f3b", 3119863),
	GAME_ENTRY_EN("dontworrybaby", "baby.exe", "a524cbb1c51589903c4043b98917f1d9", 12498411),
	GAME_ENTRY_EN("dontworryillbringthebeer", "Don't Worry, I'll Bring The Beer.exe", "495d45fb8adfd49690ae3b97921feec6", 4446095),
	GAME_ENTRY_EN("doomgraphicadventure", "DoomGraphic.exe", "0500aacb6c176d47ac0f8158f055db83", 2236047),
	GAME_ENTRY_EN("doors", "Doors.exe", "3277e3b4ec9c224fb338460be298b77c", 95135227),
	GAME_ENTRY_EN("doors", "Doors.exe", "3277e3b4ec9c224fb338460be298b77c", 95135228),  // itch.io
	GAME_ENTRY_EN("doth", "Vault.ags", "e27f5627129fa7d97f54d4be7f70cd78", 24904251),  // v1.0
	GAME_ENTRY_EN("doth", "Vault.ags", "c53486a3fc11e9fdf82d52e7bb4c7b8e", 24908946),  // v1.2
	GAME_ENTRY_EN("dots", "Sammich.ags", "dd4c8776a0b5717b1310280cba8d38be", 8063745),  // v1.2
	GAME_ENTRY_EN("dots", "Sammich.ags", "b058fe0e139cc627ebc25c35077451e0", 8595783),  // v1.3
	GAME_ENTRY("dovadulasburn", "FILES.exe", "9cb3c8dc7a8ab9c44815955696be2677", 26489645),  // En-It
	GAME_ENTRY("dovadulasburn", "DoVaDuLa'S BuRn.exe", "06a03fe35791b0578068ab1873455463", 27108565),  // En-It
	GAME_ENTRY_EN_PLATFORM("draconis", "Draconis.exe", "7a026b53d6c066a785670b958827d310", 2861993, "OROW"),
	GAME_ENTRY_EN_PLATFORM("draconis", "Draconis.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 2921005, "MAGS"),
	GAME_ENTRY_EN("draculahousemystery", "Mystery House.ags", "b63befdafb40905a61645365d6047ca7", 23182439),  // itch.io client
	GAME_ENTRY("draculator2", "Draculator II.exe", "519c0d37ab893d95f5add495355e460c", 17244503),  // Eng-Esp
	GAME_ENTRY_EN("dragonorb", "DragonOrb.exe", "2bc8f994a7d1e05ed45f35abf2128231", 269836732),
	GAME_ENTRY_EN("dragonscale", "DS.exe", "64fcaf7da0b257ea831f89c54be0ad72", 1366698),
	GAME_ENTRY_EN("dragonslayers", "Slayers.exe", "3128b9f90e2f954ba704414ae854d10b", 6914586),
	GAME_ENTRY_EN("dragontales", "dragont.exe", "465f972675db2da6040518221af5b0ba", 9240132),
	GAME_ENTRY_EN("dragoonjourney", "DragoonsJourney.exe", "ba989a02e5392a9a7550fee72629c02f", 120122456),
	GAME_ENTRY_EN("drawnflame", "Drawn to the Flame.exe", "7971a7c02d414dc8cb33b6ec36080b91", 51616597),  // Windows
	GAME_ENTRY_EN("drawnflame", "Drawn to the Flame.ags", "11ef599203105c5094f587c805871fb4", 49119557),  // Linux
	GAME_ENTRY_EN("drchuckles", "Dr Chuckles.exe", "07b30f9606f5dfde84cb66e926f7cc74", 615657195),
	GAME_ENTRY("dreadmacfarlane", "Dread.exe", "615e806856b7730afadf1fea9a756b70", 11157310),  // Eng-Fra
	GAME_ENTRY_LANG("dreadmacfarlane", "Dread.exe", "615e806856b7730afadf1fea9a756b70", 11158073, Common::FR_FRA),
	GAME_ENTRY_PLATFORM("dreadmacfarlane2", "Dread2.exe", "fb787304e66798ba9d0172665a34f4cf", 5332574, "Unfinished"),
	GAME_ENTRY_LANG("dreadmacfarlaneapprentie", "Dread Mac Farlane, apprentie pirate.exe", "01d0e6bd812abaa307bcb10fc2193416", 41950548, Common::FR_FRA),
	GAME_ENTRY_LANG("dreadmacfarlanefils", "Le fils du Pirate.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 1864534844, Common::FR_FRA),
	GAME_ENTRY("dreadmacfarlanev2ep1", "Dread Mac Farlane - episode 1.ags", "a088db68f7aadfa02149a91680dee1c5", 123567066),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep2", "Dread Mac Farlane - episode 2.ags", "e992d5daaf77f9a8a234a9ba44a9a163", 131631424),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep3", "Dread Mac Farlane - episode 3.ags", "16565ca9277d9e8eafc3eef11d1ffb74", 181377632),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep4", "Dread Mac Farlane - episode 4.ags", "47ef94e21a74a9e684f0e2b3026f2596", 253532506),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep5", "Dread Mac Farlane - episode 5.ags", "889ebb0567c8ba83d0154f2ee77d9f7a", 282494533),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep6", "Dread Mac Farlane - episode 6.ags", "7cf8279d6ed3817e5619eba694603fd2", 322722253),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep7", "Dread Mac Farlane - episode 7.ags", "caa8c564a101c2c3c0df78ff74de5eec", 156247272),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep8", "Dread Mac Farlane - episode 8.ags", "953493139f332e2be0a336246eb98167", 129754180),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep9", "Dread Mac Farlane - episode 9.ags", "99263303dbb8f86440814eeba82506b8", 188806094),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep10", "Dread Mac Farlane - episode 10.ags", "de0b2e49d690c18e30602c2619c7e104", 223828573),  // Eng-Fra
	GAME_ENTRY("dreadmacfarlanev2ep11", "Dread Mac Farlane - episode 11.ags", "045342ca09babf5dd12c24e8cb5821f5", 359125814),  // Eng-Fra
	GAME_ENTRY_EN("dreamychristmas", "Your dreamy Christmas.exe", "a4e6ec808b347f4456eae7c808e90727", 84727913),  // Windows
	GAME_ENTRY_EN("dreamychristmas", "Your dreamy Christmas.ags", "f61d34a8a5c9501962c7161fe127aba2", 81696341),  // Linux
	GAME_ENTRY_EN("dreamagine", "Game.exe", "256752c9a97b4780fc5e6f3239c8cdf1", 11122818),
	GAME_ENTRY_LANG("dreamcatadv", "dreamcatadventure.exe", "2ce63b8fabba844aaf06426814e3f40f", 10233600, Common::DE_DEU),  // Only GUI is multilang
	GAME_ENTRY("dreamcatadv", "dreamcatadventure.exe", "f992e331ad8d8af7551797947bdcad1f", 10278137),  // En-De
	GAME_ENTRY_EN_PLATFORM("dreamdiary", "Dream Diary Quest.exe", "1608e6ba3e7965580fe36e547635c0ac", 5824653, "Prototype 0.001"),
	GAME_ENTRY_EN_PLATFORM("dreamdiary", "Dream Diary Quest.exe", "1608e6ba3e7965580fe36e547635c0ac", 7320344, "Prototype 0.002"),
	GAME_ENTRY_EN_PLATFORM("dreamdiary", "Dream Diary Quest.exe", "1608e6ba3e7965580fe36e547635c0ac", 6924791, "Prototype 0.003"),
	GAME_ENTRY_EN("dreamer", "LittleDreamer.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 62151801),
	GAME_ENTRY_EN("dreamscapess", "mags112023.exe", "328a715c0a826cad7d6824c6e76b52d5", 16052012),  // Win
	GAME_ENTRY_EN("dreamscapess", "mags112023.ags", "b1f7e54ebdc65fb602f55b9a735a935c", 12897048),  // Linux
	GAME_ENTRY_EN("dreamscapess", "mags112023.exe", "328a715c0a826cad7d6824c6e76b52d5", 16052261),  // Win 1.1
	GAME_ENTRY_EN("dreamscapess", "mags112023.ags", "cbb459ef0133d5dd4e0d455104788956", 12897297),  // Linux 1.1
	GAME_ENTRY_EN("dreamsofwintermass", "Dreams of Winter Mass.exe", "1608e6ba3e7965580fe36e547635c0ac", 3305143),
	GAME_ENTRY_EN("dreamwithindream", "dream.exe", "0500aacb6c176d47ac0f8158f055db83", 4635389),
	GAME_ENTRY_EN("dressedforafight", "ladyfighterv2.exe", "7cebde856d22620450989e3b1d7896b1", 580950783),
	GAME_ENTRY_EN("driftwalker", "Driftwalker.exe", "7ddb9e776648faed5a51170d087074e9", 9437090),
	GAME_ENTRY_EN("drillkiller", "LD22_Entry.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 4121415),
	GAME_ENTRY("drlutztimetravelmachine", "Dr.Lutz Machine.exe", "2bc8f994a7d1e05ed45f35abf2128231", 12602529),  // En-It
	GAME_ENTRY_EN("drmoby", "Moby.exe", "3128b9f90e2f954ba704414ae854d10b", 1600777),
	GAME_ENTRY_EN("droi0", "DROI0.exe", "0500aacb6c176d47ac0f8158f055db83", 13815202),
	GAME_ENTRY_EN("drugsandalians", "Drugs and alians.exe", "82da2565c456dcfb265ded6fe3189c0b", 158823644),
	GAME_ENTRY_EN("drunkfredcell", "Cell.exe", "0b7529a76f38283d6e850b8d56526fc1", 811527),
	GAME_ENTRY_EN("dungeonhands", "DungeonHands.exe", "0a6704159f6f716ac80da91c430430ce", 16276450),
	GAME_ENTRY_EN("dungeonhands", "DungeonHands.exe", "f80ff6c2348f2bb90d3813719f54c870", 13751518),
	GAME_ENTRY_EN("dungeonhands", "dungeon_hands.exe", "0fe1d93da69963d5eef55ce2619c94be", 9884231),  // v7
	GAME_ENTRY_EN("duskhunters", "DuskHunters.exe", "3128b9f90e2f954ba704414ae854d10b", 3029482),
	GAME_ENTRY_EN("dutyandbeyond", "Deliver.exe", "0710e2ec71042617f565c01824f0cf3c", 28873258),
	GAME_ENTRY_EN("dutyfirst", "duty_first.exe", "4e0d0aca04563b21a47e8a0cc56605fc", 10333200),
	GAME_ENTRY_EN("duzzquest", "DuzzQuest.exe", "3128b9f90e2f954ba704414ae854d10b", 13125200),
	GAME_ENTRY_EN("duzzquest2", "DuzzQuest2.exe", "8911d942c1a71458370d37ca3e5bfdda", 59088366),
	GAME_ENTRY("earlbobby1", "Bobby.exe", "3128b9f90e2f954ba704414ae854d10b", 11556701),  // v1.0 En-De
	GAME_ENTRY("earlbobby1", "Bobby's Shoes.exe", "bb59de174d70797d774dec76a171352d", 12735626),  // v2.0 En-De
	GAME_ENTRY("earlbobby2", "Bobby's Balls.exe", "bb59de174d70797d774dec76a171352d", 4006605),  // En-De
	GAME_ENTRY("earlbobby3", "Earl Bobby 3.exe", "804add6fbbb791b7be195dc0097434a1", 4379857),  // En-De-It
	GAME_ENTRY("earlmansinthebreakout", "Earl Mansin.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 108987401),  // En-De
	GAME_ENTRY_EN("earthad", "EarthADLowEstate.ags", "201270d3ff9db041badc9c4a90a46f1f", 95124070),  // itch.io client
	GAME_ENTRY_EN("earthlingpriorities", "Earthling Priorities.exe", "9b54ea3557373188d8388ec4d1ae5eed", 21490281),
	GAME_ENTRY_EN("earthstory", "EarthStory.exe", "82da2565c456dcfb265ded6fe3189c0b", 23042784),
	GAME_ENTRY_EN("easterencounter", "Easter Encounter.exe", "779421cd8ad5268e2efdb26e04608db7", 30257364),
	GAME_ENTRY_EN("easterinron", "puzzles.exe", "02094e217c23e1d40a84891ee95010cb", 3239060),
	GAME_ENTRY_EN("easterislanddefender", "Moia 'n Seek.exe", "804add6fbbb791b7be195dc0097434a1", 6359079),
	GAME_ENTRY_EN("echidna", "Echidna Chwest.exe", "dbe281d93c914899886e77c09c3c65ec", 8769591),
	GAME_ENTRY_EN("echoesinstatic", "Echoes.ags", "65a86d4f5cc76b81c30f006bc30f2fc5", 166223457),
	GAME_ENTRY_EN("echoesofterra", "echoes.exe", "a524cbb1c51589903c4043b98917f1d9", 42866707),
	GAME_ENTRY_EN("echoesofthepast", "Echoes of the Past.exe", "111542004299dae82a02e750fdb97e59", 612857584),
	GAME_ENTRY_EN("edmund", "Edmundand the potato.exe", "615e73fc1874e92d60a1996c2330ea36", 4533124),
	GAME_ENTRY_EN("egoplanetapes", "EgoInPlanetOfTheApes.exe", "7ddb9e776648faed5a51170d087074e9", 38605725),
	GAME_ENTRY_EN("egoplanetapes", "EgoInPlanetOfTheApes.ags", "cfc308c9dcbd329a0c5bf1283e731c92", 19452067),
	GAME_ENTRY_EN("egoplanetapes", "Crossover.ags", "86a0b6f733707544035d7e823584ed4d", 16666330),
	GAME_ENTRY_EN("egress", "Egress.exe", "615e73fc1874e92d60a1996c2330ea36", 439827181),
	GAME_ENTRY_EN("egress", "Egress.exe", "981e60a9be515bb56c634856462abbc7", 441036301),  // v1.1
	GAME_ENTRY_EN("eight", "8.exe", "c7835774540a2e7d9d89a2df66fbcec4", 2412166),
	GAME_ENTRY_LANG("eikos1", "La Legende d'Eikos.exe", "0564de07d3fd5c16e6947a647061913c", 602426109, Common::FR_FRA),
	GAME_ENTRY_EN("eikos1", "The Legend of Eikos.ags", "a11aba6b5ae2c21d688da3ca1ee28e61", 599803643),
	GAME_ENTRY_LANG("eikos2", "eikos2.exe", "0564de07d3fd5c16e6947a647061913c", 1038284323, Common::FR_FRA),
	GAME_ENTRY_EN("eikos2", "eikos2.exe", "2e8d5f27d491676f4825881974e4327c", 1037984505),
	GAME_ENTRY("electrokit", "electrctGUI.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 8941606),  //Eng-Dutch
	GAME_ENTRY_EN("elegantmurdermystery", "DH.exe", "0b7529a76f38283d6e850b8d56526fc1", 2098899),
	GAME_ENTRY_EN_PLATFORM("elevation", "Elevation.ags", "bf06b9759714464c095e1c1e597f1e86", 4362276, "Beta"),  // Linux
	GAME_ENTRY_EN_PLATFORM("elevation", "Elevation.ags", "3063ec0d8db8305d1dc61d7862650f11", 4363999, "Beta"),
	GAME_ENTRY_EN("elevation", "Elevation.ags", "46d792c62262d9f83e476f96a4bfb74b", 4490040),
	GAME_ENTRY_EN_PLATFORM("elevation", "Elevation.exe", "2f2bc0b9c539d20529c0e343315d5f65", 7471160, "Beta"),  // Windows
	GAME_ENTRY_EN_PLATFORM("elevation", "Elevation.exe", "2f2bc0b9c539d20529c0e343315d5f65", 7472883, "Beta"),
	GAME_ENTRY_EN("elevation", "Elevation.exe", "82e7377dcb9236fc9b622e12baa64a49", 7605580),
	GAME_ENTRY_EN_PLATFORM("elevatorriseabyss", "Elevator.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6563864, "OROW"),
	GAME_ENTRY_EN("elevatorriseabyss", "T_E_R.exe", "f120690b506dd63cd7d1112ea6af2f77", 13366126),
	GAME_ENTRY_LANG("elfer", "socer.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1911767, Common::DE_DEU),
	GAME_ENTRY_EN("elfmotorsinc", "ElfMtrs.exe", "3421b46ff95c5885603086c39a038a20", 8036769),
	GAME_ENTRY_EN("elforescuecraby", "elfo.exe", "31457af75a89b6141c31e8ed33a57e25", 10641021),
	GAME_ENTRY_EN("elfthe4elements", "ELF2.exe", "247bd23f71d7c785a7c9aa8b06077ec0", 207254832),
	GAME_ENTRY_EN("elfthedarkness", "ELF.exe", "247bd23f71d7c785a7c9aa8b06077ec0", 128596356),
	GAME_ENTRY_EN("eliminationbyimprovisation", "Stu2.exe", "9cf51833e787cc919837d9a8bd8fc14c", 971446),
	GAME_ENTRY_EN("elmowagon", "Elmo's Wagon Conflict.exe", "615e73fc1874e92d60a1996c2330ea36", 3051412),
	GAME_ENTRY_EN("elvves", "Elves.exe", "1608e6ba3e7965580fe36e547635c0ac", 4306956),
	GAME_ENTRY_EN("emeraldeyes", "Emerald.exe", "f120690b506dd63cd7d1112ea6af2f77", 2849945),
	GAME_ENTRY_EN("emilyenough", "EE.exe", "97020973a2a635fe28eb0ba4bdfaa70b", 3141963),
	GAME_ENTRY_EN("empty", "empty.exe", "7ddb9e776648faed5a51170d087074e9", 69231380),  // v1.3.0
	GAME_ENTRY("emptymindblankfate", "EMBF.exe", "18b284c22010850f79bc5c20054a70c4", 75732051),  // Eng-Esp
	GAME_ENTRY_EN("encounters", "Encounters of the Closest Kind.exe", "06a03fe35791b0578068ab1873455463", 4393673),
	GAME_ENTRY_EN("endlessloop", "Endless Loop.exe", "615e73fc1874e92d60a1996c2330ea36", 1920165),
	GAME_ENTRY_EN("enoworld", "Enoworld.exe", "465f972675db2da6040518221af5b0ba", 16151170),
	GAME_ENTRY_LANG("enqueteolonnes", "Enquete au pays des olonnes.exe", "eee23783ae8f3d6c6d3a488c7b37c30d", 623504563, Common::FR_FRA),
	GAME_ENTRY_EN("entrapment", "Entrapment.exe", "fa876182475fd84bc104e3458b7ba362", 24899784),
	GAME_ENTRY_EN("entrapped", "Cellar.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 4644055),
	GAME_ENTRY_EN("epsteinislandp1", "The Mystery of Epstein Island Day.exe", "7971a7c02d414dc8cb33b6ec36080b91", 121973362),
	GAME_ENTRY_EN("epsteinislandp2", "The Mystery of Epstein Island Sunset.exe", "7633f0919378749cebf6dacba61ca0e1", 368117943),
	GAME_ENTRY_EN("equilibrium", "EQ.exe", "0500aacb6c176d47ac0f8158f055db83", 8398394),
	GAME_ENTRY_EN("erictheanteater", "eric.exe", "e750ecac380dca709028fc0744b9f7f6", 3865602),
	GAME_ENTRY_EN("erkrealestate", "AISRE.exe", "41e8fa920589300ed8b4aec09a658c90", 5958575),
	GAME_ENTRY_PLATFORM("errand", "errand.exe", "8212e96af620f6e02a37fdf583f79751", 4404617, "MAGS"), // Multilang
	GAME_ENTRY("errand", "errand.exe", "2e6b104176f310e7fb507183c443edfe", 4404813),  // v1.2  Multilang
	GAME_ENTRY("errand", "Errand.exe", "8212e96af620f6e02a37fdf583f79751", 4425035),  // v1.3  Windows Multilang
	GAME_ENTRY("errand", "Errand.exe", "8212e96af620f6e02a37fdf583f79751", 4421187),  // v1.3  Alt/Linux Multiang
	GAME_ENTRY("errand", "errand.exe", "8212e96af620f6e02a37fdf583f79751", 4425232),  // v1.5  Win/Linux Multilang
	GAME_ENTRY("errand", "errand.exe", "d25c3cb8a42c5565634dfef76b3cf83e", 4048015),  // Win Multilang itch.io
	GAME_ENTRY_EN("escape", "Escape.exe", "9cf51833e787cc919837d9a8bd8fc14c", 2182153),
	GAME_ENTRY_EN("escaped", "Escaped.exe", "7825fa980fcd058caadaf6d3377b1f78", 7048242),
	GAME_ENTRY_EN("escaped", "Escaped.ags", "b9162292720e02a07d11226a30c4f0b2", 4012574),
	GAME_ENTRY_EN("escaped2", "Escaped 2.exe", "f076c3f92320568193a7477c79de0732", 5028841),
	GAME_ENTRY_EN("escaped2", "Escaped 2.ags", "7fdd039f2415934d5df65c608b410a87", 1915349),
	GAME_ENTRY_EN("escapefromasmallroom", "game1.exe", "0b7529a76f38283d6e850b8d56526fc1", 803029),
	GAME_ENTRY_EN("escapefromevergreenforest", "EFEF.exe", "2e58934bbe98335f33c20a23a6c6135a", 23855385),  // v1.0
	GAME_ENTRY_EN("escapefromevergreenforest", "EFEF.exe", "2e58934bbe98335f33c20a23a6c6135a", 23855632),  // v1.2
	GAME_ENTRY_EN("escapefromlurrilous", "EFL.exe", "4bcbc24015114752b3c7971128704689", 2756979),
	GAME_ENTRY_EN("escapefromterrorbay", "eftb.exe", "665fe5817e020e6a81849429e93662e4", 4917879),
	GAME_ENTRY_EN("escapefromthechaoticcity", "ChaoticCity.exe", "ff3358d8f2726d544aadfde4f1ec8407", 93118664),
	GAME_ENTRY_EN("escapefromthegarage", "Escape.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 962798),
	GAME_ENTRY_EN("escapefromthesalemmoons", "Salem.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1394026),
	GAME_ENTRY_EN("escapefromthezombiecity", "zombie.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3216299),
	GAME_ENTRY_EN("escapeledgeoctagon", "The Ledge.exe", "37484665f9e2d00514a4b243738d4cb7", 130978869),  // Windows
	GAME_ENTRY_EN("escapeledgeoctagon", "The Ledge.ags", "9e8224227581d6cf378d32b881bc5000", 128480805),  // Linux
	GAME_ENTRY("escapethebarn", "EscapeTheBarn.exe", "82da2565c456dcfb265ded6fe3189c0b", 11451800),  // Eng-Deu
	GAME_ENTRY_EN("escapethebrothel", "Ludumdare.exe", "615e73fc1874e92d60a1996c2330ea36", 3323945),
	GAME_ENTRY_EN_PLATFORM("escapethebrothel", "EscapeDBrothel.exe", "63f8a60cc094996bd070e68cb3c4722c", 4018443, "Remake"),
	GAME_ENTRY_EN_PLATFORM("escapethebrothel", "EscapeDBrothel.ags", "fd3386f7f7e1e0b126b0e996f7946ba6", 911607, "Remake"),
	GAME_ENTRY_EN("escapetheship", "Escape.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1063503),
	GAME_ENTRY_EN("escapetocivilization", "Saanb.exe", "0b7529a76f38283d6e850b8d56526fc1", 1209470),
	GAME_ENTRY_EN("esper", "esper.exe", "0710e2ec71042617f565c01824f0cf3c", 17409715),
	GAME_ENTRY_EN("essence", "Essence.exe", "e80586fdc2db32f65658b235d8cbc159", 15416355),
	GAME_ENTRY_EN("essenceofimagination", "Essence of Imagination.exe", "0fe1d93da69963d5eef55ce2619c94be", 32847280),
	GAME_ENTRY_EN("essia", "Mnemo.exe", "615e73fc1874e92d60a1996c2330ea36", 2691247),
	GAME_ENTRY_EN("etaac", "TaxOROW.exe", "a524cbb1c51589903c4043b98917f1d9", 4474174),
	GAME_ENTRY_LANG("etaitungeek", "il etait un geek.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 2659191, Common::FR_FRA),
	GAME_ENTRY_EN("eternalchrysalis", "Eternal_Chrysalis.exe", "2b4b47cd54497526c7606878a4624f58", 39916658),
	GAME_ENTRY_EN("eternallyus", "Eternally Us.exe", "3faa59edd92158ff2cbd4b9db54acf61", 43467084),
	GAME_ENTRY_EN("eternallyus", "Eternally Us.exe", "3faa59edd92158ff2cbd4b9db54acf61", 43468014),
	GAME_ENTRY_EN("eternallyus", "Eternally Us.exe", "3faa59edd92158ff2cbd4b9db54acf61", 43468112),
	GAME_ENTRY("eternaltorpor", "Sopor Eterno.exe", "ac29dac7de6d0f6c3c37152880c66061", 60632661),  // Win Eng-Esp
	GAME_ENTRY("eternaltorpor", "Sopor Eterno.ags", "d6815c918c80b88746401dd3b1053f29", 57518657),  // Linux Eng-Esp
	GAME_ENTRY_EN("eventtimer", "Timer.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 3000682),
	GAME_ENTRY_EN("evil", "Evil.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 2459231),
	GAME_ENTRY_EN("evilenterprises", "EvilE.exe", "3128b9f90e2f954ba704414ae854d10b", 6502022),
	GAME_ENTRY_EN("evillodge", "Evillodge.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 982457),
	GAME_ENTRY_EN("exclamation", "!.exe", "0514661a4ba6772cf0db0cf839fc7003", 2785515),
	GAME_ENTRY_EN("exit", "EXIT.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 54723513),
	GAME_ENTRY_EN("exmachina", "machina.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 5545266),  //v1.2
	GAME_ENTRY_EN("explorationa", "ExplorationA.exe", "615e73fc1874e92d60a1996c2330ea36", 8536746),
	GAME_ENTRY_EN("exposedreality", "HORON.exe", "bb59de174d70797d774dec76a171352d", 4847902),
	GAME_ENTRY_EN("fadingshades", "FadingShades.exe", "f120690b506dd63cd7d1112ea6af2f77", 20166198),
	GAME_ENTRY_EN("fakethemoonlanding", "OperationStarman.exe", "495d45fb8adfd49690ae3b97921feec6", 56805472),
	GAME_ENTRY_EN("fall", "FALL.exe", "387ff720e746ae46e93f463fd58d77a4", 34152794),
	GAME_ENTRY_EN("fallenangel", "Fallen Angel.exe", "f120690b506dd63cd7d1112ea6af2f77", 3988730),
	GAME_ENTRY_EN("fallenangel", "Fallen Angel.exe", "f120690b506dd63cd7d1112ea6af2f77", 3993369),
	GAME_ENTRY_EN("fallenhero", "Fallen Hero.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 99865610),
	GAME_ENTRY("fallensoldier", "fallen soldier.exe", "7a3096ac0237cb6aa8e1718e28caf039", 81522018),  // Eng-Swe
	GAME_ENTRY_EN("fallingdark", "Falling Dark.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 451738395),  //v1.4
	GAME_ENTRY_EN("fallingdark2", "Falling Dark 2.exe", "0564de07d3fd5c16e6947a647061913c", 205636847),  //v1.06
	GAME_ENTRY_EN("fallingskywards", "Falling Skywards.exe", "b36bda97ef129952eb2d18bf5087530c", 57272115),
	GAME_ENTRY_LANG("familieherfurth", "Familie Herfurth.exe", "cefb80c104ab2a8bd1713d13d7c74d07", 5457327, Common::DE_DEU),  //v2.0
	GAME_ENTRY_EN("fanbots", "Game.exe", "aecd482222ff54206e43a029b5f0b170", 8665702),  //MAGS
	GAME_ENTRY_EN("fanbots", "Game.exe", "c58d7c2d20e502e72a504f3f23a1d6da", 12986310),  // 2.0.1
	GAME_ENTRY_EN("fanbots", "Game.exe", "4b18403fd1d150321f50cff9232d7b28", 13725413),  // 2.0.2
	GAME_ENTRY_LANG("fantasymotus", "Fantasy Motus.exe", "139775dece131b30a6d850c365d199ae", 69134041, Common::FR_FRA),
	GAME_ENTRY_EN("farcorners1", "FCotW1.exe", "98df40b0885b4e5f90f1093987f1f56c", 27336337),
	GAME_ENTRY("farnowhere", "FAR NOWHERE.exe", "06a03fe35791b0578068ab1873455463", 77186777),  // En-Fr ?
	GAME_ENTRY_EN("fashiongirl", "Fashion Girl.exe", "a4e6ec808b347f4456eae7c808e90727", 109319608),  // Windows
	GAME_ENTRY_EN("fashiongirl", "Fashion Girl.ags", "08bb57ffbec899a4bd01a65253d67a6b", 106288036),  // Linux
	GAME_ENTRY_EN("fasmo", "fasmo.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16778527),
	GAME_ENTRY_EN("fasmo", "fasmo.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16778433),
	GAME_ENTRY_EN("fasmogoeswest", "FasmoGoesWest.exe", "f120690b506dd63cd7d1112ea6af2f77", 15348030),
	GAME_ENTRY_EN("fayeking", "FKJJ.exe", "7ce746c19cf3080a9fac568979b2bf9b", 10664729),
	GAME_ENTRY("fbiquest", "FQ.exe", "465f972675db2da6040518221af5b0ba", 3838610),  // Eng-Rus
	GAME_ENTRY_EN("fearaphobia", "Fearaphobia.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 9646228),
	GAME_ENTRY_EN("fearaphobia", "Fearaphobia.ags", "2fa8f351579c6bce4bc4a69a7a6abf5f", 7166084),
	GAME_ENTRY_EN("featherweight", "Featherweight.exe", "e8b2a430042709f07183c2c249e0e0d9", 6399452),
	GAME_ENTRY_EN("femspray", "Fem.exe", "01d0e6bd812abaa307bcb10fc2193416", 49176669),
	GAME_ENTRY_EN("fengshuitv", "Feng Shui And The Art Of TV Reception.exe", "c1b88c284a3d821f93d78a2e88487932", 22687931),
	GAME_ENTRY_EN("ferragosto", "Ferragosto.exe", "f604f7f3a12da5d3bcf7a7814a14e43b", 61875571),
	GAME_ENTRY_LANG("feuersturm1", "Feuersturm.exe", "06a03fe35791b0578068ab1873455463", 5682579, Common::DE_DEU),
	GAME_ENTRY_LANG("feuersturm2", "fs2.exe", "06a03fe35791b0578068ab1873455463", 7043558, Common::DE_DEU),
	GAME_ENTRY_LANG("feuersturm3", "feuersturm3.exe", "206478d4d39e16571682b2cddf01a78f", 7826524, Common::DE_DEU),
	GAME_ENTRY_EN("fhaloness", "Fhaloness.exe", "b908419fbf01d288e9eca05eea7c8ed4", 7278107),
	GAME_ENTRY_EN("fhaloness", "Fhaloness.exe", "b908419fbf01d288e9eca05eea7c8ed4", 7277962),  // Win v0.2.1
	GAME_ENTRY_EN("fhaloness", "game.ags", "ec37e655c75f3caaa7be58bb87d5c903", 4213110),  // Mac v0.2.1
	GAME_ENTRY_EN_PLATFORM("fibbersummer", "FibbersSummerProtoWW.exe", "5b1ce55a6c1ab459a3f55187caaf2b7d", 5635377, "Prototype"),
	GAME_ENTRY_EN("fifa2004", "Soccer.exe", "475da5decb9ad2a11e64e2e2e891d8e0", 2524958),
	GAME_ENTRY_EN("fightforlife", "FFL.exe", "452e0b23ec8bac95c3e86732d19879a1", 178832193),
	GAME_ENTRY_EN("finger", "Finger of suspicion.exe", "9ba6f64cc122117779e5528e94f9b8d1", 189584333),
	GAME_ENTRY_EN("fireflystory3d", "FFS 3D.exe", "27343924ddad3be0b97bdcaa71858b1b", 2254453),
	GAME_ENTRY_EN("firstdrop", "Landinge.exe", "3e89794213fbd91efab223754f82cfdd", 11410897),
	GAME_ENTRY_EN("firststitch", "t1stitch.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2622753),  // v2
	GAME_ENTRY_EN("firststitch", "t1stitch.exe", "06a03fe35791b0578068ab1873455463", 3404581),  // v3
	GAME_ENTRY_EN("fistsofmurder", "mags1808.exe", "9cd9563150a69bc1d4eacda0ff3557f8", 55441407),  // Windows
	GAME_ENTRY_EN("fistsofmurder", "mags1808.ags", "060b511dd9f501bae44d2cc37a18a05e", 52994031),  // Linux
	GAME_ENTRY_EN("fixumdude", "fixumdude3dpa.exe", "3c886d441e027ff61bf641f4ed544a0e", 155902768),
	GAME_ENTRY_EN("flamebarrels", "Flame barrel.exe", "06a03fe35791b0578068ab1873455463", 2298745),
	GAME_ENTRY_EN("flashdown", "FlashDown.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 1655207),
	GAME_ENTRY_EN("flashmccoy", "FlashMcCoy.exe", "0564de07d3fd5c16e6947a647061913c", 16314015),
	GAME_ENTRY_EN("flightrobots", "FlightFromTheRobots.exe", "3ab9922a2ab7c5875af4c9794b33e012", 4517631),
	GAME_ENTRY_EN_PLATFORM("flightrobotsch2", "FlightFromTheRobots2.exe", "9bb70a54ea1c4063c6821c88c7427dfb", 4751416, "Original Version"),
	GAME_ENTRY_EN_PLATFORM("flightrobotsch2", "FlightFromTheRobots2.ags", "ae9236152eef64bf635f7bb63c981a88", 1643556, "Original Version"),
	GAME_ENTRY_EN("flightrobotsch2", "FlightFromTheRobots2.exe", "0595b55383d72a99cd995092c5bf8475", 5624650),
	GAME_ENTRY_EN("flightrobotsch2", "FlightFromTheRobots2.ags", "14991349066fababce1050b192c0951b", 2515766),
	GAME_ENTRY_EN("floatyrog", "FloatyRog.exe", "6e6f33162242d74dd1fa042429f16562", 2541918),
	GAME_ENTRY_EN("floatyrog", "FloatyRog.exe", "6e6f33162242d74dd1fa042429f16562", 2592017),
	GAME_ENTRY_EN("flophouse", "FlopHigh.exe", "0241777c2537fc5d077c05cde10bfa9f", 5807004),
	GAME_ENTRY_EN_PLATFORM("flophouse", "FlopHigh.exe", "0241777c2537fc5d077c05cde10bfa9f", 9159785, "Deluxe"),
	GAME_ENTRY_EN("flowergirl", "flowergirl.exe", "9cb3c8dc7a8ab9c44815955696be2677", 62544543),
	GAME_ENTRY_EN("flukie", "Flukie.exe", "7ddb9e776648faed5a51170d087074e9", 40593956),
	GAME_ENTRY_EN("fluxworld", "FluxWorld.exe", "06a03fe35791b0578068ab1873455463", 5614789),
	GAME_ENTRY_EN("flyingthinker", "Flying Thinker (eng).exe", "615e73fc1874e92d60a1996c2330ea36", 712414432),
	GAME_ENTRY_EN("flypaper", "OROW.exe", "06a03fe35791b0578068ab1873455463", 15723518),
	GAME_ENTRY_EN("focality", "Focality.exe", "6465bcd8a1ec4ee001ba2f2029a4ee02", 10679646),
	GAME_ENTRY_EN("foggydawn", "Fog3.2.exe", "615e73fc1874e92d60a1996c2330ea36", 8336612),  //v1.1
	GAME_ENTRY_EN_PLATFORM("foggynotions", "Foggy.exe", "615e73fc1874e92d60a1996c2330ea36", 8417964, "Alpha"),
	GAME_ENTRY_EN_PLATFORM("foggynotions", "Foggy.exe", "2ca80bd50763378b72cd1e1cf25afac3", 11872355, "Beta"),
	GAME_ENTRY_EN("foodwars", "Food Wars.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 128468125),  // Windows
	GAME_ENTRY_EN("foodwars", "Food Wars.ags", "c7e2353e800c13851105cfa28d9b3561", 125431945),  // Linux
	GAME_ENTRY_EN("forest", "ForestOfPeople.exe", "615e73fc1874e92d60a1996c2330ea36", 18593840),
	GAME_ENTRY_EN("forestdweller", "FD3D.exe", "e80586fdc2db32f65658b235d8cbc159", 11101168),
	GAME_ENTRY_EN("foolaround", "FoolAround.exe", "7825fa980fcd058caadaf6d3377b1f78", 43258878),
	GAME_ENTRY_EN("foolaround", "FoolAround.ags", "34b16504571f8fc59f3ed836d4c6fbd3", 40223210),
	GAME_ENTRY_EN("forge", "Forge - Chapter One.exe", "1350d2520339f030c8b20283122a027f", 181244590),  // v1.0
	GAME_ENTRY("forge", "Forge - Chapter One.exe", "1350d2520339f030c8b20283122a027f", 180565568),  // v1.1 En-It
	GAME_ENTRY_EN("forgerecap", "Loom Recap.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 9908492),
	GAME_ENTRY_EN("forms", "Forms.exe", "78f88f71bbef277d93cbe10f1d2ac404", 6738014),
	GAME_ENTRY_EN("fortressofwonders", "Fortress.exe", "465f972675db2da6040518221af5b0ba", 4474304),
	GAME_ENTRY_EN("foundations", "Foundations.exe", "8b4a2cb0849b02ffb981dc06b38c968e", 13779901),
	GAME_ENTRY_LANG("four", "4.exe", "2fb2ff7aa681aa7f1ecddaf1963a4e2f", 56328938, Common::RU_RUS),
	GAME_ENTRY_EN("framed", "Jhum.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 4572587),
	GAME_ENTRY_EN("fragile", "GGJ20.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 2580434),
	GAME_ENTRY_EN("frameonthewall", "RoomofFear_Oct.exe", "495d45fb8adfd49690ae3b97921feec6", 17366083),
	GAME_ENTRY_EN("frankenpooper", "frankenpooper.exe", "fb3b0f6bad923958d9d9198daea125e6", 24372899),
	GAME_ENTRY_EN("frankfurter1", "FRANKFURTER.exe", "3367a5cfcf7b3e1948a276ac71ce1033", 9509749),
	GAME_ENTRY_EN("frankfurter2ch1", "FRANK FURTHER.exe", "5874553fb86462672c94be2bed2a6534", 209844613),
	GAME_ENTRY_EN("frankstallone", "Driver.exe", "7ec46f3c1c857731d6f7ff022be22c14", 20844627),
	GAME_ENTRY_EN("frankthefarmhandpart1", "Frank.exe", "0006c0a95a5f35ca0d275adecf9dfa1d", 22602166),
	GAME_ENTRY_EN("frankthefarmhandpart2", "Frank2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 63832313),
	GAME_ENTRY_EN_PLATFORM("franticfranko", "steelh1.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 118746106, "WIP2"),
	GAME_ENTRY_EN("frasiercraneseattlerampage", "FRASIER.exe", "0241777c2537fc5d077c05cde10bfa9f", 3688664),
	GAME_ENTRY("freakchic", "FreakChic.exe", "0241777c2537fc5d077c05cde10bfa9f", 58493059),  // En-Fr
	GAME_ENTRY_EN("fredandbarneymeetthefuture", "Fred and Barney meet the future.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 234107316),
	GAME_ENTRY_EN("fredandbarneymeetthefuture", "Fred and Barney meet the future.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 234107660),
	GAME_ENTRY_EN("fredandbarneymeetthefuture", "Fred and Barney meet the future.ags", "5e9885d094c22bd18a5b4b0d5ffa81ff", 231627516),
	GAME_ENTRY_EN("fribbeldib", "firbbeldib.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 90835696),
	GAME_ENTRY_EN("fridgefollies", "Fridge.exe", "2ca80bd50763378b72cd1e1cf25afac3", 15002210),
	GAME_ENTRY_EN("frightfest", "FrightFest.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 162130501),
	GAME_ENTRY_EN("frightfest", "FrightFest.ags", "b0fbca28e8bd1e83878682b245734746", 159650357),
	GAME_ENTRY_EN("fritz", "Fritz.exe", "a4e6ec808b347f4456eae7c808e90727", 70554365),  // Windows
	GAME_ENTRY_EN("fritz", "Fritz.ags", "df0330e366a4c3fd13e0e2ef491a2e3a", 67522793),  // Linux
	GAME_ENTRY_EN("frogisland", "frog_island.exe", "465f972675db2da6040518221af5b0ba", 4355477),
	GAME_ENTRY_EN("frozenkingdom", "Frozen Kingdom.exe", "5cca0f8af961d2bc1c6e475437d3b23b", 331574752),  // Windows
	GAME_ENTRY_EN("frozenkingdom", "Frozen Kingdom.ags", "f6e18a377e81dd3238df54e1abf161ce", 328541132),  // Linux
	GAME_ENTRY_EN("fsis666", "666days_arrgh.exe", "06a03fe35791b0578068ab1873455463", 1948306),
	GAME_ENTRY_EN("fsis1000000quest", "$1000000.exe", "0710e2ec71042617f565c01824f0cf3c", 833468),
	GAME_ENTRY_EN("fsisachequest", "achequest.exe", "06a03fe35791b0578068ab1873455463", 2952369),
	GAME_ENTRY_EN("fsisalienation", "alienation.exe", "06a03fe35791b0578068ab1873455463", 3812243),
	GAME_ENTRY_EN("fsiscotmattcehotsvd", "valgame.exe", "06a03fe35791b0578068ab1873455463", 2199912),
	GAME_ENTRY_EN("fsismhcfhr", "das_id.exe", "0710e2ec71042617f565c01824f0cf3c", 1852114),
	GAME_ENTRY_EN("fsismountain", "fsiexpgame.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3943438),
	GAME_ENTRY_EN("fsispowercowfromuranus", "PowerCowen.exe", "0710e2ec71042617f565c01824f0cf3c", 1442185),
	GAME_ENTRY_EN("fsisunbearable", "teddyquest.exe", "06a03fe35791b0578068ab1873455463", 2276288),
	GAME_ENTRY_EN("fsivacuum", "vacuum.exe", "06a03fe35791b0578068ab1873455463", 1686153),
	GAME_ENTRY_EN("fuguestate", "Fugue.exe", "6f82c6f19745c70a30b09e662eebfc24", 344922127),
	GAME_ENTRY_EN("fulkramick", "DreamtHaus.exe", "06a03fe35791b0578068ab1873455463", 47212093),
	GAME_ENTRY_EN("funsunmishaps", "Fun, Sun & Mishaps.exe", "933200c8f306eccaaa484b1575da8528", 452348894),
	GAME_ENTRY_EN("funsunmishaps", "Fun, Sun & Mishaps.exe", "933200c8f306eccaaa484b1575da8528", 452348866),
	GAME_ENTRY_EN("funwithnumbers", "fwn.exe", "a524cbb1c51589903c4043b98917f1d9", 18743833),
	GAME_ENTRY_LANG("fuoriora", "Fuoriora.ags", "476087d40d6f86d42731eb94685db38d", 59443091, Common::IT_ITA),  // itch.io client
	GAME_ENTRY_EN("furballs1", "Fur Balls 1 Evil be thy name.exe", "c876b504a1048fe06c71c9d9a0bc5817", 11417569),
	GAME_ENTRY_EN("furryfather", "Sorrow of a Furry Father.exe", "0fe1d93da69963d5eef55ce2619c94be", 20220753),  // Windows
	GAME_ENTRY_EN("furryfather", "Sorrow of a Furry Father.ags", "5f2b5f4a20550110de2a00c46c722dee", 17186109),  // Linux
	GAME_ENTRY_EN("futuramatrivia", "FuturamaWhoSaidThat.exe", "8923670fc767b7b66ac2659556bdc3da", 125539307),  // Windows
	GAME_ENTRY_EN("futuramatrivia", "FuturamaWhoSaidThat.ags", "a02b24866dbcc53227b96d4767fd566e", 122145751),  // Linux
	GAME_ENTRY_EN("futurecity3000", "FutureCity3000.exe", "4d4b6d6e560cf32f440c39f8d3896da5", 41930580),
	GAME_ENTRY_EN("fycorpse", "Dead Dave.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 2152157),
	GAME_ENTRY_EN_PLATFORM("gabyking", "GabyKing.exe", "1fd15cc387812c04447d89729b24b097", 2260498, "OROW"),
	GAME_ENTRY_EN("gabyking", "GabyKing.exe", "1fd15cc387812c04447d89729b24b097", 2289131),
	GAME_ENTRY_LANG("galaxiaquest", "Galaxia Quest.exe", "4bcbc24015114752b3c7971128704689", 8336565, Common::DE_DEU),
	GAME_ENTRY_EN("galaxyquest", "My Game.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1809822),
	GAME_ENTRY_LANG("gallowsvillage", "Gallows Village.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2613392, Common::CS_CZE),
	GAME_ENTRY_EN("gamequest", "Game Quest.exe", "06a03fe35791b0578068ab1873455463", 6597978),
	GAME_ENTRY_EN("garbheileach", "The Secret of Garbh Eileach.exe", "25976a689b0f4d73eac69b1728377ecb", 18399510),
	GAME_ENTRY_EN("gardnerheist", "gardner_heist.exe", "7971a7c02d414dc8cb33b6ec36080b91", 80933640),
	GAME_ENTRY_EN("garfieldlasagna", "Garfield.exe", "b1c22ebffb0789de8142265620073add", 23768285),
	GAME_ENTRY_EN("gatewayremake", "Gateway Remake.exe", "0241777c2537fc5d077c05cde10bfa9f", 210991212),
	GAME_ENTRY_EN("gatewayremake", "Gateway Remake.exe", "0241777c2537fc5d077c05cde10bfa9f", 212491008),
	GAME_ENTRY_EN("gaygreg", "ggig.exe", "3128b9f90e2f954ba704414ae854d10b", 3876381),
	GAME_ENTRY_EN("gemcollector", "gemgame.exe", "789facada85e12c3ac19d5a6d4379a49", 75235335),
	GAME_ENTRY_EN("genbu", "Genbu.exe", "64fcaf7da0b257ea831f89c54be0ad72", 1892238),
	GAME_ENTRY_EN("geometricshapes1circleboy", "Circle1.exe", "06a03fe35791b0578068ab1873455463", 2389129),
	GAME_ENTRY_EN("getawayfrompluto", "Get away.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 755529),
	GAME_ENTRY_EN("getfood", "getfood.exe", "495d45fb8adfd49690ae3b97921feec6", 10600153),
	GAME_ENTRY_EN("getsomesleep", "GetSomeSleep.exe", "b5c41e08919834b549ca350a28f18f87", 66698807),
	GAME_ENTRY_EN("getsquirty", "Get Squirty.exe", "78dd4ca028ee0156b6a093d6d780aa65", 9498475),
	GAME_ENTRY_EN("gettingpicture", "Getting the picture.exe", "8f18ad4456e6dbeaf041cd91449a55ba", 7797274),  // Windows
	GAME_ENTRY_EN("gettingpicture", "Getting the picture.ags", "452def82c59d0982ca3babf2aa23fa75", 4761094),  // Linux
	GAME_ENTRY_EN("ghormakhour", "LAGRUP.exe", "a3ad5fa6463c0116a2ac8986841860e0", 1372424),
	GAME_ENTRY_EN("ghostcatchers", "GhostCatchers.exe", "338fa79960d40689063af31c671b8729", 230925663),
	GAME_ENTRY("ghostland", "Ghostland.exe", "0595b55383d72a99cd995092c5bf8475", 307398815),  // Eng-Esp
	GAME_ENTRY("ghostland", "Ghostland.exe", "0595b55383d72a99cd995092c5bf8475", 307402122),  // Eng-Esp
	GAME_ENTRY_EN("ghoststories", "Ghost Stories.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 3103761),
	GAME_ENTRY_EN("ghostv", "GhostV.exe", "f733dab121343e95a8265b4c4f124511", 93781270),
	GAME_ENTRY_EN("ghostv", "GhostV.ags", "a6d366588714328351787716ec42350e", 91225350),
	GAME_ENTRY_EN_PLATFORM("ghostv", "GhostV.ags", "760395fcadad1bd421472b24bd5e705c", 11296684, "Test"),
	GAME_ENTRY_EN("ghostvirus", "GhostInvasion.exe", "0241777c2537fc5d077c05cde10bfa9f", 2444356),
	GAME_ENTRY_EN("ghostvoyage", "Ghost Voyage.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 7870814),
	GAME_ENTRY_EN("giftingspirit", "Gifting Spirit.exe", "7130eeba055e91ebf32d95a4751816d3", 10550723),  // Windows
	GAME_ENTRY_EN("giftingspirit", "Gifting Spirit.ags", "d325fde1085cfebd7bac7672b0127ac8", 7514543),  // Linux
	GAME_ENTRY_EN("girlandrabbit", "Game.exe", "82da2565c456dcfb265ded6fe3189c0b", 2877964),
	GAME_ENTRY_EN_PLATFORM("girlandrabbit", "Game.exe", "82da2565c456dcfb265ded6fe3189c0b", 2723432, "OROW"),
	GAME_ENTRY_EN("girlinthehouse", "TheGirlInTheHouse.exe", "bbabdc84fb72ed9a340b2386118f0471", 464431933),
	GAME_ENTRY_EN("gladiatorquest", "Glad.exe", "97020973a2a635fe28eb0ba4bdfaa70b", 2821000),
	GAME_ENTRY_EN("glitchquest", "glitchquest.exe", "465f972675db2da6040518221af5b0ba", 3182715),
	GAME_ENTRY_EN_PLATFORM("glitchquest", "GQnofun.exe", "465f972675db2da6040518221af5b0ba", 3181931, "No-Fun"),
	GAME_ENTRY_EN("globalistagenda", "yamaiay.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 24536162),
	GAME_ENTRY("gloriouswolf2", "GloriousWolf2.exe", "776a62db4387dc68be92ef9933399fd5", 378308425),  // En-Fr
	GAME_ENTRY_EN("gnomeshomebrewingadventure", "gnomebrew.exe", "abb3aee32ae97a8475626cceefb0664e", 50307521),
	GAME_ENTRY_EN_PLATFORM("gnrblex", "Gnrblex_AGS.exe", "476a1a39d43ea27577eacf907173e2c1", 85038656, "MAGS"),  // MAGS v1.0.0.1
	GAME_ENTRY_EN_PLATFORM("gnrblex", "Gnrblex_AGS.exe", "476a1a39d43ea27577eacf907173e2c1", 85043059, "MAGS"),  // MAGS v1.0.0.5
	GAME_ENTRY_EN_PLATFORM("gnrblex", "Gnrblex_AGS.exe", "59fe2aa2cc67589f11707ddc5a7f01cc", 115462750, "Final/Steam"),  // Final/Steam v2.0
	GAME_ENTRY_EN("goatburn", "Goat Burn.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 49624859),
	GAME_ENTRY_EN_PLATFORM("goatburn", "Goat Burn.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 49624244, "MAGS"),
	GAME_ENTRY_EN("goatherd", "Goat Herd and the Gods.exe", "56ce1bc6ab10748cb4ddb91435cfa578", 91710325),
	GAME_ENTRY_EN_STEAM("gobyworld", "Gobyworld.exe", "c1be9e9512e62644d40aedb5bf6cd649", 33204590),
	GAME_ENTRY_EN("gobyworld", "Gobyworld.ags", "3bf6d8e922f674b131507bafc1bab97e", 30036826),  // Web/itch.io
	GAME_ENTRY("goinghome", "Going Home.ags", "411df0b8ceb282d781890f80017cbb68", 2829211),  // Eng-Heb
	GAME_ENTRY_EN("goldreddragon", "Mongoose Fitch.exe", "615e73fc1874e92d60a1996c2330ea36", 4508646),
	GAME_ENTRY_EN("goneboatfishin", "Gone Boat Fishin'.exe", "bdd1df0484e296faa348ffcb03e16273", 72936045),
	GAME_ENTRY_EN("gonefishin", "Gone fishin'.exe", "338fa79960d40689063af31c671b8729", 76569490),
	GAME_ENTRY_EN("gonemyangel", "GoneMyAngel.exe", "615e73fc1874e92d60a1996c2330ea36", 16422700),
	GAME_ENTRY_EN("gonorth", "GO NORTH.exe", "f18443f98fd61d2c655e76a17f7da905", 14398652),
	GAME_ENTRY_EN("gonorth2", "GO NORTH.exe", "618d7dce9631229b4579340b964c6810", 13294868),
	GAME_ENTRY_EN("goodgod", "Good God!.exe", "02bd7b853a95bca41a594567c4f59fd6", 8372952),
	GAME_ENTRY_EN("goodmorningmrgingerbread", "mister_gingerbread.exe", "b42f80733b6bd1ded5e29be2c683afa8", 7084332),
	GAME_ENTRY_EN("goodsantabadsanta", "x-mags.exe", "71ca0d6c1c699595f28a2125948d4a84", 1966547),
	GAME_ENTRY_EN("goontang", "GC.exe", "0710e2ec71042617f565c01824f0cf3c", 1243202),
	GAME_ENTRY_EN("gotalight", "gotalight.exe", "daa21f03e41b5d3e7fd7558be3f8616e", 4019593),
	GAME_ENTRY_EN("gpslostadventure", "G.P.s Lost Adventure [Wells8892].exe", "06a03fe35791b0578068ab1873455463", 4181945),
	GAME_ENTRY_EN("gpslostadventure", "GPs_LostAdventure.exe", "06a03fe35791b0578068ab1873455463", 4181945),
	GAME_ENTRY_EN("grandadvest", "GrandadVest.exe", "8d1ff95c16500befbdc72260d461d73f", 20824973),  // Windows
	GAME_ENTRY_EN("grandadvest", "GrandadVest.ags", "cd8d065607f8e9dbed2e7329d57e445a", 17716089),  // Linux
	GAME_ENTRY_EN("grandkitchenescape", "GrandKitchenEscape.exe", "310db16c989e9bbfa3973bc1bb6215f7", 28327560),  // Windows
	GAME_ENTRY_EN("grandkitchenescape", "GrandKitchenEscape.ags", "5d2aac79e25a74a4db113c54df4a01ec", 25110132),  // Linux
	GAME_ENTRY_EN("graveyard", "Graveyard.exe", "955b711b21d7a2df6af1bb0cccccbb08", 13699789),
	GAME_ENTRY_EN("gravitytestgame", "TEST.exe", "97d700529f5cc826f230c27acf81adfd", 768926),
	GAME_ENTRY("gray", "Gray.exe", "b36bda97ef129952eb2d18bf5087530c", 162378773),  // Eng-Pol
	GAME_ENTRY_PLATFORM("gray", "Gray.exe", "b36bda97ef129952eb2d18bf5087530c", 165386532, "Easter Egg Edition"),  // Eng-Pol
	GAME_ENTRY_EN("greateststory", "TheGreatest.exe", "1c756cab811a5e638ad7f19c215d560b", 4750238),
	GAME_ENTRY_EN("greateststory", "TheGreatest.ags", "cda910c19cf1767dafb67380b43af5ba", 1637770),
	GAME_ENTRY_EN("greglinda", "gregandlinda.exe", "618d7dce9631229b4579340b964c6810", 6105164),  // itch.io Win
	GAME_ENTRY_EN("greglinda", "pcjam_copy.exe", "615e73fc1874e92d60a1996c2330ea36", 5753320),  // itch.io Mac
	GAME_ENTRY_EN_PLATFORM("greenred", "greenandred.ags", "41c8087499ad6a34673abb8deac9cc24", 2863544, "MAGS"),
	GAME_ENTRY_EN("greenred", "greenandred.ags", "4d5cce35e6c098f7fb13b9f96ee787af", 2893145),
	GAME_ENTRY_EN("grizzlygooseofgosse", "Goose Fear.exe", "b69b5887e4a33a3b8588d52fed04b730", 32123559),
	GAME_ENTRY_EN("groundhog", "Groundhog.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 67854225),
	GAME_ENTRY_EN("grr", "Grr!.exe", "3128b9f90e2f954ba704414ae854d10b", 26753739),
	GAME_ENTRY_EN("guardiansofgold", "Guardians of Gold.exe", "933200c8f306eccaaa484b1575da8528", 429166629),
	GAME_ENTRY_EN("guardiansofgold", "Guardians of Gold.exe", "933200c8f306eccaaa484b1575da8528", 429237374),
	GAME_ENTRY_EN("gunther", "Hourgame.exe", "7587088a318140e1416e60740e10c904", 1176975),
	GAME_ENTRY_EN("guyhookcrook", "GuyByHookOrByCrook.exe", "b0a3f015490a1c2b2d87483524f96df0", 9024485),  // Windows
	GAME_ENTRY_EN("guyhookcrook", "GuyByHookOrByCrook.ags", "1d909d5622b79c5c86b202723d96088b", 5919185),  // Linux
	GAME_ENTRY_EN("guyredplanet", "rocket.exe", "82da2565c456dcfb265ded6fe3189c0b", 5375785),
	GAME_ENTRY_EN("guyslug", "GuySlug.exe", "0710e2ec71042617f565c01824f0cf3c", 1959514),
	GAME_ENTRY_EN("guyslug", "ac2game.ags", "e093aa35d36b34e353b1233f3bf5a7a3", 1407562),
	GAME_ENTRY_EN("guyver1d", "Guyver1D.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 2618588),
	GAME_ENTRY("guyverquest1", "Guyver Quest.exe", "465f972675db2da6040518221af5b0ba", 774591),  // Eng-Rus
	GAME_ENTRY("guyverquest2", "Guyver Quest 2.exe", "465f972675db2da6040518221af5b0ba", 1138940),  // Eng-Rus
	GAME_ENTRY_EN("hack", "Hack.exe", "f120690b506dd63cd7d1112ea6af2f77", 4384638),
	GAME_ENTRY_EN("hackenslashisland", "mags0902.exe", "88cf59aad15ca331ab0f854e16c84df3", 2678206),
	GAME_ENTRY_EN("halloween", "Halloween.exe", "3128b9f90e2f954ba704414ae854d10b", 5130336),  // v2.0
	GAME_ENTRY_EN("halloweenguest", "Halloweenguest.exe", "8f18ad4456e6dbeaf041cd91449a55ba", 17689741),  // Windows
	GAME_ENTRY_EN("halloweenguest", "Halloweenguest.ags", "1981397bceffc777473efe379d5cec52", 14653561),  // Linux
	GAME_ENTRY_EN("halloweenhorror", "Halloween.exe", "06a03fe35791b0578068ab1873455463", 33740975),
	GAME_ENTRY_EN("halloweenhorror", "Halloween.exe", "06a03fe35791b0578068ab1873455463", 33740433),
	GAME_ENTRY_EN("halloweenparty", "the_halloween_party.exe", "4e0d0aca04563b21a47e8a0cc56605fc", 14751046),
	GAME_ENTRY_EN_PLATFORM("hallwayofadventures", "Hallway.exe", "0710e2ec71042617f565c01824f0cf3c", 7084550, "OROW"),
	GAME_ENTRY_EN("hallwayofadventures", "Hallway.exe", "0710e2ec71042617f565c01824f0cf3c", 7085407),  // v1.1
	GAME_ENTRY_EN("hamresanden2", "The  Hamresanden Chronicles - II.exe", "afe40dc1416dd51e896ee0444d799f07", 22255435),
	GAME_ENTRY_EN("hamster", "Hamster.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1672928),
	GAME_ENTRY_EN("hamsterisland", "Hamster.exe", "0710e2ec71042617f565c01824f0cf3c", 3212291),
	GAME_ENTRY_EN_PLATFORM("hangon", "HangOn.exe", "6e861b1f476ff7cdf036082abb271329", 4203208, "MAGS"),  // MAGS
	GAME_ENTRY_PLATFORM("hangon", "HangOn.exe", "e09e4f152c8f4553e90885391ac72e46", 24219759, "itch.io v2.0.0"),  // 2.0 En-Fr
	GAME_ENTRY_EN("happyduckieadventure", "HDA.exe", "f120690b506dd63cd7d1112ea6af2f77", 1924654),
	GAME_ENTRY_EN("happyface", "^_^.exe", "b36bda97ef129952eb2d18bf5087530c", 4347757),
	GAME_ENTRY_EN("hardspace", "Hard Space!.exe", "3ecd04c0e0df079ff906ec4696d5fe23", 11494382),  // Final/Deluxe
	GAME_ENTRY_EN_PLATFORM("hardspace", "BnP.exe", "8f9c77fd2671fc7e4a91a7cf0b69a731", 8775113, "MAGS"),  // MAGS
	GAME_ENTRY_EN_PLATFORM("hardspace", "BnP.exe", "8f9c77fd2671fc7e4a91a7cf0b69a731", 8971498, "MAGS"),  // MAGS
	GAME_ENTRY("harrypotterrpg", "Harry Potter RPG.exe", "7a3096ac0237cb6aa8e1718e28caf039", 364510777),  // En-Fr
	GAME_ENTRY("harrypotterrpg", "Harry Potter RPG.exe", "0241777c2537fc5d077c05cde10bfa9f", 364169114),  // En-Fr
	GAME_ENTRY_EN("harrys21stbirthday", "harold.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3525440),
	GAME_ENTRY_EN("harrys21stbirthday", "harold.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3525444),
	GAME_ENTRY_EN("harryshopshock", "Harry and the Locked Crocs Shop Shock.exe", "495d45fb8adfd49690ae3b97921feec6", 13062995),
	GAME_ENTRY_EN("harryshopshock", "snapattack.ags", "8f45073f4c89b9b9fd8990740bafa03a", 10582339),
	GAME_ENTRY_EN("hauntedcastle", "haunted.exe", "3c22ec19306691b68c34f6ce98377265", 40287804),
	GAME_ENTRY_EN("hauntingsmanor", "Hauntings.exe", "0710e2ec71042617f565c01824f0cf3c", 24783520),
	GAME_ENTRY_EN("haven1", "A small world.exe", "b142b43c146c25443a1d155d441a6a81", 43019171),  // Windows
	GAME_ENTRY_EN("haven1", "A small world.ags", "c764faa670679c24eb4d51957387f8b4", 40539027),  // Linux
	GAME_ENTRY_EN("hawkeye", "heq.exe", "2870eb249d21d414ca1de98401190293", 136607518),
	GAME_ENTRY_EN("hawkmanor", "Hawk Manor.exe", "f0d92db6e3c9dd55425db62569858613", 447123466),
	GAME_ENTRY_EN("headbangerheaven", "Beta8.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 74939699),
	GAME_ENTRY_EN("headoverheels", "HeadOverHeels.exe", "9adc0f6d055efb4e21093f9216dd40fe", 12051069),  // Deluxe
	GAME_ENTRY_EN_PLATFORM("headoverheels", "HeadOverHeels.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 9325331, "MAGS"), // MAGS
	GAME_ENTRY_EN_PLATFORM("heartland", "Heartland1.1.exe", "261d108f9213356a351b35b54867f342", 20429469, "OROW"),
	GAME_ENTRY_EN("heartland", "Heartland V1.2.exe", "261d108f9213356a351b35b54867f342", 21237374),
	GAME_ENTRY_EN_PLATFORM("heartland", "Heartland.exe", "0829f8e184ed6a4bf36b14ba42003a67", 6702004, "Deluxe"),
	GAME_ENTRY_EN_PLATFORM("heartland", "Heartland.exe", "0829f8e184ed6a4bf36b14ba42003a67", 6701875, "Deluxe"), // v1.1
	GAME_ENTRY_EN("heartofabraxas", "orowgame.exe", "0710e2ec71042617f565c01824f0cf3c", 15632750),
	GAME_ENTRY_EN("heartpart", "HeartPart.exe", "e257b5a3b300568570c4af5e71b20e88", 3565232),
	GAME_ENTRY_EN("heatwave", "Heatwave.exe", "e2f7df57d111e57e3cf1e229088c6947", 3638644),
	GAME_ENTRY_EN("heavenhell", "Limbo Adventure.exe", "9ecb923d5169ded48d5fd2c6ed4befa4", 31138864),
	GAME_ENTRY_EN_PLATFORM("heavenhell", "Limbo Adventure.exe", "7c10efb8990fb48ded51fbcd88a6bf17", 30800724, "AKA Limbo - The Adventure Game"),
	GAME_ENTRY_EN("heavymetalnannulf", "NannulfMAGS.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 21272417),
	GAME_ENTRY_EN("hecamethroughthedoor", "He Came Through the Door.exe", "78dd4ca028ee0156b6a093d6d780aa65", 30263715),
	GAME_ENTRY_EN("heed", "Heed.exe", "e8b2a430042709f07183c2c249e0e0d9", 6300045),
	GAME_ENTRY_EN("helloneighbor", "Hello Neighbor!.exe", "a524cbb1c51589903c4043b98917f1d9", 6811100),
	GAME_ENTRY_EN("hellotaxi", "taxi.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3132739),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 4783941, "0.1.0 Alpha"),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 6127544, "0.2.0 Alpha"),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 6137066, "0.3.0 Alpha"),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 6365194, "0.4.0 Alpha"),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 6371263, "0.5.0 Alpha"),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 7535277, "1.0"),  // Windows
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.ags", "a5032265c42a1a976a86870c9a1193e0", 5049501, "1.0"),  // Linux
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "7ddb9e776648faed5a51170d087074e9", 9712775, "1.1.0"),
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.exe", "e09e4f152c8f4553e90885391ac72e46", 9769286, "1.2.0"),  // Windows
	GAME_ENTRY_EN_PLATFORM("hellspuppy", "dogfromhell.ags", "cc16dae148e82e8d79f1ad0269ec3668", 7283510, "1.2.0"),  // Linux
	GAME_ENTRY_EN("helpthegame", "HELP.exe", "06a03fe35791b0578068ab1873455463", 3686323),
	GAME_ENTRY_LANG("helycia", "helyciaHHH.exe", "7971a7c02d414dc8cb33b6ec36080b91", 332887205, Common::FR_FRA),  // Windows
	GAME_ENTRY_LANG("helycia", "helyciaHHH.ags", "d968f0691ba575759b857aa45443c465", 330390165, Common::FR_FRA),  // Linux
	GAME_ENTRY_EN("hendrixisland", "HendrixIsland.exe", "5c06d8be06d65fbda5a006ec703fbd41", 29319207),
	GAME_ENTRY("henkstroemlostincellar", "Henk_LIC.exe", "3128b9f90e2f954ba704414ae854d10b", 1391240),  // En-De
	GAME_ENTRY_EN("henman", "hen-man.exe", "615e73fc1874e92d60a1996c2330ea36", 19556067),
	GAME_ENTRY_EN("henrysmith", "Henry Smith.exe", "b5c41e08919834b549ca350a28f18f87", 10896764),
	GAME_ENTRY_EN("herby", "Herby.exe", "033f047c11ee95db0058c188ec620bb2", 6715991), // itch.io Win/Linux
	GAME_ENTRY_EN("herenosirens", "mags092018.exe", "78dd4ca028ee0156b6a093d6d780aa65", 52618339),
	GAME_ENTRY_EN("herenosirens", "mags092018.ags", "558f2f3cb55d2045d164fe342f15ac99", 50133587),
	GAME_ENTRY_EN("heroeswyrdale", "mags1904.exe", "7971a7c02d414dc8cb33b6ec36080b91", 195444903),  // Windows
	GAME_ENTRY_EN("heroeswyrdale", "mags1904.ags", "51ff78da52b2873cacb5e9b21cc1c610", 192947863),  // Linux
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("heroinesquest", "heroine's quest.exe", "35b93e905a5aeba8fafd0e5b0f4cb9b6", 7014402, 0),  // v1.2.6   Multilang
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("heroinesquest", "heroine's quest.exe", "fa5b449d9d93a9055e0ab6caf0aee77c", 7020827, 0),  // v1.2.7-8
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("heroinesquest", "heroine's quest.ags", "5a61e3d27494f480ba0e8af34cd872ac", 77680620, 0),  // v1.2.9  Win/Linux
	GAME_ENTRY_PLUGIN_STEAM_NOAUTOSAVE("heroinesquest", "heroine's quest.ags", "dc29cf98484fe8fc532a175e758a1930", 77788201, 0),  // v1.2.9 patched Win/Linux
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("heroinesquest", "heroine's quest.ags", "bd8856f443b4fb5af08bbe5a28422618", 77680828, 0),  // v1.2.9.2 Win
	GAME_ENTRY_PLUGIN_GOG_NOAUTOSAVE("heroinesquest", "heroine's quest.ags", "4d4ccff6e7d0b7828fe227c81efb286c", 77682059, 0),  // v1.2.9.2 Linux
	GAME_ENTRY("heroinesquest", "heroine's quest.exe", "0b19953a0a879b5027c98b0cdd8142f1", 6825340),
	GAME_ENTRY("heroinesquest", "heroine's quest.exe", "35b93e905a5aeba8fafd0e5b0f4cb9b6", 6952250),
	GAME_ENTRY_LANG("heroquestbeuk", "HeroQuestbeuk.exe", "06a03fe35791b0578068ab1873455463", 45021220, Common::FR_FRA),
	GAME_ENTRY_EN("hesgonehistorical", "His.exe", "465f972675db2da6040518221af5b0ba", 5768754),
	GAME_ENTRY_EN("hhgtgtowelday", "TowelDay.exe", "18456f28d9bf843b087e80072c85beca", 5431338),
	GAME_ENTRY_EN("hiddenplains", "eureka02.exe", "6afafd26476d17a5e2a8e41f690d3720", 384360829),  // Windows
	GAME_ENTRY_EN("hiddenplains", "eureka02.ags", "024d04d4d227cfb32b66a7fb8e4561f2", 381866349),  // Linux
	GAME_ENTRY_EN("hiddentreasureryansfortune", "HiddenTRF.exe", "504df40bf50a0859e3dc15b000dab5f6", 7345149),
	GAME_ENTRY_EN("hide", "Hide.exe", "6e861b1f476ff7cdf036082abb271329", 13701886),
	GAME_ENTRY_EN("highlandspirit", "Highlandspirit.exe", "0564de07d3fd5c16e6947a647061913c", 20512793),
	GAME_ENTRY_EN("highlandspirit", "Highlandspirit.exe", "0564de07d3fd5c16e6947a647061913c", 20513138),  // 0.1.0
	GAME_ENTRY_EN("him", "twin.exe", "6b4ceb9e327ac99479c08d825461f4cb", 23721672),
	GAME_ENTRY_EN("hitchhikersguidetothegalaxyremake", "HHGTG.exe", "e003041f4332f136920e636e39d3e127", 34956191),
	GAME_ENTRY("hitthefreak", "HTF.EXE", "f3a13b2d6c2e0fe04c6f466062920e23", 3103477),  // Eng-Esp
	GAME_ENTRY_EN("hiyah", "HiYah- game.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2972762),
	GAME_ENTRY_EN("hjarta", "Hjarta.exe", "1c00b5d482f5531cb1eba7360b98b3e4", 642373238),  // v1.0.1
	GAME_ENTRY_EN("hjarta", "Hjarta.exe", "1c00b5d482f5531cb1eba7360b98b3e4", 642373213),  // v1.0
	GAME_ENTRY_EN("hlobb", "Boxing.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9544151),
	GAME_ENTRY_EN("hoik", "game.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5408433),
	GAME_ENTRY_EN("hoik", "game.ags", "3b7cceb3e4bdb031dc5d8f290936e94b", 5408433),
	GAME_ENTRY_EN("holocaustmuseum", "holo.exe", "97d700529f5cc826f230c27acf81adfd", 4899123),
	GAME_ENTRY_EN("homesweetron", "RoN - Home Sweet RoN.exe", "311cf286b596ed7c68d5505424dea1bc", 14135201),
	GAME_ENTRY_EN("homunculus", "Homunculus.exe", "dbe9bab672130b9a84925cb6da64a3db", 7837340),
	GAME_ENTRY_EN("homunculus", "LD52.exe", "dbe9bab672130b9a84925cb6da64a3db", 8147293),
	GAME_ENTRY_EN("homunculus", "LD52.ags", "bb5ff285717fd45faaedb5a99311a8fb", 5038921),
	GAME_ENTRY_EN("honksadventure", "house_eng_5.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1746878),
	GAME_ENTRY_LANG("honksadventure", "housesitting_18.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1746919, Common::DE_DEU),
	GAME_ENTRY_LANG("hookhook", "Hook.exe", "f120690b506dd63cd7d1112ea6af2f77", 23414224, Common::FR_FRA),
	GAME_ENTRY_EN("hope", "Hope.exe", "f92eba780bd1177612e264263d34c555", 4195489),
	GAME_ENTRY_EN("hoppinghomeward", "Journey.exe", "c72b5b9289da6fd9aaffbe2512d298a4", 10317292),  // Windows
	GAME_ENTRY_EN("hoppinghomeward", "Journey.ags", "a1c230b16f4467852f5e618e2e21d182", 7494108),  // Linux
	GAME_ENTRY_EN("horrornaturally", "Horror, naturally.ags", "c512de1e08e9aa2e63d26522401d07cf", 70445627),
	GAME_ENTRY_EN("horrornaturally", "Horror, naturally.exe", "03741a77615d6ae1bf2cfb9e7dc8d347", 73553487),
	GAME_ENTRY("horseparkdeluxe", "Horse Park DeLuxe.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 22491099),  // En-Fr
	GAME_ENTRY("horseparkfantasy", "Horse Park Fantasy.exe", "615e73fc1874e92d60a1996c2330ea36", 16576614),  // En-Fr?
	GAME_ENTRY_EN("hotelhansen", "Hotel.exe", "3128b9f90e2f954ba704414ae854d10b", 4006863),  // v2.0
	GAME_ENTRY_EN("hotelhijinks", "HH.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1490538),
	GAME_ENTRY_EN("housedesade", "House of de Sade.exe", "173f00e582eebd082d66f1291702b373", 85765288),
	GAME_ENTRY_EN("housequest2", "house2.exe", "9cb3c8dc7a8ab9c44815955696be2677", 4471723),
	GAME_ENTRY_EN_PLATFORM("howmany", "How many.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 5925587, "OROW"), //  v1.0
	GAME_ENTRY_EN("howmany", "How many.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 5925931), //  v1.1
	GAME_ENTRY("howtheyfoundsilence", "How They Found Silence.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 5322796),  // En-Fr
	GAME_ENTRY_EN("hpunk", "H_punk.exe", "7971a7c02d414dc8cb33b6ec36080b91", 69631273),
	GAME_ENTRY_EN("hpunk", "H_punk.ags", "afcf0b3a3a9f7c6904a10ce1d9992fad", 67134233),
	GAME_ENTRY_EN("hubris", "Hubris.exe", "3b2f2a03ab0ae3ce1d2248843b9c2395", 6772869),
	GAME_ENTRY_EN("hueshaunt", "hothsv122023.exe", "328a715c0a826cad7d6824c6e76b52d5", 35765557),  // Windows
	GAME_ENTRY_EN("hueshaunt", "hothsv122023.ags", "84b788c6c0cb73070a819f453a7674d3", 32610593),  // Linux
	GAME_ENTRY_EN("hugglestrip", "HugglesGoesOnATrip.exe", "506a1ba4dbcf04ca9fd8b62e4fea0e05", 4287650),  // Windows
	GAME_ENTRY_EN("hugglestrip", "HugglesGoesOnATrip.ags", "112e1e4e8ea08e4fb60199cfbd307b89", 1095822),  // Linux
	GAME_ENTRY_LANG("humbleescape", "the improbable humble escape.exe", "615e73fc1874e92d60a1996c2330ea36", 4905648, Common::IT_ITA),
	GAME_ENTRY_EN("hungry", "Hungry!.exe", "0710e2ec71042617f565c01824f0cf3c", 15619749),
	GAME_ENTRY_LANG("hungryworm", "boom.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 934966, Common::NL_NLD),
	GAME_ENTRY_EN("huongjiaoping", "hotpot.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 47237302),
	GAME_ENTRY_EN("hurray", "hurray.exe", "0b7529a76f38283d6e850b8d56526fc1", 1905184),
	GAME_ENTRY_EN("hybrid", "hybrid.exe", "c5d2c54c20cb606519b86d3890ee7fc0", 10607084),
	GAME_ENTRY_EN("hydeandseek", "Hyde and Seek.exe", "c2f495a688dc19e66362657dee9aa895", 28066547),
	GAME_ENTRY_EN("hydeandseek", "Hyde and Seek.ags", "e465776ce562f91045c208340022c8a8", 25591011),
	GAME_ENTRY_EN("hydrate", "Hydrate.exe", "b3ba6cb504a31fd2ce43e86280eb06a7", 3968325),  // Windows
	GAME_ENTRY_EN("hydrate", "Hydrate.ags", "eb7203ea3e432183394a36f0b58d7d88", 1470261),  // Linux
	GAME_ENTRY_EN("hypnotoad", "Hypnotoad.exe", "06a03fe35791b0578068ab1873455463", 1947740),
	GAME_ENTRY_EN("iamjason", "IAMJASON.exe", "e8985d9ffbfa1eda77f2eb8d1331944a", 4843842),
	GAME_ENTRY_EN("icantsleep", "The Bait that Lures.exe", "7e2043ac91777f83869ea255d9fcbc28", 67515438),  // Gamejolt
	GAME_ENTRY_EN("icantsleep", "The Bait that Lures.exe", "7e2043ac91777f83869ea255d9fcbc28", 67515562),  // v1.2 prologue itch.io
	GAME_ENTRY_EN("icbm", "ICBM.exe", "50cfb1bd9e9022dc57ef7ba4a48b8549", 4964838),
	GAME_ENTRY_EN("iceintheair", "IceInTheAir.exe", "bf2a2e7c608d33102f4d532e5cdcddda", 4365231),  //v1.2
	GAME_ENTRY_EN("icestationzero", "Ice Station Zero.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 36606112),  // Windows
	GAME_ENTRY_EN("icestationzero", "Ice Station Zero.ags", "970bc84550c7e623e7bce6191f372137", 33569932),  // Linux
	GAME_ENTRY_EN("icecreammystery", "testgame3.ags.exe", "615e73fc1874e92d60a1996c2330ea36", 19578323),
	GAME_ENTRY_EN("id", "iD.exe", "495d45fb8adfd49690ae3b97921feec6", 19035497),
	GAME_ENTRY_EN("iforgot", "Forgot.exe", "6aa30185326552359c7865e55c045a74", 7743871),
	GAME_ENTRY_EN("iggrok", "grok.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1046039),
	GAME_ENTRY_EN("igspaceadventure", "david.exe", "0710e2ec71042617f565c01824f0cf3c", 1922597),
	GAME_ENTRY_EN("igspaceadventurer", "IGS.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1204802),
	GAME_ENTRY_EN("igspaceoddities", "Space Oddities.exe", "0b7529a76f38283d6e850b8d56526fc1", 1783628),
	GAME_ENTRY_EN("igstrangeplanet", "tsp.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 2207926),
	GAME_ENTRY_EN("iiispy", "IIISpy.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 6696408),  // v1
	GAME_ENTRY_EN("iiispy", "IIISpy.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 6696286),
	GAME_ENTRY_EN_PLATFORM("iiispy", "IIISpy.ags", "0a4731cd3995a4d055e84ab5e63ebb80", 6428749, "v1.2"),  // v1.2 Linux
	GAME_ENTRY_EN_PLATFORM("iiispy", "IIISpy.exe", "21d7482c8e968857e2653debeaa8a384", 8909917, "v1.2"),  // v1.2 Win
	GAME_ENTRY_EN("iisstabbings", "MAGS_11_2012.exe", "82da2565c456dcfb265ded6fe3189c0b", 4759681),
	GAME_ENTRY_EN("illuminationdiminishing", "IlluminationDiminishing.exe", "b142b43c146c25443a1d155d441a6a81", 21227029),
	GAME_ENTRY_EN("illuminationdiminishing", "IlluminationDiminishing.exe", "b142b43c146c25443a1d155d441a6a81", 49006279),
	GAME_ENTRY_EN("illuminationdiminishing", "IlluminationDiminishing.exe", "b142b43c146c25443a1d155d441a6a81", 49006319),
	GAME_ENTRY_EN("illusion", "Illusions.exe", "0710e2ec71042617f565c01824f0cf3c", 2576009),
	GAME_ENTRY_EN("imfree", "I'm free.exe", "d90cd956022235ed9b272fb0b2ab5792", 241986165),
	GAME_ENTRY_LANG("imfree", "Soy Libre.exe", "d90cd956022235ed9b272fb0b2ab5792", 241924247, Common::ES_ESP),
	GAME_ENTRY_EN("imnotcrazyrightthecell", "I'm not crazy.exe", "9cb3c8dc7a8ab9c44815955696be2677", 3152216),
	GAME_ENTRY_EN("imnotcrazyrightthewell", "The Well.exe", "9cb3c8dc7a8ab9c44815955696be2677", 5000358),
	GAME_ENTRY_EN("imonlysleeping", "sleeping.exe", "465f972675db2da6040518221af5b0ba", 2489565),
	GAME_ENTRY_EN("impostersyndrome", "wgj2021.exe", "4d5d25446373d4df71c6bda77a15230a", 124105480),
	GAME_ENTRY_EN("imstillhere", "PostApocalypse.exe", "615e73fc1874e92d60a1996c2330ea36", 5436580),
	GAME_ENTRY_EN("inbloom", "In-Bloom.exe", "f43f4ceb97c8b2317fce6e0b921998a5", 9288711),  // v1.4 Win
	GAME_ENTRY_EN("inbloom", "In-Bloom.ags", "a663a1c1bbd052f1339526066cce8117", 3400809),  // v1.4 Linux
	GAME_ENTRY_EN("inconvenience", "inconvenience .exe", "e23988d07454892ef2121b81942783bb", 24476414),
	GAME_ENTRY_EN("indianarodent", "Indiana Rodent.exe", "89df481678b2ddc40ecc9f83caa76b89", 5629366),  //v1.1
	GAME_ENTRY_EN_PLATFORM("indianarodent", "Indiana Rodent.exe", "510be04e5eec2aa92bb78b060f138d64", 4564295, "MAGS"),  //MAGS
	GAME_ENTRY_LANG("indyberceau", "Indy.exe", "6f01048527b457ae0bc9567738fdbe97", 3774629, Common::FR_FRA),
	GAME_ENTRY_EN("indybones", "Book of the Gods.exe", "4064f82b96cab962a1d1bca434ad9c8d", 48937857),
	GAME_ENTRY_EN("indycomingofage", "IndyCOA.exe", "90413e9ae57e222f8913b09d2bc847bc", 2368083),
	GAME_ENTRY("indygoldofgenghiskhan", "Indy.exe", "b676bb7963adc3611bc67635123d293a", 6325803),  // En-It
	GAME_ENTRY_LANG("indynouvelan", "INDIANA A.N.A.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 6645271, Common::FR_FRA),
	GAME_ENTRY_EN("indypassageofsaints", "Compiled_AGS3.2.1.exe", "4370edcb2ef8b9ecb323fb5a94770117", 78409939),
	GAME_ENTRY("indyrelicoftheviking", "Indiana Jones and the relic of the viking.exe", "de375dffcb0635bb84daf82eaddf4e3b", 63943198),  //v1.1 Multilang
	GAME_ENTRY_LANG("indyroyaume", "indiana.exe", "18b284c22010850f79bc5c20054a70c4", 3779448, Common::FR_FRA),
	GAME_ENTRY("indysecretchamber", "The Secret Chamber of Schloss Brunwald.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 3813560),  // En-It
	GAME_ENTRY_EN("ineedawee", "needawee.exe", "f120690b506dd63cd7d1112ea6af2f77", 1377697),
	GAME_ENTRY_EN("infantrydivision", "Infantry.exe", "0710e2ec71042617f565c01824f0cf3c", 9144372),
	GAME_ENTRY("infectionep1", "Infection I.exe", "7132ff7d6b0bc1e9f3e4bd4755390626", 25974295),  // En-De
	GAME_ENTRY_LANG("infimum", "Infimum.exe", "4c5305d07e225f369530dc71b8460cf7", 395870024, Common::ES_ESP),
	GAME_ENTRY_LANG("infimum", "Infimum.ags", "e9dfcd928d281f8577ba056bbc3c5676", 392757044, Common::ES_ESP),
	GAME_ENTRY("infinitemonkeys", "InfiniteMonkeys.exe", "4d7d2addcde045dae6e0363a43f9acad", 4841557),
	GAME_ENTRY_EN_PLATFORM("infinitemonkeys", "InfiniteMonkeys.exe", "21fd0f65dfa48de2b39cb8ec23b30889", 4531999, "MAGS"),
	GAME_ENTRY("infinitemonkeys", "InfiniteMonkeys.exe", "60e8d15faf490b2d317e2036b4fb9a9c", 6126255), //v1.3 En-De-Es-Por
	GAME_ENTRY_EN("infinitybit", "InfinityBit.exe", "27343924ddad3be0b97bdcaa71858b1b", 48338191),
	GAME_ENTRY_EN("inlimbo", "inLIMBO.exe", "0710e2ec71042617f565c01824f0cf3c", 1883031),
	GAME_ENTRY_EN("innersanctum", "Inner Sanctum.exe", "36f44e064eab15e502caeb60fd09f52d", 88875883),
	GAME_ENTRY_EN_PLATFORM("inourmidst", "In Our Midst.exe", "37a9920da93e4599c9deb4ccd10121c0", 14729571, "MAGS"),
	GAME_ENTRY_EN("inourmidst", "In Our Midst.exe", "ca63bb4ee13afa4792b6a4ab35a9ff9c", 15297022),  // v1.0
	GAME_ENTRY_EN("insanebert", "Insane Bert.exe", "18b284c22010850f79bc5c20054a70c4", 1675739),
	GAME_ENTRY("insidemonkeyisland", "MIIM.exe", "06a03fe35791b0578068ab1873455463", 5186306),  // En-It
	GAME_ENTRY_LANG("insidemonkeyisland", "MIIM.exe", "06a03fe35791b0578068ab1873455463", 5148690, Common::IT_ITA),
	GAME_ENTRY("insidemonkeyislandch2", "MIIM2.exe", "06a03fe35791b0578068ab1873455463", 5885525),  // En-It
	GAME_ENTRY_LANG("insidemonkeyislandch2", "MIIM2.exe", "06a03fe35791b0578068ab1873455463", 5854020, Common::IT_ITA),
	GAME_ENTRY_EN("insidemonkeyislandch3", "IMI3_Eng.exe", "06a03fe35791b0578068ab1873455463", 7364307),
	GAME_ENTRY_LANG("insidemonkeyislandch3", "IMI3.exe", "06a03fe35791b0578068ab1873455463", 7337235, Common::IT_ITA),
	GAME_ENTRY_EN("inspectorgismoe", "police.exe", "465f972675db2da6040518221af5b0ba", 2899579),
	GAME_ENTRY_EN("integerbattleship", "intbattl.exe", "0500aacb6c176d47ac0f8158f055db83", 4158273),
	GAME_ENTRY_EN("interdimensionalcs", "ICS.exe", "8d1ff95c16500befbdc72260d461d73f", 15656530),
	GAME_ENTRY_EN("intergalacticspacepancake", "space.exe", "97d700529f5cc826f230c27acf81adfd", 1797309),
	GAME_ENTRY_EN("interstellarborders", "Interstellar Borders New.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 13864633),
	GAME_ENTRY_EN("interstellarinterruption", "Interstellar Interruption.ags", "5bb241c71681dadc3be75a9795ea9be7", 6222268),
	GAME_ENTRY_EN("intestinator", "intestinator.exe", "3421b46ff95c5885603086c39a038a20", 1722265),
	GAME_ENTRY_EN_PLATFORM("intestinator", "ac2game.ags", "af8c785b9c462228b1c134ac79f83986", 423817, "Beta"),
	GAME_ENTRY_EN("intothelight", "ITL.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12145887),
	GAME_ENTRY_EN("intothelight", "ITL.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12146182),  // v2
	GAME_ENTRY_EN("intraworld", "Intra World.exe", "0241777c2537fc5d077c05cde10bfa9f", 44338673),
	GAME_ENTRY_EN("intraworld2", "troll.exe", "495d45fb8adfd49690ae3b97921feec6", 2935816),
	GAME_ENTRY_EN("invasiondeathorbs", "Invasion.exe", "0500aacb6c176d47ac0f8158f055db83", 2073761),
	GAME_ENTRY_LANG("inversion", "Inversion.exe", "615e73fc1874e92d60a1996c2330ea36", 3284798, Common::FR_FRA),
	GAME_ENTRY_EN("invincibleisland", "Invincible.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4218160),
	GAME_ENTRY_EN("invincibleisland", "Invincible.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1751882),  //v1
	GAME_ENTRY_EN("ioawn4t", "ioawn4t.ags", "34455803b41269a1a2dd44cc8b2d549f", 884275141), // Linux
	GAME_ENTRY_EN("ioawn4t", "ioawn4t.exe", "b93a59eb67e202f5e807d7a7e678ad85", 887314393), // Windows
	GAME_ENTRY_EN("ioawn4t", "ioawn4t.exe", "b93a59eb67e202f5e807d7a7e678ad85", 887315687),
	GAME_ENTRY_EN("ioawn4t", "ioawn4t.exe", "11bd8123982f9442f372ed5defc88b50", 888895169),
	GAME_ENTRY_EN("ioawn4t", "game.ags", "11bd8123982f9442f372ed5defc88b50", 888895169),
	GAME_ENTRY_EN_STEAM("ioawn4t", "ioawn4t.exe", "11bd8123982f9442f372ed5defc88b50", 888897420),
	GAME_ENTRY_EN_PLATFORM("irentedaboat", "I_rented_a_boat.exe", "792c0a0eaeba1a8846cb7b1af1e3266e", 144007992, "MAGS"),  // Windows
	GAME_ENTRY_EN_PLATFORM("irentedaboat", "I_rented_a_boat.ags", "9c8cbf1ee6f0a797fdaee29177a43224", 140971300, "MAGS"),  // Linux
	GAME_ENTRY_EN("irentedaboat", "I_rented_a_boat.exe", "69059264cbe5973082ddeec0aead82a9", 121845045),  // Windows
	GAME_ENTRY_EN("irishcoffee", "Irish Coffee.exe", "6d012687b4944c2b9167dad450579527", 18381150),
	GAME_ENTRY("isaacodyssey", "isaacsoddysee.exe", "615e73fc1874e92d60a1996c2330ea36", 5339324),  //v1.1 Eng-Esp
	GAME_ENTRY_EN("iskraigra", "AlpaM.exe", "af1683f81365165be8ec727fe9e89300", 537920902),
	GAME_ENTRY_EN("iskraigra", "AlpaM.exe", "af1683f81365165be8ec727fe9e89300", 538423494),
	GAME_ENTRY_EN("isnkill", "ISN.exe", "4d17844029d8910fbaae1bdc99e250f2", 7932669),
	GAME_ENTRY_EN("isoproject", "ISO.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 2443024),
	GAME_ENTRY_EN("isos", "ISOS.exe", "06a03fe35791b0578068ab1873455463", 7362937),
	GAME_ENTRY_EN("ispy2", "I Spy II.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 2293873),
	GAME_ENTRY_EN("itsabugslife", "It's a Bugs Life.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 201172081),
	GAME_ENTRY_EN("itsjustarongame", "RONgames.exe", "97d700529f5cc826f230c27acf81adfd", 2171873),
	GAME_ENTRY_EN("iwalkedapath", "I walked a path.exe", "fd3ecd8289bebadbf775fe8a13b9c5d7", 89018261),
	GAME_ENTRY("iwantanidentity", "Quiero una identidad - Akkad Estudios.exe", "ffaf92297a381f2770f204122b4cc26a", 404765762),  // Eng-Esp
	GAME_ENTRY("iwantout", "I_want_out!.exe", "c2cb8ff1ad3028b08fd3dab91578b934", 22218677),  //v1.1.5  Multilang
	GAME_ENTRY("iwantout", "I_want_out!.ags", "2a31f07d3105c978d9b0392000f38461", 22718900),
	GAME_ENTRY("iwantout", "I_want_out!.ags", "7db3059844c14f1d817f68b958b66f45", 22718900),
	GAME_ENTRY("iwanttodie", "ElMito.exe", "f120690b506dd63cd7d1112ea6af2f77", 11304944),  // Eng-Esp
	GAME_ENTRY_LANG("iwanttodie", "QuieroMorir.exe", "f120690b506dd63cd7d1112ea6af2f77", 11304944, Common::ES_ESP),
	GAME_ENTRY_EN("iwanttodieremake", "IWTD.exe", "981e60a9be515bb56c634856462abbc7", 62882828),
	GAME_ENTRY_LANG("iwanttodieremake", "QM.exe", "981e60a9be515bb56c634856462abbc7", 62486182, Common::ES_ESP),
	GAME_ENTRY_PLATFORM("iwanttodieremake", "QM.exe", "981e60a9be515bb56c634856462abbc7", 62806610, "Galician"),
	GAME_ENTRY_EN("iwwhiiwwhitomirotpgthegame", "IWWHIIWWHITOMIROTPG - The Game.exe", "5fd79c32d2b8bbd589f6254d611d2742", 3576019),
	GAME_ENTRY_EN("jack", "JACK.exe", "615e73fc1874e92d60a1996c2330ea36", 243357628),
	GAME_ENTRY("jacktrasheaterch1", "carrin.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2376923),  //En-It
	GAME_ENTRY_EN("jacktrekker", "Jack Trekker.exe", "cb07ae51acec1830dd67c508b90f8951", 51900973),
	GAME_ENTRY_EN("jacktrekker", "Jack Trekker - Somewhere in Egypt.exe", "17c889c0228c401c3dbf6148d3767a14", 101912263),  // v1.0.1
	GAME_ENTRY_EN_PLATFORM("jacktrekker", "Jack Trekker.exe", "8cc8a212d8ffb923db0ba740074dfa69", 43817137, "OROW"),
	GAME_ENTRY_EN("jacob", "Jacob.exe", "90413e9ae57e222f8913b09d2bc847bc", 7101001),
	GAME_ENTRY_EN("jacobvacut", "JacobVAcut.exe", "06a03fe35791b0578068ab1873455463", 8017226),
	GAME_ENTRY_EN("jacquelinewhitereddesert", "Jacqueline White.exe", "02ab2c5f53f42d494491b7f411da8d47", 119383318),
	GAME_ENTRY_EN("jailhouse", "viki.exe", "465f972675db2da6040518221af5b0ba", 3769307),
	GAME_ENTRY_EN("jakelastjourney", "Cloudgame.exe", "29f768fefd498afbf3268621248dfa89", 8978633),
	GAME_ENTRY_EN("jamesbond", "platform2.exe", "949f7440e3692b7366c2029979dee9a0", 4467701),
	GAME_ENTRY("jamesinneverland", "JamesNeverland.exe", "06a03fe35791b0578068ab1873455463", 36488607),  // En-Fr
	GAME_ENTRY("jamesperis", "James1.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 85631684),  // Eng-Esp
	GAME_ENTRY_LANG("jamesperis", "James1.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 85605688, Common::ES_ESP),
	GAME_ENTRY_LANG("jardimfadas", "O Jardim das Fadas.exe", "0241777c2537fc5d077c05cde10bfa9f", 5462107, Common::PT_BRA),
	GAME_ENTRY_EN("jasongoldenapple", "Jason and the Golden Apple.exe", "7446ce302043dac2acc8cd5cc7cc8f68", 732192390),
	GAME_ENTRY_EN("javelincatch", "JvlnCtch.exe", "3421b46ff95c5885603086c39a038a20", 1626263),
	GAME_ENTRY_EN("jetpacksam", "Jetpack Sam.exe", "173f00e582eebd082d66f1291702b373", 12574372),
	GAME_ENTRY_LANG("jezioro", "jezioropingwinie02.exe", "6e861b1f476ff7cdf036082abb271329", 251069777, Common::PL_POL),
	GAME_ENTRY_EN("jimbobabducted", "talk.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 3055790),
	GAME_ENTRY_EN("jimmysday", "jimmy.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 13419394),
	GAME_ENTRY_EN("jimmythetroublemaker", "Jimmy The Troublemaker.exe", "25976a689b0f4d73eac69b1728377ecb", 200843118),
	GAME_ENTRY_EN("jimsonjazz", "jatjc.exe", "06a03fe35791b0578068ab1873455463", 3872478),
	GAME_ENTRY_EN("joeshorriblehell", "Joe's Horrible Hell.exe", "cdcc787023c51c6fc9ffc17118f6adff", 61061393),
	GAME_ENTRY_EN("joesmiserablelife", "Joe's Miserable Life.exe", "1b9f13d430bb15bf30d0fd044358db68", 36705304),
	GAME_ENTRY("johnharris", "totp.exe", "74dc062c5f68f3d70da911f2db5737b6", 36463878),  // Eng-Hun
	GAME_ENTRY("johnjebediahgun", "JJG.exe", "a28cb95e1769ba1bfa48f850390746c2", 88957123),  // En-It
	GAME_ENTRY_EN("johnlosthiskeyep1", "Newgame.exe", "0b7529a76f38283d6e850b8d56526fc1", 1453831),
	GAME_ENTRY_EN("johnlosthiskeyep2", "Johnny.exe", "0500aacb6c176d47ac0f8158f055db83", 1182438),
	GAME_ENTRY_EN("johnnyrocket", "Rocket.exe", "a524cbb1c51589903c4043b98917f1d9", 10366294),
	GAME_ENTRY_EN("johnsavedchristmas", "John's Xmas.exe", "465f972675db2da6040518221af5b0ba", 6757902),
	GAME_ENTRY_LANG("johnsinclair", "John Sinclair- Voodoo in London.exe", "68c4cdee67c40bf96cf78e11f9073ddd", 238169392, Common::DE_DEU),
	GAME_ENTRY_EN("jokequest", "JokeQuest.exe", "1110f562862d910262090e28413eeafb", 3972204),
	GAME_ENTRY_EN("jonahsplace", "Jonah's Place.exe", "615e73fc1874e92d60a1996c2330ea36", 28363651),
	GAME_ENTRY_EN("jonahsplace", "Jonah's Place.exe", "615e73fc1874e92d60a1996c2330ea36", 28368426),
	GAME_ENTRY_EN("jonathanadv", "Jonathan's Adventures.exe", "63f8a60cc094996bd070e68cb3c4722c", 20517227),
	GAME_ENTRY_EN("jonnyfeces", "Jonny Smallvalley.exe", "01d0e6bd812abaa307bcb10fc2193416", 34437869),
	GAME_ENTRY_EN("journey", "journey.exe", "be858e83311d4f1b49e05b933b92f0a9", 2617447),
	GAME_ENTRY_EN("journeyhell", "Cropcircles.exe", "e80586fdc2db32f65658b235d8cbc159", 211064206),
	GAME_ENTRY_EN("journeyhome", "Journey Home.exe", "20aa2fac9f64e876da8ed437f609bad6", 1982898758),  //v1.1 Win
	GAME_ENTRY_EN("journeyhome", "Journey Home.ags", "0c80d863920b7b2002764a11ab825fff", 1980416054),  //v1.1 Linux
	GAME_ENTRY_EN("jugglequest", "JuggleQuest.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 1934098),
	GAME_ENTRY_LANG("juliusdangerous1", "Julius Dangerous.exe", "ddaf3807f1fe16b2813ff832b4fb471a", 83137140, Common::IT_ITA),
	GAME_ENTRY_LANG("juliusdangerous1", "Julius Dangerous.exe", "79077a68e53562082494933a21e2714f", 83175251, Common::IT_ITA),  //v1.0.0.3
	GAME_ENTRY_LANG("juliusdangerous2", "Julius Dangerous 2.exe", "e2d4a98de69b1f8e6462c387710a441a", 146931732, Common::IT_ITA),  //v1.01
	GAME_ENTRY("juliusdangerous2", "Julius Dangerous 2.exe", "15ee13935b9555a041df569af8598bfe", 159441130),  //v1.2 Win En-It
	GAME_ENTRY("juliusdangerous2", "Julius Dangerous 2.ags", "2b76bb39e520e37a829430b70486e96f", 156408534),  //v1.2 Linux En-It
	GAME_ENTRY_EN("jumpinjack", "Jumpin' Jack.exe", "338fa79960d40689063af31c671b8729", 36458660),
	GAME_ENTRY_EN("jumpinjones", "ags.exe", "36f44e064eab15e502caeb60fd09f52d", 19633830),
	GAME_ENTRY_EN("jumpjackflash", "Jump! Jack! Flash!.exe", "6b4ceb9e327ac99479c08d825461f4cb", 20103698),
	GAME_ENTRY_EN("june20th", "June 20th.exe", "f18443f98fd61d2c655e76a17f7da905", 11198272),
	GAME_ENTRY_EN("justanotherpointnclickadventure", "Advent.exe", "97d700529f5cc826f230c27acf81adfd", 8655228),
	GAME_ENTRY_EN("justanotherpointnclickadventure", "Advent.exe", "6a98b4cc2f5a55421248be53f15a6a99", 9582620),
	GAME_ENTRY_EN("justyouandme", "it'swithyou.exe", "495d45fb8adfd49690ae3b97921feec6", 19545407),
	GAME_ENTRY_EN("kada", "Ka And Da.exe", "0564de07d3fd5c16e6947a647061913c", 5026160),
	GAME_ENTRY_EN("kada", "Ka And Da.ags", "6212a9f0bb12837b34e87942539a4fa7", 1993564),
	GAME_ENTRY_EN("kanjigakusei", "KanjiGakusei.exe", "c1bce0ccfa858f0f5d2fe19997d89b05", 17604764),
	GAME_ENTRY_EN_PLATFORM("kanjigakusei", "KanjiGakusei.exe", "2e9709faad87a4f8f757dff9a0cc4cfd", 3324339, "OROW"),
	GAME_ENTRY_EN("kartquest", "Kart.exe", "465f972675db2da6040518221af5b0ba", 77317312),
	GAME_ENTRY_EN("kata", "KATA.exe", "06a03fe35791b0578068ab1873455463", 4631650),
	GAME_ENTRY_LANG("katurachroniques", "Katura.exe", "a524cbb1c51589903c4043b98917f1d9", 10662078, Common::FR_FRA),
	GAME_ENTRY_LANG("katurachroniquesrpg", "JRCK.exe", "f120690b506dd63cd7d1112ea6af2f77", 6848670, Common::FR_FRA),
	GAME_ENTRY_LANG("katuracolonisation", "test.exe", "06a03fe35791b0578068ab1873455463", 18133783, Common::FR_FRA),
	GAME_ENTRY_LANG("katuralchimie", "KaturAlchimie.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 11981482, Common::FR_FRA),
	GAME_ENTRY_LANG("katurapuzzleadv", "Katura Puzzle Adventure.exe", "a524cbb1c51589903c4043b98917f1d9", 50333223, Common::FR_FRA),
	GAME_ENTRY_LANG("katurapuzzlearenas", "KaturaPuzzleArenas.exe", "615e73fc1874e92d60a1996c2330ea36", 30476487, Common::FR_FRA),
	GAME_ENTRY_LANG("katurarpg", "Katura RPG.exe", "01d0e6bd812abaa307bcb10fc2193416", 295327636, Common::FR_FRA),
	GAME_ENTRY_EN("keptosh1", "Keptosh.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 2904848),
	GAME_ENTRY_EN("keyboardmadness", "piano.exe", "f120690b506dd63cd7d1112ea6af2f77", 1767775),
	GAME_ENTRY("keysofagamespace", "Keys.exe", "75f4c7f66b1be60af5b2d65f617b91a7", 85582285),  // En-Fr
	GAME_ENTRY("keysofagamespace", "Keys.exe", "75f4c7f66b1be60af5b2d65f617b91a7", 85581872),  //v1.2 En-Fr-Por
	GAME_ENTRY_EN("kidnapperescape", "Kidnapper Escape.exe", "792c0a0eaeba1a8846cb7b1af1e3266e", 1249374512),
	GAME_ENTRY_EN("kidnapped", "kidnapped.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1626068),
	GAME_ENTRY_EN("kikme", "hour2.exe", "97d700529f5cc826f230c27acf81adfd", 706219),
	GAME_ENTRY_EN("killereye", "killereye.exe", "0710e2ec71042617f565c01824f0cf3c", 1009042),
	GAME_ENTRY("killjoseda", "KillJD.exe", "20dc02a8f977caf5c4dc6f2a4c8d4378", 6034414),  // Eng-Esp
	GAME_ENTRY_EN("killmenow", "Kill me now.exe", "b5c41e08919834b549ca350a28f18f87", 35675885),  // Windows
	GAME_ENTRY_EN("killmenow", "Kill me now.ags", "355985efd32c26c7b937e0b8039c46a4", 33414365),  // Linux
	GAME_ENTRY_EN("kingdomlegend1", "Kingdom Legends.exe", "06a03fe35791b0578068ab1873455463", 2254139),
	GAME_ENTRY_EN("kingdomlegend2", "KL2.exe", "f1a8c734c513909d130b3ae139b5f076", 305831701),
	GAME_ENTRY_EN("kingofrock", "ags_mi3.exe", "0500aacb6c176d47ac0f8158f055db83", 5142697),
	GAME_ENTRY_LANG("kingsleyroad", "Kingsley Road, 1980.exe", "981e60a9be515bb56c634856462abbc7", 62189532, Common::ES_ESP),
	GAME_ENTRY_EN("kingsquestfororgy", "KQ.exe", "97d700529f5cc826f230c27acf81adfd", 1574600),
	GAME_ENTRY_LANG("kirja", "Kirjasto.exe", "a524cbb1c51589903c4043b98917f1d9", 3865024, Common::FI_FIN),
	GAME_ENTRY_EN("kiselyova", "innah.exe", "0710e2ec71042617f565c01824f0cf3c", 1236053),
	GAME_ENTRY_EN("kittenadv", "Kitten.exe", "9973fbc73cce23867246d3a5e3c86d01", 5423637),
	GAME_ENTRY_EN("kittyquest", "multiverbtemplate.exe", "6e861b1f476ff7cdf036082abb271329", 78650122),
	GAME_ENTRY_LANG("klopoty", "Klopoty Mirmila.ags", "f9280154ec28290a563982ad0079cbca", 3594698, Common::PL_POL),  // Win/Lin 1.0.1
	GAME_ENTRY_EN("knightpursuit", "Knight.exe", "0710e2ec71042617f565c01824f0cf3c", 1613627),
	GAME_ENTRY_EN("knightquestforgoldenring", "KQuestGoldRing.exe", "f120690b506dd63cd7d1112ea6af2f77", 2582542),
	GAME_ENTRY_EN("knightsquest3", "KQ3TOM.exe", "0710e2ec71042617f565c01824f0cf3c", 8405513),  //v1.0
	GAME_ENTRY_EN("knightsquest3", "KQ3TOM.exe", "0710e2ec71042617f565c01824f0cf3c", 8408641),
	GAME_ENTRY_EN("knightsquest4", "MAGSQuest.exe", "0500aacb6c176d47ac0f8158f055db83", 4074151),
	GAME_ENTRY("knightsquire", "KnightsquireT.exe", "0710e2ec71042617f565c01824f0cf3c", 4617812),  // Eng-Cze
	GAME_ENTRY_EN("knobblycrook", "Knobbly Crook.exe", "d6269b2f4cc3da56077bb4e2df7984d5", 2035570004),
	GAME_ENTRY_EN("knobblycrookch1", "Knobbly Crook.exe", "9d2c7d86fe1316d08678b732e4241076", 326499357),
	GAME_ENTRY_EN("knorrig", "Knorrig the Gifted Troublemaker.exe", "776a62db4387dc68be92ef9933399fd5", 21504699),  //v1.1
	GAME_ENTRY_LANG("koddurova", "Kod Durova.exe", "615e73fc1874e92d60a1996c2330ea36", 498427333, Common::RU_RUS),
	GAME_ENTRY("koffeekrisis", "Koffee Krisis.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 2423111),  // En-De
	GAME_ENTRY_EN("kongbaghdad", "baghdad.exe", "b216ee957dd677023e02d900281a02d6", 2091750),
	GAME_ENTRY_EN("korinsmines", "Korin's Mines.exe", "465f972675db2da6040518221af5b0ba", 20055117),
	GAME_ENTRY_EN("koscheitheimmortal", "Koschei.exe", "465f972675db2da6040518221af5b0ba", 2771442),
	GAME_ENTRY_EN_PLATFORM("kq3plus", "KQ3plus.ags", "3b7cceb3e4bdb031dc5d8f290936e94b", 11727219, "Beta"),
	GAME_ENTRY_EN("kq4retold", "King's Quest 4.exe", "da6140c1e246d5f7641464bda80c45d5", 148103603),
	GAME_ENTRY_EN("kq4retold", "King's Quest 4.ags", "e9e8f833b86874526cb5e356d1e287eb", 145563043),  // v1.0.0 Linux
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.exe", "da6140c1e246d5f7641464bda80c45d5", 293710098),
	GAME_ENTRY_EN("kq4retold", "ac2game.dat", "da6140c1e246d5f7641464bda80c45d5", 293710098),
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.exe", "da6140c1e246d5f7641464bda80c45d5", 312045017), // v1.0.3 Win
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.ags", "9dbe9de794929a0c11ea3cbeb92c3934", 163941398), // v1.0.3 Linux
	GAME_ENTRY_EN("kq4retold", "ac2game.dat", "da6140c1e246d5f7641464bda80c45d5", 312045017), // v1.0.3 Mac
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.exe", "da6140c1e246d5f7641464bda80c45d5", 312046797), // v1.0.4 Win
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.ags", "f472e13626996771d94066ab5661924d", 163943178), // v1.0.4 Linux
	GAME_ENTRY_EN("kq4retold", "ac2game.dat", "da6140c1e246d5f7641464bda80c45d5", 312046797), // v1.0.4 Mac
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.exe", "da6140c1e246d5f7641464bda80c45d5", 312165417), // v1.0.5 Win
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.exe", "da6140c1e246d5f7641464bda80c45d5", 166602358), // v1.0.5 Win (fixed)
	GAME_ENTRY_EN("kq4retold", "KQ4Retold.ags", "fdf0ce483760fca13a4279f161b965da", 164061798), // v1.0.5 Linux
	GAME_ENTRY_EN("kq4retold", "game.ags", "da6140c1e246d5f7641464bda80c45d5", 166602358), // v1.0.5 Mac
	GAME_ENTRY("kristmaskrisis", "Kristmas Krisis.exe", "90413e9ae57e222f8913b09d2bc847bc", 2067681),  // En-De
	GAME_ENTRY_LANG("ksauna", "Karjalan Sauna.exe", "663bbdd33432ee73be23f3c97d10053e", 14485962, Common::FI_FIN),
	GAME_ENTRY_EN("ktx1", "KTX-1.exe", "615e73fc1874e92d60a1996c2330ea36", 13806290),
	GAME_ENTRY_EN("kubik", "Kubik.exe", "63f8a60cc094996bd070e68cb3c4722c", 392213920),
	GAME_ENTRY_EN("kumastory", "Kuma Story.exe", "5d82ab003f108bf44999421884d69f55", 5621765),
	GAME_ENTRY_EN("kynigos", "Kynigos.exe", "1b9f13d430bb15bf30d0fd044358db68", 292773438),
	GAME_ENTRY_LANG("laboite", "LaBoite.exe", "4c4cb72d17cf1d8d0d3907f869ea0609", 142354237, Common::FR_FRA),
	GAME_ENTRY_EN("labratescape", "Joel.exe", "06a03fe35791b0578068ab1873455463", 3256136),
	GAME_ENTRY_EN("labratmaze", "LabRatMaze.exe", "99b6964357e805b3d0fd1a3d447593b0", 26746990),  //v1.02
	GAME_ENTRY_EN("labyrinth", "labyrnth.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2259514),  //v1.0
	GAME_ENTRY_EN("labyrinth", "labyrnth.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2358666),  //v2.0
	GAME_ENTRY_EN("labyrinth", "labyrnth.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2595160),  //v2.0u4
	GAME_ENTRY_EN("labyrinth", "labyrnth.exe", "0710e2ec71042617f565c01824f0cf3c", 7686481),  //v3.0 final
	GAME_ENTRY("lacarbonara", "La Carbonara.ags", "086024ea12068a5787d9562b266d2159", 2412912),  //v0.4 Win/Linux Eng-Esp
	GAME_ENTRY_LANG("lacicuta", "Cicuta1.exe", "1290e191d4f24893ce83ba0af5debd9e", 8665956, Common::ES_ESP),
	GAME_ENTRY_EN("lacroixpan", "La Croix Pan.exe", "c91bf675d6839c42924cf3c9a390ded3", 12411833), //v1.00
	GAME_ENTRY_EN("lacroixpan", "La Croix Pan.exe", "c91bf675d6839c42924cf3c9a390ded3", 12445773), //v1.06
	GAME_ENTRY("lacroixpan", "La Croix Pan.exe", "d9018b1792f6d959d7add4dc3f7cdb46", 12102643),  //v1.1 En-Ru-Hun
	GAME_ENTRY_LANG("lagrancastanya", "PROJECT1.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 39298012, Common::CA_ESP),
	GAME_ENTRY_LANG("lagrandeplaine", "La Grande Plaine.exe", "7a3096ac0237cb6aa8e1718e28caf039", 63873569, Common::FR_FRA),
	GAME_ENTRY_LANG("lallaveyfabianshones", "LLAVE.exe", "a8d73d686b2eb77caab8c05e3e1f1d57", 3374527, Common::ES_ESP),
	GAME_ENTRY("lamaleta", "La Maleta.ags", "144566549a27ad1b41346627ec82f586", 43741777),  // Eng-Esp
	GAME_ENTRY_EN("lambslaughter", "1hour.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 724154),
	GAME_ENTRY_EN("lancethepenguin", "LanceThePenguin.exe", "a524cbb1c51589903c4043b98917f1d9", 3838581),
	GAME_ENTRY_LANG("laodiseadelfracaso2", "La Odisea II.exe", "0710e2ec71042617f565c01824f0cf3c", 3083622, Common::ES_ESP),
	GAME_ENTRY_EN_PLATFORM("larrylotter", "Warthogs.exe", "06a03fe35791b0578068ab1873455463", 3387147, "MAGS"),
	GAME_ENTRY_EN_PLATFORM("larrylotter", "Warthogs.exe", "06a03fe35791b0578068ab1873455463", 3481314, "MAGS"),
	GAME_ENTRY("lasol", "LaSol.exe", "615e73fc1874e92d60a1996c2330ea36", 19677458),  // Multilang
	GAME_ENTRY_EN("lastbus", "The Last Bus.exe", "9175e8b91aa4efdfe685246d05fdd8bc", 23310911),
	GAME_ENTRY_EN("lastclown", "ZOM.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 867344),
	GAME_ENTRY_EN("lasthope", "Ludum Dare 46.ags", "37b6847ce6000a0a51391e8c1013bf2b", 25212295),  // Linux
	GAME_ENTRY_EN("lasthope", "Ludum Dare 46.exe", "8aff96231ca059cd61fe5ce68b83f50f", 27699095),  // Windows
	GAME_ENTRY_EN("lastlightpost", "LastLightPost.exe", "981e60a9be515bb56c634856462abbc7", 65705119),
	GAME_ENTRY_EN("lastnfurious", "Last'n'Furious.exe", "3569271305cddb7156260cce9439e543", 17696093),
	GAME_ENTRY_EN("lastnfurious", "Last'n'Furious.exe", "3569271305cddb7156260cce9439e543", 17697035),  //v1.0.4
	GAME_ENTRY_LANG("lastnovax", "THE LAST NO VAX.exe", "63f8a60cc094996bd070e68cb3c4722c", 26323807, Common::IT_ITA),
	GAME_ENTRY("lastone", "The Last One.exe", "7f057a2af1080632b4946d67382fe349", 17872546),  // Eng-Esp
	GAME_ENTRY("lastone", "The Last One.exe", "7f057a2af1080632b4946d67382fe349", 17876771),  // Eng-Esp
	GAME_ENTRY_EN("lastorder", "BlueAngel_8.exe", "a524cbb1c51589903c4043b98917f1d9", 38770175),
	GAME_ENTRY_LANG("latarta1", "BAGADV.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 10344924, Common::ES_ESP),
	GAME_ENTRY_LANG("latarta2", "BAGADV2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 311509933, Common::ES_ESP),
	GAME_ENTRY_LANG("latarta3", "BAGADVIII.exe", "615e73fc1874e92d60a1996c2330ea36", 103114105, Common::ES_ESP),
	GAME_ENTRY_EN("latelastnite", "late_last_nite v1.2.exe", "c12e28ec3233b26320026cfea4015e6b", 291463873),  //v1.2
	GAME_ENTRY_EN_PLATFORM("latelastnite", "late-last-nite.exe", "46aa841ae28db17994aa1045736627ed", 341530019, "Speakeasy Edition"),
	GAME_ENTRY_EN("laundryday", "Laundry Day.exe", "06a03fe35791b0578068ab1873455463", 2456888),
	GAME_ENTRY_EN("lavablava", "Rumble.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2294674),
	GAME_ENTRY_EN("lazaruswantspants", "pants.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1311852),
	GAME_ENTRY_EN("legacyicecream", "Cold.exe", "82da2565c456dcfb265ded6fe3189c0b", 9033418),
	GAME_ENTRY_LANG("lechuckstories", "CHUCK_272.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6274861, Common::DE_DEU),
	GAME_ENTRY_LANG("legendofrovendale", "Vampire.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 10018497, Common::RU_RUS),
	GAME_ENTRY_EN("legendofseththebard", "LOSTB 1.48.exe", "a524cbb1c51589903c4043b98917f1d9", 10669482),  //v1.48
	GAME_ENTRY_EN("legendofseththebard", "LOSTB 1.50.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 10670757),  //v1.50
	GAME_ENTRY_EN("legendofskystones", "Ludum Dare 45.ags", "38cfb1d9b153efbd94528f3c711ca5a0", 8869946),  // Linux
	GAME_ENTRY_EN("legendofskystones", "Ludum Dare 45.exe", "8aff96231ca059cd61fe5ce68b83f50f", 11356746),  // Windows
	GAME_ENTRY_LANG("legendsofmardaram", "LOM.exe", "0710e2ec71042617f565c01824f0cf3c", 50183544, Common::DE_DEU),
	GAME_ENTRY_EN("leisuresuitlarry2", "Larry 2.exe", "949f7440e3692b7366c2029979dee9a0", 11971760),
	GAME_ENTRY_EN("lelac", "Le Lac - EN.exe", "4eed763ae52a4acd9c231f09ece7b914", 188867790),
	GAME_ENTRY_LANG("lelac", "Le Lac.exe", "389f0ba58d77f1c203dc4c68b80dad44", 95040881, Common::FR_FRA),
	GAME_ENTRY("lemasabachthani", "Lema Sabachthani.exe", "c723c7872e39e4bceec1d5a2936e0172", 20641847),  // En-Fr-Es
	GAME_ENTRY_EN("leogravedigger", "graveyarder.exe", "06a03fe35791b0578068ab1873455463", 3262844),
	GAME_ENTRY_EN("leopoldkettle", "The Adventures of Leopold Kettle.exe", "92ddda7d8b443e02c39b2e28d24073b4", 10316570),
	GAME_ENTRY_LANG("lesangimmortels", "le sang des immortels.ags", "4d42dc8a737a6b4f83eda7b3d7bb6955", 35135485, Common::FR_FRA),
	GAME_ENTRY_EN("lesmiserables", "Les Miserables.exe", "0514661a4ba6772cf0db0cf839fc7003", 127582664),
	GAME_ENTRY_EN("lessthanthree", "Less Than Three.exe", "3e62eafed0fc365821b7afdbf1aec3d8", 6505133),
	GAME_ENTRY_EN("lessthanthree", "Less Than Three.exe", "3e62eafed0fc365821b7afdbf1aec3d8", 6506759),
	GAME_ENTRY_EN("letscook", "Let's Cook School of cooking.exe", "338fa79960d40689063af31c671b8729", 96730408),
	GAME_ENTRY_LANG("letteraamorosa", "Lettera amorosa.exe", "6348c3082ff79a33e408754dacb370c2", 48349480, Common::FR_FRA),
	GAME_ENTRY_LANG("libremotus", "LibreMotus.exe", "f3253e4db063395ce74e7deaa966c8ac", 4134484, Common::FR_FRA),
	GAME_ENTRY_EN("lichdom", "Lichdom.exe", "0e6ae2b49c1a38363a77024f3ce31771", 7412719),
	GAME_ENTRY_EN("life", "Life.exe", "97a2a2feb82708da8f6ed30c15a7eb53", 43833851),
	GAME_ENTRY_EN("lifeboatstoryofcedrick", "lifeboat.exe", "3128b9f90e2f954ba704414ae854d10b", 2950953),
	GAME_ENTRY_EN("lifeboatstoryofcedrick", "lifeboat.exe", "3128b9f90e2f954ba704414ae854d10b", 2952689),
	GAME_ENTRY_EN_PLATFORM("lifeboatstoryofcedrick", "lifeboat.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3563681, "Director's Cut"),
	GAME_ENTRY_EN("lifeinabox", "box.exe", "0500aacb6c176d47ac0f8158f055db83", 890794),
	GAME_ENTRY_EN("lifeofdduck", "D Duck.exe", "0710e2ec71042617f565c01824f0cf3c", 49461515),  // v0.95
	GAME_ENTRY_EN("lifeofdduck", "D Duck.exe", "0710e2ec71042617f565c01824f0cf3c", 49461615),  // v1.0
	GAME_ENTRY_EN("lifeofdduck2", "D. Duck II.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 135923689),
	GAME_ENTRY_EN("lifeworthlosing", "lwl.exe", "a524cbb1c51589903c4043b98917f1d9", 89591505),
	GAME_ENTRY_EN("liftreasureofthetanones", "Lif.exe", "18b284c22010850f79bc5c20054a70c4", 3946641),
	GAME_ENTRY_EN("lightcycles", "LightCycles.exe", "495d45fb8adfd49690ae3b97921feec6", 3415108),
	GAME_ENTRY_EN("lightningmaster", "Master.exe", "27343924ddad3be0b97bdcaa71858b1b", 231301393),
	GAME_ENTRY_EN("likeadream", "Dream.exe", "ad1801105cfae0915313c73ca7fb7f1c", 5704211),
	GAME_ENTRY_EN("likeafox", "likeafox.exe", "c7916b82f00c94013a3f7706b4d333c6", 2707737),
	GAME_ENTRY_EN("lillywizard", "Lilly.exe", "7c0c89edf8e321fad4191ee1d7fcaaae", 22269463),
	GAME_ENTRY_EN("limeylizardwastewizard", "LLWW.exe", "fab982fd31570655ac3b1858bd8265e4", 33755872),
	GAME_ENTRY_EN("limeylizardwastewizard", "LLWW.exe", "1991fde090b8c3b7cf53561d11593836", 38188301),
	GAME_ENTRY_EN("linegame", "a_noble_game.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3580379),
	GAME_ENTRY_EN("linkattus", "DaringCommandoRaid.exe", "78dd4ca028ee0156b6a093d6d780aa65", 71449594),  // Windows
	GAME_ENTRY_EN("linkattus", "DaringCommandoRaid.ags", "4d18c5e9affca7547a191a73223b70f8", 68964842),  // Linux
	GAME_ENTRY_EN("linnprotector", "Linn the Protector.exe", "3f5853e83facd0854a1a58c10ef86f1c", 19753875),
	GAME_ENTRY_EN("lionsden", "The Lion's Den.exe", "3aba76b46b4493c5541a2a17e67fdd93", 1999617),
	GAME_ENTRY_EN("littlegirlinunderland", "Underland.exe", "06a03fe35791b0578068ab1873455463", 10125940),
	GAME_ENTRY_EN("littleleonardo", "Little Leonardo.exe", "b3510ded3dccac859e386ce206b167fc", 171287601),  // Windows
	GAME_ENTRY_EN("littleleonardo", "Little Leonardo.ags", "d5fceb85f63338197a8c54ad7911145a", 168239133),  // Linux
	GAME_ENTRY_EN("livingnightmare", "Living Nightmare.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 3356797),
	GAME_ENTRY_EN_PLATFORM("livingnightmare", "Living Nightmare.exe", "4415d633ea1a2dcd03ff0eff43f182ee", 9997554, "Deluxe"),
	GAME_ENTRY_EN_PLATFORM("livingnightmareendlessdream", "LNED.exe", "4415d633ea1a2dcd03ff0eff43f182ee", 10671309, "MAGS"),
	GAME_ENTRY_EN("livingnightmareendlessdream", "LNED.exe", "4415d633ea1a2dcd03ff0eff43f182ee", 14438626),
	GAME_ENTRY_EN("livingnightmarefreedom", "LNF.exe", "6aa30185326552359c7865e55c045a74", 26580184),
	GAME_ENTRY_EN("loathesome", "The Loathesome Man.exe", "f120690b506dd63cd7d1112ea6af2f77", 2640210),
	GAME_ENTRY_EN("lockedin", "locked.exe", "0710e2ec71042617f565c01824f0cf3c", 1197367),
	GAME_ENTRY_EN("lockedout", "Clevedon.exe", "465f972675db2da6040518221af5b0ba", 1658713),
	GAME_ENTRY_EN("lockedout", "locked-out.exe", "465f972675db2da6040518221af5b0ba", 1930862),
	GAME_ENTRY_EN("lockeescape", "Brett 3.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3525589),
	GAME_ENTRY_EN("loftusandtheskycap", "loftus.exe", "4c83816b87e6e253dc8e324e89bcbca3", 24333349),
	GAME_ENTRY_EN("lonecase1", "LoneCase.exe", "8661936f40669fa1672dced4df0521a0", 3550717),  // v2.0
	GAME_ENTRY_EN("lonecase2", "Lone Case 2.exe", "cb3bbab6e565fcbd57af8f33a8609210", 6015178),
	GAME_ENTRY_EN("lonecase3", "lc 3.exe", "893df6568c294926f7efa3f3ec2ce14d", 7997004),  // v4.0
	GAME_ENTRY_EN("lonecase4", "Lone Case 4.exe", "8a27ae1c8ac0bc1ea78524fdf847c15e", 4439756),
	GAME_ENTRY_EN("lonelynight", "Lonely night.exe", "0710e2ec71042617f565c01824f0cf3c", 2488077),
	GAME_ENTRY_EN("longdistancecoughing", "Long Distance Coughing.exe", "0564de07d3fd5c16e6947a647061913c", 1319722225),  // Win v1.0.3
	GAME_ENTRY_EN("longdistancecoughing", "ac2game.dat", "0564de07d3fd5c16e6947a647061913c", 1319722225),  // Mac
	GAME_ENTRY_EN("longdistancecoughing", "Long Distance Coughing.ags", "c63e4b6fba32f7e56d62f05a5df0c965", 1316689629),  // Linux
	GAME_ENTRY_EN("longestwinter", "Groundhog.exe", "615e73fc1874e92d60a1996c2330ea36", 4954940),
	GAME_ENTRY_EN("longtooth", "Long in the Tooth.exe", "b2eb107c1f9784b7fc4f58c2a7678dea", 184375380),
	GAME_ENTRY("lonkeyisland", "lonkey.exe", "4e92c6db88a34b62eef37a621aabfb53", 44134488),  // En-De
	GAME_ENTRY("lonkeyisland", "lonkey.exe", "4e92c6db88a34b62eef37a621aabfb53", 44114957),  // En-De-It
	GAME_ENTRY_EN("lookatmech1", "look at me. look at my face. does it look like i care about school.exe", "7a3096ac0237cb6aa8e1718e28caf039", 53182290),
	GAME_ENTRY("lookingfordread", "Looking For Dread Mac Farlane.exe", "2321081ba51d1869ebe11b9ba126911b", 417051695),  // Windows  En-Fr
	GAME_ENTRY("lookingfordread", "Looking For Dread Mac Farlane.ags", "bde1905a3c6a6f7f4491378789512a68", 414566943),  // Linux  En-Fr
	GAME_ENTRY_LANG("lookingfordread", "A la recherche de Dread Mac Farlane.exe", "2321081ba51d1869ebe11b9ba126911b", 417052014, Common::FR_FRA),  // Windows
	GAME_ENTRY_LANG("lookingfordread", "A la recherche de Dread Mac Farlane.ags", "4d232dca7faf065d24f554b7be03740d", 414567262, Common::FR_FRA),  // Linux
	GAME_ENTRY_EN("lordbytes1ep1", "epis1.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1359257),
	GAME_ENTRY_EN("lordbytes1ep2", "epis2.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1483774),
	GAME_ENTRY_EN("lordbytes1ep3", "epis3.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1476971),
	GAME_ENTRY_EN("lordbytes1ep4", "epis4.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1077383),
	GAME_ENTRY_EN("lordbytes1ep5", "epis5.exe", "9cf51833e787cc919837d9a8bd8fc14c", 895362),
	GAME_ENTRY_EN("lordbytes1ep6", "epis6.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1132960),
	GAME_ENTRY_EN("lordbytes1ep7", "epis7.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1149521),
	GAME_ENTRY_EN("lordbytes1ep8", "epis8.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1158833),
	GAME_ENTRY_EN("lordbytes1ep9", "epis9.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1306295),
	GAME_ENTRY_EN("lordbytes1ep10", "epis10.exe", "9cf51833e787cc919837d9a8bd8fc14c", 2144847),
	GAME_ENTRY_EN("lordbytes1ep11", "epis11.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1517916),
	GAME_ENTRY_EN("lordbytes1ep12", "epis12.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1943409),
	GAME_ENTRY_EN("lordbytes1ep13", "epis13.exe", "9cf51833e787cc919837d9a8bd8fc14c", 2052688),
	GAME_ENTRY_EN("lordoflight", "LordofLight.exe", "0564de07d3fd5c16e6947a647061913c", 98630583),
	GAME_ENTRY_EN("lordoflight", "LordofLight.ags", "020597575b9a190f6d770c3d4b9bb180", 95597987),
	GAME_ENTRY_EN("lorrylen", "Lorry Len.exe", "f120690b506dd63cd7d1112ea6af2f77", 1252505),
	GAME_ENTRY_EN("losjovenesdelaguerra", "guerra.exe", "97d700529f5cc826f230c27acf81adfd", 4286279),
	GAME_ENTRY_EN("losjovenesdelaguerra", "guerra.exe", "97d700529f5cc826f230c27acf81adfd", 4286035),
	GAME_ENTRY("losno", "Ladies of Sorrow Night One.ags", "f2b7c42a39683cbae37be67a562fe36a", 175792352), // Windows, Linux Multi
	GAME_ENTRY("losno", "Ladies of Sorrow Night One.ags", "c472fb6ec3379d0af51076e99934ceb2", 178529328), // Windows, Linux Multi v1.1b
	GAME_ENTRY("lostanswers", "Lost Answers.exe", "c88de182eae58fdb43e5b4e587095ff5", 254684884),  // Windows  Eng-Tur
	GAME_ENTRY("lostanswers", "Lost Answers.ags", "ee0ec2b06f61dd4cb5e57fe5d7898aa0", 252104900),  // Linux  Eng-Tur
	GAME_ENTRY_EN("lostdollar", "Ron.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 4542929),
	GAME_ENTRY_EN("lostfound", "Lost and Found.exe", "fae163b58c16e194688727d0903684be", 714253827),
	GAME_ENTRY_EN("lostfound", "Lost and Found.exe", "af1683f81365165be8ec727fe9e89300", 701455243),  // v1.0.7
	GAME_ENTRY_EN("lostinthenightmare", "litn.exe", "0710e2ec71042617f565c01824f0cf3c", 5492994),  //v1.0
	GAME_ENTRY_EN("lostinthenightmare", "litn.exe", "0710e2ec71042617f565c01824f0cf3c", 5493239),  //v1.1
	GAME_ENTRY_EN_PLATFORM("lostinthenightmare", "litn.exe", "06a03fe35791b0578068ab1873455463", 7973076, "Deluxe"),  // Deluxe/SoS
	GAME_ENTRY_EN("lostinthewoods", "LITW.exe", "00328f4f1e7729144483107b96b11df8", 55203461),
	GAME_ENTRY_PLATFORM("lotto", "Lott.exe", "0564de07d3fd5c16e6947a647061913c", 6585796, "Icelandic"),
	GAME_ENTRY_PLATFORM("lotto", "Lott.ags", "1578011383e302e787d3ff906776483f", 3553200, "Icelandic"),
	GAME_ENTRY_EN("lowequest", "lowe.exe", "97d700529f5cc826f230c27acf81adfd", 1046425),
	GAME_ENTRY("lucasmaniac", "LucasManiac !.exe", "f120690b506dd63cd7d1112ea6af2f77", 28361487),  // En-Fr
	GAME_ENTRY_EN("lucidlucy", "LUCID LUCY.exe", "655363c390c7ae7225c237108edf50b7", 182038828),
	GAME_ENTRY_EN("lucylavender", "Lucy.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 7944054),
	GAME_ENTRY_EN("lukesexistentialnightmare", "Lukeexit.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 653834),
	GAME_ENTRY_EN("lunarlander", "LunarLanderPrototype.exe", "495d45fb8adfd49690ae3b97921feec6", 3510282),  // Windows
	GAME_ENTRY_EN("lunarlander", "LunarLanderPrototype.ags", "0600d67c93fb0a1b07e9fdf716d9f020", 1029626),  // Linux
	GAME_ENTRY_EN("lutherinhood", "afroTest4.exe", "2ca80bd50763378b72cd1e1cf25afac3", 26440699),
	GAME_ENTRY_EN("lydia", "Kill The Lights.exe", "9b54ea3557373188d8388ec4d1ae5eed", 20416736),
	GAME_ENTRY_EN("lydianellreno", "LydiaW.exe", "615e73fc1874e92d60a1996c2330ea36", 33971307),
	GAME_ENTRY_EN("lygophilous", "Lygophilous.exe", "0564de07d3fd5c16e6947a647061913c", 38762137),
	GAME_ENTRY_EN("machinesdreams", "Machines Have Lucid Dreams v0.91.exe", "784c7fae35c6501eaa3673ea29391a7b", 653366138),
	GAME_ENTRY_EN_PLATFORM("madjack", "MadJackBrakeShake.ags", "25e4e7303bce8c521dcfc8056afeaa6e", 26312912, "MAGS"),
	GAME_ENTRY_EN("madjack", "MadJackBrakeShake.ags", "56bf59ff32c9196fe86f9777885c8889", 26335215),  // Win/Linux v1.1
	GAME_ENTRY_EN("mafaldawest", "realgame.exe", "be559b7205d41c3f3813cb8a7e8196f6", 16292195),
	GAME_ENTRY_EN("mafiosooverkill", "Alien Shoot.exe", "0500aacb6c176d47ac0f8158f055db83", 4012551),
	GAME_ENTRY_EN("magentaspacech1", "Magenta.exe", "0595b55383d72a99cd995092c5bf8475", 43085389),
	GAME_ENTRY_EN("magic8ball", "Magic-8-Ball.exe", "82da2565c456dcfb265ded6fe3189c0b", 4233735),
	GAME_ENTRY_EN("magicalwhatevergirl", "mwg.exe", "b2b99b5b3dcaee0fa292343c5a2c429b", 7784104),
	GAME_ENTRY_EN("magicballoffortune", "Magic Ball.exe", "f120690b506dd63cd7d1112ea6af2f77", 6016698),
	GAME_ENTRY_EN("magicowl", "owl.exe", "5a4de988ab7c7a5265f263bcfd4db5af", 87406423),
	GAME_ENTRY_LANG("magnicidio", "Magnicidio.exe", "06a03fe35791b0578068ab1873455463", 30270079, Common::ES_ESP),  // v1.2.7
	GAME_ENTRY_LANG("magnumpm", "Magnum, P.M.exe", "06a03fe35791b0578068ab1873455463", 91711892, Common::FR_FRA),
	GAME_ENTRY_EN_PLATFORM("magsic", "Magsic.exe", "28f82e420b82d07651b68114f90223c8", 21543960, "MAGS"),
	GAME_ENTRY_EN("magsic", "Magsic.exe", "28f82e420b82d07651b68114f90223c8", 21544930),  //v1.01
	GAME_ENTRY_EN("magsic", "Magsic.exe", "7a436b9a2e8e80c8aaaeed09ef9e3d3d", 22360144),  //v1.03
	GAME_ENTRY_EN("magsic2", "MagsicII.exe", "28f82e420b82d07651b68114f90223c8", 30027057),
	GAME_ENTRY_EN("magsic2", "MagsicII.exe", "28f82e420b82d07651b68114f90223c8", 30029761),  // v1.02
	GAME_ENTRY_EN("majixs", "majixsis.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 1667746),
	GAME_ENTRY_EN("majorbummerdude", "visyron.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3104750),
	GAME_ENTRY_EN("mammamia", "Mamma Mia.exe", "675b7258d741fe781bed98691ce3c5d2", 8973139),  //v1.04
	GAME_ENTRY_EN_PLATFORM("manamatch", "manamatch.exe", "792c0a0eaeba1a8846cb7b1af1e3266e", 6718448, "Alpha"),
	GAME_ENTRY_EN_PLATFORM("manamatch", "manamatch.ags", "480e164f10264f197216b5abe409d526", 3681756, "Alpha"),
	GAME_ENTRY_EN("manboy", "Man Boy.exe", "1275885401b7d2ece491e704535707d9", 3038532),
	GAME_ENTRY_EN_PLATFORM("manboy", "Man Boy.exe", "1275885401b7d2ece491e704535707d9", 2932301, "OROW"),
	GAME_ENTRY_EN("mangivingup", "ManGivingUp.exe", "1a56f15e76e2650c38ef680cb0f6ad3f", 4218825),
	GAME_ENTRY_EN("maniacapartment", "Maniac apartmen.exe", "465f972675db2da6040518221af5b0ba", 4806623),
	GAME_ENTRY("maniacapartment", "ManiacApt.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 6844931),  // v1.1 En-Es-It
	GAME_ENTRY_EN("maniacland", "AliceInManiacland.exe", "74203c62b4604c4463f1c53db2d47c01", 6570499),  // Windows
	GAME_ENTRY_EN("maniacland", "AliceInManiacland.ags", "00596e8a9f0bd774528fb8c7225bed0d", 4082163),  // Linux
	GAME_ENTRY("maniacland", "AliceInManiacland.exe", "d37a4f06126fc1f3bb7e5c31bd58a014", 8015351),  // Windows (newer) En-Es-It
	GAME_ENTRY("maniacmansiondeluxe", "Maniac.exe", "3128b9f90e2f954ba704414ae854d10b", 9395050),  // v1.05 Multi
	GAME_ENTRY("maniacmansiondeluxe", "clk:mmdsetup.exe:Maniac.exe", "A:3128b9f90e2f954ba704414ae854d10b", 9395050),  // v1.05 Multi
	GAME_ENTRY("maniacmansiondeluxe", "clk:manicmdsetup.exe:Maniac.exe", "A:3128b9f90e2f954ba704414ae854d10b", 9395050),  // v1.05 Multi
	GAME_ENTRY("maniacmansiondeluxe", "Maniac.exe", "465f972675db2da6040518221af5b0ba", 10181366), // v1.3  "
	GAME_ENTRY("maniacmansiondeluxe", "Maniac.exe", "465f972675db2da6040518221af5b0ba", 10409172), // v1.4  "
	GAME_ENTRY("maniacmansiondeluxe", "clk:Maniac-Mansion-Deluxe_Win_EN-FR-ES-DE-IT.exe:Maniac.exe", "A:465f972675db2da6040518221af5b0ba", 10409172),  // v1.4
	GAME_ENTRY("maniacmetalheadmania", "Maniac Metalhead Mania.exe", "d4dbb53d3617dcbb56251eb4a332fddd", 11785951),  //En-De
	GAME_ENTRY("maniacmetalheadmania2", "Maniac Metalhead Mania II.exe", "be3275347f23aadb6d13aa75f70fcb99", 14997025),  //En-De
	GAME_ENTRY_EN("manvsfish", "ManVsFish.exe", "06a03fe35791b0578068ab1873455463", 1875086),
	GAME_ENTRY_EN("mardsrevenge", "MPLR.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1771793),
	GAME_ENTRY_LANG("martyausdemall", "test3.exe", "0710e2ec71042617f565c01824f0cf3c", 4352013, Common::DE_DEU),
	GAME_ENTRY_EN("martyroftime", "A Martyr Of Time.exe", "6b1702aec6d0a44bda59ef8f229956c5", 44733333),
	GAME_ENTRY_EN("martyroftime", "A Martyr Of Time.ags", "705622c103d602d7ebbd1a2949689aac", 42247557),
	GAME_ENTRY_EN("mash", "MASH.ags", "8fe67d5a066d4ab203db0cce102ed86c", 12536184),
	GAME_ENTRY_EN("mash", "MASH.exe", "7825fa980fcd058caadaf6d3377b1f78", 15571852),
	GAME_ENTRY_EN("masked", "masked.exe", "3d836dbfb0fc5afa9abe040f00888e20", 5496989),
	GAME_ENTRY_EN("masquerade", "Masquerade at the con.exe", "8f18ad4456e6dbeaf041cd91449a55ba", 4782146),  // Windows
	GAME_ENTRY_EN("masquerade", "Masquerade at the con.ags", "77a8f90c1db63819246defe058aeea89", 1745966),  // Linux
	GAME_ENTRY("mastersofsound", "MOS.exe", "3128b9f90e2f954ba704414ae854d10b", 17098098),  //v1.1  En-De
	GAME_ENTRY_EN("mastersofsound", "MOS.exe", "3128b9f90e2f954ba704414ae854d10b", 17098794),	//v1.2
	GAME_ENTRY("mastersofsound", "MOS.exe", "bb59de174d70797d774dec76a171352d", 20298343),	//v1.2	En-De
	GAME_ENTRY("mastersofsound", "MOS.exe", "bb59de174d70797d774dec76a171352d", 20300650),  //v1.3  En-De
	GAME_ENTRY_EN("matildacurse", "KingStephen.exe", "02ab2c5f53f42d494491b7f411da8d47", 17116555),
	GAME_ENTRY_EN("maverickgunn", "Maverick Gunn.exe", "2bc8f994a7d1e05ed45f35abf2128231", 19431941),
	GAME_ENTRY_EN("maxandmaggie", "Max.exe", "465f972675db2da6040518221af5b0ba", 5759981),
	GAME_ENTRY_EN("maxparade", "MP.exe", "0b7529a76f38283d6e850b8d56526fc1", 2070936),
	GAME_ENTRY_EN("mayak", "mayak 01.exe", "0710e2ec71042617f565c01824f0cf3c", 6151460),
	GAME_ENTRY("mechanismo", "mechanismo.exe", "1adc27370e1f40686a2f3e19c70e5f6f", 135011026),  //v1.1 Eng-Rus
	GAME_ENTRY_EN("medicaltheoriesofdrkur", "dfg.exe", "465f972675db2da6040518221af5b0ba", 3140527),
	GAME_ENTRY_EN("megacorp", "Megacorps2K20.exe", "7971a7c02d414dc8cb33b6ec36080b91", 31837387),
	GAME_ENTRY_EN("megacorp", "Megacorps2K20.ags", "465f2a582197f6aa8091f0db3fb769a8", 29340347),
	GAME_ENTRY_EN("megocannibaljungle", "mgcj.exe", "06a03fe35791b0578068ab1873455463", 7127871),
	GAME_ENTRY_EN("megostore", "Me Go Store.exe", "90413e9ae57e222f8913b09d2bc847bc", 2961962),
	GAME_ENTRY_EN("megostore2", "Me Go Store II.exe", "06a03fe35791b0578068ab1873455463", 6640781),  //MAGS
	GAME_ENTRY_EN("megostore2", "Me Go Store II.exe", "615e73fc1874e92d60a1996c2330ea36", 6942985),
	GAME_ENTRY_EN("megostore3", "Me Go Store III.exe", "06a03fe35791b0578068ab1873455463", 12040937),
	GAME_ENTRY_EN("melrin1", "Melrin.exe", "803e65c28364b6bf44b7c4610fcdaa5a", 12822475),
	GAME_ENTRY_EN_PLATFORM("melrin1", "Melrin.exe", "bd5bf8a7d33f5bbb2534a65c143b7991", 31554137, "Remake"),  // itch.io
	GAME_ENTRY_EN_PLATFORM("melrin1", "Melrin.exe", "bd5bf8a7d33f5bbb2534a65c143b7991", 31554676, "Remake"),
	GAME_ENTRY_EN("melrin2", "Melrin2.exe", "803e65c28364b6bf44b7c4610fcdaa5a", 4372756),
	GAME_ENTRY_EN("melrin2", "Melrin2.exe", "803e65c28364b6bf44b7c4610fcdaa5a", 4372768),
	GAME_ENTRY_EN("melrin3", "Melrin3.exe", "803e65c28364b6bf44b7c4610fcdaa5a", 5802221),
	GAME_ENTRY_EN_PLATFORM("melrin3", "dragon.exe", "bd5bf8a7d33f5bbb2534a65c143b7991", 28406008, "Remake"),
	GAME_ENTRY_EN("memochi", "DUST.exe", "bd36b73cda616d093a91e8c831eec59b", 178343415),  // Win
	GAME_ENTRY_EN("memochi", "game.ags", "bd36b73cda616d093a91e8c831eec59b", 178343415),  // Mac
	GAME_ENTRY_EN("memoriae", "memoriae.exe", "b5c41e08919834b549ca350a28f18f87", 13263166),
	GAME_ENTRY_EN("memoriesfade", "MemoriesFade.exe", "d5d028212a242a9841feff24ec3db3c9", 7996519),  // MAGS Win
	GAME_ENTRY_EN("memoriesfade", "MemoriesFade.ags", "a529d2492d927cb2893e636992cf4b00", 5483607),  // MAGS Linux
	GAME_ENTRY_EN("memoriesfade", "MemoriesFade.exe", "d5d028212a242a9841feff24ec3db3c9", 8000281),  // 1.1.0 Win
	GAME_ENTRY_EN("memoriesfade", "MemoriesFade.ags", "38a889fdcc95f2d0dbe6852ffcfe6359", 5487369),  // 1.1.0 Linux
	GAME_ENTRY_EN("memoriesfade", "MemoriesFade.ags", "4e327e87af5db477ec5c665b64ccd990", 5483607),
	GAME_ENTRY("memoriesofasnake", "Memoires d'un Serpent.exe", "615e73fc1874e92d60a1996c2330ea36", 111088941),  //En-Fr
	GAME_ENTRY_EN("memory", "MemoryAGSGame.exe", "a91d6931844b676411d8afe4698e9ff8", 405820699),
	GAME_ENTRY_EN("menial", "Menial.exe", "41750762e623576d25e8a202c91aa125", 48770858),  // v1.0 itch.io
	GAME_ENTRY_EN("menial", "Menial.ags", "cf138d6fa2bbf7ccbdb002c065d6d268", 45718294),
	GAME_ENTRY_EN("meninhats", "GAME.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1646510),
	GAME_ENTRY_EN("merrychristmas", "Merry Christmas.exe", "172b30c282856b382464bed5fcb61e29", 4812266),
	GAME_ENTRY_EN("meshumba", "meshumba3.exe", "615e73fc1874e92d60a1996c2330ea36", 265700112),
	GAME_ENTRY_EN_PLATFORM("messedupmothergoose", "MMG.exe", "615e73fc1874e92d60a1996c2330ea36", 5254097, "Beta 1"),
	GAME_ENTRY_EN_PLATFORM("messedupmothergoose", "MMG.exe", "615e73fc1874e92d60a1996c2330ea36", 10477725, "Beta 2"),
	GAME_ENTRY_EN_PLATFORM("messedupmothergoose", "MMG.exe", "615e73fc1874e92d60a1996c2330ea36", 12766119, "Beta 3"),
	GAME_ENTRY_EN_PLATFORM("messedupmothergoose", "MMG.exe", "615e73fc1874e92d60a1996c2330ea36", 13866989, "Beta 4"),
	GAME_ENTRY_EN_PLATFORM("messedupmothergoose", "MMG.exe", "615e73fc1874e92d60a1996c2330ea36", 25892061, "Beta 5"),
	GAME_ENTRY_EN("messedupmothergoose", "MMG.exe", "d642f2110a3dc0c48ab7b13f69b5caf6", 36523450),
	GAME_ENTRY_EN("messgoblins", "Mess Goblins.exe", "46720f26b37d0418ce0152e070a9674d", 278006146),
	GAME_ENTRY_EN_PLATFORM("meta", "META.exe", "0710e2ec71042617f565c01824f0cf3c", 9276269, "Original Version"),
	GAME_ENTRY_EN("meta", "META.exe", "06a03fe35791b0578068ab1873455463", 10113135),
	GAME_ENTRY_EN_STEAM("meta", "meta.exe", "9358670438c3fab77489a1dd229029b1", 2536926),
	GAME_ENTRY_EN_STEAM("meta", "META.ags", "6c385ee4f0d244cbc3b52d42aaeb6cd1", 4622133),
	GAME_ENTRY_EN("metaphobia", "Metaphobia.exe", "10da7427cf74ba0cc7ceb29f99ff0a67", 92083437), // itch.io 1.01 Windows
	GAME_ENTRY_EN("metaphobia", "ac2game.exe", "10da7427cf74ba0cc7ceb29f99ff0a67", 92083437), // itch.io MacOS
	GAME_ENTRY("metaphobia", "Metaphobia.exe", "10da7427cf74ba0cc7ceb29f99ff0a67", 92157492), // itch.io Eng-Ita
	GAME_ENTRY_EN_STEAM("metaphobia", "Metaphobia.exe", "10da7427cf74ba0cc7ceb29f99ff0a67", 92086491),
	GAME_ENTRY("meteorhead1", "Meteorhead.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5081311),  // En-De
	GAME_ENTRY_LANG("meteorhead2", "mh2.exe", "06a03fe35791b0578068ab1873455463", 5833556, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead3", "MeteorIII.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 15444551, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead4", "MHVIER.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6113101, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead5", "mhfive.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8788325, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead6", "exuehl.exe", "06a03fe35791b0578068ab1873455463", 6658907, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead7", "meteorkopp.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8262732, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead8", "MeteorheadVIII.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5254698, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead9", "MeteorheadIX.exe", "1275885401b7d2ece491e704535707d9", 4203283, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead10", "MHX.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 9424551, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead11", "headgar.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7806451, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead13", "newmeteorheadepisode.exe", "81a63b468681a143021f64ab88da28a3", 13415996, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorhead14", "thebrandnewmeteorhead14.exe", "8f2d3fbc7f428bea31f1021cb8c2ff5a", 20634898, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorheadns3", "newseriesIII.exe", "3c888514e33c5c1862ab7aba121b9dd4", 22787731, Common::DE_DEU),
	GAME_ENTRY_LANG("meteorheadns4", "Meteorhead Last Chapter.exe", "f18f785cade71cf3969b4c6f229454a2", 29499534, Common::DE_DEU),
	GAME_ENTRY_EN("metrocity", "Metro City Night Shift.exe", "e89db844661ed4f434e94795c62f871c", 789734564),
	GAME_ENTRY_EN("metrocity", "Metro City Night Shift.exe", "7d177d607daf005c11acacc91ec22df1", 786783897),  // v11
	GAME_ENTRY_EN("metrocity", "Metro City Night Shift.exe", "7d177d607daf005c11acacc91ec22df1", 786784099),  // v13
	GAME_ENTRY_EN("metrocity", "Metro City Night Shift.ags", "9de0e8ae38e18cf743ba7f67311476be", 783591045),
	GAME_ENTRY_EN("mi", "Mi.exe", "615e73fc1874e92d60a1996c2330ea36", 6988459),
	GAME_ENTRY("micarnivalofthedamned", "MI-COD.exe", "90413e9ae57e222f8913b09d2bc847bc", 5114086),  // Eng-Esp
	GAME_ENTRY("mickeymauserpart1", "MM.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12076323),  // En-It
	GAME_ENTRY("mickeymauserpart1", "MM.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12075852),  // En-It
	GAME_ENTRY_EN("midasheist", "midastouch.exe", "6e861b1f476ff7cdf036082abb271329", 10810464),
	GAME_ENTRY_EN("midnightmadness", "MidnightMadnessAtMoonForest.ags", "b27c3773f4a1b115194b9db48bf3cec0", 168846422),  //v1.1e
	GAME_ENTRY_EN("midnightsquadron", "MidnightSquadronMAGS.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 29963579),
	GAME_ENTRY("miillusion", "Illusion.exe", "0500aacb6c176d47ac0f8158f055db83", 3245163),  // En-Fr
	GAME_ENTRY_EN("mikasdream2", "MSD.exe", "02094e217c23e1d40a84891ee95010cb", 9951843),
	GAME_ENTRY_EN("mikesroom", "Mikes_late.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5001254),
	GAME_ENTRY_EN("milkshake", "Milkshake.exe", "615e806856b7730afadf1fea9a756b70", 1656268),
	GAME_ENTRY_EN("mindboggler", "Rebel.exe", "847ca9d174cd091b7a1b82f032bdd052", 36979705),
	GAME_ENTRY_EN("mindrape", "Duress.exe", "b7996387269db9b8a17bc358eaf37464", 125143347),  //v1.2
	GAME_ENTRY_EN("mindseye", "MindsEye.exe", "3421b46ff95c5885603086c39a038a20", 28385591),
	GAME_ENTRY_EN("mindseye", "MindsEye.exe", "3421b46ff95c5885603086c39a038a20", 28386775),  // v1.2a
	GAME_ENTRY_EN("mindseye", "MindsEye.exe", "3421b46ff95c5885603086c39a038a20", 28389228),  // v1.2b
	GAME_ENTRY_EN("mindseye", "MindsEye.exe", "8a84eb07f484540ecc59ea80bf21dc9e", 30024175),  // v1.4
	GAME_ENTRY_EN("minifeg", "Minifeg.exe", "f18443f98fd61d2c655e76a17f7da905", 108477985),  //v1.4
	GAME_ENTRY("minorminion", "Gremlin.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3962167),  // En-De
	GAME_ENTRY_PLATFORM("minorminion", "Gremlin.exe", "2cb9c81a20282b55f69343ef95cbb63d", 3962237, "MAGS"),  // En-De
	GAME_ENTRY("misc", "Misc.exe", "0564de07d3fd5c16e6947a647061913c", 25603070),  // En-Fr Win
	GAME_ENTRY("misc", "Misc.ags", "a3a49fc9613dd24b83bf6cd6ee10a30a", 22570474),  // En-Fr Linux
	GAME_ENTRY_EN("missinginaction", "teamwork.exe", "96f83f7decb523d0c621646828cad249", 3741039),
	GAME_ENTRY("missionfutura", "MissionFutura.exe", "615e73fc1874e92d60a1996c2330ea36", 3083182),  //v1.3 En-De
	GAME_ENTRY("missionfutura", "MissionZukunftia.exe", "0564de07d3fd5c16e6947a647061913c", 4305024), //v1.4 En-De
	GAME_ENTRY_LANG("missionfutura2", "MissionZukunftia2.exe", "839a7cbaa6c287ca35cf46a2505b48eb", 6672476, Common::DE_DEU),
	GAME_ENTRY_EN_PLATFORM("missingsincemidnight", "msm.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6738599, "MAGS"),
	GAME_ENTRY("missingsincemidnight", "msm.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7989479),  // En-De
	GAME_ENTRY_EN("misspingu", "Ms Pingu.exe", "618d7dce9631229b4579340b964c6810", 3567968),
	GAME_ENTRY_LANG("mistdelaescueladearte", "EA.exe", "465f972675db2da6040518221af5b0ba", 6732362, Common::ES_ESP),
	GAME_ENTRY_EN("mistook", "mistook.exe", "f452bafa2eb31215e831d3e232a9bb39", 7302775),
	GAME_ENTRY("mmm1", "MMM.exe", "465f972675db2da6040518221af5b0ba", 5476481),  // En-De-Ru
	GAME_ENTRY("mmm1", "MMM.exe", "137a2bc940f033beab257b2fa88526ba", 10917227),  // En-De-It-Ru
	GAME_ENTRY("mmm1", "MMM.exe", "086058b5f74bf8e5aa35402a8af7858f", 12528126),  // En-De-It-Ru + De speech
	GAME_ENTRY("mmm2", "MMM02.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5830457),  // En-De
	GAME_ENTRY("mmm2", "MMM02.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8042121),  // En-De + De speech
	GAME_ENTRY_LANG("mmm3", "mmme2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5795895, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm4", "mmm.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8368739, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm5", "MMM05.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6181463, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm6", "jeff.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5859742, Common::DE_DEU),
	GAME_ENTRY("mmm7", "MMM7.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5936294),  // En-De + De speech
	GAME_ENTRY("mmm8", "MMM - E8.exe", "45ab4f29031b50c8d01d10a269f77ff5", 5095385),  // En-De
	GAME_ENTRY("mmm9", "mmm_radioactive.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7537174),  // v1.04 En-De
	GAME_ENTRY("mmm9", "mmm_radioactive.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7579190),  // v1.05 En-Fr-De-Es
	GAME_ENTRY("mmm9", "mmm_radioactive.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7665414),  // v1.06 En-Fr-De-Es
	GAME_ENTRY_LANG("mmm10", "MMM-TOTWE.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8455565, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm10", "MMM-TOTWE.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8455743, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm11", "MMMania 2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7872940, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm11", "MMMania.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6123337, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm12", "Giga.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4141502, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm13", "traum.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6703140, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm14", "msytr.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5836799, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm14", "msytr.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5863583, Common::DE_DEU),
	GAME_ENTRY("mmm15", "MMM15.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8957215),	// En-De
	GAME_ENTRY("mmm15", "MMM15.exe", "06a03fe35791b0578068ab1873455463", 8503015),  // En-De-It
	GAME_ENTRY("mmm16", "Meteor Family.exe", "06a03fe35791b0578068ab1873455463", 5759077),  // En-De
	GAME_ENTRY("mmm16", "Meteor Family.exe", "06a03fe35791b0578068ab1873455463", 5777213),  // En-De-It
	GAME_ENTRY_LANG("mmm17", "mmmgui.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5019001, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm17", "mmmgui.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6503602, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm18", "Shit Happens!.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6300460, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm18", "Shit Happens!.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6300947, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm21", "mmmgui1.exe", "465f972675db2da6040518221af5b0ba", 8757734, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm22", "mmm.exe", "465f972675db2da6040518221af5b0ba", 6909531, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm23", "MANIAC.exe", "465f972675db2da6040518221af5b0ba", 5904865, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm24", "mmmgui.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6112992, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm26", "TryDott.exe", "28f82e420b82d07651b68114f90223c8", 4683840, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm27", "Hamster.exe", "465f972675db2da6040518221af5b0ba", 7497001, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm28", "Time Machine 2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7314610, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm29", "mmmgui.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8033934, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm30", "memorie.exe", "0710e2ec71042617f565c01824f0cf3c", 4300662, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm31", "MMM31.exe", "ca70858da2a347bd8ec2dedcbb4113b4", 7382014, Common::DE_DEU),
	GAME_ENTRY("mmm31", "MMM31.exe", "0710e2ec71042617f565c01824f0cf3c", 6777515),  // De-It
	GAME_ENTRY("mmm31", "MMM31.exe", "0710e2ec71042617f565c01824f0cf3c", 6777832),  // En-De-It
	GAME_ENTRY("mmm31", "MMM31.exe", "07b6c61269775f714e681c3a184a8580", 8014988),  // En-De-It + De speech
	GAME_ENTRY_LANG("mmm32", "save_mmm.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6291592, Common::DE_DEU),
	GAME_ENTRY("mmm33", "Smiley.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7567946),  // En-De
	GAME_ENTRY("mmm33", "Smiley.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7568039),  // En-De
	GAME_ENTRY_LANG("mmm34", "Helden.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 21647765, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm34", "Helden.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 21648581, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm35", "Weggebeamt.exe", "0710e2ec71042617f565c01824f0cf3c", 4255405, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm36", "Der Liebesbrief.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9499807, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm37", "SandyEppi.exe", "ca70858da2a347bd8ec2dedcbb4113b4", 10311675, Common::DE_DEU),
	GAME_ENTRY("mmm37", "MMM37.exe", "0710e2ec71042617f565c01824f0cf3c", 10092409),  // De-It
	GAME_ENTRY_LANG("mmm38", "Rescue Mission.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 6912146, Common::DE_DEU),
	GAME_ENTRY("mmm38", "mmm38.exe", "81083747db230fe1e7bbe64857467b56", 8701976),  // De-It
	GAME_ENTRY_LANG("mmm39", "erinner.exe", "465f972675db2da6040518221af5b0ba", 10525885, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm39", "Erinnerungen.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6673993, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm39", "Erinnerungen.exe", "a8cefa4dcd250caba6d155f35b1efda4", 5181693, Common::DE_DEU),  // v1.3
	GAME_ENTRY("mmm40", "Verflixte Türen.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9483433),  // En-Fr-De-Es
	GAME_ENTRY("mmm40", "MMM40.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 10526527),  // v2.05    "
	GAME_ENTRY("mmm40", "MMM40.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 10990625),  // v3.01    Multi
	GAME_ENTRY_LANG("mmm40", "MMM40.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9385965, Common::DE_DEU),  // Talkie
	GAME_ENTRY("mmm41", "mmm41.exe", "6d5a5284ed0403e75d64f42f415242a5", 8634455),  // De-It
	GAME_ENTRY_LANG("mmm42", "psychobernie.exe", "0710e2ec71042617f565c01824f0cf3c", 6696188, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm43", "Edna.exe", "0710e2ec71042617f565c01824f0cf3c", 5149386, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm44", "Razor.exe", "0710e2ec71042617f565c01824f0cf3c", 4820839, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm45", "MMM45.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 13327742, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm46", "Lost.exe", "0710e2ec71042617f565c01824f0cf3c", 5143252, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm47", "Ostern.exe", "5c183013f9d8ed144de05f239633a604", 7416584, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm48", "mmm_wendy_271.exe", "f120690b506dd63cd7d1112ea6af2f77", 8870057, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm49", "Clouso.exe", "0710e2ec71042617f565c01824f0cf3c", 12159468, Common::DE_DEU),
	GAME_ENTRY("mmm50", "mmm50.exe", "5c183013f9d8ed144de05f239633a604", 5258429),  // De-It
	GAME_ENTRY("mmm51", "Ortmaschine II.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 9261025),  // De-It
	GAME_ENTRY_LANG("mmm52", "52 v2.exe", "4d7d2addcde045dae6e0363a43f9acad", 15632826, Common::DE_DEU),
	GAME_ENTRY("mmm53", "MMM-dksz.exe", "c1df737ef943e6e3cc09d36fcd4b1ed0", 8689677),
	GAME_ENTRY("mmm53", "MMM-dksz.exe", "c1df737ef943e6e3cc09d36fcd4b1ed0", 8708254),  //v1.7 En-Fr-De-It
	GAME_ENTRY("mmm53", "MMM53-dksz.exe", "c1df737ef943e6e3cc09d36fcd4b1ed0", 50995033),  //v1.8 En-Fr-De-It + De speech
	GAME_ENTRY_LANG("mmm54", "CSI_RONVILLE.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 19836078, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm55", "ChapterofDream.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5539795, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm56", "Grotten-Urlaub.exe", "615e806856b7730afadf1fea9a756b70", 6641264, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm57", "Episode 57-v2.exe", "90413e9ae57e222f8913b09d2bc847bc", 22458183, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm58", "COURT.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8245554, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm59", "KevinGUI.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5299420, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm60", "mmm60.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6431816, Common::DE_DEU),
	GAME_ENTRY("mmm61", "bernardsRoom.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8767774),  // En-De
	GAME_ENTRY("mmm61", "bernardsRoom.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 8855002),  // En-De-Es-It
	GAME_ENTRY_LANG("mmm63", "Bernard_272.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5315794, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm64", "Baranoia.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6053354, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm65", "Save Smiley!.exe", "f120690b506dd63cd7d1112ea6af2f77", 5306178, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm66", "Hoagie.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6131305, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm66akt3", "Hoagie.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6287962, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm67", "Epi67.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 40585222, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm68", "MaP.exe", "615e806856b7730afadf1fea9a756b70", 8260817, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm69", "Samstag.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5425439, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm70", "MMB.exe", "7b8b28390b898ad22427bd92b2c38959", 38612057, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm71", "Dreiauge.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 28016268, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm72", "mindbending.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7306081, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm73", "Eabc.exe", "4d7d2addcde045dae6e0363a43f9acad", 5708460, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm74", "tedventure.exe", "90413e9ae57e222f8913b09d2bc847bc", 7768144, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm75", "Necronomicon.exe", "0710e2ec71042617f565c01824f0cf3c", 9645824, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm76", "bernoullishow.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5380631, Common::DE_DEU),
	GAME_ENTRY("mmm77", "MMM77.exe", "06a03fe35791b0578068ab1873455463", 13099621),  // En-De
	GAME_ENTRY_LANG("mmm78", "Dumm geholfen.exe", "18b284c22010850f79bc5c20054a70c4", 11787742, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm79", "Wunsch-O-Mat.exe", "74aad8dfd71ed2ae9574a60be7610c43", 5071097, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm80", "Spiel des Lebens.exe", "bb59de174d70797d774dec76a171352d", 4989738, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm81", "Bernard bricht aus!.exe", "bb59de174d70797d774dec76a171352d", 4461097, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm82", "Freundin mit Hindernissen.exe", "bb59de174d70797d774dec76a171352d", 4470551, Common::DE_DEU),
	GAME_ENTRY("mmm83", "Chasing Hoagie.exe", "c8daf8ef2bc83ea8b0f310cfce209401", 6083727),  // En-De-It
	GAME_ENTRY_LANG("mmm84", "Surf'n'Ronville.exe", "736c02211122a28ec41646a29c22688f", 11288591, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm85", "Compiled.exe", "c8daf8ef2bc83ea8b0f310cfce209401", 5003407, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm86", "Bernard bekommt Besuch.exe", "430eaebb21d406061d67a9972ad33947", 18382681, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm87", "Hotel Ronville.exe", "0ab0633f966ed01403362e6dfb5ca7d4", 12678222, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm88", "Der alltaegliche Wahnsinn.exe", "74dc062c5f68f3d70da911f2db5737b6", 4412559, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm89", "Weltdateien.exe", "e2922ce173aaab9881647510498bdaf5", 26312833, Common::DE_DEU),
	GAME_ENTRY("mmm90", "mmm.exe", "422a21f31d7da327343f54e0a69a08b1", 9581780),  //v1.3 En-De-It
	GAME_ENTRY("mmm90", "mmm90.exe", "f1d98b6c9f4fb300181a927948ec8ef5", 8606394),  //v1.4 En-De-It
	GAME_ENTRY("mmm90", "mmm90.exe", "5097c2dbd1ca671fc889d11abe55e5b2", 8993690),  //v1.4 En-De-It + De speech
	GAME_ENTRY_LANG("mmm91", "MMM-FidB.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6345594, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm93", "Moonshine.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 9184631, Common::DE_DEU),
	GAME_ENTRY("mmm94", "Mississippi.exe", "311cf286b596ed7c68d5505424dea1bc", 19670283),  // En-De-It
	GAME_ENTRY("mmm95", "BritneysEscape.exe", "37a096932a37607b8115f57d269229b9", 16216498),  // En-De-It
	GAME_ENTRY_LANG("mmm96", "Dave.exe", "430eaebb21d406061d67a9972ad33947", 12980000, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm97", "X-mas Mansion.exe", "74dc062c5f68f3d70da911f2db5737b6", 10879897, Common::DE_DEU),
	GAME_ENTRY("mmm97", "X-mas Mansion.exe", "b643c4e0619a8a271383c02b3ac9196f", 12095953),  // De-It
	GAME_ENTRY("mmm98", "mmm98.exe", "fb1e40d2828ca16537365b91a90b5bee", 14466864),
	GAME_ENTRY_LANG("mmm99", "die_premiere.exe", "3e9c6691d4b9fd70aa47c5bfd68c5c3a", 8312345, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm100", "money.exe", "93a18fcee8c601539cc99a2bf87f06aa", 51747173, Common::DE_DEU),
	GAME_ENTRY_LANG("mmm100", "money.exe", "30846981fb096c7e94534ee2c2df9586", 53536477, Common::DE_DEU),  // v1.2 De speech
	GAME_ENTRY("mmm100", "money.exe", "91b1d125f9c818b8d2f376d24774377e", 52085746),  // En-De
	GAME_ENTRY_LANG("mmm101", "MMM_MiAu.exe", "bc40ee6a0cf03a983f9fcaca906d12b8", 7696543, Common::DE_DEU),
	GAME_ENTRY("mmm101", "MMM_MiAu.exe", "bc40ee6a0cf03a983f9fcaca906d12b8", 8192536),  // En-De + De speech
	GAME_ENTRY_LANG("mmmd1", "Starter.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5174259, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd2", "002.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5104718, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd3", "003.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5275394, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd4", "004.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6843951, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd5", "005.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 4704363, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd6", "006.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6687936, Common::DE_DEU),
	DETECTION_ENTRY("mmmd7", "007.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5518012, Common::DE_DEU, "Original version", ADGF_NO_FLAGS, 0),
	GAME_ENTRY_LANG("mmmd7", "007.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5822593, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd8", "008.exe", "06a03fe35791b0578068ab1873455463", 5090954, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd9", "009.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5563742, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd10", "010.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 15086975, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd11", "011.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5247671, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd12", "012.exe", "4d7d2addcde045dae6e0363a43f9acad", 4947527, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd13", "013.exe", "4d7d2addcde045dae6e0363a43f9acad", 7352840, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd14", "014.exe", "06a03fe35791b0578068ab1873455463", 6149608, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd15", "015.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5516147, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd16", "016.exe", "4d7d2addcde045dae6e0363a43f9acad", 5879156, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd17", "017.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6069586, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd18", "018.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6102088, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd19", "019.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5157252, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd20", "020.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 4974813, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmd21", "021.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5588743, Common::DE_DEU),
	GAME_ENTRY("mmmeaster2010", "Ostereiersuche2010.exe", "2f19c54fbc8da902ad8cdebc56261da1", 9743112),  // De-It
	GAME_ENTRY("mmmeaster2011", "Ostereiersuche2011.exe", "09d2b29962dc4c3b8ee0f03a60dc93b1", 9925586),  // De-It
	GAME_ENTRY_LANG("mmmeaster2018", "easter18.exe", "fd1de819239fab19d8c5078d77c4d9d1", 12310225, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar1", "Awards-2006.exe", "f120690b506dd63cd7d1112ea6af2f77", 5556504, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar2", "EdgarAward2.exe", "7a3096ac0237cb6aa8e1718e28caf039", 5596958, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar2s2", "EdgarAward2_II.exe", "5c183013f9d8ed144de05f239633a604", 4856410, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar3", "Edgar 3.exe", "f120690b506dd63cd7d1112ea6af2f77", 6942168, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar4", "edgar4.exe", "1f6db69d0f48c58ced932629f7a9370a", 14094383, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar5", "Edgar5.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 15571932, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar6", "Edgar6.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 10504905, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar7", "Edgar 7.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7459316, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar8", "Edgar 8.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 10687183, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmedgar9", "Award.exe", "74dc062c5f68f3d70da911f2db5737b6", 4938828, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhollywood", "RonvilleViper.exe", "217e8ee8014220e9a9a2c053bd59b65a", 21158342, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw1", "Halloween.exe", "28f82e420b82d07651b68114f90223c8", 7354818, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw2", "Escape.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 10251940, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw3", "MMM - DotD.exe", "0710e2ec71042617f565c01824f0cf3c", 4843398, Common::DE_DEU),
	GAME_ENTRY("mmmhw3", "MMM - DotD.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5664499),  // En-De-It
	GAME_ENTRY_LANG("mmmhw4", "MMM-Horror.exe", "0710e2ec71042617f565c01824f0cf3c", 4300488, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw5", "Redrum.exe", "0710e2ec71042617f565c01824f0cf3c", 4410844, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw6", "TCoMM.exe", "f120690b506dd63cd7d1112ea6af2f77", 5073963, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw7", "Utschi-2006.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7399530, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw8", "racoon.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 11664995, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw9", "TMMCMFH.exe", "c8daf8ef2bc83ea8b0f310cfce209401", 31905464, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw10", "Bernard_272.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 9775462, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmhw11", "Bad and Mad.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6895237, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmm1", "EdsPaket.exe", "fbd756f1634f2f225a849b79ca4eaa9f", 12586035, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmm1", "EdsPaket.exe", "6dca2f8b0b0cd8fd0f675fe188d223fe", 13393024, Common::DE_DEU),  // Talkie
	GAME_ENTRY_LANG("mmmmm2", "MMMMM2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 14611796, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmm2", "MMMMM2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 14617195, Common::DE_DEU),  // v2.0
	GAME_ENTRY_LANG("mmmmm3", "MMMMM3.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6114755, Common::DE_DEU),
	GAME_ENTRY("mmmmm4", "bertholdsreturn.exe", "ce5605f3f83c609b8ffce472936e6d83", 10292426),  // En-De
	GAME_ENTRY_LANG("mmmmm5", "marcy.exe", "2ce63b8fabba844aaf06426814e3f40f", 10234949, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmm5", "marcy.exe", "2ce63b8fabba844aaf06426814e3f40f", 10234769, Common::DE_DEU),  // fireorange's itch.io
	GAME_ENTRY_LANG("mmmmovie1", "in da house1.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 10384508, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie2", "Doktor2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 12675202, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie3", "Kochen mit Fred.exe", "465f972675db2da6040518221af5b0ba", 19068509, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie4", "TheNewPresident.exe", "ed3b9d2327ca7d1f135d512a4878bd9b", 8327406, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie5", "DidH 3.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 13883814, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie6", "fiveyearsMMM.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5800533, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie7", "5th Maniac Birthday.exe", "c8daf8ef2bc83ea8b0f310cfce209401", 25642642, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmmovie8", "dinner_for_one.exe", "277461e060a18911d749f7ba844a1e59", 7242433, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash1", "Sandy.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8553956, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash2", "futanari.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5302087, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash3", "futa2.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5305031, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash4", "futanari3.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 4553630, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash5", "DHdF4.exe", "f120690b506dd63cd7d1112ea6af2f77", 4569740, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash6", "NoGUI.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7214705, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash7", "hsfgben.exe", "f120690b506dd63cd7d1112ea6af2f77", 4295518, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash8", "Lucas.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5654449, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash9", "Escape.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5200352, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash10", "MMM-Quiz.exe", "f120690b506dd63cd7d1112ea6af2f77", 5081606, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash11", "red-tentacle.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 12742138, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash12", "runHoagierun.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5349730, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash13", "klo.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5126624, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash14", "Bernard_272.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5125302, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash15", "mio.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6320145, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash16", "Trashmillionair.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5303845, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash17", "Halloween.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6342327, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash18", "Bernard_272.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 7988344, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash19", "BM4M.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5940382, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash20", "BritneyMussMahl.exe", "b42f80733b6bd1ded5e29be2c683afa8", 4093782, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash21", "Syd-Start2_71.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5360298, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmtrash22", "Michael.exe", "20160ec9df51a5e86572ba8448476888", 12126389, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmxmas2008", "A_Xmas_Odyssey.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6133274, Common::DE_DEU),
	GAME_ENTRY_LANG("mmmxmas2015", "Three Days Before Christmas.exe", "67f67bbd7236fda27dc510514b2e2142", 13274327, Common::DE_DEU),
	GAME_ENTRY("mobileangel", "splitconscience.exe", "615e73fc1874e92d60a1996c2330ea36", 13390925),  // Eng-Rus
	GAME_ENTRY_EN("moncul", "moncul.exe", "0710e2ec71042617f565c01824f0cf3c", 1493822),
	GAME_ENTRY_EN("moneycab", "Money Cab.exe", "173f00e582eebd082d66f1291702b373", 4806018),
	GAME_ENTRY_LANG("monkeyklon", "monkeyklon.exe", "0710e2ec71042617f565c01824f0cf3c", 5196763, Common::DE_DEU),
	GAME_ENTRY("monkeyklon", "MI-Klon.exe", "c3b72132686c4a102b3365d380c8ae3a", 7444633),  // De-It
	GAME_ENTRY_EN("monkeymountain", "monkeyy.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 1784095),
	GAME_ENTRY_EN("monkeystothemoon", "Monkeys to the Moon.exe", "77d91b65ff03c1f3a8381e39c7f693cb", 8720439),
	GAME_ENTRY_EN("monkeystothemoon", "Monkeys to the Moon.exe", "77d91b65ff03c1f3a8381e39c7f693cb", 2864790),  // Win-Linux
	GAME_ENTRY_EN("monsterfromthehountedhill", "Monster.exe", "465f972675db2da6040518221af5b0ba", 6197451),
	GAME_ENTRY_EN("monsterwantsin", "Mandle Games MONSTER.exe", "4de79e136b2fb921dc00d06a4f9f84ac", 22068111),
	GAME_ENTRY_EN("montykomodo", "Akomodation.exe", "615e73fc1874e92d60a1996c2330ea36", 120481295),
	GAME_ENTRY_EN_PLATFORM("montyonthenorm", "monty.exe", "06a03fe35791b0578068ab1873455463", 2478569, "2007 Remake"),
	GAME_ENTRY_EN_PLATFORM("montyonthenorm", "monty.exe", "f8e8f781a4a95c2bfea5f54b085df550", 3270202, "2014 Remake"),
	GAME_ENTRY_EN("montypythonmansion", "MakeoutMansion.exe", "256752c9a97b4780fc5e6f3239c8cdf1", 2951811),
	GAME_ENTRY_EN("moonlightmoggy", "Moggy.exe", "0a6704159f6f716ac80da91c430430ce", 13336128),
	GAME_ENTRY("moorlandsonata", "Moorland Sonata.exe", "7f151ffa6f87f2c5690d308bfba59805", 280454896),  // Eng-Ita
	GAME_ENTRY_EN("mooserage", "MOOSE.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 4732219),
	GAME_ENTRY_EN("mooserage2", "MOOSE2.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1251361),
	GAME_ENTRY_EN("mops", "MOPS.ags", "aba16b5f579aff9cdfba2350e86404cb", 219201624),
	GAME_ENTRY_EN("mordy1", "Mordy.exe", "0710e2ec71042617f565c01824f0cf3c", 43552329),
	GAME_ENTRY_EN("mordy2", "Mordy2.exe", "0710e2ec71042617f565c01824f0cf3c", 531648251),
	GAME_ENTRY_EN("mordy2", "Mordy2.exe", "0710e2ec71042617f565c01824f0cf3c", 5186329),
	GAME_ENTRY_EN("moremonkeys", "MoreMonkeys.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 2652682),
	GAME_ENTRY_EN("morganale1", "Professor D.exe", "6f9a7b413f14514c8314fe56fda90179", 4009582),
	GAME_ENTRY_EN("morningshift", "Ludum Dare 48.ags", "bd508b7787384ae2ad5b487692d3b807", 3793136),  // Linux
	GAME_ENTRY_EN("morningshift", "Ludum Dare 48.exe", "7c6e063343fc2ec2bfffc93a1bbd6cfe", 6280960),  // Windows
	GAME_ENTRY_EN("mort", "MORT.ags", "bf38464d610f68f57d375008cd32afe5", 1245969),
	GAME_ENTRY_EN("mort", "MORT.ags", "8af20df3f0243715ddc95bd707789b08", 1251609),  // patch2
	GAME_ENTRY_EN("mort", "MORT.ags", "d5779acfacdde353d9a26239c224ce4d", 1256471),  // patch3
	GAME_ENTRY_EN("mort", "MORT.ags", "c6ab754bd829fe00c1ac8073d3082721", 1260574),  // itch.io client
	GAME_ENTRY_LANG("mortifer", "Nous les Mortifer.exe", "7ddb9e776648faed5a51170d087074e9", 187358960, Common::FR_FRA),
	GAME_ENTRY_LANG("mortifer", "Nous les Mortifer.ags", "b67c4e1005f5ab6002a56bf959009986", 146208681, Common::FR_FRA),
	GAME_ENTRY_EN("motlpaa", "MOTLPAA.exe", "0710e2ec71042617f565c01824f0cf3c", 1575258),
	GAME_ENTRY_EN("motlpaa", "MOTLPAA.ags", "0710e2ec71042617f565c01824f0cf3c", 1575258),
	GAME_ENTRY("mourirenmer", "Mourir.exe", "97d700529f5cc826f230c27acf81adfd", 1926427),  // En-Fr
	GAME_ENTRY("mourirenmer", "Mourir.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2519706),
	GAME_ENTRY_EN("mouseoverslept", "OSD Mr Cheeses.exe", "510be04e5eec2aa92bb78b060f138d64", 4985642),
	GAME_ENTRY_EN("moustachequest", "Moustache Mike.exe", "2ca80bd50763378b72cd1e1cf25afac3", 4658444),  //v1.3
	GAME_ENTRY_EN("mrbeareng", "MrBearTeachesEnglish.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 32384398),  // Windows
	GAME_ENTRY_EN("mrbeareng", "MrBearTeachesEnglish.ags", "2d227a1054056942c7a650cddffd6309", 29352826),  // Linux
	GAME_ENTRY_EN("mrchocolate", "mrchoc.exe", "c5eced7d321ca71d10d75852ccdb0757", 28872320),
	GAME_ENTRY_EN("mrdangerscontest", "Danger.exe", "06a03fe35791b0578068ab1873455463", 7095657),  // v1.0
	GAME_ENTRY_EN("mrdangerscontest", "Danger.exe", "06a03fe35791b0578068ab1873455463", 7096987),  // v1.1
	GAME_ENTRY_EN("mrfrisby", "Xhristmas.exe", "06a03fe35791b0578068ab1873455463", 17801024),
	GAME_ENTRY("mrtijerakis", "MRTIJERAKIS.exe", "7971a7c02d414dc8cb33b6ec36080b91", 12315667),  // En-Fr-Es
	GAME_ENTRY_EN("mudlarks", "Mudlarks.exe", "dfa63386f3e7dd011447982affff1154", 311640039),
	GAME_ENTRY_EN("murderdog4", "MurderDogIV.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 9930349),
	GAME_ENTRY_EN("murdergrisly", "Murder most grisly!!.exe", "173f00e582eebd082d66f1291702b373", 18811522),
	GAME_ENTRY("murderinawheel", "MordImLaufrad.exe", "18b284c22010850f79bc5c20054a70c4", 4396809),  // En-De
	GAME_ENTRY_EN("murderinthemansion", "MitM 1.2.exe", "27df05bd72589a589c054d11d6d03c0f", 9758713),
	GAME_ENTRY_EN("murderofadrianelkwood", "elkwood.exe", "06a03fe35791b0578068ab1873455463", 3685153),
	GAME_ENTRY_EN("murphyssalvage1", "space.exe", "f8029b1e9ff5ac01ae23896af44e885a", 51351465),
	GAME_ENTRY_EN("murranchronicles1", "Jersey Devil.exe", "06a03fe35791b0578068ab1873455463", 20555872),
	GAME_ENTRY_EN("murranchronicles2", "Talons.exe", "06a03fe35791b0578068ab1873455463", 17613066),
	GAME_ENTRY_EN("murranchronicles3", "LifeDrinker.exe", "90413e9ae57e222f8913b09d2bc847bc", 13274387),
	GAME_ENTRY_EN("mushroomman", "TheMushroomMan.exe", "3128b9f90e2f954ba704414ae854d10b", 1655837),
	GAME_ENTRY_EN("musicmixer", "mixertest.exe", "0b7529a76f38283d6e850b8d56526fc1", 798369),
	GAME_ENTRY("mutagen", "mutagen.exe", "06a03fe35791b0578068ab1873455463", 7667151),  // En-Fr
	GAME_ENTRY_EN("muuyeeb", "ghost.exe", "71ca0d6c1c699595f28a2125948d4a84", 1531958),
	GAME_ENTRY_EN("myburdentokeep", "MBTK.exe", "52b5831250acb676c3c76b8e47af4b2c", 35230042),
	GAME_ENTRY_EN("mycroftsim", "MycroftSimulator.exe", "1d1deb6497d935c825b0398806e8306e", 8564800),  // Win (older)
	GAME_ENTRY_EN("mycroftsim", "MycroftSim2.exe", "1d1deb6497d935c825b0398806e8306e", 8564801),  // Win
	GAME_ENTRY_EN("mycroftsim", "MycroftSim2.ags", "497c9d258b73b887de38952c36864b8c", 5452333),  // Linux
	GAME_ENTRY_EN("mycroftsim", "ac2game.dat", "1d1deb6497d935c825b0398806e8306e", 8564801),  // Mac
	GAME_ENTRY_EN_PLATFORM("mycroftsim", "MycroftSim2.exe", "1177e20f6a84ded5fad2a5d79fff8cb5", 36486321, "Expanded Edition"),  // Win
	GAME_ENTRY_EN_PLATFORM("mycroftsim", "MycroftSim2.ags", "7c38ebc456869852c67dff549d320e88", 33317533, "Expanded Edition"),  // Linux
	GAME_ENTRY_EN("myfathersecret", "MyFathersSecret.exe", "981e60a9be515bb56c634856462abbc7", 406855726),
	GAME_ENTRY_EN("myfirstbigadv", "MY FIRST BIG ADVENTURE.exe", "82da2565c456dcfb265ded6fe3189c0b", 20143503),
	GAME_ENTRY_EN("myfirstquest", "test.exe", "0500aacb6c176d47ac0f8158f055db83", 837473),
	GAME_ENTRY_EN("mysiblings", "MySiblingsTheStones.ags", "6c997a79521dd61b6bffe76f8458a6ac", 13298394),
	GAME_ENTRY_EN("mysiblings", "MySiblingsTheStones.ags", "ce5625dc2ffd0337a4f6d367af25bb67", 13299355),  // v1.1
	GAME_ENTRY_EN("mysterioushouse", "mh.exe", "495d45fb8adfd49690ae3b97921feec6", 42400316),
	GAME_ENTRY_EN("mysterymeat", "Mystery Meat.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 40616773),
	GAME_ENTRY_EN("mysterymeat", "Mystery Meat.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 40617043),  //updated
	GAME_ENTRY_EN("mysticseer", "NickOfTime.exe", "f120690b506dd63cd7d1112ea6af2f77", 4460113),
	GAME_ENTRY_EN("nadir", "Nadir.exe", "fb7ea6d04f07fc7b0c801db309ca398e", 219543187),
	GAME_ENTRY_EN("nadir", "Nadir.ags", "342c4788634715e7acf9fdb734f5f8c8", 216899203),
	GAME_ENTRY_EN("nakedfear", "NakedFear.exe", "9f68485286be3e9295ecba2e0b4b8784", 128358123),
	GAME_ENTRY_EN("nanobots", "Nanobots.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 41977727),
	GAME_ENTRY_EN("nauticell", "Nauticell.exe", "49551ed74971e7422bd188a87bca6e19", 8457064),
	GAME_ENTRY_EN("necroquest", "necroquest01.exe", "2d111a69de0723f337bad661cb006239", 9545924),
	GAME_ENTRY_EN_PLATFORM("necroquest", "necroquest01.exe", "6d4adcef07a14b53369d23edf5117252", 12389204, "Deluxe"),
	GAME_ENTRY_PLATFORM("nedysadventure", "Nedy.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 6463558, "Deluxe"),  //v1.4 Eng-Esp
	GAME_ENTRY_EN("neiroalice", "NeiroAlice.exe", "e62760ecab805a66bc508c62582f3a9b", 347259114),
	GAME_ENTRY_EN("neiroalice", "NeiroAlice.ags", "7ff508f409a60fb58fe9ff54b7de9a77", 344092886),
	GAME_ENTRY_EN("neiroalice", "NeuroAlice.exe", "e62760ecab805a66bc508c62582f3a9b", 347258981),
	GAME_ENTRY_EN("neiroalice", "NeuroAlice.ags", "aa55837ffea30789337b05cfd37d60f8", 344092753),
	GAME_ENTRY_EN("nekusnewtrip", "nnt.exe", "c0c1865c3c8369e034095a725ca1ddbf", 35012412),
	GAME_ENTRY_EN_PLATFORM("nekusnewtrip", "square enix.exe", "a524cbb1c51589903c4043b98917f1d9", 10630694, "Chapter 1"),
	GAME_ENTRY_EN("nellycootalot", "Nelly Cootalot.exe", "18b284c22010850f79bc5c20054a70c4", 108256323),  // v1.5
	GAME_ENTRY("nellycootalot", "Nelly.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 198349190),  // v1.8 Multi
	GAME_ENTRY_EN_PLATFORM("neonwilderness", "mspaintjam.exe", "78dd4ca028ee0156b6a093d6d780aa65", 4594375, "Prototype"),
	GAME_ENTRY_EN("neosaires2070", "Neos Aires 2070 - Sunday Comes First.exe", "76703ac67835bdbfde5b378a147c8ba2", 84186100), // 1.0
	GAME_ENTRY_LANG("neosaires2070", "Neos Aires 2070.exe", "78dd4ca028ee0156b6a093d6d780aa65", 84126492, Common::ES_ESP), // 1.1
	GAME_ENTRY_EN("nerdyquest", "Nerdy Quest.exe", "430eaebb21d406061d67a9972ad33947", 19707725),
	GAME_ENTRY("nesquest", "NES Quest.exe", "8b72036706da98095057df615d07460b", 20881972),  // En-Fr-Es
	GAME_ENTRY("neveralonehl", "Hotline.exe", "615e73fc1874e92d60a1996c2330ea36", 3808222),  // Eng-Rus
	GAME_ENTRY_LANG("neverlandmonopoly", "Neverland Monopoly.exe", "a524cbb1c51589903c4043b98917f1d9", 6894065, Common::FR_FRA),
	GAME_ENTRY_LANG("newcity", "New Cityop.exe", "477399157ef0e3b5c6bf621cabec0074", 562651637, Common::ES_ESP),
	GAME_ENTRY_EN("newkidgottasteal", "YoNewKid.exe", "7ddb9e776648faed5a51170d087074e9", 7785618),
	GAME_ENTRY_EN("newkidgottasteal", "YoNewKid.ags", "6d6c74ff8c514978c5a5e045d2b3f22b", 5298306),
	GAME_ENTRY_EN("news", "WI-AA.exe", "06a03fe35791b0578068ab1873455463", 29631312),
	GAME_ENTRY_EN("nexttoevil", "Next to Evil.exe", "67c6422982dfeb22ebbb53ed8469b907", 4279804),
	GAME_ENTRY_EN("nexttoevil", "Next to Evil.exe", "67c6422982dfeb22ebbb53ed8469b907", 4279802),
	GAME_ENTRY_EN("nickitandrun", "NIAREnglish.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 14180082),
	GAME_ENTRY_LANG("niemandsland", "Das Niemandsland.exe", "430eaebb21d406061d67a9972ad33947", 14852973, Common::DE_DEU),
	GAME_ENTRY_EN("nightandday", "Night and Day.exe", "430eaebb21d406061d67a9972ad33947", 17187367),  //v1.0
	GAME_ENTRY_EN("nightandday", "Night and Day.exe", "430eaebb21d406061d67a9972ad33947", 17206287),  //v1.1
	GAME_ENTRY_EN("nightandday", "Night and Day.exe", "430eaebb21d406061d67a9972ad33947", 17209105),  //v1.2
	GAME_ENTRY_EN("nighthag", "NHS.exe", "f2332e54784086e5a2f249c1867897df", 8776852),
	GAME_ENTRY_EN("nightofthetesticle", "testicle2.exe", "18b284c22010850f79bc5c20054a70c4", 145543486),
	GAME_ENTRY_EN("nightowl", "LD51.exe", "dbe9bab672130b9a84925cb6da64a3db", 33871100),
	GAME_ENTRY_EN("nightowl", "LD51.ags", "475ee6971bd15088ab7c4271ef6dccfd", 30762728),
	GAME_ENTRY_EN("nightwatch", "NWATCH.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 2451134),
	GAME_ENTRY_EN("nightwitch", "NightWitch.exe", "9632da1faae3ef66d7ea9dd60cc19440", 28701275),
	GAME_ENTRY_EN("nightwitch", "NightWitch.ags", "220d7fcf6fc747f1861265c338ff3fdd", 25658951),
	GAME_ENTRY_EN("nightwitches", "Night Witches.exe", "c1feaa2d7845fbe03d9791e4cae121b3", 245899448),
	GAME_ENTRY_EN("nightwitches", "Night Witches.ags", "a59c9ce1f40bb5aa71d27dd2d5744170", 242868388),
	GAME_ENTRY_EN("nightwork", "Nightwork.exe", "f10516e88ec858700804ee69d041aead", 23059420),
	GAME_ENTRY_LANG("ninatonnerre", "NinaTonnerre.exe", "a524cbb1c51589903c4043b98917f1d9", 14361859, Common::FR_FRA),
	GAME_ENTRY_EN("noactionjackson", "current.exe", "3128b9f90e2f954ba704414ae854d10b", 28343366),
	GAME_ENTRY_EN("noahsquest", "Noah's Quest.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 107188362),
	GAME_ENTRY_EN("noahsquest", "Noah's Quest.ags", "c6809578c489733b6e1c3f988a167adc", 104708218),
	GAME_ENTRY_EN("nobodycares", "Nobody Cares.exe", "618d7dce9631229b4579340b964c6810", 20897642),
	GAME_ENTRY_EN("nodriver", "Twilight Zone Lost Episode #1.exe", "6b1702aec6d0a44bda59ef8f229956c5", 8518958),
	GAME_ENTRY_EN("nodriver", "Twilight Zone Lost Episode #1.ags", "21ae973bb15770b1e72ce3a9501bbf81", 6033182),
	GAME_ENTRY_EN("nofear", "Where No Fear Was.exe", "f35829c1b06e878863bd10084593a2c9", 153491794),
	GAME_ENTRY_EN("noiamspartacus", "spartacus.exe", "28f82e420b82d07651b68114f90223c8", 1133879),
	GAME_ENTRY_EN("noisymountain", "NoisyMountainE.exe", "465f972675db2da6040518221af5b0ba", 8031142),
	GAME_ENTRY_EN("nomonkeysbanana", "NMB.exe", "36f44e064eab15e502caeb60fd09f52d", 2750366),
	GAME_ENTRY("norbisquest", "Norbi's quest.exe", "3128b9f90e2f954ba704414ae854d10b", 4912333),  // Eng-Hun
	GAME_ENTRY("norbisquest15", "Norbi 1,5.exe", "f120690b506dd63cd7d1112ea6af2f77", 5455598),  // Eng-Hun
	GAME_ENTRY("norbisquest2", "Norbi 2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 14114728),  // Eng-Hun
	GAME_ENTRY("norbiwinterspecial", "NWS.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 10149860),  // Eng-Hun
	GAME_ENTRY("normalday", "Normal.exe", "465f972675db2da6040518221af5b0ba", 2401147),  // Eng-Rus
	GAME_ENTRY_EN("normancooks", "NORMAN.exe", "4d17844029d8910fbaae1bdc99e250f2", 9397734),
	GAME_ENTRY_EN("norserunereader", "NorseRuneReader.exe", "f120690b506dd63cd7d1112ea6af2f77", 2111273),
	GAME_ENTRY_EN("nosferatu", "Nos.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1680823),
	GAME_ENTRY_EN("nosunrise", "subway.exe", "618d7dce9631229b4579340b964c6810", 3598671),
	GAME_ENTRY_EN("notebookdetective", "Notebook Detective.exe", "0564de07d3fd5c16e6947a647061913c", 248302813),
	GAME_ENTRY_EN("notetoself", "elandra_nts.exe", "7e706ca442dd1277f09480b9ba08a1ec", 7176224),
	GAME_ENTRY_EN("notexplainable", "not the explanable.exe", "82da2565c456dcfb265ded6fe3189c0b", 125719833),
	GAME_ENTRY_EN("notfine", "NotFine.exe", "615e73fc1874e92d60a1996c2330ea36", 4974090),
	GAME_ENTRY_EN("notmyron", "NotMyRON.ags", "bccd5c0a82d1c61d9adc57089277a83a", 2593335),
	GAME_ENTRY_EN("noughtscrosses", "Noughts.exe", "615e73fc1874e92d60a1996c2330ea36", 2159904),
	GAME_ENTRY_EN("nsfware", "NSFWare.exe", "495d45fb8adfd49690ae3b97921feec6", 300944576),  // Windows
	GAME_ENTRY_EN("nsfware", "NSFWare Arcade Version.exe", "495d45fb8adfd49690ae3b97921feec6", 300939605),  // Windows
	GAME_ENTRY_EN("nsfware", "NSFWare Arcade Version.exe", "495d45fb8adfd49690ae3b97921feec6", 300940923),  // Windows itch.io latest
	GAME_ENTRY_EN("nsfware", "NSFWare.ags", "137f7ca599ccb12cf8cdd33a9c0569f3", 298463920),  // Linux
	GAME_ENTRY_EN("nsfware", "NSFWare Arcade Version.ags", "96d858c2a55011b6ef847d35d31816f7", 298458949),  // Linux
	GAME_ENTRY_EN("nsfware", "NSFWare Arcade Version.ags", "ff5e0639e60a09b037f271644cdafe8b", 298460267),  // Linux itch.io latest
	GAME_ENTRY_EN("ntgtfoi", "NTGTFOI.exe", "ed54d3b6e0d4532028d1cbc83c397e5f", 60697898),
	GAME_ENTRY_EN("nukemdukem", "Nukem.exe", "465f972675db2da6040518221af5b0ba", 2112440),
	GAME_ENTRY_EN("numberrescue", "NumberRescue.exe", "0241777c2537fc5d077c05cde10bfa9f", 6442933), //v1.07
	GAME_ENTRY_EN("numberrescue", "NumberRescue.exe", "495d45fb8adfd49690ae3b97921feec6", 6763067), //itch.io Rerelease Win
	GAME_ENTRY_EN("numberrescue", "NumberRescue.ags", "0c9088a91e81bc8e31b371152037f8c6", 4282411), //itch.io Rerelease Linux
	GAME_ENTRY_EN("oakleygame", "OakleysGame.exe", "1177e20f6a84ded5fad2a5d79fff8cb5", 7263137),  // Win
	GAME_ENTRY_EN("oakleygame", "OakleysGame.ags", "534df60761036f1d0696f713ec34380a", 4094349),  // Linux
	GAME_ENTRY_EN("obesebob", "hourgamebab.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 1956717),
	GAME_ENTRY_EN("objectroom", "Object Room.exe", "7825fa980fcd058caadaf6d3377b1f78", 59109472),
	GAME_ENTRY_EN("objectroom", "Object Room.ags", "53736db747af38767b86805ce53db60f", 56073804),
	GAME_ENTRY_EN("obsession", "Obsession.exe", "387ff720e746ae46e93f463fd58d77a4", 12874285), //v1.0.3
	GAME_ENTRY_EN("obsession", "Obsession.ags", "c3c9c5e7446541a79ed010cc0e0bb860", 10382365), //v1.0.3
	GAME_ENTRY_EN("odottamaton", "Odottamaton.exe", "9d9e5ea323793fc526b1533d78c4f9c6", 23814848),
	GAME_ENTRY_EN("odottamaton", "Odottamaton.exe", "9d9e5ea323793fc526b1533d78c4f9c6", 23822268),
	GAME_ENTRY_EN("odow", "ODOW.exe", "6e6f33162242d74dd1fa042429f16562", 3968102),
	GAME_ENTRY_EN("odr3", "ODR3.exe", "af11e94f91b66e11450e8daf51d09292", 7637876),
	GAME_ENTRY_EN("odr4", "ODR4.exe", "510be04e5eec2aa92bb78b060f138d64", 24227828),
	GAME_ENTRY_EN("officeolympics", "Oo.exe", "465f972675db2da6040518221af5b0ba", 30378663),
	GAME_ENTRY_EN("officereturned", "OfficeofTheReturned.ags", "fda695d784c029b3b13a63c7ee989d9a", 3722245),  // AdvJam
	GAME_ENTRY_EN("officereturned", "OfficeofTheReturned.ags", "551c685d6586066092ff51ec20f96be9", 3744375),  // v1.1
	GAME_ENTRY_EN("officeshenanigans", "OfficeShenanigans.exe", "25976a689b0f4d73eac69b1728377ecb", 6111931),
	DETECTION_ENTRY_GUIO("offtheclock", "Off The Clock.ags", "8918895c5c695bae90af13ea4f1a9807", 495345700, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), ADGF_NO_FLAGS, 0),  // Linux
	DETECTION_ENTRY_GUIO("offtheclock", "Off The Clock.exe", "b2eb107c1f9784b7fc4f58c2a7678dea", 498392456, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), ADGF_NO_FLAGS, 0),  // Win
	DETECTION_ENTRY_GUIO("offtheclock", "ac2game.dat", "b2eb107c1f9784b7fc4f58c2a7678dea", 498257938, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), ADGF_NO_FLAGS, 0),  // Mac
	GAME_ENTRY("ohdulieber", "Augustin.exe", "b142b43c146c25443a1d155d441a6a81", 13477896),  // En-Fr-De
	GAME_ENTRY_EN_PLATFORM("ohdulieber", "Augustin.exe", "b5c41e08919834b549ca350a28f18f87", 12576785, "MAGS"),
	GAME_ENTRY_EN("ohitschristmas", "soi_c.exe", "0241777c2537fc5d077c05cde10bfa9f", 17522925),
	GAME_ENTRY_EN("ohitschristmas", "soi_c.exe", "0241777c2537fc5d077c05cde10bfa9f", 17524490),
	GAME_ENTRY_EN("ohnonotagain", "ONNA.exe", "17aed5910365e6b114d667febf8ada7f", 1677534),
	GAME_ENTRY_EN("oldmansea", "OldMan.exe", "615e73fc1874e92d60a1996c2330ea36", 2244474),
	GAME_ENTRY_EN("olol", "1hour.exe", "97d700529f5cc826f230c27acf81adfd", 759002),
	GAME_ENTRY_EN("omfgparadox", "PARATOX.exe", "0710e2ec71042617f565c01824f0cf3c", 903982),
	GAME_ENTRY_EN("omnipotenttarot", "OmnipotentTarot.exe", "f120690b506dd63cd7d1112ea6af2f77", 7166774),
	GAME_ENTRY_EN("onceuponacrime", "OUAC.exe", "6ee842f73649ced615c44d4eb303687c", 6580658),
	GAME_ENTRY_EN("onceuponatime", "Oncetime.exe", "18b284c22010850f79bc5c20054a70c4", 11633390),  // v2
	GAME_ENTRY_EN("onceuponatime70s", "funky.exe", "1e724c14c98a833481dec98800c1b44c", 3926226),
	GAME_ENTRY_EN("onceuponatimekeel", "MAGS TEST.exe", "615e73fc1874e92d60a1996c2330ea36", 3004625),
	GAME_ENTRY_EN("one", "one.exe", "0710e2ec71042617f565c01824f0cf3c", 53482630),
	GAME_ENTRY_EN("oneofthem", "One_of_them.exe", "f18443f98fd61d2c655e76a17f7da905", 11052036),  // Win/Mac
	GAME_ENTRY_EN("oneofthem", "One_of_them.ags", "67af54ac6f3b9729e8a12f45b20fb098", 8909409),  // Linux
	GAME_ENTRY_EN("onemorefathom", "AGS_OMF.exe", "9b47600b5662d7f8b6a4c6ed6854a12c", 24492467),
	GAME_ENTRY_EN("oneofakind", "OneOfAKind.exe", "a524cbb1c51589903c4043b98917f1d9", 4031186),
	GAME_ENTRY_EN("onerainyday", "gewitter.exe", "82da2565c456dcfb265ded6fe3189c0b", 8409861),
	GAME_ENTRY("onerainyday", "OneRainyDay.exe", "c3b72132686c4a102b3365d380c8ae3a", 9749023),  // En-It
	GAME_ENTRY_EN_PLATFORM("oneroom", "One Room.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 14331940, "OROW"),
	GAME_ENTRY_EN("oneroom", "One Room.exe", "7a5f69f3034dc2b629ee53bd7ee5f95b", 21431873),  // v2
	GAME_ENTRY_EN("oneroomonecheese", "One Cheese.exe", "c4f5b7b29be90ba0f8128298afb917de", 3698736),
	GAME_ENTRY_EN("onespytoomany", "One spy too many.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 70521403),
	GAME_ENTRY_EN("onespytoomany", "One spy too many.ags", "8adaee00eac1982da79aedb33002ce96", 68041259),
	GAME_ENTRY_EN("onethatremains", "TOR.exe", "7ddb9e776648faed5a51170d087074e9", 22754797),  // v1.4 Win
	GAME_ENTRY_EN("onethatremains", "TOR.ags", "bea02edae2cd3e37540dff0480f4f2e6", 20267485),  // v1.4 Linux
	GAME_ENTRY_EN("oneweekoneroom", "1 week 1 room.exe", "06a03fe35791b0578068ab1873455463", 4275934),
	GAME_ENTRY_EN("onleavingthebuilding", "On Leaving The Building.exe", "fd68fced8b89792d2e90be87b33d4b19", 64776995),
	GAME_ENTRY_EN("onlythegooddieyoung", "OtGDY_En.exe", "87ccd318a469128530699388f916b86f", 153980124),
	GAME_ENTRY_EN("openquest", "OpenQuest.exe", "90413e9ae57e222f8913b09d2bc847bc", 3407165),
	GAME_ENTRY_EN("operationforklift", "Pancake.exe", "c9cd3e2cdd295c33e62cfa97b14461e9", 35971047),
	GAME_ENTRY_EN("operationnovi", "Operation Novi.exe", "465f972675db2da6040518221af5b0ba", 24583968),
	GAME_ENTRY_EN("operationreddice", "Red Dice.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 7858840),
	GAME_ENTRY_EN("operationreddice", "Augmags.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 7858840),
	GAME_ENTRY_EN("operationsavebluecup", "OSBC.exe", "f120690b506dd63cd7d1112ea6af2f77", 3040865),
	GAME_ENTRY_EN("orangeman", "Orange man.exe", "c38d6846c402bfcdedcf5c34cd8bc3ee", 1960932),
	GAME_ENTRY_EN("osd2", "OSD.TLW2.exe", "67a2bba344cd1c2df32e785800c04929", 80360684),  // v2.0
	GAME_ENTRY_EN_PLATFORM("osd2", "OSD.TLW2.exe", "a80ae4d0aea21508f1df9d18f2839bcb", 22076715, "Beta"),
	GAME_ENTRY_EN("osd3d", "OSD.3D.exe", "ade2e3bec80e9d26ef195e515cecc01e", 21843606),
	GAME_ENTRY_EN_PLATFORM("osdanish", "OSDanish.exe", "4fdacfd1e49508d3b12f9414a375067f", 8551119, "MAGS"),  // v1.1
	GAME_ENTRY("osdanish", "OSDanish.exe", "4fdacfd1e49508d3b12f9414a375067f", 8552024),  // v1.2.0 Eng-Esp
	GAME_ENTRY("osdanish", "Oceanspirit Danish.exe", "4fdacfd1e49508d3b12f9414a375067f", 8555600),  // v1.2 Win/Linux En-De-Es
	GAME_ENTRY_EN("osdarayofhope", "ray_of_hope.exe", "25976a689b0f4d73eac69b1728377ecb", 6595789),
	GAME_ENTRY_EN("osdarayofhope", "ray_of_hope.exe", "25976a689b0f4d73eac69b1728377ecb", 6596689),
	GAME_ENTRY_EN("osdarchipelago", "OSD Archipelago.exe", "510be04e5eec2aa92bb78b060f138d64", 131491224),
	GAME_ENTRY_EN("osddeath", "DeathOfOSD.exe", "510be04e5eec2aa92bb78b060f138d64", 19600168),
	GAME_ENTRY_EN("osdenise", "OSDHITNOT.exe", "82da2565c456dcfb265ded6fe3189c0b", 4471818),
	GAME_ENTRY_EN("osdfamous", "OSD Famous.exe", "4e2abe054d3f40473fc1786a9f76e58e", 2569942),
	GAME_ENTRY_EN("osdholiday", "holidayhavoc.exe", "a524cbb1c51589903c4043b98917f1d9", 3938420),
	GAME_ENTRY_EN("osdlastboss", "Oceanspirit Dennis.exe", "615e73fc1874e92d60a1996c2330ea36", 3581898),
	GAME_ENTRY_EN_PLATFORM("osdlastboss", "Oceanspirit Dennis.exe", "615e73fc1874e92d60a1996c2330ea36", 3713959, "DX"),
	GAME_ENTRY_EN("osdlol", "lol, Oceanspirit Dennis.exe", "615e73fc1874e92d60a1996c2330ea36", 2343043),
	GAME_ENTRY_EN("osdlostworld", "OSD.TLW.exe", "67c6422982dfeb22ebbb53ed8469b907", 47977264),
	GAME_ENTRY_EN("osdmancake", "OSD Birthday.exe", "510be04e5eec2aa92bb78b060f138d64", 69824389),
	GAME_ENTRY_EN("osdmightypirate", "Oceanspirit Dennis - Mighty Pirate.exe", "60394aca1e046aacd53ce9540814c4ea", 2370180),
	GAME_ENTRY_EN("osdmightyviking", "OSD-MIGHTYVIKING.exe", "804add6fbbb791b7be195dc0097434a1", 4782522),
	GAME_ENTRY_EN("osdmoby", "OSD Moby.exe", "615e73fc1874e92d60a1996c2330ea36", 35355373),
	GAME_ENTRY_EN("osdninja", "OSD Ninjas.exe", "510be04e5eec2aa92bb78b060f138d64", 176644020),
	GAME_ENTRY_EN("osdocd", "OSD-OCD.exe", "9dbf699a0e41550bb080813a233e73a8", 47758168),
	GAME_ENTRY_EN("osdpoop", "OSD Poopdeck.exe", "510be04e5eec2aa92bb78b060f138d64", 10502790),
	GAME_ENTRY_EN("osdprincess", "OSDPrincess.exe", "0241777c2537fc5d077c05cde10bfa9f", 18091230),
	GAME_ENTRY_EN("osdrpg", "Oceanspirit Dennis.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3195930),
	GAME_ENTRY_EN("osdscourge", "Oceanspirit Dennis.exe", "18456f28d9bf843b087e80072c85beca", 2419770),
	GAME_ENTRY_EN_PLATFORM("osdscourge", "Oceanspirit Dennis Source.exe", "18456f28d9bf843b087e80072c85beca", 4675107, "DX"),
	GAME_ENTRY_EN("osdscourgehd", "OSD HD.exe", "e80586fdc2db32f65658b235d8cbc159", 8355557),
	GAME_ENTRY_EN("osdsearch", "OSDSearch.exe", "c2e87de9507f0443368b300e8c882f30", 37505243),
	GAME_ENTRY_EN("osdshindig", "TSOD.exe", "615e73fc1874e92d60a1996c2330ea36", 3399768),
	GAME_ENTRY_EN("osdsquares", "Oceanspirit Dennis.exe", "615e73fc1874e92d60a1996c2330ea36", 2077415),
	GAME_ENTRY_EN("osdsweet", "OSD_LPR_3rd.exe", "c0a8c43c14bbf5f407318e8bca2b0fec", 4249689),
	GAME_ENTRY_EN_PLATFORM("osdtextual", "textual.exe", "e8cbdd7295ef5c361d7a4b47ed58a1fd", 2289508, "MAGS"),
	GAME_ENTRY_EN("osdtextual", "textual.exe", "256752c9a97b4780fc5e6f3239c8cdf1", 2655808),  // v1.3
	GAME_ENTRY_EN("osdvaginity", "OSDLHV.exe", "615e73fc1874e92d60a1996c2330ea36", 2154722),
	GAME_ENTRY_EN("osdvscloud", "DvsC.exe", "e80586fdc2db32f65658b235d8cbc159", 21550013),
	GAME_ENTRY_EN("osdvsron", "RON vs OSD.exe", "7e7d6148fa1efad740c664c4f68f4246", 15930882),
	GAME_ENTRY_EN("osdwetspot", "The Wet Spot.exe", "510be04e5eec2aa92bb78b060f138d64", 13699948),
	GAME_ENTRY_LANG("osher", "Osher.exe", "18b284c22010850f79bc5c20054a70c4", 389177994, Common::HE_ISR),
	GAME_ENTRY_EN("ossuarium", "Ossuarium.exe", "027d708642a97398eef18f83d2f2014b", 138816038),  // Windows
	GAME_ENTRY_EN("ossuarium", "Ossuarium Mac.ags", "0751be81595883d67a986db51dacd070", 135730902),  // Linux
	GAME_ENTRY_EN("ossuarium", "ac2game.dat", "cafd9003e9b343ee3da52863bec4a00c", 138868970),  // Mac
	GAME_ENTRY_EN("otakurivals", "Kintaro.exe", "465f972675db2da6040518221af5b0ba", 4045893),
	GAME_ENTRY_EN("otherworlds", "Other Worlds.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 10669659),
	GAME_ENTRY_LANG("otherworlds", "Other Worlds I.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 10709710, Common::IT_ITA),
	GAME_ENTRY_EN("otisbuildsafire", "obaf.exe", "c8d330b58da0a00a136c52ed3a20ae23", 4260705),
	GAME_ENTRY_EN("ourfinesthour", "OFH.exe", "72a1e963da14255d2b7523133f7147d9", 13617005),
	GAME_ENTRY_EN("ouroborossacrifice", "Ouroboros 1.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 109618879),  // v1.6
	GAME_ENTRY_EN("ourobouros", "I am Ourobouros.exe", "615e73fc1874e92d60a1996c2330ea36", 5841551),
	GAME_ENTRY_LANG("ourobouros", "I am Ourobouros - French.exe", "615e73fc1874e92d60a1996c2330ea36", 5843873, Common::FR_FRA),  // French
	GAME_ENTRY_EN("outbreak", "outbreak.exe", "0500aacb6c176d47ac0f8158f055db83", 3271348),
	GAME_ENTRY_EN("outbreakwarehouse", "OutbreakWarehouse.exe", "7971a7c02d414dc8cb33b6ec36080b91", 66202988),
	GAME_ENTRY_EN("outbreakwarehouse", "OutbreakWarehouse.ags", "278fc0ec0486cfd6de9326d46d82c14a", 63705948),
	GAME_ENTRY_EN("outlawgold", "Game.exe", "010029f483da2ba0a34c937ea9fd00a3", 108464409),
	GAME_ENTRY_EN("outofgas", "OutOfGas.exe", "6b4ceb9e327ac99479c08d825461f4cb", 18187986),
	GAME_ENTRY_EN("outofgas", "OutOfGas.exe", "f0e74b3673867316e1cb3120c4289c66", 18200877),  // v1.2
	GAME_ENTRY_EN("outtajuice", "Outta' Juice.exe", "0241777c2537fc5d077c05cde10bfa9f", 6549919),
	GAME_ENTRY_EN("overroger", "Over Roger.exe", "939eb39fce13405d53501bd6551c63c1", 64883691),
	GAME_ENTRY_EN("overtheedge", "jdags.exe", "05f7ff300b322bc431e7cda6a07b5976", 235453945),  // v1.0
	GAME_ENTRY_EN("overtheedge", "jdags.exe", "05f7ff300b322bc431e7cda6a07b5976", 235453751),  // v1.01
	GAME_ENTRY_EN("owlhunt", "owlhunt.exe", "09c656a68c53a182ce343f17a26430e1", 3265006),
	GAME_ENTRY_EN("owlsquest", "King.exe", "a6906aec8617b81af44cf9420731fc34", 3506408),
	GAME_ENTRY_EN("palacin", "palacin.exe", "a524cbb1c51589903c4043b98917f1d9", 19835364),
	GAME_ENTRY_EN("palettequest", "Palette Quest.exe", "0710e2ec71042617f565c01824f0cf3c", 754395),
	GAME_ENTRY_EN("palettequest2", "PaletteQuest2.exe", "2b7ae26333ba15529a8bbbdb49605133", 771805),
	GAME_ENTRY_EN("pandainspace", "pandasp.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 14340176),
	GAME_ENTRY_EN("pandor", "Pandor.exe", "a6dc66da890952431371b62659e58a62", 18995824),
	GAME_ENTRY_EN("pandor", "Pandor.exe", "a6dc66da890952431371b62659e58a62", 22741594),  // v0.1.5
	GAME_ENTRY("paperkubik", "PaperKubik.exe", "33c2a53cb4a4ae48787d42980852047b", 2659023),  // Eng-Rus
	GAME_ENTRY_EN_PLATFORM("paperplanes", "Last'n'Furious.exe", "8aff96231ca059cd61fe5ce68b83f50f", 39995017, "MAGS"),  // v0.2
	GAME_ENTRY("paperplanes", "Paper Planes.exe", "c3169a1c681007b95f517f765a139b89", 83144027),  // v1.0  En-Fr
	GAME_ENTRY_EN("paradiselost", "larywilc.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 9061068),  //v2.006
	GAME_ENTRY_EN("parameciumcomplex", "paramaecium.exe", "0500aacb6c176d47ac0f8158f055db83", 2805720),
	GAME_ENTRY_EN("paranoid", "GEEK!.exe", "949f7440e3692b7366c2029979dee9a0", 2502257),
	GAME_ENTRY_EN_PLATFORM("paranormalinvestigation", "ronpi.exe", "06a03fe35791b0578068ab1873455463", 2908717, "2007 Remake"),
	GAME_ENTRY_EN_PLATFORM("paranormalinvestigation", "ronpi.exe", "f8e8f781a4a95c2bfea5f54b085df550", 3830361, "2014 Remake"),
	GAME_ENTRY_EN("party", "Party.exe", "f120690b506dd63cd7d1112ea6af2f77", 1888987),
	GAME_ENTRY_EN("patchwork", "Patchwork.exe", "bbadca125279cb808b4ed0ff4b31448d", 110767860),
	GAME_ENTRY_EN("pathskinwalker", "Skinwalker.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 628697367),
	GAME_ENTRY_EN("pathskinwalker", "Skinwalker.ags", "d110c5fbbff9bce59d06f054d5b8be8a", 625665795),
	GAME_ENTRY("paulainwonderland", "PaulaWunderland.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2249299),  // Eng-Deu
	GAME_ENTRY_EN("paulmooseinspaceworld", "Paul Moose In Space World.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 15971406),
	GAME_ENTRY("paulquest", "calle.exe", "06a03fe35791b0578068ab1873455463", 3055837),  //Eng-Esp
	GAME_ENTRY("paulromano", "Paul Romano.exe", "ee9deee0da02406cf9b7ca076321a106", 844755077),  // v1.0 Windows Eng-Ita
	GAME_ENTRY("paulromano", "Paul Romano.ags", "8af5fa84179b24bc84d78363eb5b0d25", 842261109),  // v1.0 Linux Eng-Ita
	GAME_ENTRY("paulromano", "Paul Romano.exe", "165d69d396898d165a95b29f6e9770db", 845276527),  // v1.1 Android Eng-Ita
	GAME_ENTRY("paulromano", "Paul Romano.ags", "baef5e71ad847c8f728f2f20c15208f0", 840055169),  // v1.2 Win/Linux Eng-Ita
	GAME_ENTRY_EN("peakvalley1", "MOPV 1 The Lost Sonata.exe", "7c3c3d4b607946867e979a484d910ca0", 12249206),
	GAME_ENTRY_EN("peakvalley2", "MOPV 2 The White Lady.exe", "55c5470f14317d5e806cd4f4271a2118", 44869592),
	GAME_ENTRY_EN("peakvalley2", "MOPV 2 The White Lady.exe", "55c5470f14317d5e806cd4f4271a2118", 45221210),  // v1.0.3
	GAME_ENTRY_EN("pendek", "Pendek.exe", "6a2bd70d9b482cf8fcc9f9b99adf704c", 14652428),
	GAME_ENTRY_EN("penguincy", "Penguincy.ags", "fba95583bc891449c5d4ce3d51e7ef5e", 14790192),
	GAME_ENTRY_EN("pennispong", "Pennis.exe", "28f82e420b82d07651b68114f90223c8", 900335),
	GAME_ENTRY_EN("pennispong", "Pennis.exe", "28f82e420b82d07651b68114f90223c8", 939611),
	GAME_ENTRY_EN("pepeadventure", "pepe.exe", "0710e2ec71042617f565c01824f0cf3c", 2211759),
	GAME_ENTRY_EN("perceptions", "Perceptions.exe", "5cca0f8af961d2bc1c6e475437d3b23b", 9831096),
	GAME_ENTRY_EN("perceptions", "Perceptions.ags", "a486ab3f60b44c765c6e43acda8786ee", 6797476),
	GAME_ENTRY_EN("perelman", "Perelman.exe", "615e73fc1874e92d60a1996c2330ea36", 29477673),
	GAME_ENTRY_EN("perelman", "Perelman.exe", "615e73fc1874e92d60a1996c2330ea36", 29554382),
	GAME_ENTRY_EN("perilsofpoom", "Poom.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 10455126),
	GAME_ENTRY_EN("perilsofpoom", "Poom.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 10455152),
	GAME_ENTRY_EN("pesterquest", "PQ.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2381566),
	GAME_ENTRY_EN("petalrose", "petalrose.exe", "949f7440e3692b7366c2029979dee9a0", 742562),
	GAME_ENTRY_EN("petalrose", "petalrose.exe", "949f7440e3692b7366c2029979dee9a0", 748315),
	GAME_ENTRY_EN("petshopincident", "petshop.exe", "64fcaf7da0b257ea831f89c54be0ad72", 3660347),
	GAME_ENTRY_EN("pharmacistjones", "Pharmacist Jones.exe", "00bb363ccb0acfe57893dd3ba6f1719c", 8285817),
	GAME_ENTRY_LANG("philadvch1", "Phil's Adventure v1.1.exe", "1d1deb6497d935c825b0398806e8306e", 40332015, Common::FR_FRA),
	GAME_ENTRY_EN("philococoa", "PLHC.exe", "06a03fe35791b0578068ab1873455463", 1662310),
	GAME_ENTRY_EN("phoenix1", "phoenix1-ags.exe", "a524cbb1c51589903c4043b98917f1d9", 149928937),
	GAME_ENTRY_EN("pickpocketrpg", "Pickpocket RPG.exe", "82da2565c456dcfb265ded6fe3189c0b", 44540638),
	GAME_ENTRY_EN("pie", "pie.exe", "72a1e963da14255d2b7523133f7147d9", 2035765),
	GAME_ENTRY_EN("piginapoke", "Pig in a poke.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 18557183),  //Windows
	GAME_ENTRY_EN("piginapoke", "Pig in a poke.ags", "82bd4d4168ce29836f38d6c78120a707", 15521003),  //Linux
	GAME_ENTRY_EN("pilotlight", "Pilot.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 9707781),
	GAME_ENTRY_EN("pimpinonparakuss", "Squest2.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3717277),
	GAME_ENTRY("pinkcult", "Pink Cult.exe", "615e73fc1874e92d60a1996c2330ea36", 2731074),  // En-Fr
	GAME_ENTRY_EN("pinksky", "Pink_Sky.exe", "79077a68e53562082494933a21e2714f", 43597805),
	GAME_ENTRY_EN("pinkyalien", "alien.exe", "ba27688a81119b49a550f3bbd8d6d003", 778228),
	GAME_ENTRY_EN("piratefry2", "FryTworedo.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 8492745),
	GAME_ENTRY_EN("piratefry3", "tryout.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2318881),
	GAME_ENTRY_EN("pirates", "Pirates!.exe", "0564de07d3fd5c16e6947a647061913c", 81574110),
	GAME_ENTRY("piratescaribbean", "pirates.exe", "f120690b506dd63cd7d1112ea6af2f77", 7927119),  // En-Fr
	GAME_ENTRY_EN("piratess", "Piratess.exe", "f120690b506dd63cd7d1112ea6af2f77", 2043871),
	GAME_ENTRY_EN("piss", "PISS.exe", "e2ad4b60bafc5da468607e026d831230", 102387880),
	GAME_ENTRY_EN("pixel", "Pixel.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 3696945),
	GAME_ENTRY_EN("pixelhunt2005", "pixelhunt.exe", "057d1aa29f6fadd83209268efcfb35a3", 3321338),
	GAME_ENTRY_EN("pixelhunt2007", "Pixel Hunt.exe", "4d7d2addcde045dae6e0363a43f9acad", 2048301),
	GAME_ENTRY_EN("pixia", "Pixia.exe", "0710e2ec71042617f565c01824f0cf3c", 1286189),
	GAME_ENTRY_EN("pixxxelhunter", "pixelhunter.exe", "0710e2ec71042617f565c01824f0cf3c", 1621107),
	GAME_ENTRY_EN_PLATFORM("pizzacalls", "PizzaCalls.exe", "5b916edf70413f20906ceb9c05ac65b9", 4037078, "MAGS"),
	GAME_ENTRY_EN("pizzacalls", "PizzaCalls.exe", "5b916edf70413f20906ceb9c05ac65b9", 4043953),  // v1.01
	GAME_ENTRY_EN("pizzanostra", "PizzaNostra.exe", "f049336af42d365ab16e68d4be5cbe9a", 56205310),
	GAME_ENTRY_EN("plan10frommypants", "plan10.exe", "06a03fe35791b0578068ab1873455463", 8514311),
	GAME_ENTRY_EN("planetxmas", "PlanetX.exe", "64fcaf7da0b257ea831f89c54be0ad72", 4430665),
	GAME_ENTRY_EN("planm", "Plan M.exe", "615e73fc1874e92d60a1996c2330ea36", 4843409),
	GAME_ENTRY_EN_PLATFORM("planm", "Plan M.exe", "e80586fdc2db32f65658b235d8cbc159", 6374421, "Enhanced Edition"),
	GAME_ENTRY_EN("plantsimulator", "PlantSimulator.exe", "f18443f98fd61d2c655e76a17f7da905", 2474380),
	GAME_ENTRY_EN("plasticmines", "plasticmines.exe", "7971a7c02d414dc8cb33b6ec36080b91", 35136616),
	GAME_ENTRY_EN("platformhorde", "Platform Horde.exe", "49157a0ea75b960eded4d0811a71d3e9", 14991450),
	GAME_ENTRY_EN("playitagain", "playit.exe", "f18443f98fd61d2c655e76a17f7da905", 12379664),
	GAME_ENTRY_EN("playoflife", "The Play of Life.ags", "88dfc0fd903fe42c86337ed47fe21d20", 43332985),
	GAME_ENTRY_EN("playxylo", "Xylophone For Kids.exe", "237e51bb11691ef8ec09e23ae307fcc7", 249211663),
	GAME_ENTRY_EN("pledgequest1", "svpromo.exe", "82da2565c456dcfb265ded6fe3189c0b", 3239155),
	GAME_ENTRY_EN("pledgequest2", "pledgequest2.exe", "615e73fc1874e92d60a1996c2330ea36", 27872703),
	GAME_ENTRY_EN("pleurghburgdarkages", "Pleurgh.exe", "9cf51833e787cc919837d9a8bd8fc14c", 11555983),
	GAME_ENTRY_EN("plumberboy", "Plumberboy.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 4169240),
	GAME_ENTRY_EN("pmquestions", "pmqs.exe", "615e73fc1874e92d60a1996c2330ea36", 7376095),
	GAME_ENTRY_EN("pmuvchvt", "PMUVCHVT.exe", "06a03fe35791b0578068ab1873455463", 5127419),
	GAME_ENTRY_EN("pocketfluff", "Pketfluf.exe", "0b7529a76f38283d6e850b8d56526fc1", 10858438),
	GAME_ENTRY_LANG("pocketquest", "Pocket.exe", "465f972675db2da6040518221af5b0ba", 28532586, Common:: RU_RUS),
	GAME_ENTRY_EN("poddwellers", "Pod Dwellers.exe", "9982a80801abee56c9c8977d5e7424d7", 7305546),  // Linux
	GAME_ENTRY_EN("poddwellers", "Pod Dwellers.ags", "593038a65aae682cbcd7be9fe51fa84b", 4196662),  // Windows
	GAME_ENTRY_EN("poeng", "poe.exe", "618d7dce9631229b4579340b964c6810", 29811225),
	GAME_ENTRY_EN("politicallyyours", "Politically Yours.exe", "031bce0ec1a563faffc19a62ea17e35f", 4640266),
	GAME_ENTRY_EN("pompadourpete", "4hg.exe", "615e73fc1874e92d60a1996c2330ea36", 6008514),
	GAME_ENTRY_EN("ponderabilia", "Ponderabilia.exe", "615e73fc1874e92d60a1996c2330ea36", 21206289),
	GAME_ENTRY_EN("pong", "Pong.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 611249),
	GAME_ENTRY_EN("postcardadv", "Postcards.exe", "3bee5e1abe11201757d65a7438ae4dbe", 189938922),
	GAME_ENTRY_EN("postmansquest", "PQ.exe", "345685aaec400c69a1e6d5e18a63850c", 6896450),
	GAME_ENTRY_EN("potatohead", "Potato Head.exe", "173f00e582eebd082d66f1291702b373", 94065732),
	GAME_ENTRY("potionmagique", "La potion magique.exe", "cce3697243955a9e52fa784d43e6ccbc", 300831515),  //En-Fr
	GAME_ENTRY_EN("potionmaster", "mags0822.exe", "0e4ddc9893796a9f39395d0e0220a37b", 22214698),  // v1.1 MAGS
	GAME_ENTRY_EN("potionmaster", "mags0822.ags", "f29c8286bbbf9a0d29635cfe53c8791c", 19074070),
	GAME_ENTRY_LANG("poudlardmotus", "PoudlardMotus2.exe", "c8f9652d9bad7be352d2ca56e0253562", 33751203, Common::FR_FRA),
	GAME_ENTRY_EN("pouvoir", "pouvoir.exe", "0500aacb6c176d47ac0f8158f055db83", 1540761),
	GAME_ENTRY_EN("powernap", "powernap.exe", "615e73fc1874e92d60a1996c2330ea36", 13308487),
	GAME_ENTRY("powerunlimited", "PU Bordspel.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1842762),  //Eng-Deu
	GAME_ENTRY_EN("pqtadventure", "playpqt.exe", "0b7529a76f38283d6e850b8d56526fc1", 19914806),
	GAME_ENTRY_EN("practicescript", "Practice.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 764358),
	GAME_ENTRY_EN("practicescript", "ac2game.ags", "aa939fb3da287382313c0058ccbd18af", 258998),
	GAME_ENTRY_EN_PLATFORM("predatorspreyforplants", "Predators Prey For Plants.exe", "d44551532361c1eeec9b167de35515d1", 2318985,"MAGS"),
	GAME_ENTRY_EN("predatorspreyforplants", "Predators Prey For Plants.exe", "9f25405f49fadacb688eddecae95f775", 2683806),
	GAME_ENTRY_EN_PLATFORM("preludetoadventure", "Prelude.exe", "7db052bc30700d1f30f5330f5814f519", 7072523, "LowRezJam"),  //v0.8
	GAME_ENTRY_EN_PLATFORM("preludetoadventure", "Prelude.exe", "7db052bc30700d1f30f5330f5814f519", 10406930, "MAGS"),
	GAME_ENTRY_EN("preludetoadventure", "Prelude.exe", "7db052bc30700d1f30f5330f5814f519", 12923511),  // v1.0
	GAME_ENTRY_EN("preptime", "Prep time.exe", "8f18ad4456e6dbeaf041cd91449a55ba", 20654963),
	GAME_ENTRY_EN("preptime", "Prep time.ags", "a182adbfbad72a7e03675e27f33176c2", 17618783),
	GAME_ENTRY_EN("priderelatives", "Pride & Relatives.exe", "0564de07d3fd5c16e6947a647061913c", 6368631),
	GAME_ENTRY_EN("primordiaconduit", "Primordia - Conduit Cut.exe", "8f717a5a14ceda815292ce4065348afb", 123436539),
	GAME_ENTRY_EN("princedickless", "Prince_Dickless.exe", "615e73fc1874e92d60a1996c2330ea36", 123697941),
	GAME_ENTRY_EN("princeoflordenp1", "L_P_O_L.exe", "06a03fe35791b0578068ab1873455463", 30467985),
	GAME_ENTRY_EN("princessandallthekingdom", "Copy of PHK.exe", "465f972675db2da6040518221af5b0ba", 24658916),
	GAME_ENTRY_EN("princessmarian1", "marian.exe", "97d700529f5cc826f230c27acf81adfd", 3637811),
	GAME_ENTRY_EN("princessmarian2", "mothersday.exe", "0b7529a76f38283d6e850b8d56526fc1", 1816363),
	GAME_ENTRY_EN("princessmarian3", "birthday.exe", "0b7529a76f38283d6e850b8d56526fc1", 2999411),
	GAME_ENTRY_EN("princessmarian4", "DarkCave.exe", "0d48d8b170624e8f33bd7cd7f3ad2052", 5293954),
	GAME_ENTRY_EN("princessmarian4", "Marian+Dragon.exe", "0d48d8b170624e8f33bd7cd7f3ad2052", 5293954),
	GAME_ENTRY_EN("princessmarian5", "alba_explorer.exe", "0500aacb6c176d47ac0f8158f055db83", 2026147),
	GAME_ENTRY_EN("princessmarian6", "pmvi.exe", "465f972675db2da6040518221af5b0ba", 3038012),
	GAME_ENTRY_EN("princessmarian7", "pm-cute.exe", "28f82e420b82d07651b68114f90223c8", 2728354),
	GAME_ENTRY_EN("princessmarian8", "pmsnow.exe", "f120690b506dd63cd7d1112ea6af2f77", 1250562),
	GAME_ENTRY_EN("princessmarian9", "phantom.exe", "2a486107b3f04f789c122a1d0e1f3d5f", 2067548),
	GAME_ENTRY_EN("princessmarian10", "pmx.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3243258),
	GAME_ENTRY_EN("princessmarian11", "PMXI.exe", "609aa4339aea63a0ebc1fd6a659e6d4f", 3164504),
	GAME_ENTRY_EN("princessmarianspigeonpinger", "PMPP.exe", "e68aaf0cea1e5701a7bb50a757bb5f4b", 2929950),
	GAME_ENTRY_EN("principlesofevil", "P of E.exe", "0500aacb6c176d47ac0f8158f055db83", 43283105),
	GAME_ENTRY_EN("principlesofevil", "P of E.exe", "0500aacb6c176d47ac0f8158f055db83", 44012235),
	GAME_ENTRY_EN("principlesofevil", "P of E.exe", "f120690b506dd63cd7d1112ea6af2f77", 43410398),
	GAME_ENTRY_LANG("principlesofevil", "German.exe", "0500aacb6c176d47ac0f8158f055db83", 43284740, Common::DE_DEU),
	GAME_ENTRY_EN("principlesofevil2", "PoE II.exe", "f120690b506dd63cd7d1112ea6af2f77", 87236507),
	GAME_ENTRY_EN("prisonbreakout", "Prison Breakout Remake 2024.ags", "c0b8b8ddf3e56971e0781586e211a552", 40394767),
	GAME_ENTRY_EN("privatedetective", "PrivateDetective.exe", "9aea96d0ea823f915cd790ca0de92bbe", 22885570),  // Windows
	GAME_ENTRY_EN("privatedetective", "PrivateDetective.ags", "c589d8787feeaa96a9a04e0622388dde", 20289202),  // Linux
	GAME_ENTRY_EN_PLATFORM("procrastinator", "Procrastinator.exe", "82da2565c456dcfb265ded6fe3189c0b", 2134380, "OROW"),
	GAME_ENTRY_EN("procrastinator", "Procrastinator.exe", "82da2565c456dcfb265ded6fe3189c0b", 2135470),  // v1.2
	GAME_ENTRY_EN("prodigal0", "Prodigal 0.exe", "0710e2ec71042617f565c01824f0cf3c", 5006550),
	GAME_ENTRY_EN("prodigal", "Prodigal.exe", "0710e2ec71042617f565c01824f0cf3c", 6734566),
	GAME_ENTRY_EN("prodigal", "Prodigal.exe", "682a8c57c7678c99c1564fd43680f03e", 7486947),  // v1.1
	GAME_ENTRY_EN("prodigalshooter", "Shooter.exe", "682a8c57c7678c99c1564fd43680f03e", 2142619),
	GAME_ENTRY_EN("profneely", "ProfN.exe", "26cfa7bd1be5485e1f0385101b31e43b", 31956037),  // original release
	GAME_ENTRY_EN_PLATFORM("profneely", "ProfN.exe", "26cfa7bd1be5485e1f0385101b31e43b", 3917542, "Anniversary Update"), // v1.1
	GAME_ENTRY_EN("projectevilspy2", "Project Evilspy II.exe", "0b7529a76f38283d6e850b8d56526fc1", 1583899),
	GAME_ENTRY_EN("projectlazarus", "plazarus.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 13219923),
	GAME_ENTRY_EN("projectmadness", "LD40.ags", "c0a6bf580c38bb214da45f8383a62a6a", 6724821),  // Linux
	GAME_ENTRY_EN("projectmadness", "LD40.exe", "b142b43c146c25443a1d155d441a6a81", 9204965),  // Windows
	GAME_ENTRY_EN("proofoffiction", "PoF.exe", "9cf51833e787cc919837d9a8bd8fc14c", 2723885),
	GAME_ENTRY_EN("proposal", "Proposal.exe", "46aaeedf91c1763adc2978cacd9673e5", 4593850),
	GAME_ENTRY("providence", "Providence.exe", "1c00b5d482f5531cb1eba7360b98b3e4", 625922261),  // Eng-Ita
	GAME_ENTRY_EN("psychicsam", "Psychic Sam.exe", "340cc4078ead735cfe5caa8f21967887", 8243263),
	GAME_ENTRY_EN("psychofb", "Psycho Flashback.exe", "a524cbb1c51589903c4043b98917f1d9", 2285929),
	GAME_ENTRY_EN("psychopomp", "psychopomp.exe", "b142b43c146c25443a1d155d441a6a81", 23721476),  // Windows
	GAME_ENTRY_EN("psychopomp", "psychopomp.ags", "98a29b0b5e61d8c756c426b6945c869d", 21241332),  // Linux
	GAME_ENTRY_EN("pubmasterquest2", "shogin crystal.exe", "90baefd2f369cebe25f3aa9ad90332d2", 35191110),
	GAME_ENTRY_EN("pubmasterquestlegends", "PMQ.exe", "9e3b4641b9a6ea476a3de1baf4ff8329", 89211239),
	GAME_ENTRY_EN("pubmasterquestspirits", "PMQ.NO1.exe", "37ca4ea9b327c3c898763e178cb338b2", 23166419),
	GAME_ENTRY_EN("puddypenguin", "Penguin.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2328158),
	GAME_ENTRY_EN("purgatorio", "Purgatorio 0.1.exe", "18b284c22010850f79bc5c20054a70c4", 194293367),
	GAME_ENTRY_EN("purityofthesurf", "Surf.exe", "71ca0d6c1c699595f28a2125948d4a84", 11315703),
	GAME_ENTRY_EN("purposeretired", "Purpose.exe", "8aff96231ca059cd61fe5ce68b83f50f", 182773167),
	GAME_ENTRY_EN("pussiehunt", "kittie.exe", "03b4d6ed1b2f07082ff25367b6631da0", 4422335),
	GAME_ENTRY_EN("puttputtstew", "puttputtstew.exe", "615e73fc1874e92d60a1996c2330ea36", 6755658),
	GAME_ENTRY_EN("puzzlepumice", "MAGS_Jun_2012.exe", "82da2565c456dcfb265ded6fe3189c0b", 13358126),
	GAME_ENTRY_EN("pxenophobe", "xenophobe final.exe", "465f972675db2da6040518221af5b0ba", 78787305),  //v1.0
	GAME_ENTRY_EN("pxenophobe", "ProjXeno.exe", "465f972675db2da6040518221af5b0ba", 79053486),  //v1.1
	GAME_ENTRY_EN("questblackdiamond", "qftbd.exe", "ba27688a81119b49a550f3bbd8d6d003", 632435),
	GAME_ENTRY_EN("questfighter", "Quest Fighter.exe", "21fd0f65dfa48de2b39cb8ec23b30889", 2914128),
	GAME_ENTRY_EN("questfighter2", "Quest Fighter 2.exe", "4d7d2addcde045dae6e0363a43f9acad", 5219511),
	GAME_ENTRY_EN("questforcinema", "Questforcinema.exe", "465f972675db2da6040518221af5b0ba", 2670632),
	GAME_ENTRY_EN("questforcinema", "Questforcinema.exe", "465f972675db2da6040518221af5b0ba", 2670563),
	GAME_ENTRY("questforjesus", "QuestForJesus.exe", "495d45fb8adfd49690ae3b97921feec6", 3973088),  // Eng-Deu
	GAME_ENTRY("questfororgy", "qfo1.exe", "465f972675db2da6040518221af5b0ba", 7650106),  // En-De
	GAME_ENTRY_EN("questforthebluecup", "Quest for the Cup.exe", "9cb3c8dc7a8ab9c44815955696be2677", 8760015),
	GAME_ENTRY_EN("questforyeti", "Quest For Yeti.exe", "90413e9ae57e222f8913b09d2bc847bc", 2635580),
	GAME_ENTRY("questforyrolg", "Quest for Yrolg.exe", "c4f5b7b29be90ba0f8128298afb917de", 9388101),  // Multi
	GAME_ENTRY("questforyrolg", "quest for yrolg.exe", "89df481678b2ddc40ecc9f83caa76b89", 9709051), // v1.8
	GAME_ENTRY_STEAM("questforyrolg", "quest for yrolg.ags", "51527ed37f001db741e68b767259c81e", 8214789),
	GAME_ENTRY_STEAM("questforyrolg", "quest for yrolg.ags", "51527ed37f001db741e68b767259c81e", 8214777),
	GAME_ENTRY_STEAM("questforyrolg", "quest for yrolg.ags", "126f3b1dac3d9f26c2c079d6dc96120c", 23134906),
	GAME_ENTRY_EN("quietgame", "shhhh.exe", "618d7dce9631229b4579340b964c6810", 73311624),
	GAME_ENTRY_EN("quimbyquestanewdope", "QQuest.exe", "615e806856b7730afadf1fea9a756b70", 8801878),
	GAME_ENTRY_EN("quiteannoying", "Super Annoying League.exe", "615e73fc1874e92d60a1996c2330ea36", 22513046),  // Windows
	GAME_ENTRY_EN("quiteannoying", "Super Annoying League.ags", "72d55dd28b18091ebc793b653e94f9c2", 20722806),  // Linux
	GAME_ENTRY("raastaja", "Raastaja.ags", "16b19709b5ef271146f9bf7b5963b7d2", 2169987209),  // Eng-Fin
	GAME_ENTRY("raastaja", "Raastaja.ags", "7532f39cd52fbc68c2e6537f8b10197c", 2169987303),  // Eng-Fin
	GAME_ENTRY_LANG("raataja", "Raataja.exe", "f1095aa63e8a3d00d8018d32dbdea918", 494788763, Common::FI_FIN),
	GAME_ENTRY_EN_PLATFORM("rabbiteyemotel", "asylumjam2015.exe", "2ca80bd50763378b72cd1e1cf25afac3", 38579543, "GameJam Build"),
	GAME_ENTRY_EN("rabbiteyemotel", "asylumjam2015.exe", "2ca80bd50763378b72cd1e1cf25afac3", 40528966),  // v0.4.2
	GAME_ENTRY_EN("rabbithill", "Rabbit Hill.exe", "7ce746c19cf3080a9fac568979b2bf9b", 54728125),
	GAME_ENTRY_EN("race", "R.ACE.exe", "06a03fe35791b0578068ab1873455463", 3842314),
	GAME_ENTRY_EN("racingmanager", "raceman.exe", "465f972675db2da6040518221af5b0ba", 15971689),
	GAME_ENTRY_EN("racist", "arcade01.exe", "2bf7a3a5f0a22a140350e29bb1ac7cfb", 142097978), // Windows
	GAME_ENTRY_EN("racist", "arcade01.ags", "5c29a54b60c140190693c664bd21eae2", 139654698), // Linux
	GAME_ENTRY_EN("rackham", "Rackham.exe", "36f44e064eab15e502caeb60fd09f52d", 3672597),
	GAME_ENTRY_EN("rainbowskunk", "PrismMAGS.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 10046750),
	GAME_ENTRY_EN("rainbowtube", "Tube Rainbow.exe", "03c8c45bd00daca1a9d75d1133df5640", 5669127),
	GAME_ENTRY_LANG("rainerlesadv1", "Das Renovierungs-Desaster.ags", "f94c4cf70b779a304c4909fdf5b6512a", 111423406, Common::DE_DEU),
	GAME_ENTRY_EN("rainsnow", "Rain and Snow The Bouncer.exe", "0564de07d3fd5c16e6947a647061913c", 20576318),
	GAME_ENTRY_EN("ralphtheraven", "RalphTheRaven.exe", "0500aacb6c176d47ac0f8158f055db83", 1655198),
	GAME_ENTRY_EN("ramghost", "RAM_Ghost.exe", "f87f3e71d13b5fea0970ef95f1c87abe", 14830699),
	GAME_ENTRY_EN("rampitup", "Ramp It Up.exe", "495d45fb8adfd49690ae3b97921feec6", 115195154),  // Win v1.2.0
	GAME_ENTRY_EN("rampitup", "ac2game.dat", "495d45fb8adfd49690ae3b97921feec6", 115195154),  // Mac
	GAME_ENTRY_EN("rampitup", "Ramp It Up.ags", "5c521a3b4235909e498592f91da65fd7", 112714498),  // Linux
	GAME_ENTRY_EN("ramsesporter", "Ramses Porter and the Relay for Love.exe", "a524cbb1c51589903c4043b98917f1d9", 55490676),
	GAME_ENTRY_EN("rango", "Rango.exe", "618d7dce9631229b4579340b964c6810", 21059129),
	GAME_ENTRY_EN("rapsqlud", "RapSqLud.exe", "615e73fc1874e92d60a1996c2330ea36", 2989387),
	GAME_ENTRY_EN("rapstar15", "rs15.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 13638597),
	GAME_ENTRY("ratchannel", "Rat Channel.exe", "c5d2c54c20cb606519b86d3890ee7fc0", 303946606),  // Eng-Deu
	GAME_ENTRY_EN_PLATFORM("ratplaying", "Rat Playing Game.exe", "6b1702aec6d0a44bda59ef8f229956c5", 16414235, "Beta"),
	GAME_ENTRY_EN_PLATFORM("ratplaying", "Rat Playing Game.ags", "600bdacdcb296799312bece57f6a5625", 13928459, "Beta"),
	GAME_ENTRY_EN("ravench1", "Raven.exe", "620b3d82af532d9550c30b7c69d61600", 2287413),
	GAME_ENTRY_EN("rayandtheguitar", "The Band.exe", "465f972675db2da6040518221af5b0ba", 1518249),
	GAME_ENTRY_EN("raybexter", "Ray Bexter.exe", "d3bc2cf3a0f72f0fbe37d9edbd5ebec3", 5680319),  //v1.0
	GAME_ENTRY_EN("raybexter", "Ray Bexter.exe", "3e8667bab45f2e5d7237db53ab376400", 5930752),  //v1.3
	GAME_ENTRY_EN("raysrods", "Rays Rods.exe", "06a03fe35791b0578068ab1873455463", 7507467),
	GAME_ENTRY_EN("razorsinthenight", "Razors.exe", "0500aacb6c176d47ac0f8158f055db83", 25439808),  //v1.0
	GAME_ENTRY_EN("razorsinthenight", "Razors.exe", "0500aacb6c176d47ac0f8158f055db83", 25442827),  //v2.0
	GAME_ENTRY_EN("rcpd", "RCPD.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 3278756),
	GAME_ENTRY_EN("reactor09", "reactor09.run", "b216ee957dd677023e02d900281a02d6", 13764141),
	GAME_ENTRY_EN("reactor09", "reactor09.exe", "06a03fe35791b0578068ab1873455463", 14591606),  // updated
	GAME_ENTRY_EN("readyeddie", "ready.exe", "070a2b9bd7f54b1b6bc44f30c547d325", 9963495),
	GAME_ENTRY("reagentorange", "Lab.exe", "83d96faa4efefcc9c03c01b9517f23bb", 13377038),  // En-De-Cz
	GAME_ENTRY_EN("realitycheck1", "Reality Check.exe", "82da2565c456dcfb265ded6fe3189c0b", 9184815),  //v1.0
	GAME_ENTRY_EN("realitycheck1", "Reality Check.exe", "82da2565c456dcfb265ded6fe3189c0b", 9208605),  //v1.2
	GAME_ENTRY_EN("realitycheck2", "Reality on the Norm Reality Check 2.exe", "82da2565c456dcfb265ded6fe3189c0b", 9499643),
	GAME_ENTRY_EN("realitycheck3", "RoN Reality Check 3.exe", "82da2565c456dcfb265ded6fe3189c0b", 8386704),
	GAME_ENTRY_EN("realityinthenorm", "Reality-in-the-Norm.ags", "546f5a7467d711f725eb48d0bbaec12e", 4204865),  // Linux
	GAME_ENTRY_EN("realityinthenorm", "Reality-in-the-Norm.exe", "21d7482c8e968857e2653debeaa8a384", 6686033),  // Windows
	GAME_ENTRY("realityinthenorm", "Reality-in-the-Norm V2.ags", "d4dd771b58e8892dbf55001cacd88b71", 4210046),  // Linux v2.0 En-Fr-De-It
	GAME_ENTRY("realityinthenorm", "Reality-in-the-Norm V2.exe", "21d7482c8e968857e2653debeaa8a384", 6691214),  // Windows v2.0 En-Fr-De-It
	GAME_ENTRY_EN("recess", "Recess.exe", "c87aa6377abc18c1a1b2968ae6db08eb", 1941530),
	GAME_ENTRY_EN_PLATFORM("recess", "Recess.exe", "bcfd97b39418162cf628390894902ec8", 2560538, "MAGS"),
	GAME_ENTRY_EN("recess2", "Recess2.exe", "45ab4f29031b50c8d01d10a269f77ff5", 2805921),
	GAME_ENTRY_EN("redbeardsavesron", "rbg.exe", "06a03fe35791b0578068ab1873455463", 15285217),
	GAME_ENTRY_EN("redflaggredux", "Red Flagg.exe", "5901879fafae3812f71b3a2d73bee40e", 79414802),
	GAME_ENTRY_EN("redflaggredux", "Red Flagg.exe", "5901879fafae3812f71b3a2d73bee40e", 79417708),
	GAME_ENTRY_EN("redhotoverdrive", "RHO.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 11718460),
	GAME_ENTRY_EN("redpantsep1", "Redpants - The Princess and the Beanstalk.exe", "c0aab3e02bbaf8468770480079436f61", 28517345),
	GAME_ENTRY_EN("redpantsep2", "DBL.exe", "c0aab3e02bbaf8468770480079436f61", 15658119),
	GAME_ENTRY_EN("reefriversquestforekoban", "Quest for Ekoban.exe", "338fa79960d40689063af31c671b8729", 179342350),
	GAME_ENTRY_EN("rein", "rein.exe", "798691640b168556245d5fc261772fde", 17483005),
	GAME_ENTRY_EN("remotelyinteresting", "RemotelyInteresting_Frame42.exe", "a9093fafca0476d3d486f3ad08665cbe", 23599293),
	GAME_ENTRY_EN("rend", "rend.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 6156428),
	GAME_ENTRY("renuncio", "renuncio.exe", "4d4b6d6e560cf32f440c39f8d3896da5", 3477434),  // v1.1  Eng-Esp
	GAME_ENTRY("renuncio2", "Renuncio2.exe", "bb628c1fdcfdc091ea57430e6c4af2ac", 8533739),  //v1.1  Eng-Esp
	GAME_ENTRY_EN("reonquestep1", "Draco'sQuest.exe", "615e73fc1874e92d60a1996c2330ea36", 51243744),
	GAME_ENTRY_EN("reonquestep2", "Bully Island.exe", "615e73fc1874e92d60a1996c2330ea36", 58122181),
	GAME_ENTRY_EN_PLATFORM("repossessor", "repossesor.exe", "06a03fe35791b0578068ab1873455463", 3331695,"2007 release"),  // 2007 version
	GAME_ENTRY_EN_PLATFORM("repossessor", "repossessor.exe", "f8e8f781a4a95c2bfea5f54b085df550", 4037461,"2014 release"),  // 2014 version
	GAME_ENTRY_EN("requiem", "Requiem.exe", "615e73fc1874e92d60a1996c2330ea36", 3497044),
	GAME_ENTRY_EN("researchreservations", "Research Reservations.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 4801820),
	GAME_ENTRY_EN("researchresident", "ReRes.exe", "9cd9563150a69bc1d4eacda0ff3557f8", 26359896),
	GAME_ENTRY_EN("returnjourney", "return.exe", "0564de07d3fd5c16e6947a647061913c", 72875118),
	GAME_ENTRY_EN("returntocivilization", "ReturnTo.exe", "0710e2ec71042617f565c01824f0cf3c", 3280888),
	GAME_ENTRY_EN("revelation", "Revelation.exe", "06a03fe35791b0578068ab1873455463", 14512328),
	GAME_ENTRY_EN("rickyquest", "black.exe", "06a03fe35791b0578068ab1873455463", 3250100),
	GAME_ENTRY("righteouscityp1", "RC PI - 3.00.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 19786840),  // Eng-Ita
	GAME_ENTRY("righteouscityp2", "RCII.exe", "06a03fe35791b0578068ab1873455463", 39085407),  // Eng-Ita
	GAME_ENTRY_EN("ripperjack", "ripper_jack_v0.1.exe", "c3b72132686c4a102b3365d380c8ae3a", 2580361),
	GAME_ENTRY_LANG("rnbquest", "R`n`B Quest.exe", "465f972675db2da6040518221af5b0ba", 119895894, Common::RU_RUS),
	GAME_ENTRY_EN("roadbrollywood", "brollywood.exe", "615e73fc1874e92d60a1996c2330ea36", 3118294),
	GAME_ENTRY_EN("roadofdestiny", "ROD.exe", "618d7dce9631229b4579340b964c6810", 30127308),
	GAME_ENTRY_EN("roadracer", "TR_Bryvis.exe", "cebb3ac5c3d2df939e7f0ec8f2975b64", 25080647),
	GAME_ENTRY_EN("roastmothergoose", "RMG.exe", "00328f4f1e7729144483107b96b11df8", 46474982),
	GAME_ENTRY_EN_PLATFORM("roastmothergoose", "RMG.exe", "00328f4f1e7729144483107b96b11df8", 46474953, "MAGS"),
	GAME_ENTRY_EN_PLATFORM("robbingtheprincess", "Princess.exe", "ac461eb75959761fe159917607c246b4", 5755468, "MAGS"),  // v1.1
	GAME_ENTRY_EN("robbingtheprincess", "Princess.exe", "ac461eb75959761fe159917607c246b4", 5793384),  // v1.16
	GAME_ENTRY_EN("robertredford1", "GAME.exe", "02635a77ab660023f59519c91329f7f5", 6537985),
	GAME_ENTRY_EN("robertredford2", "ROBERT2.exe", "02635a77ab660023f59519c91329f7f5", 13075066),
	GAME_ENTRY_EN("robertredford3", "game.exe", "71ca0d6c1c699595f28a2125948d4a84", 10846423),
	GAME_ENTRY_EN("robertredford3ds", "dreamseq.ags", "97d700529f5cc826f230c27acf81adfd", 4724470),
	GAME_ENTRY_EN("robmassacreofchainsawness", "Chainsaw.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1153384),
	GAME_ENTRY_EN("robolution", "Robolution.exe", "e299aa402e6d0f16a4e2dbb8c395b52e", 130836838),
	GAME_ENTRY("roboquest", "RoboQuest 1.2.exe", "338fa79960d40689063af31c671b8729", 22776964),  // En-De-Es-Ru
	GAME_ENTRY_LANG("roboquest2009", "ROBO.exe", "465f972675db2da6040518221af5b0ba", 16227790, Common::RU_RUS),
	GAME_ENTRY("robotragedy", "Robotragedy.exe", "465f972675db2da6040518221af5b0ba", 130585260),  // Eng-Esp
	GAME_ENTRY_EN("robotragedy2", "Robotragedy 2.exe", "465f972675db2da6040518221af5b0ba", 256955387),
	GAME_ENTRY_EN("robotsdream", "Robot.exe", "36f44e064eab15e502caeb60fd09f52d", 139573270),
	GAME_ENTRY("roccioquest", "RoccioQuest.exe", "339efe6be15f7bfe779a483dbdbb3048", 5790096),  // Multi
	GAME_ENTRY("roccioquest", "RoccioQuest.exe", "7727bf5360b00bfc1947455218137803", 5797284),
	GAME_ENTRY_EN("rockabillykid", "Rockabilly Kid.exe", "ff3358d8f2726d544aadfde4f1ec8407", 2650305),
	GAME_ENTRY_EN("rockatruestory", "RON-Rock.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3917056),
	GAME_ENTRY_EN("rockburgerstreehouses", "RBTH.exe", "88cf59aad15ca331ab0f854e16c84df3", 1876674),
	GAME_ENTRY_EN("rockpaperscissors", "Rock, Paper, Scissors.exe", "615e73fc1874e92d60a1996c2330ea36", 2030667),
	GAME_ENTRY_EN("rockpaperscissors2", "Rock, Paper, Scissors 2.exe", "89a94326c8afd9e0234e269bd7330130", 2926218),
	GAME_ENTRY_EN("rockrockrock", "rrr.exe", "7dd36aa863ed40ede1b09ae505e478cc", 9362761),
	GAME_ENTRY_EN("rocktravis", "rock travis - camilla's case.exe", "17009da9820f5aa86d0588023d497db8", 126975468),
	GAME_ENTRY_EN("rocktravis", "rock travis - camilla's case.exe", "17009da9820f5aa86d0588023d497db8", 126975803),
	GAME_ENTRY_EN("rockyroams", "Rocky.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 16978200),
	GAME_ENTRY_EN("rodequest2", "RQ2.exe", "12c03a3c782237821acd590fd91af4c5", 4192097),
	GAME_ENTRY_EN("rodequest2", "RQ2b.exe", "12c03a3c782237821acd590fd91af4c5", 4192097),
	GAME_ENTRY_EN("rodequest2", "RQ2c.exe", "12c03a3c782237821acd590fd91af4c5", 4192097),
	GAME_ENTRY_EN("rodequest2", "RQ2d.exe", "12c03a3c782237821acd590fd91af4c5", 4192097),
	GAME_ENTRY_EN("rodequest2", "RQ2e.exe", "12c03a3c782237821acd590fd91af4c5", 4192097),
	GAME_ENTRY_EN("rogered", "Uncontrollable.exe", "82da2565c456dcfb265ded6fe3189c0b", 32857801),
	GAME_ENTRY_EN("rogerlameadv", "testout.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 932804),
	GAME_ENTRY_EN("rogeroddsp1", "Roger Against the Odds_1.0.0.3.exe", "7825fa980fcd058caadaf6d3377b1f78", 727867851),
	GAME_ENTRY_EN("rogerquest", "Bandymas backup.exe", "e79a70b7e24f92b1ea5ff5d1b40e81f9", 86423076),
	GAME_ENTRY_EN("rogertreasure", "Graveyard.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 2091124),
	GAME_ENTRY_EN("rogue", "Rogue.exe", "4309e1468cdde96ad97f6988e9d5bc65", 31745808),  // Windows
	GAME_ENTRY_EN("rogue", "Rogue.ags", "0813cb3228cc108239c4583e66ca9137", 28647676),  // Linux
	GAME_ENTRY_EN("rogue", "ac2game.dat", "0813cb3228cc108239c4583e66ca9137", 28647676),  // Mac
	GAME_ENTRY_EN("romanian", "Beginner.ags", "cff2e109cc5f5363db4ba43b2868a628", 296534681),
	GAME_ENTRY_EN("ronbeforethelegacy", "RONlegacy.exe", "0b7529a76f38283d6e850b8d56526fc1", 4792872),
	GAME_ENTRY_EN("ronendgame", "ron-5.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2444579),
	GAME_ENTRY_EN("ronoutbreak", "RoN-DayOfComet.exe", "465f972675db2da6040518221af5b0ba", 10497989),
	GAME_ENTRY_EN("ronsixteen", "16c.exe", "06a03fe35791b0578068ab1873455463", 2467851),
	GAME_ENTRY_EN("rootofallevil", "RootOfAllEvil.exe", "c4f5b7b29be90ba0f8128298afb917de", 3698912),
	GAME_ENTRY_EN("rootofallevil", "RootOfAllEvil.exe", "c4f5b7b29be90ba0f8128298afb917de", 3699067), // v1.0
	GAME_ENTRY_EN("rootofallevil", "RootOfAllEvil.exe", "c4f5b7b29be90ba0f8128298afb917de", 3957834), // v1.1
	GAME_ENTRY_EN("rootofallevil", "RootOfAllEvil.ags", "c4f5b7b29be90ba0f8128298afb917de", 3957834), // v1.1 Crystal Shard
	GAME_ENTRY_EN("rosauradocelestial", "RosauraMAGS.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 13190949),
	GAME_ENTRY_EN("rosellahelm", "orow.exe", "21fd0f65dfa48de2b39cb8ec23b30889", 2507570),
	GAME_ENTRY_EN("rossnoble", "RossNoble Arena.exe", "0b7529a76f38283d6e850b8d56526fc1", 2247292),
	GAME_ENTRY_EN("rowengoestowork", "Rowen.exe", "a1cef60926235b85bd0e1866b19e0dc7", 3791058),
	GAME_ENTRY_EN("rudeawakening", "RudeAwakening.exe", "0710e2ec71042617f565c01824f0cf3c", 8038257),
	GAME_ENTRY_EN("rufusstory", "BlueAngel_4.exe", "615e73fc1874e92d60a1996c2330ea36", 93715328),
	GAME_ENTRY_EN("runaway", "RunAway.exe", "82da2565c456dcfb265ded6fe3189c0b", 7685040),
	GAME_ENTRY_EN("runestones", "Project Forsetti.exe", "c0d48dcc2003d4de0410cb8612d1c3e6", 37111243),
	GAME_ENTRY_LANG("ruptquest", "Rupt.exe", "465f972675db2da6040518221af5b0ba", 20229552, Common::RU_RUS),
	GAME_ENTRY_EN("ryansdayout", "Ryan's Day out.exe", "06a03fe35791b0578068ab1873455463", 2573584),
	GAME_ENTRY("sabotagenb", "Sabotage.exe", "c8cd4b980360837c6df720da9a57944c", 472090611),  // Windows v1.1 Eng-Deu
	GAME_ENTRY("sabotagenb", "Sabotage.ags", "afd7630312d1acb8920885d0b3fd7396", 468278241),  // Linux v1.1 Eng-Deu
	GAME_ENTRY_EN("saddsonissein", "Issein.exe", "3e8667bab45f2e5d7237db53ab376400", 4070795),
	GAME_ENTRY_EN("sagemonkey", "monkey.exe", "0710e2ec71042617f565c01824f0cf3c", 16157032),
	GAME_ENTRY("saladecosmique", "Salade cosmique.exe", "615e73fc1874e92d60a1996c2330ea36", 23758116),  // Eng-Fra
	GAME_ENTRY_EN("salazarsevilplan", "Jonny Smallvalley.exe", "a524cbb1c51589903c4043b98917f1d9", 64646627),
	GAME_ENTRY_EN("samarkand", "Samarkand.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 4426913),
	GAME_ENTRY_EN("sammysperm", "OROW.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1515435),
	GAME_ENTRY_EN("sammysquest", "Sammy's Quest.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 7924985),
	GAME_ENTRY_EN("sandiknievel", "Stunt Rider.exe", "5cca0f8af961d2bc1c6e475437d3b23b", 152923394),  // Windows
	GAME_ENTRY_EN("sandiknievel", "Stunt Rider.ags", "77acc96fad068c50d2b3e9220939733b", 149889774),   // Linux
	GAME_ENTRY_LANG("sandmen", "sandmen.exe", "0b7529a76f38283d6e850b8d56526fc1", 3578745, Common::DE_DEU),
	GAME_ENTRY_EN("santaclausdown", "scdown.exe", "f120690b506dd63cd7d1112ea6af2f77", 14385095),
	GAME_ENTRY_EN("santaflight", "A Flight To Remember.exe", "2569c8f271dc356e32483d40ee16b3e9", 112461458),
	GAME_ENTRY_EN_PLATFORM("santaflight", "A Flight To Remember Remastered.exe", "5ba6c10d5b499a1d0fef84d0947a52d5", 123138210, "Remastered"),
	GAME_ENTRY_EN("santaorphanage", "Santa and the orphanage.exe", "099a8b752cba39bb76552e94197edbf4", 163644553),
	GAME_ENTRY_EN("santaquest", "Santaquest.exe", "0564de07d3fd5c16e6947a647061913c", 5318615),
	GAME_ENTRY_EN("santassidekick", "Xmas.exe", "0710e2ec71042617f565c01824f0cf3c", 1921077),
	GAME_ENTRY_EN("santasstolensleigh", "SantaStolenSleigh.exe", "414c1d7cefe1a637bc1fc155e69b642c", 15296926),
	GAME_ENTRY_EN_PLATFORM("sargasso", "jamgame.exe", "6e861b1f476ff7cdf036082abb271329", 44598553, "Prototype"),
	GAME_ENTRY_EN("sarmanyanha", "Manuscript.exe", "e88bd0c3e4578a170a4fd64bba97b301", 531055749),
	GAME_ENTRY_EN("satanquest", "Satan.exe", "9cf51833e787cc919837d9a8bd8fc14c", 4079343),
	GAME_ENTRY("satanquest", "Satan.exe", "70cd70d2fc7b2a8716fc58fc9deaf259", 4779276),  // updated Eng-Deu
	GAME_ENTRY("satchsquest", "Satch's.exe", "465f972675db2da6040518221af5b0ba", 1928613),  // Eng-Esp
	GAME_ENTRY("satchsquest", "Satch's.exe", "465f972675db2da6040518221af5b0ba", 1928653),  // Eng-Esp
	GAME_ENTRY_EN("saturdaynightlone", "SNitLNotW.exe", "ca886eb4ee6e15107424124229fb3fb7", 32927471),  // Win
	GAME_ENTRY_EN("saturdaynightlone", "SNitLNotW.exe", "ca886eb4ee6e15107424124229fb3fb7", 32928283),  // Win v1.1
	GAME_ENTRY_EN("saturdaynightlone", "Time.ags", "4869cda677dabf08b4054d39eb4bc242", 29817228),  // Linux
	GAME_ENTRY_EN("saturdaynightlone2", "SaturdayNightJam.exe", "6866919e466f1b9a37c2c37e061752f3", 18283644),  // Win
	GAME_ENTRY_EN("saturdaynightlone2", "SaturdayNightJam.ags", "bbd696b464ce82707163c3fad8dacf13", 15127144),  // Linux
	GAME_ENTRY_EN("saturdayschool", "ss.exe", "28f82e420b82d07651b68114f90223c8", 1578062),
	GAME_ENTRY_EN("saturdayschool", "ss.exe", "b216ee957dd677023e02d900281a02d6", 1588546),
	GAME_ENTRY_EN_PLATFORM("saturdayschool", "ss.exe", "c9cc759756e1f24783029948da533ccd", 1916056, "OROW"),
	GAME_ENTRY_EN("saturdaysymbiosis", "Saturday Night Symbiosis.exe", "f18443f98fd61d2c655e76a17f7da905", 24170102),
	GAME_ENTRY("saw", "Saw.exe", "f120690b506dd63cd7d1112ea6af2f77", 60869310),  // Eng-Esp
	GAME_ENTRY_PLATFORM("saw", "Saw.exe", "34a66a5033b70f4050cbe5e33a45f747", 2452460, "Deluxe"),  //v1.30 Eng-Esp
	GAME_ENTRY("saw2", "Saw2.exe", "615e806856b7730afadf1fea9a756b70", 45338514),  // Eng-Esp
	GAME_ENTRY_EN("sawn1", "Sawn 1.exe", "afe40dc1416dd51e896ee0444d799f07", 5519133),  //v2
	GAME_ENTRY_EN("scalestraining", "Scalestraining.exe", "8f18ad4456e6dbeaf041cd91449a55ba", 6300801),
	GAME_ENTRY_EN("scalestraining", "Scalestraining.ags", "81d110ed81cfc34e1e249ea0df2d4fdd", 3264621),
	GAME_ENTRY_EN("scarecrowquest", "Scarecrow and his quest for a brain.exe", "099a8b752cba39bb76552e94197edbf4", 109875238),
	GAME_ENTRY_EN("scaredstiff", "Scared Stiff.exe", "18b284c22010850f79bc5c20054a70c4", 57878023),
	GAME_ENTRY_EN("scarehouse", "Scare House.exe", "7ddb9e776648faed5a51170d087074e9", 31133714),
	GAME_ENTRY_EN("scarymaze", "Scary Maze.exe", "01823d511cc00f4de6fd920eb543c6e7", 5380554),
	GAME_ENTRY("scenario5b", "scenario.exe", "707c317aa3cf27d21107ae3a52296baa", 4151144),  // Eng-Ita
	GAME_ENTRY_EN("schnelltrial", "TSS2.exe", "18456f28d9bf843b087e80072c85beca", 11217104),
	GAME_ENTRY_LANG("schoollout", "Schoollout.exe", "3d40063da244931d67726a2d9600f1e8", 384873228, Common::CS_CZE),
	GAME_ENTRY_EN("science", "orow3.exe", "f120690b506dd63cd7d1112ea6af2f77", 1330580),
	GAME_ENTRY_LANG("sciencesfaer", "ProfM.exe", "a524cbb1c51589903c4043b98917f1d9", 9215301, Common::FR_FRA),
	GAME_ENTRY_EN("scnidersom", "scnider.exe", "3128b9f90e2f954ba704414ae854d10b", 1189237),
	GAME_ENTRY_EN("scotchanimation", "Hill.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 818788),
	GAME_ENTRY_EN("scramschool", "scramschool3.exe", "0595b55383d72a99cd995092c5bf8475", 61574828),
	GAME_ENTRY_EN("scrapmanager", "SAM game.exe", "c21b8b701a7db8eb5e602b7271f8248b", 6911422),
	GAME_ENTRY_EN_PLATFORM("scratchingaway", "Scratching Away.exe", "57e261dd3bb45761af4a002775e45710", 3270025, "Prototype"),
	GAME_ENTRY_EN("scumpub", "Scum Pub.exe", "82da2565c456dcfb265ded6fe3189c0b", 2901504),
	GAME_ENTRY_EN("scyllacharybdis", "Olympus2.exe", "b5c41e08919834b549ca350a28f18f87", 2902947),
	GAME_ENTRY_EN("scytheisland", "Scythe.exe", "0500aacb6c176d47ac0f8158f055db83", 7119760),
	GAME_ENTRY_EN("searchofmarina", "In search of Marina.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 138505062),  // Windows
	GAME_ENTRY_EN("searchofmarina", "In search of Marina.ags", "57d8e44cfe1b992e92d63f7f646bd6fa", 135468882),  // Linux
	GAME_ENTRY_EN("seashells", "Shells.exe", "0500aacb6c176d47ac0f8158f055db83", 6355325),
	GAME_ENTRY_EN("seasongreetings2002", "xmas2002.exe", "97d700529f5cc826f230c27acf81adfd", 4263336),
	GAME_ENTRY_EN("secondplace", "Second Place.exe", "615e73fc1874e92d60a1996c2330ea36", 21732416),
	GAME_ENTRY_EN("secondstime", "Seconds Times the Charm.exe", "0241777c2537fc5d077c05cde10bfa9f", 6496853),
	GAME_ENTRY_EN("secretorbs", "SecondOrbSecret.ags", "81a8cc458ae984d3fd02283597735b8d", 2728010),
	GAME_ENTRY_EN("secretquestremake", "secretquest.exe", "f120690b506dd63cd7d1112ea6af2f77", 2121159),
	GAME_ENTRY_EN("secrets", "Secrets.exe", "06a03fe35791b0578068ab1873455463", 36176884),
	GAME_ENTRY_EN("seed", "Seed.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 67903775),
	GAME_ENTRY_EN("seekye", "GGJ21.exe", "0564de07d3fd5c16e6947a647061913c", 5997590),  // GameJam
	GAME_ENTRY_EN("seekye", "GGJ21.exe", "0564de07d3fd5c16e6947a647061913c", 6448377),  // itch.io
	GAME_ENTRY("self", "Self.exe", "559b801e76fa37dc3b1fdd8844e7733a", 27802879),  // Eng-Tur
	GAME_ENTRY_LANG("senpaigetaway", "Senpai Getaway.exe", "c445aec957326f5873492d4bc79e08c6", 23708509, Common::SK_SVK),  // Win 1.0
	GAME_ENTRY_LANG("senpaigetaway", "Senpai Getaway.ags", "63fed98e48296f9216f5bdc571871352", 20591945, Common::SK_SVK),
	GAME_ENTRY_LANG("senpaigetaway", "Senpai Getaway.exe", "c445aec957326f5873492d4bc79e08c6", 23708479, Common::SK_SVK),  // Win 1.1
	GAME_ENTRY_LANG("senpaigetaway", "Senpai Getaway.ags", "5835dc024e61cee34bb53092cbbb40db", 20591915, Common::SK_SVK),
	GAME_ENTRY_EN("sepulchre", "sepulchre.exe", "38dce17f7f7b96badf3a88e11e7a9797", 28412866),
	GAME_ENTRY_EN("serina", "Serinas Transylvanian Trip.exe", "8fd84e9331691d9fd1c393a383807b64", 11447796),
	GAME_ENTRY_EN("serum", "Infected.exe", "06a03fe35791b0578068ab1873455463", 2226020),
	GAME_ENTRY_EN("sesari", "sesari.exe", "8d1ff95c16500befbdc72260d461d73f", 8721127),  // Win
	GAME_ENTRY_EN("sesari", "sesari.exe", "8d1ff95c16500befbdc72260d461d73f", 8721319),
	GAME_ENTRY_EN("sesari", "ac2game.dat", "8d1ff95c16500befbdc72260d461d73f", 8721127),  // Mac
	GAME_ENTRY_EN("sesari", "ac2game.dat", "8d1ff95c16500befbdc72260d461d73f", 8721319),
	GAME_ENTRY_EN("sevendoors", "7DOORS.exe", "18b284c22010850f79bc5c20054a70c4", 113716886),
	GAME_ENTRY_LANG("sevgilim", "sevgilim.exe", "49c3b4aa0287206f199e7bbf19d5600e", 6585836, Common::TR_TUR),
	GAME_ENTRY_EN("shadesofgrey", "Shades of Grey.exe", "5f6db89bead4f55f32d0a4bd81df6dd3", 90761711),
	GAME_ENTRY_EN("shadesofgreye", "Shades of Greye.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 16125855),
	GAME_ENTRY_EN("shadowgate", "Shadowgate Remake.exe", "9608c428fb58ab5ef3c0d74c14578be6", 111307080),
	GAME_ENTRY_EN("shadowsofron", "Shadows.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 13891684),
	GAME_ENTRY_LANG("shailadusithlenquete", "Shai-la Enquete.exe", "a524cbb1c51589903c4043b98917f1d9", 7617785, Common::FR_FRA),
	GAME_ENTRY("shailadusithlenquete", "Shai-la Enquete.exe", "a524cbb1c51589903c4043b98917f1d9", 7489302),  // En-Fr
	GAME_ENTRY_LANG_PLATFORM("shailaofthesith", "Shaila_old.exe", "a524cbb1c51589903c4043b98917f1d9", 68426311, Common::FR_FRA, "v1.0"),
	GAME_ENTRY_LANG("shailaofthesith", "Shai-la du Sith.exe", "615e73fc1874e92d60a1996c2330ea36", 76407314, Common::FR_FRA),
	GAME_ENTRY("shailaofthesith", "Shaila_of_the_Sith.exe", "a524cbb1c51589903c4043b98917f1d9", 76170347),  // En-Fr
	GAME_ENTRY_LANG("shailaofthesith", "Shai-la du Sith.ags", "409edb13aaf4eaa5389e94b350ba51f0", 62229433, Common::FR_FRA),  // itch.io updated
	GAME_ENTRY("shailaofthesithv2ep1", "Shai-la du Sith - episode 1.ags", "f42ea5c0c261ee01db77f1a280b45644", 208736461),  // En-Fr
	GAME_ENTRY("shailaofthesithv2ep2", "Shai-la du Sith - episode 2.ags", "f54ca7fae98c250948bf20cf8eb5c583", 235022927),  // En-Fr
	GAME_ENTRY("shailaofthesithv2ep2", "Shai-la du Sith - episode 2.ags", "aab833760652a0b67d20b8272a1d91ce", 235025360),  // En-Fr
	GAME_ENTRY("shailaofthesithv2ep3", "Shai-la du Sith - episode 3.ags", "2a34aef50c30b9b6ffe9544fdd02fe37", 271142298),  // En-Fr
	GAME_ENTRY("shailaofthesithv2ep4", "Shai-la du Sith - episode 4.ags", "511e5d098c53e5ae18ddc3249198eb6d", 138936294),  // En-Fr
	GAME_ENTRY("shailaofthesithv2ep5", "Shai-la du Sith - episode 5.ags", "27a0979a46b425008106733e5709d0ee", 201866182),  // En-Fr
	GAME_ENTRY_EN("shamrockhg", "Shamrock  Graduation.exe", "7ddb9e776648faed5a51170d087074e9", 7255712),
	GAME_ENTRY_EN("shapeshift", "Shafeshift for cheese by Adipson.exe", "0241777c2537fc5d077c05cde10bfa9f", 211164349),
	GAME_ENTRY_EN("shapeshift", "Shafeshift for cheese by Adipson.exe", "0241777c2537fc5d077c05cde10bfa9f", 211043510),
	GAME_ENTRY_EN("shapevillage", "Shape Village.exe", "7825fa980fcd058caadaf6d3377b1f78", 13575497),
	GAME_ENTRY_EN("shapevillage", "Shape Land.ags", "deabadb4ecba9fb40b963c155d422628", 10539829),
	GAME_ENTRY_EN("shardsofgod", "Shards of God.ags", "4b94f022e290af3ddc181263001d71ee", 76895827),  // Win v1.0
	GAME_ENTRY_EN("shardsofgod", "Shards of God.ags", "26e733a242fec82194e1f6d0f0f3d0e8", 76895828),  // Linux v1.0 fixed
	GAME_ENTRY_EN("shardsofgod", "Shards of God.ags", "921652f83e899467f974cbee74427cf9", 76897579),  // Win/Mac/Linux v1.1
	DETECTION_ENTRY_GUIO("shardsofgod", "Shards of God.ags", "2883486157a3bac3ae1f399878c4acb9", 78996529, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, 0),  // Win/Mac/Linux v1.2
	DETECTION_ENTRY_GUIO("shardsofgod", "Shards of God.ags", "a04fc28c11b24c167768a54bf04a43b7", 78998145, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GAMEOPTION_NO_AUTOSAVE), ADGF_NO_FLAGS, 0),  // Win/Mac/Linux v1.2 final
	GAME_ENTRY_EN("sharethis", "shareme.exe", "208aac8aaca15de5482efa5b429f6718", 3837255),
	GAME_ENTRY_EN("sharkysthree", "Sharky's 3.exe", "25976a689b0f4d73eac69b1728377ecb", 9249262),
	GAME_ENTRY_EN("shawshank", "Shawshank.exe", "9cb3c8dc7a8ab9c44815955696be2677", 2968469),
	GAME_ENTRY_EN("shemwood", "Shem.exe", "0710e2ec71042617f565c01824f0cf3c", 8866401),
	GAME_ENTRY("sheepquest", "SheepQuest.exe", "c62e6e17113f278357859e2b9d5e4dbf", 16505844),  // Eng-Deu
	GAME_ENTRY_EN("sheetart", "Sheet.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 7431391),
	GAME_ENTRY("sherlock", "Sherlock.exe", "615e73fc1874e92d60a1996c2330ea36", 19108029),  // En-Fr
	GAME_ENTRY_EN("sherwood", "Sherwood.ags", "1f20e6ed6713a8ca9a2f07077719163f", 2728754),
	GAME_ENTRY_EN("sherwood", "Sherwood.ags", "ba0e6688dc82593dfaa2ff0fd9334550", 2511498),  // v0.3
	GAME_ENTRY_EN("shifter", "Shifter.exe", "615e73fc1874e92d60a1996c2330ea36", 35991817),
	GAME_ENTRY_EN("shifters", "Shifters.exe", "0241777c2537fc5d077c05cde10bfa9f", 9888383),
	GAME_ENTRY_EN("shiftersboxoutsidein", "Box.exe", "fc17e9b3ab53f6b4841e2a4af5c782ff", 24471804),
	GAME_ENTRY_LANG("shiftersboxoutsidein", "Box.exe", "0514661a4ba6772cf0db0cf839fc7003", 30959098, Common::DE_DEU),  // selmiak translation
	GAME_ENTRY_EN("shittyquest", "Shitty Quest.exe", "8086c90e2519804de7034b0d4bfa2b75", 7645128),
	GAME_ENTRY_EN_PLATFORM("shivah", "shivah.exe", "6e3d6225dee662ff6450a3bfa942773b", 8680174, "MAGS"),
	GAME_ENTRY_EN_PLATFORM("shoaly", "SYCBS.exe", "7825fa980fcd058caadaf6d3377b1f78", 5511622, "MAGS"),  // Windows
	GAME_ENTRY_EN_PLATFORM("shoaly", "SYCBS.ags", "85190d3884ba314b3c3307a701ce72b8", 2475954, "MAGS"),  // Linux
	GAME_ENTRY_EN("shootabducted", "Shoot.exe", "1275885401b7d2ece491e704535707d9", 4327626),
	GAME_ENTRY_EN("shootinggame", "GUN.exe", "9cf51833e787cc919837d9a8bd8fc14c", 776082),
	GAME_ENTRY_EN("shootmyvalentine", "Valentine.exe", "06a03fe35791b0578068ab1873455463", 2859760),
	GAME_ENTRY_EN("shortcut", "ShortCut.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 4415830),
	GAME_ENTRY_EN("shrivel", "Shrivel.exe", "12c6a846b5ba9a5dde4a1b804b3e86e9", 58873190),  // Windows
	GAME_ENTRY_EN("shrivel", "Shrivel.ags", "889d3fa28cc647a430ba61412f43e369", 56392024),  // Linux
	GAME_ENTRY_EN("shunday", "Shunday.exe", "82da2565c456dcfb265ded6fe3189c0b", 4937129),
	GAME_ENTRY_EN("sierraquest1", "SierraQuest1.exe", "465f972675db2da6040518221af5b0ba", 1275381),
	GAME_ENTRY_EN("signalloss", "signal loss.exe", "0736c98683a5ee6179edd886ca674909", 6808659),  // Windows
	GAME_ENTRY_EN("signalloss", "signal loss.ags", "08ff5df9f8ac4ee79e71ad9d5350bc2a", 3700799),
	GAME_ENTRY_EN("silentknight1", "Silent Knight.exe", "465f972675db2da6040518221af5b0ba", 47414163),
	GAME_ENTRY_EN("silentknight1", "Silent Knight.exe", "465f972675db2da6040518221af5b0ba", 47336931),
	GAME_ENTRY_EN("silentknight2", "SilentKnight2.exe", "465f972675db2da6040518221af5b0ba", 39941166),
	GAME_ENTRY_EN("silentknight2", "SilentKnight2.exe", "465f972675db2da6040518221af5b0ba", 39942642),
	GAME_ENTRY_EN("silentnightcreche", "creche.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2821363),
	GAME_ENTRY_EN("simonsjourney", "Simon.exe", "0710e2ec71042617f565c01824f0cf3c", 3496858),
	GAME_ENTRY_EN("simppmdee", "simpmdee.exe", "06a03fe35791b0578068ab1873455463", 1676974),
	GAME_ENTRY("simshogwarts", "Sims Hogwarts.exe", "615e73fc1874e92d60a1996c2330ea36", 20016887),  // En-Fr
	GAME_ENTRY_EN("sinbad", "SINBAD.exe", "afe40dc1416dd51e896ee0444d799f07", 40143412),
	GAME_ENTRY_EN_PLATFORM("sinfluencer", "Sinfluencer.exe", "053f3b05db9e3caac5075c246bf7f9e1", 280261196, "Beta"),
	GAME_ENTRY_EN("sinking", "OROW_Sinking.exe", "615e73fc1874e92d60a1996c2330ea36", 13781179),
	GAME_ENTRY_EN("siren", "Siren.exe", "0564de07d3fd5c16e6947a647061913c", 10015441),
	GAME_ENTRY_EN("sisterhelper", "Little Helper.exe", "615e73fc1874e92d60a1996c2330ea36", 8922861),
	GAME_ENTRY_EN("sisterrevenge", "BlueAngel_2.exe", "a524cbb1c51589903c4043b98917f1d9", 12289754),
	GAME_ENTRY_EN("sisyphus", "Sisyphus.exe", "c6496a18d39036c853da42b5888456d3", 98454819),  // itch.io 1.0 Win/Mac
	GAME_ENTRY_EN("sisyphus", "Sisyphus Reborn.exe", "059f968398cb58cb40908b217f406863", 98804803),  // itch.io 1.0.1 Win
	GAME_ENTRY_EN_STEAM("sisyphus", "SisyphusReborn.exe", "548f7dfee3c3567ee43792823532b5ff", 99531181),
	GAME_ENTRY_EN("skippysavestheday", "First Game Test.exe", "06a03fe35791b0578068ab1873455463", 10473902),
	GAME_ENTRY_EN("skumring", "Skumring.exe", "660bec8a9540e00fc8a61be3bfd54e0a", 855262786),  //v1.0.1
	GAME_ENTRY_EN_PLATFORM("skumring", "Skumring Extra Noir Edition.exe", "d84cd417bf24f76e9c74ce1f01dc5f3b", 803654540, "Extra Noir Edition"),
	GAME_ENTRY_EN("skyadventure", "skyadventure.exe", "97f74f77ff127215fc44f35958fd2d55", 4266937),
	GAME_ENTRY_EN("skyfall", "Skyfall.exe", "5cca0f8af961d2bc1c6e475437d3b23b", 88603257),  // Windows
	GAME_ENTRY_EN("skyfall", "Skyfall.ags", "4dabdd2f7dc706a51703ae355ef28326", 85569637),  // Linux
	GAME_ENTRY_EN("slaythedragon", "dragon.exe", "3c5bd1713959ff469cb46ebe5542cfcf", 2917672),
	GAME_ENTRY_EN("slaythedragon2", "Dragon2.exe", "84dc3d6bee474e039aace3c1f37b6c7d", 5682711),
	GAME_ENTRY_EN("slaythedragon3", "Dragon III.exe", "7ddb9e776648faed5a51170d087074e9", 36162059),
	GAME_ENTRY_EN("sleepingbeauty", "SleepingBeauty.exe", "495d45fb8adfd49690ae3b97921feec6", 607993972),
	GAME_ENTRY_EN("sleepingcastle", "sleeping.exe", "1a2f8dacfa7ab2d30579a49d2ae64e05", 1076403478),
	GAME_ENTRY_EN("sleepyisland", "Sleepy Island.exe", "465f972675db2da6040518221af5b0ba", 20270790),
	GAME_ENTRY_EN("sleuth2020remake", "SLEUTH.exe", "615e73fc1874e92d60a1996c2330ea36", 11422136),
	GAME_ENTRY_EN("sleuthch1", "Sleuth.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 34770218),
	GAME_ENTRY_EN("slickadv", "SA_TheWarehouse.exe", "c9858d30ca2ba46ba7ca3de34cd2b78b", 6922014),
	GAME_ENTRY("slidersquest", "1.exe", "06a03fe35791b0578068ab1873455463", 5595444),  // Eng-Rus
	GAME_ENTRY_EN("slimequestforpizza", "slime.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1661109),
	GAME_ENTRY_EN("slothforseasons", "A Sloth For Both Seasons.exe", "0241777c2537fc5d077c05cde10bfa9f", 43374778),
	GAME_ENTRY("slothforseasons", "A Sloth For Both Seasons.exe", "0241777c2537fc5d077c05cde10bfa9f", 404709403), // 1.09 En-Fr-De
	GAME_ENTRY("slothforseasons", "A Sloth For Both Seasons.exe", "1dedb2fa0b9278ab564fef7d4fb053dc", 405040236), // 1.10
	GAME_ENTRY_EN("slugprincess", "SlugPrincess.exe", "28f82e420b82d07651b68114f90223c8", 12132209),
	GAME_ENTRY_EN("slugprincess", "SlugPrincess.exe", "28f82e420b82d07651b68114f90223c8", 12131885),
	GAME_ENTRY_EN("slvsaus", "SL VS AUS 07.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1577884),
	GAME_ENTRY_LANG("sma2", "sma21.exe", "02094e217c23e1d40a84891ee95010cb", 7093110, Common::DE_DEU),
	GAME_ENTRY_LANG("sma3", "sma31.exe", "4d17844029d8910fbaae1bdc99e250f2", 4961734, Common::DE_DEU),
	GAME_ENTRY_LANG("sma4", "smavier.exe", "02094e217c23e1d40a84891ee95010cb", 2447360, Common::DE_DEU),
	GAME_ENTRY_LANG("sma5", "sma5.exe", "02094e217c23e1d40a84891ee95010cb", 3827471, Common::DE_DEU),
	GAME_ENTRY_LANG("sma6", "sma6.exe", "64fcaf7da0b257ea831f89c54be0ad72", 3735081, Common::DE_DEU),
	GAME_ENTRY_LANG("sma7", "sma7.exe", "64fcaf7da0b257ea831f89c54be0ad72", 4048986, Common::DE_DEU),
	GAME_ENTRY_LANG("sma8", "sma8.exe", "64fcaf7da0b257ea831f89c54be0ad72", 5768623, Common::DE_DEU),
	GAME_ENTRY_LANG("sma9", "sma9.exe", "465f972675db2da6040518221af5b0ba", 16823935, Common::DE_DEU),
	GAME_ENTRY_LANG("sma10", "smax.exe", "465f972675db2da6040518221af5b0ba", 20034862, Common::DE_DEU),
	GAME_ENTRY_LANG("sma11", "SMA11.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 27866451, Common::DE_DEU),
	GAME_ENTRY_EN("smasher", "LudumDare.ags", "938e27363ec11e95ecffaf89b8fd1d6c", 11934612),
	GAME_ENTRY_EN("smileysquest", "Smiley.exe", "90413e9ae57e222f8913b09d2bc847bc", 4938049),
	GAME_ENTRY_EN("smileysquest2", "mags.exe", "3128b9f90e2f954ba704414ae854d10b", 2108492),
	GAME_ENTRY_EN("smokinweed", "SWeed.exe", "be65afc1ea59889c05e4e4cc143b3dbc", 2171942),
	GAME_ENTRY_EN("smoothhide", "SmoothHide.exe", "b142b43c146c25443a1d155d441a6a81", 10006680),
	GAME_ENTRY_EN("snake", "Snake.exe", "f120690b506dd63cd7d1112ea6af2f77", 2110526),
	GAME_ENTRY("snakesofavalon", "snakes.exe", "464fbeef013ac949e2011551048928cd", 52601238),  // Eng-Pol
	GAME_ENTRY_LANG("snakesofavalon", "snakes-polish.exe", "464fbeef013ac949e2011551048928cd", 52601245, Common::PL_POL),
	GAME_ENTRY_EN_PLATFORM("snakesofavalon", "snakes.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 79103462, "MAGS"),
	GAME_ENTRY_EN("snakesonaplane", "SOAP.exe", "06a03fe35791b0578068ab1873455463", 1589975),
	GAME_ENTRY_EN("snipermotherland", "sniper and spotter 2.exe", "7a3096ac0237cb6aa8e1718e28caf039", 134215997),  // Windows
	GAME_ENTRY_EN("snipermotherland", "ac2game.dat", "7a3096ac0237cb6aa8e1718e28caf039", 134215997),  // Mac
	GAME_ENTRY_EN("sniperpatriotic", "Sniper and spotter being patriotic.exe", "89a94326c8afd9e0234e269bd7330130", 188070576),  // Windows
	GAME_ENTRY_EN("sniperpatriotic", "ac2game.dat", "89a94326c8afd9e0234e269bd7330130", 188070576),  // Mac
	GAME_ENTRY_EN("snipertower", "Sniper and spotter climbing a tower.exe", "78dd4ca028ee0156b6a093d6d780aa65", 87039009),  // Windows
	GAME_ENTRY_EN("snipertower", "Sniper and spotter climbing a tower.exe", "78dd4ca028ee0156b6a093d6d780aa65", 87039008),  // Windows
	GAME_ENTRY_EN("snipertower", "Sniper and spotter climbing a tower.ags", "e98f78c89f4a8f1823647030804d61cb", 84554257),  // Linux
	GAME_ENTRY_EN("snipertower", "Sniper and spotter climbing a tower.ags", "8f6f967e5d63c0b20c656d4380b42d65", 84554256),  // Linux
	GAME_ENTRY_EN("snipertower", "ac2game.dat", "8f6f967e5d63c0b20c656d4380b42d65", 84554256),  // Mac
	GAME_ENTRY_EN("snowqueen", "sq-mags1209.exe", "9b6bb91688e086efeddcd804a7f5adc6", 5739164),
	GAME_ENTRY_EN("snowtheadventure", "SNOW v1.1 - Benjamin Rivers.exe", "f9021058a4d0fe5deaa9ea3bf64b677b", 17370928),
	GAME_ENTRY_EN("snowmaneatcarrot", "HourgameIBISPI_1.exe", "2ff048659aaefd20d342db6428a5f1a0", 3484230),
	GAME_ENTRY_EN("sockstoday", "Should Have Worn Socks Today.exe", "f18443f98fd61d2c655e76a17f7da905", 21599494),  // Windows
	GAME_ENTRY_EN("sockstoday", "Should Have Worn Socks Today.ags", "60926de91f63495a0da6a028604aae90", 19457016),  // Linux
	GAME_ENTRY_EN("solitude", "solitude.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2397809),
	GAME_ENTRY_EN("somethingnovel", "SN.exe", "d1bf56bc9ea61d84fdf10f135a4967ca", 10151965),
	GAME_ENTRY_EN_PLATFORM("somethingpipes", "Pipes.exe", "4d5d25446373d4df71c6bda77a15230a", 119506949, "MAGS"),  // 0.9.0
	GAME_ENTRY_EN("somethingpipes", "Pipes.exe", "4d5d25446373d4df71c6bda77a15230a", 144554792),  // 0.9.1
	GAME_ENTRY_EN("somethingpipes", "Pipes.exe", "4d5d25446373d4df71c6bda77a15230a", 71094818),  // 1.0.1
	GAME_ENTRY_EN("somethingpipes", "Pipes.ags", "64bd4db268b5297bec1aa5c086f02c24", 60504485),  // 1.0.2
	GAME_ENTRY_EN("somewhere", "somewhere.ags", "03808527826e547ed7c86412f4df38f7", 16470512),
	GAME_ENTRY_EN("somnamulizer", "Hypnos.exe", "618d7dce9631229b4579340b964c6810", 50943313),
	GAME_ENTRY_EN("songanimals", "SongAnimals.exe", "f982756f0e2c2fbeca15d199b9851559", 120864978),   //v1.0.1
	GAME_ENTRY_EN_PLATFORM("songanimals", "SongAnimals.exe", "f982756f0e2c2fbeca15d199b9851559", 117864767, "MAGS"),
	GAME_ENTRY_EN_STEAM("songanimals", "SongAnimals.exe", "5a31d98635be0e3ade3d71cbcdc881f7", 121582550),
	GAME_ENTRY_EN("sonicandfriendsinclubhouse", "Sonic and friends.exe", "71989c04f1b9812a0df87bc4f5915d4b", 50429335),
	GAME_ENTRY_EN("sonicandfriendsinclubhouse", "Sonic and friends.ags", "8e3a88dbe57f3963d3a4fb2e4d4031c2", 47952263),
	GAME_ENTRY_EN("sophiamcgrath", "SophiaMcGrath.exe", "06a03fe35791b0578068ab1873455463", 7477218),  //v1.4
	GAME_ENTRY_EN("sorenquest", "SorenQst.exe", "3421b46ff95c5885603086c39a038a20", 3237770),
	GAME_ENTRY("sosk", "Sosk.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12338591),  // Eng-Tur
	GAME_ENTRY("soulsquest", "Souls Quest.exe", "891665fea5e0c59512d32d85bd837ebb", 17883282),  // Eng-Esp
	GAME_ENTRY("sovietunterzoegersdorf1", "suzoeg.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 79618213),  // Win En-De
	GAME_ENTRY("sovietunterzoegersdorf1", "ac2game.dat", "3b7cceb3e4bdb031dc5d8f290936e94b", 79618213),  // Linux/Mac En-De
	GAME_ENTRY("sovietunterzoegersdorf2", "suz2prog.exe", "47a774ff828be0ca227ee58e7dc61467", 43921618),  // En-De
	GAME_ENTRY_EN("spacebirdmissile", "Space.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 6802296),
	GAME_ENTRY_EN("spacecatvirus", "Space Cat vs Virus.exe", "148e5a062029eb3262bd2d257b5854ef", 18718680),
	GAME_ENTRY_EN("spacecross", "spacecross.exe", "b7996387269db9b8a17bc358eaf37464", 178238779),
	GAME_ENTRY_EN("spacecross", "spacecross.ags", "d9f24fba58f05020671fc9ef67c8bf4a", 175189287),
	GAME_ENTRY("spacefreakers", "Space Freakers v2.exe", "7a3096ac0237cb6aa8e1718e28caf039", 243867164),  // En-Fr
	GAME_ENTRY_EN("spacegremlin", "SpaceGremlin.exe", "615e73fc1874e92d60a1996c2330ea36", 2526508),
	GAME_ENTRY("spacehunter", "Space Hunter.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 87639118),  // Eng-Ita
	GAME_ENTRY("spacelynxes", "SpaceLynxes.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 6593042),  // Eng-Deu
	GAME_ENTRY_EN("spacemadness", "space madness.exe", "0b7529a76f38283d6e850b8d56526fc1", 934157),
	GAME_ENTRY_EN("spacemanspace", "Spaceman in Space.exe", "043db5bc9bfbfe0e46ab0a04925e6c8b", 4055389),
	GAME_ENTRY_EN("spacemaze", "spacemaze.exe", "465f972675db2da6040518221af5b0ba", 1069228),
	GAME_ENTRY_EN("spacemisadventures", "Space Adventure X1.exe", "7c6e063343fc2ec2bfffc93a1bbd6cfe", 72933664),  // Windows
	GAME_ENTRY_EN("spacemisadventures", "Space Adventure X1.ags", "6ad80c38eed9b826302e6c818720c880", 70445840),  // Linux
	GAME_ENTRY_EN_PLATFORM("spacepoolalpha", "SpacePool.exe", "6b1a26a2f2627ab20a3fced66588d5fa", 2970802, "MAGS"), //v12
	GAME_ENTRY_EN("spacepoolalpha", "SpacePool.exe", "ef1d6fdc83c91a1a8de9eaf2630737b7", 3055777), // v16
	GAME_ENTRY_EN("spacerangersep46", "SpaceRangersEp46.exe", "4f6c7ec127e8b0ce077abb357903612f", 41103057),
	GAME_ENTRY_EN("spacerangersep52", "SpaceRangers52Grisli.exe", "4f6c7ec127e8b0ce077abb357903612f", 208346458),
	GAME_ENTRY_EN("spacerangersep52", "SpaceRangers52Grisli.exe", "4f6c7ec127e8b0ce077abb357903612f", 208346522),  //v2
	GAME_ENTRY_EN("spacetunneler", "SpaceTunneller.exe", "0241777c2537fc5d077c05cde10bfa9f", 7486642),
	GAME_ENTRY_EN_PLATFORM("spacetunneler", "SpaceTunneller.exe", "495d45fb8adfd49690ae3b97921feec6", 8218468, "Deluxe"),  // Windows
	GAME_ENTRY_EN_PLATFORM("spacetunneler", "SpaceTunneller.ags", "8683f4a31784b658dc1cf9bc9814a9b9", 5737812, "Deluxe"),  // Linux
	GAME_ENTRY_EN("spacewarep1", "Spacewar.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2270669),
	GAME_ENTRY_EN("spacewarep2", "Space.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4084181),
	GAME_ENTRY_EN("speedbuggy", "Speed Buggy Manifold Destiny.exe", "5ce6a602a8393537d433330d81be0983", 54982833),
	GAME_ENTRY_EN("spellbound", "Spellbound.exe", "588e5b40901f9c85df35ac60b9417eac", 8809110),
	GAME_ENTRY_PLATFORM("spia", "Spia.exe", "2ac709a4ea3108235333846181c55603", 4471257, "Unfinished"),
	GAME_ENTRY_EN("spidertrek", "Spider Trek.exe", "ba539d8020d4dcb134693e357e37e8ff", 4181363),  // Windows
	GAME_ENTRY_EN("spidertrek", "Spider Trek.ags", "6ef42c5f6cd0a61f400f0f465a9f8717", 1073503),  // Linux
	GAME_ENTRY_EN("spilakassinn", "Splakassinn.exe", "06a03fe35791b0578068ab1873455463", 2658189),
	GAME_ENTRY_EN("spiritboard", "Ouja.exe", "4d7d2addcde045dae6e0363a43f9acad", 6254682),
	GAME_ENTRY_EN("spitboy", "Spit boy.exe", "173f00e582eebd082d66f1291702b373", 6352569),  // v2.1
	GAME_ENTRY_EN("splinter", "Splinter.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 610251511),
	GAME_ENTRY_EN("splinter", "Splinter.ags", "0b0f455bf34c8ec12cbd2f285c337592", 607771367),
	GAME_ENTRY_EN("splitfighters", "Blast.exe", "615e73fc1874e92d60a1996c2330ea36", 37981098),
	GAME_ENTRY_EN("splurt", "Wasted.exe", "0736c98683a5ee6179edd886ca674909", 126375980),
	GAME_ENTRY("spmachinima1", "SP.exe", "4fb72c890984548ed6782063f2230942", 2741550),  // Eng-Rus
	GAME_ENTRY("spmachinima2", "SP2.exe", "4fb72c890984548ed6782063f2230942", 3320428),  // Eng-Rus
	GAME_ENTRY_EN("spooks", "Spooks.exe", "0710e2ec71042617f565c01824f0cf3c", 22888238),
	GAME_ENTRY_EN("spoonsiii", "spoonsIII.exe", "2ca80bd50763378b72cd1e1cf25afac3", 16298983),
	GAME_ENTRY_EN("spottd_boyd", "Spot the Difference.exe", "0b7529a76f38283d6e850b8d56526fc1", 933452),
	GAME_ENTRY_EN("spottd_captaind", "SpotTheDifference.exe", "0241777c2537fc5d077c05cde10bfa9f", 2853487),  // Windows
	GAME_ENTRY_EN("spottd_captaind", "SpotTheDifference.ags", "adea961dfd5fb69d78bae14e6e6b9054", 698326),  // Linux
	GAME_ENTRY_EN("sproutsofevil", "Sprouts of evil.exe", "bdd1df0484e296faa348ffcb03e16273", 22329944),
	GAME_ENTRY_EN("sq45", "SQ4,5.exe", "5cd8db602cedc8f04cd3ca290a4a2693", 4487578),  //v1.100
	GAME_ENTRY("sq45", "SQ4,5.exe", "5cd8db602cedc8f04cd3ca290a4a2693", 6886082),  //v3.0 Eng + Deu(separate download)
	GAME_ENTRY("sq7m1", "SQ7 Mania. Ep 1.exe", "256752c9a97b4780fc5e6f3239c8cdf1", 72027772),  // Eng-Rus
	GAME_ENTRY("sq7m2", "SQ7Mania2.exe", "82da2565c456dcfb265ded6fe3189c0b", 143600299),  // Eng-Rus
	GAME_ENTRY_EN("sqdote", "SQ DOTE.exe", "615e73fc1874e92d60a1996c2330ea36", 28479377),
	GAME_ENTRY_EN_PLATFORM("sqdote2", "DOTE2.exe", "7ddb9e776648faed5a51170d087074e9", 6122737, "Chapter I"),
	GAME_ENTRY_EN("sqdote2", "XENON.ags", "50153b6eac4061a96f403ddc65c14b9d", 12263830),
	GAME_ENTRY_EN("sqinc", "SQinc.exe", "3ee9783a5613040e25193e21c235a7d3", 6013823),  // Win/Mac
	GAME_ENTRY_EN("sqinc", "SQinc.ags", "0c940729c90f15bc34fdb93bdecbf94b", 4214354),  // Linux
	GAME_ENTRY_EN("sqkubik", "SQ Kubik.exe", "4fb72c890984548ed6782063f2230942", 2184808),
	GAME_ENTRY("sqm11", "SQMania1.exe", "465f972675db2da6040518221af5b0ba", 831674),  // Eng-Rus
	GAME_ENTRY_PLATFORM("sqm11", "SQM1.exe", "465f972675db2da6040518221af5b0ba", 1001506, "Remake"),  // Eng-Rus
	GAME_ENTRY("sqm12", "SQM 1x2.exe", "465f972675db2da6040518221af5b0ba", 755146),  // Eng-Rus
	GAME_ENTRY_EN_PLATFORM("sqm12", "SQM2 RMK.exe", "465f972675db2da6040518221af5b0ba", 3029288, "Remake"),
	GAME_ENTRY("sqm13", "SQM 1x3.exe", "465f972675db2da6040518221af5b0ba", 1435210),  // Eng-Rus
	GAME_ENTRY("sqm14", "SQM 1x4.exe", "465f972675db2da6040518221af5b0ba", 1383567),  // Eng-Rus
	GAME_ENTRY("sqm15", "SQ Mania Ep5.exe", "a524cbb1c51589903c4043b98917f1d9", 2896204),  // Eng-Rus
	GAME_ENTRY("sqm16", "SQM 1x6.exe", "465f972675db2da6040518221af5b0ba", 1400100),  // Eng-Rus
	GAME_ENTRY_EN("sqpotim", "sq_game.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2937151),
	GAME_ENTRY_EN("sqvn", "ENG.exe", "615e73fc1874e92d60a1996c2330ea36", 13629884),
	GAME_ENTRY_EN("sqvsb", "vsb.exe", "9fc9b41b494ec4a1072e7485ee6113fc", 5632323),
	GAME_ENTRY_EN("sqvsb", "vsb.exe", "9fc9b41b494ec4a1072e7485ee6113fc", 5631438),
	GAME_ENTRY_EN("sstrek25", "startrek.exe", "5cffd228bbf2286b3ea57aaa21087c88", 11737899), // v1.03 Win
	GAME_ENTRY_EN("sstrek25", "startrek.ags", "5009a13b7ce87da0dbedc5a171abc22f", 8572183), // v1.03 Mac
	GAME_ENTRY_EN("stairquest", "Stair Quest.exe", "e0aeab6a2c479fde167c4c43c3abb8ca", 4550699), // v1.0.1
	GAME_ENTRY_EN_PLATFORM("stairquest", "stair-quest.exe", "75494269745a5282be12278166cf662c", 33778514, "Special Edition"), // v2.0 itch.io Windows
	GAME_ENTRY_EN_PLATFORM("stairquest", "ac2game.dat", "75494269745a5282be12278166cf662c", 33778514, "Special Edition"), // v2.0 itch.io Mac
	GAME_ENTRY_EN_PLATFORM("stairquest", "stair-quest.ags", "535320876f2ef9ef066dfabc925f4e51", 31300418, "Special Edition"), // v2.0 itch.io Linux
	GAME_ENTRY_EN_PLATFORM("stairquest", "stair-quest-2023-winter.ags", "b7ef8a1689fe2fa8bf52e21ce0d8670d", 9912782, "Winter 2023 Edition"), // v2.1.23 itch.io Windows
	GAME_ENTRY_EN_PLATFORM("stairquest", "game.ags", "b7ef8a1689fe2fa8bf52e21ce0d8670d", 9912782, "Winter 2023 Edition"), // v2.1.23 itch.io Mac
	GAME_ENTRY_EN("stanamespiepisode1", "NEW.exe", "f120690b506dd63cd7d1112ea6af2f77", 19194728),
	GAME_ENTRY_EN("standupaliens", "TestGame.ags", "0f367869084896cca92a15b5cefad6d6", 1154590),
	GAME_ENTRY_EN("stansrevenge", "Gameisle.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 915036),
	GAME_ENTRY_LANG("starfreakers", "StarFreakers.exe", "f120690b506dd63cd7d1112ea6af2f77", 28073577, Common::FR_FRA),
	GAME_ENTRY_EN("stargateadv", "StarGA.exe", "0710e2ec71042617f565c01824f0cf3c", 45662205),  // v1.0
	GAME_ENTRY_EN("stargateadv", "StarGA.exe", "0710e2ec71042617f565c01824f0cf3c", 45664346),  // v1.1
	GAME_ENTRY_EN("stargateadv", "StarGA.exe", "0710e2ec71042617f565c01824f0cf3c", 45738298),  // v1.21
	GAME_ENTRY_LANG("stargatequizz", "Stargate Quizz.exe", "7c70226b560d6d9ffd9e165d6fbddd7e", 2547578, Common::FR_FRA),
	GAME_ENTRY("stargatesgc", "Stargate SGC.exe", "bdaf20d9779c01986d6d8b7e1d6118ee", 2188193),  // En-Fr
	GAME_ENTRY("stargatesolitaire", "SolitaireSG1.exe", "5529522460cb27d6a4f2619aee618590", 11658638),  // En-Fr
	GAME_ENTRY_EN_PLATFORM("starlitgrave", "starlitgrave.exe", "92ddda7d8b443e02c39b2e28d24073b4", 3779727, "Alpha"),
	GAME_ENTRY_EN_PLATFORM("starlitgrave", "starlitgrave.ags", "788021bb5db67e1f653f43158cfe8f1f", 1530495, "Alpha"),
	GAME_ENTRY_EN("starshipcaramba", "karamba.exe", "465f972675db2da6040518221af5b0ba", 21540340),
	GAME_ENTRY_EN("starshiphindenburg", "hindenburg.exe", "a524cbb1c51589903c4043b98917f1d9", 106373791),
	GAME_ENTRY_EN_PLATFORM("starshiplight", "strship1.exe", "7971a7c02d414dc8cb33b6ec36080b91", 32805168, "Beta"),
	GAME_ENTRY_EN_PLATFORM("starshiplight", "strship1.ags", "9dca4b575740e300af265aa9ec234f71", 30308128, "Beta"),
	GAME_ENTRY_EN("starshipposeidon", "Starship Poseidon.exe", "5a9abb3094d0b3f4bc09c0c77fbb8024", 4163873),
	GAME_ENTRY("starshipquasar", "quasar.exe", "8d1c6698abc66509df3dbe57a0a4144b", 11959826), // v1.1 freeware
	GAME_ENTRY_LANG("startrekgalactique", "StarTrek Aventure Galactique.exe", "920d38888c138569a5f40c4d565bb7da", 9261483, Common::FR_FRA),
	GAME_ENTRY("startrekmission", "Startrek Missions.exe", "c5169c7e073c9149ff8ca756489c9298", 300402830),  // En-Fr
	GAME_ENTRY("startrekgloriousep1", "Star Trek - Glorious Wolf (episode 1).ags", "6b4e80a9ba2361789865042a0886ac09", 156744056),  // En-Fr
	GAME_ENTRY("startrekgloriousep2", "Star Trek - Glorious Wolf (episode 1).ags", "4464e26de4596da161eb61429c830bf7", 206850101),  // En-Fr (misnamed)
	GAME_ENTRY("startrekgloriousremake", "Star Trek - Glorious Wolf.ags", "57aa944ea0430031110a55f988bcdda5", 696865216),  // En-Fr
	GAME_ENTRY_EN("startreknewton", "Anomaly.exe", "721e8a1dce90fc3ee31cade9a50d9e75", 8750111),
	GAME_ENTRY_EN("stateofmind", "State of Mind.exe", "be13cb758d3568b0532695081ab64683", 15933385),
	GAME_ENTRY_EN("stateofmind", "State of Mind.exe", "dbe9bab672130b9a84925cb6da64a3db", 15934766),
	GAME_ENTRY_EN("steamedhams", "Steamed Hams.exe", "d278eb385f1f7acabdcb6d3b331fea38", 13160830),
	GAME_ENTRY_EN("steamedhams", "Steamed Hams.exe", "99f1151d9c98ccf906c0ae7059a4854a", 13222965),  // v1.1
	GAME_ENTRY_EN("steamsquares", "Steamsquares.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 1993722),
	GAME_ENTRY_EN("steamtrek", "steamtrek.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 14807756),
	GAME_ENTRY_EN("stediddyip1employment", "stediddy1.exe", "5872fea5a958bc74c2d9ca7b2d196c42", 27136166),
	GAME_ENTRY_EN("steeplejack", "steeplejack.ags", "3025c45908736d5849ecd4d696211d30", 6114285),
	GAME_ENTRY_EN_PLATFORM("stephenkingcars", "Stephen King.exe", "82da2565c456dcfb265ded6fe3189c0b", 39260572, "Alpha"),
	GAME_ENTRY_EN("stevequest", "SteveQuest.exe", "615e73fc1874e92d60a1996c2330ea36", 4552527),  // Windows
	GAME_ENTRY_EN("stevequest", "ac2game.dat", "615e73fc1874e92d60a1996c2330ea36", 4552527),  // Linux
	GAME_ENTRY_EN_PLATFORM("stickamhour1", "IB SHAG Entry 12july.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 1661407, "Unfinished"),
	GAME_ENTRY_EN("stickamhour1", "IB SHAG Entry 12july.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 1694283),
	GAME_ENTRY_EN("stickamhour2", "IB SHAG entry 13_07_08.exe", "95dcf736be87cf7d40bf64d5b078c4b7", 1703894),
	GAME_ENTRY_EN("stickemup", "stick.exe", "0b7529a76f38283d6e850b8d56526fc1", 694226),
	GAME_ENTRY_EN("stickmanquest", "stickman quest.exe", "06a03fe35791b0578068ab1873455463", 1723909),
	GAME_ENTRY_EN("stickmenterrorists", "stick.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 2399329),
	GAME_ENTRY_EN("stickycrimsonhouse", "Crimson House Files.exe", "3b095a7872e04769d04ab45e9c1b66eb", 3610653),
	GAME_ENTRY_EN("stolenmoustache", "moustache.exe", "615e73fc1874e92d60a1996c2330ea36", 2089017),
	GAME_ENTRY_EN("stormpuncher", "NMFA Stormpuncher.exe", "0564de07d3fd5c16e6947a647061913c", 219413088),
	GAME_ENTRY_EN("stranded", "Stranded.exe", "18b284c22010850f79bc5c20054a70c4", 39791629),
	GAME_ENTRY_EN("strangerbynight", "Stranger.exe", "0500aacb6c176d47ac0f8158f055db83", 5849939),
	GAME_ENTRY_EN("strangerbynight", "Stranger.exe", "0500aacb6c176d47ac0f8158f055db83", 5854099), // v1.1
	GAME_ENTRY_EN("strangerinstickworld", "game1.exe", "a524cbb1c51589903c4043b98917f1d9", 42525810),
	GAME_ENTRY_EN("strangerthings", "StrangerThings.exe", "cc19db728abbcf657db6b76afb0e92d1", 43636017),
	GAME_ENTRY_EN("strangerthings", "StrangerThings.ags", "88784c4bc760f7a7bfdd2c77e41297c5", 41367623), // Linux
	GAME_ENTRY_EN("strangerthings", "ac2game.dat", "cc19db728abbcf657db6b76afb0e92d1", 43636017), // Mac
	GAME_ENTRY_EN("strawmanaugment", "Scarecrow.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 3571130),  //Windows
	GAME_ENTRY_EN("strawmanaugment", "Scarecrow.ags", "a3f0c8c4efabb5f559c8cc17e1ce6e10", 1082794),  //Linux
	GAME_ENTRY_EN("strawmanbomb", "Strawman Bombing Disposal.exe", "3be2b769e7d20621ed5d37b759052941", 27841111),
	GAME_ENTRY_EN("strawmanbomb", "Strawman Bombing Disposal.ags", "051330855fb89a77acbfc8b0997cfd24", 25351239),
	GAME_ENTRY_EN_PLATFORM("stuckathome", "stuckathome.exe", "465f972675db2da6040518221af5b0ba", 2540157, "v1.0"),
	GAME_ENTRY_EN_PLATFORM("stuckathome", "stuckathome.exe", "f120690b506dd63cd7d1112ea6af2f77", 2746090, "v2.0"),
	GAME_ENTRY_EN_PLATFORM("stuckathome", "stuckhome.exe", "f8e8f781a4a95c2bfea5f54b085df550", 4910855, "2015 Remake"),
	GAME_ENTRY_EN("stuckinamuddle", "Stuck in a muddle with you.exe", "f18443f98fd61d2c655e76a17f7da905", 27336942),
	GAME_ENTRY_EN("stuckonyou", "Stuck on you.exe", "5cca0f8af961d2bc1c6e475437d3b23b", 85832117),  // Windows
	GAME_ENTRY_EN("stuckonyou", "Stuck on you.ags", "e2416cf4eeb317fb2f6127b398ad0979", 82798497),  // Linux
	GAME_ENTRY_EN("studiomediocre", "Studio.exe", "9cb3c8dc7a8ab9c44815955696be2677", 10524547),
	GAME_ENTRY_EN("stygiophobia", "Stygiophobia Another Day.ags", "59b1696d414575a035c152d07b26a009", 3117650),  // itch.io client
	GAME_ENTRY_EN("subatomic", "LD48-23.exe", "75f4c7f66b1be60af5b2d65f617b91a7", 25655660),
	GAME_ENTRY_EN("subwaycrisis", "Ludum Dare 43.ags", "97341b3ddd7a0eb8ae4a247fe25dc082", 3508218),  // Linux
	GAME_ENTRY_EN("subwaycrisis", "Ludum Dare 43.exe", "78f88f71bbef277d93cbe10f1d2ac404", 5996042),  // Windows
	GAME_ENTRY_LANG("suenanfantasmas", "xn--SueanLosFantasmas.exe-ebc", "50a7ba44a0d5499628de2a46bb4a9e97", 71071264, Common::ES_ESP),  // Windows
	GAME_ENTRY_LANG("suenanfantasmas", "SuenanLosFantasmas.ags", "4092234ccc703586b54d86900550baa0", 67828236, Common::ES_ESP),  // Linux
	GAME_ENTRY_EN("summerwoes", "MAGSAugust.exe", "7921b41f852d8dd62922d87cc69d9e83", 4896812),
	GAME_ENTRY_EN("sunriseparadise", "Sunrise Paradise.exe", "838f62e456d4fbab9c3754c9d70d2b89", 13430921),
	GAME_ENTRY_EN_PLATFORM("sunriseparadise", "Sunrise Paradise.exe", "44f90e1b2f427fef02fc0d00420d2b6f", 12707470, "OROW"),
	GAME_ENTRY_EN("supaevil", "SupaEvil.exe", "0710e2ec71042617f565c01824f0cf3c", 7602318),
	GAME_ENTRY_EN("superegoch0", "Superego - Chapter 0 - (ENG).exe", "870c2e6b0f37a76dea643eb3f536fd0d", 160755419),
	GAME_ENTRY_LANG("superegoch0", "Superego - Capítulo 0 (ESP).exe", "870c2e6b0f37a76dea643eb3f536fd0d", 160657653, Common::ES_ESP),
	GAME_ENTRY_EN("supergirl", "Supergirl.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12108982),
	GAME_ENTRY_EN("superhammerquest", "Super Hama Queste.exe", "ed77faf99fc94efb68662e92bce2e985", 15954791),
	GAME_ENTRY_EN("superpitstopracing", "Super Racing.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3660473),  //v1.0
	GAME_ENTRY_EN("superpitstopracing", "Super Racing.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3660481),  //v1.01
	GAME_ENTRY_EN("suppaninja", "Suppa.exe", "a3c0ebdda1294f7a9eda08240a6faf69", 7322838),
	GAME_ENTRY_EN("suppaninja", "Suppa.exe", "d45d8959ea9932c62218905672363dbc", 8630975),
	GAME_ENTRY_EN("surreality", "Surreality.exe", "3128b9f90e2f954ba704414ae854d10b", 10773443),
	GAME_ENTRY("suspiciousmind", "MAGS March 2019.exe", "c7211b1db56a231ff1f8454704c086f1", 20550294),  // En-Fr
	GAME_ENTRY_EN("svengordan1", "Sven Gordan.exe", "0710e2ec71042617f565c01824f0cf3c", 4930040),
	GAME_ENTRY_EN("sweetmythery", "SweetMythery.exe", "7ddb9e776648faed5a51170d087074e9", 6674615),
	GAME_ENTRY_EN("sweetmythery", "SweetMythery.exe", "7ddb9e776648faed5a51170d087074e9", 6678774),  //v2
	GAME_ENTRY_EN("sweetnightmares", "ags.exe", "b5c41e08919834b549ca350a28f18f87", 26705094),
	GAME_ENTRY_EN("swordremake", "Sword.exe", "0500aacb6c176d47ac0f8158f055db83", 1847692),
	GAME_ENTRY_EN("sydneyfindsemployment", "sydney.exe", "06a03fe35791b0578068ab1873455463", 17351738),
	GAME_ENTRY_EN("sydneytreadsthecatwalk", "sydney two.exe", "615e806856b7730afadf1fea9a756b70", 31480339),
	GAME_ENTRY("symbiosis", "Symbiosis.exe", "973f6b65820ca1f4e19704a49be99d76", 7476108),  // Eng-Esp
	GAME_ENTRY_EN("syncroutine", "syncroutine.exe", "98df40b0885b4e5f90f1093987f1f56c", 11343324),
	GAME_ENTRY_LANG("tablarawls", "La Tabla de Rawls.exe", "f120690b506dd63cd7d1112ea6af2f77", 11718615, Common::ES_ESP),
	GAME_ENTRY_EN("tabletsoftibet", "The sacred tablets of Tibet.exe", "173f00e582eebd082d66f1291702b373", 42986246),
	GAME_ENTRY_EN("tactician", "Tactician Civil War.exe", "ab7375d1365c58e8f185f46894730065", 27867997),
	GAME_ENTRY("taleofalegacy", "A Tale of a Legacy.exe", "387ff720e746ae46e93f463fd58d77a4", 347029015),  // Eng-Esp
	GAME_ENTRY_LANG("taleofalegacy", "En busca de un legado.exe", "387ff720e746ae46e93f463fd58d77a4", 346758500, Common::ES_ESP),
	GAME_ENTRY_EN("talesfromtheroad", "TFTR.exe", "7ddb9e776648faed5a51170d087074e9", 7004383),  // Windows
	GAME_ENTRY_EN("talesfromtheroad", "TFTR.ags", "33cb992abc983bbdb22fe0ae2c8eb8b9", 4517071),  // Linux
	GAME_ENTRY_EN("talesofchickenry", "Chickenry.exe", "aabdafae8b57dfc48fdf158a72326c23", 50338519),
	GAME_ENTRY_LANG("talesofinfimum", "TalesOfInfimum.exe", "af1683f81365165be8ec727fe9e89300", 295583283, Common::ES_ESP),  // Windows
	GAME_ENTRY_LANG("talesofinfimum", "TalesOfInfimum.ags", "7f614d6aad2bef2f85360ad3e4f1dc8c", 292408351, Common::ES_ESP),  // Linux
	GAME_ENTRY_EN("talesofjayvin", "Tales of Jayvin.exe", "615e73fc1874e92d60a1996c2330ea36", 885298865),  // Windows
	GAME_ENTRY_EN("talesofjayvin", "ac2game.dat", "615e73fc1874e92d60a1996c2330ea36", 885298865),  // Mac
	GAME_ENTRY_EN("talesofotubania", "TalesOfOtubania.exe", "344bdaaf19f8ed676fa02e3ed7b4d52a", 2626344),
	GAME_ENTRY_LANG("taoch1ep1", "Tao.ags", "a5e4a3d0d857cb69b1df110e719a53ab", 295135618, Common::FR_FRA),
	GAME_ENTRY("taoch1ep1", "Tao.ags", "ebd28ce170ff70525b97a59464321b26", 295135560),  // En-Fr
	GAME_ENTRY("taoch1ep2", "Tao - Episode 2.ags", "37d5e533feae0ae83f5ea3a95acc4667", 387408010),  // En-Fr
	GAME_ENTRY("taoch1ep2", "Tao - Episode 2.ags", "95a03ae5e119ef0dec6dee9a63beb11b", 387409484),  // En-Fr
	GAME_ENTRY("taoch1ep3", "Tao - Episode 3.ags", "61f3b6c41cef1e86bee361601f9da997", 338845965),  // En-Fr
	GAME_ENTRY("taoch1ep4", "Tao - Episode 4.ags", "efbe9dba9052dfa45980a4a8f8bb1173", 260655181),  // En-Fr
	GAME_ENTRY("taoch2ep1", "Tao - Chapitre 2.ags", "8cd48b6e4f4c9e8582445877afbc9603", 222076248),  // En-Fr
	GAME_ENTRY("taoch2ep2", "Tao - Chapitre 2,  Episode 2.ags", "f2e667c31a4cd7a77ea84e2f459a8498", 226549126),  // En-Fr
	GAME_ENTRY("taoch2ep3", "Tao - Chapitre 2,  Episode 3.ags", "97a36c15a4c29c2906bcd1f9c6bb670b", 231332200),  // En-Fr
	GAME_ENTRY("taoch2ep4", "Tao - Chapitre 2,  Episode 4.ags", "66a97826a927258600605178be1961c5", 262605201),  // En-Fr
	GAME_ENTRY("taoch3", "Tao - Chapitre 3.ags", "5967c290ecf0475c60ef78ba7a09bb29", 541109600),  // En-Fr
	GAME_ENTRY_LANG("taospacetime", "Tao through space and time AGS.exe", "7a3096ac0237cb6aa8e1718e28caf039", 35930047, Common::FR_FRA),  // Win French
	GAME_ENTRY_LANG("taospacetime", "Tao through space and time AGS.ags", "50e4bd3554f1b1d5bf9b7b7c83920c8b", 33452975, Common::FR_FRA),  // Linux French
	GAME_ENTRY("taospacetime", "Tao through space and time AGS.exe", "7a3096ac0237cb6aa8e1718e28caf039", 35931108),  // Win   En-Fr
	GAME_ENTRY("taospacetime", "Tao through space and time AGS.ags", "8d57cca10d3825240fd094aefb2db14d", 33454036),  // Linux En-Fr
	GAME_ENTRY("taospacetime", "Tao through space and time AGS.exe", "7a3096ac0237cb6aa8e1718e28caf039", 35931411),  // Win En-Fr
	GAME_ENTRY("taospacetime", "Tao through space and time AGS.ags", "721753956c697c3d9f134e69622af201", 33454339),  // Linux En-Fr
	GAME_ENTRY_EN("tarthenia", "Tarthenia.exe", "a524cbb1c51589903c4043b98917f1d9", 1047988125),
	GAME_ENTRY_EN("tarthenia", "Tarthenia.exe", "0a00053ae87573e4dd273f0252336854", 1047989168),  // itch.io
	GAME_ENTRY_EN("tasteblood", "Taste the blood of darkness.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 199304272),  // Windows
	GAME_ENTRY_EN("tasteblood", "Taste the blood of darkness.ags", "9eac7b72519618075e9a16ec630600b6", 196268092),  // Linux
	GAME_ENTRY_EN("tattooroom", "tattooroom.exe", "615e73fc1874e92d60a1996c2330ea36", 10112843),
	GAME_ENTRY_EN("taverntales", "MAGS_07_23.exe", "8e21f6f21e88b8e50e0cdaec09e2b539", 22151992),
	GAME_ENTRY_EN("teafortwo", "detective208x.exe", "7d551cb6cc3c3424d40da9568a3c2340", 11076184),  // Windows
	GAME_ENTRY_EN("teafortwo", "ac2game.dat", "7d551cb6cc3c3424d40da9568a3c2340", 11076184),  // Mac
	GAME_ENTRY_EN("teafortwo", "detective208x.ags", "6dd4805bc7a4c9309cb0ea19bd2ed1b7", 8103492),  // Linux
	GAME_ENTRY_EN("technobabylonp3", "Technobabylon 3.exe", "c9cd3e2cdd295c33e62cfa97b14461e9", 17040188),
	GAME_ENTRY_EN("tedmcbinky", "steampunk.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3307742),
	GAME_ENTRY_EN("teethglass", "Teeth of Glass.ags", "554939a8212f9627b41a86fc07ca91ef", 30781027),  // v1.0.0
	GAME_ENTRY_EN("teethglass", "Teeth of Glass.ags", "e7cd8fe0cb4cbbd24f25f02e49c46a0f", 30783301),  // v1.0.5
	GAME_ENTRY_EN("tehhorror", "Hallow.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1758007),
	GAME_ENTRY_EN("templeofspheres", "Spheres.exe", "e9475a2e453039ca51c8c319a8c8255a", 7117373),
	GAME_ENTRY_EN("tenhumstombpart1", "tomb.exe", "0710e2ec71042617f565c01824f0cf3c", 1252175),
	GAME_ENTRY_EN("tenweeks", "Ten Weeks.exe", "b234873ef00c46eb21f73233560a9384", 38925643),  // v1.0.2 itch.io
	GAME_ENTRY_EN("terrorofthevampire", "TERRVAMP.exe", "efd2b2abf965c4b50bea79a419b82cf1", 5164804),
	GAME_ENTRY("teruteru", "Teru Teru Bozu.exe", "f626bce6c741bdfcc657024c39d9d28a", 150002572),  //En-Fr
	GAME_ENTRY_LANG("tetricity", "TetriCity2.exe", "27343924ddad3be0b97bdcaa71858b1b", 8887974, Common::FR_FRA),
	GAME_ENTRY_EN("textparsergame", "tparsergame.exe", "06a03fe35791b0578068ab1873455463", 2061002),
	GAME_ENTRY_EN("tfg-bttf", "TFG - BTTF.exe", "7971a7c02d414dc8cb33b6ec36080b91", 295151510),
	GAME_ENTRY("tfg-bttf3", "tfg - bttf iii - timeline of mi.exe", "7ddb9e776648faed5a51170d087074e9", 157518001),  //Eng-Esp
	GAME_ENTRY_LANG("tfg-bttf3", "tfg - bttf iii - timeline of mi.exe", "7ddb9e776648faed5a51170d087074e9", 159685524, Common::IT_ITA),
	GAME_ENTRY("tfg-bttf4", "tfg - bttf iv - the multitasking crystal.exe", "7ddb9e776648faed5a51170d087074e9", 176964994),  //Eng-Esp
	GAME_ENTRY_LANG("tfg-bttf4", "tfg - bttf iv - the multitasking crystal.exe", "7ddb9e776648faed5a51170d087074e9", 176965142, Common::IT_ITA),
	GAME_ENTRY("tfg-bttf5", "tfg - bttf v - m -space-timelines -.exe", "7971a7c02d414dc8cb33b6ec36080b91", 213022300),  //Eng-Esp
	GAME_ENTRY_LANG("tfg-bttf5", "tfg - bttf v - m -space-timelines -.exe", "7971a7c02d414dc8cb33b6ec36080b91", 213044448, Common::IT_ITA),
	GAME_ENTRY_EN("tfg-bttfwat", "TFG-IBBTTFWAT.exe", "7971a7c02d414dc8cb33b6ec36080b91", 285643776),
	GAME_ENTRY_EN("tfg-goonies", "TFG - THE GOONIES.exe", "7971a7c02d414dc8cb33b6ec36080b91", 62524979),
	GAME_ENTRY_EN("tfg-mi", "tfg-g&mi.exe", "7971a7c02d414dc8cb33b6ec36080b91", 251715672),
	GAME_ENTRY("tfg-pixel", "THE PIXEL HAS YOU DISK 1.exe", "7971a7c02d414dc8cb33b6ec36080b91", 289291855),  // 1.0 Eng-Esp
	GAME_ENTRY("tfg-pixel", "THE PIXEL HAS YOU DISK 1.exe", "0691254142a4dbeaf47a6efd7a4cb6f0", 376910335), // 1.1 En-Es-It
	GAME_ENTRY_EN("tfg-somi", "TFG-TSOMI-R.exe", "7971a7c02d414dc8cb33b6ec36080b91", 473724788),
	GAME_ENTRY_EN("tfg-ssgw", "TFG-SSGW.exe", "7971a7c02d414dc8cb33b6ec36080b91", 274251831),
	GAME_ENTRY("tfg-zak2", "TFG - ZAK2.exe", "7971a7c02d414dc8cb33b6ec36080b91", 155632013),  //Eng-Esp
	GAME_ENTRY_EN("tftoz1", "Cyborg Seppuku.exe", "78dd4ca028ee0156b6a093d6d780aa65", 16776249),
	GAME_ENTRY_EN("tftoz1", "Cyborg Seppuku.ags", "68dc2bc08a1c5979304f16c145a7fbb5", 14291497),
	GAME_ENTRY_EN("tftoz2", "The Goat Crone.exe", "78dd4ca028ee0156b6a093d6d780aa65", 39332941),
	GAME_ENTRY_EN("tftoz2", "The Goat Crone.ags", "efd1535495c3e875c30b0c1391c73c80", 36848189),
	GAME_ENTRY_EN("tftoz3", "Fleshworms.exe", "78dd4ca028ee0156b6a093d6d780aa65", 28663403),
	GAME_ENTRY_EN("tftoz3", "Fleshworms.ags", "eb6655814be8d6d5d5c0448d5db51d53", 26178651),
	GAME_ENTRY_EN("tftoz4", "The Construction.exe", "63f8a60cc094996bd070e68cb3c4722c", 21089665),
	GAME_ENTRY_EN("tftoz4", "The Construction.ags", "44cf8ac1234e6bd5dfc0068c8a6d601b", 17982829),
	GAME_ENTRY_EN("tharsheblows", "Thar She Blows.exe", "f18443f98fd61d2c655e76a17f7da905", 3700425),  // Win
	GAME_ENTRY_EN("tharsheblows", "Thar She Blows.ags", "2f377a0ae77da6d4c64ae0d9a74262a5", 1557870),  // Linux
	GAME_ENTRY_EN_PLATFORM("tharsheblows", "Thar She Blows (AZERTY).exe", "f18443f98fd61d2c655e76a17f7da905", 3700434, "AZERTY"),  // Win
	GAME_ENTRY_EN_PLATFORM("tharsheblows", "Thar She Blows (AZERTY).ags", "27e66d30225be4a2ab9588892bf56496", 1557879, "AZERTY"), // Linux
	GAME_ENTRY_EN("thatcrazyworld", "crworld.exe", "64fcaf7da0b257ea831f89c54be0ad72", 3219346),
	GAME_ENTRY_EN("thatdamndog", "Game.exe", "0241777c2537fc5d077c05cde10bfa9f", 379703779),
	GAME_ENTRY_EN_PLATFORM("the30minutewar", "7DG.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1659147, "OROW"),  // v1.0
	GAME_ENTRY_EN("the30minutewar", "7DG.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1659319),  // v1.01
	GAME_ENTRY_EN("the4thwall", "4thwall.exe", "615e73fc1874e92d60a1996c2330ea36", 4905269),
	GAME_ENTRY("the7thsense", "7thSense.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 17972183),  // Eng-Ita
	GAME_ENTRY_EN("theabtyoncase", "Eureka01.exe", "3f68c7f993ba21ee671b7c28b2c1b895", 158425036),  // v1.1.2 Win
	GAME_ENTRY_EN("theabtyoncase", "Eureka01.exe", "3f68c7f993ba21ee671b7c28b2c1b895", 158424910),  // v1.1.2 Linux
	GAME_ENTRY_EN("theabtyoncase", "Eureka01.exe", "3f68c7f993ba21ee671b7c28b2c1b895", 158425244),  // v1.1.1
	GAME_ENTRY_EN("theaddict", "Addict.exe", "82da2565c456dcfb265ded6fe3189c0b", 2356364),
	GAME_ENTRY_EN("theadventureofthehero", "The Adventure of the Hero.exe", "5d20aab40290fc72ec84393464913df2", 4408710),
	GAME_ENTRY_EN("theadventureofthehero", "The Adventure of the Hero.ags", "812ae2d9541db4c31fcd1b26b5ba0e32", 2133548),  // v1.1 Linux
	GAME_ENTRY_EN("theadventureofthehero", "The Adventure of the Hero.exe", "246ca45f385aeb54ebaa1fa2790fc3c1", 4689468),
	GAME_ENTRY_EN("theadventuresoffatman", "fatman.exe", "853cef07077feadd0f2ccd55c5bd747b", 56375090),  // 2003 release
	GAME_ENTRY_EN("theadventuresofturquoise", "The Adventures Of Turquoise Mcdonald.exe", "06a03fe35791b0578068ab1873455463", 2883450),
	GAME_ENTRY_EN("theagencyp1", "The Agency.exe", "06a03fe35791b0578068ab1873455463", 17018031),
	GAME_ENTRY("theaspiroxcase", "L'Affaire Aspirox.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 139865345),  //En-Fr
	GAME_ENTRY_EN("theassassin", "the-assassin.exe", "e3962995a70923a8d5a8f1cf8f932eee", 29043438),
	GAME_ENTRY_EN("theawakening", "The Awakening.exe", "0241777c2537fc5d077c05cde10bfa9f", 13613585),
	GAME_ENTRY_EN("thebar", "one room game.exe", "465f972675db2da6040518221af5b0ba", 11660587),
	GAME_ENTRY_EN("thebar", "one room game.exe", "465f972675db2da6040518221af5b0ba", 8673763),
	GAME_ENTRY_EN("thebasement", "The basement.exe", "4d5a94465148aa61c72608d6c2cde27a", 2993325),
	GAME_ENTRY_EN("theblock", "Block.exe", "615e73fc1874e92d60a1996c2330ea36", 36339968),
	GAME_ENTRY_EN("thebrokenbrain", "Brain.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 1092293),
	GAME_ENTRY("thebum", "The Bum 1.04.exe", "0212ad5fd38d64f63c5d3f2f2a0f65ee", 28673515),  // En-De-Pol
	GAME_ENTRY_EN("thebunker", "The Bunker.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12384199),
	GAME_ENTRY_EN("thebunker", "The Bunker.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 12391058),
	GAME_ENTRY_EN("theburgerflipper", "TheBurgerFlipper.exe", "615e73fc1874e92d60a1996c2330ea36", 11690724),
	GAME_ENTRY_EN("theburgomaster", "The Burgomaster.exe", "7c6e063343fc2ec2bfffc93a1bbd6cfe", 79529850),  // Windows
	GAME_ENTRY_EN("theburgomaster", "ac2game.dat", "6d26634640b3a82297b27c8aead0c9a2", 77042026),  // Mac
	GAME_ENTRY_EN("theburgomaster", "The Burgomaster.ags", "6d26634640b3a82297b27c8aead0c9a2", 77042026),  // Linux
	GAME_ENTRY_EN("thebutler", "Butler.exe", "27343924ddad3be0b97bdcaa71858b1b", 12583937),
	GAME_ENTRY_EN("thecabin", "TheCabin.exe", "0564de07d3fd5c16e6947a647061913c", 263089918),
	GAME_ENTRY_EN("thecadaversynod", "The cadaver synod.exe", "36f44e064eab15e502caeb60fd09f52d", 37438749),
	GAME_ENTRY_EN("thecadaversynod", "The cadaver synod.exe", "36f44e064eab15e502caeb60fd09f52d", 37438876),
	GAME_ENTRY_EN("thecan", "TheCan.exe", "82da2565c456dcfb265ded6fe3189c0b", 72388782),
	GAME_ENTRY_EN("thecatcase", "The Cat Case.exe", "981e60a9be515bb56c634856462abbc7", 40413519),
	GAME_ENTRY_EN("thecatsman", "The Man Who Liked Cats.exe", "6b1702aec6d0a44bda59ef8f229956c5", 14331279),
	GAME_ENTRY_EN("thecatsman", "The Man Who Liked Cats.ags", "fb3db6558cbb01eca2c8e059e43d73af", 11845503),
	GAME_ENTRY_EN("thecell2005", "The Cell.exe", "4d5a94465148aa61c72608d6c2cde27a", 3292679),
	GAME_ENTRY_EN("thecell2017", "The Cell.exe", "b142b43c146c25443a1d155d441a6a81", 21422530),
	GAME_ENTRY_EN("thecell2017", "One Room.ags", "fd8e237ecafeb6fd3d010e13b36d6f08", 18942386),
	GAME_ENTRY_EN("thechronomancers", "The Chronomancers.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 2821824),
	GAME_ENTRY_EN("thechrysalis", "Mygame.exe", "22b9c6d170613eb01afa1697b1b75cdb", 28926604),
	GAME_ENTRY_EN("thechrysalis", "Mygame.exe", "22b9c6d170613eb01afa1697b1b75cdb", 29210038),  // v1.1
	GAME_ENTRY_LANG("thecityadv", "City.exe", "465f972675db2da6040518221af5b0ba", 97965753, Common::RU_RUS),
	GAME_ENTRY_EN("thecomingofage", "LORNA0.exe", "e39488e0b1372632b38a364aceda27ce", 80294233),
	GAME_ENTRY_EN("thecondemned", "The Condemned.exe", "338fa79960d40689063af31c671b8729", 399499655),
	GAME_ENTRY_EN("thecradle", "The Cradle.exe", "7db052bc30700d1f30f5330f5814f519", 326421438),
	GAME_ENTRY("thecrazedchicken", "chicken.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 3913199), // Eng-Pol
	GAME_ENTRY_EN("thecrystalball", "OROW8.exe", "8cc8a212d8ffb923db0ba740074dfa69", 20921755),
	GAME_ENTRY("thecube", "The Cube.exe", "97d700529f5cc826f230c27acf81adfd", 2029985),  // Eng-Deu
	GAME_ENTRY("thecurseoflife", "TheCurse.exe", "90413e9ae57e222f8913b09d2bc847bc", 9651875), // Eng-Ita
	GAME_ENTRY_EN("thedarkplague", "darkplague.exe", "850d1f4d8af87b89d77650a083f7f307", 8176381),
	GAME_ENTRY_EN("thedayafter", "BETTY0.exe", "e39488e0b1372632b38a364aceda27ce", 16429392),
	GAME_ENTRY_EN("thedaynothinghappened", "MAGSMarch2011.exe", "615e73fc1874e92d60a1996c2330ea36", 9845417),
	GAME_ENTRY_EN("thedayofdarkness", "DayODark.exe", "465f972675db2da6040518221af5b0ba", 1082758),
	GAME_ENTRY_EN("thedeathoflukesimpson", "LukeDead.exe", "06a03fe35791b0578068ab1873455463", 3233789),
	GAME_ENTRY_EN("thedecorcist", "The Decorcist.exe", "d982a9e174e944d06e1332bf6cef96c3", 15139291),
	GAME_ENTRY_EN("thedeed", "TheDeed.exe", "a524cbb1c51589903c4043b98917f1d9", 3428218),
	GAME_ENTRY_EN("thedelivery", "TheDelivery.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 4655690),
	GAME_ENTRY_EN("thedelivery", "TheDelivery.ags", "eb22467716d7885c0b4c96e429830ed1", 1543734),
	GAME_ENTRY_EN("thedevilsshroudpart1", "DevilTorino256.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16661573),
	GAME_ENTRY_EN("thedevilsshroudpart1", "DevilTurin.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16780039),
	GAME_ENTRY_EN("thedevilsshroudpart2", "devillione256.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 23776250),
	GAME_ENTRY_EN("thedevilsshroudpart2", "DevilLyon.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 23850386),
	GAME_ENTRY_EN("thedevilsshroudpart3", "DevilPrague.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 16360454),
	GAME_ENTRY_EN("thedevilsshroudpart4", "DevilVaduz.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 32025519),
	GAME_ENTRY_EN("thedevilsteeth", "The Devil's Teeth.exe", "0564de07d3fd5c16e6947a647061913c", 12560271),
	GAME_ENTRY_EN("thedigitalspell", "Digital.exe", "4d17844029d8910fbaae1bdc99e250f2", 14657385),
	GAME_ENTRY_EN("thedisgracedprince", "Byzanz 2.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 12592231),
	GAME_ENTRY_EN("thedistantdoor", "DDoor.ags", "712e34e9df7ed49d8d5734885cf6b5c1", 9602695),
	GAME_ENTRY_EN("thedollhousech1", "DH.exe", "6e861b1f476ff7cdf036082abb271329", 4931346),
	GAME_ENTRY_EN("thedome", "TheDome.exe", "c2ab4c6df39f76b4290da75874a79f74", 15953711),  // Windows
	GAME_ENTRY_EN("thedome", "TheDome.ags", "b634d6f4e3b6edbf47d8ec0db9b07967", 13455647),  // Linux
	GAME_ENTRY_EN("thedreamjobep1", "TheDreamJob.exe", "82da2565c456dcfb265ded6fe3189c0b", 15955443),
	GAME_ENTRY_EN("thedusseldorfconspiracy", "dusseldorf.exe", "465f972675db2da6040518221af5b0ba", 60887427),  //v1.4
	GAME_ENTRY_EN("thedusseldorfconspiracy", "dusseldorf.exe", "465f972675db2da6040518221af5b0ba", 60878686),
	GAME_ENTRY_EN("thedwarvendaggerofblitz", "teste2.exe", "06a03fe35791b0578068ab1873455463", 9874658),
	GAME_ENTRY_EN("thedwarvendaggerofblitzch2", "TDDOB.exe", "82da2565c456dcfb265ded6fe3189c0b", 69210852),
	GAME_ENTRY_EN("theelevator", "Elevator.exe", "f10516e88ec858700804ee69d041aead", 29506915),  // Windows
	GAME_ENTRY_EN("theelevator", "Elevator.ags", "7b10f25449c5feef532155c7fbe99192", 27064147),  // Linux
	GAME_ENTRY_EN("theendlessnight", "An Endless Night.exe", "ae8d17427ae4a33be146967054f0fcf1", 7052775),
	GAME_ENTRY_EN("theenergizer", "TheEnergizer.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 54539625),
	GAME_ENTRY_EN("theepicadventures", "Epic adventures.exe", "a524cbb1c51589903c4043b98917f1d9", 17723648),
	GAME_ENTRY_EN("theerrand", "The Errand.ags", "1bf407b3d0396deefd5c02f4996806f4", 24822053),
	GAME_ENTRY_EN("theerrand", "The Errand.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 27934009),
	GAME_ENTRY_EN("theeverbeginningtale", "RRS.exe", "36f8b37f85f446e9aaccb18d85660cde", 5006033),
	GAME_ENTRY_EN_PLATFORM("theeverbeginningtale", "RRS.exe", "25976a689b0f4d73eac69b1728377ecb", 4313508, "MAGS"),
	GAME_ENTRY_EN("theexecutionofanneboleyn", "TheExecutionOfAnneBoleyn.exe", "6ee67d23067b0499eb8c6a1a9aabff8c", 8549540),
	GAME_ENTRY_EN("theexecutionofanneboleyn", "TheExecutionOfAnneBoleyn.exe", "6ee67d23067b0499eb8c6a1a9aabff8c", 8550421),
	GAME_ENTRY("theexperimentp1", "LaborEN.exe", "b9fcd61691d19d954a5cfd5fb57a6b45", 7245344),  // Eng-Hun
	GAME_ENTRY("theexperimentp2", "E2.exe", "9b590bb4d6cf923b0e9fafd8d57f77bc", 82578386),  // Eng-Hun
	GAME_ENTRY_EN("thefaketrix", "faketrix.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3111286),
	GAME_ENTRY_EN("thefamilytreasure", "LostTreasure.exe", "0710e2ec71042617f565c01824f0cf3c", 6363677),
	GAME_ENTRY_EN("thefamilytreasure", "LostTreasure.exe", "0710e2ec71042617f565c01824f0cf3c", 6373724),
	GAME_ENTRY_EN("thefarmep1", "the farm.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 88858865),
	GAME_ENTRY_EN("thefarmmags", "thefarm.exe", "0241777c2537fc5d077c05cde10bfa9f", 17372618),
	GAME_ENTRY_EN("theficklehandsoffate", "Game.exe", "2bf7a3a5f0a22a140350e29bb1ac7cfb", 4105398),
	GAME_ENTRY_EN("thefind", "The Find.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3476394),
	GAME_ENTRY_EN("thefront", "The Front.exe", "b142b43c146c25443a1d155d441a6a81", 74260367),
	GAME_ENTRY_EN("thefront", "GW.ags", "cc053bbecf43cc6894f6b1928164645c", 71780223),
	GAME_ENTRY_EN("thefrozenshore", "FrozenShore.exe", "f6a5f0a2f617b84174b5ab48147ae9f7", 28126443),
	GAME_ENTRY_EN("thefurtheststation", "TheFurthestStation.exe", "82da2565c456dcfb265ded6fe3189c0b", 17582750),
	GAME_ENTRY_EN("thegardenofhades", "hades.exe", "d304c4f27f336963c16578274264d174", 991101514),
	GAME_ENTRY_EN("thegardenofhades", "hades.ags", "7c6fe05a214e42dbbf9f2f1d173918ab", 988042294),
	GAME_ENTRY_EN("thegift", "GIFT.ags", "2bb1b2a9feae34ccf8f3f6c1f51c8972", 1643933),
	GAME_ENTRY_EN("thegift", "GIFT.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 4755889),
	GAME_ENTRY_EN("theglasssplinters", "Splinters.exe", "615e73fc1874e92d60a1996c2330ea36", 5249293),
	GAME_ENTRY_EN("thegourmet", "The Gourmet.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 49995478),
	GAME_ENTRY_EN("thegourmet", "The Gourmet.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 49995756),
	GAME_ENTRY_EN("thegourmet", "The Gourmet.ags", "d9fe3634cf4d63ab511476279d687788", 47515334),
	GAME_ENTRY_EN("thegreatcasserolecaper", "TGCC.exe", "6ee842f73649ced615c44d4eb303687c", 10902276),
	GAME_ENTRY_EN("thegreatstrokeoff", "GSO.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 84337184),
	GAME_ENTRY_EN("thegreenroom", "TheGreenroom.exe", "7ddb9e776648faed5a51170d087074e9", 6878369),
	GAME_ENTRY_EN("thegruglegends", "Grug.exe", "c6acb1705dd30f82ba026e92d6af831e", 85947629),
	GAME_ENTRY("theguilteternal", "The Guilt Eternal.exe", "78dd4ca028ee0156b6a093d6d780aa65", 435613120),  // Windows Eng-Pol
	GAME_ENTRY("theguilteternal", "The Guilt Eternal.ags", "1825e291dab02ce9f7436b1d5c22f177", 433128368),  // Linux Eng-Pol
	GAME_ENTRY_EN("thehamlet", "hamlet.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 4252451),
	GAME_ENTRY_EN("thehat", "the_hat.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 2641333),
	GAME_ENTRY_EN("thehauntedhouse", "HH.exe", "97d700529f5cc826f230c27acf81adfd", 29221222),
	GAME_ENTRY_EN("theheist", "The Heist.exe", "261d108f9213356a351b35b54867f342", 13502430),
	GAME_ENTRY_EN_PLATFORM("theheist", "The Heist.exe", "261d108f9213356a351b35b54867f342", 13502452, "OROW"),
	GAME_ENTRY_EN("thehobbitriseofthedragonking", "The Hobbit.exe", "338fa79960d40689063af31c671b8729", 113862075),
	GAME_ENTRY_EN("thehousethatatemysoul", "THTFMD.exe", "f120690b506dd63cd7d1112ea6af2f77", 3930376),
	GAME_ENTRY_EN("thehousewithoutwindows", "SchwarzWeissRot - Englisch.exe", "495d45fb8adfd49690ae3b97921feec6", 82849166),  // v1.1
	GAME_ENTRY_LANG("thehousewithoutwindows", "SchwarzWeissRot.exe", "495d45fb8adfd49690ae3b97921feec6", 82913128, Common::DE_DEU),  // v1.1
	GAME_ENTRY_LANG("thehousewithoutwindows", "xn--SchwarzWeiRot.ags-fob", "c8a30be25c77a008648a07cc9473b2e7", 71442559, Common::DE_DEU),  // v1.1
	GAME_ENTRY_LANG("thehousewithoutwindows", "SchwarzWeissRot.exe", "495d45fb8adfd49690ae3b97921feec6", 82913362, Common::DE_DEU),
	GAME_ENTRY_EN("thehuntforgoldbeard", "The hunt for Goldbeard.exe", "a524cbb1c51589903c4043b98917f1d9", 4043823),
	GAME_ENTRY_EN("thehuntforgoldbeard", "The hunt for Goldbeard.exe", "a524cbb1c51589903c4043b98917f1d9", 4043188),
	GAME_ENTRY_EN("thehuntforshaunbinda", "For Lisa Creed Made By Dan man.exe", "fd91d116e5adc4328cb22fab0b940e4d", 30753829),
	GAME_ENTRY_EN_PLATFORM("thehuntforshaunbinda", "For Lisa Creed Made By Dan man.exe", "5d6e936eaee9316e756bfe33ef8d3a19", 34290389, "Deluxe"),
	GAME_ENTRY("theinfinitystring", "String.exe", "90413e9ae57e222f8913b09d2bc847bc", 36963577),  // En-De-It-Slo
	GAME_ENTRY_EN("theiraqiparadox", "0203.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 2976283),
	GAME_ENTRY_EN("thejackyard", "The Jackyard.exe", "45d092881ef70b288382426a10251df3", 101417484),
	GAME_ENTRY_EN("thejimihendrixcase", "The Jimi Hendrix Case.exe", "ef6e791e8d47243959c1c064c90e4b1d", 3370287),
	GAME_ENTRY_EN("thejourneyhomep1", "Part1.exe", "9cf51833e787cc919837d9a8bd8fc14c", 8355767),
	GAME_ENTRY_EN("thelastharvest", "The Last Harvest.exe", "f120690b506dd63cd7d1112ea6af2f77", 6253816),
	GAME_ENTRY_EN("thelastsupperawhodunnit", "THE LAST SUPPER, A WHODUNNIT.exe", "37500274a7882e8087042cc6ec851e0c", 13447848),
	GAME_ENTRY_EN("thelighthouse", "lighthouse.exe", "a524cbb1c51589903c4043b98917f1d9", 17254094),
	GAME_ENTRY_EN("thelightningspell", "TLS.exe", "637ef614ac7c764f0ce5f49a83c8ea25", 17607032),
	GAME_ENTRY("theloneplanet", "The_Lone_Planet.exe", "5d159fb223fb7cc4f4aa345496208c03", 99398397),  //En-Cz
	GAME_ENTRY("thelongtrip", "longtrip.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 184362586),  //En-De
	GAME_ENTRY_EN("thelurkinghorror", "Lurking Horror.exe", "338fa79960d40689063af31c671b8729", 7405841),
	GAME_ENTRY_EN("thelurkinghorror", "Lurking Horror.exe", "338fa79960d40689063af31c671b8729", 7405886),
	GAME_ENTRY("themaninthehat", "The Man in the Hat.exe", "3e06d9a22f2feba5a1a1916adcebd05e", 240032128),  //En-Es
	GAME_ENTRY_EN("themanisee", "ICanSee.exe", "0241777c2537fc5d077c05cde10bfa9f", 296174305),
	GAME_ENTRY_EN("themarionette", "marionette.exe", "ff3d6e4edfca8b4f4f1c6cbf8e2781a6", 88408446),
	GAME_ENTRY_EN("themccarthychroniclesep1", "McCarthy.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 29488543),
	GAME_ENTRY_EN_PLATFORM("themcreedcase", "McReedCase.exe", "4bcbc24015114752b3c7971128704689", 2982436, "MAGS"),
	GAME_ENTRY_EN("themcreedcase", "McReedCase.exe", "4bcbc24015114752b3c7971128704689", 2179359),  //v1.05
	GAME_ENTRY_EN("themind", "The Mind.exe", "90bca43a15c25fdfec9510f3965177a8", 11252636),  // Windows
	GAME_ENTRY_EN("themind", "The Mind.ags", "881091c9dd6a88460e88b2f55968cc24", 8198024),  // Linux
	GAME_ENTRY_EN("themissinghat", "hatgame.exe", "387ff720e746ae46e93f463fd58d77a4", 22716315),
	GAME_ENTRY_EN("themuseum", "The Museum.exe", "0564de07d3fd5c16e6947a647061913c", 13291226),
	GAME_ENTRY_EN("themysteriesofstiegomoor1", "mystery.exe", "0710e2ec71042617f565c01824f0cf3c", 12062769),
	GAME_ENTRY_EN("themysteryofhauntedhollow", "magic1.exe", "3128b9f90e2f954ba704414ae854d10b", 3338806),
	GAME_ENTRY_EN("thenetherworld", "netherworld.exe", "465f972675db2da6040518221af5b0ba", 2253033),
	GAME_ENTRY_EN("thenextcurse", "TNC.exe", "88cf59aad15ca331ab0f854e16c84df3", 4125146),
	GAME_ENTRY_EN("thenorthcrown", "htw.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3452775),
	GAME_ENTRY_EN("thenorthcrown", "htw.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3452831),
	GAME_ENTRY_EN("theoccultist1", "Occultist01.exe", "776a62db4387dc68be92ef9933399fd5", 10691261),  // Windows
	GAME_ENTRY_EN("theoccultist1", "Occultist01.exe", "b781cc4ab0fbe647615fafa4830ec308", 8075941),  // Windows
	GAME_ENTRY_EN("theoccultist1", "Occultist01.ags", "bedd88519a455aea5ccd7f0c8298116d", 7584937),
	GAME_ENTRY_EN("theoccultist1", "Occultist01.ags", "4a2349e5348f00f78e2ea86ea6d3cafd", 4909201),
	GAME_ENTRY_EN("theoffice", "TheOffice.exe", "8e386ed8901186872484373c64fcd812", 355519407),  //v1.1 Win
	GAME_ENTRY_EN("theoffice", "TheOffice.ags", "6721a4f1403e28a61dca04558728f914", 353015199),  //v1.1 Linux
	GAME_ENTRY_EN("theoracle", "The Oracle.exe", "f120690b506dd63cd7d1112ea6af2f77", 7490474),  //v1.1
	GAME_ENTRY_EN("theoscillation", "theosc.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 7188083),
	GAME_ENTRY_EN("theoven", "TheOven.exe", "618d7dce9631229b4579340b964c6810", 32992976), //v1.0.0.0
	GAME_ENTRY_EN("thepaparazziprince", "Willy.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3403804),
	GAME_ENTRY_EN("thepark", "park.exe", "97d700529f5cc826f230c27acf81adfd", 709265),
	GAME_ENTRY("theparrotsnatchers", "r.exe", "a524cbb1c51589903c4043b98917f1d9", 11945759),  // En-Pol
	GAME_ENTRY_EN("thepassenger", "The Passenger.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 4367776),
	GAME_ENTRY_EN("thepassenger", "The Passenger.ags", "39af4e69a003d070a96bface061b819e", 1255820),
	GAME_ENTRY("thepathpumpkin", "The path of a pumpkin(The story of Skar).exe", "2c8f16a69b659ddfad4a043a24d8aa49", 264732994),  // En-Ita
	GAME_ENTRY_EN("thepenthouse", "Penthouse.exe", "0564de07d3fd5c16e6947a647061913c", 13596940),
	GAME_ENTRY("thephantominheritance", "Phantom.exe", "ec04c7917c003d9e07d4514ff25bf365", 27558669),  // Eng-Esp
	GAME_ENTRY_EN("thequestfortheholysalsa", "TheQuestForTheHolySalsa.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 38506266),
	GAME_ENTRY_EN("thequesttozooloo", "Zooloo.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 14436861),
	GAME_ENTRY_EN("thequesttozooloo", "Zooloo.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 14619530),
	GAME_ENTRY_EN("therail", "Rail.exe", "973f6b65820ca1f4e19704a49be99d76", 23852280),
	GAME_ENTRY_EN_PLATFORM("theratpack", "ratpack.exe", "495d45fb8adfd49690ae3b97921feec6", 2998306, "MAGS"),
	GAME_ENTRY_EN("theratpack", "ratpack.exe", "22b1af094ed4dc133fcd1994df7cbc0d", 87687067),  // Win
	GAME_ENTRY_EN("theratpack", "ratpack.ags", "2c80c924f1d77e9b4b843e93d83b803a", 84561287),  // Linux
	GAME_ENTRY_EN("theratpack", "ratpack.exe", "22b1af094ed4dc133fcd1994df7cbc0d", 87691724),  // Update 1
	GAME_ENTRY_EN("therebirth", "The Rebirth.exe", "05f7ff300b322bc431e7cda6a07b5976", 15889205),
	GAME_ENTRY_EN_PLATFORM("therebirth", "The Rebirth.exe", "05f7ff300b322bc431e7cda6a07b5976", 14641187, "OROW"),
	GAME_ENTRY_EN("there", "There.exe", "82da2565c456dcfb265ded6fe3189c0b", 131075666),
	GAME_ENTRY_EN("thereaper", "The Reaper.exe", "05f7ff300b322bc431e7cda6a07b5976", 9922339),
	GAME_ENTRY("therent", "L'Affitto.exe", "d25c3cb8a42c5565634dfef76b3cf83e", 4452748),  // Eng-Ita
	GAME_ENTRY_EN("theroadtomurder", "lbb.exe", "465f972675db2da6040518221af5b0ba", 1020984),
	GAME_ENTRY_EN("theroadtrip", "The RoadTrip.ags", "45c1f184bf50b4880bd591fb36e734e5", 3452180),
	GAME_ENTRY_EN("theroadtrip", "The RoadTrip.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 6564136),
	GAME_ENTRY_EN("theroadtrip", "The RoadTrip.exe", "ae2e93ae61a612dd8cad1290bd5f7443", 6564218),
	GAME_ENTRY_EN("therobolovers", "The Robolovers.exe", "618d7dce9631229b4579340b964c6810", 63850620),
	GAME_ENTRY_EN("therotaryclub", "The Rotary Club.exe", "01823d511cc00f4de6fd920eb543c6e7", 9641709),
	GAME_ENTRY_EN_PLATFORM("thesecretofchunkysalsa", "TSOCS.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3096950, "MAGS"),  //v1.01
	GAME_ENTRY_EN("thesecretofchunkysalsa", "TSOCS.exe", "acfd666096af934450d433148c72fa62", 56809941),  //v1.3
	GAME_ENTRY_EN("thesecretofgoldenriver", "tsotgr.exe", "06a03fe35791b0578068ab1873455463", 8397455),
	GAME_ENTRY_EN("thesecretofhuttongrammarschool", "1g1w.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 6584420),
	GAME_ENTRY_EN_PLATFORM("thesecretofhuttongrammarschool", "Hutton.exe", "da4472d08645ec27a0829667a33bd46f", 14270944, "Special Edition"),
	GAME_ENTRY_EN_PLATFORM("thesecretofhuttongrammarschool", "Hutton.exe", "597dae6c61fa074c87fd81a8303744a7", 13564399, "Special Edition"),
	GAME_ENTRY_LANG("thesecretofmaikeisland", "GVMI_2019.exe", "0564de07d3fd5c16e6947a647061913c", 365578377, Common::DE_DEU),
	GAME_ENTRY_EN("thesecretplanch1", "thesecretplan1.exe", "bb59de174d70797d774dec76a171352d", 447931405),
	GAME_ENTRY_EN("theshadowavenger", "Shadow Avenger.exe", "173f00e582eebd082d66f1291702b373", 48012607),
	GAME_ENTRY_EN("theshaft", "TheShaft.exe", "7ddb9e776648faed5a51170d087074e9", 24992168),
	GAME_ENTRY_EN("theshaft", "TheShaft.ags", "9a78dc07bc86148c7763c1dcbeed8835", 22504856),
	GAME_ENTRY_EN("theshortestjourney", "The Shortest Journey.exe", "7c70226b560d6d9ffd9e165d6fbddd7e", 21503346),
	GAME_ENTRY_EN("thesmallestpoints", "The Smallest Points.exe", "fd642138b7e6374ce0a0b9a1b0510500", 29534364),
	GAME_ENTRY("thesnaplock", "The Snaplock.exe", "7a3096ac0237cb6aa8e1718e28caf039", 31519102),  //Eng-Swe
	GAME_ENTRY_EN("thespiderweb", "The Spiders Web.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 26985889),  //Windows
	GAME_ENTRY_EN("thespiderweb", "The Spiders Web.ags", "0761ef29f813294a6babdae64d122f84", 23949709),  //Linux
	GAME_ENTRY_EN("thespoons", "Spoons.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3652176),
	GAME_ENTRY_EN("thestarryskyaboveme", "The Starry Sky Above Me.exe", "618d7dce9631229b4579340b964c6810", 14082659),
	GAME_ENTRY_EN("thestarryskyaboveme", "The Starry Sky Above Me.ags", "0761ef29f813294a6babdae64d122f84", 23949709),
	GAME_ENTRY_EN("thestinker", "The Stinker.exe", "be13cb758d3568b0532695081ab64683", 51481517),
	GAME_ENTRY_EN("thesummoned", "The Summoned.exe", "22d2d13d88310758fc76ff9d6e4f23b8", 15309757),
	GAME_ENTRY_EN("thesundownmystery", "Sundown.exe", "a08ab253c4d2f255b9139f2aa5fe7006", 31839270),
	GAME_ENTRY("thesurvivors", "Les_Survivants.exe", "c5d2c54c20cb606519b86d3890ee7fc0", 265445972),  //En-Fr
	GAME_ENTRY_EN("thesylph", "The Sylph.exe", "20d3b102325d45a9f5841b5cf4e9e45d", 6006983),
	GAME_ENTRY_EN("thethickening", "Retaliation Pt2.exe", "0500aacb6c176d47ac0f8158f055db83", 7084941),
	GAME_ENTRY_EN("thetombofthemoon", "TotM.exe", "f120690b506dd63cd7d1112ea6af2f77", 1632811),
	GAME_ENTRY_EN("thetowerpuzzles", "pzzles.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 1621139),
	GAME_ENTRY_EN("thetrap", "Darcy.exe", "0500aacb6c176d47ac0f8158f055db83", 1164147),
	GAME_ENTRY_EN("thetreasuredmedallion", "The Treasured Medallion.exe", "504df40bf50a0859e3dc15b000dab5f6", 1091122652),
	GAME_ENTRY_EN("thetreasuredmedallion", "The Treasured Medallion ver2.exe", "504df40bf50a0859e3dc15b000dab5f6", 1077540117),  // v2
	GAME_ENTRY_EN("thetreasureoflochinch", "LochInch.exe", "6e861b1f476ff7cdf036082abb271329", 4091983),
	GAME_ENTRY_EN("theuncertaintymachine", "TUMv11.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 20670414),
	GAME_ENTRY_EN("theuncertaintymachine", "TUMv11.exe", "b25674056fe8cc0b7bf0a4409c5c5bfc", 20672955),
	GAME_ENTRY_EN("theunicated", "TheUnicated.exe", "50cda45524f53cf673c5431e7df5b209", 4043771),
	GAME_ENTRY_EN("theunicated", "TheUnicated.exe", "50cda45524f53cf673c5431e7df5b209", 4043631),
	GAME_ENTRY_EN("theupliftmofopartyplan", "MI5 Bob.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 4092850),
	GAME_ENTRY("thevacuum", "Spacefiles.exe", "d0ba73645e3cbf8ccd65121417f9895f", 14805166),  // En-Fr-De-It
	GAME_ENTRY_EN("thevillage", "village01.exe", "7971a7c02d414dc8cb33b6ec36080b91", 17353070),  // Windows
	GAME_ENTRY_EN("thevillage", "village01.ags", "18e8ae4909b6664b07afc5befea7f3c0", 14856030),  // Linux
	GAME_ENTRY_EN("thevirus", "the Virus.exe", "0241777c2537fc5d077c05cde10bfa9f", 3212599),
	GAME_ENTRY_EN("thevisitor1", "The Visitor.exe", "615e73fc1874e92d60a1996c2330ea36", 97355779),
	GAME_ENTRY_EN("thevisitor2", "The Visitor 2.exe", "615e73fc1874e92d60a1996c2330ea36", 117471946),
	GAME_ENTRY_EN("thevisitor2", "The Visitor 2.exe", "615e73fc1874e92d60a1996c2330ea36", 117470190),
	GAME_ENTRY_EN("thevisitor3", "Visitor3.exe", "615e73fc1874e92d60a1996c2330ea36", 126688970),
	GAME_ENTRY_EN("thevoid", "thevoid.exe", "b142b43c146c25443a1d155d441a6a81", 6409223),
	GAME_ENTRY_EN("thewhitecanvas", "TWC.exe", "5871918713de85ee4bae331ca3284184", 416979913),
	GAME_ENTRY_LANG("thewhitecanvas", "Lienzo en blanco.exe", "5871918713de85ee4bae331ca3284184", 427232201, Common::ES_ESP),
	GAME_ENTRY("thewife", "The wife who wasn't there game.exe", "a524cbb1c51589903c4043b98917f1d9", 94508281),  //En-Fr
	GAME_ENTRY_EN("thewill", "The Will.exe", "f3aad40970211ace6278d9591e872b87", 16523785),
	GAME_ENTRY_EN("thewill", "The Will.exe", "f3aad40970211ace6278d9591e872b87", 16523066),
	GAME_ENTRY_EN("theworm", "Worm.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 40704028),
	GAME_ENTRY_EN("thinker", "ThinkerAdventure.exe", "bb4e465c16a70e1d6bc7d45f63a35a87", 4800053),  // Windows
	GAME_ENTRY_EN("thinker", "ThinkerAdventure.ags", "dc475c59572938bc8e489367a7084e5b", 1611297),  // Linux
	GAME_ENTRY("thiscitynight", "This City.exe", "2cd107c4a8a3ec22d8483514dfdf071c", 440671397),  // En-Ita
	GAME_ENTRY_EN("thisgame", "thisgame.exe", "74aad8dfd71ed2ae9574a60be7610c43", 6271585),
	GAME_ENTRY_EN("thisoddfeeling", "TOF.exe", "6d4adcef07a14b53369d23edf5117252", 6043529),
	GAME_ENTRY_EN("thistall", "YouMustBe.exe", "06a03fe35791b0578068ab1873455463", 6968891),
	GAME_ENTRY_EN("thisway", "This is the Way.exe", "9da85cf72af670ca88f9aea5a43c6b14", 64194258),
	GAME_ENTRY_EN("threeguyswalkintoheaven", "Three Guys Walk Into Heaven.exe", "c45653d1c856f002ceb59a5b865ab187", 2896291),
	GAME_ENTRY_EN("throwme", "throwmeinriver.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 101323436),
	GAME_ENTRY_EN("thrymly", "Thrymly Disguised.exe", "5d3eaccd432b93d4387fd6c20838c18b", 15301211),
	GAME_ENTRY_EN("tigerhawk", "Tiger Hawk Fighter.exe", "afe40dc1416dd51e896ee0444d799f07", 36051983),
	GAME_ENTRY_PLATFORM("tijdtripper", "HUA_playthrough_v3.exe", "a524cbb1c51589903c4043b98917f1d9", 36053213, "Prototype"),  // Eng-Hun
	GAME_ENTRY_EN("tilepuzzle", "Puzzle01.exe", "615e73fc1874e92d60a1996c2330ea36", 2819964),
	GAME_ENTRY_EN("tillcows", "Till Cows Tear Us Apart.exe", "2ee7b3c494b8f33b6967525ef07de69a", 53504480),
	GAME_ENTRY_EN("tiltor", "Tiltor.exe", "f120690b506dd63cd7d1112ea6af2f77", 17561878),
	GAME_ENTRY_EN("timeoutjoint", "timeout.exe", "06a03fe35791b0578068ab1873455463", 1647055),
	GAME_ENTRY("timeparadox", "REDQUEST.exe", "465f972675db2da6040518221af5b0ba", 32472406),  //Eng-Rus
	GAME_ENTRY_EN("timeparadoxdan", "Time Paradox.exe", "06a03fe35791b0578068ab1873455463", 1592755),
	GAME_ENTRY_EN("timeparadoxenk", "timeparadoxenk.exe", "06a03fe35791b0578068ab1873455463", 1840636),
	GAME_ENTRY("timequest1", "gam.exe", "465f972675db2da6040518221af5b0ba", 5669007),  //Eng-Rus
	GAME_ENTRY("timequest1", "TQ.exe", "465f972675db2da6040518221af5b0ba", 5671274),  //Eng-Rus v1.1
	GAME_ENTRY("timequest2", "TimeQ2.exe", "465f972675db2da6040518221af5b0ba", 5838823),  //Eng-Rus
	GAME_ENTRY_EN("timesinkofchronos", "Timesink.exe", "2bc8f994a7d1e05ed45f35abf2128231", 127528679),
	GAME_ENTRY_EN("timestone", "Time Stone.exe", "a2b79cf0a58853225f7df221a6a9e528", 16256197),
	GAME_ENTRY_EN("timestone", "Time Stone.exe", "a2b79cf0a58853225f7df221a6a9e528", 16256330),  // v1.1
	GAME_ENTRY_EN("timothylande", "Timothy Lande.exe", "0500aacb6c176d47ac0f8158f055db83", 13874628),
	GAME_ENTRY_EN("tinysoccer", "Tiny Picky Football Manager.exe", "08cb9ad3be9f966132d0c17f0dee471c", 27090817),  // Windows
	GAME_ENTRY_EN("tinysoccer", "Tiny Picky Football Manager.ags", "7fb21c498a2b2e2184dd4b351cb82edf", 3251218),  // Linux
	GAME_ENTRY_EN("tirnanog", "TirNaNog.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 240701949),
	GAME_ENTRY("toffeetrouble", "ToffeeTrouble.exe", "acda5a3cf15b9e9e1d96bdc78f6eff60", 74459735),  // Eng-Deu
	GAME_ENTRY_EN_PLATFORM("toffeetrouble", "ToffeeTrouble.exe", "f7aeb21fc351474cb039769c23b2e8e9", 69824254, "MAGS"),
	GAME_ENTRY_EN("tomandjimi", "Tom and Jimi in Blowout!.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 33291483),  // Windows
	GAME_ENTRY_EN("tomandjimi", "Tom and Jimi in Blowout!.ags", "9efd44ffa71e3d29f44ccb5f4bb9e1fe", 30255303),  // Linux
	GAME_ENTRY_EN_PLATFORM("tomandjimi", "Tom and Jimi in Crispy Duck.ags", "3ba164667625dc8763524af28c862c7f", 27551972, "Preview"),  // Linux preview
	GAME_ENTRY("tomateundescanso", "tomate.exe", "3128b9f90e2f954ba704414ae854d10b", 8865274),  //Eng-Esp
	GAME_ENTRY_EN("tombhunter", "Tomb Hunter.exe", "338fa79960d40689063af31c671b8729", 137077729),
	GAME_ENTRY_EN("tomeslayne", "TOMES 1.exe", "d93777b4ac9542258ab4013ad14b271e", 13251203),
	GAME_ENTRY_EN("tomhanksaway", "Tom Hanks Away.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1589035),
	GAME_ENTRY_EN("tomten", "Adventskalender 2021.exe", "63f8a60cc094996bd070e68cb3c4722c", 4912971),
	GAME_ENTRY_EN("tonyspaghetti", "Tony's Spaghetti.exe", "1177e20f6a84ded5fad2a5d79fff8cb5", 4520338),
	GAME_ENTRY_EN_PLATFORM("tonyspaghetti", "Tony's_Spaghetti.exe", "1177e20f6a84ded5fad2a5d79fff8cb5", 4740438, "Post-Jam"),
	GAME_ENTRY("topus", "Topus.exe", "25f2daf37e9fa873e0ddd765121d5ef0", 90520515),  // En-Fr Win
	GAME_ENTRY("topus", "Topus.ags", "5189846132206c64d04be3e0f9c1d13d", 88133043),  // En-Fr Linux
	GAME_ENTRY_EN("totheend", "chemin.exe", "06a03fe35791b0578068ab1873455463", 3179064),
	GAME_ENTRY_EN("tothemom", "To The Mountains Of Madness.exe", "6b1702aec6d0a44bda59ef8f229956c5", 18160599),
	GAME_ENTRY_EN("tothemom", "To The Mountains Of Madness.ags", "62c97d20188f62b1b92473bb5ca70152", 15674823),
	GAME_ENTRY_EN("totti1", "Tochan.exe", "2ca80bd50763378b72cd1e1cf25afac3", 9982703),
	GAME_ENTRY_EN("totti2", "good evening tochan.exe", "2ca80bd50763378b72cd1e1cf25afac3", 3408230),
	GAME_ENTRY_EN_PLATFORM("tooblivion", "who07.exe", "465f972675db2da6040518221af5b0ba", 1889598, "Beta"),
	GAME_ENTRY_EN("toomanykittens", "Too Many Kittens.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 3926440),
	GAME_ENTRY_EN("tots", "T.O.T.S.exe", "82da2565c456dcfb265ded6fe3189c0b", 7662566),
	GAME_ENTRY_EN("tradeforward", "Trade it forward.exe", "776a62db4387dc68be92ef9933399fd5", 160811438),  //v1.1
	GAME_ENTRY("trancepacific", "MAGS Sep 2007.exe", "f120690b506dd63cd7d1112ea6af2f77", 6007305),  // En-Fr-It
	GAME_ENTRY_EN_PLATFORM("trancepacific", "MAGS Sep 2007.exe", "f120690b506dd63cd7d1112ea6af2f77", 5999008, "MAGS"),  //v1.02
	GAME_ENTRY("trappedbalcony", "TrappedOnTheBalcony.exe", "733a619770a4f3673e28773b2c3f0fd9", 29228892),  // Eng-Esp
	GAME_ENTRY("trappedbalcony", "TrappedOnTheBalcony.exe", "733a619770a4f3673e28773b2c3f0fd9", 29229026),  // Eng-Esp
	GAME_ENTRY_EN("trappedhell1", "phycho.exe", "0500aacb6c176d47ac0f8158f055db83", 697557),
	GAME_ENTRY_EN("trappedhell2", "richphycho2.exe", "0500aacb6c176d47ac0f8158f055db83", 715802),
	GAME_ENTRY("trappedinabuilding", "T I A B.exe", "722c8bd17ace161f1bba2d9d92806eaf", 2406330),  //v1 Eng-Norsk
	GAME_ENTRY_EN("trappedinabuilding", "T I A B.exe", "722c8bd17ace161f1bba2d9d92806eaf", 5370872),
	GAME_ENTRY_EN("trappedinabuilding", "T I A B.exe", "722c8bd17ace161f1bba2d9d92806eaf", 5370873),
	GAME_ENTRY("trashquest", "TQ.exe", "465f972675db2da6040518221af5b0ba", 2420457),  //Eng-Rus
	GAME_ENTRY_LANG("tresacordes", "Tres Acordes.exe", "38375404171b2419c5cbefa69b2a6ac0", 33254762, Common::ES_ESP),
	GAME_ENTRY_EN_PLATFORM("travelers", "Game.exe", "7971a7c02d414dc8cb33b6ec36080b91", 3349037, "Prototype"),
	GAME_ENTRY("trevordaisoninouterspace", "TrevorDaison.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 22641402),  // En-De
	GAME_ENTRY("trexmusclesam1", "Trex and Muscle Sam.exe", "91696f9333f36bdba272220c644c72e7", 120822652),
	GAME_ENTRY("trexmusclesam1", "Trex and Muscle Sam.exe", "91696f9333f36bdba272220c644c72e7", 120822636),  // En-Ita
	GAME_ENTRY_LANG("trexmusclesam1", "T-REX e MUSCLE SAM big trouble in S P F.exe", "2478c1d6e6ae0b48b97cd4104cf0d8a6", 120845536, Common::IT_ITA),  //Ita dub
	GAME_ENTRY("trianglehead", "TriangleHeadsAdventure.exe", "b5c41e08919834b549ca350a28f18f87", 3949664),  // Eng-Tur
	GAME_ENTRY_EN("trickers", "Trickers.exe", "dbe9bab672130b9a84925cb6da64a3db", 4701302),
	GAME_ENTRY_EN("trilbysnotes", "notes.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5736960),
	GAME_ENTRY_EN_PLATFORM("trilbysnotes", "notes se.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 5880445, "Special Edition"),
	GAME_ENTRY_EN("trilbytheartoftheft", "artoftheft.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8940823),
	GAME_ENTRY_EN("trilbytheartoftheft", "artoftheft.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 8940666), // v1.3.4
	GAME_ENTRY_EN("trippy", "Trippy.ags", "637d8271310d556da9dd6cdb5bd709d9", 14578561),
	GAME_ENTRY_EN("trivialpassyouit", "Trivial Pass You It.exe", "01823d511cc00f4de6fd920eb543c6e7", 2737077),
	GAME_ENTRY_EN("trollsong", "Troll Song Verse One.exe", "7c80bfa036bef402ad957a0885926645", 20905473),
	GAME_ENTRY_EN("tropicjim", "tropicJim.exe", "2a16e8992e482076d1b0ae221583b68d", 8883928), // Win 1.05
	GAME_ENTRY_EN("tropicjim", "tropicJim.exe", "54d08fb7c836263666a2105e6522ee23", 8958178), // Win 1.06
	GAME_ENTRY_EN("tropicjim", "tropicJim.ags", "cf9e62ae9480e9b25ef6738de69a983a", 5816516), // Linux 1.05
	GAME_ENTRY_EN("tropicjim", "tropicJim.ags", "c193e4ea4f0cb6cd52673067da3e770f", 5816526), // Linux 1.06
	GAME_ENTRY_EN("troublekingdom", "Trouble Kingdom.exe", "0241777c2537fc5d077c05cde10bfa9f", 13787675),
	GAME_ENTRY_EN("troublingteleportation", "Troubling Teleportation.exe", "28a946e8a278814362613f8600375438", 43468313),
	GAME_ENTRY_EN("trumpsbed", "SITB.exe", "c1a6f1752b85d9127d6f9d11bcb124bb", 67920649),  // Windows
	GAME_ENTRY_EN("trumpsbed", "SITB.ags", "0d79c6f044b0f44b07e8800c5224fbf8", 65180409),  // Linux
	GAME_ENTRY_EN("truthmatter", "Liars.exe", "615e73fc1874e92d60a1996c2330ea36", 6636818),
	GAME_ENTRY_EN("tunnelvision", "TunnelVision.exe", "aa8082a7cc63d9b14d6be5dac69f10ac", 27915422),  // Win/Mac
	GAME_ENTRY_EN("tunnelvision", "TunnelVision.exe", "aa8082a7cc63d9b14d6be5dac69f10ac", 27915669),  // Win/Mac
	GAME_ENTRY_EN("tunnelvision", "TunnelVision.ags", "ca92226f62424c66bff8ccfbe7faffc2", 24755338),  // Linux
	GAME_ENTRY_EN("turtlesintime", "Turtles.exe", "465f972675db2da6040518221af5b0ba", 2309552),
	GAME_ENTRY("tvabroder", "TvaBroder.exe", "7a669a96e488653db661d285bbc12783", 17875297),  // En-Sw
	GAME_ENTRY_EN("tvquest2015", "TvQuest.exe", "c3b72132686c4a102b3365d380c8ae3a", 10999697),
	GAME_ENTRY_EN("twelvethirteenep1", "1213.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2940069),
	GAME_ENTRY_EN("twelvethirteenep1", "1213.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2942386),
	GAME_ENTRY_EN_PLATFORM("twelvethirteenep1", "1213_ep1.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3283696, "Special Edition"), // From SE
	GAME_ENTRY_EN("twelvethirteenep2", "1213_ep2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3602361),
	GAME_ENTRY_EN_PLATFORM("twelvethirteenep2", "1213_ep2.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3656275, "Special Edition"), // From SE
	GAME_ENTRY_EN("twelvethirteenep3", "1213_ep3.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3205606),
	GAME_ENTRY_EN_PLATFORM("twelvethirteenep3", "1213_ep3.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 3246117, "Special Edition"), // From SE
	DETECTION_ENTRY_GUIO("twelvethirteense", "1213 SE.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 644458, Common::EN_ANY, nullptr, GUIO2(GUIO_NOLANG, GUIO_NOLAUNCHLOAD), ADGF_NO_FLAGS, 0),  // Menu
	GAME_ENTRY_EN("twentiesflappersvsthemummy", "Twenties Flappers vs The Mummy.exe", "a524cbb1c51589903c4043b98917f1d9", 15042196),
	GAME_ENTRY("twoghosts", "Ghosts.exe", "2f2bc0b9c539d20529c0e343315d5f65", 4525238),  // En-Fr
	GAME_ENTRY("twoghosts", "Ghosts.exe", "2f2bc0b9c539d20529c0e343315d5f65", 4525343),  // En-Fr
	GAME_ENTRY_EN("twoofakind", "toak.exe", "465f972675db2da6040518221af5b0ba", 24644765),
	GAME_ENTRY_EN("ugalembrace", "UgalsEmbrace.exe", "308026eea716ec1aeed39f7f8d8cfd18", 30046377),
	GAME_ENTRY_EN("uglyfiles", "ugly.exe", "0394af1c29e1060fcdbacf2a3dd9b231", 4169486),
	GAME_ENTRY_EN("ulitsadimitrova", "ulitsa.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 291828379),
	GAME_ENTRY_EN("unbound", "Unbound.exe", "900b277d7e1601c65b42868cd7fae662", 10448702),
	GAME_ENTRY_EN("uncontrollable", "OROW2015.exe", "615e73fc1874e92d60a1996c2330ea36", 3012777),
	GAME_ENTRY_EN("underthebed", "The Weird Thing Under The Bed.exe", "9982a80801abee56c9c8977d5e7424d7", 43104879),
	GAME_ENTRY_EN("underthebed", "The Weird Thing Under The Bed.ags", "630e945a0e4b88bafd5e20761d724c0f", 39995995),
	GAME_ENTRY_EN("underwateradv", "fishres.exe", "3128b9f90e2f954ba704414ae854d10b", 3970713),
	GAME_ENTRY_EN("underworld", "Underworld.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 5147661),
	GAME_ENTRY_EN("undyep1", "heyderundy.exe", "39fe9b76597c2d8c14922b8369a4a4db", 9842962),  // v1.2
	GAME_ENTRY_EN("undyep1", "heyderundy.exe", "39fe9b76597c2d8c14922b8369a4a4db", 10055040),  // v1.3
	GAME_ENTRY_EN("undyep2", "UndyXmas.exe", "39fe9b76597c2d8c14922b8369a4a4db", 7457723),  // v1.0
	GAME_ENTRY_EN("unexpectedguest", "unexpectedGuest.exe", "5e1d1fbbaadb46b5cfd5474d71080c9d", 4541793),
	GAME_ENTRY_EN("unexpectedquest", "UQ.exe", "f120690b506dd63cd7d1112ea6af2f77", 1837663),
	GAME_ENTRY_EN_PLATFORM("unfair", "Unfair.exe", "8d1ff95c16500befbdc72260d461d73f", 5203436, "MAGS"),
	GAME_ENTRY_EN("unfair", "Unfair.exe", "8d1ff95c16500befbdc72260d461d73f", 5212077),
	GAME_ENTRY_EN("unfinished", "mags.exe", "0710e2ec71042617f565c01824f0cf3c", 12092514),
	GAME_ENTRY_EN("unfinishedbusiness", "business.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 2202413),
	GAME_ENTRY("unfinishedtales", "shst.exe", "28f82e420b82d07651b68114f90223c8", 1013809),  // Eng-Esp
	GAME_ENTRY_EN("unfoldingspider", "The Unfolding Spider.exe", "615e73fc1874e92d60a1996c2330ea36", 4099592),
	GAME_ENTRY_EN("unganeedsmumba", "UNGA needs MUMBA.exe", "2ca6bb6d5b2710ac89fea7d69c2eaf77", 5470102),
	GAME_ENTRY_EN("unintelligentdesign", "UD.exe", "5ca1bc01c5a45388bd5c84ef36077361", 6019596),
	GAME_ENTRY_EN("unkhour_apple", "apple.exe", "201ac17f13d0a9bcfc99213ea0161757", 1099184),
	GAME_ENTRY_EN("unkhour_nmhour1", "nmhour1.exe", "9cf51833e787cc919837d9a8bd8fc14c", 1377323),
	GAME_ENTRY_EN("unlabeledtape", "cv.exe", "465f972675db2da6040518221af5b0ba", 1502178),
	GAME_ENTRY_EN("unlicensedkill", "UTK.exe", "0564de07d3fd5c16e6947a647061913c", 10766019),
	GAME_ENTRY_EN_PLATFORM("unlikelyprometheus", "The Unlikely Prometheus.exe", "a370a1db43a1d376d09e43469abba1d1", 56868120, "GameJam Build"),
	GAME_ENTRY_EN("unprintablemagenta", "Magenta.exe", "615e73fc1874e92d60a1996c2330ea36", 113259258),
	GAME_ENTRY_LANG("unprofe", "Un Profe.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 3465541, Common::ES_ESP),
	GAME_ENTRY_EN("unraveling", "unraveling.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 2927577),
	GAME_ENTRY_EN("unscripted", "LD47.ags", "b7de4a31e3118480997975b7c6c204e7", 3547098),  // Linux
	GAME_ENTRY_EN("unscripted", "LD47.exe", "7c6e063343fc2ec2bfffc93a1bbd6cfe", 6034922),  // Windows
	GAME_ENTRY_EN("unsolvedmystery", "UCOM.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 1593175),
	GAME_ENTRY_EN("untildawn", "Until Dawn Tomorrow.exe", "7f151ffa6f87f2c5690d308bfba59805", 256429497),
	GAME_ENTRY_EN("untilfurthernotice", "Until further notice.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 391431917),
	GAME_ENTRY_EN("untilfurthernotice", "Until further notice.ags", "68d52c561704d8690e258d78eec52b1c", 388951773),
	GAME_ENTRY_EN("upducted", "Upducted.exe", "4f50fc611da96a6d5453cdbefa971521", 6504712),  // Windows
	GAME_ENTRY_EN("upducted", "Upducted.ags", "803a57bc967b6fa1293373fe203ae773", 3471092),  // Linux
	GAME_ENTRY("urbanwitchstory", "Urban Witch Story.exe", "636950f50c877767bca7e2b1250632f7", 441430956),  //Eng-Esp Win
	GAME_ENTRY("urbanwitchstory", "Urban Witch Story.ags", "92f7e6a4ffe81d40c1f20eb68dcb5ae7", 346573336),  //Eng-Esp Win 1.4
	GAME_ENTRY("urbanwitchstory", "Urban Witch Story.ags", "ea3bede3ffc8904199e915274feddfe6", 438411774),  //Eng-Esp Lin
	GAME_ENTRY_EN("uydearmyfriendsremake", "Urusei Yatsura.exe", "df94207a3720d4bb2404ca8be23e9d72", 4252149), // version R12b
	GAME_ENTRY_EN("urgentquest", "urgent quest.exe", "655363c390c7ae7225c237108edf50b7", 6260710),
	GAME_ENTRY("uropa", "U-ropa.exe", "0241777c2537fc5d077c05cde10bfa9f", 12999971),  //En-Fr
	GAME_ENTRY("utopiaoftyrant", "Utopia_of_a_Tyrant.exe", "f8a42e09e40a7ab1cd2a21f74a5e0980", 80966652),  //Eng-Tur
	GAME_ENTRY_EN("vacantbodies", "VacantBodies.exe", "5b916edf70413f20906ceb9c05ac65b9", 9095068),
	GAME_ENTRY_EN("vacationquestthing", "MGA.exe", "95b7dd55f6e15c8a2118856ed9fe8ff9", 943647),
	GAME_ENTRY_EN("valhallaextinction", "MAGSjuly2016.exe", "95864ae16f51c512985007dca83c1370", 4464850),
	GAME_ENTRY_PLATFORM("vankairbreak", "Airbreak.exe", "28567bd2dc355a02a0ba58749e753f87", 23126098, "MAGS"), //En-De v1.01
	GAME_ENTRY("vankairbreak", "Airbreak.exe", "28567bd2dc355a02a0ba58749e753f87", 23126775), //En-De v1.03
	GAME_ENTRY_EN_PLATFORM("vectorvendetta", "Vector Vendetta.exe", "c4f5b7b29be90ba0f8128298afb917de", 2717602, "MAGS 2008"),
	GAME_ENTRY_EN_PLATFORM("vectorvendetta", "Vector Vendetta.exe", "c4f5b7b29be90ba0f8128298afb917de", 7952316, "MAGS 2010"),
	GAME_ENTRY_EN("vectorvendetta", "Vector Vendetta.exe", "6a2022426677308b7e4ed2a4699758b1", 5591078),
	GAME_ENTRY_EN("vegetablepatch", "VPET.exe", "0b7529a76f38283d6e850b8d56526fc1", 2656540),
	GAME_ENTRY_EN("vegetablepatch2", "VPET!2!.exe", "0b7529a76f38283d6e850b8d56526fc1", 12933096),
	GAME_ENTRY_EN("veggietales3d", "Veggie Tales 3D.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 5645075),
	GAME_ENTRY_EN("venator", "notarpg.exe", "58fa77748873bdecee8b3ef1b564c45e", 88605764),
	GAME_ENTRY_EN("vertigo", "Vertigo.exe", "0564de07d3fd5c16e6947a647061913c", 4562464),  // Windows
	GAME_ENTRY_EN("vertigo", "Vertigo.ags", "cd16482c8d241fe2b66a57c6ebc4918f", 1529868),  // Linux
	GAME_ENTRY_EN("verybadtaste", "Very Bad Taste.exe", "afe40dc1416dd51e896ee0444d799f07", 17721013),
	GAME_ENTRY_EN("veteranshootout", "Vet kill.exe", "0710e2ec71042617f565c01824f0cf3c", 3641458),
	GAME_ENTRY_EN("veteranshootout", "ac2game.ags", "612356b09ddfadfbacfe0139c24b5e76", 3089506),
	GAME_ENTRY_EN("vexationisland", "rodney.exe", "615e73fc1874e92d60a1996c2330ea36", 9433484),
	GAME_ENTRY_EN("vicwreckleshalloweencostume", "vic.exe", "01d0e6bd812abaa307bcb10fc2193416", 2913241),
	GAME_ENTRY_EN("vicwreckleshalloweencostume", "vic.exe", "615e73fc1874e92d60a1996c2330ea36", 3167446),
	GAME_ENTRY("vikingguardsman", "Byzanz.exe", "36f44e064eab15e502caeb60fd09f52d", 11244691),  //Eng-Swe
	GAME_ENTRY_EN("virmachina", "Cyber.exe", "9da85cf72af670ca88f9aea5a43c6b14", 24619320),
	GAME_ENTRY_EN("virmachina", "Cyber.ags", "82a1293d6e986bed1d207351bf855182", 22029096),
	GAME_ENTRY_EN("virtualpiano", "Synth.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1419081),
	GAME_ENTRY_EN("vivarium", "Vivarium.exe", "e5744908aea934a7adfe301a27c71168", 77505388),
	GAME_ENTRY_EN("vizita", "Vizita.ags", "cdcb14d867fabaf7571b9951b34775ad", 903382075),
	GAME_ENTRY_EN("vizita", "Vizita.ags", "ce63f354683d007fd1a729ad554ca147", 903415457),
	GAME_ENTRY_PLATFORM("vizita", "Vizita Romanian.ags", "dad6359d89eb3096b6710ad9cc287d1a", 900793616, "Romanian"),
	GAME_ENTRY_PLATFORM("vizita", "Vizita Romanian.ags", "5c963cde50bab26bf0f93d5e32daa3bf", 900793617, "Romanian"),
	GAME_ENTRY_EN("voxinrama", "voxinrama.ags", "fcf753ad836899ef4b8f120310252d07", 47620042),  // itch.io client
	GAME_ENTRY_LANG("voyagesfuturo", "AtelierRAB.exe", "495d45fb8adfd49690ae3b97921feec6", 1818625248, Common::FR_FRA),  // Windows
	GAME_ENTRY_LANG("voyagesfuturo", "AtelierRAB.ags", "6550012348b11be9ef0063c4720fb9bf", 1816144592, Common::FR_FRA),  // Linux
	GAME_ENTRY_EN("voodoodave", "vdtm.exe", "06a03fe35791b0578068ab1873455463", 4451423),
	GAME_ENTRY_PLATFORM("vrrontour", "Virtual Reality.exe", "430eaebb21d406061d67a9972ad33947", 9448048, "v05"),
	GAME_ENTRY_PLATFORM("vrrontour", "Virtual Reality.exe", "f8e8f781a4a95c2bfea5f54b085df550", 10327680, "v08"),
	GAME_ENTRY_EN("wagesofdarkness", "wages_of_darkness.exe", "615e73fc1874e92d60a1996c2330ea36", 26997472),
	GAME_ENTRY_EN_PLATFORM("wagesofdarkness", "wages_of_darkness.exe", "82da2565c456dcfb265ded6fe3189c0b", 20873104, "MAGS"),
	GAME_ENTRY_EN("waguogambo", "Wagu.ags", "6620245ab6ad49c2fdf032dc824acac1", 2984010),
	GAME_ENTRY_EN("waitingboyfriend", "1983.exe", "495d45fb8adfd49690ae3b97921feec6", 2591438),
	GAME_ENTRY_EN("wakeup", "Wakeup.exe", "60289244201d5cde2ac388c2d43ec8cd", 9249285),
	GAME_ENTRY_EN("wallardgromoid", "plannedday.exe", "82da2565c456dcfb265ded6fe3189c0b", 122212233),
	GAME_ENTRY_EN("walle", "walle.exe", "1f15bff3fe3922b9c0fa0ef1c7dbf61b", 9613116),
	GAME_ENTRY_EN("walkcyclegen", "walkcyclist.exe", "1e81f0cba7e94fb658acd8e24ff1089f", 1861346),
	GAME_ENTRY("waltersasteroid", "HDGame.exe", "465f972675db2da6040518221af5b0ba", 8390872),  //Eng-Deu
	GAME_ENTRY_EN("warningfragilech1", "WFragile.exe", "05f7ff300b322bc431e7cda6a07b5976", 4758891),
	GAME_ENTRY_EN("warptile", "WarpTile.exe", "e296afc66e33b911c20437912be78573", 18679078),
	GAME_ENTRY_EN("warstars", "MAGS.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 2356146),
	GAME_ENTRY_EN("washedashore", "Achtung!.exe", "06a03fe35791b0578068ab1873455463", 7926840),
	GAME_ENTRY_EN_PLATFORM("washedashore", "Washed.exe", "06a03fe35791b0578068ab1873455463", 2771873, "Deluxe"),
	GAME_ENTRY_LANG("waskocleaner", "WaskoTheCleaner.ags", "8e1480089e43b1e25137a495dcaa151c", 4975086, Common::ES_ESP),
	GAME_ENTRY_EN("wasted", "Wasted.exe", "f120690b506dd63cd7d1112ea6af2f77", 27870661),
	GAME_ENTRY_EN("waterquest", "KOSTAS.exe", "f120690b506dd63cd7d1112ea6af2f77", 12403015),
	GAME_ENTRY_EN("wduprodigal", "vtprodigal.exe", "9d430fcdf4e2787ed39185924246da7c", 294267579),  // Windows
	GAME_ENTRY_EN("wduprodigal", "vtprodigal.ags", "44a50cc46c4b2b6940d0070c78a0a7b3", 291157671),  // Linux
	GAME_ENTRY_EN("wearevectors", "WAV.exe", "a524cbb1c51589903c4043b98917f1d9", 5851536),
	GAME_ENTRY_EN_PLATFORM("weathered", "Weathered.exe", "60850a0b0f1ef2e2deb15a815c0a1715", 110310651, "Beta"),
	GAME_ENTRY_LANG("weepinglilium", "weepinglilium.ags", "b570d980a264f085d00f52936e72ad43", 99619088, Common::IT_ITA),  // itch.io client
	GAME_ENTRY_EN("wegotlost", "dream.exe", "0710e2ec71042617f565c01824f0cf3c", 8820049),
	GAME_ENTRY_EN("weightloss", "Narcoleptic Weight Loss Expert.exe", "6d2f8e80c5f2372b705fdd4cc32f3579", 3940985),
	GAME_ENTRY_EN("wellmeetagain", "We'll meet again.exe", "981e60a9be515bb56c634856462abbc7", 105830995),  //Windows
	GAME_ENTRY_EN("wellmeetagain", "We'll meet again.ags", "a6ecfd9b1980220d23008cf19adc5648", 102795839),  //Linux
	GAME_ENTRY_EN("wellwellwell", "wellwellwell.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 3552969),
	GAME_ENTRY("wendywhedon1", "WendyWhedon.exe", "7ddb9e776648faed5a51170d087074e9", 1144408799),  // En-Fr Win
	GAME_ENTRY("wendywhedon1", "WendyWhedon.ags", "3242ae40d7f7239d0a9a20562fbe2cfe", 1141921487),  // En-Fr Linux
	GAME_ENTRY("wendywhedon2", "WendyWhedon2.exe", "0564de07d3fd5c16e6947a647061913c", 1445032064), // En-Fr
	GAME_ENTRY("wendywhedon3", "WendyWhedon3.exe", "0564de07d3fd5c16e6947a647061913c", 1863230268), // En-Fr
	GAME_ENTRY_EN("wereweever", "didweever.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 83746941),  // v0.1.1
	GAME_ENTRY_EN("westroot", "PotatoWestern.exe", "06a03fe35791b0578068ab1873455463", 2052270),
	GAME_ENTRY_EN("wet", "Wet.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 1271457),
	GAME_ENTRY_EN("whacksoul", "WAS.exe", "0564de07d3fd5c16e6947a647061913c", 49649029),
	GAME_ENTRY_EN("whackunilin", "whack.exe", "88cf59aad15ca331ab0f854e16c84df3", 1246740),
	GAME_ENTRY_EN("whataclown", "whataclown.ags", "fe28ecf27a7962fe8c00f32a84480e71", 8460122),  // itch.io client
	GAME_ENTRY_EN("whathappened", "what!.exe", "08cb9ad3be9f966132d0c17f0dee471c", 48181831),
	GAME_ENTRY_EN("whatisthat", "somtin.exe", "0500aacb6c176d47ac0f8158f055db83", 1289618),
	GAME_ENTRY_EN("whatlinusbruckmansees", "LinusBruckman.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 320171493),
	GAME_ENTRY_EN_PLATFORM("whatsinthesafe", "whatsinthesafe.exe", "618d7dce9631229b4579340b964c6810", 14614945, "Original Version"),  // v1.0
	GAME_ENTRY_EN("whatsinthesafe", "What's In the Safe - Talkie.exe", "23a67b6de10ec35e9f5a4dfc7d928222", 15086904),  // v1.1
	GAME_ENTRY_EN("whatspunkyfound", "What Spunky Found.exe", "618d7dce9631229b4579340b964c6810", 6481808),  // v1.1.0
	GAME_ENTRY_EN("wheeloftorture", "Wheel of Torture.exe", "f6006e2f65412b684fb537cf49d35672", 15090596),
	GAME_ENTRY_EN("whentheworldcalls", "When the world calls.exe", "338fa79960d40689063af31c671b8729", 1111169904),
	GAME_ENTRY_EN("whentimestops", "When Time Stops.exe", "c48d0beedcdc1b05e9e25dcd60de46a9", 142215901),
	GAME_ENTRY_EN("where", "Where.exe", "0b7529a76f38283d6e850b8d56526fc1", 6808177),
	GAME_ENTRY_EN("wherebedragons", "Where Be Dragons.exe", "d8ccbb83c73ca8520b19412ce0d8de10", 2607599),
	GAME_ENTRY_EN("wheredidhumansgo", "AdvGameChallenge22.exe", "805aebfc9a02cd02f892de6cd2cb7c87", 34137412),
	GAME_ENTRY("wheredidhumansgo", "AdvGameChallenge22.exe", "805aebfc9a02cd02f892de6cd2cb7c87", 34137817),  // Windows Eng-Ita
	GAME_ENTRY("wheredidhumansgo", "AdvGameChallenge22.ags", "d8eefb1fd0d9132d413f3d977c9712f8", 31022789),  // Linux Eng-Ita
	GAME_ENTRY_EN("wheredidsamgo", "WDSG.exe", "0710e2ec71042617f565c01824f0cf3c", 4921841),
	GAME_ENTRY("wheredidsamgo", "WDSamGo.exe", "90c820d7154dda79ac2af5fbbf60887f", 10719373),  // En-It
	GAME_ENTRY_EN_PLATFORM("wheresmhatma", "WMHM.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 50659613, "MAGS"),
	GAME_ENTRY_EN_PLATFORM("wheresmhatma", "g3.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 50659613, "MAGS"),
	GAME_ENTRY_EN("wheresmhatma", "WMHM.exe", "25f919423520b921a041ec854e3a0217", 51766424),  //v2.0
	GAME_ENTRY_EN("whichwitch", "witch.exe", "06a03fe35791b0578068ab1873455463", 10110407),  //v1.1
	GAME_ENTRY("whitemoredeadep1", "WIMD Ep1.exe", "615e73fc1874e92d60a1996c2330ea36", 83348710),  //En-Fr
	GAME_ENTRY_LANG("whitemoredeadep1", "WIMD Ep1.exe", "615e73fc1874e92d60a1996c2330ea36", 83183259, Common::FR_FRA),
	GAME_ENTRY_LANG("whitemoredeadep2", "WIMD-EPII.exe", "e1470998fea556ea5f2342fd8779ccce", 340727789, Common::FR_FRA),
	GAME_ENTRY_LANG("whitemoredeadep3", "WIMD-Episode3.exe", "5529522460cb27d6a4f2619aee618590", 624193679, Common::FR_FRA),
	GAME_ENTRY_EN("whodunit", "Whodunit.exe", "089fab88e6e1075a2f5b271f6f5b3c57", 26093101),
	GAME_ENTRY_EN("whokilledbambi", "wkb.exe", "0500aacb6c176d47ac0f8158f055db83", 2216313),
	GAME_ENTRY_EN("whokilledwho", "WhoKilledWho.exe", "06a03fe35791b0578068ab1873455463", 2065059),
	GAME_ENTRY_EN("whopper", "What a whopper.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 17615905),  // Windows
	GAME_ENTRY_EN("whopper", "What a whopper.ags", "62609ee630e7fa32abab1ebb08f0b289", 14579725),  // Linux
	GAME_ENTRY_EN("whoseturn", "Turnip.exe", "2f2bc0b9c539d20529c0e343315d5f65", 63902342),
	GAME_ENTRY("whowantstoliveagain", "Bond.exe", "465f972675db2da6040518221af5b0ba", 2497443),  // En-De
	GAME_ENTRY_EN("whowantstoliveforever", "WWtLF.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 6917937),
	GAME_ENTRY_EN("whywrongface", "FaceMaker Disguise.ags", "43dbfc82b03ce6421f0fcfdbe438fa06", 38303813),
	GAME_ENTRY_EN("wickedwitchwest", "OzAGS.exe", "7ddb9e776648faed5a51170d087074e9", 6140184),
	GAME_ENTRY_EN("wickedwitchwest", "OzAGS.ags", "29a4f5a59d7e5ca48e831a3cd7207d75", 3652872),
	GAME_ENTRY_EN("wilfred2088", "Wilfred.exe", "d5ad2548650f8933378adfec9a2bbc31", 19882396),
	GAME_ENTRY_EN("williamsnightmare", "Killer.exe", "0b7529a76f38283d6e850b8d56526fc1", 3991683),
	GAME_ENTRY_LANG("willowhouse", "Game.exe", "5bc696cf7178870b21db6ac87972befd", 132161306, Common::DE_DEU),
	GAME_ENTRY_EN("winamillion", "Win a Million!.exe", "9f8a9d74c09f188af9af3e263f8b59bc", 14440837),  // Windows
	GAME_ENTRY_EN("winamillion", "Win a Million!.ags", "f27b5383ae90ac0bd30f4f0869aa91e9", 11404657),  // Linux
	GAME_ENTRY_EN("winnersdontdodrugs", "windrugs.exe", "ff3358d8f2726d544aadfde4f1ec8407", 2573704),
	GAME_ENTRY_EN("winterrose", "Winter Rose.exe", "0500aacb6c176d47ac0f8158f055db83", 37443620),
	GAME_ENTRY_EN("winterrose", "Winter Rose.exe", "0500aacb6c176d47ac0f8158f055db83", 37444174),  //v1.1
	GAME_ENTRY_EN("winterrose", "Winter Rose.exe", "0500aacb6c176d47ac0f8158f055db83", 37443633),  //v1.2
	GAME_ENTRY_EN("winterrose", "Winter Rose.exe", "0500aacb6c176d47ac0f8158f055db83", 37444693),  //v1.3
	GAME_ENTRY_EN("wisp", "Wisp.exe", "9cb3c8dc7a8ab9c44815955696be2677", 2090032),
	GAME_ENTRY("witchcuisine", "Witchcraft Cuisine.exe", "b781cc4ab0fbe647615fafa4830ec308", 29708682),  // Win Eng-Ita
	GAME_ENTRY("witchcuisine", "Witchcraft Cuisine.exe", "b781cc4ab0fbe647615fafa4830ec308", 29706965),  // Win Eng-Ita
	GAME_ENTRY("witchcuisine", "Witchcraft Cuisine.ags", "e21087d20f4671e9082ba9d0063e450a", 26541942),  // Linux
	GAME_ENTRY("witchcuisine", "games.ags", "90fe5cdf64cc46965c2e3d5c53af193a", 46076827),  // Mac
	GAME_ENTRY_EN("witchlullaby", "The Witch's Lullaby.exe", "ca511538e3e3244813db0f905a31810a", 70312050),  // v1.1
	GAME_ENTRY_EN("witchlullaby", "The Witch's Lullaby.ags", "3e89d1cade6b23ae330390a2924882f0", 59534130),  // v1.2
	GAME_ENTRY_EN("witchnight", "wnight.exe", "9cf51833e787cc919837d9a8bd8fc14c", 2792150),
	GAME_ENTRY_EN("witchron", "Witch!.exe", "b52d56422af9cb50fd5cf369af69388f", 1838433),
	GAME_ENTRY_EN("witchwayout", "Witch Way Out.exe", "7db052bc30700d1f30f5330f5814f519", 11725682),  // Win
	GAME_ENTRY_EN("witchwayout", "Witch Way Out.exe", "7db052bc30700d1f30f5330f5814f519", 11725858),  // Win 1.1
	GAME_ENTRY_EN("witchwayout", "Witch Way Out.ags", "6b858df8a7b9e2af69a0b2b0b40e70cd", 8690526),  // Linux
	GAME_ENTRY_EN("witchwayout", "Witch Way Out.ags", "b21ab145870168d0e0c5ad30b5bc8b83", 8690702),  // Linux 1.1
	GAME_ENTRY_EN("witchwizardcup", "Witch.exe", "f120690b506dd63cd7d1112ea6af2f77", 2693486),
	GAME_ENTRY_EN("witchywoo", "Witchy-Woo.exe", "615e73fc1874e92d60a1996c2330ea36", 153249144),
	GAME_ENTRY_EN("witness", "Witness!.exe", "0710e2ec71042617f565c01824f0cf3c", 803884),
	GAME_ENTRY_EN("wizardhangover", "hangover.exe", "72a1e963da14255d2b7523133f7147d9", 1645263),
	GAME_ENTRY_EN("wizardhangover", "hangover.exe", "72a1e963da14255d2b7523133f7147d9", 1837194),  // final
	GAME_ENTRY_EN("wizardhood", "wiz.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3693530),
	GAME_ENTRY_EN_PLATFORM("wizardhood", "wiz.exe", "f3a13b2d6c2e0fe04c6f466062920e23", 3690587, "MAGS"),
	GAME_ENTRY_EN("woo", "Woo.exe", "06a03fe35791b0578068ab1873455463", 4770481),  //v1.2.3
	GAME_ENTRY_EN("woo", "Woo.exe", "06a03fe35791b0578068ab1873455463", 4771204),  //v1.2.4
	GAME_ENTRY_EN("woof", "woof.exe", "0710e2ec71042617f565c01824f0cf3c", 777374),
	GAME_ENTRY_EN("woolyrockbottom", "Wooly.exe", "261d108f9213356a351b35b54867f342", 3409152),
	GAME_ENTRY_EN("worldisweird", "worldweird.exe", "22fd810f7844c6366df3024b65a3411f", 341060078),
	GAME_ENTRY_EN("worldoftoto", "WorldOfToto.ags", "03bf57f3cf0974e210dacd00ec7ba574", 135938736),
	GAME_ENTRY_EN("worldoftoto", "WorldOfToto.exe", "2f2bc0b9c539d20529c0e343315d5f65", 139047620),
	GAME_ENTRY("worldofwarcraftquest", "IMBA.exe", "3a96a134156aeccee37daae9a7d5232d", 31743641),  //En-De
	GAME_ENTRY_EN("worldscollide", "Worlds Literally Collide.ags", "e78364c4e480cb710ab21e3bc629c882", 9606492),
	GAME_ENTRY_EN("worldscollide", "Worlds Literally Collide.ags", "0596351d05e977240b34bea9b5b80177", 9606416),
	GAME_ENTRY_LANG("wormholech1", "WH1.exe", "0564de07d3fd5c16e6947a647061913c", 294320191, Common::ES_ESP),
	GAME_ENTRY_EN("wrathofthesolonoids", "verb.exe", "0500aacb6c176d47ac0f8158f055db83", 3582078),
	GAME_ENTRY_EN("wrecked", "AGS_Wrecked.exe", "310def5b0f0e3b1ec1e1f0198011c17c", 171806325),
	GAME_ENTRY_EN("wrongchannel", "wrongchannel.exe", "09e8e7ecf1f748e57cda4b22661a8be7", 16509778),
	GAME_ENTRY_EN("wronggame", "Thewronggame.exe", "7fa22c52586671a68bfa4cfe855a3012", 2082833),
	GAME_ENTRY_LANG("wulffenstein", "wulffenstein.exe", "615e73fc1874e92d60a1996c2330ea36", 2575514, Common::DE_DEU),
	GAME_ENTRY_EN("y", "LD49.ags", "e970fc2371524fefd8b52f0906b96f12", 12219717),  // Linux
	GAME_ENTRY_EN("y", "LD49.exe", "776a62db4387dc68be92ef9933399fd5", 15326041),  // Windows
	GAME_ENTRY("yago", "Yago.exe", "7c6e063343fc2ec2bfffc93a1bbd6cfe", 864427955),  //Eng-Esp Win
	GAME_ENTRY("yago", "Yago.ags", "1f5c1900e2e1406a0beba1e7d04520c2", 861940131),  //Eng-Esp Linux
	GAME_ENTRY_EN("yipquest", "Yip Quest.ags", "8c940f9a41296602e8fce1f6698f64ea", 16478722),
	GAME_ENTRY_EN("yipquest", "YQuest.ags", "88cfc1f39d3980cdf16b2f10b6c3cb3f", 16494301),  //v1.1 Win/Linux
	GAME_ENTRY_EN("yipquest", "game.ags", "88cfc1f39d3980cdf16b2f10b6c3cb3f", 16494301),  //v1.1 Mac
	GAME_ENTRY_EN("yoda", "Yoda.exe", "a01a9639ce30bdcd5bf82e528b51fa06", 2461339),
	GAME_ENTRY_EN("yourgrace", "YOURgrace.exe", "0241777c2537fc5d077c05cde10bfa9f", 2986272),
	GAME_ENTRY_EN("yourlate", "Your late.exe", "02635a77ab660023f59519c91329f7f5", 2719997),
	GAME_ENTRY_EN("zakrepixeled", "ZAK.exe", "06885f43ad98fa6a50772b664ad2a81b", 5090444), // v0.1.4
	GAME_ENTRY_EN("zakrepixeled", "ZAK.exe", "305a819bce4004ef3c9b1a3a9f6ac1c8", 5741632), // v0.2.2.2
	GAME_ENTRY_EN("zakrepixeled", "ZAK.exe", "7bf50af77154660465704b80cec1476d", 5816900), // v0.2.3.3
	GAME_ENTRY_EN("zakthemaniac", "maniaczak.exe", "dee872e31c6d205e68aa9d87a542f07b", 24492152),
	GAME_ENTRY_EN("zedquest", "NewZud.exe", "0500aacb6c176d47ac0f8158f055db83", 1367132),
	GAME_ENTRY_EN("zeke", "ZTO.exe", "f120690b506dd63cd7d1112ea6af2f77", 1201019),
	GAME_ENTRY_EN("zempquest", "HBDZemp.30", "06a03fe35791b0578068ab1873455463", 2028630),
	GAME_ENTRY_EN("ziggyshorts", "ziggyhalemilkgame.exe", "b5d827e7fa1376f25ab30d9b700f99b4", 17145928),
	GAME_ENTRY_EN("zippermeteor", "TheZipperMeteor.exe", "06a03fe35791b0578068ab1873455463", 11886514),
	GAME_ENTRY_EN("zogmoonbuckle", "Zog_Moonbuckle.exe", "0a3d57052500c7e93510ca2c317b6991", 37433537),
	GAME_ENTRY_EN("zombiefish", "FZombie.exe", "3128b9f90e2f954ba704414ae854d10b", 4220305),
	GAME_ENTRY_EN("zombienation", "zomnat v1.1.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 46111952),
	GAME_ENTRY_EN("zombienation", "zomnat v1.1.exe", "e88fd6a23a5e498d7b0d50e3bb914085", 46065287),
	GAME_ENTRY_EN("zombietown", "phychocar1.exe", "0500aacb6c176d47ac0f8158f055db83", 818731),
	GAME_ENTRY_EN("zombietrainep2", "datacompletegame.exe", "7a3096ac0237cb6aa8e1718e28caf039", 28086203),
	GAME_ENTRY_EN("zooreal", "zoo_real.exe", "3b7cceb3e4bdb031dc5d8f290936e94b", 24184795),
	GAME_ENTRY_EN_PLATFORM("zugzwang", "Zugzwang.exe", "28f82e420b82d07651b68114f90223c8", 13758471, "MAGS"),  //1.0
	GAME_ENTRY_EN("zugzwang", "Zugzwang.exe", "28f82e420b82d07651b68114f90223c8", 13870211),  //1.01
	GAME_ENTRY_EN("zugzwang", "Zugzwang.exe", "28f82e420b82d07651b68114f90223c8", 13878812),  //1.03
	GAME_ENTRY_EN("zugzwang", "Zugzwang.exe", "28f82e420b82d07651b68114f90223c8", 13879416),  //1.04
	GAME_ENTRY_EN("zugzwang", "Zugzwang.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 17209702),  //2.0
	GAME_ENTRY_EN("zugzwang", "Zugzwang.exe", "6cddccb3744ec5c6af7c398fb7b3b11c", 17315873),  //final

	//RuCOMM games and demos
	DEMO_ENTRY_LANG("snowyadventures", "snowyadventures.exe", "27343924ddad3be0b97bdcaa71858b1b", 86439776, Common::RU_RUS),
	DEMO_ENTRY_LANG("tbdiy", "tbdiy.exe", "27343924ddad3be0b97bdcaa71858b1b", 4020251, Common::RU_RUS),

	GAME_ENTRY_LANG("einsteinmachine", "em.exe", "bb59de174d70797d774dec76a171352d", 76169675, Common::RU_RUS),
	GAME_ENTRY_LANG("evildead4", "evil dead.exe", "465f972675db2da6040518221af5b0ba", 2141432,Common::RU_RUS),
	GAME_ENTRY_LANG("goodman", "goodman.exe", "465f972675db2da6040518221af5b0ba", 2699578,Common::RU_RUS),
	GAME_ENTRY_LANG("prisonersofice", "newyearq.exe", "b26aa198e5175000f037b84d8a4038f5", 84723231, Common::RU_RUS),
	GAME_ENTRY_LANG("prisonersofice", "newyearq.exe", "b26aa198e5175000f037b84d8a4038f5", 88100040, Common::RU_RUS),


	{ AD_TABLE_END_MARKER, 0 }
};

/**
 * The fallback game descriptor used by the fallback detection code
 */
static AGSGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOLANG)
	},
	0
};

} // namespace AGS
