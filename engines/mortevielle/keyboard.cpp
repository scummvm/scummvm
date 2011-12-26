char readkey1() {
	char c;

	char readkey1_result;
	input >> kbd >> c;
	readkey1_result = c;
	return readkey1_result;
}

int testou() {
	char ch;

	int testou_result;
	input >> kbd >> ch;
	switch (ch)  {
	case '\23' :
		sonoff = ! sonoff;
		break;
	case '\1':
	case '\3':
	case '\5'  : {
		newgd = (cardinal)pred(int, ord(ch)) >> 1;
		testou_result = 61;
		return testou_result;
	}
	break;
	case '\10'  : {
		newgd = her;
		testou_result = 61;
		return testou_result;
	}
	break;
	case '\24' : {
		newgd = tan;
		testou_result = 61;
		return testou_result;
	}
	break;
	case '\26' :
		if (set::of(1, 2, eos).has(c_zzz)) {
			zzuul(adcfiec + 161, 0, 1644);
			c_zzz = succ(int, c_zzz);
			testou_result = 61;
			return testou_result;
		}
		break;
	case '\33' :
		if (keypressed())  input >> kbd >> ch;
		break;
	}
	testou_result = ord(ch);
	return testou_result;
}

void teskbd() {
	int dum;

	if (keypressed())  dum = testou();
}

