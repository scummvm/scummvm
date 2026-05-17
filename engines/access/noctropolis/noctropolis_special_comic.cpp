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

/* Comic Data From Noctropolis Macos, offset 0x99960 */

#include "noctropolis_comicviewer.h"
#include "noctropolis_special_comic.h"

namespace Access {

namespace Noctropolis {

static const struct Polygon poly_p1_b0 = { {
	{ 376, 89 },
	{ 427, 104 },
	{ 450, 145 },
	{ 422, 191 },
	{ 377, 203 },
	{ 320, 182 },
	{ 305, 145 },
	{ 328, 104 },
} };

static const struct ComicBox block_p1_b0_boxes[] = {
	{ 3, 244, 453, 88, "     ...SLOW DISSECTION...\n  LIVING AUTOPSY! NO TOOLS...\nRIP HIS HEART FROM ITS PLACE! OH,\n THE PAIN... THE IMMACULATE PAIN!!", "     ...UNE LENTE DISSECTION...\n     UNE AUTOPSIE VIVANTE! PAS\nD'INSTRUMENTS... LUI ARRACHER LE\nCOEUR! OH, QUELLE DOULEUR... UNE\n        DOULEUR IMMACULEE!!", "     ...LO DISECARIA...\n LE HARIA LA AUTOPSIA VIVO...\n SIN INSTRUMENTOS. LE SACARIA\n EL CORAZON... LO HARIA GRITAR!!", "     ... LANGSAM ZERTEILEN...\n  VIVISEKTION. NICHT SCHNEIDEN...\n    REI\xe1""EN! OH, DER SCHMERZ..." },
};

static const struct ComicBlock block_p1_0 = { &poly_p1_b0, -1, -1, block_p1_b0_boxes, 1 };

static const struct Polygon poly_p1_b1 = { {
	{ 187, 0 },
	{ 451, 0 },
	{ 451, 95 },
	{ 187, 95 },
} };

static const struct ComicBox block_p1_b1_boxes[] = {
	{ 0, 246, 200, 73, "NOCTROPOLIS CITY PENITENTIARY.", "PENITENCIER MUNICIPAL DE NOCTROPOLIS", "PRISION DE NOCTROPOLIS.", "ST\x8e""DTISCHES GEF\x8e""NGNIS NOCTROPOLIS." },
};

static const struct ComicBlock block_p1_1 = { &poly_p1_b1, -1, -1, block_p1_b1_boxes, 1 };

static const struct Polygon poly_p1_b2 = { {
	{ 187, 95 },
	{ 451, 95 },
	{ 451, 210 },
	{ 187, 210 },
} };

static const struct ComicBox block_p1_b2_boxes[] = {
	{ 0, 246, 207, 187, "CELL 427-JJ, MAXIMUM SECURITY WING. CURRENT\nINMATE: DR. WILLIAM BLISH, A.K.A. MASTER\nMACABRE (ARCH CRIMINAL)", "CELLULE 427-JJ, QUARTIER DE HAUTE SECURITE.\nDETENU ACTUEL: DR. WILLIAM BLISH, ALIAS\nMAITRE MACABRE (GRAND CRIMINEL)", "CELDA 427-JJ DE MAXIMA SEGURIDAD. OCUPANTE\nACTUAL: DR. WILLIAM BLISH, ALIAS MAESTRO\nMACABRO (ARCHICRIMINAL)", "ZELLE 427-JJ DES HOCHSICHERHEITSTRAKTES.\nDERZEITIGER INSASSE: DR. WILLIAM BLISH, ALIAS\nDOKTOR MAKABER (ERZSCHURKE)" },
	{ 3, 244, 334, 108, "    ...INTOLERABLE! MY TALENTS\n ARE WASTING BEHIND THESE BLOODY\nBARS. BLOOD! THAT'S THE ANSWER.\n   OH, TO HAVE DARKSHEER AT\n          MY MERCY...", "    ... C'EST INTOLERABLE! MES\n  TALENTS GACHES DERRIERE CES\n  BARREAUX! DU SANG! VOILA LA\n   REPONSE. OH... AVOIR DARKSHEER\n            A MA MERCI...", "    ...INTOLERABLE! MI TALENTO\n DESPERDICIADO TRAS ESTAS REJAS.\n SI PUDIERA PONER MIS MANOS\n    SOBRE TENEBROSO...", "      ... UNERTR\x8e""GLICH! MEINE\n     TALENTE VERGEUDET HINTER\n  GITTERN, WIE BEI EINEM BLUTIGEN\n ANF\x8e""NGER. BLUT! DAS IST DIE ANTWORT.\n   OH, WENN ICH DOCH NUR DARKSHEER\n      IN MEINER GEWALT H\x8e""TTE..." },
};

static const struct ComicBlock block_p1_2 = { &poly_p1_b2, -1, -1, block_p1_b2_boxes, 2 };

static const struct Polygon poly_p1_b3 = { {
	{ 187, 210 },
	{ 283, 210 },
	{ 283, 312 },
	{ 233, 337 },
	{ 187, 314 },
} };

static const struct ComicBox block_p1_b3_boxes[] = {
	{ 3, 246, 308, 181, "      IT IS ALWAYS REFRESHING\n   TO ENCOUNTER THOSE WHO\nAPPRECIATE THE AESTHETICS OF\n  PROPER PAIN DISPENSATION.", "        C'EST TOUJOURS\n  RAFRAICHISSANT DE RENCONTRER\n   CEUX QUI SAVENT APPRECIER\n      L'ESTHETIQUE DE LA DOULEUR\n             BIEN INFLIGEE.", "  SIEMPRE ES AGRADABLE\n  ESCUCHAR A UN\n  PROFESIONAL DEL DOLOR.", "            ES IST IMMER\n      ERFRISCHEND, JEMANDEN\n    KENNENZULERNEN, DER DIE\n  \x8e""STHETIK DER SCHMERZVERTEILUNG\n         ZU W\x9a""RDIGEN WEI\xe1""." },
	{ 4, 244, 5, 173, "            WHO...?", "\n          QUI...?", "     QUIEN ERES?", "\n               WER...?" },
};

static const struct ComicBlock block_p1_3 = { &poly_p1_b3, -1, -1, block_p1_b3_boxes, 2 };

static const struct Polygon poly_p1_b4 = { {
	{ 187, 314 },
	{ 233, 337 },
	{ 283, 312 },
	{ 283, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p1_b4_boxes[] = {
	{ 3, 246, 297, 300, "      I AM FLUX. I HAVE COME\n  TO OFFER YOU A PROPOSITION.\n   BUT I FIND THIS BARRIER\n  BOTHERSOME. COME, JOIN ME\n        IN THE CORRIDOR.", "\n      JE M'APPELLE FLUX. JE\n    SUIS VENU VOUS FAIRE UNE\nPROPOSITION. MAIS JE TROUVE\n CETTE BARRIERE GENANTE. VENEZ,\n     REJOIGNEZ-MOI DANS\n         LE COULOIR.", "    SOY FLUX Y VENGO A\n  HACERTE UNA PROPOSICION.\n  PERO ESTOS BARROTES SON\n  MOLESTOS. POR QUE NO SALES\n    AL CORREDOR?", "          NENNEN SIE MICH\n    EINFACH FLUX. ICH BIN GEKOMMEN,\n     UM IHNEN EINEN VORSCHLAG ZU\n  UNTERBREITEN. ABER ICH FINDE DIESE\n   BARRIERE SEHR ST\x99""REND. KOMMEN\n     SIE DOCH ZU MIR HERAUS\n           AUF DEN FLUR." },
};

static const struct ComicBlock block_p1_4 = { &poly_p1_b4, -1, -1, block_p1_b4_boxes, 1 };

static const struct Polygon poly_p1_b5 = { {
	{ 283, 210 },
	{ 451, 210 },
	{ 451, 399 },
	{ 283, 399 },
} };

static const struct ComicBox block_p1_b5_boxes[] = {
	{ 3, 244, 392, 184, "        THE GUARDS...", "\n        LES GARDES...", "        LOS GUARDIAS...", "        ABER DIE WACHEN..." },
	{ 4, 246, 144, 248, "     HAVE BEEN... RELIEVED. I\n SHARE YOUR VIEWPOINT, MACABRE,\n  YOUR GIFTS ARE BEING WASTED\n           HERE...", "      ONT ETE... RELEVES. JE\n  PARTAGE VOTRE POINT DE VUE,\n  MACABRE, VOTRE TALENT N'EST\n        PAS EXPLOITE ICI...", "   HAN IDO A... DESCANSAR.\n ESTOY DE ACUERDO, MACABRO.\n TU TALENTO SE DESPERDICIA\n           AQUI...", "      WURDEN...ABGEL\x99""ST. ICH\n   TEILE IHRE ANSICHT, MAKABER,\n   DA\xe1"" IHRE TALENTE HIER NUR\n        VERGEUDET WERDEN." },
};

static const struct ComicBlock block_p1_5 = { &poly_p1_b5, -1, -1, block_p1_b5_boxes, 2 };

static const struct ComicBlock page_1_blocks[] = {
	block_p1_0,
	block_p1_1,
	block_p1_2,
	block_p1_3,
	block_p1_4,
	block_p1_5,
};

static struct ComicPage page_1 = { "COMDATA/SEPG01.SCN", 98, 55, 1, 0, page_1_blocks, 6 };

static const struct Polygon poly_p2_b0 = { {
	{ 369, 36 },
	{ 450, 36 },
	{ 450, 150 },
	{ 369, 150 },
} };

static const struct ComicBox block_p2_b0_boxes[] = {
	{ 2, 244, 158, 132, "     OF COURSE, FLUX, YOU'RE\nTHE BOSS... NOW TELL ME MORE\n      ABOUT DARKSHEER!", "     BIEN SUR, FLUX... C'EST\nVOUS LE BOSS... MAIS DITES M'EN\n UN PEU PLUS SUR CE DARKSHEER!", "  POR SUPUESTO, FLUX. ERES\nEL JEFE... AHORA HABLAME\n      DE TENEBROSO!", "     SELBSTVERST\x8e""NDLICH, FLUX,\nSIE SIND DER BOSS...ERZ\x8e""HLEN\n   SIE MIR MEHR \x9a""BER DARKSHEER!" },
};

static const struct ComicBlock block_p2_0 = { &poly_p2_b0, -1, -1, block_p2_b0_boxes, 1 };

static const struct Polygon poly_p2_b1 = { {
	{ 187, 0 },
	{ 450, 0 },
	{ 450, 217 },
	{ 187, 217 },
} };

static const struct ComicBox block_p2_b1_boxes[] = {
	{ 2, 246, 68, 68, "    ...I OFFER YOU FREEDOM. A\n  CHANCE TO REALIZE YOUR DREAM\nOF RIPPING DARKSHEER'S HEART TO\n BLOODY SHREDS! IN EXCHANGE, I\n    DEMAND YOUR ALLEGIANCE.", "\n    ...JE VOUS OFFRE LA LIBERTE.\n   UNE CHANCE DE REALISER VOTRE\nREVE EN ARRACHANT LE COEUR DE\n  DARKSHEER POUR LE REDUIRE EN\n   LAMBEAUX SANGLANTS! EN ECHANGE,\n       JE VOUS DEMANDE VOTRE\n             FIDELITE", "    ...TE OFREZCO LA LIBERTAD\n Y UNA OPORTUNIDAD DE REALIZAR\n TUS DESEOS DE DESTROZAR A\n TENEBROSO. A CAMBIO, SOLO PIDO\n QUE TE SUBORDINES A MI.", "      ...ICH BIETE IHNEN DIE\n    FREIHEIT, DAZU EINE CHANCE,\n IHREN TRAUM ZU VERWIRKLICHEN UND\n       DARKSHEER DAS HERZ\n   RAUSZUREI\xe1""EN. DAF\x9a""R\n     VERLANGE ICH GEHORSAM." },
};

static const struct ComicBlock block_p2_1 = { &poly_p2_b1, -1, -1, block_p2_b1_boxes, 1 };

static const struct Polygon poly_p2_b2 = { {
	{ 187, 150 },
	{ 285, 217 },
	{ 322, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p2_b2_boxes[] = {
	{ 3, 246, 296, 212, "     LATER. THE OTHERS AWAIT\n        OUR ARRIVAL.", "     PLUS TARD. LES AUTRES\n        NOUS ATTENDENT.", "  MAS TARDE. LOS DEMAS\n        NOS ESPERAN.", "       SP\x8e""TER. DIE ANDEREN\n         WARTEN AUF UNS." },
	{ 3, 244, 346, 258, "     OTHERS?", "\n        LES AUTRES?", "   LOS DEMAS?", "\n           ANDEREN?" },
	{ 3, 246, 296, 212, "       TOPHAT, DREALMER,\n  GREENTHUMB AND THE SUCCUBUS\nHAVE ALSO ACCEPTED ME AS THEIR\n LEADER. TOGETHER, WE WILL BRING\n   NOCTROPOLIS TO ITS KNEES!", "       TOPHAT, DREALMER,\n  MAINVERTE ET LA SUCCUBE ONT\n EGALEMENT ACCEPTE DE M'OBEIR.\n  ENSEMBLE, NOUS ALLONS METTRE\n     NOCTROPOLIS A GENOUX!", "   LA DIVA, MORFEICO,\n  DEDOVERDE Y LA SUCCUBUS\n TAMBIEN ME HAN ACEPTADO COMO\n LIDER. JUNTOS, PONDREMOS A\n   NOCTROPOLIS DE RODILLAS!", "         MISS ZYLINDERCHEN,\n    TR\x8e""UMLER, GR\x9a""ND\x8e""UMLING UND\n  SUCCUBUS HABEN MICH EBENFALLS\n    ALS ANF\x9a""HRER AKZEPTIERT.\n     GEMEINSAM WERDEN WIR\n       NOCTROPOLIS IN DIE\n          KNIE ZWINGEN!" },
};

static const struct ComicBlock block_p2_2 = { &poly_p2_b2, -1, -1, block_p2_b2_boxes, 3 };

static const struct Polygon poly_p2_b3 = { {
	{ 285, 217 },
	{ 450, 217 },
	{ 450, 296 },
	{ 302, 296 },
} };

static const struct ComicBox block_p2_b3_boxes[] = {
	{ 4, 244, 115, 196, "     AH, FLUX, I LIKE YOUR\n             STYLE!", "     AH, FLUX, J'AIME BIEN\n         VOTRE STYLE!", "     ME GUSTA TU\n       ESTILO, FLUX!", "         HMM, FLUX, IHR STIL\n            GEF\x8e""LLT MIR." },
};

static const struct ComicBlock block_p2_3 = { &poly_p2_b3, -1, -1, block_p2_b3_boxes, 1 };

static const struct Polygon poly_p2_b4 = { {
	{ 302, 296 },
	{ 450, 296 },
	{ 450, 399 },
	{ 322, 399 },
} };

static const struct ComicBox block_p2_b4_boxes[] = {
	{ 4, 246, 145, 263, "     YES... I BELIEVE YOU'LL\nFIND THAT WE HAVE MUCH IN COMMON.", "\n     OUI... JE CROIS QUE VOUS\n VERREZ BIENTOT QUE NOUS AVONS\n       BEAUCOUP DE CHOSES\n           EN COMMUN...", "  SI... VERAS QUE TENEMOS\n MUCHO EN COMUN.", "       JA, ICH GLAUBE, SIE\nWERDEN FESTSTELLEN, DA\xe1"" WIR EINE\n      MENGE GEMEINSAM HABEN." },
};

static const struct ComicBlock block_p2_4 = { &poly_p2_b4, -1, -1, block_p2_b4_boxes, 1 };

static const struct ComicBlock page_2_blocks[] = {
	block_p2_0,
	block_p2_1,
	block_p2_2,
	block_p2_3,
	block_p2_4,
};

static struct ComicPage page_2 = { "COMDATA/SEPG02.SCN", 98, 55, 1, 0, page_2_blocks, 5 };

static const struct Polygon poly_p3_b0 = { {
	{ 187, 0 },
	{ 451, 0 },
	{ 451, 186 },
	{ 187, 186 },
} };

static const struct ComicBox block_p3_b0_boxes[] = {
	{ 2, 246, 98, 99, "       COLLECTIVELY, YOU ARE\n   ARMAGEDDON: A NIGHTMARE FOR\nTHE CITY OF NIGHT! GO NOW, EXPLORE\n EVERY PERVERSION. LET TERROR REIGN\n  IN THE STREETS AND CHAOS CONSUME\n           THE WHOLE!", "        ENSEMBLE, VOUS ETES\n    L'APOCALYPSE, UN CAUCHEMAR\n  POUR LA CITE DE LA NUIT! ALLEZ,\n   MAINTENANT. EXPLOREZ CHAQUE\n   PERVERSION. LAISSEZ LA TERREUR\n    REGNER DANS LES RUES, ET LE\n       CHAOS CONSUMERA TOUT!", "    JUNTOS SEREMOS EL\n   ARMAGEDON, UNA PESADILLA PARA\n LA CIUDAD DE LA NOCHE. DEDICAOS A\n CUANTA PERVERSION SE OS OCURRA.\n  SEMBRAD EL TERROR EN LAS CALLES!", "       GEMEINSAM SEID IHR DAS\n    ARMAGEDDON, DER ALPTRAUM F\x9a""R\n  DIE STADT DER NACHT! GEHT JETZT\n  UND LEBT EURE PERVERSIONEN AUS!\n   LA\xe1""T DEN TERROR IN DEN STRA\xe1""EN\n      REGIEREN UND DAS CHAOS\n        ALLES VERSCHLINGEN!" },
};

static const struct ComicBlock block_p3_0 = { &poly_p3_b0, -1, -1, block_p3_b0_boxes, 1 };

static const struct Polygon poly_p3_b1 = { {
	{ 187, 186 },
	{ 451, 186 },
	{ 451, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p3_b1_boxes[] = {
	{ 0, 246, 221, 318, "THE DREALMER.", "DREALMER.", "MORFEICO.", "TR\x8e""UMLER." },
	{ 0, 246, 270, 267, "TOPHAT.", "TOPHAT.", "LA DIVA.", "MISS ZYLINDERCHEN." },
	{ 0, 246, 327, 289, "MASTER MACABRE.", "MAITRE MACABRE.", "MACABRO.", "DOKTOR MAKABER." },
	{ 0, 246, 362, 251, "GREENTHUMB.", "MAINVERTE.", "DEDOVERDE.", "GR\x9a""ND\x8e""UMLING." },
	{ 0, 246, 419, 278, "THE SUCCUBUS.", "LA SUCCUBE.", "LA SUCCUBUS.", "SUCCUBUS." },
};

static const struct ComicBlock block_p3_1 = { &poly_p3_b1, -1, -1, block_p3_b1_boxes, 5 };

static const struct ComicBlock page_3_blocks[] = {
	block_p3_0,
	block_p3_1,
};

static struct ComicPage page_3 = { "COMDATA/SEPG03.SCN", 98, 56, 0, 0, page_3_blocks, 2 };

static const struct Polygon poly_p4_b0 = { {
	{ 187, 0 },
	{ 451, 0 },
	{ 451, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p4_b0_boxes[] = {
	{ 0, 246, 216, 189, "NOCTROPOLIS CITY STAGGERS UNDER THE VILLAINS'\nRUTHLESS ONSLAUGHT. WORKING IN CONCERT UNDER THE\nDIRECTION OF THE ENIGMATIC MASTERMIND, FLUX, THE\nINSIDIOUS QUINTET RELENTLESSLY DEVOUR THE CITY OF\nNIGHT A BITE AT A TIME....", "NOCTROPOLIS VACILLE SOUS LES ASSAUTS FAROUCHES\nDES MALEFIQUES. TRAVAILLANT DE CONCERT SOUS LA\nDIRECTION DE FLUX, LE MYSTERIEUX CERVEAU, LES\nCINQ EVADES DEVORENT SANS RELACHE LA CITE DE LA\nNUIT... BOUCHEE APRES BOUCHEE...", " NOCTROPOLIS SE ESTREMECE BAJO EL ASALTO DE\n LOS CRIMINALES. TRABAJANDO BAJO LA\n DIRECCION DEL MISTERIOSO FLUX, EL\n QUINTETO HACE CUNDIR EL PANICO EN LAS\n CALLES...", "NOCTROPOLIS ERBEBT UNTER DEM GNADENLOSEN ANSTURM\nDES B\x99""SEN. DANK DER KOORDINATION DES GEHEIMNISVOLLEN\nDRAHTZIEHERS FLUX VERSCHLINGT DAS QUINTETT DIE STADT\nDER NACHT BISSEN F\x9a""R BISSEN..." },
};

static const struct ComicBlock block_p4_0 = { &poly_p4_b0, -1, -1, block_p4_b0_boxes, 1 };

static const struct ComicBlock page_4_blocks[] = {
	block_p4_0,
};

static struct ComicPage page_4 = { "COMDATA/SEPG04.SCN", 98, 57, 1, 0, page_4_blocks, 1 };

static const struct Polygon poly_p5_b0 = { {
	{ 187, 0 },
	{ 451, 0 },
	{ 451, 78 },
	{ 187, 78 },
} };

static const struct ComicBox block_p5_b0_boxes[] = {
	{ 0, 246, 216, 43, "NOCTROPOLIS CITY CATHEDRAL.", "CATHEDRALE DE NOCTROPOLIS.", "CATEDRAL DE NOCTROPOLIS.", "DOM VON NOCTROPOLIS." },
};

static const struct ComicBlock block_p5_0 = { &poly_p5_b0, -1, -1, block_p5_b0_boxes, 1 };

static const struct Polygon poly_p5_b1 = { {
	{ 187, 78 },
	{ 343, 78 },
	{ 343, 185 },
	{ 187, 185 },
} };

static const struct ComicBox block_p5_b1_boxes[] = {
	{ 4, 255, 29, 76, "      YO, FATHER, C'MON MAN,\n I KNOW THEY'RE TOUGH 'N ALL, BUT\nWE JUST GOTTA DO SOMETHIN'... THE\n     POLICE SURE AIN'T!", "      YO, PERE, ECOUTE, MEC.\n JE SAIS QUE C'EST DES DURS, TOUT\n  \x80""A... MAIS FAUT FAIRE QUELQUE\n   CHOSE... LA POLICE NE FAIT RIEN,\n           \x80""A C'EST SUR!", "    OIGA, PADRE, LA GENTE\n CREE QUE ESTO ES EL FIN, PERO\n NOSOTROS PODEMOS HACER ALGO...\n LA POLICIA ES INUTIL!", "      ALSO, VATER, KOMMEN SIE\n   SCHON, ICH WEI\xe1"", DA\xe1"" SIE EIN\n    HARTER BURSCHE SIND, ABER WIR\n M\x9a""SSEN WAS TUN, DENN DIE POLIZEI\n      TUT BESTIMMT NICHTS!" },
	{ 3, 255, 344, 87, "     YEAH, FATHER, JUST LET\n THE WARDS GIVE IT A TRY.", "     OUAIS, PERE, LAISSEZ\n     FAIRE LES PUPILLES!", "   SI, PADRE, DEJE QUE LOS\n GUARDIANES LO INTENTEMOS.", "     JAU, VATER, GEBEN SIE DER\n  WEHR DOCH EINMAL EINE CHANCE." },
};

static const struct ComicBlock block_p5_1 = { &poly_p5_b1, -1, -1, block_p5_b1_boxes, 2 };

static const struct Polygon poly_p5_b2 = { {
	{ 343, 78 },
	{ 451, 78 },
	{ 451, 167 },
	{ 343, 167 },
} };

static const struct ComicBox block_p5_b2_boxes[] = {
	{ 4, 255, 189, 50, "     ABSOLUTELY NOT! ALL FIVE\n   OF THEM ARE TWISTED, COLD-\nBLOODED KILLERS, RICO... THEY'D EAT\n THE WARDS ALIVE! MAYBE... MAYBE\n      STILETTO CAN HELP US.", "\n     CERTAINEMENT PAS! CE SONT\n     CINQ DEMENTS QUI TUENT DE\n SANG-FROID, RICO... ILS NE FERAIENT\nQU'UNE BOUCHEE DE VOUS! PEUT-ETRE\n    QUE... PEUT-ETRE QUE STILETTO\n             PEUT NOUS AIDER.", "     DE ESO NADA! SE TRATA\n  DE CINCO CRIMINALES SIN\n PIEDAD QUE OS DEVORARIAN VIVOS!\n QUIZAS STILETTO PUEDA AYUDARNOS.", "\n     BESTIMMT NICHT! DAS SIND\n   ALLE F\x9a""NF GEST\x99""RTE, KALTBL\x9a""TIGE\nKILLER, RICO...DIE W\x9a""RDEN DIE WEHR\n LEBENDIG ZERFLEISCHEN. VIELLEICHT...\n   VIELLEICHT KANN UNS STILETTO\n              HELFEN." },
};

static const struct ComicBlock block_p5_2 = { &poly_p5_b2, -1, -1, block_p5_b2_boxes, 1 };

static const struct Polygon poly_p5_b3 = { {
	{ 187, 185 },
	{ 333, 185 },
	{ 333, 399 },
	{ 187, 399 },
} };

static const struct ComicBox block_p5_b3_boxes[] = {
	{ 4, 255, 40, 250, "       IF STILETTO CAN'T HELP,\n  THEN IT'S OVER... ESPECIALLY FOR\n ME. THE SUCCUBUS HASN'T FORGOTTEN\n HER HATE, IT'S PROBABLY STRONGER\n    THAN EVER. GOD HELP ME WHEN\n            SHE COMES!", "         SI STILETTO NE PEUT\n    PAS NOUS AIDER, ALORS TOUT\n EST FINI... SURTOUT POUR MOI. LA\nSUCCUBE N'A PAS OUBLIE SA HAINE, ET\n  ELLE EST SUREMENT PLUS FORTE QUE\n   JAMAIS! QUE DIEU ME VIENNE EN\n      AIDE SI ELLE VIENT ICI!", "       SI STILETTO SE NIEGA, SE\n  ACABARA TODO... EN ESPECIAL\n PARA MI. LA SUCCUBUS NO OLVIDA.\n Y AHORA DEBE SER MAS FUERTE QUE\n NUNCA. QUE DIOS SE APIADE DE MI\n     SI ELLA VIENE!", "       WENN UNS STILETTO NICHT\n   HELFEN KANN, IST ALLES VORBEI...\n BESONDERS F\x9a""R MICH. SUCCUBUS HAT\n  IHREN HA\xe1"" BESTIMMT NICHT\n   VERGESSEN, WAHRSCHEINLICH\n    IST ER EHER NOCH GEWACHSEN.\n    GOTT STEH MIR BEI, WENN SIE\n               KOMMT!" },
};

static const struct ComicBlock block_p5_3 = { &poly_p5_b3, -1, -1, block_p5_b3_boxes, 1 };

static const struct Polygon poly_p5_b4 = { {
	{ 333, 167 },
	{ 451, 167 },
	{ 451, 399 },
	{ 333, 399 },
} };

static const struct ComicBox block_p5_b4_boxes[] = {
	{ 4, 255, 211, 173, "      WELL IF IT AIN'T FATHER\n  DESMOND! COME TO GIVE POOR\nSTILETTO MORE ADVICE, PRIEST? NOW,\n           LET'S SEE...", "       MAIS C'EST LE PERE\n  DESMOND! ON EST VENU DONNER\n DES CONSEILS A STILETTO, CURE?\n          BON, VOYONS VOIR...", "   QUE SORPRESA, PADRE\n  DESMOND! VIENE A DAR CONSEJOS\n A LA POBRE STILETTO?", "\n      NA, WENN DAS NICHT VATER\n  DESMOND IST! VORBEIGEKOMMEN, UM\nDER ARMEN STILETTO NOCH EIN PAAR\n  RATSCHL\x8e""GE ZU GEBEN, PRIESTER?\n    NA, DANN SCHAUEN WIR DOCH MAL..." },
};

static const struct ComicBlock block_p5_4 = { &poly_p5_b4, -1, -1, block_p5_b4_boxes, 1 };

static const struct ComicBlock page_5_blocks[] = {
	block_p5_0,
	block_p5_1,
	block_p5_2,
	block_p5_3,
	block_p5_4,
};

static struct ComicPage page_5 = { "COMDATA/SEPG05.SCN", 98, 58, 1, 0, page_5_blocks, 5 };

static const struct Polygon poly_p6_b0 = { {
	{ 188, 0 },
	{ 320, 0 },
	{ 320, 107 },
	{ 188, 107 },
} };

static const struct ComicBox block_p6_b0_boxes[] = {
	{ 1, 255, 292, 98, "    ...\"GOD WANTS ME TO BE\n  HAPPY\",... \"DARKSHEER WILL\nRETURN MY LOVE\". THAT'S WHAT\n  YOU TOLD ME...\n      WHAT A LOAD OF BULL!", "       ...\"DIEU VEUT QUE TU\n                 SOIS HEUREUSE,\"... \n   \"DARKSHEER TE RENDRA \n                    TON AMOUR...\" \n     C'EST CE QUE VOUS DISIEZ...\n     RIEN QUE DES CONNERIES!", "    ...DIOS QUIERE QUE\n SEAS FELIZ... TENEBROSO\n ENTENDERA TU AMOR... ESO ME\n DIJO, RECUERDA?", "       ...\"GOTT WILL, DA\xe1""\n               ICH GL\x9a""CKLICH BIN.\"\n...\"DARKSHEER WIRD MEINE\n                       LIEBE ERWIDERN\",\n    DAS WAR'S DOCH, WAS SIE MIR\n    ERZ\x8e""HLST HABEN...WAS F\x9a""R EINE\n               SCHEI\xe1""E!" },
};

static const struct ComicBlock block_p6_0 = { &poly_p6_b0, -1, -1, block_p6_b0_boxes, 1 };

static const struct Polygon poly_p6_b1 = { {
	{ 320, 0 },
	{ 451, 0 },
	{ 451, 107 },
	{ 320, 107 },
} };

static const struct ComicBox block_p6_b1_boxes[] = {
	{ 2, 255, 195, 91, "    STILETTO, I UNDERSTAND\nYOUR PAIN, BUT I'VE COME TO\n  DISCUSS URGENT MATTERS.", "      STILETTO, JE COMPRENDS\n  TA PEINE, MAIS JE SUIS VENU TE\n      PARLER D'UNE AFFAIRE\n           IMPORTANTE...", "    STILETTO, COMPRENDO TU\n DOLOR, PERO ME TRAEN\n ASUNTOS MUY URGENTES.", "      STILETTO, ICH VERSTEHE\n  JA DEINEN SCHMERZ, ABER ICH\n     MU\xe1"" WICHTIGE DINGE MIT DIR\n            BESPRECHEN." },
};

static const struct ComicBlock block_p6_1 = { &poly_p6_b1, -1, -1, block_p6_b1_boxes, 1 };

static const struct Polygon poly_p6_b2 = { {
	{ 188, 107 },
	{ 320, 107 },
	{ 320, 210 },
	{ 188, 210 },
} };

static const struct ComicBox block_p6_b2_boxes[] = {
	{ 3, 255, 330, 78, "      THE ARCHVILLAINS HAVE\n  ESCAPED FROM PRISON! THEY'RE\nTEARING THIS CITY APART! NOW,\n YOU'VE GOT TO PULL YOURSELF\n           TOGETHER.", "      LES ARCHI-CRIMINELS SE\n  SONT EVADES! ILS SONT EN TRAIN\n DE RUINER NOTRE VILLE! TU DOIS\n     ABSOLUMENT TE SECOUER,\n          MAINTENANT!", "    LOS ARCHIVILLANOS HAN\n  ESCAPADO DE PRISION Y SIEMBRAN\n EL TERROR! SOLO TU PUEDES\n ENFRENTARTE A ELLOS.", "      DIE ERZSCHURKEN SIND\n  AUS DEM GEF\x8e""NGNIS ENTKOMMEN!\nSIE REI\xe1""EN DIESE STADT IN ST\x9a""CKE!\n     DU MU\xe1""T DICH ZUSAMMENNEHMEN!" },
};

static const struct ComicBlock block_p6_2 = { &poly_p6_b2, -1, -1, block_p6_b2_boxes, 1 };

static const struct Polygon poly_p6_b3 = { {
	{ 320, 107 },
	{ 451, 107 },
	{ 451, 210 },
	{ 320, 210 },
} };

static const struct ComicBox block_p6_b3_boxes[] = {
	{ 4, 255, 159, 77, "    WHO THE HELL DO YOU THINK\n  YOU ARE?! I DON'T HAVE TO DO\n A DAMN THING FOR YOU OR THIS\n             CITY!", "      MAIS VOUS VOUS PRENEZ\n  POUR QUI? JE NE SUIS PAS\n OBLIGEE DE FAIRE QUOIQUE CE SOIT,\n      NI POUR VOUS, NI POUR CETTE\n           FOUTUE VILLE!", "  QUIEN TE CREES QUE ERES?\n NO TE DEBO NADA NI A TI NI A\n ESTA MALDITA CIUDAD!", "      WER ZUR H\x99""LLE GLAUBEN\n  SIE, DA\xe1"" SIE SIND? ICH MU\xe1""\n\x9a""BERHAUPT NICHTS TUN, WEDER F\x9a""R\n      SIE NOCH F\x9a""R DIESE STADT!" },
};

static const struct ComicBlock block_p6_3 = { &poly_p6_b3, -1, -1, block_p6_b3_boxes, 1 };

static const struct Polygon poly_p6_b4 = { {
	{ 188, 210 },
	{ 320, 210 },
	{ 320, 399 },
	{ 188, 399 },
} };

static const struct ComicBox block_p6_b4_boxes[] = {
	{ 3, 255, 314, 280, "       IF I WANT TO SIT ON MY\n   ASS 'N DRINK MYSELF BLIND, THEN\n THAT'S MY OWN BUSINESS! FIND\n  ANOTHER SHOULDER TO CRY ON\n          PRIEST...\n     I DON'T CARE ANYMORE.", "            SI JE PREFERE\n       RESTER ASSISE A M'IMBIBER\n   CONSCIENCIEUSEMENT, C'EST MON\nTROUVEZ UNE AUTRE EPAULE PROBLEME!\n     POUR PLEURER, CURE... JE M'EN\n          FOUS, MAINTENANT.", "    SI HE DECIDIDO SENTARME A\n  BEBER, ESO ES ASUNTO\n MIO. BUSCA OTRO HOMBRO SOBRE\n EL QUE LLORAR, SACERDOTE.\n A MI NO ME IMPORTA NADA.", "        WENN ICH AUF MEINEM\n    HINTERN SITZEN UND MICH BLIND\n SAUFEN M\x99""CHTE, DANN IST DAS ALLEIN\n MEINE ANGELEGENHEIT! SUCHEN SIE SICH EINE\n    ANDERE SCHULTER ZUM AUSHEULEN,\n    PRIESTER, MIR IST DAS ALLES EGAL!" },
};

static const struct ComicBlock block_p6_4 = { &poly_p6_b4, -1, -1, block_p6_b4_boxes, 1 };

static const struct Polygon poly_p6_b5 = { {
	{ 320, 210 },
	{ 451, 210 },
	{ 451, 399 },
	{ 320, 399 },
} };

static const struct ComicBox block_p6_b5_boxes[] = {
	{ 4, 255, 194, 272, "     GOOD-BYE, STILETTO....", "\n       AU REVOIR, STILETTO....", "     ADIOS, STILETTO....", "\n    AUF WIEDERSEHEN, STILETTO...." },
};

static const struct ComicBlock block_p6_5 = { &poly_p6_b5, -1, -1, block_p6_b5_boxes, 1 };

static const struct ComicBlock page_6_blocks[] = {
	block_p6_0,
	block_p6_1,
	block_p6_2,
	block_p6_3,
	block_p6_4,
	block_p6_5,
};

static struct ComicPage page_6 = { "COMDATA/SEPG06.SCN", 98, 58, 1, 0, page_6_blocks, 6 };

static const struct Polygon poly_p7_b0 = { {
	{ 354, 4 },
	{ 441, 4 },
	{ 441, 123 },
	{ 354, 123 },
} };

static const struct ComicBox block_p7_b0_boxes[] = {
	{ 2, 255, 172, 71, "       I BESEECH THEE IN AN\n   HOUR OF GREAT NEED... SEND\nDARKSHEER BACK TO US... SEND\n         HIM BACK!", "       JE T'IMPLORE DANS UN\n     MOMENT DE GRAND BESOIN...\n  RAMENE DARKSHEER PARMI NOUS...\n    NOUS AVONS BESOIN DE LUI!", "    ACUDO A TI EN ESTA HORA\n  DE NECESIDAD... ENVIANOS\n A TENEBROSO... TRAELO\n   DE REGRESO!", "       ICH BESCHW\x99""RE DICH, IN\n   DIESER STUNDE DER NOT...SCHICKE\n   UNS DARKSHEER ZUR\x9a""CK...SCHICK'\n            IHN ZUR\x9a""CK!" },
};

static const struct ComicBlock block_p7_0 = { &poly_p7_b0, -1, -1, block_p7_b0_boxes, 1 };

static const struct Polygon poly_p7_b1 = { {
	{ 187, 0 },
	{ 451, 0 },
	{ 451, 188 },
	{ 187, 188 },
} };

static const struct ComicBox block_p7_b1_boxes[] = {
	{ 4, 255, 61, 66, "     OH, GOD IN HEAVEN...", "\n         OH, DIEU DES CIEUX...", "     OH, DIOS MIO...", "\n       OH, GOTT IM HIMMEL..." },
};

static const struct ComicBlock block_p7_1 = { &poly_p7_b1, -1, -1, block_p7_b1_boxes, 1 };

static const struct ComicBlock page_7_blocks[] = {
	block_p7_0,
	block_p7_1,
};

static struct ComicPage page_7 = { "COMDATA/SEPG07.SCN", 98, 59, 1, 0, page_7_blocks, 2 };

const struct ComicPage *SpecialComicPages[] = {
	&page_1,
	&page_2,
	&page_3,
	&page_4,
	&page_5,
	&page_6,
	&page_7,
	NULL
};

} // end namespace Noctropolis

} // end namespace Access
