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

#ifndef WATCHMAKER_T2D_INTERNAL_H
#define WATCHMAKER_T2D_INTERNAL_H

#include "watchmaker/types.h"

#define T2D_MAX_WINDOWS                                     32

#define T2D_MAX_BITMAPS_IN_WIN                              64
#define T2D_MAX_TEXTS_IN_WIN                                64
#define T2D_MAX_BUTTONS_IN_WIN                              64

#define T2D_BM_OFF                                          (1<<31)
#define T2D_BM_SCROLL                                       (1<<30)
#define T2D_BM2TEXT_MASK                                    0x3F000000
#define T2D_BM2TEXT_SHIFT                                   24

#define T2D_WIN_SCANNER_BACKGROUND                          1
#define T2D_BM_SCANNER_BACKGROUND_WIN                       0
#define T2D_BM_SCANNER_BACKGROUND_EXIT                      1
#define T2D_BM_SCANNER_BACKGROUND_PIC_LOW                   2
#define T2D_BM_SCANNER_BACKGROUND_PIC_HI                    3
#define T2D_BM_SCANNER_BACKGROUND_SELECTOR                  4
#define T2D_BT_SCANNER_BACKGROUND_EXIT                      1
#define T2D_BT_SCANNER_BACKGROUND_SELECTOR                  2

#define T2D_WIN_SCANNER_TOOLBAR                             2
#define T2D_BM_SCANNER_TOOLBAR_WIN                          0
#define T2D_BM_SCANNER_TOOLBAR_EXIT                         1
#define T2D_BM_SCANNER_TOOLBAR_PREVIEW_FREEZE               2
#define T2D_BM_SCANNER_TOOLBAR_PREVIEW_OFF                  3
#define T2D_BM_SCANNER_TOOLBAR_PREVIEW_ON                   4
#define T2D_BM_SCANNER_TOOLBAR_SELECTAREA_FREEZE            5
#define T2D_BM_SCANNER_TOOLBAR_SELECTAREA_OFF               6
#define T2D_BM_SCANNER_TOOLBAR_SELECTAREA_ON                7
#define T2D_BM_SCANNER_TOOLBAR_SCAN_FREEZE                  8
#define T2D_BM_SCANNER_TOOLBAR_SCAN_OFF                     9
#define T2D_BM_SCANNER_TOOLBAR_SCAN_ON                      10
#define T2D_BT_SCANNER_TOOLBAR_MOVE                         1
#define T2D_BT_SCANNER_TOOLBAR_EXIT                         2
#define T2D_BT_SCANNER_TOOLBAR_PREVIEW_OFF                  3
#define T2D_BT_SCANNER_TOOLBAR_SELECTAREA_OFF               4
#define T2D_BT_SCANNER_TOOLBAR_SCAN_OFF                     5

#define T2D_WIN_SCANNER_PROGRESSBAR                         3
#define T2D_BM_SCANNER_PROGRESSBAR_WIN                      0
#define T2D_BM_SCANNER_PROGRESSBAR_EXIT                     1
#define T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_OFF             2
#define T2D_BM_SCANNER_PROGRESSBAR_PROGRESS_ON              3
#define T2D_BM_SCANNER_PROGRESSBAR_SCANNING_OFF             4
#define T2D_BM_SCANNER_PROGRESSBAR_SCANNING_ON              5

#define T2D_WIN_COMPUTER_BACKGROUND                         4
#define T2D_BM_COMPUTER_BACKGROUND_WIN                      0
#define T2D_BM_COMPUTER_EMAIL_ON                            1
#define T2D_BM_COMPUTER_SEARCH_ON                           2
#define T2D_BM_COMPUTER_QUIT_ON                             3
#define T2D_BM_COMPUTER_SCRITTE_1                           4
#define T2D_BM_COMPUTER_SCRITTE_2                           5
#define T2D_BM_COMPUTER_SCRITTE_3                           6
#define T2D_BM_COMPUTER_SCRITTE_4                           7
#define T2D_BT_COMPUTER_BACKGROUND_EMAIL                    1
#define T2D_BT_COMPUTER_BACKGROUND_SEARCH                   2
#define T2D_BT_COMPUTER_BACKGROUND_QUIT                     3
#define T2D_BT_BackgroundWin_TGA__cia1                      4
#define T2D_BT_BackgroundWin_TGA__cia2                      5

