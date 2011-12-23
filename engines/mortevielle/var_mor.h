
/*         Constantes, Types et Variables pour le

              M A N O I R   DE   M O R T E V I E L L E

                                                    ( version Nov 1988
                                                             +
                                                  musique & paroles Oct 88 )
                                                                             */


/*---------------------------------------------------------------------------*/
/*-------------------------   CONSTANTES   ----------------------------------*/
/*---------------------------------------------------------------------------*/

const real freq0 = 1.19318e6;

const integer seg_syst = 0x6fed;
const integer segmou = 0x6f00;
const integer segdon = 0x6c00;
const integer adani = 0x7314;
const integer adword = 0x4000;
const integer adtroct = 0x406b;
const integer adcfiec = 0x4300;
const integer adbruit = 0x5cb0;/*2C00;*/
const integer adbruit1 = 0x6924;/*3874;*/
const integer adbruit2 = 0x6b30;/*3A80;*/
const integer adbruit3 = 0x6ba6;/*3AF6;*/
const integer adbruit5 = 0x3b50;
const integer adson = 0x5cb0;/*2C00;*/
const integer adson2 = 0x60b0;/*3000;*/
const integer offsetb1 = 6;
const integer offsetb2 = 4;
const integer offsetb3 = 6;

const integer null = 255;

const integer tempo_mus = 71;
const integer tempo_bruit = 78;
const integer tempo_f = 80;
const integer tempo_m = 89;

const integer ti1 = 410;
const integer ti2 = 250;
const integer maxti = 7975;
const integer maxtd = 600;
const integer max_rect = 14;

const integer ams = 0;
const integer cga = 1;
const integer ega = 2;
const integer her = 3;
const integer tan = 4;

const integer c_repon = 0;
const integer c_st41 = 186;
const integer c_tparler = 247;
const integer c_paroles = 292;
const integer c_tmlieu = 435;
const integer c_dialpre = 456;
const integer c_action = 476;
const integer c_saction = 497;
const integer c_dis = 502;
const integer c_fin = 510;    /*  =>   n'existe pas  ; si !! */

const integer arega = 0;
const integer asoul = 154;
const integer aouvr = 282;
const integer achai = 387;
const integer acha = 492;
const integer arcf = 1272;
const integer arep = 1314;
const integer amzon = 1650;
const integer fleche = 1758;

const integer no_choice = 0;
const integer invent = 1;
const integer depla = 2;
const integer action = 3;
const integer saction = 4;
const integer discut = 5;
const integer fichier = 6;
const integer sauve = 7;
const integer charge = 8;

const integer attacher = 0x301;
const integer attendre = 0x302;
const integer defoncer = 0x303;
const integer dormir = 0x304;
const integer ecouter = 0x305;
const integer entrer = 0x306;
const integer fermer = 0x307;
const integer fouiller = 0x308;
const integer frapper = 0x309;
const integer gratter = 0x30a;
const integer lire = 0x30b;
const integer manger = 0x30c;
const integer mettre = 0x30d;
const integer ouvrir = 0x30e;
const integer prendre = 0x30f;
const integer regarder = 0x310;
const integer sentir = 0x311;
const integer sonder = 0x312;
const integer sortir = 0x313;
const integer soulever = 0x314;
const integer tourner = 0x315;

const integer scacher = 0x401;
const integer sfouiller = 0x402;
const integer slire = 0x403;
const integer sposer = 0x404;
const integer sregarder = 0x405;

  const array<0,17,byte> tabdbc
          = {{7, 23, 7, 14, 13, 9, 14, 9, 5, 12, 6, 12, 13, 4, 0, 4, 5, 9}};
  const array<0,15,byte> tabdph
          = {{0, 10, 2, 0, 2, 10, 3, 0, 3, 7, 5, 0, 6, 7, 7, 10}};
  const array<0,25,byte> typcon
          = {{0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3}};
  const array<0,25,byte> intcon
          = {{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}};
  const array<0,363,byte> tnocon
          = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0}};


const matrix<1,8,1,4,byte> don
        = {{{{ 7, 37, 22,  8}},
          {{19, 33, 23,  7}},
          {{31, 89, 10, 21}},
          {{43, 25, 11,  5}},
          {{55, 37,  5,  8}},
          {{64, 13, 11,  2}},
          {{62, 22, 13,  4}},
          {{62, 25, 13,  5}}}};

const array<1,2,varying_string<11> > fic
    = {{ " Sauvegarde",
        " Chargement" }};

const array<0,1,byte> addv
   = {{8,8}};

const char recom[] = " Recommence  ";

     const char f3[] = "F3: Encore";
     const char f8[] = "F8: Suite";

  const integer max_patt = 20;


 const array<0,15,byte> rang
      = {{15,14,11,7,13,12,10,6,9,5,3,1,2,4,8,0}};


/*---------------------------------------------------------------------------*/
/*--------------------------------   TYPES   --------------------------------*/
/*---------------------------------------------------------------------------*/

typedef varying_string<11> path_chars;
typedef real long_integer;
typedef varying_string<255> str255;
typedef varying_string<125> str125;
typedef varying_string<30> str30;
typedef varying_string<11> str11;
typedef file<byte> fichier_byte;
struct sav_chaine {
                  integer conf;
                  array<0,10,char> pourc;
                 array<0,42,char> teauto;
                   array<0,30,char> sjer;
                 integer mlieu,iboul,ibag,icave,ivier,ipuit;
                 integer derobj,iloic,icryp;
                  boolean ipre;
                 char heure;
};
struct registres {
           integer ax,bx,cx,dx,bp,si,di,ds,es,flags;
};
typedef array<1,1410,char> phrase;
typedef array<0,maxti,integer> tabint;
 struct ind {
         integer indis;
         byte point;
 };
