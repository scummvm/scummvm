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

#include "hollywood/gameplay/inventory_actions.h"

#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

void GameplayLoopDelegate::beginSharedInventorySpeechLine(uint16 rowIndex, byte frameIndex) {
	(void)rowIndex;
	(void)frameIndex;
}

byte GameplayLoopDelegate::randomSharedInventorySpeechFrame(byte maxFrameIndex) {
	(void)maxFrameIndex;
	return 0;
}

void GameplayLoopDelegate::playSharedInventorySound(byte sampleId) {
	(void)sampleId;
}

void GameplayLoopDelegate::showTravelScreenViewer() {
}

bool GameplayLoopDelegate::showInventoryMedia(InventoryMediaId mediaId) {
	(void)mediaId;
	return false;
}

bool GameplayLoopDelegate::playSueTapeRecording() {
	return false;
}

void beginRonSpeechLine(GameplayLoopDelegate *delegate, uint16 rowIndex, byte frameIndex) {
	delegate->beginSharedInventorySpeechLine(rowIndex, frameIndex);
}

void beginRonRandomSpeechLine(GameplayLoopDelegate *delegate, uint16 rowIndex, byte maxFrameIndex) {
	beginRonSpeechLine(delegate, rowIndex, delegate->randomSharedInventorySpeechFrame(maxFrameIndex));
}

void playRonInventoryChange(GameplayLoopDelegate *delegate) {
	delegate->playSharedInventorySound(1);
}

void setInventoryItemResourcePage(GameplayState &gameState, byte owner, byte itemId, byte pageIndex) {
	if (owner >= GameplayState::kInventoryOwnerCount || itemId >= GameplayState::kInventoryOwnerSlotStride)
		return;

	gameState.inventoryItemResourcePageByOwnerAndItemId[owner][itemId] = pageIndex;
	gameState.inventoryPanelDirty = true;
}

void advanceRonKnifePage(GameplayState &gameState, GameplayLoopDelegate *delegate, byte owner) {
	if ((gameState.multiToolKnifeState & 1) != 0) {
		if (gameState.multiToolKnifeState < 9)
			++gameState.multiToolKnifeState;
		else
			gameState.multiToolKnifeState = 0;
	}

	byte pageIndex = 0;
	switch (gameState.multiToolKnifeState) {
	case 0:
		gameState.multiToolKnifeState = 1;
		pageIndex = 0x1e;
		break;
	case 2:
		gameState.multiToolKnifeState = 3;
		pageIndex = 0x2d;
		break;
	case 4:
		gameState.multiToolKnifeState = 5;
		pageIndex = 0x7d;
		break;
	case 6:
		gameState.multiToolKnifeState = 7;
		pageIndex = 0x0e;
		break;
	case 8:
		gameState.multiToolKnifeState = 9;
		pageIndex = 0x18;
		break;
	default:
		break;
	}

	if (pageIndex != 0)
		setInventoryItemResourcePage(gameState, owner, 0x2d, pageIndex);
	playRonInventoryChange(delegate);
	beginRonSpeechLine(delegate, 0x35, 0);
}

void closeRonKnifePage(GameplayState &gameState, GameplayLoopDelegate *delegate, byte owner) {
	if ((gameState.multiToolKnifeState & 1) == 0) {
		beginRonSpeechLine(delegate, 0x0b, 0);
		return;
	}

	if (gameState.multiToolKnifeState == 9)
		gameState.multiToolKnifeState = 0;
	else
		++gameState.multiToolKnifeState;

	setInventoryItemResourcePage(gameState, owner, 0x2d, 0x68);
	playRonInventoryChange(delegate);
	beginRonSpeechLine(delegate, 0x35, 0);
}

void grantRonItem(GameplayState &gameState, GameplayLoopDelegate *delegate, byte owner, byte itemId) {
	gameState.addInventoryItem(owner, itemId);
	playRonInventoryChange(delegate);
}

