/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "ags/lib/allegro/unicode.h"
#include "ags/globals.h"

namespace AGS3 {

/* ugetc: */
int (*ugetc)(const char *s) = utf8_getc;
/* ugetxc: */
int (*ugetx)(char **s) = utf8_getx;
/* ugetxc: */
int (*ugetxc)(const char * const *s) = (int (*)(const char * const *)) utf8_getx;
/* usetc: */
int (*usetc)(char *s, int c) = utf8_setc;
/* uwidth: */
int (*uwidth)(const char *s) = utf8_width;
/* ucwidth: */
int (*ucwidth)(int c) = utf8_cwidth;
/* uisok: */
int (*uisok)(int c) = utf8_isok;

struct UTYPE_INFO {
	int id;
	AL_METHOD(int, u_getc, (const char *s));
	AL_METHOD(int, u_getx, (char **s));
	AL_METHOD(int, u_setc, (char *s, int c));
	AL_METHOD(int, u_width, (const char *s));
	AL_METHOD(int, u_cwidth, (int c));
	AL_METHOD(int, u_isok, (int c));
	int u_width_max;
};

static UTYPE_INFO utypes[] =
{
	{ U_ASCII,    ascii_getc,    ascii_getx,    ascii_setc,    ascii_width,   ascii_cwidth,   ascii_isok,    1    },
	{ U_UTF8,     utf8_getc,     utf8_getx,     utf8_setc,     utf8_width,    utf8_cwidth,    utf8_isok,     4     }
};


/* _find_utype:
 *  Helper for locating a string type description.
 */
static UTYPE_INFO *find_utype(int type) {
	int i;

	if (type == U_CURRENT)
		type = _G(utype);

	for (i = 0; i < (int)(sizeof(utypes) / sizeof(UTYPE_INFO)); i++)
		if (utypes[i].id == type)
			return &utypes[i];

	return nullptr;
}

void set_uformat(int type) {
	UTYPE_INFO *info = find_utype(type);
	assert(info);

	if (info) {
		_G(utype) = info->id;
		ugetc = info->u_getc;
		ugetx = (int (*)(char **)) info->u_getx;
		ugetxc = (int (*)(AL_CONST char * const*)) info->u_getx;
		usetc = info->u_setc;
		uwidth = info->u_width;
		ucwidth = info->u_cwidth;
		uisok = info->u_isok;
	}
}

int get_uformat(void) {
	return _G(utype);
}

size_t ustrsize(const char *s) {
	return strlen(s);
}

static int utf8_validate(int c) {
	if (c < 0 || c > 0x10FFFF || (0xD800 <= c && c <= 0xDFFF))
		return 0xFFFD;
	return c;
}

int utf8_getc(const char *s) {
	int c = *((const unsigned char *)(s++));
	int n, t;

	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8 - n)) - 1;

		while (--n > 0) {
			t = *((const unsigned char *)(s++));

			if ((!(t & 0x80)) || (t & 0x40))
				return '^';

			c = (c << 6) | (t & 0x3F);
		}
	}

	return utf8_validate(c);
}

int utf8_getx(char **s) {
	int c = *((unsigned char *)((*s)++));
	int n, t;

	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8 - n)) - 1;

		while (--n > 0) {
			t = *((unsigned char *)((*s)++));

			if ((!(t & 0x80)) || (t & 0x40)) {
				(*s)--;
				return '^';
			}

			c = (c << 6) | (t & 0x3F);
		}
	}

	return utf8_validate(c);
}

int utf8_setc(char *s, int c) {
	int size, bits, b, i;

	c = utf8_validate(c);
	if (c < 128) {
		*s = c;
		return 1;
	}

	bits = 7;
	while (c >= (1 << bits))
		bits++;

	size = 2;
	b = 11;

	while (b < bits) {
		size++;
		b += 5;
	}

	b -= (7 - size);
	s[0] = c >> b;

	for (i = 0; i < size; i++)
		s[0] |= (0x80 >> i);

	for (i = 1; i < size; i++) {
		b -= 6;
		s[i] = 0x80 | ((c >> b) & 0x3F);
	}

	return size;
}

int utf8_width(const char *s) {
	int c = *((const unsigned char *)s);
	int n = 1;

	if (c & 0x80) {
		while (c & (0x80 >> n))
			n++;
	}

	return n;
}

