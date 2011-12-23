void droite(integer x, integer y, integer xx, integer yy, integer coul) {
	integer step, i;
	real a, b;
	real xr, yr, xro, yro;

	xr = x;
	yr = y;
	xro = xx;
	yro = yy;
	/*writeln(le_file,'(',x:4,',',y:4,') ==> (',xx:4,',',yy:4,')');*/
	if (abs(y - yy) > abs(x - xx)) {
		a = (real)((x - xx)) / (y - yy);
		b = (yr * xro - yro * xr) / (y - yy);
		i = y;
		if (y > yy)  step = -1;
		else step = 1;
		do {
			putpix(gd, trunc(a * i + b), i, coul);
			i = i + step;
		} while (!(i == yy));
		/*writeln(le_file,'X == ',a:3:3,' * Y + ',b:3:3);*/
	} else {
		a = (real)((y - yy)) / (x - xx);
		b = ((yro * xr) - (yr * xro)) / (x - xx);
		i = x;
		if (x > xx)  step = -1;
		else step = 1;
		do {
			putpix(gd, i, trunc(a * i + b), coul);
			i = i + step;
		} while (!(i == xx));
		/*writeln(le_file,'Y == ',a:3:3,' * X + ',b:3:3);*/
	}
}