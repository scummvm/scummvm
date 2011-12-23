#include "ptoc.h"



/*$v-*/
/*$k-*/
/*$x-*/
/*$c-*/

#include "var_mor.h"          /* les variables */
#include "keyboard.h"          /* la gestion du clavier */
#include "mouse.h"          /* la souris  */
#include "level15.h"          /* les fonctions de base */
#include "droite.h"          /* pour faire une droite */
#include "boite.h"
#include "sprint.h"          /* l'affichage des textes */
#include "outtext.h"          /* fonctions sup‚rieures d'affichage */
#include "parole.h"          /* les fonctions de la parole */
#include "alert.h"
#include "parole2.h"          /* les fonctions de la parole */
/*  debug              le debugging */
#include "ques.h"          /* les questions */
#include "menu.h"          /* les fonctions du menu */
#include "mor.h"          /* divers */
#include "taffich.h"          /* chargement des dessins */
#include "ovd1.h"          /* les fonctions sur disque 1 */
#include "mor2.h"          /* divers */
#include "actions.h"          /* les actions */
#include "prog.h"          /* fonctions toplevel */
#include "disk.h"          /* chargements et sauvegardes */

/*  ecrihexa            utilitaire */

void divers(integer np, boolean b) {
	teskbd();
	do {
		parole(np, 0, 0);
		atf3f8(key);
		if (newgd != gd) {
			gd = newgd;
			hirs();
			aff50(b);
		}
	} while (!(key == 66));
}

/* NIVEAU 0 */

int main(int argc, const char *argv[]) {
	/*init_debug;*/
	/*  ecri_seg;*/
	pio_initialize(argc, argv);
	gd = cga;
	newgd = gd;
	zuul = false;
	tesok = false;
	chartex();
	charpal();
	charge_cfiph();
	charge_cfiec();
	zzuul(adcfiec + 161, 0, 1644);
	c_zzz = 1;
	init_nbrepm();
	init_mouse();
	/*  crep:=memw[$7f00:0];
	  memw[$7f00:0]:= crep+1;
	  if (memw[$7f00:0]<> crep+1) then
	     begin
	       hirs;
	       show_mouse;
	       crep:=do_alert('[1][ 512 k  minimum requis  !!][OK]',1);
	       clrscr;
	       halt;
	     end;
	  if (Dseg+$1000>adbruit5) then
	     begin
	       hirs;
	       show_mouse;
	       crep:=do_alert('[1][ Il ne reste pas assez de |place m‚moire  !!][OK]',1);
	       clrscr;
	       halt;
	     end;*/
	init_lieu();
	arret = false;
	sonoff = false;
	f2_all = false;
	textcolor(9);
	teskbd();
	dialpre();
	newgd = gd;
	teskbd();
	if (newgd != gd)  gd = newgd;
	hirs();
	ades = 0x7000;
	aff50(false);
	mlec = 0;
	divers(142, false);

	ani50();
	divers(143, true);
	suite();
	music();
	adzon();
	takesav(0);
	if (rech_cfiec)  charge_cfiec();
	for (crep = 1; crep <= c_zzz; crep ++) zzuul(adcfiec + 161, 0, 1644);
	charge_bruit5();
	init_menu();

	theure();
	dprog();
	hirs();
	dessine_rouleau();
	show_mouse();
	do {
		tjouer();
	} while (!arret);
	hide_mouse();
	clrscr;
	/*out_debug;*/
	return EXIT_SUCCESS;
}

