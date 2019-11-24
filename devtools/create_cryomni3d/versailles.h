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

#ifndef VERSAILLES_H
#define VERSAILLES_H

// This file contains static data and should be included only once

#define VERSAILLES_GAMEID MKTAG('V', 'R', 'S', 'L')
#define VERSAILLES_VERSION 1

struct SubtitleEntry {
	uint32 frameStart;
	char const *const text;
};

#define MAX_SUBTITLE_ENTRIES 256
struct Subtitle {
	char const *const videoName;
	SubtitleEntry const entries[MAX_SUBTITLE_ENTRIES];
};

#define VERSAILLES_LOCALIZED_FILENAMES_COUNT 5
static char const *const versaillesFRlocalizedFilenames[] = {
	"DIALOG1.GTO",
	"tous_doc.txt",
	"lien_doc.txt",
	"credits.txt",
	"LEB001__.WAV",
};
static char const *const versaillesDElocalizedFilenames[] = {
	"DIALOG1.ALM",
	"tous_doc.ALM",
	"lien_doc.alm",
	"credits.ALM",
	"LEB1_ALM.WAV",
};
static char const *const versaillesENlocalizedFilenames[] = {
	"DIALOG1.GB",
	"tous_doc.gb",
	"lien_doc.txt",
	"credits.GB",
	"LEB1_GB.WAV",
};
static char const *const versaillesESlocalizedFilenames[] = {
	"DIALOG1.SP",
	"tous_doc.SP",
	"lien_doc.txt",
	"credits.SP",
	"LEB1_SP.WAV",
};
static char const *const versaillesITlocalizedFilenames[] = {
	"DIALOG1.ITA",
	"tous_doc.ita",
	"lien_doc.txt",
	"credits.ita",
	"LEB1_IT.WAV",
};
static char const *const versaillesBRlocalizedFilenames[] = {
	"DIALOG1.BR",
	"tous_doc.BR",
	"lien_doc.txt",
	"credits.BR",
	"LEB1_BR.WAV",
};
static char const *const versaillesKOlocalizedFilenames[] = {
	"DIALOG1.KR",
	"tous_doc.KR",
	"lien_doc.txt",
	"credits.KR",
	"LEB1_GB.WAV",
};
static char const *const versaillesZTlocalizedFilenames[] = {
	"DIALOG1.TW",
	"tous_doc.TW",
	"lien_doc.txt",
	"credits.TW",
	"LEB1_GB.WAV",
};

static char const versaillesFREpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesFREpilPwd[] = "LELOUPETLATETE";
static char const versaillesBREpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesBREpilPwd[] = "ARAPOSAEACEGONHA";
static char const versaillesDEEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesDEEpilPwd[] = "DIEENTENUNDDERPUDEL";
static char const versaillesENEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesENEpilPwd[] = "FOXANDCRANE";
static char const versaillesESEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesESEpilPwd[] = "ELZORROYLAGRULLA";
static char const versaillesITEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesITEpilPwd[] = "LEANATREEILCANE";
static char const versaillesKOEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesKOEpilPwd[] = "FOXANDCRANE";
static char const versaillesZTEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesZTEpilPwd[] = "FOXANDCRANE";

static char const versaillesFRBombPwd[] = "JEMENVAISMAISLETATDEMEURERATOUJOURS";
static char const versaillesBRBombPwd[] = "O PODER DE UM REI NAO O TORNA IMORTAL";
static char const versaillesDEBombPwd[] =
    "MONARCHEN IST ES NICHT GEGEBEN VOLLKOMMENHEIT ZU ERREICHEN";
static char const versaillesENBombPwd[] = "IT IS NOT IN THE POWER OF KINGS TO ATTAIN PERFECTION";
static char const versaillesESBombPwd[] = "NO ES PODER DE REYES EL ALCANZAR LA PERFECCION";
static char const versaillesITBombPwd[] = "AI SOVRANI NON E DATO RAGGIUNGERE LA PERFEZIONE";
static char const versaillesKOBombPwd[] = "IT IS NOT IN THE POWER OF KINGS TO ATTAIN PERFECTION";
static char const versaillesZTBombPwd[] = "IT IS NOT IN THE POWER OF KINGS TO ATTAIN PERFECTION";

#define VERSAILLES_MESSAGES_COUNT 146
#define VERSAILLES_MESSAGES_COUNT_CJK 151
#define VERSAILLES_PAINTINGS_COUNT 48

static char const *const versaillesFRmessages[] = {
	"Il est interdit d'ouvrir cette porte pour l'instant.",               /*   0 */
	"Cette porte est ferm" "\x8e" "e " "\x88" " clef.",                   /*   1 */
	"Cette porte est ferm" "\x8e" "e.",                                   /*   2 */
	"Ce tiroir est vide.",                                                /*   3 */
	"Vous ne pouvez pas atteindre la b" "\x89" "che.",                    /*   4 */
	"Il n'y a rien dans cet oranger",                                     /*   5 */
	"Ceci n'est pas un oranger!",                                         /*   6 */
	"Il fait trop sombre. ",                                              /*   7 */
	"Le coffre est ferm" "\x8e" ". ",                                     /*   8 */
	"Vous pouvez ouvrir la porte",                                        /*   9 */
	"Il faudrait quelque chose pour atteindre la bombe.",                 /*  10 */
	"Ce vase est vide.",                                                  /*  11 */
	"Maintenant, vous pouvez y aller.",                                   /*  12 */
	"Vous n" "\xd5" "avez plus le temps de vous renseigner sur la "
	"Cour!",                                                              /*  13 */
	"Il est trop tard pour regarder les tableaux!",                       /*  14 */
	"Attendez ! Transmettez donc vos indices " "\x88" " l'huissier.",     /*  15 */
	"Vous ne pouvez pas atteindre le papier.",                            /*  16 */
	"Vers l'apothicairerie",                                              /*  17 */
	"Attention : Vous allez pouvoir terminer ce niveau, mais vous "
	"n'avez pas effectu" "\x8e" " toutes les actions necessaires pour "
	"la suite. Il est conseill" "\x8e" " de SAUVEGARDER votre partie "
	"maintenant.",                                                        /*  18 */
	"Attention : Vous allez pouvoir terminer ce niveau, mais vous "
	"n'avez peut-" "\x90" "tre pas effectu" "\x8e" " toutes les "
	"actions necessaires pour la suite. Il est conseill" "\x8e" " de "
	"SAUVEGARDER votre partie maintenant.",                               /*  19 */
	"Vous ne pouvez pas vous d" "\x8e" "placer en portant une " "\x8e"
	"chelle!",                                                            /*  20 */
	"Il n'y a plus rien ici",                                             /*  21 */
	"Au revoir ...",                                                      /*  22 */
	"VERSAILLES,",                                                        /*  23 */
	"Complot " "\x88" " la Cour du Roi Soleil",                           /*  24 */
	"Consulter l'espace documentaire",                                    /*  25 */
	"           Reprendre la partie en cours",                            /*  26 */
	"           Commencer une nouvelle partie",                           /*  27 */
	"           Charger une partie",                                      /*  28 */
	"           Sauver la partie",                                        /*  29 */
	"           Afficher les sous-titres : OUI",                          /*  30 */
	"           Afficher les sous-titres : NON",                          /*  31 */
	"           Musique : OUI",                                           /*  32 */
	"           Musique : NON",                                           /*  33 */
	"           Une seule musique sur disque dur (20 Mo)",                /*  34 */
	"           Toutes les musiques sur disque dur (92 Mo)",              /*  35 */
	"           Aucune musique sur disque dur (lecture CD)",              /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"Volume",                                                             /*  39 */
	"Quitter le jeu",                                                     /*  40 */
	"",                                                                   /*  41 */
	"Visiter le ch" "\x89" "teau",                                        /*  42 */
	"Cr" "\x8e" "dits",                                                   /*  43 */
	"           Reprendre la visite en cours",                            /*  44 */
	"           Sauver la visite",                                        /*  45 */
	"           Charger une visite",                                      /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D : normal",                                         /*  48 */
	"           Omni3D : lent",                                           /*  49 */
	"           Omni3D : tr" "\x8f" "s lent",                             /*  50 */
	"           Omni3D : rapide",                                         /*  51 */
	"           Omni3D : tr" "\x8f" "s rapide",                           /*  52 */
	"Confirmer",                                                          /*  53 */
	"Annuler",                                                            /*  54 */
	"libre",                                                              /*  55 */
	"sans nom",                                                           /*  56 */
	"Attention : la partie en cours va " "\x90" "tre abandonn" "\x8e"
	"e.",                                                                 /*  57 */
	"Retour",                                                             /*  58 */
	"Le chateau",                                                         /*  59 */
	"Retour Menu Principal",                                              /*  60 */
	"Sommaire Espace documentaire",                                       /*  61 */
	"Plan du ch" "\x89" "teau et des jardins",                            /*  62 */
	"Plan des int" "\x8e" "rieurs du ch" "\x89" "teau",                   /*  63 */
	"Probl" "\x8f" "me d'" "\x8e" "criture sur dique dur : disque "
	"plein ",                                                             /*  64 */
	nullptr,                                                              /*  65 */
	"Veuillez ins" "\x8e" "rer le CD ",                                   /*  66 */
	"Veuillez ins" "\x8e" "rer le CD %d et presser une touche",           /*  67 */
	"Les arts",                                                           /*  68 */
	"Le r" "\x8f" "gne",                                                  /*  69 */
	"La Cour",                                                            /*  70 */
	"Vie de Ch" "\x89" "teau",                                            /*  71 */
	"Le ch" "\x89" "teau et les jardins",                                 /*  72 */
	"Chronologie",                                                        /*  73 */
	"Bassin d'Apollon",                                                   /*  74 */
	"Le Ch" "\x89" "teau",                                                /*  75 */
	"Colonnade",                                                          /*  76 */
	"Labyrinthe",                                                         /*  77 */
	"Latone",                                                             /*  78 */
	"Orangerie",                                                          /*  79 */
	"Parterre d'eau",                                                     /*  80 */
	"Tapis vert",                                                         /*  81 */
	"Grandes Ecuries",                                                    /*  82 */
	"Petites Ecuries",                                                    /*  83 */
	"Les jardins",                                                        /*  84 */
	"Avant cour",                                                         /*  85 */
	"Grand Canal",                                                        /*  86 */
	"Parterre du Midi",                                                   /*  87 */
	"Parterre du nord",                                                   /*  88 */
	"Potager du Roi",                                                     /*  89 */
	"Salle de bal",                                                       /*  90 */
	"Bassin de Neptune",                                                  /*  91 */
	"Pi" "\x8f" "ce d'eau des suisses",                                   /*  92 */
	"Aiguilles (Inutile!)",                                               /*  93 */
	"Ciseaux",                                                            /*  94 */
	"Papier",                                                             /*  95 */
	"Pamphlet sur les arts",                                              /*  96 */
	"Petite clef 1",                                                      /*  97 */
	"Papier r" "\x8e" "v" "\x8e" "l" "\x8e",                              /*  98 */
	"Papier t" "\x89" "ch" "\x8e",                                        /*  99 */
	"Papier du coffre",                                                   /* 100 */
	"Pamphlet sur la lign" "\x8e" "e royale",                             /* 101 */
	"Bougie allum" "\x8e" "e",                                            /* 102 */
	"Bougie",                                                             /* 103 */
	"Clef ",                                                              /* 104 */
	"Carton " "\x88" " dessin",                                           /* 105 */
	"Carton " "\x88" " dessin",                                           /* 106 */
	"Fausse esquisse",                                                    /* 107 */
	"Echelle",                                                            /* 108 */
	"Esquisse d" "\x8e" "truite",                                         /* 109 */
	"pinceau",                                                            /* 110 */
	"pinceau Or",                                                         /* 111 */
	"pinceau Rouge",                                                      /* 112 */
	"Fusain",                                                             /* 113 */
	"Papier",                                                             /* 114 */
	"Pamphlet sur l" "\xd5" "architecture",                               /* 115 */
	"Petite clef 2",                                                      /* 116 */
	"Archer(inutile!)",                                                   /* 117 */
	"Partition",                                                          /* 118 */
	"Queue de billard",                                                   /* 119 */
	"Autorisation",                                                       /* 120 */
	"Reproduction des m" "\x8e" "dailles",                                /* 121 */
	"Tiroir " "\x88" " m" "\x8e" "dailles",                               /* 122 */
	"Clef de la petite porte d" "\xd5" "Apollon",                         /* 123 */
	"Nourriture",                                                         /* 124 */
	"Pamphlet sur la religion",                                           /* 125 */
	"Epigraphe",                                                          /* 126 */
	"Pamphlet sur le gouvernement",                                       /* 127 */
	"Plume",                                                              /* 128 */
	"Pense-b" "\x90" "te",                                                /* 129 */
	"Lunette",                                                            /* 130 */
	"Plan Vauban",                                                        /* 131 */
	"Plan Vauban",                                                        /* 132 */
	"Cordon",                                                             /* 133 */
	"Gravure",                                                            /* 134 */
	"Petite clef 3",                                                      /* 135 */
	"Petite clef 4",                                                      /* 136 */
	"M" "\x8e" "morandum",                                                /* 137 */
	"Plans du chateau",                                                   /* 138 */
	"Plans du chateau",                                                   /* 139 */
	"Clef des combles",                                                   /* 140 */
	"Fables",                                                             /* 141 */
	"Plan du Labyrinthe",                                                 /* 142 */
	"Outil",                                                              /* 143 */
	"M" "\x8e" "dicament",                                                /* 144 */
	"Eteignoir",                                                          /* 145 */
};

static char const *const versaillesFRpaintings[] = {
	"\"Entr" "\x8e" "e des animaux dans l'arche\"\rGerolamo Bassano",     /*  0: 41201 */
	"\"Le repas d'Emma" "\x9f" "s\"\rJacopo Bassano",                     /*  1: 41202 */
	"\"La Madeleine aux pieds de J" "\x8e" "sus Christ\"\rSustris",       /*  2: 41203 */
	"\"La sortie de l'arche\"\rGerolamo Bassano ",                        /*  3: 41204 */
	"\"Le frappement du rocher\"\rJacopo Bassano",                        /*  4: 41205 */
	"\"La Bataille d'Arbelles\"\rJoseph Parrocel",                        /*  5: 41301 */
	"\"Alexandre Le Grand vainqueur de Darius " "\x88" " la bataille "
	"d'Arbelles\"\rLe Bourguignon",                                       /*  6: 41302 */
	"\"Le Combat de Leuze\"\rJoseph Parrocel",                            /*  7: 42401 */
	"\"Sainte C" "\x8e" "cile avec un ange tenant une partition "
	"musicale\"\rDominiquin",                                             /*  8: 42901 */
	"\"Don Francisco du Moncada \"\rVan Dyck",                            /*  9: 42902 */
	"\"Le Petit Saint Jean Baptiste\"\rLe Carrache",                      /* 10: 42903 */
	"\"Saint Mathieu\"\rValentin",                                        /* 11: 42904 */
	"\"Le Denier de C" "\x8e" "sar \"\rValentin",                         /* 12: 42905 */
	"\"Saint Luc\"\rValentin",                                            /* 13: 42906 */
	"\"Le mariage mystique de Sainte Catherine\"\r Alessandro Turchi",    /* 14: 42907 */
	"\"R" "\x8e" "union de buveurs\"\rNicolas Tournier",                  /* 15: 42908 */
	"\"La diseuse de Bonne aventure \"\rValentin",                        /* 16: 42909 */
	"\"le roi David jouant de la harpe \"\rDominiquin",                   /* 17: 42910 */
	"\"Sainte Madeleine\"\rDominiquin",                                   /* 18: 42911 */
	"\"Autoportrait \"\rVan Dyck",                                        /* 19: 42912 */
	"\"Saint Jean l'" "\x8e" "vang" "\x8e" "liste\"\r Valentin",          /* 20: 42913 */
	"\"Agar secouru par un ange \"\rGiovanni Lanfranco",                  /* 21: 42914 */
	"\"Saint Marc \"\rValentin",                                          /* 22: 42915 */
	"\"M" "\x8e" "l" "\x8e" "agre ayant " "\x88" " ses pieds la hure "
	"du sanglier de Calydon\"\r Jacques Rousseau",                        /* 23: 43090 */
	"\"Le Roi en costume romain\"\rJean Warin",                           /* 24: 43091 */
	"\"attalante\"\rJacques Rousseau",                                    /* 25: 43092 */
	"\"En" "\x8e" "e portant Anchise\"\rSpada",                           /* 26: 43100 */
	"\"David et Bethsab" "\x8e" "e\"\rV" "\x8e" "ron" "\x8f" "se",        /* 27: 43101 */
	"\"La fuite en Egypte\"\rGuido R" "\x8e" "ni ",                       /* 28: 43102 */
	"\"Louis XIV " "\x88" " cheval\"\rPierre Mignard",                    /* 29: 43103 */
	"\"La magnificience royale & le progr" "\x8f" "s des beaux "
	"arts\"\rHouasse",                                                    /* 30: 43104 */
	"\"Le Sacrifice d'Iphig" "\x8e" "nie\"\rCharles de la Fosse",         /* 31: 43130 */
	"\"Buste de Louis XIV\"\rsculpt" "\x8e" " par le Chevalier Bernin ",  /* 32: 43131 */
	"\"Diane d" "\x8e" "couvrant son berger Endymion endormi dans les "
	"bras de Morph" "\x8e" "e\"\rGabriel Blanchard",                      /* 33: 43132 */
	"\"La vierge & Saint Pierre\"\rGuerchin",                             /* 34: 43140 */
	"\"Les P" "\x8e" "lerins d'Emma" "\x9f" "s\"\rV" "\x8e" "ron"
	"\x8f" "se",                                                          /* 35: 43141 */
	"\"La sainte Famille\"\rV" "\x8e" "ron" "\x8f" "se",                  /* 36: 43142 */
	"\"La famille de Darius aux pieds d'Alexandre\"\rCharles LeBrun",     /* 37: 43143 */
	"\"Saint Jean-Baptiste\"\rRapha" "\x91" "l",                          /* 38: 43144 */
	"\"Marie de m" "\x8e" "dicis\"\rVan Dyck",                            /* 39: 43150 */
	"\"Hercule luttant contre Achelous\"\rGuido R" "\x8e" "ni",           /* 40: 43151 */
	"\"Le Centaure Nessus porte Dejanire\"\rGuido R" "\x8e" "ni",         /* 41: 43152 */
	"\"Saint Fran" "\x8d" "ois d'Assise r" "\x8e" "confort" "\x8e" " "
	"apr" "\x8f" "s sa stigmatisation\"\rSeghers",                        /* 42: 43153 */
	"\"Thomiris faisant tremper la t" "\x90" "te de Cyrus dans le "
	"sang\"\rRubens",                                                     /* 43: 43154 */
	"\"Hercule tuant l'Hydre\"\rGuido R" "\x8e" "ni",                     /* 44: 43155 */
	"\"Hercule sur le b" "\x9e" "cher\"\rGuido R" "\x8e" "ni",            /* 45: 43156 */
	"\"Portrait du Prince Palatin & de son fr" "\x8f" "re le Prince "
	"Robert\"\rVan Dyck",                                                 /* 46: 43157 */
	"\"La descente de Croix \"\rCharles Lebrun",                          /* 47: 45260 */
};

static char const *const versaillesBRmessages[] = {
	"No momento " "\x8e" " proibido abrir esta porta.",                   /*   0 */
	"Esta porta est" "\x87" " trancada.",                                 /*   1 */
	"Esta porta est" "\x87" " trancada.",                                 /*   2 */
	"Esta gaveta est" "\x87" " vazia.",                                   /*   3 */
	"Voc" "\x90" " n" "\x8b" "o pode alcan" "\x8d" "ar o toldo.",         /*   4 */
	"N" "\x8b" "o h" "\x87" " nada nesta laranjeira.",                    /*   5 */
	"Isto n" "\x8b" "o " "\x8e" " uma laranjeira",                        /*   6 */
	"Est" "\x87" " escuro demais. ",                                      /*   7 */
	"O ba" "\x9c" " est" "\x87" " trancado. ",                            /*   8 */
	"Voc" "\x90" " pode abrir a porta.",                                  /*   9 */
	"Voc" "\x90" " precisar" "\x87" " de algo para que consiga alcan" ""
	"\x8d" "ar a bomba.",                                                 /*  10 */
	"Este vaso est" "\x87" " vazio.",                                     /*  11 */
	"Agora voc" "\x90" " pode passar.",                                   /*  12 */
	"Voc" "\x90" " n" "\x8b" "o tem  tempo para aprender sobre a "
	"Corte!",                                                             /*  13 */
	"" "\x83" " tarde demais para olhar para quadros!",                   /*  14 */
	"Espere! Entregue suas pistas ao criado.",                            /*  15 */
	"Voc" "\x90" " n" "\x8b" "o pode alcan" "\x8d" "ar o papel.",         /*  16 */
	"Para a botica.",                                                     /*  17 */
	"Aviso: Voc" "\x90" " poder" "\x87" " passar para o fim deste n" ""
	"\x92" "vel,  mas ainda n" "\x8b" "o executou todas as a" "\x8d" "" ""
	"\x9b" "es necess" "\x87" "rias para continuar. " "\x83" " melhor "
	"que voc" "\x90" " SALVE este jogo agora.",                           /*  18 */
	"Aviso: Voc" "\x90" " poder" "\x87" " passar para o fim deste n" ""
	"\x92" "vel,  mas pode ainda n" "\x8b" "o ter executado todas as a" ""
	"\x8d" "" "\x9b" "es necess" "\x87" "rias para continuar. " "\x83" ""
	" melhor que voc" "\x90" " SALVE este jogo agora.",                   /*  19 */
	"Voc" "\x90" " n" "\x8b" "o pode se mover enquanto estiver "
	"carregando uma escada!",                                             /*  20 */
	"N" "\x8b" "o h" "\x87" " mais nada aqui.",                           /*  21 */
	"Adeus ...",                                                          /*  22 */
	"VERSAILLES,",                                                        /*  23 */
	"Intriga na Corte de Luis XIV",                                       /*  24 */
	"Consulte a zona de documenta" "\x8d" "" "\x8b" "o.",                 /*  25 */
	"           Continuar este jogo",                                     /*  26 */
	"           Come" "\x8d" "ar um novo jogo ",                          /*  27 */
	"           Carregar um jogo",                                        /*  28 */
	"           Salvar este jogo",                                        /*  29 */
	"           Mostrar legendas: SIM",                                   /*  30 */
	"           Mostrar legendas: N" "\xcc" "O",                          /*  31 */
	"           M" "\x9c" "sica: SIM",                                    /*  32 */
	"           M" "\x9c" "sica: N" "\xcc" "O",                           /*  33 */
	"           Parte da m" "\x9c" "sica para o Disco R" "\x92" "gido "
	"(20 Mb)",                                                            /*  34 */
	"           Toda a m" "\x9c" "sica para o Disco R" "\x92" "gido "
	"(92 Mb)",                                                            /*  35 */
	"           Nenhuma m" "\x9c" "sica para o Disco R" "\x92" "gido "
	"(ler do CD)",                                                        /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"Volume",                                                             /*  39 */
	"Sair do jogo",                                                       /*  40 */
	"",                                                                   /*  41 */
	"Visite o Castelo",                                                   /*  42 */
	"Cr" "\x8e" "ditos",                                                  /*  43 */
	"           Continuar esta visita",                                   /*  44 */
	"           Salvar esta visita",                                      /*  45 */
	"           Carregar uma visita",                                     /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D: normal",                                          /*  48 */
	"           Omni3D: devagar",                                         /*  49 */
	"           Omni3D: muito devagar",                                   /*  50 */
	"           Omni3D: r" "\x87" "pido",                                 /*  51 */
	"           Omni3D: muito r" "\x87" "pido",                           /*  52 */
	"Confirmar",                                                          /*  53 */
	"Cancelar",                                                           /*  54 */
	"livre",                                                              /*  55 */
	"sem nome ",                                                          /*  56 */
	"Aviso: este jogo est" "\x87" " prestes a ser abortado.",             /*  57 */
	"Retornar",                                                           /*  58 */
	"O Castelo",                                                          /*  59 */
	"Retornar ao Menu Principal",                                         /*  60 */
	"Sum" "\x87" "rio da Zona de Documenta" "\x8d" "" "\x8b" "o ",        /*  61 */
	"Mapa do Castelo e Jardins",                                          /*  62 */
	"Planta do interior do Castelo",                                      /*  63 */
	"" "\x83" " imposs" "\x92" "vel gravar no disco r" "\x92" "gido: "
	"disco cheio",                                                        /*  64 */
	nullptr,                                                              /*  65 */
	"Insira o CD ",                                                       /*  66 */
	"Insira o  CD n. %d e pressione uma tecla. ",                         /*  67 */
	"As Artes",                                                           /*  68 */
	"O Reino",                                                            /*  69 */
	"A Corte ",                                                           /*  70 */
	"A Vida no Castelo",                                                  /*  71 */
	"O Castelo e os Jardins ",                                            /*  72 */
	"Cronologia",                                                         /*  73 */
	"A Fonte de Apollo",                                                  /*  74 */
	"O Castelo",                                                          /*  75 */
	"Colunata",                                                           /*  76 */
	"Labirinto",                                                          /*  77 */
	"Latona",                                                             /*  78 */
	"Laranjal",                                                           /*  79 */
	"Espelhos de " "\x87" "gua",                                          /*  80 */
	"Tapis vert",                                                         /*  81 */
	"Grandes Est" "\x87" "bulos",                                         /*  82 */
	"Pequenos Est" "\x87" "bulos ",                                       /*  83 */
	"Os jardins ",                                                        /*  84 */
	"Frente da Corte",                                                    /*  85 */
	"Grande Canal",                                                       /*  86 */
	"Canteiro Sul",                                                       /*  87 */
	"Canteiro Norte ",                                                    /*  88 */
	"O Jardim de Vegetais do Rei",                                        /*  89 */
	"Sal" "\x8b" "o de Dan" "\x8d" "a",                                   /*  90 */
	"A Fonte de Netuno",                                                  /*  91 */
	"O Lago dos Guardas Su" "\x92" "" "\x8d" "os",                        /*  92 */
	"Agulhas (In" "\x9c" "teis!)",                                        /*  93 */
	"Tesouras",                                                           /*  94 */
	"Papel",                                                              /*  95 */
	"Panfleto sobre as artes ",                                           /*  96 */
	"Chave pequena 1",                                                    /*  97 */
	"Papel transformado",                                                 /*  98 */
	"Papel manchado",                                                     /*  99 */
	"Papel do ba" "\x9c" "",                                              /* 100 */
	"Panfleto sobre a linhagem real ",                                    /* 101 */
	"Vela acesa ",                                                        /* 102 */
	"Vela",                                                               /* 103 */
	"Chave ",                                                             /* 104 */
	"Pasta de desenhos ",                                                 /* 105 */
	"Pasta de desenhos ",                                                 /* 106 */
	"Esbo" "\x8d" "o falso ",                                             /* 107 */
	"Escada",                                                             /* 108 */
	"Esbo" "\x8d" "o destru" "\x92" "do ",                                /* 109 */
	"escova",                                                             /* 110 */
	"escova dourada ",                                                    /* 111 */
	"escova",                                                             /* 112 */
	"L" "\x87" "pis de carv" "\x8b" "o",                                  /* 113 */
	"Papel",                                                              /* 114 */
	"Panfleto sobre arquitetura",                                         /* 115 */
	"Chave pequena 2",                                                    /* 116 */
	"Arqueiro (in" "\x9c" "til!)",                                        /* 117 */
	"Partitura musical ",                                                 /* 118 */
	"Taco de bilhar ",                                                    /* 119 */
	"Autoriza" "\x8d" "" "\x8b" "o",                                      /* 120 */
	"Reprodu" "\x8d" "" "\x8b" "o de medalhas",                           /* 121 */
	"Gaveta com  medalhas",                                               /* 122 */
	"Chave para a pequena porta de Apollo ",                              /* 123 */
	"Comida",                                                             /* 124 */
	"Panfleto sobre religi" "\x8b" "o ",                                  /* 125 */
	"Ep" "\x92" "grafe",                                                  /* 126 */
	"Panfleto sobre o governo ",                                          /* 127 */
	"Pena",                                                               /* 128 */
	"Memorandum",                                                         /* 129 */
	"Telesc" "\x97" "pio",                                                /* 130 */
	"Planta de Vauban ",                                                  /* 131 */
	"Planta de Vauban ",                                                  /* 132 */
	"Cord" "\x8b" "o",                                                    /* 133 */
	"Gravura",                                                            /* 134 */
	"Chave Pequena 3",                                                    /* 135 */
	"Chave Pequena 4",                                                    /* 136 */
	"Memorandum",                                                         /* 137 */
	"Plantas do Castelo",                                                 /* 138 */
	"Plantas do Castelo",                                                 /* 139 */
	"Chave para o s" "\x97" "t" "\x8b" "o",                               /* 140 */
	"F" "\x87" "bulas",                                                   /* 141 */
	"Mapa do Labirinto",                                                  /* 142 */
	"Ferramenta",                                                         /* 143 */
	"Medicamentos",                                                       /* 144 */
	"Apagador de velas ",                                                 /* 145 */
};

static char const *const versaillesBRpaintings[] = {
	"\"Os Animais entrando na Arca \"\rGerolamo Bassano",                 /*  0: 41201 */
	"\"O Lanche em Emmaus\"\rJacopo Bassano",                             /*  1: 41202 */
	"\"Maria Madalena aos p" "\x8e" "s de Cristo \"\r",                   /*  2: 41203 */
	"\"Saindo da Arca \"\rGerolamo Bassano",                              /*  3: 41204 */
	"\"Atingindo a Pedra\"\rJacopo Bassano",                              /*  4: 41205 */
	"\"A Batalha de Arbelles\"\rJoseph Parrocel",                         /*  5: 41301 */
	"\"Alexandre o Grande, Vencedor de Darisu na batalha de "
	"Arbelles\"\rLe Bourguignon",                                         /*  6: 41302 */
	"\"O Combate de Leuze\"\rJoseph Parrocel",                            /*  7: 42401 */
	"\" Santa Cecilia com um anjo segurando uma nota" "\x8d" "" "\x8b" ""
	"o musical \"\rIl Domenichino",                                       /*  8: 42901 */
	"\" Don Francisco de Moncada \"\rVan Dyck",                           /*  9: 42902 */
	"\"O jovem Jo" "\x8b" "o Batista \"\rCarracci",                       /* 10: 42903 */
	"\" S" "\x8b" "o Mateus \"\rValentin",                                /* 11: 42904 */
	"\" Rever" "\x90" "ncia a C" "\x8e" "sar \"\rValentin",               /* 12: 42905 */
	"\"S" "\x8b" "o Lucas \"\rValentin",                                  /* 13: 42906 */
	"\" O Casamento M" "\x92" "stico de Santa Catarina \"\r Alessandro "
	"Turchi",                                                             /* 14: 42907 */
	"\" O Encontro dos Bebedores \"\rNicolas Tournier",                   /* 15: 42908 */
	"\" O Adivinho \"\rValentin",                                         /* 16: 42909 */
	"\" Rei David  tocando a Harpa \"\rIl Domenichino",                   /* 17: 42910 */
	"\" Maria Madalena \"\rIl Domenichino",                               /* 18: 42911 */
	"\" Auto-retrato \"\rVan Dyck",                                       /* 19: 42912 */
	"\" S" "\x8b" "o Jo" "\x8b" "o, o Evangelista \"\r Valentin",         /* 20: 42913 */
	"\"Hagar sendo ajudado por um anjo \"\rGiovanni Lanfranco",           /* 21: 42914 */
	"\" S" "\x8b" "o Marcos \"\rValentin",                                /* 22: 42915 */
	"\" Meleager com a cabe" "\x8d" "a do javali de Calydon a seus p" ""
	"\x8e" "s \"\r Jacques Rousseau",                                     /* 23: 43090 */ /* BUG: Switched */
	"\" O Rei em vestes Romanas \"\rJean Warin",                          /* 24: 43091 */
	"\" Atalanta \"\rJacques Rousseau",                                   /* 25: 43092 */
	"\" Aeneas carregando Anchises \"\rSpada",                            /* 26: 43100 */
	"\" David e Bethsheba \"\rVeronese",                                  /* 27: 43101 */
	"\" O V" "\x99" "o ao Egito \"\rGuido Reni",                          /* 28: 43102 */
	"\" Luis XIV no dorso de um cavalo \"\rPierre Mignard",               /* 29: 43103 */
	"\"A  Magnific" "\x90" "ncia Real e o Progresso das Belas Artes "
	"\"\rHouasse",                                                        /* 30: 43104 */
	"\" O Sacrif" "\x92" "cio de Iphigeneia \"\rCharles de La Fosse",     /* 31: 43130 */
	"\" Busto de Luis XIV \"\resculpido por Bernini",                     /* 32: 43131 */
	"\" Diana surpreende Endymion nos bra" "\x8d" "os de Morpheus "
	"\"\rGabriel Blanchard",                                              /* 33: 43132 */
	"\" S" "\x8b" "o Paulo e a Virgem Maria \"\rIl Guercino",             /* 34: 43140 */
	"\" Os Disc" "\x92" "pulos de Emmaus \"\rVeronese",                   /* 35: 43141 */
	"\" A Fam" "\x92" "lia Sagrada \"\rVeronese",                         /* 36: 43142 */
	"\" A fam" "\x92" "lia de Darius aos p" "\x8e" "s de Alexandre "
	"\"\rCharles Le Brun",                                                /* 37: 43143 */
	"\" S" "\x8b" "o Jo" "\x8b" "o Batista \"\rRaphael",                  /* 38: 43144 */
	"\" Maria de Medici \"\rVan Dyck",                                    /* 39: 43150 */
	"\" H" "\x8e" "rcules lutando com Achelous \"\rGuido Reni",           /* 40: 43151 */
	"\" O Centauro Nessus levando Deianeira embora. \"\rGuido Reni",      /* 41: 43152 */
	"\" S" "\x8b" "o Francisco de Assis confortado ap" "\x97" "s "
	"receber a stigmata \"\rSeghers",                                     /* 42: 43153 */
	"\" Thomiris mergulhando a cabe" "\x8d" "a de Cyrus em sangue "
	"\"\rRubens",                                                         /* 43: 43154 */
	"\" H" "\x8e" "rcules Matando a Hidra \"\rGuido Reni",                /* 44: 43155 */
	"\" H" "\x8e" "rcules em chamas  \"\rGuido Reni",                     /* 45: 43156 */
	"\" Retrato do Pr" "\x92" "ncipe Palatino & seu irm" "\x8b" "o Pr" ""
	"\x92" "ncipe Roberto \"\rVan Dyck",                                  /* 46: 43157 */
	"\" Cristo sendo retirado da Cruz \"\rCharles Le Brun",               /* 47: 45260 */
};

