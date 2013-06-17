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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef __acci_h__
#define __acci_h__

namespace Avalanche {

/* verb codes */
const char vb_exam = '\1'; const char vb_open = '\2'; const char vb_pause = '\3'; const char vb_get = '\4'; const char vb_drop = '\5';
const char vb_inv = '\6'; const char vb_talk = '\7'; const char vb_give = '\10'; const char vb_drink = '\11'; const char vb_load = '\12';
const char vb_save = '\13'; const char vb_pay = '\14'; const char vb_look = '\15'; const char vb_break = '\16'; const char vb_quit = '\17';
const char vb_sit = '\20'; const char vb_stand = '\21'; const char vb_go = '\22'; const char vb_info = '\23'; const char vb_undress = '\24';
const char vb_wear = '\25'; const char vb_play = '\26'; const char vb_ring = '\27'; const char vb_help = '\30';
const char vb_larrypass = '\31'; const char vb_phaon = '\32'; const char vb_boss = '\33'; const char vb_pee = '\34';
const char vb_cheat = '\35'; const char vb_magic = '\36'; const char vb_restart = '\37'; const char vb_eat = '\40';
const char vb_listen = '\41'; const char vb_buy = '\42'; const char vb_attack = '\43'; const char vb_password = '\44';
const char vb_dir = '\45'; const char vb_die = '\46'; const char vb_score = '\47'; const char vb_put = '\50';
const char vb_kiss = '\51'; const char vb_climb = '\52'; const char vb_jump = '\53'; const char vb_highscores = '\54';
const char vb_wake = '\55'; const char vb_hello = '\56'; const char vb_thanks = '\57';

const char vb_smartalec = '\371'; const char vb_expletive = '\375';

const char pardon = '\376'; /* =didn't understand / wasn't given. */

struct vocab {
         byte n; varying_string<11> w;
};

struct ranktype {
            word score; varying_string<12> title;
};


const integer nowords = 277; /* how many words does the parser know? */
const char nowt = '\372';
const char moved = '\0'; /* This word was moved. (Usually because it was the subject of
  conversation.) */

const integer first_password = 89; /* Words[first_password] should equal "TIROS". */

const array<1,nowords,vocab> words = 

/* Verbs, 1-49 */
{{{1, "EXAMINE"},   {1, "READ"},      {1, "XAM"},                  /* short */
 {2, "OPEN"},      {2, "LEAVE"},     {2, "UNLOCK"},
 {3, "PAUSE"},     {47, "TA"},            /* Early to avoid Take and Talk. */
 {4, "TAKE"},      {4, "GET"},       {4, "PICK"},
 {5, "DROP"},      {6, "INVENTORY"}, {7, "TALK"},
 {7, "SAY"},       {7, "ASK"},
 {8, "GIVE"},      {9, "DRINK"},     {9, "IMBIBE"},
 {9, "DRAIN"},     {10, "LOAD"},      {10, "RESTORE"},
 {11, "SAVE"},      {12, "BRIBE"},     {12, "PAY"},
 {13, "LOOK"},      {14, "BREAK"},     {15, "QUIT"},
 {15, "EXIT"},      {16, "SIT"},       {16, "SLEEP"},
 {17, "STAND"},

 {18, "GO"},        {19, "INFO"},      {20, "UNDRESS"},
 {20, "DOFF"},
 {21, "DRESS"},     {21, "WEAR"},      {21, "DON"},
 {22, "PLAY"},
 {22, "STRUM"},     {23, "RING"},      {24, "HELP"},
 {25, "KENDAL"},    {26, "CAPYBARA"},  {27, "BOSS"},
 {255,"NINET"},     /* block for NINETY */
 {28, "URINATE"},   {28, "MINGITE"},   {29, "NINETY"},
 {30,"ABRACADABRA"},{30, "PLUGH"},     {30, "XYZZY"},
 {30, "HOCUS"},     {30, "POCUS"},     {30, "IZZY"},
 {30, "WIZZY"},     {30, "PLOVER"},
 {30,"MELENKURION"},{30, "ZORTON"},    {30, "BLERBI"},
 {30, "THURB"},     {30, "SNOEZE"},    {30, "SAMOHT"},
 {30, "NOSIDE"},    {30, "PHUGGG"},    {30, "KNERL"},
 {30, "MAGIC"},     {30, "KLAETU"},    {30, "VODEL"},
 {30, "BONESCROLLS"},{30, "RADOF"},

 {31, "RESTART"},
 {32, "SWALLOW"},   {32, "EAT"},       {33, "LISTEN"},
 {33, "HEAR"},      {34, "BUY"},       {34, "PURCHASE"},
 {34, "ORDER"},     {34, "DEMAND"},
 {35, "ATTACK"},    {35, "HIT"},       {35, "KILL"},
 {35, "PUNCH"},     {35, "KICK"},      {35, "SHOOT"},
 {35, "FIRE"},

 /* Passwords, 36: */

 {36, "TIROS"},     {36, "WORDY"},     {36, "STACK"},
 {36, "SHADOW"},    {36, "OWL"},       {36, "ACORN"},
 {36, "DOMESDAY"},  {36, "FLOPPY"},    {36, "DIODE"},
 {36, "FIELD"},     {36, "COWSLIP"},   {36, "OSBYTE"},
 {36, "OSCLI"},     {36, "TIMBER"},    {36, "ADVAL"},
 {36, "NEUTRON"},   {36, "POSITRON"},  {36, "ELECTRON"},
 {36, "CIRCUIT"},   {36, "AURUM"},     {36, "PETRIFY"},
 {36, "EBBY"},      {36, "CATAPULT"},  {36, "GAMERS"},
 {36, "FUDGE"},     {36, "CANDLE"},    {36, "BEEB"},
 {36, "MICRO"},     {36, "SESAME"},    {36, "LORDSHIP"},

 {37, "DIR"},       {37, "LS"},        {38, "DIE"},
 {39, "SCORE"},
 {40, "PUT"},       {40, "INSERT"},    {41, "KISS"},
 {41, "SNOG"},      {41, "CUDDLE"},    {42, "CLIMB"},
 {42, "CLAMBER"},   {43, "JUMP"},      {44, "HIGHSCORES"},
 {44, "HISCORES"},  {45, "WAKEN"},     {45, "AWAKEN"},
 {46, "HELLO"},     {46, "HI"},        {46, "YO"},
 {47, "THANKS"},       /* = 47, "ta", which was defined earlier. */


/* Nouns - Objects: 50-100. */

 {50, "WINE"},      {50, "BOOZE"},    {50,"NASTY"},
 {50, "VINEGAR"},   {51, "MONEYBAG"},
 {51, "BAG"},       {51, "CASH"},     {51,"DOSH"},
 {51, "WALLET"},
 {52, "BODKIN"},    {52, "DAGGER"},   {53,"POTION"},
 {54, "CHASTITY"},  {54, "BELT"},     {55,"BOLT"},
 {55, "ARROW"},     {55, "DART"},
 {56, "CROSSBOW"},  {56, "BOW"},      {57,"LUTE"},
 {58, "PILGRIM"},   {58, "BADGE"},    {59,"MUSHROOMS"},
 {59, "TOADSTOOLS"},{60, "KEY"},      {61,"BELL"},
 {62, "PRESCRIPT"}, {62, "SCROLL"},   {62,"MESSAGE"},
 {63, "PEN"},       {63, "QUILL"},    {64,"INK"},
 {64, "INKPOT"},    {65, "CLOTHES"},  {66,"HABIT"},
 {66, "DISGUISE"},  {67, "ONION"},

 {99,"PASSWORD"},

/* Objects from Also are placed between 101 and 131. */

/* Nouns - People - Male, 150-174 */
 {150, "AVVY"},      {150,"AVALOT"},    {150,"YOURSELF"},
 {150, "ME"},        {150,"MYSELF"},    {151,"SPLUDWICK"},
 {151, "THOMAS"},    {151,"ALCHEMIST"}, {151,"CHEMIST"},
 {152, "CRAPULUS"},  {152,"SERF"},      {152,"SLAVE"},
 {158, "DU"},      /* <<< Put in early for Baron DU Lustie to save confusion with Duck & Duke.*/
 {152, "CRAPPY"},    {153,"DUCK"},      {153,"DOCTOR"},
 {154, "MALAGAUCHE"},
 {155, "FRIAR"},     {155,"TUCK"},      {156,"ROBIN"},
 {156, "HOOD"},      {157,"CWYTALOT"},  {157,"GUARD"},
 {157, "BRIDGEKEEP"},{158,"BARON"},     {158,"LUSTIE"},
 {159, "DUKE"},      {159,"GRACE"},     {160,"DOGFOOD"},
 {160, "MINSTREL"},  {161,"TRADER"},    {161,"SHOPKEEPER"},
 {161,"STALLHOLDER"},
 {162, "PILGRIM"},   {162,"IBYTHNETH"}, {163,"ABBOT"},
 {163, "AYLES"},     {164,"PORT"},      {165,"SPURGE"},
 {166, "JACQUES"},   {166,"SLEEPER"},   {166,"RINGER"},

/* Nouns- People - Female: 175-199 */
 {175, "WIFE"},      {175,"ARKATA"},    {176,"GEDALODAVA"},
 {176, "GEIDA"},     {176,"PRINCESS"},  {178,"WISE"},
 {178, "WITCH"},

/* Pronouns, 200-224 */
 {200, "HIM"},       {200,"MAN"},       {200,"GUY"},
 {200, "DUDE"},      {200,"CHAP"},      {200,"FELLOW"},
 {201, "HER"},       {201,"GIRL"},      {201,"WOMAN"},
 {202, "IT"},        {202,"THING"},

 {203,"MONK"},       {204,"BARMAN"},    {204,"BARTENDER"},

/* Prepositions, 225-249 */
 {225, "TO"},        {226,"AT"},        {227,"UP"},
 {228, "INTO"},      {228,"INSIDE"},    {229,"OFF"},
 {230, "UP"},        {231,"DOWN"},      {232,"ON"},


/* Please, 251 */
 {251, "PLEASE"},

/* About, 252 */
 {252, "ABOUT"}, {252, "CONCERNING"},

/* Swear words, 253 */
       /*              I M P O R T A N T    M E S S A G E

          DO *NOT* READ THE LINES BELOW IF YOU ARE OF A SENSITIVE
          DISPOSITION. THOMAS IS *NOT* RESPONSIBLE FOR THEM.
          GOODNESS KNOWS WHO WROTE THEM.
          READ THEM AT YOUR OWN RISK. BETTER STILL, DON'T READ THEM.
          WHY ARE YOU SNOOPING AROUND IN MY PROGRAM, ANYWAY? */

 {253, "SHIT"},      {28 ,"PISS"},    {28 ,"PEE"},
 {253, "FART"},      {253,"FUCK"},    {253,"BALLS"},
 {253, "BLAST"},     {253,"BUGGER"},  {253,"KNICKERS"},
 {253, "BLOODY"},    {253,"HELL"},    {253,"DAMN"},
 {253, "SMEG"},
   /* and other even ruder words. You didn't read them, did you? Good. */

/* Answer-back smart-alec words, 249 */
 {249, "YES"},       {249,"NO"},        {249,"BECAUSE"},

/* Noise words, 255 */
 {255, "THE"},       {255,'A'},         {255,"NOW"},
 {255, "SOME"},      {255,"AND"},       {255,"THAT"},
 {255, "POCUS"},     {255,"HIS"},       
 {255, "THIS"},      {255,"SENTINEL"}}};        /* for "Ken SENT Me" */

const char what[] = "That's not possible!";

const array<1,9,ranktype> ranks = 
{{{0, "Beginner"},    {10, "Novice"},
{20, "Improving"},   {35, "Not bad"},
{50, "Passable"},    {65, "Good"},
{80, "Experienced"}, {108, "The BEST!"},
{maxint, "copyright'93"}}};


#ifdef __acci_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN varying_string<11> thats;
EXTERN varying_string<20> unknown;
EXTERN array<1,11,varying_string<20> > realwords;
EXTERN char verb,person,thing,thing2;
EXTERN boolean polite;
#undef EXTERN
#define EXTERN extern


void clearwords();
void parse();
void lookaround();
void opendoor();
void do_that();
void verbopt(char n, string& answer, char& anskey);
void have_a_drink();

} // End of namespace Avalanche.

#endif