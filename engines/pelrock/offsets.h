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
#ifndef PELROCK_OFFSETS_H
#define PELROCK_OFFSETS_H

#include "common/scummsys.h"

namespace Pelrock {

static const uint32_t cursor_offsets[5] = {
	0x0FDDFD,
	0x0FDCDD,
	0x0FDF1D,
	0x0FE33D,
	0x367EF0};

static const uint32_t kBalloonFramesOffset = 2176936;
static const uint32_t kBalloonFramesSize = 24950;

static const uint32_t ALFRED7_ALFRED_COMB_R = 67768;
static const uint32_t ALFRED7_ALFRED_COMB_L = 88408;

static const uint32_t kAlternateSettingsMenuOffset = 910097;
static const uint32_t kAlternateSettingsPaletteOffset = 1038141; // 640 * 480
static const uint32_t kSettingsPaletteOffset = 0x2884c2;         // 640 * 480

#define DESCRIPTION_BASE_OFFSET 0x4715D
#define NUM_DESCRIPTIONS 113

static const uint32 kInventoryDescriptionsOffset = 0x4715D;
static const uint32 kInventoryDescriptionsSize = 7868;
static const uint32 kMenuTextOffset = 0x49203;
static const uint32 kMenuTextSize = 230;
static const uint32 kAlfredResponsesOffset = 0x441DC;
static const uint32 kConversationTerminatorOffset = 0x0492EE;
static const uint32 kAlfredResponsesSize = 12163;
static const uint32 kCreditsOffset = 0x49F60;
static const uint32 kCreditsSize = 2540;

const uint32_t pegatina_offsets[137] = {
	0x000000, 0x00005B, 0x0000B6, 0x000298, 0x00047A, 0x0023C8, 0x004316, 0x004376,
	0x005119, 0x005EBC, 0x0083ED, 0x008529, 0x0092C4, 0x00A3AA, 0x00B490, 0x00B6A6,
	0x00C05A, 0x00CA0E, 0x00D3D0, 0x00D46E, 0x00F036, 0x00FB8F, 0x00FC55, 0x0119D7,
	0x013759, 0x01391F, 0x014A9D, 0x015C1B, 0x017601, 0x018FE7, 0x019048, 0x0190A9,
	0x01910A, 0x0197F4, 0x019EDE, 0x01A7EC, 0x01B0FA, 0x01B8C4, 0x01C644, 0x01D83A,
	0x01E104, 0x01E8C6, 0x01F45D, 0x01FBBB, 0x02011D, 0x02052F, 0x020A95, 0x020E5B,
	0x0210B3, 0x0216E6, 0x021D5E, 0x0233A3, 0x0249E8, 0x025777, 0x026506, 0x028E2B,
	0x02B82F, 0x02C9D7, 0x02E4CA, 0x02FFBD, 0x03234A, 0x0346D7, 0x036A83, 0x038E2F,
	0x03B18D, 0x03D4EB, 0x03DEC9, 0x03F813, 0x04115D, 0x045303, 0x0494A9, 0x04955F,
	0x049615, 0x0496CB, 0x0499E1, 0x049EC7, 0x04A023, 0x04A447, 0x04BA6D, 0x04BFA1,
	0x04CE33, 0x04CF09, 0x04DB3B, 0x052885, 0x0575CF, 0x05775B, 0x057D79, 0x058397,
	0x058969, 0x058F50, 0x05A9DB, 0x05C561, 0x05C72E, 0x05C8FB, 0x05EAC1, 0x060C87,
	0x060D19, 0x060E62, 0x061039, 0x0613C2, 0x061764, 0x061847, 0x062535, 0x062D4B,
	0x064F11, 0x0670D7, 0x067381, 0x0675A9, 0x0677EF, 0x067A98, 0x067DDE, 0x068115,
	0x0684E3, 0x068A76, 0x068F30, 0x0693C8, 0x0696AD, 0x06C2C9, 0x06C84D, 0x07095D,
	0x071854, 0x07274B, 0x073642, 0x074539, 0x075454, 0x0791DA, 0x07CF60, 0x07E4AB,
	0x07ECED, 0x07F52F, 0x07FD71, 0x080591, 0x080B24, 0x080B84, 0x080F39, 0x0812F5,
	0x0816B1};

const byte pegatina_rooms[140] = {
	0, 0, 0, 0, 0, 0, 0,                            // Sprites 0-6: Room 0
	2, 2,                                           // Sprites 7-8: Room 2
	3, 3, 3, 3, 3, 3, 3, 3,                         // Sprites 9-16: Room 3
	4, 4, 4, 4, 4,                                  // Sprites 17-21: Room 4
	5, 5,                                           // Sprites 22-23: Room 5
	7,                                              // Sprite 24: Room 7
	8, 8,                                           // Sprites 25-26: Room 8
	9, 9, 9, 9, 9,                                  // Sprites 27-31: Room 9
	12, 12,                                         // Sprites 32-33: Room 12
	13, 13, 13,                                     // Sprites 34-36: Room 13
	12,                                             // Sprite 37: Room 12
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, // Sprites 38-49: Room 15
	16, 16,                                         // Sprites 50-51: Room 16
	17, 17,                                         // Sprites 52-53: Room 17
	19, 19, 19, 19, 19,                             // Sprites 54-58: Room 19

	0, 0, 0, 0, 0, 0, 0, // Sprites 59-65: Room 0
	33, 33,              // Sprites 66-67: Room 33
	29, 29,              // Sprites 68-69: Room 29
	0, 0, 0,             // Sprites 70-72: Room 0

	34, 35, 31, 25,         // Sprites 73-76: Various rooms
	31,                     // Sprite 77: Room 31
	32,                     // Sprite 78: Room 32
	21, 25,                 // Sprites 79-80: Rooms 21, 25
	0,                      // Sprite 81: Room 0
	0, 0, 0, 0, 0,          // Sprites 82-86: Room 0
	4, 4, 4, 4,             // Sprites 87-90: Room 4
	0, 0, 0, 0,             // Sprites 91-94: Room 0
	0, 0, 0, 0, 0, 0,       // Sprites 95-100: Room 0
	33, 33,                 // Sprites 101-102: Room 33
	47, 47,                 // Sprites 103-104: Room 47
	52, 52, 52, 52, 52,     // Sprites 105-109: Room 52
	52, 52, 52, 52, 52, 52, // Sprites 110-115: Room 52
	41,                     // Sprite 116: Room 41
	0,                      // Sprite 117: Room 0
	30,                     // Sprite 118: Room 30
	44, 44, 44, 44,         // Sprites 119-122: Room 44
	31,                     // Sprite 123: Room 31
	46, 46,                 // Sprites 124-125: Room 46
	31,                     // Sprite 126: Room 31
	51, 52, 53, 54          // Sprites 127-130: Various rooms
};

enum TextIndices {
	ESTAN_CERRADOS,
	HOY_NO_DISPONIBLES,
	YA_ABIERTO_M,
	YA_CERRADO_M,
	YA_ABIERTA_F,
	YA_CERRADA_F,
	HELADERIA_CERRADA,
	POBRE_PERO_NO_HE_LLEGADO_A_ESO,
	QUEBUENA_ESTA,
	BOTONVERDEPARASACAR_BOTONVERDEPARACANCELAR,
	PRIMEROMETA_TARJETA,
	NOTENGOMASDINERO,
	MEHEVUELTOAPINCHAR,
	QUEHASIDOESO,
	QUIENANDAAHI,
	YOMEVOY,
	TIENDA_CERRADA,
	NOSE_ENTERARIA,
	PRIMERO_ABRIRLO,
	QUELOSCOJA_SUPADRE,
	PRETENDEUSTED_SOBORNARME,
	MUYBIEN_1,
	CUANDOMELOPIDA,
	DEACUERDO,
	NECESITODNI,
	QUE_RECIBO_ACAMBIO,
	ESPOCO,
	ALTO,
	NIPARAEMPEZAR,
	PARAQUE,
	DEPIEDRANO_DEHIELO,
	NO_EMPECEMOS,
	CUERPO_DANONE,
	CABEZA_HUECA,
	ESO_LO_SERAS_TU,
	DEMASIADO_NO_PUEDO_PENSAR,
	UN_POCO_RESPETO,
	NO_THEY_MAKEYOU_FAT,
	RELOJ_HA_CAMBIADO,
	CORRESPONDENCIA_AJENA,
	ANDA,
	TUCREES,
	NOESAMIAQUIENDEBES,
	AQUIENENTONCES,
	LIBROSSECRETOS,
	VENGA_ACA,
	TODOS,
	EL_LIBRO_NOESTA_AQUI,
	TENDRE_DEJAR_LIBRO,
	TRABAJARIA_MEJOR_SI_NO_ME_MOLESTARA,
	REGALO_LIBRO_RECETAS,
	YSI_METIRA_MAQUINA,
	QUITA_ESAS_MANOS,
	QUEASCO,
	QUESESTO_RECETA,
	YAESTA_ABIERTO,
	VAESTAR_POCOFUERTE,
	CUENTOPARECIDO,
	COSASAPRENDIDO,
	PERIODICOSENSACIONALISTA,
	HOJAENTREPAGINAS,
	NOENTIENDONADA,
	NOTENGODINERO,
	CUESTA1000,
	AQUITIENE,
	MUYBIEN,
	YASEEGIPCIO,
	QUELASTIMA_NOSEEGIPCIO,
	FORMULAVIAJETIEMPO,
	PARECECERRADO,
	NOVIO2METROS,
	GRANIDEA,
	SELORECOMIENDO,
	OIGAUSTED,
	ESAMI,
	VENGAAHORAMISMO,
	CUIDADOIMPRUDENTE,
	QUEOSCUROESTAESTO,
	MENUDAAVENTURA,
	NECESITOGASOLINA,
	YANOSEHACEONCOMOANTES,
	NADIELOHAVISTO,
	AYAYAY,
	OIGAUSTED2,
	LEESTOYVIGILANDO,
	OIGA,
	CAPITULOPARADOJAS,
	HAYQUECELEBRARLO,
	PESADEMASIADO,
	NINGUNATEMAAPROPIADO,
	PARAQUECOGERBARRO,
	BUENOCOGEREUNPOCO,
	ABSOLUTAMENTECERRADO,
	NOSETEOCURRAACERCARTE,
	PUERTAAUTENTICA_IZQUIERDA,
	OHMISALVADOR,
	VOYPORTI_PRINCESA,
	AMISBRAZOS,
	DIOSMIOQUEESESTO,
	QUEPASA,
	OLVIDECERRARTRAMPILLA,
	NOTEPREOCUPES_VOLVERE,
	ALACONUSTED,
	MEMEO,
	POR5MINUTOS,
	TALUEGOLUCAS,
	SISUPIERA_COMBINACION,
	PARECE_COMBINACION_CAJAFUERTE,
	GRANCANTIDAD_DINERO,
	TEAPETECE_BUENRATO,
	YLOSCONDONES,
	QUEASCO_CASIMEMEA,
	HECHOELPRIMO,
	MEHANTOMADO_EL_PELO,
	PESADO_UNRATO,
	TRAIDOR,
	TUTIA,
	LATUYA,
	GORDO,
	FIDEO,
	LIMPIACULO,
	CONTUTURBANTE,
	OSO,
	COMADREJA,
	CABEZON,
	TUABUELO,
	TUMUJER,
	PERDEDOR,
	SOYMEJORQUETU,
	TRAMPOSO,
	MALPERDEDOR,
	PARAUNAVEZ,
	MEJORMELARGO,
	NOTENGOPARCHES,
	NOTENGOPEGAMENTO,
	MUNECO_ARREGLADO,
	MAREDEDEU,
	PROBARLIBRO,
	PRACTICAR_MAS,
	AQUI_NO_NECESITO,
	DIOSHALCON,
	OHGRANOSIRIS,
	HEMEAQUI,
	OHSOBEK,
	OHTOTH,
	TODOSLASCOSAS,
	HELLEGADOPURO,
	DIOSDELATURBULENCIA,
	OHANUBIS,
	HEVENIDO,
	HELLEGADOATI,
	OHPTHA,
	LASPUERTASDELCIELO,
	VAYASUENHO,
	PARAQUE_2,
	YESO,
	UNPOCODESESPERADO,
	COMBINACIONESMEJORES,
	NOSEQUEPRETENDES_CONESO,
	COMO,
	MUCHOSENTIDO,
	PORPROBAR,
	NOLOENTIENDO,
	PARAESONOSIRVE,
	PRUEBAOTRACOSA,
	SIHOMBREQUEMAS,
	NOSEQUEPRETENDES,
	COSASRARAS,
	ARTE_O_LOCURA,
	UTILIDADES,
	TITULOJUEGO,
	MENSAJEOTRAEPOCA,
	NOERAAUTENTICO,
	PRIMERINGREDIENTE,
	DOSINGREDIENTES,
	TRESINGREDIENTES,
	CUATROINGREDIENTES,
	LIBRO_ABURRIDO,
	DEACUERDO_2,
	GAMBERROS,
	QUIENYO,
	PINTA_BUENAPERSONA,
	DEMO_FINAL,
	DIOSHALCON_2,
	GRANOSIRIS,
	HEMEAQUI_ISIS,
	OHSOBEK_2,
	OHTOTH_2,
	PROTEGEN_MI_CUERPO,
	HELLEGADO_PURO,
	DIOSDELATURBULENCIA_2,
	OHANUBIS_2,
	HEVENIDO_2,
	HELLEGADO_ATI,
	OHPTHA_2,
	LASPUERTAS_DELCIELO,
};

// Description offsets relative to DESCRIPTION_BASE_OFFSET
const uint16_t description_offsets[NUM_DESCRIPTIONS] = {
	0x0000, // Object 0: Historia de la Princesa Zenna y su amante insatisfecho
	0x0058, // Object 1: Nombre: Alfred Pelrock
	0x00C4, // Object 2: La tipica tarjeta por la que te sacan commisiones
	0x010E, // Object 3: Una pequeña foto de Alfred
	0x012C, // Object 4: Un ladrillo
	0x013B, // Object 5: 1000 pesetas
	0x014B, // Object 6: Una alargadera con un extremo suelto
	0x0173, // Object 7: Un amuleto egipcio con forma de escarabajo alado
	0x01A7, // Object 8: Dice: OM OM RASKAMAMOM
	0x01C1, // Object 9: Es una carta de la Asociacion Ra-Amoniana
	0x020A, // Object 10: Un carnet de biblioteca
	0x0247, // Object 11: Titulo: Canticos espirituales en formato *.zip
	0x02B3, // Object 12: Titulo: Pasion Flagrante
	0x0327, // Object 13: Titulo: El Valenciano en los comienzos del siglo XXI
	0x039E, // Object 14: Titulo: El sistema inmunologico de los cefalopodos (v.I)
	0x0412, // Object 15: Titulo: Dos y dos son 5
	0x0493, // Object 16: Titulo: La parte creativa
	0x057B, // Object 17: Titulo: 10 maneras de preparar fideos chinos
	0x065C, // Object 18: Titulo: Los Peces Gato del Rio Tajo
	0x06D8, // Object 19: Titulo: Gato por liebre
	0x0753, // Object 20: Titulo: Hiper-cocina para solteros
	0x07CE, // Object 21: Titulo: El camaleon humano
	0x084E, // Object 22: Titulo: Psiquiatria Avanzada (vol. 8)
	0x08CA, // Object 23: Titulo: Sistemas de alcantarillado en el siglo XV
	0x0949, // Object 24: Titulo: Cartas de amor de Pol Pot a su novia
	0x09CC, // Object 25: Titulo: El gran libro de las preposiciones
	0x0A50, // Object 26: Titulo: Corazon, vida y muerte de un tenista
	0x0ACC, // Object 27: Titulo: Analisis de la vida de los funcionarios
	0x0B4D, // Object 28: Titulo: Ensayos sobre la putrefaccion
	0x0BC9, // Object 29: Titulo: Cocinar bien es imposible
	0x0C49, // Object 30: Titulo: 1000 formas de hacer sonar un claxon
	0x0CC8, // Object 31: Titulo: El arte de la peluqueria
	0x0D3B, // Object 32: Titulo: Analisis de las tramas de las mejores telecomedias
	0x0DC7, // Object 33: Titulo: Tratado de las empanadillas
	0x0E40, // Object 34: Titulo: Misterios de los numeros
	0x0EBA, // Object 35: Titulo: Como vender mas
	0x0F31, // Object 36: Titulo: Todos podemos estar de moda
	0x0FAD, // Object 37: Titulo: La economia capitalista (Tomo VI)
	0x102E, // Object 38: Titulo: Aventuras con mis hemorrides
	0x10AB, // Object 39: Titulo: Automate. Tomo IV: Suicidio
	0x1128, // Object 40: Titulo: El cienpies azul
	0x11A1, // Object 41: Titulo: Guia sexual de la mosca
	0x121E, // Object 42: Titulo: La Oveja. El gran misterio
	0x1297, // Object 43: Titulo: Mi libro de cocina
	0x1309, // Object 44: Titulo: Ariel
	0x1377, // Object 45: Titulo: Matar cucarachas con la mirada
	0x13F4, // Object 46: Titulo: Telepatia: Caso practico
	0x1476, // Object 47: Titulo: Vida y obra de Paquirrin
	0x14F4, // Object 48: Titulo: Odas para aliviar el estrenimiento
	0x1577, // Object 49: Titulo: Mi vida en el gran mercado
	0x15F4, // Object 50: Titulo: Oda al tocino
	0x1669, // Object 51: Titulo: Como escribir una novela
	0x16E0, // Object 52: Titulo: Recogiendo oro en las cloacas
	0x175B, // Object 53: Titulo: Como comer bien. Tomo XXI. Entrantes
	0x17DD, // Object 54: Titulo: No tengo nada mejor que hacer
	0x185D, // Object 55: Titulo: Los Heraldos Negros
	0x18D1, // Object 56: Titulo: La Piedra Rosetta
	0x194A, // Object 57: Titulo: Fabulas de Ciencia Ficcion
	0x19C3, // Object 58: Titulo: Elogio de la pereza
	0x1A35, // Object 59: Un pequeño altavoz
	0x1A52, // Object 60: Un altavoz mediano
	0x1A70, // Object 61: Un altavoz grande
	0x1A8C, // Object 62: Un extintor
	0x1AA3, // Object 63: Parece un extintor, pero es un termo de cafe. De los de antes, de los grandes
	0x1B2D, // Object 64: Un termo con cafe
	0x1B53, // Object 65: Una cafetera expres para una taza
	0x1B8D, // Object 66: La tarjeta de acceso de Erika
	0x1BBE, // Object 67: Billetes de avion para salir por patas hacia Valencia
	0x1C26, // Object 68: Una bolsa de patatas fritas
	0x1C52, // Object 69: Llave de la habitacion de Lucy
	0x1C82, // Object 70: Llave de la habitacion de Erika
	0x1CB3, // Object 71: Un ordenador de sobremesa
	0x1CE1, // Object 72: Un cuadro
	0x1CF9, // Object 73: Una moto mega-retuneada
	0x1D24, // Object 74: Una lamparita
	0x1D3F, // Object 75: Un libro gordo
	0x1D5C, // Object 76: Un libro finito
	0x1D7A, // Object 77: Un poco de escayola vieja
	0x1DA3, // Object 78: Un cacho de ladrillo
	0x1DCC, // Object 79: Un boligrafo
	0x1DE5, // Object 80: Un radiocasete
	0x1E00, // Object 81: Es una pistola. (Real como la vida misma)
	0x1E49, // Object 82: Una pieza de fruta
	0x1E6B, // Object 83: Un frasco de pastillas para dormir
	0x1EA0, // Object 84: Una pulsera
	0x1EB9, // Object 85: Una estatua pequeña
	0x1EE0, // Object 86: Una jodida (disculpen las molestias) cinta de video
	0x1F4F, // Object 87: Una jodida (disculpen las molestias) cadena hifi
	0x1FBE, // Object 88: Una magdalena
	0x1FD8, // Object 89: Un poco de cecina
	0x1FFC, // Object 90: Un televisor portatil con forma de Mickey Mouse
	0x2053, // Object 91: Un destornillador
	0x2071, // Object 92: Alicates de electricista
	0x2097, // Object 93: Un cable
	0x20AE, // Object 94: Una linterna
	0x20C6, // Object 95: Unas pilas gigantes
	0x20E7, // Object 96: La bolsa de basura negra
	0x2116, // Object 97: Foto de un tal Gerardo (desconocido)
	0x2158, // Object 98: Una cinta de casete
	0x2179, // Object 99: Un walkman
	0x218F, // Object 100: Un papel con un telefono
	0x21BE, // Object 101: Una llave grande de metacrilato
	0x21F1, // Object 102: Una pequeña llave
	0x220E, // Object 103: Autenticas naranjas de Nules
	0x2236, // Object 104: No se haga el loco: Llameme !!!
	0x2285, // Object 105: Folletos explicativos sobre el SIDA
	0x22BE, // Object 106: Un pin que acredita mi sabiduria
	0x22E2, // Object 107: Una bayeta para frotar lamparas magicas
	0x2316, // Object 108: Parches ultra-fuertes
	0x2337, // Object 109: Pegamento que te cagas
	0x235A, // Object 110: Una replica de Alfred pinchada
	0x2383, // Object 111: Una cinta del Rey Elvis
	0x23A4, // Object 112: Una caja de condone
};

struct ExtraImages {
	uint32 offset;
	uint32 paletteOffset;
	byte numChunks;
};

const ExtraImages extraScreens[] = {
	{0x00, // Portrait above bed
	 0x7984,
	 8},
	{0x1A9EE, // Computer screen
	 0x305A2,
	 8},
	{0x647C3, // Alfred circle
	 0x7B6B1,
	 4},
	{0x6FBCD, // Recipe
	 0x7B6B1,
	 8},
	{0x7BA11, // Newspaper
	 0x88745,
	 8},
	{0x9237B, // tablet
	 0xB0EE7,
	 8},
	{0xB11ED, // map
	 0xDE011,
	 8},
	{0xFFC47, // girl book
	 0x1180C9,
	 8},
	{0x1183C5, // book
	 0x1358F3,
	 8},
	{0x152A88, // portrait
	 0x15BFC8,
	 8},
};

struct AlfredSpecialAnimOffset {
	int numFrames = 0;
	int w = 0;
	int h = 0;
	int numBudas;
	int loops;
	uint32 offset;
	int stride = 0;

	AlfredSpecialAnimOffset(int nF, int width, int height, int nBudas, uint32 off, int loops)
		: numFrames(nF), w(width), h(height), numBudas(nBudas), offset(off), loops(loops) {
		stride = w * h;
	}
	AlfredSpecialAnimOffset() {
	}
};

static const AlfredSpecialAnimOffset alfredSpecialAnims[] = {
	{10, 51, 102, 1, 559685, 1}, // READ BOOK
	{10, 51, 102, 1, 578943, 1}, // READ RECIPE
	{3, 45, 87, 0, 37000, 1}, // ELECTRIC SHOCK 1
	{2, 82, 58, 0, 53106, 20}, // ELECTRIC SHOCK 3
};

} // End of namespace Pelrock
#endif
