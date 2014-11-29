////////////////////////////////////////////////////////
// Class to hold the grid layout of our environment
// Used for navigation and AI, not graphics

#ifndef GRID_H
#define GRID_H
#include <iostream>
#include <vector>
#include <assert.h>
#include "GameApplication.h"

#define NODESIZE 50.0

class GridNode {
private:
	int nodeID;			// identify for the node
	int rCoord;			// row coordinate
	int cCoord;			// column coordinate
	bool clear;			// is the node walkable?
			
public:
	GridNode();			// default constructor
	GridNode(int nID, int row, int column, bool isC = true);	// create a node
	~GridNode();		// destroy a node
	
	Ogre::SceneNode* mSceneNode;	//used to select grid node
	Ogre::Entity *entity;			// a pointer to the entity in this node
	GridNode* parent;

	bool wp;
	//static int worldID;
	bool operator<(GridNode* k);
	char contains;		// For printing... B = blocked, S = start, G = goal, numbers = path
	int FofX;			// Holds the distance + heristic
	
	int gX;				// distance to the start
	int whichlist;		// 0 no list, 1 openlist, 2 closedlist
	
	void setID(int id);				// set the node id
	int getID(){ return nodeID; }	// get the node ID
	void setRow(int r);				// set the row coordinate
	void setColumn(int c);			// set the column coordinate
	int getRow();					// get the row and column coordinate of the node
	int getColumn();
	void setClear();				// set the node as walkable
	void setOccupied();				// set the node as occupied
	Ogre::Vector3 getPosition(int rows, int cols);	// return the position of this node
	
	bool isClear();					// is the node walkable

};

class GridRow {  // helper class
public:
	std::vector<GridNode> data;
	GridRow(int size) {data.resize(size);};
	~GridRow(){};
};

class Grid {
private:
	Ogre::SceneManager* mSceneMgr;	// pointer to scene graph
	std::vector<GridRow> data;		// actually hold the grid data
	int nRows;						// number of rows
	int nCols;						// number of columns
public:
	Grid(Ogre::SceneManager* mSceneMgr, int numRows, int numCols);	// create a grid
	~Grid();					// destroy a grid

	std::vector<GridNode*> waypoints;
	std::list<GridNode*> AStar(GridNode* s,GridNode* g);////Astar path between a start and a goal.  Returns a vector of nodes or null if the path is blocked
	double howFar(GridNode* s, GridNode* g);
	GridNode* getNode(int r, int c);		// get the node specified 
	//gets directions and returns that node else returns null
	GridNode* getNorthNode(GridNode* n);	// get adjacent nodes;
	GridNode* getSouthNode(GridNode* n);
	GridNode* getEastNode(GridNode* n);
	GridNode* getWestNode(GridNode* n);
	GridNode* getNENode(GridNode* n);
	GridNode* getNWNode(GridNode* n);
	GridNode* getSENode(GridNode* n);
	GridNode* getSWNode(GridNode* n);
	std::vector<GridNode*> getNeighbors(GridNode* n);
	
	Ogre::Vector3 getPosition(int r, int c);	// return the position  
	
	int getDistance(GridNode* node1, GridNode* node2);  // get Manhattan accounting for diagonals distance between between two nodes
	int getRow();			//Gets size of the grid
	int getCol();			//Gets size of the grid
	void printToFile();		// Print a grid to a file.  Good for debugging
	void loadObject(std::string name, std::string filename, int row, int height, int col, float scale = 1); // load and place a model in a certain location.
	bool buildCheck(int r, int c); //used only for building the world
	bool blockCheck(int r, int c); // used to check that if grid r,c would 
	std::list<GridNode*> buildPath();
	
};

#endif