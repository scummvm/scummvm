#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include <stdlib.h>
#include "grid.h"


colony::colony(int r, int c)
{
	int i, j; // indicies

	rows = r;
	cols = c;
	m_nColony_count = 0;

	/*
	 * Set up initial city.  All cells on the
	 * grid are set to NO_LIFE
	 */

	for (i = 0; i < rows; i++)
		for (j = 0; j < cols; j++)
			grid[i][j]=NO_LIFE;
}

colony::~colony(void)
{
}

colony& colony::operator=(const colony& c)
{
	int i, j;

	for (i = 0; i < this->rows; i++)
		for (j = 0; j < this->cols; j++)
			this->grid[i][j] = c.grid[i][j];
	this->m_nColony_count=c.m_nColony_count;

	return (*this);
}

colony::operator==(const colony& c)
{
	int i, j;

	if (this->m_nColony_count!=c.m_nColony_count)
		return 0;

	for (i = 0; i < this->rows; i++)
		for (j = 0; j < this->cols; j++)
			if (this->grid[i][j] != c.grid[i][j])
				return 0;
	return 1;
}

colony::operator!=(const colony& c)
{
	return (! (*this == c));
}

void colony::evolve(colony clny)
{
	int r, c;       	// for local cell check
	int neighbors;  	// # of neighbors in local area
	int pos_r, pos_c; // will step across every part of board

	/*
	 *  apply evolve conditions centered at pos_r, pos_c
	 */

	for (pos_r = 0; pos_r < rows; pos_r++)
		for (pos_c = 0; pos_c < cols; pos_c++) {
			neighbors = 0;
			for (r = pos_r-1; r <= pos_r+1; r++) {
				/*
				*  make sure it's not out of bounds of grid
				*/
				if (r < 0)
					continue;
				if (r >= rows)
					break;

				for (c = pos_c-1; c <= pos_c+1; c++) {
					if (c < 0)
						continue;
					if ( c >= cols)
						break;
					/* don't count center as neighbor */
					if ((r == pos_r) && (c == pos_c))
						continue;
					if (clny.grid[r][c] == LIFE)
						neighbors++;
				}
			}

			/* game of life conditions! */
			if ((neighbors < 2 || neighbors > 3) &&
						clny.grid[pos_r][pos_c] == LIFE)
				flip(pos_r,pos_c);
			else if (neighbors == 3 &&
						clny.grid[pos_r][pos_c] == NO_LIFE)
				flip(pos_r,pos_c);
		}
}


void colony::flip(int r, int c)
{
	if (grid[r][c] == LIFE) {
		grid[r][c]=NO_LIFE;
		m_nColony_count--;
	}
	 else {
		grid[r][c]=LIFE;
		m_nColony_count++;
	 }
}
