
void dem1() {
	untyped_file f;
	int k;

	/* -- demande de disk 1 -- */
	assign(f, "mort.005");
	/*$i-*/
	k = ioresult;
	reset(f);
	while (ioresult != 0) {
		show_mouse();
		k = do_alert(al_mess, 1);
		hide_mouse();
		reset(f);
	}
	close(f);
}

void takesav(int n) {
	untyped_file f;
	int i;
	varying_string<10> st;

	dem1();
	/* -- chargement du fichier 'sauve#n.mor' -- */
	st = string("sav") + chr(n + 48) + ".mor";
	assign(f, st);
	reset(f, 497);
	blockread(f, s1, 1);
	if (ioresult != 0) {
		i = do_alert(err_mess, 1);
		exit(0);
	}
	s = s1;
	for (i = 0; i <= 389; i ++) tabdon[i + acha] = bufcha[i];
	close(f);
}

void ld_game(int n) {
	hide_mouse();
	maivid();
	takesav(n);
	/* -- disquette 2 -- */
	dem2();
	/* -- mises en place -- */
	theure();
	dprog();
	antegame();
	show_mouse();
}

void sv_game(int n) {
	untyped_file f;
	int i;

	hide_mouse();
	tmaj3();
	dem1();
	/* -- sauvegarde du fichier 'sauve#n.mor' -- */
	for (i = 0; i <= 389; i ++) bufcha[i] = tabdon[i + acha];
	s1 = s;
	if (s1.mlieu == 26)  s1.mlieu = 15;
	assign(f, string("sav") + chr(n + 48) + ".mor");
	rewrite(f, 497);
	blockwrite(f, s1, 1);
	close(f);
	dem2();
	show_mouse();
}




