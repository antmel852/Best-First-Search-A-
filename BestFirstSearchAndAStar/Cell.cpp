#include "Cell.h"

Cell::Cell()
{
	xPos = 0;
	yPos = 0;
	h = 1000;
	g = 1000;
	f = 0;
	parent = nullptr;
}

Cell::Cell(int r, int c, double newH, Cell* p)
{
	xPos = r;
	yPos = c;
	h = newH;
	g = 1000;
	f = 0;
	parent = p;

}

Cell::Cell(int r, int c, double newH, double newG, Cell* p)
{
	xPos = r;
	yPos = c;
	h = newH;
	g = newG;
	f = 0;
	parent = p;

}

bool Cell::operator== (const Cell& other)
{
	return (this->xPos == other.xPos && this->yPos == other.yPos);
}