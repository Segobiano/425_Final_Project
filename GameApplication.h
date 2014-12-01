#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "Agent.h"
#include "Tower.h"

class Tower;

class GameApplication : public BaseApplication
{
private:
	int restartR;
	int restartC;

	Grid* grid;
	GridNode* selectedNode;
	
	Ogre::Entity* floor;
	std::list<Agent*>::iterator iterAgent;
	std::list<GridNode*> moveAround;
	
	Ogre::SceneNode* mSelected;
	Ogre::SceneNode* mTrap;
	std::vector<Ogre::SceneNode*> mNumList;	
	
	int selectedR;
	int selectedC;
	double timecall;
	
	Agent* agent;						// store a pointer to the character
	std::list<Agent*> agentList;		// Lecture 5: now a list of agents
	std::list<Agent*> invisList;		// put agents in this list once they are dead or reach the end
	std::list<Agent*>::iterator iterBB;	
	std::list<Agent*>::iterator nextAgent;	//next agent to activate
	std::list<Tower*> towerList;		// a list of towers! 

	int mLives;		//these values need to be initialized still in .cpp if they are going to be used
	int mWalls;
	int mWaypoint;
	int wAgent;
	int round;

	float counter;	//timer determines the release of the next agent
	float cash;

	int row;
	int col;

	std::list<Ogre::SceneNode*> wpNodes;
	int Wpnum;

	enum GameState
	{
		GAME_LOAD,
		GAME_MENU,
		GAME_BUILD,
		GAME_RUNNING,
		GAME_END
	};

	GameState State;		//the game state
	
	GridNode* start;
	GridNode* goal;

	//GUI 
	OgreBites::Slider* mSampleSlider;		
	OgreBites::ParamsPanel* mStartPanel;
	OgreBites::ParamsPanel* mStatusPanel;
	OgreBites::Button* initBuildPhaseB;
	OgreBites::Button* initCreepPhaseB;
	OgreBites::Button* endGameB;

	//camera
	Ogre::Vector3 vTranslate;	// used to translate camera according to mouse pos

	void createBoard();

	void loadEnv();			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters
	void toggleState(GameState s);	//toggle game to change states
	//void createStatusGUI();	// creat a GUI to display agent status
	
public:
    GameApplication(void);			//constructor
    virtual ~GameApplication(void);	//deconstructor

	//GUI callbacks
	virtual void createGUI(void); // Lecture 16
    virtual void buttonHit(OgreBites::Button* b); // Lecture 16
	virtual void sliderMoved(OgreBites::Slider* s);

	void addTime(Ogre::Real deltaTime);		// update the game state

	//////////////////////////////////////////////////////////////////////////
	// Lecture 4: keyboard interaction
	// moved from base application
	// OIS::KeyListener
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	////////////////////////////////////////////////////////////////////////////

protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
