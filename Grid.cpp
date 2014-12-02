#include "Grid.h"
#include <iostream>
#include <fstream>
#include <queue>

//Comparitor for two nodes so they can be added in the PQ
//class CompareNode {
bool operator< (const GridNode& n1, const GridNode& n2)
{
	return n1.FofX > n2.FofX;	
}
//Overload the > operator.
bool operator> (const GridNode& n1, const GridNode& n2)
{
	return n1.FofX < n2.FofX;	
}
//};

////////////////////////////////////////////////////////////////
// create a node
GridNode::GridNode(int nID, int row, int column, bool isC)
{
	this->clear = isC;
	this->FofX = NULL;
	this->gX = NULL;
	this->whichlist = 0;//in no list 
	this->parent = NULL;
	this->rCoord = row;
	this->cCoord = column;
	this->wp = false;
	this->entity = NULL;
	this->mSceneNode = NULL;
	this->mTower = NULL;

	if (isC)
		this->contains = '.';
	else
		this->contains = 'B';
	
}

// default constructor
GridNode::GridNode()
{
	nodeID = -999;			// mark these as currently invalid
	this->clear = true;
	this->contains = '.';
} 

////////////////////////////////////////////////////////////////
// destroy a node
GridNode::~GridNode()
{}  // doesn't contain any pointers, so it is just empty

////////////////////////////////////////////////////////////////
// set the node id
void 
GridNode::setID(int id)
{
	this->nodeID = id;
}

////////////////////////////////////////////////////////////////
// set the x coordinate
void 
GridNode::setRow(int r)
{
	this->rCoord = r;
}

////////////////////////////////////////////////////////////////
// set the y coordinate
void 
GridNode::setColumn(int c)
{
	this->cCoord = c;
}

////////////////////////////////////////////////////////////////
// get the x and y coordinate of the node
int 
GridNode::getRow()
{
	return rCoord;
}

int 
GridNode::getColumn()
{
	return cCoord;
}

bool GridNode::operator<(GridNode* k)
{
  return (this->FofX < k->FofX);
}

// return the position of this node
Ogre::Vector3 
GridNode::getPosition(int rows, int cols)
{
	Ogre::Vector3 t;
	t.z = (rCoord * NODESIZE) - (rows * NODESIZE)/2.0 + (NODESIZE/2.0); 
	t.y = 0; 
	t.x = (cCoord * NODESIZE) - (cols * NODESIZE)/2.0 + (NODESIZE/2.0); 
	return t;
}

////////////////////////////////////////////////////////////////
// set the node as walkable
void 
GridNode::setClear()
{
	this->clear = true;
	this->contains = '.';
}

////////////////////////////////////////////////////////////////
// set the node as occupied
void 
GridNode::setOccupied()
{
	this->clear = false;
	this->contains = 'B';
}

////////////////////////////////////////////////////////////////
// is the node walkable
bool 
GridNode::isClear()
{
	return this->clear;
}

////////////////////////////////////////////////////////////////
//tower getters and setters, if a node has a tower
void 
GridNode::setTower(Tower* t)
{
	if (t == NULL) 
	{  
		mTower = NULL;
	}
	else mTower = t;
}

