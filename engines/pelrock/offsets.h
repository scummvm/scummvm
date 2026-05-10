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

#include "pelrock/types.h"

namespace Pelrock {

// Indices to in game text responses in JUEGO.EXE
enum TextStringId {
	kTextEstanCerrados,
	kTextHoyNoDisponibles,
	kTextYaAbiertoM,
	kTextYaCerradoM,
	kTextYaAbiertaF,
	kTextYaCerradaF,
	kTextHeladeraCerrada,
	kTextPobreNoHeLlegadoAEso,
	kTextQueBuenaEsta,
	kTextBotonVerdeParaSacarCancelar,
	kTextPrimeroMetaTarjeta,
	kTextNoTengoMasDinero,
	kTextMeHeVueltoAPinchar,
	kTextQueHaSidoEso,
	kTextQuienAndaAhi,
	kTextYoMeVoy,
	kTextTiendaCerrada,
	kTextNoseEnteraria,
	kTextPrimeroAbrirlo,
	kTextQueLosCojaSupadre,
	kTextPretendeUstedSobornarme,
	kTextMuyBien1,
	kTextCuandoMeLoPida,
	kTextDeAcuerdo,
	kTextNecesitaDni,
	kTextQueReciboACambio,
	kTextEsPoco,
	kTextAlto,
	kTextNiParaEmpezar,
	kTextParaQue,
	kTextDePiedraNoDeHielo,
	kTextNoEmpecemos,
	kTextCuerpoDanone,
	kTextCabezaHueca,
	kTextEsoLoSerasTu,
	kTextDemasiadoNoPuedoPensar,
	kTextUnPocoRespeto,
	kTextNoTheyMakeyouFat,
	kTextRelojHaCambiado,
	kTextCorrespondenciaAjena,
	kTextAnda,
	kTextTuCrees,
	kTextNoEsAMiAQuienDebes,
	kTextAQuienEntonces,
	kTextLibrosSecretos,
	kTextVengaAca,
	kTextTodos,
	kTextElLibroNoEstaAqui,
	kTextTendreDejarLibro,
	kTextTrabajariaMejorSiNoMeMolestara,
	kTextRegaloLibroRecetas,
	kTextYSiMeTiraMaquina,
	kTextQuitaEsasManos,
	kTextQueAsco,
	kTextQuesEstoReceta,
	kTextYaEstaAbierto,
	kTextVaestarPocoFuerte,
	kTextCuentoParecido,
	kTextCosasAprendido,
	kTextPeriodicoSensacionalista,
	kTextHojaEntrePaginas,
	kTextNoEntiendonada,
	kTextNoTengoDinero,
	kTextCuesta1000,
	kTextAquiTiene,
	kTextMuyBien,
	kTextYaSeEgipcio,
	kTextQueLastimaNoSeeEgipcio,
	kTextFormulaViajeAlTiempo,
	kTextPareceCerrado,
	kTextNovio2Metros,
	kTextGranIdea,
	kTextSeLorecomiendo,
	kTextOigaUsted,
	kTextEsAMi,
	kTextVengaAhoraMismo,
	kTextCuidadoImprudente,
	kTextQueOscuroEstaEsto,
	kTextMenudaAventura,
	kTextNecesitaGasolina,
	kTextYaNoSeHaceOnComoAntes,
	kTextNadieLaHaVisto,
	kTextAyAyAy,
	kTextOigaUsted2,
	kTextLeEstoyVigilando,
	kTextOiga,
	kTextCapituloParadojas,
	kTextHayQueCelebrarlo,
	kTextPesaDemasiado,
	kTextNingunaTamanhoApropiado,
	kTextParaQueCogeBarro,
	kTextBuenoCogereUnPoco,
	kTextAbsolutamenteCerrado,
	kTextNoSeTeCurraCercarte,
	kTextPuertaAutenticaIzquierda,
	kTextOhMiSalvador,
	kTextVoyPoriPrincesa,
	kTextAMisBrazos,
	kTextDiosMioQueEsEsto,
	kTextQuePasa,
	kTextOlvideCerrarTrampilla,
	kTextNotePreocupesVolvere,
	kTextAlaConUsted,
	kTextMeMeo,
	kTextPor5Minutos,
	kTextTaLuegoLucas,
	kTextSiSupieraCombinacion,
	kTextPareceCombinacionCajaFuerte,
	kTextGranCantidadDinero,
	kTextTeApeteceBuenRato,
	kTextYLosCondones,
	kTextQueAscoCasiMeMea,
	kTextHechoElPrimo,
	kTextMeHanTomadoElPelo,
	kTextPesadoUnRato,
	kTextTraidor,
	kTextTuTia,
	kTextLaTuya,
	kTextGordo,
	kTextFideo,
	kTextLimpiaculos,
	kTextConTuTurbante,
	kTextOso,
	kTextComadreja,
	kTextCabezon,
	kTextTuAbuelo,
	kTextTuMujer,
	kTextPerdedor,
	kTextSoyMejorQueTu,
	kTextTramposo,
	kTextMalPerdedor,
	kTextParaUnaVez,
	kTextMejorMeLargo,
	kTextNoTengoParches,
	kTextNoTengoPegamento,
	kTextMuecoArreglado,
	kTextMareDeDou,
	kTextProbarLibro,
	kTextPracticarMas,
	kTextAquiNoNecesito,
	kTextDiosHalcon,
	kTextOhGranOsiris,
	kTextHemeAqui,
	kTextOhSobek,
	kTextOhToth,
	kTextTodasLasCosas,
	kTextHeLlegadoPuro,
	kTextDiosDeLaTurbulencia,
	kTextOhAnubis,
	kTextHeVenido,
	kTextHeLlegadoATi,
	kTextOhPtha,
	kTextLasPuertasDelCielo,
	kTextVayaSuenho,
	kTextParaQue2,
	kTextYEso,
	kTextUnPocoDeseperado,
	kTextCombinacionesMejores,
	kTextNoSeQuePretendeConEso,
	kTextComo,
	kTextMuchoSentido,
	kTextPorProbar,
	kTextNoLoEntiendo,
	kTextParaEsoNoSirve,
	kTextPruebaOtraCosa,
	kTextSiHombrQueEmas,
	kTextNoSeQuePretendes,
	kTextCosasRaras,
	kTextArteOLocura,
	kTextUtilidades,
	kTextTituloJuego,
	kTextMensajeOtraEpoca,
	kTextNoEraAutentico,
	kTextPrimerIngrediente,
	kTextDosIngredientes,
	kTextTresIngredientes,
	kTextCuatroIngredientes,
	kTextLibroAburrido,
	kTextDeAcuerdo2,
	kTextGamberros,
	kTextQuienYo,
	kTextPintaBuenaPersona,
	kTextDemoFinal,
	kTextDiosHalcon2,
	kTextGranOsiris,
	kTextHemeAquiIsis,
	kTextOhSobek2,
	kTextOhToth2,
	kTextProtigenMiCuerpo,
	kTextHeLlegadoPuro2,
	kTextDiosDeLaTurbulencia2,
	kTextOhAnubis2,
	kTextHeVenido2,
	kTextHeLlegadoATi2,
	kTextOhPtha2,
	kTextLasPuertasDelCielo2,
};

// ALFRED.7 extra screen data (file offsets given per entry in extraScreens[])
static const ExtraScreen extraScreens[] = {
	{0x0000000, 0x0007984, 8},      // 0 - Portrait above bed
	{0x001A9EE, 0x00305A2, 8},        // 1 - Computer screen
	{0x00647C3, 0x007B6B1, 4},        // 2 - Alfred circle
	{0x006FBCD, 0x007B6B1, 8},        // 3 - Recipe
	{0x007BA11, 0x0088745, 8},        // 4 - Newspaper
	{0x009237B, 0x00B0EE7, 8},        // 5 - Tablet
	{0x00B11F1, 0x00DE011, 8},        // 6 - Map
	{0x00FFC47, 0x01180C9, 8},        // 7 - Girl book
	{0x01183C9, 0x01358F3, 8},         // 8 - Spellbook
	{0x0152A88, 0x015BFC8, 8},         // 9 - Portrait
	{0x0299E8C, 0x02B3B7C, 8},         // 10 - CD
	{0x02B3E7C, 0x02D5898, 8},         // 11 - Pyramid map
	{0x02331EA, 0x0236AA8, 8},         // 12 - CENSORED
	{0x0226358, 0x0236AA8, 8},         // 13 - Background book
	{0x02EAA32, 0x0309A80, 8}          // 14 - Ending
};


// Alfred.7 special animation data (file index given per entry in alfredSpecialAnims[])
static const uint32 kAlfredAnimReadBookOffset = 559685;       // 0  - READ BOOK
static const uint32 kAlfredAnimReadRecipeOffset = 578943;     // 1  - READ RECIPE
static const uint32 kAlfredAnimElectricShock1Offset = 37000;  // 2  - ELECTRIC SHOCK 1
static const uint32 kAlfredAnimElectricShock3Offset = 53106;  // 3  - ELECTRIC SHOCK 3
static const uint32 kAlfredAnimThrowOffset = 20724;           // 4  - Throw
static const uint32 kAlfredAnimThrowSize = 62480;             // 4  - Throw explicit size
static const uint32 kAlfredAnimCrocodileOffset = 1556540;     // 5  - Crocodile
static const uint32 kAlfredAnimManholeOffset = 1583702;       // 6  - Exit manhole
static const uint32 kAlfredAnimClimbDownOffset = 1761234;     // 7  - Climbs down
static const uint32 kAlfredAnimClimbUpOffset = 1766378;       // 8  - Climbs up
static const uint32 kAlfredAnimExitTunnelOffset = 1770196;    // 9  - Exits tunnel
static const uint32 kAlfredAnimWorkersOffset = 1600956;       // 10 - With workers
static const uint32 kAlfredAnimMunheco1Offset = 2060916;      // 11 - Doll 1
static const uint32 kAlfredAnimMunheco2Offset = 2115632;      // 12 - Doll 2
static const uint32 kAlfredAnimMunheco3Offset = 1526432;      // 13 - Doll 3
static const uint32 kAlfredAnimDescamisaOffset = 2972568;     // 14 - Descamisa
static const uint32 kAlfredAnimSecretPassageOffset = 1749464; // 15 - Secret passage
static const uint32 kAlfredAnimInBedOffset = 3038454;         // 16 - Alfred in bed

} // End of namespace Pelrock
#endif
