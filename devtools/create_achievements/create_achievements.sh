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
        python steam_achievements.py ${VERBOSE:+"-v"} --steamid "$1"
        echo -----------------------------------------
    fi
}

add_steamlike_gog() {
    if ! test -f "gen/galaxy-$2.ini"; then
        python steam_achievements.py ${VERBOSE:+"-v"} --steamid "$1" --saveasgalaxyid "$2"
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
add_steam 80310
add_steam 80330
add_steam 80340
add_steam 80350
add_steam 80360
add_steam 212050
add_steam 227000
add_steam 236930
add_steam 252370
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
add_steam 307580
add_steam 336130
add_steam 336140
add_steam 338420
add_steam 357490
add_steam 364350
add_steam 364390
add_steam 367110
add_steam 370910
add_steamlike_gog 370910 1460710709
add_steam 396090
add_steam 420180
add_steam 425600
add_steamlike_gog 425600 1444830704
add_steam 439310
add_steam 468530
add_steam 509920
add_steam 551840
add_steam 556060
add_steamlike_gog 556060 1745746005
add_steam 561770
add_steam 595560
add_steam 603870
add_steam 610900
add_steam 631570
add_steamlike_gog 631570 1845001352
add_steam 654550
add_steam 673850
add_steam 761460
add_steam 782280
add_steam 872750
add_steamlike_gog 872750 1455980545
add_steam 904750
add_steam 1028740
add_steam 1133950
add_steam 1142230
add_steam 1147030
add_steam 1181570
add_steam 1220930
add_steam 1251910
add_steam 1270590

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

#ASYLUM games:
add_steam 284050

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
