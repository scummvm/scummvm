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

#include "bagel/hodjnpodj/wordsearch/wordlist.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

const char *astrWLCat[NUMBEROFLISTS] = {
	"Parts of an Airplane",     //0
	"Animals",
	"Baseball Terms",
	"Books of the Bible",
	"Birds",
	"Body Parts",               //5
	"Breads",
	"Desserts",
	"Parts of a Castle",
	"Religious Leaders",
	"Clothing",                 //10
	"Units of Currency",
	"Computer Terms",
	"Diseases and Disorders",
	"Drinks",
	"Elements",                 //15
	"Fabrics",
	"Movie-Making Terms",
	"Fruit",
	"Furniture",
	"Gems",                     //20
	"Herbs and Spices",
	"Legal Terms",              //22
	"Mathematical Terms",
	"Military Ranks",
	"Musical Instruments",      //25
	"Pastas",
	"Parts of a House",
	"Sailing Terms",
	"Shapes",
	"Shells",
	"Sports and Games",
	"Tools",
	"Vehicles",
	"Units of Measure",
	"Cities of the U.S.",
	"World Capitals",
	"U.S. Presidents",
	"Trees",
	"Flowers",
	"Things on a Dinner Table",
	"Parts of a Car",
	"Cheeses",
	"Rivers of the World",
	"Sports Teams",
	"Movie Titles",
	"Rock Groups",              //46
	"Heavenly Bodies",
	"Astronauts & Cosmonauts",
	"Artists"
};

