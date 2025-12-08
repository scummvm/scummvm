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

static const uint32_t kAlternateSettingsMenuOffset = 910097; // Placeholder offset
static const uint32_t kSettingsPaletteOffset = 1038141;      // 640 * 480

#define DESCRIPTION_BASE_OFFSET 0x4715D
#define NUM_DESCRIPTIONS 113

static const uint32 kInventoryDescriptionsOffset = 0x4715D;
static const uint32 kInventoryDescriptionsSize = 7868;


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

} // End of namespace Pelrock
#endif
