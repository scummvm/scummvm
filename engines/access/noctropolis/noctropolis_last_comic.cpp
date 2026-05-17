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

/* Comic Data from Noctropolis Macos, offset 0x999a0 */

#include "noctropolis_comicviewer.h"
#include "noctropolis_last_comic.h"


namespace Access {

namespace Noctropolis {

static const struct Polygon poly_p1_b0 = { {
	{ 105, 247 },
	{ 580, 247 },
	{ 580, 321 },
	{ 105, 321 },
} };

static const struct ComicBox block_p1_b0_boxes[] = {
	{ 0, 246, 105, 318, "IN THE OUTSKIRTS OF NOCTROPOLIS CITY, THREE SILHOUETTES\nDETACH THEMSELVES FROM THE NIGHTSCAPE AND STRUGGLE TO FIND\nFORM BENEATH THE WAN LIGHTING OF THE CITY AMUSEMENT PARK.\nSTARTLED TO ENCOUNTER EACH OTHER, THE PREDATORS PAUSE JUST\nLONG ENOUGH TO DEMAND ANSWERS....", "DANS LA BANLIEUE DE NOCTROPOLIS CITY, TROIS SILHOUETTES\nSE DETACHENT DANS LA NUIT ET LUTTENT POUR PRENDRE FORME\nDANS LA LUMIERE BLAFARDE DU PARC D'ATTRACTIONS DE LA VILLE.\nFASCINES PAR CETTE RENCONTRE, LES PREDATEURS S'ARRETENT A\nPEINE LE TEMPS DE DEMANDER QUELQUES REPONSES...", "EN LA OSCURIDAD DE NOCTROPOLIS, TRES SILUETAS\nINTENTAN ORIENTARSE EN MEDIO DEL PARQUE DE ATRACCIONES.\nABSORTOS EN BUSCARSE ENTRE SI, LOS MERODEADORES NO\nATIENDEN A NADA MAS...", "IN EINEM RANDBEZIRK VON NOCTROPOLIS L\x99""SEN SICH DREI\nSILHOUETTEN AUS DER NACHT UND GEWINNEN M\x9a""HSAM KLARE UMRISSE IN\nDER SCHWACHEN BELEUCHTUNG DES ST\x8e""DTISCHEN VERGN\x9a""GUNGSPARKS. IN\nIHRER \x9a""BERRASCHUNG, SICH HIER ZU TREFFEN, UNTERBRECHEN DIESE\nRAUBTIERE IHREN BEUTEZUG GERADE LANGE GENUG,\nUM EIN PAAR ANTWORTEN ZU VERLANGEN..." },
};

static const struct ComicBlock block_p1_0 = { &poly_p1_b0, -1, -1, block_p1_b0_boxes, 1 };

static const struct Polygon poly_p1_b1 = { {
	{ 58, 0 },
	{ 580, 0 },
	{ 580, 399 },
	{ 58, 399 },
} };

static const struct ComicBox block_p1_b1_boxes[] = {
	{ 0, 246, 10, 50, "A SADISTIC MAGICIAN...\n\nA RUTHLESS GUNSLINGER...\n\nA WANTON CANNIBAL...\n\nEACH SUMMONED FROM DARKNESS BY THE STROKE OF MIDNIGHT.", "UNE MAGICIENNE SADIQUE...\n\nUN MALFRAT CRUEL...\n\nUN CANNIBALE PERVERS...\n\nTOUS ONT  ETE  INVOQUES PAR LES TENEBRES \nLORSQUE MINUIT A SONNE", "UNA MAGA SADICA...\n\nUN ASESINO DESPIADADO...\n\nUNA CANIBAL OBSESA...\n\nLOS TRES PARECEN INVOCADOS POR LAS TINIEBLAS.", "EINE SADISTISCHE MAGIERIN...\n\nEIN GNADENLOSER REVOLVERHELD...\n\nEINE B\x99""SARTIGE KANNIBALIN...\n\nSCHLAG MITTERNACHT WERDEN SIE ALLE AUS DEM DUNKEL GERUFEN..." },
};

static const struct ComicBlock block_p1_1 = { &poly_p1_b1, 99, 123, block_p1_b1_boxes, 1 };

static const struct ComicBlock page_1_blocks[] = {
	block_p1_0,
	block_p1_1,
};

static struct ComicPage page_1 = { "COMDATA/LEPG1_2.SCN", 98, 41, 1, 0, page_1_blocks, 2 };

static const struct Polygon poly_p2_b0 = { {
	{ 106, 0 },
	{ 146, 0 },
	{ 146, 186 },
	{ 106, 186 },
} };

static const struct ComicBox block_p2_b0_boxes[] = {
	{ 0, 246, 114, 178, "TOPHAT... A DANGEROUS MIX OF MISCHIEF AND MAGIC.", "TOPHAT... UN DANGEREUX MELANGE DE MECHANCETE ET DE MAGIE.", "LA DIVA... PELIGROSA MEZCLA DE MAGIA Y CRUELDAD.", "MISS ZYLINDERCHEN...EINE GEF\x8e""HRLICHE MISCHUNG AUS\nBOSHEIT UND ZAUBEREI." },
};

static const struct ComicBlock block_p2_0 = { &poly_p2_b0, -1, -1, block_p2_b0_boxes, 1 };

static const struct Polygon poly_p2_b1 = { {
	{ 165, 0 },
	{ 207, 0 },
	{ 207, 186 },
	{ 165, 186 },
} };

static const struct ComicBox block_p2_b1_boxes[] = {
	{ 0, 246, 176, 178, "DESPERADO... A HARDENED GUNMAN WITH A QUICK TEMPER\nAND LIGHTNING DRAW.", "DESPERADO... UN TIREUR ENTRAINE, QUI SE FACHE VITE ET\nDEGAINE ENCORE PLUS VITE.", "DESESPERADO... ASESINO SIEMPRE DISPUESTO A SACAR\nSU ARMA.", "SHOWDOWN... EIN ERFAHRENER PISTOLENSCH\x9a""TZE, DER\nBLITZSCHNELL ZIEHT." },
};

static const struct ComicBlock block_p2_1 = { &poly_p2_b1, -1, -1, block_p2_b1_boxes, 1 };

static const struct Polygon poly_p2_b2 = { {
	{ 224, 0 },
	{ 265, 0 },
	{ 256, 186 },
	{ 224, 186 },
} };

static const struct ComicBox block_p2_b2_boxes[] = {
	{ 0, 246, 235, 178, "WIDOWMARK... A NYMPHOMANIAC THAT DEVOURS HER MATES.", "WIDOWMARK... UNE NYMPHOMANE QUI DEVORE SES AMANTS.", "VIUDA NEGRA... NINFOMANIACA Y CANIBAL.", "DIE SCHWARZE WITWE...EINE NYMPHOMANIN, DIE IHRE\nGELIEBTEN AUFREISST." },
};

static const struct ComicBlock block_p2_2 = { &poly_p2_b2, -1, -1, block_p2_b2_boxes, 1 };

static const struct Polygon poly_p2_b3 = { {
	{ 126, 356 },
	{ 253, 356 },
	{ 227, 399 },
	{ 152, 399 },
} };

static const struct ComicBox block_p2_b3_boxes[] = {
	{ 0, 246, 150, 350, "DARKSHEER... VIGILANTE. SENTINEL. EYES OF THE NIGHT.", "DARKSHEER... LE VIGILE. LA SENTINELLE. LES YEUX DE LA NUIT.", "TENEBROSO... VIGILANTE, CENTINELA.", "DARKSHEER...DER W\x8e""CHTER. DER BESCH\x9a""TZER. DIE AUGEN\nDER NACHT." },
};

static const struct ComicBlock block_p2_3 = { &poly_p2_b3, -1, -1, block_p2_b3_boxes, 1 };

static const struct Polygon poly_p2_b4 = { {
	{ 60, 0 },
	{ 317, 0 },
	{ 317, 399 },
	{ 60, 399 },
} };

static const struct ComicBox block_p2_b4_boxes[] = {
	{ 3, 244, 250, 218, "     TOPHAT? WIDOWMARK! WHAT\nTHE HELL ARE YOU TWO DOIN'\n             HERE?", "      TOPHAT? WIDOWMARK!\n QU'EST-CE QUE VOUS FAITES ICI\n        TOUTES LES DEUX?", "     DIVA? VIUDA NEGRA! QUE\nDEMONIOS HACEIS \n             AQUI?", "           ZYLINDERCHEN?\n SCHWARZE WITWE? WAS ZUR H\x99""LLE\n       MACHT IHR BEIDE HIER?" },
};

static const struct ComicBlock block_p2_4 = { &poly_p2_b4, -1, -1, block_p2_b4_boxes, 1 };

static const struct Polygon poly_p2_b5 = { {
	{ 319, 0 },
	{ 377, 0 },
	{ 409, 105 },
	{ 318, 147 },
} };

static const struct ComicBox block_p2_b5_boxes[] = {
	{ 4, 244, 129, 18, "      YOU LADIES BETTER TALK\n  FAST, 'CAUSE I GOT OTHER\nBUSINESS HERE, AND MY FINGER'S\n  JUST ITCHIN' FOR THE TRIGGER!", "      VOUS FERIEZ MIEUX DE\n  VOUS EXPLIQUER , ET VITE, J'AI\nAUTRE CHOSE A FAIRE, ET MON DOIGT\n   ME DEMANGE SUR LA GACHETTE!", "     MEJOR HABLAD RAPIDO\n  PORQUE TENGO UN ASUNTO\nQUE LIQUIDAR Y MIS DEDOS ESTAN\n ANSIOSOS POR DISPARAR!", "          ALSO, LADIES, IHR\n     REDET BESSER SCHNELL, DENN\nICH HABE HEIR ETWAS ZU ERLEDIGEN,\n       UND MEIN ABZUGSFINGER\n       JUCKT SCHON GEWALTIG!" },
};

static const struct ComicBlock block_p2_5 = { &poly_p2_b5, -1, -1, block_p2_b5_boxes, 1 };

static const struct Polygon poly_p2_b6 = { {
	{ 377, 0 },
	{ 530, 0 },
	{ 453, 244 },
} };

static const struct ComicBox block_p2_b6_boxes[] = {
	{ 2, 244, 240, 132, "     STAY OUT OF MY WAY\nGUNSLINGER! I WAS INVITED!", "     HORS DE MON CHEMIN!\n      JE SUIS INVITEE!", "   APARTATE DE MI\nCAMINO! ESTOY INVITADA!", "       HALT DICH BLO\xe1"" AUS\n  MEINEN ANGELEGENHEITEN RAUS!\n      ICH WURDE EINGELADEN!" },
};

static const struct ComicBlock block_p2_6 = { &poly_p2_b6, -1, -1, block_p2_b6_boxes, 1 };

static const struct Polygon poly_p2_b7 = { {
	{ 530, 0 },
	{ 580, 0 },
	{ 580, 145 },
	{ 496, 105 },
} };

static const struct ComicBox block_p2_b7_boxes[] = {
	{ 2, 244, 320, 108, "     INDEED, MY DEAR. IT WOULD\nSEEM THAT OUR MEETING IS NOT SO\n     CIRCUMSTANTIAL, AFTER ALL.", "      VRAIMENT, TRES CHERE...\n   IL ME SEMBLE QUE NOTRE\n  RENCONTRE N'EST PAS TOUT A FAIT\n       FORTUITE, FINALEMENT...", "  CARAMBA, QUERIDA. PARECE\nQUE NUESTRO ENCUENTRO NO HA\n   SIDO TAN CASUAL.", "           WIRKLICH, MEINE\n     LIEBE? ES SCHEINT, ALS SEI\n    UNSER ZUSAMMENTREFFEN HIER\n         NICHT GANZ ZUF\x8e""LLIG." },
};

static const struct ComicBlock block_p2_7 = { &poly_p2_b7, -1, -1, block_p2_b7_boxes, 1 };

static const struct Polygon poly_p2_b8 = { {
	{ 349, 232 },
	{ 445, 233 },
	{ 445, 334 },
	{ 339, 334 },
} };

static const struct ComicBox block_p2_b8_boxes[] = {
	{ 3, 244, 459, 168, "        THAT'S VERY POSSIBLE,\n   DESPERADO. LATELY, DARKSHEER'S\n BEEN QUITE INTENT UPON ELIMINATING\nHIS ENEMIES, AND I DARE SAY WE THREE\nARE THE LAST LEFT. BUT CONSIDER THE\n GORGEOUS POSSIBILITIES IF THE\n    THREE OF US ELECTED TO JOIN\n           AGAINST HIM!", "         C'EST TRES POSSIBLE,\n   DESPERADO. DERNIEREMENT,\n  DARKSHEER A MIS TOUTE SON\n ENERGIE DANS LA DESTRUCTION DE SES\n   ENNEMIS, ET J'OSE DIRE QUE NOUS\n         SOMMES LES DERNIERS.", "             ES MUY PROBABLE,\n  DESESPERADO. TENEBROSO QUIERE\n INTENTAR ELIMINAR A SUS ENEMIGOS\nY ME PARECE QUE SOLO QUEDAMOS \nNOSOTROS TRES. PERO TE IMAGINAS QUE\n PASARIA SI UNIERAMOS NUESTRAS\n    FUERZAS PARA ACABAR CON\n           EL?", "               DAS IST GUT\n        M\x99""GLICH, SHOWDOWN. IN\n     LETZTER ZEIT LEGT DARKSHEER\n   ES DARAUF AN, ALLE SEINE FEINDE\n  AUSZUSCHALTEN. UND ICH WAGE MAL\n    ZU BEHAUPTEN, WIR DREI SIND DIE\n         LETZTEN \x9a""BERLEBENDEN." },
	{ 3, 244, 459, 168, nullptr, "          MAIS PENSEZ AUX\n   POSSIBILITES FASCINANTES QUI\n   S'OFFRENT A NOUS SI NOUS NOUS\n        UNISSONS CONTRE LUI!", nullptr, "       ABER MALT EUCH BLO\xe1""\n   MAL DIE M\x99""GLICHKEITEN AUS,\n    WENN WIR DREI UNS GEGEN IHN\n          VERB\x9a""NDETEN!" },
	{ 4, 244, 137, 260, "     THREE AGAINST ONE... LADY,\nI LIKE THEM ODDS.", "      TROIS CONTRE UN...\n  JE PREFERE L'EQUILIBRE.", "     TRES CONTRA UNO... ME\n GUSTA COMO SUENA.", "       DREI GEGEN EINEN...\nLADY, DAS VERH\x8e""LTNIS GEF\x8e""LLT MIR." },
};

static const struct ComicBlock block_p2_8 = { &poly_p2_b8, -1, -1, block_p2_b8_boxes, 3 };

static const struct Polygon poly_p2_b9 = { {
	{ 318, 147 },
	{ 409, 105 },
	{ 439, 203 },
	{ 349, 232 },
	{ 317, 238 },
} };

static const struct ComicBox block_p2_b9_boxes[] = {
	{ 1, 244, 451, 160, "     I RECEIVED ONE OF\nDARKSHEER'S PRETTY INVITATIONS\n          AS WELL.", "     J'AI MOI AUSSI RE\x80""U\nUNE DES INVITATIONS DE DARKSHEER.", "   RECIBI UNA DE LAS\nPRECIOSAS INVITACIONES DE\n       TENEBROSO.", "         ICH HABE AUCH EINE\n    VON DARKSHEERS H\x9a""BSCHEN\n      EINLADUNGEN BEKOMMEN." },
};

static const struct ComicBlock block_p2_9 = { &poly_p2_b9, -1, -1, block_p2_b9_boxes, 1 };

static const struct Polygon poly_p2_b10 = { {
	{ 472, 221 },
	{ 566, 222 },
	{ 566, 292 },
	{ 472, 292 },
} };

static const struct ComicBox block_p2_b10_boxes[] = {
	{ 4, 244, 319, 196, "     ALL RIGHT, WE FIGHT\n  TOGETHER. BUT NO MATTER WHO\n  BRINGS HIM DOWN, I GET\n      DARKSHEER'S BODY.", "      D'ACCORD, NOUS NOUS\n  BATTRONS ENSEMBLE. MAIS JE\n  VEUX LE CORPS DE DARKSHEER,\n    MEME SI CE N'EST PAS\n       MOI QUI LE TUE.", "   ESTA BIEN, PELEAREMOS\n  JUNTOS. PERO NO IMPORTA QUIEN\n  LO MATE. YO QUIERO SU\n    CADAVER PARA MI.", "             IN ORDNUNG,\n    TUN WIR UNS ZUSAMMEN. ABER\n   EGAL, WER IHN FERTIGMACHT, ICH\n         BEKOMME DARKSHEERS\n                K\x99""RPER!" },
};

static const struct ComicBlock block_p2_10 = { &poly_p2_b10, -1, -1, block_p2_b10_boxes, 1 };

static const struct Polygon poly_p2_b11 = { {
	{ 496, 105 },
	{ 580, 145 },
	{ 580, 233 },
	{ 466, 200 },
} };

static const struct ComicBox block_p2_b11_boxes[] = {
	{ 2, 244, 312, 188, "     THAT STINKIN' WEASEL SENT\nME ONE TOO... THIS WHOLE DAMN\n    THING STINKS LIKE A SET-UP!", "     CE RAT PUANT M'EN A AUSSI\nENVOYE UNE...TOUTE CETTE HISTOIRE\n        SENT LE COUP MONTE!", "   ESE CERDO ME ENVIO UNA\nTAMBIEN...  ESTE ASUNTO\n   HUELE A ENCERRONA!", "         DIESES STINKENDE\n     WIESEL HAT MIR AUCH EINE\n   GESCHICKT...DAS GANZE RIECHT\n         NACH EINER FALLE." },
};

static const struct ComicBlock block_p2_11 = { &poly_p2_b11, -1, -1, block_p2_b11_boxes, 1 };

static const struct Polygon poly_p2_b12 = { {
	{ 459, 320 },
	{ 583, 320 },
	{ 581, 375 },
	{ 458, 375 },
} };

static const struct ComicBox block_p2_b12_boxes[] = {
	{ 4, 244, 302, 262, "        OH, WIDOWMARK, HOW\n   DELICIOUSLY MORBID! OF COURSE\n YOU MAY CLAIM YOUR TROPHY. TELL\nME, DON'T YOU BOTH JUST ADORE THE\n   INVITATIONS? I SIMPLY LOVE\n    DARKSHEER'S FLAIR FOR THE\n            DRAMATIC!", "        OH, WIDOWMARK, QUELLE\n   CHARMANTE ATTENTION MORBIDE!\n BIEN SUR, VOUS POURREZ RECLAMER\n  VOTRE TROPHEE. DITES-MOI, VOUS\n   N'ADOREZ PAS LES INVITATIONS?\n    J'ADORE LE FLAIR DE DARKSHEER\n       POUR LES HISTOIRES\n              DRAMATIQUES!", "        QUE DELICIOSAMENTE\n  MORBOSA ERES, VIUDA NEGRA! POR\n SUPUESTO QUE TE DAREMOS EL TROFEO.\n NO OS ENCANTA LA AFICION QUE\n SIENTE TENEBROSO POR LO\n            DRAMATICO?", "         OH, SCHWARZE WITWE,\n    WIE HERRLICH MORBID! NAT\x9a""RLICH\n   SOLLST DU DEINE TROPH\x8e""E HABEN!\nSAGT MAL, GEFALLEN EUCH DARKSHEERS\n    EINLADUNGEN AUCH SO GUT? ICH\n        BEWUNDERE SEIN FLAIR\n          F\x9a""R DAS DRAMATISCHE." },
	{ 2, 244, 270, 342, "     YEAH, WHATEVER, TOPHAT.\nDARK BOY MUST BE WAITIN' INSIDE\n             FOR US....", "     OUI, PEUT-ETRE, TOPHAT.\n     IL DOIT NOUS ATTENDRE A\n         L'INTERIEUR...", "     LO QUE TU DIGAS, DIVA.\nADELANTE. EL CHICO OSCURO DEBE\n    ESTAR ESPERANDO...", "       JAJA, WAS AUCH IMMER,\n    ZYLINDERCHEN. DER FINSTERE\n  BUBE MU\xe1"" WOHL DRINNEN AUF UNS\n              WARTEN..." },
	{ 4, 244, 340, 275, "     THEN LET'S GET GOING...\n       I WANT TO FEED!", "     ALLONS-Y... JE DOIS\n       ME NOURRIR!", "   ENTONCES VANOS...\n       TENGO HAMBRE!", "      DANN LA\xe1""T UNS GEHEN...\n   ICH WILL HEUTE ZEITIG ESSEN!" },
};

static const struct ComicBlock block_p2_12 = { &poly_p2_b12, -1, -1, block_p2_b12_boxes, 3 };

static const struct ComicBlock page_2_blocks[] = {
	block_p2_0,
	block_p2_1,
	block_p2_2,
	block_p2_3,
	block_p2_4,
	block_p2_5,
	block_p2_6,
	block_p2_7,
	block_p2_8,
	block_p2_9,
	block_p2_10,
	block_p2_11,
	block_p2_12,
};

static struct ComicPage page_2 = { "COMDATA/LEPG3_4.SCN", 98, 42, 1, 0, page_2_blocks, 13 };

static const struct Polygon poly_p3_b0 = { {
	{ 59, 265 },
	{ 110, 265 },
	{ 79, 232 },
	{ 191, 211 },
	{ 302, 234 },
	{ 273, 266 },
	{ 324, 266 },
	{ 324, 399 },
	{ 59, 399 },
} };

static const struct ComicBox block_p3_b0_boxes[] = {
	{ 7, 246, 265, 225, "      ONE LITTLE, TWO LITTLE,\nTHREE LITTLE INDIANS...WELCOME TO\n          THRILLSVILLE.", "       UN PETIT INDIEN, DEUX\n   PETITS INDIENS, TROIS PETITS\n    INDIENS... BIENVENUE DANS LA\n         VILLE DES FRISSONS!", "   UN INDIECITO, DOS\nINDIECITOS, TRES INDIECITOS EN\n      EL PARQUECITO.", "          EINS, ZWEI, DREI\n  KLEINE NEGERLEIN...WILLKOMMEN\n          IN RISIKOHAUSEN." },
	{ 3, 244, 234, 318, "       WHERE IS HE?", "\n           OU EST-IL?", "       DONDE ESTA?", "\n             WO IST ER?" },
	{ 3, 244, 254, 319, "      HE'S WATCHING US FROM\nTHE SHADOWS, OF COURSE... HONEY,\n           WE'RE HOME!", "      IL NOUS REGARDE DANS\nL'OMBRE, EVIDEMMENT. CHERIE, NOUS\n         SOMMES CHEZ NOUS!", "    MIRANDONOS DESDE LAS\nSOMBRAS, POR SUPUESTO. AQUI\n    ESTAMOS, QUERIDO!", "        ER BEOBACHTET UNS\n    NAT\x9a""RLICH AUS DEM SCHATTEN.\n     SCHATZ, WIR SIND ZU HAUSE!" },
	{ 3, 244, 272, 325, "      THAT BASTARD'S MORE\n     YELLA THAN I THOUGHT!", "     CE BATARD CRIE PLUS\n   FORT QUE JE NE CROYAIS!", "   ESE BASTARDO ES PEOR\n QUE UNA PATADA EN EL TRASERO!", "        DER BASTARD IST NOCH\n      FEIGER, ALS ICH DACHTE!" },
};

static const struct ComicBlock block_p3_0 = { &poly_p3_b0, -1, -1, block_p3_b0_boxes, 4 };

static const struct Polygon poly_p3_b1 = { {
	{ 325, 0 },
	{ 479, 0 },
	{ 479, 133 },
	{ 325, 133 },
} };

static const struct ComicBox block_p3_b1_boxes[] = {
	{ 1, 244, 475, 70, "      YOU-HOO, DARKSHEER!\nCOME OUT, COME OUT WHEREVER\n           YOU ARE!", "      YOU-HOU, DARKSHEER!\n     VENEZ! ALLEZ, SORTEZ\n           DE LA...!", "      HOLA, TENEBROSO!\nDONDEQUIERA QUE ESTES, DEJATE\n           VER!", "       JUHUUH, DARKSHEER!\n   KOMM RAUS, KOMM RAUS, WO\n          IMMER DU BIST!" },
	{ 2, 244, 130, 84, "    SHUDDUP TOPHAT! NO REASON\nTHE WHOLE DAMN WORLD'S GOTTA\n          KNOW WE'RE COMIN'!", "    LA FERME,TOPHAT! LE MONDE\nENTIER N'A PAS BESOIN DE SAVOIR\n        QUE NOUS ARRIVONS!", "    CALLATE, DIVA! NO HAY\nRAZON PARA REVELARLE NUESTRA\n          POSICION!", "             SCHNAUZE, \n     ZYLINDERCHEN! ES MU\xe1"" JA\n NICHT GLEICH DIE GANZE VERDAMMTE\n        WELT H\x99""REN, DA\xe1"" WIR\n              KOMMEN!" },
};

static const struct ComicBlock block_p3_1 = { &poly_p3_b1, -1, -1, block_p3_b1_boxes, 2 };

static const struct Polygon poly_p3_b2 = { {
	{ 325, 133 },
	{ 479, 133 },
	{ 479, 196 },
	{ 325, 196 },
} };

static const struct ComicBox block_p3_b2_boxes[] = {
	{ 2, 244, 208, 198, "      THIS IS BULL-DIP! HIDE 'N\n  SEEK IS A SISSY-BOY KID-GAME!\nA MAN OUGHTA DO HIS FIGHTIN' OUT IN\n        THE OPEN, I TELL YA.", "      C'EST NUL! LES JEUX DE\n  CACHE-CACHE, C'EST BON POUR LA\nMATERNELLE! UN HOMME DOIT SAVOIR\n     SE BATTRE A DECOUVERT, MOI\n         JE VOUS LE DIS.", "      ESTO APESTA! SOLO LOS\n  COBARDES SE ESCONDEN ASI!\nUN HOMBRE VERDADERO DA LA CARA Y\n        PELEA.", "\n         DAS IST DOCH ALLES\n        BULLENSCHEI\xe1""E HIER!\n  VERSTECKSPIELEN IST WAS F\x9a""R FEIGE\n   JUNGS! EIN MANN SOLLTE SEINE\n        K\x8e""MPFE GANZ OFFEN\n           AUSTRAGEN!" },
};

static const struct ComicBlock block_p3_2 = { &poly_p3_b2, -1, -1, block_p3_b2_boxes, 1 };

static const struct Polygon poly_p3_b3 = { {
	{ 325, 196 },
	{ 478, 196 },
	{ 478, 212 },
	{ 588, 212 },
	{ 588, 290 },
	{ 511, 290 },
	{ 458, 312 },
	{ 325, 229 },
} };

static const struct ComicBox block_p3_b3_boxes[] = {
	{ 3, 244, 460, 182, "      I KNOW HE'S HERE...\n      I CAN SMELL HIM!", "      JE SAIS QU'IL EST LA... \n      JE LE SENS!", "  SE QUE ESTA AQUI...\n      PUEDO OLERLO!", "       ICH WEI\xe1"", DA\xe1"" ER HIER\n    IST....ICH KANN IHN WITTERN!" },
	{ 4, 244, 247, 139, "      OH, HE'S HERE, LUSCIOUS,\n AND HE'S CLOSER THAN YOU KNOW.\n I DO BELIEVE I'VE FOUND OUR\n          SLIPPERY PREY...", "      OH, IL EST LA, MA BELLE,\n  ET PLUS PRES QUE VOUS NE\n CROYEZ. JE CROIS QUE J'AI TROUVE\n     NOTRE PROIE INSSAISISSABLE.", "      SI, ESTA AQUI,\n Y MAS CERCA DE LO QUE CREES.\n CREO QUE DE UN MOMENTO A\n OTRO SE DEJARA VER...", "        OH, ER IST SOGAR\n   GANZ BESTIMMT HIER, S\x9a""\xe1""E, UND\n N\x8e""HER, ALS DU AHNST. ICH GLAUBE,\n    ICH HABE UNSERE BEUTE\n           ENTDECKT." },
};

static const struct ComicBlock block_p3_3 = { &poly_p3_b3, -1, -1, block_p3_b3_boxes, 2 };

static const struct Polygon poly_p3_b4 = { {
	{ 325, 229 },
	{ 458, 312 },
	{ 439, 340 },
	{ 446, 373 },
	{ 472, 394 },
	{ 325, 394 },
} };

static const struct ComicBox block_p3_b4_boxes[] = {
	{ 3, 244, 471, 265, "    SHOW ME A TARGET, TOPHAT!\n I'M GONNA DOUBLE THAT SCUM'S\n       WEIGHT WITH HOT LEAD!", "      MONTREZ-MOI LA CIBLE,\n TOPHAT! QUAND JE L'AURAI CRIBLE\n  DE BALLES, IL PESERA DEUX FOIS\n            PLUS LOURD!", "   DIME DONDE, DIVA!\n MANDARE A ESA PESTE AL\n       CEMENTERIO!", "      WENN DU IHN GEFUNDEN\n    HAST, DANN ZEIG MIR DAS ZIEL,\n ZYLINDERCHEN! ICH VERDOPPLE DAS\n   GEWICHT DIESES ST\x9a""CKS ABSCHAUM\n               MIT BLEI!" },
};

static const struct ComicBlock block_p3_4 = { &poly_p3_b4, -1, -1, block_p3_b4_boxes, 1 };

static const struct Polygon poly_p3_b5 = { {
	{ 458, 312 },
	{ 511, 290 },
	{ 588, 290 },
	{ 588, 394 },
	{ 412, 394 },
	{ 446, 373 },
	{ 439, 340 },
} };

static const struct ComicBox block_p3_b5_boxes[] = {
	{ 4, 244, 290, 294, "     CERTAINLY, DESPERADO, BUT\nIT IS PERHAPS MORE ACCURATE TO\n     SAY THAT HE HAS FOUND US!", "\n     BIEN SUR, DESPERADO. MAIS\nIL SERAIT PEUT-ETRE PLUS JUSTE DE\n   DIRE QUE C'EST LUI QUI NOUS\n            A TROUVES!", "     SEGURO, DESESPERADO.\nPERO ME TEMO QUE SERA EL QUIEN\n     NOS DESCUBRA PRIMERO!", "\n        SICHER, SHOWDOWN.\n  ABER ES W\x8e""RE VIELLEICHT PR\x8e""ZISER,\n       ZU SAGEN, DA\xe1"" ER UNS\n            GEFUNDEN HAT." },
};

static const struct ComicBlock block_p3_5 = { &poly_p3_b5, -1, -1, block_p3_b5_boxes, 1 };

static const struct ComicBlock page_3_blocks[] = {
	block_p3_0,
	block_p3_1,
	block_p3_2,
	block_p3_3,
	block_p3_4,
	block_p3_5,
};

static struct ComicPage page_3 = { "COMDATA/LEPG5_6.SCN", 98, 43, 0, 0, page_3_blocks, 6 };

static const struct Polygon poly_p4_b0 = { {
	{ 62, 0 },
	{ 147, 137 },
	{ 62, 137 },
} };

static const struct ComicBox block_p4_b0_boxes[] = {
	{ 1, 244, 135, 92, "     WHAT THE HELL...?", "\n     MAIS QU'EST-CE QUE...?", "     QUE DEMONIOS...?", "\n        WAS ZUR H\x99""LLE...?" },
};

static const struct ComicBlock block_p4_0 = { &poly_p4_b0, -1, -1, block_p4_b0_boxes, 1 };

static const struct Polygon poly_p4_b1 = { {
	{ 62, 0 },
	{ 238, 0 },
	{ 147, 137 },
} };

static const struct ComicBox block_p4_b1_boxes[] = {
	{ 5, 246, 182, 46, "    SHOWTIME KIDDIES...\n        GERONIMO!", "\n  QUE LE SPECTACLE COMMENCE...\n        GERONIMO!", "    HOLA, CHICOS...\n        SORPRESA!", "\n JETZT IST SHOWTIME, KINDERCHEN...\n            GERONIMO!" },
};

static const struct ComicBlock block_p4_1 = { &poly_p4_b1, -1, -1, block_p4_b1_boxes, 1 };

static const struct Polygon poly_p4_b2 = { {
	{ 238, 0 },
	{ 322, 0 },
	{ 322, 137 },
	{ 147, 137 },
} };

static const struct ComicBox block_p4_b2_boxes[] = {
	{ 4, 244, 15, 28, "    HOO-HOO! BOGEY AT\n     TWELVE O'CLOCK HIGH!", "      HO HO! ENNEMI A\n   12 HEURES DROIT DESSUS!", "    TAN PUNTUAL COMO\n     SIEMPRE!", "             HO-HO!\n     FEINDFLIEGER OBEN AUF\n           ZW\x99""LF UHR!" },
	{ 2, 244, 65, 55, "    IT'S A FREAKIN' AMBUSH!", "\n     C'EST UNE EMBUSCADE!", "    ESTO ES UNA EMBOSCADA!", " \n      DAS IST EIN VERDAMMTER\n           HINTERHALT!" },
};

static const struct ComicBlock block_p4_2 = { &poly_p4_b2, -1, -1, block_p4_b2_boxes, 2 };

static const struct Polygon poly_p4_b3 = { {
	{ 62, 137 },
	{ 147, 137 },
	{ 147, 399 },
	{ 62, 399 },
} };

static const struct ComicBox block_p4_b3_boxes[] = {
	{ 3, 246, 161, 233, "    UNHHH... TOUCHDOWN!", "\n    UNHHH... ATTERRISSAGE!", "    EXACTO!", "\n  UFF...DER ADLER IST GELANDET!" },
};

static const struct ComicBlock block_p4_3 = { &poly_p4_b3, -1, -1, block_p4_b3_boxes, 1 };

static const struct Polygon poly_p4_b4 = { {
	{ 147, 137 },
	{ 322, 137 },
	{ 322, 399 },
	{ 147, 399 },
} };

static const struct ComicBox block_p4_b4_boxes[] = {
	{ 2, 244, 5, 193, "    YOU TWO STAY BACK!\n       HE'S MINE!", "       RESTEZ EN ARRIERE!\n         IL EST A MOI!!", "    RETROCEDED!\n       ES MIO!", "        IHR BEIDEN HALTET\n     EUCH RAUS! ER GEH\x99""RT MIR!" },
};

static const struct ComicBlock block_p4_4 = { &poly_p4_b4, -1, -1, block_p4_b4_boxes, 1 };

static const struct Polygon poly_p4_b5 = { {
	{ 474, 176 },
	{ 514, 193 },
	{ 528, 226 },
	{ 519, 256 },
	{ 478, 281 },
	{ 440, 269 },
	{ 421, 232 },
	{ 432, 195 },
} };

static const struct ComicBox block_p4_b5_boxes[] = {
	{ 4, 244, 250, 187, "   HOLY #@*$!! NOOOOOO!", "\n   SAINT #@*$!! NOOOOOON!", "  #@*$!! NOOOOOO!", "\n VERDAMMTE #@*$!! NEEEEEEIIIIN!" },
};

static const struct ComicBlock block_p4_5 = { &poly_p4_b5, -1, -1, block_p4_b5_boxes, 1 };

static const struct Polygon poly_p4_b6 = { {
	{ 322, 103 },
	{ 583, 103 },
	{ 583, 251 },
	{ 457, 282 },
	{ 321, 251 },
} };

static const struct ComicBox block_p4_b6_boxes[] = {
	{ 3, 244, 464, 108, "     I'M GOING TO EAT YOU\n        UP LITTLE BOY!", "     JE VAIS TE MANGER TOUT\n       CRU, PETIT HOMME!", "    TE DEVORARE,\n        MUCHACHITO!", "        KOMM, KLEINER, ICH\n   HAB' DICH ZUM FRESSEN GERN!" },
	{ 4, 246, 256, 83, "    BE CAREFUL, WIDOWMARK,\nTHIS TIME YOU MAY BITE OFF MORE\n    THAN YOU CAN CHEW!", "     ATTENTION, WIDOWMARK,\n  CETTE FOIS-CI, VOUS RISQUEZ\n    D'AVOIR LES YEUX PLUS GROS\n       QUE LE VENTRE!", "    CUIDADO, VIUDA,\nESTE BOCADO TE PUEDE RESULTAR\n    INDIGESTO!", "     VORSICHT, WITWE, BEI\xe1""\n   NICHT MEHR AB, ALS DU AUCH\n         KAUEN KANNST!" },
};

static const struct ComicBlock block_p4_6 = { &poly_p4_b6, -1, -1, block_p4_b6_boxes, 2 };

static const struct Polygon poly_p4_b7 = { {
	{ 321, 251 },
	{ 457, 282 },
	{ 457, 399 },
	{ 321, 399 },
} };

static const struct ComicBox block_p4_b7_boxes[] = {
	{ 3, 244, 444, 243, "   HELP ME! OH MY G-MMMRPH....", "\n   A L'AIDE! OH MON D-MMMRPH...", "   AYUDADME! MI D-MMMRPH....", "\n   HILFE! OH MEIN G-MMMRPH..." },
};

static const struct ComicBlock block_p4_7 = { &poly_p4_b7, 99, 58, block_p4_b7_boxes, 1 };

static const struct Polygon poly_p4_b8 = { {
	{ 457, 282 },
	{ 583, 251 },
	{ 583, 399 },
	{ 457, 399 },
} };

static const struct ComicBox block_p4_b8_boxes[] = {
	{ 4, 244, 283, 260, "    DAMN... WRAPPED HER UP\nSLICK AS SNOT AND DISAPPEARED\n   FASTER 'N SANTY CLAUS!", "    ZUT... IL L'A EMBALLEE\nCOMME UNE MOMIE ET IL A DISPARU\n  PLUS VITE QUE LE PERE NOEL!", "   ESA HA BAJADO CON\nMAS VELOCIDAD QUE\n   PAPA NOEL!", "       VERDAMMT...WICKELT\n    SIE EIN WIE DER BLITZ UND\n VERSCHWINDET SCHNELLER ALS DER\n          WEIHNACHTSMANN!" },
	{ 4, 244, 326, 284, "       IT WOULD SEEM THAT\n  WIDOWMARK GOT IN OVER HER\nHEAD. I'D NO IDEA HE COULD\n USE HIS CLOAK LIKE THAT....", "        IL ME SEMBLE QUE\n  WIDOWMARK S'EST FAIT AVOIR. JE\nNE SAVAIS PAS QU'IL POUVAIT UTILISER\n        SA CAPE COMME \x80""A....", "      PARECE QUE VIUDA\n  NEGRA HA PERDIDO LAS PATAS.\n NO SABIA QUE EL PODIA USAR\n SU CAPA DE ESA FORMA...", "\n         SCHEINT SO, ALS\n   STECKTE DIE WITWE BIS \x9a""BER BEIDE\nOHREN IN DER SACHE DRIN. ICH HATTE\n   JA KEINE AHNUNG, WAS ER MIT\n        SEINEM CAPE ALLES\n        ANSTELLEN KANN..." },
	{ 4, 244, 326, 284, "      I GUESS WE OUGHTA GO\n   FIND 'EM... WE CAN COVER MORE\n GROUND IF WE SPLIT UP. WATCH\n     YOUR BACK, LADY.", "      JE CROIS QU'IL FAUT LES\n   RETROUVER. NOUS COUVRIRONS\n  PLUS DE TERRAIN SI NOUS NOUS\n    SEPARONS. FAITES ATTENTION A\n           VOUS, MADAME.", "     VAYAMOS A POR\n  EL. CUBRIREMOS MAS TERRENO\n SI NOS SEPARAMOS. CUIDA\n    TU ESPALDA.", "        SCH\x8e""TZE, WIR M\x9a""SSEN\n    IHN SUCHEN GEHEN. WIR HABEN\n  BESSERE CHANCEN, WENN WIR UNS\n     AUFTEILEN. HALT' DIR DEN\n        R\x9a""CKEN FREI, LADY..." },
};

static const struct ComicBlock block_p4_8 = { &poly_p4_b8, -1, -1, block_p4_b8_boxes, 3 };

static const struct ComicBlock page_4_blocks[] = {
	block_p4_0,
	block_p4_1,
	block_p4_2,
	block_p4_3,
	block_p4_4,
	block_p4_5,
	block_p4_6,
	block_p4_7,
	block_p4_8,
};

static struct ComicPage page_4 = { "COMDATA/LEPG7_8.SCN", 98, 44, 0, 0, page_4_blocks, 9 };

static const struct Polygon poly_p5_b0 = { {
	{ 168, 83 },
	{ 184, 55 },
	{ 185, 63 },
	{ 269, 13 },
	{ 282, 91 },
	{ 191, 89 },
	{ 192, 100 },
} };

static const struct ComicBox block_p5_b0_boxes[] = {
	{ 2, 246, 22, 70, "     QUIET WITCH, OR I'LL PULL\n  IT ALL OUT! I'M TAKING YOU DOWN,\nWIDOWMARK. IF YOU COOPERATE, I\n       MAY LET YOU LIVE.", "     DU CALME, SORCIERE, OU\n  JE VAIS M'ENERVER! JE SUIS LE\nPLUS FORT, WIDOWMARK. SI VOUS\n   COOPEREZ, JE VOUS LAISSERAI\n         PEUT-ETRE LA VIE SAUVE.", "    QUIETA, BRUJA, O TE HARE\n  CAER! TE TENGO,\nVIUDA NEGRA. SI COOPERAS,\n     QUIZAS VIVAS.", "        RUHE, ELENDE HEXE,\n  ODER ICH REI\xe1""E DIR JEDES DAVON\n EINZELN AUS! H\x99""R ZU, WITWE, WENN\n     DU MIR HILFST, LASSE ICH DICH\n         VIELLEICHT AM LEBEN!" },
};

static const struct ComicBlock block_p5_0 = { &poly_p5_b0, -1, -1, block_p5_b0_boxes, 1 };

static const struct Polygon poly_p5_b1 = { {
	{ 59, 2 },
	{ 322, 2 },
	{ 322, 116 },
	{ 230, 116 },
	{ 230, 133 },
	{ 186, 133 },
	{ 186, 128 },
	{ 59, 128 },
} };

static const struct ComicBox block_p5_b1_boxes[] = {
	{ 3, 244, 200, 30, "    OUCH! MY HAIR...!", "\n      AIE! MES CHEVEUX!", "    OH! MI PELO...!", "\n         AUA! MEINE HAARE...!" },
};

static const struct ComicBlock block_p5_1 = { &poly_p5_b1, -1, -1, block_p5_b1_boxes, 1 };

static const struct Polygon poly_p5_b2 = { {
	{ 59, 128 },
	{ 186, 128 },
	{ 186, 194 },
	{ 59, 194 },
} };

static const struct ComicBox block_p5_b2_boxes[] = {
	{ 7, 244, 215, 85, "      HAH, FOUND YA WEASEL!\n MUCH AS I'D LIKE TO SEE WHAT\nHE'D DO TA WIDOWMARK, NOW'S THE\n  PERFECT TIME TA CALL HIM OUT...", "      HAH, JE T'AI TROUVE,\n SALE PUTOIS! J'AIMERAIS BEAUCOUP\nVOIR CE QU'IL VA FAIRE A WIDOWMARK,\n  MAIS C'EST LE MOMENT PARFAIT...", "      ESTA ENTRETENIDO.\n AUNQUE ME GUSTARIA VER LO\nQUE LE HARA A VIUDA NEGRA, ES\n  EL MOMENTO PERFECTO...", "        HA, DA IST DIE RATTE!\n    SO GERNE ICH AUCH SEHEN\n  W\x9a""RDE, WAS ER MIT DER SCHWARZEN\n WITWE ANSTELLT, JETZT IST DIE RICHTIGE\n      ZEIT, DIE SACHE HINTER\n          UNS ZU BRINGEN..." },
};

static const struct ComicBlock block_p5_2 = { &poly_p5_b2, -1, -1, block_p5_b2_boxes, 1 };

static const struct Polygon poly_p5_b3 = { {
	{ 186, 134 },
	{ 235, 134 },
	{ 235, 194 },
	{ 186, 194 },
} };

static const struct ComicBox block_p5_b3_boxes[] = {
	{ 3, 244, 275, 80, "    LET THE LADY GO,\nDUNG-LICKER! YOU N' I GOT\n  BUSINESS TA TEND TO!", "    LAISSE PARTIR LA\nFEMME, LECHE-BOUSE! TOI ET MOI,\n  IL FAUT QU'ON DISCUTE!", "   QUE LA DAMA SE\nLAS ARREGLE COMO PUEDA. YO\n  TENGO ALGO QUE HACER.", "      LA\xe1"" DIE LADY GEHEN,\n  DUNGFRESSER! DU UND ICH, WIR\n    HABEN WAS ZU ERLEDIGEN!" },
};

static const struct ComicBlock block_p5_3 = { &poly_p5_b3, -1, -1, block_p5_b3_boxes, 1 };

static const struct Polygon poly_p5_b4 = { {
	{ 230, 116 },
	{ 322, 116 },
	{ 322, 249 },
	{ 230, 249 },
} };

static const struct ComicBox block_p5_b4_boxes[] = {
	{ 4, 246, 71, 96, "    WHA...? OOOOPH!", "\n       QU...? OOOOUPH!", "    QUE...? OOOOH!", "\n           WAS...? UFF!" },
	{ 3, 244, 318, 113, "    I'M GOING TO NAIL YOU\nJUST LIKE THIS FOR EVERY STRAND\n      OF HAIR I LOST!", "    JE VAIS TE DONNER UN\nCOUP POUR CHAQUE CHEVEU QUE\n         J'AI PERDU!", "    TE HARE UNA HERIDA\nCOMO ESTA POR CADA CABELLO QUE\n      PERDI!", "    F\x9a""R JEDE EINZELNE LOCKE,\nDIE ICH VERLOREN HABE, ZAHLE ICH\n           ES DIR HEIM!" },
};

static const struct ComicBlock block_p5_4 = { &poly_p5_b4, 99, 25, block_p5_b4_boxes, 2 };

static const struct Polygon poly_p5_b5 = { {
	{ 59, 194 },
	{ 92, 199 },
	{ 111, 221 },
	{ 108, 258 },
	{ 98, 258 },
	{ 102, 268 },
	{ 59, 268 },
} };

static const struct ComicBox block_p5_b5_boxes[] = {
	{ 3, 246, 145, 190, "    GETTING AWAY! WIDOWMARK'S\n  GETTING AWAY....", "       ELLE S'ENFUIT!\n     WIDOWMARK S'ENFUIT...", "    ADIOS, VIUDA NEGRA!", "         SIE ENTKOMMT!\n     DIE SCHWARZE WITWE\n         ENTKOMMT MIR!" },
};

static const struct ComicBlock block_p5_5 = { &poly_p5_b5, -1, -1, block_p5_b5_boxes, 1 };

static const struct Polygon poly_p5_b6 = { {
	{ 98, 258 },
	{ 181, 257 },
	{ 138, 398 },
} };

static const struct ComicBox block_p5_b6_boxes[] = {
	{ 3, 244, 198, 273, "    TIME TO DIE, RATMEAT!\n       MAKE YER MOVE!", "      TON HEURE EST VENUE,\n  RAT PUANT! A TOI DE JOUER!", "  ES HORA DE MORIR, RATA!\n      DEFIENDETE!", "       ZEIT ZUM STERBEN,\nRATTENGESICHT! MACH DEINEN ZUG!" },
};

static const struct ComicBlock block_p5_6 = { &poly_p5_b6, -1, -1, block_p5_b6_boxes, 1 };

static const struct Polygon poly_p5_b7 = { {
	{ 59, 194 },
	{ 230, 194 },
	{ 230, 268 },
	{ 59, 268 },
} };

static const struct ComicBox block_p5_b7_boxes[] = {
	{ 5, 246, 220, 247, "  ...LET HER GO...CONCENTRATE!\nDON'T GIVE DESPERADO THE TIME HE\n           NEEDS.", "    ... LA LAISSER PARTIR...\n  SE CONCENTRER! NE PAS LAISSER\n     A DESPERADO LE TEMPS DONT\n         IL A BESOIN.", "  ...AHORA DEBO\nCONCENTRARME Y NO DARLE TIEMPO\n  A DESESPERADO.", "      ... LA\xe1"" SIE GEHEN... \n  KONZENTRIER' DICH! DU DARFST\n SHOWDOWN NICHT DIE ZEIT GEBEN,\n         DIE ER BRAUCHT." },
};

static const struct ComicBlock block_p5_7 = { &poly_p5_b7, -1, -1, block_p5_b7_boxes, 1 };

static const struct Polygon poly_p5_b8 = { {
	{ 323, 0 },
	{ 478, 0 },
	{ 503, 97 },
	{ 323, 97 },
} };

static const struct ComicBox block_p5_b8_boxes[] = {
	{ 6, 246, 174, 66, "    I'M HIT! NEVER MIND -\nKEEP FOCUSED! THE GRENADE HAS\n    GOT TO HIT CLOSE....", "    JE SUIS TOUCHE! RESTER\nCONCENTRE! LA GRENADE DOIT\n    TOMBER ASSEZ PRES... ", "   ESTOY HERIDO! PERO NO\nIMPORTA. LA GRANADA GOLPEARA\n    Y...", "    ICH BIN GETROFFEN...EGAL,\nKONZENTRATION! DIE GRANATE MU\xe1""\n   DICHT NEBEN IHM TREFFEN..." },
};

static const struct ComicBlock block_p5_8 = { &poly_p5_b8, 99, 13, block_p5_b8_boxes, 1 };

static const struct Polygon poly_p5_b9 = { {
	{ 478, 0 },
	{ 588, 0 },
	{ 588, 97 },
	{ 503, 97 },
} };

static const struct ComicBox block_p5_b9_boxes[] = {
	{ 6, 246, 356, 52, "    DAMN, HURTS LIKE THE\nMOTHER OF ALL PAIN! LET IT BLEED.", "         AIE, \x80""A FAIT\n      TRES MAL! JE SAIGNE.", "   MALDITA SEA, COMO\nDUELE! NO DEBO PENSAR EN ESO.", "    VERDAMMT, DAS SCHMERZT\n  WIE VERR\x9a""CKT. EGAL, LA\xe1"" ES\n          BLUTEN..." },
};

static const struct ComicBlock block_p5_9 = { &poly_p5_b9, -1, -1, block_p5_b9_boxes, 1 };

static const struct Polygon poly_p5_b10 = { {
	{ 323, 97 },
	{ 453, 97 },
	{ 453, 206 },
	{ 323, 206 },
} };

static const struct ComicBox block_p5_b10_boxes[] = {
	{ 8, 244, 156, 69, "     OH, $%#@! I'M BLIND AS A\n  FRIGGIN' BAT! WHAT THE HELL'S HE\n      PUT IN THOSE DAMNED\n          GRENADES?!", "     OH, $%#@! JE SUIS AUSSI\n  AVEUGLE QU'UNE TAUPE! MAIS\n  QU'EST-CE QU'IL PEUT BIEN METTRE\n    DANS CES MAUDITES GRENADES?", "    $%#@! ESTOY CIEGO\n  COMO UNA RATA! QUE DEMONIOS\n      PUSO EN ESA\n          GRANADA?!", "       OH, $%#@! ICH BIN\n    BLIND WIE 'NE VERFLUCHTE\n  FLEDERMAUS! WAS ZUR H\x99""LLE HAT\n     ER IN DIESE VERDAMMTEN\n        GRANATEN GEPACKT?" },
};

static const struct ComicBlock block_p5_10 = { &poly_p5_b10, 99, 10, block_p5_b10_boxes, 1 };

static const struct Polygon poly_p5_b11 = { {
	{ 453, 97 },
	{ 588, 97 },
	{ 588, 143 },
	{ 454, 143 },
} };

static const struct ComicBox block_p5_b11_boxes[] = {
	{ 6, 244, 303, 150, "    CAN'T LET HIM CIRCLE\n       'ROUND ON ME!", "    JE NE PEUX PAS LE\nLAISSER TOURNER AUTOUR DE MOI!", "   NO DEBO DEJAR QUE ME\n       SORPRENDA!", "      ICH DARF IHN NICHT IN\n     MEINEN R\x9a""CKEN LASSEN!" },
	{ 2, 244, 303, 155, "      YER TRICKS DON'T MAKE\n NO DIFFERENCE! SO COME 'N GET\nME YA FREAKIN' DEVIL, 'CAUSE\n   I'M GONNA BLOW YA TO HELL!", "         VOS RUSES NE\n   M'IMPRESSIONNENT PAS! ALORS\nVENEZ ME CHERCHER, SALE DEMON,\n     JE VAIS VOUS RENVOYER\n           EN ENFER!", "    TUS TRUCOS NO TE SERVIRAN\n DE NADA! VEN A DONDE PUEDA\nVERTE PARA VOLARTE LA CABEZA", "      DEINE TRICKS HELFEN DIR\n AUCH NICHT! LOS, KOMM DOCH HER\nUND HOL' MICH, DU TEUFEL, DAMIT ICH\n   DICH ZUR H\x99""LLE PUSTEN KANN!" },
};

static const struct ComicBlock block_p5_11 = { &poly_p5_b11, -1, -1, block_p5_b11_boxes, 2 };

static const struct Polygon poly_p5_b12 = { {
	{ 401, 206 },
	{ 504, 206 },
	{ 481, 296 },
	{ 427, 296 },
} };

static const struct ComicBox block_p5_b12_boxes[] = {
	{ 8, 246, 242, 207, "    HE'S COMPLETELY\nDISORIENTED. FIRING BLIND...NOW.\n   TAKE HIM OUT NOW!", "    IL EST COMPLETEMENT\nDESORIENTE. IL TIRE AU HASARD...JE\n VAIS M'EN OCCUPER, MAINTENANT!", "  ESTA DESORIENTADO\nY DISPARA A CIEGAS. ESTA ES MI\n   OPORTUNIDAD!", "       ER HAT V\x99""LLIG DIE\n    ORIENTIERUNG VERLOREN,\n   FEUERT BLINDLINGS DRAUFLOS.\n      JETZT...SCHALTE IHN\n            JETZT AUS!" },
};

static const struct ComicBlock block_p5_12 = { &poly_p5_b12, -1, -1, block_p5_b12_boxes, 1 };

static const struct Polygon poly_p5_b13 = { {
	{ 504, 206 },
	{ 588, 206 },
	{ 588, 320 },
	{ 481, 296 },
} };

static const struct ComicBox block_p5_b13_boxes[] = {
	{ 4, 244, 325, 179, "    WHERE THE HELL ARE\nYOU NIGHTBREED? WHEN I FIND YOU,\n    I'M GONNA - UNNNGH!!!", "     OU DIABLE ETES-VOUS\n DONC, SUPPOT DE LA NUIT? SI JE\n    VOUS ATTRAPE, JE VAIS--\n           UNNNGH!!!", "    DONDE ESTAS, DEMONIO?\nCUANDO TE ENCUENTRE TE... UNNNGH!!!", "    WO ZUR H\x99""LLE STECKST\nDU NACHTBRUT? WENN ICH DICH FINDE,\n    WERDE ICH DI---UMMMPF!!!" },
};

static const struct ComicBlock block_p5_13 = { &poly_p5_b13, -1, -1, block_p5_b13_boxes, 1 };

static const struct ComicBlock page_5_blocks[] = {
	block_p5_0,
	block_p5_1,
	block_p5_2,
	block_p5_3,
	block_p5_4,
	block_p5_5,
	block_p5_6,
	block_p5_7,
	block_p5_8,
	block_p5_9,
	block_p5_10,
	block_p5_11,
	block_p5_12,
	block_p5_13,
};

static struct ComicPage page_5 = { "COMDATA/LEPG9_10.SCN", 98, 45, 0, 0, page_5_blocks, 14 };

static const struct Polygon poly_p6_b0 = { {
	{ 255, 0 },
	{ 324, 0 },
	{ 324, 83 },
	{ 269, 83 },
} };

static const struct ComicBox block_p6_b0_boxes[] = {
	{ 5, 246, 335, 59, "    ...AND THEN THERE WERE TWO.", "\n     ... ET ILS FURENT DEUX.", "    ...Y SOLO QUEDARON DOS.", "\n  ... DA WAREN'S NUR NOCH ZWEI..." },
};

static const struct ComicBlock block_p6_0 = { &poly_p6_b0, -1, -1, block_p6_b0_boxes, 1 };

static const struct Polygon poly_p6_b1 = { {
	{ 192, 83 },
	{ 324, 83 },
	{ 324, 199 },
	{ 192, 199 },
} };

static const struct ComicBox block_p6_b1_boxes[] = {
	{ 8, 246, 5, 39, "    TOUGH LUCK, GUNSLINGER,\n   CAN'T SAY I'LL MISS YOU.", "    PAS DE CHANCE, MAIS JE NE\n  PENSE PAS QUE TU ME MANQUES\n          A L'AVENIR.", "  MALA SUERTE, PISTOLERO,\n NO TE ECHARE DE MENOS.", "    TJA, PECH, REVOLVERHELD,\n ABER ICH KANN NICHT BEHAUPTEN,\n  DA\xe1"" ICH DICH VERMISSEN WERDE." },
};

static const struct ComicBlock block_p6_1 = { &poly_p6_b1, -1, -1, block_p6_b1_boxes, 1 };

static const struct Polygon poly_p6_b2 = { {
	{ 74, 185 },
	{ 156, 185 },
	{ 156, 243 },
	{ 73, 243 },
} };

static const struct ComicBox block_p6_b2_boxes[] = {
	{ 1, 246, 160, 250, "    WIDOWMARK!", "\n         WIDOWMARK!", "  VIUDA NEGRA!", "\r         SCHWARZE WITWE!" },
};

static const struct ComicBlock block_p6_2 = { &poly_p6_b2, -1, -1, block_p6_b2_boxes, 1 };

static const struct Polygon poly_p6_b3 = { {
	{ 199, 278 },
	{ 225, 253 },
	{ 265, 246 },
	{ 325, 259 },
	{ 325, 316 },
	{ 276, 316 },
	{ 283, 300 },
	{ 208, 300 },
} };

static const struct ComicBox block_p6_b3_boxes[] = {
	{ 8, 244, 52, 216, "    ...PAY! HE'S GOING TO\n  PAY BIG! WITH HIS HEART, AND HIS\n BLOOD, AND HIS FEAR... OH, HIS\n   FEAR! HE'S GOING TO PAY....", "     ...PAYER! IL VA PAYER\n  TRES CHER! AVEC SON COEUR, ET\n SON SANG, ET SA PEUR... OH, SA\n          PEUR! IL VA PAYER....", "    ...LO PAGARAS, CERDO!\n  PAGARAS CON TU CORAZON, TU\n SANGRE Y TU ANGUSTIA... OH, SU\n  ANGUSTIA! EL PAGARA...", "    ...BEZAHLEN! UND WIE ER\n  BEZAHLEN WIRD DAF\x9a""R! MIT SEINEM\nHERZEN, SEINEM BLUT, SEINER ANGST...\n   OH JA, SEINER PANISCHEN ANGST.\n     ER WIRD DAF\x9a""R BEZAHLEN..." },
};

static const struct ComicBlock block_p6_3 = { &poly_p6_b3, -1, -1, block_p6_b3_boxes, 1 };

static const struct Polygon poly_p6_b4 = { {
	{ 60, 300 },
	{ 283, 300 },
	{ 276, 316 },
	{ 216, 316 },
	{ 251, 399 },
	{ 60, 399 },
} };

static const struct ComicBox block_p6_b4_boxes[] = {
	{ 3, 244, 175, 290, "    DESPERADO...?!", "\n        DESPERADO...?!", "    DESESPERADO...?!", "\n         SHOWDOWN...?!" },
};

static const struct ComicBlock block_p6_4 = { &poly_p6_b4, -1, -1, block_p6_b4_boxes, 1 };

static const struct Polygon poly_p6_b5 = { {
	{ 216, 316 },
	{ 324, 316 },
	{ 324, 399 },
	{ 251, 399 },
} };

static const struct ComicBox block_p6_b5_boxes[] = {
	{ 3, 246, 250, 338, "    HE GOT WHAT HE DESERVED.", "\n   IL A EU CE QU'IL MERITAIT...", "   TUVO LO QUE MERECIA.", "         ER HAT BEKOMMEN,\n       WAS ER VERDIENT HAT." },
	{ 4, 244, 52, 295, "    YOU...!", "\n          VOUS...!", "    TU...!", "\n                 DU...!" },
};

static const struct ComicBlock block_p6_5 = { &poly_p6_b5, -1, -1, block_p6_b5_boxes, 2 };

static const struct Polygon poly_p6_b6 = { {
	{ 324, 0 },
	{ 586, 0 },
	{ 586, 99 },
	{ 433, 99 },
	{ 389, 86 },
	{ 382, 50 },
	{ 362, 34 },
	{ 347, 52 },
} };

static const struct ComicBox block_p6_b6_boxes[] = {
	{ 2, 244, 242, 69, "    I DON'T KNOW HOW YOU\nREACHED THE TOP BEFORE ME, BUT\n  I'M NOT IMPRESSED.", "    JE NE SAIS PAS COMMENT\n VOUS AVEZ PU ARRIVER ICI AVANT\n    MOI, MAIS JE NE SUIS\n     PAS IMPRESSIONNEE.", "   NO SE COMO LLEGASTE\nAQUI ANTES QUE YO, PERO NO ME\n  IMPRESIONAS.", "       ICH WEI\xe1"" ZWAR NICHT,\n  WIE DU VOR MIR RAUFGEKOMMEN\n     BIST, ABER ES BEEINDRUCKT\n           MICH NICHT." },
};

static const struct ComicBlock block_p6_6 = { &poly_p6_b6, -1, -1, block_p6_b6_boxes, 1 };

static const struct Polygon poly_p6_b7 = { {
	{ 324, 0 },
	{ 347, 52 },
	{ 362, 34 },
	{ 382, 50 },
	{ 389, 86 },
	{ 405, 124 },
	{ 402, 150 },
	{ 418, 197 },
	{ 454, 245 },
	{ 324, 245 },
} };

static const struct ComicBox block_p6_b7_boxes[] = {
	{ 2, 246, 132, 71, "        I'M SURPRISED YOU\n    HAVEN'T LEARNED THAT THE\n TAPESTRY OF NIGHT IS WOVEN WITH\nSURPRISES, WIDOWMARK. BUT, IF YOU\n  UNDERESTIMATE IT, THE DARKNESS\n    WILL SWALLOW YOU WHOLE.", "       JE SUIS SURPRIS DE\n    VOIR QUE VOUS NE SAVEZ PAS\n QUE LA TAPISSERIE DE LA NUIT EST\nTISSEE DE SURPRISES, WIDOWMARK. SI\n VOUS LA SOUS-ESTIMEZ, LA NUIT VOUS\n     ENGLOUTIRA TOUTE ENTIERE.", "    ES CURIOSO QUE AUN\n  NO SEPAS QUE LA NOCHE ESTA\n LLENA DE SORPRESAS, VIUDA NEGRA.\n  SI SUBESTIMAS A LAS TINIEBLAS,\n    PEOR PARA TI.", "          ICH BIN ERSTAUNT,\n     DA\xe1"" DU IMMER NOCH NICHT\n GELERNT HAST, DA\xe1"" DER STOFF DER\nNACHT AUS \x9a""BERRASCHUNGEN GEWOBEN\n  WIRD, SCHWARZE WITWE. ABER WENN\n    DU DIE FINSTERNIS UNTERSCH\x8e""TZT,\n      WIRD SIE DICH VERSCHLINGEN." },
};

static const struct ComicBlock block_p6_7 = { &poly_p6_b7, -1, -1, block_p6_b7_boxes, 1 };

static const struct Polygon poly_p6_b8 = { {
	{ 389, 86 },
	{ 433, 99 },
	{ 586, 99 },
	{ 586, 150 },
	{ 402, 150 },
	{ 405, 124 },
} };

static const struct ComicBox block_p6_b8_boxes[] = {
	{ 2, 244, 242, 138, "    VERY POETIC!", "\n       TRES POETIQUE!", "    MUY POETICO!", "\n   WIE AUSGESPROCHEN POETISCH!" },
	{ 4, 246, 337, 55, "    I'M GLAD YOU LIKE IT,\n  BUT IT'S NOT POETRY, IT'S A\n   FUNDAMENTAL MAXIM THAT-", "      SI VOUS AIMEZ, TANT\n   MIEUX, MAIS CE N'EST PAS DE\n  LA POESIE, C'EST UNE MAXIME\n       FONDAMENTALE QUI --", "   ME ALEGRA QUE TE GUSTE,\n  PERO NO ES POESIA. ES UNA\n  REGLA BASICA QUE-", "       SCH\x99""N, DA\xe1"" ES DIR\n   GEF\x8e""LLT, ABER ES IST KEINE\n POESIE. ES IST EINE FUNDAMENTALE\n          MAXIME, DIE..." },
};

static const struct ComicBlock block_p6_8 = { &poly_p6_b8, -1, -1, block_p6_b8_boxes, 2 };

static const struct Polygon poly_p6_b9 = { {
	{ 402, 150 },
	{ 586, 150 },
	{ 586, 245 },
	{ 454, 245 },
	{ 418, 197 },
} };

static const struct ComicBox block_p6_b9_boxes[] = {
	{ 6, 244, 261, 201, "       I DON'T GIVE A RAT'S ASS\n   ABOUT YOUR MAXIMS! I LIVE BY MY\n OWN RULES AND I'VE WAITED A LONG\nTIME FOR YOUR HEAD. I NEVER EAT\n   THE HEAD, YOU KNOW? I KEEP IT\n        IN A SAFE PLACE....", "             JE ME FOUS\n   COMPLETEMENT DE VOS MAXIMES!\n JE VIS SELON MES PROPRES REGLES,\n  ET J'ATTENDS VOTRE TETE DEPUIS\n LONGTEMPS. JE NE MANGE JAMAIS LES\n  TETES, VOUS SAVEZ... JE LES GARDE\n         DANS UN ENDROIT SUR...", "    ME IMPORTAN POCO TUS\n  REGLAS! YO TENGO LAS MIAS Y HE\n ESPERADO MUCHO POR TU CABEZA.\n  NUNCA LAS COMO, SABES? LAS\n   GUARDO EN LUGAR SEGURO...", "           ICH GEBE KEINEN\n   RATTENFURZ AUF DEINE MAXIMEN!\n ICH LEBE NACH MEINEN EIGENEN REGELN,\nUND ICH HABE SCHON VIEL ZU LANGE AUF\n  DEINEN KOPF GEWARTET. ICH ESSE DEN\n   KOPF NIEMALS AUF, WEI\xe1""T DU. DEN \n    BEWAHRE ICH AN EINEM\n         EHRENPLATZ AUF..." },
};

static const struct ComicBlock block_p6_9 = { &poly_p6_b9, -1, -1, block_p6_b9_boxes, 1 };

static const struct Polygon poly_p6_b10 = { {
	{ 324, 245 },
	{ 586, 245 },
	{ 586, 290 },
	{ 388, 399 },
	{ 324, 399 },
} };

static const struct ComicBox block_p6_b10_boxes[] = {
	{ 2, 244, 176, 303, "    IT'S PAYBACK TIME!", "\n    IL EST TEMPS DE PAYER!", "  LA HORA HA LLEGADO!", "\n             ZAHLTAG!" },
};

static const struct ComicBlock block_p6_10 = { &poly_p6_b10, -1, -1, block_p6_b10_boxes, 1 };

static const struct ComicBlock page_6_blocks[] = {
	block_p6_0,
	block_p6_1,
	block_p6_2,
	block_p6_3,
	block_p6_4,
	block_p6_5,
	block_p6_6,
	block_p6_7,
	block_p6_8,
	block_p6_9,
	block_p6_10,
};

static struct ComicPage page_6 = { "COMDATA/LEPG1112.SCN", 98, 46, 0, 0, page_6_blocks, 11 };

static const struct Polygon poly_p7_b0 = { {
	{ 112, 50 },
	{ 206, 50 },
	{ 164, 91 },
	{ 216, 136 },
	{ 116, 136 },
	{ 158, 92 },
} };

static const struct ComicBox block_p7_b0_boxes[] = {
	{ 7, 244, 220, 49, "     SUCKER!", "\n             SALAUD !", "  CHUPA ESTA!", "\n           PENNER!" },
};

static const struct ComicBlock block_p7_0 = { &poly_p7_b0, 99, 104, block_p7_b0_boxes, 1 };

static const struct Polygon poly_p7_b1 = { {
	{ 175, 102 },
	{ 254, 102 },
	{ 186, 208 },
	{ 140, 136 },
	{ 216, 136 },
} };

static const struct ComicBox block_p7_b1_boxes[] = {
	{ 7, 246, 254, 107, "     DAMN HER....", "\n          MAUDITE SOIT...!", "     MALDITA...", "\n        DIESE VERDAMMTE...!" },
};

static const struct ComicBlock block_p7_1 = { &poly_p7_b1, -1, -1, block_p7_b1_boxes, 1 };

static const struct Polygon poly_p7_b2 = { {
	{ 59, 119 },
	{ 129, 119 },
	{ 161, 168 },
	{ 161, 268 },
	{ 59, 268 },
} };

static const struct ComicBox block_p7_b2_boxes[] = {
	{ 5, 246, 171, 205, "     VENOM!!!", "\n           DU VENIN!!!", "    VENENO!!!", "\n             GIFT!!!" },
};

static const struct ComicBlock block_p7_2 = { &poly_p7_b2, 99, 126, block_p7_b2_boxes, 1 };

static const struct Polygon poly_p7_b3 = { {
	{ 161, 190 },
	{ 322, 190 },
	{ 322, 282 },
	{ 161, 281 },
} };

static const struct ComicBox block_p7_b3_boxes[] = {
	{ 7, 244, 323, 172, "   ...GAINING! THE BASTARD'S\n   GAINING ON ME!", "     ...IL ME RATTRAPE! CE\n      CHIEN ME RATTRAPE!", "   ...ME PILLO! EL BASTARDO\n  ME HA PILLADO!", "    ...FERTIG! DIESER BASTARD\n  MACHT MICH WIRKLICH FERTIG!" },
};

static const struct ComicBlock block_p7_3 = { &poly_p7_b3, -1, -1, block_p7_b3_boxes, 1 };

static const struct Polygon poly_p7_b4 = { {
	{ 323, 0 },
	{ 487, 0 },
	{ 464, 168 },
	{ 322, 157 },
} };

static const struct ComicBox block_p7_b4_boxes[] = {
	{ 2, 246, 176, 48, "      I TOLD YOU TO BEHAVE!\n  EITHER SETTLE DOWN NOW, OR\n YOU CAN JOIN DESPERADO ON THE\n       BODY BAG EXPRESS!", "\n      JE VOUS AI DIT DE VOUS\n  TENIR TRANQUILLE! SOIT VOUS\n VOUS CALMEZ TOUT DE SUITE, SOIT\n VOUS ALLEZ REJOINDRE DESPERADO\n       AU RAYON DES CHERS\n            DISPARUS!", "    TE LO DIJE!\n  RINDETE O IRAS A UNIRTE A\n DESESPERADO EN EL EXPRESO AL\n       INFIERNO!", "      ICH HABE DIR GESAGT,\n  DU SOLLST DICH BENEHMEN! H\x99""R\n    JETZT AUF, ODER DU KANNST\n    SHOWDOWN IM LEICHENSACK\n         GESELLSCHAFT LEISTEN!" },
};

static const struct ComicBlock block_p7_4 = { &poly_p7_b4, -1, -1, block_p7_b4_boxes, 1 };

static const struct Polygon poly_p7_b5 = { {
	{ 428, 193 },
	{ 464, 168 },
	{ 522, 157 },
	{ 586, 178 },
	{ 586, 240 },
	{ 487, 257 },
	{ 431, 230 },
} };

static const struct ComicBox block_p7_b5_boxes[] = {
	{ 4, 244, 319, 99, "    WAIT...!", "\n         ATTENDEZ...!", "  ESPERA...!", "\n            WARTE...!" },
};

static const struct ComicBlock block_p7_5 = { &poly_p7_b5, -1, -1, block_p7_b5_boxes, 1 };

static const struct Polygon poly_p7_b6 = { {
	{ 322, 157 },
	{ 464, 168 },
	{ 428, 193 },
	{ 431, 230 },
	{ 456, 246 },
	{ 322, 249 },
} };

static const struct ComicBox block_p7_b6_boxes[] = {
	{ 1, 246, 433, 251, "    AHHHH! THAT'S IT WITCH,\nTIME TO PUNCH YOUR TICKET...\n  ALL ABOARD!", "       AHHHH! C'EST \x80""A,\n    SORCIERE, IL EST TEMPS...\n           ALLONS-Y!", "  BIEN! TOMA TU BOLETO \nAL INFIERNO. TODOS A\n  BORDO!", "     NA GUT, ALTE HEXE, DAS\n   WAR'S! ZEIT ZUM EINT\x9a""TEN!" },
};

static const struct ComicBlock block_p7_6 = { &poly_p7_b6, -1, -1, block_p7_b6_boxes, 1 };

static const struct Polygon poly_p7_b7 = { {
	{ 530, 258 },
	{ 586, 240 },
	{ 586, 399 },
	{ 530, 399 },
} };

static const struct ComicBox block_p7_b7_boxes[] = {
	{ 8, 246, 330, 290, "   AND THEN THERE WAS ONE...", "\n  ET IL N'EN RESTA PLUS QU'UNE...", "   Y SOLO UNO QUEDO...", "\n   ...DA WAR ES NUR NOCH EINER...." },
};

static const struct ComicBlock block_p7_7 = { &poly_p7_b7, -1, -1, block_p7_b7_boxes, 1 };

static const struct ComicBlock page_7_blocks[] = {
	block_p7_0,
	block_p7_1,
	block_p7_2,
	block_p7_3,
	block_p7_4,
	block_p7_5,
	block_p7_6,
	block_p7_7,
};

static struct ComicPage page_7 = { "COMDATA/LEPG1314.SCN", 98, 47, 0, 0, page_7_blocks, 8 };

static const struct Polygon poly_p8_b0 = { {
	{ 59, 0 },
	{ 324, 0 },
	{ 324, 45 },
	{ 278, 49 },
	{ 214, 80 },
	{ 188, 136 },
	{ 59, 124 },
} };

static const struct ComicBox block_p8_b0_boxes[] = {
	{ 5, 246, 218, 54, "   ...SO WEAK. IT'S THE VENOM...\n NOT SURE I CAN MAKE IT. STOP IT!\nSTAY ALERT!  I'M TWO DOWN WITH\n    ONE TO GO... WONDER WHERE\n           SHE IS...", "\n   ...SI FAIBLE. C'EST LE VENIN...\n JE NE SAIS PAS SI JE TIENDRAI LE\nCOUP. STOP! RESTER EVEILLE! JE\nSUIS AU BOUT DU ROULEAU... JE ME\n    DEMANDE OU ELLE PEUT\n          BIEN ETRE....", "   ...ESTOY DEBIL. ES EL VENENO...\n NO SE SI PODRE. BASTA!\nESTATE ALERTA! AUN TE QUEDA\n    UNA... DONDE\n           ESTARA?", "\n   ...SO SCHWACH. DAS IST DAS\n GIFT...WEI\xe1"" NICHT, OB ICH ES SCHAFFE.\nH\x99""R AUF! BLEIB WACH! ZWEI SIND SCHON\n  ABGEFR\x9a""HST\x9a""CKT, EINE IST NOCH AUF\n   DER WARTELISTE! ICH FRAGE\n      MICH, WO SIE STECKT..." },
};

static const struct ComicBlock block_p8_0 = { &poly_p8_b0, -1, -1, block_p8_b0_boxes, 1 };

static const struct Polygon poly_p8_b1 = { {
	{ 324, 45 },
	{ 278, 49 },
	{ 214, 80 },
	{ 188, 136 },
	{ 324, 149 },
} };

static const struct ComicBox block_p8_b1_boxes[] = {
	{ 3, 244, 252, 74, "    HELLO HANDSOME, BUY A\n       GIRL A DRINK?", "     SALUT, BEAU GOSSE, TU\n      M'OFFRES UN VERRE?", "    HOLA, HERMOSO. ME\n     PAGAS UNA COPA?", "    HALLO, H\x9a""BSCHER. L\x8e""DST\n  DU MICH ZU 'NEM DRINK EIN?" },
	{ 1, 246, 354, 151, "    TOPHAT...!", "\n          TOPHAT...!", "    DIVA...", "\n          ZYLINDERCHEN..!" },
};

static const struct ComicBlock block_p8_1 = { &poly_p8_b1, -1, -1, block_p8_b1_boxes, 2 };

static const struct Polygon poly_p8_b2 = { {
	{ 129, 215 },
	{ 245, 216 },
	{ 245, 315 },
	{ 129, 315 },
} };

static const struct ComicBox block_p8_b2_boxes[] = {
	{ 3, 244, 255, 199, "      YOU MAY BE DARKNESS\n INCARNATE, DEARY, BUT YOU BLEED\n   JUST LIKE ANY MISERABLE\n            STOOGE.", "      VOUS ETES PEUT-ETRE\n L'INCARNATION DES TENEBRES, TRES\n CHER, MAIS VOUS SAIGNEZ COMME\n     LE PREMIER CRETIN VENU!", "     SERAS LA ENCARNACION\n DE LA NOCHE, PERO TU SANGRE ES\n  ROJA COMO LA DE CUALQUIER\n            IDIOTA.", "        DU MAGST JA DIE\n   VERK\x99""RPERTE DUNKELHEIT SEIN,\n  SCHATZI, ABER DU BLUTEST GENAU\n    WIE JEDES ARME SCHWEIN." },
};

static const struct ComicBlock block_p8_2 = { &poly_p8_b2, -1, -1, block_p8_b2_boxes, 1 };

static const struct Polygon poly_p8_b3 = { {
	{ 59, 124 },
	{ 324, 149 },
	{ 324, 209 },
	{ 59, 232 },
} };

static const struct ComicBox block_p8_b3_boxes[] = {
	{ 1, 244, 195, 174, "    CORRECT! YOU WIN THE\n     PRIZE, BABY FACE!", "    EXACT! TU AS GAGNE LE\n  GROS LOT, GUEULE D'AMOUR!", "    EXACTO! HAS GANADO\n   EL PRECIO, NENE!", "    STIMMT! DER KANDIDAT HAT\n HUNDERT PUNKTE, MILCHGESICHT!" },
};

static const struct ComicBlock block_p8_3 = { &poly_p8_b3, 99, 27, block_p8_b3_boxes, 1 };

static const struct Polygon poly_p8_b4 = { {
	{ 60, 303 },
	{ 181, 303 },
	{ 181, 399 },
	{ 59, 399 },
} };

static const struct ComicBox block_p8_b4_boxes[] = {
	{ 3, 244, 196, 298, "        I WATCHED YOU TAKE\n   OUT DESPERADO AND WIDOWMARK...\n    YOU WERE MARVY! BUT I CAN'T\n  IMAGINE WHY YOU'RE WORKING SO\n  HARD TO REMOVE THE COMPETITION.\n       WE NEED EACH OTHER...", "        JE VOUS AI OBSERVE\n     PENDANT QUE VOUS VOUS\n    OCCUPIEZ DE DESPERADO ET DE\n      WIDOWMARK... VOUS ETIEZ\n           MERVEILLEUX!", "    FUISTE MUY HABIL PARA\n  MATAR A DESESPERADO Y VIUDA\n  NEGRA. PERO NO CONTASTE CON\n  MI ASTUCIA.\n   RECUERDA QUE SOMOS UNO...", "        ICH HABE ZUGESCHAUT,\n     WIE DU SHOWDOWN UND DIESE\n SPINNE AUSGESCHALTET HAST...DU BIST\n SUPERSUPERSUPER, BABIIIEE! ABER ICH\n  WEI\xe1"" GAR NICHT, WARUM DU DICH SO\n     ANSTRENGST MIT MIR. WIR\n     BRAUCHEN EINANDER DOCH..." },
	{ 3, 244, 196, 298, nullptr, "\n      MAIS JE NE COMPRENDS\n   PAS POURQUOI VOUS FAITES\n  TOUS CES EFFORTS POUR VOUS\n DEBARRASSER DE VOS ADVERSAIRES.\n   NOUS AVONS BESOIN DE CETTE\n           RIVALITE...", nullptr, nullptr },
};

static const struct ComicBlock block_p8_4 = { &poly_p8_b4, -1, -1, block_p8_b4_boxes, 2 };

static const struct Polygon poly_p8_b5 = { {
	{ 191, 305 },
	{ 324, 305 },
	{ 324, 399 },
	{ 191, 399 },
} };

static const struct ComicBox block_p8_b5_boxes[] = {
	{ 4, 244, 12, 285, "     ...LIKE DIFFERENT SIDES\n  OF THE SAME COIN, SWEET CHEEKS,\n WE DEFINE OUR OPPOSITE! MMMMM,\n   YUMMY... TASTES JUST LIKE\n            CHICKEN!", "    ... COMME LES DEUX COTES\n  D'UNE PIECE, NOUS DEFINISSONS\n NOTRE CONTRAIRE! MMMMM, MIAM...\n       ON DIRAIT DU POULET!", "     ...COMO LAS DOS CARAS\n  DE UNA MONEDA. SOMOS LOS\n OPUESTOS QUE NO PUEDEN VIVIR\n  EL UNO SIN EL OTRO...", "     ...WIE DIE BEIDEN SEITEN\n  DERSELBEN M\x9a""NZE, KNUDDELB\x8e""R.\n OHNE DEN ANDEREN G\x8e""BE ES KEINEN\n   VON UNS. HMMMM, LECKER...\n        SCHMECKT WIE H\x9a""HNCHEN!" },
};

static const struct ComicBlock block_p8_5 = { &poly_p8_b5, -1, -1, block_p8_b5_boxes, 1 };

static const struct Polygon poly_p8_b6 = { {
	{ 324, 0 },
	{ 562, 0 },
	{ 524, 30 },
	{ 523, 102 },
	{ 322, 102 },
} };

static const struct ComicBox block_p8_b6_boxes[] = {
	{ 6, 246, 200, 83, "    STAY CONSCIOUS... JUST\n       STAY AWAKE...!", "      RESTER CONSCIENT...\n       RESTER EVEILLE...!", "   MANTENTE DESPIERTO...\n    NO TE DUERMAS...!", "     BLEIB' BEI BEWU\xe1""TSEIN....\n      BLEIB' BLO\xe1"" WACH...!" },
};

static const struct ComicBlock block_p8_6 = { &poly_p8_b6, -1, -1, block_p8_b6_boxes, 1 };

static const struct Polygon poly_p8_b7 = { {
	{ 562, 0 },
	{ 524, 30 },
	{ 523, 102 },
	{ 558, 131 },
	{ 589, 135 },
} };

static const struct ComicBox block_p8_b7_boxes[] = {
	{ 2, 244, 340, 81, "    NOW FOR SOME REAL FUN!", "          ON VA ENFIN\n       POUVOIR S'AMUSER!", "   AHORA NOS DIVERTIREMOS!", "        UND JETZT ZU ETWAS\n         RICHTIG SPA\xe1""IGEM!" },
};

static const struct ComicBlock block_p8_7 = { &poly_p8_b7, -1, -1, block_p8_b7_boxes, 1 };

static const struct Polygon poly_p8_b8 = { {
	{ 324, 102 },
	{ 523, 102 },
	{ 558, 131 },
	{ 589, 135 },
	{ 589, 260 },
	{ 324, 260 },
} };

static const struct ComicBox block_p8_b8_boxes[] = {
	{ 4, 244, 227, 82, "     YEE-HAW, RIDE'M COWBOY!\n  I REALLY DON'T THINK YOU'RE\n INTO THIS, DEAR...I WONDER IF\nIT WOULD HELP IF I SHOVED ANOTHER\n      COIN IN YOUR SLOT!", "\n     YIPPY, ALLEZ GO! JE CROIS\n   VRAIMENT QUE VOUS N'AVEZ PAS\n  LA TETE A CE QUE VOUS FAITES,\n MON CHER! JE ME DEMANDE SI VOUS\n   N'AURIEZ PAS BESOIN D'UN\n        PETIT COUP DE MAIN!", "    VENGA, VAQUERO!\n  VEAMOS SI SABES JUGAR A\n ESTO... BAILARAS MEJOR SI\nPONGO OTRA MONEDA EN LA\n      MAQUINA?", "             JUHUUH, LOS,\n         REITE SIE ZU, COWBOY!\n  IRGENDWIE SCHEINT DIR DAS NICHT SO\n RICHTIG ZU GEFALLEN, SCHATZ. OB\nES WOHL HELFEN W\x9a""RDE, WENN ICH\n NOCH EINE WEITERE M\x9a""NZE IN DEINEN\n     SCHLITZ EINWERFE?" },
};

static const struct ComicBlock block_p8_8 = { &poly_p8_b8, -1, -1, block_p8_b8_boxes, 1 };

static const struct Polygon poly_p8_b9 = { {
	{ 324, 260 },
	{ 461, 260 },
	{ 461, 399 },
	{ 324, 399 },
} };

static const struct ComicBox block_p8_b9_boxes[] = {
	{ 4, 244, 181, 258, "    AND TO THINK YOU WANTED\nTO STAY IN TONIGHT! OH, YOU\n         SILLY BOY!", "      ET DIRE QUE VOUS NE\n  PENSIEZ PAS SORTIR CE SOIR!\n        PAUVRE IMBECILE!", "  Y PENSAR QUE IBAS A\nQUEDARTE EN CASA HOY,\n    CHICO TONTO!", "    NA BITTE, UND DU WOLLTEST\nHEUTE ABEND ZU HAUSE BLEIBEN!\n         DUMMERLE....!" },
};

static const struct ComicBlock block_p8_9 = { &poly_p8_b9, -1, -1, block_p8_b9_boxes, 1 };

static const struct Polygon poly_p8_b10 = { {
	{ 461, 260 },
	{ 589, 260 },
	{ 589, 399 },
	{ 461, 399 },
} };

static const struct ComicBox block_p8_b10_boxes[] = {
	{ 4, 244, 314, 253, "      WELL I'M SURE WE COULD\n  HAVE MORE FUN, BUT IT IS A\nSCHOOL NIGHT...SO, I'LL JUST\n       MURDER YOU NOW!!", "      JE SUIS SURE QUE NOUS\n  POURRIONS NOUS AMUSER ENCORE\nPLUS, MAIS C'EST UNE NUIT SPECIALE...\n     ALORS JE VAIS VOUS TUER!", "   ME GUSTARIA DIVERTIRME\n  UN POCO MAS, PERO DEBO IRME,\n ASI QUE TE MATARE YA!", "\n        NAJA, ALSO ICH SCH\x8e""TZE,\n    WIR K\x99""NNTEN NOCH MEHR SPA\xe1""\n  MITEINANDER HABEN, ABER ICH MU\xe1""\n    ZEITIG HEIM. ALSO WERDE ICH\n      DICH JETZT EINFACH\n       KURZ UMBRINGEN!!" },
};

static const struct ComicBlock block_p8_10 = { &poly_p8_b10, -1, -1, block_p8_b10_boxes, 1 };

static const struct ComicBlock page_8_blocks[] = {
	block_p8_0,
	block_p8_1,
	block_p8_2,
	block_p8_3,
	block_p8_4,
	block_p8_5,
	block_p8_6,
	block_p8_7,
	block_p8_8,
	block_p8_9,
	block_p8_10,
};

static struct ComicPage page_8 = { "COMDATA/LEPG1516.SCN", 98, 48, 1, 0, page_8_blocks, 11 };

static const struct Polygon poly_p9_b0 = { {
	{ 187, 0 },
	{ 306, 0 },
	{ 334, 35 },
	{ 311, 82 },
	{ 247, 95 },
	{ 187, 83 },
} };

static const struct ComicBox block_p9_b0_boxes[] = {
	{ 2, 255, 75, 40, "    BACK OFF, TOPHAT!", "\n        RECULE, TOPHAT!", "    ATRAS, DIVA!", "\n      ZUR\x9a""CK, ZYLINDERLADY!" },
};

static const struct ComicBlock block_p9_0 = { &poly_p9_b0, 99, 25, block_p9_b0_boxes, 1 };

static const struct Polygon poly_p9_b1 = { {
	{ 187, 83 },
	{ 247, 95 },
	{ 308, 102 },
	{ 278, 153 },
	{ 289, 230 },
	{ 319, 261 },
	{ 255, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p9_b1_boxes[] = {
	{ 2, 255, 10, 150, "        TOPHAT'S OUT FOR THE\n   COUNT. I FOUND DESPERADO AND I\n ASSUME THAT MOUND OF HAIR AND\nBLOOD IS WHAT'S LEFT OF WIDOWMARK.\n YOU SURE AS HELL WERE THOROUGH...\n         ARE YOU O.K.?", "\n        TOPHAT EST ELIMINEE.\n   J'AI TROUVE DESPERADO ET JE\nSUPPOSE QU'IL NE RESTE DE WIDOWMARK\nQUE CES TAS DE CHEVEUX COUVERTS DE\n     SANG. TU AS FAIT DU BEAU\n         TRAVAIL... \x80""A VA?", "      LA DIVA ESTA FUERA DE\n   COMBATE. VI A DESESPERADO Y\n SUPONGO QUE AQUELLOS RESTOS\n ES LO QUE QUEDA DE VIUDA NEGRA.\n PARECE QUE TE HAS DIVERTIDO...\n        ESTAS BIEN?", "        MISS ZYLINDERCHEN IST\n   AUSGEZ\x8e""HLT. ICH HABE SHOWDOWN\n   GEFUNDEN, UND ICH VERMUTE MAL,\n  DIESER HAUFEN BLUTIGER HAARE IST\n   ALLES, WAS VON DER SCHWARZEN\n   WITWE \x9a""BRIGGEBLIEBEN IST." },
	{ 2, 255, 10, 150, nullptr, nullptr, nullptr, "      ALSO DU ARBEITEST\n    SCHON VERDAMMT GR\x9a""NDLICH.\n       HE, BIST DU OK?" },
	{ 4, 246, 44, 208, "    I'M FINE! YOU'VE GOT A\nLOT OF NERVE, STILETTO, BUTTING\n  IN WHERE YOU DON'T BELONG!", "\n    JE VAIS TRES BIEN! TU AS\nDU CRAN, STILETTO D'INTERVENIR DANS\n   DES AFFAIRES QUI NE TE\n        REGARDENT PAS!", "   PERFECTAMENTE! NO SE\n POR QUE TE METES DONDE NO TE\n LLAMAN, STILETTO!", "\n    MIR GEHT'S GUT. DU HAST\nVIELLEICHT NERVEN, STILETTO, HIER\n  EINFACH REINZUPLATZEN, WO DU\n    GAR NICHTS VERLOREN HAST!" },
};

static const struct ComicBlock block_p9_1 = { &poly_p9_b1, -1, -1, block_p9_b1_boxes, 3 };

static const struct Polygon poly_p9_b2 = { {
	{ 278, 153 },
	{ 309, 102 },
	{ 451, 102 },
	{ 451, 251 },
	{ 353, 277 },
	{ 289, 230 },
} };

static const struct ComicBox block_p9_b2_boxes[] = {
	{ 1, 255, 443, 168, "        I'M YOUR PARTNER! I\n   DON'T KNOW WHAT YOUR PROBLEM\n IS LATELY. YOU'RE OBVIOUSLY ON\nSOME EGO TRIP, TAKING OUT ALL THE\n BUTTHEADS SINGLE-HANDEDLY, BUT IF\n  I HADN'T SHOWN, TOPHAT WOULD'VE\n    FINISHED YOU OFF, HERO!", "       JE SUIS TA PARTENAIRE!\n   JE NE SAIS PAS QUEL EST TON\n     PROBLEME DEPUIS QUELQUE\n              TEMPS.", "        SOMOS SOCIOS! NO\n   SE QUE TE PASA ULTIMAMENTE.\n TE HAS VUELTO EGOISTA, NO CUENTAS\n CONMIGO PARA NADA... PERO DE NO\n SER POR MI, LA DIVA TE HUBIERA\n    MATADO, HEROE!", "       ICH BIN DEINE PARTNERIN!\n    ICH WEI\xe1"" NICHT, WAS IN LETZTER\n        ZEIT MIT DIR LOS IST!" },
	{ 1, 255, 443, 168, nullptr, "\n     TU ES APPAREMMENT DANS\n  UN TRIP SOLITAIRE, TU ESSAYES\n  D'ELIMINER TOUS LES MALFRATS\n  DE LA VILLE D'UNE SEULE MAIN.\n   MAIS SANS MON INTERVENTION,\n     TOPHAT T'AURAIT REGLE TON\n           COMPTE, HEROS! ", nullptr, "           DU BIST WOHL AUF\n       EINEM EGO-TRIP, DA\xe1"" DU\n   ALLE MISTBACKEN GANZ ALLEINE\n AUSSCHALTEN WILLST, ABER WENN ICH\n    NICHT AUFGETAUCHT W\x8e""RE, H\x8e""TTE\n        MISS ZYLINDERCHEN DICH\n          HELDEN ABSERVIERT!" },
};

static const struct ComicBlock block_p9_2 = { &poly_p9_b2, -1, -1, block_p9_b2_boxes, 2 };

static const struct Polygon poly_p9_b3 = { {
	{ 319, 261 },
	{ 353, 277 },
	{ 329, 399 },
	{ 255, 399 },
} };

static const struct ComicBox block_p9_b3_boxes[] = {
	{ 3, 246, 384, 308, "      YOU ARE FULL OF CRAP,\n  GIRL! I HAD EVERYTHING UNDER\nCONTROL...JUST DROP IT BECAUSE THIS\n     DISCUSSION IS OVER!", "      TU NE RACONTES QUE DES\n  CONNERIES, FEMME. JE CONTROLAIS\nPARFAITEMENT LA SITUATION... LAISSE\n  TOMBER, DE TOUTE MANIERE, CETTE\n      DISCUSSION EST TERMINEE!", "  TE DAS MUCHA IMPORTANCIA,\n MUCHACHITA! TODO ESTABA BAJO\n CONTROL... LARGATE YA! NO HAY\n NADA DE QUE HABLAR!", "          DU ERZ\x8e""HLST NUR\n  BL\x99""DSINN, STILETTO! ICH HATTE\nALLES WUNDERBAR UNTER KONTROLLE...\n  VERGI\xe1"" ES, DIE DISKUSSION IST\n             GELAUFEN!" },
};

static const struct ComicBlock block_p9_3 = { &poly_p9_b3, -1, -1, block_p9_b3_boxes, 1 };

static const struct Polygon poly_p9_b4 = { {
	{ 353, 277 },
	{ 451, 251 },
	{ 451, 399 },
	{ 329, 399 },
} };

static const struct ComicBox block_p9_b4_boxes[] = {
	{ 4, 246, 198, 243, "      TAKE TOPHAT TO THE\n  POLICE, AND DON'T FORGET\nABOUT THE OTHER BODIES. I'M\n    GOING TO SHADOWLAIR.", "      EMMENE TOPHAT A LA\n  POLICE, ET N'OUBLIE PAS LES\n  AUTRES CADAVRES. JE VAIS A\n       L'ANTRE D'OMBRE.", "   LLEVA A LA DIVA A LA\n  POLICIA Y NO OLVIDES LOS\n DOS CADAVERES. YO VOY A\n LA CUEVA SOMBRIA.", "     BRING ZYLINDERCHEN ZUR\n  POLIZEI, UND VERGI\xe1"" DIE ANDEREN\nLEICHEN NICHT. ICH GEHE ZUM\n    SCHATTENHORT ZUR\x9a""CK." },
};

static const struct ComicBlock block_p9_4 = { &poly_p9_b4, -1, -1, block_p9_b4_boxes, 1 };

static const struct ComicBlock page_9_blocks[] = {
	block_p9_0,
	block_p9_1,
	block_p9_2,
	block_p9_3,
	block_p9_4,
};

static struct ComicPage page_9 = { "COMDATA/LEPG17.SCN", 98, 49, 1, 0, page_9_blocks, 5 };

static const struct Polygon poly_p10_b0 = { {
	{ 59, 0 },
	{ 228, 0 },
	{ 222, 115 },
	{ 59, 115 },
} };

static const struct ComicBox block_p10_b0_boxes[] = {
	{ 0, 246, 69, 103, "SHADOWLAIR. LOCATED DEEP UNDERGROUND, THE REFORMED\nTEMPLE IS A REMNANT OF AN ERA PASSED. ONCE THE\nHALLOWED SHRINE OF AN ELITE SECRET ORDER OF\nINTELLIGENTSIA, THE TEMPLE NOW SERVES A SINGLE MAN\nAS THE BASTION OF MIDNIGHT... THE REFUGE OF SHADOW...\nTHE SANCTUARY OF DARKSHEER.", "L'ANTRE D'OMBRE.  ENFOUI DANS LES PROFONDEURS DE\nLA VILLE, LE TEMPLE REFORME TEMOIGNE DES SPLENDEURS\nDU PASSE. AUTREFOIS UTILISE COMME SANCTUAIRE POUR UN\nORDRE SECRET, LE TEMPLE NE SERT PLUS AUJOURD'HUI QU'A\nUN SEUL HOMME, QUI S'EN SERT DE BASTION NOCTURNE...\nLE REFUGE DE L'OMBRE... LE SANCTUAIRE DE DARKSHEER.", "LA CUEVA SOMBRIA. REFUGIO SUBTERRANEO HECHO A\nPARTIR DE UN TEMPLO ANTIGUO. LO QUE UNA VEZ FUE\nLA ELITE DE UNA ORDEN SECRETA DE ESTUDIOSOS,\nAHORA ES EL HOGAR DE UN SOLO HOMBRE, EL AMO DE\nLA NOCHE... EL SANTUARIO DE TENEBROSO.", "DER SCHATTENHORT. DER UNTERIRDISCHE EHEMALIGE\nTEMPEL IST DAS \x9a""BERBLEIBSEL EINER LANGE\nVERGANGENEN \x8e""RA. EINST DIENTE ER ALS HEILIGER\nSCHREIN EINES GEHEIMBUNDES DER GEISTIGEN ELITE.\nHEUTE DIENT ER NUR EINEM EINZIGEN MANN ALS BASTION\nDER MITTERNACHT...ALS HORT DER FINSTERNIS...\nALS ZUFLUCHT VON DARKSHEER." },
	{ 5, 246, 177, 102, "        LIQUIDARK... SO COLD IT\n    BURNS! IT'S ALWAYS LIKE THAT\n WHEN I'M INJURED. HOW MANY TIMES\nHAVE I DONE THIS? BAPTIZED IN THE\nICY WETNESS OF DISTILLED DARKNESS...\n      IT WORKS EVERY TIME...\n         SAVES MY SKIN.", "        LE LIQUIDARK... IL EST SI\n    FROID QU'IL BRULE! C'EST COMME\n \x80""A QUAND JE SUIS BLESSE, EN TOUT\nCAS. COMMENT AI-JE REUSSI? BAPTISE\nDANS L'HUMIDITE GLACIALE DES TENEBRES\n  DISTILLEES... IL FONCTIONNE TOUT\n       LE TEMPS... ET ME SAUVE\n                LA VIE.", "        LIQUIOSCURO... TAN FRIO\n    QUE QUEMA! SIEMPRE PASA CUANDO\n ESTOY HERIDO. CUANTAS VECES HE\nHECHO ESTO? ES MI BAUTISMO EN LA\nHELADA HUMEDAD DE LAS TINIEBLAS...\n   SIEMPRE FUNCIONA...\n   SIEMPRE ME SALVA.", "         DUSTER\x99""L... SO KALT,\n      DA\xe1"" ES BRENNT. SO IST ES\n  IMMER, WENN ICH VERLETZT BIN. WIE\n  OFT HABE ICH DAS SCHON GETAN?\n GETAUFT IN DER EISIGEN FEUCHTIGKEIT\n   DESTILLIERTER DUNKELHEIT...\n         FUNKTIONIERT JEDESMAL...\n              RETTET MICH." },
};

static const struct ComicBlock block_p10_0 = { &poly_p10_b0, -1, -1, block_p10_b0_boxes, 2 };

static const struct Polygon poly_p10_b1 = { {
	{ 59, 115 },
	{ 222, 115 },
	{ 230, 286 },
	{ 59, 286 },
} };

static const struct ComicBox block_p10_b1_boxes[] = {
	{ 7, 246, 153, 252, "        REMEMBERING... HOW IT\n   ALL STARTED. I WAS JUST A PUNK\n ORPHAN. FATHER DESMOND... WHAT A\nSALESMAN! HE HAD US ALL JUMPING TO\n BECOME WARDS... TO HELP CLEAN UP\n   THE STREETS OF NOCTROPOLIS.\n     BIRTH OF A VIGILANTE...", "          JE ME SOUVIENS...\n    COMMENT TOUT A COMMENCE.\n J'ETAIS ORPHELIN. LE PERE DESMOND...\nQUEL VENDEUR! NOUS VOULIONS TOUS\nDEVENIR DES PUPILLES... POUR L'AIDER\n     A NETTOYER LES RUES DE\n     NOCTROPOLIS. LA NAISSANCE\n          D'UN VIGILE....", "        RECUERDO COMO EMPEZO\n  TODO. YO ERA UN PILLUELO DE LAS\n CALLES. EL PADRE DESMOND... QUE\n TIO! NOS TENIA A TODOS SALTANDO\n PARA CONVERTIRNOS EN GUARDIANES Y\n   LIMPIAR LAS CALLES.\n     ASI ME HICE VIGILANTE...", "     ERINNERUNGEN...WIE ALLES\n     BEGANN. ICH WAR NUR EIN\n     STREUNENDER WAISENKNABE." },
	{ 7, 246, 153, 252, nullptr, nullptr, nullptr, "          VATER DESMOND...\n   WAS F\x9a""R EIN VERK\x8e""UFER! BRACHTE\n  UNS ALLE DAZU, NICHTS SEHNLICHER\n     ZU WOLLEN, ALS DER WEHR\nANZUGEH\x99""REN... DIE STRA\xe1""EN AUFR\x8e""UMEN.\n       GEBURT EINER B\x9a""RGERWEHR..." },
};

static const struct ComicBlock block_p10_1 = { &poly_p10_b1, -1, -1, block_p10_b1_boxes, 2 };

static const struct Polygon poly_p10_b2 = { {
	{ 193, 286 },
	{ 326, 286 },
	{ 326, 399 },
	{ 226, 399 },
} };

static const struct ComicBox block_p10_b2_boxes[] = {
	{ 7, 246, 196, 276, "         YEARS LATER, POKING\n  AROUND: SUBTERRAINIA... STUMBLED\nACROSS THE BROTHERHOOD'S TEMPLE.\nMAN, I DIDN'T KNOW WHAT I'D FOUND!\n       ...TOOK IT SLOW...\n        WORKED IN SECRET...", "         DES ANNEES PLUS TARD,\n  EN EXPLORANT SUBTERRAINIA... J'AI\n RETROUVE PAR HASARD LE TEMPLE DE\nLA CONFRERIE. MAIS JE NE SAVAIS PAS\n CE QUE JE VENAIS DE DECOUVRIR! IL\n    M'A FALLU LONGTEMPS... J'AI\n     TRAVAILLE DANS LE PLUS\n             GRAND SECRET....", "    Y LUEGO DESCUBRI A\n  SUBTERRAINIA Y A ESTE\n TEMPLO OLVIDADO.\n AUN NO SABIA LO QUE HABIA\n   HALLADO! PERO \n   TRABAJE EN SECRETO...", "        JAHRE SP\x8e""TER, ALS ICH\n   IN SUBTERRAINIA HERUMST\x99""BERTE...\n  STOLPERTE \x9a""BER DEN TEMPEL DER\n  BRUDERSCHAFT. MANN, ICH WU\xe1""TE\n     JA NICHT MAL, WAS ICH\n      DA GEFUNDEN HATTE! LIE\xe1""\n       MIR ZEIT...ARBEITETE\n           IM GEHEIMEN..." },
};

static const struct ComicBlock block_p10_2 = { &poly_p10_b2, -1, -1, block_p10_b2_boxes, 1 };

static const struct Polygon poly_p10_b3 = { {
	{ 326, 0 },
	{ 491, 0 },
	{ 491, 93 },
	{ 448, 93 },
	{ 448, 81 },
	{ 337, 81 },
	{ 337, 93 },
	{ 326, 93 },
} };

static const struct ComicBox block_p10_b3_boxes[] = {
	{ 6, 246, 195, 85, "        I'VE PROBABLY READ IT A\n   THOUSAND TIMES... THE BIG BOOK...\n     MY BIBLE AND BLUEPRINT: A\n    COMPENDIUM OF KNOWLEDGE AND\n   THEOREMS AS RESEARCHED BY THE\n      BROTHERHOOD OF THE NIGHT.", "\n        J'AI PROBABLEMENT DU LE\n    LIRE UN MILLIER DE FOIS... LE\n   GRAND LIVRE... MA BIBLE... UN\n CONCENTRE DE CONNAISSANCE ET DE\n   THEOREMES TEL QUE LE DICTE\n       LA CONFRERIE DE LA NUIT.", "        LEI EL LIBRO MAS DE\n   MIL VECES... FUE MI BIBLIA...\n     CONTENIA TODOS LOS \n   DESCUBRIMIENTOS DE LA \n     HERMANDAD DE LA NOCHE.", "           ICH MU\xe1"" ES SCHON\n    HUNDERTMAL GELESEN HABEN...\n  DAS GROSSE BUCH... MEINE BIBEL UND\n GEBRAUCHSANWEISUNG - DIE SAMMLUNG\n  ALLEN WISSENS UND DER THEOREME\n       DER BRUDERSCHAFT DER NACHT." },
};

static const struct ComicBlock block_p10_3 = { &poly_p10_b3, -1, -1, block_p10_b3_boxes, 1 };

static const struct Polygon poly_p10_b4 = { {
	{ 491, 0 },
	{ 593, 0 },
	{ 593, 157 },
	{ 530, 143 },
	{ 471, 151 },
	{ 471, 93 },
	{ 491, 93 },
} };

static const struct ComicBox block_p10_b4_boxes[] = {
	{ 6, 246, 330, 88, "        THE BOOK TAUGHT ME\n   HOW TO DISTILL LIQUIDARK. I\n PUT WHAT I LEARNED TO GOOD USE...\nSPENT COUNTLESS HOURS IN THAT DAMN\n LAB... UNTIL I FOUND A WAY TO BIND\n  THE LIQUIDARK TO FABRIC AND\n           BODY ARMOR.", "        LE LIVRE M'A APPRIS\n   A DISTILLER LE LIQUIDARK. J'AI\n MIS DE MON MIEUX CES CONNAISSANCES\nEN PRATIQUE... DES HEURES INFINIES\n DANS CE LABO AVANT DE... TROUVER\n  UN MOYEN DE LIER LE LIQUIDARK\n    ET LE TISSU DE MON COSTUME.", "        EN EL LIBRO VI COMO\n   DESTILAR EL LIQUIOSCIRO. LE\n DI UN BUEN USO... PASE MUCHAS\nHORAS EN EL LABORATORIO HASTA QUE\n  PUDE CONSTRUIR CON EL MI\n           ARMADURA.", "        DAS BUCH LEHRTE MICH,\n     DUSTER\x99""L ZU DESTILLIEREN.\n   ICH WANDTE MEIN WISSEN GUT AN,\n   VERBRACHTE ENDLOSE STUNDEN IN\n  DIESEM VERDAMMTEN LABOR, BIS ICH\n      EINEN WEG FAND, DUSTER\x99""L\n        AN GEWEBE ZU BINDEN." },
};

static const struct ComicBlock block_p10_4 = { &poly_p10_b4, -1, -1, block_p10_b4_boxes, 1 };

static const struct Polygon poly_p10_b5 = { {
	{ 326, 93 },
	{ 337, 93 },
	{ 337, 81 },
	{ 448, 81 },
	{ 448, 93 },
	{ 471, 93 },
	{ 471, 151 },
	{ 438, 173 },
	{ 426, 200 },
	{ 326, 200 },
} };

static const struct ComicBox block_p10_b5_boxes[] = {
	{ 5, 246, 439, 161, "         THE NOCTROGLYPH...\n   IT TOOK WEEKS JUST TO FIGURE\n OUT WHAT THE HELL IT WAS, LET\nALONE HOW TO WORK IT! POWERFUL...\n  SAVED MY BUTT MORE THAN A\n           FEW TIMES...", "\n         LE NOCTROGLYPHE...\n   IL M'A FALLU DES SEMAINES POUR\n COMPRENDRE DE QUOI IL S'AGISSAIT,\nSANS MEME PARLER DE L'UTILISER! MAIS\n    IL M'A SAUVE LA VIE PLUS\n          D'UNE FOIS...", "     EL NOCTROGLIFO...\n   ME LLEVO SEMANAS DESCUBRIR\n LO QUE ERA Y HACERLO FUNCIONAR.\n ES MUY PODEROSO... ME HA SACADO\n DE APUROS MAS DE UNA VEZ...", "         DER NOCTROGLYPH...\n    ICH HABE WOCHEN GEBRAUCHT,\n  NUR UM HERAUSZUFINDEN, WAS DAS\n IST, GAR NICHT DAVON ZU REDEN, WIE\n  ER FUNKTIONIERT. M\x8e""CHTIG....HAT\n     MEINEN HINTERN MEHR ALS\n    NUR EIN PAARMAL GERETTET." },
};

static const struct ComicBlock block_p10_5 = { &poly_p10_b5, -1, -1, block_p10_b5_boxes, 1 };

static const struct Polygon poly_p10_b6 = { {
	{ 426, 200 },
	{ 438, 173 },
	{ 471, 151 },
	{ 530, 143 },
	{ 593, 157 },
	{ 593, 245 },
	{ 593, 233 },
	{ 491, 245 },
	{ 443, 233 },
} };

static const struct ComicBox block_p10_b6_boxes[] = {
	{ 6, 246, 279, 203, "        THOUGHT I WAS READY...\n  GONE AND FORGED MYSELF INTO\nONE BAD-ASS HERO! THEN I MET HER\n IN A BLIND ALLEY ONE NIGHT...\n  THOUGHT SHE WAS SOME PUNK THIEF...\n           STILETTO...", "        JE CROYAIS ETRE PRET...\n   JE ME SUIS FORGE UNE IMAGE DE\n HEROS PEU COMMODE! ET PUIS UN\nJOUR, JE L'AI RENCONTREE DANS UNE\n    ALLEE SOMBRE... JE CROYAIS\n     QUE C'ETAIT UNE VOLEUSE...\n            STILETTO....", "    CREI ESTAR LISTO...\n  YA ME HABIA FRAGUADO COMO\n SUPER-HEROE! Y ENTONCES LA\n ENCONTRE A ELLA, A\n ESA VAGABUNDA...\n           STILETTO...", "        DACHTE, ICH W\x8e""RE BEREIT...\n    MACHTE AUS MIR SELBST EINEN\n   VERDAMMTEN SUPERHELDEN. UND\n DANN, EINES ABENDS IN EINER DUNKLEN\n    SEITENSTRA\xe1""E, TRAF ICH SIE...\n     DACHTE, SIE W\x8e""RE NUR\n     EINE DIEBIN...STILETTO..." },
};

static const struct ComicBlock block_p10_6 = { &poly_p10_b6, -1, -1, block_p10_b6_boxes, 1 };

static const struct Polygon poly_p10_b7 = { {
	{ 326, 200 },
	{ 426, 200 },
	{ 426, 312 },
	{ 395, 373 },
	{ 326, 399 },
} };

static const struct ComicBox block_p10_b7_boxes[] = {
	{ 8, 246, 156, 213, "        SHE LAID ME OUT FLAT!\n   BEFORE I KNEW WHAT'D HAPPENED,\nSHE HAD A BLADE AT MY THROAT... SHE\n LIKED THE COSTUME THOUGH. GOOD\n THING TOO, OR SHE'D PROBABLY HAVE\n      GUTTED ME RIGHT THERE.", "        ELLE M'A EU! AVANT QUE\n   JE PUISSE COMPRENDRE CE QUI SE\n PASSAIT, ELLE AVAIT DEJA POSE SA\n LAME SUR MA GORGE! ELLE AIMAIT\n  MON COSTUME.. HEUREUSEMENT,\n  SINON ELLE M'AURAIT EGORGE\n             SUR PLACE.", "     ME HIZO CAER Y, ANTES\n DE SABER LO QUE OCURRIA,\n APOYO UN CUCHILLO EN MI GARGANTA.\n QUERIA ROBARME EL DISFRAZ.\n  LA CONVENCI PARA HABLAR.", "         SIE LEGTE MICH AUFS\n    KREUZ! BEVOR ICH WU\xe1""TE, WAS\n MIR GESCHAH, HATTE ICH IHR MESSER\nAN DER KEHLE! ABER DER ANZUG GEFIEL\n IHR; WAS F\x9a""R EIN GL\x9a""CK, SONST H\x8e""TTE\n    SIE MICH WOHL AN ORT UND\n         STELLE AUSGEWEIDET." },
};

static const struct ComicBlock block_p10_7 = { &poly_p10_b7, -1, -1, block_p10_b7_boxes, 1 };

static const struct Polygon poly_p10_b8 = { {
	{ 326, 399 },
	{ 395, 373 },
	{ 426, 233 },
	{ 492, 233 },
	{ 494, 312 },
	{ 517, 379 },
	{ 593, 399 },
} };

static const struct ComicBox block_p10_b8_boxes[] = {
	{ 8, 246, 240, 282, "        I TOOK HER BACK TO\n  SHADOWLAIR WHERE SHE AGREED TO\n BECOME MY PARTNER. WE PROWLED\nTHE STREETS TOGETHER. UNARMED, SHE'S\n VICIOUS, BUT WITH THOSE BLADES,\n   SHE'S DOWNRIGHT LETHAL! WE\n    BECAME A FORMIDABLE TEAM...\n       DARKSHEER AND STILETTO.", "         JE L'AI RAMENEE A\n    L'ANTRE D'OMBRE, OU ELLE\n ACCEPTA DE DEVENIR MA PARTENAIRE.\n   NOUS AVONS ECUME LES RUES\nENSEMBLE... SANS ARME, ELLE EST\n   RUSEE, MAIS AVEC SES COUTEAUX,\n         ELLE EST MORTELLE!", "        LA TRAJE A LA CUEVA\n  SOMBRIA Y ACEPTO SER MI SOCIA.\n JUNTOS RECORRIMOS LAS CALLES.\n ES MORTAL CON SUS CUCHILLOS.\n ERAMOS UN GRAN EQUIPO...\n       TENEBROSO Y STILETTO.", "      ICH NAHM SIE MIT ZUR\x9a""CK\n  ZUM SCHATTENHORT, UND SIE WILLIGTE\n  EIN, MEINE PARTNERIN ZU WERDEN." },
	{ 8, 246, 240, 282, nullptr, "          NOUS FORMIONS\n        UNE EQUIPE UNIQUE...\n        DARKSHEER ET STILETTO.", nullptr, "            WIR TRIEBEN UNS\n    ZUSAMMEN RUM. UNBEWAFFNET\n  IST SIE SCHON GEF\x8e""HRLICH, ABER\n MIT DIESEN KLINGEN IST SIE EINFACH\n    T\x99""DLICH! WIR WURDEN EIN\n       UNSCHLAGBARES TEAM...\n       DARKSHEER UND STILETTO." },
};

static const struct ComicBlock block_p10_8 = { &poly_p10_b8, -1, -1, block_p10_b8_boxes, 2 };

static const struct Polygon poly_p10_b9 = { {
	{ 491, 245 },
	{ 593, 245 },
	{ 593, 399 },
	{ 517, 379 },
	{ 494, 312 },
} };

static const struct ComicBox block_p10_b9_boxes[] = {
	{ 8, 246, 305, 212, "        WE'VE BEEN TOGETHER\n   EVER SINCE... WELL, EXCEPT\n LATELY... I GUESS SHE HAS A RIGHT\nTO BE TICKED THAT I'VE BEEN WORKING\n ALONE. THE VENOM'S GONE... AND THE\n   BULLET SCORE'S HEALED... LIKE I\n    SAID, IT WORKS EVERY TIME.", "\n       NOUS SOMMES ENSEMBLE\n   DEPUIS CE JOUR-LA.... EN FAIT,\nUN PEU MOINS DEPUIS QUELQUE TEMPS.\nJE PENSE QU'ELLE A LE DROIT\n  D'ETRE VEXEE PARCE QUE J'AI\n         TRAVAILLE SEUL.", "      DESDE ENTONCES\n   ESTAMOS JUNTOS... HASTA HACE\n POCO... SUPONGO QUE TIENE RAZON\n AL ENFADARSE PORQUE YO TRABAJE\n SOLO. BIEN. EL VENENO SE HA IDO Y\n  LA HERIDA DE BALA ESTA CURADA...\n       SIEMPRE FUNCIONA.", "         SEITDEM SIND WIR\n    ZUSAMMEN...NAJA, BIS AUF DIE\n LETZTE ZEIT. SCH\x8e""TZE, SIE HAT DAS\n RECHT, ETWAS SAUER ZU SEIN, DA\xe1"" ICH\nALLEIN GEARBEITET HABE. DAS GIFT IST\n   FORT...DIE WUNDE VERHEILT...WIE\n    GESAGT, FUNKTIONIERT\n             JEDESMAL." },
	{ 8, 246, 305, 212, nullptr, "       LE VENIN EST PARTI... \n  ET LES BLESSURES DES BALLES\nONT ETE GUERIES. COMME JE VOUS\n    L'AI DIT, \x80""A MARCHE A TOUS\n           LES COUPS.", nullptr, nullptr },
};

static const struct ComicBlock block_p10_9 = { &poly_p10_b9, -1, -1, block_p10_b9_boxes, 2 };

static const struct ComicBlock page_10_blocks[] = {
	block_p10_0,
	block_p10_1,
	block_p10_2,
	block_p10_3,
	block_p10_4,
	block_p10_5,
	block_p10_6,
	block_p10_7,
	block_p10_8,
	block_p10_9,
};

static struct ComicPage page_10 = { "COMDATA/LEPG1819.SCN", 98, 50, 1, 0, page_10_blocks, 10 };

static const struct Polygon poly_p11_b0 = { {
	{ 232, 82 },
	{ 323, 83 },
	{ 323, 165 },
	{ 239, 165 },
} };

static const struct ComicBox block_p11_b0_boxes[] = {
	{ 7, 246, 332, 32, "         THEY COINED ME THEIR\n   CHAMPION...THE PRESS! JUST A\n     LOT OF GLAMOUR AND WINDOW\n DRESSING. OH, THEY WERE USUALLY\nLOYAL, BUT THERE WERE TIMES THEY\n       TURNED ON ME OVERNIGHT!", "\n        IL M'ONT CONSACRE COMME\n   LEUR CHAMPION... LA PRESSE! EN\nFAIT, BEAUCOUP DE POUDRE AUX YEUX...\nOH, ILS ETAIENT LOYALS, MAIS IL M'ONT\n      PARFOIS ABANDONNE SANS\n           EXPLICATION!", "         ME HICIERON SU\n  CAMPEON LA PRENSA! YO\n  ERA EL MAS FAMOSO DE LA\n CIUDAD. Y POR LO GENERAL ERAN\n LEALES CONMIGO, PERO TAMBIEN\n         ES QUE UNO SE CANSA!", "       SIE HABEN MICH ZU IHREM\n    HELDEN GEMACHT....DIE PRESSE!\n  JEDE MENGE GLAMOUR UND SHOW.\n    NAJA, MEISTENS HABEN SIE MICH\nUNTERST\x9a""TZT, ABER ES GAB AUCH ZEITEN,\n     DA HABEN SIE SICH \x9a""BER NACHT\n        GEGEN MICH GEWANDT!" },
};

static const struct ComicBlock block_p11_0 = { &poly_p11_b0, -1, -1, block_p11_b0_boxes, 1 };

static const struct Polygon poly_p11_b1 = { {
	{ 61, 0 },
	{ 323, 0 },
	{ 322, 234 },
	{ 62, 234 },
} };

static const struct ComicBox block_p11_b1_boxes[] = {
	{ 7, 246, 166, 33, "         I'M STILL NOT SURE\n   I'M DOING THE RIGHT THING...\n THIS CITY HAS COUNTED ON ME TO\nALWAYS BE THERE... LURKING IN THE\n       EVER-PRESENT SHADOWS.", "        JE NE SAIS PAS ENCORE\n   SI J'AI PRIS LA BONNE DECISION...\nCETTE VILLE A TOUJOURS COMPTE SUR\n    MOI... RODANT DANS L'OMBRE\n          OMNIPRESENTE.", "         NO ESTOY SEGURO DE\n ESTAR HACIENDO LO CORRECTO...\n ESTA CIUDAD HA CONTADO CONMIGO\n DESDE QUE ME CONVERTI EN\n       TENEBROSO.", "        ICH BIN MIR IMMER NOCH\n       NICHT SICHER, OB ICH DAS\n RICHTIGE TUE. DIE STADT HAT SICH\nDARAUF VERLASSEN, DA\xe1"" ICH IMMER F\x9a""R\n    SIE DA BIN.. IN DEN SCHATTEN,\n          DIE \x9a""BERALL SIND." },
};

static const struct ComicBlock block_p11_1 = { &poly_p11_b1, -1, -1, block_p11_b1_boxes, 1 };

static const struct Polygon poly_p11_b2 = { {
	{ 322, 234 },
	{ 266, 257 },
	{ 242, 320 },
	{ 261, 369 },
	{ 322, 399 },
} };

static const struct ComicBox block_p11_b2_boxes[] = {
	{ 8, 246, 84, 219, "       ...I DESERVE MORE!", "\n   ... JE MERITE MIEUX QUE \x80""A!", "       ...MEREZCO MAS!", "\n       ... ICH VERDIENE MEHR!" },
};

static const struct ComicBlock block_p11_2 = { &poly_p11_b2, -1, -1, block_p11_b2_boxes, 1 };

static const struct Polygon poly_p11_b3 = { {
	{ 62, 235 },
	{ 322, 234 },
	{ 322, 399 },
	{ 62, 399 },
} };

static const struct ComicBox block_p11_b3_boxes[] = {
	{ 5, 246, 181, 280, "         I'VE WORKED HARD TO\n   BECOME WHAT I AM, BUT I'M SO\n DAMN TIRED! FIGHTING OFF EVERY\nCRACKPOT AND PSYCHO THAT WANTS TO\n   USE NOCTROPOLIS AS THEIR\n     PERSONAL PLAYGROUND...", "         J'AI TRAVAILLE DUR\n     POUR DEVENIR CE QUE JE SUIS\n AUJOU'D'HUI, MAIS JE SUIS VRAIMENT\nFATIGUE! JE ME SUIS BATTU CONTRE TOUS\n MALFAITEURS ET AUTRES PSYCHOPATHES\n  QUI DECIDAIENT UN JOUR D'ADOPTER\n        NOCTROPOLIS COMME\n          TERRAIN DE JEU...", "         TRABAJE MUCHO PARA\n  CONVERTIRME EN LO QUE SOY,\n PERO TODO TIENE UN FIN. PELEAR\n CONTRA CADA RATERO O SICOPATA\n QUE QUIERE HACER DE NOCTROPOLIS\n        SUS DOMINIOS...", "         ICH HABE HART\n   GEARBEITET, UM DAS ZU WERDEN,\n       WAS ICH HEUTE BIN." },
	{ 5, 246, 181, 280, nullptr, nullptr, nullptr, "         ABER ICH BIN ES SO\n    VERDAMMT LEID, GEGEN JEDEN\n  VERR\x9a""CKTEN UND PSYCHOPATHEN ZU\n   K\x8e""MPFEN, DER NOCTROPOLIS ALS\n   SEINEN PERS\x99""NLICHEN SPIELPLATZ\n             BETRACHTET..." },
};

static const struct ComicBlock block_p11_3 = { &poly_p11_b3, -1, -1, block_p11_b3_boxes, 2 };

static const struct Polygon poly_p11_b4 = { {
	{ 323, 0 },
	{ 471, 0 },
	{ 324, 175 },
} };

static const struct ComicBox block_p11_b4_boxes[] = {
	{ 0, 246, 349, 61, "NOCTROPOLIS CITY HALL", "MAIRIE DE NOCTROPOLIS", "AYUNTAMIENTO DE NOCTROPOLIS", "RATHAUS VON NOCTROPOLIS" },
};

static const struct ComicBlock block_p11_4 = { &poly_p11_b4, -1, -1, block_p11_b4_boxes, 1 };

static const struct Polygon poly_p11_b5 = { {
	{ 471, 0 },
	{ 586, 0 },
	{ 586, 170 },
} };

static const struct ComicBox block_p11_b5_boxes[] = {
	{ 0, 246, 349, 61, "NOCTROPOLIS CITY HALL", "MAIRIE DE NOCTROPOLIS", "AYUNTAMIENTO DE NOCTROPOLIS", "RATHAUS VON NOCTROPOLIS" },
};

static const struct ComicBlock block_p11_5 = { &poly_p11_b5, -1, -1, block_p11_b5_boxes, 1 };

static const struct Polygon poly_p11_b6 = { {
	{ 376, 111 },
	{ 534, 111 },
	{ 534, 187 },
	{ 376, 187 },
} };

static const struct ComicBox block_p11_b6_boxes[] = {
	{ 4, 255, 235, 51, "        AHH DARKSHEER, WE WERE\n   JUST DISCUSSING YOU... YOUR\n RECENT CAMPAIGN AGAINST THE\nCITY'S MORE NOTORIOUS ENEMIES\n   HAS BEEN MOST IMPRESSIVE!\n            WELL DONE!", "\n        AHH... DARKSHEER! NOUS\n   PARLIONS JUSTEMENT DE VOUS...\nVOTRE DERNIERE CAMPAGNE CONTRE LES\n ENNEMIS NOTOIRES DE LA VILLE ETAIT\n     VRAIMENT IMPRESSIONNANTE!\n            BIEN JOUE! ", "        AH, TENEBROSO.\n  HABLABAMOS DE TI... TU\n RECIENTE CRUZADA CONTRA LOS\n ENEMIGOS MAS NOTABLES DE\n LA CIUDAD HA SIDO IMPRESIONANTE!\n          BIEN HECHO!", "        AAH, DARKSHEER, WIR\n   REDEN GERADE \x9a""BER SIE. IHRE\n J\x9a""NGSTE KAMPAGNE GEGEN DIE \x9a""BLEN\n  FEINDE DER STADT WAR \x8e""U\xe1""ERST\n       EINDRUCKSVOLL. SEHR GUT\n              GEMACHT!" },
	{ 4, 246, 247, 71, "        THANK YOU MR. MAYOR.\n   YOU'LL BE PLEASED TO KNOW\n THAT TOPHAT IS IN CUSTODY AND\nTHAT DESPERADO AND WIDOWMARK WON'T\n     TROUBLE THE CITIZENS OF\n      NOCTROPOLIS EVER AGAIN.", "\n       MERCI, MONSIEUR LE MAIRE.\n   VOUS SEREZ HEUREUX D'APPRENDRE\n QUE TOPHAT EST EN PRISON, ET QUE\nDESPERADO ET WIDOWMARK N'ENNUIERONT\n    PLUS JAMAIS LES HABITANTS\n         DE NOCTROPOLIS.", "        GRACIAS, ALCALDE.\n  LE GUSTARA SABER QUE LA\n DIVA ESTA EN PRISION Y QUE\n DESESPERADO Y VIUDA NEGRA NO\n   VOLVERAN A MOLESTAR A LOS\n   CIUDADANOS DE NOCTROPOLIS.", "         VIELEN DANK, HERR\n     B\x9a""RGERMEISTER. ES WIRD SIE\n     FREUEN, ZU H\x99""REN, DA\xe1"" MISS\n  ZYLINDERCHEN IN HAFT GENOMMEN\n WURDE, UND DA\xe1"" SHOWDOWN UND DIE\n     SCHWARZE WITWE DIE B\x9a""RGER VON\n     NOCTROPOLIS NIE WIEDER\n         BEL\x8e""STIGEN WERDEN." },
};

static const struct ComicBlock block_p11_6 = { &poly_p11_b6, -1, -1, block_p11_b6_boxes, 2 };

static const struct Polygon poly_p11_b7 = { {
	{ 323, 261 },
	{ 378, 270 },
	{ 396, 292 },
	{ 396, 336 },
	{ 367, 366 },
	{ 323, 368 },
} };

static const struct ComicBox block_p11_b7_boxes[] = {
	{ 3, 255, 431, 210, "         R-RETIRE? BUT YOU\n   CAN'T! WE'RE JUST NOT CAPABLE\nOF DEALING WITH THE MISCREANTS THAT\nPLAGUE THE CITY... FOR THE LOVE OF\n  GOD, THINK OF MY PUBLIC IMAGE!\n    THERE'S AN ELECTION COMING...", "\n       P-PRENDRE VOTRE RETRAITE?\n   MAIS C'EST IMPOSSIBLE! NOUS NE\nPOURRONS PAS LUTTER SEUL CONTRE\nLES MALFAITEURS QUI RUINENT NOTRE\n CITE! POUR L'AMOUR DU CIEL, PENSEZ\n      A MON IMAGE! IL Y A DES\n         ELECTIONS BIENTOT...", "         RETIRARTE? NO\n   PUEDES! NOSOTROS NO SOMOS\n CAPACES DE HACER FRENTE A LOS\n DESALMADOS QUE ASEDIAN ESTA\n   CIUDAD. POR FAVOR, ESPERA\n       A LAS ELECCIONES!", "         Z-Z-ZUR RUHE SETZEN?\n   ABER DAS GEHT NICHT! WIR WERDEN\n DOCH ALLEINE NICHT MIT ALL DIESEN\n\x9a""BELT\x8e""TERN FERTIG, DIE UNSERE STADT\n   PLAGEN! UM GOTTES WILLEN,\n   DENKEN SIE DOCH AM MEIN IMAGE...\n      DIE WAHLEN STEHEN VOR\n             DER T\x9a""R!" },
};

static const struct ComicBlock block_p11_7 = { &poly_p11_b7, -1, -1, block_p11_b7_boxes, 1 };

static const struct Polygon poly_p11_b8 = { {
	{ 507, 213 },
	{ 535, 226 },
	{ 544, 250 },
	{ 535, 277 },
	{ 506, 292 },
	{ 475, 277 },
	{ 466, 255 },
	{ 475, 226 },
} };

static const struct ComicBox block_p11_b8_boxes[] = {
	{ 4, 246, 266, 169, "       ACTUALLY MR MAYOR,\n   THAT'S WHY I CAME TO SEE YOU...\n I HAVE DECIDED TO RETIRE. AS A\n COURTESY, I THOUGHT YOU SHOULD\n   KNOW BEFORE ANY PUBLIC\n        ANNOUNCEMENT.", "        EN FAIT, MONSIEUR LE\n    MAIRE, C'EST POUR \x80""A QUE JE\n SUIS VENU VOUS VOIR. J'AI DECIDE DE\nPRENDRE MA RETRAITE. JE PENSAIS QU'IL\n  ETAIT PLUS CONVENABLE DE VOUS\n     PREVENIR PERSONNELLEMENT\n       AVANT DE L'ANNONCER\n         OFFICIELLEMENT.", "      EN REALIDAD POR ESO\n   VENIA A VERLE, ALCALDE...\n HE DECIDIDO RETIRARME. ME\n PARECIO QUE DEBIA DECIRSELO A\n    USTED ANTES QUE A\n        NADIE.", "          EIGENTLICH BIN ICH\n      GENAU DESWEGEN HIER, HERR\n       B\x9a""RGERMEISTER. ICH HABE\n  BESCHLOSSEN, MICH ZUR RUHE ZU\n  SETZEN. ICH DACHTE, SIE SOLLTEN ES\n       ERFAHREN, BEVOR ICH ES\n        \x99""FFENTLICH BEKANNTGEBE." },
};

static const struct ComicBlock block_p11_8 = { &poly_p11_b8, -1, -1, block_p11_b8_boxes, 1 };

static const struct Polygon poly_p11_b9 = { {
	{ 324, 175 },
	{ 585, 170 },
	{ 585, 267 },
	{ 323, 267 },
} };

static const struct ComicBox block_p11_b9_boxes[] = {
	{ 4, 255, 224, 135, "      EXCELLENT! ACCORDING TO\n  OUR LISTS, THAT ACCOUNTS FOR\nEVERY CRIMINAL MASTERMIND TO DATE.\n  YOU'VE CERTAINLY EARNED A REST!", "      EXCELLENT! D'APRES NOS\n  LISTES, TOUS LES CRIMINELS SONT\nMAINTENANT SOUS LES VERROUS.\n VOUS AVEZ BIEN GAGNE UN PEU\n           DE REPOS!", "      EXCELENTE! SEGUN NUESTRA\n  LISTA, ESO DEJA LA CIUDAD LIMPIA\n DE CRIMINALES IMPORTANTES.\n  TE HAS GANADO UN DESCANSO!", "       AUSGEZEICHNET! NACH\n  UNSERER LISTE W\x8e""REN DAMIT JETZT\nALLE MEISTERVERBRECHER ABGEHAKT.\n    SIE HABEN SICH WIRKLICH EINE\n           PAUSE VERDIENT!" },
};

static const struct ComicBlock block_p11_9 = { &poly_p11_b9, -1, -1, block_p11_b9_boxes, 1 };

static const struct Polygon poly_p11_b10 = { {
	{ 378, 270 },
	{ 506, 270 },
	{ 506, 324 },
	{ 378, 324 },
} };

static const struct ComicBox block_p11_b10_boxes[] = {
	{ 4, 246, 233, 219, "        I DON'T GIVE A DAMN\n   ABOUT YOUR POPULARITY RATING!\nAND, AS YOU MENTIONED BEFORE,\nNOCTROPOLIS' BOGEY MEN ARE EITHER\n DEAD, OR IN LOCK-UP... MY\n  DECISION IS NOT NEGOTIABLE.", "       JE N'AI QUE FAIRE DE\n   VOTRE POPULARITE! ET, COMME\n JE VOUS L'AI DIT LES ENNEMIS DE\nNOCTROPOLIS SONT TOUS MORTS OU EN\n    PRISON... MA DECISION EST\n          IRREVOCABLE.", "         NO ME IMPORTA TU\n   IMAGEN PUBLICA! Y TODOS LOS\n CRIMINALES IMPORTANTES ESTAN\n MUERTOS O ENCERRADOS. MI\n  DECISION NO ES NEGOCIABLE.", "       IHRE BELIEBTHEIT K\x9a""MMERT\n    MICH EINEN DRECK! UND WIE SIE\n   BEREITS SO RICHTIG BEMERKTEN,\n SIND DIE B\x99""SEWICHTE ALLE ENTWEDER\n  TOT ODER IM GEF\x8e""NGNIS. MEINE\n   ENTSCHEIDUNG IST UNUMST\x99""\xe1""LICH." },
};

static const struct ComicBlock block_p11_10 = { &poly_p11_b10, -1, -1, block_p11_b10_boxes, 1 };

static const struct Polygon poly_p11_b11 = { {
	{ 323, 324 },
	{ 506, 324 },
	{ 506, 399 },
	{ 323, 399 },
} };

static const struct ComicBox block_p11_b11_boxes[] = {
	{ 4, 255, 239, 288, "        MR. MAYOR, I PROPOSE\n   THAT WE THROW DARKSHEER A\nHERO'S PARADE! AFTERWARD, YOU\nCOULD GIVE HIM THE KEY TO THE\n  CITY AT A TELEVISED CEREMONY!\n   IT WOULD BE GREAT PUBLICITY...", "       MONSIEUR LE MAIRE, JE\n   PROPOSE D'ORGANISER UN DEFILE\n   POUR NOTRE HEROS, DARKSHEER!\nENSUITE, VOUS POURRIEZ LUI REMETTRE\n LA CLEF DE LA VILLE AU COURS D'UNE\n  EMISSION TELEVISEE! CELA VOUS\n      FERAIT UNE EXCELLENTE\n             PUBLICITE.", "        ALCALDE, PROPONGO\n   QUE LE DESPIDAMOS CON UN \n DESFILE. USTED PODRIA DARLE\n LAS LLAVES DE LA CIUDAD\n   EN UN ACTO TELEVISADO!\n    SERIA UNA GRAN PUBLICIDAD...", "       HERR B\x9a""RGERMEISTER, ICH\n   SCHLAGE VOR, WIR VERANSTALTEN\n     ZU EHREN DARKSHEERS EINE\nHELDENPARADE! ANSCHLIE\xe1""END K\x99""NNTEN\nSIE IHM DIE EHRENSCHL\x9a""SSEL ZUR STADT\n    \x9a""BERREICHEN, LIVE IM FERNSEHEN.\n       DAS W\x8e""RE PRIMA PUBLICITY!" },
};

static const struct ComicBlock block_p11_11 = { &poly_p11_b11, -1, -1, block_p11_b11_boxes, 1 };

static const struct Polygon poly_p11_b12 = { {
	{ 506, 267 },
	{ 585, 267 },
	{ 585, 399 },
	{ 506, 399 },
} };

static const struct ComicBox block_p11_b12_boxes[] = {
	{ 4, 255, 362, 245, "      WELL, DARKSHEER, WHAT\n      DO YOU THINK?", "       ALORS, DARKSHEER,\n  QU'EST-CE QUE VOUS EN PENSEZ?", "      BIEN, TENEBROSO, QUE\n      DICES A ESO?", "      NUN, DARKSHEER, WAS\n      HALTEN SIE DAVON?" },
	{ 2, 246, 332, 314, "      I SUPPOSE THAT WOULD\n      BE ALL RIGHT...", "        JE PENSE QUE \x80""A\n        POURRAIT ALLER...", "      SUPONGO QUE ES UNA\n      BUENA IDEA...", "      ICH SCH\x8e""TZE, DAS GEHT\n       IN ORDNUNG..." },
};

static const struct ComicBlock block_p11_12 = { &poly_p11_b12, -1, -1, block_p11_b12_boxes, 2 };

static const struct ComicBlock page_11_blocks[] = {
	block_p11_0,
	block_p11_1,
	block_p11_2,
	block_p11_3,
	block_p11_4,
	block_p11_5,
	block_p11_6,
	block_p11_7,
	block_p11_8,
	block_p11_9,
	block_p11_10,
	block_p11_11,
	block_p11_12,
};

static struct ComicPage page_11 = { "COMDATA/LEPG2021.SCN", 98, 15, 1, 0, page_11_blocks, 13 };

static const struct Polygon poly_p12_b0 = { {
	{ 186, 112 },
	{ 220, 123 },
	{ 229, 144 },
	{ 219, 167 },
	{ 186, 177 },
	{ 155, 169 },
	{ 144, 144 },
	{ 158, 118 },
} };

static const struct ComicBox block_p12_b0_boxes[] = {
	{ 1, 255, 244, 170, "      FORGIVE ME FATHER, FOR\n      I HAVE SINNED...", "\n    PARDONNEZ-MOI, MON PERE,\n    CAR J'AI PECHE...", "     PERDONEME, PADRE, PORQUE\n      HE PECADO...", "     VERGIB MIR, VATER, DENN\n     ICH HABE GES\x9a""NDIGT..." },
};

static const struct ComicBlock block_p12_0 = { &poly_p12_b0, -1, -1, block_p12_b0_boxes, 1 };

static const struct Polygon poly_p12_b1 = { {
	{ 57, 3 },
	{ 179, 3 },
	{ 182, 140 },
	{ 57, 137 },
} };

static const struct ComicBox block_p12_b1_boxes[] = {
	{ 0, 246, 84, 113, "THE NOCTROPOLIS CITY CATHEDRAL", "CATHEDRALE DE NOCTROPOLIS", "LA CATEDRAL DE NOCTROPOLIS", "DER DOM VON NOCTROPOLIS" },
	{ 5, 255, 168, 93, "      I DON'T KNOW WHAT ELSE\n   TO DO... WHO TO TALK TO. I\n    WONDER IF DESMOND EVEN\n          KNOWS YET.", "       JE NE SAIS PAS QUOI\n   FAIRE... NI A QUI PARLER. JE\n ME DEMANDE SI LE PERE DESMOND\n         EST AU COURANT...", "      NO SE QUE HACER NI A\n   QUIEN HABLAR. ME PREGUNTO\n      SI DESMOND LO\n         SABRA.", "      ICH WEI\xe1"" NICHT MEHR,\n  WAS ICH NOCH TUN KANN....MIT WEM\n ICH NOCH REDEN KANN. OB ES VATER\n  DESMOND \x9a""BERHAUPT SCHON WEI\xe1""?" },
};

static const struct ComicBlock block_p12_1 = { &poly_p12_b1, -1, -1, block_p12_b1_boxes, 2 };

static const struct Polygon poly_p12_b2 = { {
	{ 193, 3 },
	{ 316, 3 },
	{ 316, 137 },
	{ 190, 140 },
} };

static const struct ComicBox block_p12_b2_boxes[] = {
	{ 7, 255, 325, 32, "      I HATE COMING HERE. THE\n CONFESSIONAL ALWAYS MAKES ME\n  FEEL SO... GUILTY. I HATE\n          COMING HERE.", "\n     J'AI HORREUR DE VENIR ICI.\n JE ME SENS TOUJOURS... COUPABLE\n  DANS UN CONFESSIONNAL. JE HAIS\n            CET ENDROIT.", "      ODIO VENIR AQUI. EL\n CONFESIONARIO SIEMPRE ME\n  HACE SENTIR... CULPABLE.", "\n       ICH HASSE ES HIER. BEI\n DER BEICHTE F\x9a""HLE ICH MICH IMMER\n    SO...SCHULDIG. ICH HASSE\n               ES HIER." },
};

static const struct ComicBlock block_p12_2 = { &poly_p12_b2, -1, -1, block_p12_b2_boxes, 1 };

static const struct Polygon poly_p12_b3 = { {
	{ 57, 152 },
	{ 181, 149 },
	{ 177, 396 },
	{ 57, 396 },
} };

static const struct ComicBox block_p12_b3_boxes[] = {
	{ 3, 255, 201, 181, "      STILETTO, HAVE YOU SEEN\n     THE PAPER? IT READS:\n    \"DARKSHEER TO RETIRE\"!\n NOW, WHAT THE HELL IS GOING ON?", "      STILETTO, TU AS LU LES\n    JOURNAUX? \"DARKSHEER PREND\n    SA RETRAITE!\" MAIS QUE SE\n            PASSE-T-IL?", "      STILETTO, HAS LEIDO\n     LA PRENSA? DICE QUE\n    TENEBROSO SE RETIRA!\n QUE DEMONIOS ESTA PASANDO?", "      STILETTO, HAST DU DIE\n     ZEITUNG GELESEN? DA STEHT\n    \"DARKSHEER IN PENSION\"!\n  WAS ZUR H\x99""LLE IST DA LOS?" },
	{ 3, 255, 132, 160, "       I WISH I KNEW, FATHER.\n  IT'S NOT THE SAME BETWEEN US\nANYMORE... WE HARDLY EVEN SPEAK\n            ANYMORE.", "       J'AIMERAIS LE SAVOIR,\n  PERE. CE N'EST PLUS LA MEME\nCHOSE ENTRE NOUS DEPUIS QUELQUE\n             TEMPS...", "       ESO QUERRIA SABER YO.\nTODO HA CAMBIADO ENTRE NOSOTROS.\n     YA NO HABLAMOS SIN PELEAR.", "       ICH W\x9a""NSCHTE, ICH W\x9a""\xe1""TE\n  ES, VATER. ES WAR IN LETZTER\n ZEIT ZWISCHEN UNS NICHT MEHR SO\n   WIE FR\x9a""HER...WIR REDEN KAUM\n          NOCH MITEINANDER." },
};

static const struct ComicBlock block_p12_3 = { &poly_p12_b3, -1, -1, block_p12_b3_boxes, 2 };

static const struct Polygon poly_p12_b4 = { {
	{ 191, 149 },
	{ 316, 152 },
	{ 316, 396 },
	{ 196, 396 },
} };

static const struct ComicBox block_p12_b4_boxes[] = {
	{ 1, 255, 325, 262, "        I HAD TO READ IT IN THE\n   PAPER, TOO. THAT SON OF A - HE\n DOESN'T GIVE A DAMN ABOUT THINGS...\n  ABOUT ME!  HE'S JUST GOING TO\n   THROW IT ALL AWAY WITHOUT\n          LOOKING BACK.", "       JE L'AI APPRIS MOI AUSSI\n  DANS LE JOURNAL. CE FILS DE P...,\n IL SE FICHE PAS MAL DES AUTRES...\n ET DE MOI! IL VA SIMPLEMENT TOUT\n      ABANDONNER SANS REGRET.", "        ME HE ENTERADO POR\n   LA PRENSA. ESE HIJO DE... NO\n  FUE CAPAZ DE DECIRMELO!\n   NOS HA DADO A TODOS LA\n      ESPALDA.", "\n        SELBST ICH MU\xe1""TE ES AUS\n   DER ZEITUNG ERFAHREN. DIESER\n VERDAMMTE---ER K\x9a""MMERT SICH EINEN\nDRECK UM ALLES...UM MICH! ER SCHMEI\xe1""T\n   EINFACH ALLES WEG, OHNE SICH\n         AUCH NUR UMZUDREHEN." },
	{ 1, 255, 189, 258, "      EASY STILETTO, THERE\n  MUST BE AN EXPLANATION...", "     DU CALME, STILETTO, IL\n DOIT BIEN Y AVOIR UNE\n       EXPLICATION!", "      CALMA, STILETTO, DEBE\n DE HABER UNA EXPLICACION...", "      BERUHIGE DICH, STILETTO,\n  ES MU\xe1"" EINE VERN\x9a""NFTIGE\n    ERKL\x8e""RUNG DAF\x9a""R GEBEN." },
};

static const struct ComicBlock block_p12_4 = { &poly_p12_b4, -1, -1, block_p12_b4_boxes, 2 };

static const struct Polygon poly_p12_b5 = { {
	{ 448, 140 },
	{ 482, 153 },
	{ 498, 186 },
	{ 485, 219 },
	{ 448, 235 },
	{ 409, 216 },
	{ 399, 183 },
	{ 404, 154 },
} };

static const struct ComicBox block_p12_b5_boxes[] = {
	{ 2, 255, 201, 219, "       THERE, THERE, STILETTO,\n    OF COURSE GOD HEARS YOUR\nPRAYERS. HE WANTS YOU TO BE HAPPY..\n BUT RIGHT NOW YOU MUST TALK TO\n  DARKSHEER. HE NO LONGER TAKES\n         MY COUNSEL...", "\n      ALLONS, ALLONS, STILETTO,\n    BIEN SUR QUE DIEU ENTEND TES\n   PRIERES... IL VEUT QUE TU SOIS\n HEUREUSE... MAIS POUR L'INSTANT, TU\n    DOIS PARLER A DARKSHEER. IL\n       REFUSE D'ECOUTER MES\n              CONSEILS.", "        TE EQUIVOCAS,\n    DIOS TE ESCUCHA Y DESEA\n QUE SEAS FELIZ...\n HABLALE A TENEBROSO. SIEMPRE\n EL TE HACE MAS CASO QUE A\n         MI...", "\n       KOMM, KOMM, STILETTO,\n   NAT\x9a""RLICH H\x99""RT GOTT DEINE GEBETE.\n  ER WILL, DA\xe1"" DU GL\x9a""CKLICH BIST...\n ABER ERSTMAL MU\xe1""T DU MIT DARKSHEER\n  REDEN. ER H\x99""RT NICHT MEHR AUF\n            MEINEN RAT." },
};

static const struct ComicBlock block_p12_5 = { &poly_p12_b5, -1, -1, block_p12_b5_boxes, 1 };

static const struct Polygon poly_p12_b6 = { {
	{ 317, 0 },
	{ 400, 0 },
	{ 400, 83 },
	{ 317, 83 },
} };

static const struct ComicBox block_p12_b6_boxes[] = {
	{ 1, 255, 406, 57, "      DO YOU BELIEVE GOD'S\n REALLY THERE, FATHER? IS ALL\nTHAT CRAP ABOUT HEARING PRAYERS\n AND GIVING BLESSINGS FOR REAL...?", "      VOUS CROYEZ VRAIMENT\n  QUE DIEU EST LA, PERE? TOUTES\n  CES HISTOIRES DE PRIERES ET DE\n BENEDICTIONS, C'EST DONC VRAI?", "     CREE QUE DIOS ESTA\n AQUI, PADRE? Y QUE ESCUCHA\n NUESTRAS PLEGARIAS?", "\n      GLAUBEN SIE DENN WIRKLICH,\n DA\xe1"" ES EINEN GOTT GIBT, VATER? DA\xe1""\n  AN ALL DIESEM UNFUG MIT GEBETEN\n      UND SEGEN WAS DRAN IST?" },
};

static const struct ComicBlock block_p12_6 = { &poly_p12_b6, -1, -1, block_p12_b6_boxes, 1 };

static const struct Polygon poly_p12_b7 = { {
	{ 496, 0 },
	{ 580, 0 },
	{ 580, 154 },
	{ 496, 154 },
} };

static const struct ComicBox block_p12_b7_boxes[] = {
	{ 6, 255, 306, 79, "      IF THERE IS A GOD, HE\n  DOESN'T HEAR MY PRAYERS...", "      S'IL Y A UN DIEU, IL\n  N'ENTEND PAS MES PRIERES.", "    SI HAY UN DIOS, SE HA\n  OLVIDADO DE MI...", "      WENN ES EINEN GOTT GIBT,\n  DANN H\x99""RT ER MEINE GEBETE\n            NICHT." },
};

static const struct ComicBlock block_p12_7 = { &poly_p12_b7, -1, -1, block_p12_b7_boxes, 1 };

static const struct Polygon poly_p12_b8 = { {
	{ 317, 83 },
	{ 446, 83 },
	{ 446, 292 },
	{ 317, 292 },
} };

static const struct ComicBox block_p12_b8_boxes[] = {
	{ 2, 255, 126, 112, "       YOU DON'T UNDERSTAND,\n FATHER...I LOVE HIM! I LOVE HIM\n AND HE'S READY TO WALK OUT OF\n          MY LIFE!", "       VOUS NE COMPRENEZ PAS,\n MON PERE... JE L'AIME! JE L'AIME\n         ET IL M'ABANDONNE!", "       USTED NO COMPRENDE,\n PADRE... YO LO AMO! Y AHORA VA\n A ESCAPAR DE MI!", "       SIE VERSTEHEN NICHT,\n  VATER...ICH LIEBE IHN! ICH LIEBE\n IHN, UND ER WILL EINFACH SO AUS\n   MEINEM LEBEN VERSCHWINDEN!" },
	{ 1, 255, 431, 102, "      I KNOW YOUR FEELINGS...\n THAT'S WHY YOU HAVE TO BE THE\nONE TO CONVINCE HIM THAT THIS\n DECISION TO RETIRE IS WRONG.", "     JE CONNAIS TES SENTIMENTS...\n C'EST POUR CETTE RAISON QUE TU\nDOIS LE CONVAINCRE QUE SA DECISION\n     DE PARTIR EST UNE ERREUR.", "    SIEMPRE LO HE SABIDO...\n ES POR ESO QUE ERES TU QUIEN\n DEBE CONVENCERLO DE QUE ESTA\n EN UN ERROR AL RETIRARSE.", "      ICH KENNE DEINE GEF\x9a""HLE...\n    DARUM IST ES JA AUCH DEINE\nAUFGABE, IHN ZU \x9a""BERZEUGEN, DA\xe1""\n  SEIN ENTSCHLU\xe1"", SICH ZUR RUHE ZU\n         SETZEN, FALSCH IST." },
};

static const struct ComicBlock block_p12_8 = { &poly_p12_b8, -1, -1, block_p12_b8_boxes, 2 };

static const struct Polygon poly_p12_b9 = { {
	{ 317, 292 },
	{ 446, 292 },
	{ 446, 399 },
	{ 317, 399 },
} };

static const struct ComicBox block_p12_b9_boxes[] = {
	{ 4, 255, 156, 238, "        COME, I'LL WALK YOU\n  OUT. STILETTO, I KNOW IN MY HEART\nTHAT IF YOU CAN PERSUADE DARKSHEER\n  TO STAY, THEN HE'LL RETURN YOUR\n   AFFECTION.  YOU WILL TRY,\n          WON'T YOU?", "\n        VIENS, JE VAIS TE\n  RACCOMPAGNER. STILETTO, JE SAIS,\nAU FOND DE MON COEUR, QUE SI TU\nPEUX PERSUADER DARKSHEER DE RESTER,\n     IL TE RENDRA TON AFFECTION.\n            TU ESSAIERAS? ", "        VE, SE QUE TU PUEDES\n  CONVENCER A TENEBROSO.\n  MUESTRALE TUS SENTIMIENTOS. LO\n  INTENTARAS, VERDAD?", "\n        KOMM, ICH BRINGE DICH\n  ZUR T\x9a""R. STILETTO, ICH WEI\xe1"" GENAU,\nWENN DU ES SCHAFFST, DARKSHEER ZUM\n BLEIBEN ZU \x9a""BERREDEN, WIRD ER DEINE\n  GEF\x9a""HLE ERWIDERN. DU WIRST ES\n      VERSUCHEN, NICHT WAHR?" },
};

static const struct ComicBlock block_p12_9 = { &poly_p12_b9, -1, -1, block_p12_b9_boxes, 1 };

static const struct Polygon poly_p12_b10 = { {
	{ 448, 154 },
	{ 580, 154 },
	{ 580, 399 },
	{ 448, 399 },
} };

static const struct ComicBox block_p12_b10_boxes[] = {
	{ 4, 255, 252, 263, "      I-I'LL TRY, FATHER...", "        JE... J'ESSAIERAI,\n        MON PERE.", "  LO INTENTARE, PADRE...", "         ICH...ICH WERDE ES\n         VERSUCHEN, VATER..." },
};

static const struct ComicBlock block_p12_10 = { &poly_p12_b10, -1, -1, block_p12_b10_boxes, 1 };

static const struct ComicBlock page_12_blocks[] = {
	block_p12_0,
	block_p12_1,
	block_p12_2,
	block_p12_3,
	block_p12_4,
	block_p12_5,
	block_p12_6,
	block_p12_7,
	block_p12_8,
	block_p12_9,
	block_p12_10,
};

static struct ComicPage page_12 = { "COMDATA/LEPG2223.SCN", 98, 51, 1, 0, page_12_blocks, 11 };

static const struct Polygon poly_p13_b0 = { {
	{ 188, 0 },
	{ 270, 0 },
	{ 285, 141 },
	{ 270, 145 },
	{ 188, 91 },
} };

static const struct ComicBox block_p13_b0_boxes[] = {
	{ 2, 246, 13, 86, "      WHAT THE HELL?!", "\n      QUE SE PASSE-T-IL?!", "      QUE DEMONIOS?!", "\n        WAS ZUR H\x99""LLE?!" },
};

static const struct ComicBlock block_p13_0 = { &poly_p13_b0, 99, 25, block_p13_b0_boxes, 1 };

static const struct Polygon poly_p13_b1 = { {
	{ 188, 91 },
	{ 270, 145 },
	{ 240, 156 },
	{ 222, 189 },
	{ 188, 189 },
} };

static const struct ComicBox block_p13_b1_boxes[] = {
	{ 1, 246, 261, 193, "      YOU'D BETTER HAVE A\nDAMN GOOD REASON FOR DOING\n      THAT, STILETTO!", "     J'ESPERE QUE TU AS UNE\n    BONNE RAISON DE FAIRE \x80""A,\n            STILETTO!", "  SUPONGO QUE TENDRAS UNA\n BUENA RAZON PARA HACER\n      ESTO, STILETTO!", "      ICH HOFFE, DU HAST EINEN\n VERDAMMT GUTEN GRUND DAF\x9a""R,\n            STILETTO!" },
};

static const struct ComicBlock block_p13_1 = { &poly_p13_b1, -1, -1, block_p13_b1_boxes, 1 };

static const struct Polygon poly_p13_b2 = { {
	{ 270, 0 },
	{ 369, 0 },
	{ 351, 140 },
	{ 285, 141 },
} };

static const struct ComicBox block_p13_b2_boxes[] = {
	{ 1, 255, 372, 62, "        I THOUGHT YOU MIGHT\n   BE GETTING SOFT...THAT MIGHT\n  EXPLAIN YOUR \"RETIREMENT!\" I\n APPRECIATE YOU TALKING WITH ME\n  BEFORE PUBLICLY DISSOLVING OUR\n          PARTNERSHIP.", "        JE VOYAIS BIEN QUE TU\n   DEVENAIS MOU... CE QUI EXPLIQUE\nTA \"RETRAITE\"!  C'EST GENTIL DE ME\n   PARLER AVANT DE DISSOUDRE EN\n        PUBLIC NOTRE EQUIPE!", "     PENSE QUE TE ESTABAS\n  DEBILITANDO Y QUE ESO EXPLICABA\n  TU \"RETIRO\"! YO HUBIERA\n PREFERIDO QUE HABLARAS CONMIGO\n ANTES DE DISOLVER PUBLICAMENTE\n    NUESTRA SOCIEDAD.", "         ICH DACHTE, DU WIRST\n    VIELLEICHT WEICH. DAS K\x99""NNTE\n  DEINE \"PENSIONIERUNG\" ERKL\x8e""REN.\n DANKE, DA\xe1"" DU ES MIR SO SCHONEND\n BEIGEBRACHT UND \x99""FFENTLICH DAS\n    ENDE UNSERER PARTNERSCHAFT\n        BEKANNTGEGEBEN HAST." },
};

static const struct ComicBlock block_p13_2 = { &poly_p13_b2, -1, -1, block_p13_b2_boxes, 1 };

static const struct Polygon poly_p13_b3 = { {
	{ 369, 0 },
	{ 450, 0 },
	{ 450, 95 },
	{ 360, 66 },
} };

static const struct ComicBox block_p13_b3_boxes[] = {
	{ 2, 246, 191, 75, "      LOOK, STILETTO, I'M\n  SORRY...IF IT'S ANY CONSOLATION,\n THE HARDEST THING WILL BE\n         LEAVING YOU...", "      ECOUTE, STILETTO, JE\n   SUIS DESOLE. SI \x80""A PEUT TE\nCONSOLER, SACHE QUE LE PLUS DUR\n       POUR MOI SERA DE TE\n            QUITTER....", "  LO SIENTO, STILETTO, SI\n ESO TE SIRVE DE CONSUELO, LO\n MAS DURO HA SIDO DEJARTE.", "      H\x99""R MAL, STILETTO, ES\n  TUT MIR LEID. WENN ES DICH\n BERUHIGT: DICH ZU VERLASSEN, WIRD\n   DAS SCHWERSTE DARAN SEIN..." },
};

static const struct ComicBlock block_p13_3 = { &poly_p13_b3, -1, -1, block_p13_b3_boxes, 1 };

static const struct Polygon poly_p13_b4 = { {
	{ 360, 66 },
	{ 450, 95 },
	{ 450, 189 },
	{ 415, 189 },
	{ 395, 159 },
	{ 351, 140 },
} };

static const struct ComicBox block_p13_b4_boxes[] = {
	{ 1, 255, 473, 175, "    DO YOU REALLY MEAN THAT?", "\nTU PENSES VRAIMENT CE QUE TU DIS?", "   LO DICES EN SERIO?", "\n     MEINST DU DAS EHRLICH?" },
};

static const struct ComicBlock block_p13_4 = { &poly_p13_b4, -1, -1, block_p13_b4_boxes, 1 };

static const struct Polygon poly_p13_b5 = { {
	{ 222, 189 },
	{ 240, 156 },
	{ 285, 141 },
	{ 318, 138 },
	{ 318, 202 },
	{ 254, 201 },
	{ 285, 226 },
} };

static const struct ComicBox block_p13_b5_boxes[] = {
	{ 3, 255, 309, 112, "    LET ME SHOW YOU...", "\n  LAISSE-MOI TE LE PROUVER....", "   TE LO DEMOSTRARE...", "\n    KOMM, ICH ZEIGE ES DIR...." },
	{ 3, 255, 339, 112, "    OH, YESSS...! WHAT IS IT?\n     WHY ARE YOU STOPPING?!", "           OH, OUIIII..!\n QU'EST-CE QU'IL Y A? POURQUOI\n           TU ARRETES?!", "  QUE HACES?\r  POR QUE TE DETIENES?!", "        OH, JAAAA...!\n   WAS IST LOS? WARUM H\x99""RST\n           DU AUF?" },
};

static const struct ComicBlock block_p13_5 = { &poly_p13_b5, -1, -1, block_p13_b5_boxes, 2 };

static const struct Polygon poly_p13_b6 = { {
	{ 318, 138 },
	{ 351, 140 },
	{ 395, 159 },
	{ 415, 189 },
	{ 380, 228 },
	{ 383, 201 },
	{ 318, 202 },
} };

static const struct ComicBox block_p13_b6_boxes[] = {
	{ 4, 246, 121, 110, "      THIS CAN'T WORK OUT,\n STILETTO...I'VE GOT A PARADE\nJUST HOURS FROM NOW, AFTER THAT\n     I'M LEAVING TOWN.", "      STILETTO, \x80""A NE PEUT\n PAS MARCHER... J'AI UN DEFILE\nDANS QUELQUES HEURES, ET JE QUITTE\n        LA VILLE ENSUITE... ", "   ES TARDE,\n STILETTO... AHORA DEBO\n ASISTIR AL DESFILE. LUEGO\n     ME IRE DE LA CIUDAD.", "      DAS KANN NICHT GUTGEHEN,\n  STILETTO...IN EIN PAAR STUNDEN\n    IST DIESE PARADE, DANACH\n   VERSCHWINDE ICH AUS DER STADT." },
	{ 3, 255, 453, 122, "     I KNOW. BUT THERE'S\n SOMETHING YOU DON'T KNOW...\n SOMETHING I'VE NEVER TOLD YOU.", "       JE SAIS. MAIS IL Y A\n  QUELQUE CHOSE QUE TU NE SAIS\n   PAS... QUELQUE CHOSE QUE JE NE\n            T'AI JAMAIS DIT....", "     LO SE. PERO HAY\n ALGO QUE NO SABES...\n ALGO QUE NUNCA TE HE DICHO.", "       ICH WEI\xe1"". ABER DA IST\n  NOCH ETWAS, DAS DU NICHT WEI\xe1""T...\n     ETWAS, DAS ICH DIR NIE\n         GESAGT HABE..." },
};

static const struct ComicBlock block_p13_6 = { &poly_p13_b6, -1, -1, block_p13_b6_boxes, 2 };

static const struct Polygon poly_p13_b7 = { {
	{ 277, 399 },
	{ 274, 367 },
	{ 318, 334 },
	{ 361, 371 },
	{ 359, 399 },
} };

static const struct ComicBox block_p13_b7_boxes[] = {
	{ 3, 246, 381, 284, "       ...I'M SORRY, STILETTO,\n    BUT IT CAN'T BE LIKE THAT.\n I'VE MADE OTHER PLANS... GOALS\nTHAT JUST DON'T ACCOMMODATE A\n RELATIONSHIP. SOMEDAY YOU'LL\n   UNDERSTAND... GOODBYE.", "      JE SUIS DESOLE, STILETTO,\n   MAIS \x80""A NE PEUT PAS SE PASSER\nCOMME \x80""A. J'AI D'AUTRES PLANS... DES\n PLANS QUI NE ME PERMETTENT PAS DE\nCONSERVER UNE RELATION AMOUREUSE...\n      UN JOUR, TU COMPRENDRAS...\n              AU REVOIR.", "   ...LO SIENTO, STILETTO,\n   PERO NO PUEDE SER. HE HECHO\n OTROS PLANES... EN LOS QUE NO\n ES POSIBLE MANTENER UNA\n RELACION. ALGUN DIA LO\n   ENTENDERAS... ADIOS.", "\n      ES TUT MIR LEID, STILETTO,\n    ABER DAS KANN ICH NICHT. ICH\n HABE ANDERE PL\x8e""NE...ZIELE, DIE SICH\nMIT EINER BEZIEHUNG NICHT VEREINBAREN\n  LASSEN. EINES TAGES WIRST DU MICH\n        VERSTEHEN. LEB WOHL." },
};

static const struct ComicBlock block_p13_7 = { &poly_p13_b7, -1, -1, block_p13_b7_boxes, 1 };

static const struct Polygon poly_p13_b8 = { {
	{ 254, 201 },
	{ 383, 201 },
	{ 359, 399 },
	{ 277, 399 },
} };

static const struct ComicBox block_p13_b8_boxes[] = {
	{ 4, 255, 104, 230, "      I... LOVE YOU! IF YOU\n  WON'T CHANGE YOUR MIND ABOUT\nLEAVING, THEN PLEASE... LET ME\n      COME WITH YOU.", "      JE... JE T'AIME! SI TU\n NE VEUX PAS CHANGER D'AVIS AU\nSUJET DE TON DEPART, ALORS JE T'EN\n     PRIE... LAISSE-MOI VENIR\n            AVEC TOI!", "     YO... TE AMO! Y SI\n ESO NO TE HACE DESISTIR DE\n PARTIR... LLEVAME CONTIGO", "       ICH... LIEBE DICH! WENN\n   DU DEINE PL\x8e""NE SCHON NICHT\n    \x8e""NDERST, DANN...LA\xe1"" MICH\n      WENIGSTENS MIT DIR GEHEN!" },
};

static const struct ComicBlock block_p13_8 = { &poly_p13_b8, -1, -1, block_p13_b8_boxes, 1 };

static const struct ComicBlock page_13_blocks[] = {
	block_p13_0,
	block_p13_1,
	block_p13_2,
	block_p13_3,
	block_p13_4,
	block_p13_5,
	block_p13_6,
	block_p13_7,
	block_p13_8,
};

static struct ComicPage page_13 = { "COMDATA/LEPG24.SCN", 98, 52, 1, 0, page_13_blocks, 9 };

static const struct Polygon poly_p14_b0 = { {
	{ 323, 111 },
	{ 417, 111 },
	{ 417, 164 },
	{ 323, 164 },
} };

static const struct ComicBox block_p14_b0_boxes[] = {
	{ 1, 255, 440, 161, "       NEVER AGAIN!!!", "\n         PLUS JAMAIS \x80""A!!!", "    NUNCA MAS!", "\n            NIE WIEDER!!!" },
};

static const struct ComicBlock block_p14_0 = { &poly_p14_b0, -1, -1, block_p14_b0_boxes, 1 };

static const struct Polygon poly_p14_b1 = { {
	{ 476, 278 },
	{ 589, 278 },
	{ 589, 353 },
	{ 476, 353 },
} };

static const struct ComicBox block_p14_b1_boxes[] = {
	{ 4, 255, 322, 247, "      NEVER EVER AGAIN!!!", "\n  JAMAIS, NON, PLUS JAMAIS \x80""A!!!", "       NUNCA MAS!!!", "\n     NIEMALS, NIEMALS WIEDER!!!" },
};

static const struct ComicBlock block_p14_1 = { &poly_p14_b1, -1, -1, block_p14_b1_boxes, 1 };

static const struct Polygon poly_p14_b2 = { {
	{ 323, 0 },
	{ 589, 0 },
	{ 589, 83 },
	{ 451, 213 },
	{ 323, 90 },
} };

static const struct ComicBox block_p14_b2_boxes[] = {
	{ 2, 255, 225, 104, "      ...FURTHER MORE, IN\n  RECOGNITION OF CONSPICUOUS\nSERVICE TO THE REGISTERED VOTERS\n    OF NOCTROPOLIS CITY...", "     ... DE PLUS, EN\n  REMERCIEMENT DES SERVICES\n  DESINTERESSES RENDUS AUX\n  ELECTEURS DE NOCTROPOLIS...", "   ...Y EN RECONOCIMIENTO\n  A LOS SERVICIOS PRESTADOS\n A LOS ELECTORES DE\n    NOCTROPOLIS...", "      ...DESWEITEREN M\x99""CHTE\n  ICH, IN ANERKENNUNG DER DIENSTE,\nDIE ER F\x9a""R DIE REGISTRIERTEN W\x8e""HLER\n   VON NOCTROPOLIS GELEISTET HAT..." },
};

static const struct ComicBlock block_p14_2 = { &poly_p14_b2, -1, -1, block_p14_b2_boxes, 1 };

static const struct Polygon poly_p14_b3 = { {
	{ 323, 90 },
	{ 451, 213 },
	{ 451, 300 },
	{ 323, 300 },
} };

static const struct ComicBox block_p14_b3_boxes[] = {
	{ 2, 255, 160, 227, "      ...IT IS MY HONOR AND\n  PRIVELEGE TO IMPART THE KEY\nTO NOCTROPOLIS CITY TO DARKSHEER,\n WHO LIKE YOUR MAYOR, \"GOT THE\n        JOB DONE IN '51!\"", "      ... J'AI L'HONNEUR ET\n  LE PRIVILEGE D'OFFRIR LA CLEF\nDE NOCTROPOLIS A DARKSHEER, QUI,\n  COMME VOTRE MAIRE, A \"FAIT SON\n          JOB EN '51!\"", "      ...TENGO EL HONOR Y\n EL PRIVILEGIO DE ENTREGAR LAS\n LLAVES DE NOCTROPOLIS A\n TENEBROSO, QUE, AL IGUAL QUE\n  YO, SABE HACER SU TRABAJO", "      ...DEN SCHL\x9a""SSEL ZUR\n  STADT NOCTROPOLIS AN DARKSHEER\n\x9a""BERREICHEN, DER - WIE AUCH ICH ALS\n    B\x9a""RGERMEISTER - STETS SEIN\n          BESTES GEGEBEN HAT." },
};

static const struct ComicBlock block_p14_3 = { &poly_p14_b3, -1, -1, block_p14_b3_boxes, 1 };

static const struct Polygon poly_p14_b4 = { {
	{ 451, 213 },
	{ 589, 83 },
	{ 589, 300 },
	{ 451, 300 },
} };

static const struct ComicBox block_p14_b4_boxes[] = {
	{ 2, 246, 313, 215, "     UH...THANK-YOU MR. MAYOR.\n     THANKS TO EVERYONE!", "     EUH... MERCI, MONSIEUR LE\n   MAIRE... MERCI TOUT LE MONDE!", "     EH... GRACIAS, ALCALDE.\n     GRACIAS A TODOS!", "     EHM... VIELEN DANK, HERR\n  B\x9a""RGERMEISTER. DANKE, IHNEN\n             ALLEN!" },
};

static const struct ComicBlock block_p14_4 = { &poly_p14_b4, -1, -1, block_p14_b4_boxes, 1 };

static const struct ComicBlock page_14_blocks[] = {
	block_p14_0,
	block_p14_1,
	block_p14_2,
	block_p14_3,
	block_p14_4,
};

static struct ComicPage page_14 = { "COMDATA/LEPG2526.SCN", 98, 53, 1, 0, page_14_blocks, 5 };

static const struct Polygon poly_p15_b0 = { {
	{ 225, 22 },
	{ 293, 35 },
	{ 283, 103 },
	{ 213, 90 },
} };

static const struct ComicBox block_p15_b0_boxes[] = {
	{ 5, 246, 312, 85, "      THE MOMENT OF TRUTH...\nDIDN'T THINK IT WOULD BE SO HARD.", "      LE MOMENT DE VERITE...\n    JE NE PENSAIS PAS QUE CE\n         SERAIT SI DUR.", "  EL MOMENTO DE LA VERDAD...\n NUNCA PENSE QUE SERIA TAN DURO.", "     DER MOMENT DER WAHRHEIT...\n  H\x8e""TTE NIE GEDACHT, DA\xe1"" ES SO\n      SCHWER SEIN W\x9a""RDE." },
};

static const struct ComicBlock block_p15_0 = { &poly_p15_b0, -1, -1, block_p15_b0_boxes, 1 };

static const struct Polygon poly_p15_b1 = { {
	{ 213, 155 },
	{ 250, 118 },
	{ 289, 155 },
	{ 253, 193 },
} };

static const struct ComicBox block_p15_b1_boxes[] = {
	{ 5, 246, 309, 176, "      STILL, I KNOW IT'S THE\nRIGHT CHOICE... I'VE GOT A BRIGHTER\n       FUTURE OUT THERE...", "      POURTANT, JE SAIS QUE\nC'EST LE BON CHOIX... MON FUTUR\n  SERA PLUS BRILLANT AILLEURS...", "  PERO SE QUE HAGO LO\n CORRECTO... ME ESPERA UN\n   FUTURO BRILLANTE...", "      TROTZDEM, ICH WEI\xe1"", DA\xe1""\n   ES DIE RICHTIGE ENTSCHEIDUNG\n  WAR... MEINE ZUKUNFT STRAHLT NUN\n          VIEL HELLER..." },
};

static const struct ComicBlock block_p15_1 = { &poly_p15_b1, -1, -1, block_p15_b1_boxes, 1 };

static const struct Polygon poly_p15_b2 = { {
	{ 239, 209 },
	{ 304, 272 },
	{ 210, 292 },
} };

static const struct ComicBox block_p15_b2_boxes[] = {
	{ 6, 246, 22, 277, "     TIME TO GO... FAREWELL\nNOCTROPOLIS... YOU'LL ALWAYS BE\n            HOME.", "     IL EST TEMPS DE PARTIR...\nADIEU, NOCTROPOLIS... TU RESTERAS\n     TOUJOURS DANS MON COEUR.", "  ES HORA DE DECIR ADIOS,\nNOCTROPOLIS... SIEMPRE SERAS\n      MI HOGAR.", "      ZEIT ZUM GEHEN...AUF\n   WIEDERSEHEN, NOCTROPOLIS...\n       DU WIRST IMMER MEIN\n           ZUHAUSE SEIN." },
};

static const struct ComicBlock block_p15_2 = { &poly_p15_b2, -1, -1, block_p15_b2_boxes, 1 };

static const struct Polygon poly_p15_b3 = { {
	{ 187, 0 },
	{ 452, 0 },
	{ 452, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p15_b3_boxes[] = {
	{ 0, 246, 198, 39, "ASSUAGED BY THE KNOWLEDGE THAT DARKSHEER HAS CLEANSED\nAWAY ALL VESTIGE OF ARCHVILLAINY, THE CITY OF NIGHT\nRELAXES ITS GUARD AND SLEEPS... PERCHANCE TO DREAM...", "RASSUREE DE SAVOIR QUE DARKSHEER A ELIMINE TOUTE TRACE\nDES GRANDS CRIMINELS, LA CITE DE LA NUIT RELACHE SA\nGARDE ET S'ENDORT... ESPERANT UN REVE...", " TRANQUILA AL SABER QUE TENEBROSO LA HA LIBRADO DE\n SUS ENEMIGOS, LA CIUDAD DE LA NOCHE SE RELAJA Y SE\n DISPONE A DORMIR...", "BES\x8e""NFTIGT DURCH DAS WISSEN, DA\xe1"" DARKSHEER ALLE NESTER\nDES ERZSCHURKENTUMS AUSGER\x8e""UCHERT HAT, ENTSPANNT SICH\nDIE STADT DER NACHT UND SCHL\x8e""FT EIN...VIELLEICHT\nTR\x8e""UMT SIE AUCH..." },
};

static const struct ComicBlock block_p15_3 = { &poly_p15_b3, -1, -1, block_p15_b3_boxes, 1 };

static const struct ComicBlock page_15_blocks[] = {
	block_p15_0,
	block_p15_1,
	block_p15_2,
	block_p15_3,
};

static struct ComicPage page_15 = { "COMDATA/LEPG27.SCN", 98, 54, 0, 0, page_15_blocks, 4 };

const struct ComicPage *LastComicPages[] = {
	&page_1,
	&page_2,
	&page_3,
	&page_4,
	&page_5,
	&page_6,
	&page_7,
	&page_8,
	&page_9,
	&page_10,
	&page_11,
	&page_12,
	&page_13,
	&page_14,
	&page_15,
	NULL
};

} // end namespace Noctropolis

} // end namespace Access
