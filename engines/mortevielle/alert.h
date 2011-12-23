/* overlay */ integer do_alert(str255 str_, integer n);


const integer nligne = 7;
const matrix<1, 2, 1, 3, integer> coord
= {{ {{150, 72, 103}},
		{{143, 107, 183}}
	}
};



static void decod(str255 s, integer &nbc, integer &nbl, integer &col, str255 &c, str30 &cs) {
	integer i, k;
	boolean v;

	val(s[2], nbc, i);
	c = "";
	nbl = 0;
	i = 5;
	k = 0;
	v = true;
	col = 0;

	while (s[i] != ']') {
		c = c + s[i];
		if ((s[i] == '|') || (s[i + 1] == ']')) {
			if (k > col)  col = k;
			k = 0;
			nbl = nbl + 1;
		} else if (s[i] != ' ')  v = false;
		i = i + 1;
		k = k + 1;
	}
	if (v)  {
		c = "";
		col = 20;
	} else {
		c = c + ']';
		col = col + 6;
	}
	i = i + 1;
	cs = copy(s, i, 30);
	if (res == 2)  col = col * 6;
	else col = col * 10;
}



static void posit(integer ji, integer &coldep, integer &esp) {
	putxy(coldep + (40 + esp) *pred(integer, ji), 98);
}



static void fait_boite(integer lidep, integer nli, integer tx) {
	integer x, y, xx, yy;

	if (tx > 640)  tx = 640;
	x = 320 - ((cardinal)tx >> 1);
	y = pred(integer, lidep) << 3;
	xx = x + tx;
	yy = y + (nli << 3);
	box(15, gd, x, y, xx, yy, 255);
	box(0, gd, x, y + 2, xx, y + 4, 255);
	box(0, gd, x, yy - 4, xx, yy - 2, 255);
}



static void fait_choix(str30 c, integer &coldep, integer &nbcase, array<1, 2, varying_string<3> > &s, integer &esp) {
	integer i, l, x;
	char ch;

	i = 1;
	x = coldep;
	for (l = 1; l <= nbcase; l ++) {
		s[l] = "";
		do {
			i = i + 1;
			ch = c[i];
			s[l] = s[l] + ch;
		} while (!(c[i + 1] == ']'));
		i = i + 2;
		while (length(s[l]) < 3)  s[l] = s[l] + ' ';
		putxy(x, 98);
		writeg(string(' ') + s[l] + ' ', 0);
		x = x + esp + 40;
	}
}

integer do_alert(str255 str_, integer n) {
	integer coldep, esp, i, l, nbcase, quoi, ix;
	str255 st, chaine;
	matrix<1, 2, 1, 2, integer> limit;
	char c, dumi;
	array<1, 2, varying_string<3> > s;
	integer cx, cy, cd, nbcol, nblig;
	boolean touch, newaff, test, test1, test2, test3, dum;
	str30 cas;


	/*debug('** do_alert **');*/
	integer do_alert_result;
	hide_mouse();
	while (keypressed())  input >> kbd >> dumi;
	clic = false;
	decod(str_, nbcase, nblig, nbcol, chaine, cas);
	sauvecr(50, succ(integer, nligne) << 4);

	i = 0;
	if (chaine == "") {
		fait_boite(10, 5, nbcol);
	} else {
		fait_boite(8, 7, nbcol);
		i = 0;
		ywhere = 70;
		do {
			cx = 320;
			st = "";
			while (!(set::of('\174', '\135', eos).has(chaine[i + 1]))) {
				i = i + 1;
				st = st + chaine[i];
				if (res == 2)  cx = cx - 3;
				else cx = cx - 5;
			}
			putxy(cx, ywhere);
			ywhere = ywhere + 6;
			writeg(st, 4);
			i = i + 1;
		} while (!(chaine[i] == ']'));
	}
	if (nbcase == 1)  esp = nbcol - 40;
	else esp = (cardinal)(nbcol - nbcase * 40) >> 1;
	coldep = 320 - ((cardinal)nbcol >> 1) + ((cardinal)esp >> 1);
	fait_choix(cas, coldep, nbcase, s, esp);
	limit[1][1] = ((cardinal)(coldep) >> 1) * res;
	limit[1][2] = limit[1][1] + 40;
	if (nbcase == 1) {
		limit[2][1] = limit[2][2];
	} else {
		limit[2][1] = ((cardinal)(320 + ((cardinal)esp >> 1)) >> 1) * res;
		limit[2][2] = (limit[2][1]) + 40;
	}
	show_mouse();
	quoi = 0;
	dum = false;
	do {
		dumi = '\377';
		mov_mouse(dum, dumi);
		cx = x_s;
		cy = y_s;
		test = (cy > 95) && (cy < 105);
		newaff = false;
		if (test) {
			test1 = (cx > limit[1][1]) && (cx < limit[1][2]);
			test2 = test1;
			if (nbcase > 1)  test2 = test1 || ((cx > limit[2][1]) && (cx < limit[2][2]));
			if (test2) {
				newaff = true;
				if (test1)  ix = 1;
				else ix = 2;
				if (ix != quoi) {
					hide_mouse();
					if (quoi != 0) {
						posit(quoi, coldep, esp);
						writeg(string(' ') + s[quoi] + ' ', 0);
					}
					posit(ix, coldep, esp);
					writeg(string(' ') + s[ix] + ' ', 1);
					quoi = ix;
					show_mouse();
				}
			}
		}
		if ((quoi != 0) && ! newaff) {
			hide_mouse();
			posit(quoi, coldep, esp);
			writeg(string(' ') + s[quoi] + ' ', 0);
			quoi = 0;
			show_mouse();
		}
		test3 = (cy > 95) && (cy < 105) && (((cx > limit[1][1]) && (cx < limit[1][2]))
		                                    || ((cx > limit[2][1]) && (cx < limit[2][2])));
	} while (!clic);
	clic = false;
	hide_mouse();
	if (! test3)  {
		quoi = n;
		posit(n, coldep, esp);
		writeg(string(' ') + s[n] + ' ', 1);
	}
	charecr(50, succ(integer, nligne) << 4);
	show_mouse();
	do_alert_result = quoi;
	return do_alert_result;
}
