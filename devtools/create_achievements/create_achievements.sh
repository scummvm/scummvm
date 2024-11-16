#!/bin/sh

set -e

err_handler () {
    [ $? -eq 0 ] && exit
    echo "FAILURE: $0 failed to create achievements.dat"
}

usage() {
    echo "Usage: $0 [OPTION]..."
    echo "    -f, --force    Forced regeneration of all files"
    echo "    -h, --help     Show usage text"
    echo "    -v, --verbose  Verbose output"
    exit 42
}

add_steam() {
    if ! test -f "gen/steam-$1.ini"; then
        ./steam_achievements.py ${VERBOSE:+"-v"} --steamid "$1"
        echo -----------------------------------------
    fi
}

add_steamlike_gog() {
    if ! test -f "gen/galaxy-$2.ini"; then
        ./steam_achievements.py ${VERBOSE:+"-v"} --steamid "$1" --saveasgalaxyid "$2"
        echo -----------------------------------------
    fi
}

trap err_handler EXIT

FORCE=0
VERBOSE=""

while [ -n "$1" ]; do
    case "$1" in
        -f|--force)   FORCE=1 ;;
        -h|--help)    usage ;;
        -v|--verbose) VERBOSE="-v" ;;
        *) echo "$1 is invalid option"; usage ;;
    esac
    shift
done

if test "$FORCE" = "1"; then
    rm -f gen/*
fi

#AGS games:
add_steam 80310
add_steamlike_gog 80310 1207659073
add_steam 80330
add_steamlike_gog 80330 1207662883
add_steam 80340
add_steamlike_gog 80340 1207662893
add_steam 80350
add_steamlike_gog 80350 1207662903
add_steam 80360
add_steamlike_gog 80360 1207662913
add_steam 212050
add_steamlike_gog 212050 1207659043
add_steam 227000
add_steamlike_gog 227000 1207659144
add_steam 236930
add_steamlike_gog 236930 1207664393
add_steam 252370
add_steamlike_gog 252370 1207660263
add_steam 253110
add_steam 264560
add_steam 270610
add_steamlike_gog 270610 1469845437
add_steam 279260
add_steam 283180
add_steam 283880
add_steam 288930
add_steam 296850
add_steam 302690
add_steam 307570
add_steamlike_gog 307570 1430234218
add_steam 307580
add_steam 336130
add_steamlike_gog 336130 1454499527
add_steam 336140
add_steamlike_gog 336140 1637485057
add_steam 338420
add_steam 357490
add_steam 364350
add_steam 364390
add_steam 367110
add_steam 370910
add_steamlike_gog 370910 1460710709
add_steam 396090
add_steam 398450
add_steam 416250
add_steam 420180
add_steam 425600
add_steamlike_gog 425600 1444830704
add_steam 431850
add_steam 439310
add_steam 448440
add_steam 468530
add_steam 509920
add_steam 517910
add_steam 551840
add_steam 556060
add_steamlike_gog 556060 1745746005
add_steam 561770
add_steam 595560
add_steam 603870
add_steam 610900
add_steamlike_gog 610900 1489938431
add_steam 631570
add_steamlike_gog 631570 1845001352
add_steam 654550
add_steam 655250
add_steam 673850
add_steam 710600
add_steam 733300
add_steam 761460
add_steam 763030
add_steam 782280
add_steam 827830
add_steam 872750
add_steamlike_gog 872750 1455980545
add_steam 904600
add_steam 904750
add_steamlike_gog 904750 1664228813
add_steam 917380
add_steam 962700
add_steam 1026080
add_steam 1028740
add_steam 1036200
add_steam 1060560
add_steam 1098770
add_steamlike_gog 1098770 1602588462
add_steam 1133950
add_steamlike_gog 1133950 2098172050
add_steam 1142230
add_steam 1147030
add_steam 1172800
add_steamlike_gog 1172800 1321012195
add_steam 1181570
add_steam 1182310
add_steamlike_gog 1182310 1790032718
add_steam 1196520
add_steam 1220930
add_steam 1259140
add_steam 1251910
add_steam 1270590
add_steam 1285960
add_steam 1305500
add_steam 1343390
add_steam 1355910
add_steam 1369520
add_steamlike_gog 1369520 1392294208
add_steam 1497290
add_steam 1507530
add_steam 1581490
add_steamlike_gog 1581490 2134608058
add_steam 1606640
add_steam 1644080
add_steam 1650590
add_steam 1709730
add_steamlike_gog 1709730 1086301307
add_steam 1834750
add_steam 1900280
add_steam 1902850
add_steamlike_gog 1902850 1123332294
add_steam 2097090
add_steam 2118540
add_steam 2163620
add_steam 2217060
add_steam 2314850
add_steamlike_gog 2314850 1562711791
add_steam 2474030
add_steam 2581560
add_steamlike_gog 2581560 1855964876
add_steam 2661780
add_steam 2664930
add_steam 2702680

#TWINE games:
add_steam 397330

#WME games:
add_steam 257690
add_steamlike_gog 257690 1457085654
add_steam 270570
add_steam 281060
add_steam 281080
add_steam 286320
add_steam 286360
add_steam 378630
add_steam 291710
add_steam 337130
add_steam 340370
add_steam 349140
add_steam 405780
add_steam 574420
add_steam 1064660
add_steam 1603980
add_steam 1642970

#ASYLUM games:
add_steam 284050

#TWP games:
add_steam 569860

#TODO: check for 7zip, since it produces smaller files

touch --date="2000-01-01 00:00:00" gen/* static/*
if test "$VERBOSE" = "-v"; then
    zip -9jX achievements.dat gen/* static/*
else
    zip -9jX achievements.dat gen/* static/* >/dev/null
fi
mv $VERBOSE -f achievements.dat ../../dists/engine-data

git add gen/* ../../dists/engine-data/achievements.dat

echo SUCCESS
echo Please run:
echo "    git commit -m 'ACHIEVEMENTS: Generated description files'"

exit 0