static char const *const versaillesDEmessages[] = {
	"Das " "\x85" "ffnen dieser T" "\x9f" "r ist zur Zeit nicht "
	"gestattet.",                                                         /*   0 */
	"Diese T" "\x9f" "r ist verschlossen.",                               /*   1 */
	"Diese T" "\x9f" "r ist zu.",                                         /*   2 */
	"Diese Schublade ist leer.",                                          /*   3 */
	"Sie k" "\x9a" "nnen die Plane nicht erreichen.",                     /*   4 */
	"In diesem Orangenbaum ist nichts zu finden.",                        /*   5 */
	"Dies hier ist kein Orangenbaum.",                                    /*   6 */
	"Es ist zu dunkel.",                                                  /*   7 */
	"Die Kiste ist verschlossen.",                                        /*   8 */
	"Sie k" "\x9a" "nnen diese T" "\x9f" "r " "\x9a" "ffnen.",            /*   9 */
	"Sie ben" "\x9a" "tigen etwas, um die Bombe zu erreichen.",           /*  10 */
	"Diese Vase ist leer.",                                               /*  11 */
	"Jetzt k" "\x9a" "nnen Sie gehen.",                                   /*  12 */
	"Sie haben keine Zeit mehr, sich " "\x9f" "ber den Hof zu "
	"informieren!",                                                       /*  13 */
	"Es ist zu sp" "\x8a" "t, um sich die Bilder anzusehen.",             /*  14 */
	"Warten Sie! Geben Sie Ihre Hinweise doch an den Diener weiter.",     /*  15 */
	"Sie k" "\x9a" "nnen das Papier nicht erreichen.",                    /*  16 */
	"Zur Apotheke",                                                       /*  17 */
	"Vorsicht! Sie k" "\x9a" "nnen dieses Level beenden, aber Sie "
	"haben noch nicht alle notwendigen Informationen, um "
	"weiterzukommen. Es empfiehlt sich, Ihr Spiel jetzt zu SPEICHERN.",   /*  18 */
	"Vorsicht! Sie k" "\x9a" "nnen dieses Level beenden, aber Sie "
	"haben vielleicht noch nicht alle notwendigen Informationen, um "
	"weiterzukommen. Es empfiehlt sich, Ihr Spiel jetzt zu SPEICHERN.",   /*  19 */
	"Sie k" "\x9a" "nnen sich nicht von der Stelle bewegen, wenn Sie "
	"eine Leiter tragen!",                                                /*  20 */
	"Hier ist nichts mehr zu tun.",                                       /*  21 */
	"Auf Wiedersehen...",                                                 /*  22 */
	"VERSAILLES,",                                                        /*  23 */
	"Verschw" "\x9a" "rung am Hof",                                       /*  24 */
	"Die Enzyklop" "\x8a" "die",                                          /*  25 */
	"           Spiel wieder aufnehmen",                                  /*  26 */
	"           Spiel starten",                                           /*  27 */
	"           Spiel laden",                                             /*  28 */
	"           Spiel speichern",                                         /*  29 */
	"           Anzeige der Untertitel: JA",                              /*  30 */
	"           Anzeige der Untertitel: NEIN",                            /*  31 */
	"           Musik: JA",                                               /*  32 */
	"           Musik: NEIN",                                             /*  33 */
	"           ein Musikst" "\x9f" "ck auf Festplatte kopieren (20 "
	"MB)",                                                                /*  34 */
	"           komplette Musik auf Festplatte kopieren (92 MB)",         /*  35 */
	"           Musik von CD-ROM spielen",                                /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"Lautst" "\x8a" "rke",                                                /*  39 */
	"Programm beenden",                                                   /*  40 */
	"",                                                                   /*  41 */
	"Der Rundgang",                                                       /*  42 */
	"Credits",                                                            /*  43 */
	"           Besichtigung wieder aufnehmen",                           /*  44 */
	"           Rundgang speichern",                                      /*  45 */
	"           Rundgang laden",                                          /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D: normal",                                          /*  48 */
	"           Omni3D: langsam",                                         /*  49 */
	"           Omni3D: sehr langsam",                                    /*  50 */
	"           Omni3D: schnell",                                         /*  51 */
	"           Omni3D: sehr schnell",                                    /*  52 */
	"Best" "\x8a" "tigen",                                                /*  53 */
	"Abbrechen",                                                          /*  54 */
	"Frei",                                                               /*  55 */
	"Ohne Namen",                                                         /*  56 */
	"Vorsicht! Das begonnene Spiel wird beendet.",                        /*  57 */
	"Zur" "\x9f" "ck",                                                    /*  58 */
	"Das Schloss",                                                        /*  59 */
	"Zur" "\x9f" "ck zum Hauptmen" "\x9f",                                /*  60 */
	"Gesamt" "\x9f" "bersicht Enzyklop" "\x8a" "die",                     /*  61 */
	"Pl" "\x8a" "ne von Schloss und G" "\x8a" "rten",                     /*  62 */
	"Pl" "\x8a" "ne der Innenr" "\x8a" "ume des Schlosses",               /*  63 */
	"Problem beim Schreiben auf Festplatte: Nicht genug Speicherplatz",   /*  64 */
	nullptr,                                                              /*  65 */
	"Legen Sie die CD-ROM %d ein",                                        /*  66 */
	"Legen Sie die CD-ROM %d und dr" "\x9f" "cken Sie eine Taste",        /*  67 */
	"Die K" "\x9f" "nste",                                                /*  68 */
	"Die Regierung",                                                      /*  69 */
	"Der Hof",                                                            /*  70 */
	"Leben bei Hofe",                                                     /*  71 */
	"Das Schloss und die G" "\x8a" "rten",                                /*  72 */
	"Chronologie",                                                        /*  73 */
	"Apollobecken",                                                       /*  74 */
	"Das Schloss",                                                        /*  75 */
	"S" "\x8a" "ulengang",                                                /*  76 */
	"Labyrinth",                                                          /*  77 */
	"Latone",                                                             /*  78 */
	"Orangerie",                                                          /*  79 */
	"Wasserterrasse",                                                     /*  80 */
	"Gr" "\x9f" "ner Teppich",                                            /*  81 */
	"Gro" "\xa7" "e Reitst" "\x8a" "lle",                                 /*  82 */
	"Kleine Reitst" "\x8a" "lle",                                         /*  83 */
	"Die G" "\x8a" "rten",                                                /*  84 */
	"Vorhof",                                                             /*  85 */
	"Gro" "\xa7" "er Kanal",                                              /*  86 */
	"S" "\x9f" "dterrasse",                                               /*  87 */
	"Nordterrasse",                                                       /*  88 */
	"K" "\x9a" "niglicher Gem" "\x9f" "segarten",                         /*  89 */
	"Ballsaal",                                                           /*  90 */
	"Neptunbecken",                                                       /*  91 */
	"Wasserraum der Schweizer",                                           /*  92 */
	"Nadeln (Unn" "\x9a" "tig!)",                                         /*  93 */
	"Schere",                                                             /*  94 */
	"Papier",                                                             /*  95 */
	"Schm" "\x8a" "hschrift " "\x9f" "ber die K" "\x9f" "nste",           /*  96 */
	"Kleiner Schl" "\x9f" "ssel 1",                                       /*  97 */
	"Aufgedecktes Papier",                                                /*  98 */
	"Beflecktes Papier",                                                  /*  99 */
	"Papier aus der Kiste",                                               /* 100 */
	"Schm" "\x8a" "hschrift " "\x9f" "ber die k" "\x9a" "nigliche "
	"Ahnenreihe",                                                         /* 101 */
	"Brennende Kerze",                                                    /* 102 */
	"Kerze",                                                              /* 103 */
	"Schl" "\x9f" "ssel",                                                 /* 104 */
	"Zeichenkarton",                                                      /* 105 */
	"Zeichenkarton",                                                      /* 106 */
	"Falsche Skizze",                                                     /* 107 */
	"Leiter",                                                             /* 108 */
	"Zerst" "\x9a" "rte Skizze",                                          /* 109 */
	"Pinsel",                                                             /* 110 */
	"Goldpinsel",                                                         /* 111 */
	"Rotpinsel",                                                          /* 112 */
	"Zeichenkohle",                                                       /* 113 */
	"Papier",                                                             /* 114 */
	"Schm" "\x8a" "hschrift " "\x9f" "ber die Architektur",               /* 115 */
	"Kleiner Schl" "\x9f" "ssel 2",                                       /* 116 */
	"Bogensch" "\x9f" "tze (unn" "\x9a" "tig!)",                          /* 117 */
	"Partitur",                                                           /* 118 */
	"Billardstock",                                                       /* 119 */
	"Genehmigung",                                                        /* 120 */
	"Reproduktion der Medaillen",                                         /* 121 */
	"Medaillen-Schublade",                                                /* 122 */
	"Schl" "\x9f" "ssel zur kleinen Apollo-T" "\x9f" "r",                 /* 123 */
	"Nahrungsmittel",                                                     /* 124 */
	"Schm" "\x8a" "hschrift " "\x9f" "ber die Religion",                  /* 125 */
	"Inschrift",                                                          /* 126 */
	"Schm" "\x8a" "hschrift " "\x9f" "ber die Regierung",                 /* 127 */
	"Feder",                                                              /* 128 */
	"Merkzeichen",                                                        /* 129 */
	"Fernrohr",                                                           /* 130 */
	"Vaubanplan",                                                         /* 131 */
	"Plan Vauban",                                                        /* 132 */
	"Schnur",                                                             /* 133 */
	"Schnitt",                                                            /* 134 */
	"Kleiner Schl" "\x9f" "ssel 3",                                       /* 135 */
	"Kleiner Schl" "\x9f" "ssel 4",                                       /* 136 */
	"Memorandum",                                                         /* 137 */
	"Schlosspl" "\x8a" "ne",                                              /* 138 */
	"Schlosspl" "\x8a" "ne",                                              /* 139 */
	"Schl" "\x9f" "ssel zum Dachstuhl",                                   /* 140 */
	"Fabeln",                                                             /* 141 */
	"Plan zum Labyrinth",                                                 /* 142 */
	"Werkzeug",                                                           /* 143 */
	"Medikament",                                                         /* 144 */
	"Trichter",                                                           /* 145 */
};

static char const *const versaillesDEpaintings[] = {
	"\"Einzug der Tiere in die Arche\"\rGerolamo Bassano",                /*  0: 41201 */
	"\"Das Mahl von Emmaus\"\rJacopo Bassano",                            /*  1: 41202 */
	"\"Magdalena zu F" "\x9f\xa7" "en Jesu Christi\"\rSustris",           /*  2: 41203 */
	"\"Auszug aus der Arche\"\rGerolamo Bassano",                         /*  3: 41204 */
	"\"Schlag gegen den Felsen\"\rJacopo Bassano",                        /*  4: 41205 */
	"\"Die Schlacht von Arbil\"\rJoseph Parrocel",                        /*  5: 41301 */
	"\"Alexander der Gro" "\xa7" "e als Sieger " "\x9f" "ber Darius in "
	"der Schlacht von Arbil\"\rLe Bourguignon",                           /*  6: 41302 */
	"\"Der Kampf des Leukippos\"\rJoseph Parrocel",                       /*  7: 42401 */
	"\"Die heilige C" "\x8a" "cilia mit einem Engel, der ein "
	"Notenblatt h" "\x8a" "lt\"\rDominiquin",                             /*  8: 42901 */
	"\"Don Francisco du Moncada\"\rVan Dyck",                             /*  9: 42902 */
	"\"Der kleine Johannes der T" "\x8a" "ufer\"\rLe Carrache",           /* 10: 42903 */
	"\"Der heilige Matth" "\x8a" "us\"\rValentin",                        /* 11: 42904 */
	"\"Der Denar des C" "\x8a" "sar\"\rValentin",                         /* 12: 42905 */
	"\"Der heilige Lukas\"\rValentin",                                    /* 13: 42906 */
	"\"Die geheimnisvolle Hochzeit der heiligen "
	"Katharina\"\rAlessandro Turchi",                                     /* 14: 42907 */
	"\"Zusammenkunft der Trinker\"\rNicolas Tournier",                    /* 15: 42908 */
	"\"Die Wahrsagerin\"\rValentin",                                      /* 16: 42909 */
	"\"Der K" "\x9a" "nig David beim Harfenspiel\"\rDominiquin",          /* 17: 42910 */
	"\"Die heilige Magdalena\"\rDominiquin",                              /* 18: 42911 */
	"\"Selbstportr" "\x8a" "t\"\rVan Dyck",                               /* 19: 42912 */
	"\"Der heilige Johannes Evangelist\"\rValentin",                      /* 20: 42913 */
	"\"Agar, dem ein Engel Beistand leistet\"\rGiovanni Lanfranco",       /* 21: 42914 */
	"\"Der heilige Markus\"\rValentin",                                   /* 22: 42915 */
	"\"Meleagros mit dem kalydonischen Eberkopf zu seinen F" "\x9f"
	"\xa7" "en\"\rJacques Rousseau",                                      /* 23: 43090 */
	"\"Der K" "\x9a" "nig in r" "\x9a" "mischem Gewand\"\rJean Warin",    /* 24: 43091 */ /* BUG: Switched */
	"\"Attalante\"\rJacques Rousseau",                                    /* 25: 43092 */
	"\"\x80" "neas tr" "\x8a" "gt Anchises\"\rSpada",                     /* 26: 43100 */
	"\"David und Bethsabee\"\rV" "\x8e" "ron" "\x8f" "se",                /* 27: 43101 */
	"\"Die Flucht nach " "\x80" "gypten\"\rGuido R" "\x8e" "ni",          /* 28: 43102 */
	"\"Ludwig XIV. zu Pferd\"\rPierre Mignard",                           /* 29: 43103 */
	"\"Die k" "\x9a" "nigliche Pracht und der Fortschritt der sch"
	"\x9a" "nen K" "\x9f" "nste\"\rHouasse",                              /* 30: 43104 */
	"\"Das Opfer der Iphigenie\"\rCharles de la fosse",                   /* 31: 43130 */
	"\"B" "\x9f" "ste Ludwigs XIV.\"\rSkulptur von Bernini",              /* 32: 43131 */
	"\"Diana entdeckt ihren Hirten Endymion, der in Morpheus' Armen "
	"eingeschlafen ist\"\rGabriel Blanchard",                             /* 33: 43132 */
	"\"Die Jungfrau und der heilige Petrus\"\rGuerchin",                  /* 34: 43140 */
	"\"Die Emmaus-Pilger\"\rV" "\x8e" "ron" "\x8f" "se",                  /* 35: 43141 */
	"\"Die heilige Familie\"\rV" "\x8e" "ron" "\x8f" "se",                /* 36: 43142 */
	"\"Die Familie des Darius zu F" "\x9f\xa7" "en "
	"Alexanders\"\rCharles Le Brun",                                      /* 37: 43143 */
	"\"Der heilige Johannes der T" "\x8a" "ufer\"\rRapha" "\x91" "l",     /* 38: 43144 */
	"\"Maria von Medici\"\rVan Dyck",                                     /* 39: 43150 */
	"\"Herkules k" "\x8a" "mpft gegen Acheloos\"\rGuido R" "\x8e" "ni",   /* 40: 43151 */
	"\"Der Zentaur Nessus tr" "\x8a" "gt Deianeira\"\rGuido R" "\x8e"
	"ni",                                                                 /* 41: 43152 */
	"\"Der heilige Franz von Assisi, nach seiner Stigmatisierung gest"
	"\x8a" "rkt\"\rSeghers",                                              /* 42: 43153 */
	"\"Thomiris taucht den Kopf des Cyrus in Blut\"\rRubens",             /* 43: 43154 */
	"\"Herkules t" "\x9a" "tet Hydra\"\rGuido R" "\x8e" "ni",             /* 44: 43155 */
	"\"Herkules auf dem Scheiterhaufen\"\rGuido R" "\x8e" "ni",           /* 45: 43156 */
	"\"Portr" "\x8a" "t des pf" "\x8a" "lzischen Prinzen und seines "
	"Bruders, Prinz Robert\"\rVan Dyck",                                  /* 46: 43157 */
	"\"Die Kreuzabnahme\"\rCharles Le Brun",                              /* 47: 45260 */
};

static char const *const versaillesENmessages[] = {
	"For the moment it is forbidden to open this door.",                  /*   0 */
	"This door is locked.",                                               /*   1 */
	"This door is locked.",                                               /*   2 */
	"This drawer is empty.",                                              /*   3 */
	"You cannot reach the covering.",                                     /*   4 */
	"There is nothing in this orange tree.",                              /*   5 */
	"This is not an orange tree!",                                        /*   6 */
	"It is too dark. ",                                                   /*   7 */
	"The casket is locked. ",                                             /*   8 */
	"You can open the door.",                                             /*   9 */
	"You will need something to be able to reach the bomb.",              /*  10 */
	"This vase is empty.",                                                /*  11 */
	"Now you can go through.",                                            /*  12 */
	"You have no time to learn about the Court!",                         /*  13 */
	"It is too late for looking at pictures!",                            /*  14 */
	"Wait! Give your clues to the usher.",                                /*  15 */
	"You cannot reach the paper.",                                        /*  16 */
	"To the apothecary's.",                                               /*  17 */
	"Warning: You will be able to come to the end of this level, but "
	"you have not performed all the actions necessary in order to be "
	"able to continue. You are advised to SAVE this game now.",           /*  18 */
	"Warning: You will be able to come to the end of this level, but "
	"you may not have performed all the actions necessary in order to "
	"be able to continue. You are advised to SAVE this game now.",        /*  19 */
	"You cannot move about while carrying a ladder!",                     /*  20 */
	"There is nothing else here.",                                        /*  21 */
	"Goodbye ...",                                                        /*  22 */
	"VERSAILLES,",                                                        /*  23 */
	"A Game of Intrigue at the Court of Louis XIV",                       /*  24 */
	"Consult the documentation zone",                                     /*  25 */
	"           Continue this game",                                      /*  26 */
	"           Start a new game",                                        /*  27 */
	"           Load a game",                                             /*  28 */
	"           Save this game",                                          /*  29 */
	"           Display subtitles: YES",                                  /*  30 */
	"           Display subtitles: NO",                                   /*  31 */
	"           Music: YES",                                              /*  32 */
	"           Music: NO",                                               /*  33 */
	"           One piece of music to hard disk (20 Mo)",                 /*  34 */
	"           All music to hard disk (92 Mo)",                          /*  35 */
	"           No music to hard disk (CD read)",                         /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"Volume",                                                             /*  39 */
	"Quit game",                                                          /*  40 */
	"",                                                                   /*  41 */
	"Visit the chateau",                                                  /*  42 */
	"Credits",                                                            /*  43 */
	"           Continue this visit",                                     /*  44 */
	"           Save this visit",                                         /*  45 */
	"           Load a visit",                                            /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D: standard",                                        /*  48 */
	"           Omni3D: slow",                                            /*  49 */
	"           Omni3D: very slow",                                       /*  50 */
	"           Omni3D: fast",                                            /*  51 */
	"           Omni3D: very fast",                                       /*  52 */
	"Confirm",                                                            /*  53 */
	"Cancel",                                                             /*  54 */
	"free",                                                               /*  55 */
	"nameless ",                                                          /*  56 */
	"Warning: this game is about to be aborted.",                         /*  57 */
	"Return",                                                             /*  58 */
	"The Chateau",                                                        /*  59 */
	"Return to Main Menu",                                                /*  60 */
	"Summary of Documentation Zone",                                      /*  61 */
	"Map of Chateau and gardens",                                         /*  62 */
	"Interior plan of chateau",                                           /*  63 */
	"Impossible to write to hard disk: disk full",                        /*  64 */
	nullptr,                                                              /*  65 */
	"Insert the CD ",                                                     /*  66 */
	"Insert the %d CD and press any key",                                 /*  67 */
	"The Arts",                                                           /*  68 */
	"The Reign",                                                          /*  69 */
	"The Court",                                                          /*  70 */
	"Life in the Chateau",                                                /*  71 */
	"The Chateau and gardens",                                            /*  72 */
	"Chronology",                                                         /*  73 */
	"The Apollo Fountain",                                                /*  74 */
	"The Chateau",                                                        /*  75 */
	"Colonnade",                                                          /*  76 */
	"Maze",                                                               /*  77 */
	"Latona",                                                             /*  78 */
	"Orangery",                                                           /*  79 */
	"Water Parterre",                                                     /*  80 */
	"Tapis vert",                                                         /*  81 */
	"Grand Stables",                                                      /*  82 */
	"Small Stables",                                                      /*  83 */
	"The gardens",                                                        /*  84 */
	"Forecourt",                                                          /*  85 */
	"Grand Canal",                                                        /*  86 */
	"South Parterre",                                                     /*  87 */
	"North Parterre",                                                     /*  88 */
	"The King's vegetable garden",                                        /*  89 */
	"Ballroom",                                                           /*  90 */
	"The Neptune Fountain",                                               /*  91 */
	"The Lake of the Swiss Guards",                                       /*  92 */
	"Aiguilles (Useless!)",                                               /*  93 */
	"Scissors",                                                           /*  94 */
	"Paper",                                                              /*  95 */
	"Pamphlet on the arts",                                               /*  96 */
	"Small key 1",                                                        /*  97 */
	"Transformed paper",                                                  /*  98 */
	"Stained paper",                                                      /*  99 */
	"Paper from casket",                                                  /* 100 */
	"Pamphlet on royal lineage",                                          /* 101 */
	"Lighted candle ",                                                    /* 102 */
	"Candle",                                                             /* 103 */
	"Key ",                                                               /* 104 */
	"Sketch folder",                                                      /* 105 */
	"Sketch folder",                                                      /* 106 */
	"Fake sketch",                                                        /* 107 */
	"Ladder",                                                             /* 108 */
	"Destroyed sketch",                                                   /* 109 */
	"brush",                                                              /* 110 */
	"golden brush",                                                       /* 111 */
	"red brush",                                                          /* 112 */
	"Charcoal pencil",                                                    /* 113 */
	"Paper",                                                              /* 114 */
	"Pamphlet on architecture",                                           /* 115 */
	"Small key 2",                                                        /* 116 */
	"Archer(useless!)",                                                   /* 117 */
	"Musical score",                                                      /* 118 */
	"Billiard cue",                                                       /* 119 */
	"Authorisation",                                                      /* 120 */
	"Reproduction of medals",                                             /* 121 */
	"Drawer with medals",                                                 /* 122 */
	"Key to small Apollo door",                                           /* 123 */
	"Food",                                                               /* 124 */
	"Pamphlet on religion",                                               /* 125 */
	"Epigraph",                                                           /* 126 */
	"Pamphlet on government",                                             /* 127 */
	"Quill",                                                              /* 128 */
	"Memo",                                                               /* 129 */
	"Telescope",                                                          /* 130 */
	"Vauban Plan",                                                        /* 131 */
	"Vauban Plan",                                                        /* 132 */
	"Cord",                                                               /* 133 */
	"Engraving",                                                          /* 134 */
	"Small key 3",                                                        /* 135 */
	"Small key 4",                                                        /* 136 */
	"Memorandum",                                                         /* 137 */
	"Plans of Chateau",                                                   /* 138 */
	"Plans of Chateau",                                                   /* 139 */
	"Key to attic",                                                       /* 140 */
	"Fables",                                                             /* 141 */
	"Plan of Maze",                                                       /* 142 */
	"Tool",                                                               /* 143 */
	"Medicine",                                                           /* 144 */
	"Candle snuffer",                                                     /* 145 */
};

static char const *const versaillesENpaintings[] = {
	"\"The Animals entering the Ark\"\rGerolamo Bassano",                 /*  0: 41201 */
	"\"The Supper at Emmaus\"\rJacopo Bassano",                           /*  1: 41202 */
	"\"Mary Magdalen at the feet of the Christ",                          /*  2: 41203 */
	"\"Leaving the Ark\"\rGerolamo Bassano",                              /*  3: 41204 */
	"\"Striking the Rock\"\rJacopo Bassano",                              /*  4: 41205 */
	"\"The Battle of Arbelles\"\rJoseph Parrocel",                        /*  5: 41301 */
	"\"Alexander the Great, Vanquisher of Darisu at the battle of "
	"Arbelles\"\rLe Bourguignon",                                         /*  6: 41302 */
	"\"The Combat of Leuze\"\rJoseph Parrocel",                           /*  7: 42401 */
	"\"Saint Cecilia with an angel holding musical notation\"\rIl "
	"Domenichino",                                                        /*  8: 42901 */
	"\"Don Francisco of Moncada\"\rVan Dyck",                             /*  9: 42902 */
	"\"The young John the Baptist\"\rCarracci",                           /* 10: 42903 */
	"\"Saint Matthew\"\rValentin",                                        /* 11: 42904 */
	"\"Render unto Caesar\"\rValentin",                                   /* 12: 42905 */
	"\"Saint Luke\"\rValentin",                                           /* 13: 42906 */
	"\"The Mystical Marriage of Saint Catherine\"\r Alessandro Turchi",   /* 14: 42907 */
	"\"The Meeting of the Drinkers\"\rNicolas Tournier",                  /* 15: 42908 */
	"\"The Fortune-teller\"\rValentin",                                   /* 16: 42909 */
	"\"King David playing the Harp\"\rIl Domenichino",                    /* 17: 42910 */
	"\"Mary Magdalen\"\rIl Domenichino",                                  /* 18: 42911 */
	"\"Self-portrait\"\rVan Dyck",                                        /* 19: 42912 */
	"\"Saint John the Evangelist\"\r Valentin",                           /* 20: 42913 */
	"\"Hagar being helped by an angel\"\rGiovanni Lanfranco",             /* 21: 42914 */
	"\"Saint Mark\"\rValentin",                                           /* 22: 42915 */
	"\"Meleager with the head of the boar of Calydon at his "
	"feet\"\rJacques Rousseau",                                           /* 23: 43090 */ /* BUG: Switched */
	"\"The King in Roman dress\"\rJean Warin",                            /* 24: 43091 */
	"\"Atalanta\"\rJacques Rousseau",                                     /* 25: 43092 */
	"\"Aeneas carrying Anchises\"\rSpada",                                /* 26: 43100 */
	"\"David and Bethsheba\"\rVeronese",                                  /* 27: 43101 */
	"\"The Flight into Egypt\"\rGuido Reni",                              /* 28: 43102 */
	"\"Louis XIV on horseback\"\rPierre Mignard",                         /* 29: 43103 */
	"\"Royal Magnificence and the Progress of the Beaux "
	"Arts\"\rHouasse",                                                    /* 30: 43104 */
	"\"The Sacrifice of Iphigeneia\"\rCharles de La Fosse",               /* 31: 43130 */
	"\"Bust of Louis XIV\"\rsculpted by Bernini",                         /* 32: 43131 */
	"\"Diana  discovers Endymion in the arms of Morpheus\"\rGabriel "
	"Blanchard",                                                          /* 33: 43132 */
	"\"Saint Peter and the Virgin Mary\"\rIl Guercino",                   /* 34: 43140 */
	"\"The Disciples at Emmaus\"\rVeronese",                              /* 35: 43141 */
	"\"The Holy Family\"\rVeronese",                                      /* 36: 43142 */
	"\"Darius' family at the feet of Alexander\"\rCharles LeBrun",        /* 37: 43143 */
	"\"Saint John the Baptist\"\rRaphael",                                /* 38: 43144 */
	"\"Marie de Medici\"\rVan Dyck",                                      /* 39: 43150 */
	"\"Hercules struggling with Achelous\"\rGuido Reni",                  /* 40: 43151 */
	"\"The Centaur Nessus carrying away Deianeira\"\rGuido Reni",         /* 41: 43152 */
	"\"Saint Francis of Assissi comforted after receiving the "
	"stigmata\"\rSeghers",                                                /* 42: 43153 */
	"\"Thomiris plunging Cyrus' head in blood\"\rRubens",                 /* 43: 43154 */
	"\"Hercules Slaying the Hydra\"\rGuido Reni",                         /* 44: 43155 */
	"\"Hercules in flames \"\rGuido Reni",                                /* 45: 43156 */
	"\"Portrait of the Prince Palatine & his brother Prince "
	"Robert\"\rVan Dyck",                                                 /* 46: 43157 */
	"\"Christ being taken down from the cross\"\rCharles Lebrun",         /* 47: 45260 */
};