#define T2D_TEXT_COMPUTER_SCRITTE_1                         1
#define T2D_TEXT_COMPUTER_SCRITTE_2                         2
#define T2D_TEXT_COMPUTER_SCRITTE_3                         3
#define T2D_TEXT_COMPUTER_SCRITTE_4                         4

#define T2D_WIN_COMPUTER_ERROR                              5
#define T2D_BM_COMPUTER_COMERROR_WIN                        0
#define T2D_BM_COMPUTER_DIALERROR_WIN                       1
#define T2D_BM_COMPUTER_SEARCHERROR_WIN                     2
#define T2D_BM_COMPUTER_ERROR_COMQUIT_ON                    3
#define T2D_BM_COMPUTER_ERROR_DIALQUIT_ON                   4
#define T2D_BM_COMPUTER_ERROR_DIALUP_ON                     5
#define T2D_BM_COMPUTER_ERROR_OK_ON                         6
#define T2D_BT_COMPUTER_ERROR_WIN                           1
#define T2D_BT_COMPUTER_ERROR_MOVE                          2
#define T2D_BT_COMPUTER_ERROR_COMQUIT                       3
#define T2D_BT_COMPUTER_ERROR_DIALUP                        4
#define T2D_BT_COMPUTER_ERROR_DIALQUIT                  5
#define T2D_BT_COMPUTER_ERROR_OK                                6
#define T2D_BT_ComError_Win_TGA                                 7
#define T2D_BT_DialError_Win_TGA                                8
#define T2D_BT_SearchError_Win_TGA                          9

#define T2D_WIN_COMPUTER_DIALUP                             7
#define T2D_BM_COMPUTER_DIALUP_WIN                          0
#define T2D_BM_COMPUTER_DIALUP_1_ON                         1
#define T2D_BM_COMPUTER_DIALUP_2_ON                         2
#define T2D_BM_COMPUTER_DIALUP_3_ON                         3
#define T2D_BM_COMPUTER_DIALUP_4_ON                         4
#define T2D_BM_COMPUTER_DIALUP_5_ON                         5
#define T2D_BM_COMPUTER_DIALUP_6_ON                         6
#define T2D_BM_COMPUTER_DIALUP_7_ON                         7
#define T2D_BM_COMPUTER_DIALUP_8_ON                         8
#define T2D_BM_COMPUTER_DIALUP_9_ON                         9
#define T2D_BM_COMPUTER_DIALUP_0_ON                         10
#define T2D_BM_COMPUTER_DIALUP_ENTER_ON                     11
#define T2D_BM_COMPUTER_DIALUP_C_ON                         12
#define T2D_BM_COMPUTER_DIALUP_OK_ON                        13
#define T2D_BM_COMPUTER_DIALUP_EXIT_ON                      14
#define T2D_BM_COMPUTER_DIALUP_NUMBER                       15
#define T2D_BT_COMPUTER_DIALUP_MOVE                         1
#define T2D_BT_COMPUTER_DIALUP_1                            2
#define T2D_BT_COMPUTER_DIALUP_2                            3
#define T2D_BT_COMPUTER_DIALUP_3                            4
#define T2D_BT_COMPUTER_DIALUP_4                            5
#define T2D_BT_COMPUTER_DIALUP_5                            6
#define T2D_BT_COMPUTER_DIALUP_6                            7
#define T2D_BT_COMPUTER_DIALUP_7                            8
#define T2D_BT_COMPUTER_DIALUP_8                            9
#define T2D_BT_COMPUTER_DIALUP_9                            10
#define T2D_BT_COMPUTER_DIALUP_0                            11
#define T2D_BT_COMPUTER_DIALUP_ENTER                        12
#define T2D_BT_COMPUTER_DIALUP_C                            13
#define T2D_BT_COMPUTER_DIALUP_OK                           14
#define T2D_BT_COMPUTER_DIALUP_EXIT                         15
#define T2D_BT_DialUpWin_TGA__Dial                              16
#define T2D_BT_DialUpWin_TGA__Enter                             17
#define T2D_BT_DialUpWin_TGA__Dialling                      18

#define T2D_TEXT_COMPUTER_DIALUP_NUMBER                     1

