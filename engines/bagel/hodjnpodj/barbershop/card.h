/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * card.h
 *
 * HISTORY
 *
 *      1.00      04/19/94     Jos     Creation
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *
 ****************************************************************/
#ifndef __game_card_H__
#define __game_card_H__

#include "sprite.h"

#define TAB_COUNT	5
#define TAB_STACK	7
#define TABLEAU		35

#define STOCK		27
#define STOCK_DRAW	3

#define SUITS		5
#define	PIPS		12

#define	ONE_CARD	0
#define TWO_CARD	1
#define THREE_CARD	2
#define FOUR_CARD	3
#define FIVE_CARD	4
#define SIX_CARD	5
#define SEVEN_CARD	6
#define EIGHT_CARD	7
#define NINE_CARD	8
#define TEN_CARD	9
#define CUST_CARD	10
#define BARB_CARD	11
#define MANI_CARD	60

enum suit {suit_none = -1, brush, comb, mirror, razor, sissor, manicurist};
enum loc {loc_none = -1, fnd, tab, stock = 6, used};

class CStack;
class CCard;

class OSpr : public CSprite {
public:
	OSpr();

	CCard	*m_cCard;
};

class CCard {
private:	// vars
	suit	m_enSuit;
	int		m_nPip;

public:		// functions

	CCard(suit enSuit, int nPip);
	CCard(int);
    CCard();
	~CCard();

	int		GetValue();
	int		GetPip() { return (m_nPip); }
	BOOL	IsFace() { return (m_nPip >= CUST_CARD); }
	BOOL	IsCustomer() { return (m_nPip == CUST_CARD); }
	BOOL	IsBarber() { return (m_nPip == BARB_CARD); }
	BOOL	IsManicurist() { return (m_nPip == MANI_CARD); }
	BOOL	operator==(int nValue) { return (nValue == GetValue()); }
	CCard	&operator=(const CCard&);

public:		// vars
	CCard	*m_pPrevCard;
	CCard	*m_pNextCard;
	OSpr	*m_pSprite;
	CStack	*m_pStack;
	BOOL	m_bIsBack;
	CPoint	m_cOrigin;
};

#endif // __game_card_H__