static char const *const versaillesESmessages[] = {
	"Por el momento est" "\x87" " prohibido abrir esta puerta.",          /*   0 */
	"Esta puerta est" "\x87" " cerrada.",                                 /*   1 */
	"Esta puerta est" "\x87" " cerrada.",                                 /*   2 */
	"Este caj" "\x97" "n est" "\x87" " vac" "\x92" "o.",                  /*   3 */
	"No puedes alcanzar la cubierta.",                                    /*   4 */
	"No hay nada en este naranjo.",                                       /*   5 */
	"\xc1" "Esto no es un naranjo!",                                      /*   6 */
	"Est" "\x87" " demasiado oscuro.",                                    /*   7 */
	"El cofre est" "\x87" " cerrado. ",                                   /*   8 */
	"Puedes abrir la puerta.",                                            /*   9 */
	"Necesitar" "\x87" "s algo para poder alcanzar la bomba.",            /*  10 */
	"Este jarr" "\x97" "n est" "\x87" " vac" "\x92" "o.",                 /*  11 */
	"Ahora puedes pasar.",                                                /*  12 */
	"\xc1" "No tienes tiempo para aprender sobre la Corte.!",             /*  13 */
	"\xc1" "Es demasiado tarde para admirar los cuadros!",                /*  14 */
	"\xc1" "Espera! Dale tus pistas al ujier.",                           /*  15 */
	"No puedes alcanzar el papel.",                                       /*  16 */
	"A la botica.",                                                       /*  17 */
	"Aviso: Podr" "\x87" "s llegar al final de este nivel, pero no has "
	"realizado todas las acciones necesarias para poder continuar. Se "
	"te aconseja que GUARDES este juego ahora.",                          /*  18 */
	"Aviso: Podr" "\x87" "s llegar al final de este nivel, pero puede "
	"que no hayas realizado todas las acciones necesarias para poder "
	"continuar. Se te aconseja que GUARDES este juego ahora.",            /*  19 */
	"No puedes moverte por ah" "\x92" " mientras lleves una escalera.",   /*  20 */
	"Aqu" "\x92" " no hay nada m" "\x87" "s.",                            /*  21 */
	"Adi" "\x97" "s ...",                                                 /*  22 */
	"VERSALLES,",                                                         /*  23 */
	"Intriga en la Corte de Luis XIV",                                    /*  24 */
	"Consultar la zona de documentaci" "\x97" "n",                        /*  25 */
	"           Continuar el juego",                                      /*  26 */
	"           Empezar un juego nuevo",                                  /*  27 */
	"           Cargar un juego",                                         /*  28 */
	"           Guardar este juego",                                      /*  29 */
	"           Mostrar subt" "\x92" "tulos: SI",                         /*  30 */
	"           Mostrar subt" "\x92" "tulos: NO",                         /*  31 */
	"           M" "\x9c" "sica: SI",                                     /*  32 */
	"           M" "\x9c" "sica: NO",                                     /*  33 */
	"           Una parte de la m" "\x9c" "sica en el disco duro (20 "
	"Mb)",                                                                /*  34 */
	"           Toda la m" "\x9c" "sica en el disco duro (92 Mb)",        /*  35 */
	"           No guardar m" "\x9c" "sica en el disco duro (leer CD)",   /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"Volumen",                                                            /*  39 */
	"Salir del juego",                                                    /*  40 */
	"",                                                                   /*  41 */
	"Visitar el palacio",                                                 /*  42 */
	"Cr" "\x8e" "ditos",                                                  /*  43 */
	"           Continuar la visita",                                     /*  44 */
	"           Guardar esta visita",                                     /*  45 */
	"           Cargar una visita",                                       /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D: normal",                                          /*  48 */
	"           Omni3D: despacio",                                        /*  49 */
	"           Omni3D: muy despacio",                                    /*  50 */
	"           Omni3D: r" "\x87" "pido",                                 /*  51 */
	"           Omni3D: muy r" "\x87" "pido",                             /*  52 */
	"Confirmar",                                                          /*  53 */
	"Cancelar",                                                           /*  54 */
	"libre",                                                              /*  55 */
	"sin nombre",                                                         /*  56 */
	"Aviso: este juego va a ser abandonado.",                             /*  57 */
	"Vuelta",                                                             /*  58 */
	"El Palacio",                                                         /*  59 */
	"Volver al Men" "\x9c" " Principal",                                  /*  60 */
	"Sumario de la Zona de Documentaci" "\x97" "n",                       /*  61 */
	"Mapa del Palacio y los jardines",                                    /*  62 */
	"Plano interior del palacio",                                         /*  63 */
	"Imposible escribir en el disco duro: disco lleno",                   /*  64 */
	nullptr,                                                              /*  65 */
	"Inserta el CD",                                                      /*  66 */
	"Inserta el %d CD y pulsa cualquier tecla",                           /*  67 */
	"Las Artes",                                                          /*  68 */
	"El  Reinado",                                                        /*  69 */
	"La Corte",                                                           /*  70 */
	"Vida en el Palacio",                                                 /*  71 */
	"El Palacio y los jardines",                                          /*  72 */
	"Cronolog" "\x92" "a",                                                /*  73 */
	"La Fuente de Apolo",                                                 /*  74 */
	"El Palacio",                                                         /*  75 */
	"Colonnade",                                                          /*  76 */
	"Laberinto",                                                          /*  77 */
	"Latona",                                                             /*  78 */
	"Orangerie",                                                          /*  79 */
	"El Parterre de agua",                                                /*  80 */
	"Alfombra verde",                                                     /*  81 */
	"Las Caballerizas Grandes",                                           /*  82 */
	"Las Caballerizas Peque" "\x96" "as",                                 /*  83 */
	"Los jardines",                                                       /*  84 */
	"El Patio",                                                           /*  85 */
	"El Gran Canal",                                                      /*  86 */
	"El Parterre Sur",                                                    /*  87 */
	"El Parterre Norte",                                                  /*  88 */
	"El huerto del Rey",                                                  /*  89 */
	"El Sal" "\x97" "n de Baile",                                         /*  90 */
	"La Fuente de Neptuno",                                               /*  91 */
	"El Lago de los Guardas Suizos",                                      /*  92 */
	"Agujas (" "\xc1" "Sin uso!)",                                        /*  93 */
	"Tijeras",                                                            /*  94 */
	"Papel",                                                              /*  95 */
	"Panfleto sobre las artes",                                           /*  96 */
	"Llave peque" "\x96" "a 1",                                           /*  97 */
	"Papel transformado",                                                 /*  98 */
	"Papel manchado",                                                     /*  99 */
	"Papel del cofre",                                                    /* 100 */
	"Panfleto sobre linaje real",                                         /* 101 */
	"Vela encendida",                                                     /* 102 */
	"Vela",                                                               /* 103 */
	"Llave",                                                              /* 104 */
	"Carpeta de bocetos",                                                 /* 105 */
	"Carpeta de bocetos",                                                 /* 106 */
	"Boceto falso",                                                       /* 107 */
	"Escalera",                                                           /* 108 */
	"Boceto destruido",                                                   /* 109 */
	"pincel",                                                             /* 110 */
	"pincel dorado",                                                      /* 111 */
	"pincel rojo",                                                        /* 112 */
	"Carboncillo",                                                        /* 113 */
	"Papel",                                                              /* 114 */
	"Panfleto sobre arquitectura",                                        /* 115 */
	"Llave peque" "\x96" "a 2",                                           /* 116 */
	"Arquero(" "\xc1" "sin uso!)",                                        /* 117 */
	"Partitura",                                                          /* 118 */
	"Taco de Billar",                                                     /* 119 */
	"Autorizaci" "\x97" "n",                                              /* 120 */
	"Reproducci" "\x97" "n de medallas",                                  /* 121 */
	"Caj" "\x97" "n con medallas",                                        /* 122 */
	"Llave de la peque" "\x96" "a puerta de Apolo",                       /* 123 */
	"Comida",                                                             /* 124 */
	"Panfleto sobre religi" "\x97" "n",                                   /* 125 */
	"Ep" "\x92" "grafe",                                                  /* 126 */
	"Panfleto sobre el gobierno",                                         /* 127 */
	"Pluma",                                                              /* 128 */
	"Memo",                                                               /* 129 */
	"Telescopio",                                                         /* 130 */
	"Plano de Vauban",                                                    /* 131 */
	"Plano de Vauban",                                                    /* 132 */
	"Cord" "\x97" "n",                                                    /* 133 */
	"Grabado",                                                            /* 134 */
	"Llave peque" "\x96" "a 3",                                           /* 135 */
	"Llave peque" "\x96" "a 4",                                           /* 136 */
	"Memorandum",                                                         /* 137 */
	"Planos del Palacio",                                                 /* 138 */
	"Planos del Palacio",                                                 /* 139 */
	"Llave del " "\x87" "tico",                                           /* 140 */
	"F" "\x87" "bulas",                                                   /* 141 */
	"Plano del laberinto",                                                /* 142 */
	"Herramienta",                                                        /* 143 */
	"Medicina",                                                           /* 144 */
	"Apagavelas",                                                         /* 145 */
};

static char const *const versaillesESpaintings[] = {
	"\"Animales entrando en el Arca\"\rGerolamo Bassano ",                /*  0: 41201 */
	"\"La Cena de Emmaus\"\rJacopo Bassano",                              /*  1: 41202 */
	"\"Mar" "\x92" "a Magdalena a los pies de Cristo\"",                  /*  2: 41203 */
	"\"Saliendo del Arca\"\rGerolamo Bassano ",                           /*  3: 41204 */
	"\"Golpeando la Roca\"\rJacopo Bassano",                              /*  4: 41205 */
	"\"La Batalla de Arbelles\"\rJoseph Parrocel",                        /*  5: 41301 */
	"\"Alejandro el Grande, Conquistador de Darisu en la batalla de "
	"Arbelles\"\rLe Bourguignon ",                                        /*  6: 41302 */
	"\"El Combate de Leuze\"\rJoseph Parrocel",                           /*  7: 42401 */
	"\"Santa Cecilia con un " "\x87" "ngel sosteniendo una "
	"partitura\"\rIl Domenichino",                                        /*  8: 42901 */
	"\"Don Francisco de Moncada\"\rVan Dyck",                             /*  9: 42902 */
	"\"El joven Juan Bautista\"\rCarracci",                               /* 10: 42903 */
	"\"San Mateo\"\rValentin",                                            /* 11: 42904 */
	"\"Rendici" "\x97" "n ante el C" "\x8e" "sar\"\rValentin",            /* 12: 42905 */
	"\"San Lucas\"\rValentin",                                            /* 13: 42906 */
	"\"El matrimonio m" "\x92" "stico de Santa Catalina\"\r Alessandro "
	"Turchi",                                                             /* 14: 42907 */
	"\"La Reuni" "\x97" "n de los Bebedores\"\rNicolas Tournier",         /* 15: 42908 */
	"\"Vidente\"\rValentin",                                              /* 16: 42909 */
	"\"El Rey David tocando el arpa\"\rIl Domenichino",                   /* 17: 42910 */
	"\"Mar" "\x92" "a Magdalena\"\rIl Domenichino",                       /* 18: 42911 */
	"\"Autorretrato\"\rVan Dyck",                                         /* 19: 42912 */
	"\"San Juan el Evangelista\"\r Valentin",                             /* 20: 42913 */
	"\"Hagar siendo ayudada por un " "\x87" "ngel\"\rGiovanni "
	"Lanfranco",                                                          /* 21: 42914 */
	"\"San Marcos\"\rValentin",                                           /* 22: 42915 */
	"\"Meleager con la cabeza del verraco de Calydon a sus "
	"pies\"\rJacques Rousseau",                                           /* 23: 43090 */
	"\"El Rey vestido de romano\"\rJean Warin",                           /* 24: 43091 */
	"\"Atalanta\"\rJacques Rousseau",                                     /* 25: 43092 */
	"\"Eneas llevando a Anchises\"\rSpada",                               /* 26: 43100 */
	"\"David y Betsab" "\x8e" "\"\rVeronese",                             /* 27: 43101 */
	"\"El Vuelo a Egipto\"\rGuido Reni",                                  /* 28: 43102 */
	"\"Luis XIV a caballo\"\rPierre Mignard",                             /* 29: 43103 */
	"\"La Magnificencia Real y el Progreso de las Bellas "
	"Artes\"\rHouasse",                                                   /* 30: 43104 */
	"\"El sacrificio de Ifigenia\"\rCharles de La Fosse",                 /* 31: 43130 */
	"\"Busto de Luis XIV\"\rsculpted by Bernini",                         /* 32: 43131 */
	"\"Diana descubre a Endymion en los brazos de Morfeo\"\rGabriel "
	"Blanchard",                                                          /* 33: 43132 */
	"\"San Pedro y la V" "\x92" "rgen Mar" "\x92" "a\"\rIl Guercino",     /* 34: 43140 */
	"\"Los disc" "\x92" "pulos en Emmaus\"\rVeronese",                    /* 35: 43141 */
	"\"La Sagrada Familia\"\rVeronese",                                   /* 36: 43142 */
	"\"La familia de Dar" "\x92" "o a los pies de Alejandro\"\rCharles "
	"LeBrun",                                                             /* 37: 43143 */
	"\"San Juan Bautista\"\rRaphael",                                     /* 38: 43144 */
	"\"Mar" "\x92" "a de M" "\x8e" "dicis\"\rVan Dyck",                   /* 39: 43150 */
	"\"H" "\x8e" "rcules luchando contra Aqueleo\"\rGuido Reni",          /* 40: 43151 */
	"\"El Centauro Nessus raptando a Deianeira\"\rGuido Reni",            /* 41: 43152 */
	"\"San Francisco de As" "\x92" "s consolado tras recibir el "
	"estigma\"\rSeghers",                                                 /* 42: 43153 */
	"\"Thomiris hundiendo la cabeza de Ciro en sangre\"\rRubens",         /* 43: 43154 */
	"\"H" "\x8e" "rcules destruyendo a Hydra\"\rGuido Reni",              /* 44: 43155 */
	"\"H" "\x8e" "rcules en llamas\"\rGuido Reni",                        /* 45: 43156 */
	"\"Retrato del Pr" "\x92" "ncipe Palatine y de su hermano el Pr"
	"\x92" "ncipe Roberto\"\rVan Dyck",                                   /* 46: 43157 */
	"\"El Descendimiento de Cristo\"\rCharles Lebrun",                    /* 47: 45260 */
};

static char const *const versaillesITmessages[] = {
	"Per ora " "\x8f" " vietato aprire questa porta.",                    /*   0 */
	"Questa porta " "\x8f" " chiusa a chiave.",                           /*   1 */
	"Questa porta " "\x8f" " chiusa.",                                    /*   2 */
	"Questo cassetto " "\x8f" " vuoto.",                                  /*   3 */
	"Non puoi raggiungere il" "\xca" "rivestimento.",                     /*   4 */
	"Non c" "\xd5\x8f" " niente in questo arancio.",                      /*   5 */
	"Questo non " "\x8f" " un arancio!",                                  /*   6 */
	"\xe9" " troppo buio. ",                                              /*   7 */
	"Lo scrigno " "\x8f" " chiuso. ",                                     /*   8 */
	"Puoi aprire la porta.",                                              /*   9 */
	"Avrai bisogno di qualcosa per poter raggiungere la bomba.",          /*  10 */
	"Questo vaso " "\x8f" " vuoto.",                                      /*  11 */
	"Ora ci puoi passare.",                                               /*  12 */
	"Non hai tempo per studiare la Corte!",                               /*  13 */
	"\xe9" " troppo tardi per guardare i quadri!",                        /*  14 */
	"Aspetta! Dai al messo i tuoi indizi.",                               /*  15 */
	"Non puoi raggiungere il foglio.",                                    /*  16 */
	"Alla farmacia.",                                                     /*  17 */
	"Attenzione: sarai in grado di raggiungere la fine di questo "
	"livello, ma non hai eseguito tutte le azioni necessarie per poter "
	"continuare. Ti consigliamo di SALVARE la partita ora.",              /*  18 */
	"Attenzione: sarai in grado di raggiungere la fine di questo "
	"livello, ma potresti non aver eseguito tutte le azioni necessarie "
	"per poter continuare. Ti consigliamo di SALVARE la partita ora.",    /*  19 */
	"Non puoi muoverti mentre trasporti una scala!",                      /*  20 */
	"Non c" "\xd5\x8f" " nient" "\xd5" "altro qui.",                      /*  21 */
	"Arrivederci...",                                                     /*  22 */
	"VERSAILLES,",                                                        /*  23 */
	"Complotto alla Corte del Re Sole",                                   /*  24 */
	"Consulta lo spazio documentativo",                                   /*  25 */
	"           Continua questa partita ",                                /*  26 */
	"           Avvia una nuova partita",                                 /*  27 */
	"           Carica una partita ",                                     /*  28 */
	"           Salva questa partita ",                                   /*  29 */
	"           Mostra sottotitoli: S" "\xea",                            /*  30 */
	"           Mostra sottotitoli: NO",                                  /*  31 */
	"           Musica: S" "\xea",                                        /*  32 */
	"           Musica: NO",                                              /*  33 */
	"           Un brano musicale sul disco fisso (20 Mb)",               /*  34 */
	"           Tutta la musica sul disco fisso (92 Mb)",                 /*  35 */
	"           Nessuna musica sul disco fisso (lettore CD)",             /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"Volume",                                                             /*  39 */
	"Esci dal gioco",                                                     /*  40 */
	"",                                                                   /*  41 */
	"Visita il castello",                                                 /*  42 */
	"Riconoscimenti",                                                     /*  43 */
	"           Continua questa visita ",                                 /*  44 */
	"           Salva questa visita ",                                    /*  45 */
	"           Carica una visita ",                                      /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D: standard",                                        /*  48 */
	"           Omni3D: lento",                                           /*  49 */
	"           Omni3D: molto lento",                                     /*  50 */
	"           Omni3D: veloce",                                          /*  51 */
	"           Omni3D: molto veloce",                                    /*  52 */
	"Conferma",                                                           /*  53 */
	"Annulla",                                                            /*  54 */
	"libero",                                                             /*  55 */
	"senza nome ",                                                        /*  56 */
	"Attenzione: questa partita sta per essere annullata.",               /*  57 */
	"Ritorno",                                                            /*  58 */
	"Il Castello",                                                        /*  59 */
	"Ritorna al Men" "\x9d" " Principale",                                /*  60 */
	"Sommario dello spazio documentativo",                                /*  61 */
	"Mappa del Castello e dei giardini",                                  /*  62 */
	"Cartina interna del castello",                                       /*  63 */
	"Impossibile scrivere sul disco fisso: disco pieno",                  /*  64 */
	nullptr,                                                              /*  65 */
	"Inserisci il CD ",                                                   /*  66 */
	"Inserisci il CD %d e premi un tasto",                                /*  67 */
	"Le Arti",                                                            /*  68 */
	"Il Regno",                                                           /*  69 */
	"La Corte",                                                           /*  70 */
	"La vita nel Castello",                                               /*  71 */
	"Il Castello e i giardini",                                           /*  72 */
	"Cronologia",                                                         /*  73 */
	"La fontana di Apollo",                                               /*  74 */
	"Il castello",                                                        /*  75 */
	"Il colonnato",                                                       /*  76 */
	"Il labirinto",                                                       /*  77 */
	"Latona",                                                             /*  78 */
	"L" "\xd5" "aranceto",                                                /*  79 */
	"Parterre d" "\xd5" "acqua",                                          /*  80 */
	"Tapis vert",                                                         /*  81 */
	"Grande scuderia",                                                    /*  82 */
	"Piccola scuderia",                                                   /*  83 */
	"I giardini",                                                         /*  84 */
	"Corte esterna",                                                      /*  85 */
	"Gran Canale",                                                        /*  86 */
	"Parterre di sud",                                                    /*  87 */
	"Parterre di nord",                                                   /*  88 */
	"L" "\xd5" "orto del Re",                                             /*  89 */
	"La sala da ballo",                                                   /*  90 */
	"La fontana di Nettuno",                                              /*  91 */
	"Il lago delle guardie svizzere",                                     /*  92 */
	"Aiguilles (Inutile!)",                                               /*  93 */
	"Forbici",                                                            /*  94 */
	"Documento",                                                          /*  95 */
	"Libello delle arti",                                                 /*  96 */
	"Piccola chiave 1",                                                   /*  97 */
	"Documento trasformato",                                              /*  98 */
	"Documento macchiato",                                                /*  99 */
	"Documento dello scrigno",                                            /* 100 */
	"Libello sulla discendenza reale",                                    /* 101 */
	"Candela accesa ",                                                    /* 102 */
	"Candela",                                                            /* 103 */
	"Chiave ",                                                            /* 104 */
	"Cartella dei disegni ",                                              /* 105 */
	"Cartella dei disegni",                                               /* 106 */
	"Disegno falso",                                                      /* 107 */
	"Scala",                                                              /* 108 */
	"Disegno distrutto",                                                  /* 109 */
	"pennello",                                                           /* 110 */
	"pennello d" "\xd5" "oro",                                            /* 111 */
	"pennello rosso",                                                     /* 112 */
	"Carboncino",                                                         /* 113 */
	"Carta",                                                              /* 114 */
	"Libello sull" "\xd5" "architettura",                                 /* 115 */
	"Piccola chiave 2",                                                   /* 116 */
	"Archer (inutile!)",                                                  /* 117 */
	"Partitura",                                                          /* 118 */
	"Stecca da biliardo",                                                 /* 119 */
	"Autorizzazione",                                                     /* 120 */
	"Riproduzione delle medaglie",                                        /* 121 */
	"Disegnatore con medaglie",                                           /* 122 */
	"Chiave per la porta di Apollo",                                      /* 123 */
	"Cibo",                                                               /* 124 */
	"Libello sulla religione",                                            /* 125 */
	"Epigrafe",                                                           /* 126 */
	"Libello sul governo",                                                /* 127 */
	"Penna d" "\xd5" "oca",                                               /* 128 */
	"Appunto",                                                            /* 129 */
	"Telescopio",                                                         /* 130 */
	"Progetto di Vauban",                                                 /* 131 */
	"Progetto di Vauban",                                                 /* 132 */
	"Corda",                                                              /* 133 */
	"Incisione",                                                          /* 134 */
	"Piccola chiave 3",                                                   /* 135 */
	"Piccola chiave 4",                                                   /* 136 */
	"Memorandum",                                                         /* 137 */
	"Progetti del castello",                                              /* 138 */
	"Progetti del castello",                                              /* 139 */
	"Chiave dell" "\xd5" "attico",                                        /* 140 */
	"Favole",                                                             /* 141 */
	"Progetto del labirinto",                                             /* 142 */
	"Attrezzo",                                                           /* 143 */
	"Medicina",                                                           /* 144 */
	"Smoccolatoio",                                                       /* 145 */
};

static char const *const versaillesITpaintings[] = {
	"\"Entrata degli animali nell'arca\"\rGerolamo Bassano",              /*  0: 41201 */
	"\"La cena di Emmaus\"\rJacopo Bassano",                              /*  1: 41202 */
	"\"La Maddalena ai piedi del Cristo\"",                               /*  2: 41203 */
	"\"Uscita dall'arca\"\rGerolamo Bassano",                             /*  3: 41204 */
	"\"Scalfittura della roccia\"\rJacopo Bassano",                       /*  4: 41205 */
	"\"La battaglia di Arbelles\"\rJoseph Parrocel",                      /*  5: 41301 */
	"\"Alessandro il Grande, vincitore di Dario nella battaglia "
	"Arbelles\"\rLe Bourguignon",                                         /*  6: 41302 */
	"\"Il combattimento di Leuze\"\rJoseph Parrocel",                     /*  7: 42401 */
	"\"Santa Cecilia con un angelo che tiene una partitura "
	"musicale\"\rIl Domenichino",                                         /*  8: 42901 */
	"\"Don Francisco di Moncada\"\rVan Dyck",                             /*  9: 42902 */
	"\"Il giovane santo Giovanni il Battista\"\rCarracci",                /* 10: 42903 */
	"\"San Matteo\"\rValentin",                                           /* 11: 42904 */
	"\"Il rifiuto di Cesare\"\rValentin",                                 /* 12: 42905 */
	"\"San Luca\"\rValentin",                                             /* 13: 42906 */
	"\"Il matrimonio mistico di Santa Caterina\"\r Alessandro Turchi",    /* 14: 42907 */
	"\"Convivio di bevitori\"\rNicolas Tournier",                         /* 15: 42908 */
	"\"L'indovina\"\rValentin",                                           /* 16: 42909 */
	"\"Re Davide che suona l'arpa\"\rIl Domenichino",                     /* 17: 42910 */
	"\"Maria Maddalena\"\rIl Domenichino",                                /* 18: 42911 */
	"\"Autoritratto\"\rVan Dyck",                                         /* 19: 42912 */
	"\"San Giovanni l'Evangelista\"\r Valentin",                          /* 20: 42913 */
	"\"Agar soccorso da un angelo\"\rGiovanni Lanfranco",                 /* 21: 42914 */
	"\"San Marco\"\rValentin",                                            /* 22: 42915 */
	"\"Meleagro con la testa del cinghiale di Calidone ai suoi "
	"piedi\"\r Jacques Rousseau",                                         /* 23: 43090 */
	"\"Il Re in abito romano\"\rJean Warin",                              /* 24: 43091 */
	"\"Atalanta\"\rJacques Rousseau",                                     /* 25: 43092 */
	"\"Enea che porta Anchise\"\rSpada",                                  /* 26: 43100 */
	"\"Davide e Betsabea\"\rVeronese",                                    /* 27: 43101 */
	"\"La fuga in Egitto\"\rGuido Reni",                                  /* 28: 43102 */
	"\"Luigi XIV a cavallo\"\rPierre Mignard",                            /* 29: 43103 */
	"\"La magnificenza reale e i progressi delle Belle Arti\"\rHouasse",  /* 30: 43104 */
	"\"Il sacrificio di Ifigenia\"\rCharles de La Fosse",                 /* 31: 43130 */
	"\"Busto di Luigi XIV\"\rsculpted by Bernini",                        /* 32: 43131 */
	"\"Diana scopre il pastore Endimione assopito tra le braccia di "
	"Morfeo\"\rGabriel Blanchard",                                        /* 33: 43132 */
	"\"San Pietro e la Vergine Maria\"\rIl Guercino",                     /* 34: 43140 */
	"\"I pellegrini di Emmaus\"\rVeronese",                               /* 35: 43141 */
	"\"La Sacra Famiglia\"\rVeronese",                                    /* 36: 43142 */
	"\"La famiglia di Dario ai piedi di Alessandro\"\rCharles LeBrun",    /* 37: 43143 */
	"\"San Giovanni il Battista\"\rRaphael",                              /* 38: 43144 */
	"\"Maria de' Medici\"\rVan Dyck",                                     /* 39: 43150 */
	"\"Ercole in lotta con Achel" "\x98" "o\"\rGuido Reni",               /* 40: 43151 */
	"\"Il Centauro Nesso che porta Deianira\"\rGuido Reni",               /* 41: 43152 */
	"\"San Francesco d'Assisi confortato dopo aver ricevuto le "
	"stimmate\"\rSeghers",                                                /* 42: 43153 */
	"\"Thomiris che immerge la testa di Ciro nel sangue\"\rRubens",       /* 43: 43154 */
	"\"Ercole che uccide l'Idra\"\rGuido Reni",                           /* 44: 43155 */
	"\"Ercole sul rogo " "\xd2" " \rGuido Reni",                          /* 45: 43156 */
	"\"Ritratto del Principe Palatine e di suo fratello il Principe "
	"Robert\"\rVan Dyck",                                                 /* 46: 43157 */
	"\"Cristo calato dalla Croce\"\rCharles Lebrun",                      /* 47: 45260 */
};

