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

static char const versaillesFREpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesFREpilPwd[] = "LELOUPETLATETE";
static char const versaillesDEEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesDEEpilPwd[] = "DIEENTENUNDDERPUDEL";
static char const versaillesENEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesENEpilPwd[] = "FOXANDCRANE";
static char const versaillesESEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesESEpilPwd[] = "ELZORROYLAGRULLA";
static char const versaillesITEpilMsg[] = "FELIXFORTUNADIVINUMEXPLORATUMACTUIIT";
static char const versaillesITEpilPwd[] = "LEANATREEILCANE";

static char const versaillesFRBombPwd[] = "JEMENVAISMAISLETATDEMEURERATOUJOURS";
static char const versaillesDEBombPwd[] =
    "MONARCHEN IST ES NICHT GEGEBEN VOLLKOMMENHEIT ZU ERREICHEN";
static char const versaillesENBombPwd[] = "IT IS NOT IN THE POWER OF KINGS TO ATTAIN PERFECTION";
static char const versaillesESBombPwd[] = "NO ES PODER DE REYES EL ALCANZAR LA PERFECCION";
static char const versaillesITBombPwd[] = "AI SOVRANI NON E DATO RAGGIUNGERE LA PERFEZIONE";

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

#endif
