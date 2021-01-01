Umfangreichere Informationen über die Änderungen des aktuellen experimentellen
Programmcodes finden Sie auf Englisch unter:
        https://github.com/scummvm/scummvm/commits/

#### 2.3.0 (DD.MM.YYYY)

 Neue Spiele:
   - Unterstützung für Grim Fandango hinzugefügt.
   - Unterstützung für Escape from Monkey Island hinzugefügt.
   - Unterstützung für The Longest Journey hinzugefügt.
   - Unterstützung für Myst 3: Exile hinzugefügt.
   - Unterstützung für Hades' Challenge hinzugefügt.
   - Unterstützung für Little Big Adventure hinzugefügt.
   - Unterstützung für Red Comrades 1: Save the Galaxy hinzugefügt.
   - Unterstützung für Red Comrades 2: For the Great Justice hinzugefügt.
   - Unterstützung für Transylvania hinzugefügt.
   - Unterstützung für Crimson Crown hinzugefügt.
   - Unterstützung für OO-Topos hinzugefügt.
   - Unterstptzung für die Glulx interactive fiction-Spiele hinzugefügt.

 Allgemein:
   - Die ScummVM-Benutzeroberfläche unterstützt nun UTF-32.
   - Code der Roland MT-32-Emulation auf Munt 2.4.1 aktualisiert.

 Dreamweb:
   - Darstellungsfehler in der russischen Fan-Übersetzung behoben.

 Glk:
   - Probleme mit Spielständen in mehreren Sub-Engines behoben.
   - Speicherüberlauf im Erkennungsmechanismus für Level9-Spiele behoben.
   - Unterstützung für die Spiele der IF Comp 2020 hinzugefügt.
   - Die Sub-Engine Glulx ist jetzt aktiviert.

 Gob:
   - Unterstützung für die russische Fan-Übersetzung von Bargon Attack hinzugefügt.
   - Unterstützung für die russische Übersetzung von Woodruff hinzugefügt.

 Grim:
   - Unterstützung für die Version in brasilianischem Portugiesisch von Grim Fandango hinzugefügt.
   - Unterstützung für die russische Version von Escape from Monkey Island hinzugefügt.

 Kyra:
   - Unterstützung für die japanische Sega-CD-Version von Eye of the Beholder hinzugefügt.
   - Unterstützung für die hebräische Fan-Übersetzung von Legend of Kyrandia hinzugefügt.

 Lure:
   - Die Feuer-Animation im ersten Raum wird nun auch nach dem Laden eines Spielstands abgespielt.
   - Unterstützung für MT-32 korrigiert.

 SAGA:
   - Unterstützung für ITE GOG Mac CD v1.1 hinzugefügt.
   - Unterstützung für die japanische Version von ITE PC-98 hinzugefügt.

 SCI:
   - Unterstützung für RGB-Rendering (16/32bpp) für SCI0 - SCI1.1-Spiele hinzugefügt. Dadurch werden Probleme
     mit der Farbpalette während Bildschirmübergängen und Probleme bei der Darstellung von Mac-QuickTime-Videos korrigiert.
   - Unterstützung für benutzerdefinierte Farbpaletten in SQ3 und LSL2 des FreeSCI-Projekts hinzugefügt.
     Wenn diese Mods aktiviert werden, wird die grafische Darstellung in diesen beiden Spielen verbessert.
   - Unterstützung für die koreanischen Fan-Übersetzungen von Castle of Dr. Brain, Eco Quest, Gabriel Knight,
     King's Quest 1, 5 und 6, Laura Bow 2: Dagger of Amon Ra und Space Quest 4 hinzugefügt.
   - Unterstützung für zahlreiche koreanische Übersetzungen des scummkor-Projektes hinzugefügt.
   - Unterstützung für die russische Version von LSL6 hinzugefügt.
   - Unterstützung für eine alternative russische Version von LSL7 hinzugefügt.

 SCUMM:
   - Textdarstellung in der chinesischen, japanischen und koreanischen Version von The Dig und
     the Curse of Monkey Island korrigiert. Diese Fehlerkorrekturen beinhalten auch einige allgemeine
     Verbesserungen an der Textdarstellung (hauptsächlich bezüglich der Formatierung von Text-Umbrüchen).
   - Darstellung des Pause- und Neustart-Dialogs in der chinesischen, japanischen und koreanischen Version
     der Benutzeroberfläche korrigiert.
   - Unterstützung für zahlreiche koreanische Übersetzungen des scummkor-Projektes hinzugefügt.
   - Unterstützung für die Russobit-M-Versionen von Pajama2 und SpyOzone hinzugefügt.
   - Sprachausgabe in der Akella-Version von COMI korrigiert.

 Tinsel:
   - Unterstützung für die Rückkehr zur ScummVM-Benutzeroberfläche hinzugefügt.

 TsAGE:
   - Unterstützung für eine weitere Disketten-Version hinzugefügt.

 Stark:
   - Unterstützung für die ungarische Übersetzung hinzugefügt.

 Supernova:
   - Teil 1 von Supernova ist nun in einer italienischen Übersetzung verfügbar.

 Sword25:
   - Unterstützung für die entpackte Version hinzugefügt.
   - Absturz behoben, der auftritt, wenn kroatisch als Spielsprache gewählt wird.

 Ultima:
   - Ultima 4: Erweiterung des Debuggers um mehrere Befehle.
   - Ultima 4: Die Return-Taste kann nun dazu genutzt werden, die ZStats-Anzeige zu verlassen.

 Xeen:
   - Gelegentliche Grafikfehler in Kampf-Szenen korrigiert.
   - Verbesserte Unterstützung für Zwischensequenzen, die nun dem Original genauer entsprechen.

 AmigaOS-Portierung:
   - Unterstützung für native Dateisystem-Dialoge hinzugefügt.
   - Der nuked OPL Adlib-Treiber ist wieder aktiviert.

 MorphOS-Portierung:
   - Unterstützung für native Dateisystem-Dialoge hinzugefügt.
   - Unterstützung für die Cloud-Anbindung hinzugefügt.
   - Der nuked OPL Adlib-Treiber ist wieder aktiviert.