char    acList[NUMBEROFLISTS][WORDSPERLIST][20] = {
	{
		// 0
		{ "EMERGENCY EXIT" },  //5
		{ "LANDING GEAR" },    //10
		{ "LUGGAGE RACK" },    //12
		{ "MOVIE SCREEN" },    //13
		{ "OXYGEN MASK" },     //14
		{ "TRAY TABLES" },    //22
		{ "FLIGHT DECK" },     //7
		{ "AILERON" },         //0
		{ "ALTIMETER" },       //1
		{ "CABIN" },           //2
		{ "COCKPIT" },         //3
		{ "COMPASS" },         //4
		{ "ENGINE" },          //6
		{ "FUEL TANK" },       //8
		{ "FUSELAGE" },        //9
		{ "LAVATORY" },        //11
		{ "PROPELLOR" },       //15
		{ "RADAR DOME" },      //16
		{ "RADIO" },          //17
		{ "RUDDER" },         //18
		{ "SEAT BELT" },      //19
		{ "STABILIZER" },     //20
		{ "TAIL FIN" },       //21
		{ "TURBOJET" },       //23
		{ "WING" }            //24
	},
	{
		// 1
		{ "BADGER" },       //0
		{ "BEAR" },         //
		{ "COYOTE" },       //
		{ "DEER" },         //
		{ "DONKEY" },       //
		{ "ELEPHANT" },     //
		{ "FOX" },          //
		{ "FROG" },         //
		{ "GIRAFFE" },      //
		{ "HORSE" },        //
		{ "KANGAROO" },         //
		{ "LEOPARD" },      //
		{ "LION" },     //
		{ "MONKEY" },       //
		{ "OTTER" },        //
		{ "RABBIT" },       //
		{ "RHINOCEROS" },       //
		{ "SEAL" },         //
		{ "SHEEP" },        //
		{ "SNAKE" },        //
		{ "SQUIRREL" },         //
		{ "TIGER" },        //
		{ "WALRUS" },       //
		{ "WOLF" },         //
		{ "ZEBRA" }     //
	},
	{
		// 2
		{ "BALK" },         //0
		{ "BATTER" },       //
		{ "BOX SCORE" },        //
		{ "BULLPEN" },      //
		{ "BUNT" },         //
		{ "CATCHER" },      //
		{ "DIAMOND" },      //
		{ "double" },       //
		{ "DUGOUT" },       //
		{ "FOUL" },         //
		{ "GLOVE" },        //
		{ "HOME PLATE" },       //
		{ "INFIELD" },      //
		{ "INNING" },       //
		{ "OUTFIELD" },         //
		{ "PITCHER" },      //
		{ "PITCHOUT" },         //
		{ "SHORTSTOP" },        //
		{ "SINGLE" },       //
		{ "SPITBALL" },         //
		{ "STOLEN BASE" },      //
		{ "STRIKE" },       //
		{ "TRIPLE" },       //
		{ "WALK" },     //
		{ "WARNING TRACK" }     //
	},
	{
		//3
		{ "AMOS" },         //0
		{ "DANIEL" },       //
		{ "ESTHER" },               //
		{ "EXODUS" },                       //
		{ "EZRA" },         //
		{ "GENESIS" },      //
		{ "HOSEA" },        //
		{ "ISAIAH" },       //
		{ "JEREMIAH" },         //
		{ "JOB" },      //
		{ "JOEL" },         //
		{ "JONAH" },        //
		{ "JOSHUA" },       //
		{ "JUDGES" },       //
		{ "MICAH" },        //
		{ "NEHEMIAH" },         //
		{ "NUMBERS" },      //
		{ "OBADIAH" },      //
		{ "PROVERBS" },         //
		{ "RUTH" },         //
		{ "PSALMS" }        //
	},
	{
		//4
		{ "CRANE" },         //0
		{ "CROW" },         //
		{ "DUCK" },         //
		{ "EAGLE" },        //
		{ "FINCH" },        //
		{ "GOOSE" },        //
		{ "GULL" },         //      //
		{ "HAWK" },         //
		{ "HERON" },        //
		{ "LARK" },         //      //
		{ "MAGPIE" },       //
		{ "OWL" },      //
		{ "PIGEON" },       //
		{ "QUAIL" },        //
		{ "RAVEN" },        //
		{ "SWAN" },     //
		{ "TURKEY" }        //
	},
	{
		//5
		{ "ANKLE" },         //0
		{ "BRAIN" },        //
		{ "CHEEK" },        //
		{ "ELBOW" },        //
		{ "ESOPHAGUS" },        //
		{ "EYE" },      //
		{ "FINGER" },       //
		{ "HEART" },        //
		{ "HEEL" },         //
		{ "HIP" },      //
		{ "JAW" },      //
		{ "INTESTINE" },        //
		{ "KNEE" },         //
		{ "LIVER" },        //
		{ "LUNG" },     //
		{ "MOUTH" },        //      //
		{ "NECK" },         //
		{ "PANCREAS" },         //
		{ "SKULL" },        //
		{ "SPINE" },        //
		{ "SPLEEN" },       //
		{ "STOMACH" },      //
		{ "THIGH" },        //
		{ "THROAT" },       //
		{ "TOE" }       //
	},
	{
		//6
		{ "BAGEL" },         //0
		{ "BAGUETTE" },         //
		{ "BISCUIT" },      //
		{ "BRIOCHE" },      //
		{ "CRACKER" },      //
		{ "CROISSANT" },        //
		{ "CROUTON" },      //
		{ "CRULLER" },      //
		{ "DANISH" },       //
		{ "FOCACCIA" },         //
		{ "FRUITCAKE" },        //
		{ "MATZO" },        //
		{ "MUFFIN" },       //
		{ "PITA" },         //
		{ "POPOVER" },      //
		{ "PUMPERNICKEL" },         //
		{ "ROLL" },         //
		{ "RYE" },      //
		{ "SCONE" },        //
		{ "SOURDOUGH" },        //
		{ "STOLEN" },       //
		{ "TOAST" },        //
		{ "WAFER" },        //
		{ "WHOLE WHEAT" },      //
		{ "ZWIEBACK" }      //
	},
	{
		//7
		{ "BAKED ALASKA" },         //0
		{ "BAKLAVA" },              //
		{ "BROWNIE" },      //
		{ "CAKE" },     //
		{ "CANNOLI" },      //
		{ "COMPOTE" },      //
		{ "COOKIE" },       //
		{ "CREPE SUZETTE" },        //
		{ "DOUGHNUT" },     //
		{ "FRAPPE" },       //
		{ "FUDGE" },        //
		{ "HALVAH" },       //
		{ "ICE CREAM" },        //
		{ "JELLO" },        //
		{ "MACAROON" },     //      //
		{ "MOUSSE" },       //
		{ "PARFAIT" },      //
		{ "PIE" },      //
		{ "PROFITEROLE" },      //
		{ "PUDDING" },      //
		{ "SORBET" },       //
		{ "STRUDEL" },      //
		{ "SUNDAE" },       //
		{ "TORTE" },        //
		{ "ZABAGLIONE" },       //
	},
	{
		//8
		{ "BAILEY" },         //0
		{ "BARBICAN" },     //
		{ "BASTION" },      //
		{ "BATTLEMENT" },       //
		{ "CISTERN" },      //
		{ "CORBEL" },       //
		{ "CRENELLE" },     //
		{ "CURTAIN WALL" },     //
		{ "DRAWBRIDGE" },       //
		{ "DUNGEON" },      //
		{ "ESCARPMENT" },       //
		{ "GATEWAY" },      //
		{ "GREAT HALL" },       //
		{ "HORNWORK" },     //
		{ "KEEP" },     //
		{ "MACHICOLATION" },        //
		{ "MERLON" },       //
		{ "MOAT" },     //
		{ "PARAPET" },      //
		{ "PORTCULLIS" },       //
		{ "RAMPART" },      //
		{ "THRONE ROOM" },      //
		{ "TURRET" },       //
		{ "WARD" },     //
		{ "WATCHTOWER" },       //
	},
	{
		//9
		{ "AYATOLLAH" },         //0
		{ "BISHOP" },       //
		{ "CARDINAL" },     //
		{ "CHANCELLOR" },       //
		{ "CHAPLAIN" },     //
		{ "CLERIC" },       //
		{ "CLERGYMAN" },        //
		{ "CURATE" },       //
		{ "DALAI LAMA" },       //
		{ "DEACON" },       //
		{ "IMAM" },     //
		{ "MINISTER" },     //
		{ "MONSIGNOR" },        //
		{ "MUFTI" },        //      //
		{ "MULLAH" },       //
		{ "PADRE" },        //
		{ "POPE" },     //
		{ "PREACHER" },     //
		{ "PRELATE" },      //
		{ "PRIEST" },       //
		{ "PRIOR" },        //
		{ "PROCTOR" },      //
		{ "PROVOST" },      //
		{ "RABBI" },        //
		{ "SHAMAN" },       //
	},
	{
		//10
		{ "BANDANNA" },         //0
		{ "BOLERO" },       //
		{ "CAFTAN" },       //
		{ "CAMISOLE" },     //
		{ "CHEMISE" },      //
		{ "COAT" },     //      //
		{ "DRESS" },        //
		{ "DUNGAREES" },        //
		{ "GLOVES" },       //
		{ "HALTER" },       //
		{ "JACKET" },       //
		{ "JODHPURS" },     //
		{ "KIMONO" },       //
		{ "LEDERHOSEN" },       //
		{ "LEOTARD" },      //
		{ "MUUMUU" },       //
		{ "NEGLIGEE" },     //
		{ "OVERALLS" },     //
		{ "PANTS" },        //
		{ "PARKA" },        //
		{ "SARONG" },       //
		{ "SHIRT" },        //
		{ "SKIRT" },        //
		{ "STOCKING" },     //
		{ "SUIT" },     //
	},
	{
		//11
		{ "AUSTRAL" },         //0
		{ "BOLIVAR" },
		{ "COLON" },
		{ "CRUZEIRO" },
		{ "DINAR" },
		{ "DIRHAM" },
		{ "DOLLAR" },
		{ "DRACHMA" },
		{ "FRANC" },
		{ "GUILDER" },
		{ "KRONE" },
		{ "KWACHA" },
		{ "LIRA" },
		{ "MARK" },
		{ "PESO" },
		{ "POUND" },
		{ "QUETZAL" },
		{ "RAND" },
		{ "RIAL" },
		{ "RUBLE" },
		{ "RUPEE" },
		{ "SHEKEL" },
		{ "SHILLING" },
		{ "YEN" },
		{ "ZLOTY" },
	},
	{
		//12
		{ "SURGE PROTECTOR" },  //24
		{ "WORK STATION" }, //25
		{ "FLOPPY DISK" },  //7
		{ "PROCESSOR" },    //21
		{ "HARDWARE" }, //11
		{ "INTERFACE" },    //12
		{ "HARD DISK" },    //10
		{ "KEYBOARD" }, //13
		{ "BACKUP" },         //1
		{ "BINARY" },   //2
		{ "CURSOR" },   //5
		{ "FORMAT" },   //8
		{ "HACKER" },   //9
		{ "LAPTOP" },   //14
		{ "MEMORY" },   //15
		{ "PRINTER" },  //20
		{ "PROGRAM" },  //22
		{ "SOFTWARE" }, //23
		{ "MONITOR" },  //17
		{ "MODEM" },    //16
		{ "MOUSE" },    //18
		{ "PIXEL" },    //19
		{ "CRASH" },    //6
		{ "byte" }, //3
		{ "BUG" },  //4
	},
	{
		//13
		{ "ANEMIA" },         //0
		{ "ANGINA" },
		{ "ANTHRAX" },
		{ "BERIBERI" },
		{ "BURSITIS" },
		{ "CANCER" },
		{ "GLAUCOMA" },
		{ "GINGIVITIS" },
		{ "HEPATITIS" },
		{ "HERPES" },
		{ "HYPERTENSION" },
		{ "IMPETIGO" },
		{ "LEUKEMIA" },
		{ "MEASLES" },
		{ "MUMPS" },
		{ "PHLEBITIS" },
		{ "PSORIASIS" },
		{ "RABIES" },
		{ "RICKETS" },
		{ "RUBELLA" },
		{ "SCURVY" },
		{ "SMALLPOX" },
		{ "TETANUS" },
		{ "TOXEMIA" },
		{ "TYPHOID" },
	},
	{
		//14
		{ "ALE" },         //0
		{ "BEER" },
		{ "CHAMPAGNE" },
		{ "COFFEE" },
		{ "CREAM" },
		{ "DAIQUIRI" },
		{ "EGG NOG" },
		{ "FRAPPE" },
		{ "GIMLET" },
		{ "HOT CHOCOLATE" },
		{ "JUICE" },
		{ "JULEP" },
		{ "MARTINI" },
		{ "MILK" },
		{ "NECTAR" },
		{ "PUNCH" },
		{ "RICKEY" },
		{ "SAKE" },
		{ "SANGRIA" },
		{ "SARSAPARILLA" },
		{ "SCHNAPPS" },
		{ "SELTZER" },
		{ "TEA" },
		{ "WATER" },
		{ "WINE" },
	},
	{
		//15
		{ "ARSENIC" },         //0
		{ "BORON" },
		{ "CALCIUM" },
		{ "CARBON" },
		{ "COBALT" },
		{ "COPPER" },
		{ "FLOURINE" },
		{ "GOLD" },
		{ "HELIUM" },
		{ "HYDROGEN" },
		{ "IRON" },
		{ "KRYPTON" },
		{ "NEON" },
		{ "NICKEL" },
		{ "NITROGEN" },
		{ "OXYGEN" },
		{ "PHOSPHORUS" },
		{ "PLUTONIUM" },
		{ "RADIUM" },
		{ "SILICON" },
		{ "SILVER" },
		{ "SODIUM" },
		{ "SULFUR" },
		{ "URANIUM" },
		{ "ZINC" },
	},
	{
		//16
		{ "ANGORA" },
		{ "BURLAP" },
		{ "CALICO" },
		{ "CASHMERE" },
		{ "CHIFFON" },
		{ "CHINTZ" },
		{ "CORDUROY" },
		{ "COTTON" },
		{ "DENIM" },
		{ "FLANNEL" },
		{ "GINGHAM" },
		{ "MOHAIR" },
		{ "NYLON" },
		{ "POLYESTER" },
		{ "RAYON" },
		{ "SATIN" },
		{ "SEERSUCKER" },
		{ "SILK" },
		{ "SPANDEX" },
		{ "TAFFETA" },
		{ "TERRYCLOTH" },
		{ "TWEED" },
		{ "VELOUR" },
		{ "VELVET" },
		{ "WOOL" },
	},
	{
		//17 Movie Terms
		{ "SPECIAL EFFECT" },   //20
		{ "FREEZE FRAME" }, //10
		{ "KLIEG LIGHT" },  //13
		{ "ANIMATION" },    //1
		{ "TIME LAPSE" },   //22
		{ "COMMISSARY" },   //3
		{ "CONTINUITY" },   //4
		{ "SCREENPLAY" },   //18
		{ "SOUNDTRACK" },   //19
		{ "TREATMENT" },    //23
		{ "VOICEOVER" },    //24
		{ "DIRECTOR" },     //6
		{ "DISSOLVE" },     //7
		{ "KEY GRIP" },     //12
		{ "FADEOUT" },      //9
		{ "LOCATION" },     //14
		{ "PRODUCER" },     //17
		{ "STAND IN" },     //21
		{ "CREDITS" },      //5
		{ "MONTAGE" },      //15
		{ "OUTTAKE" },      //16
		{ "GAFFER" },       //11
		{ "CAMEO" },        //2
		{ "ZOOM" },     //25
		{ "DUB" },      //8
	},
	{
		//18
		{ "APPLE" },
		{ "APRICOT" },
		{ "BANANA" },
		{ "BLUEBERRY" },
		{ "CANTALOUPE" },
		{ "CHERRY" },
		{ "CRANBERRY" },
		{ "GRAPE" },
		{ "GUAVA" },
		{ "KIWI" },
		{ "KUMQUAT" },
		{ "LEMON" },
		{ "LIME" },
		{ "NECTARINE" },
		{ "ORANGE" },
		{ "PAPAYA" },
		{ "PEACH" },
		{ "PEAR" },
		{ "PERSIMMON" },
		{ "PLUM" },
		{ "POMEGRANATE" },
		{ "RASPBERRY" },
		{ "STRAWBERRY" },
		{ "TAMARIND" },
		{ "TANGERINE" },
	},
	{
		//19
		{ "ARMOIRE" },
		{ "BEANBAG" },
		{ "BED" },
		{ "BOOKSHELF" },
		{ "BUREAU" },
		{ "CHAISE LOUNGE" },
		{ "CABINET" },
		{ "CHAIR" },
		{ "CHEST" },
		{ "COMMODE" },
		{ "COUCH" },
		{ "CREDENZA" },
		{ "DAVENPORT" },
		{ "DESK" },
		{ "DIVAN" },
		{ "DRESSER" },
		{ "FUTON" },
		{ "HUTCH" },
		{ "OTTOMAN" },
		{ "RECLINER" },
		{ "SECRETARY" },
		{ "SOFA" },
		{ "STOOL" },
		{ "TABLE" },
		{ "TALLBOY" },
	},
	{
		// 20 Gems
		{ "ALEXANDRITE" },  //2
		{ "LAPIS LAZULI" }, //13
		{ "TOURMALINE" },   //23
		{ "AQUAMARINE" },   //5
		{ "BLOODSTONE" },   //6
		{ "TURQUOISE" },    //24
		{ "MOONSTONE" },    //14
		{ "AMETHYST" },     //4
		{ "PERIDOT" },      //18
		{ "SAPPHIRE" },     //20
		{ "TIGEREYE" },     //21
		{ "CITRINE" },      //7
		{ "DIAMOND" },      //8
		{ "EMERALD" },      //9
		{ "JASPER" },       //12
		{ "GARNET" },       //10
		{ "ZIRCON" },       //25
		{ "AGATE" },        //1
		{ "TOPAZ" },        //22
		{ "AMBER" },        //3
		{ "PEARL" },        //17
		{ "JADE" },     //11
		{ "ONYX" },     //15
		{ "OPAL" },     //16
		{ "RUBY" },     //19
	},
	{
		//21
		{ "ANISEED" },
		{ "BASIL" },
		{ "CARAWAY" },
		{ "CARDAMOM" },
		{ "CAYENNE" },
		{ "CHIVES" },
		{ "CILANTRO" },
		{ "CINNAMON" },
		{ "CLOVES" },
		{ "CUMIN" },
		{ "DILL" },
		{ "FENNEL" },
		{ "GARLIC" },
		{ "GINGER" },
		{ "MARJORAM" },
		{ "MINT" },
		{ "NUTMEG" },
		{ "OREGANO" },
		{ "PAPRIKA" },
		{ "PARSLEY" },
		{ "ROSEMARY" },
		{ "SAFFRON" },
		{ "SAGE" },
		{ "TARRAGON" },
		{ "THYME" },
	},
	{
		// 22 legal terms
		{ "CORPUS DELICTI" },       //4
		{ "HABEAS CORPUS" },        //8
		{ "JURISPRUDENCE" },        //12
		{ "RECOGNIZANCE" },     //18
		{ "DEPOSITION" },       //5
		{ "INJUNCTION" },       //11
		{ "LITIGATION" },       //14
		{ "INDEMNIFY" },        //9
		{ "SEQUESTER" },        //19
		{ "AFFIDAVIT" },        //0
		{ "CONTRACT" },     //3
		{ "EASEMENT" },     //6
		{ "SUBPOENA" },     //20
		{ "CODICIL" },      //2
		{ "PROBATE" },      //17
		{ "WITNESS" },      //24
		{ "ESCROW" },       //7
		{ "INDICT" },       //10
		{ "MALICE" },       //15
		{ "PAROLE" },       //16
		{ "BRIEF" },        //1
		{ "TRIAL" },        //22
		{ "LIEN" },     //13
		{ "TORT" },     //21
		{ "WILL" },     //23
	},
	{
		//23
		{ "PERMUTATION" },  //20
		{ "PROGRESSION" },  //21
		{ "COEFFICIENT" },  //4
		{ "DENOMINATOR" },  //9
		{ "COORDINATE" },   //7
		{ "LOGARITHM" },    //18
		{ "ALGORITHM" },    //1
		{ "CONGRUENT" },    //5
		{ "CALCULUS" },     //3
		{ "CONSTANT" },     //6
		{ "FRACTION" },     //14
		{ "FUNCTION" },     //15
		{ "DIVIDEND" },     //10
		{ "EQUATION" },     //11
		{ "EXPONENT" },     //12
		{ "VARIABLE" },     //24
		{ "INTEGER" },      //16
		{ "TANGENT" },      //23
		{ "DECIMAL" },      //8
		{ "BINARY" },       //2
		{ "FACTOR" },       //13
		{ "NUMBER" },       //19
		{ "SECANT" },       //22
		{ "VERTEX" },       //25
		{ "LOCUS" },        //17
	},
	{
		//24
		{ "WARRANT OFFICER" },  //25
		{ "JUDGE-ADVOCATE" },   //14
		{ "PETTY OFFICER" },    //19
		{ "QUARTERMASTER" },    //21
		{ "AIDE-DE-CAMP" }, //3
		{ "LIEUTENANT" },   //15
		{ "COMMANDER" },    //9
		{ "COMMODORE" },    //10
		{ "BRIGADIER" },    //5
		{ "CENTURION" },    //7
		{ "ADJUTANT" },     //1
		{ "MARSHALL" },     //17
		{ "SERGEANT" },     //24
		{ "CORPORAL" },     //11
		{ "ADMIRAL" },      //2
		{ "AIRMAN" },       //4
		{ "CAPTAIN" },      //6
		{ "COLONEL" },      //8
		{ "ENSIGN" },       //12
		{ "GENERAL" },      //13
		{ "ORDERLY" },      //18
		{ "PRIVATE" },      //20
		{ "RECRUIT" },      //22
		{ "SEAMAN" },       //23
		{ "MAJOR" },        //16
	},
	{
		//25
		{ "BASS" },
		{ "CELLO" },
		{ "CLARINET" },
		{ "CORNET" },
		{ "CYMBALS" },
		{ "DRUM" },
		{ "FIFE" },
		{ "FLUTE" },
		{ "GONG" },
		{ "GUITAR" },
		{ "HARMONICA" },
		{ "HARP" },
		{ "HORN" },
		{ "OBOE" },
		{ "OCARINA" },
		{ "PIANO" },
		{ "PICCOLO" },
		{ "TIMPANI" },
		{ "TRIANGLE" },
		{ "TROMBONE" },
		{ "TRUMPET" },
		{ "TUBA" },
		{ "VIOLA" },
		{ "VIOLIN" },
		{ "XYLOPHONE" },
	},
	{
		//26
		{ "CANNELLONI" },
		{ "CAPELLINI" },
		{ "FETTUCINI" },
		{ "FUSILLI" },
		{ "GNOCCHI" },
		{ "KREPLACH" },
		{ "LASAGNA" },
		{ "LINGUINE" },
		{ "LO MEIN" },
		{ "MACARONI" },
		{ "MANICOTTI" },
		{ "NOODLES" },
		{ "PAPPARDELLE" },
		{ "PENNE" },
		{ "RAMEN" },
		{ "RAVIOLI" },
		{ "RIGATONI" },
		{ "ROTELLE" },
		{ "ROTINI" },
		{ "SEMOLINA" },
		{ "SHELLS" },
		{ "SPAGHETTI" },
		{ "TORTELLINI" },
		{ "VERMICELLI" },
		{ "ZITI" },
	},
	{
		//27
		{ "ATTIC" },
		{ "BASEMENT" },
		{ "BATHROOM" },
		{ "BEDROOM" },
		{ "CHIMNEY" },
		{ "CLOSET" },
		{ "CRAWL SPACE" },
		{ "DECK" },
		{ "DEN" },
		{ "DINING ROOM" },
		{ "DOOR" },
		{ "DORMER" },
		{ "EAVE" },
		{ "FIREPLACE" },
		{ "FURNACE" },
		{ "GARAGE" },
		{ "GABLE" },
		{ "GUTTER" },
		{ "KITCHEN" },
		{ "PORCH" },
		{ "ROOF" },
		{ "SKYLIGHT" },
		{ "STAIRS" },
		{ "WALL" },
		{ "WINDOW" },
	},
	{
		//28
		{ "BOOM" },
		{ "BROACH" },
		{ "CENTERBOARD" },
		{ "CRABBING" },
		{ "DAVITS" },
		{ "FORECASTLE" },
		{ "FORESTAY" },
		{ "GUNWALE" },
		{ "HALYARD" },
		{ "JIB" },
		{ "LATEEN" },
		{ "LUFFING" },
		{ "LUGSAIL" },
		{ "MAINSAIL" },
		{ "MAINSHEET" },
		{ "MAST" },
		{ "RUDDER" },
		{ "SHROUD" },
		{ "SPAR" },
		{ "SPINNAKER" },
		{ "TACK" },
		{ "TILLER" },
		{ "TRIM" },
		{ "WARP" },
		{ "YAW" },
	},
	{
		//29
		{ "ARC" },
		{ "CIRCLE" },
		{ "CLUB" },
		{ "CONE" },
		{ "CRESCENT" },
		{ "CROSS" },
		{ "CUBE" },
		{ "DIAMOND" },
		{ "ELLIPSE" },
		{ "HEART" },
		{ "HEXAGON" },
		{ "OCTAGON" },
		{ "OVAL" },
		{ "PENTAGON" },
		{ "PYRAMID" },
		{ "RECTANGLE" },
		{ "RING" },
		{ "RHOMBUS" },
		{ "SPADE" },
		{ "SPIRAL" },
		{ "SQUARE" },
		{ "STAR" },
		{ "TRAPEZOID" },
		{ "TRIANGLE" },
		{ "WEDGE" },
	},
	{
		//30
		{ "ABALONE" },
		{ "BARNACLE" },
		{ "BIVALVE" },
		{ "CHITON" },
		{ "CLAM" },
		{ "COCKLE" },
		{ "CONCH" },
		{ "COWRIE" },
		{ "COQUINA" },
		{ "CRAB" },
		{ "LIMPET" },
		{ "MUREX" },
		{ "MUSSEL" },
		{ "NAUTILUS" },
		{ "OYSTER" },
		{ "PERIWINKLE" },
		{ "QUAHOG" },
		{ "SAND DOLLAR" },
		{ "SCALLOP" },
		{ "SCAPHOPOD" },
		{ "SNAIL" },
		{ "TRITON" },
		{ "VOLUTE" },
		{ "WENTLETRAP" },
		{ "WHELK" },
	},
	{
		//31
		{ "ARCHERY" },
		{ "BACKGAMMON" },
		{ "BOCCI" },
		{ "BOWLING" },
		{ "BRIDGE" },
		{ "CHARADES" },
		{ "CHECKERS" },
		{ "CHESS" },
		{ "CRICKET" },
		{ "CROQUET" },
		{ "HOCKEY" },
		{ "JACKS" },
		{ "JAI ALAI" },
		{ "KARATE" },
		{ "LACROSSE" },
		{ "MAH JONGG" },
		{ "MONOPOLY" },
		{ "PARCHEESI" },
		{ "PICTIONARY" },
		{ "RUGBY" },
		{ "SCRABBLE" },
		{ "SKIING" },
		{ "SOCCER" },
		{ "TENNIS" },
		{ "WHIST" },
	},
	{
		//32
		{ "ADZ" },
		{ "ANVIL" },
		{ "AUGER" },
		{ "CALIPER" },
		{ "CHISEL" },
		{ "COUNTERSINK" },
		{ "DRILL" },
		{ "GOUGE" },
		{ "HAMMER" },
		{ "HOE" },
		{ "KNIFE" },
		{ "LEVEL" },
		{ "MALLET" },
		{ "MITRE BOX" },
		{ "PITCHFORK" },
		{ "PLANE" },
		{ "PUNCH" },
		{ "RASP" },
		{ "REAMER" },
		{ "ROUTER" },
		{ "SANDER" },
		{ "SAW" },
		{ "SCREWDRIVER" },
		{ "SICKLE" },
		{ "WRENCH" },
	},
	{
		//33
		{ "AIRPLANE" },
		{ "BICYCLE" },
		{ "BOBSLED" },
		{ "WAGON" },
		{ "CAR" },
		{ "DIRIGIBLE" },
		{ "DOGSLED" },
		{ "ELEVATOR" },
		{ "GLIDER" },
		{ "KAYAK" },
		{ "MOPED" },
		{ "MOTORCYCLE" },
		{ "RICKSHAW" },
		{ "ROCKET" },
		{ "SEDAN CHAIR" },
		{ "SNOWMOBILE" },
		{ "SUBMARINE" },
		{ "TANDEM" },
		{ "TOBOGGAN" },
		{ "TRAIN" },
		{ "TROLLEY" },
		{ "TRUCK" },
		{ "UNICYCLE" },
		{ "YACHT" },
		{ "ZEPPELIN" },
	},
	{
		//34
		{ "ACRE" },
		{ "AMPERE" },
		{ "ANGSTROM" },
		{ "BUSHEL" },
		{ "CALORIE" },
		{ "DEGREE" },
		{ "DYNE" },
		{ "ERG" },
		{ "FARAD" },
		{ "FOOT" },
		{ "FURLONG" },
		{ "GRAM" },
		{ "HECTARE" },
		{ "INCH" },
		{ "JOULE" },
		{ "LUMEN" },
		{ "METER" },
		{ "MILE" },
		{ "NEWTON" },
		{ "OHM" },
		{ "PARSEC" },
		{ "ROENTGEN" },
		{ "VOLT" },
		{ "WATT" },
		{ "YARD" },
	},
	{
		//35
		{ "ATLANTA" },
		{ "ALBUQUERQUE" },
		{ "BALTIMORE" },
		{ "BOSTON" },
		{ "BUFFALO" },
		{ "CLEVELAND" },
		{ "CHICAGO" },
		{ "DALLAS" },
		{ "DENVER" },
		{ "DETROIT" },
		{ "DULUTH" },
		{ "HOUSTON" },
		{ "LOUISVILLE" },
		{ "MEMPHIS" },
		{ "MIAMI" },
		{ "MILWAUKEE" },
		{ "MINNEAPOLIS" },
		{ "NEWARK" },
		{ "OAKLAND" },
		{ "PHOENIX" },
		{ "PITTSBURGH" },
		{ "SCRANTON" },
		{ "SEATTLE" },
		{ "TAMPA" },
		{ "WICHITA" },
	},
	{
		//36
		{ "ATHENS" },
		{ "BANGKOK" },
		{ "BERLIN" },
		{ "BRUSSELS" },
		{ "CAIRO" },
		{ "DAMASCUS" },
		{ "DUBLIN" },
		{ "HANOI" },
		{ "HAVANA" },
		{ "HELSINKI" },
		{ "JERUSALEM" },
		{ "KHARTOUM" },
		{ "LISBON" },
		{ "LONDON" },
		{ "MADRID" },
		{ "MANAGUA" },
		{ "MOSCOW" },
		{ "NAIROBI" },
		{ "OSLO" },
		{ "PARIS" },
		{ "RIYADH" },
		{ "ROME" },
		{ "TOKYO" },
		{ "VIENNA" },
		{ "WARSAW" },
	},
	{
		// 37 Presidents
		{ "ROOSEVELT" },    //21
		{ "WASHINGTON" },   //24
		{ "JEFFERSON" },    //11
		{ "CLINTON" },  //4
		{ "COOLIDGE" }, //5
		{ "MCKINLEY" }, //16
		{ "GARFIELD" }, //7
		{ "HARRISON" }, //9
		{ "KENNEDY" },  //13
		{ "LINCOLN" },  //14
		{ "MADISON" },  //15
		{ "JOHNSON" },  //12
		{ "MONROE" },   //17
		{ "CARTER" },   //3
		{ "HOOVER" },   //10
		{ "REAGAN" },   //20
		{ "TRUMAN" },   //23
		{ "WILSON" },   //25
		{ "ADAMS" },    //1
		{ "GRANT" },    //8
		{ "NIXON" },    //18
		{ "BUSH" }, //2
		{ "FORD" }, //6
		{ "POLK" }, //19
		{ "TAFT" }, //22
	},
	{
		// 38
		{ "APPLE" },
		{ "ASH" },
		{ "BEECH" },
		{ "BIRCH" },
		{ "CEDAR" },
		{ "CHERRY" },
		{ "CHESTNUT" },
		{ "CYPRESS" },
		{ "DOGWOOD" },
		{ "EUCALYPTUS" },
		{ "HEMLOCK" },
		{ "HICKORY" },
		{ "LINDEN" },
		{ "MAGNOLIA" },
		{ "MAPLE" },
		{ "OAK" },
		{ "PALM" },
		{ "PEAR" },
		{ "PINE" },
		{ "REDWOOD" },
		{ "SEQUOIA" },
		{ "SPRUCE" },
		{ "SYCAMORE" },
		{ "WALNUT" },
		{ "WILLOW" },
	},
	{
		// 39
		{ "ASTER" },
		{ "BEGONIA" },
		{ "BUTTERCUP" },
		{ "CARNATION" },
		{ "CHRYSANTHEMUM" },
		{ "CROCUS" },
		{ "DAFFODIL" },
		{ "DAHLIA" },
		{ "DAISY" },
		{ "FORGET ME NOT" },
		{ "GERANIUM" },
		{ "GLADIOLUS" },
		{ "HYACINTH" },
		{ "IRIS" },
		{ "LILY" },
		{ "MARIGOLD" },
		{ "ORCHID" },
		{ "PANSY" },
		{ "PETUNIA" },
		{ "POPPY" },
		{ "ROSE" },
		{ "SNAPDRAGON" },
		{ "TULIP" },
		{ "VIOLET" },
		{ "ZINNIA" },
	},
	{
		// 40
		{ "BOWL" },
		{ "BUTTER" },
		{ "CANDLESTICK" },
		{ "CENTERPIECE" },
		{ "CHOPSTICKS" },
		{ "COFFEEPOT" },
		{ "DECANTER" },
		{ "FORK" },
		{ "GLASS" },
		{ "GOBLET" },
		{ "KETCHUP" },
		{ "KNIFE" },
		{ "LADLE" },
		{ "NAPKIN" },
		{ "PEPPER" },
		{ "PITCHER" },
		{ "PLACEMAT" },
		{ "PLATE" },
		{ "PLATTER" },
		{ "SALT" },
		{ "SAUCER" },
		{ "SPOON" },
		{ "SUGAR" },
		{ "TRIVET" },
		{ "TUREEN" },
	},
	{
		// 41 Car parts
		{ "STEERING WHEEL" },   //21
		{ "WINDSHIELD" },   //25
		{ "GLOVEBOX" }, //10
		{ "HEADLIGHT" },    //11
		{ "DEFROSTER" },    //6
		{ "HEADREST" }, //12
		{ "GAS TANK" }, //9
		{ "ODOMETER" }, //17
		{ "RADIATOR" }, //19
		{ "BATTERY" },  //1
		{ "CHASSIS" },  //4
		{ "ENGINE" },   //7
		{ "FENDER" },   //8
		{ "HUBCAP" },   //14
		{ "MIRROR" },   //15
		{ "MUFFLER" },  //16
		{ "PISTON" },   //18
		{ "SUNROOF" },  //22
		{ "CLUTCH" },   //5
		{ "BUMPER" },   //3
		{ "BRAKE" },    //2
		{ "TRUNK" },    //24
		{ "RADIO" },    //20
		{ "HORN" }, //13
		{ "TIRE" }, //23
	},
	{
		// 42
		{ "AMERICAN" },
		{ "BOURSIN" },
		{ "BRIE" },
		{ "CAMEMBERT" },
		{ "CHEDDAR" },
		{ "COLBY" },
		{ "COTTAGE" },
		{ "EDAM" },
		{ "FARMERS" },
		{ "FETA" },
		{ "GOAT" },
		{ "GORGONZOLA" },
		{ "GOUDA" },
		{ "GRUYERE" },
		{ "JARLSBERG" },
		{ "LIMBURGER" },
		{ "MUENSTER" },
		{ "MOZZARELLA" },
		{ "PARMESAN" },
		{ "PIMENTO" },
		{ "PROVOLONE" },
		{ "RICOTTA" },
		{ "ROMANO" },
		{ "ROQUEFORT" },
		{ "SWISS" },
	},
	{
		// 43
		{ "AMAZON" },
		{ "AVON" },
		{ "COLORADO" },
		{ "COLUMBIA" },
		{ "CONGO" },
		{ "DANUBE" },
		{ "ELBE" },
		{ "EUPHRATES" },
		{ "GANGES" },
		{ "HUDSON" },
		{ "JORDAN" },
		{ "LENA" },
		{ "MEKONG" },
		{ "MISSISSIPPI" },
		{ "NIGER" },
		{ "NILE" },
		{ "RHINE" },
		{ "RHONE" },
		{ "RIO GRANDE" },
		{ "SEINE" },
		{ "THAMES" },
		{ "TIBER" },
		{ "TIGRIS" },
		{ "VOLGA" },
		{ "YANGTZE" },
	},
	{
		// 44 Sports teams
		{ "CARDINALS" },    //5
		{ "REDSKINS" }, //21
		{ "DOLPHINS" }, //8
		{ "PATRIOTS" }, //17
		{ "PENGUINS" }, //18
		{ "VIKINGS" },  //24
		{ "BRONCOS" },  //3
		{ "DODGERS" },  //7
		{ "YANKEES" },  //25
		{ "PIRATES" },  //19
		{ "PISTONS" },  //20
		{ "PACKERS" },  //16
		{ "BRAVES" },   //2
		{ "EAGLES" },   //9
		{ "GIANTS" },   //10
		{ "KNICKS" },   //13
		{ "LAKERS" },   //14
		{ "BRUINS" },   //4
		{ "RED SOX" },  //22
		{ "TIGERS" },   //23
		{ "BEARS" },    //1
		{ "CUBS" }, //6
		{ "JAZZ" }, //11
		{ "JETS" }, //12
		{ "METS" }, //15
	},
	{
		// 45
		{ "CITIZEN KANE" },     //5
		{ "LOST HORIZON" },     //13
		{ "MOONSTRUCK" },       //14
		{ "CASABLANCA" },       //4
		{ "GOLDFINGER" },       //10
		{ "SPELLBOUND" },       //21
		{ "STAGECOACH" },       //22
		{ "STAR WARS" },        //23
		{ "NINOTCHKA" },        //15
		{ "PINNOCHIO" },        //17
		{ "HIGH NOON" },        //11
		{ "FANTASIA" },     //6
		{ "GASLIGHT" },     //7
		{ "TOOTSIE" },      //24
		{ "PLATOON" },      //18
		{ "REBECCA" },      //19
		{ "AIRPORT" },      //0
		{ "BEN HUR" },      //1
		{ "CABARET" },      //3
		{ "PATTON" },       //16
		{ "BUGSY" },        //2
		{ "ROCKY" },        //20
		{ "GHOST" },        //8
		{ "GIGI" },     //9
		{ "JAWS" },     //12
	},
	{
		// 46
		{ "ROLLING STONES" },   //19
		{ "FLEETWOOD MAC" },    //9
		{ "TALKING HEADS" },    //23
		{ "MOTLEY CRUE" },  //13
		{ "STEPPENWOLF" },      //21
		{ "SUPERTRAMP" },       //22
		{ "BEACH BOYS" },   //2
		{ "PINK FLOYD" },   //15
		{ "ROXY MUSIC" },       //20
		{ "PEARL JAM" },    //14
		{ "AEROSMITH" },    //0
		{ "METALLICA" },    //11
		{ "BEATLES" },      //3
		{ "ANIMALS" },      //1
		{ "BLONDIE" },      //4
		{ "CHICAGO" },      //5
		{ "GENESIS" },      //10
		{ "MONKEES" },      //12
		{ "RAMONES" },      //18
		{ "EAGLES" },       //8
		{ "POLICE" },       //16
		{ "DOORS" },        //7
		{ "QUEEN" },        //17
		{ "WINGS" },            //24
		{ "DEVO" },         //6
	},
	{
		// 47
		{ "ANDROMEDA" },
		{ "ARCTURUS" },
		{ "BETELGEUSE" },
		{ "CALLISTO" },
		{ "CANOPUS" },
		{ "CERES" },
		{ "CRAB NEBULA" },
		{ "DENEB" },
		{ "EUROPA" },
		{ "GANYMEDE" },
		{ "JUPITER" },
		{ "MARS" },
		{ "MERCURY" },
		{ "MILKY WAY" },
		{ "MOON" },
		{ "POLLUX" },
		{ "PROCYON" },
		{ "RIGEL" },
		{ "SATURN" },
		{ "SIRIUS" },
		{ "SPICA" },
		{ "SUN" },
		{ "TITAN" },
		{ "VEGA" },
		{ "VENUS" },
	},
	{
		// 48
		{ "ALDRIN" },
		{ "ARMSTRONG" },
		{ "BORMAN" },
		{ "CARPENTER" },
		{ "CHAFEE" },
		{ "COLLINS" },
		{ "CONRAD" },
		{ "COOPER" },
		{ "CRIPPEN" },
		{ "GAGARIN" },
		{ "GLENN" },
		{ "GRISSOM" },
		{ "LEONOV" },
		{ "LOVELL" },
		{ "MCNAIR" },
		{ "MITCHELL" },
		{ "RESNICK" },
		{ "RIDE" },
		{ "SCHIRRA" },
		{ "SCHMITT" },
		{ "SCOBEE" },
		{ "SHEPARD" },
		{ "SLAYTON" },
		{ "TITOV" },
		{ "WHITE" },
	},
	{
		// 49
		{ "BOSCH" },
		{ "CALDER" },
		{ "CEZANNE" },
		{ "CHAGALL" },
		{ "CHRISTO" },
		{ "DALI" },
		{ "DA VINCI" },
		{ "DONATELLO" },
		{ "GAUGUIN" },
		{ "JOHNS" },
		{ "MATISSE" },
		{ "MICHELANGELO" },
		{ "MONET" },
		{ "NEVELSON" },
		{ "OKEEFE" },
		{ "OLDENBURG" },
		{ "PICASSO" },
		{ "POLLOCK" },
		{ "REMBRANDT" },
		{ "RENOIR" },
		{ "RUBENS" },
		{ "VAN GOGH" },
		{ "WARHOL" },
		{ "WHISTLER" },
		{ "WYETH" },
	}
};