Tower* 
GridNode::getTower()
{
	if (!mTower) { return NULL; }
	return mTower;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// create a grid
Grid::Grid(Ogre::SceneManager* mSceneMgr, int numRows, int numCols)
{
	this->mSceneMgr = mSceneMgr; 

	assert(numRows > 0 && numCols > 0);
	this->nRows = numRows;
	this->nCols = numCols;

	data.resize(numCols, GridRow(numRows));
		
	// put the coordinates in each node
	// give each node a sceneNode for selection by clicking
	int count = 0;
	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
		{
			GridNode *n = this->getNode(i,j);
			n->setRow(i);
			n->setColumn(j);
			n->setID(count);
			n->setTower(NULL);
			count++;
		}
		
}

/////////////////////////////////////////
// destroy a grid
Grid::~Grid(){};  														

////////////////////////////////////////////////////////////////////
//Getter for getting the number or rows and cols
int Grid::getRow(){
	return this->nRows;
}
int Grid::getCol(){
	return this->nCols;
}
////////////////////////////////////////////////////////////////
// get the node specified 
GridNode* 
Grid::getNode(int r, int c)
{
	if (r >= nRows || c >= nCols || r < 0 || c < 0)
		return NULL;

	return &this->data[c].data[r];
}

////////////////////////////////////////////////////////////////
// get adjacent nodes;
GridNode* 
Grid::getNorthNode(GridNode* n)
{
	if (n!=NULL){
		//check that its not in the first row
		if (n->getRow()!=0){
			//check to see if the node is clear this is a design choice to make a* easier
			if ((getNode(n->getRow()-1,n->getColumn()))->isClear()){
				return getNode(n->getRow()-1,n->getColumn());
			}
		}
	}
	return NULL;
}

GridNode* 
Grid::getSouthNode(GridNode* n)
{
	if (n!=NULL){
		//check that its not in the last row
		if (n->getRow()!=nRows-1){
			//check to see if the node is clear this is a design choice to make a* easier
			if (getNode(n->getRow()+1,n->getColumn())->isClear()){
				return getNode(n->getRow()+1,n->getColumn());
			}
		}
	}
	return NULL;
	
}

GridNode* 
Grid::getEastNode(GridNode* n)
{
	if (n!=NULL){
		//check that its not in the last column
		if (n->getColumn()!=nCols-1){
			//check to see if the node is clear this is a design choice to make a* easier
			if ((getNode(n->getRow(),n->getColumn()+1))->isClear()){
				return getNode(n->getRow(),n->getColumn()+1);
			}
		}
	}
	return NULL;
}

GridNode* 
Grid::getWestNode(GridNode* n)
{
	if (n!=NULL){
		//check that its not in the first column
		if (n->getColumn()!=0){
			//check to see if the node is clear this is a design choice to make a* easier
			if (getNode(n->getRow(),n->getColumn()-1)->isClear()){
				return getNode(n->getRow(),n->getColumn()-1);
			}
		}
	}
	return NULL;
}

GridNode* 
Grid::getNENode(GridNode* n)  
{
	if (n!=NULL){
		//check that its not in the first row or last column
		if ((n->getRow()!=0)&&(n->getColumn()!=nCols-1)){
			//check to see if the node is clear this is a design choice to make a* easier
			if (getNode(n->getRow()-1,n->getColumn()+1)->isClear()&&Grid::getNorthNode(n)!=NULL&&Grid::getEastNode(n)!=NULL){
				return getNode(n->getRow()-1,n->getColumn()+1);
			}
		}
	}
	return NULL;
}

GridNode* 
Grid::getNWNode(GridNode* n) 
{
	if (n!=NULL){
		//check that its not in the first row or first column
		if ((n->getRow()!=0)&&(n->getColumn()!=0)){
			//check to see if the node is clear this is a design choice to make a* easier diagnol movies check also the cardinals
			if (getNode(n->getRow()-1,n->getColumn()-1)->isClear()&&Grid::getNorthNode(n)!=NULL&&Grid::getWestNode(n)!=NULL){
				return getNode(n->getRow()-1,n->getColumn()-1);
			}
		}
	}
	return NULL;
}

GridNode* 
Grid::getSENode(GridNode* n) 
{
	if (n!=NULL){
		//check that its not in the last row or last column
		if ((n->getRow()!=nRows-1)&&(n->getColumn()!=nCols-1)){
			//check to see if the node is clear this is a design choice to make a* easier diagnol movies check also the cardinals
			if (getNode(n->getRow()+1,n->getColumn()+1)->isClear()&&Grid::getSouthNode(n)!=NULL&&Grid::getEastNode(n)!=NULL){
				return getNode(n->getRow()+1,n->getColumn()+1);
			}
		}
	}
	return NULL;
}

GridNode* 
Grid::getSWNode(GridNode* n) 
{
	if (n!=NULL){
		//check that its not in the last row or first column
		if ((n->getRow()!=nRows-1)&&(n->getColumn()!=0)){
				//check to see if the node is clear this is a design choice to make a* easier diagnol movies check also the cardinals
			if (getNode(n->getRow()+1,n->getColumn()-1)->isClear()&&Grid::getSouthNode(n)!=NULL&&Grid::getWestNode(n)!=NULL){
				return getNode(n->getRow()+1,n->getColumn()-1);
			}
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////
//gets all Neighbors and returns a vector of the nodes
//Created to make the call easier in the case you need all of them but kept the signle direction funtions
std::vector<GridNode*> 
Grid::getNeighbors(GridNode* n){
	std::vector<GridNode*> out;
	//All the if's check that the node is not out of bounds of occupied
	if (getNorthNode(n)!=NULL){
		out.push_back(getNorthNode(n));		
	}
	if (getSouthNode(n)!=NULL){
		out.push_back(getSouthNode(n));
	}
	if (getEastNode(n)!=NULL){
		out.push_back(getEastNode(n));
	}
	if (getWestNode(n)!=NULL){
		out.push_back(getWestNode(n));
	}
	if (getNENode(n)!=NULL){
		out.push_back(getNENode(n));
	}
	if (getNWNode(n)!=NULL){
		out.push_back(getNWNode(n));
	}
	if (getSENode(n)!=NULL){
		out.push_back(getSENode(n));
	}
	if (getSWNode(n)!=NULL){
		out.push_back(getSWNode(n));
	}

	return out;

}
////////////////////////////////////////////////////////////////
//get distance between between two nodes using diagonal and "city block" ignoring walls.  This is a design choice to make the heristric admissable for a*
int 
Grid::getDistance(GridNode* node1, GridNode* node2)
{
	if (node1==NULL||node2==NULL){
		return 0;
	}
	int manDistance=0;
	int nrow1=node1->getRow();
	int ncol1=node1->getColumn();
	int nrow2=node2->getRow();
	int ncol2=node2->getColumn();
	//Diagnol move because they are not on either the same row or column
	while ((nrow1 != nrow2) && (ncol1 != ncol2)){
		manDistance+=14;
		if (nrow1 > nrow2){
			nrow1--;
		}
		else{
			nrow1++;
		}
		if (ncol1 > ncol2){
			ncol1--;
		}
		else{
			ncol1++;
		}
	}

	//we got to the point by a diagonal move
	if ((nrow1==nrow2)&&(ncol1==ncol2)){
		return manDistance;
	}

	//on the correct row
	if (nrow1>=nrow2){
		manDistance+=(nrow1-nrow2)*10;
	}
	else{manDistance+=(nrow2-nrow1)*10;}
	
	//on the correct column
	if (ncol1>=ncol2){
		manDistance+=(ncol1-ncol2)*10;
	}
	//on the correct column
	else{manDistance+=(ncol2-ncol1)*10;}

	return manDistance;
}

///////////////////////////////////////////////////////////////////////////////
// Print out the grid in ASCII
void 
Grid::printToFile()
{
	std::string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= "Grid.txt"; //if txt file is in the same directory as cpp file
	std::ofstream outFile;
	outFile.open(path);

	if (!outFile.is_open()) // oops. there was a problem opening the file
	{
		std::cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	
		return;
	}

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			outFile << this->getNode(i, j)->contains << " ";
		}
		outFile << std::endl;
	}
	outFile.close();
}


void // load and place a model in a certain location.
Grid::loadObject(std::string name, std::string filename, int row, int height, int col, float scale)
{
	using namespace Ogre;

	if (row >= nRows || col >= nCols || row < 0 || col < 0)
		return;

	Entity *ent = mSceneMgr->createEntity(name, filename);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(name,
        Ogre::Vector3(0.0f, 0.0f,  0.0f));
    node->attachObject(ent);
    node->setScale(scale, scale, scale);


	GridNode* gn = this->getNode(row, col);
	node->setPosition(getPosition(row, col)); 
	node->setPosition(getPosition(row, col).x, height, getPosition(row, col).z);
	gn->setOccupied();
	gn->entity = ent;
}

////////////////////////////////////////////////////////////////////////////
// Added this method and changed GridNode version to account for varying floor 
// plane dimensions. Assumes each grid is centered at the origin.
// It returns the center of each square. 
Ogre::Vector3 
Grid::getPosition(int r, int c)	
{
	Ogre::Vector3 t;
	t.z = (r * NODESIZE) - (this->nRows * NODESIZE)/2.0 + NODESIZE/2.0; 
	t.y = 0; 
	t.x = (c * NODESIZE) - (this->nCols * NODESIZE)/2.0 + NODESIZE/2.0; 
	return t;
}


//Astar path between a start and a goal.  Returns a vector of nodes or null if the path is blocked
std::list<GridNode*>
Grid::AStar(GridNode* s,GridNode* g){

	std::vector<GridNode*>OpenList;
	std::vector<GridNode*>ClosedList;

	GridNode* current;

	//makes sure that the FofX value is reset
	for (int i = 0; i < getRow(); i++){
		for (int j = 0; j < getCol(); j++){
			GridNode *n = getNode(i,j);
			n->FofX=NULL;
			n->gX=NULL;
			n->whichlist=0;
			n->parent=NULL;
		}
	}
	//sets the distance to the start as 0 and push to the openlist
	OpenList.push_back(s);
	s->gX=0;
	s->FofX=s->gX+getDistance(s,g);
	s->whichlist=1;
	
	//while nothing is in the openlist
	while (!OpenList.empty()){
		GridNode* tempNode=OpenList[0];
		int OpenIndex=0;
		//finds the lowest value since the std PQ doesnt work
		for (int z=0; z<OpenList.size();z++){
			if (OpenList[z]->FofX<tempNode->FofX){
				tempNode=OpenList[z];
				OpenIndex=z;
			}
		}
		//if we reached we popped the goal
		if (tempNode->getRow()==g->getRow()&&tempNode->getColumn()==g->getColumn()){
			//traces back through the parents till you get the path and push onto a new list
			//g->parent=current;
			std::list<GridNode*> finalpath;
			GridNode* par=g;
			while (par!=NULL){
				finalpath.push_front(par);
				par=par->parent;
			}
			return finalpath;
			
		}
		//this is the Top and Pop of my PQ
		current=tempNode;
		OpenList.erase(OpenList.begin() + OpenIndex);

		current->whichlist=2;//flag its in closed
		std::vector<GridNode*>neighbors=getNeighbors(current);
		//iterate through neighbors
		for (int i=0;i<neighbors.size();i++){
			//check that its not already in the closedlist
			if (neighbors[i]->whichlist!=2){
				//tentive is the current distance
				int tentive=current->gX+getDistance(current,neighbors[i]);
				if (neighbors[i]->whichlist==0||tentive<neighbors[i]->gX){
					neighbors[i]->parent=current;
					neighbors[i]->gX=tentive;
					neighbors[i]->FofX=neighbors[i]->gX+getDistance(neighbors[i],g);
					//if not on the openlist add it
					if (neighbors[i]->whichlist==0){
						OpenList.push_back(neighbors[i]);
						//Flag its on the open list
						neighbors[i]->whichlist=1;
					}
				}
			}
		}
	}
	
	std::list<GridNode*> NoPath=std::list<GridNode*>();
	//Nopath to the goal exist
	return NoPath;
}

bool Grid::buildCheck(int r,int c){
	this->getNode(r,c)->setOccupied(); 
	for (int i=0;i<(waypoints.size()-1);i++){
		waypoints[i]->setClear();
		waypoints[i+1]->setClear();
		std::list<GridNode*> path = AStar(waypoints[i],waypoints[i+1]);
		waypoints[i]->setOccupied();
		waypoints[i+1]->setOccupied();

		//no path was found if the list is empty meaning he is blocked from the goal
		if (path.empty()){
			this->getNode(r,c)->setClear(); 
			return false;
		}
	}
	return true;
}


//used to check is the r,c is blocking  (when you build towers use this to check and pass in the r,c)
bool Grid::blockCheck(int r,int c){
	//set it occupied and check for a path
	this->getNode(r,c)->setOccupied(); 
	for (int i=0;i<(waypoints.size()-1);i++){
		std::list<GridNode*> path = AStar(waypoints[i],waypoints[i+1]);
		//no path was found if the list is empty meaning he is blocked from the goal
		if (path.empty()){
			this->getNode(r,c)->setClear(); 
			return false;
		}
	}
	return true;
}


// build the path each agent will follow to each waypoint
std::list<GridNode*> Grid::buildPath()
{
	std::list<GridNode*> finalpath;
	for (int i = 0; i< (waypoints.size() - 1); i++)
	{	
		std::list<GridNode*> path = AStar(waypoints[i], waypoints[i+1]); //This is the function that does the work
		finalpath.insert(finalpath.end(), path.begin(), path.end());
	}
	return finalpath;
}