#define T2D_WIN_COMPUTER_SEARCH                             8
#define T2D_BM_COMPUTER_SEARCH_ALPHABETICAL_WIN             0
#define T2D_BM_COMPUTER_SEARCH_CASENAME_WIN                 1
#define T2D_BM_COMPUTER_SEARCH_SEARCH_ON                    2
#define T2D_BM_COMPUTER_SEARCH_EXIT_ON                      3
#define T2D_BM_COMPUTER_SEARCH_NAME_ON                      4
#define T2D_BM_COMPUTER_SEARCH_SURNAME_ON                   5
#define T2D_BM_COMPUTER_SEARCH_CASENAME_ON                  6
#define T2D_BM_COMPUTER_SEARCH_YEAR_ON                      7
#define T2D_BM_COMPUTER_SEARCH_NAME                         8
#define T2D_BM_COMPUTER_SEARCH_SURNAME                      9
#define T2D_BM_COMPUTER_SEARCH_CASENAME                     10
#define T2D_BM_COMPUTER_SEARCH_YEAR                         11
#define T2D_BT_COMPUTER_SEARCH_WIN                          1
#define T2D_BT_COMPUTER_SEARCH_MOVE                         2
#define T2D_BT_COMPUTER_SEARCH_SEARCH                       3
#define T2D_BT_COMPUTER_SEARCH_ALPHABETICAL                 4
#define T2D_BT_COMPUTER_SEARCH_CASENAME                     5
#define T2D_BT_COMPUTER_SEARCH_NAME_TEXT                    6
#define T2D_BT_COMPUTER_SEARCH_SURNAME_TEXT                 7
#define T2D_BT_COMPUTER_SEARCH_CASENAME_TEXT                8
#define T2D_BT_COMPUTER_SEARCH_YEAR_TEXT                    9
#define T2D_BT_COMPUTER_SEARCH_EXIT                         10
#define T2D_BT_Search_Alphabetical_TGA__search      11
#define T2D_BT_Search_Alphabetical_TGA__findby      12
#define T2D_BT_Search_Alphabetical_TGA__name            13
#define T2D_BT_Search_Alphabetical_TGA__surname     14
#define T2D_BT_Search_Alphabetical_TGA__subject     15
#define T2D_BT_Search_Alphabetical_TGA__casename    16
#define T2D_BT_Search_Alphabetical_TGA__year            17

#define T2D_TEXT_COMPUTER_SEARCH_NAME                       1
#define T2D_TEXT_COMPUTER_SEARCH_SURNAME                    2
#define T2D_TEXT_COMPUTER_SEARCH_CASENAME                   3
#define T2D_TEXT_COMPUTER_SEARCH_YEAR                       4

#define T2D_WIN_COMPUTER_EMAIL                              9
#define T2D_BM_COMPUTER_EMAIL_WIN                           0
#define T2D_BM_COMPUTER_EMAIL_EXIT_ON                       1
#define T2D_BM_COMPUTER_EMAIL_ICON_1                        2
#define T2D_BM_COMPUTER_EMAIL_ICON_2                        3
#define T2D_BT_COMPUTER_EMAIL_WIN                           1
#define T2D_BT_COMPUTER_EMAIL_MOVE                          2
#define T2D_BT_COMPUTER_EMAIL_EXIT                          3
#define T2D_BT_COMPUTER_EMAIL_SCROLLUP                      4
#define T2D_BT_COMPUTER_EMAIL_SCROLLDOWN                    5
#define T2D_BT_COMPUTER_EMAIL_ICON_1                        6
#define T2D_BT_COMPUTER_EMAIL_ICON_2                        7
#define T2D_BT_EMail_Win_TGA__email                     8
#define T2D_BT_EMail_Win_TGA__avail                     9

