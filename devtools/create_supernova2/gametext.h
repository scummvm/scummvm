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
	"Gehe",    //Go
	"Schau",    //Look
	"Nimm",    //Take
	"\231ffne",    //Open
	"Schlie\341e",    //Close
	// 5
	"Dr\201cke",    //Push
	"Ziehe",    //Pull
	"Benutze",    //Use
	"Rede",    //Talk
	"Gib",    //Give
	// 10
	"Gehe zu ",    //Go to 
	"Schau ",    //Look at 
	"Nimm ",    //Take 
	"\231ffne ",    //Open 
	"Schlie\341e ",    //Close 
	// 15
	"Dr\201cke ",    //Push 
	"Ziehe ",    //Pull 
	"Benutze ",    //Use 
	"Rede mit ",    //Talk to 
	"Gib ",    //Give 
	// 20
	" an ",    // to 
	" mit ",    // with 
	"Es ist nichts Besonderes daran.",    //There's nothing special about it.
	"|", //Dialog separator
	"Gespr\204ch beenden",    //End of conversation
	// 25
	"   F1 Hilfe",  // kStringHelpOverview1
	"   F2 Anleitung",  // kStringHelpOverview2
	"   F3 Programminformationen",  //kStringHelpOverview3
	"   F4 Textgeschwindigkeit",    //kStringHelpOverview4
	"   F5 Laden/Speichern",  // kStringHelpOverview5
	// 30
	"  ESC Vorspann \201berspringen",   // kStringHelpOverview6
	"Alt X Spiel beenden",     // kStringHelpOverview7
	"Textgeschwindigkeit:",    //Text speed:
	"Spiel abbrechen?",    //Leave game?
	"Ja",    //Yes
	// 35
	"Nein",    //No
	"Laden",    //Load
	"Speichern",    //Save
	"Zur\201ck",    //Back
	"Neustart",    //Restart
	// 40
	"Schreibfehler",    //write error
	"Das tr\204gst du doch bei dir.",    //You already carry this.
	"Du bist doch schon da.",    //You are already there.
	"Das ist geschlossen.",    //This is closed.
	"Das hast du doch schon.",    //You already have that.
	// 45
	"Das brauchst du nicht.",    //You don't need that.
	"Das kannst du nicht nehmen.",    //You can't take that.
	"Das l\204\341t sich nicht \224ffnen.",    //This cannot be opened.
	"Das ist schon offen.",    //This is already opened.
	"Das ist verschlossen.",    //This is locked.
	// 50
	"Das l\204\341t sich nicht schlie\341en.",    //This cannot be closed.
	"Das ist schon geschlossen.",    //This is already closed.
	"Behalt es lieber!",    //Better keep it!
	"Das geht nicht.",    //You can't do that.
	"^(C) 1994 Thomas und Steffen Dingel#",    //^(C) 1994 Thomas and Steffen Dingel#
	// 55
	"Story und Grafik:^ Thomas Dingel#",    //Story and Graphics:^ Thomas Dingel#
	"Programmierung:^ Steffen Dingel#",    //Programming:^ Steffen Dingel#
	"Musik:^ Bernd Hoffmann#",    //Music:^ Bernd Hoffmann#
	"Getestet von ...#",    //Tested by ...#
	"^Das war's.#",    //^That's it.#
	// 60
	"^Schlu\341!#",    //^Over!#
	"^Ende!#",    //^End!#
	"^Aus!#",    //^Done!#
	"^Tsch\201\341!#",    //^Bye!#
	"Oh!",    //Oh!
	// 65
	"Nicht schlecht!",    //Not bad!
	"Supersound!",    //Supersound!
	"Klasse!",    //Great!
	"Nicht zu fassen!",    //I can't believe it!
	"Super, ey!",    //Dope, yo!
	// 70
	"Fantastisch!",    //Fantastic!
	"Umwerfend!",    //Stunning!
	"Genial!",    //Brilliant!
	"Spitze!",    //Awesome!
	"Jawoll!",    //Alright!
	// 75
	"Hervorragend!",    //Outstanding!
	"Ultragut!",    //Ultra-good!
	"Megacool!",    //Mega cool!
	"Yeah!",    //Yeah!
	"Ein W\204chter betritt den Raum.|Du wirst verhaftet.",    //A guard enters the room.|You are getting arrested.
	// 80
	"Die n\204chsten paar Jahre|verbringst du im Knast.",    //You will spend the next|few years in jail.
	"Es wird Alarm ausgel\224st.",    //The alarm is about to be set off.
	"Du h\224rst Schritte.",    //You are hearing footsteps.
	"Um das Schloss zu \224ffnen,|brauchst du einige Zeit.",    //You will take some time,|to pick that lock.
	"Du ger\204tst in Panik|und ziehst die Keycard|aus der T\201r.",    //You are panicking|and remove the keycard|from the door.
	// 85
	"Du hast deinen Auftrag|noch nicht ausgef\201hrt.",    //You have not completed|your task yet.
	"Obwohl du die Alarmanlage noch|nicht ausgeschaltet hast,|entscheidest du dich, zu fliehen.",    //Although you haven't|disabled the alarm yet,|you decide to escape.
	"Du entledigst dich der Einbruchswerkzeuge|und nimmst ein Taxi zum Kulturpalast.",    //You get rid of your burglar tools|and take a cab to the Palace of Culture.
	"Diese T\201r brauchst|du nicht zu \224ffnen.",    //You don't need|to open this door.
	"Uff, es hat geklappt!",    //Phew, it worked!
	// 90
	"Zur\201ck im Quartier der Gangster ...",    //Back in the gangsters' hideout ...
	"Das lief ja wie am Schn\201rchen!",    //Everything went like clockwork!
	"Hier, dein Anteil von 30000 Xa.",    //Here, your share of 30000 Xa.
	"Wo ist denn der Saurierkopf?",    //Where's the dinosaur skull?
	"Dazu hatte ich keine Zeit mehr.",    //I didn't have enough time for that.
	// 95
	"Was? Du spinnst wohl!|Dann kriegst du auch deinen|Anteil nicht. Raus!",    //What? You're nuts!|Then you won't get your|share. Beat it!
	"Der Sauger ist schon dort.",    //The suction cup is already there.
	"Du heftest den Sauger an die Wand|und h\204lst dich daran fest.",    //You attach the suction cup to the wall|and hold on to it.
	"Du stellst dich auf den|Boden nimmst den Sauger|wieder von der Wand",    //You stand on the floor|then remove the suction cup from the wall
	"Die Alarmanlage ist|schon ausgeschaltet.",    //The alarm system is|already switched off.
	// 100
	"Um die Anlage abzuschalten,|brauchst du einige Zeit.",    //To turn off the system,|you need some time.
	"Die Alarmanlage ist jetzt ausgeschaltet.",    //The alarm system is now switched off.
	"Saurier",    //Dinosaur
	"Du hast jetzt besseres zu tun,|als das Ding anzuschauen.",    //You have better things to do now|than look at that thing.
	"Eingang",    //Entrance
	// 105
	"T\201r",    //Door
	"Strasse zum Stadtzentrum",    //Road to the city center
	"Kamera",    //Security camera
	"Hoffentlich bemerkt dich niemand.",    //Hopefully nobody will notice you.
	"Haupteingang",    //Main entrance
	// 110
	"Gang",    //Corridor
	"Ziemlich gro\341.",    //Quite large.
	"Saurierkopf",    //Dinosaur head
	"Dies ist der Kopf,|den du suchst.",    //This is the head|you're looking for.
	"Alarmanlage",    //Alarm system
	// 115
	"Sauger",    //Suction cup
	"Wand",    //Wall
	"Loch",    //Opening
	"Buchstabe",    //Letter
	"Sie ist sehr massiv.",    //It is very massive.
	// 120
	"Hmm, X und Y, irgendwo|habe ich die Buchstaben|schon gesehen.",    //Hmm, X and Y|I have seen these letters|somewhere before.
	"Deine Zeit ist um, Fremder!",    //Your Time is up, Stranger!
	"Du hast das Seil|doch schon festgebunden.",    //You already tied the rope.
	"Das w\201rde wenig bringen.",    //That would have little effect.
	"Sonnenstich, oder was?",    //Sunstroke, or what?
	// 125
	"Du merkst, da\341 der Boden|unter dir nachgibt, und|springst zur Seite.",    //You notice that the ground|is giving way under you,|and you leap aside.
	"Puzzleteil",    //Puzzle piece
	"Neben diesem Stein|ist kein freies Feld.",    //There's no free square|next to this stone.
	"Du spielst gerade ein|Adventure, kein Rollenspiel!",    //You are currently playing an|Adventure, not a Role-Playing Game!
	"Du kannst das Seil|nirgends befestigen.",    //There's nowhere|to attach the rope.
	// 130
	"Es pa\341t nicht|zwischen die Steine.",    //It does not fit|between the stones.
	"Das ist doch|oben festgebunden!",    //That is already|tied up above!
	"Hey, das ist|mindestens 10 Meter tief!",    //Hey, that is|at least 10 meters deep!
	"In dem Schlitz|ist nichts mehr.",    //There is nothing|left in the slot.
	"Das ist mindestens 5 Meter tief!",    //That is at least 5 meters deep!
	// 135
	"Du versuchst, den Sarg zu|\224ffnen, aber der Deckel bewegt|sich keinen Millimeter.",    //You try to open the coffin,|but the lid does not|move a millimeter.
	"Du hast die Kugel schon gedr\201ckt.",    //You have already|pushed the ball.
	"Die Kugel bewegt sich ein St\201ck.",    //The ball moves a bit.
	"Herzlichen Gl\201ckwunsch!",    //Congratulations!
	"Sie haben das Spiel gel\224st|und gewinnen 400 Xa!",    //You solved the game|and won 400 Xa!
	// 140
	"Vielen Dank f\201r die Benutzung eines|VIRTUAL-REALITY-SYSTEMS-Produkts!",    //Thank you for using a|VIRTUAL-REALITY-SYSTEMS product!
	"N",    //N
	"O",    //E
	"S",    //S
	"W",    //W
	// 145
	"Seil",    //Rope
	"Schild",    //Sign
	"Darauf steht:|\"Willst du finden das|richtige Loch, so wage|dich in die Pyramide!\".",    //It reads:|"Want to find|the right hole? Then dare|to enter the pyramid!".
	"Es ist eine kleine \231ffnung.",    //It is a small opening.
	"Pyramide",    //Pyramid
	// 150
	"Komisch! Was soll eine Pyramide|bei den Axacussanern? Deine|eigenen Gedanken scheinen|den Spielverlauf zu beeinflussen.",    //Weird! What is a pyramid doing|at the Axacussians? Your own thoughts seem to influence|the course of the game.
	"Sonne",    //Sun
	"Sch\224n!",    //Nice!
	"\"Hallo Fremder, wenn du diesen|Raum betreten hast, bleibt|dir nur noch eine Stunde Zeit,|um deine Aufgabe zu erf\201llen!\"",    //"Hello, Stranger, when you enter|this room, you have only an hour|to accomplish your task!"
	"rechte Seite",    //right side
	// 155
	"linke Seite",    //left side
	"Knopf",    //Button
	"Schrift",    //Inscription
	"Tomate",    //Tomato
	"Komisch!",    //Funny!
	// 160
	"Messer",    //Knife
	"Es ist ein relativ stabiles Messer.",    //It is a relatively sturdy knife.
	"Monster",    //Monster
	"Es ist dick und|ungef\204hr 15 Meter lang.",    //It is thick and|about 15 meters long.
	"Augen",    //Eyes
	// 165
	"Mund",    //Mouth
	"Es ist nur eine Statue.",    //It's just a statue.
	"Zettel",    //Note
	"Darauf steht:|\"Wenn du fast am Ziel|bist, tu folgendes:|Sauf!\"",    //It reads:|"When you're almost there,|do the following:|Drink!"
	"Es ist ca. 10 Meter tief.",    //It is about 10 meters deep.
	// 170
	"Oben siehst du helles Licht.",    //Above you is a bright light.
	"Darauf steht:|\"Ruhe eine Minute im Raum|zwischen den Monstern,|und du wirst belohnt!\"",    //It reads:|"Rest a minute in the room|between the monsters,|and you'll be rewarded!"
	"Schlitz",    //Slot
	"Du kommst mit den|H\204nden nicht rein.",    //You cannot get in|with your hands.
	"Es ist ca. 5 Meter tief.",    //It is about 5 meters deep.
	// 175
	"Steine",    //Stones
	"Platte",    //Plate
	"Sarg",    //Coffin
	"Ausgang",    //Exit
	"Unheimlich!",    //Creepy!
	// 180
	"Zahnb\201rste",    //Toothbrush
	"Die Sache mit der|Artus GmbH scheint dir zu|Kopf gestiegen zu sein.",    //The thing with the|Artus GmbH seems to have|gotten to your head.
	"Zahnpastatube",    //Toothpaste
	"Kugel",    //Ball
	"Hmm, die Kugel sieht lose aus.",    //Hmm, the ball looks loose.
	// 185
	"Auge",    //Eye
	"Irgendwas stimmt damit nicht.",    //Something is wrong with that.
	"Sieht nach Metall aus.",    //It looks like metal.
	"Ein Taxi kommt angerauscht,|du steigst ein.",    //A taxi arrives, and you get in.
	"Du dr\201ckst auf den Knopf, aber nichts passiert",    //You press the button, but nothing happens
	// 190
	"Es ist leer.",    //It is empty.
	"Du findest ein kleines Ger\204t,|einen Ausweis und einen Xa.",    //You find a small device,|an ID card and a Xa.
	"Du heftest den|Magnet an die Stange.",    //You attach the|magnet to the pole.
	"Stange mit Magnet",    //Pole with magnet
	"Raffiniert!",    //Cunning!
	// 195
	"Du mu\341t das|Ger\204t erst kaufen.",    //You must buy|this device first.
	"Du legst den Chip|in das Ger\204t ein.",    //You insert the chip|into the device.
	"Du \201berspielst die CD|auf den Musikchip.",    //You transfer the CD|to the Music chip.
	"Ohne einen eingelegten|Musikchip kannst du auf dem|Ger\204t nichts aufnehmen.",    //Without an inserted|music chip, you can not|record on the device.
	"Du nimmst den Chip|aus dem Ger\204t.",    //You remove the chip|from the device.
	// 200
	"Es ist kein Chip eingelegt.",    //There is no chip inserted.
	"Wozu? Du hast sowieso nur die eine CD.",    //What for? You only have one CD anyway.
	"Die \"Mad Monkeys\"-CD. Du hast|sie schon tausendmal geh\224rt.",    //The "Mad Monkeys" CD.|You've heard them a thousand times.
	"Du h\224rst nichts.|Der Chip ist unbespielt.",    //All you hear is silence.|The chip is empty.
	"Du h\224rst dir den Anfang|der \201berspielten CD an.",    //You are listening to the beginning|of the copied CD.
	// 205
	"Es ist kein Chip einglegt.",    //There is no chip inserted.
	"Du trinkst etwas von den Zeug, danach|f\201hlst du dich leicht beschwipst.",    //You drink some of the stuff,|then begin to feel slightly tipsy.
	"%d Xa",    //%d Xa
	"Als du ebenfalls aussteigst haben|die anderen Passagiere das|Fluggel\204nde bereits verlassen.",    //When you get off the plane|the other passengers|have already left the airport.
	"Flughafen",    //Airport
	// 210
	"Stadtzentrum",    //Downtown
	"Kulturpalast",    //Palace of Culture
	"Erde",    //Earth
	"Privatwohnung",    //Private apartment
	"(Taxi verlassen)",    //(Leave the taxi)
	// 215
	"(Bezahlen)",    //(Pay)
	"Adresse:|                              ",    //Address:|                              
	"Fuddeln gilt nicht!|Zu diesem Zeitpunkt kannst du diese|Adresse noch gar nicht kennen!",    //Fiddling with the system doesn't work!|At this time you can not|even know this address!
	"Du hast nicht|mehr genug Geld.",    //You do not|have enough money left.
	"Du merkst, da\341 das Taxi stark beschleunigt.",    //You notice the taxi is accelerating rapidly.
	// 220
	"F\201nf Minuten sp\204ter ...",    //Five minutes later ...
	"Du hast doch schon eine Stange",    //You already have a pole
	"Du s\204gst eine der Stangen ab.",    //You saw off one of the poles.
	"Du betrittst das einzige|offene Gesch\204ft, das|du finden kannst.",    //You enter the only|open shop that|you can find.
	"Die Kabine ist besetzt.",    //The cabin is occupied.
	// 225
	"He, nimm erstmal das Geld|aus dem R\201ckgabeschlitz!",    //Hey, take the money|from the return slot!
	"Du hast doch schon bezahlt.",    //You have already paid.
	"Du hast nicht mehr genug Geld.",    //You do not have enough money left.
	"Du wirfst 10 Xa in den Schlitz.",    //You put 10 Xa in the slot.
	"Dir wird schwarz vor Augen.",    //You are about to pass out.
	// 230
	"Du ruhst dich eine Weile aus.",    //You rest for a while.
	"An der Wand steht:|\"Ich kenne eine tolle Geheimschrift:|A=Z, B=Y, C=X ...|0=0, 1=9, 2=8 ...\"",    //On the Wall is:|"I know a great cypher:|A=Z, B=Y, C=X ...|0=0, 1=9, 2=8 ..."
	"Ok, ich nehme es.",    //OK, I'll take it.
	"Nein danke, das ist mir zu teuer.",    //No thanks, that's too expensive for me.
	"Ich w\201rde gern etwas kaufen.",    //I would like to buy something.
	// 235
	"Ich bin's, Horst Hummel.",    //It's me, Horst Hummel.
	"Haben Sie auch einen Musikchip f\201r das Ger\204t?",    //Do you have a music chip for the device?
	"Eine tolle Maske, nicht wahr?",    //It's a great mask, right?
	"Komisch, da\341 sie schon drei Jahre da steht.",    //Strange that it has been there for three years.
	"Ein starker Trunk. Zieht ganz sch\224n rein.",    //A strong drink. It hits you pretty hard.
	// 240
	"Ein Abspiel- und Aufnahmeger\204t f\201r die neuen Musikchips.",    //A playback and recording device for the new music chips.
	"Eine ARTUS-Zahnb\201rste. Der letzte Schrei.",    //An ARTUS toothbrush. The latest craze.
	"Verkaufe ich massenhaft, die Dinger.",    //I sell these things in bulk.
	"Das sind echte Rarit\204ten. B\201cher in gebundener Form.",    //These are real rarities. Books in bound form.
	"Die Encyclopedia Axacussana.",    //The Encyclopedia Axacussana.
	// 245
	"Das gr\224\341te erh\204ltliche Lexikon auf 30 Speicherchips.",    //The largest available dictionary on 30 memory chips.
	"\232ber 400 Trilliarden Stichw\224rter.",    //Over 400 sextillion keywords.
	"Die ist nicht zu verkaufen.",    //It is not for sale.
	"So eine habe ich meinem Enkel zum Geburtstag geschenkt.",    //I gave one to my grandson for his birthday.
	"Er war begeistert von dem Ding.",    //He was excited about this thing.
	// 250
	"Der stammt aus einem bekannten Computerspiel.",    //It comes from a well-known computer game.
	"Robust, handlich und stromsparend.",    //Sturdy, handy and energy-saving.
	"Irgendein lasches Ges\224ff.",    //Some cheap swill.
	"Das sind Protestaufkleber gegen die hohen Taxigeb\201hren.",    //These are stickers protesting the high taxi fees.
	"Das ist Geschirr aus der neuen Umbina-Kollektion.",    //These are dishes from the new Umbina-Collection.
	// 255
	"H\204\341lich, nicht wahr?",    //Ugly, right?
	"Aber verkaufen tut sich das Zeug gut.",    //But this stuff sells well.
	"Das kostet %d Xa.",    //That costs %d Xa.
	"Schauen Sie sich ruhig um!",    //Take a look around!
	"Unsinn!",    //Nonsense!
	// 260
	"Tut mir leid, die sind|schon alle ausverkauft.",    //I'm very sorry,|they are already sold out.
	"Guten Abend.",    //Good evening.
	"Hallo.",    //Hello.
	"Huch, Sie haben mich aber erschreckt!",    //Yikes, you scared me!
	"Wieso?",    //How so?
	// 265
	"Ihre Verkleidung ist wirklich t\204uschend echt.",    //Your disguise is deceptively real-looking.
	"Welche Verkleidung?",    //What disguise?
	"Na, tun Sie nicht so!",    //Stop pretending you don't know!
	"Sie haben sich verkleidet wie der Au\341erirdische,|dieser Horst Hummel, oder wie er hei\341t.",    //You disguised yourself as that extraterrestrial guy,|Horst Hummel, or whatever his name is.
	"Ich BIN Horst Hummel!",    //I AM Horst Hummel!
	// 270
	"Geben Sie's auf!",    //Give it up!
	"An Ihrer Gestik merkt man, da\341 Sie|ein verkleideter Axacussaner sind.",    //You can tell from your gestures that you are|a disguised Axacussan.
	"Der echte Hummel bewegt sich|anders, irgendwie ruckartig.",    //The real Hummel moves|differently, kind of jerky.
	"Weil er ein Roboter ist! ICH bin der Echte!",    //Because he is a robot! I am the real one!
	"Ach, Sie spinnen ja!",    //Oh, you are crazy!
	// 275
	"Sie Trottel!!!",    //You Idiot!!!
	"Seien Sie still, oder ich werfe Sie raus!",    //Shut up or I'll kick you out!
	"Taschenmesser",    //Pocket knife
	"Hey, da ist sogar eine S\204ge dran.",    //Hey, there's even a saw on it.
	"20 Xa",    //20 Xa
	// 280
	"Discman",    //Discman
	"Da ist noch die \"Mad Monkeys\"-CD drin.",    //The "Mad Monkeys" CD is still in there.
	"Mit dem Ding sollst du dich|an der Wand festhalten.",    //You should hold onto the wall|using that thing.
	"Spezialkeycard",    //Special keycard
	"Damit sollst du die|T\201ren knacken k\224nnen.",    //With that you should be able to crack the doors.
	// 285
	"Alarmknacker",    //Alarm cracker
	"Ein kleines Ger\204t, um|die Alarmanlage auszuschalten.",    //A small device|to turn off the alarm.
	"Karte",    //Keycard
	"Raumschiff",    //Spaceship
	"Damit bist du hierhergekommen.",    //You came here with it.
	// 290
	"Fahrzeuge",    //Vehicles
	"Du kannst von hier aus nicht erkennen,|was das f\201r Fahrzeuge sind.",    //You cannot tell from here|what those vehicles are.
	"Fahrzeug",    //Vehicle
	"Es scheint ein Taxi zu sein.",    //It seems to be a taxi.
	"Komisch, er ist verschlossen.",    //Funny, it is closed.
	// 295
	"Portemonnaie",    //Wallet
	"Das mu\341 ein Axacussaner|hier verloren haben.",    //This must have been|lost by an Axacussan.
	"Ger\204t",    //Device
	"Auf dem Ger\204t steht: \"Taxi-Call\".|Es ist ein kleiner Knopf daran.",    //The device says "Taxi Call."|There is a small button on it.
	"Ausweis",    //ID card
	// 300
	"Auf dem Ausweis steht:|  Berta Tschell|  Axacuss City|  115AY2,96A,32",    //On the card it reads: | Berta Tschell | Axacuss City | 115AY2,96A,32
	"Treppe",    //Staircase
	"Sie f\201hrt zu den Gesch\204ften.",    //It leads to the shops.
	"Gesch\204ftsstra\341e im Hintergrund",    //Business street in the background
	"Die Stra\341e scheint kein Ende zu haben.",    //The road seems to have no end.
	// 305
	"Stange",    //Rod
	"Pfosten",    //Post
	"Gel\204nder",    //Railing
	"Plakat",    //Poster
	"Musik Pur - Der Musikwettbewerb!|Heute im Kulturpalast|Hauptpreis:|Fernsehauftritt mit Horst Hummel|Sponsored by Artus GmbH",    //Pure Music - The Music Competition!|Today at the Palace of Culture|Main Prize:|Television appearance with Horst Hummel|Sponsored by Artus GmbH
	// 310
	"Kabine",    //Cabin
	"Sie ist frei!",    //It is free!
	"Sie ist besetzt.",    //It is occupied.
	"F\201\341e",    //Feet
	"Komisch, die|F\201\341e scheinen|erstarrt zu sein.",    //Strange, the|feet seem to be frozen.
	// 315
	"Haube",    //Hood
	"Sieht aus wie beim Fris\224r.",    //Looks like the hairdresser.
	"400 Xa",    //400 Xa
	"10 Xa",    //10 Xa
	"Dar\201ber steht:|\"Geldeinwurf: 10 Xa\".",    //It says:|"Coins: 10 Xa".
	// 320
	"Dar\201ber steht:|\"Gewinnausgabe / Geldr\201ckgabe\".",    //It says:|"Prize / Money Return".
	"Stuhl",    //Chair
	"Etwas Entspannung k\224nntest du jetzt gebrauchen.",    //You could use some relaxation right about now.
	"Gekritzel",    //Scribble
	"Gesicht",    //Face
	// 325
	"Nicht zu fassen! Die|W\204nde sind genauso beschmutzt|wie auf der Erde.",    //Unbelievable! The walls|are just as dirty|as those on Earth.
	"B\201cher",    //Books
	"Lexikon",    //Dictionary
	"Pflanze",    //Plant
	"Maske",    //Mask
	// 330
	"Schlange",    //Snake
	"Becher",    //Cup
	"Joystick",    //Joystick
	"Eine normale Zahnb\201rste,|es steht nur \"Artus\" darauf.",    //An ordinary toothbrush.|It says "Artus" on it.
	"Musikger\204t",    //Music device
	// 335
	"Ein Ger\204t zum Abspielen und|Aufnehmen von Musikchips.|Es ist ein Mikrofon daran.",    //A device for playing and recording music chips.|There is a microphone on it.
	"Flasche",    //Bottle
	"Auf dem Etikett steht:|\"Enth\204lt 10% Hyperalkohol\".",    //The label says: "Contains 10% hyperalcohol".
	"Kiste",    //Box
	"Verk\204ufer",    //Seller
	// 340
	"Was? Daf\201r wollen Sie die Karte haben?",    //What? Do you want the card for that?
	"Sie sind wohl nicht ganz \201ber|die aktuellen Preise informiert!",    //You are probably not completely|informed about the current prices!
	"Ich bin's, Horst Hummel!",    //It's me, Horst Hummel!
	"Sch\224nes Wetter heute!",    //Nice weather today!
	"K\224nnen Sie mir sagen, von wem ich eine Eintrittskarte f\201r den Musikwettbewerb kriegen kann?",    //Can you tell me who can get me a ticket for the music contest?
	// 345
	"Ok, hier haben Sie den Xa.",    //OK, here is the Xa.
	"Ich biete Ihnen 500 Xa.",    //I offer you 500 Xa.
	"Ich biete Ihnen 1000 Xa.",    //I offer you 1000 Xa.
	"Ich biete Ihnen 5000 Xa.",    //I offer you 5000 Xa.
	"Ich biete Ihnen 10000 Xa.",    //I offer you 10000 Xa.
	// 350
	"Vielen Dank f\201r Ihren Kauf!",    //Thank you for your purchase!
	"Was bieten Sie mir|denn nun f\201r die Karte?",    //What will you offer me|for the card?
	"Hallo, Sie!",    //Hello to you!
	"Was wollen Sie?",    //What do you want?
	"Wer sind Sie?",    //Who are you?
	// 355
	"Horst Hummel!",    //Horst Hummel!
	"Kenne ich nicht.",    //Never heard of him.
	"Was, Sie kennen den ber\201hmten Horst Hummel nicht?",    //What, you don't know the famous Horst Hummel?
	"Ich bin doch der, der immer im Fernsehen zu sehen ist.",    //I'm the guy who is always on TV.
	"Ich kenne Sie wirklich nicht.",    //I really do not know you.
	// 360
	"Komisch.",    //Funny.
	"Aha.",    //Aha.
	"Ja, kann ich.",    //Yes, I can.
	"Von wem denn?",    //From whom?
	"Diese Information kostet einen Xa.",    //This information costs a Xa.
	// 365
	"Wie Sie meinen.",    //As you say.
	"Sie k\224nnen die Karte von MIR bekommen!",    //You can get the card from ME!
	"Aber nur eine Teilnahmekarte,|keine Eintrittskarte.",    //But only a participation ticket,|not an entrance ticket.
	"Was wollen Sie daf\201r haben?",    //What do you want for it?
	"Machen Sie ein Angebot!",    //Make an offer!
	// 370
	"Das ist ein gutes Angebot!",    //That's a good offer!
	"Daf\201r gebe ich Ihnen meine|letzte Teilnahmekarte!",    //For that I give you my|last participation card!
	"(Dieser Trottel!)",    //(That Idiot!)
	"Ich w\201rde gern beim Musikwettbewerb zuschauen.",    //I would like to watch the music competition.
	"Ich w\201rde gern am Musikwettbewerb teilnehmen.",    //I would like to participate in the music competition.
	// 375
	"Wieviel Uhr haben wir?",    //What time is it?
	"Ja.",    //Yes.
	"Nein.",    //No.
	"Hallo, Leute!",    //Hi guys!
	"Hi, Fans!",    //Hi, fans!
	// 380
	"Gute Nacht!",    //Good night!
	"\216h, wie geht es euch?",    //Uh, how are you?
	"Sch\224nes Wetter heute.",    //Nice weather today.
	"Hmm ...",    //Hmm ...
	"Tja ...",    //Well ...
	// 385
	"Also ...",    //So ...
	"Ok, los gehts!",    //OK let's go!
	"Ich klimper mal was auf dem Keyboard hier.",    //I'll fix something on the keyboard here.
	"Halt, sie sind doch schon drangewesen!",    //Stop, you have already been on it!
	"He, Sie! Haben Sie|eine Eintrittskarte?",    //Hey, you! Do you have|a ticket?
	// 390
	"Ja nat\201rlich, hier ist meine Teilnahmekarte.",    //Yes of course, here is my participation ticket.
	"Sie sind Teilnehmer! Fragen|Sie bitte an der Kasse nach,|wann Sie auftreten k\224nnen.",    //You are a participant!|Please ask at the checkout|when you can go on stage.
	"\216h, nein.",    //Uh, no.
	"He, wo ist Ihr Musikchip?",    //Hey, where's your music chip?
	"Laber nicht!",    //Stop talking!
	// 395
	"Fang an!",    //Get started!
	"Einen Moment, ich mu\341 erstmal \201berlegen, was ich|euch spiele.",    //One moment, I have to think about what I'm playing for you.
	"Anfangen!!!",    //Begin!!!
	"Nun denn ...",    //Well then ...
	"Raus!",    //Out!
	// 400
	"Buh!",    //Boo!
	"Aufh\224ren!",    //Stop!
	"Hilfe!",    //Help!
	"Ich verziehe mich lieber.",    //I'd prefer to get lost.
	"Mist, auf dem Chip war|gar keine Musik drauf.",    //Damn, there was no music on the chip at all.
	// 405
	"Das ging ja voll daneben!",    //That went completely wrong!
	"Du n\204herst dich der B\201hne,|aber dir wird mulmig zumute.",    //You approach the stage,|but you feel queasy.
	"Du traust dich nicht, vor|so vielen Menschen aufzutreten|und kehrst wieder um.",    //You do not dare to appear|in front of so many people|and turn around.
	"Oh, Sie sind Teilnehmer!|Dann sind Sie aber sp\204t dran.",    //Oh, you are a participant!|But you are late.
	"Spielen Sie die Musik live?",    //Do you play the music live?
	// 410
	"Dann geben Sie bitte Ihren Musikchip ab!|Er wird bei Ihrem Auftritt abgespielt.",    //Then please submit your music chip!|It will be played during your performance.
	"Oh, Sie sind sofort an der Reihe!|Beeilen Sie sich! Der B\201hneneingang|ist hinter dem Haupteingang rechts.",    //Oh, it's your turn!|Hurry! The stage entrance|is to the right behind the main entrance.
	"Habe ich noch einen zweiten Versuch?",    //Can I have another try?
	"Nein!",    //No!
	"Haben Sie schon eine Eintrittskarte?",    //Do you already have a ticket?
	// 415
	"Tut mir leid, die Karten|sind schon alle ausverkauft.",    //I'm sorry, the tickets|are already sold out.
	"Mist!",    //Crap!
	"Haben Sie schon eine Teilnahmekarte?",    //Do you already have a participation ticket?
	"Ja, hier ist sie.",    //Yes, here it is.
	"Tut mir leid, die Teilnahmekarten|sind schon alle ausverkauft.",    //I'm sorry, the participation tickets|are already sold out.
	// 420
	"Schei\341e!",    //Crap!
	"Das kann ich Ihnen|leider nicht sagen.",    //I can not tell you that.
	"Wo ist denn nun Ihr Musikchip?",    //Where is your music chip?
	"Jetzt beeilen Sie sich doch!",    //Now hurry up!
	"Huch, Sie sind hier bei einem Musik-,|nicht bei einem Imitationswettbewerb",    //Huh, you're here at a music contest,|not at an imitation contest
	// 425
	"Imitationswettbewerb?|Ich will niemanden imitieren.",    //Imitation contest?|I do not want to imitate anyone.
	"Guter Witz, wieso sehen Sie|dann aus wie Horst Hummel?",    //Good joke. Then why do you look like Horst Hummel?
	"Na, nun h\224ren Sie auf! So perfekt ist|ihre Verkleidung auch wieder nicht.",    //Oh come on! Your disguise isn't that perfect.
	"Ich werde Ihnen beweisen, da\341 ich Horst Hummel bin,|indem ich diesen Wettbewerb hier gewinne.",    //I will prove to you that I am Horst Hummel|by winning this competition.
	"Dann kann ich in dieser verdammten Fernsehshow|auftreten.",    //Then I can perform in this|damn TV show.
	// 430
	"Du hampelst ein bi\341chen zu|der Musik vom Chip herum.|Die Leute sind begeistert!",    //You're rocking a little bit|to the music from the chip.|The audience is excited!
	"Guten Abend. Diesmal haben wir|einen besonderen Gast bei uns.",    //Good evening. This time we have|a special guest with us.
	"Es ist der Gewinner des gestrigen|Musikwettbewerbs im Kulturpalast,|der dort vor allem durch seine|Verkleidung aufgefallen war.",    //He is the winner of yesterday's music competition in the Palace of Culture.|He was particularly noteworthy|because of his disguise.
	"Sie haben das Wort!",    //You have the floor!
	"Nun ja, meine erste Frage lautet: ...",    //Well, my first question is ...
	// 435
	"Warum haben Sie sich sofort nach|Ihrer Landung entschlossen, f\201r|die Artus-GmbH zu arbeiten?",    //Why did you decide immediately|after your arrival to work for|Artus GmbH?
	"Es war meine freie Entscheidung.|Die Artus-GmbH hat mir einfach gefallen.",    //It was a decision I made on my own.|I just decided I liked Artus-GmbH.
	"Wieso betonen Sie, da\341 es|Ihre freie Entscheidung war?|Haben Sie Angst, da\341 man Ihnen|nicht glaubt?",    //Why do you stress that|it was your own decision?|Are you afraid that nobody will believe you otherwise?
	"Also, ich mu\341 doch sehr bitten!|Was soll diese unsinnige Frage?",    //How dare you!|What is with this nonsensical question?
	"Ich finde die Frage wichtig.|Nun, Herr Hummel, was haben|Sie dazu zu sagen?",    //I think the question is important.|Well, Mr. Hummel, what do you have to say?
	// 440
	"Auf solch eine Frage brauche|ich nicht zu antworten!",    //I don't feel that I have|to answer such a question!
	"Gut, dann etwas anderes ...",    //Alright, something else then ...
	"Sie sind von Beruf Koch.|Wie hie\341 das Restaurant,|in dem Sie auf der Erde|gearbeitet haben?",    //You are a chef by profession.|What was the name of the restaurant|where you worked|on Earth?
	"Hmm, da\341 wei\341 ich nicht mehr.",    //Hmm, I do not remember that.
	"Sie wollen mir doch nicht weismachen,|da\341 Sie den Namen vergessen haben!",    //Do you really expect me to believe you cannot remember the name?
	// 445
	"Schlie\341lich haben Sie|zehn Jahre dort gearbeitet!",    //After all, you worked there for ten years!
	"Woher wollen Sie das wissen?",    //How do you know that?
	"Nun, ich komme von der Erde,|im Gegensatz zu Ihnen!",    //Well, I come from Earth,|unlike you!
	"Langsam gehen Sie zu weit!",    //Now you've gone too far!
	"Sie sind ein Roboter!|Das merkt man schon an|Ihrer dummen Antwort!|Sie sind nicht optimal|programmiert!",    //You are a robot!|It is obvious from|your stupid answer!|You are not even programmed|correctly!
	// 450
	"Wenn Sie jetzt nicht mit Ihren|Beleidigungen aufh\224ren, mu\341 ich|Ihnen das Mikrofon abschalten!",    //If you do not stop right now|with your insults, I will have|to turn off the microphone!
	"Ich bin der echte Horst Hummel,|und hier ist der Beweis!",    //I am the real Horst Hummel,|and here is the proof!
	"Am n\204chsten Morgen sind alle|Zeitungen voll mit deiner spektakul\204ren|Enth\201llung des Schwindels.",    //The next morning, all the papers|are full of your spectacular|revelation of fraud.
	"Die Manager der Artus-GmbH und Commander|Sumoti wurden sofort verhaftet.",    //The managers of Artus-GmbH and Commander|Sumoti were arrested immediately.
	"Nach dem Stre\341 der letzten Tage,|entscheidest du dich, auf die|Erde zur\201ckzukehren.",    //After these stressful last few days|you decide to return to Earth.
	// 455
	"W\204hrend du dich vor Interviews|kaum noch retten kannst, ...",    //While you can barely save|yourself from interviews, ...
	"... arbeiten die Axacussanischen|Techniker an einem Raumschiff,|das dich zur Erde zur\201ckbringen soll.",    //... the Axacussan|technicians are working on a spaceship|to bring you back to Earth.
	"Eine Woche sp\204ter ist der|Tag des Starts gekommen.",    //One week later, the day of the launch has arrived.
	"Zum dritten Mal in deinem|Leben verbringst du eine lange|Zeit im Tiefschlaf.",    //For the third time in your life,|you spend a long time|in deep sleep.
	"Zehn Jahre sp\204ter ...",    //Ten years later ...
	// 460
	"Du wachst auf und beginnst,|dich schwach an deine|Erlebnisse zu erinnern.",    //You wake up and begin|to faintly remember|your experiences.
	"Um dich herum ist alles dunkel.",    //Everything is dark around you.
	"Sie zeigt %d an.",    //It displays %d.
	"Ich interessiere mich f\201r den Job, bei dem man \201ber Nacht",    //I'm interested in the job where you can get
	"reich werden kann.",    //rich overnight.
	// 465
	"Ich verkaufe frische Tomaten.",    //I sell fresh tomatoes.
	"Ich bin der Klempner. Ich soll hier ein Rohr reparieren.",    //I am the plumber. I'm supposed to fix a pipe here.
	"Ja, h\224rt sich gut an.",    //Yes, it sounds good.
	"Krumme Gesch\204fte? F\201r wen halten Sie mich? Auf Wiedersehen!",    //Crooked business? Who do you think I am? Goodbye!
	"\216h - k\224nnten Sie mir das Ganze nochmal erkl\204ren?",    //Uh - could you explain that to me again?
	// 470
	"Wie gro\341 ist mein Anteil?",    //How big is my share?
	"Machen Sie es immer so, da\341 Sie Ihre Komplizen \201ber ein Graffitti anwerben?",    //Do you always use graffiti to recruit your accomplices?
	"Hmm, Moment mal, ich frage den Boss.",    //Hmm wait, I will ask the boss.
	"Kurze Zeit sp\204ter ...",    //A short while later ...
	"Ok, der Boss will dich sprechen.",    //OK, the boss wants to talk to you.
	// 475
	"Du betrittst die Wohnung und|wirst zu einem Tisch gef\201hrt.",    //You enter the apartment and are led to a table.
	"Hmm, du willst dir also|etwas Geld verdienen?",    //Hmm, so you want to earn some money?
	"Nun ja, wir planen|einen n\204chtlichen Besuch|eines bekannten Museums.",    //Well, we're planning|a nightly visit|to a well-known museum.
	"Wie sieht's aus, bist du interessiert?",    //So, are you interested?
	"Halt, warte!",    //Stop, wait!
	// 480
	"\232berleg's dir, es springen|30000 Xa f\201r dich raus!",    //Think about it, your share would be|30000 Xa!
	"30000?! Ok, ich mache mit.",    //30000?! Alright, count me in.
	"Gut, dann zu den Einzelheiten.",    //Good, now then to the details.
	"Bei dem Museum handelt es|sich um das Orzeng-Museum.",    //The museum in question is|the Orzeng Museum.
	"Es enth\204lt die wertvollsten|Dinosaurierfunde von ganz Axacuss.",    //It contains the most valuable|dinosaur discoveries of Axacuss.
	// 485
	"Wir haben es auf das Sodo-Skelett|abgesehen. Es ist weltber\201hmt.",    //We're aiming to get the Sodo skeleton.|It is world-famous.
	"Alle bekannten Pal\204ontologen haben|sich schon damit besch\204ftigt.",    //All known paleontologists|have already dealt with it.
	"Der Grund daf\201r ist, da\341 es allen|bis jetzt bekannten Erkenntnissen|\232ber die Evolution widerspricht.",    //The reason for this is that it contradicts all known|knowledge about evolution.
	"Irgendein verr\201ckter Forscher|bietet uns 200.000 Xa,|wenn wir ihm das Ding beschaffen.",    //Some crazy researcher|will give us 200,000 Xa|if we retrieve that thing for him.
	"So, jetzt zu deiner Aufgabe:",    //So, now to your task:
	// 490
	"Du dringst durch den Nebeneingang|in das Geb\204ude ein.",    //You enter the building through|the side entrance.
	"Dort schaltest du die Alarmanlage aus,|durch die das Sodo-Skelett gesichert wird.",    //There you switch off the alarm system,|which secures the Sodo skeleton.
	"Wir betreten einen anderen Geb\204udeteil|und holen uns das Gerippe.",    //We'll enter another part of the building|and fetch the skeleton.
	"Deine Aufgabe ist nicht leicht.|Schau dir diesen Plan an.",    //Your task is not easy.|Look at this plan.
	"Unten siehst du die kleine Abstellkammer,|durch die du in die Austellungsr\204ume kommst.",    //Below you can see the small storage room,|through which you come to the showrooms.
	// 495
	"Bei der mit Y gekennzeichneten|Stelle ist die Alarmanlage.",    //The alarm system is at the location marked Y.
	"Bei dem X steht ein gro\341er Dinosaurier|mit einem wertvollen Sch\204del.|Den Sch\204del nimmst du mit.",    //The X marks the spot with a big dinosaur|with a valuable skull.|You will take the skull with you.
	"Nun zu den Problemen:",    //Now for the problems:
	"Die wei\341 gekennzeichneten|T\201ren sind verschlossen.",    //The marked white doors|are locked.
	"Sie m\201ssen mit einer Spezialkeycard ge\224ffnet|werden, was jedoch einige Zeit dauert.",    //They have to be opened with a special keycard,|which can take a while.
	// 500
	"Au\341erdem gibt es in den auf der Karte|farbigen R\204umen einen Druck-Alarm.",    //In addition, there are pressure alarms|in the rooms which are colored on the map.
	"Du darfst dich dort nicht l\204nger|als 16 bzw. 8 Sekunden aufhalten,|sonst wird Alarm ausgel\224st.",    //You can not stay there longer than|16 or 8 seconds,|or the alarm will go off.
	"Im Raum oben rechts ist|eine Kamera installiert.",    //In the room at the top right|there is a camera installed.
	"Diese wird jedoch nur von|der 21. bis zur 40. Sekunde|einer Minute \201berwacht.",    //However, it is only monitored|between the 21st and the 40th second|of every minute.
	"Das gr\224\341te Problem ist der W\204chter.",    //The biggest problem is the guard.
	// 505
	"Er braucht f\201r seine Runde genau|eine Minute, ist also ungef\204hr|zehn Sekunden in einem Raum.",    //He needs exactly one minute for his round,|so he is in each room|for about ten seconds.
	"Du m\201\341test seine Schritte h\224ren k\224nnen,|wenn du in der Abstellkammer bist|und der W\204chter dort vorbeikommt.",    //You should be able to hear his footsteps|if you are in the closet|and the guard passes by.
	"Wenn du es bis zur Alarmanlage|geschafft hast, h\204ngst du dich|mit dem Sauger an die Wand,|damit du keinen Druck-Alarm ausl\224st.",    //If you make it to the alarm system,|you'll use the sucker to hang on the wall|to avoid triggering the pressure alarm.
	"Die Alarmanlage schaltest du|mit einem speziellen Ger\204t aus.",    //You switch off the alarm system|with a special device.
	"Wenn du das geschafft hast, nichts|wie raus! Aber keine Panik,|du darfst keinen Alarm ausl\224sen.",    //Once you're done, get out of there!|But do not panic!|You must not set off the alarm.
	// 510
	"So, noch irgendwelche Fragen?",    //So, any more questions?
	"Also gut.",    //All right then.
	"Du bekommst 30000 Xa.",    //You get 30,000 Xa.
	"Ja, die Methode hat sich bew\204hrt.",    //Yes, that method has proven itself worthy.
	"Hast du sonst noch Fragen?",    //Do you have any questions?
	// 515
	"Nachdem wir alles gekl\204rt|haben, kann es ja losgehen!",    //Now that we are on the same page we can get started!
	"Zur vereinbarten Zeit ...",    //At the agreed upon time ...
	"Du stehst vor dem Orzeng Museum,|w\204hrend die Gangster schon in einen|anderen Geb\204uderteil eingedrungen sind.",    //You stand in front of the Orzeng Museum,|while the gangsters have already penetrated|into another part of the building.
	"Wichtiger Hinweis:|Hier ist die letzte M\224glichkeit,|vor dem Einbruch abzuspeichern.",    //Important note:|Here is the last possibility to save|before the break-in.
	"Wenn Sie das Museum betreten haben,|k\224nnen Sie nicht mehr speichern!",    //Once you enter the museum|you will not be able to save!
	// 520
	"Stecken Sie sich Ihre|Tomaten an den Hut!",    //You can keep your tomatoes!
	"Das kann ja jeder sagen!",    //Anyone can say that!
	"Niemand \224ffnet.",    //Nobody answers.
	"Welche Zahl willst du eingeben:      ",    //What number do you want to enter:      
	"Falsche Eingabe",    //Invalid input
	// 525
	"Der Aufzug bewegt sich.",    //The elevator is moving.
	"Die Karte wird|nicht angenommen.",    //The card|is not accepted.
	"Da ist nichts mehr.",    //There is nothing left.
	"Da ist ein Schl\201ssel unter dem Bett!",    //There's a key under the bed!
	"Hey, da ist etwas unter dem|Bett. Nach dem Ger\204usch zu|urteilen, ist es aus Metall.",    //Hey, there is something under the|bed. Judging by the noise,|it is made of metal.
	// 530
	"Mist, es gelingt dir nicht,|den Gegenstand hervorzuholen.",    //Damn, you do not succeed in getting the object out.
	"Die Klappe ist schon offen.",    //The flap is already open.
	"Der Schl\201sssel pa\341t nicht.",    //The key does not fit.
	"Du steckst den Chip in die|Anlage, aber es passiert nichts.|Die Anlage scheint kaputt zu sein.",    //You put the chip in the stereo,|but nothing happens.|The stereo seems to be broken.
	"Es passiert nichts. Das Ding|scheint kaputt zu sein.",    //Nothing happens. The thing|seems to be broken.
	// 535
	"Hochspannung ist ungesund, wie du aus|Teil 1 eigentlich wissen m\201\341test!",    //High voltage is unhealthy, as you|should already know|from Part 1!
	"Es h\204ngt ein Kabel heraus.",    //A cable hangs out.
	"Irgendetwas hat hier|nicht ganz funktioniert.",    //Something did not|quite work out here.
	"Du ziehst den Raumanzug an.",    //You put on your space suit.
	"Du ziehst den Raumanzug aus.",    //You take off your space suit.
	// 540
	"Das ist schon verbunden.",    //That is already connected.
	"Die Leitung ist hier|schon ganz richtig.",    //The cable is already|at the right place.
	"Roger W.! Wie kommen Sie denn hierher?",    //Roger W.! How did you get here?
	"Ach, sieh mal einer an! Sie schon wieder!",    //Oh, look at that! It's you again!
	"Wo haben Sie denn|Ihr Schiff gelassen?",    //Where did you|leave your ship?
	// 545
	"Schauen Sie mal hinter mich auf|den Turm! Da oben h\204ngt es.",    //Take a look behind me, up on|the tower! It's up there.
	"Ich hatte es scheinbar etwas zu|eilig, aber ich mu\341te unbedingt|zu den Dreharbeiten nach Xenon!",    //Apparently I was too much in a hurry,|but I had to be at the film shooting in Xenon!
	"Mich wundert, da\341 es die Leute|hier so gelassen nehmen.",    //I am surprised that people|here take things so calmly.
	"Die tun gerade so, als ob der Turm|schon immer so schr\204g gestanden h\204tte!",    //They are pretending that the tower|has always been that slanted!
	"Hat er auch, schon seit|mehreren Jahrhunderten!",    //It has, for|several centuries, actually!
	// 550
	"\216h ... ach so. Und von wo|kommen Sie? Sie hatten's ja|wohl auch ziemlich eilig.",    //Uh ... I see. And where are you coming from? It seems you were in quite a hurry as well.
	"Ich komme von Axacuss.",    //I come from Axacuss.
	"Hmm, was mach ich jetzt blo\341?",    //Hmm, what am I going to do now?
	"Ich kenne ein gutes Cafe nicht|weit von hier, da k\224nnen|wir uns erstmal erholen.",    //I know a good cafe not far from here,|where we can get some rest.
	"Ok, einverstanden.",    //OK, I agree.
	// 555
	"Faszinierend!",    //Fascinating!
	"Taxis",    //Taxis
	"Hier ist ja richtig was los!",    //There seems to be something really going on here!
	"Axacussaner",    //Axacussan
	"Teilnahmekarte",    //Participation card
	// 560
	"Axacussanerin",    //Axacussian
	"Darauf steht:|\"115AY2,96A\"",    //It reads:|"115AY2,96A"
	"Darauf steht:|\"115AY2,96B\"",    //It reads:|"115AY2,96B"
	"Darauf steht:|\"341,105A\"",    //It reads:|"341,105A"
	"Darauf steht:|\"341,105B\"",    //It reads:|"341,105B"
	// 565
	"Klingel",    //Bell
	"Anzeige",    //Display
	"Tastenblock",    //Keypad
	"Es sind Tasten von 0 bis 9 darauf.",    //There are keys from 0 to 9 on it.
	"Chip",    //Chip
	// 570
	"Es ist ein Musikchip!",    //It's a music chip!
	"Klappe",    //Hatch
	"Sie ist mit einem altmodischen|Schlo\341 verschlossen.",    //It is secured with an old-fashioned lock.
	"Musikanlage",    //Music system
	"Toll, eine in die Wand|integrierte Stereoanlage.",    //Great, a built-in stereo|in the wall.
	// 575
	"Boxen",    //Speakers
	"Ganz normale Boxen.",    //Ordinary speakers.
	"Stifte",    //Pencils
	"Ganz normale Stifte.",    //Ordinary pencils.
	"Metallkl\224tzchen",    //Metal blocks
	// 580
	"Es ist magnetisch.",    //It is magnetic.
	"Bild",    //Image
	"Ein ungew\224hnliches Bild.",    //An unusual picture.
	"Schrank",    //Cabinet
	"Er ist verschlossen",    //It is closed
	// 585
	"Aufzug",    //Elevator
	"unter Bett",    //under bed
	"Unter dem Bett sind bestimmt wichtige|Dinge zu finden, nur kommst du nicht darunter.|Du br\204uchtest einen Stock oder so etwas.",    //Under the bed are certainly important|things to find, only you cannot reach underneath.|You need a stick or something.
	"Schl\201ssel",    //Key
	"Ein kleiner Metallschl\201ssel.",    //A small metal key.
	// 590
	"Schalter",    //Switch
	"Griff",    //Handle
	"Luke",    //Hatch
	"Raumanzug",    //Space suit
	"Ein zusammenfaltbarer Raumanzug.",    //A collapsible spacesuit.
	// 595
	"Leitung",    //Cable
	"Irgendetwas scheint hier|kaputtgegangen zu sein.",    //Something seems to|have broken here.
	"Sie h\204ngt lose von der Decke runter.",    //It hangs loose from the ceiling.
	"Zur Erinnerung:|Dir ist es gelungen, aus den|Artus-Geheimb\201ros zu fliehen.",    //Reminder:|You managed to escape from the|Artus-GmbH secret offices.
	"Nun befindest du dich in|einem Passagierraumschiff,|das nach Axacuss City fliegt.",    //Now you are in a passenger|spaceship that|flies to Axacuss City.
	// 600
	"W\204hrend des Fluges schaust du dir|das axacussanische Fernsehprogramm an.|Du st\224\341t auf etwas Interessantes ...",    //During the flight, you watch the|Axacussan TV program.|You come across something interesting ...
	"Herzlich willkommen!",    //Welcome!
	"Heute zu Gast ist Alga Lorch.|Sie wird Fragen an den Erdling|Horst Hummel stellen.",    //Alga Lorch will be present today.|She will ask questions to the Earthling|Horst Hummel.
	"Horst wird alle Fragen|beantworten, soweit es|ihm m\224glich ist.",    //Horst will answer all|questions as fully|as possible.
	"Sie haben das Wort, Frau Lorch!",    //You have the floor, Mrs Lorch!
	// 605
	"Herr Hummel, hier ist meine erste Frage: ...",    //Mr. Hummel, here is my first question: ...
	"Sie sind nun ein ber\201hmter Mann auf Axacuss.|Aber sicher vermissen Sie auch Ihren Heimatplaneten.",    //You are now a famous man on Axacuss.|But surely you miss your home planet.
	"Wenn Sie w\204hlen k\224nnten, w\201rden Sie lieber|ein normales Leben auf der Erde f\201hren,|oder finden Sie das Leben hier gut?",    //If you could choose, would you prefer|to lead a normal life on Earth,|or do you find life here good?
	"Ehrlich gesagt finde ich es sch\224n,|ber\201hmt zu sein. Das Leben ist|aufregender als auf der Erde.",    //Honestly, I think it's nice to be|famous. Life is more exciting here|than on Earth.
	"Au\341erdem sind die Leute von der|Artus GmbH hervorragende Freunde.",    //In addition, the people of|Artus GmbH are excellent friends.
	// 610
	"Nun ja, planen Sie denn trotzdem,|irgendwann auf die Erde zur\201ckzukehren?",    //Well, are you still planning|to return to Earth someday?
	"Das kann ich Ihnen zum jetzigen|Zeitpunkt noch nicht genau sagen.",    //At this point in time,|I haven't made up my mind, yet.
	"Aber ich versichere Ihnen, ich|werde noch eine Weile hierbleiben.",    //But I assure you,|I will stay here for a while.
	"Aha, mich interessiert au\341erdem,|ob es hier auf Axacuss etwas gibt,|das Sie besonders m\224gen.",    //I see. I'm also interested in|whether there's anything here on Axacuss that you particularly like.
	"Oh mir gef\204llt der ganze Planet,|aber das Beste hier sind die|hervorragenden Artus-Zahnb\201rsten!",    //Oh I like the whole planet,|but the best thing here are the|extraordinary Artus toothbrushes!
	// 615
	"Zahnb\201rsten von solcher Qualit\204t|gab es auf der Erde nicht.",    //Toothbrushes of such quality|do not exist on Earth.
	"\216h, ach so.",    //Um, I see.
	"Pl\224tzlich lenkt dich eine|Lautsprecherstimme vom Fernseher ab.",    //Suddenly, a speaker's voice|distracts you from the television.
	"\"Sehr geehrte Damen und Herren,|wir sind soeben auf dem Flughafen|von Axacuss City gelandet.\"",    //"Ladies and Gentlemen,|We just landed at the airport|at Axacuss City."
	"\"Ich hoffe, Sie hatten einen angenehmen Flug.|Bitte verlassen Sie das Raumschiff! Auf Wiedersehen!\"",    //"I hope you had a nice flight.|Please leave the spaceship! Goodbye!"
	// 620
	"W\204hrend die anderen Passagiere|aussteigen, versuchst du,|den Schock zu verarbeiten.",    //While the other passengers|are disembarking, you are trying|to handle the shock.
	"\"Ich mu\341 beweisen, da\341 dieser|Roboter der falsche Horst|Hummel ist!\", denkst du.",    //"I have to prove that this robot|is the wrong Horst|Hummel!", you think to yourself.
	"\"Diese Mistkerle von der Artus GmbH und|Commander Sumoti m\201ssen entlarvt werden!\"", //"These bastards from Artus GmbH and|Commander Sumoti must be unmasked!"
	"Sieht gef\204hrlich aus!", //Looks dangerous
	"Das Auge ist schon offen.", //This Eye is already opened
	// 625
	"Es gelingt dir, zu fliehen.", //You manage to escape
	NULL
};

#endif // GAMETEXT_H
