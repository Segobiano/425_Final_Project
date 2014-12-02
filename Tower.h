#pragma once
#include "BaseApplication.h"
#include "Agent.h"
#include "Grid.h"

class GridNode;
class Grid;
class Agent;
class GameApplication;

class Tower
{
private:
	enum Status
	{
		SHOOTING,
		SEARCHING,
		BUILDING,
		INACTIVE
	};

	Ogre::SceneManager* mSceneMgr;
	Ogre::SceneNode* mBodyNode;
	Ogre::SceneNode* orbNode;		//pointer to projectile/slug node
	Ogre::Entity* orbEntity;		//
	Ogre::Entity* mBodyEntity;
	float mHeight;

	Agent* mTarget;			//target to shoot at. Set to null if no target
	GridNode* mNode;		//tower's node on the grid
	Grid* grid;				//grid world
	//pointer to sphere/cannonball thing sphere

	GameApplication* mGame;	//pointer to the game, use for list of agents

	Status mStatus;			//is the tower searching or shooting
	int mLevel;				//level of the tower determines power and weapons

	Ogre::Vector3 orbDir;	//direction of the orb
	Ogre::Real range;		//range of tower
	float reload_time;		//max time between orbs
	float time_til_shoot;	//time left until next orb
	float orb_speed;		//speed of the orb
	
	bool inRange(Agent* a);			//check if agent is in range of the tower

	void build();						//build tower up
	void setReloadTime(float time);		//set a new reload time
	void shoot(Ogre::Real deltaTime);	//update orb movement towards target
	void reload();						//reset orb position
	void search(std::list<Agent*> agentList);	//search for a target withing a vicinity and update mTarget
	
public:
	Tower(Ogre::SceneManager* sceneMgr, Grid* g, GridNode* n, std::string name, 
			std::string filename, float height, float scale, GameApplication* game);
	~Tower();

	void levelUp();	// level up tower, changing some of its traits
	int getLevel() { return mLevel; }

	void setStatus(Status s);
	void setPosition(GridNode* n);	//set to new grid position
	void update(Ogre::Real deltaTime, std::list<Agent*> agentList);	//update tower's status, target, and shoot
};