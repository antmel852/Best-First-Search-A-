#include "glut.h"
#include "Cell.h"
#include "CompareBFSCells.h"
#include "CompareAStarCells.h"
#include <time.h>
#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>
using namespace std;

const int W = 600;
const int H = 600;
const int WALL = 1;
const int SPACE = 0;
const int START = 2;
const int TARGET = 3;
const int MARKED = 4; // Cells marked (visited) by start bfs.
const int PATH = 5; // The shortest path between the points (START and TARGET).

const int MSZ = 100;

int maze[MSZ][MSZ] = { 0 };
int targetCoords[2];

bool isStart = true;
bool runBFS = false;
bool runAStar = false;

vector<Cell*> cells; // The vector holding all of the cells from the starting point.
vector<Cell*> visited;
vector<Cell*> notVisited;
priority_queue<Cell, vector<Cell>, CompareBFSCells>  BFSPq;
priority_queue<Cell, vector<Cell>, CompareAStarCells>  aStarPq;

void InitMaze();
double distance(int x1, int y1);
double manhattanDistance(int x1, int y1);

void init()
{
	srand(time(0));
	glClearColor(1, 1, 1, 0);// color of window background
	glOrtho(-1, 1, -1, 1, 0, 1);

	InitMaze();
}

void InitMaze()
{
	int i, j;

	for (i = 0; i < MSZ; i++) // frame of WALLS
	{
		maze[0][i] = WALL;
		maze[MSZ - 1][i] = WALL;
		maze[i][0] = WALL;
		maze[i][MSZ - 1] = WALL;
	}

	for (i = 1; i < MSZ - 1; i++)
		for (j = 1; j < MSZ - 1; j++)
		{
			if (i % 2 == 1)
			{
				if (rand() % 100 > 20)
					maze[i][j] = SPACE;
				else
					maze[i][j] = WALL;
			}
			else // the line is even
			{
				if (rand() % 100 > 35)
					maze[i][j] = WALL;
				else
					maze[i][j] = SPACE;
			}
		}

	maze[MSZ / 2][MSZ / 2] = START;
	targetCoords[0] = rand() % MSZ; // Save ending X point of target.
	targetCoords[1] = rand() % MSZ; // Save ending Y Point of target.
	maze[targetCoords[0]][targetCoords[1]] = TARGET;

	Cell* sCell = new Cell(MSZ / 2, MSZ / 2, 0, 0, nullptr); // Create starting point cell. 
	Cell* eCell = new Cell(targetCoords[0], targetCoords[1], 0, nullptr); // Create ending point cell.

	BFSPq.push(*sCell);
	aStarPq.push(*sCell);
	notVisited.push_back(sCell);
}

void DrawMaze()
{
	int i, j;
	int xsz = W / MSZ;
	int ysz = H / MSZ;
	double x, y;
	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			switch (maze[i][j])   // set color
			{
			case SPACE:
				glColor3d(0.9, 0.9, 0.9);
				break;
			case WALL:
				glColor3d(0.3, 0, 0);
				break;
			case START:
				glColor3d(0, 1, 0);
				break;
			case TARGET:
				glColor3d(1, 0, 0);
				break;
			case MARKED: // Color cells visited by BFS from starting point.
				glColor3d(0.9, 0.9, 0.5);
				break;
			case PATH: // Color the path found as a solution.
				glColor3d(1, 0.2, 1);
				break;
			}
			// draw square
			x = 2.0 * (j * xsz) / W - 1; // value in range [-1,1)
			y = 2.0 * (i * ysz) / H - 1; // value in range [-1,1)
			glBegin(GL_POLYGON);
			glVertex2d(x, y);
			glVertex2d(x, y + (2.0 * ysz) / H);
			glVertex2d(x + 2.0 * xsz / W, y + (2.0 * ysz) / H);
			glVertex2d(x + 2.0 * xsz / W, y);
			glEnd();
		}
}

// Calculate the H for a BFS run.
double distance(int x1, int y1)
{
	return sqrt((x1 - targetCoords[0]) * (x1 - targetCoords[0]) + (y1 - targetCoords[1]) * (y1 - targetCoords[1]));
}

// Calculate the H for an A* run.
double manhattanDistance(int x1, int y1)
{
	return (abs(x1 - targetCoords[0]) + abs(y1 - targetCoords[1]));
}


void resetMaze() 
{
	for (int i = 0; i < MSZ; i++)
	{
		for (int j = 0; j < MSZ; j++)
		{
			if (maze[i][j] == MARKED || maze[i][j] == PATH)
				maze[i][j] = SPACE;
		}
	}
	while (!BFSPq.empty())
		BFSPq.pop();
	while (!aStarPq.empty())
		aStarPq.pop();
	visited.clear();
	notVisited.clear();

	Cell* sCell = new Cell(MSZ / 2, MSZ / 2, 0, 0, nullptr); // Create starting point cell. 
	Cell* eCell = new Cell(targetCoords[0], targetCoords[1], 0, nullptr); // Create ending point cell.

	BFSPq.push(*sCell);
	aStarPq.push(*sCell);
	notVisited.push_back(sCell);
}

void restorePath(Cell* c)
{
	// Restore Path.
	while (maze[c->getXPos()][c->getYPos()] != START)
	{
		maze[c->getXPos()][c->getYPos()] = PATH;
		c = c->getParent();
	}
}