#define T2D_WIN_COMPUTER_DOCUMENT                           10
#define T2D_BM_COMPUTER_DOCUMENT_WIN                        0
#define T2D_BM_COMPUTER_DOCUMENT_SCROLLUP_ON                1
#define T2D_BM_COMPUTER_DOCUMENT_SCROLLDOWN_ON              2
#define T2D_BM_COMPUTER_DOCUMENT_EXIT_ON                    3
#define T2D_BM_COMPUTER_DOCUMENT_FIRST                          4
#define T2D_BM_COMPUTER_DOCUMENT_ANTONIOCORONA              4
#define T2D_BM_COMPUTER_DOCUMENT_JACOBKRENN                 5
#define T2D_BM_COMPUTER_DOCUMENT_PETERDUKES                 6
#define T2D_BM_COMPUTER_DOCUMENT_VICTORVALENCIA             7
#define T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_1             8
#define T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_2             9
#define T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_3             10
#define T2D_BM_COMPUTER_DOCUMENT_MERCURY_1969_4             11
#define T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_1          12
#define T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_2          13
#define T2D_BM_COMPUTER_DOCUMENT_CONFIDENCE_1942_3          14
#define T2D_BM_COMPUTER_DOCUMENT_ARL_1998_1                 15
#define T2D_BM_COMPUTER_DOCUMENT_CONV_CONFIDENCE            16
#define T2D_BM_COMPUTER_DOCUMENT_CONV_MERCURY               17
#define T2D_BM_COMPUTER_DOCUMENT_GREGOR_MEM                 18
#define T2D_BM_COMPUTER_DOCUMENT_BENNA                      19
#define T2D_BM_COMPUTER_DOCUMENT_ANDREA                     20
#define T2D_BM_COMPUTER_DOCUMENT_BELLONDS                   21
#define T2D_BM_COMPUTER_DOCUMENT_DANIELE                    22
#define T2D_BM_COMPUTER_DOCUMENT_EDO                        23
#define T2D_BM_COMPUTER_DOCUMENT_FEDE                       24
#define T2D_BM_COMPUTER_DOCUMENT_FIABE                      25
#define T2D_BM_COMPUTER_DOCUMENT_FOX                        26
#define T2D_BM_COMPUTER_DOCUMENT_GIUSTI                     27
#define T2D_BM_COMPUTER_DOCUMENT_KRIZIA                     28
#define T2D_BM_COMPUTER_DOCUMENT_NONJI                      29
#define T2D_BM_COMPUTER_DOCUMENT_SPARTECO                   30
#define T2D_BM_COMPUTER_DOCUMENT_TAG                        31
#define T2D_BM_COMPUTER_DOCUMENT_TIZ                        32
#define T2D_BM_COMPUTER_DOCUMENT_WALTER                     33
#define T2D_BM_COMPUTER_DOCUMENT_VICKY                      34
#define T2D_BM_COMPUTER_DOCUMENT_PIETRO                     35
#define T2D_BM_COMPUTER_DOCUMENT_LAST                           35
#define T2D_BT_COMPUTER_DOCUMENT_WIN                        1
#define T2D_BT_COMPUTER_DOCUMENT_MOVE                       2
#define T2D_BT_COMPUTER_DOCUMENT_SCROLL                     3
#define T2D_BT_COMPUTER_DOCUMENT_EXIT                       4
#define T2D_BT_COMPUTER_DOCUMENT_SCROLLUP                   5
#define T2D_BT_COMPUTER_DOCUMENT_SCROLLDOWN                 6
#define T2D_BT_Document_Win_TGA                                         7

#define T2D_WIN_COMPUTER_DOCLIST                            11
#define T2D_BM_COMPUTER_DOCLIST_WIN                         0
#define T2D_BM_COMPUTER_DOCLIST_EXIT_ON                     1
#define T2D_BM_COMPUTER_DOCLIST_SCROLLUP_ON                 2
#define T2D_BM_COMPUTER_DOCLIST_SCROLLDOWN_ON               3
#define T2D_BM_COMPUTER_DOCLIST_LINES_START                 4
#define T2D_BM_COMPUTER_DOCLIST_LINES_END                   7
#define T2D_BT_COMPUTER_DOCLIST_WIN                         1
#define T2D_BT_COMPUTER_DOCLIST_MOVE                        2
#define T2D_BT_COMPUTER_DOCLIST_EXIT                        3
#define T2D_BT_COMPUTER_DOCLIST_SCROLLUP                    4
#define T2D_BT_COMPUTER_DOCLIST_SCROLLDOWN                  5
#define T2D_BT_COMPUTER_DOCLIST_LINES_START                 6
#define T2D_BT_COMPUTER_DOCLIST_LINES_END                   12
#define T2D_BT_DocList_Win_TGA                                  13

#define T2D_TEXT_COMPUTER_DOCLIST_LINES_START               1
#define T2D_TEXT_COMPUTER_DOCLIST_LINES_END                 7