int utf8_cwidth(int c) {
	int size, bits, b;

	c = utf8_validate(c);
	if (c < 128)
		return 1;

	bits = 7;
	while (c >= (1 << bits))
		bits++;

	size = 2;
	b = 11;

	while (b < bits) {
		size++;
		b += 5;
	}

	return size;
}

int utf8_isok(int c) {
	return utf8_validate(c) == c;
}


int ascii_getc(const char *s) {
	return *((const unsigned char *)s);
}

int ascii_getx(char **s) {
	return *((unsigned char *)((*s)++));
}

int ascii_setc(char *s, int c) {
	*s = c;
	return 1;
}

int ascii_width(const char *s) {
	return 1;
}

int ascii_cwidth(int c) {
	return 1;
}

int ascii_isok(int c) {
	return ((c >= 0) && (c <= 255));
}



int ustrlen(const char *s) {
	int c = 0;
	assert(s);

	while (ugetxc(&s))
		c++;

	return c;
}

int utolower(int c) {
	if ((c >= 65 && c <= 90) ||
		(c >= 192 && c <= 214) ||
		(c >= 216 && c <= 222) ||
		(c >= 913 && c <= 929) ||
		(c >= 931 && c <= 939) ||
		(c >= 1040 && c <= 1071))
		return c + 32;
	if ((c >= 393 && c <= 394))
		return c + 205;
	if ((c >= 433 && c <= 434))
		return c + 217;
	if ((c >= 904 && c <= 906))
		return c + 37;
	if ((c >= 910 && c <= 911))
		return c + 63;
	if ((c >= 1025 && c <= 1036) ||
		(c >= 1038 && c <= 1039))
		return c + 80;
	if ((c >= 1329 && c <= 1366) ||
		(c >= 4256 && c <= 4293))
		return c + 48;
	if ((c >= 7944 && c <= 7951) ||
		(c >= 7960 && c <= 7965) ||
		(c >= 7976 && c <= 7983) ||
		(c >= 7992 && c <= 7999) ||
		(c >= 8008 && c <= 8013) ||
		(c >= 8040 && c <= 8047) ||
		(c >= 8072 && c <= 8079) ||
		(c >= 8088 && c <= 8095) ||
		(c >= 8104 && c <= 8111) ||
		(c >= 8120 && c <= 8121) ||
		(c >= 8152 && c <= 8153) ||
		(c >= 8168 && c <= 8169))
		return c + -8;
	if ((c >= 8122 && c <= 8123))
		return c + -74;
	if ((c >= 8136 && c <= 8139))
		return c + -86;
	if ((c >= 8154 && c <= 8155))
		return c + -100;
	if ((c >= 8170 && c <= 8171))
		return c + -112;
	if ((c >= 8184 && c <= 8185))
		return c + -128;
	if ((c >= 8186 && c <= 8187))
		return c + -126;
	if ((c >= 8544 && c <= 8559))
		return c + 16;
	if ((c >= 9398 && c <= 9423))
		return c + 26;

	switch (c) {
	case 256:
	case 258:
	case 260:
	case 262:
	case 264:
	case 266:
	case 268:
	case 270:
	case 272:
	case 274:
	case 276:
	case 278:
	case 280:
	case 282:
	case 284:
	case 286:
	case 288:
	case 290:
	case 292:
	case 294:
	case 296:
	case 298:
	case 300:
	case 302:
	case 306:
	case 308:
	case 310:
	case 313:
	case 315:
	case 317:
	case 319:
	case 321:
	case 323:
	case 325:
	case 327:
	case 330:
	case 332:
	case 334:
	case 336:
	case 338:
	case 340:
	case 342:
	case 344:
	case 346:
	case 348:
	case 350:
	case 352:
	case 354:
	case 356:
	case 358:
	case 360:
	case 362:
	case 364:
	case 366:
	case 368:
	case 370:
	case 372:
	case 374:
	case 377:
	case 379:
	case 381:
	case 386:
	case 388:
	case 391:
	case 395:
	case 401:
	case 408:
	case 416:
	case 418:
	case 420:
	case 423:
	case 428:
	case 431:
	case 435:
	case 437:
	case 440:
	case 444:
	case 453:
	case 456:
	case 459:
	case 461:
	case 463:
	case 465:
	case 467:
	case 469:
	case 471:
	case 473:
	case 475:
	case 478:
	case 480:
	case 482:
	case 484:
	case 486:
	case 488:
	case 490:
	case 492:
	case 494:
	case 498:
	case 500:
	case 506:
	case 508:
	case 510:
	case 512:
	case 514:
	case 516:
	case 518:
	case 520:
	case 522:
	case 524:
	case 526:
	case 528:
	case 530:
	case 532:
	case 534:
	case 994:
	case 996:
	case 998:
	case 1000:
	case 1002:
	case 1004:
	case 1006:
	case 1120:
	case 1122:
	case 1124:
	case 1126:
	case 1128:
	case 1130:
	case 1132:
	case 1134:
	case 1136:
	case 1138:
	case 1140:
	case 1142:
	case 1144:
	case 1146:
	case 1148:
	case 1150:
	case 1152:
	case 1168:
	case 1170:
	case 1172:
	case 1174:
	case 1176:
	case 1178:
	case 1180:
	case 1182:
	case 1184:
	case 1186:
	case 1188:
	case 1190:
	case 1192:
	case 1194:
	case 1196:
	case 1198:
	case 1200:
	case 1202:
	case 1204:
	case 1206:
	case 1208:
	case 1210:
	case 1212:
	case 1214:
	case 1217:
	case 1219:
	case 1223:
	case 1227:
	case 1232:
	case 1234:
	case 1236:
	case 1238:
	case 1240:
	case 1242:
	case 1244:
	case 1246:
	case 1248:
	case 1250:
	case 1252:
	case 1254:
	case 1256:
	case 1258:
	case 1262:
	case 1264:
	case 1266:
	case 1268:
	case 1272:
	case 7680:
	case 7682:
	case 7684:
	case 7686:
	case 7688:
	case 7690:
	case 7692:
	case 7694:
	case 7696:
	case 7698:
	case 7700:
	case 7702:
	case 7704:
	case 7706:
	case 7708:
	case 7710:
	case 7712:
	case 7714:
	case 7716:
	case 7718:
	case 7720:
	case 7722:
	case 7724:
	case 7726:
	case 7728:
	case 7730:
	case 7732:
	case 7734:
	case 7736:
	case 7738:
	case 7740:
	case 7742:
	case 7744:
	case 7746:
	case 7748:
	case 7750:
	case 7752:
	case 7754:
	case 7756:
	case 7758:
	case 7760:
	case 7762:
	case 7764:
	case 7766:
	case 7768:
	case 7770:
	case 7772:
	case 7774:
	case 7776:
	case 7778:
	case 7780:
	case 7782:
	case 7784:
	case 7786:
	case 7788:
	case 7790:
	case 7792:
	case 7794:
	case 7796:
	case 7798:
	case 7800:
	case 7802:
	case 7804:
	case 7806:
	case 7808:
	case 7810:
	case 7812:
	case 7814:
	case 7816:
	case 7818:
	case 7820:
	case 7822:
	case 7824:
	case 7826:
	case 7828:
	case 7840:
	case 7842:
	case 7844:
	case 7846:
	case 7848:
	case 7850:
	case 7852:
	case 7854:
	case 7856:
	case 7858:
	case 7860:
	case 7862:
	case 7864:
	case 7866:
	case 7868:
	case 7870:
	case 7872:
	case 7874:
	case 7876:
	case 7878:
	case 7880:
	case 7882:
	case 7884:
	case 7886:
	case 7888:
	case 7890:
	case 7892:
	case 7894:
	case 7896:
	case 7898:
	case 7900:
	case 7902:
	case 7904:
	case 7906:
	case 7908:
	case 7910:
	case 7912:
	case 7914:
	case 7916:
	case 7918:
	case 7920:
	case 7922:
	case 7924:
	case 7926:
	case 7928:
		return c + 1;
	case 304:
		return c + -199;
	case 376:
		return c + -121;
	case 385:
		return c + 210;
	case 390:
		return c + 206;
	case 398:
		return c + 79;
	case 399:
		return c + 202;
	case 400:
		return c + 203;
	case 403:
		return c + 205;
	case 404:
		return c + 207;
	case 406:
	case 412:
		return c + 211;
	case 407:
		return c + 209;
	case 413:
		return c + 213;
	case 415:
		return c + 214;
	case 422:
	case 425:
	case 430:
		return c + 218;
	case 439:
		return c + 219;
	case 452:
	case 455:
	case 458:
	case 497:
		return c + 2;
	case 902:
		return c + 38;
	case 908:
		return c + 64;
	case 8025:
	case 8027:
	case 8029:
	case 8031:
		return c + -8;
	case 8124:
	case 8140:
	case 8188:
		return c + -9;
	case 8172:
		return c + -7;
	default:
		return c;
	}
}

