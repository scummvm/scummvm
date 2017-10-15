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
	"Sie f\204hrt ins Cockpit.",             // kStringCockpitHatchDescription
	"Sie f\204hrt zur K\201che.",            // kStringKitchenHatchDescription
	"Sie f\204hrt zu den Tiefschlafkammern.", // kStringStasisHatchDescription
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
	"Geschwindigkeit: ",
	"8000 hpm",
	"0 hpm",
	"Ziel: Arsano 3",
	"Entfernung: ",
	//345
	" Lichtjahre",
	"Dauer der Reise bei momentaner Geschwindigkeit:",
	" Tage",
	"Vergi\341 nicht, du bist nur der|Schiffskoch und hast keine Ahnung,|wie man ein Raumschiff fliegt.",
	"Achtung: Triebwerke funktionsunf\204hig",
	//350
	"Energievorrat ersch\224pft",
	"Notstromversorgung aktiv",
	"Was?! Keiner im Cockpit!|Die sind wohl verr\201ckt!",
	"Du hast die Platte schon aufgelegt.",
	"Es ist doch gar keine Platte aufgelegt.",
	//355
	"Die Platte scheint einen Sprung zu haben.",
	"Schneid doch besser ein|l\204ngeres St\201ck Kabel ab!",
	"Das ist befestigt.",
	"Zu niedriger Luftdruck soll ungesund sein.",
	"Er zeigt Null an.",
	//360
	"Er zeigt Normaldruck an.",
	"Komisch, es ist nur|noch ein Raumanzug da.",
	"Du mu\341t erst hingehen.",
	"Das Kabel ist im Weg.",
	"Das Kabel ist schon ganz|richtig an dieser Stelle.",
	//365
	"Womit denn?",
	"Die Leitung ist zu kurz.",
	"Was ist denn das f\201r ein Chaos?|Und au\341erdem fehlt das Notraumschiff!|Jetzt wird mir einiges klar.|Die anderen sind gefl\201chtet,|und ich habe es verpennt.",
	"Es ist nicht spitz genug.",
	"Du wirst aus den Anzeigen nicht schlau.",
	//370
	"La\341 lieber die Finger davon!",
	"An dem Kabel ist doch gar kein Stecker.",
	"Du solltest die Luke vielleicht erst \224ffnen.",
	"Das Seil ist im Weg.",
	"Das ist geschlossen.",
	//375
	"Das geht nicht.|Die Luke ist mindestens|5 Meter \201ber dem Boden.",
	"Was n\201tzt dir der Anschlu\341|ohne eine Stromquelle?!",
	"Die Spannung ist auf Null abgesunken.",
	"Es zeigt volle Spannung an.",
	"Du mu\341t die Luke erst \224ffnen.",
	//380
	"Das Seil ist hier schon ganz richtig.",
	"Das Kabel ist zu kurz.",
	"",
	"",
	"",
	//385
	NULL
};



#endif // GAMETEXT_H
