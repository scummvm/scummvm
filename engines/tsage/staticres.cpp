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

#include "tsage/staticres.h"

namespace TsAGE {

const byte CURSOR_ARROW_DATA[] = {
	15, 0, 15, 0, 0, 0, 0, 0, 9, 0,
	0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF,	0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09
};

const byte CURSOR_WALK_DATA[] = {
	15, 0, 15, 0, 7, 0, 7, 0, 9, 0,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09,	0x09, 0x09, 0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x09, 0x09, 0x09, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x09,	0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09
};

char const *const LOOK_SCENE_HOTSPOT = "You see nothing special.";
char const *const USE_SCENE_HOTSPOT = "That accomplishes nothing.";
char const *const TALK_SCENE_HOTSPOT = "Yak, yak.";
char const *const SPECIAL_SCENE_HOTSPOT = "That is a unique use for that.";
char const *const DEFAULT_SCENE_HOTSPOT = "That accomplishes nothing.";
char const *const SAVE_ERROR_MSG = "Error occurred saving game. Please do not try to restore this game!";
char const *const SAVING_NOT_ALLOWED_MSG = "Saving is not allowed at this time.";
char const *const RESTORING_NOT_ALLOWED_MSG = "Restoring is not allowed at this time.";
// FIXME: Spelling incorrect. possesion should be possession. Fixing changes game data and thus may cause issues
char const *const INV_EMPTY_MSG = "You have nothing in your possesion.";

char const *const QUIT_CONFIRM_MSG = "Do you want to quit playing this game?";
char const *const RESTART_MSG = "Do you want to restart this game?";
char const *const GAME_PAUSED_MSG = "Game is paused.";
char const *const OK_BTN_STRING = " Ok ";
char const *const CONTINUE_BTN_STRING = " Continue ";
char const *const CANCEL_BTN_STRING = "Cancel";
char const *const QUIT_BTN_STRING = " Quit ";
char const *const RESTART_BTN_STRING = "Restart";
char const *const SAVE_BTN_STRING = "Save";
char const *const RESTORE_BTN_STRING = "Restore";
char const *const SOUND_BTN_STRING = "Sound";
char const *const RESUME_BTN_STRING = " Resume \rplay";
char const *const LOOK_BTN_STRING = "Look";
char const *const PICK_BTN_STRING = "Pick";

// Spanish version
char const *const ESP_LOOK_SCENE_HOTSPOT = "No ves nada especial.";
char const *const ESP_USE_SCENE_HOTSPOT = "Con eso no conseguir\240s nada.";
char const *const ESP_TALK_SCENE_HOTSPOT = "Yak, yak.";
char const *const ESP_SPECIAL_SCENE_HOTSPOT = "Es una forma original de usar eso.";
char const *const ESP_DEFAULT_SCENE_HOTSPOT = "No ves nada especial.";
char const *const ESP_SAVE_ERROR_MSG = "Error salvando el juego. \255Por favor, no intentes recuperarlo!";
char const *const ESP_SAVING_NOT_ALLOWED_MSG = "No es posible salvar en este momento.";
char const *const ESP_RESTORING_NOT_ALLOWED_MSG = "No es posible recuperar una partida en este momento.";
char const *const ESP_INV_EMPTY_MSG = "No llevas nada contigo.";

char const *const ESP_OK_BTN_STRING = " Ok ";
char const *const ESP_CONTINUE_BTN_STRING = " Continuar ";
char const *const ESP_CANCEL_BTN_STRING = "Cancelar";
char const *const ESP_RESTORE_BTN_STRING = "Recuperar";
char const *const ESP_SOUND_BTN_STRING = "Sonido";
char const *const ESP_LOOK_BTN_STRING = "Mirar";
char const *const ESP_PICK_BTN_STRING = "Escoger";

// Russian version
char const *const RUS_LOOK_SCENE_HOTSPOT = "\x8D\xA8\xE7\xA5\xA3\xAE \xAD\xA5\xAE\xA1\xEB\xE7\xAD\xAE\xA3\xAE."; // "Ничего необычного."
char const *const RUS_USE_SCENE_HOTSPOT = "\x22\x81\xA5\xE1\xAF\xAE\xAB\xA5\xA7\xAD\xAE."; // "Бесполезно."
char const *const RUS_TALK_SCENE_HOTSPOT = "\x81\xAB\xA0-\xA1\xAB\xA0."; // "Бла-бла."
char const *const RUS_SPECIAL_SCENE_HOTSPOT = "\x22\x8D\xA0\xA4\xAE \xA6\xA5 \xA4\xAE\xA4\xE3\xAC\xA0\xE2\xEC\xE1\xEF \xA4\xAE \xE2\xA0\xAA\xAE\xA3\xAE."; // "Надо же додуматься до такого."
char const *const RUS_DEFAULT_SCENE_HOTSPOT = "\x81\xA5\xE1\xAF\xAE\xAB\xA5\xA7\xAD\xAE."; // "Бесполезно."
char const *const RUS_SAVE_ERROR_MSG = "\x8E\xE8\xA8\xA1\xAA\xA0 \xA7\xA0\xA3\xE0\xE3\xA7\xAA\xA8 \xE1\xAE\xE5\xE0\xA0\xAD\xF1\xAD\xAD\xAE\xA9 \xA8\xA3\xE0\xEB!"; // "Ошибка загрузки сохранённой игры!"
char const *const RUS_SAVING_NOT_ALLOWED_MSG = "\x91\xA5\xA9\xE7\xA0\xE1 \xA7\xA0\xAF\xA8\xE1\xEC \xAD\xA5\xA2\xAE\xA7\xAC\xAE\xA6\xAD."; // "Сейчас запись невозможна."
char const *const RUS_RESTORING_NOT_ALLOWED_MSG = "\x87\xA0\xA3\xE0\xE3\xA7\xAA\xA0 \xAD\xA5\xA2\xAE\xA7\xAC\xAE\xA6\xAD\xA0."; // "Загрузка невозможна."
char const *const RUS_INV_EMPTY_MSG = "\x22\x88\xAD\xA2\xA5\xAD\xE2\xA0\xE0\xEC \xAF\xE3\xE1\xE2."; // "Инвентарь пуст."

char const *const RUS_OK_BTN_STRING = " \x4F\x6B "; // " Ok "
char const *const RUS_CONTINUE_BTN_STRING = "\x84\xA0\xAB\xA5\xA5"; // "Далее"
char const *const RUS_CANCEL_BTN_STRING = "\x8E\xE2\xAC\xA5\xAD\xA0"; // "Отмена"
char const *const RUS_RESTORE_BTN_STRING = "\x91\xE7\xA8\xE2\xA0\xE2\xEC"; // "Считать"
char const *const RUS_SOUND_BTN_STRING = "  \x87\xA2\xE3\xAA  "; // "  Звук  "
char const *const RUS_LOOK_BTN_STRING = "\x83\xAB\xA0\xA7"; // "Глаз"
char const *const RUS_PICK_BTN_STRING = "\x90\xE3\xAA\xA0"; // "Рука"

namespace Ringworld {

// Dialog resources
char const *const HELP_MSG = "Ringworld\rRevenge of the Patriarch\x14\rScummVM Version\r\r\
\x01 Keyboard shortcuts...\rF2 - Sound options\rF3 - Quit\r\
F4 - Restart\rF5 - Save game\rF7 - Restore Game\rF10 - Pause game";
char const *const WATCH_INTRO_MSG = "Do you wish to watch the introduction?";
char const *const START_PLAY_BTN_STRING = " Start Play ";
char const *const INTRODUCTION_BTN_STRING = "Introduction";
char const *const OPTIONS_MSG = "\x01Options...";

// Scene specific resources
char const *const EXIT_MSG = "   EXIT   ";
char const *const SCENE6100_CAREFUL = "Be careful! The probe cannot handle too much of that.";
char const *const SCENE6100_TOUGHER = "Hey! This is tougher than it looks!";
char const *const SCENE6100_ONE_MORE_HIT = "You had better be more careful. One more hit like that \
and the probe may be destroyed.";
char const *const SCENE6100_DOING_BEST = "I'm doing the best I can. I just hope it holds together!";
char const *const SCENE6100_REPAIR = "\r\rQuinn and Seeker repair the probe....";
char const *const SCENE6100_ROCKY_AREA = "The rocky area should be directly ahead of you. Do you see it?";
char const *const SCENE6100_REPLY = "Yes. Now if I can just avoid those sunbeams.";
char const *const SCENE6100_TAKE_CONTROLS = "You had better take the controls Seeker. My hands are sweating.";
char const *const SCENE6100_SURPRISE = "You surprise me Quinn. I would have thought you of hardier stock.";
char const *const SCENE6100_SWEAT = "Humans sweat, Kzin twitch their tail. What's the difference?";
char const *const SCENE6100_VERY_WELL = "Very well. I will retrieve the stasis box and return the probe. \
Wait for it's return in the lander bay.";

char const *const DEMO_HELP_MSG = " Help...\rF2 - Sound Options\rF3 - Exit demo\r\rPress ENTER\rto continue";
char const *const DEMO_PAUSED_MSG = "Ringworld\x14 demo is paused";
char const *const DEMO_EXIT_MSG = "Press ENTER to resume the Ringworld\x14 demo. Press ESC to exit";
char const *const EXIT_BTN_STRING = "Exit";
char const *const DEMO_BTN_STRING = "Demo";
char const *const DEMO_RESUME_BTN_STRING = "Resume";

// Spanish version
char const *const ESP_QUIT_CONFIRM_1_MSG = "\250Quieres abandonar el juego?";
char const *const ESP_QUIT_CONFIRM_2_MSG = "\250Quieres abandonar esta partida?";
char const *const ESP_RESTART_MSG = "   \250Quieres empezar de nuevo?    ";
char const *const ESP_GAME_PAUSED_MSG = "Juego pausado";
char const *const ESP_QUIT_BTN_STRING = " Abandonar ";
char const *const ESP_RESTART_BTN_1_STRING = "Empezar \rde nuevo";
char const *const ESP_RESTART_BTN_2_STRING = "Empezar de nuevo";
char const *const ESP_SAVE_BTN_STRING = "Salvar";
char const *const ESP_RESUME_BTN_STRING = " Continuar \rjuego";

// Dialog resources
char const *const ESP_HELP_MSG = "Mundo Anillo\x14\rLa Venganza del Patriarca\rVersi\242n ScummVM\r\r\
\x01 Teclas de aceso r\240pido...\rF2 - Opciones de Sonido\rF3 - Abandonar\r\
F4 - Empezar de nuevo\rF5 - Salvar\rF7 - Recuperar\rF10 - Pausa";
char const *const ESP_WATCH_INTRO_MSG = "        \250Quieres ver la introducci\242n?         ";
char const *const ESP_START_PLAY_BTN_STRING = " Comenzar el Juego ";
char const *const ESP_INTRODUCTION_BTN_STRING = "Introducci\242n";
char const *const ESP_OPTIONS_MSG = "\x01Opciones...";

// Scene specific resources
char const *const ESP_EXIT_MSG = "   SALIR   ";
char const *const ESP_SCENE6100_CAREFUL = "\255Cuidado! La sonda no puede manipular mucho de eso.";
char const *const ESP_SCENE6100_TOUGHER = "\255Hey! Esto es m\240s duro de lo que parece!";
char const *const ESP_SCENE6100_ONE_MORE_HIT = "Deber\241as tener m\240s cuidado. Un impacto m\240s como \
ese y la sonda podr\241a ser destruida";
char const *const ESP_SCENE6100_DOING_BEST = "Lo hago lo mejor que puedo. \255Espero que se mantenga de \
una pieza!";
char const *const ESP_SCENE6100_REPAIR = "\r\rQuinn y Seeker reparan la sonda....";
char const *const ESP_SCENE6100_ROCKY_AREA = "La zona rocosa deber\241a estar justo en frente de t\241. \
\250La ves?";
char const *const ESP_SCENE6100_REPLY = "S\241. Ahora a ver si soy capaz de esquivar esos rayos de luz \
solar.";
char const *const ESP_SCENE6100_TAKE_CONTROLS = "Toma t\243 los controles Seeker. Me sudan las manos.";
char const *const ESP_SCENE6100_SURPRISE = "Me sorprendes Quinn. Pens\202 que aguantar\241as m\240s";
char const *const ESP_SCENE6100_SWEAT = "Los humanos sudan, los Kzinti mueven convulsivamente su cola. \
\250D\242nde est\240 la diferencia?";
char const *const ESP_SCENE6100_VERY_WELL = "Muy bien. Recoger\202 la caja est\240sica y traer\202 la \
sonda. Espera en la bodega de carga.";

// Russian version
char const *const RUS_QUIT_CONFIRM_MSG = "\x82\xEB \xE5\xAE\xE2\xA8\xE2\xA5 \xA2\xEB\xA9\xE2\xA8 \xA8\xA7 \xA8\xA3\xE0\xEB?"; // "Вы хотите выйти из игры?"
char const *const RUS_RESTART_MSG = "\x82\xEB \xE5\xAE\xE2\xA8\xE2\xA5 \xAD\xA0\xE7\xA0\xE2\xEC \xA7\xA0\xAD\xAE\xA2\xAE?"; // "Вы хотите начать заново?"
char const *const RUS_GAME_PAUSED_MSG = "  \x8F\xA0\xE3\xA7\xA0.  "; // "  Пауза.  ";
char const *const RUS_QUIT_BTN_STRING = " \x82\xEB\xA9\xE2\xA8"; // " Выйти";
char const *const RUS_RESTART_BTN_STRING = "\x87\xA0\xAD\xAE\xA2\xAE"; // "Заново";
char const *const RUS_SAVE_BTN_STRING = " \x87\xA0\xAF\xA8\xE1\xEC "; // " Запись ";
char const *const RUS_RESUME_BTN_STRING = " \x8D\xA0\xA7\xA0\xA4 \xA2\r\xA8\xA3\xE0\xE3"; // " Назад в\rигру";

// Dialog resources
char const *const RUS_HELP_MSG = "\x8C\xA8\xE0\x2D\x8A\xAE\xAB\xEC\xE6\xAE\r\x8C\xA5\xE1\xE2\xEC \x8F\xA0\xE2\xE0\xA8\xA0\xE0\xE5\xA0\x14\r\x82\xA5\xE0\xE1\xA8\xEF ScummVM\r\r\x01 \x83\xAE\xE0\xEF\xE7\xA8\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xA8\rF2 - \x87\xA2\xE3\xAA\rF3 - \x82\xEB\xA9\xE2\xA8\rF4 - \x87\xA0\xAD\xAE\xA2\xAE\rF5 - \x87\xA0\xAF\xA8\xE1\xEC\rF7 - \x91\xE7\xA8\xE2\xA0\xE2\xEC\rF10 - \x8F\xA0\xE3\xA7\xA0"; // "Мир-Кольцо\rМесть Патриарха\x14\rВерсия ScummVM\r\r\x01 Горячие клавиши\rF2 - Звук\rF3 - Выйти\rF4 - Заново\rF5 - Запись\rF6 - Считать\rF7 - Пауза"
char const *const RUS_WATCH_INTRO_MSG = "    \x8F\xAE\xE1\xAC\xAE\xE2\xE0\xA5\xE2\xEC \xA2\xE1\xE2\xE3\xAF\xAB\xA5\xAD\xA8\xA5?    "; // "    Посмотреть вступление?    "
char const *const RUS_START_PLAY_BTN_STRING = " \x8D\xA0\xE7\xA0\xE2\xEC  "; // " Начать ";
char const *const RUS_INTRODUCTION_BTN_STRING = " \x82\xE1\xE2\xE3\xAF\xAB\xA5\xAD\xA8\xA5 "; // " Вступление ";
char const *const RUS_OPTIONS_MSG = "\x01\x8D\xA0\xE1\xE2\xE0\xAE\xA9\xAA\xA8"; // "\x01Настройки"

// Scene specific resources
char const *const RUS_EXIT_MSG = " \x82\xEB\xA9\xE2\xA8 "; // " Выйти "
char const *const RUS_SCENE6100_CAREFUL = "\x8E\xE1\xE2\xAE\xE0\xAE\xA6\xAD\xAE\x21 \x87\xAE\xAD\xA4 \xAC\xAE\xA6\xA5\xE2 \xAD\xA5 \xA2\xEB\xA4\xA5\xE0\xA6\xA0\xE2\xEC."; // "Осторожно! Зонд может не выдержать."
char const *const RUS_SCENE6100_TOUGHER = "\x9D\xA9\x21 \x9D\xE2\xAE \xE1\xAB\xAE\xA6\xAD\xA5\xA5, \xE7\xA5\xAC \xAA\xA0\xA6\xA5\xE2\xE1\xEF!"; // "Эй! Это сложнее, чем кажется!"
char const *const RUS_SCENE6100_ONE_MORE_HIT = "\x8D\xA5 \xA7\xA5\xA2\xA0\xA9 \xAF\xAE \xE1\xE2\xAE\xE0\xAE\xAD\xA0\xAC. \x85\xE9\xF1 \xAE\xA4\xA8\xAD \xE2\xA0\xAA\xAE\xA9 \xE3\xA4\xA0\xE0, \xA8 \xA7\xAE\xAD\xA4 \xAC\xAE\xA6\xA5\xE2 \xE0\xA0\xA7\xA2\xA0\xAB\xA8\xE2\xEC\xE1\xEF."; // "Не зевай по сторонам. Ещё один такой удар, и зонд может развалиться."
char const *const RUS_SCENE6100_DOING_BEST = "\x91\xE2\xA0\xE0\xA0\xEE\xE1\xEC. \x8D\xA0\xA4\xA5\xEE\xE1\xEC, \xED\xE2\xA0 \xE8\xE3\xE2\xAA\xA0 \xA2\xEB\xA4\xA5\xE0\xA6\xA8\xE2!"; // "Стараюсь. Надеюсь, эта шутка выдержит!"
char const *const RUS_SCENE6100_REPAIR = "\r\r\x8A\xE3\xA8\xAD \xA8 \x88\xE9\xE3\xE9\xA8\xA9 \xE7\xA8\xAD\xEF\xE2 \xA7\xAE\xAD\xA4..."; // "\r\rКуин и Ищущий чинят зонд..."
char const *const RUS_SCENE6100_ROCKY_AREA = "\x8F\xE0\xEF\xAC\xAE \xAF\xAE \xAA\xE3\xE0\xE1\xE3 \xA4\xAE\xAB\xA6\xA5\xAD \xA1\xEB\xE2\xEC \xE1\xAA\xA0\xAB\xA8\xE1\xE2\xEB\xA9 \xE3\xE7\xA0\xE1\xE2\xAE\xAA. \x82\xA8\xA4\xA8\xE8\xEC \xA5\xA3\xAE?"; // "Прямо по курсу должен быть скалистый участок. Видишь его?"
char const *const RUS_SCENE6100_REPLY = "\x84\xA0. \x8E\xE1\xE2\xA0\xAB\xAE\xE1\xEC \xAE\xA1\xEA\xA5\xE5\xA0\xE2\xEC \xAB\xE3\xE7\xA8 \xAF\xAE\xA4\xE1\xAE\xAB\xAD\xA5\xE7\xAD\xA8\xAA\xAE\xA2."; // "Да. Осталось объехать лучи подсолнечников."
char const *const RUS_SCENE6100_TAKE_CONTROLS = "\x8F\xA5\xE0\xA5\xE5\xA2\xA0\xE2\xA8\x2D\xAA\xA0 \xE3\xAF\xE0\xA0\xA2\xAB\xA5\xAD\xA8\xA5, \x88\xE9\xE3\xE9\xA8\xA9. \x93 \xAC\xA5\xAD\xEF \xE0\xE3\xAA\xA8 \xA2\xE1\xAF\xAE\xE2\xA5\xAB\xA8."; // "Перехвати-ка управление, Ищущий. У меня руки вспотели."
char const *const RUS_SCENE6100_SURPRISE = "\x92\xEB \xE3\xA4\xA8\xA2\xAB\xEF\xA5\xE8\xEC \xAC\xA5\xAD\xEF. \x8A\xE3\xA8\xAD. \x8C\xAD\xA5 \xAA\xA0\xA7\xA0\xAB\xAE\xE1\xEC, \xE2\xEB \xAD\xA5 \xE2\xA0\xAA\xAE\xA9 \xE0\xA0\xA7\xAC\xA0\xA7\xAD\xEF."; // "Ты удивляешь меня, Куин. Мне казалось, ты не такой размазня."
char const *const RUS_SCENE6100_SWEAT = "\x8B\xEE\xA4\xA8 \xAF\xAE\xE2\xA5\xEE\xE2, \xAA\xA7\xA8\xAD\xEB \xAF\xAE\xA4\xF1\xE0\xA3\xA8\xA2\xA0\xEE\xE2 \xE5\xA2\xAE\xE1\xE2\xAE\xAC. \x82 \xE7\xF1\xAC \xE0\xA0\xA7\xAD\xA8\xE6\xA0?"; // "Люди потеют, кзины подёргивают хвостом. В чём разница?"
char const *const RUS_SCENE6100_VERY_WELL = "\x8B\xA0\xA4\xAD\xAE. \x9F \xA7\xA0\xA1\xA5\xE0\xE3 \xE1\xE2\xA0\xA7\xA8\xE1\x2D\xAA\xAE\xAD\xE2\xA5\xA9\xAD\xA5\xE0 \xA8 \xA2\xA5\xE0\xAD\xE3 \xA7\xAE\xAD\xA4 \xAD\xA0 \xA1\xA0\xA7\xE3. \x86\xA4\xA8 \xA2 \xA3\xA0\xE0\xA0\xA6\xAD\xAE\xAC \xAE\xE2\xE1\xA5\xAA\xA5."; // "Ладно. Я заберу стазис-контейнер и верну зонд на базу. Жди в гаражном отсеке."

} // End of namespace Ringworld

namespace BlueForce {

// Dialog resources
char const *const HELP_MSG = "Blue Force\x14\rScummVM Version\r\r\
Keyboard shortcuts...\rF2 - Sound options\rF3 - Quit\r\
F4 - Restart\rF5 - Save game\rF7 - Restore Game\rF10 - Pause game";
char const *const WATCH_INTRO_MSG = "Do you wish to watch the introduction?";
char const *const START_PLAY_BTN_STRING = " Play ";
char const *const INTRODUCTION_BTN_STRING = " Watch ";
char const *const OPTIONS_MSG = "Options...";

// Blue Force general messages
char const *const BF_NAME = "Blue Force";
char const *const BF_COPYRIGHT = " Copyright, 1993 Tsunami Media, Inc.";
char const *const BF_ALL_RIGHTS_RESERVED = "All Rights Reserved";
char const *const BF_19840518 = "May 18, 1984";
char const *const BF_19840515 = "May 15, 1984";
char const *const BF_3_DAYS = "Three days later";
char const *const BF_11_YEARS = "Eleven years later.";
char const *const BF_NEXT_DAY = "The Next Day";
char const *const BF_ACADEMY = "Here we are at the Academy";

// Scene 50 hotspots
char const *const GRANDMA_FRANNIE = "Grandma Frannie";
char const *const MARINA = "Marina";
char const *const POLICE_DEPARTMENT = "Police Department";
char const *const TONYS_BAR = "Tony's Bar";
char const *const CHILD_PROTECTIVE_SERVICES = "Child Protective Services";
char const *const ALLEY_CAT = "Alley Cat";
char const *const CITY_HALL_JAIL = "City Hall & Jail";
char const *const JAMISON_RYAN = "Jamison & Ryan";
char const *const BIKINI_HUT = "Bikini Hut";

// Scene 60 radio messages
char const *const RADIO_BTN_LIST[8] = { "10-2 ", "10-4 ", "10-13", "10-15", "10-27", "10-35", "10-97", "10-98" };

// Scene 570 computer messageS
char const *const SCENE570_PASSWORD = "PASSWORD - }, ";
char const *const SCENE570_C_DRIVE = "C:\\";
char const *const SCENE570_RING = "RING";
char const *const SCENE570_PROTO = "PROTO";
char const *const SCENE570_WACKY = "WACKY";
char const *const SCENE570_COBB = "COBB";
char const *const SCENE570_LETTER = "LETTER";
char const *const SCENE570_RINGEXE = "RINGEXE";
char const *const SCENE570_RINGDATA = "RINGDATA";
char const *const SCENE570_PROTOEXE = "PROTOEXE";
char const *const SCENE570_PROTODATA = "PROTODATA";
char const *const SCENE570_WACKYEXE = "WACKYEXE";
char const *const SCENE570_WACKYDATA = "WACKYDATA";

// Scene 180 messages
char const *const THE_NEXT_DAY = "The Next Day";

// Spanish version
char const *const ESP_QUIT_CONFIRM_MSG = "\250Quieres dejar de jugar?";
char const *const ESP_RESTART_MSG = "\250Quieres volver a comenzar el juego?";
char const *const ESP_GAME_PAUSED_MSG = "Juego en pausa.";
char const *const ESP_QUIT_BTN_STRING = " Salir ";
char const *const ESP_RESTART_BTN_1_STRING = "Reiniciar";
char const *const ESP_RESTART_BTN_2_STRING = "Volver a empezar";
char const *const ESP_SAVE_BTN_STRING = "Guardar";
char const *const ESP_RESUME_BTN_STRING = " Seguir \rjugando";

// Dialog resources
char const *const ESP_HELP_MSG = "Blue Force\x14\rVersi\242n ScummVM\r\r\
Funciones del Teclado...\rF2 - Sonido\rF3 - Salir del Juego\r\
F4 - Recomenzar\rF5 - Guardar\rF7 - Recuperar\rF10 - Pausa";
char const *const ESP_WATCH_INTRO_MSG = "\250Quieres ver la introducci\242n?";
char const *const ESP_START_PLAY_BTN_STRING = " Jugar ";
char const *const ESP_INTRODUCTION_BTN_STRING = " Ver ";
char const *const ESP_OPTIONS_MSG = "Opciones...";

// Blue Force general messages
char const *const ESP_BF_ALL_RIGHTS_RESERVED = "Todos los derechos reservados";
char const *const ESP_BF_19840518 = "18 de Mayo de 1984";
char const *const ESP_BF_19840515 = "15 de Mayo de 1984";
char const *const ESP_BF_3_DAYS = "Tres d\241as despu\202s";
char const *const ESP_BF_11_YEARS = "Once a\244os despu\202s.";

// Scene 50 hotspots
char const *const ESP_GRANDMA_FRANNIE = "La abuela Frannie";
char const *const ESP_POLICE_DEPARTMENT = "Departamento de Polic\241a";
char const *const ESP_TONYS_BAR = "El Bar de Tony";
char const *const ESP_CHILD_PROTECTIVE_SERVICES = "Servicio de Protecci\242n de Menores";
char const *const ESP_CITY_HALL_JAIL = "Ayuntamiento & C\240rcel";

// Scene 180 messages
char const *const ESP_THE_NEXT_DAY = "Al d\241a siguiente";

// Russian version
char const *const RUS_QUIT_CONFIRM_MSG = "\x82\xEB \xE5\xAE\xE2\xA8\xE2\xA5 \xA2\xEB\xA9\xE2\xA8 \xA8\xA7 \xA8\xA3\xE0\xEB?"; // "Вы хотите выйти из игры?"
char const *const RUS_RESTART_MSG = "\x82\xEB \xE5\xAE\xE2\xA8\xE2\xA5 \xAD\xA0\xE7\xA0\xE2\xEC \xA7\xA0\xAD\xAE\xA2\xAE?";   // "Вы хотите начать заново?"
char const *const RUS_GAME_PAUSED_MSG = "  \x8F\xA0\xE3\xA7\xA0.  ";                                                          // "  Пауза.  ";
char const *const RUS_QUIT_BTN_STRING = " \x82\xEB\xA9\xE2\xA8";                                                              // " Выйти";
char const *const RUS_RESTART_BTN_STRING = "\x87\xA0\xAD\xAE\xA2\xAE";                                                        // "Заново";
char const *const RUS_SAVE_BTN_STRING = " \x87\xA0\xAF\xA8\xE1\xEC ";                                                         // " Запись ";
char const *const RUS_RESUME_BTN_STRING = " \x8D\xA0\xA7\xA0\xA4 \xA2\r\xA8\xA3\xE0\xE3";                                     // " Назад в\rигру";

// Dialog resources
char const *const RUS_HELP_MSG = "\x91\xE2\xE0\xA0\xA6\xA8 \xAF\xAE\xE0\xEF\xA4\xAA\xA0\x14\r\x82\xA5\xE0\xE1\xA8\xEF ScummVM\r\r\x01 \x83\xAE\xE0\xEF\xE7\xA8\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xA8\rF2 - \x87\xA2\xE3\xAA\rF3 - \x82\xEB\xA9\xE2\xA8\rF4 - \x87\xA0\xAD\xAE\xA2\xAE\rF5 - \x87\xA0\xAF\xA8\xE1\xEC\rF7 - \x91\xE7\xA8\xE2\xA0\xE2\xEC\rF10 - \x8F\xA0\xE3\xA7\xA0"; // Стражи порядка\x14\rВерсия ScummVM\r\r\x01 Горячие клавиши\rF2 - Звук\rF3 - Выйти\rF4 - Заново\rF5 - Запись\rF6 - Считать\rF7 - Пауза"
char const *const RUS_WATCH_INTRO_MSG = "    \x8F\xAE\xE1\xAC\xAE\xE2\xE0\xA5\xE2\xEC \xA2\xE1\xE2\xE3\xAF\xAB\xA5\xAD\xA8\xA5?    ";                                                                                                                                                                                                                                                                                          // "    Посмотреть вступление?    "
char const *const RUS_START_PLAY_BTN_STRING = " \x8D\xA0\xE7\xA0\xE2\xEC  ";                                                                                                                                                                                                                                                                                                                                                   // " Начать ";
char const *const RUS_INTRODUCTION_BTN_STRING = " \x82\xE1\xE2\xE3\xAF\xAB\xA5\xAD\xA8\xA5 ";                                                                                                                                                                                                                                                                                                                                  // " Вступление ";
char const *const RUS_OPTIONS_MSG = "\x01\x8D\xA0\xE1\xE2\xE0\xAE\xA9\xAA\xA8";                                                                                                                                                                                                                                                                                                                                                // "\x01Настройки"

// Blue Force general messages
char const *const RUS_BF_NAME = "\x91\xE2\xE0\xA0\xA6\xA8 \xAF\xAE\xE0\xEF\xA4\xAA\xA0";	// "Стражи порядка"
char const *const RUS_BF_COPYRIGHT = "\x80\xA2\xE2\xAE\xE0\xE1\xAA\xA8\xA5 \xAF\xE0\xA0\xA2\xA0, 1993 Tsunami Media, Inc.";	// "Авторские права, 1993 Tsunami Media, Inc."
char const *const RUS_BF_ALL_RIGHTS_RESERVED = "\x82\xE1\xA5 \xAF\xE0\xA0\xA2\xA0 \xA7\xA0\xE9\xA8\xE9\xA5\xAD\xEB";	// "Все права защищены"
char const *const RUS_BF_19840518 = "18 \xAC\xA0\xEF 1984 \xA3\xAE\xA4\xA0";	// "18 мая 1984 года"
char const *const RUS_BF_19840515 = "15 \xAC\xA0\xEF 1984 \xA3\xAE\xA4\xA0";	// "15 мая 1984 года"
char const *const RUS_BF_3_DAYS = "3 \xA4\xAD\xEF \xE1\xAF\xE3\xE1\xE2\xEF";	// "3 дня спустя"
char const *const RUS_BF_11_YEARS = "11 \xAB\xA5\xE2 \xE1\xAF\xE3\xE1\xE2\xEF";	// "11 лет спустя"

// Scene 50 hotspots
char const *const RUS_GRANDMA_FRANNIE = "\x81\xA0\xA1\xE3\xAB\xEF \x94\xE0\xED\xAD\xAD\xA8";	// "Бабуля Фрэнни"
char const *const RUS_MARINA = "\x8F\xE0\xA8\xE7\xA0\xAB";	// "Причал"
char const *const RUS_POLICE_DEPARTMENT = "\x93\xAF\xE0\xA0\xA2\xAB\xA5\xAD\xA8\xA5";	// "Управление"
char const *const RUS_TONYS_BAR = "\x81\xA0\xE0 \x92\xAE\xAD\xA8";	// "Бар Тони"
char const *const RUS_CHILD_PROTECTIVE_SERVICES = "\x91\xAB\xE3\xA6\xA1\xA0 \xA7\xA0\xE9\xA8\xE2\xEB \xA4\xA5\xE2\xA5\xA9";	// "Служба защиты детей"
char const *const RUS_ALLEY_CAT = "\x81\xAE\xE3\xAB\xA8\xAD\xA3";	// "Боулинг"
char const *const RUS_CITY_HALL_JAIL = "\x90\xA0\xE2\xE3\xE8\xA0 \xA8 \xE2\xEE\xE0\xEC\xAC\xA0";	// "Ратуша и тюрьма"
char const *const RUS_JAMISON_RYAN = "\x8E\xE4\xA8\xE1 \x84\xA6\xED\xAC\xA8\xE1\xAE\xAD\xA0";	// "Офис Джэмисона"
char const *const RUS_BIKINI_HUT = "\x22\x81\xA8\xAA\xA8\xAD\xA8 \x95\xA0\xE2\x22";	// ""Бикини Хат""

// Scene 570 computer messageS
char const *const RUS_SCENE570_PASSWORD = "  \xAF\xA0\xE0\xAE\xAB\xEC -> ";	//"  пароль -> "
char const *const RUS_SCENE570_C_DRIVE = "c:\\";
char const *const RUS_SCENE570_RING = "ring";
char const *const RUS_SCENE570_PROTO = "proto";
char const *const RUS_SCENE570_WACKY = "wacky";
char const *const RUS_SCENE570_COBB = "\xAA\xAE\xA1\xA1";	//"кобб"
char const *const RUS_SCENE570_LETTER = "\xAF\xA8\xE1\xEC\xAC\xAE";	//"письмо"
char const *const RUS_SCENE570_RINGEXE = "ringexe";
char const *const RUS_SCENE570_RINGDATA = "ringdata";
char const *const RUS_SCENE570_PROTOEXE = "protoexe";
char const *const RUS_SCENE570_PROTODATA = "protodata";
char const *const RUS_SCENE570_WACKYEXE = "wackyexe";
char const *const RUS_SCENE570_WACKYDATA = "wackydat ";

// Scene 180 messages
char const *const RUS_THE_NEXT_DAY = "\x93\xE2\xE0\xAE\xAC";	// "Утром"

} // End of namespace BlueForce

namespace Ringworld2 {

char const *const CONSOLE125_MESSAGES[] = {
	NULL, "Select Language", "Computer Services", "Food Services", "Entertainment Services",
	"Main Menu", "Exit Menu", "Previous Menu", "Interworld", "Hero's Tongue", "Personal Log",
	"Database", "Starchart", "Iso-Opto Disk Reader", "Eject Disk", "Meals", "Snacks",
	"Beverages", "Human Basic Snacks", "Kzin Basic Snacks", "Hot Beverages", "Cold Beverages",
	"Music", "Outpost Alpha", " ", " ", "Done", "A-G", "H-O", "P-S", "T-Z", "Tchaikovsky",
	"Mozart", "Bach", "Rossini"
};

char const *const CONSOLE325_MESSAGES[] = {
	NULL, "Select Language", "Database", "Star Chart", "Scan Ops", "Deep Scan",
	"Short Scan", "Main Menu", "Previous Menu", "Exit Menu", "Interworld", "Hero's Tongue",
	"Done", " ", " ", "Passive Enabled", "Active Enabled", "Technological", "Biological",
	"Geographical", "Astronomical", "Dipole Anomaly Sweep", "Structural Analysis",
	"A-G", "N-O", "P-S", "T-Z", "Tchaikovsky", "Mozart", "Bach", "Rossini"
};

// Scene 825 Autodoc messages
char const *const MAIN_MENU = "main menu";
char const *const DIAGNOSIS = "diagnosis";
char const *const ADVANCED_PROCEDURES = "advanced procedures";
char const *const VITAL_SIGNS = "vital signs";
char const *const OPEN_DOOR = "open door";
char const *const TREATMENTS = "treatments";
char const *const NO_MALADY_DETECTED =  "no malady detected";
char const *const NO_TREATMENT_REQUIRED = "no treatment required";
char const *const ACCESS_CODE_REQUIRED = "access code required";
char const *const INVALID_ACCESS_CODE = "invalid access code";
char const *const FOREIGN_OBJECT_EXTRACTED = "foreign object extracted";

// Scene 1330/7 Options dialog messages
char const *const AUTO_PLAY_ON = "Auto-Play is On";
char const *const AUTO_PLAY_OFF = "Auto-Play is Off";
char const *const START_NEW_CARD_GAME = "Start a new game";
char const *const QUIT_CARD_GAME = "Quit Outpost Alpha";
char const *const CONTINUE_CARD_GAME = "Continue Outpost Alpha";

//
char const *const HELP_MSG = "\x1\rRETURN TO\r RINGWORLD\x14";
char const *const CHAR_TITLE = "\x01Select Character:";
char const *const CHAR_QUINN_MSG = "  Quinn  ";
char const *const CHAR_SEEKER_MSG = "  Seeker  ";
char const *const CHAR_MIRANDA_MSG = "Miranda";
char const *const CHAR_CANCEL_MSG = "  Cancel  ";

char const *const GAME_VERSION = "ScummVM Version";
char const *const SOUND_OPTIONS = "Sound options";
char const *const QUIT_GAME = "Quit";
char const *const RESTART_GAME = "Restart";
char const *const SAVE_GAME = "Save game";
char const *const RESTORE_GAME = "Restore game";
char const *const SHOW_CREDITS = "Show credits";
char const *const PAUSE_GAME = "Pause game";
char const *const RESUME_PLAY = "  Resume play  ";
char const *const R2_RESTART_MSG = "Go to the beginning of game?";
char const *const F2 = "F2";
char const *const F3 = "F3";
char const *const F4 = "F4";
char const *const F5 = "F5";
char const *const F7 = "F7";
char const *const F8 = "F8";
char const *const F10 = "F10";

char const *const DONE_MSG = "Done";
char const *const YES_MSG = " Yes ";
char const *const NO_MSG = " No ";
char const *const USE_INTERCEPTOR = "Do you want to use your interceptor card?";
char const *const USE_DOUBLE_AGENT = "Do you want to use your double agent?";
char const *const NEED_INSTRUCTIONS = "Do you want instructions?";
char const *const WRONG_ANSWER_MSG = "Wrong respond value sent.";
char const *const BUY_FULL_GAME_MSG = "To order the complete game, RETURN TO RINGWORLD, call 1-800-482-3766!";

const byte scene1550JunkLocationsDefault[] = {
	20, 7,  41, 6,
	3,  6,  42, 11,
	10, 15, 43, 6,
	15, 1,  44, 7,
	1,  1,  2,  1,
	1,  1,  21, 12,
	5,  1,  36, 1,
	5,  1,  28, 2,
	9,  1,  10, 7,
	9,  1,  12, 10,
	19, 1,  10, 8,
	19, 1,  2,  13,
	25, 1,  31, 1,
	27, 1,  15, 6,
	27, 1,  20, 7,
	28, 1,  24, 4,
	6,  2,  22, 1,
	6,  2,  16, 5,
	12, 2,  40, 1,
	12, 2,  6,  11,
	18, 2,  21, 5,
	20, 5,  19, 4,
	20, 5,  18, 8,
	1,  6,  20, 11,
	1,  6,  18, 15,
	1,  6,  16, 4,
	7,  6,  6,  16,
	8,  6,  23, 9,
	8,  6,  38, 10,
	8,  6,  14, 13,
	8,  6,  6,  14,
	8,  6,  11, 15,
	10, 6,  3,  1,
	10, 6,  8,  2,
	10, 6,  13, 3,
	10, 6,  6,  15,
	17, 6,  4,  15,
	5,  7,  26, 11,
	25, 7,  27, 3,
	28, 7,  21, 5,
	2,  8,  23, 5,
	14, 8,  21, 5,
	14, 8,  22, 16,
	22, 8,  34, 3,
	22, 8,  24, 7,
	6,  9,  38, 5,
	6,  9,  32, 6,
	6,  9,  18, 7,
	9,  9,  34, 15,
	9,  9,  35, 16,
	18, 9,  1,  5,
	18, 9,  24, 11,
	26, 9,  21, 10,
	1,  10, 21, 9,
	1,  10, 12, 13,
	11, 10, 21, 16,
	15, 10, 8,  1,
	15, 10, 12, 6,
	15, 10, 14, 10,
	20, 10, 14, 10,
	20, 10, 39, 11,
	3,  11, 5,  9,
	3,  11, 4,  13,
	5,  11, 32, 7,
	5,  11, 20, 8,
	5,  11, 20, 11,
	7,  12, 22, 11,
	7,  12, 2,  12,
	7,  12, 23, 16,
	8,  12, 25, 9,
	13, 12, 23, 10,
	16, 12, 3,  11,
	17, 12, 25, 10,
	17, 12, 28, 15,
	25, 12, 8,  15,
	26, 12, 7,  14,
	1,  13, 8,  6,
	10, 13, 28, 11,
	21, 13, 25, 13,
	21, 13, 24, 16,
	25, 13, 25, 3,
	5,  14, 17, 6,
	5,  14, 23, 16,
	12, 14, 36, 15,
	12, 14, 17, 16,
	13, 14, 40, 13,
	13, 14, 38, 14,
	19, 14, 18, 10,
	2,  15, 4,  14,
	8,  15, 1,  13,
	12, 15, 21, 4,
	12, 15, 27, 7,
	12, 15, 28, 8,
	13, 15, 34, 1,
	13, 15, 31, 5,
	14, 15, 21, 16,
	15, 15, 29, 1,
	15, 15, 23, 13,
	25, 15, 24, 6,
	25, 15, 23, 11,
	28, 15, 8,  6,
	28, 15, 9,  10,
	1,  16, 1,  1,
	1,  16, 22, 5,
	7,  16, 3,  4,
	8,  16, 7,  1,
	11, 17, 4,  12,
	18, 17, 1,  13,
	22, 17, 21, 6,
	22, 17, 28, 15,
	27, 17, 12, 7,
	27, 17, 8,  8,
	27, 17, 14, 11,
	27, 17, 18, 15,
	27, 17, 2,  16,
	6,  18, 24, 7,
	14, 18, 21, 13,
	27, 18, 38, 4,
	28, 18, 20, 1,
	1,  18, 11, 15,
	9,  18, 7,  1,
	9,  18, 13, 12,
	16, 18, 32, 10,
	16, 18, 25, 13,
	16, 18, 31, 14,
	25, 18, 20, 7,
	28, 18, 21, 1
};

const byte scene1550AreaMap[] = {
	2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,
	16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  18,
	17, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19,
	7,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	6,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  10, 0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14
};

const byte scene1550JunkX[] = {0,  98, 135, 183, 229, 81, 133, 185, 235, 75, 131, 187, 241, 70,  129, 190, 247};
const byte scene1550JunkY[] = {0,  42, 42,  42,  42,  67, 67,  67,  67,  92, 92,  92,  92,  116, 116, 116, 116};
const byte k5A750[] = {
	9, 10, 7, 13, 7, 8, 9,  7, 9, 10,
	2, 3,  3, 2,  2, 2, 4,  3, 3, 4,
	3, 2,  3, 4,  3, 8, 10, 4, 0
};
const byte k5A76D[] = {
	3, 3,  3, 4,  3, 3, 3,  3, 1, 3,
	3, 3,  3, 7,  3, 7, 3,  3, 3, 3,
	3, 3,  3, 3,  3, 3, 3,  3, 3
};
const byte scene1550JunkRegions[] = {0,  8,  15,  16,  12,  7,  18,  17,  13,  6,  19,  20,  14,  5,   11,  10,  9};
const byte scene1550SpecialAreas[] = {
	23, 3,  1,
	23, 4,  1,
	26, 3,  1,
	26, 4,  1,
	15, 16, 2,
	2,  16, 3,
	3,  16, 4,
	10, 7,  5,
	11, 7,  6,
	3,  0,  7,
	4,  0,  8,
	24, 11, 9,
	25, 11, 10,
	12, 8,  11,
	9,  11, 12
};

const byte k5A7F6[] = {
	4,  11, 6,
	5,  14, 5,
	5,  11, 6,
	5,  7,  5,
	6,  9,  6,
	7,  16, 7,
	7,  12, 7,
	8,  6,  5,
	9,  9,  7,
	10, 13, 9,
	10, 6,  8,
	11, 10, 9,
	12, 15, 10,
	13, 12, 11,
	14, 8,  9,
	15, 16, 1,
	15, 10, 11
};

const BalloonRecord balloonData[] = {
	{ 0, -2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 0, 1 },
	{ 0, 0, 1 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 0, 2, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 0, 0, -1 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 0, -1 },
	{ 0, 0, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 0, 0 },
	{ 0, 0, -1 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 2, 0, 0 },
	{ 0, 0, -1 },
	{ -2, 0, 0 },
	{ 0, 0, -1 },
	{ -2, 0, 0 },
	{ 0, 0, -1 },
	{ 2, 0, 0 },
	{ 0, 0, -1 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 2, 0, 0 },
	{ 0, 2, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 0, 1 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, 0, 1 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, 0, -1 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, -2, 0 },
	{ 0, 0, -1 },
	{ -2, 0, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ 0, -2, 0 },
	{ 2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, 0, 0 },
	{ 2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ -2, 0, 0 },
	{ 0, -2, 0 },
	{ 0, 2, 0 },
	{ -2, 0, 0 }
};

} // End of namespace Ringworld2

} // End of namespace TsAGE