int utoupper(int c) {
	if ((c >= 97 && c <= 122) ||
		(c >= 224 && c <= 246) ||
		(c >= 248 && c <= 254) ||
		(c >= 945 && c <= 961) ||
		(c >= 963 && c <= 971) ||
		(c >= 1072 && c <= 1103))
		return c + -32;
	if ((c >= 598 && c <= 599))
		return c + -205;
	if ((c >= 650 && c <= 651))
		return c + -217;
	if ((c >= 941 && c <= 943))
		return c + -37;
	if ((c >= 973 && c <= 974))
		return c + -63;
	if ((c >= 1105 && c <= 1116) ||
		(c >= 1118 && c <= 1119))
		return c + -80;
	if ((c >= 1377 && c <= 1414))
		return c + -48;
	if ((c >= 7936 && c <= 7943) ||
		(c >= 7952 && c <= 7957) ||
		(c >= 7968 && c <= 7975) ||
		(c >= 7984 && c <= 7991) ||
		(c >= 8000 && c <= 8005) ||
		(c >= 8032 && c <= 8039) ||
		(c >= 8064 && c <= 8071) ||
		(c >= 8080 && c <= 8087) ||
		(c >= 8096 && c <= 8103) ||
		(c >= 8112 && c <= 8113) ||
		(c >= 8144 && c <= 8145) ||
		(c >= 8160 && c <= 8161))
		return c + 8;
	if ((c >= 8048 && c <= 8049))
		return c + 74;
	if ((c >= 8050 && c <= 8053))
		return c + 86;
	if ((c >= 8054 && c <= 8055))
		return c + 100;
	if ((c >= 8056 && c <= 8057))
		return c + 128;
	if ((c >= 8058 && c <= 8059))
		return c + 112;
	if ((c >= 8060 && c <= 8061))
		return c + 126;
	if ((c >= 8560 && c <= 8575))
		return c + -16;
	if ((c >= 9424 && c <= 9449))
		return c + -26;

	switch (c) {
	case 255:
		return c + 121;
	case 257:
	case 259:
	case 261:
	case 263:
	case 265:
	case 267:
	case 269:
	case 271:
	case 273:
	case 275:
	case 277:
	case 279:
	case 281:
	case 283:
	case 285:
	case 287:
	case 289:
	case 291:
	case 293:
	case 295:
	case 297:
	case 299:
	case 301:
	case 303:
	case 307:
	case 309:
	case 311:
	case 314:
	case 316:
	case 318:
	case 320:
	case 322:
	case 324:
	case 326:
	case 328:
	case 331:
	case 333:
	case 335:
	case 337:
	case 339:
	case 341:
	case 343:
	case 345:
	case 347:
	case 349:
	case 351:
	case 353:
	case 355:
	case 357:
	case 359:
	case 361:
	case 363:
	case 365:
	case 367:
	case 369:
	case 371:
	case 373:
	case 375:
	case 378:
	case 380:
	case 382:
	case 387:
	case 389:
	case 392:
	case 396:
	case 402:
	case 409:
	case 417:
	case 419:
	case 421:
	case 424:
	case 429:
	case 432:
	case 436:
	case 438:
	case 441:
	case 445:
	case 453:
	case 456:
	case 459:
	case 462:
	case 464:
	case 466:
	case 468:
	case 470:
	case 472:
	case 474:
	case 476:
	case 479:
	case 481:
	case 483:
	case 485:
	case 487:
	case 489:
	case 491:
	case 493:
	case 495:
	case 498:
	case 501:
	case 507:
	case 509:
	case 511:
	case 513:
	case 515:
	case 517:
	case 519:
	case 521:
	case 523:
	case 525:
	case 527:
	case 529:
	case 531:
	case 533:
	case 535:
	case 995:
	case 997:
	case 999:
	case 1001:
	case 1003:
	case 1005:
	case 1007:
	case 1121:
	case 1123:
	case 1125:
	case 1127:
	case 1129:
	case 1131:
	case 1133:
	case 1135:
	case 1137:
	case 1139:
	case 1141:
	case 1143:
	case 1145:
	case 1147:
	case 1149:
	case 1151:
	case 1153:
	case 1169:
	case 1171:
	case 1173:
	case 1175:
	case 1177:
	case 1179:
	case 1181:
	case 1183:
	case 1185:
	case 1187:
	case 1189:
	case 1191:
	case 1193:
	case 1195:
	case 1197:
	case 1199:
	case 1201:
	case 1203:
	case 1205:
	case 1207:
	case 1209:
	case 1211:
	case 1213:
	case 1215:
	case 1218:
	case 1220:
	case 1224:
	case 1228:
	case 1233:
	case 1235:
	case 1237:
	case 1239:
	case 1241:
	case 1243:
	case 1245:
	case 1247:
	case 1249:
	case 1251:
	case 1253:
	case 1255:
	case 1257:
	case 1259:
	case 1263:
	case 1265:
	case 1267:
	case 1269:
	case 1273:
	case 7681:
	case 7683:
	case 7685:
	case 7687:
	case 7689:
	case 7691:
	case 7693:
	case 7695:
	case 7697:
	case 7699:
	case 7701:
	case 7703:
	case 7705:
	case 7707:
	case 7709:
	case 7711:
	case 7713:
	case 7715:
	case 7717:
	case 7719:
	case 7721:
	case 7723:
	case 7725:
	case 7727:
	case 7729:
	case 7731:
	case 7733:
	case 7735:
	case 7737:
	case 7739:
	case 7741:
	case 7743:
	case 7745:
	case 7747:
	case 7749:
	case 7751:
	case 7753:
	case 7755:
	case 7757:
	case 7759:
	case 7761:
	case 7763:
	case 7765:
	case 7767:
	case 7769:
	case 7771:
	case 7773:
	case 7775:
	case 7777:
	case 7779:
	case 7781:
	case 7783:
	case 7785:
	case 7787:
	case 7789:
	case 7791:
	case 7793:
	case 7795:
	case 7797:
	case 7799:
	case 7801:
	case 7803:
	case 7805:
	case 7807:
	case 7809:
	case 7811:
	case 7813:
	case 7815:
	case 7817:
	case 7819:
	case 7821:
	case 7823:
	case 7825:
	case 7827:
	case 7829:
	case 7841:
	case 7843:
	case 7845:
	case 7847:
	case 7849:
	case 7851:
	case 7853:
	case 7855:
	case 7857:
	case 7859:
	case 7861:
	case 7863:
	case 7865:
	case 7867:
	case 7869:
	case 7871:
	case 7873:
	case 7875:
	case 7877:
	case 7879:
	case 7881:
	case 7883:
	case 7885:
	case 7887:
	case 7889:
	case 7891:
	case 7893:
	case 7895:
	case 7897:
	case 7899:
	case 7901:
	case 7903:
	case 7905:
	case 7907:
	case 7909:
	case 7911:
	case 7913:
	case 7915:
	case 7917:
	case 7919:
	case 7921:
	case 7923:
	case 7925:
	case 7927:
	case 7929:
		return c + -1;
	case 305:
		return c + -232;
	case 383:
		return c + -300;
	case 454:
	case 457:
	case 460:
	case 499:
		return c + -2;
	case 477:
	case 1010:
		return c + -79;
	case 595:
		return c + -210;
	case 596:
		return c + -206;
	case 601:
		return c + -202;
	case 603:
		return c + -203;
	case 608:
		return c + -205;
	case 611:
		return c + -207;
	case 616:
		return c + -209;
	case 617:
	case 623:
		return c + -211;
	case 626:
		return c + -213;
	case 629:
		return c + -214;
	case 640:
	case 643:
	case 648:
		return c + -218;
	case 658:
		return c + -219;
	case 837:
		return c + 84;
	case 940:
		return c + -38;
	case 962:
		return c + -31;
	case 972:
		return c + -64;
	case 976:
		return c + -62;
	case 977:
		return c + -57;
	case 981:
		return c + -47;
	case 982:
		return c + -54;
	case 1008:
		return c + -86;
	case 1009:
		return c + -80;
	case 7835:
		return c + -59;
	case 8017:
	case 8019:
	case 8021:
	case 8023:
		return c + 8;
	case 8115:
	case 8131:
	case 8179:
		return c + 9;
	case 8126:
		return c + -7205;
	case 8165:
		return c + 7;
	default:
		return c;
	}
}

