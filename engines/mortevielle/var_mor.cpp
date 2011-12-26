
/*         Constantes, Types et Variables pour le

              M A N O I R   DE   M O R T E V I E L L E

                                                    ( version Nov 1988
                                                             +
                                                  musique & paroles Oct 88 )
                                                                             */


/*---------------------------------------------------------------------------*/
/*-------------------------   CONSTANTES   ----------------------------------*/
/*---------------------------------------------------------------------------*/

const float freq0 = 1.19318e6;

const int seg_syst = 0x6fed;
const int segmou = 0x6f00;
const int segdon = 0x6c00;
const int adani = 0x7314;
const int adword = 0x4000;
const int adtroct = 0x406b;
const int adcfiec = 0x4300;
const int adbruit = 0x5cb0;/*2C00;*/
const int adbruit1 = 0x6924;/*3874;*/
const int adbruit2 = 0x6b30;/*3A80;*/
const int adbruit3 = 0x6ba6;/*3AF6;*/
const int adbruit5 = 0x3b50;
const int adson = 0x5cb0;/*2C00;*/
const int adson2 = 0x60b0;/*3000;*/
const int offsetb1 = 6;
const int offsetb2 = 4;
const int offsetb3 = 6;

const int null = 255;

const int tempo_mus = 71;
const int tempo_bruit = 78;
const int tempo_f = 80;
const int tempo_m = 89;

const int ti1 = 410;
const int ti2 = 250;
const int maxti = 7975;
const int maxtd = 600;
const int max_rect = 14;

const int ams = 0;
const int cga = 1;
const int ega = 2;
const int her = 3;
const int tan = 4;

const int c_repon = 0;
const int c_st41 = 186;
const int c_tparler = 247;
const int c_paroles = 292;
const int c_tmlieu = 435;
const int c_dialpre = 456;
const int c_action = 476;
const int c_saction = 497;
const int c_dis = 502;
const int c_fin = 510;    /*  =>   n'existe pas  ; si !! */

const int arega = 0;
const int asoul = 154;
const int aouvr = 282;
const int achai = 387;
const int acha = 492;
const int arcf = 1272;
const int arep = 1314;
const int amzon = 1650;
const int fleche = 1758;

const int no_choice = 0;
const int invent = 1;
const int depla = 2;
const int action = 3;
const int saction = 4;
const int discut = 5;
const int fichier = 6;
const int sauve = 7;
const int charge = 8;

const int attacher = 0x301;
const int attendre = 0x302;
const int defoncer = 0x303;
const int dormir = 0x304;
const int ecouter = 0x305;
const int entrer = 0x306;
const int fermer = 0x307;
const int fouiller = 0x308;
const int frapper = 0x309;
const int gratter = 0x30a;
const int lire = 0x30b;
const int manger = 0x30c;
const int mettre = 0x30d;
const int ouvrir = 0x30e;
const int prendre = 0x30f;
const int regarder = 0x310;
const int sentir = 0x311;
const int sonder = 0x312;
const int sortir = 0x313;
const int soulever = 0x314;
const int tourner = 0x315;

const int scacher = 0x401;
const int sfouiller = 0x402;
const int slire = 0x403;
const int sposer = 0x404;
const int sregarder = 0x405;