static char const *const versaillesKOmessages[] = {
	"\xc1\xf6\xb1\xdd\xc0\xba" " " "\xc0\xcc" " " "\xb9\xae\xc0\xcc" " "
	"" "\xbf\xad\xb8\xae\xc1\xf6" " " ""
	"\xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9" ".",                               /*   0 */
	"\xb9\xae\xc0\xcc" " " "\xc0\xe1\xb0\xe5\xbd\xc0\xb4\xcf\xb4\xd9" ""
	".",                                                                  /*   1 */
	"\xb9\xae\xc0\xcc" " " "\xc0\xe1\xb0\xe5\xbd\xc0\xb4\xcf\xb4\xd9" ""
	".",                                                                  /*   2 */
	"\xbc\xad\xb6\xf8\xc0\xcc" " " ""
	"\xba\xf1\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9" ".",                       /*   3 */
	"\xb5\xa4\xb0\xb3\xbf\xa1" " " "\xb4\xea\xc1\xf6" " " ""
	"\xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9" ".",                               /*   4 */
	"\xc0\xcc" " " "\xb3\xaa\xb9\xab\xbf\xa3" " " "\xbe\xc6\xb9\xab" " "
	"" "\xb0\xcd\xb5\xb5" " " "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9" ".",     /*   5 */
	"\xbf\xc0\xb7\xbb\xc1\xf6\xb3\xaa\xb9\xab\xb0\xa1" " " ""
	"\xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9" "!",                               /*   6 */
	"\xb3\xca\xb9\xab" " " "\xbe\xee\xb5\xd3\xbd\xc0\xb4\xcf\xb4\xd9" ""
	".",                                                                  /*   7 */
	"\xbb\xf3\xc0\xda\xb0\xa1" " " ""
	"\xc0\xe1\xb0\xe5\xbd\xc0\xb4\xcf\xb4\xd9" ". ",                      /*   8 */
	"\xb9\xae\xc0\xbb" " " "\xbf\xad" " " "\xbc\xf6" " " ""
	"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9" ".",                               /*   9 */
	"\xc6\xf8\xc5\xba\xbf\xa1" " " "\xb4\xea\xc0\xbb" " " "\xbc\xf6" " "
	"" "\xc0\xd6\xb4\xc2" " " "\xb0\xcd\xc0\xcc" " " ""
	"\xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9" ".",                       /*  10 */
	"\xb2\xc9\xba\xb4\xc0\xcc" " " ""
	"\xba\xf1\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9" ".",                       /*  11 */
	"\xc0\xcc\xc1\xa6" " " "\xc1\xf6\xb3\xaa" " " "\xb0\xa1\xb5\xb5" " "
	"" "\xb5\xcb\xb4\xcf\xb4\xd9" ".",                                    /*  12 */
	"\xb4\xf5" " " "\xbe\xcb\xbe\xc6\xb3\xbe" " " ""
	"\xbd\xc3\xb0\xa3\xc0\xcc" " " "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9" ""
	"!",                                                                  /*  13 */
	"\xb1\xd7\xb8\xb2" " " "\xb0\xa8\xbb\xf3\xc7\xcf\xb1\xe2\xbf\xa3" ""
	" " "\xb4\xca\xbd\xc0\xb4\xcf\xb4\xd9" "!",                           /*  14 */
	"\xc0\xe1\xb1\xf1" "! " "\xc0\xc7\xc0\xfc\xb0\xfc\xbf\xa1\xb0\xd4" ""
	" " "\xb4\xdc\xbc\xad\xb8\xa6" ".",                                   /*  15 */
	"\xc1\xbe\xc0\xcc\xbf\xa1" " " "\xb4\xea\xc1\xf6" " " ""
	"\xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9" ".",                               /*  16 */
	"\xbe\xe0\xb1\xb9\xc0\xb8\xb7\xce" ".",                               /*  17 */
	"\xc1\xd6\xc0\xc7" ": " "\xb4\xe7\xbd\xc5\xc0\xba" " " "\xc0\xcc" ""
	" " "\xb7\xb9\xba\xa7\xc0\xc7" " " "\xb3\xa1\xb1\xee\xc1\xf6" " " ""
	"\xb5\xb5\xb4\xde\xc7\xd2" " " "\xbc\xf6" " " ""
	"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9" ", " "\xc7\xcf\xc1\xf6\xb8\xb8" ""
	" " "\xb0\xe8\xbc\xd3\xc7\xcf\xb1\xe2" " " ""
	"\xc0\xa7\xc7\xd8\xbc\xad" " " "\xc7\xca\xbf\xe4\xc7\xd1" " " ""
	"\xb8\xf0\xb5\xe7" " " "\xc7\xe0\xb5\xbf\xc0\xbb" " " ""
	"\xbf\xcf\xbc\xf6\xc7\xcf\xc1\xf6" " " ""
	"\xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9" ". " "\xc1\xf6\xb1\xdd" ""
	" " "\xc0\xcc" " " "\xb0\xd4\xc0\xd3\xc0\xbb" " " ""
	"\xc0\xfa\xc0\xe5\xc7\xcf\xbd\xca\xbd\xc3\xbf\xc0" ".",               /*  18 */
	" " "\xc1\xd6\xc0\xc7" ": " "\xb4\xe7\xbd\xc5\xc0\xba" " " ""
	"\xc0\xcc" " " "\xb7\xb9\xba\xa7\xc0\xc7" " " ""
	"\xb3\xa1\xb1\xee\xc1\xf6" " " "\xb5\xb5\xb4\xde\xc7\xd2" " " ""
	"\xbc\xf6" " " "\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9" ", " ""
	"\xc7\xcf\xc1\xf6\xb8\xb8" " " "\xb0\xe8\xbc\xd3\xc7\xcf\xb1\xe2" ""
	" " "\xc0\xa7\xc7\xd8\xbc\xad" " " "\xc7\xca\xbf\xe4\xc7\xd1" " " ""
	"\xb8\xf0\xb5\xe7" " " "\xc7\xe0\xb5\xbf\xc0\xbb" " " ""
	"\xbf\xcf\xbc\xf6\xc7\xcf\xc1\xf6" " " ""
	"\xbe\xca\xbe\xd2\xc1\xf6\xb5\xb5" " " ""
	"\xb8\xf0\xb8\xa8\xb4\xcf\xb4\xd9" ". " "\xc1\xf6\xb1\xdd" " " ""
	"\xc0\xcc" " " "\xb0\xd4\xc0\xd3\xc0\xbb" " SAVE" ""
	"\xc7\xcf\xbd\xca\xbd\xc3\xbf\xc0" ".",                               /*  19 */
	"\xbb\xe7\xb4\xd9\xb8\xae\xb8\xa6" " " "\xb0\xa1\xc1\xf6\xb0\xed" ""
	" " "\xbf\xf2\xc1\xf7\xc0\xcf" " " "\xbc\xf6" " " ""
	"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9" "!",                               /*  20 */
	"\xbe\xc6\xb9\xab\xb0\xcd\xb5\xb5" " " ""
	"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9" ".",                               /*  21 */
	"\xc0\xcc\xb8\xb8" "...",                                             /*  22 */
	"\xba\xa3\xb8\xa3\xbb\xe7\xc0\xcc\xc0\xaf" ",",                       /*  23 */
	"\xb7\xe7\xc0\xcc" " 14" "\xbc\xbc" " " "\xb1\xc3\xc0\xfc\xc0\xc7" ""
	" " "\xc0\xbd\xb8\xf0",                                               /*  24 */
	"\xb9\xae\xbc\xad" " " "\xbf\xb5\xbf\xaa\xc0\xbb" " " ""
	"\xc2\xfc\xc1\xb6",                                                   /*  25 */
	"           " "\xb0\xd4\xc0\xd3\xc0\xbb" " " "\xb0\xe8\xbc\xd3",      /*  26 */
	"           " "\xbb\xf5\xb7\xce\xbf\xee" " " ""
	"\xb0\xd4\xc0\xd3\xc0\xbb" " " "\xbd\xc3\xc0\xdb",                    /*  27 */
	"           " "\xb0\xd4\xc0\xd3\xc0\xbb" " " "\xb7\xce\xb5\xe5",      /*  28 */
	"           " "\xb0\xd4\xc0\xd3\xc0\xbb" " " "\xc0\xfa\xc0\xe5",      /*  29 */
	"           " "\xc0\xda\xb8\xb7" " " "\xc7\xa5\xbd\xc3" ": " ""
	"\xbf\xb9",                                                           /*  30 */
	"           " "\xc0\xda\xb8\xb7" " " "\xc7\xa5\xbd\xc3" ": " ""
	"\xbe\xc6\xb4\xcf\xbf\xc0",                                           /*  31 */
	"           " "\xc0\xbd\xbe\xc7" ": " "\xbf\xb9",                     /*  32 */
	"           " "\xc0\xbd\xbe\xc7" ": " "\xbe\xc6\xb4\xcf\xbf\xc0",     /*  33 */
	"           " "\xc7\xd1" " " "\xc0\xbd\xbe\xc7\xc0\xbb" " HDD" ""
	"\xbf\xa1" " " "\xc0\xfa\xc0\xe5" "(20 Mo)",                          /*  34 */
	"           " "\xb8\xf0\xb5\xe7" " " "\xc0\xbd\xbe\xc7\xc0\xbb" " "
	"HDD" "\xbf\xa1" " " "\xc0\xfa\xc0\xe5" "(92 Mo)",                    /*  35 */
	"           " "\xbe\xc6\xb9\xab" " " "\xc0\xbd\xbe\xc7\xb5\xb5" " "
	"HDD" "\xbf\xa1" " " "\xc0\xfa\xc0\xe5" " " "\xbe\xc8\xc7\xd4" " "
	"(lecture CD)",                                                       /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"\xba\xbc\xb7\xfd",                                                   /*  39 */
	"\xb0\xd4\xc0\xd3" " " "\xc1\xdf\xb4\xdc",                            /*  40 */
	"",                                                                   /*  41 */
	"\xbc\xba\xc0\xbb" " " "\xc1\xb6\xbb\xe7",                            /*  42 */
	"\xc5\xa9\xb7\xb9\xb5\xf7",                                           /*  43 */
	"           " "\xc1\xb6\xbb\xe7\xb8\xa6" " " "\xb0\xe8\xbc\xd3",      /*  44 */
	"           " "\xc1\xb6\xbb\xe7\xb8\xa6" " " "\xc0\xfa\xc0\xe5",      /*  45 */
	"           " "\xc1\xb6\xbb\xe7\xb8\xa6" " " "\xb7\xce\xb5\xe5",      /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D: " "\xc7\xa5\xc1\xd8",                             /*  48 */
	"           Omni3D: " "\xb4\xc0\xb8\xae\xb0\xd4",                     /*  49 */
	"           Omni3D: " "\xbe\xc6\xc1\xd6" " " ""
	"\xb4\xc0\xb8\xae\xb0\xd4",                                           /*  50 */
	"           Omni3D: " "\xba\xfc\xb8\xa3\xb0\xd4",                     /*  51 */
	"           Omni3D: " "\xbe\xc6\xc1\xd6" " " ""
	"\xba\xfc\xb8\xa3\xb0\xd4",                                           /*  52 */
	"\xc8\xae\xc0\xce",                                                   /*  53 */
	"\xc3\xeb\xbc\xd2",                                                   /*  54 */
	"\xba\xf1\xbe\xee" " " "\xc0\xd6\xc0\xbd",                            /*  55 */
	"\xc0\xcc\xb8\xa7" " " "\xbe\xf8\xc0\xbd",                            /*  56 */
	"\xc1\xd6\xc0\xc7" ": " "\xc0\xcc" " " "\xb0\xd4\xc0\xd3\xc0\xba" ""
	" " "\xc1\xdf\xb4\xdc\xb5\xc9" " " "\xb0\xcd\xc0\xd3" ".",            /*  57 */
	"\xb5\xb9\xbe\xc6\xb0\xa8",                                           /*  58 */
	"\xbc\xba",                                                           /*  59 */
	"\xb8\xde\xc0\xce" " " "\xb8\xde\xb4\xba\xb7\xce" " " ""
	"\xb5\xb9\xbe\xc6\xb0\xa8",                                           /*  60 */
	"\xb9\xae\xbc\xad" " " "\xbf\xb5\xbf\xaa" " " "\xbf\xe4\xbe\xe0",     /*  61 */
	"\xbc\xba\xb0\xfa" " " "\xc1\xa4\xbf\xf8" " " "\xc1\xf6\xb5\xb5",     /*  62 */
	"\xbc\xba\xc0\xc7" " " "\xb3\xbb\xba\xce" " " ""
	"\xbc\xb3\xb0\xe8\xb5\xb5",                                           /*  63 */
	"HDD" "\xbf\xa1" " " "\xbe\xb2\xb4\xc2" " " "\xb0\xcd\xc0\xcc" " " ""
	"\xba\xd2\xb0\xa1\xb4\xc9\xc7\xd4" ": " "\xbf\xa9\xba\xd0" " " ""
	"\xba\xce\xc1\xb7",                                                   /*  64 */
	nullptr,                                                              /*  65 */
	"CD" "\xb8\xa6" " " "\xb3\xd6\xc0\xb8\xbd\xc3\xbf\xc0" " ",           /*  66 */
	"%d " "\xb9\xf8\xc2\xb0" " CD" "\xb8\xa6" " " "\xb3\xd6\xb0\xed" " "
	"" "\xbe\xc6\xb9\xab" " " "\xc5\xb0\xb3\xaa" " " ""
	"\xb4\xa9\xb8\xa3\xbd\xc3\xbf\xc0",                                   /*  67 */
	"\xbf\xb9\xbc\xfa",                                                   /*  68 */
	"\xc5\xeb\xc4\xa1",                                                   /*  69 */
	"\xb1\xc3\xc1\xa4",                                                   /*  70 */
	"\xbc\xba\xbf\xa1\xbc\xad\xc0\xc7" " " "\xbb\xfd\xc8\xb0",            /*  71 */
	"\xbc\xba\xb0\xfa" " " "\xc1\xa4\xbf\xf8",                            /*  72 */
	"\xbf\xac\xb4\xeb\xb1\xe2",                                           /*  73 */
	"\xbe\xc6\xc6\xfa\xb7\xce" " " "\xba\xd0\xbc\xf6",                    /*  74 */
	"\xbc\xba",                                                           /*  75 */
	"\xc4\xdd\xb7\xce\xb3\xaa\xb5\xe5",                                   /*  76 */
	"\xb9\xcc\xb7\xce",                                                   /*  77 */
	"\xb6\xf3\xc5\xe4\xb3\xaa",                                           /*  78 */
	"\xb0\xa8\xb1\xd6\xbf\xf8",                                           /*  79 */
	"\xb9\xb0\xc0\xc7" " " "\xc8\xad\xb4\xdc",                            /*  80 */
	"\xb5\xb5\xb9\xda\xb4\xeb",                                           /*  81 */
	"\xb4\xeb" " " "\xc1\xb6\xb7\xc3\xc0\xe5",                            /*  82 */
	"\xbc\xd2" " " "\xc1\xb6\xb7\xc3\xc0\xe5",                            /*  83 */
	"\xc1\xa4\xbf\xf8",                                                   /*  84 */
	"\xbe\xd5" " " "\xb6\xe3",                                            /*  85 */
	"\xb4\xeb\xbf\xee\xc7\xcf",                                           /*  86 */
	"\xb3\xb2\xc2\xca" " " "\xc8\xad\xb4\xdc",                            /*  87 */
	"\xba\xcf\xc2\xca" " " "\xc8\xad\xb4\xdc",                            /*  88 */
	"\xb1\xb9\xbf\xd5\xc0\xc7" " " "\xbe\xdf\xc3\xa4\xb9\xe7",            /*  89 */
	"\xb9\xab\xb5\xb5\xbd\xc7",                                           /*  90 */
	"\xb3\xdc\xc6\xaa" " " "\xba\xd0\xbc\xf6",                            /*  91 */
	"\xbd\xba\xc0\xa7\xbd\xba\xbf\xeb\xba\xb4\xc0\xc7" " " ""
	"\xc8\xa3\xbc\xf6",                                                   /*  92 */
	"\xb9\xd9\xb4\xc3" "(" "\xbe\xb5\xb8\xf0\xbe\xf8\xc0\xbd" "!)",       /*  93 */
	"\xb0\xa1\xc0\xa7",                                                   /*  94 */
	"\xc1\xbe\xc0\xcc",                                                   /*  95 */
	"\xbf\xb9\xbc\xfa\xbf\xa1" " " "\xb0\xfc\xc7\xd1" " " ""
	"\xb9\xae\xbc\xad",                                                   /*  96 */
	"\xc0\xdb\xc0\xba" " " "\xbf\xad\xbc\xe8" " 1",                       /*  97 */
	"\xba\xaf\xc7\xfc\xb5\xc8" " " "\xc1\xbe\xc0\xcc",                    /*  98 */
	"\xc2\xf8\xbb\xf6\xc7\xd1" " " "\xc1\xbe\xc0\xcc",                    /*  99 */
	"\xbb\xf3\xc0\xda\xc0\xc7" " " "\xc1\xbe\xc0\xcc",                    /* 100 */
	"\xc8\xb2\xbd\xc7" " " "\xb0\xa1\xb0\xe8\xbf\xa1" " " ""
	"\xb0\xfc\xc7\xd1" " " "\xb9\xae\xbc\xad",                            /* 101 */
	"\xb9\xe0\xc7\xf4\xc1\xf8" " " "\xc3\xca",                            /* 102 */
	"\xc3\xca",                                                           /* 103 */
	"\xbf\xad\xbc\xe8",                                                   /* 104 */
	"\xbd\xba\xc4\xc9\xc4\xa1" " " "\xb9\xad\xc0\xbd",                    /* 105 */
	"\xbd\xba\xc4\xc9\xc4\xa1" " " "\xb9\xad\xc0\xbd",                    /* 106 */
	"\xc0\xa7\xc1\xb6" " " "\xbd\xba\xc4\xc9\xc4\xa1",                    /* 107 */
	"\xbb\xe7\xb4\xd9\xb8\xae",                                           /* 108 */
	"\xc6\xc4\xb1\xab\xb5\xc8" " " "\xbd\xba\xc4\xc9\xc4\xa1",            /* 109 */
	"\xba\xd7",                                                           /* 110 */
	"\xc8\xb2\xb1\xdd\xbb\xf6" " " "\xba\xd7",                            /* 111 */
	"\xba\xd3\xc0\xba\xbb\xf6" " " "\xba\xd7",                            /* 112 */
	"\xb8\xf1\xc5\xba" " " "\xbf\xac\xc7\xca",                            /* 113 */
	"\xc1\xbe\xc0\xcc",                                                   /* 114 */
	"\xb0\xc7\xc3\xe0\xbf\xa1" " " "\xb0\xfc\xc7\xd1" " " ""
	"\xb9\xae\xbc\xad",                                                   /* 115 */
	"\xc0\xdb\xc0\xba" " " "\xbf\xad\xbc\xe8" " 2",                       /* 116 */
	"\xbb\xe7\xbc\xf6" "(" "\xbe\xb5\xb8\xf0\xbe\xf8\xc0\xbd" "!)",       /* 117 */
	"\xc0\xbd\xbe\xc7" " " "\xbe\xc7\xba\xb8",                            /* 118 */
	"\xb4\xe7\xb1\xb8" " " "\xc5\xa5",                                    /* 119 */
	"\xc7\xe3\xb0\xa1",                                                   /* 120 */
	"\xb8\xde\xb4\xde" " " "\xba\xb9\xc1\xa6",                            /* 121 */
	"\xb8\xde\xb4\xde\xc0\xcc" " " "\xc0\xd6\xb4\xc2" " " ""
	"\xbc\xad\xb6\xf8",                                                   /* 122 */
	"\xbe\xc6\xc6\xfa\xb7\xce" " " "\xc0\xdb\xc0\xba" " " ""
	"\xb9\xae\xc0\xc7" " " "\xbf\xad\xbc\xe8",                            /* 123 */
	"\xc0\xbd\xbd\xc4",                                                   /* 124 */
	"\xc1\xbe\xb1\xb3\xbf\xa1" " " "\xb0\xfc\xc7\xd1" " " ""
	"\xb9\xae\xbc\xad",                                                   /* 125 */
	"\xbc\xf6\xbc\xf6\xb2\xb2\xb3\xa2",                                   /* 126 */
	"\xc5\xeb\xc4\xa1\xbf\xa1" " " "\xb0\xfc\xc7\xd1" " " ""
	"\xb9\xae\xbc\xad",                                                   /* 127 */
	"\xb1\xea\xc6\xe6",                                                   /* 128 */
	"\xb8\xde\xb8\xf0",                                                   /* 129 */
	"\xb8\xc1\xbf\xf8\xb0\xe6",                                           /* 130 */
	"\xba\xb8\xb9\xe6" " " "\xbc\xb3\xb0\xe8\xb5\xb5",                    /* 131 */
	"\xba\xb8\xb9\xe6" " " "\xbc\xb3\xb0\xe8\xb5\xb5",                    /* 132 */
	"\xb2\xf6",                                                           /* 133 */
	"\xc1\xb6\xc6\xc7",                                                   /* 134 */
	"\xc0\xdb\xc0\xba" " " "\xbf\xad\xbc\xe8" " 3",                       /* 135 */
	"\xc0\xdb\xc0\xba" " " "\xbf\xad\xbc\xe8" " 4",                       /* 136 */
	"\xba\xf1\xb8\xc1\xb7\xcf",                                           /* 137 */
	"\xbc\xba\xc0\xc7" " " "\xbc\xb3\xb0\xe8\xb5\xb5",                    /* 138 */
	"\xbc\xba\xc0\xc7" " " "\xbc\xb3\xb0\xe8\xb5\xb5",                    /* 139 */
	"\xb4\xd9\xb6\xf4\xb9\xe6" " " "\xbf\xad\xbc\xe8",                    /* 140 */
	"\xbf\xec\xc8\xad",                                                   /* 141 */
	"\xb9\xcc\xb7\xce\xc0\xc7" " " "\xbc\xb3\xb0\xe8\xb5\xb5",            /* 142 */
	"\xb5\xb5\xb1\xb8",                                                   /* 143 */
	"\xbe\xe0",                                                           /* 144 */
	"\xc3\xd0\xba\xd2" " " "\xb2\xf4\xb4\xc2" " " "\xb1\xe2\xb1\xb8",     /* 145 */
	"\xc1\xa6" "%d" "\xc0\xe5",                                           /* 146 */
	"\xc1\xb6\xbb\xe7\xb8\xa6",                                           /* 147 */
	/* These ones are not in messages but it's simpler */
	"\xbf\xa9\xbf\xec\xbf\xcd \xb5\xce\xb7\xe7\xb9\xcc",                  /* 148/Epigraph */
	nullptr,                                                              /* 149/Memo */
	"\xbf\xcf\xba\xae\xc7\xd4\xbf\xa1 \xc0\xcc\xb8\xa3\xb4\xc2 "
	"\xb0\xcd\xc0\xba \xb1\xb9\xbf\xd5\xc0\xc7 "
	"\xb1\xc7\xb7\xc2\xbf\xa1 \xc0\xd6\xc1\xf6 \xbe\xca\xb4\xd9",         /* 150/Bomb */
};

static char const *const versaillesKOpaintings[] = {
	"\"" "\xb3\xeb\xbe\xc6\xc0\xc7" " " "\xb9\xe6\xc1\xd6\xb7\xce" " " ""
	"\xb5\xe9\xbe\xee\xbf\xc0\xb4\xc2" " " ""
	"\xb5\xbf\xb9\xb0\xb5\xe9\"\r\xc1\xa6\xb7\xd1\xb6\xf3\xb8\xf0" " " ""
	"\xb9\xd9\xbb\xe7\xb3\xeb",                                           /*  0: 41201 */
	"\"" "\xbf\xa5\xb8\xb6\xbf\xec\xbd\xba\xc0\xc7" " " ""
	"\xc0\xfa\xb3\xe1\xbd\xc4\xbb\xe7\"\r\xc0\xda\xc4\xda\xc6\xf7" " " ""
	"\xb9\xd9\xbb\xe7\xb3\xeb",                                           /*  1: 41202 */
	"\"" "\xb1\xd7\xb8\xae\xbd\xba\xb5\xb5\xc0\xc7" " " ""
	"\xb9\xdf\xb9\xd8\xbf\xa1" " " "\xc0\xd6\xb4\xc2" " " ""
	"\xb8\xb7\xb4\xde\xb6\xf3" " " "\xb8\xb6\xb8\xae\xbe\xc6\"\r",        /*  2: 41203 */
	"\"" "\xb3\xeb\xbe\xc6\xc0\xc7" " " "\xb9\xe6\xc1\xd6\xb8\xa6" " " ""
	"\xb6\xb0\xb3\xaa\xb8\xe7\"\r\xc1\xa6\xb7\xd1\xb6\xf3\xb8\xf0" " " ""
	"\xb9\xd9\xbb\xe7\xb3\xeb",                                           /*  3: 41204 */
	"\"" "\xb9\xd9\xc0\xa7\xb8\xa6" " " ""
	"\xb1\xfa\xb8\xe7\"\r\xc0\xda\xc4\xda\xc6\xf7" " " ""
	"\xb9\xd9\xbb\xe7\xb3\xeb",                                           /*  4: 41205 */
	"\"" "\xbe\xc6\xb8\xa3\xba\xa7" "(Arbelles)" "\xc0\xc7" " " ""
	"\xc0\xfc\xc5\xf5\"\r\xc1\xb6\xbc\xc1" " " ""
	"\xc6\xc4\xb7\xce\xbc\xbf",                                           /*  5: 41301 */
	"\"" "\xbe\xcb\xb7\xba\xbb\xea\xb4\xf5" " " "\xb4\xeb\xbf\xd5" ", " ""
	"\xbe\xc6\xb8\xa3\xba\xa7\xc0\xc7" " " ""
	"\xc0\xfc\xc5\xf5\xbf\xa1\xbc\xad" " " ""
	"\xb4\xd9\xb8\xae\xbf\xec\xbd\xba\xbf\xa1" " " "\xb4\xeb\xc7\xd1" ""
	" " "\xbd\xc2\xb8\xae\"\r\xb8\xa3" " " ""
	"\xba\xce\xb8\xa3\xb1\xcd\xb3\xf3",                                   /*  6: 41302 */
	"\"" "\xb7\xe7\xc0\xcc\xc1\xee" "(Leuze)" "\xc0\xc7" " " ""
	"\xc0\xfc\xc5\xf5\"\r\xc1\xb6\xbc\xc1" " " ""
	"\xc6\xc4\xb7\xce\xbc\xbf",                                           /*  7: 42401 */
	"\"" "\xb1\xe2\xba\xb8\xb9\xfd\xc0\xbb" " " ""
	"\xb0\xa1\xc1\xf6\xb0\xed" " " "\xc0\xd6\xb4\xc2" " " ""
	"\xc3\xb5\xbb\xe7\xbf\xcd" " " "\xc7\xd4\xb2\xb2" " " ""
	"\xc0\xd6\xb4\xc2" " " "\xbc\xba" " " ""
	"\xbc\xbc\xbd\xc7\xb8\xae\xbe\xc6\"\r" " " "\xc0\xcf" " " ""
	"\xb5\xb5\xb8\xde\xb4\xcf\xc4\xa1\xb3\xeb",                           /*  8: 42901 */
	"\"" "\xb8\xf9\xc4\xab\xb4\xd9\xc0\xc7" " " "\xb5\xbf" " " ""
	"\xc7\xc1\xb6\xf5\xbd\xc3\xbd\xba\xc4\xda\"\r\xb9\xdd" " " ""
	"\xb4\xd9\xc0\xcc\xc5\xa9",                                           /*  9: 42902 */
	"\"" "\xc0\xfe\xc0\xba" " " "\xbc\xbc\xb7\xca" " " ""
	"\xbf\xe4\xc7\xd1\"\r\xc4\xab\xb6\xf3\xc4\xa1",                       /* 10: 42903 */
	"\"" "\xbc\xba" " " ""
	"\xb8\xb6\xc5\xc2\"\r\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce",               /* 11: 42904 */
	"\"" "\xc4\xc9\xbb\xe7\xb8\xa3\xbf\xa1\xb0\xd4" " " ""
	"\xb3\xd1\xb0\xdc\xc1\xd6\xb8\xe7\"\r\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce",/* 12: 42905 */
	"\"" "\xbc\xba" " " ""
	"\xb4\xa9\xb0\xa1\"\r\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce",               /* 13: 42906 */
	"\"" "\xbc\xba" " " "\xb1\xee\xb6\xdf\xb8\xb0\xc0\xc7" " " ""
	"\xbd\xc5\xba\xf1\xb7\xce\xbf\xee" " " ""
	"\xb0\xe1\xc8\xa5\"\r\xbe\xcb\xb7\xb9\xbb\xea\xb5\xe5\xb7\xce" " " ""
	"\xc5\xf5\xb8\xa3\xc4\xa1",                                           /* 14: 42907 */
	"\"" "\xb8\xb6\xbd\xc3\xb4\xc2" " " ""
	"\xbb\xe7\xb6\xf7\xb5\xe9\xc0\xc7" " " ""
	"\xb8\xf0\xc0\xd3\"\r\xb4\xcf\xc4\xdd\xb6\xf3\xbd\xba" " " ""
	"\xb6\xd1\xb8\xa3\xb4\xcf\xbf\xa1",                                   /* 15: 42908 */
	"\"" "\xc1\xa1\xc0\xe5\xc0\xcc\"\r\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce",  /* 16: 42909 */
	"\"" "\xc7\xcf\xc7\xc1\xb8\xa6" " " ""
	"\xbf\xac\xc1\xd6\xc7\xcf\xb4\xc2" " " ""
	"\xb4\xd9\xc0\xad\xbf\xd5\"\r\xc0\xcf" " " ""
	"\xb5\xb5\xb8\xde\xb4\xcf\xc4\xa1\xb3\xeb",                           /* 17: 42910 */
	"\"" "\xb8\xb7\xb4\xde\xb6\xf3" " " ""
	"\xb8\xb6\xb8\xae\xbe\xc6\"\r\xc0\xcf" " " ""
	"\xb5\xb5\xb8\xde\xb4\xcf\xc4\xa1\xb3\xeb",                           /* 18: 42911 */
	"\"" "\xc0\xda\xc8\xad\xbb\xf3\"\r\xb9\xdd" " " ""
	"\xb4\xd9\xc0\xcc\xc5\xa9",                                           /* 19: 42912 */
	"\"" "\xc0\xfc\xb5\xb5\xc0\xda" " " "\xbc\xba" " " ""
	"\xbf\xe4\xc7\xd1\"\r\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce",               /* 20: 42913 */
	"\"" "\xc3\xb5\xbb\xe7\xc0\xc7" " " "\xb5\xb5\xbf\xf2\xc0\xbb" " " ""
	"\xb9\xde\xb4\xc2" " " ""
	"\xc7\xcf\xb0\xa5\"\r\xc1\xf6\xbf\xc0\xb9\xd9\xb4\xcf" " " ""
	"\xb6\xfb\xc7\xc1\xb6\xfb\xc4\xda",                                   /* 21: 42914 */
	"\"" "\xbc\xba" " " ""
	"\xb8\xb6\xb0\xa1\"\r\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce",               /* 22: 42915 */
	"\"" "\xb9\xdf" "  " "\xbe\xc6\xb7\xa1\xbf\xa1" " " ""
	"\xc4\xae\xb8\xae\xb5\xbf\xc0\xc7" " " "\xb8\xe4\xb5\xc5\xc1\xf6" ""
	" " "\xb8\xd3\xb8\xae\xb8\xa6" " " "\xb3\xf5\xc0\xba" " " ""
	"\xb8\xe1\xb7\xb9\xbe\xc6\xb1\xd7\xb7\xce\xbd\xba\"\r\xc0\xda\xb2\xf4" ""
	" " "\xb7\xe7\xbc\xd2",                                               /* 23: 43090 */ /* BUG: Switched */
	"\"" "\xb7\xce\xb8\xb6\xc0\xc7" " " "\xbf\xca\xc0\xbb" " " ""
	"\xc0\xd4\xc0\xbb" " " "\xbf\xd5\"\r\xc0\xe5" " " ""
	"\xbf\xcd\xb8\xb0" " ",                                               /* 24: 43091 */
	"\"" "\xbe\xc6\xc5\xbb\xb6\xf5\xc5\xb8\"\r\xc0\xda\xb2\xf4" " " ""
	"\xb7\xe7\xbc\xd2",                                                   /* 25: 43092 */
	"\"" "\xbe\xc8\xc5\xb0\xbc\xbc\xbd\xba\xb8\xa6" " " ""
	"\xb5\xa5\xb7\xc1\xb0\xa1\xb4\xc2" " " ""
	"\xbe\xc6\xbf\xa1\xb3\xd7\xc0\xcc\xbe\xc6\xbd\xba\"\r\xbd\xba\xc6\xc4\xb4\xd9",/* 26: 43100 */
	"\"" "\xb4\xd9\xc0\xad\xb0\xfa" " " ""
	"\xba\xa3\xb6\xdf\xbc\xbc\xb9\xd9\"\r\xb1\xb8\xc0\xcc\xb5\xb5" " " ""
	"\xb7\xb9\xb4\xcf",                                                   /* 27: 43101 */
	"\"" "\xc0\xcc\xc1\xfd\xc6\xae\xb7\xce\xc0\xc7" " " ""
	"\xb5\xb5\xc1\xd6\"\r\xb1\xb8\xc0\xcc\xb5\xb5" " " ""
	"\xb7\xb9\xb4\xcf",                                                   /* 28: 43102 */
	"\"" "\xb8\xbb\xc0\xbb" " " "\xc5\xb8\xb0\xed" " " ""
	"\xc0\xd6\xb4\xc2" " " "\xb7\xe7\xc0\xcc" " 14" ""
	"\xbc\xbc\"\r\xc7\xc7\xbf\xa1\xb8\xa3" " " ""
	"\xb9\xcc\xb3\xc4\xb5\xe5",                                           /* 29: 43103 */
	"\"" "\xb1\xb9\xbf\xd5\xc0\xc7" " " ""
	"\xc0\xe5\xb4\xeb\xc7\xd4\xb0\xfa" " " "\xb9\xcc\xbc\xfa" "(" ""
	"\xc7\xd0\xb1\xb3" ")" "\xc0\xc7" " " ""
	"\xb9\xdf\xc0\xfc\"\r\xc8\xc4\xbe\xc6\xbd\xba",                       /* 30: 43104 */
	"\"" "\xc0\xcc\xc7\xc7\xb0\xd4\xb4\xcf\xbe\xc6\xc0\xc7" " " ""
	"\xc8\xf1\xbb\xfd\"\r\xbb\xfe\xb8\xa6" " " "\xb5\xe9" " " ""
	"\xb6\xf3" " " "\xc6\xf7\xbd\xba",                                    /* 31: 43130 */
	"\"" "\xb7\xe7\xc0\xcc" " 14" "\xbc\xbc\xc0\xc7" " " ""
	"\xc8\xe4\xbb\xf3\"\r\xba\xa3\xb8\xa3\xb4\xcf\xb4\xcf",               /* 32: 43131 */
	"\"" "\xb4\xd9\xc0\xcc\xbe\xc6\xb3\xaa\xb0\xa1" " " ""
	"\xc0\xe1\xb5\xe9\xbe\xee\xc0\xd6\xb4\xc2" " " ""
	"\xbf\xa3\xb5\xf0\xb9\xcc\xbf\xc2" "(" "\xb4\xde\xc0\xc7" " " ""
	"\xbf\xa9\xbd\xc5" " Selene" "\xc0\xc7" " " ""
	"\xbb\xe7\xb6\xfb\xc0\xbb" " " "\xb9\xde\xb4\xc2" " " ""
	"\xbe\xe7\xc4\xa1\xb1\xe2" " " "\xb9\xcc\xbc\xd2\xb3\xe2" ")" ""
	"\xc0\xbb" " " ""
	"\xb9\xdf\xb0\xdf\xc7\xcf\xb4\xd9\"\r\xb0\xa1\xba\xea\xb8\xae\xbf\xa4" ""
	" " "\xba\xed\xb6\xfb\xc2\xf7",                                       /* 33: 43132 */
	"\"" "\xbc\xba" " " "\xba\xa3\xb5\xe5\xb7\xce\xbf\xcd" " " ""
	"\xb5\xbf\xc1\xa4\xb3\xe0" " " ""
	"\xb8\xb6\xb8\xae\xbe\xc6\"\r\xc0\xcf" " " ""
	"\xb1\xb8\xbf\xa1\xb8\xa3\xbd\xc3\xb3\xeb",                           /* 34: 43140 */
	"\"" "\xbf\xa5\xb8\xb6\xbf\xec\xbd\xba\xbf\xa1\xbc\xad\xc0\xc7" " " ""
	"\xbb\xe7\xb5\xb5\xb5\xe9\"\r\xba\xa3\xb7\xce\xb3\xd7\xc1\xee",       /* 35: 43141 */
	"\"" ""
	"\xbd\xc5\xbc\xba\xb0\xa1\xc1\xb7\"\r\xba\xa3\xb7\xce\xb3\xd7\xc1\xee",/* 36: 43142 */
	"\"" "\xbe\xcb\xb7\xba\xbb\xea\xb4\xf5\xc0\xc7" " " ""
	"\xb9\xdf\xb9\xd8\xbf\xa1" " " "\xc0\xd6\xb4\xc2" " " ""
	"\xb4\xd9\xb8\xae\xbf\xec\xbd\xba" " " ""
	"\xb0\xa1\xc1\xb7\"\r\xbb\xfe\xb8\xa6" " " "\xb8\xa3" " " ""
	"\xba\xea\xb7\xf8",                                                   /* 37: 43143 */
	"\"" "\xbc\xbc\xb7\xca" " " ""
	"\xbf\xe4\xc7\xd1\"\r\xb6\xf3\xc6\xc4\xbf\xa4",                       /* 38: 43144 */
	"\"" "\xb8\xb6\xb8\xae" " " "\xb5\xe5" " " ""
	"\xb8\xde\xb5\xf0\xc4\xa1\"\r\xb9\xdd" " " ""
	"\xb4\xd9\xc0\xcc\xc5\xa9",                                           /* 39: 43150 */
	"\"" "\xbe\xc6\xc4\xcc\xb7\xce\xbf\xec\xbd\xba\xbf\xcd" " " ""
	"\xbd\xce\xbf\xec\xb4\xc2" " " ""
	"\xc7\xec\xb8\xa3\xc5\xa7\xb7\xb9\xbd\xba\"\r\xb1\xb8\xc0\xcc\xb5\xb5" ""
	" " "\xb7\xb9\xb4\xcf",                                               /* 40: 43151 */
	"\"" "\xb5\xa5\xc0\xcc\xbe\xc6\xb3\xd7\xc0\xcc\xbe\xc6" "(" ""
	"\xc7\xec\xb8\xa3\xc5\xa7\xb7\xb9\xbd\xba\xc0\xc7" " " ""
	"\xbe\xc6\xb3\xbb" ")" "\xb8\xa6" " " ""
	"\xbb\xaf\xbe\xc6\xb0\xa1\xb4\xc2" " " ""
	"\xc4\xcb\xc5\xb8\xbf\xec\xb7\xce\xbd\xba" " " ""
	"\xb3\xd7\xbc\xad\xbd\xba\"\r\xb1\xb8\xc0\xcc\xb5\xb5" " " ""
	"\xb7\xb9\xb4\xcf",                                                   /* 41: 43152 */
	"\"" "\xbc\xba\xc8\xe7" "(" "\xe1\xa1\xfd\xdd" ")" "\xc0\xbb" " " ""
	"\xb9\xde\xc0\xba" " " "\xc8\xc4" " " "\xc0\xa7\xbe\xc8\xc0\xbb" " "
	"" "\xb4\xc0\xb3\xa2\xb4\xc2" " " ""
	"\xbe\xc6\xbd\xc3\xbd\xc3\xc0\xc7" " " "\xbc\xba" " " ""
	"\xc7\xc1\xb6\xf5\xbd\xc3\xbd\xba\xc4\xda\"\r\xc1\xa6\xb0\xa1\xbd\xba",/* 42: 43153 */
	"\"" "\xc4\xfb\xb7\xce\xbd\xba\xc0\xc7" " " ""
	"\xb8\xd3\xb8\xae\xb8\xa6" " " "\xc7\xc7\xb7\xce" " " ""
	"\xb9\xb0\xb5\xe9\xc0\xcc\xb4\xc2" " " ""
	"\xc5\xe4\xb9\xcc\xb8\xae\xbd\xba\"\r\xb7\xe7\xba\xa5\xbd\xba",       /* 43: 43154 */
	"\"" "\xc8\xf7\xb5\xe5\xb6\xf3\xb8\xa6" " " ""
	"\xc1\xd7\xc0\xcc\xb4\xc2" " " ""
	"\xc7\xec\xb8\xa3\xc5\xa7\xb7\xb9\xbd\xba\"\r\xb1\xb8\xc0\xcc\xb5\xb5" ""
	" " "\xb7\xb9\xb4\xcf",                                               /* 44: 43155 */
	"\"" "\xc8\xef\xba\xd0\xc7\xd1" " " ""
	"\xc7\xec\xb8\xa3\xc5\xa7\xb7\xb9\xbd\xba\"\r\xb1\xb8\xc0\xcc\xb5\xb5" ""
	" " "\xb7\xb9\xb4\xcf" " ",                                           /* 45: 43156 */
	"\"" "\xbb\xa1\xb6\xfb\xb6\xaf" " " "\xbf\xd5\xc0\xda\xbf\xcd" " " ""
	"\xb1\xd7\xc0\xc7" " " "\xc7\xfc\xc1\xa6\xc0\xce" " " ""
	"\xb7\xce\xba\xa3\xb8\xa3" " " "\xbf\xd5\xc1\xb6\xc0\xc7" " " ""
	"\xc3\xca\xbb\xf3\xc8\xad\"\r\xb9\xdd" " " ""
	"\xb4\xd9\xc0\xcc\xc5\xa9",                                           /* 46: 43157 */
	"\"" "\xbd\xca\xc0\xda\xb0\xa1\xbf\xa1" " " ""
	"\xb8\xf8\xb9\xdb\xc7\xf4" " " "\xc0\xd6\xb4\xc2" " " ""
	"\xb1\xd7\xb8\xae\xbd\xba\xb5\xb5\"\r\xbb\xfe\xb8\xa6" " " ""
	"\xb8\xa3" " " "\xba\xea\xb7\xf8" " ",                                /* 47: 45260 */
};