bool beginRonSimpleInventorySpeech(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		uint16 handlerId) {
	const GameplayState &gameState = vm->gameState();

	switch (handlerId) {
	case 2:   // Usar item con item: generic "No funcionaria".
		beginRonRandomSpeechLine(delegate, 0x01, 2);
		return true;
	case 20:  // Abrir closed/unsafe item: generic bad idea.
	case 56:  // Usar iman with microphone: generic bad idea.
		beginRonRandomSpeechLine(delegate, 0x13, 1);
		return true;
	case 30:  // Usar algodon as cleaning rag: Ron refuses to clean.
		beginRonRandomSpeechLine(delegate, 0x1d, 1);
		return true;
	case 32:  // Usar bisturi as tool: "I am a journalist, not a surgeon."
		beginRonRandomSpeechLine(delegate, 0x1f, 1);
		return true;
	case 34:  // Usar control remoto violently: Ron avoids violence.
		beginRonRandomSpeechLine(delegate, 0x21, 1);
		return true;
	case 40:  // Usar cuerda on people: Ron refuses to tie people up.
		beginRonRandomSpeechLine(delegate, 0x27, 1);
		return true;
	case 44:  // Usar lupa where not needed.
		beginRonRandomSpeechLine(delegate, 0x2b, 2);
		return true;
	case 52:  // Usar estaca with serrucho: too small to sharpen.
		beginRonSpeechLine(delegate, 0x33, 0);
		return true;
	case 53:  // Usar serrucho with estaca: Ron would cut it badly.
		beginRonSpeechLine(delegate, 0x34, 0);
		return true;
	case 58:  // Usar flor del Nilo with ponchera/glass: keeps well without water.
		beginRonSpeechLine(delegate, 0x36, 0);
		return true;
	case 61:  // Usar varita zahori with diamond: it reacts.
		beginRonSpeechLine(delegate, 0x39, 0);
		return true;
	case 64:  // Usar paja with candil: wrong fuel.
		beginRonSpeechLine(delegate, 0x3c, 0);
		return true;
	case 66:  // Usar folleto with hair treatment: needs glue.
		beginRonSpeechLine(delegate, 0x3e, 0);
		return true;
	case 67:  // Usar pintura with picture: Ron could repaint it, but time matters.
		beginRonSpeechLine(delegate, 0x3f, 0);
		return true;
	case 70:  // Dar any item to hombre menguante: he is too drunk/asleep.
		beginRonRandomSpeechLine(delegate, 0x42, 2);
		return true;
	case 71:  // Dar any item to rata: it is still stunned.
		beginRonRandomSpeechLine(delegate, 0x43, 1);
		return true;
	case 72:  // Dar pildoras del Dr. Mabuse to hombre menguante.
		beginRonSpeechLine(delegate, 0x44, 0);
		return true;
	case 73:  // Dar tratamiento capilar to hombre menguante.
		beginRonSpeechLine(delegate, 0x45, 0);
		return true;
	case 74:  // Usar agenda: nothing interesting to note right now.
		beginRonSpeechLine(delegate, 0x46, 0);
		return true;
	case 76:  // Dar agenda: Ron will not give away his work tool.
		beginRonSpeechLine(delegate, 0x47, 0);
		return true;
	case 77:  // Mirar algodon (look cotton): soft.
		beginRonSpeechLine(delegate, 0x48, 0);
		return true;
	case 78:  // Mirar bisturi (look scalpel): Jack's use is unpleasant.
		beginRonSpeechLine(delegate, 0x49, 0);
		return true;
	case 79:  // Mirar carta (look letter): handwriting is terrible.
		beginRonSpeechLine(delegate, 0x4a, 0);
		return true;
	case 80:  // Mirar control remoto (look remote): drives Dr. Fly's device.
		beginRonSpeechLine(delegate, 0x4b, 0);
		return true;
	case 81:  // Usar control remoto (use remote): experiment is not ready yet.
		beginRonSpeechLine(delegate, 0x4c, 2);
		return true;
	case 82:  // Mirar estaca (look stake): sharp enough to be driven in.
		beginRonSpeechLine(delegate, 0x4d, 0);
		return true;
	case 83:  // Mirar iman (look magnet): small but powerful.
		beginRonSpeechLine(delegate, 0x4e, 0);
		return true;
	case 84:  // Mirar jeringuilla (look syringe): empty.
		beginRonSpeechLine(delegate, 0x4f, 0);
		return true;
	case 88:  // Mirar artilugio del Dr. Mosca (look Dr. Fly device).
		beginRonSpeechLine(delegate, 0x52, 0);
		return true;
	case 89:  // Usar artilugio del Dr. Mosca (use Dr. Fly device): wrong place.
		beginRonSpeechLine(delegate, 0x53, 0);
		return true;
	case 90:  // Mirar pastillero bomba / pastillero bomba con iman.
		beginRonSpeechLine(delegate, 0x54, 0);
		return true;
	case 91:  // Mirar pastillero con iman.
		beginRonSpeechLine(delegate, 0x55, 0);
		return true;
	case 93:  // Mirar pildoras del Dr. Mabuse.
		beginRonSpeechLine(delegate, 0x56, 0);
		return true;
	case 95:  // Mirar pastillero.
		beginRonSpeechLine(delegate, 0x59, 0);
		return true;
	case 97:  // Mirar engrasador para largas distancias.
		beginRonSpeechLine(delegate, 0x5a, 0);
		return true;
	case 98:  // Mirar filete durillo.
		beginRonSpeechLine(delegate, 0x5b, 0);
		return true;
	case 99:  // Mirar papiro.
		beginRonSpeechLine(delegate, 0x5c, 0);
		return true;
	case 100: // Mirar serrucho.
		beginRonSpeechLine(delegate, 0x5d, 0);
		return true;
	case 101: // Mirar sobre.
		beginRonSpeechLine(delegate, 0x5e, 0);
		return true;
	case 103: // Mirar tratamiento adelgazante.
		beginRonSpeechLine(delegate, 0x5f, 0);
		return true;
	case 104: // Usar tratamiento adelgazante / capilar: Ron does not need it.
		beginRonSpeechLine(delegate, 0x60, 0);
		return true;
	case 105: // Mirar cinta de Sue.
		beginRonSpeechLine(delegate, 0x61, gameState.ronTapeRecorderState == 2 ? 1 : 0);
		return true;
	case 106: // Mirar algodon empapado de grasa.
		beginRonSpeechLine(delegate, 0x62, 0);
		return true;
	case 107: // Mirar increible hombre menguante.
		beginRonSpeechLine(delegate, 0x63, 0);
		return true;
	case 108: // Hablar/usar/dar hombre menguante: he is sleeping.
		beginRonSpeechLine(delegate, 0x64, 0);
		return true;
	case 109: // Mirar maletin de Jack el destripador.
		beginRonSpeechLine(delegate, 0x65, 0);
		return true;
	case 111: // Mirar ponchera.
		beginRonSpeechLine(delegate, 0x66, 0);
		return true;
	case 112: // Mirar cordon.
		beginRonSpeechLine(delegate, 0x67, 0);
		return true;
	case 113: // Mirar globo.
		beginRonSpeechLine(delegate, 0x68, 0);
		return true;
	case 114: // Mirar paquete.
		beginRonSpeechLine(delegate, 0x69, 0);
		return true;
	case 117: // Mirar cadena.
		beginRonSpeechLine(delegate, 0x6a, 0);
		return true;
	case 118: // Mirar babas de mosca.
		beginRonSpeechLine(delegate, 0x6b, 0);
		return true;
	case 119: // Mirar revienta-craneos.
		beginRonSpeechLine(delegate, 0x6c, 0);
		return true;
	case 120: // Mirar nota de Frankenstein.
		beginRonSpeechLine(delegate, 0x6d, 0);
		return true;
	case 121: // Dar nota de Frankenstein: keep the case quiet.
		beginRonSpeechLine(delegate, 0x6e, 0);
		return true;
	case 122: // Mirar ticket.
		beginRonSpeechLine(delegate, 0x6f, 0);
		return true;
	case 123: // Mirar microfono.
		beginRonSpeechLine(delegate, 0x70, 0);
		return true;
	case 125: // Mirar pie de microfono.
		beginRonSpeechLine(delegate, 0x72, 0);
		return true;
	case 126: // Mirar cerebro de Frankenstein.
		beginRonSpeechLine(delegate, 0x73, 0);
		return true;
	case 127: // Usar/dar body part to wrong target.
		beginRonSpeechLine(delegate, 0x74, 0);
		return true;
	case 128: // Mirar colmillo de tigre.
		beginRonSpeechLine(delegate, 0x75, 0);
		return true;
	case 129: // Mirar gafas de sol.
		beginRonSpeechLine(delegate, 0x76, 0);
		return true;
	case 130: // Usar gafas de sol.
		beginRonSpeechLine(delegate, 0x77, 0);
		return true;
	case 131: // Mirar pamela.
		beginRonSpeechLine(delegate, 0x78, 0);
		return true;
	case 132: // Usar pamela.
		beginRonSpeechLine(delegate, 0x79, 0);
		return true;
	case 134: // Usar/dar dinero.
		beginRonSpeechLine(delegate, 0x7b, 0);
		return true;
	case 135: // Mirar pergamino.
		beginRonSpeechLine(delegate, 0x7c, 0);
		return true;
	case 136: // Usar pergamino outside Karnak altar.
		beginRonSpeechLine(delegate, 0x7d, 0);
		return true;
	case 137: // Mirar semillas.
		beginRonSpeechLine(delegate, 0x7e, 0);
		return true;
	case 138: // Mirar flor del Nilo.
		beginRonSpeechLine(delegate, 0x7f, 0);
		return true;
	case 139: // Mirar navaja.
		beginRonSpeechLine(delegate, 0x80, 0);
		return true;
	case 140: // Mirar pelo de Amesis-Huni.
		beginRonSpeechLine(delegate, 0x81, 0);
		return true;
	case 141: // Mirar baston de Ra.
		beginRonSpeechLine(delegate, 0x82, 0);
		return true;
	case 142: // Usar baston de Ra in the wrong place.
		beginRonSpeechLine(delegate, 0x83, 0);
		return true;
	case 143: // Mirar piernas de Frankenstein.
		beginRonSpeechLine(delegate, 0x84, 0);
		return true;
	case 144: // Mirar maza.
		beginRonSpeechLine(delegate, 0x85, 0);
		return true;
	case 145: // Mirar aguja e hilo quirurgicos.
		beginRonSpeechLine(delegate, 0x86, 0);
		return true;
	case 146: // Usar aguja e hilo quirurgicos outside the experiment.
		beginRonSpeechLine(delegate, 0x87, 0);
		return true;
	case 148: // Mirar folleto de adelgazamiento pegajoso.
		beginRonSpeechLine(delegate, 0x88, 0);
		return true;
	case 149: // Mirar palo.
		beginRonSpeechLine(delegate, 0x89, 0);
		return true;
	case 150: // Mirar sal.
		beginRonSpeechLine(delegate, 0x8a, 0);
		return true;
	case 151: // Mirar varita zahori.
		beginRonSpeechLine(delegate, 0x8b, 2);
		return true;
	case 152: // Mirar jeringuilla llena de savia.
		beginRonSpeechLine(delegate, 0x8c, 0);
		return true;
	case 153: // Mirar margarita.
		beginRonSpeechLine(delegate, 0x8d, 0);
		return true;
	case 154: // Mirar camafeo de Samarkanda.
		beginRonSpeechLine(delegate, 0x8e, 0);
		return true;
	case 155: // Abrir camafeo de Samarkanda: do not waste the sun ray.
		beginRonSpeechLine(delegate, 0x8f, 0);
		return true;
	case 156: // Mirar candil.
		beginRonSpeechLine(delegate, 0x90, gameState.ronLampFueled ? 1 : 0);
		return true;
	case 157: // Mirar cuerda.
		beginRonSpeechLine(delegate, 0x91, 0);
		return true;
	case 158: // Mirar hueso.
		beginRonSpeechLine(delegate, 0x92, 0);
		return true;
	case 159: // Mirar camison de Sherilyn.
		beginRonSpeechLine(delegate, 0x93, 0);
		return true;
	case 160: // Usar camison de Sherilyn: wrong size.
		beginRonSpeechLine(delegate, 0x94, 0);
		return true;
	case 161: // Mirar cuadro.
		beginRonSpeechLine(delegate, 0x95, 0);
		return true;
	case 162: // Mirar/abrir perfume: empty.
		beginRonSpeechLine(delegate, 0x50, 1);
		return true;
	case 163: // Mirar tronco y brazos de Frankenstein.
		beginRonSpeechLine(delegate, 0x96, 0);
		return true;
	case 164: // Mirar aceite.
		beginRonSpeechLine(delegate, 0x97, 0);
		return true;
	case 165: // Mirar gominola.
		beginRonSpeechLine(delegate, 0x98, 0);
		return true;
	case 166: // Mirar filete.
		beginRonSpeechLine(delegate, 0x99, 0);
		return true;
	case 167: // Mirar punado de paja.
		beginRonSpeechLine(delegate, 0x9a, 0);
		return true;
	case 168: // Mirar madero.
		beginRonSpeechLine(delegate, 0x9b, 0);
		return true;
	case 169: // Mirar baraja de cartas.
		beginRonSpeechLine(delegate, 0x9c, 0);
		return true;
	case 170: // Usar baraja de cartas.
		beginRonSpeechLine(delegate, 0x9d, 0);
		return true;
	case 171: // Mirar calcetin remendado.
		beginRonSpeechLine(delegate, 0x9e, 0);
		return true;
	case 172: // Usar calcetin remendado.
		beginRonSpeechLine(delegate, 0x9f, 0);
		return true;
	case 173: // Mirar diamante mas grande del mundo.
		beginRonSpeechLine(delegate, 0xa0, 0);
		return true;
	case 174: // Mirar llave.
		beginRonSpeechLine(delegate, 0xa1, 0);
		return true;
	case 175: // Hablar con cabeza de Frankenstein.
		beginRonSpeechLine(delegate, 0xa2, 0);
		return true;
	case 176: // Mirar cabeza de Frankenstein.
		beginRonSpeechLine(delegate, 0xa3, 0);
		return true;
	case 177: // Mirar globo inflado con gas.
		beginRonSpeechLine(delegate, 0xa4, 0);
		return true;
	case 178: // Usar globo inflado / bisturi with Dr. Fly device: generic no use.
		beginRonSpeechLine(delegate, 0xa5, 0);
		return true;
	case 179: // Mirar pedrusco.
		beginRonSpeechLine(delegate, 0xa6, 0);
		return true;
	case 180: // Mirar gorro de aviador.
		beginRonSpeechLine(delegate, 0xa7, 0);
		return true;
	case 181: // Usar gorro de aviador.
		beginRonSpeechLine(delegate, 0xa8, 0);
		return true;
	case 182: // Mirar pala.
		beginRonSpeechLine(delegate, 0xa9, 0);
		return true;
	case 183: // Mirar libro de Karl.
		beginRonSpeechLine(delegate, 0xaa, 0);
		return true;
	case 185: // Usar libro de Karl.
		beginRonSpeechLine(delegate, 0xac, 0);
		return true;
	case 186: // Mirar ponchera.
		beginRonSpeechLine(delegate, 0xad, 0);
		return true;
	case 187: // Mirar calzoncillo.
		beginRonSpeechLine(delegate, 0xae, 0);
		return true;
	case 188: // Usar calzoncillo.
		beginRonSpeechLine(delegate, 0xaf, 0);
		return true;
	case 189: // Mirar tratamiento capilar.
		beginRonSpeechLine(delegate, 0xb0, 0);
		return true;
	case 190: // Mirar espejo.
		beginRonSpeechLine(delegate, 0xb1, 0);
		return true;
	case 191: // Mirar pinzas.
		beginRonSpeechLine(delegate, 0xb2, 0);
		return true;
	case 192: // Hablar con espejo.
		beginRonSpeechLine(delegate, 0xb3, 0);
		return true;
	case 193: // Mirar rodaja de limon.
		beginRonSpeechLine(delegate, 0xb4, 0);
		return true;
	case 194: // Mirar folleto de adelgazamiento.
		beginRonSpeechLine(delegate, 0xb5, 0);
		return true;
	case 195: // Mirar pelicula.
		beginRonSpeechLine(delegate, 0xb6, 0);
		return true;
	case 196: // Mirar lupa.
		beginRonSpeechLine(delegate, 0xb7, 0);
		return true;
	case 197: // Mirar paraguas.
		beginRonSpeechLine(delegate, 0xb8, 0);
		return true;
	case 198: // Mirar saxofon.
		beginRonSpeechLine(delegate, 0xb9, 0);
		return true;
	case 199: // Usar saxofon.
		beginRonSpeechLine(delegate, 0xba, 0);
		return true;
	case 200: // Mirar cascara de huevo.
		beginRonSpeechLine(delegate, 0xbb, 0);
		return true;
	case 201: // Mirar bolso de Taffy.
		beginRonSpeechLine(delegate, 0xbc, 0);
		return true;
	case 203: // Mirar trocito de alambre.
		beginRonSpeechLine(delegate, 0xbe, 0);
		return true;
	case 204: // Mirar bote de pintura.
		beginRonSpeechLine(delegate, 0xbf, 0);
		return true;
	case 205: // Mirar tapa.
		beginRonSpeechLine(delegate, 0xc0, 0);
		return true;
	case 206: // Mirar pelota de cera.
		beginRonSpeechLine(delegate, 0xc1, 0);
		return true;
	case 208: // Mirar rata.
		beginRonSpeechLine(delegate, 0xc2, 0);
		return true;
	case 209: // Mirar maletin de Charlie.
		beginRonSpeechLine(delegate, 0xc3, 0);
		return true;
	case 210: // Abrir maletin de Charlie.
		beginRonSpeechLine(delegate, 0xc4, 0);
		return true;
	case 213: // Mirar foto de Robert Feynman.
		beginRonSpeechLine(delegate, 0xc6, 0);
		return true;
	case 215: // Mirar cartera.
		beginRonSpeechLine(delegate, 0xc8, 0);
		return true;
	case 216: // Usar/dar cartera.
		beginRonSpeechLine(delegate, 0xc9, 0);
		return true;
	case 218: // Mirar cuerda larga.
		beginRonSpeechLine(delegate, 0xcb, 0);
		return true;
	case 219: // Mirar huevo.
		beginRonSpeechLine(delegate, 0xcc, 0);
		return true;
	case 220: // Abrir huevo.
		beginRonSpeechLine(delegate, 0xcd, 0);
		return true;
	case 221: // Usar rata.
		beginRonSpeechLine(delegate, 0xce, 0);
		return true;
	case 226: // Generic use: no need.
		beginRonRandomSpeechLine(delegate, 0xd5, 1);
		return true;
	case 227: // Generic take: cannot take that.
		beginRonRandomSpeechLine(delegate, 0xd6, 1);
		return true;
	case 228: // Mirar sobre de Billy Ford.
		beginRonSpeechLine(delegate, 0xd7, 0);
		return true;
	case 229: // Abrir sobre de Billy Ford.
		beginRonSpeechLine(delegate, 0xd8, 0);
		return true;
	case 230: // Generic cut glass failure.
		beginRonSpeechLine(delegate, 0xd9, 0);
		return true;
	case 231: // Generic item combination failure.
		beginRonRandomSpeechLine(delegate, 0xda, 1);
		return true;
	case 234: // Agenda note: interesting, write it down.
		beginRonSpeechLine(delegate, 0xdb, 0);
		return true;
	case 235: // Generic "No sirve".
		beginRonSpeechLine(delegate, 0xe3, 0);
		return true;
	case 236: // Generic "No alcanzo".
		beginRonSpeechLine(delegate, 0xe4, 0);
		return true;
	case 238: // Generic "use the buttons there".
		beginRonSpeechLine(delegate, 0xe6, 0);
		return true;
	default:
		return false;
	}
}

