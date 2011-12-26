void boite(int x, int y, int dx, int dy, int coul) {
	int i;
	int xi, yi, xo, yo;

	/* debug('boite'); */
	hide_mouse();
	if (res == 1) {
		x = (cardinal)x >> 1;
		dx = (cardinal)dx >> 1;
	}
	xi = x;
	yi = y;
	xo = x;
	yo = y;
	for (i = 0; i <= dx + dy; i ++) {
		putpix(gd, xi, yi, coul);
		if (xi == x + dx)  {
			if (gd != cga)  putpix(gd, pred(int, xi), yi, coul);
			yi = succ(int, yi);
		} else xi = succ(int, xi);
		putpix(gd, xo, yo, coul);
		if (yo == y + dy)  xo = succ(int, xo);
		else {
			if (gd != cga)  putpix(gd, succ(int, xo), yo, coul);
			yo = succ(int, yo);
		}
	}
	show_mouse();
}