#define T2D_WIN_COMPUTER_EMAIL_VIEWER                       12
#define T2D_BM_COMPUTER_EMAIL_VIEWER_WIN                    0
#define T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLUP_ON            1
#define T2D_BM_COMPUTER_EMAIL_VIEWER_SCROLLDOWN_ON          2
#define T2D_BM_COMPUTER_EMAIL_VIEWER_EXIT_ON                3
#define T2D_BM_COMPUTER_EMAIL_1                             4
#define T2D_BM_COMPUTER_EMAIL_2                             5
#define T2D_BM_COMPUTER_EMAIL_TEXT_1                        6
#define T2D_BM_COMPUTER_EMAIL_TEXT_2                        7
#define T2D_BT_COMPUTER_EMAIL_VIEWER_WIN                    1
#define T2D_BT_COMPUTER_EMAIL_VIEWER_MOVE                   2
#define T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLL                 3
#define T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLLUP               4
#define T2D_BT_COMPUTER_EMAIL_VIEWER_SCROLLDOWN             5
#define T2D_BT_COMPUTER_EMAIL_VIEWER_EXIT                   6
#define T2D_BT_EMail_Viewer_Win_TGA                 7

#define T2D_TEXT_COMPUTER_EMAIL_1                           1
#define T2D_TEXT_COMPUTER_EMAIL_2                           2

#define T2D_WIN_DIARIO                                      13
#define T2D_BM_DIARIO_PAGE_1                                0
#define T2D_BM_DIARIO_PAGE_2                                1
#define T2D_BM_DIARIO_PAGE_3                                2
#define T2D_BM_DIARIO_PAGE_4                                3
#define T2D_BM_DIARIO_PAGE_LEFT_ON                          4
#define T2D_BM_DIARIO_PAGE_RIGHT_ON                         5
#define T2D_BM_DIARIO_EXIT_ON                               6
#define T2D_BM_DIARIO_PAGE_1_TEXT_1                         11
#define T2D_BM_DIARIO_PAGE_1_TEXT_2                         12
#define T2D_BM_DIARIO_PAGE_1_TEXT_3                         13
#define T2D_BM_DIARIO_PAGE_1_TEXT_4                         14
#define T2D_BM_DIARIO_PAGE_1_TEXT_5                         15
#define T2D_BM_DIARIO_PAGE_1_TEXT_6                         16
#define T2D_BM_DIARIO_PAGE_2_TEXT_1                         21
#define T2D_BM_DIARIO_PAGE_2_TEXT_2                         22
#define T2D_BM_DIARIO_PAGE_2_TEXT_3                         23
#define T2D_BM_DIARIO_PAGE_2_TEXT_4                         24
#define T2D_BM_DIARIO_PAGE_2_TEXT_5                         25
#define T2D_BM_DIARIO_PAGE_2_TEXT_6                         26
#define T2D_BM_DIARIO_PAGE_3_TEXT_1                         31
#define T2D_BM_DIARIO_PAGE_3_TEXT_2                         32
#define T2D_BM_DIARIO_PAGE_3_TEXT_3                         33
#define T2D_BM_DIARIO_PAGE_3_TEXT_4                         34
#define T2D_BM_DIARIO_PAGE_4_TEXT_1                         41
#define T2D_BM_DIARIO_PAGE_4_TEXT_2                         42
#define T2D_BM_DIARIO_PAGE_4_TEXT_3                         43
#define T2D_BM_DIARIO_PAGE_4_TEXT_4                         44
#define T2D_BM_DIARIO_PAGE_4_TEXT_5                         45
#define T2D_BM_DIARIO_PAGE_4_TEXT_6                         46
#define T2D_BM_DIARIO_PAGE_4_TEXT_7                         47
#define T2D_BM_DIARIO_PAGE_4_TEXT_8                         48
#define T2D_BM_DIARIO_PAGE_4_TEXT_9                         49
#define T2D_BM_DIARIO_PAGE_4_TEXT_10                        50
#define T2D_BM_DIARIO_PAGE_4_TEXT_11                        51
#define T2D_BM_DIARIO_PAGE_4_TEXT_12                        52
#define T2D_BT_DIARIO_PAGE_LEFT                             1
#define T2D_BT_DIARIO_PAGE_RIGHT                            2
#define T2D_BT_DIARIO_EXIT                                  3
#define T2D_BT_DIARIO_PAGE_1_TEXT_1                         11
#define T2D_BT_DIARIO_PAGE_1_TEXT_2                         12
#define T2D_BT_DIARIO_PAGE_1_TEXT_3                         13
#define T2D_BT_DIARIO_PAGE_1_TEXT_4                         14
#define T2D_BT_DIARIO_PAGE_1_TEXT_5                         15
#define T2D_BT_DIARIO_PAGE_1_TEXT_6                         16
#define T2D_BT_DIARIO_PAGE_2_TEXT_1                         21
#define T2D_BT_DIARIO_PAGE_2_TEXT_2                         22
#define T2D_BT_DIARIO_PAGE_2_TEXT_3                         23
#define T2D_BT_DIARIO_PAGE_2_TEXT_4                         24
#define T2D_BT_DIARIO_PAGE_2_TEXT_5                         25
#define T2D_BT_DIARIO_PAGE_2_TEXT_6                         26
#define T2D_BT_DIARIO_PAGE_3_TEXT_1                         31
#define T2D_BT_DIARIO_PAGE_3_TEXT_2                         32
#define T2D_BT_DIARIO_PAGE_3_TEXT_3                         33
#define T2D_BT_DIARIO_PAGE_3_TEXT_4                         34
#define T2D_BT_DIARIO_PAGE_4_TEXT_1                         41
#define T2D_BT_DIARIO_PAGE_4_TEXT_2                         42
#define T2D_BT_DIARIO_PAGE_4_TEXT_3                         43
#define T2D_BT_DIARIO_PAGE_4_TEXT_4                         44
#define T2D_BT_DIARIO_PAGE_4_TEXT_5                         45
#define T2D_BT_DIARIO_PAGE_4_TEXT_6                         46
#define T2D_BT_DIARIO_PAGE_4_TEXT_7                         47
#define T2D_BT_DIARIO_PAGE_4_TEXT_8                         48
#define T2D_BT_DIARIO_PAGE_4_TEXT_9                         49
#define T2D_BT_DIARIO_PAGE_4_TEXT_10                        50
#define T2D_BT_DIARIO_PAGE_4_TEXT_11                        51
#define T2D_BT_DIARIO_PAGE_4_TEXT_12                        52