bool isRonInventoryAction(uint16 handlerId) {
	if (handlerId >= 55 && handlerId <= 73)
		return true;
	if (handlerId >= 74 && handlerId <= 213 && handlerId != 94)
		return true;
	if (handlerId >= 215 && handlerId <= 221)
		return true;

	switch (handlerId) {
	case 2:
	case 20:
	case 30:
	case 32:
	case 34:
	case 40:
	case 44:
	case 52:
	case 53:
	case 226:
	case 227:
	case 228:
	case 229:
	case 230:
	case 231:
	case 232:
	case 233:
	case 234:
	case 235:
	case 236:
	case 237:
	case 238:
	case 239:
		return true;
	default:
		return false;
	}
}

void dispatchRonInventoryAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		uint16 handlerId, byte owner) {
	GameplayState &gameState = vm->gameState();

	switch (handlerId) {
	case 75:
		// Mirar agenda: destination selector.
		delegate->showTravelScreenViewer();
		break;
	case 55:
		// Usar iman con pastillero variants: attach magnet and transform the pillbox.
		gameState.removeInventoryItem(owner, 0x07);
		if (gameState.hasInventoryItem(owner, 0x0c)) {
			gameState.removeInventoryItem(owner, 0x0c);
			gameState.addInventoryItem(owner, 0x0b);
		} else if (gameState.hasInventoryItem(owner, 0x0f)) {
			gameState.removeInventoryItem(owner, 0x0f);
			gameState.addInventoryItem(owner, 0x0d);
		} else if (gameState.hasInventoryItem(owner, 0x3b)) {
			gameState.removeInventoryItem(owner, 0x3b);
			gameState.addInventoryItem(owner, 0x0d);
			gameState.ronPendingMabusePillsInMagnetPillbox = true;
		}
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x35, 0);
		break;
	case 57:
		// Usar pie de microfono con algodon empapado de grasa: creates long-distance greaser.
		gameState.removeInventoryItem(owner, 0x17);
		gameState.removeInventoryItem(owner, 0x24);
		gameState.addInventoryItem(owner, 0x10);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x35, 0);
		break;
	case 59:
		// Usar navaja con madero: original is state-gated; generic failure until that state exists.
		beginRonRandomSpeechLine(delegate, 0x01, 2);
		break;
	case 60:
		// Usar folleto pegajoso con tratamiento capilar: creates slimming treatment.
		gameState.removeInventoryItem(owner, 0x34);
		gameState.removeInventoryItem(owner, 0x54);
		gameState.addInventoryItem(owner, 0x15);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x38, 0);
		break;
	case 62:
		// Usar cuerda con cordon: creates cuerda larga.
		beginRonSpeechLine(delegate, 0x3a, 0);
		gameState.removeInventoryItem(owner, 0x3d);
		gameState.removeInventoryItem(owner, 0x1b);
		gameState.addInventoryItem(owner, 0x6b);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x35, 0);
		break;
	case 63:
		// Usar aceite con candil: fuels the lamp.
		gameState.removeInventoryItem(owner, 0x43);
		gameState.ronLampFueled = true;
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x3b, 0);
		break;
	case 65:
		// Usar filete con pedrusco: scene-flag gated; use original fallback until modeled.
		beginRonRandomSpeechLine(delegate, 0xda, 1);
		break;
	case 68:
		// Usar pase de Taffy con foto de Robert Feynman: creates forged pass.
		gameState.removeInventoryItem(owner, 0x67);
		gameState.removeInventoryItem(owner, 0x68);
		gameState.addInventoryItem(owner, 0x66);
		gameState.ronPosterPhotoRemoved = true;
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x40, 0);
		break;
	case 69:
		// Usar navaja/sobre with poster: state-gated photo grant; fallback is generic failure.
		beginRonRandomSpeechLine(delegate, 0x01, 2);
		break;
	case 85:
		// Mirar magnetofono.
		if (gameState.ronTapeRecorderState == 0) {
			beginRonSpeechLine(delegate, 0x50, 0);
			gameState.ronTapeRecorderState = 1;
		} else if (gameState.ronTapeRecorderState == 1) {
			beginRonSpeechLine(delegate, 0x50, 1);
		} else if (gameState.ronTapeRecorderState == 2) {
			beginRonSpeechLine(delegate, 0x50, 2);
		}
		break;
	case 86:
		// Usar magnetofono: empty warning, or play Sue's recorded tape.
		if (gameState.ronTapeRecorderState < 2) {
			beginRonSpeechLine(delegate, 0x50, 1);
		} else {
			const bool firstPlayback = !gameState.ronTravelScreenUnlocked;
			if (firstPlayback)
				beginRonSpeechLine(delegate, 0x51, 0);
			if (delegate->playSueTapeRecording() && !Engine::shouldQuit() &&
					!delegate->shouldExitGameplayLoop()) {
				if (firstPlayback)
					beginRonSpeechLine(delegate, 0x51, 9);
				gameState.ronTravelScreenUnlocked = true;
			}
		}
		break;
	case 87:
		// Abrir magnetofono: retrieve Sue's tape if it is inside.
		if (gameState.ronTapeRecorderState < 2) {
			beginRonSpeechLine(delegate, 0x50, 1);
		} else {
			gameState.addInventoryItem(owner, 0x16);
			playRonInventoryChange(delegate);
			gameState.ronTapeRecorderState = 1;
		}
		break;
	case 92:
		// Abrir pastillero con iman: optionally grants Dr. Mabuse's pills.
		if (gameState.ronPendingMabusePillsInMagnetPillbox) {
			gameState.addInventoryItem(owner, 0x0e);
			gameState.ronPendingMabusePillsInMagnetPillbox = false;
			playRonInventoryChange(delegate);
			beginRonSpeechLine(delegate, 0x56, 0);
		} else {
			beginRonSpeechLine(delegate, 0x50, 1);
		}
		break;
	case 96:
		// Abrir pastillero: grants empty pillbox and Mabuse pills.
		gameState.removeInventoryItem(owner, 0x3b);
		gameState.addInventoryItem(owner, 0x0f);
		gameState.addInventoryItem(owner, 0x0e);
		gameState.ronPendingMabusePillsInMagnetPillbox = false;
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x56, 0);
		break;
	case 102:
		// Abrir sobre: grants letter and papyrus once.
		if (gameState.hasInventoryItem(owner, 0x04) || gameState.hasInventoryItem(owner, 0x12)) {
			beginRonSpeechLine(delegate, 0x50, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x04);
		gameState.addInventoryItem(owner, 0x12);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x35, 0);
		break;
	case 110:
		// Abrir maletin de Jack: grants cotton, scalpel, syringe, and saw once.
		if (gameState.hasInventoryItem(owner, 0x03)) {
			beginRonSpeechLine(delegate, 0x50, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x02);
		gameState.addInventoryItem(owner, 0x03);
		gameState.addInventoryItem(owner, 0x08);
		gameState.addInventoryItem(owner, 0x13);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x35, 0);
		break;
	case 115:
		// Abrir paquete: grants remote control and Dr. Fly device.
		gameState.removeInventoryItem(owner, 0x1d);
		gameState.addInventoryItem(owner, 0x05);
		gameState.addInventoryItem(owner, 0x0a);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0x35, 0);
		break;
	case 116:
		// Cerrar paquete: original callback is a no-op.
		break;
	case 124:
		// Abrir microfono: grants the small magnet hidden inside.
		if (gameState.hasInventoryItem(owner, 0x07)) {
			beginRonSpeechLine(delegate, 0x50, 1);
			break;
		}
		grantRonItem(gameState, delegate, owner, 0x07);
		beginRonSpeechLine(delegate, 0x71, 0);
		break;
	case 133:
		// Mirar dinero: original follows with a money counter; show the lead-in line for now.
		beginRonSpeechLine(delegate, 0x7a, 0);
		break;
	case 147:
		if (delegate->showInventoryMedia(kInventoryMediaFrankensteinDiary) &&
				!Engine::shouldQuit() && !delegate->shouldExitGameplayLoop())
			gameState.frankensteinDiaryRead = true;
		break;
	case 184:
		// Abrir libro de Karl: grants the letter hidden inside once.
		if (gameState.hasInventoryItem(owner, 0x14)) {
			beginRonSpeechLine(delegate, 0x50, 1);
			break;
		}
		grantRonItem(gameState, delegate, owner, 0x14);
		beginRonSpeechLine(delegate, 0xab, 0);
		break;
	case 202:
		// Abrir bolso de Taffy: grants Taffy's pass unless a pass is already present.
		if (gameState.hasInventoryItem(owner, 0x66) || gameState.hasInventoryItem(owner, 0x67)) {
			beginRonSpeechLine(delegate, 0x50, 1);
			break;
		}
		beginRonSpeechLine(delegate, 0xbd, 0);
		grantRonItem(gameState, delegate, owner, 0x67);
		break;
	case 207:
		delegate->showInventoryMedia(gameState.ronPosterPhotoRemoved ?
			kInventoryMediaPosterWithoutPhoto : kInventoryMediaPoster);
		break;
	case 211:
		if (delegate->showInventoryMedia(kInventoryMediaForgedPass) &&
				!Engine::shouldQuit() && !delegate->shouldExitGameplayLoop())
			beginRonSpeechLine(delegate, 0xc5, 0);
		break;
	case 212:
		delegate->showInventoryMedia(kInventoryMediaTaffyPass);
		break;
	case 217:
		// Abrir cartera: grants money once.
		if (gameState.ronWalletOpened) {
			beginRonSpeechLine(delegate, 0xca, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x29);
		gameState.ronEgyptianMoneyAmount += 0x23;
		playRonInventoryChange(delegate);
		gameState.ronWalletOpened = true;
		beginRonSpeechLine(delegate, 0xca, 0);
		break;
	case 232:
		// Abrir navaja: cycles through the original multi-tool pages.
		advanceRonKnifePage(gameState, delegate, owner);
		break;
	case 233:
		// Cerrar navaja: resets the multi-tool page.
		closeRonKnifePage(gameState, delegate, owner);
		break;
	case 237:
		// Usar cinta de Sue con magnetofono: inserts the tape into the recorder.
		gameState.removeInventoryItem(owner, 0x16);
		gameState.ronTapeRecorderState = 2;
		playRonInventoryChange(delegate);
		break;
	case 239:
		// Usar navaja con trocito de alambre: requires the correct knife tool page.
		if (gameState.multiToolKnifeState != 7) {
			beginRonRandomSpeechLine(delegate, 0xda, 1);
			break;
		}
		setInventoryItemResourcePage(gameState, owner, 0x5f, 0x40);
		playRonInventoryChange(delegate);
		beginRonSpeechLine(delegate, 0xbe, 1);
		break;
	default:
		beginRonSimpleInventorySpeech(vm, delegate, handlerId);
		break;
	}
}

