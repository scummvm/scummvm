#!/bin/sh

set -e

trap "echo FAILURE: $0 failed to create achievements.dat" ERR

#AGS games:
python steam_achievements.py -v --steamid 80310
python steam_achievements.py -v --steamid 80330
python steam_achievements.py -v --steamid 80340
python steam_achievements.py -v --steamid 80350
python steam_achievements.py -v --steamid 80360
python steam_achievements.py -v --steamid 212050
python steam_achievements.py -v --steamid 227000
python steam_achievements.py -v --steamid 236930
python steam_achievements.py -v --steamid 252370
python steam_achievements.py -v --steamid 253110
python steam_achievements.py -v --steamid 264560
python steam_achievements.py -v --steamid 270610
python steam_achievements.py -v --steamid 270610 --saveasgalaxyid 1469845437
python steam_achievements.py -v --steamid 279260
python steam_achievements.py -v --steamid 283180
python steam_achievements.py -v --steamid 283880
python steam_achievements.py -v --steamid 288930
python steam_achievements.py -v --steamid 296850
python steam_achievements.py -v --steamid 302690
python steam_achievements.py -v --steamid 307570
python steam_achievements.py -v --steamid 307580
python steam_achievements.py -v --steamid 336130
python steam_achievements.py -v --steamid 336140
python steam_achievements.py -v --steamid 338420
python steam_achievements.py -v --steamid 357490
python steam_achievements.py -v --steamid 364350
python steam_achievements.py -v --steamid 364390
python steam_achievements.py -v --steamid 367110
python steam_achievements.py -v --steamid 370910
python steam_achievements.py -v --steamid 370910 --saveasgalaxyid 1460710709
python steam_achievements.py -v --steamid 396090
python steam_achievements.py -v --steamid 420180
python steam_achievements.py -v --steamid 425600
python steam_achievements.py -v --steamid 425600 --saveasgalaxyid 1444830704
python steam_achievements.py -v --steamid 439310
python steam_achievements.py -v --steamid 468530
python steam_achievements.py -v --steamid 509920
python steam_achievements.py -v --steamid 551840
python steam_achievements.py -v --steamid 556060
python steam_achievements.py -v --steamid 556060 --saveasgalaxyid 1745746005
python steam_achievements.py -v --steamid 561770
python steam_achievements.py -v --steamid 595560
python steam_achievements.py -v --steamid 603870
python steam_achievements.py -v --steamid 610900
python steam_achievements.py -v --steamid 631570
python steam_achievements.py -v --steamid 631570 --saveasgalaxyid 1845001352
python steam_achievements.py -v --steamid 654550
python steam_achievements.py -v --steamid 673850
python steam_achievements.py -v --steamid 761460
python steam_achievements.py -v --steamid 782280
python steam_achievements.py -v --steamid 872750
python steam_achievements.py -v --steamid 872750 --saveasgalaxyid 1455980545
python steam_achievements.py -v --steamid 904750
python steam_achievements.py -v --steamid 1028740
python steam_achievements.py -v --steamid 1133950
python steam_achievements.py -v --steamid 1142230
python steam_achievements.py -v --steamid 1147030
python steam_achievements.py -v --steamid 1181570
python steam_achievements.py -v --steamid 1220930
python steam_achievements.py -v --steamid 1251910
python steam_achievements.py -v --steamid 1270590

#TWINE games:
python steam_achievements.py -v --steamid 397330

#WME games:
python steam_achievements.py -v --steamid 257690
python steam_achievements.py -v --steamid 257690 --saveasgalaxyid 1457085654
python steam_achievements.py -v --steamid 270570
python steam_achievements.py -v --steamid 281060
python steam_achievements.py -v --steamid 281080
python steam_achievements.py -v --steamid 286320
python steam_achievements.py -v --steamid 286360
python steam_achievements.py -v --steamid 378630
python steam_achievements.py -v --steamid 291710
python steam_achievements.py -v --steamid 337130
python steam_achievements.py -v --steamid 340370
python steam_achievements.py -v --steamid 349140
python steam_achievements.py -v --steamid 405780
python steam_achievements.py -v --steamid 574420
python steam_achievements.py -v --steamid 1064660

zip -9j achievements.dat gen/* static/*
mv -vf achievements.dat ../../dists/engine-data

echo SUCCESS
exit 0
