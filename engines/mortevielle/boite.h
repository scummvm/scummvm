void boite(integer x, integer y, integer dx, integer dy, integer coul) {
	integer i;
	integer xi, yi, xo, yo;

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
			if (gd != cga)  putpix(gd, pred(integer, xi), yi, coul);
			yi = succ(integer, yi);
		} else xi = succ(integer, xi);
		putpix(gd, xo, yo, coul);
		if (yo == y + dy)  xo = succ(integer, xo);
		else {
			if (gd != cga)  putpix(gd, succ(integer, xo), yo, coul);
			yo = succ(integer, yo);
		}
	}
	show_mouse();
}
