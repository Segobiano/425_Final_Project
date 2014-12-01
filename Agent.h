//#include "BaseApplication.h"
//#include <deque>


#pragma once
#include "BaseApplication.h"
#include <deque>

class GridNode;
class Grid;

class Agent
{
private:	
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height;						// height the character should be moved up
	float scale;						// scale of character from original model
	bool active;

	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};

	Ogre::AnimationState* mAnims[13];		// master animation list
	AnimID mBaseAnimID;						// current base (full- or lower-body) animation
	AnimID mTopAnimID;						// current top (upper-body) animation
	bool mFadingIn[13];						// which animations are fading in
	bool mFadingOut[13];					// which animations are fading out
	//Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	void setupAnimations();					// load this character's animations
	void fadeAnimations(Ogre::Real deltaTime);				// blend from one animation to another
	void updateAnimations(Ogre::Real deltaTime);			// update the animation frame
	
	GridNode* myNode;						//Agents node in the world
	Grid* grid;								//grid world
	
	// for locomotion
	Ogre::Real mDistance;					// The distance the agent has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
		// The list of points we are walking to
	Ogre::Real mWalkSpeed;					// The speed at which the object is moving
	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);			// update the character's walking
	void printPathToFile();	  //Prints the path the orge moved to file (its just like the print in grid but it clears)

	bool procedural;						// Is this character performing a procedural animation

public:

	Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale, Grid* g,GridNode* node);
	~Agent();
		
	float mHealth;		//indicates the status of the agent
	Ogre::Real mTimer;	
	std::deque<Ogre::Vector3> mWalkList;

	double howFar(Agent* g);
	void adjustSpeed(Ogre::Real r);					// increment/decrement speed
	void setPosition(float x, float y, float z);	// set position
	Ogre::Vector3 getPosition();					// get position of agent
	
	void update(Ogre::Real deltaTime);				// update the agent
	void moveTo(GridNode* g);						// Moves agent
	void moveAgent(std::list<GridNode*> gn);		// moves agent the shortest path

	void setBaseAnimation(AnimID id, bool reset = false);	// choose animation to display
	void setTopAnimation(AnimID id, bool reset = false);
	
	void toggleVisibility(bool b);	// toggle agent invisible or visible
	bool isVisible();				// check if agent is visible
	
	float checkHealth() { return mHealth; }	// check agent's health

	bool intersects(Ogre::Entity* e);		// agent intersects with an entity
	
	bool isActive() { return active; }
	void toggleActive(bool b);				// toggle agent to active or inactive state

};