#!/bin/sh

set -e

trap "echo FAILURE: $0 failed to create achievements.dat" ERR

add_steam() {
    python steam_achievements.py -v --steamid "$1"
    echo -----------------------------------------
}

add_steamlike_gog() {
    python steam_achievements.py -v --steamid "$1" --saveasgalaxyid "$2"
    echo -----------------------------------------
}

rm -f gen/*

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

zip -9j achievements.dat gen/* static/*
mv -vf achievements.dat ../../dists/engine-data

echo SUCCESS
exit 0