bool dispatchInventoryFixedAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		const GameplayLoopCursorState &state) {
	if (state.relationModeActive || !state.inventoryItemSelected)
		return false;

	// The original main callback table redirects these handlers to scene-local callbacks.
	const uint16 sceneId = vm->gameState().mainFlowStateId / 10;
	if ((sceneId == 304 && state.inventoryActionHandlerId == 89) ||
			(sceneId == 307 &&
				(state.inventoryActionHandlerId == 81 || state.inventoryActionHandlerId == 127)) ||
			(sceneId == 108 && state.inventoryActionHandlerId == 178) ||
			(vm->gameState().mainFlowStateId == 0x082a && state.inventoryActionHandlerId == 136))
		return false;

	return dispatchSharedInventoryAction(vm, delegate, state.inventoryActionHandlerId,
		state.inventoryOwner);
}

bool dispatchInventoryRelationAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		const GameplayLoopCursorState &state) {
	if (!state.relationModeActive || !state.inventoryItemSelected)
		return false;

	return dispatchSharedInventoryAction(vm, delegate, state.inventoryActionHandlerId,
		state.inventoryOwner);
}

bool dispatchSharedInventoryAction(HollywoodEngine *vm, GameplayLoopDelegate *delegate,
		uint16 handlerId, byte owner) {
	// Mirrors the low shared table installed by InstallSceneActionCallbackTable
	// at 004d6000. This is not the larger main-scene callback table.
	const bool sharedHandler =
		handlerId == 0 || handlerId == 1 || handlerId == 35 ||
		handlerId == 41 || handlerId == 45 || handlerId == 49 ||
		(handlerId >= 2 && handlerId <= 34) ||
		handlerId == 36 || handlerId == 37 ||
		handlerId == 38 || handlerId == 39 || handlerId == 40 ||
		handlerId == 42 || handlerId == 43 || handlerId == 44 ||
		handlerId == 46 || handlerId == 47 || handlerId == 48 ||
		(handlerId >= 50 && handlerId <= 71);
	const bool ronHandler = owner == 0 && isRonInventoryAction(handlerId);
	if (!sharedHandler && !ronHandler)
		return false;

	vm->cursor()->leaveInteractiveMode();

	if (ronHandler) {
		dispatchRonInventoryAction(vm, delegate, handlerId, owner);
		if (!Engine::shouldQuit() && !delegate->shouldExitGameplayLoop()) {
			vm->cursor()->enterInteractiveMode();
			vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
		}
		return true;
	}

	switch (handlerId) {
	case 0:
	case 1:
	case 35:
	case 41:
	case 45:
	case 49:
		// Original default callback: consume the action without speech.
		break;
	case 2:
		delegate->beginSharedInventorySpeechLine(1, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 3:
		delegate->beginSharedInventorySpeechLine(2, 0);
		break;
	case 4:
		delegate->beginSharedInventorySpeechLine(3, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 5:
	{
		const byte variant = delegate->randomSharedInventorySpeechFrame(2);
		if (variant == 2)
			delegate->beginSharedInventorySpeechLine(3, 1);
		else
			delegate->beginSharedInventorySpeechLine(4, variant);
		break;
	}
	case 6:
		delegate->beginSharedInventorySpeechLine(5, 0);
		break;
	case 7:
		delegate->beginSharedInventorySpeechLine(6, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 8:
		delegate->beginSharedInventorySpeechLine(7, 0);
		break;
	case 9:
		delegate->beginSharedInventorySpeechLine(8, 0);
		break;
	case 10:
		delegate->beginSharedInventorySpeechLine(9, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 11:
		delegate->beginSharedInventorySpeechLine(0x0a, 0);
		break;
	case 12:
		delegate->beginSharedInventorySpeechLine(0x0b, 0);
		break;
	case 13:
		delegate->beginSharedInventorySpeechLine(0x0c, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 14:
		delegate->beginSharedInventorySpeechLine(0x0d, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 15:
		delegate->beginSharedInventorySpeechLine(0x0e, 0);
		break;
	case 16:
		delegate->beginSharedInventorySpeechLine(0x0f, delegate->randomSharedInventorySpeechFrame(2));
		break;
	case 17:
		delegate->beginSharedInventorySpeechLine(0x10, 0);
		break;
	case 18:
		delegate->beginSharedInventorySpeechLine(0x11, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 19:
		delegate->beginSharedInventorySpeechLine(0x12, delegate->randomSharedInventorySpeechFrame(2));
		break;
	case 20:
		delegate->beginSharedInventorySpeechLine(0x13, 0);
		break;
	case 21:
		delegate->beginSharedInventorySpeechLine(0x14, 0);
		break;
	case 22:
		delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	case 23:
		delegate->beginSharedInventorySpeechLine(0x16, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 24:
		delegate->beginSharedInventorySpeechLine(0x17, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 25:
		delegate->beginSharedInventorySpeechLine(0x18, delegate->randomSharedInventorySpeechFrame(1));
		break;
	case 26:
		delegate->beginSharedInventorySpeechLine(0x19, 0);
		break;
	case 27:
		delegate->beginSharedInventorySpeechLine(0x1a, 0);
		break;
	case 28:
		delegate->beginSharedInventorySpeechLine(0x1b, 0);
		break;
	case 29:
		delegate->beginSharedInventorySpeechLine(0x1c, 0);
		break;
	case 30:
		delegate->beginSharedInventorySpeechLine(0x1d, 0);
		break;
	case 31:
		delegate->beginSharedInventorySpeechLine(0x1e, 0);
		break;
	case 32:
		delegate->beginSharedInventorySpeechLine(0x1f, 0);
		break;
	case 33:
		delegate->beginSharedInventorySpeechLine(0x20, 0);
		break;
	case 34:
		delegate->beginSharedInventorySpeechLine(0x21, 0);
		break;
	case 36:
		delegate->beginSharedInventorySpeechLine(0x23, 0);
		break;
	case 37:
		// Original callback 0x25: row 0x43 frame 1, row 0x24 frame 0,
		// then row 0x43 frame 2, with a render pass between each line.
		delegate->beginSharedInventorySpeechLine(0x43, 1);
		delegate->beginSharedInventorySpeechLine(0x24, 0);
		delegate->beginSharedInventorySpeechLine(0x43, 2);
		break;
	case 38:
		delegate->beginSharedInventorySpeechLine(0x25, 0);
		break;
	case 39:
		delegate->beginSharedInventorySpeechLine(0x26, 0);
		break;
	case 40:
		delegate->beginSharedInventorySpeechLine(0x27, 0);
		break;
	case 42:
		delegate->beginSharedInventorySpeechLine(0x29, 0);
		break;
	case 43:
		delegate->beginSharedInventorySpeechLine(0x2a, 0);
		break;
	case 44:
		delegate->beginSharedInventorySpeechLine(0x2b, 0);
		break;
	case 46:
		delegate->beginSharedInventorySpeechLine(0x2d, 0);
		break;
	case 47:
		delegate->beginSharedInventorySpeechLine(0x2e, 0);
		break;
	case 48:
		delegate->beginSharedInventorySpeechLine(0x2f, 0);
		break;
	case 50:
		delegate->beginSharedInventorySpeechLine(0x31, 0);
		break;
	case 51:
		delegate->beginSharedInventorySpeechLine(0x32, 0);
		break;
	case 52:
		delegate->beginSharedInventorySpeechLine(0x33, 0);
		break;
	case 53:
		delegate->beginSharedInventorySpeechLine(0x34, 0);
		break;
	case 54:
		delegate->beginSharedInventorySpeechLine(0x35, 0);
		break;
	case 55:
		delegate->beginSharedInventorySpeechLine(0x36, 0);
		break;
	case 56:
		delegate->beginSharedInventorySpeechLine(0x37, 0);
		break;
	case 57:
		delegate->beginSharedInventorySpeechLine(0x38, 0);
		break;
	case 58:
		delegate->beginSharedInventorySpeechLine(0x39, 0);
		break;
	case 59:
		delegate->beginSharedInventorySpeechLine(0x3a, 0);
		break;
	case 60:
		delegate->beginSharedInventorySpeechLine(0x3b, 0);
		break;
	case 61:
		delegate->beginSharedInventorySpeechLine(0x3c, 0);
		break;
	case 62:
		delegate->beginSharedInventorySpeechLine(0x3d, 0);
		break;
	case 63:
		delegate->beginSharedInventorySpeechLine(0x3e, 0);
		break;
	case 64:
		delegate->beginSharedInventorySpeechLine(0x3f, 0);
		break;
	case 65:
		delegate->beginSharedInventorySpeechLine(0x40, 0);
		break;
	case 66:
	{
		// Original callback 0x42: grant item 0x22 once.
		GameplayState &gameState = vm->gameState();
		if (gameState.hasInventoryItem(owner, 0x22)) {
			delegate->beginSharedInventorySpeechLine(0x41, 1);
			break;
		}
		gameState.addInventoryItem(owner, 0x22);
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x41, 0);
		break;
	}
	case 67:
		delegate->beginSharedInventorySpeechLine(0x42, 0);
		break;
	case 68:
	{
		// Original callback 0x44: combine item 0x08 with item 0x0f,
		// grant item 0x06, then run callback 0x25's speech sequence.
		GameplayState &gameState = vm->gameState();
		delegate->beginSharedInventorySpeechLine(0x43, 0);
		gameState.removeInventoryItem(owner, 0x08);
		gameState.removeInventoryItem(owner, 0x0f);
		gameState.addInventoryItem(owner, 0x06);
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x43, 1);
		delegate->beginSharedInventorySpeechLine(0x24, 0);
		delegate->beginSharedInventorySpeechLine(0x43, 2);
		break;
	}
	case 69:
		delegate->beginSharedInventorySpeechLine(0x44, 0);
		break;
	case 70:
	{
		// Original callback 0x46: advance item 0x1a through its alternate pages.
		GameplayState &gameState = vm->gameState();
		if ((gameState.multiToolKnifeState & 1) != 0) {
			if (gameState.multiToolKnifeState < 9)
				++gameState.multiToolKnifeState;
			else
				gameState.multiToolKnifeState = 0;
		}

		byte pageIndex = 0;
		switch (gameState.multiToolKnifeState) {
		case 0:
			gameState.multiToolKnifeState = 1;
			pageIndex = 0x1e;
			break;
		case 2:
			gameState.multiToolKnifeState = 3;
			pageIndex = 0x2d;
			break;
		case 4:
			gameState.multiToolKnifeState = 5;
			pageIndex = 0x7d;
			break;
		case 6:
			gameState.multiToolKnifeState = 7;
			pageIndex = 0x0e;
			break;
		case 8:
			gameState.multiToolKnifeState = 9;
			pageIndex = 0x18;
			break;
		default:
			break;
		}

		if (owner < GameplayState::kInventoryOwnerCount && pageIndex != 0) {
			gameState.inventoryItemResourcePageByOwnerAndItemId[owner][0x1a] = pageIndex;
			gameState.inventoryPanelDirty = true;
		}
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	}
	case 71:
	{
		// Original callback 0x47: reset item 0x1a to page 0x68 after a valid state.
		GameplayState &gameState = vm->gameState();
		if ((gameState.multiToolKnifeState & 1) == 0) {
			delegate->beginSharedInventorySpeechLine(0x0b, 0);
			break;
		}

		if (gameState.multiToolKnifeState == 9)
			gameState.multiToolKnifeState = 0;
		else
			++gameState.multiToolKnifeState;

		if (owner < GameplayState::kInventoryOwnerCount) {
			gameState.inventoryItemResourcePageByOwnerAndItemId[owner][0x1a] = 0x68;
			gameState.inventoryPanelDirty = true;
		}
		delegate->playSharedInventorySound(1);
		delegate->beginSharedInventorySpeechLine(0x15, 0);
		break;
	}
	default:
		return false;
	}

	if (!Engine::shouldQuit() && !delegate->shouldExitGameplayLoop()) {
		vm->cursor()->enterInteractiveMode();
		vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
	return true;
}

} // End of namespace Hollywood
