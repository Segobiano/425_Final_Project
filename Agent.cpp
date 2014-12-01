#include "Agent.h"
#include "Grid.h"

Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale, Grid* g, GridNode* node)
{
	using namespace Ogre;
	grid = g;
	mSceneMgr = SceneManager; // keep a pointer to where this agent will be
	myNode = node;
	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;
	
	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	
	mBodyNode->setPosition(grid->getPosition(myNode->getRow(),myNode->getColumn()));
	mBodyNode->translate(0, height, 0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale, scale, scale); // Scale the figure

	setupAnimations();  // load the animation for this character

	// configure walking parameters
	mWalkSpeed = 35.0f;	
	mDirection = Ogre::Vector3::ZERO;

	mHealth = 1.0;
	active = false;
}

Agent::~Agent()
{
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
}

Ogre::Vector3
Agent::getPosition()
{
	return mBodyNode->getPosition();
}

void 
Agent::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y+height, z);	
}

// update is called at every frame from GameApplication::addTime
void
Agent::update(Ogre::Real deltaTime) 
{
	// only update if visible
	if (isVisible())
	{
		this->updateAnimations(deltaTime);	// Update animation playback
		this->updateLocomote(deltaTime);	// Update Locomotion
	}

}

void 
Agent::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
		{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
		"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < 13; i++)
	{
		mAnims[i] = mBodyEntity->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_IDLE_BASE);
	setTopAnimation(ANIM_IDLE_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

void 
Agent::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id; 

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}
	
void Agent::setTopAnimation(AnimID id, bool reset)
{
	if (mTopAnimID >= 0 && mTopAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void 
Agent::updateAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime; // how much time has passed since the last update

	if (mTopAnimID != ANIM_IDLE_TOP)
	if (mTopAnimID != ANIM_NONE)
	/*if (mTimer >= mAnims[mTopAnimID]->getLength())
		{
			setTopAnimation(ANIM_IDLE_TOP, true);
			setBaseAnimation(ANIM_IDLE_BASE, true);
			mTimer = 0;
		}
	*/
	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void 
Agent::fadeAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	for (int i = 0; i < 13; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

bool 
Agent::nextLocation()
{
	//while the que of location isnt empty
	if (mWalkList.empty())
             return false;
	mDestination = mWalkList.front();  // this gets the front of the deque
    mWalkList.pop_front();             // this removes the front of the deque
 
    mDirection = mDestination - mBodyNode->getPosition();
    mDistance = mDirection.normalise();
	
	return true;
}

void 
Agent::updateLocomote(Ogre::Real deltaTime)
{
	//This is the update locomotion from the previous assignment
	// Set idle animation
	if (mDirection == Ogre::Vector3::ZERO)
	{
            if (nextLocation()) 
            {
				Ogre::Vector3 src = mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
					if ((1.0f + src.dotProduct(mDirection)) < 0.0001f) 
					{
						mBodyNode->yaw(Ogre::Degree(180));
					}
					else
					{
						Ogre::Quaternion quat = src.getRotationTo(mDirection);
						mBodyNode->rotate(quat);
					}
				setBaseAnimation(ANIM_RUN_BASE);
				setTopAnimation(ANIM_RUN_TOP);
             }
	}
	else
	{
       Ogre::Real move = mWalkSpeed * deltaTime;
       mDistance -= move;
	   if (mDistance <= 0.0f)
	   {
		   mBodyNode->setPosition(mDestination);
		   mDirection = Ogre::Vector3::ZERO;
		   if (!nextLocation())
		   {                  
			   setBaseAnimation(ANIM_IDLE_BASE);
			   setTopAnimation(ANIM_IDLE_TOP);
		   } 
		   else
		   {
			   Ogre::Vector3 src = mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
			   if ((1.0f + src.dotProduct(mDirection)) < 0.0001f) 
			   {
				   mBodyNode->yaw(Ogre::Degree(180));
			   }
			   else
			   {
				   Ogre::Quaternion quat = src.getRotationTo(mDirection);
				   mBodyNode->rotate(quat);
			   } 
		   }
	   }
	   else { mBodyNode->translate(mDirection * move); }
	}
	
}
//moves agent the shortest path between s (start) and g (goal) 
void Agent::moveTo(GridNode* g)
{
	//The orge is on the goal node return nothing
	if (myNode->getRow()==g->getRow()&&myNode->getColumn()==g->getColumn()){ return; }
	
	//if the goal is occupied then you cant go to it
	if (!g->isClear()){ return; }

	std::list<GridNode*> path = grid->AStar(myNode,g);//This is the function that does the work

	//no path was found if the list is empty meaning he is blocked from the goal
	if (path.empty()){ return; }

	//sets the orge location to the goal is it makes its past the above checks
	myNode = g;
	Ogre::Vector3 location;
	int pathcounter = 0;
	//take the a Star path and push onto the walk list
	for (std::list<GridNode*>::iterator it=path.begin(); it != path.end(); ++it){
		location = grid->getPosition((*it)->getRow(),(*it)->getColumn());
		location[1] = height;
		if (it == path.begin())
		{
			//put S if its the start for the write out
			(*it)->contains = 'S';
		}
		else
		{
			//push char int for write out
			(*it)->contains = (char)'0'+(pathcounter%10);
			pathcounter++;
		}
		mWalkList.push_back(location);
	}
	g->contains = 'G';
	//rights the path out
	printPathToFile();
}

void 
Agent::printPathToFile()
{
	std::string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	//gets the unique name of the agent for the filename
	path+=mBodyEntity->getName();
	path+= ".txt"; //if txt file is in the same directory as cpp file
	std::ofstream outFile;
	outFile.open(path);

	if (!outFile.is_open()) // oops. there was a problem opening the file
	{
		std::cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	
		return;
	}

	for (int i = 0; i < grid->getRow(); i++)
	{
		for (int j = 0; j < grid->getCol(); j++)
		{
			outFile << grid->getNode(i, j)->contains << " ";
			//clears the contains as its writing the content out
			if (grid->getNode(i, j)->contains!='B'){
				grid->getNode(i, j)->contains='.';}
		}
		outFile << std::endl;
	}
	outFile.close();
}

void
Agent::moveAgent(std::list<GridNode*> gn)
{
	mWalkList.clear();
	mDirection = Ogre::Vector3::ZERO;
	Ogre::Vector3 location(0,0,0);
	for (std::list<GridNode*>::iterator it = gn.begin(); it != gn.end(); ++it)
	{
		location = grid->getPosition((*it)->getRow(),(*it)->getColumn());
		location[1] = height;
		
		mWalkList.push_back(location);
	}
	//gets the vector of the location he needs to move to
	//Ogre::Vector3 location=grid->getPosition(g->getRow(),g->getColumn());
	//multiple the height by 2 because it stops him from flipping
	//location[1]=height;
	mWalkList.push_back(location);
}

// hide/show agent
void
Agent::toggleVisibility(bool b)
{
	mBodyEntity->setVisible(b);
}

// is agent visible?
bool
Agent::isVisible()
{
	return mBodyEntity->isVisible();
}

///////////////////////////////////////////////////////////////
// collides with any other bounding boxes?
bool
Agent::intersects(Ogre::Entity* e)
{
	if (e == NULL) { return false; }

	Ogre::AxisAlignedBox mBox = mBodyEntity->getWorldBoundingBox();
	Ogre::AxisAlignedBox eBox = e->getWorldBoundingBox();

	return mBox.intersects(eBox);
}


// set agent active or inactive
void
Agent::toggleActive(bool b)
{
	active = b;
}

// increment/decrement speed
void 
Agent::adjustSpeed(Ogre::Real r)					
{ 
	mWalkSpeed += r;
}