// Checks the neighbor status of a cell at a given point and change it accordingly on a BFS run.
void checkBFSNeighbor(Cell* cell, int xPos, int yPos)
{
	if (maze[xPos][yPos] == SPACE)
	{
		Cell* newCell = new Cell(xPos, yPos, distance(xPos, yPos), cell);
		BFSPq.push(*newCell);
		maze[xPos][yPos] = MARKED;
	}
	else if (maze[xPos][yPos] == TARGET)
	{
		cout << "Solution Found!\n";
		runBFS = false;
		restorePath(cell);
	}
}

// Checks the neighbor status of a cell at a given point and change it accordingly on an A* run.
void checkAStarNeighbor(Cell* cell, int xPos, int yPos)
{
	vector<Cell*>::iterator visitIter;
	vector<Cell*>::iterator notVisitIter;
	if (maze[xPos][yPos] == SPACE)
	{
		Cell* newCell = new Cell(xPos, yPos, manhattanDistance(xPos, yPos), cell->getG() + 1, cell);
		newCell->setF(newCell->getG() + newCell->getH());
		visitIter = find(visited.begin(), visited.end(), newCell);
		notVisitIter = find(notVisited.begin(), notVisited.end(), newCell);
		if (visitIter == visited.end() && notVisitIter == notVisited.end())
		{
			aStarPq.push(*newCell);
			notVisited.push_back(newCell);
			maze[xPos][yPos] = MARKED;
		}
	}
	else if (maze[xPos][yPos] == TARGET)
	{
		cout << "Solution Found!\n";
		runAStar = false;
		restorePath(cell);
	}
}

// Check all neighbors of a given cell on a BFS run.
void checkBFSNeighbors(Cell* cell)
{
	checkBFSNeighbor(cell, cell->getXPos() + 1, cell->getYPos()); // Down
	if (runBFS)
		checkBFSNeighbor(cell, cell->getXPos() - 1, cell->getYPos()); // Up
	if (runBFS)
		checkBFSNeighbor(cell, cell->getXPos(), cell->getYPos() + 1); // Right
	if (runBFS)
		checkBFSNeighbor(cell, cell->getXPos(), cell->getYPos() - 1); // Left
}

// Check all neighbors of a given cell on an A* run.
void checkAStarNeighbors(Cell* cell)
{
	checkAStarNeighbor(cell, cell->getXPos() + 1, cell->getYPos()); // Down
	if (runAStar)
		checkAStarNeighbor(cell, cell->getXPos() - 1, cell->getYPos()); // Up
	if (runAStar)
		checkAStarNeighbor(cell, cell->getXPos(), cell->getYPos() + 1); // Right
	if (runAStar)
		checkAStarNeighbor(cell, cell->getXPos(), cell->getYPos() - 1); // Left
}

// Run one iteration of BFS.
void BFSIteration()
{
	Cell* pCurrent = new Cell(); // Current cell.
	if (BFSPq.empty())
	{
		cout << "There is no solution!\n";
		runBFS = false;
		return;
	}
	else
	{
		*pCurrent = BFSPq.top();
		BFSPq.pop();
		if (isStart)
		{
			pCurrent->setH(distance(pCurrent->getXPos(), pCurrent->getYPos()));
			isStart = false;
		}
		if (maze[pCurrent->getXPos()][pCurrent->getYPos()] == TARGET)
		{
			runBFS = false;
			cout << "The solution has been found\n";
			restorePath(pCurrent);
			return;
		}
		else
		{
			if (maze[pCurrent->getXPos()][pCurrent->getYPos()] != START)
				maze[pCurrent->getXPos()][pCurrent->getYPos()] = MARKED;
			checkBFSNeighbors(pCurrent);
		}
	}
}

// Run one iteration of A*.
void aStarIteration()
{
	vector<Cell*>::iterator notVisit;
	Cell* pCurrent = new Cell(); // Current cell.
	if (aStarPq.empty())
	{
		cout << "There is no solution!\n";
		runAStar = false;
		return;
	}
	else
	{
		*pCurrent = aStarPq.top();
		aStarPq.pop();
		if (isStart)
		{
			pCurrent->setH(manhattanDistance(pCurrent->getXPos(), pCurrent->getYPos()));
			pCurrent->setF(0);
			isStart = false;
		}
		if (maze[pCurrent->getXPos()][pCurrent->getYPos()] == TARGET)
		{
			runAStar = false;
			cout << "The solution has been found\n";
			restorePath(pCurrent);
			return;
		}
		else
		{
			visited.push_back(pCurrent);
			notVisit = find(notVisited.begin(), notVisited.end(), pCurrent);
			if (notVisit != notVisited.end())
			{
				notVisited.erase(notVisit);
			}
			if (maze[pCurrent->getXPos()][pCurrent->getYPos()] != START)
				maze[pCurrent->getXPos()][pCurrent->getYPos()] = MARKED;
			checkAStarNeighbors(pCurrent);
		}
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer
	DrawMaze();
	glutSwapBuffers(); // show all
}

void idle()
{
	if (runBFS)
		BFSIteration();
	if (runAStar)
		aStarIteration();
	glutPostRedisplay(); // go to display
}

void menu(int choice)
{
	switch (choice)
	{
	case 0: // BFS
		runBFS = true;
		break;
	case 1: // A*
		runAStar = true;
		break;
	case 2:
		resetMaze();
		break;
	}
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("First Example");

	glutDisplayFunc(display); // display is window refresh function
	glutIdleFunc(idle);  // idle is background function

	// menu
	glutCreateMenu(menu);
	glutAddMenuEntry("Best First Search", 0);
	glutAddMenuEntry("A*", 1);
	glutAddMenuEntry("Reset Maze", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	init();

	glutMainLoop();
}