int ustrcmp(const char *s1, const char *s2) {
	int c1, c2;
	assert(s1);
	assert(s2);

	for (;;) {
		c1 = ugetxc(&s1);
		c2 = ugetxc(&s2);

		if (c1 != c2)
			return c1 - c2;

		if (!c1)
			return 0;
	}
}

int ustrncmp(const char *s1, const char *s2, int n) {
	int c1, c2;
	assert(s1);
	assert(s2);

	if (n <= 0)
		return 0;

	for (;;) {
		c1 = ugetxc(&s1);
		c2 = ugetxc(&s2);

		if (c1 != c2)
			return c1 - c2;

		if ((!c1) || (--n <= 0))
			return 0;
	}
}

int ustricmp(const char *s1, const char *s2) {
	int c1, c2;
	assert(s1);
	assert(s2);

	for (;;) {
		c1 = utolower(ugetxc(&s1));
		c2 = utolower(ugetxc(&s2));

		if (c1 != c2)
			return c1 - c2;

		if (!c1)
			return 0;
	}
}

int ustrnicmp(const char *s1, const char *s2, int n) {
	int c1, c2;
	assert(s1);
	assert(s2);

	if (n <= 0)
		return 0;

	for (;;) {
		c1 = utolower(ugetxc(&s1));
		c2 = utolower(ugetxc(&s2));

		if (c1 != c2)
			return c1 - c2;

		if ((!c1) || (--n <= 0))
			return 0;
	}
}