typedef array<0,maxtd,ind> tabind;

typedef matrix<1,7,0,24,byte> tab_mlieu;

 typedef array<0,255,real> table;
typedef array<0,255,integer> tablint;

  typedef integer word1;
struct chariot {
           integer val,
           code,
           acc,
           freq,
           rep;
};

struct doublet {
        byte x,y;
};
typedef array<1,16,doublet> tabdb;
typedef array<0,107,integer> tfxx;
struct rectangle {
            integer x1, x2, y1, y2;
            boolean etat;
};

typedef array<1,max_rect,rectangle> mult_rect;
typedef varying_string<40> str40;

 struct pattern {
             byte tay, tax;
             matrix<1,max_patt, 1,max_patt,byte> des;
 };


  struct nhom {
           byte n;     /* numero entre 0 et 32 */
           array<0,3,byte> hom;
  };

  typedef array<0,15,nhom> t_nhom;

  struct t_pcga {
             byte p;
             t_nhom a;
  };

  typedef array<0,90,t_pcga> pal_cga;


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


integer x,
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

array<0,6400,integer> t_cph;
array<0,4000,byte> tabdon;
/*   t_dxx  : array[0..121] of integer;*/
str125 stpou;                               /* donne le % d'indices */
byte is;
char mode;
fichier_byte ficdes;
str125 al_mess,
err_mess,
ind_mess,
al_mess2;

 array<1,8,integer> invt,
nbrep,
nbrepm,
 disc;
  array<0,4,integer> msg;
 array<1,7,integer> depl;
 array<1,8,varying_string<22> > inv;
 array<1,7,varying_string<23> > dep;
 array<1,21,varying_string<10> > act;
 array<1,5,varying_string<11> > self_;
 array<1,8,varying_string<5> > dis;
 array<1,7,char> touv;
 sav_chaine s,s1;
array<0,390,byte> bufcha;

matrix<1,6,0,23,byte> lettres;

array<0,15,byte> palher;

    tabint t_mot;
integer tay_tchar;
    tabind t_rec;
   file<ind> sauv_t;
   untyped_file fibyte;
   tab_mlieu v_lieu;
        tfxx l;
      tablint tbi;
chariot c1, c2, c3;
   real addfix;
   pal_cga palsav;
   array<0,90,tabdb> tabpal;
   pal_cga palcga;
      array<0,14,pattern> tpt;



/*---------------------------------------------------------------------------*/
/*--------------------   PROCEDURES  ET  FONCTIONS   ------------------------*/
/*---------------------------------------------------------------------------*/

/* procedure box(c,Gd,xo,yo,xi,yi,patt:integer); external 'c:\mc\boite.com'; */

void hirs()
{
      const array<0,13,byte> tandy
           = {{113,80,90,14,63,6,50,56,2,3,6,7,0,0}};
      const array<0,12,byte> herc
        = {{50,40,41,9,103,3,100,100,2,3,0,0,0}};
       integer i, j;

  switch (gd) {
    case cga :
     {
       graphcolormode;
       graphbackground(0);
       palette(1);
       res=1;
     }
     break;
    case ams :
     {
       hires;
       inline_((real)(0xb8)/6/0/        /*  =>  mov ax,6 */
              0xcd/0x10);       /*  =>  int 16   */
       port[0x3d9]=15;
       port[0x3df]=0;
       port[0x3dd]=15;
       res=2;
     }
     break;
    case ega :
     {
       inline_((real)(0xb8) / 14 / 0 /     /*  MOV AX, 14   ; mode video 14 = 640*200 16 couleurs */
              0xcd / 0x10);        /*  INT 16  */
       res=2;
     }
     break;
    case her :
     {
       port[0x3bf]=3;
       port[0x3b8]=2;
       for( i=0; i <= 12; i ++)
       {
         port[0x3b4]=i;
         port[0x3b5]=herc[i];
       }
       inline_((real)(0xfc)/0xb9/0/0x80/0xb8/0/0xb0/0x8e/0xc0/0x31/0xff/0x31/0xc0/0xf3/0xab);
       port[0x3b8]=10;
       res=2;
     }
     break;
    case tan :
     {
       port[0x3d8]=0;
       port[0x3da]=3;
       port[0x3de]=0x14;
       for( i=0; i <= 13; i ++)
         {
           port[0x3d4]=i;
           port[0x3d5]=tandy[i];
         }
       port[0x3da]=2;
       port[0x3de]=0;
       port[0x3d8]=port[0x3d8] | (11 & 0xef);
       port[0x3dd]=port[0x3dd] | 1;
       port[0x3df]=port[0x3df] | 0xc0;
       box(0,gd,0,0,640,200,255);
       res=1;
     }
     break;
  }
}

/* procedure affput(Chx,Gd,x,y,coul,char:integer); external 'c:\mc\divaf.com'; */

void affcar(integer gd,integer x,integer y,integer coul,integer char)
{
  if (res==1)  affput(1,gd,((cardinal)x >> 1),y,coul,char);
           else affput(1,gd,x,y,coul,char);
}

void putpix(integer gd,integer x,integer y,integer coul)
{
  affput(0,gd,x,y,coul,0);
}