#### 2.2.0 "Interactive Fantasy" (27.09.2020)

 Neue Spiele:
   - Unterstützung für Blazing Dragons hinzugefügt.
   - Unterstützung für Griffon Legend hinzugefügt.
   - Unterstützung für Spiele von Interactive Fiction hinzugefügt, basierend auf folgenden Engines:
     ADRIFT (außer Version 5), AdvSys, AGT, Alan 2 & 3
     Archetype (neu implementiert für Glk, basierend auf dem originalen Pascal-Quellcode)
     Hugo, JACL, Level 9, Magnetic Scrolls, Quest, Scott Adams,
     ZCode (alle ZCode-Spiele, außer Infocom graphical version 6).
     Aktuell werden mehr als 1600 Spiele erkannt und unterstützt.
   - Unterstützung für Operation Stealth hinzugefügt.
   - Unterstützung für Police Quest: SWAT hinzugefügt.
   - Unterstützung für die englische Übersetzung von "Prince and the Coward" hinzugefügt.
   - Unterstützung für Ultima IV - Quest of the Avatar hinzugefügt.
   - Unterstützung für Ultima VI - The False Prophet hinzugefügt.
   - Unterstützung für Ultima VIII - Pagan hinzugefügt.

 Neue Portierungen:
   - Die Version für MorphOS wurde zu großen Teilen neu geschrieben.

 Allgemein:
   - Alle Engines unterstützen nun automatisch gespeicherte Spielstände.
   - Fehler werden nun im Debugger angezeigt, anstatt ScummVM einfach abstürzen zu lassen.
   - Artikel werden bei der Sortierung der Spiele im Launcher nicht mehr berücksichtigt.
   - Hebräisch wird nun korrekt in der Benutzeroberfläche dargestellt (benötigt die FriBiDi-Bibliothek).
   - Code der Roland MT-32-Emulation auf Munt 2.4.0 aktualisiert.
   - Die Standard-Stimme der Sprachausgabe kann auf unterstützten Plattformen nun angepasst werden.
   - Unterstützung für Discord-Integration auf unterstützten Plattformen (Windows Vista+, macOS 10.9+ 64 Bit) hinzugefügt.
   - Die Unterstützung für Tastenkürzel über den Keymapper wurde erheblich verbessert.

 BBVS:
   - Unterstützung für die Demo-Version hinzugefügt. Die Demo-Version ist auf unserer Website verfügbar.
   - Unterstützung für die Demo-Version des Loogie-Minigames hinzugefügt.

 Dreamweb:
   - Unterstützung für russische Fan-Übersetzung hinzugefügt.
   - Animationsgeschwindigkeit korrigiert.

 Illusions:
   - Geschwindigkeit für die Untertitel korrigiert (maximale Geschwindigkeit ergibt eine gute Geschwindigkeit)
   - Unterstützung für die russische Version von Duckman hinzugefügt.

 Kyra:
   - Unterstützung für die SegaCD-Version von Eye of the Beholder I (mit CD-Audio, animierten
     Zwischensequenzen und der Karten-Funktion) hinzugefügt.
   - Unterstützung für die PC-98-Version von Eye of the Beholder I hinzugefügt.
   - Unterstützung für die spanische Version von Eye of the Beholder I und II, Legend of Kyrandia 
     (Fan-Übersetzung auf CD-ROM) und Legend of Kyrandia 2 (Disketten-Version und Fan-Übersetzung
     auf CD-ROM hinzugefügt. Unterstützung für die spanische Version von Lands of Lore (Disketten-Version
     und Fan-Übersetzung auf CD-ROM) korrigiert.

 Lab:
   - Fehler behoben, der in einigen Räumen zu einem Audio-Loop geführt hat.

 Neverhood:
   - Unterstützung für eine umfangreichere Demo-Version hinzugefügt. Die Demo-Version ist auf unserer Website verfügbar.

 Prince:
   - Fehlerhafte Anzeige der Item-Beschreibungen im Inventar korrigiert.
   - Englischsprachige Übersetzung hinzugefügt.
   - Fehler behoben, der bei der Betrachtung bestimmter Gegenstände zu einer Endlosschleife geführt hat.

 Queen:
   - Fehler beim Laden von Spielständen aus dem Launcher heraus korrigiert.
   - Fehler behoben, der zu Spielbeginn zu einer zufälligen Verzögerung geführt hat.

 SCI:
   - Deutliche Verbesserung der Amiga- und Mac-Audiotreiber.
   - Verbesserung der MIDI-Wiedergabe und Korrektur zahlreicher Audio-Probleme.
   - Korrektur von 30 Skript-Fehler in ECO2, GK1, KQ4, KQ5, KQ6, KQ7, LB1, LONGBOW, PHANT2, QFG1, QFG3, QFG4, SQ5 und SQ6.
   - Korrektur eines Skript-Fehlers, der zu seltenen und zufälligen Abstürzen in den meisten Sierra-Spielen zwischen 1992 und 1996 geführt hat.
   - Unterstützung für Inside the Chest / Behind the Developer's Shield hinzugefügt.
   - Unterstützung für die deutsche Disketten-Version von Leisure Suit Larry 6 hinzugefügt.
   - Unterstützung für die hebräische Version von Torin's Passage hinzugefügt.
   - Unterstützung für die italienische Version von Lighthouse hinzugefügt.
   - Unterstützung für die polnische Version von KQ5, LSL2, LSL3, LSL5 und der Disketten-Version von LSL6 hinzugefügt.
   - Fehler in der russischen Version von LSL1 korrigiert.
   - Fehler in Phantasmagoria 2 korrigiert, der beim Aufruf eines Easter Eggs auftritt.
   - Automatisch erzeugte Spielstände in QFG3 repariert.
   - Spielstand-Vorschaubilder für QFG4 und Shivers werden nun nicht mehr durch Bedienelemente verdeckt.
   - Zufällige Abstürze in der Windows-Version behoben, die auftreten, wenn ein Spiel beendet wird.
   - Unterstützung für den Roland D-110-Audiotreiber hinzugefügt.
   - Die Option "Digitale Geräusch-Effekte bevorzugen" wird nun auch für SCI01/SCI1-Spiele berücksichtigt.

 SCUMM:
   - Fehlerhafte Farbpalette korrigiert, die zu falscher Darstellung in der NES-Version von Maniac Mansion führt.
   - Darstellung der Taschenlampe in der NES-Version von Maniac Mansion entspricht nun dem Original.
   - Bisherige NES-Farbpalette durch genauere NTSC-Palette von Mesen ersetzt.
   - Neue Option zur Benutzeroberfläche hinzugefügt, welche die Auswahl einer Farbpalette basierend auf dem NES Classic erlaubt.
   - Farbpalette in der Apple //gs-Version von Maniac Mansion verbessert.
   - Absturz beim Betreten der Garage in der Apple //gs-Version von Maniac Mansion behoben.
   - Unterstützung für die klassische Version der Remastered-Edition von Full Throttle hinzugefügt.

 Supernova:
   - Verbesserung der englischsprachigen Übersetzung.

 Sky:
   - Synchronisation der Musik-Lautstärke zwischen dem nativen Einstellungs-Dialog des Spiels und der ScummVM-Konfiguration korrigiert.

 Sword1:
   - Unterstützung für lokalisierte Menüs in der russischen Version der Novy Disk Trilogy hinzugefügt.
   - Unterstützung für eine spanische Demoversion hinzugefügt (auf unserer Website verfügbar).

 Sword2:
   - Unterstützung für eine spanische Demoversion hinzugefügt (auf unserer Website verfügbar).

 Titanic:
   - Das Spiel stürzt nicht mehr ab, wenn man sich nach den Bedürfnissen des Barbot erkundigt.

 Wintermute:
   - System zur Verfolgung von Errungenschaften hinzugefügt, für mehr als 10 Spiele verfügbar.

 Xeen:
   - Fehlendes Sprite hinzugefügt, welches dargestellt werden soll, wenn ein Gegner von einem Energie-Strahl getroffen wird.
   - Das Spiel friert nun nicht mehr wegen fehlerhafter Mob-Daten an der Spitze des Hexenturms ein.
   - Absturz beim Laden einige Spielstände aus dem Launcher heraus korrigiert.
   - Die Quelle in Nightshadow kann nun geheilt werden.
   - Fehler beim Laden einiger Items behoben.
   - Im Quest-Dialog können nun die Pfeiltasten verwendet werden.
   - Fehlerhafte Ausrichtung der Spiegel im Dragon Tower korrigiert.
   - Absturz beim Lesen des Buchs im Tresor der großen Pyramide korrigiert.
   - Attribute können nun nicht mehr negativ werden.
   - Fehler bei der Darstellung einiger Animationen behoben.

 ZVision:
   - Korrektur eines neu aufgetrenenen Fehlers im Zusammenhang mit dem Tresor-Puzzle von Zork Nemesis: The Forbidden Lands.
   - In Zork: Grad Inquisitor kann nun ein perfekter Punkte-Stand erzielt werden.

 Android-Portierung:
   - Unterstützung für den Immersive Full-Screen Mode hinzugefügt.
   - Unterstützung für Tastatur und Maus verbessert.

 iOS-Portierung:
   - Erweiterung der Grafik-Unterstützung für einige Spiele (zum Beispiel SCI-Spiele mit hochauflösenden Videos).
   - "Beenden"-Button entfernt, um den iOS-Design-Richtlinien zu entsprehcne.
   - Eingabehilfen für die virtuelle Tastatur entfernt. Dies bedeutet, dass bei der Verwendung einer
     externen Tastatur keine zusätzliche Leiste im unteren Bereich des Bildschirms mehr angezeigt wird.
   - Sofern unterstützt wird bei einem Wechsel in eine andere App der aktuelle Spielstand in ScummVM
     gespeichert und bei der Rückkehr in die ScummVM-App wiederhergestellt.

 Linux-Portierung:
   - Anstelle des Datei-Browsers von ScummVM kann nun der Datei-Browser des Betriebssystems verwendet werden.

 MacOS X-Portierung:
   - Unscharfe Darstellung bei der Verwendung von Retina-Displays behoben. Leider musste hierzu die Unterstützung
     für das Dark-Theme für Fenster-Dekorationen entfernt werden.
   - Darstellung des Spiel-Icons in der Taskbar korrigiert (diese Funktion war seit ScummVM 1.9.0 defekt).

 RISC OS-Portierung:
   - Für VFP optimierte Version zur verbesserten Unterstützung neuerer Hardware hinzugefügt.

 Windows-Portierung:
   - Sprachausgabe um die Unterstützung für OneCore-Stimmen erweitert.
   - Die aktive Unterstützung für Windows Vista und älter wird eingestellt. Während wir auf absehbare
     Zeit ScummVM für diese Systeme weiterhin zur Verfügung stellen werden, werden einige neuere
     Funktionen möglicherweise fehlen.


#### 2.1.2 "Fixing Powerful Windows" (31.03.2020)

 Windows-Portierung:
   - Fehler korrigiert, der zu einer Update-Schleife des Sparkle-Updaters geführt hat.

 MacOS X-Portierung:
   - Fehler behoben, der dazu führt, dass ScummVM beim Start unter Mac OS X 10.5 und älter einfriert.
   - Anwendungs-Icon unter Mac OS X 10.5 und älter korrigiert.


#### 2.1.1 ":More sheep:" (31.01.2020)

 Allgemein:
   - Absturz beim Wechsel zwischen einigen Sprachen der Benutzeroberfläche korrigiert.
   - Ein Druck auf die ESC-Taste speichert nicht mehr irrtümlich Einstellungen im Optionen-Dialog ab.
   - Verbesserte Audio-Wiedergabe für FM-TOWNS/PC-98.
   - Unterstützung für Griechisch in der Benutzeroberfläche verbessert.

 Netzwerk:
   - Verbesserte Fehlerbehandlung.
   - Spielstände werden nur wenn nötig heruntergeladen.

 Bladerunner:
   - Fehlerhafte Savestates in einigen Szenen korrigiert.
   - Unterstützung für 120 FPS hinzugefügt.
   - Unterstützung für Thumbnails verbessert.
   - Unterstützung für virtuelle Keyboards im Speicherbildschirm auf betroffenen Plattformen hinzugefügt.

 Kyra:
   - End-Sequenz der Amiga-Version von EOB1 korrigiert (wird nur abgespielt, wenn alle Bonus-Quests erfüllt sind.
   - Die Häufigkeit, mit der Monster in EOB1 Gegenstände fallen lassen wurde korrigiert.
   - Geheimer Geheimtrank in Legend of Kyrandia 2 kann nun genutzt werden.
   - Sound-Probleme in Legend of Kyrandia 2 korrigiert.
   - Grafikfehler in Legend of Kyrandia 1 und 3, EOB1 und den FM-TOWNS-Versionen von EOB1 und EOB2 korrigiert.

 Mohawk:
   - Änderungen in den Spiele-Optionen werden nun über Sitzungen hinweg gespeichert.

 Queen:
   - Neu aufgetretener Fehler im Zusammenhang mit dem Dialog des Portiers behoben.

 SCI:
   - Zahlreiche Skript-Korrekturen in CAMELOT, ECO2, GK1, GK2, KQ7,
     PHANT1, PQ1VG, QFG3, QFG4, SQ5 und SQ6.
   - Unterstützung für den zensierten Modus in Phantasmagoria 1 hinzugefügt.
   - Unterstützung für die polische Version von LSL7 hinzugefügt.
   - Unterstützung für die italienische Version von GK2 hinzugefügt.
   - Unterstützung für die portugiesische Version von GK2 hinzugefügt.
   - Unterstützung für die russische Version von KQ7 hinzugefügt.
   - Unterstützung für die russische Version von SQ1VGA hinzugefügt.
   - Unterstützung für von Fans erstellte Untertitel für GK2 hinzugefügt.

 SCUMM:
   - Unterstützung für Pajama2 (UK-Version) hinzugefügt.

 Supernova:
   - F5 kann nun im Hauptmenü genutzt werden.

 Toltecs:
   - Unterstützung für die tschechische Version hinzugefügt.
   - Fehler behoben, der dazu geführt hat, dass Spiele-Menüs bei der Rückker zum Launcher nicht geschlossen wurden.

 Wintermute:
   - Mehrere fehlende Spiel-Varianten und Demos zu den Erkennungstabellen hinzugefügt.
   - Neu aufgetretener Fehler im Zusammenhang mit dem Stack-Handling behoben.
   - Das Verhalten von Edit-Boxen wurde korrigiert.
   - Verbesserung der Unterstützung von chinesischen Spielversionen.

 Xeen:
   - Anzeige von Goldmünzen und Edelsteinen auf der Nintendo Switch korrigiert.
   - Ausgänge zur Taverne in Swords of Xeen korrigiert.
   - Absturz beim Laden von Deep Mine Alpha in der CD-Version von World of Xeen behoben.

 GUI:
   - Der Reiter zum Anpassen der MIDI-Optionen wird nicht mehr angezeigt, wenn das Spiel kein MIDI unterstützt.

 Alle Portierungen:
   - Bildschirmdarstellung in einigen Bildschirm-Modi korrigiert.

 3DS-Portierung:
   - Erhebliche Verbesserungen.

 AmigaOS4-Portierung:
   - Kleinere Verbesserung (stack cookie, Build-Automatisierung).

 Android-Portierung:
   - Verbesserung der Dateisystem-Navigation.
   - HiDPI-Displays werden nun korrekt unterstützt.
   - Verbesserung der Keyboard-Unterstützung.

 iOS-Portierung:
   - Der Startbildschirm-Indikator wird nun automatisch auf dem iPhone X und späteren Modellen versteckt.

 MacOS X-Portierung:
   - Verbesserte Unterstützung für den "Dark Mode".

 RISC OS-Portierung:
   - Absturz behoben, der auftritt, wenn auf ein nicht verfügbares Laufwerk zugegriffen wird.
   - Versions-Anforderungen für DigitalRenderer reduziert.

 Switch-Portierung:
   - Cloud-Integration hinzugefügt.

 OpenPandora-Portierung:
   - Kleinere Verbesserungen.

 Windows-Portierung:
   - Unterstützung für Sprachausgabe in der Benutzeroberfläche (Text-to-Speech) hinzugefügt.
   - Bildschirmfotos können nun auch in Ordnern mit Unicode-Zeichen gespeichert werden.


#### 2.1.0 "Electric Sheep" (13.10.2019)

 Neue Spiele:
   - Unterstützung für Blade Runner hinzugefügt.
   - Unterstützung für Duckman: The Graphic Adventure of a Private Dick hinzugefügt.
   - Unterstützung für Hoyle Bridge hinzugefügt.
   - Unterstützung für Hoyle Children's Collection hinzugefügt.
   - Unterstützung für Hoyle Classic Games hinzugefügt.
   - Unterstützung für Hoyle Solitaire hinzugefügt.
   - Unterstützung für Hyperspace Delivery Boy! hinzugefügt.
   - Unterstützung für Might and Magic IV - Clouds of Xeen hinzugefügt.
   - Unterstützung für Might and Magic V - Darkside of Xeen hinzugefügt.
   - Unterstützung für Might and Magic - World of Xeen hinzugefügt.
   - Unterstützung für Might and Magic - World of Xeen 2 CD Talkie (mit Sprachausgabe) hinzugefügt.
   - Unterstützung für Might and Magic - Swords of Xeen hinzugefügt.
   - Unterstützung für Mission Supernova Teil 1: Das Schicksal des Horst Hummel hinzugefügt.
   - Unterstützung für Mission Supernova Teil 2: Der Doppelgänger hinzugefügt.
   - Unterstützung für Quest for Glory: Shadows of Darkness hinzugefügt.
   - Unterstützung für The Prince and the Coward hinzugefügt.
   - Unterstützung für Versailles 1685 hinzugefügt.

 Neue Portierungen:
   - Portierung auf Nintendo Switch hinzugefügt.

 Allgemein:
   - Verbesserte Performance der grafischen Benutzeroberfläche.
   - Skalier-Optionen hinzugefügt, die steuern, wie das ScummVM-Fenster auf
     dem Bildschirm dargestellt wird.
   - Fehlerhafte Steuerung des Mauszeigers korrigiert, wenn dieser über die Tastatur
     gesteuert wird.
   - Code der Roland MT-32-Emulation auf Munt 2.3.0 aktualisiert.
   - Unbekannte Spiele-Versionen können nun einfacher gemeldet werden.
   - Spielstände können nun in einer Cloud synchronisiert werden.
   - Unterstützung für Text-to-Speech als Bedienungshilfe auf einigen Plattformen hinzugefügt.

 ADL:
   - Verbesserung der Farbtreue.
   - TV-Emulationsmodus hinzugefügt.
   - Unterstützung für das WOZ-Dateiformat hinzugefügt.

 Drascula:
   - Fehler beim Laden eines in Kapitel 1 gespeicherten Spielstands korrigiert.

 Full Pipe:
   - Die Spielzeit wird nun korrekt wiederhergestellt, wenn ein Spielstand geladen wird.
   - Fehler behoben, der zu einem enormen Speicherverbrauch in Szene 22 führt.

 Kyra:
   - Unterstützung für die Amiga-Version von Eye of the Beholder I + II hinzugefügt.
   - Unterstützung für die FM-Towns-Version von Eye of the Beholder II hinzugefügt.
   - Mehrere Fehlerkorrekturen.

 MOHAWK:
   - Hauptmenü für die 25th Anniversary Edition von Myst ME hinzugefügt.
   - Hauptmenü für die 25th Anniversary Edition von Riven hinzugefügt.
   - Automatisches Speichern in Speicherplatz 0 für Myst und Riven hinzugefügt.
   - Tastenkürzel für das Laden und Speichern von Spielständen wie im Handbuch
     von Myst und Riven dokumentiert hinzugefügt.
   - Absturz im Observatorium in Myst behoben.
   - Absturz auf Jungle Island in Riven behoben.
   - Benutzerfreundlichkeit für einige Puzzles in Myst und Riven verbessert.
   - Korrektur mehrerer Abstürze, Grafik- und Sound-Fehler in Myst und Riven.

 Mortevielle:
   - Unterstützung für Sprach-Synthese auf einigen Plattformen hinzugefügt.

 SCI:
   - Unterstützung für LarryScale, eine qualitativ hochwertige Skalierung für Leisure Suit Larry 7, hinzugefügt.
   - Korrektur von über 100 Fehlern in den Original-Skripten von CAMELOT, ECO1, ECO2,
     FREDDYPHARKAS, GK1, HOYLES, ICEMAN, KQ6, LB1, LB2, LONGBOW, LSL6,
     MOTHERGOOSE256, PQ3, PQ4, QFG1VGA, QFG4 und SQ4.
   - Ein Fehler in ScummVM 2.0.0, der zu Lade-Problemen bei den Macintosh-Versionen von
     Freddy Pharkas, King's Quest 6 und Quest for Glory 1 (VGA) führte, wurde behoben.
   - Absturz in der Macintosh-Version von Freddy Pharkas behoben. Das Spiel kann nun abgeschlossen werden.
   - Fehler beim Laden automatisch gespeicherter Spielstände behoben.

 SCUMM:
   - Lippensynchronisation für v6- und v7+-Spiele hinzugefügt.
   - Verbesserung der Audio-Qualität in den Spielen von Humongous Entertainment durch Verwendung
     des Miles AdLib-Treibers.
   - Potenzielle Stabel-Überläufe in The Dig und Full Throttle behoben.
   - Sprachausgabe in der U-Boot-Szene von Indiana Jones and the Fate of Atlantis korrigiert.
     Benutzer müssen nun die Datei monster.sou mit einer aktuellen Version der scummvm-tools
     neu erstellen, damit diese Fehlerkorrektur auch bei komprimierten Audiodateien greift.
   - Fehler in der deutschen Version von SPY Fox 3: Operation Ozone korrigiert. Das Spiel kann
     nun komplett durchgespielt werden.
   - Sound-Treiber für die Amiga-Versionen von Monkey Island 2 und Indiana Jones and the Fate of Atlantis hinzugefügt.

 Sherlock:
   - Absturz in der spanischen Version behoben, der auftritt, wenn mit der Dame im Geschäft des
     Schneiders gesprochen wird.

 SKY:
   - Workaround für einen Fehler des Originalspiels hinzugefügt, der die Darstellung des Intros
     verbessert. Bilder werden nun nicht mehr abgeschnitten, sondern korrekt dargestellt, da diese
     im Vollbild-Format (320x200px) in den Spieldaten enthalten sind.

 Tinsel:
   - Fehler beim Laden von Spielständen in Discworld 1 korrigiert, der auftritt, wenn Rincewind einen Gegenstand trägt.
   - Fehler korrigiert, der einen Absturz in Discworld 1 verursachen kann.
   - In Discworld 1 werden Gegenstände, die abgelegt werden und sich nie im Gepäck oder in Rincewinds Inventar
     befanden, nun korrekt im Gepäck abgelegt, anstatt verloren zu gehen.

 Titanic:
   - Fehler korrigiert, der das Spiel zum Absturz bringen kann, wenn ein Stockwerk manuell angewählt wird.
   - Der Text-Parser wird nun zwischen den einzelnen Sätzen einer Unterhaltung korrekt zurückgesetzt.
   - Fehler korrigiert, der zu einer fehlerhaften Darstellung des Mauszeigers führen kann.
   - Fehler korrigiert, der zu einer Animations-Schleife des Barbots führen kann, wenn vor ihm ein Spielstand geladen wurde.
   - Fehler korrigiert, der zum Absturz führen kann, wenn der Papagei danach gefragt wird, wer das Schiff sabotiert hat.

 Tucker:
   - Mehrere Grafikfehler in Bud Tucker in Double Trouble behoben.
   - Mehrere Probleme mit der Darstellung von Zeichen und Untertiteln behoben.
   - Fehlerhafte Sound-Effekte korrigiert.
   - Die Blasen im "Plugs Shop" sind nun immer sichtbar.
   - Fehlende Animation hinzugefügt, wenn sich Ego und Billie auf dem Boot befinden.
   - Fehler korrigiert, der in der zweiten Museums-Szene zu einer Sackgasse führen kann.
   - Bud kann sich nun nicht mehr außerhalb des begehbaren Bereichs bewegen, wenn er den Club besucht.
   - Bud kann außerdem nicht mehr durch geschlossene Türen gehen.
   - Im Inventar kann nun das Mausrad zum scrollen verwendet werden.
   - Die Sprachausgabe kann nun übersprungen werden.
   - Verbesserte Behandlung von Spielständen sowie Unterstützung für automatisches Speichern hinzugefügt.

 ZVISION:
   - Grafikfehler in Zork: Grand Inquisitor behoben.
   - Benötigte Schriftarten werden nun zusammen mit ScummVM ausgeliefert.

 Android-Portierung:
   - Die Android-Portierung verwendet nun den OpenGL Grafik-Manager.
   - Button zum Einblenden der virtuellen Tastatur hinzugefügt.
   - Unterstützung für die Zwischenablage hinzugefügt.
   - Der Touchpad-Modus kann nun über eine eigene GUI-Option aktiviert werden.
   - Externe Speichermedien können nun genutzt werden.

 iOS-Portierung:
   - Unterstützung für Smart Keyboard hinzugefügt.
   - Drei-Finger-Wischgeste hinzugefügt, um Pfeiltasten zu simulieren.
   - Zoom-Gesten hinzugefügt, um die virtuelle Tastatur ein- und auszublenden.
   - Zusätzliche Tastensymbole, die nicht auf der Tastatur vorhanden sind, werden oberhalb der Tastatur eingeblendet.

 macOS-Portierung:
   - Anstelle des ScummVM-Dateibrowsers kommt nun der Betriebssystem-Dateibrowser zum Einsatz.
   - Die Dokumentation kann nun über das Hilfe-Menü aufgerufen werden.

 PS Vita-Portierung:
   - Unterstützung für Touch-Steuerung hinzugefügt.

 PSP-Portierung:
   - Unterstützung für Seitenverhältniskorrektur hinzugefügt.
   - Bewegung des Mauszeigers ist nun gleichmäßiger.
   - Einstellungsmöglichkeit für Mauszeiger-Geschwindigkeit und Joystick-Deadzone hinzugefügt.

 SDL-Portierungen (einschließlich Windows, Linux und macOS):
   - Unterstützung für Gamecontroller hinzugefügt.
   - Spiele können nun via Drag-and-Drop zu ScummVM hinzugefügt werden.

 Windows-Portierung:
   - Anstelle des ScummVM-Dateibrowsers kommt nun der Betriebssystem-Dateibrowser zum Einsatz.

 RISC OS-Portierung:
   - StrongHelp-Handbuch hinzugefügt.
   - Fehler können nun über !Reporter gemeldet werden.

#### 2.0.0 (17.12.2017)

 Neue Spiele:
   - Unterstützung für Full Pipe hinzugefügt.
   - Unterstützung für Hi-Res Adventure #3: Cranston Manor hinzugefügt.
   - Unterstützung für Hi-Res Adventure #4: Ulysses and the Golden Fleece hinzugefügt.
   - Unterstützung für Hi-Res Adventure #5: Time Zone hinzugefügt.
   - Unterstützung für Hi-Res Adventure #6: The Dark Crystal hinzugefügt.
   - Unterstützung für Riven hinzugefügt.
   - Unterstützung für Starship Titanic (Raumschiff Titanic) hinzugefügt.

 Neue Spiele (Sierra SCI2 - SCI3):
   - Unterstützung für Gabriel Knight hinzugefügt.
   - Unterstützung für Gabriel Knight 2 hinzugefügt.
   - Unterstützung für King's Quest VII hinzugefügt.
   - Unterstützung für King's Questions hinzugefügt.
   - Unterstützung für Leisure Suit Larry 6 (hires) hinzugefügt.
   - Unterstützung für Leisure Suit Larry 7 hinzugefügt.
   - Unterstützung für Lighthouse hinzugefügt.
   - Unterstützung für Mixed-Up Mother Goose Deluxe hinzugefügt.
   - Unterstützung für Phantasmagoria hinzugefügt.
   - Unterstützung für Phantasmagoria 2 hinzugefügt.
   - Unterstützung für Police Quest 4 hinzugefügt.
   - Unterstützung für RAMA hinzugefügt.
   - Unterstützung für Shivers hinzugefügt.
   - Unterstützung für Space Quest 6 hinzugefügt.
   - Unterstützung für Torin's Passage hinzugefügt.

 Neue Portierungen:
   - Portierung für PSP Vita hinzugefügt.
   - Portierung für RISC OS hinzugefügt.

 Allgemein:
   - Bilineare Filterung für den SDL2-Vollbildmodus hinzugefügt.
   - Fehler behoben, der einen Absturz im Optionen-Dialog verursacht.
   - Kommandozeilen-Option hinzugefügt, mit der das aktuelle oder ein gewähltes
     Verzeichnis nach unterstützten Spielen durchsucht werden kann.
   - Viele Optionen der Benutzeroberfläche können nun übernommen werden, ohne
     den Optionen-Dialog zu schließen.
   - Sprache der Benutzeroberfläche kann ohne Programmneustart geändert werden.
   - Integrierter Munt MT-32-Emulator auf Version 2.0.3 aktualisiert.
   - Verbesserte Joystick-Behandlung.
   - Verbesserte Audio-Latenz.
   - Verbesserte Verwaltung des ScummVM-Fensters in Spielen, die den Bildschirm-Modus
     wechseln.
   - Darstellung von Listen verbessert (z.B. im Speichern-Dialog)
   - Speicherort für Bildschirmfotos geändert. Dies behebt Probleme, die auftreten,
     wenn ScummVM in einem Verzeichnis ohne Schreibrechte installiert ist. Ebenfalls
     wurde eine Einstellung hinzugefügt, mit der dieser Speicherort verändert werden kann.
   - Fehler im Multithreading behoben, der zu einem Absturz in Spielen mit MP3-Audio führen kann.

 ADL:
   - Fehler behoben, der zum Einfrieren von ScummVM führt, wenn das Schild in der Rakete
     in Mission Asteroid gelesen wird.

 AGI:
   - Fehler behoben, der verursacht, dass Spiel-Skripte blockiert werden, nachdem ein Spielstand
     geladen wird, der gespeichert wurde, während Musik abgespielt wurde (dieser Fehler kann z.B.
     im Poker-Hinterzimmer in Police Quest 1 auftreten).
   - Fehlerhaftes Cursor-Verhalten in Manhunter korrigiert.
   - Geschwindigkeit der Nachtclub-Arcade-Sequenz in der Apple IIgs-Version von Manhunter korrigiert.
   - Maximale Spielgeschwindigkeit auf 40 FPS begrenzt, um sicherzustellen, dass das Spiel nicht zu schnell läuft.

 AGOS:
   - Einstellung für die Untertitel-Geschwindigkeit in der hebräischen Version von
     "Simon the Sorcerer 1" korrigiert.

 Composer:
   - Unterstützung für das Speichern und Laden von Spielständen aus dem Hauptmenü heraus hinzugefügt.
   - Die französische Version von Gregory wird nun korrekt erkannt.
   - Die deutsche Version von Baba Yaga wird nun korrekt erkannt.

 Cruise:
   - Schrift-Darstellung korrigiert.

 Drascula:
   - Fehler behoben, der es unmöglich gemacht hat, im Gasthaus mehr als einmal mit dem
     Trunkenbold zu sprechen.
   - Lautstärke-Regelung hinzugefügt und Lautstärke-Synchronisation zwischen dem Spiel
     und den ScummVM-Einstellungen korrigiert.
   - Spielstände können jetzt mithilfe des GMM gespeichert und geladen werden.

 Dreamweb:
   - Fehler behoben, der zu einem Absturz führt, wenn die letzten Steine unter der Kirche
     eingesammelt werden.
   - Die italienische CD-Version wird nun korrekt erkannt.

 Kyra:
   - Pufferüberlauf in Lands of Lore behoben.
   - Absturz aufgrund fehlender Farbpaletten-Daten in der Diskettenversion von Legend of
     Kyrandia behoben.

 MADE:
   - Fehlerhafte Audio-Wiedergabe behoben (Fehler #9753).

 MADS:
   - Fehler behoben, der zu einem Absturz nach dem Start von Rex Nebular and the Cosmic
     Gender Bender führt.
   - Fehler behoben, der zu einem Absturz führen kann, wenn Rex gefangen genommen wird

 MOHAWK:
   - Verbesserungen an den originalen Spieldaten vorgenommen, um die Anweisungen
     zum Öffnen des Tresors in Myst ME zu korrigieren.
   - Situationen korrigiert, in denen Myst scheinbar nicht mehr reagiert.
   - Behandlung der Audio-Wiedergabe in Myst überarbeitet und verbessert.
   - Absturz während des Klavier-Rätsels in Myst behoben.

 Neverhood:
   - Absturz im Musikhaus in der russischen DR-Version behoben.
   - Absturz im späteren Spiel in der russischen DR-Version behoben.

 Pegasus:
   - Fehler beim Laden eines Spiels aus dem Launcher korrigiert.
   - Einige Events werden nun ignoriert, während die ScummVM-Oberfläche sichtbar ist.
     Damit wird beispielsweise korrigiert, dass das Spieleigene Menü geöffnet wird,
     wenn der GMM mittels ESC geschlossen wird.
   - Mehrere Abstürze im Zusammenhang mit dem gemeinsamen Bildschirmplatz korrigiert.
   - Verbesserte Performance beim Ausblenden des Bildschirms.

 SAGA:
   - Absturz behoben, der Auftritt, wenn das Verb "Gib" in IHNM verwendet wird.
   - Gorrister ist nun nicht mehr unsichtbar und hängt nicht mehr fest, wenn
     ein Spiel am Liegeplatz in IHNM geladen wird.
   - Hintergrundfarbe des Gesprächs-Feldes in IHNM korrigiert.
   - Unterstützung für die französische Fan-Übersetzung von Inherit the Earth hinzugefügt.

 SCI:
   - Script-Fehler in Laura Bow 2: Dagger of Amon Ra korrigiert, der es unmöglich
     machte, den Party-Raum mit dem großen goldenen Kopf im Museum (Raum 350) zu verlassen.
     Dieser Fehler tritt auch auf, wenn der originale Interpreter verwendet wird.
   - Verbesserte Ladezeit bei Verwendung des MT-32-Emulators.
   - Verbesserte Behandlung von MT-32-Halleffekten in SCI0-Spielen.
   - Verbesserte Auswahl synthetisierter Soundeffekte in SCI0-Spielen.
   - Verbesserte Auswahl der digitalen Audio-Ausgabe in SQ4.
   - Verbesserte Ressourcenverwaltung.
   - Verbesserte Fehlerbehandlung fehlerhafter MIDI-Daten.
   - Fehler behoben, der die Speicherstände mit der Zeit mit unnötigen Daten gefüllt hat.
   - Fehlerhafter Tag/Nacht-Rhythmus in QFG3 behoben.
   - Skript-Fehler in Police Quest 3 behoben. Wenn der Spieler Marie das Medaillon
     übergibt, erhält er dafür 10 Punkte. Damit ist es jetzt möglich, das Spiel mit
     einer perfekten Punktzahl zu beenden. Dieser Fehler ist auch im Originalspiel
     vorhanden.
   - Verschiedene andere Skript-Fehler wurden behoben.
   - Verbesserung der Lautstärke-Einstellungen.

 SCUMM:
   - Absturz in Amiga-Spielen behoben.
   - Fehler behoben, der verursacht, dass zwei Soundtracks gleichzeitig in Monkey Island 2
     abgespielt werden.
   - Fehler behoben, der verursacht, dass sich die Caponier nicht tarnen, wenn der blaue
     Kristall in Zak McKracken verwendet wird.
   - Fehler in Maniac Mansion behoben, der verursacht, dass Dr. Fred in der
     Labor-Zwischensequenz in die falsche Richtung schaut.
   - Fehler behoben, der verursacht, dass in V0 und V1-Spielen die Charaktere eine Zeile
     zu hoch dargstellt werden.
   - Fehler in Maniac Mansion behoben, der verursacht, dass Purpur-Tentakel im Labor-Eingang
     erscheint, nachdem es davongejagt wurde.
   - Fehler in Maniac Mansion behoben, der verursacht, dass der Strom nicht wieder eingeschaltet
     wird, wenn das Labor betreten wird, während Dr. Fred den Strom abgeschaltet hat.
   - Fehler in Maniac Mansion behoben, der zu einer fehlerhaften Darstellung von Charakteren
     führen kann.

 Sherlock:
   - Die italienische Fan-Übersetzuhng von Serrated Scalpel wird nun korrekt erkannt.

 Sky:
   - Kollisionsabfrage korrigiert.

 Sword1:
   - Vorschaubild für Spielstände hinzugefügt, die über den spieleigenen Dialog angefertigt werden.
   - Fehler korrigiert, der dazu führt, dass die Audio- und Untertitel-Einstellungen geändert
     werden, wenn der spieleigene Laden/Speichern-Dialog geöffnet wird.

 Tinsel:
   - Fehler in Discworld 2 behoben, der verursacht, dass der Text und die Sprachausgabe
     nicht durchgehend angezeigt bzw. abgespielt wird.
   - Absturz im Spiel behoben, der auftritt, wenn alle Spielstand-Plätze mit langen
     Namen verwendet werden.

 TsAGE:
   - Kürzlich eingeführten Fehler korrigiert, der verhindert, dass die Animationen in
     Return to Ringworld abgespielt werden.
   - Darstellungsfehler in der Demo-Version von Return to Ringworld behoben.
   - Möglichen Fehler beim Laden von Spielstände in Return to Ringworld behoben.
   - Fehler im Audio-Code behoben.
   - Absturz bei der Rückkehr zum Launcher korrigiert.

 Voyeur:
   - Fehler behoben, der verursacht, dass Hintergründe in statischen Räumen nicht dargestellt werden.
   - Audio-Wiedergabe in VCR korrigiert.
   - Fehler behoben, der verhindert, dass das Spiel aus dem VCR-Bildschirm heraus verlassen werden kann.
   - Workaround für einen Fehler im Original-Spiel hinzugefügt, der zur Verwendung ungültiger
     Hotspot-IDs führt.

 macOS X-Portierung:
   - Verbundene MIDI-Geräte können jetzt manuell ausgewählt werden. Es wird nicht mehr
     automatisch das erste Gerät ausgewählt.
   - Verhalten des Datei-Browsers verbessert.

#### 1.9.0 "Myst-ery U.F.O.s release" (17.10.2016)

 Neue Spiele:
   - Unterstützung für Myst hinzugefügt.
   - Unterstützung für Myst: Masterpiece Edition hinzugefügt.
   - Unterstützung für U.F.O.s./Gnap: Der Schurke aus dem All hinzugefügt.
   - Unterstützung für Hi-Res Adventure #0: Mission Asteroid hinzugefügt.
   - Unterstützung für Hi-Res Adventure #1: Mystery House hinzugefügt.
   - Unterstützung für Hi-Res Adventure #2: The Wizard and the Princess hinzugefügt.

 Allgemein:
   - Probleme mit dem MS ADPCM-Decoder behoben.
   - Fehlerhafte Tonhöhe der CMS/GameBlaster-Emulation korrigiert.
   - SDL-Backend verwendet jetzt SDL2 als Standard. SDL1 ist eine Rückfalloption.

 AGI:
   - Unterstützung für Hercules-Darstellung (Grün + Bernstein) hinzugefügt.
   - Unterstützung für hochauflösende Hercules-Schriftart hinzugefügt
     (auch außerhalb der Hercules-Darstellung nutzbar).
   - Optionale Funktion "Pause, wenn Befehle eingegeben werden" hinzugefügt.
     Diese Funktion war im originalen Interpreter nur im Hercules-Darstellungsmodus
     verfügbar.

 Beneath a Steel Sky:
   - Fehlerhafte Animation für Officer Blunt behoben, die dafür sorgte, dass das
     Spiel nicht komplett durchgespielt werden konnte, da eine weitere Interaktion mit
     diesem Charakter nicht möglich war.

 Gob:
   - Grafikfehler in Gobliiins (EGA-Version) behoben.

 Kyra:
   - Italienische Übersetzung von EOB1 aktualisiert.
   - Fehler behoben, der einen Absturz in "Lands of Lore" verursacht.

 SCI:
   - Fehlende Dialogzeile in QfG3 hinzugefügt, die mit drei zusätzlichen Punkten
     belohnt wird. Diese Dialogzeile fehlt im Originalspiel, weshalb
     es bislang unmöglich war, die maximale Punktzahl im Spiel zu erreichen.
   - Fehler in "Space Quest 1" behoben, der Probleme mit dem Spinnenroboter verursachte.
   - Fehler in "Laura Bow: The Colonel's Bequest" behoben, der einen Absturz verursachte,
     wenn mit der Rüstung in Raum 37 (Haupthaus, Untergeschoss) interagiert wird. Dieser
     Fehler existiert auch im Originalspiel.
   - Automatisches Speichern im Fan-Spiel "Cascade Quest" repariert.
   - Fehler in "Conquest of the Longbow: The Adventures of Robin Hood" behoben, der einen
     Absturz verursacht, wenn sich der Spieler durch den Sherwood Forest bewegt.
   - Unterstützung für die Demo-Version des "ImagiNation Network (INN)" hinzugefügt.

 SCUMM:
   - Fehlende Übersetzungen in den "Beenden"- und "Neustart"-Dialogen in "Pyjama Sam 1" behoben.
   - Grafikfehler in DOTT behoben, die auftraten, wenn ein Spielstand geladen wurde, in dem
     die Stereoanlage in Grüntentakels Zimmer eingeschaltet ist.
   - Timing und Wegfindung in "Maniac Mansion (C64- und Apple II-Version)" verbessert.
   - Unterstützung für niederländische Demo von Let's Explore the Airport with Buzzy hinzugefügt.

 Sherlock:
   - Fehler behoben, der einen Absturz in "The Case of the Serrated Scalpel" verursachen konnte.
   - Problem mit Item-Hotspots in "The Case of the Serrated Scalpel" behoben.
   - Fehler behoben, der in "The Case of the Rose Tattoo" dazu führt, dass sich das Spiel aufhängt.

 Amiga-Portierung:
   - Unterstützung für automatische Aktualisierungen über AmiUpdate hinzugefügt.

 Linux-Portierung:
   - Grundlegende Unterstützung für das snap-Paketierungssystem hinzugefügt.

 Windows-Portierung:
   - Taskleisten-Integration unter Windows 10 und höher repariert.
   - Tastenbelegung für Tastaturen korrigiert, die keine QWERTY-Tastaturen sind.
   - Unterstützung für automatische Aktualisierungen über WinSparkle hinzugefügt.


#### 1.8.1 "Where Is Your Android?" (25.05.2016)

 Neue Portierungen:
   - Portierung für den Nintendo 3DS hinzugefügt.
   - Portierung für Android SDL hinzugefügt.

 Allgemein:
   - "TESTING"-Markierung von mehreren unterstützten Spielen entfernt.
   - Chinesische Übersetzung (Pinyin) der Benutzeroberfläche hinzugefügt.
   - Ruckeln des Mauszeigers im ScummVM-Programmfenster behoben, welches auf
     einigen Systemen auftrat.

 BBVS:
   - Fehler beim erneuten Starten des Spiels behoben.

 CinE:
   - Fehler beim Laden der Soundeffekte behoben.

 Drascula:
   - Text-Ausrichtung ist jetzt originalgetreu.
   - Charakter tritt nicht mehr aus dem Bildschirmbereich heraus.
   - Laden eines Spielstandes in der "Pendulum"-Szene repariert.
   - Falscher Hintergrund für Inventar-Gegenstände im Kapitel 6 in der
     spanischen Version korrigiert.
   - Geschwindigkeit der Animationen korrigiert. Animationen wurden nur halb
     so schnell wie im originalen Interpreter abgespielt.
   - Rauschen am Beginn und/oder am Ende der Sprachausgabe behoben.
     Dieser Fehler trat hauptsächlich in der spanischen Version auf.
   - Verzögerung während der Interaktion mit dem Verben-Menü und dem Inventar behoben.
   - Fehler behoben, durch den die Axt im Schloss mehrfach aufgehoben werden konnte.

 Gob:
   - Aufhängen während Sound-Initialisierung in mehreren Spielen behoben.

 KYRA:
   - Potentieller Absturz behoben, der in "Hand of Fate" auftritt, wenn der
     Sumpfschlangentrank an der Ratte verwendet wird.
     (HINWEIS: Dieser Fehler wurde bereits in Version 1.8.0 behoben,
     jedoch nicht in der Neues-Datei erwähnt).
   - Fehlende Stimm-Reaktionen korrigiert, wenn Gegner in der CD-Version von
     Lands of Lore getroffen wurden.

 Lab:
   - Aufhängen während der End-Sequenz behoben.
   - Interne Spiel-Bedienelemente verbessert.
   - Aufhängen bei einigen Animationen im Spiel behoben.

 SAGA:
   - Fehlerhafte Farben der Bedienelemente in der französischen und deutschen
     Version von "I Have No Mouth and I Must Scream" korrigiert.

 SCI:
   - Cursor-Hilfsroutinen funktionieren nun korrekt auf OpenPandora und anderen
     Geräten, die einen Touchscreen und analoge Sticks/Mäuse zur gleichen Zeit
     unterstützen.
   - Skript-Fehlerbehebung, um den fehlerhaften Endkampf in der mehrsprachigen
     Version von King's Quest 5 zu korrigieren. Betroffen sind die französische,
     deutsche und spanische Version.
   - Ungültiger Speicherzugriff beim Laden der defekten Audiospur im Abspann
     von King's Quest 5 behoben.
   - Probleme mit der Einstellung der Bildschirmauflösung beim Speichern in
     King's Quest 6 behoben.

 SCUMM:
   - Erkennung von Maniac Mansion innerhalb von Day of the Tentacle in der
     Windows-Version von ScummVM repariert.
   - In der EGA-Version von Loom wurde ein Sound-Effekt nicht korrekt angehalten,
     wenn AdLib verwendet wurde. Dieser Fehler wurde behoben.

 Baphomets Fluch 2.5:
   - Option zur Auswahl von englischer Sprachausgabe anstelle der deutschen,
     wenn in der gewählten Sprache keine Sprachausgabe verfügbar ist, hinzugefügt.
   - Ressourcen-Freigabe beim Beenden des Spiels korrigiert.
   - Fehler beim Neustart des Spiels nach dem Wechsel der Spiel-Sprache behoben.
   - Flackern im Hauptmenü behoben.
   - Lange Dauer des Speichervorgangs unter Windows behoben.

 Windows-Portierung:
   - Absturz im Zusammenhang mit nicht-funktionierenden MIDI-Geräten behoben.

 Mac OS X-Portierung:
   - Das Dock-Menü für ScummVM enthält nun eine Liste der zuletzt gespielten Spiele,
     wenn ScummVM nicht läuft, und ermöglicht den direkten Start dieser Spiele.
   - Sparkle-Updater für vereinfachte Programmaktualisierungen hinzugefügt.

 GCW0-Portierung:
   - Verbesserte Unterstützung für die in ScummVM integrierte Dokumentation.

#### 1.8.0 "Lost with Sherlock" (04.03.2016)

 Neue Spiele:
   - Unterstützung für Rex Nebular and the Cosmic Gender Bender hinzugefügt.
   - Unterstützung für Sfinx hinzugefügt.
   - Unterstützung für Zork Nemesis: Das verbotene Land hinzugefügt.
   - Unterstützung für Zork: Der Großinquisitor hinzugefügt.
   - Unterstützung für Die ungelösten Fälle von Sherlock Holmes: Das gezackte
     Skalpell hinzugefügt.
   - Unterstützung für Die ungelösten Fälle von Sherlock Holmes: Das Geheimnis
     der tätowierten Rose hinzugefügt.
   - Unterstützung für Beavis and Butthead in Virtual Stupidity hinzugefügt.
   - Unterstützung für Amazon: Guardians of Eden hinzugefügt.
   - Unterstützung für Baphomets Fluch 2.5: Die Rückkehr der Tempelritter
     hinzugefügt.
   - Unterstützung für Labyrinth of Time hinzugefügt.

 Neue Portierungen:
   - Portierung für den Raspberry Pi hinzugefügt.
   - Portierung für den GCW Zero (GCW0) hinzugefügt.

 Allgemein:
   - Code für Munt-MT-32-Emulation auf Version 1.5.0 aktualisiert.

 SDL:
   - Alt+x beendet ScummVM nicht mehr. Verwenden Sie stattdessen
     Cmd+q/Strg+q/Strg+z und beachten Sie die Hinweise in der Liesmich-Datei.
   - Auf POSIX-Systemen befolgen wir nun die Spezifikation XDG Base Directory
     für die Speicherung von Benutzerdaten. Dies führt zu neuen
     Speicherorten für unsere Konfigurationsdatei, unsere Log-Datei sowie für
     den standardmäßig voreingestellten Speicherort für Spielstände. Wir
     unterstützen weiterhin die vorherigen Speicherorte. Solange diese vorhanden
     sind, werden wir diese weiter verwenden. Bitte beachten Sie die
     Liesmich-Datei für weitere Informationen. Speicherorte auf Mac OS X sind
     von dieser Änderung nicht betroffen.

 3 Skulls of the Toltecs:
   - Unterstützung für AdLib-Musik verbessert.

 AGI:
   - Es ist nun möglich, die Maus-Unterstützung zu deaktivieren (außer bei
     Amiga-Versionen und Fan-Spielen, die eine Maus benötigen).
   - Fehlerhafte Lautstärke in PCjr-Spielen korrigiert.
   - Umfangreiche Änderung im Grafik-Subsystem.
   - Unterstützung für Übergänge, Schriftarten und Mauszeigern für Apple IIgs,
     Amiga und Atari (die Systemschriftart Atari ST 8x8 ist nicht in ScummVM
     enthalten)
   - Eine PC-Version kann jetzt wie eine Apple IIgs-Version dargestellt werden
     (inklusive Farbpalette, Cursor, Übergänge und Schriftart). Sie müssen
     lediglich den gewünschten Darstellungsmodus auswählen.
   - Apple IIgs-Spiele laufen nicht mehr zu schnell.
   - Unterstützung für automatisches Speichern / Laden hinzugefügt
     (verwendet von Mixed Up Mother Goose).
   - Feste Verzögerung von 2 Sekunden bei Raumwechseln entfernt und durch
     Heuristik ersetzt.
   - Fehlerhafte Tastenbelegungen nach abspeichern/laden behoben.

 AGOS:
   - Arpeggio-Effekt in der Musik der Amiga-Version von Elvira 1 repariert.
   - Lade- und Speicherfortschritt in der PC-Version von Waxworks repariert.
   - Verb-Feld in der Amiga-Version von Simon the Sorcerer 1 repariert.
   - Accolade AdLib- und MT32-Treiber für folgende Spiele hinzugefügt:
     Elvira 1, Elvira 2, Waxworks und Simon the Sorcerer 1 (Demoversion)
   - AdLib-Ausgabe in Simon the Sorcerer 1 hinzugefügt. Dies verbessert die
     AdLib-Ausgabe erheblich und erhöht die Originaltreue.

 Baphomets Fluch 1:
   - Sprachausgabe in Macintosh-Versionen korrigiert, wenn ScummVM
     auf Big-Endian-Systemen ausgeführt wird.
   - Fehler beim Laden eines Spielstandes aus dem Hauptmenü in der
     Bull's Head Hill-Szene korrigiert. Dieser Fehler trat womöglich auch
     in anderen Szenen auf.

 CinE:
   - Unterstützung für Musik in der CD-Version von Future Wars hinzugefügt.

 MADE:
   - Unterstützung für AdLib-Musik in Return to Zork verbessert.

 SAGA:
   - Unterstützung für AdLib-Musik verbessert.

 SCI:
   - Behandlung der Musik-Priorität extrem verbessert.
   - Viele Fehler in den originalen Skripten behoben, die auch bei
     Verwendung des originalen Interpreters auftreten.
     Folgende Spiele sind davon betroffen:
     KQ6 (Sprache und Untertitel), LSL5, PQ1, QfG1 (EGA), QfG (VGA),
     QfG2, QfG3, SQ1, SQ4 (CD)
   - Rückkehr aus dem ScummVM-Menü im Spiel sollte nun immer funktionieren.
   - Verbesserte Unterstützung für japanische PC-9801-Spiele
   - Verwende standardmäßig die hochauflösende Version von KQ6
     (kann in den Spieloptionen umgeschaltet werden)

 SCUMM:
   - Umfangreiche Verbesserung der Textdarstellung in koreanischen Versionen
   - Originaler Code der Geh-Animation in Maniac Mansion v0-v1 hinzugefügt.
   - Es ist nun möglich, Maniac Mansion innerhalb von Day of the
     Tentacle zu spielen. Bitte Liesmich-Datei für weitere Details lesen.
   - Alt+x kann jetzt auf allen Plattformen dazu verwendet werden, SCUMM-Spiele
     zu beenden.
   - Lippensynchronisation in neueren Spielen von Humongous Entertainment verbessert.

 Tinsel:
   - Unterstützung für AdLib-Musik in Discworld 1 verbessert.

#### 1.7.0 "The Neverrelease" (21.07.2014)

 Neue Spiele:
   - Unterstützung für Chivalry is Not Dead hinzugefügt.
   - Unterstützung für Return to Ringworld hinzugefügt.
   - Unterstützung für The Neverhood hinzugefügt.
   - Unterstützung für Mortville Manor hinzugefügt.
   - Unterstützung für Voyeur hinzugefügt.

 Allgemein:
   - Munt-MT-32-Emulationscode zu Version 1.4.0 aktualisiert.
   - Von unseren eigenen JPEG- und PNG-Dekodieren zu libjpeg(-turbo) und libpng
     gewechselt, welche schneller sind und mehr Bilder verarbeiten können.
     (HINWEIS: Der Wechsel zu libpng fand bereits in Version 1.6.0 statt, wurde
     jedoch nicht in der Datei NEUES erwähnt.)
   - Allgemeine Ausgabe für OpenGL (ES) hinzugefügt (basierend auf
     GSoC-Aufgabe).
   - Die Benutzeroberfläche kann jetzt mit 32 Bit Farbentiefe gerendert werden.
   - Das Kompilierungssystem wurde baukastenartiger gemacht, damit es einfacher
     ist, neue Engines hinzuzufügen.

 SDL:
   - OpenGL-Grafikmodus hinzugefügt, der auf unserer allgemeinen OpenGL-Ausgabe
     basiert. Damit ist Grafikausgabe in beliebiger Größe möglich. Dieser
     unterstützt jedoch nicht Spezialfilter wie AdvMAME, HQ usw.

 AGOS:
   - Unterstützung für Mausrad bei Inventar und Spielstandliste hinzugefügt.
   - Anzeige der Interaktionsverben in Simon the Sorcerer 2 aktiviert.
   - Fehler bei Loyalitätseinstufung in englischer 4CD-Version von
     Floyd – Es gibt noch Helden beseitigt. (Dies war offensichtlich ein Fehler
     im ursprünglichen Spiel. Es ist zurzeit nicht bekannt, ob noch weitere
     Versionen hiervon betroffen sind.)

 Baphomets Fluch 1:
   - Unterstützung für MPEG-2-Videos wiederhergestellt.

 Baphomets Fluch 2:
   - Unterstützung für MPEG-2-Videos wiederhergestellt.

 CGE:
   - Option für einen „Farbenblindheitsmodus“ zum ScummVM-Menü hinzugefügt.

 Gob:
   - Video-Qualität in Urban Runner verbessert.

 Hopkins:
   - Option für einen Brutalo-Modus zu ScummVM-Menü hinzugefügt.
   - Fehler beseitigt, durch welchen die Musik manchmal vorzeitig zu spielen
     aufhörte.

 Pegasus:
   - Mehrere seltene Abstürze und Störungen beseitigt
   - Mehrere Fehler beseitigt, die aus der ausführbaren Datei des Originalspiels
     übernommen wurden.

 SCI:
   - Unterstützung für die detailreicheren RAVE-Lippensynchronisationsdaten in
     der Windows-Version von King’s Quest 6 hinzugefügt. Portraits wirken beim
     Sprechen nun viel ausdrucksstarker.
   - Unterstützung für gleichzeitige Sprachausgabe und Untertitel in den
     CD-Versionen von Laura Bow 2 und King’s Quest 6 hinzugefügt (kann sowohl im
     Spiel selbst über den neuen „Dual“-Audiostatus als auch über ScummVMs
     Audio-Optionen aktiviert werden).
   - Musikübergänge korrigiert.
   - Mehrere Skriptfehler in Camelot, Crazy Nick's, Hoyle 3, QFG1VGA, KQ5,
     KQ6, LB2, LSL2, LSL5, Pharkas, PQ1VGA, SQ4 und SQ5 beseitigt.
   - MIDI-Parser verbessert, sodass Musikereignisse genauer verarbeitet werden.

 SCUMM:
   - Spielstandsbenennungsschema von HE-Spielen geändert, sodass immer der
     Zielname des Spiels enthalten ist.
   - Fehler beseitigt, durch den man mehrere Trainer in Backyard Football hatte.
   - AdLib-Unterstützung für Loom und Indiana Jones and the Last Crusade
     verbessert. Dadurch klingen Geräusch-Effekte wie beispielsweise von der
     Schreibmaschine und vom Wasserfall wie im Original.
   - Unterstützung für Steam-Versionen von Indiana Jones and the Last Crusade,
     Indiana Jones and the Fate of Atlantis, Loom und The Dig hinzugefügt.
     Sowohl Windows- als auch Macintosh-Versionen werden unterstützt.

 TONY:
   - Spielstände von Tony Tough funktionieren nun auf Big-Endian-Systemen.

 Tinsel:
   - Discworld 1 und 2 stürzen nicht mehr auf Big-Endian-Systemen ab.

 Android-Portierung:
   - Experimentelle Unterstützung für die OUYA-Konsole hinzugefügt.

 PS2-Portierung:
   - Konfigurierbare TV-Modi hinzugefügt: NTSC und PAL.
   - Konfigurierbare Grafikmodi hinzugefügt: SDTV stufenweise, SDTV
     Zeilensprung, EDTV stufenweise und VESA.
   - Konfigurierbare Option für zu verwendende Festplattenpartition hinzugefügt
   - Konfigurierbare Option für zu verwendende IP-Adresse hinzugefügt.
   - Konfigurierbare Option zum Aktivieren und Deaktivieren von
     USB-Massenspeichergeräten hinzugefügt.

 Tizen-Portierung:
   - Die BADA-Portierung wurde mit Tizen zusammengeführt/aktualisiert.

#### 1.6.0 "+4 to engines" (31.05.2013)

 Neue Spiele:
   - Unterstützung für 3 Skulls of the Toltecs hinzugefügt.
   - Unterstützung für Eye of the Beholder hinzugefügt.
   - Unterstützung für Eye of the Beholder II: The Legend of Darkmoon
     hinzugefügt.
   - Unterstützung für Hopkins FBI hinzugefügt.
   - Unterstützung für Tony Tough and the Night of Roasted Moths hinzugefügt.
   - Unterstützung für The Journeyman Project: Pegasus Prime hinzugefügt.
   - Unterstützung für Macintosh-Version von Discworld 1 hinzugefügt.

 Allgemein:
   - Eine neue Spielstandauswahl hinzugefügt, die auf einem Raster kleiner
     Vorschaubilder basiert. Diese wird nur bei einer Auflösung von mindestens
     640x400 unterstützt. Die alte Spielstandauswahl ist weiterhin verfügbar und
     wird bei Spielen verwendet, die keine kleinen Vorschaubilder unterstützen.
     Es ist zudem möglich, die alte Spielstandauswahl als Voreinstellung
     festzulegen.
   - Untersystem für Videodekodierung umgeschrieben.
   - Galicische Übersetzung hinzugefügt.
   - Finnische Übersetzung hinzugefügt.
   - Weißrussische Übersetzung hinzugefügt.
   - Die Verwendung des Mausrades auf einem Schieberegler ändert nun den Wert im
     kleinstmöglichen Umfang. Dies ist berechenbarer als das alte Verhalten,
     nach welchem der Wert um „ein Pixel“ geändert wurde, wodurch er sich
     manchmal überhaupt nicht geändert hat.
   - MT-32-Emulationscode auf den neusten Stand des Munt-Projekt-Schnappschusses
     gebracht.
   - Fenster für FluidSynth-Einstellungen hinzugefügt, hauptsächlich für Hall-
     und Chor-Einstellungen.
   - Abstürze einiger Smacker-Videos beseitigt.

 Cine:
   - Audio-Unterstützung für Amiga und AtariST-Versionen von Future Wars
     verbessert. Musik wird nun langsam ausgeblendet anstatt abrupt zu stoppen.
     Die Balance-Regelung der Geräusch-Effekte funktioniert nun richtig, wenn
     sie vom Spiel verlangt wird.

 CGE:
   - Soltys enthält ein Puzzle, bei welchem man die ALT-Taste drücken muss,
     während man auf ein Objekt klickt. Dieses Puzzle wurde auf Geräten
     deaktiviert, auf denen diese Taste nicht verfügbar ist.

 Drascula:
   - Einige Probleme der Benutzeroberfläche im Zusammenhang mit dem Bildschirm
     zum Speichern und Laden gelöst.
   - Erweiterte Spielstandfunktionsweise hinzugefügt, einschließlich Zeitstempel
     und kleiner Vorschaubilder für Spielstände und die Möglichkeit, Spielstände
     aus der Spieleliste heraus zu laden und zu löschen. Es ist nun möglich, die
     Menüs von ScummVM zum Speichern und Laden zu verwenden.
   - Die Taste F7 (vorher ohne Funktion) zeigt nun immer das Lade-Menü von
     ScummVM. Die Taste F10 zeigt entweder das originale Lade-Menü oder das
     Lade-Menü von ScummVM, abhängig davon, ob der Benutzer eingestellt hat,
     dass die Menüs von ScummVM zum Speichern und Laden verwendet werden sollen.

 Dreamweb:
   - Da das Spiel nun Freeware ist, gibt es einen kleinen zusätzlichen
     Hilfetext, der die verfügbaren Befehle bei Terminals im Spiel aufzählt,
     wenn der Spiele den Befehl „help“ eingibt. Zuvor mussten Spieler die
     verfügbaren Befehle im Handbuch nachschlagen. Da dieser Bezug auf das
     Handbuch eine Art Kopierschutz ist, kann dieser zusätzliche Text durch den
     Kopierschutz-Parameter bei den Kommandozeilenoptionen von ScummVM
     deaktiviert und aktiviert werden.

 Groovie:
   - Optionen für Filmgeschwindigkeit vereinfacht und spezifische Option für
     The 7th Guest hinzugefügt. Filmoptionen sind nun „normal“ und
     „fast“ (schnell), wobei letztere die Filmgeschwindigkeit in T7G erhöht, um
     derjenigen der iOS-Version zu entsprechen. Der Spieleintrag muss
     möglicherweise erneut zur Spieleliste hinzugefügt werden, damit die neue
     Einstellung sichtbar wird.

 SAGA:
   - Musikunterstützung für Macintosh-Version von
     I Have No Mouth, and I Must Scream hinzugefügt.

 SCUMM:
   - Audio-Treiber für Macintosh-Version von Monkey Island 2 integriert. Nun
     wird die auf Samples basierende Audio-Ausgabe richtig unterstützt. Dieselbe
     Ausgabe wird für die m68k-Macintosh-Version von Indiana Jones and the
     Fate of Atlantis verwendet.
   - Verbesserte Musikunterstützung für Macintosh-Version von Monkey Island 1.
     Nun werden die originalen Instrumente verwendet, anstatt sie näherungsweise
     mit Instrumenten von General MIDI zu simulieren, und sollte viel ähnlicher
     zum Original klingen.
   - Sound- und Musikunterstützung für Macintosh-Version von Loom hinzugefügt.
   - Handhabung von Doppelklicks in Macintosh-Version von Loom hinzugefügt.
   - Größere Fehlerbeseitigungen in INSANE (bei den Motorradkämpfen in Vollgas).

 TOUCHE:
   - Unterstützung hinzugefügt für verbesserte Musik von James Woodcock.
     http://www.jameswoodcock.co.uk/category/scummvm-music-enhancement-project/

#### 1.5.0 "Picnic Basket" (27.07.2012)

 Neue Spiele:
   - Unterstützung für Backyard Baseball 2003 hinzugefügt.
   - Unterstützung für Blue Force hinzugefügt.
   - Unterstützung für Darby der Drache hinzugefügt.
   - Unterstützung für Dreamweb hinzugefügt.
   - Unterstützung für Geisha hinzugefügt.
   - Unterstützung für Gregor und der Heißluftballon hinzugefügt.
   - Unterstützung für Magic Tales: Liam Finds a Story hinzugefügt.
   - Unterstützung für Once Upon A Time: Little Red Riding Hood hinzugefügt.
   - Unterstützung für Sleeping Cub's Test of Courage hinzugefügt.
   - Unterstützung für Soltys hinzugefügt.
   - Unterstützung für The Princess and the Crab hinzugefügt.

 Allgemein:
   - MT-32-Emulationscode auf den neusten Stand des Munt-Projekt-Schnappschusses
     gebracht. Die Emulation hat sich drastisch verbessert.
   - Unterstützung für TrueType-Schriftarten über FreeType2 in unserer
     grafischen Benutzeroberfläche hinzugefügt. Damit einhergehend wurde auch
     GNU FreeFont zu unserem modernen Thema hinzugefügt. Beachten Sie, dass
     nicht alle Ports hiervon profitieren.
   - Baskische Übersetzung hinzugefügt.
   - Spiel- und engine-spezifische Optionen in den Engines AGI, DREAMWEB, KYRA,
     QUEEN, SKY und SCI hinzugefügt. Es ist nun möglich, diese über den
     Engine-Reiter anzusprechen, wenn man ein Spiel hinzufügt oder dessen
     Spieloptionen bearbeitet. In den meisten Fällen müssen Sie jedes Spiel
     einmal starten oder diese alle in ScummVMs Spieleliste neu hinzufügen, um
     den Reiter für spielspezifische Optionen zu erhalten.
   - Aussehen von vorhersagendem Eingabedialog verbessert.
   - Verschiedene Verbesserungen der grafischen Benutzeroberfläche.

 Baphomets Fluch 1:
   - Falsche Geräusch-Effekte in der DOS-/Windows-Demo korrigiert.
   - Unterstützung für PlayStation-Videos hinzugefügt.
   - Fehlende Untertitel zur Demo hinzugefügt.

 Baphomets Fluch 2:
   - Unterstützung für PlayStation-Videos hinzugefügt.

 Cine:
   - Roland-MT-32-Ausgabetreiber integriert.

 Drascula:
   - Spanische Untertitel zur Zwischensequenz mit Von Braun
     hinzugefügt (3069981: Keine Untertitel in Szene mit „Von Braun“).

 Gob:
   - Absturz in Lost in Time beseitigt.
   - AdLib-Abspieler umgeschrieben. Den nun funktionierenden MDY-Abspieler in
     Fascination und Geisha aktiviert.

 SCUMM:
   - Unterstützung für die Macintosh-Version von SPY Fox in Hold the Mustard
     hinzugefügt.
   - Dialog zur Auswahl des Schwierigkeitsgrads für Loom FM-TOWNS hinzugefügt.
   - Grafische Störungen in HE98-Version von Pajama Sam's Lost & Found
     beseitigt.

 iPhone-Portierung:
   - Verhalten von Geste für F5 (Menü) geändert, um stattdessen das Hauptmenü zu
     öffnen.
   - Unterstützung für spezifische Mauszeigerpaletten hinzugefügt. Hierdurch
     wird beispielsweise der rote Zeiger im modernen Thema verwendet.
   - Unterstützung für Seitenverhältniskorrektur hinzugefügt.
   - Unterstützung für 16 Bits pro Pixel bei Spielen integriert.

 Maemo-Portierung:
   - Unterstützung für Nokia 770 mit dem Betriebssystem OS2008 HE hinzugefügt.
   - Konfigurierbare Tastenzuweisung hinzugefügt.

 Windows-Portierung:
   - Standard-Verzeichnis für Spielstände bei
     Windows NT4/2000/XP/Vista/7 geändert.
     (Die Stapelverarbeitungsdatei migration.bat kann verwendet werden, um die
     Spielstände vom alten Standard-Verzeichnis in das neue zu kopieren.)

#### 1.4.1 "Subwoofer Release" (27.01.2012)

 AGOS:
   - Das Laden von Videos direkt aus InstallShield-Archiven in der
     Windows-Version von Floyd - Es gibt noch Helden korrigiert.

 Baphomets Fluch 2:
   - Leichte Grafikverbesserung für PSX-Version.


 BASS:
   - Unterstützung für verbesserte Musik von James Woodcock hinzugefügt
     (http://www.jameswoodcock.co.uk/?p=7695).

 KYRA:
   - Fehler in der originalen Benutzeroberfläche von Lands of Lore beseitigt,
     der dazu führte, dass ScummVM abstürzte, wenn der Anwender keine
     durchgehend fortlaufenden Speicherplätze verwendete.
   - Unterstützung für originale DOS-Speicherstände von Lands of Lore
     hinzugefügt (trifft auch auf Speicherstände zu, die mit der GOG-Version
     gemacht wurden).

 SCI:
   - Abfolgebedingung bei SCI1.1-Palettenänderungen korrigiert. Dies behebt
     einen Fehler in QFG1VGA, wenn man in Erana's Peace schläft.
   - Die Option, um zwischen digitalisierten und synthetisierten
     Geräusch-Effekten auszuwählen, wurde bis auf Weiteres deaktiviert, bis eine
     anwenderfreundlichere Benutzeroberflächen-Option möglich ist.
     Digitale Geräusch-Effekte werden vorerst immer bevorzugt.
   - Fehler in einem Fall beseitigt, bei welchem bei Beginn eines neuen Liedes
     nicht alle Kanäle zurückgesetzt wurden und somit einige Noten falsch
     klangen.

#### 1.4.0 "10th Anniversary" (11.11.2011)

 Neue Spiele:
   - Unterstützung für Lands of Lore: The Throne of Chaos hinzugefügt.
   - Unterstützung für Blue's Birthday Adventure hinzugefügt.
   - Unterstützung für Ringworld: Revenge Of The Patriarch hinzugefügt.
   - Unterstützung für die Amiga-Version von Conquests of the Longbow
     hinzugefügt.

 Neue Portierungen:
   - Portierung für PlayStation 3 hinzugefügt.

 Allgemein:
   - ARM-Assembler-Routinen für Stereo-Umkehr bei Audio repariert.
   - Unterstützung für unkomplizierte Zusammenstellung mit MacPorts
     hinzugefügt.

 AGI:
   - Sound-Unterstützung für die DOS-Version von Winnie the Pooh in the
     Hundred Acre Wood integriert.

 AGOS:
   - Unterstützung integriert für direktes Laden von Dateien aus
     InstallShield-Archiven für Floyd - Es gibt noch Helden und
     Simon the Sorcerer's Game Pack.
   - Laden und Speichern in der PC-Version von Waxworks repariert.
   - Musik in den PC-Versionen von Elvira 1 und 2 sowie Waxworks korrigiert.

 Baphomets Fluch 1:
   - Aufhängen in Windows-Demo beseitigt.
   - Absturz beseitigt, wenn Untertitelpaket für Zwischensequenzen in
     Macintosh-Version verwendet wird.


 Groovie:
   - Unterstützung für die iOS-Version von The 7th Guest hinzugefügt.

 Lure:
   - Absturz beseitigt, wenn man versucht, gleichzeitig zu sprechen und etwas zu
     fragen.

 SCI:
   - Bessere Handhabung der Auswahl zwischen digitalen und synthetisierten
     Geräusch-Effekten hinzugefügt. Wenn das Kontrollkästchen
     „AdLib-/MIDI-Modus“ markiert ist, wird die Engine digitale Geräusch-Effekte
     bevorzugen, ansonsten werden stattdessen deren synthetisierten Gegenstücke
     bevorzugt, wenn beide Versionen des gleichen Effekts existieren.

 SCUMM:
   - PC-Lautsprecher-Unterstützung für V5-Spiele integriert.
   - Prioritätsfehler in iMuse beseitigt. Dadurch sollte AdLib-Musik besser
     klingen, da wichtige Noten nicht mehr unterbrochen werden.
   - CMS-Unterstützung integriert für Loom, The Secret of Monkey Island und
     Indiana Jones and the Last Crusade.
   - Palettenhandhabung in der Amiga-Version von Indiana Jones and the
     Fate of Atlantis verbessert.

 Tinsel:
   - Löschen von Spielständen aus der Liste der Speicherstände korrigiert (im
     Startmenü und im ScummVM-Menü innerhalb des Spiels).
   - Die US-Version von Discworld II zeigt nun den korrekten Titelbildschirm und
     die richtige Sprachenflagge.

 Android-Portierung:
   - Plugins bei Android 3.x repariert.
   - Standardmäßigen Speicherort für Spielstände auf SD-Karte verschoben.

#### 1.3.1 "All Your Pitches" (12.07.2011)

 Allgemein:
   - Audiogeräte-Erkennung und Zurückgreifen auf Alternativen verbessert.
     Es sollten keine stillen Fehler mehr aufgrund ungültiger Audiogeräte
     auftreten. Stattdessen sollte ScummVM eine geeignete Alternative auswählen.

 Mohawk:
   - Erkennungseinträge für weitere Varianten einiger Spiele aus der Reihe
     Living Books hinzugefügt.

 Tinsel:
   - Rückschrittsfehler beseitigt, der Discworld undurchspielbar machte.

 SAGA:
   - Rückschrittsfehler im Fortbewegungscode des Drachen in Erben der Erde
     beseitigt, der dort zu Abstürzen führte.
   - Rückschrittsfehler beseitigt, der zu unterschiedlichen Abstürzen in
     I Have No Mouth, and I Must Scream führte.

 SCI:
   - Erkennungseinträge für einige Macintosh-Versionen von Spielen hinzugefügt.
   - Audio-Einstellungen werden nun für die CD-Version von EcoQuest 1 korrekt
     gespeichert.

 SCUMM:
   - Grafikfehler in FM-TOWNS-Versionen von Spielen auf ARM-Geräten beseitigt
     (Android, iPhone, usw.).

#### 1.3.0 "Runner" (28.05.2011)

 Neue Spiele:
   - Unterstützung für Backyard Baseball hinzugefügt.
   - Unterstützung für Backyard Baseball 2001 hinzugefügt.
   - Unterstützung für Urban Runner hinzugefügt.
   - Unterstützung für Playtoons: Bambou le Sauveur de la Jungle hinzugefügt.
   - Unterstützung für Toonstruck hinzugefügt.
   - Unterstützung für V1- und V2-Spiele aus der Reihe Living Books hinzugefügt.
   - Unterstützung für Hugo's House of Horrors, Hugo 2: Whodunit?
     und Hugo 3: Jungle of Doom hinzugefügt.
   - Unterstützung für Amiga-SCI-Spiele hinzugefügt (außer Conquests of the
     Longbow).
   - Unterstützung für Macintosh-SCI1-Spiele hinzugefügt.

 Neue Portierungen:
   - Portierung für WebOS hinzugefügt.

 Allgemein:
   - Unterstützung für ladbare Module auf Plattformen ohne dynamischen Lader
     hinzugefügt. (GSoC-Aufgabe)
   - Dänische Übersetzung hinzugefügt.
   - Norwegische Bokmål-Übersetzung hinzugefügt.
   - Norwegische Nynorsk-Übersetzung hinzugefügt.
   - Schwedische Übersetzung hinzugefügt.
   - Debug-Konsole zu Cine-, Draci-, Gob-, MADE-, Sword1-, Touche- und
     Tucker-Engine hinzugefügt.
   - Bedeutende Speicherlecks geschlossen. Rückkehr zur Spieleliste sollte nun
     brauchbarer sein.

 AGOS:
   - Speicherlecks in Simon 2 und Floyd geschlossen.

 Cine:
   - Speicherlecks und ungültige Speicherzugriffe beseitigt.
   - Future Wars sollte stabiler laufen.
   - Operation Stealth ist nun komplett durchspielbar, auch wenn noch erhebliche
     grafische Störungen vorhanden sind, weshalb das Spiel nicht offiziell
     unterstützt wird.

 Drascula:
   - Deutsche und französische Untertitel zur Zwischensequenz mit Von Braun
     hinzugefügt (3069981: Keine Untertitel in Szene mit „Von Braun“).
   - Französische Übersetzung des Spiels verbessert.
   - Unterstützung für Rückkehr zur Spieleliste hinzugefügt.

 Gob:
   - Einige Fehler beseitigt, bei denen ein Goblin bei erneutem Laden in
     Gobliiins feststeckte.

 Kyra:
   - Speicherlecks geschlossen.

 Parallaction:
   - Problem behoben, das zu Abstürzen bei Verlassen der Engine führen konnte.
   - Speicherlecks in Amiga-Version von Nippon Safes geschlossen.

 SCI:
   - Einen CMS-Musiktreiber für Spiele von SCI1 bis SCI1.1 hinzugefügt.
   - Option zum Ein- und Ausschalten der Antifehlerdiffusion in
     ScummVM-Oberfläche hinzugefügt.
   - Mehrere vorher fehlende Teile des Spielzustands zu Speicherständen
     hinzugefügt, wie beispielsweise die gespielte Zeit, skripterzeugte Fenster,
     der Skript-Zeichenketten-Freispeicher und Informationen im Zusammenhang mit
     dem Textparser in alten EGA-Spielen.
   - Unterstützung für die Vergrößerungsmauszeiger in SCI1.1 hinzugefügt.
   - Unterstützung für die Tasten + und - auf dem Ziffernblock hinzugefügt.
   - Unterstützung für alternative General-MIDI-Titel in den
     Windows-CD-Versionen von Eco Quest, Jones in the Fast Lane, King's Quest 5
     und Space Quest 4 hinzugefügt.
   - Unterstützung für alternative Windows-Mauszeiger in der Windows-Version von
     King's Quest 6 hinzugefügt.
   - Unterstützung für gleichzeitige Sprachausgabe und Untertitel in den
     CD-Versionen von Space Quest 4 und Freddy Pharkas hinzugefügt.
   - Lecks beim Laden von Ressourcen beseitigt.
   - Einige Probleme in den Räumen von Skate-O-Rama in Space Quest 4 behoben.
   - Einige Probleme in Hoyle Classic Card Games behoben.
   - Einige Grafikstörungen beseitigt (wie z. B. dass einige Teile des
     Bildschirms in manch seltenen Fällen nicht ordnungsgemäß gelöscht wurden).
   - Mehrere Skriptfehler beseitigt.
   - Mehrere Probleme im Zusammenhang mit dem Finden des Wegs und Aufhängen des
     Spiels behoben (z. B. hing sich das Spiel in der Duschszene von
     Laura Bow 1 auf und es gab Schwierigkeiten beim Finden des Wegs in einigen
     Bildschirmen während der Verfolgungssequenz in Laura Bow 2).
   - Mehrere Probleme im Zusammenhang mit Musik und möglichem Aufhängen
     behoben (z. B. hatte sich in seltenen Fällen die Musik aufgehängt,
     wenn man einen Spielstand außerhalb des Palastes in Quest for Glory 3
     geladen hatte).
   - Mögliche Probleme und mögliches Aufhängen bei Bildschirmen zur Importierung
     von Figuren für Quest for Glory 2 und 3 behoben.
   - Fehler beseitigt, der in der SCI1-CD-Version von Mixed Up Mother Goose das
     Spiel dazu brachte, sich nach Tommy Tuckers Lied aufzuhängen.
   - Skriptfehler in der CD-Version von King's Quest 5 beseitigt, der das Spiel
     unter gewissen Umständen zum Aufhängen brachte, wenn man im dunklen Wald
     aus dem Haus der Hexe ging.
   - Funktionstasten funktionieren nun richtig, wenn der Ziffernblock
     aktiviert ist.
   - Unterstützung für Skripte in Fan-Spielen verbessert.
   - Unterstützung für nicht-englische Versionen von Spielen verbessert.
   - Viele Verbesserungen und Fehlerbeseitigungen im Zusammenhang mit
     MT-32-Musik eingebracht (z. B. Hall).
   - Musik ist nicht mehr verstimmt, wenn man einen Spielstand lädt.

 SCUMM:
   - Unterstützung für FM-TOWNS-Versionen von Spielen verbessert.

 Sky:
   - Abstürze während Zwischensequenzen bei mehreren Portierungen beseitigt
     (Android, OpenGL, ...).

 Teenagent:
   - Speicherlecks geschlossen.

 Tinsel:
   - Speicherlecks in Nebenroutinen geschlossen.
   - Unterstützung für verbesserte Musik für die deutsche
     CD-Wiederveröffentlichung namens „Neon Edition“ von Discworld 1
     hinzugefügt.

 Touche:
   - Speicherlecks beseitigt und kleinere Probleme behoben.

 Tucker:
   - Umgehungen für mehrere im Originalspiel vorhandene Probleme hinzugefügt.

 SDL-Portierungen:
   - Speicherlecks bei Mausoberflächen geschlossen.

 Android-Portierung:
   - Wechsel zu offizieller NDK-Tool-Kette für Zusammenstellung
   - Grafikausgabe für verschiedene Geräte korrigiert.
   - Mehrere Abstürze beseitigt.
   - Wechsel zu nativer Bildschirmauflösung, um Textlesbarkeit zu verbessern
   - Unterstützung für Pausieren/Fortsetzen hinzugefügt.
   - Unterstützung für Spiele, die 16-Bit-Grafik verwenden, hinzugefügt.
   - Leistung erheblich verbessert.
   - Unterstützung für „Vollbildmodus“-Option hinzugefügt. Entfernt man dort das
     Häkchen, bleibt das Seitenverhältnis beibehalten.
   - Neuen Grafikmodus für lineare Filterung hinzugefügt.
   - Eingabesystem wurde überholt (siehe README.Android).
   - MIDI-Treiber basierend auf SONiVOX' Embedded Audio Synthesis (EAS)
     hinzugefügt.

 Nintendo-DS-Portierung:
   - Unterstützung für ladbare Module hinzugefügt.

 PSP-Portierung:
   - Unterstützung für ladbare Module hinzugefügt.
   - Bildbetrachter hinzugefügt.

 PS2-Portierung:
   - Unterstützung für ladbare Module hinzugefügt.

 Portierung für Wii/GameCube:
   - Unterstützung für ladbare Module hinzugefügt.
   - 16-Bit-Mauszeiger in HE-Spielen korrigiert.

#### 1.2.1 "Bork Bork Bork" (19.12.2010)

 Allgemein:
   - Ungarische Übersetzung hinzugefügt.
   - Brasilianisch-portugiesische Übersetzung hinzugefügt.

 Cruise:
   - Problem behoben, dass Raoul beim Untersuchen des Buchs erschien.

 Groovie:
   - Rückschrittsfehler beseitigt, der die russische Version von T7G abstürzen
     ließ.

 Lure:
   - Mehrere Bewegungsfehler bei Nicht-Spielern beseitigt.

#### 1.2.0 "FaSCInating release" (15.10.2010)

 Neue Spiele:
   - Unterstützung für Fascination hinzugefügt.

 Neue Spiele (Sierra SCI0 - SCI1.1):
   - Unterstützung für Codename: ICEMAN hinzugefügt.
   - Unterstützung für Conquests of Camelot hinzugefügt.
   - Unterstützung für Conquests of the Longbow (EGA und VGA) hinzugefügt.
   - Unterstützung für Das Schloß von Dr. Brain (EGA und VGA) hinzugefügt.
   - Unterstützung für EcoQuest: Die Suche nach Cetus hinzugefügt.
   - Unterstützung für EcoQuest 2: Lost Secret of the Rainforest hinzugefügt.
   - Unterstützung für Freddy Pharkas: Cowboy-Apotheker hinzugefügt.
   - Unterstützung für Hoyle's Book of Games 1 hinzugefügt.
   - Unterstützung für Hoyle's Book of Games 2 hinzugefügt.
   - Unterstützung für Hoyle's Book of Games 3 (EGA und VGA) hinzugefügt.
   - Unterstützung für Hoyle Classic Card Games hinzugefügt.
   - Unterstützung für Jones in the Fast Lane hinzugefügt.
   - Unterstützung für King's Quest I (SCI-Remake) hinzugefügt.
   - Unterstützung für King's Quest IV (SCI-Version) hinzugefügt.
   - Unterstützung für King's Quest V (EGA und VGA) hinzugefügt.
   - Unterstützung für King's Quest VI (hohe und niedrige Auflösung)
     hinzugefügt.
   - Unterstützung für Laura Bow: The Colonel's Bequest hinzugefügt.
   - Unterstützung für Laura Bow 2: Der Dolch des Amon Ra hinzugefügt.
   - Unterstützung für Leisure Suit Larry 1 (SCI-Remake) (EGA und VGA)
     hinzugefügt.
   - Unterstützung für Leisure Suit Larry 2 hinzugefügt.
   - Unterstützung für Leisure Suit Larry 3 hinzugefügt.
   - Unterstützung für Leisure Suit Larry 5 (EGA und VGA) hinzugefügt.
   - Unterstützung für Leisure Suit Larry 6 (niedrige Auflösung) hinzugefügt.
   - Unterstützung für Mixed-up Fairy Tales hinzugefügt.
   - Unterstützung für Mixed-up Mother Goose hinzugefügt.
   - Unterstützung für Pepper's Adventures in Time hinzugefügt.
   - Unterstützung für Police Quest I (SCI-Remake) hinzugefügt.
   - Unterstützung für Police Quest II hinzugefügt.
   - Unterstützung für Police Quest III (EGA und VGA) hinzugefügt.
   - Unterstützung für Quest for Glory I/Hero's Quest hinzugefügt.
   - Unterstützung für VGA-Remake von Quest for Glory I hinzugefügt.
   - Unterstützung für Quest for Glory II hinzugefügt.
   - Unterstützung für Quest for Glory III hinzugefügt.
   - Unterstützung für Slater & Charlie Go Camping hinzugefügt.
   - Unterstützung für Space Quest I (SCI-Remake) (EGA und VGA) hinzugefügt.
   - Unterstützung für Space Quest III hinzugefügt.
   - Unterstützung für Space Quest IV (EGA und VGA) hinzugefügt.
   - Unterstützung für Space Quest V hinzugefügt.
   - Unterstützung für The Island of Dr. Brain hinzugefügt.

 Neue Portierungen:
   - Portierung für Android hinzugefügt.
   - Portierung für Dingux hinzugefügt.
   - Portierung für Caanoo hinzugefügt (basierend auf Portierung von GP2XWiz).
   - Portierung für OpenPandora hinzugefügt.

 Allgemein:
   - Veraltete PalmOS-Portierung entfernt.
   - Wechsel zum „schnellen“ DOSBox-OPL-Emulator
   - Absturz in Rjp1-Abspiel-Code beseitigt, der die Amiga-Version von FOTAQ
     betraf.
   - Unterstützung bei weiteren Spielen für Dateienanordnung des originalen
     Datenträgermediums hinzugefügt.
   - Unterstützung für Benutzeroberflächen-Lokalisationen hinzugefügt (darunter
     auch Deutsch).
   - Benutzeroberfläche verbessert, indem Tipps und Optionsschaltflächen
     hinzugefügt wurden.
   - Benutzerfreundlichkeit der Oberfläche verbessert, indem weitere belanglose
     Optionen bei bestimmten Spielen verborgen wurden, von denen sie nicht
     unterstützt werden.

 AGI:
   - Gewisse Anzahl an Grafikstörungen beseitigt.
   - PIC zeichnet nun kodiertes Bild perfekt.
   - Unterstützung für MIDI-Geräte hinzugefügt.
   - Unterstützung für präzise Tandy-Sound-Emulation hinzugefügt. Diese ist nun
     standardmäßig festgelegt.

 Baphomets Fluch 2:
   - Problem behoben, durch das in manchen Zwischensequenzen Sprachausgabe
     fehlte.
   - Speicherleck beseitigt, das irgendwann das Spiel zum Aufhängen brachte
     (2976008 - BS2: Spiel friert in britischem Museum ein).

 Drascula:
   - Gewisse Anzahl an Grafikstörungen beseitigt.
   - Viele Zwischensequenzen feiner gemacht.
   - Verhalten des Befehlmenüs verändert. Jetzt wird es angezeigt, wenn man
     die Maus nach oben bewegt.

 Groovie:
   - Unterstützung für Macintosh-Version von The 7th Guest hinzugefügt.
   - Unterstützung für spielspezifische MT-32-Instrumentänderungen hinzugefügt.

 KYRA:
   - Einige kleinere Grafikstörungen beseitigt.
   - Vorher fehlende Neuplatzierung einiger Gegenstände im Spiel integriert.
   - Unterstützung für das Spielen von Kyrandia 3 mit originaler Anordnung der
     CD-Dateien hinzugefügt.

 LURE:
   - Fehler beseitigt, bei dem Goewin im Wehrtor stecken bleiben konnte.
   - Problem mit Ratpouch beseitigt, bei dem er wiederholt zwischen zwei Räumen
     hin- und herlief.
   - Fehler beseitigt, dass Goewin nach der Drachenhöhle aus dem Zeitplan kam.
   - Fehler beseitigt, bei dem der Spieler im Ausgangsraum des Abflusses
     stecken bleiben konnte.

 Parallaction:
   - Erster Teil von The Big Red Adventure ist nun komplett durchspielbar.

 SAGA:
   - Grafikstörungen in mehreren Szenen beseitigt.

 SCUMM:
   - Mehrere Verbesserungen in Maniac Mansion NES

 PSP-Portierung:
   - Neue Systemgestaltung: Kleinere Grafikprobleme behoben und
     16-Bit-Unterstützung ermöglicht.
   - Wiedergabe von MP3-Dateien mittels Media Engine ermöglicht. Das bedeutet,
     dass die Portierung für MP3-Dateien (entgegengesetzt zu Ogg) optimiert ist.
   - Viele Optimierungen vorgenommen. Alles sollte schneller laufen.

 Wii-Portierung:
   - Unterstützung für USB2-Massenspeichergeräte hinzugefügt (erfordert
     The Homebrew Channel >= v1.0.8 mit IOS58).

 GameCube-Portierung:
   - Unterstützung für DVDs mit ISO9660-Dateisystem hinzugefügt.

 GP2X-Portierung:
   - Unterstützung für dynamische Engine-Plugins hinzugefügt (experimentell).
   - Kontrollsystem überarbeitet und Touchscreen-Unterstützung verbessert.

 GP2XWiz/Caanoo-Portierung:
   - Code für Herunterskalierung verbessert, um „Verschiebungsfehler“ im Bild
     (Tearing) zu minimieren.
   - Kontrollsystem überarbeitet und Touchscreen-Unterstützung verbessert.
   - System von GP2XWIZ in GPH umbenannt, um die Bandbreite der unterstützten
     Geräte besser wiederzuspiegeln.

#### 1.1.1 "Better version" (02.05.2010)

 Neue Portierungen:
   - Portierung für Nintendo 64 hinzugefügt. (Eigentlich schon in 1.1.0
     hinzugefügt, aber es wurde vergessen, dies zu erwähnen.)

 Allgemein:
   - Hier und da einige kleinere Fehler beseitigt.

 Drascula:
   - Rückschrittsfehler beseitigt, der dazu führte, dass einige Texte immer in
     Englisch waren, selbst wenn eine andere Sprache eingestellt war.
     (2970211 - DRASCULA: Fehlende deutsche Übersetzung)

 KYRA:
   - Fehler beseitigt, der DOS-Versionen vor dem Anzeigen der Mitwirkendenliste
     abstürzen ließ, wenn AdLib-Musik ausgewählt war.

 LURE:
   - Mehrere Speicherlecks beseitigt.
   - Probleme bei Handhabung von mitlaufenden Figuren beseitigt, wenn sie
     an der Ausführung von Aktionen durch geschlossene Türen zwischen Räumen
     gehindert werden.
   - Problem von Goewin behoben, dass sie dem Spieler nicht immer genau aus den
     Höhlen gefolgt ist.

 Tinsel:
   - Rückschrittsfehler bei Video-Wiedergabe in Discworld 2 beseitigt.

 Parallaction:
   - Mehrere Abstürze und weitere Rückschrittsfehler in Nippon Safes beseitigt,
     einschließlich der Fehler 2969211, 2969232, 2969234, 2969257, 2970141.

 Portierung für Wii/GameCube (Erwähnung bei 1.1.0 ebenfalls vergessen):
   - Unterstützung für Spiele, die 16-Bit-Grafik verwenden, hinzugefügt.
   - Vollständige Grafiküberholung (neue Bildschirm-Modi, bessere Leistung,
     Bildglättung, Einstellungen für feinkörnige Übertastung)
   - Neuen Optionsdialog für spezifische Einstellungen für Wii/Gamecube
     hinzugefügt.
   - Grafikstörung bei kleinen Vorschaubildern von Spielständen beseitigt.
   - Unterstützung für SMB-Einhängepunkte hinzugefügt (öffentliche Verzeichnisse
     von Samba/Windows).
   - Bildschirm-Konsole hinzugefügt, welche angezeigt wird, wenn ScummVM
     ungewöhnlich beendet wurde.
   - Einige Abstürze beseitigt, wenn mit Ogg Vorbis komprimierte Sound-Dateien
     verwendet werden. Lesen Sie die beiliegende Datei READMII.txt für weitere
     Informationen.

 PSP-Portierung (Erwähnung bei 1.1.0 ebenfalls vergessen):
   - Plugin-Unterstützung hinzugefügt, was es auf der PSP Phat ermöglicht, jedes
     Spiel laufen zu lassen.
   - Neue virtuelle Tastatur hinzugefügt, die für schnelle D-Pad-Eingabe
     optimiert ist.

#### 1.1.0 "Beta quadrant" (04.04.2010)

 Neue Spiele:
   - Unterstützung für Blue's Art Time Activities hinzugefügt.
   - Unterstützung für Blue's Reading Time Activities hinzugefügt.
   - Unterstützung für Freddi Fisch und das Rätsel der Korallenbucht
     hinzugefügt.
   - Unterstützung für Pajama Sam: Games to Play on Any Day hinzugefügt.
   - Unterstützung für SPY Fox: Alarm im Weltall hinzugefügt.
   - Unterstützung für Drači Historie hinzugefügt.
   - Unterstützung für TeenAgent hinzugefügt.

 Allgemein:
   - Unterstützung für eine benutzerdefinierte SJIS-Schriftart für FM-TOWNS- und
     PC98-Spiele hinzugefügt.
   - Unterstützung für 16-Bit-Grafik hinzugefügt. (GSoC-Aufgabe)
   - MIDI-System von QuickTime bei Mac OS X entfernt; es war fehlerhaft und ließ
     sich auf modernen Systemen nicht kompilieren.

 CinE:
   - Unterstützung für Menüs im Amiga-Stil für Amiga-Versionen von Future Wars
     hinzugefügt.

 KYRA:
   - Unterstützung für die Amiga-Version von The Legend of Kyrandia hinzugefügt.
     (Sound-Unterstützung kam im Rahmen einer GSoC-Aufgabe dazu.)
   - KYRA angepasst, um die benutzerdefinierte SJIS-Schriftart zu unterstützen.

 SCUMM:
   - Unterstützung für die PC-Engine-Version von Loom hinzugefügt.
   - Unterstützung für Musik und Geräusch-Effekte in der Amiga-Version von
     The Secret of Monkey Island hinzugefügt. (GSoC-Aufgabe)
   - Einige weitere Fehler im Zusammenhang mit Amiga-Versionen von Spielen
     beseitigt.
   - Unterstützung für originalen Dialog zum Speichern und Laden in MM NES
     hinzugefügt.
   - Unterstützung hinzugefügt für Speicherpunkt-Codes für Sega CD MI1 über den
     Debugger-Befehl „passcode“.
   - Unterstützung für Kanji-Renderung in japanischer Version von
     Monkey Island Sega CD hinzugefügt.

#### 1.0.0 "Shiny Logo" (15.11.2009)

 Neue Portierungen:
   - Portierung für MotoEZX und MotoMAGX hinzugefügt.

 Allgemein:
   - Einige Fehler in grafischer Benutzeroberfläche beseitigt.
   - Projektlogo und -symbole aktualisiert.
   - Lizenz für mehrere PS2-Portierungsdateien verdeutlicht.

 AGI:
   - Absturz bei Spielbeendigung beseitigt.
   - Absturz bei Erkennung einiger Spiele beseitigt.

 AGOS:
   - Code für das Speichern und Laden für PC-Version von Waxworks korrigiert.
   - Undo-Funktion für Rückgängigmachen des letzten Spielzugs bei Spielen aus
     Game Pack korrigiert.

 Baphomets Fluch 1:
   - Problem behoben, durch das in manchen Räumen Hintergrundgeräusche fehlten.

 CinE:
   - Abstürze in den Demos von Future Wars und Operation Stealth beseitigt.

 Cruise:
   - Aufhängen bei Spielpause beseitigt.

 Gob:
   - Grafikstörung bei Lost in Time beseitigt.
   - Rückschrittsfehler im Zusammenhang mit Klickbereichen in Gob2 beseitigt.
   - Mehrere Rückschrittsfehler in Gob3 beseitigt.
   - Absturz im Vorspann von Bargon Attack beseitigt.
   - Animationsabläufe in Win3.1-Version von Gob3 korrigiert.

 Groovie:
   - Video-Leistung bei PSP korrigiert.
   - Fehler mit leerem Menü in einigen Instanzen beseitigt.

 Kyra:
   - Mögliche Zerstörung des Neustart-Spielstands in Kyrandia 1 beseitigt.
   - Grafikstörung bei kaputter Brücke beseitigt.
   - Beseitigung eines Fehlers, durch den Brandon in einigen Situationen in
     Kyrandia 1 unsichtbar wurde

 MADE:
   - Leistung bei NDS korrigiert.
   - Absturz in Demo von Return to Zork beseitigt.

 Parallaction:
   - Mehrere Rückschrittsfehler in Nippon Safes beseitigt.
   - Musik in Sushi-Bar in Nippon Safes korrigiert.

 SCUMM:
   - Absturz in Macintosh-Versionen von Putt-Putt Joins the Parade und Fatty
     Bear's Birthday Surprise beseitigt.
   - Speichern des Spielstands in Macintosh-Versionen von HE-Spielen korrigiert.
   - Standardmäßigen Spielstandpfad für spätere HE-Spiele korrigiert.
   - Palette in NES-Version von Maniac Mansion korrigiert.
   - Palette in Amiga-Version von The Secret of Monkey Island korrigiert.
   - Fehler beseitigt, bei dem in The Dig die Musik nicht aufhörte, wenn man
     während Zwischensequenzen Esc drückte.
   - Lippensynchronisation in Fatty Bear korrigiert.
   - Absturz in Vollgas beseitigt, wenn man ins Inventar geht.

 Tinsel:
   - Absturz in Palast beseitigt.
   - Absturz beseitigt, wenn man mit älterer Dame spricht.
   - Teilweise Fehler beseitigt, bei dem Text in DW1 außerhalb des Bildschirms
     angezeigt wurde.

 NDS-Portierung:
   - Mauspad unterhalb von Bildschirmtastatur hinzugefügt.
   - Möglichkeit zum Scrollen hinzugefügt, wenn Mauszeiger eine Kante des
     Bildschirms erreicht (im Mauspad-Modus).
   - Audio-Dateien beginnend von track01.wav werden nun genauso gelesen wie
     track1.wav.
   - Fehler beseitigt, wenn Modus bei aktiver Tastatur gewechselt wird.

 PSP-Portierung:
   - Video-Flimmern und -Streckung in einigen Situationen beseitigt.
   - Unterstützung für Unterbrechen/Fortsetzen verbessert.

 WinCE-Portierung:
   - Kompatibilität mit VGA-Geräten verbessert.

#### 1.0.0rc1 "Grog XD" (31.08.2009)

 Neue Spiele:
   - Unterstützung für Discworld hinzugefügt.
   - Unterstützung für Discworld 2 - Vermutlich vermisst hinzugefügt.
   - Unterstützung für Return to Zork hinzugefügt.
   - Unterstützung für Leather Goddesses of Phobos 2 hinzugefügt.
   - Unterstützung für The Manhole hinzugefügt.
   - Unterstützung für Rodney's Funscreen hinzugefügt.
   - Unterstützung für Cruise for a Corpse hinzugefügt.

 Allgemein:
   - Experimentellen AdLib-Emulator aus DOSBox hinzugefügt.
   - Schnelle Eintragssuche zu Startmenü hinzugefügt.
   - Aussehen von moderner Themenoberfläche verbessert.
   - Verschiedene Oberflächenoptionen für jedes Spiel einstellbar
   - „Durchsuchen“-Dialog verbessert.

 Neue Portierungen:
   - Portierung für GP2X Wiz hinzugefügt.

 AGI:
   - Kompatibilität für Sierra-Spiele erhöht.
   - Alle „unbekannten“ Befehle integriert.

 Beneath a Steel Sky:
   - Spielgeschwindigkeit geändert, um der des Originalspiels zu entsprechen
     (lief vorher zu schnell).

 Baphomets Fluch 1:
   - Unterstützung für Original-Zwischensequenzen hinzugefügt.
   - Unterstützung für die nun hinfälligen MPEG2-Zwischensequenzen eingestellt.
   - Unterstützung für die PlayStation-Version hinzugefügt.

 Baphomets Fluch 2:
   - Unterstützung für Original-Zwischensequenzen hinzugefügt.
   - Unterstützung für die nun hinfälligen MPEG2-Zwischensequenzen eingestellt.
   - Unterstützung für das Abspielen von Ton aus Zwischensequenzen ohne Video
     eingestellt.
   - Unterstützung für die PlayStation-Version hinzugefügt.

 Gob:
   - Neues Spielstandsformat eingeführt, um eine schwerwiegende Schwachstelle
     des alten Formats zu beseitigen, wodurch die Kompatibilität mit alten
     Spielständen, die auf Big-Endian-Systemen erstellt wurden, aufhört.

 Groovie:
   - Schwierigkeitsgrad von Mikroskop-Puzzle erhöht, um dem des Originalspiels
     zu entsprechen.

 KYRA:
   - Unterstützung für auf PC-Lautsprecher basierende Musik und Geräusch-Effekte
     hinzugefügt.
   - Unterstützung für 16-Farben-Fehlerdiffusion in Kyrandia PC-9801
     hinzugefügt.

 PSP-Portierung:
   - Unterstützung für Ruhe-Modus (unterbrechen/fortsetzen) hinzugefügt.

 WinCE-Portierung:
   - Geschwindigkeitsoptimierte Versionen von niedrig auflösendem Smartphone und
     niedrig auflösenden 2x-Grafikwandlern
   - Neuer Grafikwandler für Seitenverhältniskorrektur für VGA-Geräte (oder
     Geräte mit höherer Farbentiefe)
   - Unterstützung für MPEG-2 und FLAC eingestellt.

#### 0.13.1 "SAGA returns" (27.04.2009)

 AGOS:
   - Absturz nach Wiedergabe von OmniTV-Video in Floyd - Es gibt noch Helden
     beseitigt.
   - Absturz bei Erforschung der Jack-the-Ripper-Szene in der PC-Version von
     Waxworks beseitigt.
   - Palettenstörung in der AtariST-Version von Elvria 2 beseitigt.
   - Geräusch beseitigt, dass vorkommen konnte, wenn man die Pyramiden-Szene in
     Waxworks erforscht.

 Gob:
   - Absturz in der italienischen Version von Woodruff beseitigt.

 Groovie:
   - Einige Probleme mit der Musik in The 7th Guest behoben.

 Parallaction:
   - Fehler bei Sarkophag-Rätsel in Nippon Safes beseitigt.

 SAGA:
   - Absturz in Erben der Erde beseitigt.
   - Störungen im Dialog für das Speichern und Laden beseitigt.

 Sword2:
   - Zufällige Sound-Missbildung beseitigt, wenn originale Sound-Dateien
     verwendet werden.

 Startmenü:
   - Fall korrigiert, in dem Speicherinformationen unverwertbar werden
     konnten.
   - Kleinen Mauszeiger im modernen Thema korrigiert.
   - Fehler in Themen-Engine beseitigt, welcher Abstürze verursachen konnte.
   - Pfadauswahldialog in 1x-Modus vergrößert.

 Portierung für iPhone:
   - Handhabung der Rücktaste auf der iPhoneSoft-Tastatur korrigiert.

 DS-Portierung:
   - Unterstützung für die Funktion des globalen Hauptmenüs hinzugefügt.

 PS2-Portierung:
   - Zu neuer grafischer Benutzeroberfläche und neuem Themen-Code gewechselt.
   - Bei allen möglichen Geräten wird nun das Ablegen, Spielen und Speichern von
     Spielständen unterstützt (CD, Festplatte, USB, MC und Fernzugriffspunkt).
   - Speicherverwaltung/Vorauslesen für jedes Medium verbessert.
   - Unterstützung für den Wechsel „Zurück zur Spieleliste“ hinzugefügt.

 Symbian-Portierung:
   - Bluetooth-Mausunterstützung hinzugefügt.
   - Unterstützung für den Wechsel „Zurück zur Spieleliste“ hinzugefügt.

 WinCE-Portierung:
   - Problem behoben, das bei VGA-Geräten zu zufälligen Abstürzen führen konnte.

#### 0.13.0 "More Guests" (28.02.2009)

 Allgemein:
   - MIDI-Treiber für Atari ST / FreeMint hinzugefügt.
   - „Laden“-Schaltfläche zu Startmenü hinzugefügt (nicht von allen Engines
     unterstützt). (GSoC-Aufgabe)
   - Dialog für neues globales Hauptmenü hinzugefügt, das von allen Engines
     verwendet werden kann. (GSoC-Aufgabe)
   - Funktion hinzugefügt, um von laufenden Spielen aus zur Spieleliste
     zurückzukehren (über globales Hauptmenü). (GSoC-Aufgabe)
   - Oberflächen-Renderung umgeschrieben, um einen vektorbasierten Ansatz zu
     verwenden. (GSoC-Aufgabe)
   - Oberflächenkonfiguration umgeschrieben, um XML zu verwenden. (GSoC-Aufgabe)

 Neue Spiele:
   - Unterstützung für Blue's 123 Time Activities hinzugefügt.
   - Unterstützung für Blue's ABC Time Activities hinzugefügt.
   - Unterstützung für Bud Tucker in Double Trouble hinzugefügt.
   - Unterstützung für The 7th Guest hinzugefügt.

 AGOS:
   - Unterstützung für Original-Zwischensequenzen von Floyd - Es gibt
     noch Helden hinzugefügt.
   - Unterstützung für Textkompression in der AtariST-Version von Elvira 1
     hinzugefügt.
   - Fehler beim Kombinieren von Gegenständen in Waxworks beseitigt.
   - Fehler bei Anzeige von Zauberspruch-Beschreibungen in Elvira 2 beseitigt.

 KYRA:
   - Unterstützung für automatisches Speichern hinzugefügt.
   - Unterstützung für MIDI-Musik hinzugefügt.

 Parallaction:
   - Mitwirkendenliste in der Amiga-Demo von Nippon Safes wird nun richtig
     angezeigt.

 SCUMM:
   - Funkspruch-Effekt in The Dig integriert.

#### 0.12.0 "&nbsp;" (31.08.2008)

 Neue Spiele:
   - Unterstützung für The Legend of Kyrandia: Book Two: Hand of Fate
     hinzugefügt.
   - Unterstützung für The Legend of Kyrandia: Book Three: Malcolm's Revenge
     hinzugefügt.
   - Unterstützung für Lost in Time hinzugefügt.
   - Unterstützung für Woodruff and the Schnibble of Azimuth hinzugefügt.
   - Unterstützung für die PC-Version von Waxworks hinzugefügt.
   - Unterstützung für die Macintosh-Version von I Have No Mouth, and I
     Must Scream hinzugefügt.
   - Unterstützung für Drascula: The Vampire Strikes Back hinzugefügt.

 Allgemein:
   - CAMD-MIDI-Treiber für AmigaOS4 hinzugefügt.
   - PS2-Portierung wiederbelebt (war sie schon in 0.11.1, aber es wurde
     in den Veröffentlichungshinweisen vergessen).
   - Zahlreiche Speicherlecks in allen Engines gestopft (Teil der Aufgaben des
     GSoC '08).
   - Doppelte Audio-Pufferung zu SDL-System hinzugefügt, was die Probleme mit
     dem MT-32-Emulator in Mac OS X behebt (vorerst nur bei Mac OS X
     aktiviert).

 AGOS:
   - Abstürze während bestimmter Musikstücke in Amiga-Versionen von Elvira 1 und
     Simon the Sorcerer 1 beseitigt.
   - Palettenprobleme in Amiga-Versionen von Simon the Sorcerer 1 behoben.

 Queen:
   - Sprachausgabe wird nun mit richtiger Abtastfrequenz wiedergegeben. (Vorher
     war die Wiedergabegeschwindigkeit etwas zu niedrig.)

 SCUMM:
   - Teile von Digital iMUSE umgeschrieben, was einige Fehler beseitigt.
   - Code für interne Zeitplanung umgeschrieben, was einige
     Geschwindigkeitsprobleme behebt, beispielsweise in COMI.
   - Unterstützung für Geräusch-Effekte in Amiga-Versionen von Zak McKracken
     verbessert.
   - Unterstützung für AdLib-MIDI-Kombination in Monkey Island 1
     (Diskettenversion) hinzugefügt.

#### 0.11.1 "Fixed exist()nce" (29.02.2008)

 SCUMM:
   - Verbesserungen für Untersystem von Digital iMUSE. Dies behebt verschiedene
     Störungen in The Curse of Monkey Island.
   - Fehlerbeseitigungen für verschiedene Mauszeiger in HE-Spielen

 AGI:
   - Fehlerbeseitigung für Grabgestalten in King's Quest 4
   - Fehlerbeseitigung für Palettenwechsel in von Fans erstellten Spielen
     mittels AGIPAL

 Lure:
   - Einige Abstürze bei Gesprächen in der deutschen Version beseitigt.
   - Funktionsweise des optionalen Kopierschutzdialogs in der deutschen Version
     korrigiert.
   - Speicherung von Gesprächsflaggen hinzugefügt, um festzustellen, ob eine
     bestimmte Gesprächsoption zuvor ausgewählt wurde oder nicht.
   - Störung beseitigt, die dazu führen konnte, dass Verwandlungsglitzern ein
     zweites Mal vorkam.
   - Verhalten von Goewin korrigiert, wenn man sich nach der Begegnung mit dem
     Drachen ihr wieder anschließt.

 SAGA:
   - Fehler bei Rattenlabyrinth in Erben der Erde beseitigt, der das Spiel
     undurchspielbar machte.
   - Fehlerbeseitigungen für Spielstart von Erben der Erde und I Have No Mouth
     bei einer gewissen Anzahl von Plattformen
   - Anzahl gleichzeitig geöffneter Dateien in I Have No Mouth reduziert, um zu
     ermöglichen, dass das Spiel auf Plattformen läuft, welche nur eine
     begrenzte Anzahl Dateien geöffnet haben können (z. B. auf der PSP).
   - Grafikstörung in Erben der Erde bei überlappender Sprachausgabe beseitigt.
   - Palettenstörung in Erben der Erde beseitigt, wenn man die Karte anschaut,
     während man sich am Dock befindet.

#### 0.11.0 "Your Palindrome" (15.01.2008)

 Neue Spiele:
   - Unterstützung für Elvira: Mistress of the Dark hinzugefügt.
   - Unterstützung für Elvira 2: The Jaws of Cerberus hinzugefügt.
   - Unterstützung für I Have No Mouth, and I Must Scream (Demo und Vollversion)
     hinzugefügt.
   - Unterstützung für PreAGI-Spiel Mickey's Space Adventure hinzugefügt.
   - Unterstützung für PreAGI-Spiel Troll's Tale hinzugefügt.
   - Unterstützung für PreAGI-Spiel Winnie the Pooh in the Hundred Acre Wood
     hinzugefügt.
   - Unterstützung für Amiga-Version von Waxworks hinzugefügt.
   - Unterstützung für Lure of the Temptress hinzugefügt.

 Neue Portierungen:
   - Portierung für iPhone hinzugefügt.
   - Maemo-Portierung für Nokia-Internet-Tablets hinzugefügt.

 Allgemein:
   - ARM-Assembler-Routinen für Code im Soundmixer und bei Video-Wiedergabe in
     SCUMM hinzugefügt, was zu manch guten Beschleunigungen bei einigen
     Portierungen führt.
   - Art und Weise der internen Handhabung von Tastatureingaben verbessert, was
     ein paar seltsame Macken in einigen Spiel-/Port-Kombinationen beseitigt.
   - Optionalen Dialog zum Bestätigen des Beendens zu SDL-System hinzugefügt.
   - Dmedia-MIDI-Treiber für IRIX hinzugefügt.
   - Erkennung neuer Spielvarianten und lokalisierter Versionen verbessert.
   - Interner Zugriff auf Dateien vollständig überarbeitet. (GSoC-Aufgabe)
   - Option hinzugefügt, um Spiele durch Drücken der Entf-Taste von der Liste zu
     entfernen.
   - Unterstützung für die Ersetzung des Präfixes „~/“ durch „$HOME“ in Pfaden
     auf POSIX-Systemen (Linux, Mac OS X usw.) hinzugefügt.

 AGI:
   - Unterstützung für AGI256- und AGI256-2-Hacks hinzugefügt. (GSoC-Aufgabe)
   - Unterstützung für Amiga-Menüs und -Paletten hinzugefügt. (GSoC-Aufgabe)
   - Bessere Unterstützung für frühe AGI-Titel von Sierra

 AGOS:
   - Abstürze im Zusammenhang mit OmniTV-Wiedergabe in
     Floyd - Es gibt noch Helden beseitigt.
   - Verbesserte Funktionalität auf 64-Bit-Systemen

 Baphomets Fluch 1:
   - Unterstützung für mit FLAC komprimierte Musik hinzugefügt.

 Kyrandia:
   - Unterstützung für Macintosh-Version hinzugefügt.

 Parallaction:
   - Unterstützung für Amiga-Version von Nippon Safes, Inc. hinzugefügt.
   - Viele Fehler beseitigt.

 Queen:
   - Unterstützung für AdLib-Musik hinzugefügt.
   - Wiedergabe von fehlenden Musikschemas in Amiga-Version hinzugefügt.

 SCUMM:
   - Überspringen von Untertiteln (über „.“-Taste) für ältere Spiele
     hinzugefügt, welche diese Funktion bisher nicht hatten (z. B. Zak, MM,
     Indy3, Loom).
   - Unterstützung für chinesische Version von COMI hinzugefügt.
   - Bessere Unterstützung für östliche Versionen von Spielen
   - Verschiedene Fehlerbeseitigungen für COMI und andere Spiele
   - Unterstützung für originale Spielstandmenüs hinzugefügt (nur zum
     Durchschauen). Verwenden Sie Alt+F5, um auf diese Funktion zuzugreifen.
   - Unterstützung für spanische NES-Version von Maniac Mansion hinzugefügt.
   - Bessere Unterstützung für deutsche C64-Version von Maniac Mansion
   - Fehler mit Mauszeigern in Windows-Versionen von Spielen von Humongous
     Entertainment beseitigt.

 SAGA:
   - Unterstützung für komprimierte Geräusch-Effekte, Sprachausgabe und Musik
     hinzugefügt.

 Touche:
   - Umgehungen von einigen Störungen/Problemen im Originalspiel hinzugefügt.

 WinCE-Portierung:
   - Compiler erneut gewechselt. Nun wird cegcc verwendet
     (http://cegcc.sourceforge.net/).
   - Plugins werden nun für WinCE unterstützt (aber in dieser Veröffentlichung
     nicht verwendet).
   - Aktion „Freie Ansicht“ umgestaltet, hauptsächlich für die Anforderungen der
     Lure-Engine.
   - Kleinere Optimierungseinstellung, um dem Wachstum der ausführbaren Datei
     entgegenzuwirken.
   - Verschiedene Fehler beseitigt.

 GP2X-Portierung:
   - Unterstützung für F200-Touchscreen
   - Verschiedene Fehler in Eingabe-Code beseitigt.

#### 0.X.0 "Tic-tac-toe edition" (20.06.2007)

 Neue Spiele:
   - Engine Cinematique evo 1 hinzugefügt. Momentan wird nur Future Wars
     unterstützt.
   - Engine für Touché: Die Abenteuer des fünften Musketiers hinzugefügt.
   - Unterstützung für Gobliins 2 hinzugefügt.
   - Unterstützung für Simon the Sorcerer's Game Pack hinzugefügt.
   - Unterstützung für Ween: The Prophecy hinzugefügt.
   - Unterstützung für Bargon Attack hinzugefügt.
   - Sierras AGI-Engine hinzugefügt.
   - Unterstützung für Goblins 3 hinzugefügt.
   - Parallaction-Engine hinzugefügt. Momentan wird nur Nippon Safes Inc.
     unterstützt.

 Allgemein:
   - Dialog hinzugefügt, der es ermöglicht, das Oberflächenthema zur Laufzeit
     auszuwählen.
   - „Durchsuchen“-Funktion zum Startmenü hinzugefügt, die es ermöglicht, nach
     allen bekannten Spielen in allen Unterverzeichnissen eines angegebenen
     Verzeichnisses zu suchen (um dies zu verwenden, halten Sie die
     Umschalttaste [Shift] gedrückt und klicken auf „Spiel hinzufügen“).
   - Die Art und Weise verbessert, auf welche die automatische Erkennung die
     Zielnamen generiert. Nun werden die Variante des Spiels, dessen Sprache und
     Plattform miteinbezogen.
   - Wiederholtes Tastendrücken wird nun auf zentralisierte Weise gehandhabt
     anstatt auf einer Einzelfallbasis. (Einfach ausgedrückt: Alle Engines
     handhaben nun wiederholtes Tastendrücken.)

 Baphomets Fluch 1:
   - Unterstützung für DXA-Zwischensequenzen hinzugefügt.
   - Unterstützung für Macintosh-Version hinzugefügt.

 Baphomets Fluch 2:
   - Unterstützung für DXA-Zwischensequenzen hinzugefügt.
   - „Schnellmodus“ hinzugefügt (verwenden Sie Strg+f, um diesen ein- und
     auszuschalten).

 Queen:
   - Unterstützung für Amiga-Versionen hinzugefügt.
   - Einige Soundstörungen beseitigt.

 SCUMM:
   - Unterstützung für nicht-interaktive Demos von HE-Spielen hinzugefügt (CUP).
   - Synchronisierung von Bild und Ton in SMUSH-Videos verbessert (betrifft
     The Dig, Vollgas, COMI).
   - Geschwindigkeit von NES-Sound-Code verbessert.
   - Viele (manchmal ernsthafte) Laufprobleme von Figuren behoben, insbesondere
     in Zak McKracken und Maniac Mansion, indem für diese Spiele der Code für
     das Laufen umgeschrieben wurde.
   - Einige andere Probleme behoben.
   - Unterstützung für die Wiedergabe von DXA-Filmen in HE-Spielen hinzugefügt.

 Simon:
   - Simon-Engine in AGOS umbenannt.

 Kyrandia:
   - Unterstützung für FM-TOWNS-Version hinzugefügt (sowohl für Englisch als
     auch Japanisch).

 BASS:
   - Lange bestehenden Schriftarten-Fehler beseitigt. Wir haben die Schriftart
     der Steuerungskonsole für den LINC-Bereich und Terminals verwendet, und die
     LINC-Schriftart in der Steuerungskonsole. Dies verursachte viele
     Buchstaben-Probleme (von denen wir einige umgangen haben) im
     LINC-Bereich und bei Terminals, insbesondere bei nicht-englischen Sprachen.

 Nintendo-DS-Portierung:
   - Neue unterstützte Engines: AGI, CINE und SAGA
   - Option, um den Mauszeiger anzuzeigen
   - Wortvervollständigung bei Tastatur für AGI-Spiele
   - Viele Optimierungen

 Symbian-Portierung:
   - Unterstützung für MP3 bei S60v3- und UIQ3-Versionen hinzugefügt.
   - Umstieg auf SDL 1.2.11 für Fehlerbeseitigungen und Verbesserungen
   - Verbesserte Leistung für S60v3 und UIQ3 mittels ARM Target.
   - Eingeschränkte Unterstützung für Geräte von Symbian OS9 bedingt durch
     Compiler-Probleme
   - Handhabung der Tastenzuweisung aktualisiert.

 WinCE-Portierung:
   - Wechsel zur Verwendung einer GCC-Tool-Kette zur Zusammensetzung
   - Größere Aktualisierung zur SDL-Bibliothek - besser, schneller,
     kompatibler :-)
     Die enthaltenen Fehlerbeseitigungen sind zu zahlreich, um sie hier
     aufzuführen. Die meisten Aktualisierungen in dieser Version konzentrierten
     sich auf die Infrastruktur. Diese führen zu schnellerer Ausführung,
     bedeutend erhöhter Kompatibilität und Betriebssystemfreundlichkeit -
     insbesondere für die Handhabung der Tastatur-/Maus-Eingabe und Anzeige
     (z. B. keine plötzlich auftauchenden Aufklappelemente mehr während des
     Spielens).

 Windows-Portierung:
   - Der standardmäßige Speicherort der Konfigurationsdatei wurde geändert, um
     Systeme mit mehreren Benutzern zu unterstützen.

 PalmOS-Portierung:
   - PalmOS Porting SDK wird nun verwendet, was die Verwendung der
     Standardbibliotheken der Programmiersprache C ermöglicht.

#### 0.9.1 "PalmOS revived" (29.10.2006)

 Neue Portierungen:
   - Portierung für Nintendo DS hinzugefügt.
   - Portierung für GP2X hinzugefügt.
   - Portierung für GP32 hinzugefügt.

 Allgemein:
   - Potenzielle Ausschussdaten und/oder Absturz in Debug-Konsole beseitigt.
   - Einschränkung bei 27 Spielen durch Spielkennung aufgehoben, wenn diese über
     das Startmenü hinzugefügt werden.

 SCUMM:
   - Unterstützung für internationale Versionen von HE-Spielen verbessert.
   - Seltene Figurzeichnungsstörung beseitigt.
   - Fehler beim Finden des Wegs während Smart Star Challenge in
     Big Thinkers 1st Grade beseitigt.
   - Grafikstörungen in Magenumgebung von Pyjama Sam 3 beseitigt.
   - Grafikstörungen in HE80-Version von Töff-Töff reist durch die Zeit
     beseitigt.
   - Fehler beseitigt, dass sich FM-TOWNS-Version von Indy3 bei Amiga-Portierung
     nicht starten ließ.
   - Es ist nun möglich, während SMUSH-Zwischensequenzen die Untertitel
     ein- und auszuschalten.

 Simon:
   - Unterstützung für internationale Versionen von The Feeble Files
     (Floyd - Es gibt noch Helden) verbessert.
   - Undefiniertes Verhalten beim Laden von Musik korrigiert.
   - Absturz bei der Anzeige mancher Untertitel in Floyd - Es gibt noch Helden
     beseitigt.
   - Knackender Sound in Mac-Versionen von Floyd - Es gibt noch Helden
     beseitigt.

 BASS:
   - Leerzeichenabstand in LINC-Terminals bei Diskettenversion v0.0303
     korrigiert.
   - Rückschrittsfehler beseitigt, der zu falscher AdLib-Musik-Emulation führte.

 Baphomets Fluch 1:
   - Abstürze im Zusammenhang mit Sprachausgabe beseitigt.

 Baphomets Fluch 2:
   - Stabilere Handhabung der optionalen Datei startup.inf

 Kyrandia:
   - Scrollen im Vorspann von Kyrandia benötigt nun weniger Rechenleistung,
     läuft dafür jedoch nicht mehr so glatt ab wie vorher.
   - Winzige Grafikstörung im Vorspann von Kyrandia beseitigt.
   - Handhabung der Bildschirmaktualisierung verbessert; beschleunigt die
     Darstellung auf kleinen Geräten.
   - Laden der Ressourcen verbessert; schnellerer Spielstart.

 PSP-Portierung:
   - Abstürze während scrollender Szenen in bestimmten SCUMM-Spielen beseitigt.
   - Speicherung von kleinen Vorschaubildern in SCUMM-Spielständen hinzugefügt.

 PS2-Portierung:
   - Überlagerung verwendet nun höhere Auflösung.
   - Es kann nun auch von USB, Festplatte usw. hochgefahren werden.

 WinCE-Portierung:
   - Mehrere Fehler beseitigt.
   - Unterstützung für 2002-basierende Geräte wieder hinzugefügt.

#### 0.9.0 "The OmniBrain loves you" (25.06.2006)

 Neue Spiele:
   - Kyra-Engine hinzugefügt (für die Kyrandia-Reihe). Momentan wird nur der
     erste Teil der Reihe unterstützt.
   - Unterstützung für Floyd - Es gibt noch Helden hinzugefügt.

 Allgemein:
   - Von CVS zu Subversion gewechselt.
   - Teilweise Restrukturierung unseres Quellcode-Baumes
   - Fehler beseitigt, der verhinderte, dass die Einstellung des Grafikwandlers
     für Spiele mit der Auflösung 640*480 durch die Kommandozeile übergangen
     werden konnte.
   - Untertitelsteuerungen zu Optionen-Dialog im Startmenü hinzugefügt.
   - Grafische Benutzeroberfläche wurde vollständig neu gestaltet und es können
     nun verschiedene Stil-Themen verwendet werden.

 SCUMM:
   - Spielerkennungscode umgeschrieben, für verbesserte Genauigkeit und bessere
     Unterstützung von Fan-Übersetzungen. Die Erkennung sollte nun viel besser
     mit Spielen funktionieren, deren MD5-Prüfsumme unbekannt ist.
   - Untertitelsteuerungen zu Optionen-Dialog hinzugefügt.
   - Grafische Störungen in einigen HE-Spielen beseitigt.
   - Palettenstörungen in Big Thinkers 1st Grade beseitigt.
   - Fehler bei Lied in der Küche von Pyjama Pit (1) beseitigt.

 SAGA:
   - Sound-Verzerrung in der Demo von Inherit the Earth beseitigt.

 Simon:
   - Verbesserte Unterstützung für Hebräisch
   - Viele längst überfällige Säuberungen und Restrukturierungen wurden
     vorgenommen, um mit Floyd - Es gibt noch Helden im Einklang zu sein.
   - Seltenen MIDI-Fehler beseitigt, der dazu führte, dass ein Kanal die
     Lautstärke änderte, ohne diese an die Hauptlautstärke anzugleichen.
   - Verzögerung nach Vorspann in der Demo von Simon the Sorcerer 1
     (englisch, mit Sprachausgabe) beseitigt.
   - Musiktempo in DOS-Versionen von Simon the Sorcerer 1 korrigiert.

 Baphomets Fluch 1:
   - Unterstützung für die Option --save-slot hinzugefügt.

 Baphomets Fluch 2:
   - Handhabung des Zugriffs auf Spielressourcen im Speicher umgeschrieben. Dies
     sollte Angleichungsprobleme beheben, die bezüglich einiger Plattformen
     berichtet wurden.
   - Fehlende Spieldaten werden großzügiger behandelt.

 WinCE-Portierung:
   - Hinzugefügt: PocketPC: Vertikale Überabtastungsskalierung 320x200=>320x240,
     wenn Konsole nicht gezeigt wird
   - Hinzugefügt: PocketPC: Rechtsklick bei doppeltem Antippen (n0p)
   - Behoben: Überall: Probleme in einigen Fällen beim Abschneiden von
     Grafiken
   - Hinzugefügt: PocketPC: Mausemulation durch Tasten
   - Hinzugefügt: Smartphones: Aufklappen virtueller Tastatur
   - Beseitigt: Smartphones: Fehler bei Bildschirmspeicher-Transfer bei
     SDL-Portierung
   - Hinzugefügt: Überall: Mauszeiger auf Konsole sichtbar bei Verwendung
     einer emulierten Maus
   - Hinzugefügt: Überall: Modus für umgekehrtes Querformat
   - Beseitigt: PocketPC: Dialoge abgeschnitten/nicht neu gezeichnet

#### 0.8.2 "Broken Broken Sword 2" (08.02.2006)

 Allgemein:
   - Zusammensetzen von OS-X-Paket bei Verwendung von GCC 3.3 repariert.

 SCUMM:
   - Unterstützung für das Drehen und Skalieren von Grafikobjekten in HE-Spielen
     hinzugefügt.

 Sword2:
   - Im letzten Moment Absturz beseitigt, der bei der Wiedergabe von
     Geräusch-Effekten auftrat.

 WinCE-Portierung:
   - Seltsame Abstürze mit DOTT-Briefmarke und in FOA sollten beseitigt sein
     (Fehlerberichte mit Kennung 1399830 und 1392815).
   - Fehler beseitigt, bei dem Monkey Island auf Smartphones die Tastaturkonsole
     blockierte (danke, Knakos).
   - QVGA-Smartphone-Erkennung repariert (diesmal wirklich :p).
   - Problem mit Doppelrechtsklick bei Smartphone behoben.

#### 0.8.1 "Codename: missing" (31.01.2006)

 Allgemein:
   - Kompilierung mit GCC 2.95 repariert.
   - LaTex-Dokumentation repariert.
   - Zu neuem, verbesserten Logo gewechselt, das zum neuen Seiten-Design passt.
   - Aussagekräftigere Spieletitel in allen Engines
   - Absturz beseitigt, wenn versucht wurde, Seitenverhältniskorrektur in
     Spielen zu verwenden, in welchen dies nicht möglich ist.
   - Potenzielle Sicherheitsschwachstelle mit übergroßen Pfadumgebungsvariablen
     beseitigt.
   - Standardmäßige Verstärkung für FluidSynth-Musiktreiber gemindert und sie
     konfigurierbar gemacht.

 SCUMM:
   - Optimierungen für das Scrollen in COMI, sodass es weniger Rechenleistung
     benötigt
   - Unterstützung für deutsche Maniac-Mansion-NES-Version hinzugefügt.
   - Fehler mit Maustastenzuständen in COMI beseitigt.
   - Überlauf beseitigt, wenn man Bedienungskonsole für Roboter in The Dig
     verwendet.
   - Unterstützung für Sound-Code hinzugefügt, der von Liedern in HE-Spielen
     verwendet wird.
   - Schatten in späteren HE-Spielen verbessert.
   - Untertitelstörungen in HE-Spielen beseitigt.
   - Musik/Sound für HE-Spiele verbessert.
   - Unterstützung für internationale Versionen von HE-Spielen verbessert.
   - Unterstützung für Macintosh-Versionen von Spielen verbessert.
   - Mehrere kleine Fehler beseitigt.

 BASS:
   - Absturz beseitigt, wenn Geschwindigkeits- bzw. Lautstärkeregler angeklickt
     und dann aus dem ScummVM-Fenster gezogen wurden.

 Gob:
   - Verschwinden von Mauszeiger korrigiert, wenn Level-Passwort eingegeben
     wird.
   - Anwender wird gewarnt, falls er versucht, die CD-Version unter Windows
     direkt von CD zu spielen.

 Queen:
   - Buchstabensatz für spanische Version korrigiert.

 SAGA:
   - Digitale Musikwiedergabe unter BE-Systemen repariert.

 Simon:
   - Präzisere MD5-basierte Spielerkennung integriert.
   - Unterstützung für polnische Version von Simon the Sorcerer 2 hinzugefügt.
   - Ausblenden während Ritt zu Goblinlager in Simon the Sorcerer 2 korrigiert.
   - Palettenverzögerung am Ende von Simon the Sorcerer 1 korrigiert.
   - Geräusch-Schleifen in Windows-Version von Simon the Sorcerer 2 korrigiert.

 Sword1:
   - Fehler beseitigt, bei dem Geräusch-Schleifen während Zwischensequenzen
     oder angezeigten Steuerungsdialogen weiterhin abgespielt wurden.
   - Speicherdialog behauptete irrtümlicherweise, ein Eingabe-/Ausgabe-Fehler
     sei aufgetreten, wenn die Liste der Spielstände unbenutzte Plätze hatte und
     Spielstände komprimiert waren.
   - Fehler beim Scrollen beseitigt, der dazu führte, dass Endsequenz falsch
     angezeigt wurde.

 Sword2:
   - Fehlerbeseitigungen und Säuberungen bei Abspann. Der deutsche Abspann
     funktioniert nun.
   - Fehler mit fehlender Sprachausgabe/Musik in zweiter Hälfte des Spiels
     beseitigt, was, wie berichtet wurde, in einigen Versionen des Spiels
     vorkam.

 PS2-Portierung:
   - Vollständig umgeschrieben und wird nun richtig offiziell.

 PSP-Portierung:
   - Fehler beseitigt, der Baphomets Fluch 1 zum Abstürzen brachte und dazu
     führte, dass Spiele, die extrahierte CDDA-Titel verwenden (insbesondere die
     CD-Version von Monkey Island 1), nach einer Weile nicht mehr einwandfrei
     funktionierten.

 WinCE-Portierung:
   - Lesen Sie die Datei backends/wince/README-WinCE, um die neusten
     Veränderungen zu erfahren.
   - Fehler beseitigt, bei dem die Konsole verschwand, wenn ein Listen-Widget in
     der grafischen Benutzeroberfläche geöffnet wurde.
   - Patches von Knakos (Fehlerbeseitigung bei QVGA-Smartphones, einfachere
     Tastenzuweisung und vereinfachter Konsolenwechsel)

#### 0.8.0 (30.10.2005)
 Neue Spiele:
   - SAGA-Engine hinzugefügt (für die Spiele „I Have No Mouth, and I Must
     Scream“ und „Erben der Erde“).
   - Gob-Engine hinzugefügt (für die Spielreihe Goblins). Momentan wird nur das
     erste Goblins-Spiel unterstützt.

 Neue Portierungen:
   - Portierung für PlayStation 2 hinzugefügt.
   - Portierung für PlayStation Portable (PSP) hinzugefügt.
   - Portierung für AmigaOS 4 hinzugefügt.
   - Portierung für EPOC/SymbianOS hinzugefügt.
   - Fehler in OS/2-Portierung beseitigt.

 Allgemein:
   - Mauszeiger-Handhabung im SDL-System überarbeitet. Nun können Zeiger eine
     eigene Palette und Skalierung haben. Dies wird jetzt für Spiele von
     Humongous Entertainment verwendet.
   - FluidSynth-MIDI-Treiber hinzugefügt.
   - Grafische Benutzeroberfläche für SoundFont-Einstellungen hinzugefügt
     (werden momentan nur von CoreAudio und FluidSynth-MIDI-Treibern
     verwendet).
   - Der MPEG-Abspieler konnte aussetzen, wenn der Ton vorzeitig endet.
   - Automatische Skalierung der Benutzeroberfläche verbessert, um den vollen
     Vorteil des Bildschirms auszunutzen.
   - Fehlerbeseitigungen für GCC 4

 SCUMM:
   - Unterstützung für Titel von Humongous Entertainment für Mac hinzugefügt.
   - Unterstützung hinzugefügt für mehrere Dateinamen/Versionen bei der
     Verwendung eines einzelnen Ziels.
   - Render-Modi CGA und Hercules für frühe LEC-Titel hinzugefügt.
   - Dialoge hinzugefügt, die angezeigt werden, wenn man die Textgeschwindigkeit
     oder Musiklautstärke über Tastenkürzel verändert.
   - Unterstützung für die NES-Version von Maniac Mansion hinzugefügt.
   - Unterstützung für kleine Vorschaubilder bei Speicherständen hinzugefügt.
   - Kompatibilität mit HE-Spielständen aufgehört (nur HE v71 und höher).
   - Möglichkeit hinzugefügt, die Einbeziehung der Engines HE und SCUMM v7 & v8
     bei der Kompilierung zu deaktivieren.
   - Die letzten wenigen bekannten Musikstörungen bei Sam & Max beseitigt.
     (Es gibt - wahrscheinlich - jedoch immer noch geringfügig fehlende
     Funktionen.)
   - Unterstützung für Comodore64-Version von Zak McKracken hinzugefügt.
   - Alle Demoziele und plattformspezifische Ziele entfernt. Konfigurationsdatei
     wird automatisch aktualisiert.

 Sword2:
   - Ressourcen-Manager dazu gebracht, Ressourcen intelligenter auslaufen zu
     lassen.
   - Leistung verbessert, wenn Spiel von CD anstatt Festplatte gespielt wird.
   - Handhabung von Geräusch-Effekten vereinfacht - wieder einmal.
   - Code-Säuberungen und -Restrukturierungen.
   - Lange bestehenden Fehler bei der Dekompression von Sound aus den
     CLU-Dateien für Sprachausgabe und Musik beseitigt. Es wurde eine Abtastung
     zuviel erzeugt, was als leichtes Knackgeräusch am Ende einiger Sounds
     wahrgenommen werden konnte. Dateien, die mit älteren Versionen von
     compress_sword2 komprimiert wurden, werden natürlich immer noch denselben
     Fehler haben. Möglicherweise wollen Sie diese neu erzeugen.

#### 0.7.1 (27.03.2005)
 Allgemein:
   - MT-32-Emulator hinzugefügt [er wurde eigentlich schon in Version 0.7.0
     hinzugefügt, aber wir haben vergessen, ihn in dieser Datei zu
     erwähnen :-) ].
   - MPEG-Filmwiedergabe für Baphomets Fluch 1 & 2 nun weniger speicherhungrig

 SCUMM:
   - Falsche Figurenanimation in INSANE von Vollgas beseitigt.

 Windows-Mobile-Portierung (PocketPC / Smartphone):
   - FOTAQ-Absturz auf allen Plattformen WIEDER beseitigt, wenn man Hotel
     verlässt.
   - Bessere Emulation niedriger Qualität von AdLib für FOTAQ
   - Zufällig defekte Option zum Verstecken der Werkzeugleiste repariert
     (danke, iKi).
   - Erste Hardware-Tastenzuweisung repariert (wurde vorher nicht angezeigt).
   - Kurze Aussetzer in BASS & Simon beim Smartphone beseitigt, wenn
     Tastenkürzel zum Überspringen von Sätzen verwendet wird.
   - Zonentastenaktion beim Smartphone repariert (nun der Taste 9 zugewiesen).
   - Experimentelle Fehlerbeseitigung Dritter für VGA-Modus (SE-VGA)
   - Tastenzuweisungsoption im Startmenü hinzugefügt
     (ab Version 1.2.0 in deutscher Lokalisation zu finden unter:
     Optionen / Sonstiges / Tasten).
   - Abhängigkeit von AYGSHELL.DLL beseitigt, um auf Plattformen mit CE .Net zu
     funktionieren.
   - Probleme mit Tastenzuweisung behoben, die sich in 0.7.0 eingeschlichen
     haben.
   - Interaktive Action-Sequenz in Vollgas sollte spielbarer sein.
   - Neue Tastenzuweisungsoption „FT Cheat“, um alle Action-Sequenzen in Vollgas
     zu gewinnen
   - Beenden von Simon-Spiel bei Smartphones mit Aktionstaste

 Sword2:
   - Absturz beseitigt, der aufgetreten ist, wenn versucht wurde, Musik von CD 1
     und CD 2 gleichzeitig wiederzugeben.
   - Absturz bei Zwischensequenz-Abspieler beseitigt, wenn Sprachdatei fehlt.

 BASS:
   - Absturz beseitigt, wenn man mit spanischem Text in Diskettenversion mit dem
     Gärtner redet.

#### 0.7.0 (24.12.2004)
 Neue Spiele:
   - 26 Titel von Humongous Entertainment hinzugefügt; nur wenige sind
     durchspielbar.

 Allgemein:
   - Unterstützung für mit FLAC (verlustfrei) komprimierte Audio-Dateien
     hinzugefügt.
   - Bildschirmanzeige zu SDL-System hinzugefügt.
   - Teilweise API-System umgeschrieben.
   - Kommentare und die Reihenfolge der Abschnitte in der Konfigurationsdatei
     werden nun beibehalten.
   - Grafikwandler AdvMame auf Grundlage von scale2x 2.0 aktualisiert -
     AdvMame3x sieht nun schöner aus und AdvMame 2x wird durch MMX beschleunigt.
   - MMX-i386-Assembler-Versionen von HQ2x- und HQ3x-Grafikwandlern hinzugefügt.
   - Option für „Extrapfad“ hinzugefügt, der die Einbindung von Spieldateien in
     einem zusätzlichen Verzeichnis erlaubt (z. B. für umgewandelte
     Zwischensequenzen oder Ähnliches).
   - Tastenkombinationen Alt+x und Strg+z für das Beenden auf Systemen der Marke
     Unix (wie z. B. Linux) deaktiviert - zugunsten von Strg+q
     (Ausnahme: Mac OS X verwendet nach wie vor Cmd+q).
   - Gesonderte kleinere Schriftart für die Konsole, wodurch mehr Informationen
     auf einen Blick sichtbar werden, z. B. im SCUMM-Debugger
   - Unterstützung für das Einstellen der Ausgabefrequenz zur Laufzeit
     hinzugefügt, auch wenn es hierfür noch keine Option in der
     Benutzeroberfläche gibt.
   - Anstatt des aktuellen Verzeichnisses wird nun folgendes Standardverzeichnis
     auf folgenden Plattformen für Spielstände verwendet:
         - Mac OS X:            $HOME/Documents/ScummVM Savegames/
         - Andere UNIX-Systeme: $HOME/.scummvm/
   - Neuen „Über“-Dialog mit rollender Liste aller Mitwirkenden hinzugefügt.

 SCUMM:
   - Das alte Ziel zak256 entfernt; zakTowns wird nun stattdessen verwendet.
   - Native Unterstützung für Macintosh-Versionen durch Verwendung einer
     speziellen Container-Datei hinzugefügt.
   - Glatter horizontaler Bildlauf für The Dig, Vollgas und COMI hinzugefügt
     (mit der originalen Engine übereinstimmend).
   - Teilweise Text-Engine umgeschrieben, was verschiedene Fehler beseitigt,
     besonders in neueren Spielen (The Dig, COMI).
   - Zeichnungsstörungen bei Figuren in V1 von Maniac und Zak beseitigt.
   - Schiff-zu-Schiff-Grafikstörungen in COMI beseitigt.
   - Palettenstörungen in COMI beseitigt.

 Queen:
   - Einige Probleme bei Dreamcast-System behoben.

 Sword1:
   - Unterstützung für komprimierte Sprachausgabe und Musik hinzugefügt.
   - Unterstützung für die Demo hinzugefügt.
   - Bessere Unterstützung für tschechische Version
   - Umgehungslösungen für in einigen Spielversionen vorhandene Skript- und
     Untertitel-Fehler hinzugefügt.

 Sword2:
   - Speicher-/Ressourcen-Verwaltung vereinfacht.
   - Handhabung von Geräusch-Effekten vereinfacht.
   - Unterstützung für komprimierte Sprachausgabe und Musik hinzugefügt.
   - Verschiedene kleinere Fehler beseitigt.

 BASS:
   - Umgehungslösungen für einige seltene Skriptfehler hinzugefügt, durch deren
     Auftreten man das Spiel nicht mehr gewinnen konnte.

#### 0.6.1b (03.08.2004)
 Allgemein:
   - Fehler bei Kopieren und Einfügen im Startmenü beseitigt, der den
     Speicherpfad beschädigen konnte.
   - Abstürze bei 64-Bit-Architekturen beseitigt.

 SCUMM:
   - Durch VOC verursachten Absturz in der DOTT-Diskettenversion beseitigt.
   - Palettenprobleme in Amiga-Version von MI2 behoben.

 Simon:
   - Durch VOC verursachten Absturz beseitigt.

#### 0.6.1 (25.07.2004)
 Allgemein:
   - Sound-Störung beseitigt, wenn zwei oder mehr Ogg-Vorbis-Sounds als
     Datenstrom aus demselben Datei-Handle ausgegeben werden, z. B. im Vorspann
     von Sam & Max bei Verwendung der Datei monster.sog.

 SCUMM:
   - Wie gewöhnlich: viele Fehlerbeseitigungen für Spiele der SCUMM-Engine
   - Grafikdekodierer für 3DO-Spiele von Humongous Entertainment hinzugefügt.
   - Zahlreiche Fehlerbeseitigungen für Spiele von Humongous Entertainment
   - Fehler in Vollgas beseitigt, sodass Schwierigkeitsgrad im Kampf dem des
     Originals entspricht.
   - Digital iMuse verbessert.

 Sword1:
   - ScummVM warnt nun den Anwender, wenn Speichern fehlschlägt, anstatt
     abzustürzen.
   - Leicht anwenderfreundlicherer Dialog zum Speichern/Laden
   - Bildmaskenfehler vor Nicos Wohnung beseitigt.

 Simon:
   - Abstürze in einigen internationalen Versionen beseitigt.

#### 0.6.0 (14.03.2004)
 Neue Spiele:
   - Engine für Baphomets Fluch 1 hinzugefügt.
   - Engine für Baphomets Fluch 2 hinzugefügt.
   - Engine für Flight of the Amazon Queen hinzugefügt.
   - Unterstützung für die V1-SCUMM-Spiele Maniac Mansion und Zak McKracken
     hinzugefügt.
   - SCUMM-Spiel Vollgas wird nun unterstützt.

 Allgemein:
   - Untertitel sind nun standardmäßig ausgeschaltet. Kommandozeilen-Option „-n“
     aktiviert die Untertitel von jetzt an.
   - Grafikwandler HQ2x und HQ3x hinzugefügt.
   - Sound-Code für mehr Flexibilität und Effizienz umgeschrieben.
   - Native MT-32-Unterstützung verbessert.
   - AdLib-GM-Emulationstabelle so umgestaltet, dass sie genaueres Software-MIDI
     bietet.
   - Standardmäßige Datei Makefile verwendet nun Konfigurationsskript.
   - Startmenü und Optionsdialoge erheblich verbessert (in Bearbeitung).
   - Viele weitere Verbesserungen „unter der Motorhaube“ wie z. B. die neue
     Konfigurationshandhabung und Plugin-Fähigkeiten

 Simon:
   - Spieldateien-Dekodierer für Amiga-Diskettenversionen hinzugefügt.
   - Unterstützung für Inventargrafiken in Amiga-Versionen hinzugefügt.
   - Verschiedene Vorfälle kurzer Aussetzer im Spiel beseitigt.
   - Kleinere Störungen im Dialog Laden/Speichern in nicht-englischen Versionen
     beseitigt.
   - Problem mit fehlenden Inventarpfeilen in einigen Versionen von
     Simon the Sorcerer 1 wurde behoben.

 SCUMM:
   - Viele, viele, viele Fehlerbeseitigungen für die SCUMM-Engine. Wirklich
     viele. Und das ist eine ganze Menge.
   - INSANE-Unterstützung für die „Action-Sequenzen“ von Vollgas hinzugefügt.
   - Option zur Auswahl zwischen AdLib, PCjr und PC-Lautsprecher für frühere
     Spiele hinzugefügt.
   - AdLib-Unterstützung für indy3ega und loom (EGA) hinzugefügt.
   - MIDI-Unterstützung für loom (EGA), monkeyega und monkeyvga hinzugefügt.
   - Unterstützung für Geräusch-Effekte für indy3/monkeyega/monkeyvga/pass
     hinzugefügt.
   - FM-TOWNS-Ziele für Loom und Indy3 hinzugefügt.
   - Menü im Spiel (aufrufbar mit F5) umgeschrieben, um einfachere Verwendung zu
     ermöglichen.
   - Verbesserte Unterstützung für Sound in FM-TOWNS (YM2612-Emulation,
     Geräusch-Schleifen).
   - Klassische SCUMM-V1-Varianten von Maniac Mansion und Zak McKracken werden
     nun unterstützt und sind durchspielbar.
   - Musiksystem Digital iMUSE umgeschrieben.
   - Mehrere Fehler im Musiksystem Analog iMUSE beseitigt.
   - Musik und Sound für verschiedene Amiga-Versionen verbessert.
   - Kompression für Audio-Dateien von Fate of Atlantis und Simon the Sorcerer 2
     verbessert.
   - Tastaturkampf funktioniert nun in Fate of Atlantis.
   - Tastaturunterstützung für Kanonenkampf in Curse of Monkey Island
   - Tastaturunterstützung für Rennfahr-Szene in Vollgas

#### 0.5.1 (06.08.2003)
- Programm-Code für Spielstände von Beneath a Steel Sky umgeschrieben (siehe
  Hinweis in README unter „Known Bugs“).
- Beseitigung von Fehlern beim Überspringen von Dialogen sowie bei der
  Musiklautstärke und von mehreren Abstürzen/Ursachen für das Aufhängen in
  Beneath a Steel Sky
- Fehler beim Überspringen von Dialogen in V7-SCUMM-Spielen beseitigt.
- Störung beim Beenden von ScummVM im Vollbildmodus unter Mac OS X
  beseitigt.
- Mehrere COMI-Fehler beseitigt im Zusammenhang mit der Platzierung/Skalierung
  von Figuren.
- Vollständige Hebräisch-Unterstützung für Simon the Sorcerer 1 und 2
  hinzugefügt.
- Mehrere Fehlerbeseitigungen für MorphOS- und Dreamcast-Portierung
- Auf Dreamcast laufen nun Simon the Sorcerer 1 & 2.
- Vollbildschirmmodus-Problem unter Mac OS X behoben, bei welchem man die Maus
  nicht im oberen Teil des Bildschirms verwenden konnte, indem zu einer
  fehlerbereinigten Version von SDL verlinkt wurde.

#### 0.5.0 (02.08.2003)
- Verbesserte Versionen von Maniac Mansion und Zak McKracken werden nun
  unterstützt und sind komplett durchspielbar.
- Unterstützung für Amiga-Version von Monkey Island 1 hinzugefügt.
- Erste Unterstützung (nicht spielbar) für V1-Variante von Maniac Mansion/
  Zak McKracken
- Unterstützung zum Spielen von Curse of Monkey Island von CD unter Mac OS X
  verbessert.
- Um Spielstände für CD 2 zu laden, wird nicht mehr zuerst die CD 1 benötigt.
- Engine von iMuse umgeschrieben und viele Musikfehler beseitigt (besonders bei
  Monkey Island 2).
- Unterstützung für Musik in DOS-Versionen von Spielen von Humongous
  Entertainment und Simon the Sorcerer 2 (XMIDI-Format)
- Vollständige Musikunterstützung für Simon the Sorcerer 2.
- Musik- und Sound-Unterstützung in Zak256 verbessert.
- Option für Seitenverhältniskorrektur hinzugefügt.
- Viele weitere Fehlerbeseitigungen, Verbesserungen und Optimierungen

#### 0.4.1 (25.05.2003)
- Filter AdvMame3x hinzugefügt.
- Absturz in Curse of Monkey Island (und wahrscheinlich auch anderen Spielen)
  beseitigt.
- Flughafentüren in Zak256 korrigiert.
- Absturz in SDL-System beseitigt.
- Mehrere Fehler bei iMuse beseitigt.

#### 0.4.0 (11.05.2003)
- Unterstützung für Curse of Monkey Island (experimentell)
- Unterstützung für EGA-Versionen von Loom, Monkey Island und Indy3 hinzugefügt.
- Verbesserte Musikunterstützung in Indy3 und Diskettenversionen von
  Monkey Island
- Viele Verbesserungen und Fehlerbeseitigungen für Simon the Sorcerer 1 & 2
- Programm-Code im sehr frühen Entwicklungsstadium für Beneath a Steel Sky.
  Bitte nicht erwarten, dass er irgendetwas tut.
- Noch mehr Unterstützung im frühen Entwicklungsstadium für V2-SCUMM-Spiele
  (Maniac Mansion und Zak)
- Vorbereitende Unterstützung für frühe Titel von Humongous Entertainment (sehr
  experimentell)
- Neue Debug-Konsole und mehrere Verbesserungen bei Benutzeroberfläche und
  Startmenü mit Spieleliste
- Neuer Programm-Code für das Speichern und Laden (einfacher zu erweitern,
  während Kompatibilität erhalten wird)
- Dreamcast-Portierung funktioniert nun mit neuen Spielen, für welche die
  Unterstützung in Version 0.3.0b hinzukam.
- Neue offizieller PalmOS-Portierung
- Verschiedene kleine und nicht so kleine Fehlerbeseitigungen für SCUMM-Spiele
- Großes Speicherleck bei The Dig/ComI beseitigt.
- SMUSH-Code optimiert, Auslassen von Bildern für langsamere Maschinen
  hinzugefügt.
- Programm-Code-Säuberungen

#### 0.3.0b (08.12.2002)
- Massive Säuberungsarbeiten bei iMUSE. Die Musik in Sam & Max wird nun korrekt
  abgespielt.
- Viele Fehlerbeseitigungen für Zak256 + Unterstützung für Sound und Musik
- Musikunterstützung für Simon the Sorcerer auf Plattformen mit echtem MIDI
- Experimentelle Unterstützung für Indy3 (VGA) - Indiana Jones and the
  Last Crusade
- Vervollständigte Unterstützung für Monkey1-VGA-Diskette, The Dig
- Implementierung von akos16 durchgeführt für Costumes (Grafikdateien von
  Figuren) in The Dig und Vollgas.
- Implementierung von Digital iMuse für Musik in The Dig und Vollgas
- Synchronisation von Sprachausgabe und Musik bei Loom-CD erheblich verbessert.
- MIDI-Emulation durch AdLib hinzugefügt, für Plattformen ohne Sequencer-
  Unterstützung.
- Programm-Code-Trennung verschiedener Engine-Teile in einzelne Bibliotheken
- Mehrere Fehler beseitigt, um Abstürzen und Aufhängen in Simon the Sorcerer
  zu verhindern.
- Hunderte Fehlerbeseitigungen für viele andere Spiele
- Neue SMUSH-Video-Engine, für Vollgas und The Dig
- Neue Benutzeroberfläche im Spiel
- Startmenü mit Spieleliste

#### 0.2.0 (14.04.2002)
- Engine-Kern umgeschrieben.
- Verbesserte Benutzeroberfläche im Spiel, einschließlich
  Optionen-/Lautstärke-Einstellungen
- Funktion für automatisches Speichern
- Weitere Kommandozeilenoptionen und Konfigurationsdatei hinzugefügt.
- Neue Portierungen und Plattformen (MorphOS, Macintosh, Dreamcast, Solaris,
  IRIX, usw.)
- Grafikfilter hinzugefügt (2xSAI, Super2xSAI, SuperEagle, AdvMame2x).
- Unterstützung für mit MAD MP3 komprimierte Audio-Dateien
- Unterstützung für erste Nicht-SCUMM-Spiele (Simon the Sorcerer)
- Unterstützung für V4-SCUMM-Spiele (Loom-CD)
- Verbesserte Unterstützung von V6-SCUMM-Spielen (Sam & Max ist nun komplett
  durchspielbar)
- Experimentelle Unterstützung für V7-SCUMM-Spiele (Vollgas/The Dig)
- Experimentelle Unterstützung für V3-SCUMM-Spiele (Zak256/Indy3)

#### 0.1.0 (13.01.2002)
- Eine Menge Änderungen

#### 0.0.2 (12.10.2001)
- Fehlerbeseitigungen
- Unterstützung für Laden & Speichern

#### 0.0.1 (08.10.2001)
- Erste Version
