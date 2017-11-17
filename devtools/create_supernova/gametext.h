/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This is a utility for generating a data file for the supernova engine.
 * It contains strings extracted from the original executable as well
 * as translations and is required for the engine to work properly.
 */

#ifndef GAMETEXT_H
#define GAMETEXT_H

#include <stddef.h>

// This file contains the strings in German and is encoded using CP850 encoding.
// Other language should be provided as po files also using the CP850 encoding.

// TODO: add the strings from the engine here, add an Id string in comment.
// And in the engine add a StringId enum with all the Ids = index in this array.

const char *gameText[] = {
	// 0
	"Gehe",         // kStringCommandGo
	"Schau",        // kStringCommandLook
	"Nimm",         // kStringCommandTake
	"\231ffne",     // kStringCommandOpen
	"Schlie\341e",  // kStringCommandClose
	// 5
	"Dr\201cke",    // kStringCommandPress
	"Ziehe",        // kStringCommandPull
	"Benutze",      // kStringCommandUse
	"Rede",         // kStringCommandTalk
	"Gib",          // kStringCommandGive
	// 10
	"Gehe zu ",     // kStringStatusCommandGo
	"Schau ",       // kStringStatusCommandLook
	"Nimm ",        // kStringStatusCommandTake
	"\231ffne ",    // kStringStatusCommandOpen
	"Schlie\341e ", // kStringStatusCommandClose
	// 15
	"Dr\201cke ",   // kStringStatusCommandPress
	"Ziehe ",       // kStringStatusCommandPull
	"Benutze ",     // kStringStatusCommandUse
	"Rede mit ",    // kStringStatusCommandTalk
	"Gib ",         // kStringStatusCommandGive
	// 20
	"V2.02",            // kStringTitleVersion
	"Teil 1:",          // kStringTitle1
	"Das Schicksal",    // kStringTitle2
	"des Horst Hummel", // kStringTitle3
	"^(C) 1994 Thomas und Steffen Dingel#",  // kStringIntro1
	// 25
	"Story und Grafik:^ Thomas Dingel#",     // kStringIntro2
	"Programmierung:^ Steffen Dingel#",      // kStringIntro3
	"Musik:^ Bernd Hoffmann#",               // kStringIntro4
	"Getestet von ...#",                     // kStringIntro5
	"^Matthias Neef#",                       // kStringIntro6
	// 30
	"^Sascha Otterbach#",                    // kStringIntro7
	"^Thomas Mazzoni#",                      // kStringIntro8
	"^Matthias Klein#",                      // kStringIntro9
	"^Gerrit Rothmaier#",                    // kStringIntro10
	"^Thomas Hassler#",                      // kStringIntro11
	// 35
	"^Rene Koch#",                           // kStringIntro12
	"\233",                                  // kStringIntro13
	"Hmm, er scheint kaputt zu sein.",       // kStringBroken
	"Es ist nichts Besonderes daran.",       // kStringDefaultDescription
	"Das mu\341t du erst nehmen.",              // kStringTakeMessage
	// 40
	"Keycard",                               // kStringKeycard
	"Die Keycard f\224r deine Schr\204nke.", // kStringKeycardDescription
	"Taschenmesser",                         // kStringKnife
	"Es ist nicht mehr das sch\204rfste.",   // kStringKnifeDescription
	"Armbanduhr",                            // kStringWatch
	// 45
	"Discman",                               // kStringDiscman
	"Es ist eine \"Mad Monkeys\"-CD darin.", // kStringDiscmanDescription
	"Luke",                                  // kStringHatch
	"Knopf",                                 // kStringButton
	"Er geh\224rt zu der gro\341en Luke.",   // kStringHatchButtonDescription
	// 50
	"Leiter",                                // kStringLadder
	"Ausgang",                               // kStringExit
	"Sie f\201hrt ins Cockpit.",             // kStringCockpitHatchDescription
	"Sie f\201hrt zur K\201che.",            // kStringKitchenHatchDescription
	"Sie f\201hrt zu den Tiefschlafkammern.", // kStringStasisHatchDescription
	// 55
	"Dies ist eine der Tiefschlafkammern.",  // kStringStasisHatchDescription2
	"Schlitz",                               // kStringSlot
	"Es ist ein Keycard-Leser.",             // kStringSlotDescription
	"Gang",                                  // kStringCorridor
	"Computer",                              // kStringComputer
	// 60
	"ZWEIUNDVIERZIG",                        // kStringComputerPassword
	"Instrumente",                           // kStringInstruments
	"Hmm, sieht ziemlich kompliziert aus.",  // kStringInstrumentsDescription1
	"Monitor",                               // kStringMonitor
	"Dieser Monitor sagt dir nichts.",       // kStringMonitorDescription
	// 65
	"Bild",                                  // kStringImage
	"Herb!",                                 // kStringGenericDescription1
	"Toll!",                                 // kStringGenericDescription2
	"Genial!",                               // kStringGenericDescription3
	"Es scheint noch nicht fertig zu sein.", // kStringGenericDescription4
	// 70
	"Magnete",                               // kStringMagnete
	"Damit werden Sachen auf|dem Tisch festgehalten.", // kStringMagneteDescription
	"Stift",                                 // kStringPen
	"Ein Kugelschreiber.",                   // kStringPenDescription
	"Schrank",                               // kStringShelf
	// 75
	"Fach",                                  // kStringCompartment
	"Steckdose",                             // kStringSocket
	"Toilette",                              // kStringToilet
	"Pistole",                               // kStringPistol
	"Es ist keine Munition drin.",           // kStringPistolDescription
	// 80
	"B\201cher",                             // kStringBooks
	"Lauter wissenschaftliche B\201cher.",   // kStringBooksDescription
	"Kabelrolle",                            // kStringSpool
	"Da sind mindestens zwanzig Meter drauf.", // kStringSpoolDescription
	"Buch",                                  // kStringBook
	// 85
	"Unterw\204sche",                        // kStringUnderwear
	"Ich habe keine Lust, in|der Unterw\204sche des|Commanders rumzuw\201hlen.", // kStringUnderwearDescription
	"Kleider",                               // kStringClothes
	"Krimskram",                             // kStringJunk
	"Es ist nichts brauchbares dabei.",      // kStringJunkDescription
	// 90
	"Ordner",                                // kStringFolders
	"Darauf steht \"Dienstanweisungen|zur Mission Supernova\".|Es steht nichts wichtiges drin.", // kStringFoldersDescription
	"Poster",                                // kStringPoster
	"Ein Poster von \"Big Boss\".",          // kStringPosterDescription1
	"Ein Poster von \"Rock Desaster\".",     // kStringPosterDescription2
	// 95
	"Box",                                   // kStringSpeaker
	"Schallplatte",                          // kStringRecord
	"Die Platte ist von \"Big Boss\".",      // kStringRecordDescription
	"Schallplattenst\204nder",               // kStringRecordStand
	"Du hast jetzt keine Zeit, in|der Plattensammlung rumzust\224bern.", // kStringRecordStandDescription
	// 100
	"Plattenspieler",                        // kStringTurntable
	"Sieht aus, als k\204me|er aus dem Museum.", // kStringTurntableDescription
	"Leitung",                               // kStringWire
	"Stecker",                               // kStringPlug
	"Manche Leute haben schon|einen komischen Geschmack.", // kStringImageDescription1
	// 105
	"Zeichenger\204te",                      // kStringDrawingInstruments
	"Auf dem Zettel sind lauter|unverst\204ndliche Skizzen und Berechnungen.|(Jedenfalls f\201r dich unverst\204ndlich.)", // kStringDrawingInstrumentsDescription
	"Schachspiel",                           // kStringChessGame
	"Es macht wohl Spa\341, an|der Decke Schach zu spielen.", // kStringChessGameDescription1
	"Tennisschl\204ger",                     // kStringTennisRacket
	// 110
	"Fliegt Boris Becker auch mit?",         // kStringTennisRacketDescription
	"Tennisball",                            // kStringTennisBall
	"Dein Magnetschachspiel. Schach war|schon immer deine Leidenschaft.", // kStringChessGameDescription2
	"Bett",                                  // kStringBed
	"Das ist dein Bett. Toll, nicht wahr?",  // kStringBedDescription
	// 115
	"Das ist eins deiner drei F\204cher.",   // kStringCompartmentDescription
	"Alben",                                 // kStringAlbums
	"Deine Briefmarkensammlung.",            // kStringAlbumsDescription
	"Seil",                                  // kStringRope
	"Es ist ungef\204hr 10 m lang und 4 cm dick.", // kStringRopeDescription
	// 120
	"Das ist dein Schrank.",                 // kStringShelfDescription
	"Es sind Standard-Weltraum-Klamotten.",  // kStringClothesDescription
	"Str\201mpfe",                           // kStringSocks
	"Es ist|\"Per Anhalter durch die Galaxis\"|von Douglas Adams.", // kStringBookHitchhiker
	"Klo",                                   // kStringBathroom
	// 125
	"Ein Klo mit Saugmechanismus.",          // kStringBathroomDescription
	"Dusche",                                // kStringShower
	"Das ist eine Luke !!!",                 // kStringHatchDescription1
	"Dies ist eine Luke !!!",                // kStringHatchDescription2
	"Helm",                                  // kStringHelmet
	// 130
	"Es ist der Helm zum Raumanzug.",        // kStringHelmetDescription
	"Raumanzug",                             // kStringSuit
	"Der einzige Raumanzug, den die|anderen hiergelassen haben ...", // kStringSuitDescription
	"Versorgung",                            // kStringLifeSupport
	"Es ist der Versorgungsteil zum Raumanzug.", // kStringLifeSupportDescription
	// 135
	"Schrott",                               // kStringScrap
	"Da ist eine L\201sterklemme dran, die|noch ganz brauchbar aussieht.|Ich nehme sie mit.", // kStringScrapDescription1
	"L\201sterklemme",                       // kStringTerminalStrip
	"Junge, Junge! Die Explosion hat ein|ganz sch\224nes Durcheinander angerichtet.", // kStringScrapDescription2
	"Reaktor",                               // kStringReactor
	// 140
	"Das war einmal der Reaktor.",           // kStringReactorDescription
	"D\201se",                               // kStringNozzle
	"blauer K\201rbis",                      // kStringPumpkin
	"Keine Ahnung, was das ist.",            // kStringPumpkinDescription
	"Landef\204hre",                         // kStringLandingModule
	// 145
	"Sie war eigentlich f\201r Bodenuntersuchungen|auf Arsano 3 gedacht.", // kStringLandingModuleDescription
	"Sie f\201hrt nach drau\341en.",        // kStringHatchDescription3
	"Generator",                            // kStringGenerator
	"Er versorgt das Raumschiff mit Strom.", // kStringGeneratorDescription
	"Ein St\201ck Schrott.",                 // kStringScrapDescription3
	// 150
	"Es ist ein Sicherheitsknopf.|Er kann nur mit einem spitzen|Gegenstand gedr\201ckt werden.", // kSafetyButtonDescription
	"Tastatur",                              // kStringKeyboard
	"langes Kabel mit Stecker",              // kStringGeneratorWire
	"leere Kabelrolle",                      // kStringEmptySpool
	"Keycard des Commanders",                // kStringKeycard2
	// 155
	"Hey, das ist die Keycard des Commanders!|Er mu\341 sie bei dem \201berst\201rzten|Aufbruch verloren haben.", // kStringKeycard2Description
	"Klappe",                                // kStringTrap
	"Spannungmessger\204t",                  // kStringVoltmeter
	"Klemme",                                // kStringClip
	"Sie f\201hrt vom Generator zum Spannungmessger\204t.", // kStringWireDescription
	// 160
	"Stein",                                 // kStringStone
	"Loch",                                  // kStringCaveOpening
	"Es scheint eine H\224hle zu sein.",     // kStringCaveOpeningDescription
	"Hier bist du gerade hergekommen.",      // kStringExitDescription
	"H\224hle",                              // kStringCave
	// 165
	"Schild",                                // kStringSign
	"Diese Schrift kannst du nicht lesen.",  // kStringSignDescription
	"Eingang",                               // kStringEntrance
	"Stern",                                 // kStringStar
	"Raumschiff",                            // kStringSpaceshift
	// 170
	"Portier",                               // kStringPorter
	"Du siehst doch selbst, wie er aussieht.", // kStringPorterDescription
	"T\201r",                                // kStringDoor
	"Kaugummi",                              // kStringChewingGum
	"Gummib\204rchen",                       // kStringGummyBears
	// 175
	"Schokokugel",                           // kStringChocolateBall
	"\232berraschungsei",                    // kStringEgg
	"Lakritz",                               // kStringLiquorice
	"Tablette",                              // kStringPill
	"Die Plastikh\201lle zeigt einen|Mund mit einer Sprechblase. Was|darin steht, kannst du nicht lesen.", // kStringPillDescription
	// 180
	"Automat",                               // kStringVendingMachine
	"Sieht aus wie ein Kaugummiautomat.",    // kStringVendingMachineDescription
	"Die Toiletten sind denen|auf der Erde sehr \204hnlich.", // kStringToiletDescription
	"Treppe",                                // kStringStaircase
	"M\201nzen",                             // kStringCoins
	// 185
	"Es sind seltsame|K\224pfe darauf abgebildet.", // kStringCoinsDescription
	"Tablettenh\201lle",                     // kStringTabletPackage
	"Darauf steht:\"Wenn Sie diese|Schrift jetzt lesen k\224nnen,|hat die Tablette gewirkt.\"", // kStringTabletPackageDescription
	"Stuhl",                                 // kStringChair
	"Schuhe",                                // kStringShoes
	// 190
	"Wie ist der denn mit|Schuhen hier reingekommen?", // kStringShoesDescription
	"Froschgesicht",                         // kStringFrogFace
	"Gekritzel",                             // kStringScrible
	"\"Mr Spock was here\"",                 // kStringScribleDescription
	"Brieftasche",                           // kStringWallet
	// 195
	"Speisekarte",                           // kStringMenu
	"\"Heute empfehlen wir:|Fonua Opra mit Ulk.\"", // kStringMenuDescription
	"Tasse",                                 // kStringCup
	"Sie enth\204lt eine gr\201nliche Fl\201ssigkeit.", // kStringCupDescription
	"10-Buckazoid-Schein",                   // kStringBill
	// 200
	"Nicht gerade sehr viel Geld.",          // kStringBillDescription
	"Keycard von Roger",                     // kStringKeycard3
	"Anzeige",                               // kStringAnnouncement
	"Hmm, seltsame Anzeigen.",               // kStringAnnouncementDescription
	"Roger W.",                              // kStringRoger
	// 205
	"Ufo",                                   // kStringUfo
	"Der Eingang scheint offen zu sein.",    // kStringUfoDescription
	"Tablett",                               // kStringTray
	"Es ist irgendein Fra\341 und|etwas zu Trinken darauf.", // kStringTrayDescription
	"Stange",                                // kStringLamp
	// 210
	"Es scheint eine Lampe zu sein.",        // kStringLampDescription
	"Augen",                                 // kStringEyes
	"Es ist nur ein Bild.",                  // kStringEyesDescription
	"Sieht etwas anders aus als auf der Erde.", // kStringSocketDescription
	"Metallblock",                           // kStringMetalBlock
	// 215
	"Er ist ziemlich schwer.",               // kStringMetalBlockDescription
	"Roboter",                               // kStringRobot
	"Den hast du erledigt.",                 // kStringRobotDescription
	"Tisch",                                 // kStringTable
	"Ein kleiner Metalltisch.",              // kStringTableDescription
	// 220
	"Zellent\201r",                          // kStringCellDoor
	"Hier warst du eingesperrt.",            // kStringCellDoorDescription
	"Laptop",                                // kStringLaptop
	"Armbanduhr",                            // kStringWristwatch
	"S\204ule",                              // kStringPillar
	// 225
	"Auf einem Schild an der T\201r steht \"Dr. Alab Hansi\".", // kStringDoorDescription1
	"Auf einem Schild an der T\201r steht \"Saval Lun\".", // kStringDoorDescription2
	"Auf einem Schild an der T\201r steht \"Prof. Dr. Ugnul Tschabb\".", // kStringDoorDescription3
	"Auf einem Schild an der T\201r steht \"Alga Hurz Li\".", // kStringDoorDescription4
	"Diese T\201r w\201rde ich lieber|nicht \224ffnen. Nach dem Schild zu|urteilen, ist jemand in dem Raum.", // kStringDontEnter
	// 230
	"Axacussaner",                           // kStringAxacussan
	"Du m\201\341test ihn irgendwie ablenken.", // kStringAxacussanDescription
	"Komisches Bild.",                       // kStringImageDescription2
	"Karte",                                 // kStringMastercard
	"Darauf steht: \"Generalkarte\".",       // kStringMastercardDescription
	// 235
	"Lampe",                                 // kStringLamp2
	"Seltsam!",                              // kStringGenericDescription5
	"Geld",                                  // kStringMoney
	"Es sind 500 Xa.",                       // kStringMoneyDescription1
	"Schlie\341fach",                           // kStringLocker
	// 240
	"Es hat ein elektronisches Zahlenschlo\341.", // kStringLockerDescription
	"Brief",                                 // kStringLetter
	"W\201rfel",                             // kStringCube
	"Sonderbar!",                            // kStringGenericDescription6
	"Affenstark!",                           // kStringGenericDescription7
	// 245
	"Komisches Ding",                        // kStringStrangeThing
	"Wundersam",                             // kStringGenericDescription8
	"Es ist ein Axacussanerkopf auf dem Bild.", // kStringImageDescription3
	"Pflanze",                               // kStringPlant
	"Figur",                                 // kStringStatue
	// 250
	"Stark!",                                // kStringStatueDescription
	"Sie ist den Pflanzen auf der Erde sehr \204hnlich.", // kStringPlantDescription
	"Er funktioniert nicht.",                // kStringComputerDescription
	"Graffiti",                              // kStringGraffiti
	"Seltsamer B\201roschmuck!",             // kStringGraffitiDescription
	// 255
	"Es sind 350 Xa.",                       // kStringMoneyDescription2
	"Dschungel",                             // kStringJungle
	"Lauter B\204ume.",                      // kStringJungleDescription
	"^             E#N#D#E ...########",     // kStringOutro1
	"#       ... des ersten Teils!########", // kStringOutro2
	// 260
	"#########",                             // kStringOutro3
	"^Aber:#",                               // kStringOutro4
	"Das Abenteuer geht weiter, ...##",      // kStringOutro5
	"... wenn Sie sich f\201r 30,- DM registrieren lassen!##", // kStringOutro6
	"(Falls Sie das nicht schon l\204ngst getan haben.)##", // kStringOutro7
	// 265
	"In^ Teil 2 - Der Doppelg\204nger^ erwarten Sie:##", // kStringOutro8
	"Knifflige Puzzles,##",                  // kStringOutro9
	"noch mehr Grafik und Sound,##",         // kStringOutro10
	"ein perfekt geplanter Museumseinbruch,##", // kStringOutro11
	"das Virtual-Reality-Spiel \"Indiana Joe\"##", // kStringOutro12
	// 270
	"und vieles mehr!##",                     // kStringOutro13
	"\233",                                   // kStringOutro14
	"Leitung mit Stecker",                    // kStringWireAndPlug
	"Leitung mit L\201sterklemme",            // kStringWireAndClip
	"langes Kabel mit Stecker",               // kStringWireAndPlug2
	// 275
	"Darauf steht:|\"Treffpunkt Galactica\".", // kStringSignDescription2
	"M\201nze",                               // kStringCoin
	"Darauf steht:|\"Zutritt nur f\201r Personal\".", // kStringDoorDescription5
	"Darauf steht:|\"Toilette\".",            // kStringDoorDescription6
	"Es ist die Keycard des Commanders.",     // kStringKeycard2Description2
	// 280
	"Kabelrolle mit L\201sterklemme",         // kSringSpoolAndClip
	"Zwei Tage nach dem Start|im Cockpit der \"Supernova\" ...", // kStringIntroCutscene1
	"Entferung von der Sonne: 1 500 000 km.|Gehen Sie auf 8000 hpm, Captain!", // kStringIntroCutscene2
	"Ok, Sir.",                               // kStringIntroCutscene3
	"Geschwindigkeit:",                       // kStringIntroCutscene4
	// 285
	"Zweitausend hpm",                        // kStringIntroCutscene5
	"Dreitausend",                            // kStringIntroCutscene6
	"Viertausend",                            // kStringIntroCutscene7
	"F\201nftausend",                         // kStringIntroCutscene8
	"Sechstausend",                           // kStringIntroCutscene9
	// 290
	"Siebentausend",                          // kStringIntroCutscene10
	"Achttau...",                             // kStringIntroCutscene11
	"Was war das?",                           // kStringIntroCutscene12
	"Keine Ahnung, Sir.",                     // kStringIntroCutscene13
	"Ingenieur an Commander, bitte kommen!",  // kStringIntroCutscene14
	// 295
	"Was ist los?",                           // kStringIntroCutscene15
	"Wir haben einen Druckabfall im Hauptantriebssystem, Sir.|Einen Moment, ich schaue sofort nach, woran es liegt.", // kStringIntroCutscene16
	"Schei\341e, der Ionenantrieb ist explodiert!|Die Teile sind \201ber den ganzen|Maschinenraum verstreut.", // kStringIntroCutscene17
	"Ach, du meine G\201te!|Gibt es irgendeine M\224glichkeit,|den Schaden schnell zu beheben?", // kStringIntroCutscene18
	"Nein, Sir. Es sieht schlecht aus.",      // kStringIntroCutscene19
	// 300
	"Hmm, die Erde zu alarmieren, w\201rde zu lange dauern.", // kStringIntroCutscene20
	"Ich darf kein Risiko eingehen.|Captain, geben Sie sofort Alarm!", // kStringIntroCutscene21
	"Commander an alle! Achtung, Achtung!|Begeben Sie sich sofort zum Notraumschiff!", // kStringIntroCutscene22
	"Ich wiederhole:|Begeben Sie sich sofort zum Notraumschiff!", // kStringIntroCutscene23
	"Captain, bereiten Sie alles f\201r den Start vor!|Wir m\201ssen zur\201ck zur Erde!", // kStringIntroCutscene24
	// 305
	"Eine Stunde sp\204ter ...",              // kStringIntroCutscene25
	"Die Besatzung hat die \"Supernova\" verlassen.", // kStringIntroCutscene26
	"Das Schiff wird zwar in acht Jahren sein Ziel|erreichen, allerdings ohne Mannschaft.", // kStringIntroCutscene27
	"Das ist das kl\204gliche Ende|der Mission Supernova.", // kStringIntroCutscene28
	"Sie k\224nnen jetzt ihren Computer ausschalten.", // kStringIntroCutscene29
	// 310
	"Halt!",                                  // kStringIntroCutscene30
	"Warten Sie!",                            // kStringIntroCutscene31
	"Es regt sich etwas im Schiff.",          // kStringIntroCutscene32
	"Uuuuaaaahhhhh",                          // kStringIntroCutscene33
	"Huch, ich bin ja gefesselt!|Wo bin ich?", // kStringIntroCutscene34
	// 315
	"Ach so, das sind ja die Sicherheitsgurte.|Ich arbeite ja jetzt in diesem Raumschiff hier.", // kStringIntroCutscene35
	"Was? Schon zwei Uhr! Wieso|hat mich denn noch keiner|aus dem Bett geschmissen?", // kStringIntroCutscene36
	"Ich werde mal nachsehen.",               // kStringIntroCutscene37
	"Autsch!",                                // kStringIntroCutscene38
	"Schei\341etagenbett!",                   // kStringIntroCutscene39
	// 320
	"Erst mal den Lichtschalter finden.",     // kStringIntroCutscene40
	"Hmm, gar nicht so einfach|bei Schwerelosigkeit.", // kStringIntroCutscene41
	"Ah, hier ist er.",                       // kStringIntroCutscene42
	"In der K\201che warst du schon|oft genug, im Moment hast|du keinen Appetit.", // kStringShipHall1
	"Flugziel erreicht", // kStringShipSleepCabin1
	// 325
	"Energie ersch\224pft", // kStringShipSleepCabin2
	"Tiefschlafprozess abgebrochen", // kStringShipSleepCabin3
	"Schlafdauer in Tagen:", // kStringShipSleepCabin4
	"Bitte legen Sie sich in die angezeigte Schlafkammer.", // kStringShipSleepCabin5
	"Bitte Passwort eingeben:", // kStringShipSleepCabin6
	// 330
	"Schlafdauer in Tagen:", // kStringShipSleepCabin7
	"Bitte legen Sie sich in die angezeigte Schlafkammer.", // kStringShipSleepCabin8
	"Falsches Passwort", // kStringShipSleepCabin9
	"Es w\201rde wenig bringen,|sich in eine Schlafkammer zu legen,|die nicht eingeschaltet ist.", // kStringShipSleepCabin10
	"Dazu mu\341t du erst den Raumanzug ausziehen.", // kStringShipSleepCabin11
	// 335
	"Was war das?", // kStringShipSleepCabin12
	"Achtung", // kStringShipSleepCabin13
	"Du wachst mit brummendem Sch\204del auf|und merkst, da\341 du nur getr\204umt hast.", // kStringShipSleepCabin14
	"Beim Aufprall des Raumschiffs|mu\341t du mit dem Kopf aufgeschlagen|und bewu\341tlos geworden sein.", // kStringShipSleepCabin15
	"Was steht dir jetzt wohl wirklich bevor?", // kStringShipSleepCabin16
	// 340
	"Geschwindigkeit: ", // kStringShipCockpit1
	"8000 hpm", // kStringShipCockpit2
	"0 hpm", // kStringShipCockpit3
	"Ziel: Arsano 3", // kStringShipCockpit4
	"Entfernung: ", // kStringShipCockpit5
	//345
	" Lichtjahre", // kStringShipCockpit6
	"Dauer der Reise bei momentaner Geschwindigkeit:", // kStringShipCockpit7
	" Tage", // kStringShipCockpit8
	"Vergi\341 nicht, du bist nur der|Schiffskoch und hast keine Ahnung,|wie man ein Raumschiff fliegt.", // kStringShipCockpit9
	"Achtung: Triebwerke funktionsunf\204hig", // kStringShipCockpit10
	//350
	"Energievorrat ersch\224pft", // kStringShipCockpit11
	"Notstromversorgung aktiv", // kStringShipCockpit12
	"Was?! Keiner im Cockpit!|Die sind wohl verr\201ckt!", // kStringShipCockpit13
	"Du hast die Platte schon aufgelegt.", // kStringShipCabinL3_1
	"Es ist doch gar keine Platte aufgelegt.", // kStringShipCabinL3_2
	//355
	"Die Platte scheint einen Sprung zu haben.", // kStringShipCabinL3_3
	"Schneid doch besser ein|l\204ngeres St\201ck Kabel ab!", // kStringShipCabinL3_4
	"Das ist befestigt.", // kStringShipCabinL3_5
	"Zu niedriger Luftdruck soll ungesund sein.", // kStringShipAirlock1
	"Er zeigt Null an.", // kStringShipAirlock2
	//360
	"Er zeigt Normaldruck an.", // kStringShipAirlock3
	"Komisch, es ist nur|noch ein Raumanzug da.", // kStringShipAirlock4
	"Du mu\341t erst hingehen.", // kStringShipHold1
	"Das Kabel ist im Weg.", // kStringCable1
	"Das Kabel ist schon ganz|richtig an dieser Stelle.", // kStringCable2
	//365
	"Womit denn?", // kStringCable3
	"Die Leitung ist zu kurz.", // kStringCable4
	"Was ist denn das f\201r ein Chaos?|Und au\341erdem fehlt das Notraumschiff!|Jetzt wird mir einiges klar.|Die anderen sind gefl\201chtet,|und ich habe es verpennt.", // kStringShipHold2
	"Es ist nicht spitz genug.", // kStringShipHold3
	"Du wirst aus den Anzeigen nicht schlau.", // kStringShipHold4
	//370
	"La\341 lieber die Finger davon!", // kStringShipHold5
	"An dem Kabel ist doch gar kein Stecker.", // kStringShipHold6
	"Du solltest die Luke vielleicht erst \224ffnen.", // kStringShipHold7
	"Das Seil ist im Weg.", // kStringShipHold8
	"Das ist geschlossen.", // kStringShipHold9
	//375
	"Das geht nicht.|Die Luke ist mindestens|5 Meter \201ber dem Boden.", // kStringShipHold10
	"Was n\201tzt dir der Anschlu\341|ohne eine Stromquelle?!", // kStringShipHold11
	"Die Spannung ist auf Null abgesunken.", // kStringShipHold12
	"Es zeigt volle Spannung an.", // kStringShipHold13
	"Du mu\341t die Luke erst \224ffnen.", // kStringShipHold14
	//380
	"Das Seil ist hier schon ganz richtig.", // kStringShipHold15
	"Das Kabel ist zu kurz.", // kStringShipHold16
	"Die Raumschiffe sind alle verschlossen.", // kStringArsanoMeetup1
	"Unsinn!", // kStringArsanoMeetup2
	"Komisch! Auf einmal kannst du|das Schild lesen! Darauf steht:|\"Treffpunkt Galactica\".", // kStringArsanoMeetup3
	//385
	"Durch deinen Helm kannst|du nicht sprechen.", // kStringArsanoEntrance1
	"Wo soll ich die Schuhe ablegen?", // kStringArsanoEntrance2
	"Was, das wissen Sie nicht?", // kStringArsanoEntrance3
	"Sie befinden sich im Restaurant|\"Treffpunkt Galactica\".", // kStringArsanoEntrance4
	"Wir sind bei den interessantesten|Ereignissen in der Galaxis|immer zur Stelle.", // kStringArsanoEntrance5
	//390
	"Wenn Sie meinen.", // kStringArsanoEntrance6
	"In der Toilette gibt es|Schlie\341f\204cher f\201r Schuhe.", // kStringArsanoEntrance7
	"Wenn Sie das Lokal betreten|wollen, m\201ssen Sie erst|ihre Schuhe ausziehen.", // kStringArsanoEntrance8
	"Wollen Sie, da\341 ich Sie rau\341schmei\341e?", // kStringArsanoEntrance9
	"Hhius otgfh Dgfdrkjlh Fokj gf.", // kStringArsanoEntrance10
	//395
	"Halt!", // kStringArsanoEntrance11
	"Uhwdejkt!", // kStringArsanoEntrance12
	"Sie m\201ssen erst ihre Schuhe ausziehen, Sie Trottel!", // kStringArsanoEntrance13
	"Was f\204llt ihnen ein!|Sie k\224nnen doch ein Lokal|nicht mit Schuhen betreten!", // kStringArsanoEntrance14
	"Fragen Sie nicht so doof!", // kStringArsanoEntrance15
	// 400
	"Das w\201rde ich an ihrer|Stelle nicht versuchen!", // kStringArsanoEntrance16
	"Du ziehst deine Schuhe|aus und legst sie in|eins der Schlie\341f\204cher.", // kStringArsanoEntrance17
	"Du ziehst deine Schuhe wieder an.", // kStringArsanoEntrance18
	"Du durchsuchst die Klos nach|anderen brauchbaren Sachen,|findest aber nichts.", // kStringArsanoEntrance19
	"Bevor du aufs Klo gehst,|solltest du besser deinen|Raumanzug ausziehen.", // kStringArsanoEntrance20
	// 405
	"Du gehst seit sieben Jahren das|erste Mal wieder aufs Klo!", // kStringArsanoEntrance21
	"In einem der Schlie\341f\204cher,|die sich auch im Raum befinden,|findest du einige M\201nzen.", // kStringArsanoEntrance22
	"Mach doch zuerst das Fach leer!", // kStringArsanoEntrance23
	"Komisch! Auf einmal kannst du|das Schild lesen! Darauf steht:|\"Zutritt nur f\201r Personal\".", // kStringArsanoEntrance24
	"Komisch! Auf einmal kannst|du das Schild lesen!|Darauf steht:\"Toilette\".", // kStringArsanoEntrance25
	// 410
	"Du ziehst den Raumanzug wieder an.", // kStringArsanoEntrance26
	"Nicht so gewaltt\204tig!", // kStringArsanoEntrance27
	"Wo bin ich hier?", // kStringArsanoDialog1
	"Sch\224nes Wetter heute, nicht wahr?", // kStringArsanoDialog2
	"W\201rden Sie mich bitte durchlassen.", // kStringArsanoDialog3
	// 415
	"Hey Alter, la\341 mich durch!", // kStringArsanoDialog4
	"Was haben Sie gesagt?", // kStringArsanoDialog5
	"Sprechen Sie bitte etwas deutlicher!", // kStringArsanoDialog6
	"Wieso das denn nicht?", // kStringArsanoDialog7
	"Wo soll ich die Schuhe ablegen?", // kStringArsanoDialog8
	// 420
	"Schwachsinn! Ich gehe jetzt nach oben!", // kStringArsanoDialog9
	"|", // kStringDialogSeparator
	"K\224nnten Sie mir ein Gericht empfehlen?", // kStringDialogArsanoRoger1
	"Wie lange dauert es denn noch bis zur Supernova?", // kStringDialogArsanoRoger2
	"Sie kommen mir irgendwie bekannt vor.", // kStringDialogArsanoRoger3
	// 425
	"Was wollen Sie von mir?", // kStringDialogArsanoMeetup3_1
	"Hilfe!!", // kStringDialogArsanoMeetup3_2
	"Warum sprechen Sie meine Sprache?", // kStringDialogArsanoMeetup3_3
	"Ja, ich bin einverstanden.", // kStringDialogArsanoMeetup3_4
	"Nein, lieber bleibe ich hier, als mit Ihnen zu fliegen.", // kStringDialogArsanoMeetup3_5
	// 430
	"Darf ich hier Platz nehmen?", // kStringArsanoRoger1
	"Klar!", // kStringArsanoRoger2
	"Hey, Witzkeks, la\341 die Brieftasche da liegen!", // kStringArsanoRoger3
	"Das ist nicht deine.", // kStringArsanoRoger4
	"Roger ist im Moment nicht ansprechbar.", // kStringArsanoRoger5
	// 435
	"Bestellen Sie lieber nichts!", // kStringArsanoRoger6
	"Ich habe vor zwei Stunden mein Essen|bestellt und immer noch nichts bekommen.", // kStringArsanoRoger7
	"Noch mindestens zwei Stunden.", // kStringArsanoRoger8
	"Haben Sie keine Idee, womit wir uns|bis dahin die Zeit vertreiben k\224nnen?", // kStringArsanoRoger9
	"Hmm ... im Moment f\204llt mir nichts ein, aber vielleicht|hat der Spieler des Adventures ja eine Idee.", // kStringArsanoRoger10
	// 440
	"Nein, Sie m\201ssen sich irren.|Ich kenne Sie jedenfalls nicht.", // kStringArsanoRoger11
	"Aber ihre Kleidung habe ich irgendwo schon mal gesehen.", // kStringArsanoRoger12
	"Ja? Komisch.", // kStringArsanoRoger13
	"Jetzt wei\341 ich's. Sie sind Roger W. !", // kStringArsanoRoger14
	"Pssst, nicht so laut, sonst will|gleich jeder ein Autogramm von mir.", // kStringArsanoRoger15
	// 445
	"Ich habe extra eine Maske auf, damit|ich nicht von jedem angelabert werde.", // kStringArsanoRoger16
	"\216h ... ach so.", // kStringArsanoRoger17
	"Wann kommt denn das n\204chste SQ-Abenteuer raus?", // kStringArsanoRoger18
	"SQ 127 m\201\341te in einem Monat erscheinen.", // kStringArsanoRoger19
	"Was, Teil 127 ??", // kStringArsanoRoger20
	// 450
	"Bei uns ist gerade Teil 8 erschienen.", // kStringArsanoRoger21
	"Hmm ... von welchem Planeten sind Sie denn?", // kStringArsanoRoger22
	"Von der Erde.", // kStringArsanoRoger23
	"Erde? Nie geh\224rt.", // kStringArsanoRoger24
	"Wahrscheinlich irgendein Kaff, wo Neuerungen|erst hundert Jahre sp\204ter hingelangen.", // kStringArsanoRoger25
	// 455
	"\216h ... kann sein.", // kStringArsanoRoger26
	"Aber eins m\201ssen Sie mir erkl\204ren!", // kStringArsanoRoger27
	"Wieso sehen Sie mir so verdammt \204hnlich, wenn|Sie nicht von Xenon stammen, wie ich?", // kStringArsanoRoger28
	"Keine Ahnung. Bis jetzt dachte ich immer, Sie w\204ren ein|von Programmierern auf der Erde erfundenes Computersprite.", // kStringArsanoRoger29
	"Was? Lachhaft!", // kStringArsanoRoger30
	// 460
	"Wie erkl\204ren Sie sich dann,|da\341 ich ihnen gegen\201bersitze?", // kStringArsanoRoger31
	"Ja, das ist in der Tat seltsam.", // kStringArsanoRoger32
	"Halt, jetzt wei\341 ich es. Sie sind von der Konkurrenz,|von \"Georgefilm Games\" und wollen mich verunsichern.", // kStringArsanoRoger33
	"Nein, ich bin nur ein Ahnungsloser Koch von der Erde.", // kStringArsanoRoger34
	"Na gut, ich glaube Ihnen. Lassen wir jetzt|dieses Thema, langsam wird es mir zu bunt!", // kStringArsanoRoger35
	// 465
	"Eine Partie Schach! Das ist eine gute Idee.", // kStringArsanoRoger36
	"Schach? Was ist das denn?", // kStringArsanoRoger37
	"Schach ist ein interessantes Spiel.|Ich werde es Ihnen erkl\204ren.", // kStringArsanoRoger38
	"Knapp zwei Stunden sp\204ter ...", // kStringArsanoRoger39
	"Roger W. steht kurz vor dem Schachmatt|und gr\201belt nach einem Ausweg.", // kStringArsanoRoger40
	// 470
	"Du tippst auf den Tasten herum,|aber es passiert nichts.", // kStringArsanoGlider1
	"Alle Raumschiffe haben|den Planeten verlassen.", // kStringArsanoMeetup2_1
	"Alle Raumschiffe haben den Planeten|verlassen, bis auf eins ...", // kStringArsanoMeetup2_2
	"Was wollen Sie denn schon wieder?", // kStringArsanoMeetup2_3
	"Nein.", // kStringArsanoMeetup2_4
	// 475
	"Haben Sie zuf\204llig meine Brieftasche gesehen?|Ich mu\341 Sie irgendwo verloren haben.", // kStringArsanoMeetup2_5
	"Ohne die Brieftasche kann ich nicht|starten, weil meine Keycard darin ist.", // kStringArsanoMeetup2_6
	"Oh! Vielen Dank.", // kStringArsanoMeetup2_7
	"Wo ist denn Ihr Raumschiff?|Soll ich Sie ein St\201ck mitnehmen?", // kStringArsanoMeetup2_8
	"Wo wollen Sie denn hin?", // kStringArsanoMeetup2_9
	// 480
	"Ok, steigen Sie ein!", // kStringArsanoMeetup2_10
	"Wie Sie wollen.", // kStringArsanoMeetup2_11
	"Huch, du lebst ja noch!", // kStringArsanoMeetup2_12
	"Das w\201rde ich jetzt nicht tun, schlie\341lich|steht Roger W. neben seinem Schiff.", // kStringArsanoMeetup2_13
	"Ich glaube, er wacht auf.", // kStringArsanoMeetup3_1
	// 485
	"Ja, sieht so aus.", // kStringArsanoMeetup3_2
	"Sie befinden sich im Raumschiff \"Dexxa\".", // kStringArsanoMeetup3_3
	"Wir kommen vom Planeten Axacuss und|sind aus dem gleichen Grund hier wie Sie,|n\204mlich zur Erforschung der Supernova.", // kStringArsanoMeetup3_4
	"Sie k\224nnen beruhigt sein, wir wollen Ihnen nur helfen.", // kStringArsanoMeetup3_5
	"Und wieso hat der Typ im Raumanzug|eben auf mich geschossen?", // kStringArsanoMeetup3_6
	// 490
	"Das war eine Schreckreaktion.", // kStringArsanoMeetup3_7
	"Schlie\341lich ist es f\201r uns das erste Mal,|da\341 wir auf eine fremde Intelligenz treffen.", // kStringArsanoMeetup3_8
	"Wie wir festgestellt haben, ist|Ihr Raumschiff v\224llig zerst\224rt.", // kStringArsanoMeetup3_9
	"Wahrscheinlich k\224nnen Sie nicht|mehr auf ihren Heimatplaneten zur\201ck.", // kStringArsanoMeetup3_10
	"Wir bieten Ihnen an, Sie|mit nach Axacuss zu nehmen.", // kStringArsanoMeetup3_11
	// 495
	"Sind Sie sich da wirklich sicher?", // kStringArsanoMeetup3_12
	"Wenn ich es mir genau \201berlege,|fliege ich doch lieber mit.", // kStringArsanoMeetup3_13
	"Gut, wir nehmen Sie unter der|Bedingung mit, da\341 wir Sie jetzt|sofort in Tiefschlaf versetzen d\201rfen.", // kStringArsanoMeetup3_14
	"Diese Art des Reisens ist Ihnen|ja scheinbar nicht unbekannt.", // kStringArsanoMeetup3_15
	"Sie werden in vier Jahren nach der|Landung der \"Dexxa\" wieder aufgeweckt.", // kStringArsanoMeetup3_16
	// 500
	"Sind Sie damit einverstanden?", // kStringArsanoMeetup3_17
	"Gut, haben Sie noch irgendwelche Fragen?", // kStringArsanoMeetup3_18
	"Keine Panik!", // kStringArsanoMeetup3_19
	"Wir tun Ihnen nichts.", // kStringArsanoMeetup3_20
	"Wir sprechen nicht ihre Sprache,|sondern Sie sprechen unsere.", // kStringArsanoMeetup3_21
	// 505
	"Nach einer Gehirnanalyse konnten|wir Ihr Gehirn an unsere Sprache anpassen.", // kStringArsanoMeetup3_22
	"Was? Sie haben in mein Gehirn eingegriffen?", // kStringArsanoMeetup3_23
	"Keine Angst, wir haben sonst nichts ver\204ndert.", // kStringArsanoMeetup3_24
	"Ohne diesen Eingriff w\204ren|Sie verloren gewesen.", // kStringArsanoMeetup3_25
	"Ich habe keine weiteren Fragen mehr.", // kStringArsanoMeetup3_26
	// 510
	"Gut, dann versetzen wir Sie jetzt in Tiefschlaf.", // kStringArsanoMeetup3_27
	"Gute Nacht!", // kStringArsanoMeetup3_28
	"Du wachst auf und findest dich in|einem geschlossenen Raum wieder.", // kStringAxacussCell_1
	"Du dr\201ckst den Knopf,|aber nichts passiert.", // kStringAxacussCell_2
	"Das ist befestigt.", // kStringAxacussCell_3
	// 515
	"Bei deinem Fluchtversuch hat|dich der Roboter erschossen.", // kStringAxacussCell_4
	"Du i\341t etwas, aber|es schmeckt scheu\341lich.", // kStringAxacussCell_5
	"Ok.", // kStringOk
	"Ach, Ihnen geh\224rt die. Ich habe sie eben im Sand gefunden.", // kStringDialogArsanoMeetup2_1
	"Nein, tut mir leid.", // kStringDialogArsanoMeetup2_2
	// 520
	"Nein, danke. Ich bleibe lieber hier.", // kStringDialogArsanoMeetup2_3
	"Ja, das w\204re gut.", // kStringDialogArsanoMeetup2_4
	"Zur Erde.", // kStringDialogArsanoMeetup2_5
	"Zum Pr\204sident der Galaxis.", // kStringDialogArsanoMeetup2_6
	"Nach Xenon.", // kStringDialogArsanoMeetup2_7
	// 525
	"Mir egal, setzen Sie mich irgendwo ab!", // kStringDialogArsanoMeetup2_8
	"Ich habe gerade Ihre Brieftasche gefunden!", // kStringDialogArsanoMeetup2_9
	"Sie lag da dr\201ben hinter einem Felsen.", // kStringDialogArsanoMeetup2_10
	"Ich wollte nur wissen, ob Sie die Brieftasche wiederhaben.", // kStringDialogArsanoMeetup2_11
	"\216h ... nein, mein Name ist M\201ller.", // kStringDialogAxacussCorridor5_1
	// 530
	"Oh, ich habe mich im Gang vertan.", // kStringDialogAxacussCorridor5_2
	"W\201rden Sie mich bitte zum Fahrstuhl lassen?", // kStringDialogAxacussCorridor5_3
	"Ich gehe wieder.", // kStringDialogAxacussCorridor5_4
	"Dann gehe ich eben wieder.", // kStringDialogAxacussCorridor5_5
	"Ach, halten Sie's Maul, ich gehe trotzdem!", // kStringDialogAxacussCorridor5_6
	// 535
	"Wenn Sie mich durchlassen gebe ich Ihnen %d Xa.",  // kStringDialogAxacussCorridor5_7
	"Hallo!", // kStringDialogX1
	"Guten Tag!", // kStringDialogX2
	"Ich bin's, Horst Hummel.", // kStringDialogX3
	"Sie schon wieder?", // kStringAxacussCorridor5_1
	// 540
	"Halt! Sie sind doch dieser Hummel.|Bleiben Sie sofort stehen!", // kStringAxacussCorridor5_2
	"Sehr witzig!", // kStringAxacussCorridor5_3
	"Kann auch sein, auf jeden Fall|sind Sie der Nicht-Axacussaner.", // kStringAxacussCorridor5_4
	"Nein!", // kStringAxacussCorridor5_5
	"Das m\201\341te schon ein bi\341chen mehr sein.", // kStringAxacussCorridor5_6
	// 545
	"Ok, dann machen Sie da\341 Sie wegkommen!", // kStringAxacussCorridor5_7
	"Du stellst dich hinter die S\204ule.", // kStringAxacussBcorridor_1
	"Welche Zahlenkombination willst|du eingeben?", // kStringAxacussOffice1_1
	"Hmm, das haut nicht ganz hin,|aber irgendwie mu\341 die Zahl|mit dem Code zusammenh\204ngen.", // kStringAxacussOffice1_2
	"Das war die falsche Kombination.", // kStringAxacussOffice1_3
	// 550
	"Streng geheim", // kStringAxacussOffice1_4
	"418-98", // kStringAxacussOffice1_5
	"Sehr geehrter Dr. Hansi,", // kStringAxacussOffice1_6
	"Ich mu\341 Ihren Roboterexperten ein Lob aussprechen. Die", // kStringAxacussOffice1_7
	"Imitation von Horst Hummel ist perfekt gelungen, wie ich", // kStringAxacussOffice1_8
	// 555
	"heute bei der \232bertragung des Interviews feststellen", // kStringAxacussOffice1_9
	"konnte. Dem Aufschwung Ihrer Firma durch die Werbe-", // kStringAxacussOffice1_10
	"kampagne mit dem falschen Horst Hummel d\201rfte ja jetzt", // kStringAxacussOffice1_11
	"nichts mehr im Wege stehen.", // kStringAxacussOffice1_12
	"PS: Herzlichen zum Geburtstag!", // kStringAxacussOffice1_13
	// 560
	"Hochachtungsvoll", // kStringAxacussOffice1_14
	"Commander Sumoti", // kStringAxacussOffice1_15
	"Nicht zu fassen!", // kStringAxacussOffice1_16
	"Hey, hinter dem Bild ist Geld|versteckt. Ich nehme es mit.", // kStringAxacussOffice3_1
	"Jetzt verschwinden Sie endlich!", // kStringAxacussElevator_1
	// 565
	"Huch, ich habe mich vertan.", // kStringAxacussElevator_2
	"Nachdem du zwei Stunden im|Dschungel herumgeirrt bist,|findest du ein Geb\204ude.", // kStringAxacussElevator_3
	"Du h\204ttest besser vorher|den Stecker rausgezogen.", // kStringShock
	"Der Axacussaner hat dich erwischt.", // kStringShot
	"Das ist schon geschlossen.", // kStringCloseLocker_1
	// 570
	"Irgendwie ist ein Raumhelm|beim Essen unpraktisch.", // kStringIsHelmetOff_1
	"Schmeckt ganz gut.", // kStringGenericInteract_1
	"Da war irgendetwas drin,|aber jetzt hast du es|mit runtergeschluckt.", // kStringGenericInteract_2
	"Du hast es doch schon ge\224ffnet.", // kStringGenericInteract_3
	"In dem Ei ist eine Tablette|in einer Plastikh\201lle.", // kStringGenericInteract_4
	// 575
	"Du i\341t die Tablette und merkst,|da\341 sich irgendetwas ver\204ndert hat.", // kStringGenericInteract_5
	"Komisch! Auf einmal kannst du die Schrift lesen!|Darauf steht:\"Wenn Sie diese Schrift jetzt|lesen k\224nnen, hat die Tablette gewirkt.\"", // kStringGenericInteract_6
	"Das mu\341t du erst nehmen.", // kStringGenericInteract_7
	"Sie ist leer.", // kStringGenericInteract_8
	"Du findest 10 Buckazoids und eine Keycard.", // kStringGenericInteract_9
	// 580
	"Es ist eine Art elektronische Zeitung.", // kStringGenericInteract_10
	"Halt, hier ist ein interessanter Artikel.", // kStringGenericInteract_11
	"Hmm, irgendwie komme|ich mir verarscht vor.", // kStringGenericInteract_12
	" an ", // kPhrasalVerbParticleGiveTo
	" mit ", // kPhrasalVerbParticleUseWith
	// 585
	"Es ist eine Uhr mit extra|lautem Wecker. Sie hat einen|Knopf zum Verstellen der Alarmzeit.|Uhrzeit: %s   Alarmzeit: %s", // kStringGenericInteract_13
	"Neue Alarmzeit (hh:mm) :", // kStringGenericInteract_14
	"Die Luft hier ist atembar,|du ziehst den Anzug aus.", // kStringGenericInteract_15
	"Hier drinnen brauchtst du deinen Anzug nicht.", // kStringGenericInteract_16
	"Du mu\341t erst den Helm abnehmen.", // kStringGenericInteract_17
	// 590
	"Du mu\341t erst den Versorgungsteil abnehmen.", // kStringGenericInteract_18
	"Du ziehst den Raumanzug aus.", // kStringGenericInteract_19
	"Du ziehst den Raumanzug an.", // kStringGenericInteract_20
	"Die Luft hier ist atembar,|du ziehst den Anzug aus.", // kStringGenericInteract_21
	"Hier drinnen brauchtst du deinen Anzug nicht.", // kStringGenericInteract_22
	// 595
	"Den Helm h\204ttest du|besser angelassen!", // kStringGenericInteract_23
	"Du ziehst den Helm ab.", // kStringGenericInteract_24
	"Du ziehst den Helm auf.", // kStringGenericInteract_25
	"Du mu\341t erst den Anzug anziehen.", // kStringGenericInteract_26
	"Den Versorgungsteil h\204ttest du|besser nicht abgenommen!", // kStringGenericInteract_27
	// 600
	"Du nimmst den Versorgungsteil ab.", // kStringGenericInteract_28
	"Du ziehst den Versorgungsteil an.", // kStringGenericInteract_29
	"Die Leitung ist hier unn\201tz.", // kStringGenericInteract_30
	"Stark, das ist ja die Fortsetzung zum \"Anhalter\":|\"Das Restaurant am Ende des Universums\".", // kStringGenericInteract_31
	"Moment mal, es ist ein Lesezeichen drin,|auf dem \"Zweiundvierzig\" steht.", // kStringGenericInteract_32
	// 605
	"Das tr\204gst du doch bei dir.", // kStringGenericInteract_33
	"Du bist doch schon da.", // kStringGenericInteract_34
	"Das hast du doch schon.", // kStringGenericInteract_35
	"Das brauchst du nicht.", // kStringGenericInteract_36
	"Das kannst du nicht nehmen.", // kStringGenericInteract_37
	// 610
	"Das l\204\341t sich nicht \224ffnen.", // kStringGenericInteract_38
	"Das ist schon offen.", // kStringGenericInteract_39
	"Das ist verschlossen.", // kStringGenericInteract_40
	"Das l\204\341t sich nicht schlie\341en.", // kStringGenericInteract_41
	"Behalt es lieber!", // kStringGenericInteract_42
	// 615
	"Das geht nicht.", // kStringGenericInteract_43
	"Gespr\204ch beenden", // kStringConversationEnd
	"Du hast das komische Gef\201hl,|da\341 drau\341en etwas passiert,|und eilst zum Restaurant.", // kStringSupernova1
	"Da! Die Supernova!", // kStringSupernova2
	"Zwei Minuten sp\204ter ...", // kStringSupernova3
	// 620
	"Hey, was machen Sie in meinem Raumschiff?!", // kStringSupernova4
	"Geben Sie mir sofort meine Brieftasche wieder!", // kStringSupernova5
	"Versuchen Sie das ja nicht nochmal!", // kStringSupernova6
	"Und jetzt raus mit Ihnen!", // kStringSupernova7
	"Zehn Minuten sp\204ter ...", // kStringSupernova8
	//625
	"Textgeschwindigkeit:", // kStringTextSpeed
	"Was war das f\201r ein Ger\204usch?", // kStringGuardNoticed1
	"Ich werde mal nachsehen.", // kStringGuardNoticed2
	NULL
};



#endif // GAMETEXT_H