#define T2D_WIN_PDA_MAIN                                    14
#define T2D_BM_PDA_MAIN_WIN                                 0
#define T2D_BM_PDA_MAIN_TONE_ON                             1
#define T2D_BM_PDA_MAIN_LOG_ON                              2
#define T2D_BM_PDA_MAIN_TIME                                3
#define T2D_BM_PDA_MAIN_QUIT_ON                             4
#define T2D_BT_PDA_MAIN_TONE                                1
#define T2D_BT_PDA_MAIN_LOG                                 2
#define T2D_BT_PDA_MAIN_QUIT                                3

#define T2D_TEXT_PDA_MAIN_TIME                              1

#define T2D_WIN_PDA_LOG                                     15
#define T2D_BM_PDA_LOG_WIN                                  0
#define T2D_BM_PDA_LOG_SCROLLUP_ON                          1
#define T2D_BM_PDA_LOG_SCROLLDOWN_ON                        2
#define T2D_BM_PDA_LOG_SB                                   3
#define T2D_BM_PDA_LOG_EXIT_ON                              4
#define T2D_BM_PDA_LOG_DATA_START                           5
#define T2D_BM_PDA_LOG_DATA_END                             25
#define T2D_BM_PDA_LOG_TEXT_START                           26
#define T2D_BM_PDA_LOG_TEXT_END                             46
#define T2D_BT_PDA_LOG_WIN                                  1
#define T2D_BT_PDA_LOG_SCROLL                               2
#define T2D_BT_PDA_LOG_SCROLLUP                             3
#define T2D_BT_PDA_LOG_SCROLLDOWN                           4
#define T2D_BT_PDA_LOG_BARRA                                5
#define T2D_BT_PDA_LOG_SB                                   6
#define T2D_BT_PDA_LOG_EXIT                                 7
#define T2D_BT_PDA_Log_Win_TGA                          8

#define T2D_TEXT_PDA_LOG_DATA_START                         1
#define T2D_TEXT_PDA_LOG_DATA_END                           21
#define T2D_TEXT_PDA_LOG_TEXT_START                         22
#define T2D_TEXT_PDA_LOG_TEXT_END                           42

