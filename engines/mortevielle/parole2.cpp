void rot_chariot() {
	c1 = c2;
	c2 = c3;
	{
		c3.val = 32;
		c3.code = 9;
	}
}

void init_chariot() {
	{
		c3.rep = 0;
		c3.freq = 0;
		c3.acc = 0;
	}
	rot_chariot();
	rot_chariot();
}


void trait_ph() {
	const int deca[3] = {300, 30, 40};

	ptr_tcph = pred(int, num_ph);
	ledeb = swap(t_cph[ptr_tcph]) + deca[typlec];
	lefin = swap(t_cph[ptr_tcph + 1]) + deca[typlec];
	nb_word = lefin - ledeb;
	ptr_tcph = (cardinal)ledeb >> 1;
	ptr_word = 0;
	do {
		memw[adword + ptr_word] = t_cph[ptr_tcph];
		ptr_word = ptr_word + 2;
		ptr_tcph = ptr_tcph + 1;
	} while (!(ptr_tcph >= ((cardinal)lefin >> 1)));

	ptr_oct = 0;
	ptr_word = 0;
	init_chariot();

	do {
		rot_chariot();
		charg_car();
		trait_car();
	} while (!(ptr_word >= nb_word));

	rot_chariot();
	trait_car();
	entroct(ord('#'));
}



void parole(int rep, int ht, int typ) {
	int savph[501];
	int i;
	int tempo;


	if (sonoff)  return;
	num_ph = rep;
	haut = ht;
	typlec = typ;
	if (typlec != 0) {
		for (i = 0; i <= 500; i ++) savph[i] = t_cph[i];
		tempo = tempo_bruit;
	} else if (haut > 5)  tempo = tempo_f;
	else tempo = tempo_m;
	addfix = (float)((tempo - addv[0])) / 256;
	cctable(tbi);
	switch (typ) {
	case 1 : {
		charge_bruit();
		/*if zuul then zzuul(adbruit,0,1095);*/
		regenbruit();
	}
	break;
	case 2 : {
		charge_son();
		charge_phbruit();
	}
	break;
	}
	trait_ph();
	litph(tbi, typ, tempo);
	if (typlec != 0)
		for (i = 0; i <= 500; i ++) {
			t_cph[i] = savph[i];
			mlec = typlec;
		}
	writepal(numpal);
}