static const Subtitle versaillesKOsubtitles[] = {
	{
		"a0_vf.hns", {
			{
				75, "\xc5\xc2\xbe\xe7\xc0\xcc" " " "\xc0\xcf\xb3\xe2" " " ""
				"\xc1\xdf\xbf\xa1\xbc\xad" " " "\xb0\xa1\xc0\xe5" " " ""
				"\xb3\xf4\xc0\xcc" " " "\xb6\xb9\xc0\xbb" " " "\xb6\xa7"
			},
			{
				95, "\xbe\xcb\xbd\xc5" " " "\xb1\xc3\xc0\xfc\xb0\xfa" " " ""
				"\xb0\xb0\xc0\xba"
			},
			{115, "\xba\xa3\xb8\xa3\xbb\xe7\xc0\xcc\xc0\xaf" " " "\xbc\xba\xc0\xba"},
			{
				130, "\xc7\xcf\xb4\xc3\xbf\xa1\xbc\xad" " " ""
				"\xba\xd2\xb2\xc9\xc3\xb3\xb7\xb3" "  " ""
				"\xc5\xb8\xbf\xc0\xb8\xa3\xb0\xed"
			},
			{
				150, "\xbe\xc6\xb9\xab\xb0\xcd\xb5\xb5" " " "\xb3\xb2\xc1\xf6" " " ""
				"\xbe\xca\xc0\xb8\xb8\xae\xb6\xf3" "."
			},
			{180, ""},
			{
				210, "\xb1\xb9\xbf\xd5\xb2\xb2\xbc\xad\xb4\xc2" " " "\xbe\xee\xb6\xb2" ""
				" " "\xb9\xcc\xc4\xa3" " " "\xc0\xda\xc0\xc7" " " ""
				"\xc1\xfe\xc0\xcc\xb6\xf3\xb0\xed" " " ""
				"\xbb\xfd\xb0\xa2\xc7\xcf\xbd\xc5\xb4\xd9" ". " ""
				"\xc0\xaf\xc4\xa1\xc7\xd1" " " "\xbb\xf3\xbb\xf3\xc0\xc7" " " ""
				"\xb0\xe1\xb0\xfa\xb6\xf3\xb0\xed" "...."
			},
			{
				250, "\xb8\xf0\xb5\xe7" " " "\xb9\xab\xb8\xae\xb5\xe9\xb0\xfa" " " ""
				"\xc7\xd4\xb2\xb2"
			},
			{
				270, "\xba\xc0\xb6\xa5\xc0\xbb" " " ""
				"\xc7\xd8\xb9\xe6\xbd\xc3\xc5\xb0\xb7\xc1\xb4\xc2" " " ""
				"\xbd\xc3\xb5\xb5\xb4\xc2" " "
			},
			{
				290, "\xb0\xe1\xb1\xb9" " " "\xbd\xc7\xc6\xd0\xc7\xd2" " " ""
				"\xb0\xcd\xc0\xcc\xb4\xd9"
			},
			{
				310, "\xbd\xc3\xb0\xa3\xc0\xcc" " " "\xb4\xd9" " " ""
				"\xb5\xc7\xbe\xfa\xb1\xe2\xbf\xa1"
			},
			{
				325, "\xb1\xd7\xb8\xae\xb0\xed" " " "\xb3\xaa\xb4\xc2" " " ""
				"\xba\xb0\xc3\xb3\xb7\xb3" " " "\xba\xfb\xb3\xaa\xb4\xc2" " " ""
				"\xbf\xd5\xbf\xa1\xb0\xd4" " " "\xb9\xdd\xc7\xd7\xc7\xd1\xb4\xd9"
			},
			{
				350, "\xbf\xd5\xc0\xc7" " " "\xc1\xd6\xc0\xa7\xb8\xa6" " " ""
				"\xb5\xb5\xb4\xc2" " " "\xc7\xe0\xbc\xba\xb0\xfa"
			},
			{
				370, "\xc1\xa6\xb8\xf1\xb0\xfa" " " "\xc7\xa5\xc1\xa6\xb5\xe9" " " ""
				"\xb1\xd7\xb8\xae\xb0\xed"
			},
			{
				390, "\xba\xd2\xc5\xb8\xb4\xc2" " " "\xc8\xb6\xba\xd2" " " ""
				"\xbe\xc6\xb7\xa1" " " "\xb3\xaa\xc5\xb8\xb3\xaa\xb4\xc2"
			},
			{
				410, "\xc0\xcc\xbc\xd9\xc0\xc7" " " "\xbe\xcf\xc8\xa3\xb8\xa6" " " ""
				"\xb0\xae\xb0\xed\xbc\xad"
			},
			{
				430, "\xb3\xaa\xb4\xc2" " " "\xbc\xf6\xbc\xf6\xb2\xb2\xb3\xa2\xbf\xcd" ""
				" " "\xc0\xbd\xb8\xf0\xb8\xa6" " " ""
				"\xc3\xa3\xbe\xc6\xb0\xa1\xb3\xeb\xb6\xf3"
			},
			{
				450, "\xc0\xcc\xb0\xc7" " " "\xb8\xb6\xc4\xa1" " " ""
				"\xbf\xef\xba\xce\xc2\xa2\xb4\xc2" " " "\xbe\xee\xb6\xb2" " " ""
				"\xc0\xbd\xb8\xf0\xc3\xb3\xb7\xb3" " " ""
				"\xb5\xe9\xb8\xae\xb4\xc2\xb1\xba" "! " "\xb1\xb9\xbf\xd5\xc0\xbb" ""
				" " "\xb8\xf0\xbc\xc5\xbe\xdf" " " "\xc7\xcf\xb1\xe2" " " ""
				"\xb6\xa7\xb9\xae\xbf\xa1" " " "\xb3\xad" " " "\xc0\xcc" " " ""
				"\xc0\xcf\xc0\xbb" " " "\xc1\xb6\xbb\xe7\xc7\xd2" " " "\xbc\xf6" " "
				"" "\xbe\xf8\xb3\xd7" ". " "\xb6\xf3\xb6\xfb\xb5\xe5" ", " ""
				"\xbb\xe7\xb0\xc7\xc0\xbb" " " "\xb8\xc3\xbe\xc6\xc1\xd6\xb0\xd4" ""
				". " "\xc4\xa7\xbd\xc7\xc0\xc7" " " ""
				"\xbd\xc3\xc1\xbe\xc0\xb8\xb7\xce\xbc\xad" ", " ""
				"\xb9\xe6\xc7\xd8\xb9\xde\xc1\xf6" " " "\xbe\xca\xb0\xed" " " ""
				"\xbe\xee\xb5\xf0\xb5\xe7\xc1\xf6" " " "\xb0\xa5" " " "\xbc\xf6" " "
				"" "\xc0\xd6\xb3\xd7" "."
			},
			{
				536, "\xc0\xda\xb3\xd9" " " "\xbc\xba" ", " "\xbb\xe7\xb6\xf7\xb5\xe9" ""
				", " "\xb1\xd7\xb8\xae\xb0\xed" " " "\xb0\xfc\xbd\xc0\xbf\xa1" " " ""
				"\xb4\xeb\xc7\xd8" " " "\xc0\xdf" " " "\xbe\xcb\xb0\xed" " " ""
				"\xc0\xd6\xc1\xf6" ". " "\xb0\xe8\xbc\xd3" " " ""
				"\xc1\xfa\xb9\xae\xc0\xbb" " " "\xc7\xcf\xb0\xed" " " ""
				"\xbc\xf6\xbb\xf3\xc7\xd1" " " "\xb0\xcd\xc0\xcc" " " ""
				"\xb9\xdf\xb0\xdf\xb5\xc7\xb0\xc5\xb5\xe7" " " ""
				"\xb3\xaa\xbf\xa1\xb0\xd4" " " "\xba\xb8\xb0\xed\xc7\xcf\xb0\xd4" ""
				"."
			},
			{
				622, "\xc7\xcf\xc1\xf6\xb8\xb8" " " "\xc1\xb6\xbd\xc9\xc7\xcf\xb0\xd4" ""
				"! " "\xbe\xee\xb6\xb2" " " "\xb9\xcc\xc4\xa3" " " ""
				"\xc0\xdb\xc0\xda\xb0\xa1" " " "\xb1\xb9\xbf\xd5\xc0\xbb" " " ""
				"\xbd\xc3\xc7\xd8\xc7\xcf\xb7\xc1\xc7\xd1\xb4\xd9\xb0\xed" " " ""
				"\xbc\xd2\xb9\xae\xc0\xbb" " " ""
				"\xc6\xdb\xb6\xdf\xb8\xae\xc1\xf6\xb4\xc2" " " "\xb8\xbb\xb0\xd4" ""
				"! " "\xbc\xad\xb5\xce\xb8\xa3\xb0\xd4" ", " "\xb4\xdc" " " ""
				"\xc7\xcf\xb7\xe7\xc0\xc7" " " "\xbd\xc3\xb0\xa3" " " ""
				"\xb9\xdb\xbf\xa1\xb4\xc2" " " "\xbe\xf8\xbe\xee" "."
			},
			{690, "8" "\xbd\xc3" " " "\xb9\xdd\xc0\xcc\xb1\xba" "."},
		}
	}, /* a0_vf.hns */
	{
		"a1_vf.hns", {
			{
				358, "\xc6\xf3\xc7\xcf" ", " "\xbd\xc3\xb0\xa3\xc0\xcc" " " ""
				"\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9" "."
			},
			{
				388, "\xb1\xb9\xbf\xd5\xc0\xcc" " " "\xc0\xe1\xbf\xa1\xbc\xad" " " ""
				"\xb1\xfa\xc0\xda" ", " "\xc1\xd6\xc4\xa1\xc0\xc7\xc0\xce" " " ""
				"\xb4\xd9\xb2\xa4\xc0\xcc" " " "\xb1\xd7\xc0\xc7" " " ""
				"\xbe\xc8\xba\xce\xb8\xa6" " " "\xb9\xaf\xb4\xc2\xb4\xd9" "."
			},
			{
				440, "\xc6\xf3\xc7\xcf\xb2\xb2\xbc\xb1" " " "\xc0\xdf" " " ""
				"\xc1\xd6\xb9\xab\xbc\xcc\xbd\xc0\xb4\xcf\xb1\xee" "? " ""
				"\xb3\xca\xb9\xab" " " "\xb4\xfe\xc1\xf6\xb4\xc2" " " ""
				"\xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb1\xee" "? " ""
				"\xb8\xb6\xbb\xe7\xc1\xf6\xb8\xa6" " " ""
				"\xb9\xde\xc0\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee" "?"
			},
			{500, "\xb0\xed\xb8\xbf\xb1\xba" ", " "\xb4\xd9\xb2\xb7" "."},
			{
				510, "\xc6\xf3\xc7\xcf\xb2\xb2\xbc\xb1" " " "\xc1\xf6\xb3\xad" " " ""
				"\xb5\xce" " " "\xb4\xde\xb0\xa3" " " "\xbe\xc6\xc1\xd6" " " ""
				"\xb0\xc7\xb0\xad\xc7\xcf\xbd\xca\xb4\xcf\xb4\xd9" "."
			},
			{550, ""},
			{
				580, "\xba\xc0\xb6\xa5\xc0\xcc" " " "\xb1\xb9\xbf\xd5\xc0\xc7" " " ""
				"\xbc\xd5\xc0\xbb" " " "\xbe\xc4\xb4\xc2" " " "\xb5\xbf\xbe\xc8" ""
				", " "\xb8\xc5\xc0\xcf" " " "\xbe\xc6\xc4\xa7" " " ""
				"\xb1\xd7\xb7\xa8\xb4\xf8" " " "\xb0\xcd\xc3\xb3\xb7\xb3" " " ""
				"\xb0\xa1\xc1\xb7\xc0\xcc" " " "\xb5\xb5\xc2\xf8\xc7\xd1\xb4\xd9" ""
				". " "\xb9\xab\xbd\xc3\xc0\xaf" ", " "\xb1\xb9\xbf\xd5\xc0\xc7" " " ""
				"\xb5\xbf\xbb\xfd" ", " "\xb8\xf9\xbc\xbc\xb4\xba" ", " ""
				"\xb5\xb5\xc6\xd8" ", " "\xb8\xc7\xb4\xc0" " " ""
				"\xb0\xf8\xc0\xdb\xc0\xcc" " " "\xb1\xb9\xbf\xd5\xc0\xbb" " " ""
				"\xbe\xcb\xc7\xf6\xc7\xcf\xb1\xe2" " " "\xc0\xa7\xc7\xd8" " " ""
				"\xb5\xb5\xc2\xf8\xc7\xdf\xb4\xd9" "."
			},
			{713, ""},
			{
				750, "\xc0\xda" ", " "\xb3\xbb" " " "\xbe\xc6\xb5\xe9\xbe\xc6" ", " ""
				"\xbf\xc0\xb4\xc3" " " "\xbe\xc6\xc4\xa7\xbf\xa1\xb4\xc2" " " ""
				"\xbe\xf0\xc1\xa6" " " "\xc0\xcc\xb8\xae" " " ""
				"\xbb\xe7\xb3\xc9\xc0\xbb" " " "\xc7\xcf\xb7\xaf" " " ""
				"\xb3\xaa\xbc\xb9\xb4\xc0\xb3\xc4" "?"
			},
			{795, ""},
		}
	}, /* a1_vf.hns */
	{
		"a2_vf.hns", {
			{
				60, "\xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9" ". " "\xb4\xe7\xbd\xc5\xc0\xc7" ""
				" " "\xc0\xdc\xc0\xce\xc7\xd1" " " "\xb9\xe6\xb9\xfd\xc0\xba" " " ""
				"\xbf\xd5\xb1\xb9\xc0\xc7" " " "\xc6\xf2\xc8\xad\xb8\xa6" " " ""
				"\xba\xb8\xc1\xb8\xc7\xd2" " " "\xbc\xf6" " " ""
				"\xbe\xf8\xc0\xbb\xb0\xcc\xb4\xcf\xb4\xd9" "."
			},
			{
				74, "\xb4\xd9\xb8\xa5" " " "\xc0\xcc\xb5\xe9\xc0\xba" " " ""
				"\xc8\xae\xbd\xc5\xc7\xcf\xb0\xed" " " "\xc0\xd6\xc1\xf6\xbf\xe4" ""
				", " "\xb1\xb9\xbf\xd5\xb2\xb2\xbc\xad" " " ""
				"\xb5\xda\xb6\xf3\xb8\xa6" " " "\xc6\xf7\xb8\xa3\xbd\xba" " " ""
				"\xb0\xf8\xc0\xdb\xbf\xa1\xb0\xd4" " " ""
				"\xba\xb8\xb3\xbb\xb6\xf3\xb0\xed" " " ""
				"\xb8\xbb\xbe\xb8\xc7\xcf\xbc\xcc\xb4\xd9\xb4\xc2" " " ""
				"\xb0\xcd\xc0\xbb" "..."
			},
			{
				130, "\xbe\xc6\xb8\xb6\xb5\xb5" ", " "\xb1\xd7\xb4\xc2" " " "\xb1\xd7" ""
				" " "\xb0\xf8\xc0\xdb\xc0\xc7" " " "\xb0\xb3\xc1\xbe\xc0\xbb" " " ""
				"\xbe\xf2\xbe\xee\xb3\xbe" " " "\xbc\xf6" " " ""
				"\xbe\xf8\xc0\xbb\xb0\xcc\xb4\xcf\xb4\xd9" "."
			},
			{160, ""},
			{
				200, "\xb1\xb9\xbf\xd5\xc0\xc7" " " "\xbe\xc6\xc4\xa7" " " ""
				"\xbf\xb9\xbd\xc4\xc0\xba" " " ""
				"\xbc\xbc\xc1\xa4\xbd\xc4\xc0\xb8\xb7\xce" " " "\xb3\xa1\xc0\xbb" ""
				" " "\xb8\xce\xb4\xc2\xb4\xd9" ". " "\xc3\xb9" " " ""
				"\xb9\xe6\xb9\xae\xb0\xb4" " " "\xc1\xdf" " " ""
				"\xc7\xe3\xb6\xf4\xb5\xc8" " " "\xbd\xc5\xc7\xcf\xb5\xe9" " " ""
				"\xbe\xd5\xbf\xa1\xbc\xad" ", " "\xc0\xcc\xb9\xdf\xbb\xe7\xb4\xc2" ""
				" " "\xb1\xb9\xbf\xd5\xc0\xc7" " " ""
				"\xb8\xf6\xb4\xdc\xc0\xe5\xc0\xbb" " " "\xc7\xcf\xb0\xed" " " ""
				"\xb0\xa1\xb9\xdf\xc0\xbb" " " "\xbe\xba\xbf\xee\xb4\xd9" "."
			},
			{307, ""},
			{
				360, "\xb1\xd7\xb7\xaf\xb8\xe9" " " "\xbf\xb9\xba\xb9" " " ""
				"\xc0\xc7\xbd\xc4\xc0\xbb" " " "\xc0\xa7\xc7\xd8" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xcc" " " "\xbb\xec\xb7\xd5\xc0\xb8\xb7\xce" ""
				" " "\xb3\xaa\xb0\xa1\xb5\xb5\xb7\xcf" " " ""
				"\xc1\xd8\xba\xf1\xb5\xc8\xb4\xd9" "."
			},
			{406, ""},
			{
				440, "\xbd\xc2\xc0\xce\xb5\xc8" " " "\xbd\xc5\xc7\xcf\xb0\xa1" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xcc" " " "\xbe\xc6\xc4\xa7" " " ""
				"\xbd\xc4\xbb\xe7\xb8\xa6" " " "\xc7\xcf\xb4\xc2" " " ""
				"\xb5\xbf\xbe\xc8" " " "\xc7\xd4\xb2\xb2" " " "\xc7\xd1\xb4\xd9" ""
				"."
			},
			{
				520, "\xb0\xa2\xc0\xda" " " "\xb1\xb9\xbf\xd5\xc0\xcc" " " ""
				"\xc3\xc4\xb4\xd9\xba\xc1" " " "\xc1\xd6\xb1\xe2\xb8\xa6" " " ""
				"\xb9\xd9\xb6\xf3\xb0\xed" " " "\xc0\xd6\xc0\xb8\xb8\xe7" ", " ""
				"\xb1\xb9\xbf\xd5\xb2\xb2" " " "\xbe\xc8\xba\xce\xb8\xa6" " " ""
				"\xbf\xa9\xc2\xe3\xb0\xed" ", " "\xb0\xa2\xc1\xbe" " " ""
				"\xbc\xd2\xb9\xae\xc0\xbb" " " ""
				"\xc6\xdb\xb6\xdf\xb8\xae\xb0\xc5\xb3\xaa" " " ""
				"\xb5\xe8\xb4\xc2\xb4\xd9" "."
			},
			{
				600, "\xbd\xc3\xc1\xbe\xb5\xe9\xc0\xcc" " " "\xb1\xb9\xbf\xd5\xc0\xc7" ""
				" " "\xb8\xe9\xb5\xb5\xb8\xa6" " " "\xc7\xd8\xc1\xd6\xb0\xed" " " ""
				"\xbc\xd5\xc0\xbb" " " "\xbe\xc4\xbe\xee" " " "\xc1\xd6\xb0\xed" " "
				"" "\xbf\xca\xc0\xbb" " " "\xc0\xd4\xc7\xf4" " " "\xc1\xd8\xb4\xd9" ""
				". " "\xc0\xcc\xb7\xb1" " " "\xb1\xb9\xbf\xd5\xc0\xc7" " " ""
				"\xc0\xcf\xbb\xf3\xc0\xba" " " "\xbb\xe7\xb6\xf7\xb5\xe9" " " ""
				"\xbe\xd5\xbf\xa1\xbc\xad" " " "\xc1\xf8\xc7\xe0\xb5\xc8\xb4\xd9" ""
				".."
			},
			{679, ""},
			{
				815, "\xbf\xca\xc0\xbb" " " "\xc0\xd4\xc0\xba" " " "\xc8\xc4" ", " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xc4\xa7\xbd\xc7\xb7\xce" " " ""
				"\xb5\xb9\xbe\xc6\xb0\xa1" " " "\xb0\xa3\xb4\xdc\xc7\xd1" " " ""
				"\xbe\xc6\xc4\xa7" " " "\xb1\xe2\xb5\xb5\xb8\xa6" " " ""
				"\xb5\xe5\xb8\xb0\xb4\xd9" "."
			},
			{860, ""},
			{
				948, "\xb1\xe2\xb5\xb5\xb8\xa6" " " "\xb5\xe5\xb8\xb0" " " "\xc8\xc4" ""
				", " "\xbd\xc5\xc7\xcf\xb5\xe9\xb0\xfa" " " "\xc7\xd4\xb2\xb2" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xc8\xb8\xc0\xc7\xbd\xc7\xb7\xce" ""
				" " "\xb0\xa1\xbc\xad" " " "\xbf\xd5\xb1\xb9\xbf\xa1" " " ""
				"\xb0\xfc\xc7\xd1" " " "\xc0\xcf\xc0\xbb" " " ""
				"\xb3\xed\xc0\xc7\xc7\xd1\xb4\xd9" "."
			},
			{1027, ""},
		}
	}, /* a2_vf.hns */
	{
		"a3_vf.hns", {
			{
				30, "\xbf\xb9\xba\xb9" " " "\xc0\xc7\xbd\xc4\xc0\xcc" " " ""
				"\xb3\xa1\xb3\xad" " " "\xc8\xc4" ", " "\xb1\xb9\xbf\xd5\xc0\xba" ""
				" " "\xc8\xb8\xc0\xc7\xbd\xc7\xb7\xce" " " "\xb0\xa1\xbc\xad" " " ""
				"\xbd\xc5\xc7\xcf\xb5\xe9\xb0\xfa" " " "\xc7\xd4\xb0\xd4" " " ""
				"\xb1\xb9\xbb\xe7\xb8\xa6" " " "\xb4\xd9\xb7\xe7\xbe\xfa\xb4\xd9" ""
				".  " "\xba\xf1\xb9\xd0\xc0\xbb" " " ""
				"\xc0\xaf\xc1\xf6\xc7\xcf\xb1\xe2" " " "\xc0\xa7\xc7\xd8" " " ""
				"\xc3\xe2\xc0\xd4\xc0\xcc" " " ""
				"\xb1\xdd\xc1\xf6\xb5\xc7\xbe\xfa\xb4\xd9" "."
			},
			{
				110, "\xbd\xc5\xc7\xcf" " " "\xc1\xdf" " " "\xb0\xa1\xc0\xe5" " " ""
				"\xc1\xdf\xbf\xe4\xc7\xd1" " " "\xc0\xce\xb9\xb0\xc0\xce" " " ""
				"\xb7\xe7\xba\xce\xbe\xc6" " " "\xc8\xc4\xc0\xdb\xc0\xcc" " " ""
				"\xc7\xcf\xb7\xe7\xc0\xc7" " " "\xc0\xc7\xc1\xa6\xb8\xa6" " " ""
				"\xb0\xf8\xc1\xf6\xc7\xdf\xb4\xd9" "."
			},
			{211, ""},
			{280, "\xb8\xf0\xb5\xce" " " "\xbe\xc9\xc0\xb8\xbd\xc3\xbf\xc0" "!"},
			{
				290, "\xb1\xb9\xbf\xd5\xc0\xba" " " "\xbf\xa9\xb7\xaf" " " ""
				"\xc0\xc7\xb0\xdf\xc0\xbb" " " "\xb5\xe8\xb0\xed" ", " ""
				"\xb0\xe1\xc1\xa4\xc0\xbb" " " "\xb3\xbb\xb7\xc8\xb4\xd9" ". " ""
				"\xc7\xd7\xbb\xf3" " " "\xb1\xd7\xb4\xc2" " " ""
				"\xb1\xc7\xc0\xa7\xb8\xa6" " " "\xc0\xd2\xc1\xf6" " " ""
				"\xbe\xca\xbe\xd2\xb4\xd9" "."
			},
			{
				360, "\xb1\xd7\xb5\xbf\xbe\xc8" " " "\xc1\xb6\xbd\xc5\xb5\xe9\xc0\xba" ""
				" " "\xb1\xb9\xbf\xd5\xc0\xcc" " " "\xc8\xa6\xbf\xa1" " " ""
				"\xb8\xf0\xbd\xc0\xc0\xbb" " " ""
				"\xb5\xe5\xb7\xaf\xb3\xbb\xb1\xe2\xb8\xa6" " " ""
				"\xb1\xe2\xb4\xd9\xb7\xc8\xb4\xd9" "."
			},
			{411, ""},
			{
				440, "\xbf\xa9\xb7\xaf\xba\xd0" ", " "\xb1\xb9\xbf\xd5\xb2\xb2\xbc\xad" ""
				" " "\xbf\xc0\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9" "."
			},
			{450, ""},
			{
				550, "\xc1\xa4\xbf\xc0\xb0\xa1" " " "\xb5\xc7\xbe\xfa\xb4\xd9" ".  " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xb9\xcc\xbb\xe7\xbf\xa1" " " ""
				"\xc2\xfc\xbc\xae\xc7\xd1\xb4\xd9" ". " "\xc1\xb6\xbd\xc5\xc0\xba" ""
				" " "\xb1\xd7" " " "\xb5\xda\xb8\xa6" " " ""
				"\xb5\xfb\xb8\xa5\xb4\xd9" ".  " "\xb1\xb9\xbf\xd5\xc0\xba" " " ""
				"\xbf\xd5\xc0\xda\xbf\xcd" " " "\xb0\xf8\xc1\xd6\xbf\xcd" " " ""
				"\xc7\xd4\xb2\xb2" " '" "\xc0\xa7\xb4\xeb\xc7\xd1" " " "\xb9\xe6" ""
				"'" "\xc0\xbb" " " "\xb0\xa1\xb7\xce\xc1\xfa\xb7\xaf" " " ""
				"\xbf\xb9\xb9\xe8\xb4\xe7\xc0\xb8\xb7\xce" " " ""
				"\xc7\xe2\xc7\xd1\xb4\xd9" "."
			},
			{687, ""},
			{
				728, "\xba\xce\xb5\xf0" " " "\xc0\xcc" " " "\xc5\xba\xbf\xf8\xc0\xbb" " "
				"" "\xb5\xe9\xbe\xee\xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0" ", " ""
				"\xc6\xf3\xc7\xcf" ". " "\xc0\xdc\xc0\xce\xc7\xd1" " " ""
				"\xbc\xd2\xbc\xdb\xc0\xc7" " " "\xc8\xf1\xbb\xfd\xb9\xb0\xc0\xcc" ""
				" " "\xb5\xc8" " " "\xb3\xaa\xc0\xc7" " " ""
				"\xc1\xb6\xc4\xab\xb8\xa6" " " "\xb4\xeb\xbd\xc5\xc7\xd8" " " ""
				"\xb1\xb9\xbf\xd5\xb2\xb2" " " ""
				"\xb0\xa3\xc3\xbb\xc7\xd5\xb4\xcf\xb4\xd9" "."
			},
			{805, "\xbe\xcb\xb0\xda\xb4\xd9" ", " "\xbe\xcb\xb0\xda\xb4\xd9" "...."},
			{840, ""},
			{
				1060, "\xb1\xb9\xbf\xd5\xc0\xba" " " "\xc8\xb2\xbd\xc7" " " ""
				"\xb0\xb6\xb7\xaf\xb8\xae\xbf\xa1" " " ""
				"\xc2\xfc\xbc\xae\xc7\xdf\xb0\xed" ", " "\xbf\xd5\xc0\xda\xbf\xcd" ""
				" " "\xb0\xf8\xc1\xd6\xb0\xa1" " " "\xb1\xd7" " " ""
				"\xb5\xda\xb8\xa6" " " "\xc0\xa7\xb0\xe8" " " ""
				"\xc1\xfa\xbc\xad\xbf\xa1" " " "\xb5\xfb\xb6\xf3" " " ""
				"\xc0\xda\xb8\xae\xc7\xdf\xb4\xd9" ". " "\xb1\xb9\xbf\xd5\xc0\xba" ""
				" " "\xc0\xcc\xbe\xdf\xb1\xe2\xc7\xcf\xb4\xc2" " " ""
				"\xb0\xcd\xc0\xbb" " " "\xb1\xdd\xc7\xdf\xb0\xed" " " ""
				"\xb0\xe6\xb0\xc7\xc7\xcf\xb5\xb5\xb7\xcf" " " ""
				"\xbf\xe4\xb1\xb8\xc7\xdf\xb4\xd9" "."
			},
			{1220, ""},
		}
	}, /* a3_vf.hns */
	{
		"a4_vf.hns", {
			{
				90, "\xb9\xcc\xbb\xe7" " " "\xb5\xbf\xbe\xc8" ",  " "\xc8\xb2\xbd\xc7" ""
				" " "\xbd\xc4\xb4\xe7\xc0\xc7" " " "\xb0\xfc\xb8\xae\xc0\xda" ", " ""
				"\xb0\xed\xb1\xe2" " " "\xb1\xc1\xb4\xc2" " " "\xbb\xe7\xb6\xf7" ""
				", " "\xbb\xa7" " " "\xb1\xc1\xb4\xc2" " " ""
				"\xbb\xe7\xb6\xf7\xb0\xfa" " " "\xb1\xd7" " " ""
				"\xba\xb8\xc1\xb6\xb5\xe9\xc0\xba" " " "\xb8\xb8\xc2\xf9\xc0\xbb" ""
				" " "\xc1\xd8\xba\xf1\xc7\xcf\xb4\xc0\xb6\xf3" " " ""
				"\xba\xd0\xc1\xd6\xc7\xdf\xb4\xd9" "."
			},
			{
				200, "\xc0\xfc\xbc\xb3\xc0\xfb\xc0\xce" " " "\xbd\xc4\xbf\xe5\xc0\xbb" ""
				" " "\xb0\xa1\xc1\xf6\xb0\xed" " " "\xc0\xd6\xb4\xc2" " " ""
				"\xb1\xba\xc1\xd6\xb0\xa1" " " "\xbc\xd5\xb4\xeb\xc1\xf6" " " ""
				"\xbe\xca\xc0\xba" " " "\xc0\xbd\xbd\xc4\xc0\xba" " " ""
				"\xb4\xd9\xb8\xa5" " " "\xc0\xcc\xb5\xe9\xc0\xcc" " " ""
				"\xb8\xd4\xb0\xd4" " " "\xb5\xc9" " " "\xb0\xcd\xc0\xcc\xb0\xed" ""
				", " "\xba\xb8\xc5\xeb" " " "\xbb\xe7\xb6\xf7\xb5\xe9\xc0\xba" " " ""
				"\xb1\xd7" " " "\xbd\xc4\xb4\xe7\xc0\xb8\xb7\xce\xba\xce\xc5\xcd" ""
				" " "\xb3\xb2\xc0\xba" " " "\xb0\xcd\xc0\xbb" " " ""
				"\xb1\xb8\xc0\xd4\xc7\xcf\xb4\xc2" " " "\xb0\xcd\xc0\xbb" " " ""
				"\xb2\xa8\xb8\xae\xc1\xf6" " " "\xbe\xca\xbe\xd2\xb4\xd9" "."
			},
			{300, ""},
			{
				320, "\xb8\xb9\xc0\xba" " " "\xbb\xe7\xb6\xf7\xb5\xe9\xc0\xcc" " " ""
				"\xc2\xfc\xbc\xae\xc7\xd1" " " "\xb8\xb8\xc2\xf9\xc0\xba" " " ""
				"\xb8\xf0\xb5\xe7" " " "\xc1\xb6\xbd\xc5\xb5\xe9\xc0\xcc" " " ""
				"\xbc\xad\xb5\xd1\xb7\xaf" " " "\xc2\xfc\xbc\xae\xc7\xcf\xb4\xc2" ""
				" " "\xb8\xda\xc1\xf8" " " ""
				"\xc0\xe5\xb0\xfc\xc0\xcc\xbe\xfa\xb4\xd9" "."
			},
			{
				366, "\xbf\xa9\xb7\xaf\xba\xd0" ", " "\xb1\xb9\xbf\xd5\xb2\xb2\xbc\xad" ""
				" " "\xbd\xc4\xbb\xe7\xc1\xdf\xc0\xcc\xbd\xca\xb4\xcf\xb4\xd9" "."
			},
			{393, ""},
			{
				610, "\xbf\xc0\xc1\xf7" " " "\xc8\xb2\xbd\xc7" " " ""
				"\xb0\xa1\xc1\xb7\xb5\xe9\xb8\xb8\xc0\xcc" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xc7" " " "\xc5\xd7\xc0\xcc\xba\xed\xbf\xa1" ""
				" " "\xbe\xc9\xc0\xbb" " " "\xbc\xf6" " " ""
				"\xc0\xd6\xbe\xfa\xb0\xed" ", " "\xb0\xf8\xc0\xdb" " " ""
				"\xba\xce\xc0\xce\xb5\xe9\xb8\xb8\xc0\xcc" " " "\xb1\xd7" " " ""
				"\xbe\xd5\xbf\xa1" " " "\xbe\xc9\xc0\xbb" " " "\xbc\xf6" " " ""
				"\xc0\xd6\xbe\xfa\xb4\xd9" "."
			},
			{685, ""},
			{
				705, "\xc0\xda" ", " "\xb3\xbb" " " "\xbe\xc6\xb5\xe9\xbe\xc6" ", " ""
				"\xbf\xec\xb8\xae\xb8\xa6" " " "\xc0\xa7\xc7\xd1" " " ""
				"\xc0\xdc\xc4\xa1\xb8\xa6" " " "\xc0\xdf" " " ""
				"\xc1\xd8\xba\xf1\xc7\xcf\xb0\xed" " " "\xc0\xd6\xb4\xc0\xb3\xc4" ""
				"?"
			},
			{747, ""},
			{
				760, "\xb8\xb8\xc2\xf9" " " "\xb5\xbf\xbe\xc8" ", " ""
				"\xbc\xbc\xb0\xa1\xc1\xf6" " " "\xc4\xda\xbd\xba\xb0\xa1" " " ""
				"\xb0\xa2\xb0\xa2" " " "\xbf\xa9\xbc\xb8" " " ""
				"\xc1\xa2\xbd\xc3\xbf\xa1" " " "\xb4\xe3\xb0\xdc\xc1\xae" " " ""
				"\xc1\xa6\xb0\xf8\xb5\xc7\xbe\xfa\xb4\xd9" ": " ""
				"\xbd\xba\xc7\xc1\xbf\xcd" " " "\xc1\xa6" "1" "\xc4\xda\xbd\xba" ""
				", " "\xb1\xb8\xbf\xee" " " "\xb0\xed\xb1\xe2\xbf\xcd" " " ""
				"\xb5\xf0\xc0\xfa\xc6\xae" "..."
			},
			{847, ""},
		}
	}, /* a4_vf.hns */
	{
		"a5_vf.hns", {
			{
				13, "\xb8\xb8\xc2\xf9" " " "\xc8\xc4\xbf\xa1\xb4\xc2" " " ""
				"\xb0\xfa\xc0\xcf\xc0\xcc" ",..."
			},
			{36, ""},
			{
				90, "..." "\xb1\xd7\xb7\xb1" " " "\xc8\xc4" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xb1\xb9\xbb\xe7\xbf\xa1" " " ""
				"\xb8\xee" " " "\xbd\xc3\xb0\xa3\xb5\xbf\xbe\xc8" " " ""
				"\xc0\xfc\xb3\xe4\xc7\xdf\xb4\xd9" ". " "\xc0\xcc\xc1\xee\xc0\xbd" ""
				" " "\xb8\xcd\xb6\xdf\xb3\xf3" " " ""
				"\xc8\xc4\xc0\xdb\xba\xce\xc0\xce\xc0\xc7" " " ""
				"\xb9\xe6\xbf\xa1\xbc\xad" " " "\xc1\xf7\xb9\xab\xb8\xa6" " " ""
				"\xbc\xf6\xc7\xe0\xc7\xcf\xb4\xc2" " " "\xb0\xcd\xc0\xbb" " " ""
				"\xc1\xc1\xbe\xc6\xc7\xdf\xb0\xed" ", " "\xc1\xbe\xc1\xbe" " " ""
				"\xbd\xc5\xc7\xcf\xb5\xe9\xb0\xfa" " " "\xc7\xd4\xb2\xb2\xc0\xce" ""
				" " "\xc3\xa4" " " "\xbc\xf6\xc7\xe0\xc7\xdf\xb4\xd9" "."
			},
			{
				200, "\xb1\xd7\xb0\xa1" " " "\xba\xf1\xb9\xd0\xbd\xba\xb7\xb4\xb0\xd4" ""
				" " "\xb0\xe1\xc8\xa5\xc7\xd1" " " "\xb1\xd7\xb3\xe0\xc0\xc7" " " ""
				"\xb0\xc5\xc3\xb3\xbf\xa1\xbc\xad" " " ""
				"\xc0\xda\xbd\xc5\xbf\xa1\xb0\xd4" " " "\xc7\xca\xbf\xe4\xc7\xd1" ""
				" " "\xc6\xf2\xc8\xad\xbf\xcd" " " ""
				"\xc6\xf2\xbf\xc2\xc7\xd4\xc0\xbb" " " "\xbe\xf2\xc0\xbb" " " ""
				"\xbc\xf6" " " "\xc0\xd6\xbe\xfa\xb4\xd9" "."
			},
			{296, ""},
			{
				310, "\xbc\xbc" " " "\xbd\xc3\xb0\xa3" " " "\xb5\xbf\xbe\xc8" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xbc\xad\xb7\xf9\xb8\xa6" " " ""
				"\xbc\xbc\xb9\xd0\xc7\xcf\xb0\xd4" " " ""
				"\xb0\xcb\xc5\xe4\xc7\xdf\xb4\xd9" "...."
			},
			{345, ""},
			{
				360, "\xc4\xab\xb5\xe5\xb3\xee\xc0\xcc\xb4\xc2" " " ""
				"\xbd\xc5\xc7\xcf\xb5\xe9\xbf\xa1\xb0\xd4" " " ""
				"\xc0\xd6\xbe\xee\xbc\xad" ", " "\xb1\xba\xc1\xd6\xb8\xa6" " " ""
				"\xb5\xfb\xb6\xf3\xb4\xd9\xb4\xcf\xc1\xf6" " " ""
				"\xbe\xca\xbe\xc6\xb5\xb5" " " "\xb5\xc7\xb4\xc2" " " ""
				"\xb5\xe5\xb9\xae" " " "\xbc\xf8\xb0\xa3" " " "\xc1\xdf" " " ""
				"\xc7\xcf\xb3\xaa\xbf\xb4\xb4\xd9" ". " "\xbe\xee\xb6\xb2" " " ""
				"\xc0\xcc\xb4\xc2" " " "\xbe\xc6\xc1\xd6" " " ""
				"\xbf\xad\xc1\xa4\xc0\xfb\xc0\xb8\xb7\xce" " " ""
				"\xc4\xab\xb5\xe5\xb3\xee\xc0\xcc\xb8\xa6" " " "\xc7\xdf\xb4\xd9" ""
				"."
			},
			{
				460, "\xc4\xab\xb5\xe5" " " "\xc5\xd7\xc0\xcc\xba\xed\xc0\xba" " " ""
				"\xc1\xa6\xb9\xfd" " " "\xbc\xf6\xc0\xd4\xc0\xbb" " " ""
				"\xc1\xd6\xb1\xe2\xb5\xb5" " " "\xc7\xcf\xb4\xc2\xb5\xa5" ", " ""
				"\xbe\xee\xb6\xb2" " " "\xc0\xcc\xb4\xc2" " " ""
				"\xbc\xd3\xc0\xd3\xbc\xf6\xb8\xa6" " " "\xbd\xe1\xbc\xad" " " ""
				"\xc8\xb2\xbd\xc7\xc0\xbb" " " ""
				"\xba\xd2\xb8\xed\xbf\xb9\xbd\xba\xb7\xb4\xb0\xd4" " " ""
				"\xc7\xcf\xb4\xc2" " " "\xb0\xcd\xb5\xb5" " " ""
				"\xc1\xd6\xc0\xfa\xc7\xcf\xc1\xf6" " " "\xbe\xca\xbe\xd2\xb4\xd9" ""
				"."
			},
			{560, ""},
			{
				665, "\xb4\xd9\xb8\xa5" " " "\xc0\xcc\xb5\xe9\xc0\xba" " " ""
				"\xb4\xe7\xb1\xb8\xbf\xcd" " " "\xb0\xb0\xc0\xcc" " " ""
				"\xb1\xe2\xbc\xfa\xc0\xcc" " " "\xc7\xca\xbf\xe4\xc7\xd1" " " ""
				"\xb0\xd4\xc0\xd3\xc0\xbb" " " "\xc7\xcf\xb1\xe2\xb5\xb5" " " ""
				"\xc7\xcf\xc1\xf6\xb8\xb8" ", " "\xb0\xdd\xc1\xb6\xb3\xf4\xc0\xba" ""
				" " "\xb4\xeb\xc8\xad\xbe\xdf\xb8\xbb\xb7\xce" " " ""
				"\xba\xa3\xb8\xa3\xbb\xe7\xc0\xcc\xc0\xaf\xbf\xa1\xbc\xad" " " ""
				"\xb0\xa1\xc0\xe5" " " "\xb3\xf4\xc0\xcc" " " ""
				"\xc0\xce\xc1\xa4\xb9\xde\xb4\xc2" " " "\xb1\xe2\xbb\xdd" " " ""
				"\xc1\xdf" " " "\xc7\xcf\xb3\xaa\xc0\xcc\xb4\xd9" "."
			},
			{766, ""},
		}
	}, /* a5_vf.hns */
	{
		"a6_vf.hns", {
			{
				45, "\xbf\xc0\xc8\xc4" " " "\xb4\xd9\xbc\xb8\xbd\xc3\xb0\xa1" " " ""
				"\xb4\xd9\xb0\xa1\xbf\xc0\xc0\xda" ", " "\xc1\xf7\xb9\xab\xb8\xa6" ""
				" " "\xb8\xb6\xc4\xa3" " " "\xb1\xb9\xbf\xd5\xc0\xba" " " ""
				"\xc1\xa4\xbf\xf8\xbf\xa1\xbc\xad" " " ""
				"\xbb\xea\xc3\xa5\xc7\xcf\xb0\xed" " " "\xb0\xc7\xb9\xb0\xc0\xbb" ""
				" " "\xb5\xd1\xb7\xaf\xba\xbb\xb4\xd9" "."
			},
			{
				90, "... " "\xb4\xeb\xb8\xae\xbc\xae" " " ""
				"\xbe\xc8\xb6\xe3\xbf\xa1\xbc\xad" " " ""
				"\xb1\xe2\xb4\xd9\xb8\xae\xb0\xed" " " "\xc0\xd6\xb4\xf8" " " ""
				"\xbd\xc5\xc7\xcf\xb5\xe9\xb0\xfa" " " "\xc7\xd4\xb2\xb2" " " ""
				"\xc7\xdf\xb4\xd9" "."
			},
			{160, ""},
			{
				170, "\xb8\xee\xb8\xee" " " "\xc6\xaf\xba\xb0\xc7\xd1" " " ""
				"\xc0\xcc\xb5\xe9\xb8\xb8\xc0\xcc" " " "\xb1\xb9\xbf\xd5\xb0\xfa" ""
				" " "\xc7\xd4\xb2\xb2" " " "\xc7\xd2" " " "\xbc\xf6" " " ""
				"\xc0\xd6\xbe\xfa\xb4\xd9" "."
			},
			{208, ""},
			{
				295, "\xb1\xd7\xb5\xe9\xc0\xba" " " "\xbe\xc6\xb7\xa1\xc3\xfe\xc0\xc7" ""
				" " "\xb0\xb6\xb7\xaf\xb8\xae\xb8\xa6" " " "\xc1\xf6\xb3\xaa" " " ""
				"\xc5\xd7\xb6\xf3\xbd\xba\xb7\xce" " " "\xb1\xd7\xb8\xae\xb0\xed" ""
				" " "\xb9\xb0\xc0\xc7" " " "\xc8\xad\xb4\xdc\xc0\xb8\xb7\xce" " " ""
				"\xb3\xaa\xbe\xc6\xb0\xac\xb4\xd9" ". " "\xb1\xb9\xbf\xd5\xc0\xba" ""
				" " "\xb1\xd7\xb0\xf7\xbf\xa1\xbc\xad" " " "\xbc\xba\xc0\xc7" " " ""
				"\xbf\xdc\xb0\xfc\xc0\xbb" " " "\xb0\xa8\xbb\xf3\xc7\xcf\xb4\xc2" ""
				" " "\xb0\xcd\xc0\xbb" " " "\xc1\xc1\xbe\xc6\xc7\xdf\xb4\xd9" "."
			},
			{
				430, "\xb0\xa8\xb1\xd6\xbf\xf8\xc0\xb8\xb7\xce\xba\xce\xc5\xcd" " " ""
				"\xc4\xdd\xb7\xce\xb3\xaa\xb5\xe5\xb1\xee\xc1\xf6" ", " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xc1\xf8\xc7\xe0\xc1\xdf\xc0\xce" ""
				" " "\xbf\xa9\xb7\xaf" " " "\xc0\xdb\xc7\xb0\xc0\xbb" " " ""
				"\xc2\xf7\xb7\xca\xc2\xf7\xb7\xca" " " ""
				"\xbc\xbc\xb9\xd0\xc7\xcf\xb0\xd4" " " ""
				"\xbb\xec\xc7\xc7\xbe\xfa\xb4\xd9" "."
			},
			{492, ""},
			{
				945, "\xc0\xcc" " " "\xc7\xe0\xc2\xf7\xb4\xc2" " " "\xbc\xd2" " " ""
				"\xba\xa3\xb4\xcf\xbd\xba\xbf\xa1\xbc\xad" " " "\xb3\xa1\xc0\xcc" ""
				" " "\xb3\xad\xb4\xd9" ". " "\xb1\xb9\xbf\xd5\xc0\xba" " " ""
				"\xc3\xd1\xb5\xb6\xc0\xc7" " " ""
				"\xb5\xb5\xbd\xc3\xb7\xce\xba\xce\xc5\xcd" " " "\xbf\xc2" " " ""
				"\xb0\xc5\xb7\xed\xb9\xe8\xb8\xa6" " " "\xb0\xa1\xc1\xf6\xb0\xed" ""
				" " "\xc0\xd6\xb4\xc2\xb5\xa5" ", " "\xc1\xc1\xc0\xba" " " ""
				"\xb3\xaf\xbe\xbe\xbf\xa1\xb4\xc2" " " "\xbf\xee\xc7\xcf\xc0\xc7" ""
				" " "\xb8\xbc\xc0\xba" " " "\xb9\xb0\xc0\xa7\xb7\xce" ", " ""
				"\xbd\xc5\xc7\xcf\xb5\xe9\xb0\xfa" " " "\xc7\xd4\xb2\xb2" " " ""
				"\xc0\xcc" " " "\xb9\xe8\xb8\xa6" " " "\xc5\xb8\xb0\xed" " " ""
				"\xb3\xaa\xbf\xc2\xb4\xd9" "."
			},
		}
	}, /* a6_vf.hns */
	{
		"a7_vf.hns", {
			{
				40, "\xc0\xdf" " " "\xbf\xad\xb7\xc1\xc1\xf8" " " "\xb4\xeb\xb7\xce" ""
				", " "\xbf\xa9\xb8\xa7" " " "\xc0\xfa\xb3\xe1\xbf\xa1\xb4\xc2" " " ""
				"\xb1\xb9\xbf\xd5\xc0\xba" " " "\xb0\xf8\xbf\xf8\xc0\xc7" " " ""
				"\xbd\xa3" " " "\xbc\xd3\xbf\xa1" " " "\xb8\xb6\xb7\xc3\xb5\xc8" " "
				"" "\xb0\xa3\xb4\xdc\xc7\xd1" " " "\xbd\xc4\xbb\xe7\xb7\xce" " " ""
				"\xc0\xfa\xb3\xe1\xc0\xbb" " " "\xb4\xeb\xbd\xc5\xc7\xd1\xb4\xd9"
			},
			{107, ""},
			{
				198, "\xb9\xe3\xc0\xcc" " " "\xb1\xed\xbe\xee\xc1\xf6\xb0\xed" ", " ""
				"\xb1\xb9\xbf\xd5\xc0\xcc" " " "\xc4\xa7\xbd\xc7\xbf\xa1" " " ""
				"\xb5\xe9\xb8\xe9" " " "\xb8\xf0\xb5\xe7" " " "\xc0\xcc\xb0\xa1" " "
				"" "\xbc\xba\xc0\xb8\xb7\xce" " " ""
				"\xb5\xb9\xbe\xc6\xb0\xa3\xb4\xd9" "."
			},
			{245, ""},
			{
				355, "\xba\xce\xc0\xce\xb5\xe9\xc0\xba" " " "\xb1\xb9\xbf\xd5\xc0\xcc" ""
				" " "\xbe\xc6\xc4\xa7" " " "\xbf\xb9\xba\xb9" " " ""
				"\xc0\xc7\xbd\xc4\xbf\xa1" " " "\xc3\xca\xb4\xeb\xb9\xde\xc1\xf6" ""
				" " "\xb8\xf8\xc7\xdf\xb0\xed" ", " "\xc0\xfa\xb3\xe1" " " ""
				"\xc4\xa7\xbd\xc7\xbf\xa1" " " "\xb5\xe5\xb4\xc2" " " ""
				"\xc0\xc7\xbd\xc4\xbf\xa1\xb5\xb5" " " ""
				"\xc3\xca\xb4\xeb\xb9\xde\xc1\xf6" " " ""
				"\xb8\xf8\xc7\xcf\xbf\xb4\xb4\xd9" ". " "\xb1\xb9\xbf\xd5\xc0\xba" ""
				" " "\xb1\xd7\xb5\xe9\xbf\xa1\xb0\xd4" " " ""
				"\xbb\xec\xb7\xd5\xbf\xa1\xbc\xad" " " ""
				"\xc0\xfa\xb3\xe1\xc0\xce\xbb\xe7\xb8\xa6" " " ""
				"\xc7\xcf\xb5\xb5\xb7\xcf" " " "\xc7\xcf\xbf\xb4\xb4\xd9" "."
			},
			{442, ""},
			{
				454, "\xc0\xcc" " " "\xc7\xcf\xc1\xf6\xc0\xc7" " " "\xb1\xe4" " " ""
				"\xb9\xe3\xc0\xcc" " " "\xba\xce\xc0\xce\xb5\xe9\xc0\xc7" " " ""
				"\xc8\xde\xbd\xc4\xc0\xbb" " " "\xc0\xa7\xc7\xd8" " " ""
				"\xc3\xe6\xba\xd0\xc7\xcf\xb1\xe2\xb8\xa6" " " ""
				"\xba\xf4\xb0\xda\xbd\xc0\xb4\xcf\xb4\xd9" "."
			},
			{512, ""},
			{
				647, "\xb1\xd7\xb8\xae\xb0\xed" " " "\xc4\xa7\xbd\xc7\xb7\xce" " " ""
				"\xb5\xe5\xb4\xc2" " " "\xc0\xc7\xbd\xc4\xc0\xcc" " " ""
				"\xbd\xc3\xc0\xdb\xb5\xc8\xb4\xd9" ". " "\xbe\xc6\xc4\xa7" " " ""
				"\xbf\xb9\xba\xb9" " " "\xc0\xc7\xbd\xc4\xbf\xa1" " " ""
				"\xc2\xfc\xbc\xae\xc7\xd1" " " "\xbd\xc5\xc7\xcf\xb5\xe9\xc0\xba" ""
				" " "\xc0\xcc\xb0\xf7\xbf\xa1\xb5\xb5" " " ""
				"\xc2\xfc\xbc\xae\xc7\xcf\xb1\xe2\xb7\xce" " " ""
				"\xb5\xc7\xbe\xee\xc0\xd6\xb4\xd9" "."
			},
			{722, ""},
			{
				733, "\xb4\xa9\xb1\xb8\xbf\xa1\xb0\xd4" " " "\xc3\xd0\xb4\xeb\xb8\xa6" ""
				" " "\xb5\xe9\xb0\xd4" " " "\xc7\xd2\xb1\xee\xbf\xe4" ", " ""
				"\xc6\xf3\xc7\xcf" "?"
			},
			{763, ""},
			{
				925, "\xb1\xb9\xbf\xd5\xc0\xba" " " "\xc0\xcf\xbb\xf3\xc0\xc7" " " ""
				"\xc7\xe0\xc0\xa7\xb8\xa6" " " "\xb0\xed\xb1\xcd\xc7\xd1" " " ""
				"\xc7\xe0\xc0\xa7\xb7\xce" " " "\xb9\xd9\xb2\xd9\xbe\xee" " " ""
				"\xb3\xf5\xbe\xd2\xb4\xd9" ". " "\xc4\xa7\xbd\xc7\xb1\xee\xc1\xf6" ""
				" " "\xc3\xd0\xb4\xeb\xb8\xa6" " " "\xb5\xe9\xb0\xed" " " ""
				"\xb0\xa5" " " "\xbc\xf6" " " "\xc0\xd6\xb4\xc2" " " ""
				"\xb1\xc7\xb8\xae\xb4\xc2" " " "\xc0\xaf\xb8\xed" " " ""
				"\xc0\xce\xbb\xe7\xbf\xa1\xb0\xd4" " " ""
				"\xc1\xd6\xbe\xee\xc1\xb3\xb0\xed" ", " "\xc1\xbe\xc1\xbe" " " ""
				"\xb9\xe6\xb9\xae\xc7\xd1" " " ""
				"\xbf\xdc\xb1\xb9\xc0\xce\xbf\xa1\xb0\xd4\xb5\xb5" " " ""
				"\xc1\xd6\xbe\xee\xc1\xb3\xb4\xd9" "..."
			},
			{1048, ""},
			{
				1060, "..." "\xb1\xd7\xb4\xc2" " " "\xc4\xa7\xbd\xc7\xb1\xee\xc1\xf6" " " ""
				"\xb1\xb9\xbf\xd5\xb0\xfa" " " "\xc7\xd4\xb2\xb2" " " "\xb0\xa5" " "
				"" "\xbc\xf6" " " "\xc0\xd6\xb4\xc2" " " "\xb1\xc7\xb8\xae\xb8\xa6" ""
				" " "\xbe\xf2\xb0\xd4" " " "\xb5\xc8" " " ""
				"\xb0\xcd\xc0\xcc\xb4\xd9" "."
			},
			{1100, ""},
			{
				1115, "\xb1\xd7\xb7\xaf\xb3\xaa" " " "\xbf\xc0\xb4\xc3" " " ""
				"\xbe\xc6\xc4\xa7" "...."
			},
			{1135, ""},
			{
				1188, "\xb9\xdd\xbf\xaa\xc0\xcc\xb4\xd9" "! " ""
				"\xb9\xdd\xbf\xaa\xc0\xcc\xbe\xdf" "! " "\xc0\xcc\xb8\xae" " " ""
				"\xbf\xc0\xb0\xd4" ", " "\xb6\xf3\xb6\xfb\xb5\xe5" ", " "\xc0\xcc" ""
				" " "\xb9\xcc\xc4\xa3" " " "\xc0\xda\xb8\xa6" " " ""
				"\xb8\xb7\xb0\xd4" "!"
			},
			{
				1230, "\xbf\xa9\xb7\xaf\xba\xd0" ", " ""
				"\xbd\xba\xb1\xee\xba\xfc\xb7\xbc\xb6\xf3" " " ""
				"\xc8\xc4\xc0\xdb\xc0\xcc" " " ""
				"\xc0\xfa\xb3\xe1\xc0\xce\xbb\xe7\xb8\xa6" " " ""
				"\xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9" "."
			},
		}
	}, /* a7_vf.hns */
	{
		"a8_vf.hns", {
			{
				212, "\xb9\xcc\xc4\xa5\xb4\xeb\xb7\xce" " " "\xb9\xcc\xc4\xa3" " " ""
				"\xc0\xda\xb0\xa1" " " "\xc0\xcc" " " "\xbc\xba\xc0\xbb" " " ""
				"\xc6\xc4\xb1\xab\xc7\xcf\xb8\xae\xb6\xf3\xb0\xed" " " ""
				"\xb4\xa9\xb0\xa1" " " "\xb2\xde\xbf\xa1\xb3\xaa" " " ""
				"\xbb\xfd\xb0\xa2\xc7\xdf\xb0\xda\xb4\xc2\xb0\xa1" "? " ""
				"\xb4\xe7\xbd\xc5\xc0\xba" " " "\xb4\xd9\xbd\xc3" " " ""
				"\xbd\xc3\xc0\xdb\xc7\xd8\xbe\xdf" " " "\xc7\xd1\xb4\xd9" ". " ""
				"\xc1\xf6\xb1\xdd" " " "\xb4\xe7\xc0\xe5" "."
			},
			{320, ""},
		}
	}, /* a8_vf.hns */
	{
		"a9_vf.hns", {
			{
				30, "\xc0\xcc" " " "\xc0\xcf\xbf\xa1" " " "\xb4\xeb\xc7\xd1" " " ""
				"\xc0\xda\xb3\xd7\xc0\xc7" " " "\xbf\xad\xc1\xa4\xc0\xbb" " " ""
				"\xb1\xb9\xbf\xd5\xb2\xb2" " " "\xb8\xbb\xbe\xb8" " " ""
				"\xb5\xe5\xb7\xc8\xb3\xd7" ", " "\xb6\xf3\xb6\xfb\xb5\xe5" ". " ""
				"\xb1\xb9\xbf\xd5\xb2\xb2\xbc\xad\xb4\xc2" " " ""
				"\xc0\xda\xb3\xd7\xb8\xa6" " " "\xb3\xf4\xc0\xcc" " " ""
				"\xc6\xf2\xb0\xa1\xc7\xcf\xb0\xed" " " ""
				"\xbd\xcd\xbe\xee\xc7\xcf\xbd\xc3\xb3\xd7" "."
			},
			{125, ""},
			{
				200, "\xba\xc0\xb6\xa5\xc0\xba" " " "\xc0\xda\xb3\xd7\xb0\xa1" " " ""
				"\xb0\xfc\xb0\xe8\xc7\xdf\xb4\xf8" " " "\xc0\xcf\xbf\xa1" " " ""
				"\xb0\xfc\xc7\xd8" " " "\xbf\xec\xb8\xae\xbf\xa1\xb0\xd4" " " ""
				"\xb8\xf0\xb5\xe7" " " "\xb0\xcd\xc0\xbb" " " ""
				"\xb8\xbb\xc7\xdf\xb3\xd7" "."
			},
			{
				300, "\xbf\xc0\xb4\xc3" " " "\xc0\xcf\xc0\xbb" " " ""
				"\xba\xb8\xb8\xe9\xbc\xad" " " "\xb8\xf0\xb5\xe7" " " ""
				"\xb3\xeb\xbf\xa9\xbf\xf2\xc0\xcc" " " ""
				"\xbb\xe7\xb6\xf3\xc1\xb3\xb3\xd7" ". " "\xbf\xc0\xb4\xc3" " " ""
				"\xc0\xcf\xc0\xba" " " "\xb9\xe0\xc7\xf4\xc1\xf6\xb1\xe2" " " ""
				"\xba\xb8\xb4\xd9\xb4\xc2" " " "\xbe\xee\xb5\xd2\xbc\xd3\xbf\xa1" ""
				" " "\xb9\xaf\xc7\xf4\xc1\xae\xbe\xdf\xb8\xb8" " " ""
				"\xc7\xcf\xb0\xda\xc1\xf6" "."
			},
			{
				400, "\xb8\xee\xb8\xee" " " "\xc0\xcc\xbb\xf3\xc7\xd1" " " ""
				"\xc1\xa1\xc0\xcc" " " "\xb3\xb2\xbe\xc6\xc0\xd6\xb1\xe2\xb4\xc2" ""
				" " "\xc7\xcf\xb3\xd7" ". " "\xc7\xcf\xc1\xf6\xb8\xb8" ", " ""
				"\xc0\xda\xb3\xd7\xb0\xa1" " " "\xbe\xee\xb5\xf4" " " ""
				"\xb0\xa1\xb5\xe7\xc1\xf6" ", " "\xbf\xec\xb8\xae\xc0\xc7" " " ""
				"\xb4\xeb\xbb\xe7\xb7\xce" " " "\xb4\xeb\xc1\xa2\xb9\xde\xc0\xbb" ""
				" " "\xb8\xb8\xc7\xd1" " " "\xc3\xe6\xba\xd0\xc7\xd1" " " ""
				"\xc0\xda\xb0\xdd\xc0\xba" " " "\xc0\xd6\xb3\xd7" "."
			},
			{515, ""},
		}
	}, /* a9_vf.hns */
};