int anWordIndex[NUMBEROFLISTS][WORDSPERLIST] = {
	{5, 10, 12, 13, 14, 22, 7, 0, 1, 2, 3, 4, 6, 8, 9, 11, 15, 16, 17, 18, 19, 20, 21, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
//10
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{23, 24, 6, 20, 10, 11, 9, 12, 0, 1, 4, 7, 8, 13, 14, 19, 21, 22, 16, 15, 17, 18, 5, 2, 3 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{19, 9, 12, 0, 21, 2, 3, 17, 18, 22, 23, 5, 6, 11, 8, 13, 16, 20, 4, 14, 15, 10, 1, 24, 7 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
//20:
	{1, 12, 22, 4, 5, 23, 13, 3, 17, 19, 20, 6, 7, 8, 11, 9, 24, 0, 21, 2, 16, 10, 14, 15, 18 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{4, 8, 12, 18, 5, 11, 14, 9, 19, 0, 3, 6, 20, 2, 17, 24, 7, 10, 15, 16, 1, 22, 13, 21, 23 },
	{19, 20, 3, 8, 6, 17, 0, 4, 2, 5, 13, 14, 9, 10, 11, 23, 15, 22, 7, 1, 12, 18, 21, 24, 16 },
	{24, 13, 18, 20, 2, 14, 8, 9, 4, 6, 0, 16, 23, 10, 1, 3, 5, 7, 11, 12, 17, 19, 21, 22, 15 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
//30:
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{20, 23, 10, 3, 4, 15, 6, 8, 12, 13, 14, 11, 16, 2, 9, 19, 22, 24, 0, 7, 17, 1, 5, 18, 21 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
//40:
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{20, 24, 9, 10, 5, 11, 8, 16, 18, 0, 3, 6, 7, 13, 14, 15, 17, 21, 4, 2, 1, 23, 19, 12, 22 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{4, 20, 7, 16, 17, 23, 2, 6, 24, 18, 19, 15, 1, 8, 9, 12, 13, 3, 21, 22, 0, 5, 10, 11, 14 },
	{5, 13, 14, 4, 10, 21, 22, 23, 15, 17, 11, 6, 7, 24, 18, 19, 0, 1, 3, 16, 2, 20, 8, 9, 12 },
	{19, 9, 23, 13, 21, 22, 2, 15, 20, 14, 0, 11, 3, 1, 4, 5, 10, 12, 18, 8, 16, 7, 17, 24, 6 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
};

} // namespace WordSearch
} // namespace HodjNPodj
} // namespace Bagel
