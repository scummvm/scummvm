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

#ifndef HODJNPODJ_RIDDLES_H
#define HODJNPODJ_RIDDLES_H

#include "bagel/afxwin.h"
//#include <afxext.h>
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include <errors.h>
#include <sprite.h>
#include <text.h>
#include <button.h>
#include <sound.h>

namespace Bagel {
namespace HodjNPodj {
namespace Riddles {

// Easter egg parameters
#define	FISH_X				13
#define	FISH_Y              40
#define	NUM_FISH_CELS		25

#define	SKIER_X				6
#define	SKIER_Y             98
#define	NUM_SKIER_CELS		14

#define	NESSIE_X			17
#define	NESSIE_Y            24
#define	NUM_NESSIE_CELS     27

#define ANIM_X 				5
#define	ANIM_Y              24
#define	ANIM_DX             211
#define ANIM_DY             173
#define	ANIM_SLEEP			100			// In milliseconds

#define COLUMN_1_X			213
#define	COLUMN_1_Y			25
#define	COLUMN_1_DX			38
#define COLUMN_1_DY			195

#define COLUMN_2_X			332
#define	COLUMN_2_Y			25
#define	COLUMN_2_DX			50
#define COLUMN_2_DY			187

#define COLUMN_3_X			397
#define	COLUMN_3_Y			25
#define	COLUMN_3_DX			40
#define COLUMN_3_DY			197

#define COLUMN_4_X			451
#define	COLUMN_4_Y			25
#define	COLUMN_4_DX			63
#define COLUMN_4_DY			227

#define COLUMN_5_X			544
#define	COLUMN_5_Y			25
#define	COLUMN_5_DX			53
#define COLUMN_5_DY			38

#define COLUMN_6_X			560
#define	COLUMN_6_Y			63
#define	COLUMN_6_DX			63
#define COLUMN_6_DY			59

#define COLUMN_7_X			584
#define	COLUMN_7_Y			122
#define	COLUMN_7_DX			40
#define COLUMN_7_DY			88

#define COLUMN_WAV			".\\SOUND\\COLUMN.WAV"

#define MAX_RIDDLE_LENGTH  159
#define MIN_RIDDLE_LENGTH   20
#define MAX_ANSWERS         13
#define MAX_ANSWER_LENGTH   23

#define	FISH_ANIM			".\\art\\ridfish.bmp"
#define FISH_WAV			".\\sound\\lakefish.wav"
#define NESS_ANIM			".\\art\\nessie.bmp"
#define NESS_WAV			".\\sound\\serpent.wav"
#define	SKIER_ANIM			".\\art\\skier.bmp"
#define SKIER_WAV			".\\sound\\jetski.wav"

typedef struct {
    USHORT nSoundId;
    char text[MAX_RIDDLE_LENGTH + 1];
    char answers[MAX_ANSWERS][MAX_ANSWER_LENGTH + 1];
} RIDDLE;

class CMyEdit: public CEdit {

    public:

    private:

    protected:
        VOID OnChar(UINT, UINT, UINT);
        VOID OnSysChar(UINT, UINT, UINT);
        VOID OnKeyDown(UINT, UINT, UINT);
        VOID OnSysKeyDown(UINT, UINT, UINT);
        DECLARE_MESSAGE_MAP()
};

class CRiddlesWindow : public CFrameWnd
{
    public:
        CRiddlesWindow(VOID);
        VOID        PlayGame(VOID);
        VOID        PaintScreen(VOID);
        VOID        LoadIniSettings(VOID);
        VOID        ParseAnswer(const CHAR *);
        VOID        GamePause(VOID);
        VOID        GameResume(VOID);

    private:
    	VOID 		OnSoundNotify(CSound *pSound);

    protected:
        VOID        FlushInputEvents(VOID);
        BOOLEAN     CheckUserGuess(const CHAR *);
        ERROR_CODE  RepaintSpriteList(VOID);
        ERROR_CODE  LoadRiddle(VOID);
        ERROR_CODE  BuildSpriteList(VOID);
        ERROR_CODE  DisplayLine(const CHAR *, INT, INT, INT);
        INT         CharToIndex(CHAR);
        ERROR_CODE  ValidateRiddle(RIDDLE *);
        ERROR_CODE  LoadMasterSprites(VOID);
        VOID        GameReset(VOID);
        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        VOID        HandleError(ERROR_CODE);
        VOID        DeleteSprite(CSprite *);

        VOID        OnPaint();
        VOID        OnMouseMove(UINT, CPoint);
        VOID        OnLButtonDown(UINT, CPoint);
        VOID        OnClose();
        VOID        OnSetFocus(CWnd *);
        VOID        OnTimer(UINT);
    	long 		OnMCINotify( WPARAM, LPARAM);
    	long 		OnMMIONotify( WPARAM, LPARAM);
        HBRUSH      OnCtlColor(CDC *, CWnd *, UINT);

        DECLARE_MESSAGE_MAP()

        CRect       m_rNewGameButton;
        CMyEdit    *m_pEditText;
        CPalette   *m_pGamePalette;
        CBmpButton *m_pScrollButton;
        RIDDLE     *m_pRiddle;
        CSprite    *m_pSunDial;
        CSound     *m_pSoundTrack;
        INT         m_nTimeLimit;
        INT         m_nInitTimeLimit;
        INT         m_nTimer;
        INT         m_nDifficultyLevel;
        UINT        m_nRiddleNumber;
        BOOL        m_bGameActive;
        BOOL        m_bPause;
        BOOL        m_bIgnoreScrollClick;
};

} // namespace Riddles
} // namespace HodjNPodj
} // namespace Bagel

#endif