static char const *const versaillesZTmessages[] = {
	"\xa5\xd8\xab" "e" "\xb8" "T" "\xa4\xee\xb6" "}" "\xb1\xd2\xb3" "o" ""
	"\xae\xb0\xaa\xf9\xa1" "C",                                           /*   0 */
	"\xb3" "o" "\xae\xb0\xaa\xf9\xac" "O" ""
	"\xc2\xea\xb5\xdb\xaa\xba\xa1" "C",                                   /*   1 */
	"\xb3" "o" "\xae\xb0\xaa\xf9\xac" "O" ""
	"\xc2\xea\xb5\xdb\xaa\xba\xa1" "C",                                   /*   2 */
	"\xb3" "o" "\xad\xd3\xc2" "d" "\xa4" "l" "\xac" "O" ""
	"\xaa\xc5\xaa\xba\xa1" "C",                                           /*   3 */
	"\xa7" "A" "\xb5" "L" "\xaa" "k" ""
	"\xb1\xb5\xc4\xb2\xa8\xec\xa8\xba\xad\xd3\xbb\\\xa4" "l" "\xa1" "C",  /*   4 */
	"\xb3" "o" "\xb4\xca\xbe\xef\xa4" "l" "\xbe\xf0\xa4" "W" "\xa8" "S" ""
	"\xa6\xb3\xaa" "F" "\xa6\xe8\xa1" "C",                                /*   5 */
	"\xb3" "o" "\xa4\xa3\xac" "O" "\xbe\xef\xa4" "l" "\xbe\xf0\xa1" "I",  /*   6 */
	"\xa4\xd3\xb7" "t" "\xa4" "F" "\xa1" "C",                             /*   7 */
	"\xb3" "o" "\xad\xd3\xa4" "p" "\xbd" "c" "\xa4" "l" "\xa4" "W" ""
	"\xc2\xea\xa4" "F" "\xa1" "C",                                        /*   8 */
	"\xa7" "A" "\xa5" "i" "\xa5" "H" "\xa5\xb4\xb6" "}" ""
	"\xa8\xba\xae\xb0\xaa\xf9\xa1" "C",                                   /*   9 */
	"\xa7" "A" "\xbb\xdd\xad" "n" "\xa4" "@" "\xa8\xc7\xaa" "F" ""
	"\xa6\xe8\xa1" "A" "\xa4" "~" ""
	"\xaf\xe0\xb1\xb5\xc4\xb2\xa8\xec\xac\xb5\xbc" "u" "\xa1" "C",        /*  10 */
	"\xb3" "o" "\xad\xd3\xaa\xe1\xb2" "~" "\xac" "O" ""
	"\xaa\xc5\xaa\xba\xa1" "C",                                           /*  11 */
	"\xb2" "{" "\xa6" "b" "\xa7" "A" "\xa5" "i" "\xa5" "H" "\xb8" "g" ""
	"\xb9" "L" "\xa4" "F" "\xa1" "C",                                     /*  12 */
	"\xa7" "A" "\xa8" "S" "\xa6\xb3\xae\xc9\xb6\xa1\xa4" "F" ""
	"\xb8\xd1\xae" "c" "\xa7\xca\xa1" "I",                                /*  13 */
	"\xad" "n" "\xac\xdd\xb9\xcf\xb5" "e" "\xa1" "A" "\xb2" "{" "\xa6" ""
	"b" "\xa4" "]" "\xa4\xd3\xb1\xdf\xa4" "F" "\xa1" "C",                 /*  14 */
	"\xb5\xa5\xa4" "@" "\xa4" "U" "\xa1" "I" "\xa7\xe2\xa7" "A" ""
	"\xaa\xba\xbd" "u" "\xaf\xc1\xa5\xe6\xb5\xb9\xaa\xf9\xa9\xd0\xa1" ""
	"C",                                                                  /*  15 */
	"\xa7" "A" "\xb7" "p" "\xa4\xa3\xa8\xec\xa8\xba\xb1" "i" ""
	"\xaf\xc8\xa1" "C",                                                   /*  16 */
	"\xa9\xb9\xc3\xc4\xbe\xaf\xae" "v" "\xb3" "B" "\xa1" "C",             /*  17 */
	"\xc4\xb5\xa7" "i" "\xa1" "G" "\xa7" "A" "\xa5" "i" "\xa5" "H" ""
	"\xab" "e" "\xa9\xb9\xa6\xb9\xbc" "h" ""
	"\xaa\xba\xb5\xb2\xa7\xf4\xa6" "a" "\xc2" "I" "\xa1" "A" ""
	"\xa6\xfd\xac" "O" "\xa7" "A" "\xa9" "|" ""
	"\xa5\xbc\xa7\xb9\xa6\xa8\xa5" "i" "\xa5" "H" "\xc4" "~" ""
	"\xc4\xf2\xb6" "i" "\xa6\xe6\xb9" "C" ""
	"\xc0\xb8\xa9\xd2\xbb\xdd\xaa\xba\xa4" "@" ""
	"\xa4\xc1\xa6\xe6\xb0\xca\xa1" "C" "\xab\xd8\xc4\xb3\xa7" "A" ""
	"\xb2" "{" "\xa6" "b" "\xb4" "N" "\xc0" "x" "\xa6" "s" "\xb9" "C" ""
	"\xc0\xb8\xb6" "i" "\xab\xd7\xa1" "C",                                /*  18 */
	"\xc4\xb5\xa7" "i" "\xa1" "G" "\xa7" "A" "\xa5" "i" "\xa5" "H" ""
	"\xab" "e" "\xa9\xb9\xa6\xb9\xbc" "h" ""
	"\xaa\xba\xb5\xb2\xa7\xf4\xa6" "a" "\xc2" "I" "\xa1" "A" ""
	"\xa6\xfd\xac" "O" "\xa7" "A" "\xa6\xb3\xa5" "i" "\xaf\xe0\xa9" "|" ""
	"\xa5\xbc\xa7\xb9\xa6\xa8\xa5" "i" "\xa5" "H" "\xc4" "~" ""
	"\xc4\xf2\xb6" "i" "\xa6\xe6\xb9" "C" ""
	"\xc0\xb8\xa9\xd2\xbb\xdd\xaa\xba\xa4" "@" ""
	"\xa4\xc1\xa6\xe6\xb0\xca\xa1" "C" "\xab\xd8\xc4\xb3\xa7" "A" ""
	"\xb2" "{" "\xa6" "b" "\xb4" "N" "\xc0" "x" "\xa6" "s" "\xb9" "C" ""
	"\xc0\xb8\xb6" "i" "\xab\xd7\xa1" "C",                                /*  19 */
	"\xa7" "A" "\xa4\xa3\xaf\xe0\xb1" "a" "\xb5\xdb\xa4" "@" "\xae" "y" ""
	"\xbc\xd3\xb1\xe8\xa8\xec\xb3" "B" "\xb6" "]" "\xa1" "I",             /*  20 */
	"\xb3" "o" "\xb8\xcc\xa8" "S" "\xa6\xb3\xa7" "O" "\xaa\xba\xaa" "F" ""
	"\xa6\xe8\xa1" "C",                                                   /*  21 */
	"\xa6" "A" "\xa8\xa3\xa1" "I",                                        /*  22 */
	"\xa4" "Z" "\xba\xb8\xc1\xc9\xa1\xd0\xae" "c" ""
	"\xa7\xca\xba\xc3\xb6\xb3",                                           /*  23 */
	"\xb8\xf4\xa9\xf6\xa4" "Q" "\xa5" "|" "\xae" "c" ""
	"\xa7\xca\xaa\xba\xb3\xb1\xbf\xd1",                                   /*  24 */
	"\xb0\xd1\xbe\\\xa4\xe5\xa5\xf3\xb0\xcf",                             /*  25 */
	"           " "\xc4" "~" "\xc4\xf2\xa5\xbb\xa6\xb8\xb9" "C" ""
	"\xc0\xb8",                                                           /*  26 */
	"           " "\xb6" "}" "\xa9" "l" "\xb7" "s" "\xb9" "C" ""
	"\xc0\xb8",                                                           /*  27 */
	"           " "\xb8\xfc\xa4" "J" "\xb9" "C" "\xc0\xb8\xb6" "i" ""
	"\xab\xd7",                                                           /*  28 */
	"           " "\xc0" "x" "\xa6" "s" "\xa5\xbb\xa6\xb8\xb9" "C" ""
	"\xc0\xb8",                                                           /*  29 */
	"           " "\xc5\xe3\xa5\xdc\xa6" "r" "\xb9\xf5\xa1" "G" "\xb6" ""
	"}" "\xb1\xd2",                                                       /*  30 */
	"           " "\xc5\xe3\xa5\xdc\xa6" "r" "\xb9\xf5\xa1" "G" ""
	"\xc3\xf6\xb3\xac",                                                   /*  31 */
	"           " "\xad\xb5\xbc\xd6\xa1" "G" "\xb6" "}" "\xb1\xd2",       /*  32 */
	"           " "\xad\xb5\xbc\xd6\xa1" "G" "\xc3\xf6\xb3\xac",          /*  33 */
	"           " "\xb1" "N" "\xa4" "@" ""
	"\xad\xba\xad\xb5\xbc\xd6\xbd\xc6\xbb" "s" "\xa6\xdc\xb5" "w" ""
	"\xba\xd0" "(20 Mb)",                                                 /*  34 */
	"           " "\xb1" "N" ""
	"\xa9\xd2\xa6\xb3\xad\xb5\xbc\xd6\xbd\xc6\xbb" "s" "\xa6\xdc\xb5" ""
	"w" "\xba\xd0" "(92 Mb)",                                             /*  35 */
	"           " "\xa4\xa3\xb1" "N" "\xad\xb5\xbc\xd6\xbd\xc6\xbb" "s" ""
	"\xa6\xdc\xb5" "w" "\xba\xd0" "(" "\xb1" "qCD" "\xbc\xbd\xa9\xf1" ""
	")",                                                                  /*  36 */
	nullptr,                                                              /*  37 */
	nullptr,                                                              /*  38 */
	"\xad\xb5\xb6" "q",                                                   /*  39 */
	"\xc2\xf7\xb6" "}" "\xb9" "C" "\xc0\xb8",                             /*  40 */
	"",                                                                   /*  41 */
	"\xb0\xd1\xc6" "[" "\xab\xb0\xb3\xf9",                                /*  42 */
	"\xbb" "s" "\xa7" "@" "\xb8" "s",                                     /*  43 */
	"           " "\xc4" "~" "\xc4\xf2\xa5\xbb\xa6\xb8\xb0\xd1\xb3" "X",  /*  44 */
	"           " "\xc0" "x" "\xa6" "s" "\xa5\xbb\xa6\xb8\xb0\xd1\xb3" ""
	"X",                                                                  /*  45 */
	"           " "\xb8\xfc\xa4" "J" "\xb0\xd1\xb3" "X" "\xb6" "i" ""
	"\xab\xd7",                                                           /*  46 */
	nullptr,                                                              /*  47 */
	"           Omni3D" "\xa1" "G" "\xbc\xd0\xb7\xc7",                    /*  48 */
	"           Omni3D" "\xa1" "G" "\xba" "C" "\xb3" "t",                 /*  49 */
	"           Omni3D" "\xa1" "G" "\xb7\xa5\xba" "C" "\xb3" "t",         /*  50 */
	"           Omni3D" "\xa1" "G" "\xa7\xd6\xb3" "t",                    /*  51 */
	"           Omni3D" "\xa1" "G" "\xb7\xa5\xa7\xd6\xb3" "t",            /*  52 */
	"\xbd" "T" "\xa9" "w",                                                /*  53 */
	"\xa8\xfa\xae\xf8",                                                   /*  54 */
	"\xaa\xc5\xa5\xd5\xc4\xe6\xa6\xec",                                   /*  55 */
	"\xb5" "L" "\xc0\xc9\xa6" "W",                                        /*  56 */
	"\xc4\xb5\xa7" "i" "\xa1" "G" ""
	"\xb7\xc7\xb3\xc6\xa9\xf1\xb1\xf3\xb3" "o" "\xad\xd3\xb9" "C" ""
	"\xc0\xb8\xa1" "C",                                                   /*  57 */
	"\xaa\xf0\xa6" "^",                                                   /*  58 */
	"\xab\xb0\xb3\xf9",                                                   /*  59 */
	"\xa6" "^" "\xa5" "D" "\xbf\xef\xb3\xe6",                             /*  60 */
	"\xa4\xe5\xa5\xf3\xb0\xcf\xba" "K" "\xad" "n",                        /*  61 */
	"\xab\xb0\xb3\xf9\xa4\xce\xaa\xe1\xb6\xe9\xa6" "a" "\xb9\xcf",        /*  62 */
	"\xab\xb0\xb3\xf9\xa4\xba\xb3\xa1\xb5\xb2\xba" "c" "\xb9\xcf",        /*  63 */
	"\xb5" "L" "\xaa" "k" "\xbc" "g" "\xa4" "J" "\xb5" "w" ""
	"\xba\xd0\xa1" "G" "\xb5" "w" "\xba\xd0\xa4" "w" "\xba\xa1",          /*  64 */
	nullptr,                                                              /*  65 */
	"\xa9\xf1\xa4" "J" "\xa5\xfa\xba\xd0\xa4\xf9",                        /*  66 */
	"\xa9\xf1\xa4" "J" "\xb2\xc4" "%d" "\xb1" "i" ""
	"\xa5\xfa\xba\xd0\xa4\xf9\xa8\xc3\xab\xf6\xa5\xf4\xa4" "@" ""
	"\xc1\xe4",                                                           /*  67 */
	"\xc3\xc0\xb3" "N",                                                   /*  68 */
	"\xb0\xf5\xac" "F" "\xaa\xcc",                                        /*  69 */
	"\xae" "c" "\xa7\xca\xa4" "H" "\xad\xfb",                             /*  70 */
	"\xab\xb0\xb3\xf9\xa4\xa4\xaa\xba\xa5\xcd\xac\xa1",                   /*  71 */
	"\xab\xb0\xb3\xf9\xa4\xce\xaa\xe1\xb6\xe9",                           /*  72 */
	"\xac\xf6\xa8\xc6\xaa\xed",                                           /*  73 */
	"\xaa\xfc\xaa" "i" "\xc3\xb9\xbc" "Q" "\xac" "u",                     /*  74 */
	"\xab\xb0\xb3\xf9",                                                   /*  75 */
	"\xac" "W" "\xb4" "Y",                                                /*  76 */
	"\xb0" "g" "\xae" "c",                                                /*  77 */
	"\xa9\xd4\xc5" "M" "\xae" "R" "\xbc" "Q" "\xac" "u",                  /*  78 */
	"\xbe\xef\xa4" "l" "\xb7\xc5\xab\xc7",                                /*  79 */
	"\xa4\xf4\xa6\xc0\xb0\xcf",                                           /*  80 */
	"\xba\xf1\xbd\xae\xa4" "j" "\xb9" "D",                                /*  81 */
	"\xa4" "j" "\xb0\xa8\xb4\\",                                          /*  82 */
	"\xa4" "p" "\xb0\xa8\xb4\\",                                          /*  83 */
	"\xaa\xe1\xb6\xe9",                                                   /*  84 */
	"\xab" "e" "\xae" "x",                                                /*  85 */
	"\xa4" "j" "\xb9" "B" "\xaa" "e",                                     /*  86 */
	"\xab" "n" "\xaf\xf3\xa9" "W",                                        /*  87 */
	"\xa5" "_" "\xaf\xf3\xa9" "W",                                        /*  88 */
	"\xb0\xea\xa4\xfd\xaa\xba\xb5\xe6\xb6\xe9",                           /*  89 */
	"\xbb" "R" "\xc6" "U",                                                /*  90 */
	"\xae\xfc\xac" "v" "\xa4\xa7\xac" "u",                                /*  91 */
	"\xa6" "u" "\xbd\xc3\xa4\xa7\xb4\xf2",                                /*  92 */
	"\xc0" "@" "\xa4" "l",                                                /*  93 */
	"\xb0\xc5\xa4" "M",                                                   /*  94 */
	"\xaf\xc8",                                                           /*  95 */
	"\xc3\xf6\xa9\xf3\xc3\xc0\xb3" "N" "\xaa\xba\xa4" "p" "\xa5" "U" ""
	"\xa4" "l",                                                           /*  96 */
	"\xa4" "p" "\xc6" "_" "\xb0\xcd" "1",                                 /*  97 */
	"\xa7\xef\xc5\xdc\xb9" "L" "\xaa\xba\xaf\xc8",                        /*  98 */
	"\xa7\xcb\xc5\xbc\xaa\xba\xaf\xc8",                                   /*  99 */
	"\xb1" "q" "\xa4" "p" "\xbd" "c" "\xa4" "l" ""
	"\xb8\xcc\xae\xb3\xa8\xec\xaa\xba\xaf\xc8",                           /* 100 */
	"\xc3\xf6\xa9\xf3\xac\xd3\xab\xc7\xa6\xe5\xb2\xce\xaa\xba\xa4" "p" ""
	"\xa5" "U" "\xa4" "l",                                                /* 101 */
	"\xc2" "I" "\xbf" "U" "\xaa\xba\xc4\xfa\xc0\xeb",                     /* 102 */
	"\xc4\xfa\xc0\xeb",                                                   /* 103 */
	"\xc6" "_" "\xb0\xcd" " ",                                            /* 104 */
	"\xaf\xf3\xb9\xcf\xa7\xa8",                                           /* 105 */
	"\xaf\xf3\xb9\xcf\xa7\xa8",                                           /* 106 */
	"\xb0\xb2\xaf\xf3\xb9\xcf",                                           /* 107 */
	"\xbc\xd3\xb1\xe8",                                                   /* 108 */
	"\xb3" "Q" "\xaf" "}" "\xc3" "a" "\xaa\xba\xaf\xf3\xb9\xcf",          /* 109 */
	"\xa8\xea\xa4" "l",                                                   /* 110 */
	"\xaa\xf7\xa6\xe2\xaa\xba\xa8\xea\xa4" "l",                           /* 111 */
	"\xac\xf5\xa6\xe2\xaa\xba\xa8\xea\xa4" "l",                           /* 112 */
	"\xac\xb4\xb5\xa7",                                                   /* 113 */
	"\xaf\xc8",                                                           /* 114 */
	"\xc3\xf6\xa9\xf3\xab\xd8\xbf" "v" "\xaa\xba\xa4" "p" "\xa5" "U" ""
	"\xa4" "l",                                                           /* 115 */
	"\xa4" "p" "\xc6" "_" "\xb0\xcd" "2",                                 /* 116 */
	"Archer(useless!)",                                                   /* 117 */
	"\xbc\xd6\xc3\xd0",                                                   /* 118 */
	"\xbc\xb2\xb2" "y" "\xb1\xec",                                        /* 119 */
	"\xb3\\\xa5" "i" "\xb5\xfd\xa9\xfa",                                  /* 120 */
	"\xbe\xb1\xb3\xb9\xaa\xba\xa6" "A" "\xbb" "s" "\xab" "~",             /* 121 */
	"\xa9\xf1\xbe\xb1\xb3\xb9\xaa\xba\xc3" "o" "\xc2" "d",                /* 122 */
	"\xb3" "q" "\xa9\xb9\xaa\xfc\xaa" "i" "\xc3\xb9\xa4" "p" ""
	"\xaa\xf9\xaa\xba\xc6" "_" "\xb0\xcd",                                /* 123 */
	"\xad\xb9\xaa\xab",                                                   /* 124 */
	"\xc3\xf6\xa9\xf3\xa9" "v" "\xb1\xd0\xaa\xba\xa4" "p" "\xa5" "U" ""
	"\xa4" "l",                                                           /* 125 */
	"\xb8" "O" "\xa4\xe5",                                                /* 126 */
	"\xc3\xf6\xa9\xf3\xac" "F" "\xa9\xb2\xaa\xba\xa4" "p" "\xa5" "U" ""
	"\xa4" "l",                                                           /* 127 */
	"\xc3" "Z" "\xa4\xf2\xb5\xa7",                                        /* 128 */
	"\xb3\xc6\xa7\xd1\xbf\xfd",                                           /* 129 */
	"\xb1\xe6\xbb\xb7\xc3\xe8",                                           /* 130 */
	"\xa8" "U" "\xa8\xb9\xb3" "]" "\xad" "p" "\xb9\xcf",                  /* 131 */
	"\xa8" "U" "\xa8\xb9\xb3" "]" "\xad" "p" "\xb9\xcf",                  /* 132 */
	"\xc3\xb7\xaf\xc1",                                                   /* 133 */
	"\xc0" "J" "\xaa\xa9",                                                /* 134 */
	"\xa4" "p" "\xc6" "_" "\xb0\xcd" "3",                                 /* 135 */
	"\xa4" "p" "\xc6" "_" "\xb0\xcd" "4",                                 /* 136 */
	"\xab" "K" "\xba\xe0",                                                /* 137 */
	"\xab\xb0\xb3\xf9\xb3" "]" "\xad" "p" "\xb9\xcf",                     /* 138 */
	"\xab\xb0\xb3\xf9\xb3" "]" "\xad" "p" "\xb9\xcf",                     /* 139 */
	"\xbb\xd5\xbc\xd3\xaa\xba\xc6" "_" "\xb0\xcd",                        /* 140 */
	"\xb4" "J" "\xa8\xa5",                                                /* 141 */
	"\xb0" "g" "\xae" "c" "\xb3" "]" "\xad" "p" "\xb9\xcf",               /* 142 */
	"\xa4" "u" "\xa8\xe3",                                                /* 143 */
	"\xc3\xc4",                                                           /* 144 */
	"\xba\xb6\xb7\xc0\xc4\xfa\xc0\xeb\xaa\xba\xb0\xc5\xa4" "l",           /* 145 */
	"Act %d",                                                             /* 146 */
	"Visit",                                                              /* 147 */
	/* These ones are not in messages but it's simpler */
	nullptr,                                                              /* 148/Epigraph */
	nullptr,                                                              /* 149/Memo */
	nullptr,                                                              /* 150/Bomb */
};