int uoffset(const char *s, int index) {
	const char *orig = s;
	const char *last;
	assert(s);

	if (index < 0)
		index += ustrlen(s);

	while (index-- > 0) {
		last = s;
		if (!ugetxc(&s)) {
			s = last;
			break;
		}
	}

	return (intptr_t)s - (intptr_t)orig;
}

int ugetat(const char *s, int index) {
	assert(s);
	return ugetc(s + uoffset(s, index));
}

char *ustrlwr(char *s) {
	int pos = 0;
	int c, lc;
	assert(s);

	while ((c = ugetc(s + pos)) != 0) {
		lc = utolower(c);

		if (lc != c)
			usetat(s + pos, 0, lc);

		pos += uwidth(s + pos);
	}

	return s;
}

char *ustrupr(char *s) {
	int pos = 0;
	int c, uc;
	assert(s);

	while ((c = ugetc(s + pos)) != 0) {
		uc = utoupper(c);

		if (uc != c)
			usetat(s + pos, 0, uc);

		pos += uwidth(s + pos);
	}

	return s;
}

const char *ustrstr(const char *s1, const char *s2) {
	int len;
	assert(s1);
	assert(s2);

	len = ustrlen(s2);
	while (ugetc(s1)) {
		if (ustrncmp(s1, s2, len) == 0)
			return s1;

		s1 += uwidth(s1);
	}

	return nullptr;
}

int usetat(char *s, int index, int c) {
	int oldw, neww;
	assert(s);

	s += uoffset(s, index);

	oldw = uwidth(s);
	neww = ucwidth(c);

	if (oldw != neww)
		memmove(s + neww, s + oldw, ustrsizez(s + oldw));

	usetc(s, c);

	return neww - oldw;
}

int ustrsizez(const char *s) {
	const char *orig = s;
	assert(s);

	do {
	} while (ugetxc(&s) != 0);

	return (intptr_t)s - (intptr_t)orig;
}

const char *setlocale(int type, const char *language) {
	const char *locale = "C";
	// TODO: If needed for alfont
	return locale;
}

int need_uconvert(const char *s, int type, int newtype) {
	// TODO: need_uconvert
	return 0;
}

int uvszprintf(char *buf, int size, const char *format, va_list args) {
	error("TODO: uvszprintf");
	return 0;
}

} // namespace AGS3