const byte tabdbc[18] = {7, 23, 7, 14, 13, 9, 14, 9, 5, 12, 6, 12, 13, 4, 0, 4, 5, 9};
const byte tabdph[16] = {0, 10, 2, 0, 2, 10, 3, 0, 3, 7, 5, 0, 6, 7, 7, 10};
const byte typcon[26] = {0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3};
const byte intcon[26] = {1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
const byte tnocon[364] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


const matrix<1, 8, 1, 4, byte> don
= {{{{ 7, 37, 22,  8}},
		{{19, 33, 23,  7}},
		{{31, 89, 10, 21}},
		{{43, 25, 11,  5}},
		{{55, 37,  5,  8}},
		{{64, 13, 11,  2}},
		{{62, 22, 13,  4}},
		{{62, 25, 13,  5}}
	}
};

const array<1, 2, varying_string<11> > fic
= {{ " Sauvegarde",
		" Chargement"
	}
};

const array<0, 1, byte> addv
= {{8, 8}};

const char recom[] = " Recommence  ";

const char f3[] = "F3: Encore";
const char f8[] = "F8: Suite";

const int max_patt = 20;


const byte rang[16] = {15, 14, 11, 7, 13, 12, 10, 6, 9, 5, 3, 1, 2, 4, 8, 0};


/*---------------------------------------------------------------------------*/
/*--------------------------------   TYPES   --------------------------------*/
/*---------------------------------------------------------------------------*/

typedef float long_int;
typedef file<byte> fichier_byte;
struct sav_chaine {
	int conf;
	char pourc[11];
	char teauto[43];
	char sjer[31];
	int mlieu, iboul, ibag, icave, ivier, ipuit;
	int derobj, iloic, icryp;
	boolean ipre;
	char heure;
};
struct registres {
	int ax, bx, cx, dx, bp, si, di, ds, es, flags;
};
typedef array<1, 1410, char> phrase;
struct ind {
	int indis;
	byte point;
};
typedef array<0, maxtd, ind> tabind;

typedef matrix<1, 7, 0, 24, byte> tab_mlieu;

typedef int word1;
struct chariot {
	int val,
	        code,
	        acc,
	        freq,
	        rep;
};

struct doublet {
	byte x, y;
};
typedef array<1, 16, doublet> tabdb;
struct rectangle {
	int x1, x2, y1, y2;
	boolean etat;
};

typedef array<1, max_rect, rectangle> mult_rect;

struct pattern {
	byte tay, tax;
	matrix<1, max_patt, 1, max_patt, byte> des;
};


struct nhom {
	byte n;     /* numero entre 0 et 32 */
	array<0, 3, byte> hom;
};

typedef array<0, 15, nhom> t_nhom;

struct t_pcga {
	byte p;
	t_nhom a;
};

typedef array<0, 90, t_pcga> pal_cga;


/*---------------------------------------------------------------------------*/
/*------------------------------   VARIABLES   ------------------------------*/
/*---------------------------------------------------------------------------*/

boolean blo,
        bh1,
        bf1,
        bh2,
        bh4,
        bf4,
        bh5,
        bh6,
        bh8,
        bt3,
        bt7,
        bh9,

        sonoff,
        main1,
        choisi,
        test0,
        f2_all,
        imen,
        cache,
        iesc,
        perdu,
        col,
        syn,
        fouil,
        zuul,
        tesok,
        obpart,
        okdes,
        solu,
        arret,
        anyone,
        brt,
        rect,
        rech_cfiec,
        active_menu;


int x,
        y,
        t,
        vj,
        li,
        vh,
        vm,
        jh,
        mh,
        cs,
        gd,                /*  Gd = graph device  */
        hdb,
        hfb,
        heu,
        jou,
        key,
        min,
        num,
        max,
        res,
        ment,
        haut,
        caff,
        maff,
        crep,
        ades,
        iouv,
        inei,
        ctrm,
        dobj,
        msg3,
        msg4,
        mlec,
        newgd,
        c_zzz,
        mchai,
        menup,
        ipers,
        ledeb,
        lefin,
        mpers,
        mnumo,
        xprec,
        yprec,
        perdep,
        prebru,
        typlec,
        num_ph,
        xwhere,
        ywhere,
        numpal,
        lderobj,
        nb_word,
        ptr_oct,
        k_tempo,
        ptr_tcph,
        ptr_word,
        color_txt;

array<0, 6400, int> t_cph;
array<0, 4000, byte> tabdon;
/*   t_dxx  : array[0..121] of int;*/
Common::String stpou;                               /* donne le % d'indices */
byte is;
char mode;
fichier_byte ficdes;
Common::String al_mess,
       err_mess,
       ind_mess,
       al_mess2;

array<1, 8, int> invt,
      nbrep,
      nbrepm,
      disc;
array<0, 4, int> msg;
array<1, 7, int> depl;
array<1, 8, varying_string<22> > inv;
array<1, 7, varying_string<23> > dep;
array<1, 21, varying_string<10> > act;
array<1, 5, varying_string<11> > self_;
array<1, 8, varying_string<5> > dis;
array<1, 7, char> touv;
sav_chaine s, s1;
array<0, 390, byte> bufcha;

matrix<1, 6, 0, 23, byte> lettres;

array<0, 15, byte> palher;

int t_mot[maxti + 1];
int tay_tchar;
tabind t_rec;
file<ind> sauv_t;
untyped_file fibyte;
tab_mlieu v_lieu;
int l[108];
int tbi[256];
chariot c1, c2, c3;
float addfix;
pal_cga palsav;
array<0, 90, tabdb> tabpal;
pal_cga palcga;
array<0, 14, pattern> tpt;



/*---------------------------------------------------------------------------*/
/*--------------------   PROCEDURES  ET  FONCTIONS   ------------------------*/
/*---------------------------------------------------------------------------*/

/* procedure box(c,Gd,xo,yo,xi,yi,patt:int); external 'c:\mc\boite.com'; */

void hirs() {
	const byte tandy[14] = {113, 80, 90, 14, 63, 6, 50, 56, 2, 3, 6, 7, 0, 0};
	const byte herc[13] = {50, 40, 41, 9, 103, 3, 100, 100, 2, 3, 0, 0, 0};
	int i, j;

	switch (gd) {
	case cga : {
		graphcolormode;
		graphbackground(0);
		palette(1);
		res = 1;
	}
	break;
	case ams : {
		hires;
		inline_((float)(0xb8) / 6 / 0 /   /*  =>  mov ax,6 */
		        0xcd / 0x10);     /*  =>  int 16   */
		port[0x3d9] = 15;
		port[0x3df] = 0;
		port[0x3dd] = 15;
		res = 2;
	}
	break;
	case ega : {
		inline_((float)(0xb8) / 14 / 0 /     /*  MOV AX, 14   ; mode video 14 = 640*200 16 couleurs */
		        0xcd / 0x10);        /*  INT 16  */
		res = 2;
	}
	break;
	case her : {
		port[0x3bf] = 3;
		port[0x3b8] = 2;
		for (i = 0; i <= 12; i ++) {
			port[0x3b4] = i;
			port[0x3b5] = herc[i];
		}
		inline_((float)(0xfc) / 0xb9 / 0 / 0x80 / 0xb8 / 0 / 0xb0 / 0x8e / 0xc0 / 0x31 / 0xff / 0x31 / 0xc0 / 0xf3 / 0xab);
		port[0x3b8] = 10;
		res = 2;
	}
	break;
	case tan : {
		port[0x3d8] = 0;
		port[0x3da] = 3;
		port[0x3de] = 0x14;
		for (i = 0; i <= 13; i ++) {
			port[0x3d4] = i;
			port[0x3d5] = tandy[i];
		}
		port[0x3da] = 2;
		port[0x3de] = 0;
		port[0x3d8] = port[0x3d8] | (11 & 0xef);
		port[0x3dd] = port[0x3dd] | 1;
		port[0x3df] = port[0x3df] | 0xc0;
		box(0, gd, 0, 0, 640, 200, 255);
		res = 1;
	}
	break;
	}
}

/* procedure affput(Chx,Gd,x,y,coul,char:int); external 'c:\mc\divaf.com'; */

void affcar(int gd, int x, int y, int coul, int char) {
	if (res == 1)  affput(1, gd, ((cardinal)x >> 1), y, coul, char);
	else affput(1, gd, x, y, coul, char);
}

void putpix(int gd, int x, int y, int coul) {
	affput(0, gd, x, y, coul, 0);
}