static char const *const versaillesZTpaintings[] = {
	"\"The Animals entering the Ark\"\rGerolamo Bassano",                 /*  0: 41201 */
	"\"The Supper at Emmaus\"\rJacopo Bassano",                           /*  1: 41202 */
	"\"Mary Magdalen at the feet of the Christ",                          /*  2: 41203 */
	"\"Leaving the Ark\"\rGerolamo Bassano",                              /*  3: 41204 */
	"\"Striking the Rock\"\rJacopo Bassano",                              /*  4: 41205 */
	"\"The Battle of Arbelles\"\rJoseph Parrocel",                        /*  5: 41301 */
	"\"Alexander the Great, Vanquisher of Darisu at the battle of "
	"Arbelles\"\rLe Bourguignon",                                         /*  6: 41302 */
	"\"The Combat of Leuze\"\rJoseph Parrocel",                           /*  7: 42401 */
	"\"Saint Cecilia with an angel holding musical notation\"\rIl "
	"Domenichino",                                                        /*  8: 42901 */
	"\"Don Francisco of Moncada\"\rVan Dyck",                             /*  9: 42902 */
	"\"The young John the Baptist\"\rCarracci",                           /* 10: 42903 */
	"\"Saint Matthew\"\rValentin",                                        /* 11: 42904 */
	"\"Render unto Caesar\"\rValentin",                                   /* 12: 42905 */
	"\"Saint Luke\"\rValentin",                                           /* 13: 42906 */
	"\"The Mystical Marriage of Saint Catherine\"\r Alessandro Turchi",   /* 14: 42907 */
	"\"The Meeting of the Drinkers\"\rNicolas Tournier",                  /* 15: 42908 */
	"\"The Fortune-teller\"\rValentin",                                   /* 16: 42909 */
	"\"King David playing the Harp\"\rIl Domenichino",                    /* 17: 42910 */
	"\"Mary Magdalen\"\rIl Domenichino",                                  /* 18: 42911 */
	"\"Self-portrait\"\rVan Dyck",                                        /* 19: 42912 */
	"\"Saint John the Evangelist\"\r Valentin",                           /* 20: 42913 */
	"\"Hagar being helped by an angel\"\rGiovanni Lanfranco",             /* 21: 42914 */
	"\"Saint Mark\"\rValentin",                                           /* 22: 42915 */
	"\"Meleager with the head of the boar of Calydon at his "
	"feet\"\rJacques Rousseau",                                           /* 23: 43090 */ /* BUG: Switched */
	"\"The King in Roman dress\"\rJean Warin",                            /* 24: 43091 */
	"\"Atalanta\"\rJacques Rousseau",                                     /* 25: 43092 */
	"\"Aeneas carrying Anchises\"\rSpada",                                /* 26: 43100 */
	"\"David and Bethsheba\"\rVeronese",                                  /* 27: 43101 */
	"\"The Flight into Egypt\"\rGuido Reni",                              /* 28: 43102 */
	"\"Louis XIV on horseback\"\rPierre Mignard",                         /* 29: 43103 */
	"\"Royal Magnificence and the Progress of the Beaux "
	"Arts\"\rHouasse",                                                    /* 30: 43104 */
	"\"The Sacrifice of Iphigeneia\"\rCharles de La Fosse",               /* 31: 43130 */
	"\"Bust of Louis XIV\"\rsculpted by Bernini",                         /* 32: 43131 */
	"\"Diana  discovers Endymion in the arms of Morpheus\"\rGabriel "
	"Blanchard",                                                          /* 33: 43132 */
	"\"Saint Peter and the Virgin Mary\"\rIl Guercino",                   /* 34: 43140 */
	"\"The Disciples at Emmaus\"\rVeronese",                              /* 35: 43141 */
	"\"The Holy Family\"\rVeronese",                                      /* 36: 43142 */
	"\"Darius' family at the feet of Alexander\"\rCharles LeBrun",        /* 37: 43143 */
	"\"Saint John the Baptist\"\rRaphael",                                /* 38: 43144 */
	"\"Marie de Medici\"\rVan Dyck",                                      /* 39: 43150 */
	"\"Hercules struggling with Achelous\"\rGuido Reni",                  /* 40: 43151 */
	"\"The Centaur Nessus carrying away Deianeira\"\rGuido Reni",         /* 41: 43152 */
	"\"Saint Francis of Assissi comforted after receiving the "
	"stigmata\"\rSeghers",                                                /* 42: 43153 */
	"\"Thomiris plunging Cyrus' head in blood\"\rRubens",                 /* 43: 43154 */
	"\"Hercules Slaying the Hydra\"\rGuido Reni",                         /* 44: 43155 */
	"\"Hercules in flames \"\rGuido Reni",                                /* 45: 43156 */
	"\"Portrait of the Prince Palatine & his brother Prince "
	"Robert\"\rVan Dyck",                                                 /* 46: 43157 */
	"\"Christ being taken down from the cross\"\rCharles Lebrun",         /* 47: 45260 */
};

