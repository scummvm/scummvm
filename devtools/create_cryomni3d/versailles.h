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

#define VERSAILLES_LOCALIZED_FILENAMES_COUNT 5
static char const *const versaillesFRlocalizedFilenames[] = {
	"DIALOG1.GTO",
	"tous_doc.txt",
	"lien_doc.txt",
	"credits.txt",
	"LEB001__.WAV",
};

static char const versaillesFREpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesFREpilPwd[] = "LELOUPETLATETE";

static char const versaillesFRBombPwd[] = "JEMENVAISMAISLETATDEMEURERATOUJOURS";

#define VERSAILLES_MESSAGES_COUNT 146
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

#endif