#define T2D_WIN_PDA_TONE                                    16
#define T2D_BM_PDA_TONE_WIN                                 0
#define T2D_BM_PDA_TONE_HELP_ON                             1
#define T2D_BM_PDA_TONE_EXIT_ON                             2
#define T2D_BM_PDA_TONE_WAVE_1                              3
#define T2D_BM_PDA_TONE_WAVE_2                              4
#define T2D_BM_PDA_TONE_REDLINE                             5
#define T2D_BM_PDA_TONE_BARRA_1                             6
#define T2D_BM_PDA_TONE_BARRA_2                             7
#define T2D_BM_PDA_TONE_ACQUIRE_ON                          8
#define T2D_BM_PDA_TONE_PROCESS_ON                          9
#define T2D_BM_PDA_TONE_RISULTATO                           10
#define T2D_BT_PDA_TONE_WIN                                 1
#define T2D_BT_PDA_TONE_HELP                                2
#define T2D_BT_PDA_TONE_EXIT                                3
#define T2D_BT_PDA_TONE_ACQUIRE                             4
#define T2D_BT_PDA_TONE_PROCESS                             5
#define T2D_BT_PDA_Tone_Win_TGA                         6

//#define T2D_TEXT_PDA_TONE_RISULTATO                           1

#define T2D_WIN_PDA_HELP                                    17
#define T2D_BM_PDA_HELP_WIN                                 0
#define T2D_BM_PDA_HELP_WIN2                                1
#define T2D_BM_PDA_HELP_SCROLLUP_ON                         2
#define T2D_BM_PDA_HELP_SCROLLDOWN_ON                       3
#define T2D_BM_PDA_HELP_EXIT_ON                             4
#define T2D_BM_PDA_HELP_TEXT_START                          5
#define T2D_BM_PDA_HELP_TEXT_END                            15
#define T2D_BT_PDA_HELP_WIN                                 1
#define T2D_BT_PDA_HELP_MOVE                                2
#define T2D_BT_PDA_HELP_SCROLL                              3
#define T2D_BT_PDA_HELP_SCROLLUP                            4
#define T2D_BT_PDA_HELP_SCROLLDOWN                          5
#define T2D_BT_PDA_HELP_EXIT                                6
#define T2D_BT_Help_Win_TGA                                     7

#define T2D_TEXT_PDA_HELP_START                             1
#define T2D_TEXT_PDA_HELP_END                               11

#define T2D_WIN_OPTIONS_MAIN                                18
#define T2D_BM_OPTIONS_WIN                                  1
#define T2D_BM_OPTIONS_CONTORNO                             2
#define T2D_BM_OPTIONS_SAVE                                 3
#define T2D_BM_OPTIONS_LOAD                                 4
#define T2D_BM_OPTIONS_OPTIONS                              5
#define T2D_BM_OPTIONS_GRIGLIA                              6
#define T2D_BM_OPTIONS_FRECCIA_SU_OFF                       7
#define T2D_BM_OPTIONS_FRECCIA_SU_ON                        8
#define T2D_BM_OPTIONS_FRECCIA_GIU_OFF                      9
#define T2D_BM_OPTIONS_FRECCIA_GIU_ON                       10
#define T2D_BM_OPTIONS_SAVE_START                           11
#define T2D_BM_OPTIONS_SAVE_END                             T2D_BM_OPTIONS_SAVE_START+8
#define T2D_BM_OPTIONS_TEXT_SAVE_START                      20
#define T2D_BM_OPTIONS_TEXT_SAVE_END                        T2D_BM_OPTIONS_TEXT_SAVE_START+8
#define T2D_BM_OPTIONS_SOUND_ON                             29
#define T2D_BM_OPTIONS_MUSIC_ON                             30
#define T2D_BM_OPTIONS_SPEECH_ON                            31
#define T2D_BM_OPTIONS_SUBTITLES_ON                         32
#define T2D_BM_OPTIONS_SOUND_OFF                            33
#define T2D_BM_OPTIONS_MUSIC_OFF                            34
#define T2D_BM_OPTIONS_SPEECH_OFF                           35
#define T2D_BM_OPTIONS_SUBTITLES_OFF                        36
#define T2D_BM_OPTIONS_SOUND_BARRA                          37
#define T2D_BM_OPTIONS_SOUND_PIROLO                         38
#define T2D_BM_OPTIONS_MUSIC_BARRA                          39
#define T2D_BM_OPTIONS_MUSIC_PIROLO                         40
#define T2D_BM_OPTIONS_SPEECH_BARRA                         41
#define T2D_BM_OPTIONS_SPEECH_PIROLO                        42
#define T2D_BM_OPTIONS_MAINMENU_OFF                         43
#define T2D_BM_OPTIONS_MAINMENU_ON                          44
#define T2D_BT_OPTIONS_SAVE                                 1
#define T2D_BT_OPTIONS_LOAD                                 2
#define T2D_BT_OPTIONS_OPTIONS                              3
#define T2D_BT_OPTIONS_QUIT                                 4
#define T2D_BT_OPTIONS_CLOSE                                5
#define T2D_BT_OPTIONS_FRECCIA_SU                           6
#define T2D_BT_OPTIONS_FRECCIA_GIU                          7
#define T2D_BT_OPTIONS_SAVE_START                           8
#define T2D_BT_OPTIONS_SAVE_END                             T2D_BT_OPTIONS_SAVE_START+8
#define T2D_BT_OPTIONS_SOUND                                17
#define T2D_BT_OPTIONS_MUSIC                                18
#define T2D_BT_OPTIONS_SPEECH                               19
#define T2D_BT_OPTIONS_SUBTITLES                            20
#define T2D_BT_OPTIONS_SOUND_BARRA                          21
#define T2D_BT_OPTIONS_MUSIC_BARRA                          22
#define T2D_BT_OPTIONS_SPEECH_BARRA                         23
#define T2D_BT_OPTIONS_MAINMENU                             24