static const Subtitle versaillesZTsubtitles[] = {
	{
		"a0_vf.hns", {
			{
				75, "\xa4" "Z" "\xba\xb8\xc1\xc9\xab\xb0\xb3\xf9\xa1" "@" "\xb4" "N" ""
				"\xb9\xb3\xa8\xc8\xa8\xaf\xae" "c" "\xb7\xb5\xa1" "@" "\xb1" "N" ""
				"\xa6" "b" "\xae" "L" "\xa6\xdc\xae\xc9\xa1" "@" "\xb2" "l" ""
				"\xa4\xf5\xa8" "R" "\xa4\xd1\xa1" "@" "\xa6" "i" ""
				"\xac\xb0\xa5\xad\xa6" "a"
			},
			{180, ""},
			{
				210, "\xb0\xea\xa4\xfd\xbb" "{" "\xac\xb0\xb3" "o" "\xac" "O" "\xa4" "@" ""
				"\xad\xd3\xba\xc6\xa4" "l" "\xaa\xba\xa4\xdb\xb7" "Q" "\xa1" "A" ""
				"\xa9" "_" "\xab\xe4\xb2\xa7\xb7" "Q" "\xaa\xba\xb5\xb2\xaa" "G" ""
				"\x85" "."
			},
			{
				260, "\xb0\xa3\xab" "D" "\xa8\xb9\xa5\xd0\xa5\xce\xba\xc9\xa5" "L" ""
				"\xa9\xd2\xa6\xb3\xaa\xba\xa4" "O" "\xb6" "q" "\xa1" "@" "\xa7" "_" ""
				"\xab" "h" "\xb3\xa3\xb1" "N" "\xc2" "k" ""
				"\xa9\xf3\xb5\xea\xaa\xc5\xa1" "@" "\xa6" "]" ""
				"\xac\xb0\xae\xc9\xb6\xa1\xa5\xbf\xa6" "b" "\xac" "y" "\xb3" "u" ""
				"\xa1" "@" "\xa7\xda\xa6" "V" "\xa6" "p" "\xac" "P" ""
				"\xaf\xeb\xb0\xaa\xb6\xc6\xaa\xba\xb0\xea\xa4\xfd\xac" "D" ""
				"\xbe\xd4\xa1" "@" "\xa5\xce\xb3\xf2\xc2\xb6\xa6" "b" "\xa5" "L" ""
				"\xb6" "g" "\xb3\xf2\xaa\xba\xb2\xb3\xac" "P" "\xa1" "@" ""
				"\xa7\xe4\xa5" "X" "\xc1\xbc\xc3" "D" "\xa9" "M" "\xad" "p" ""
				"\xb9\xba\xa1" "@" "\xa6\xb3\xbc\xd0\xc3" "D" "\xa1" "A" ""
				"\xa6\xb3\xc0" "Y" "\xba\xfc\xa1" "@" ""
				"\xc1\xd9\xa6\xb3\xa5\xec\xaf\xc1\xaa\xba\xb4" "J" "\xa8\xa5\xa1" ""
				"@" "\xa5\xad\xae\xa7\xa7\xda\xbf" "U" "\xbf" "N" "\xaa\xba\xaf" ""
				"O" "\xa6" "L" "\xa1" "C" "\xa6\xb6\xa4\xf1\xaf" "S" "\xa9" "M" ""
				"\xb3\xb1\xbf\xd1\xa1" "C"
			},
			{
				450, "\xb3" "o" "\xaa" "F" "\xa6\xe8\xc5\xaa\xb0" "_" ""
				"\xa8\xd3\xb9\xb3\xac" "O" "\xba\xc6\xa4" "l" "\xaa\xba\xad" "J" ""
				"\xa8\xa5\xb6\xc3\xbb" "y" "\xa1" "I" "\xb2" "{" "\xa6" "b" ""
				"\xa7\xda\xa5\xb2\xb6\xb7\xaa" "A" "\xa8\xcd\xb0\xea\xa4\xfd\xa1" ""
				"A" "\xa9\xd2\xa5" "H" "\xa7\xda\xb5" "L" "\xaa" "k" "\xb3" "B" ""
				"\xb2" "z" "\xb3" "o" "\xa5\xf3\xa8\xc6\xa1" "C" ""
				"\xa9\xd4\xb6\xa9\xbc" "w" "\xa1" "A" "\xa7\xda\xad" "n" "\xa7" "A" ""
				"\xad" "t" "\xb3" "d" "\xbd\xd5\xac" "d" "\xb3" "o" ""
				"\xa5\xf3\xa8\xc6\xb1\xa1\xa1" "C" "\xa8\xad\xac\xb0\xb9\xec\xae" ""
				"c" "\xaa\xba\xa8\xcd\xaa\xcc\xa1" "A" "\xa7" "A" "\xa5" "i" "\xa5" ""
				"H" "\xa6\xdb\xa5\xd1\xa6" "a" "\xa6" "b" "\xae" "c" ""
				"\xa4\xa4\xa8\xab\xb0\xca\xa1" "C"
			},
			{
				536, "\xa7" "A" "\xa4" "F" "\xb8\xd1\xab\xb0\xb3\xf9\xa1" "A" "\xbb" "{" ""
				"\xc3\xd1\xb8\xcc\xad\xb1\xaa\xba\xa4" "H" "\xa9" "M" ""
				"\xa5\xa6\xaa\xba\xc2\xa7\xb8" "`" "\xa1" "C" "\xa6" "b" ""
				"\xbd\xd5\xac" "d" "\xae\xc9\xa5\xb2\xb6\xb7\xa4\xa3\xc2" "_" ""
				"\xa6" "a" "\xb0\xdd\xb0\xdd\xc3" "D" "\xa1" "A" "\xa8\xc3\xa6" "b" ""
				"\xa7\xe4\xa8\xec\xa5\xf4\xa6\xf3\xa5" "i" ""
				"\xba\xc3\xaa\xba\xa8\xc6\xb1\xa1\xae\xc9\xa6" "V" ""
				"\xa7\xda\xb3\xf8\xa7" "i" "\xa1" "C"
			},
			{
				622, "\xa6\xfd\xac" "O" "\xad" "n" "\xa4" "p" "\xa4\xdf\xa1" "I" ""
				"\xa4\xa3\xad" "n" "\xb4\xb2\xa7" "G" "\xc1\xc1\xa8\xa5\xa1" "A" ""
				"\xbb\xa1\xa6\xb3\xad\xd3\xba\xc6\xa4" "l" "\xa6" "b" ""
				"\xab\xc2\xaf\xd9\xb0\xea\xa4\xfd\xa1" "I" "\xa5" "B" "\xa7" "A" ""
				"\xb0\xca\xa7" "@" "\xad" "n" "\xa7\xd6\xa1" "A" "\xa6" "]" ""
				"\xac\xb0\xa7" "A" "\xa5" "u" "\xa6\xb3\xa4" "@" ""
				"\xa4\xd1\xaa\xba\xae\xc9\xb6\xa1" "...."
			},
			{690, "\xa4" "K" "\xc2" "I" "\xa5" "b" "\xa1" "C"},
		}
	}, /* a0_vf.hns */
	{
		"a1_vf.hns", {
			{
				150, "\xa4" "Z" "\xba\xb8\xc1\xc9\xa1\xd0\xae" "c" ""
				"\xa7\xca\xba\xc3\xb6\xb3"
			},
			{225, ""},
			{
				358, "\xb0\xa1\xa4" "U" "\xa1" "A" "\xae\xc9\xb6\xa1\xa8\xec\xa4" "F" ""
				"\xa1" "C"
			},
			{
				388, "\xb7\xed\xb0\xea\xa4\xfd\xbf\xf4\xa8\xd3\xae\xc9\xa1" "A" ""
				"\xad\xba\xae" "u" "\xc2\xe5\xa5\xcd\xb9" "F" "\xa5" "i" ""
				"\xa9\xf8\xc0\xcb\xac" "d" "\xa5" "L" "\xaa\xba\xb0\xb7\xb1" "d" ""
				"\xa1" "C"
			},
			{
				440, "\xb0\xa1\xa4" "U" "\xba\xce\xb1" "o" "\xa5" "i" "\xa6" "n" "\xa1" ""
				"H" "\xb7" "|" "\xc4\xb1\xb1" "o" "\xa4\xd3\xbc\xf6\xb6\xdc\xa1" ""
				"H" "\xc1\xd9\xac" "O" "\xad" "n" "\xa6" "A" "\xab\xf6\xbc\xaf\xa4" ""
				"@" "\xa4" "U" "\xa1" "H"
			},
			{500, "\xc1\xc2\xc1\xc2\xa1" "A" "\xb9" "F" "\xa5" "i" "\xa9\xf8\xa1" "C"},
			{
				510, "\xb9" "L" "\xa5" "h" "\xa8\xe2\xad\xd3\xa4\xeb\xa5" "H" ""
				"\xa8\xd3\xa1" "A" "\xb0\xa1\xa4" "U" "\xaa\xba\xb0\xb7\xb1" "d" ""
				"\xb1\xa1\xa7\xce\xb3\xa3\xab\xdc\xa8" "}" "\xa6" "n" "\xa1" "C"
			},
			{550, ""},
			{
				580, "\xb7\xed\xa8\xb9\xa5\xd0\xb2" "M" "\xac" "~" ""
				"\xb0\xea\xa4\xfd\xaa\xba\xa4\xe2\xae\xc9\xa1" "A" ""
				"\xac\xd3\xb1\xda\xaa\xba\xa4" "H" "\xa4" "@" "\xa6" "p" ""
				"\xa9\xb9\xb1" "`" "\xaa\xba\xa6" "b" "\xa8" "C" ""
				"\xa4\xd1\xa6\xad\xb1\xe1\xa9\xe8\xb9" "F" "\xa6\xb9\xa6" "a" ""
				"\xa1" "C" "\xac\xd3\xa4\xd3\xa4" "l" "\xa1" "A" ""
				"\xb0\xea\xa4\xfd\xaa\xba\xa5" "S" "\xa7\xcc\xa1" "A" "\xa5" "H" ""
				"\xa4\xce\xbd" "q" "\xa6" "]" "\xa4\xbd\xc0\xef\xa1" "A" ""
				"\xb3\xa3\xab" "e" "\xa8\xd3\xa5\xd8\xb8" "@" ""
				"\xb0\xea\xa4\xfd\xb0" "_" "\xa8\xad\xa1" "C"
			},
			{713, ""},
			{
				750, "\xb6\xe2\xa1" "I" "\xab\xc4\xa4" "l" "\xa1" "A" "\xa7" "A" ""
				"\xa4\xb5\xa4\xd1\xa6\xad\xa4" "W" "\xad" "n" ""
				"\xa4\xb0\xbb\xf2\xae\xc9\xad\xd4\xa5" "X" "\xb5" "o" "\xa5" "h" ""
				"\xc2" "y" "\xaf" "T" "\xa9" "O" "\xa1" "H"
			},
			{795, ""},
		}
	}, /* a1_vf.hns */
	{
		"a2_vf.hns", {
			{
				60, "\xa4\xa3\xa1" "A" "\xac\xd3\xa5" "S" "\xa1" "C" "\xa7" "A" ""
				"\xaa\xba\xb2\xca\xb3\xa5\xa4\xe8\xaa" "k" ""
				"\xa4\xa3\xaf\xe0\xab\xec\xb4" "_" "\xb0\xea\xa4\xba\xaa\xba\xa9" ""
				"M" "\xa5\xad\xa1" "C"
			},
			{
				74, "\xb7\xed\xb5" "M" "\xa1" "A" "\xb0\xea\xa4\xfd\xa4" "w" "\xb8" "g" ""
				"\xb4\xa3\xa8\xec\xad" "n" ""
				"\xac\xa3\xbb\xba\xa7\xf9\xa9\xd4\xb4\xb5\xa5\xfd\xa5\xcd\xab" "e" ""
				"\xa5" "h" "\xa8\xa3\xa9\xd4\xa5\xb1\xb4\xb5\xa4\xbd\xc0\xef"
			},
			{
				130, "\xa7\xda\xb8\xf2\xa7" "A" "\xab" "O" "\xb5\xfd\xa1" "A" "\xa5" "L" ""
				"\xb5" "L" "\xaa" "k" ""
				"\xa8\xa3\xa8\xec\xa9\xd4\xa5\xb1\xb4\xb5\xa4\xbd\xc0\xef\xa1" "C"
			},
			{160, ""},
			{
				200, "\xb0\xea\xa4\xfd\xb0" "_" "\xa7\xc9\xbf" "d" "\xac" "~" ""
				"\xa4\xa7\xab\xe1\xa4\xce\xb4\xc2\xa6\xda\xb6" "i" ""
				"\xbf\xd6\xa4\xa7\xab" "e" "\xa1" "A" "\xb2" "z" "\xbe" "v" "\xae" ""
				"v" "\xb7" "|" "\xa5\xfd\xac\xb0\xb0\xea\xa4\xfd\xae\xde\xb2" "z" ""
				"\xa1" "A" "\xc0\xb9\xa4" "W" "\xb5" "u" "\xb0\xb2\xbe" "v" "\xa1" ""
				"C"
			},
			{307, ""},
			{
				360, "\xb5" "M" "\xab\xe1\xb0\xea\xa4\xfd\xab" "K" ""
				"\xb7\xc7\xb3\xc6\xab" "e" "\xa9\xb9\xa4" "j" "\xc6" "U" "\xa1" "A" ""
				"\xb6" "i" "\xa6\xe6\xb5\xdb\xb3" "T" "\xbb\xf6\xa6\xa1\xa1" "C"
			},
			{406, ""},
			{
				440, "\xa9" "^" "\xad\xe3\xaa\xba\xb4\xc2\xa6\xda\xad\xcc\xa5" "i" ""
				"\xa5" "H" "\xb3\xad\xa6\xf1\xb0\xea\xa4\xfd\xa4" "@" "\xa6" "P" ""
				"\xa8\xc9\xa5\xce\xa4" "p" "\xc2" "I" "\xa4\xdf\xa1" "C"
			},
			{
				520, "\xa8" "C" "\xad\xd3\xa4" "H" ""
				"\xb3\xa3\xa7\xc6\xb1\xe6\xb0\xea\xa4\xfd\xaf\xe0\xac\xdd\xa5" "L" ""
				"\xa4" "@" "\xb2\xb4\xa1" "A" "\xb8\xdf\xb0\xdd\xa5" "L" ""
				"\xaa\xba\xb0\xb7\xb1" "d" ""
				"\xb1\xa1\xa7\xce\xa8\xc3\xb2\xe2\xc5\xa5\xa9\xce\xb4\xb2\xbc\xbd\xb7\xed\xa4\xd1\xaa\xba\xc1\xc1\xa8\xa5\xa1" ""
				"C"
			},
			{
				600, "\xb0\xea\xa4\xfd\xa6" "b" "\xa4\xbd\xb2\xb3\xaa\xba\xaa" "`" ""
				"\xa5\xd8\xa4\xa7\xa4" "U" "\xa1" "A" "\xa7\xea\xba" "t" ""
				"\xb5\xdb\xa5" "L" "\xa9\xb9\xb1" "`" ""
				"\xaa\xba\xa8\xa4\xa6\xe2\xa1" "C" "\xa6\xb3\xa4" "H" ""
				"\xc0\xb0\xa5" "L" "\xab" "c" "\xc5\xbd\xa1" "A" "\xac" "~" "\xc1" ""
				"y" "\xa1" "A" "\xb5" "M" "\xab\xe1\xb5\xdb\xb8\xcb\xa1" "C"
			},
			{679, ""},
			{
				815, "\xb5\xdb\xa6" "n" "\xb8\xcb\xab\xe1\xa1" "A" ""
				"\xb0\xea\xa4\xfd\xab" "K" "\xa6" "^" "\xa8\xec\xa6\xdb\xa4" "v" ""
				"\xaa\xba\xb9\xec\xab\xc7\xa1" "A" "\xb0\xb5\xa4" "@" ""
				"\xad\xd3\xc2\xb2\xb5" "u" "\xaa\xba\xa6\xad\xc3\xab\xa1" "C"
			},
			{860, ""},
			{
				948, "\xc3\xab\xa7" "i" "\xa4\xa7\xab\xe1\xa1" "A" ""
				"\xb0\xea\xa4\xfd\xb7" "|" "\xc0" "H" "\xb5\xdb\xa9" "x" ""
				"\xad\xfb\xa1" "A" "\xab" "e" "\xa9\xb9\xc4\xb3\xb7" "|" "\xc6" "U" ""
				"\xa1" "A" "\xa6" "b" "\xa8\xba\xb8\xcc\xb0" "Q" ""
				"\xbd\xd7\xb0\xea\xae" "a" "\xa8\xc6\xb0\xc8\xa1" "C"
			},
			{1027, ""},
		}
	}, /* a2_vf.hns */
	{
		"a3_vf.hns", {
			{
				30, "\xb5\xdb\xb3" "T" "\xbb\xf6\xa6\xa1\xb5\xb2\xa7\xf4\xab\xe1\xa1" ""
				"A" "\xb0\xea\xa4\xfd\xab" "K" "\xab" "e" "\xa9\xb9\xc4\xb3\xb7" ""
				"|" "\xc6" "U" "\xa1" "C" "\xbb" "P" "\xa9" "x" ""
				"\xad\xfb\xad\xcc\xb3" "B" "\xb2" "z" "\xb0\xea\xae" "a" ""
				"\xaa\xba\xa8\xc6\xb0\xc8\xa1" "C"
			},
			{
				110, "\xac\xb0\xbd" "T" "\xab" "O" "\xbe\xf7\xb1" "K" "\xa1" "A" ""
				"\xa9\xd2\xa6\xb3\xaa\xba\xaa\xf9\xb1" "N" "\xb3" "Q" ""
				"\xa4\xcf\xc2\xea\xa1" "C" "\xa9" "x" "\xad\xfb\xbf" "c" ""
				"\xa5\xcb\xab" "K" "\xb6" "}" "\xa9" "l" ""
				"\xb7\xed\xa4\xd1\xaa\xba\xad\xab\xad" "n" "\xc4\xb3\xc3" "D" ""
				"\xa1" "C"
			},
			{211, ""},
			{280, "\xa6" "U" "\xa6\xec\xa1" "A" "\xbd\xd0\xa7\xa4\xa1" "I"},
			{
				290, "\xb0\xea\xa4\xfd\xb7" "|" "\xb6\xc9\xc5\xa5\xa8\xd3\xa6\xdb\xa6" ""
				"U" "\xa4\xe8\xa4\xa3\xa6" "P" "\xaa\xba\xb7" "N" "\xa8\xa3\xa1" ""
				"A" "\xb5" "M" "\xab\xe1\xb0\xb5\xa8" "M" "\xa9" "w" "\xa1" "C" ""
				"\xa5" "L" "\xb3" "q" "\xb1" "`" "\xb7" "|" "\xb1\xb5\xa8\xfc\xa4" ""
				"j" "\xa6" "h" "\xbc\xc6\xa4" "H" "\xaa\xba\xb7" "N" "\xa8\xa3\xa1" ""
				"C"
			},
			{
				360, "\xa6" "b" "\xb3" "o" "\xa6" "P" "\xae\xc9\xa1" "A" ""
				"\xb2\xb3\xb4\xc2\xa6\xda\xa6" "b" "\xa4" "j" "\xc6" "U" ""
				"\xa4\xa4\xb5\xa5\xad\xd4\xb0\xea\xa4\xfd\xb2" "{" "\xa8\xad\xa1" ""
				"C"
			},
			{411, ""},
			{
				440, "\xa6" "U" "\xa6\xec\xa1" "A" "\xb0\xa1\xa4" "U" "\xbe" "r" ""
				"\xa8\xec\xa1" "C"
			},
			{450, ""},
			{
				550, "\xb2" "{" "\xa6" "b" "\xac" "O" ""
				"\xa4\xa4\xa4\xc8\xae\xc9\xa4\xc0\xa1" "C" "\xb0\xea\xa4\xfd\xa5" ""
				"h" "\xb0\xd1\xa5" "[" "\xc0\xb1\xbc\xbb\xa1" "C" ""
				"\xb4\xc2\xa6\xda\xa6" "b" "\xa5" "L" ""
				"\xaa\xba\xab\xe1\xad\xb1\xb1\xc6\xa6\xa8\xa4" "@" "\xa6" "C" ""
				"\xa1" "C" "\xb0\xea\xa4\xfd\xa6" "b" "\xa4\xfd\xa4" "l" "\xa9" "M" ""
				"\xa4\xbd\xa5" "D" "\xaa\xba\xb3\xad\xa6" "P" "\xa4" "U" "\xa1" "A" ""
				"\xac\xef\xb9" "L" "\xa4" "j" "\xc6" "U" "\xa1" "A" "\xab" "e" ""
				"\xa9\xb9\xb1\xd0\xb0\xf3\xa1" "C"
			},
			{687, ""},
			{
				728, "\xa6" "p" "\xaa" "G" "\xb0\xa1\xa4" "U" "\xa4\xaf\xb7" "O" "\xa1" ""
				"A" "\xc4" "@" "\xb7" "N" "\xac\xdd\xac\xdd\xb3" "o" ""
				"\xa5\xf7\xbd\xd0\xc4" "@" "\xae\xd1\xa1" "A" "\xbd\xd0\xa8" "D" ""
				"\xb0\xa1\xa4" "U" "\xac\xb0\xa7\xda\xa8\xba\xa6" "b" ""
				"\xa4\xa3\xa4\xbd\xaa\xba\xbc" "f" "\xa7" "P" "\xa4\xa4\xa1" "A" ""
				"\xa6\xa8\xac\xb0\xc4\xeb\xac\xb9\xaa\xcc\xaa\xba\xa8\xe0\xa4" "l" ""
				"\xa6\xf9\xad\xde\xa1" "C"
			},
			{
				805, "\xa7\xda\xad\xcc\xb7" "|" "\xaa" "`" "\xb7" "N" "\xb3" "o" ""
				"\xa5\xf3\xa8\xc6\xa1" "A" "\xa5\xfd\xa5\xcd\xa1" "A" ""
				"\xa7\xda\xad\xcc\xb7" "|" "\xaa" "`" "\xb7" "N"
			},
			{840, ""},
			{
				1060, "\xb0\xea\xa4\xfd\xa6" "b" "\xac\xd3\xae" "a" "\xaa\xf8\xb4" "Y" ""
				"\xa4\xa4\xaf\xb8\xa9" "w" "\xa1" "A" "\xa4\xfd\xa4" "l" "\xa9" "M" ""
				"\xa4\xbd\xa5" "D" "\xa8\xcc\xb6\xa5\xbc" "h" ""
				"\xa5\xfd\xab\xe1\xaf\xb8\xa6" "b" "\xa5" "L" ""
				"\xaa\xba\xab\xe1\xad\xb1\xa1" "C" "\xb0\xea\xa4\xfd\xc4" "Y" ""
				"\xb8" "T" "\xa5\xe6\xbd\xcd\xa8\xc3\xad" "n" "\xa8" "D" "\xa5" "L" ""
				"\xad\xcc\xb0" "@" "\xb7" "q" "\xa1" "C"
			},
			{1220, ""},
		}
	}, /* a3_vf.hns */
	{
		"a4_vf.hns", {
			{
				90, "\xa6" "b" "\xc0\xb1\xbc\xbb\xae\xc9\xa1" "A" "\xac\xd3\xae" "a" ""
				"\xbc" "p" "\xa9\xd0\xaa\xba\xa9" "x" "\xad\xfb\xa1" "A" "\xaf" "N" ""
				"\xa6\xd7\xa6" "K" "\xa1" "A" "\xc4\xd1\xa5" "]" "\xae" "v" ""
				"\xb3\xc5\xa9" "M" "\xa5" "L" "\xad\xcc\xaa\xba\xa7" "U" ""
				"\xa4\xe2\xa5\xbf\xa6\xa3\xb5\xdb\xb7\xc7\xb3\xc6\xc0\\\xc2" "I" ""
				"\xa1" "C"
			},
			{
				200, "\xb0\xea\xa4\xfd\xaa\xba\xad" "G" "\xa4" "f" "\xab\xdc\xa4" "j" ""
				"\xa1" "A" "\xa6\xd3\xb0\xea\xa4\xfd\xa8" "S" "\xa6\xb3\xa6" "Y" ""
				"\xa7\xb9\xaa\xba\xad\xb9\xaa\xab\xab" "h" "\xb7" "|" "\xaf" "d" ""
				"\xb5\xb9\xa5" "L" "\xa4" "H" "\xad\xb9\xa5\xce\xa1" "A" ""
				"\xa6\xd3\xa4" "@" "\xaf\xeb\xa4" "H" "\xa4" "]" "\xa4\xa3\xb7" "|" ""
				"\xac\xdd\xa4\xa3\xb0" "_" "\xa6" "V" "\xbc" "p" ""
				"\xa9\xd0\xc1\xca\xb6" "R" "\xb3\xd1\xbe" "l" "\xb5\xe6\xc0" "a" ""
				"\xaa\xba\xa6\xe6\xac\xb0\xa1" "C"
			},
			{300, ""},
			{
				320, "\xa4\xbd\xb6" "}" "\xa5\xce\xc0\\\xac" "O" "\xa5\xf3\xa4" "j" ""
				"\xa8\xc6\xa1" "A" ""
				"\xa9\xd2\xa6\xb3\xaa\xba\xb4\xc2\xa6\xda\xb3\xa3\xb7" "|" ""
				"\xbb\xb0\xa5" "h" "\xb0\xd1\xa5" "["
			},
			{
				366, "\xa6" "U" "\xa6\xec\xa5\xfd\xa5\xcd\xa1" "A" "\xb0\xa1\xa4" "U" ""
				"\xa5\xce\xc0\\\xa1" "C"
			},
			{393, ""},
			{
				610, "\xa5" "u" "\xa6\xb3\xac\xd3\xab\xc7\xaa\xba\xa4" "H" "\xa5" "i" ""
				"\xa5" "H" "\xa9" "M" "\xb0\xea\xa4\xfd\xa6" "P" "\xa7\xa4\xa1" "A" ""
				"\xa6\xd3\xa5" "B" "\xa5" "u" "\xa6\xb3\xa4" "k" ""
				"\xa4\xbd\xc0\xef\xa5" "i" "\xa5" "H" "\xa7\xa4\xa6" "b" "\xa5" "L" ""
				"\xaa\xba\xad\xb1\xab" "e" "\xa1" "C"
			},
			{685, ""},
			{
				705, "\xb6\xe2\xa1" "A" "\xab\xc4\xa4" "l" "\xa1" "A" "\xa7" "A" ""
				"\xb7\xc7\xb3\xc6\xac\xb0\xa7\xda\xad\xcc\xaa\xed\xba" "t" "\xc3" ""
				"M" "\xb3" "N" "\xb6\xdc\xa1" "H"
			},
			{747, ""},
			{
				760, "\xa6" "b" "\xa5\xce\xc0\\\xae\xc9\xa1" "A" "\xa6\xb3\xa4" "T" ""
				"\xb9" "D" "\xb5" "{" "\xa7\xc7\xa1" "A" "\xa8" "C" "\xa4" "@" ""
				"\xb9" "D" "\xb3\xa3\xa6\xb3\xa4\xbb\xb9" "D" "\xa4\xa3\xa6" "P" ""
				"\xaa\xba\xb5\xe6\xc0" "a" "\xa1" "G" "\xb4\xf6\xa9" "M" "\xb6" "}" ""
				"\xad" "G" "\xb5\xe6\xa1" "A" "\xaf" "N" "\xa6\xd7\xa9" "M" "\xc2" ""
				"I" "\xa4\xdf"
			},
			{847, ""},
		}
	}, /* a4_vf.hns */
	{
		"a5_vf.hns", {
			{13, "\xc0\\\xab\xe1\xa6\xb3\xa4\xf4\xaa" "G...." "\xa1" "A....."},
			{36, ""},
			{
				90, "\xa4\xa7\xab\xe1\xb0\xea\xa4\xfd\xaa\xe1\xbc\xc6\xa4" "p" ""
				"\xae\xc9\xaa\xba\xae\xc9\xb6\xa1\xa8\xd3\xb3" "B" "\xb2" "z" ""
				"\xb0\xea\xae" "a" "\xa8\xc6\xb0\xc8\xa1" "C" "\xa6" "b" "\xb3" "o" ""
				"\xac" "q" "\xae\xc9\xb6\xa1\xa4\xba\xa1" "A" "\xa5" "L" ""
				"\xb3\xdf\xc5" "w" "\xa6" "b" "\xb0\xd2\xaf" "S" ""
				"\xbb\xfa\xa4\xd2\xa4" "H" "\xaa\xba\xa9\xd0\xa4" "l" ""
				"\xb8\xcc\xbf\xec\xa4\xbd\xa1" "A" ""
				"\xa6\xb3\xa8\xc7\xae\xc9\xad\xd4\xb7" "|" "\xa6\xb3\xa9" "x" ""
				"\xad\xfb\xb3\xad\xa6\xf1\xa1" "C"
			},
			{
				200, "\xb3" "o" "\xad\xd3\xbb" "P" "\xa5" "L" "\xaf\xb5\xb1" "K" ""
				"\xa6\xa8\xb1" "B" "\xa4" "k" "\xa4" "H" "\xaa\xba\xa9\xd0\xa4" "l" ""
				"\xb8\xcc\xa1" "A" "\xa5" "L" "\xb7" "P" ""
				"\xa8\xfc\xa8\xec\xa5\xad\xc0" "R" "\xa4\xce\xb7\xc5\xb7" "x" ""
				"\xa1" "A" "\xb3" "o" "\xa5\xbf\xac" "O" "\xa5" "L" "\xa4" "u" ""
				"\xa7" "@" "\xae\xc9\xa9\xd2\xbb\xdd\xad" "n" "\xaa\xba\xa1" "C"
			},
			{296, ""},
			{
				310, "\xb0\xea\xa4\xfd\xa5\xce\xa4" "T" "\xad\xd3\xa4" "p" ""
				"\xae\xc9\xaa\xba\xae\xc9\xb6\xa1\xb8\xd4\xb2\xd3\xac" "d" ""
				"\xac\xdd\xa6" "U" "\xb6\xb5\xa8\xc6\xb0\xc8"
			},
			{345, ""},
			{
				360, "\xb9\xef\xb4\xc2\xa6\xda\xa6\xd3\xa8\xa5\xa1" "A" "\xb3" "o" ""
				"\xac" "O" "\xa4" "@" "\xa4\xd1\xb7\xed\xa4\xa4\xa5" "L" ""
				"\xad\xcc\xa4\xa3\xb6\xb7\xb9\xb3\xb0\xea\xa4\xfd\xaa\xba\xbc" "v" ""
				"\xa4" "l" "\xa4" "@" "\xbc\xcb\xa1" "A" "\xc0" "H" ""
				"\xae\xc9\xb8\xf2\xb5\xdb\xb0\xea\xa4\xfd\xa8" "C" "\xa4" "@" ""
				"\xa8" "B" "\xa6\xe6\xb0\xca\xaa\xba\xae\xc9\xa8\xe8\xa1" "C"
			},
			{
				460, "\xa6\xb3\xa8\xc7\xa4" "H" "\xaa\xb1\xb5" "P" "\xa1" "C" "\xb5" "P" ""
				"\xae\xe0\xa5" "i" "\xa5" "H" "\xb4\xa3\xa8\xd1\xa4" "@" ""
				"\xa8\xc7\xa6\xac\xa4" "J" "\xa1" "A" "\xa6\xb3\xa8\xc7\xa4" "H" ""
				"\xb7" "|" "\xb2" "@" "\xa4\xa3\xb5" "S" "\xbf\xdd\xa6" "a" "\xab" ""
				"_" "\xb5\xdb\xa5\xa2\xc3" "d" "\xaa\xba\xa6" "M" "\xc0" "I" "\xa5" ""
				"X" "\xa6\xd1\xa4" "d" "\xa1" "C"
			},
			{560, ""},
			{
				665, "\xa8\xe4\xa5" "L" "\xa4" "H" "\xab" "h" "\xb1" "q" "\xa8\xc6\xa4" ""
				"@" "\xa8\xc7\xbb\xdd\xad" "n" ""
				"\xa7\xde\xa5\xa9\xaa\xba\xac\xa1\xb0\xca\xa1" "A" "\xa6" "p" ""
				"\xbc\xb2\xb2" "y" "\xb5\xa5\xa1" "C" "\xa6\xfd\xac" "O" "\xa4" "~" ""
				"\xb5\xd8\xac" "v" "\xb7\xb8\xaa\xba\xb9\xef\xb8\xdc\xac" "O" ""
				"\xa4" "Z" "\xba\xb8\xb6\xeb\xae" "c" ""
				"\xa4\xa4\xb3\xcc\xa8\xfc\xb3\xdf\xb7" "R" "\xaa\xba\xae" "T" ""
				"\xbc\xd6"
			},
			{766, ""},
		}
	}, /* a5_vf.hns */
	{
		"a6_vf.hns", {
			{
				45, "\xb1\xb5\xaa\xf1\xa4" "U" "\xa4\xc8\xa4\xad\xc2" "I" ""
				"\xc4\xc1\xaa\xba\xae\xc9\xad\xd4\xa1" "A" "\xb5\xb2\xa7\xf4\xa4" ""
				"@" "\xa4\xd1\xaa\xba\xa4" "u" "\xa7" "@" "\xa1" "A" ""
				"\xb0\xea\xa4\xfd\xb7" "|" ""
				"\xa8\xec\xaa\xe1\xb6\xe9\xb8\xcc\xb4\xb2\xa8" "B" ""
				"\xa8\xc3\xb0\xd1\xc6" "[" "\xa6" "U" "\xad\xd3\xae" "x" "\xb0" "| "
				""
			},
			{
				90, "..." "\xa5" "L" "\xa5" "[" "\xa4" "J" "\xa6" "b" "\xa4" "j" "\xb2" ""
				"z" "\xa5\xdb\xae" "x" "\xb0" "|" "\xb5\xa5\xad\xd4\xa5" "L" ""
				"\xaa\xba\xb4\xc2\xa6\xda\xa1" "C"
			},
			{160, ""},
			{
				170, "\xa5" "u" "\xa6\xb3\xc0\xf2\xb1" "o" "\xaf" "S" ""
				"\xb3\\\xaa\xba\xa4" "H" "\xa4" "~" "\xaf\xe0\xbb" "P" ""
				"\xb0\xea\xa4\xfd\xa4" "@" "\xa6" "P" "\xb4\xb2\xa8" "B" "\xa1" "C"
			},
			{208, ""},
			{
				295, "\xa4" "H" "\xb8" "s" "\xac\xef\xb9" "L" "\xa7" "C" "\xaa\xf8\xb4" ""
				"Y" "\xa1" "A" "\xa8\xab\xa6" "V" "\xae" "x" "\xb0" "|" "\xa1" "A" ""
				"\xb5" "M" "\xab\xe1\xab" "e" ""
				"\xa9\xb9\xa4\xf4\xa6\xc0\xaf\xf3\xa9" "W" "\xa1" "A" "\xa6" "b" ""
				"\xa8\xba\xb8\xcc\xb0\xea\xa4\xfd\xb3\xdf\xc5" "w" "\xaa\xe1\xa4" ""
				"@" "\xa8\xc7\xae\xc9\xb6\xa1\xaa" "Y" ""
				"\xbd\xe0\xab\xb0\xb3\xf9\xaa\xba\xb4\xba\xc6" "[" "\xa1" "C"
			},
			{
				430, "\xb1" "q" "\xbe\xef\xb6\xe9\xa8\xec\xac" "W" "\xb4" "Y" "\xa1" "A" ""
				"\xb0\xea\xa4\xfd\xa5" "J" "\xb2\xd3\xb9\xee\xac\xdd\xa8" "C" ""
				"\xb6\xb5\xa4" "u" "\xa7" "@" "\xb6" "i" ""
				"\xa6\xe6\xa4\xa4\xaa\xba\xb1\xa1\xa7\xce\xa1" "C"
			},
			{492, ""},
			{
				945, "\xb0\xd1\xb3" "X" "\xa6" "b" "\xa4" "p" ""
				"\xab\xc2\xa5\xa7\xb4\xb5\xb5\xb2\xa7\xf4\xa1" "C" ""
				"\xb0\xea\xa4\xfd\xb1" "q" "\xb9" "D" "\xa9" "_" ""
				"\xab\xb0\xa7\xe4\xa8\xd3\xa5\xad\xa9\xb3\xb2\xee\xaa\xba\xb2\xee\xa4\xd2\xa1" ""
				"A" "\xa6" "b" "\xa4\xd1\xae\xf0\xa8" "}" "\xa6" "n" ""
				"\xaa\xba\xae\xc9\xad\xd4\xa1" "A" "\xa5" "L" "\xad\xcc\xb4" "N" ""
				"\xb7" "|" "\xb1" "a" "\xb5\xdb\xae" "c" "\xa7\xca\xaa\xba\xa4" "H" ""
				"\xa6" "b" "\xb9" "B" "\xaa" "e" "\xa4" "W" "\xaf\xe8\xa6\xe6\xa1" ""
				"C"
			},
		}
	}, /* a6_vf.hns */
	{
		"a7_vf.hns", {
			{
				40, "\xa6" "b" "\xae" "L" "\xa4\xd1\xb1\xdf\xa4" "W" "\xa1" "A" ""
				"\xb2\xb3\xa4" "H" "\xac\xd2\xaa\xbe\xad" "n" "\xa6" "b" ""
				"\xaa\xe1\xb6\xe9\xaa\xba\xa4" "p" "\xbe\xf0\xc2" "O" ""
				"\xb6\xa1\xa5\xce\xab" "K" "\xc0\\\xa8\xd3\xa8\xfa\xa5" "N" ""
				"\xb1\xdf\xc0\\\xa1" "C"
			},
			{107, ""},
			{
				198, "\xb5" "M" "\xab\xe1\xa1" "A" "\xb7\xed\xa9" "]" ""
				"\xb1\xdf\xa8\xd3\xc1" "{" "\xae\xc9\xa1" "A" ""
				"\xa9\xd2\xa6\xb3\xa4" "H" "\xb3\xa3\xa6" "^" ""
				"\xab\xb0\xb3\xf9\xb0\xd1\xa5" "[" "\xb0\xea\xa4\xfd\xaa\xba\xb4" ""
				"N" "\xb9\xec\xbb\xf6\xa6\xa1\xa1" "C"
			},
			{245, ""},
			{
				355, "\xa4" "k" "\xa4" "h" "\xa4\xa3\xaf\xe0\xb0\xd1\xa5" "[" ""
				"\xb0\xea\xa4\xfd\xaa\xba\xb4" "N" "\xb9\xec\xbb\xf6\xa6\xa1\xa1" ""
				"A" "\xa4" "]" "\xa4\xa3\xb7" "|" "\xc0\xf2\xc1\xdc\xb0\xd1\xa5" ""
				"[" "\xb0" "_" "\xa7\xc9\xa9" "M" "\xb5\xdb\xb3" "T" "\xc2\xa7\xa1" ""
				"C" "\xb0\xea\xa4\xfd\xa5" "l" "\xb6\xb0\xa6" "o" "\xad\xcc\xab" ""
				"e" "\xa9\xb9\xa4" "j" "\xc6" "U" "\xa1" "A" "\xa6" "V" "\xa6" "o" ""
				"\xad\xcc\xb9" "D" "\xb1\xdf\xa6" "w" "\xa1" "C"
			},
			{442, ""},
			{
				454, "\xa4" "k" "\xa4" "h" "\xad\xcc\xa1" "A" "\xa7\xc6\xb1\xe6\xa6" "b" ""
				"\xb3" "o" "\xa9" "]" "\xb1\xdf\xae\xc9\xa4\xc0\xa1" "A" "\xac" "O" ""
				"\xbe" "A" "\xa6" "X" "\xa7" "A" ""
				"\xad\xcc\xa5\xf0\xae\xa7\xaa\xba\xae\xc9\xb6\xa1\xa1" "C"
			},
			{512, ""},
			{
				647, "\xb5" "M" "\xab\xe1\xb4" "N" "\xb9\xec\xbb\xf6\xa6\xa1\xab" "K" ""
				"\xa5\xbf\xa6\xa1\xb6" "}" "\xa9" "l" "\xa1" "C" "\xa6\xad\xa4" "W" ""
				"\xb0\xd1\xa5" "[" "\xb5\xdb\xb3" "T" ""
				"\xc2\xa7\xaa\xba\xb4\xc2\xa6\xda\xa5\xb2\xb5" "M" "\xb7" "|" ""
				"\xa8\xd3\xb0\xd1\xa5" "[" "\xb4" "N" ""
				"\xb9\xec\xbb\xf6\xa6\xa1\xa1" "C"
			},
			{722, ""},
			{
				733, "\xad" "n" "\xbd\xd0\xbd\xd6\xb0\xf5\xc0\xeb\xa9" "O" "\xa1" "A" ""
				"\xb0\xa1\xa4" "U" "\xa1" "H"
			},
			{763, ""},
			{
				925, "\xb0\xea\xa4\xfd\xa8" "C" "\xa4\xd1\xaa\xba\xa6\xe6\xb0\xca\xac" ""
				"O" "\xa4" "@" "\xba\xd8\xac\xb0\xb0\xaa\xa9" "|" ""
				"\xaa\xba\xaa\xed\xb2" "{" "\xa1" "C" "\xa6" "b" "\xa5" "L" "\xa4" ""
				"W" "\xa7\xc9\xae\xc9\xac\xb0\xa5" "L" ""
				"\xae\xb3\xc4\xfa\xc0\xeb\xaa\xba\xc5" "v" "\xa7" "Q" "\xa1" "A" ""
				"\xb7" "|" "\xbd\xe7\xb5\xb9\xa6" "a" "\xa6\xec\xaf" "S" ""
				"\xae\xed\xaa\xba\xa4" "H" "\xa1" "A" "\xb3" "q" "\xb1" "`" "\xac" ""
				"O" "\xa8\xd3\xb3" "X" "\xaa\xba\xa5" "~" "\xbb\xab"
			},
			{1048, ""},
			{
				1060, "\xa6" "]" "\xa6\xb9\xa5" "L" "\xb4" "N" "\xc0\xf2\xb1" "o" ""
				"\xb3\xad\xa6\xf1\xb0\xea\xa4\xfd\xab" "e" "\xa9\xb9\xb9\xec\xae" ""
				"c" "\xaa\xba\xc5" "v" "\xa7" "Q" "\xa1" "C"
			},
			{1100, ""},
			{
				1115, "\xa6\xfd\xac" "O" "\xa1" "A" "\xb4" "N" "\xa6" "b" "\xb3" "o" ""
				"\xa4\xd1\xb1\xdf\xa4" "W...."
			},
			{1135, ""},
			{
				1188, "\xa4\xf5\xa1" "I" "\xa4\xf5\xa1" "I" "\xa9\xd4\xb6\xa9\xbc" "w" ""
				"\xa1" "A" "\xb9" "L" "\xa8\xd3\xa1" "A" "\xaa\xfd\xa4\xee\xb3" "o" ""
				"\xad\xd3\xba\xc6\xa4" "l" "\xa1" "I"
			},
			{
				1230, " " "\xa6" "U" "\xa6\xec\xa1" "A" "\xa5" "v" "\xa5" "[" ""
				"\xa9\xac\xb7\xe7\xa9\xd4\xab" "J" "\xc0\xef\xa6" "V" "\xa4" "j" ""
				"\xae" "a" "\xb9" "D" "\xb1\xdf\xa6" "w" "\xa1" "C"
			},
		}
	}, /* a7_vf.hns */
	{
		"a8_vf.hns", {
			{
				212, "\xa7" "Y" "\xa8\xcf\xac" "O" "\xa6" "b" "\xb9\xda\xa4\xa4\xa1" "A" ""
				"\xbd\xd6\xaf\xe0\xb1\xb5\xa8\xfc\xa4" "@" "\xad\xd3\xba\xc6\xa4" ""
				"l" "\xb3\xba\xb5" "M" "\xaf\xe0\xb0\xf7\xb7\xb4\xb1\xbc\xb3" "o" ""
				"\xae" "y" "\xab\xb0\xb3\xf9\xa9" "O" "\xa1" "H" "\xa6" "A" ""
				"\xb8\xd5\xa4" "@" "\xa6\xb8\xa7" "a" "\xa1" "I" "\xb2" "{" "\xa6" ""
				"b" "\xa4\xa3\xb0\xb5\xa1" "A" "\xb4" "N" "\xa5\xc3\xbb\xb7\xa8" ""
				"S" "\xa6\xb3\xbe\xf7\xb7" "|" "\xa4" "F" "\xa1" "C"
			},
			{320, ""},
		}
	}, /* a8_vf.hns */
	{
		"a9_vf.hns", {
			{
				30, "\xa9\xd4\xb6\xa9\xbc" "w" "\xa1" "A" "\xa7\xda\xa4" "w" "\xb8" "g" ""
				"\xa6" "V" "\xb0\xea\xa4\xfd\xb3\xf8\xa7" "i" "\xa7" "A" ""
				"\xbc\xf6\xa4\xdf\xb3" "B" "\xb2" "z" "\xb3" "o" ""
				"\xa5\xf3\xa8\xc6\xaa\xba\xba" "A" "\xab\xd7\xa4" "F" "\xa1" "C" ""
				"\xb0\xea\xa4\xfd\xad" "n" "\xa7\xda\xa6" "V" "\xa7" "A" ""
				"\xaa\xed\xa5\xdc\xb7" "q" "\xb7" "N" "\xa1" "C"
			},
			{125, ""},
			{
				200, "\xa8\xb9\xa5\xd0\xa5\xfd\xa5\xcd\xa7" "i" "\xb6" "D" ""
				"\xa7\xda\xad\xcc\xa9\xd2\xa6\xb3\xa7" "A" "\xb8" "g" ""
				"\xbe\xfa\xb9" "L" "\xaa\xba\xa8\xc6\xa1" "C"
			},
			{
				300, "\xa7\xda\xa5\xad\xc0" "R" "\xa6" "a" "\xac\xdd\xb5\xdb\xb3" "o" ""
				"\xa4\xa3\xa5\xad\xa4" "Z" "\xaa\xba\xa4" "@" ""
				"\xa4\xd1\xb5\xb2\xa7\xf4\xa1" "A" ""
				"\xa5\xa6\xc0\xb3\xb8\xd3\xa9\xf1\xa6" "b" ""
				"\xa4\xeb\xa5\xfa\xa4\xa7\xa4" "U" "\xa1" "A" ""
				"\xa6\xd3\xa4\xa3\xac" "O" "\xa9\xf1\xa6" "b" ""
				"\xb6\xa7\xa5\xfa\xa4\xa7\xa4" "U" "\xa1" "C"
			},
			{
				400, "\xc1\xd9\xac" "O" "\xa6\xb3\xa4" "@" ""
				"\xa8\xc7\xaf\xab\xaf\xb5\xa5\xbc\xb8\xd1\xa1" "C" "\xa6\xfd\xac" ""
				"O" "\xa1" "A" "\xa5\xfd\xa5\xcd\xa1" "A" "\xb5" "L" "\xbd\xd7\xa7" ""
				"A" "\xa8\xec\xa8\xba\xb8\xcc\xa1" "A" "\xa7" "A" "\xb3\xa3\xb0" ""
				"t" "\xba\xd9\xac\xb0\xac" "O" "\xa7\xda\xad\xcc\xaa\xba\xaf" "S" ""
				"\xa8\xcf\xa1" "C"
			},
			{515, ""},
		}
	}, /* a9_vf.hns */
};

#endif
