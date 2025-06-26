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

#ifndef TOT_TEXTS_H
#define TOT_TEXTS_H
 #include "common/str.h"
#include "common/keyboard.h"

namespace Tot {


static const char *const animMessages[2][290] = {
//Spanish
{
	"Las \xA3ltimas  encuestas  realizadas",
	"acerca  de  la  siniestralidad  en ",
	"los transportes arrojan resultados",
	"alentadores...",
	"",
	// 5
	"El cien por cien  de los encuestados",
	"declararon no haber sufrido nunca un",
	"accidente mortal...",
	"",
	"",
	// 10
	"Es una noticia que nos anima a salir",
	"de viaje con m\xA0s seguridad.",
	"",
	"",
	"",
	// 15
	"\xAD\xADPor un tornillo!!...",
	"Por un m\xA1sero y simple",
	"tornillo  del  tr\xA1""fugo",
	"del trapecio...",
	"",
	// 20
	"Si lo hubiese apretado",
	"bien, no estar\xA1""a ahora",
	"sin trabajo...",
	"",
	"",
	// 25
	"\xAD\xADNoticia de \xA3ltima hora!!",
	"",
	"",
	"",
	"",
	// 30
	"Llevo  sin dormir toda",
	"la noche, pensando qu\x82",
	"voy a hacer.",
	"",
	"",
	// 35
	"Seguramente  no  podr\x82",
	"encontrar  trabajo  en",
	"mucho tiempo...",
	"Aunque  ponga mucha f\x82",
	"la oficina de empleo.",
	// 40
	"Y  para  animarme  m\xA0s",
	"todav\xAD""a, ma\xA4""ana  noche",
	"es HALLOWEEN...",
	"",
	"Es que tengo la negra.",
	// 45
	"Grave  accidente  en  TWELVE MOTORS...",
	"un empleado olvid\xA2 apretar un tornillo",
	"y media  cadena  de montaje ha quedado",
	"para chatarra...",
	"las p\x82rdidas se  estiman cuantiosas...",
	// 50
	"No se descartan las acciones legales",
	"contra ese inconsciente...",
	"",
	"",
	"",
	// 55
	"\xAD""Dios mio!... hasta en",
	"la televisi\xA2n...",
	"Seguro  que  hasta  mi",
	"perro  encuentra curro",
	"antes que yo...",
	// 60
	"\xADHORROR!...una factura",
	"",
	"",
	"",
	"",
	// 65
	"\xA8"
	"Oficina de empleo?...",
	"\xA8"
	"y para m\xA1?...",
	"",
	"\xADSe habr\xA0n equivocado!", "",
	// 70
	"\xA8Tan  pronto lograron",
	"encontrarme  trabajo?",
	"",
	"Claro... antes de ver",
	"las noticias...",
	// 75
	"Estimado se\xA4or: Nos dirigimos a",
	"usted  para  comunicarle que su",
	"solicitud n\xA7:93435 fu\x82 aceptada",
	"y le hemos encontrado un puesto",
	"de acuerdo con sus aptitudes...",
	// 80
	"Deber\xA0 presentarse  ma\xA4""ana, dia",
	"31  de  Octubre en la direcci\xA2n",
	"abajo indicada,  para ocupar el",
	"puesto de ENCARGADO GENERAL.",
	"",
	//85
	"Le adjuntamos las  credenciales",
	"de presentaci\xA2n.",
	"Deseando no verle  por  aqu\xA1 en",
	"mucho tiempo se despide:",
	"    Leonor Scott.",
	// 90
	"\xAD\xADQue ilu!! encargado",
	"general...  si ya  me",
	"lo decia  mi madre...",
	"<<nene tienes cara de",
	"encargado general>>",
	// 95
	"Tendr\x82  que conseguir",
	"ropa  m\xA0s  de acuerdo",
	"con mi nuevo cargo.",
	"",
	"Manos a la obra...",
	// 100
	"\xADV\xA0yase!... no queremos vendedores...",
	"y menos de planes de jubilaci\xA2n.",
	"",
	"",
	"",
	// 105
	"No soy ning\xA3n vendedor... me envia la oficina",
	"de empleo... tenga mis credenciales.",
	"",
	"",
	"",
	// 110
	"\xADVaya! eres t\xA3...llegas con retraso. Los ancianos est\xA0n",
	"esper\xA0ndote en el sal\xA2n. Debes cuidarlos muy bien.",
	"",
	"",
	"",
	// 115
	"Lo siento, no sab\xA1""a a que hora deb\xA1""a presentarme, en",
	"la oficina de empleo no me lo dijeron.",
	"",
	"",
	"",
	// 120
	"Bueno... no importa... ya est\xA0s aqu\xA1.",
	"Ese coche es tuyo \xA8no?...",
	"",
	"",
	"",
	// 125
	"Si... es precioso \xA8verdad?...",
	"",
	"",
	"",
	"",
	// 130
	"Vaya... 200 caballos... elevalunas el\x82""ctricos... ",
	"inyecci\xA2n electr\xA2nica, llantas de aleaci\xA2n...",
	"",
	"",
	"",
	// 135
	"Pues si...",
	"",
	"",
	"",
	"",
	// 140
	"Airbag... asientos envolventes... aire acondicionado",
	"",
	"",
	"",
	"",
	// 145
	"Es el m\xA0s alto de la gama...",
	"",
	"",
	"",
	"",
	// 150
	"ABS, equipo de m\xA3sica con CD, estructura reforzada",
	"indeformable, detector de radar...",
	"",
	"",
	"",
	// 155
	"Parece que te gusta, \xA8no?", "", "", "", "",
	// 160
	"Bueno, no est\xA0 mal... pero... gracias de todos modos.", "", "", "", "",
	// 165
	"\xA8Gracias?... \xA8qu\x82?... \xAD"
	"espera!... ese es mi coche...",
	"",
	"",
	"",
	"",
	// 170
	"Toma una copita, te la has ganado...", "", "", "", "",
	// 175
	"\xA8""Donde estoy? ... \xA8Qu\x82 me ha pasado?", "", "", "", "",
	// 180
	"Tranquilizate, no est\xA0s muerto...",
	"Est\xA0s tras la puerta de acero...",
	"",
	"",
	"",
	// 185
	"Te necesitamos para encontrar a Jacob,",
	"nuestro Gran Maestre.",
	"",
	"",
	"",
	// 190
	"Hace unos dias se puso a estudiar el libro",
	"que me diste y desapareci\xA2...",
	"",
	"",
	"",
	// 195
	"Creemos que realiz\xA2 un conjuro sin darse cuenta...",
	"",
	"",
	"",
	"",
	// 200
	"Por fortuna, pudimos ponernos en contacto",
	"con \x82l, y nos dijo que est\xA0 en problemas...",
	"",
	"",
	"",
	// 205
	"Nos pidi\xA2 que envi\xA0semos a alguien en su ayuda,",
	"y TU eres el elegido...",
	"",
	"",
	"",
	// 210
	"Para ir a las CAVERNAS ETERNAS debes morir...",
	"Ning\xA3n cuerpo con alma puede estar all\xA1...",
	"",
	"",
	"",
	// 215
	"Haremos un ritual para crear un v\xA1nculo entre tu",
	"cuerpo y tu alma, s\xA2lo as\xA1 podr\xA0s volver a la vida...",
	"",
	"",
	"",
	// 220
	"Si rescatas a Jacob, el v\xA1nculo te traer\xA0 de regreso...", "", "", "", "",
	// 225
	"Ahora... \xADPrep\xA0rate!... Ha llegado tu hora...", "", "", "", "",
	// 230
	"\xADGracias ILUSO!...", "", "", "", "",
	// 235
	"Ahora tendr\x82 un cuerpo nuevo y podr\x82",
	"volver a la vida...",
	"",
	"",
	"",
	// 240
	"Tu mientras tanto, puedes seguir en",
	"las CAVERNAS ETERNAS...",
	"",
	"",
	"",
	// 245
	"No te preocupes, seguro que nos mandan",
	"otro encargado...  JA,JA,JA...",
	"",
	"",
	"",
	// 250
	"I knew I shouldn't of",
	"listened to her. \"Oh,",
	"it'll only take you a",
	"minute. I Know it's",
	"supposed to be Henry's",
	// 255
	"by saying that he would have more",
	"to say on that subject in a week",
	"or two!",
	"",
	"",
	// 260
	"be his own reputation",
	"that he's thinking of.",
	"",
	"",
	""
},
//English
{
	"The  latest  nationwide car safety",
	"surveys have  shown very  positive",
	"results    regarding    accidental",
	"accidents...",
	"",
	// 5
	"100%  of  the  people  interviewed",
	"reported that never in their lives",
	"had they  ever  suffered  a  fatal",
	"accident...",
	"",
	// 10
	"This being  very  reassuring news",
	"indeed for today's motorist...",
	"",
	"",
	"",
	// 15
	"I don't believe it!",
	"Sacked again!  And",
	"for what? One little",
	"screw on the boss's",
	"wife's car!",
	// 20
	"responsability but",
	"who'll ever know?\"",
	"How could I have been",
	"so stupid?",
	"supposed to be Henry's",
	// 25
	"In other news.  The Prime Minister",
    "was questioned today in the Commons",
	"today about his being a constant",
	"procrastinator.  He infatically",
	"denied the alegations and responded",
	// 30
	"I knew I should have",
	"let Holmes do it. He",
	"was always better",
	"than me and his tools",
	"were far superior!",
	// 35
	"I probably won't work",
	"in this area ever",
	"again. Henry Fjord",
	"has a lot of say in",
	"this town and it'll",
	// 40
	"Trust this to happen",
	"to me just before",
	"Halloween. This is",
	"definately not my",
	"week!",
	// 45
	"This just in. A major incident has been",
	"reported within the Fjord Motor Group.",
	"",
	"",
	"",
	// 50
	"Legal action is being considered...",
    "",
	"",
	"",
	"",
	// 55
	"Oh bugger! It's on",
	"T.V.  Legal action?!",
	"How the hell can",
	"I afford that?",
	"",
	// 60
	"Hello?  What's this?","","","","",
	// 65
	"Employment Services?!",
	"For me?! This must be",
	"a mistake. How could",
	"they know already?",
	"",
	// 70
	"More to the point,",
	"how could they have",
	"found me a job?!",
	"Let's see what this",
	"says...",
	// 75
	"\"Dear Sir. We would like to inform",
	"you that your application form",
	"no:666 has been processed. We are",
	"also pleased to tell you that we",
	"",
	// 80
	"have found you a job best suited",
    "to your abilities\"",
	"",
	"",
	"",
	//85
	"We enclose the necessary documents",
	"for you records. We wish you every",
	"success and sincerly hope never to",
	"see you again.  Signed: R. Sole.\"",
	"",
	// 90
	"Care Assistant!?!",
	"Someone's obviously",
	"taking the...",
	"",
	"",
	// 95
	"Fjord! I bet he's been",
	"on the phone to the",
	"Unemployment Office and",
	"told them to give me",
	"something naff!... ",
	// 100
	"Sod off!... We don't wan't any more salesmen!",
    "",
	"",
	"",
	"",
	// 105
	"I'm not a salesman, I work here!",
	"",
	"",
	"",
	"",
	// 110
	"So you're the new Care Assistant I requested!...",
	"Well I must say that I'm very pleased to meet you.",
	"",
	"",
	"",
	// 115
	"Hey wait a minute!  What am I supposed",
	"to do around here?",
	"",
	"",
	"",
	// 120
	"The geriatrics will tell you all that",
	"you need to know. I'm out of here! ...",
	"",
	"",
	"",
	// 125
	"Eh! Er, yes it is. Beware what exactly!",
	"",
	"",
	"",
	"",
	// 130
	"Wow!... 200Bhp... Electric everything...",
	"Fuel injection...  Alloy wheels...",
	"",
	"",
	"",
	// 135
	"Er, excuse me! Beware what?",
	"",
	"",
	"",
	"",
	// 140
	"Twin overhead cams... Dual airbags...",
	"Bucket seats... Air conditioning...",
	"",
	"",
	"",
	// 145
	"Hey man, I hate to break you away from your",
	"car review...",
	"",
	"",
	"",
	// 150
	"A.B.S... C.D. system... Side impact protection",
	"system... Wow! A radar detector ...",
	"",
	"",
	"",
	// 155
	"Have I?",
	"",
	"",
	"",
	"",
	// 160
	"Ooohhh yes!","","","","",
	// 165
	"Hey wait! That's my car! Come back!",
	"",
	"",
	"",
	"",
	// 170
	"Toma una copita, te la has ganado...", "", "", "", "",
	// 175
	"\xA8""Donde estoy? ... \xA8Qu\x82 me ha pasado?", "", "", "", "",
	// 180
	"Tranquilizate, no est\xA0s muerto...",
	"Est\xA0s tras la puerta de acero...",
	"",
	"",
	"",
	// 185
	"Te necesitamos para encontrar a Jacob,",
	"nuestro Gran Maestre.",
	"",
	"",
	"",
	// 190
	"Hace unos dias se puso a estudiar el libro",
	"que me diste y desapareci\xA2...",
	"",
	"",
	"",
	// 195
	"Creemos que realiz\xA2 un conjuro sin darse cuenta...",
	"",
	"",
	"",
	"",
	// 200
	"Por fortuna, pudimos ponernos en contacto",
	"con \x82l, y nos dijo que est\xA0 en problemas...",
	"",
	"",
	"",
	// 205
	"Nos pidi\xA2 que envi\xA0semos a alguien en su ayuda,",
	"y TU eres el elegido...",
	"",
	"",
	"",
	// 210
	"Para ir a las CAVERNAS ETERNAS debes morir...",
	"Ning\xA3n cuerpo con alma puede estar all\xA1...",
	"",
	"",
	"",
	// 215
	"Haremos un ritual para crear un v\xA1nculo entre tu",
	"cuerpo y tu alma, s\xA2lo as\xA1 podr\xA0s volver a la vida...",
	"",
	"",
	"",
	// 220
	"Si rescatas a Jacob, el v\xA1nculo te traer\xA0 de regreso...", "", "", "", "",
	// 225
	"Ahora... \xADPrep\xA0rate!... Ha llegado tu hora...", "", "", "", "",
	// 230
	"\xADGracias ILUSO!...", "", "", "", "",
	// 235
	"Ahora tendr\x82 un cuerpo nuevo y podr\x82",
	"volver a la vida...",
	"",
	"",
	"",
	// 240
	"Tu mientras tanto, puedes seguir en",
	"las CAVERNAS ETERNAS...",
	"",
	"",
	"",
	// 245
	"No te preocupes, seguro que nos mandan",
	"otro encargado...  JA,JA,JA...",
	"",
	"",
	"",
	// 250
	"I knew I shouldn't of",
	"listened to her. \"Oh,",
	"it'll only take you a",
	"minute. I Know it's",
	"supposed to be Henry's",
	// 255
	"by saying that he would have more",
	"to say on that subject in a week",
	"or two!",
	"",
	"",
	// 260
	"be his own reputation",
	"that he's thinking of.",
	"",
	"",
	"",
	// 265
	"\"As from October 31st you will begin",
	"you new job at the care home for the",
	"elderly at Devillsville as:",
	"          CARE ASSISTANT.",
	"",
	// 270
	"Well stuff you Henry,",
	"I'm taking the job",
	"anyway! Besides I",
	"need the money!",
	"",
	// 275
	"You can find the geriatrics in the livingroom.",
	"... Good luck!",
	"",
	"",
	"",
	// 280
	"Oh!, one more thing though... Beware the...",
	"... Hey is that your car?",
	"",
	"",
	"",
	// 285
	"... And look at this! ...",
	"You've even left the keys in it!",
	"",
	"",
	""
}
};

static const char *const fullScreenMessages[2][56] = {
	// Spanish
    {
	// intro
    " ... Despu\x82s  de  estar  durante  ocho horas",
    "conduciendo,  haberte   perdido  tres  veces",
    "y  haber hecho  doscientos kil\xA2metros de m\xA0s",
    "llegas  a  una  casa  que se encuentra en un",
    "paraje muy tranquilo y apartado del mundanal",
    "ruido. (\xAD\xADY TAN APARTADO!!) ...",
    "Pero esto no tiene importancia ... ",
    "Hay que  encarar  el trabajo con una actitud",
    "optimista y positiva...",
    // sacrifice scene
    // 9
    "    A pesar  de todo, no lo  has  hecho tan mal.",
    "    Has  conseguido todo  lo que te  pidieron, y",
    "    van a poder celebrar su fiesta de Halloween.",
    "    Es  todo un  detalle que t\xA3 seas el invitado",
    "    de honor...",
    // 14
    "    Como recompensa a tus esfuerzos te mandan a",
    "    darte una  vuelta  por ah\xA1 mientras ellos se",
    "    encargan de  los  preparativos de la fiesta.",
    "    T\xA3, obviamente, aceptas  de  inmediato  y te",
    "    vas a tomar el fresco un rato...",
    //19
    "                  Horas m\xA0s tarde... ",
    "    La  fiesta  ya  est\xA0  preparada, regresas al",
    "    sal\xA2n a  recibir tus merecidos honores, tras",
    "    un completo dia de trabajo... ",
    // 23
    "      Despu\x82s de todo lo que hice por ellos...",
    "      Todos mis esfuerzos y desvelos para esto...",
    "      Morir envenenado, ese era mi destino...",
    // 26
    "   Presta atenci\xA2n, pues esta puede  ser la \xA3ltima",
    "   vez que hablemos contigo. Definitivamente est\xA0s",
    "   muerto, pero a\xA3n es  posible reunir tu alma con",
    "   tu cuerpo,  para  ello  debes  buscar a Jacob y",
    "   liberarlo de aquello  que lo retenga. Cuando lo",
    "   consigas podremos traeros  a ambos de vuelta al",
    "   mundo de los vivos.",
    // 33
    "   Pero debes darte prisa  pues el v\xA1nculo con tu",
    "   cuerpo se  debilita  y tu  alma podr\xA1""a  quedar",
    "   atrapada para siempre en las Cavernas Eternas.",
    "   Si necesitaras  m\xA0s  energ\xA1""a de la que tienes, ",
    "   recuerda que all\xA0 donde vas un cuerpo contiene",
    "   m\xA0s energ\xA1""a que un alma al contrario que en el",
    "   mundo f\xA1sico.",
    // 40
    "              No te demores y recuerda...",
    "      Si triunfas, el premio es la vida, si no...",
    "                  LA MUERTE ETERNA.",
    // 43
    "           Al fin lo has conseguido....",
    "               \xAD\xAD\xADSoy LIBREEEEE!!!",
    // WC
    // 45
    "... MMPFFFF!! ...",
    "... PPTRRFF!! ...",
    "... GGNNNNN!! ...",
    "... AAAHHHHH!! ...",
    // Diploma
    // 49
    "CERTIFICADO N\xA7:",
    "Yo, Jacob, el Gran Maestre ",
    "de la Hermandad de Sek Umh Nejl",
    "certifico que: ",
    "me ha liberado de las Cavernas",
    "Eternas, por lo cual le estar\x82 ",
    "por siempre agradecido."
	},

	// English
	{
	// intro
    "... After  driving  around  in  circles for what ",
    "seemed like hours, you arrive at your new place ",
    "of employment.  It's  a  grim looking place and ",
    "not  a terrific job  at best,  but it's the most ",
    "that you could hope for right now. With that in ",
    "mind you pull up and decide to give it your all...",
    "",
    "",
    "",
    // sacrifice scene
    // 9
    "    A pesar  de todo, no lo  has  hecho tan mal.",
    "    Has  conseguido todo  lo que te  pidieron, y",
    "    van a poder celebrar su fiesta de Halloween.",
    "    Es  todo un  detalle que t\xA3 seas el invitado",
    "    de honor...",
    // 14
    "    Como recompensa a tus esfuerzos te mandan a",
    "    darte una  vuelta  por ah\xA1 mientras ellos se",
    "    encargan de  los  preparativos de la fiesta.",
    "    T\xA3, obviamente, aceptas  de  inmediato  y te",
    "    vas a tomar el fresco un rato...",
    //19
    "                  Horas m\xA0s tarde... ",
    "    La  fiesta  ya  est\xA0  preparada, regresas al",
    "    sal\xA2n a  recibir tus merecidos honores, tras",
    "    un completo dia de trabajo... ",
    // 23
    "      Despu\x82s de todo lo que hice por ellos...",
    "      Todos mis esfuerzos y desvelos para esto...",
    "      Morir envenenado, ese era mi destino...",
    // 26
    "   Presta atenci\xA2n, pues esta puede  ser la \xA3ltima",
    "   vez que hablemos contigo. Definitivamente est\xA0s",
    "   muerto, pero a\xA3n es  posible reunir tu alma con",
    "   tu cuerpo,  para  ello  debes  buscar a Jacob y",
    "   liberarlo de aquello  que lo retenga. Cuando lo",
    "   consigas podremos traeros  a ambos de vuelta al",
    "   mundo de los vivos.",

    // 33
    "  However  you  must hurry,  for  the link  between ",
    "  your body and soul  will deminish  and  your soul ",
    "  will be trapped in the Eternal Caverns... FOREVER!",
		//FIXME! dont show these in the english version!
    "   Si necesitaras  m\xA0s  energ\xA1""a de la que tienes, ",
    "   recuerda que all\xA0 donde vas un cuerpo contiene",
    "   m\xA0s energ\xA1""a que un alma al contrario que en el",
    "   mundo f\xA1sico.",
    // 40
    "                     Remember.",
    "        If you succed, the prize will be life!",
    "           If you fail... ETERNAL DEATH!!!",
    // 43
    "                   You did it!",
    "               I""M FREEEEEEEEEEE!!!",
    // WC
    // 45
    "... MMPFFFF!! ...",
    "... PPTRRFF!! ...",
    "... GGNNNNN!! ...",
    "... AAAHHHHH!! ...",
    // Diploma
    // 49
    "CERTIFICADO N\xA7:",
    "Yo, Jacob, el Gran Maestre ",
    "de la Hermandad de Sek Umh Nejl",
    "certifico que: ",
    "me ha liberado de las Cavernas",
    "Eternas, por lo cual le estar\x82 ",
    "por siempre agradecido."
	},
};


static const char *const actionLine_ES[] = {
    "IR A ",
    "HABLAR CON ",
    "COGER ",
    "MIRAR ",
    "USAR ",
    "ABRIR ",
    "CERRAR ",
    " CON ",
};

static const char *const actionLine_EN[] = {
    "GO TO ",
    "TALK TO ",
    "TAKE ",
    "LOOK ",
    "USE ",
    "OPEN ",
    "CLOSE ",
    " WITH ",
};


static const char *const hardcodedObjects_ES[] = {
    "LISTA ", // 0
    "LISTA", // 1
    "CARBON", // 2
    "BOMBILLA", // 3
    "HORNACINA", // 4
    "ESTATUA DIVINA", // 5
    "MANUAL DE ALFARERO", // 6
    "ESTATUA GROTESCA",// 7
    "PARED", // 8
    "TORNO", // 9
    "VACIO"
};

static const char *const hardcodedObjects_EN[] = {
    "LIST ",
    "LIST",
    "COAL",
    "LIGHT BULB",
    "NICHE",
    "DIVINE STATUE",
    "POTTER""S HANDBOOK",
    "GROTESQUE STATUE",
    "WALL",
    "POTTER""S WHEEL",
    "FREE"
};

enum HOTKEYS {
    TALK = 0,
    PICKUP = 1,
    LOOKAT = 2,
    USE = 3,
    OPEN = 4,
    CLOSE = 5,
    YES = 6,
    NO = 7
};

static const Common::KeyCode hotkeys[2][8]{
	// Spanish
	{// TALK
	 Common::KEYCODE_h,
	 // PICKUP
	 Common::KEYCODE_c,
	 // LOOKAT
	 Common::KEYCODE_m,
	 // USE
	 Common::KEYCODE_u,
	 // OPEN
	 Common::KEYCODE_a,
	 // CLOSE
	 Common::KEYCODE_e,
	 // YES
	 Common::KEYCODE_s,
	 // NO
	 Common::KEYCODE_n},

	// English
	{// TALK
	 Common::KEYCODE_t,
	 // PICKUP
	 Common::KEYCODE_a,
	 // LOOKAT
	 Common::KEYCODE_l,
	 // USE
	 Common::KEYCODE_u,
	 // OPEN
	 Common::KEYCODE_o,
	 // CLOSE
	 Common::KEYCODE_c,
	 // YES
	 Common::KEYCODE_y,
	 // NO
	 Common::KEYCODE_n}
	};

} // End of namespace Tot
#endif