#define T2D_TEXT_OPTIONS_SAVE_START                         1
#define T2D_TEXT_OPTIONS_SAVE_END                           T2D_TEXT_OPTIONS_SAVE_START+1

#define T2D_WIN_MAINMENU_MAIN                               19
#define T2D_BM_MAINMENU_WIN                                 1
#define T2D_BM_MAINMENU_PLAY_ON                             2
#define T2D_BM_MAINMENU_PLAY_OFF                            3
#define T2D_BM_MAINMENU_LOAD_ON                             4
#define T2D_BM_MAINMENU_LOAD_OFF                            5
#define T2D_BM_MAINMENU_EXIT_ON                             6
#define T2D_BM_MAINMENU_EXIT_OFF                            7
#define T2D_BT_MAINMENU_PLAY                                1
#define T2D_BT_MAINMENU_LOAD                                2
#define T2D_BT_MAINMENU_EXIT                                3

#define T2D_WIN_GAMEOVER_MAIN                               20
#define T2D_BM_GAMEOVER_WIN                                 1
#define T2D_BT_GameOver_Win_TGA                     1

namespace Watchmaker {

struct t2dBUTTON {
	uint8 on;                                                                                   // on/off
	struct SRect lim;                                                                           // limiti del bottone
	int32 tnum;                                                                                 // numero bitmap col tooltip

	t2dBUTTON() {
		reset();
	}

	void reset() {
		on = 0;
		lim.reset();
		tnum = 0;
	}
};

struct t2dWINDOW {
	int32 px, py;                                                                               // poszione finestra
	int32 sy;                                                                                   // scroll interno finestra
	struct SDDBitmap bm[T2D_MAX_BITMAPS_IN_WIN];                                                // bitmaps ordinate
	struct SDDText text[T2D_MAX_TEXTS_IN_WIN];                                                  // testi
	t2dBUTTON bt[T2D_MAX_BUTTONS_IN_WIN];                                                       // bottoni ordinati
	bool NOTSTRETCHED;

	t2dWINDOW() {
		reset();
	}

	void reset() {
		px = 0;
		py = 0;
		sy = 0;
		for (uint i = 0; i < ARRAYSIZE(bm); i++) bm[i].reset();
		for (uint i = 0; i < ARRAYSIZE(text); i++) text[i].reset();
		for (uint i = 0; i < ARRAYSIZE(bt); i++) bt[i].reset();
		NOTSTRETCHED = false;
	}
};

struct PDALogS {
	int PDALogInd = 0;
	int IndentX = 0;
};

struct SavesS {
	int NFile;

	int Hour;
	int Min;
	int Sec;
	int Day;
	int Month;
	int Year;
};

void ChangeExclusiveMouseMode();

void PDA_Tone_Acquire_Help_Func();
void SetHelpWin(Fonts &fonts, void (*Func)(void), int NArg, ...);
void CaricaSaves(WGame &game);
void SortSaves();
void RefreshSaveImg(WGame &game, int Pos, uint8 Type);
void FormattaLogs(WGame &game);

} // End of namespace Watchmaker

#endif // WATCHMAKER_T2D_INTERNAL_H
