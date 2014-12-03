#include "Tower.h"

Tower::Tower(Ogre::SceneManager* sceneMgr, Grid* g, GridNode* n, std::string name, 
			 std::string filename, float height, float scale, GameApplication* game)
{
	if (g == NULL || n == NULL || game == NULL)
	{
		std::cout << "ERROR: Invalid Tower Parameter(s)." << std::endl;
		return;
	}

	mSceneMgr = sceneMgr;
	mHeight = height;
	mGame = game;
	grid = g;
	mNode = n;

	//set up tower in scene
	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Tower constructor" << std::endl;
		return;
	}
	//set up tower visuals
	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyEntity->setCastShadows(false);
	mBodyEntity->setMaterialName("Examples/BumpyMetal");
	mBodyNode->attachObject(mBodyEntity);		// attach the model to the scene node
	//place in position according to n
	mBodyNode->setPosition(grid->getPosition(mNode->getRow(), mNode->getColumn()));
	mBodyNode->scale(scale, scale, scale);	// Scale the figure
	mBodyNode->translate(0, mHeight, 0);	// set on plane

	//set up orb visuals
	orbEntity = mSceneMgr->createEntity(name + "'s orb", "geosphere4500.mesh");
	orbEntity->setMaterialName("Examples/TextureEffect2");
	orbEntity->setCastShadows(false);
	orbNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	orbNode->attachObject(orbEntity);
	orbNode->setPosition(grid->getPosition(mNode->getRow(), mNode->getColumn()));
	orbNode->scale(0.03f, 0.03f, 0.03f);
	orbNode->translate(0, mHeight * 2, 0);	// move orb slightly above tower
	orbNode->setVisible(false);
	
	mNode->setOccupied();

	mTarget = NULL;
	mStatus = BUILDING;
	mLevel = 0;
	range = 100;
	orb_speed = 0.5;
	reload_time = 5.0;	//default reload time
	time_til_shoot = reload_time;

	mNode->setTower(this);		// node that holds tower should point to it
}

// return whether agent is within range of the tower
bool 
Tower::inRange(Agent* a)
{
	//get distance between tower and agent, if it is close, return true
	Ogre::Real distance = (mBodyNode->getPosition() - a->getPosition()).normalise();
	if (distance <= range && a->isVisible()) return true;
	else return false;
}

// build tower up with an animation
// translate upwards til on plane
void
Tower::build()
{
	//animate building, when done, set to searching
	mStatus = SEARCHING;
}

// update orb's location and check for collision
void
Tower::shoot(Ogre::Real deltaTime)
{
	Ogre::Real move;	
	//std::cout << "Pew, Pew" << std::endl;
	if (mTarget->intersects(orbEntity)) // orb hit target, deal damage and reload
	{
		mTarget->mHealth--; //decrease targets health
		reload(); // if collides, reset orb back at tower
		//show some sort of effect here so player knows a collision happened
		return;
	}
	if (!mTarget->isVisible())	// can't see target, cease fire and find new target
	{
		mTarget = NULL;
		reload();
		orbNode->setVisible(false);
		mStatus = SEARCHING;
		return;
	}

	//target is moving, must continously update direction
	orbDir = mTarget->getPosition() - orbNode->getPosition();
	move = orb_speed * deltaTime;
	orbNode->translate(orbDir * move);
}

// set new reload time, and reset time_til_shoot
void
Tower::setReloadTime(float time)
{
	reload_time = time;
	time_til_shoot = reload_time;
}

// reset orb to shoot again
void
Tower::reload()
{
	orbNode->setPosition(grid->getPosition(mNode->getRow(), mNode->getColumn()));
	orbNode->translate(0, mHeight * 2, 0);	// move orb slightly above tower
}

// search for a target from agent list that is within a certain distance
// if so, target it and change states
void 
Tower::search(std::list<Agent*> agentList)
{
	if (agentList.front() == NULL) { return; }	// no more targets

	std::list<Agent*>::iterator iter;
	for (iter = agentList.begin(); iter != agentList.end(); iter++)
	{
		if (*iter != NULL && inRange(*iter))
		{
			mTarget = (*iter);	// set agent as target
			mStatus = SHOOTING;
			orbNode->setVisible(true);
			return;				// found a target, quit looking
		}
	}
}

////////////////////////////////////////////////
//public methods ///////////////////////////////

// tower increments a level, changing some traits
void
Tower::levelUp()
{
	mLevel++;	//level up! +1
	//now change traits based on new level
	if (mLevel == 1)
	{
		orb_speed = 1.0;
		range += 20;
		mBodyEntity->setMaterialName("Examples/Chrome");
	}
	else if (mLevel == 2)
	{
		orb_speed = 1.2;
		range += 30;
		mBodyEntity->setMaterialName("Examples/TextureEffect1");
	}
	else // maxed out, can't get more powerful than this
		return;
}

// set status if you need to force a state
void
Tower::setStatus(Status s)
{
	mStatus = s;
}

// set new node and move to that node's position
void
Tower::setPosition(GridNode* n)
{
	mNode = n;
	Ogre::Vector3 pos = mNode->getPosition(mNode->getRow(), mNode->getColumn());
	mBodyNode->setPosition(pos[0], pos[1] + mHeight, pos[2]);
}

// update tower's status, target, and shoot
void
Tower::update(Ogre::Real deltaTime, std::list<Agent*> agentList)
{
	if (mStatus == BUILDING)
	{
		//continue constructing animation, growing up from the ground
		//return
		build();
	}
	if (mStatus == SEARCHING)
	{
		search(agentList);	// check for target, update status if one is found
	}
	if (mStatus == SHOOTING)
	{
		// shoot if target is selected, visible, and in range
		if (!mTarget) 
		{ 
			reload();
			orbNode->setVisible(false);
			mStatus = SEARCHING; 
		}
		else if (!mTarget->isVisible() || !inRange(mTarget)) 
		{ 
			mTarget = NULL;
			reload();
			orbNode->setVisible(false);
			mStatus = SEARCHING;
		}
		else shoot(deltaTime);	// update orb
	}
	else // tower is inactive
	{
		//do nothing?
	}

}
