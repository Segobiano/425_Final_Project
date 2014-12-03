#include "GameApplication.h"
#include "Grid.h" // Lecture 5
#include <fstream>
#include <sstream>
#include <map> 
#include <time.h>
#include "movableText.h"

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void)
{
	grid = NULL;
	agent = NULL; // Init member data

	mSelected = NULL;
	selectedNode = NULL;

	start = NULL;
	goal = NULL;

	Wpnum = 0;

	vTranslate = Ogre::Vector3::ZERO;

	mLives = 1;
	mWalls = 0;
	mWaypoint = 0; 
	round = 1;
	
	cash = 50.0;
	counter = 0.0;
}
//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (agent != NULL)  // clean up memory
		delete agent; 
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv();
	setupEnv();
	loadObjects();
	loadCharacters();
	//createGUI();
	
}
//////////////////////////////////////////////////////////////////
// Lecture 5: Returns a unique name for loaded objects and agents
std::string getNewName() // return a unique name 
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "object_" + s;	// append the current count onto the string
}

// Lecture 5: Load level from file!
void // Load the buildings or ground plane, etc
GameApplication::loadEnv()
{
	using namespace Ogre;	// use both namespaces
	using namespace std;
	row = 15;
	col = 15;
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), row*NODESIZE, col*NODESIZE, row, col, true, 1, row, col, Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName("Examples/floord");
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);
	//mSceneMgr->setSkyDome(true, "Examples/SpaceSkyPlane", 5, 8);
	mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

	grid = new Grid(mSceneMgr, col, row);

	Ogre::Entity* ent = mSceneMgr->createEntity("test", "geosphere4500.mesh");
	ent->setMaterialName("Examples/Grass");
	//Examples/Chrome
	ent->setCastShadows(false);
	mSelected = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mSelected->attachObject(ent);
	mSelected->scale(0.1f,0.1f,0.1f); // cube is 100 x 100
	mSelected->setPosition(grid->getPosition(0,0).x, 1.0f, grid->getPosition(0,0).z);
	mSelected->setVisible(false,true);

	ent2 = mSceneMgr->createEntity("trap", "geosphere4500.mesh");
	ent2->setMaterialName("Examples/Hilite/Yellow");
	//Examples/Chrome
	ent2->setCastShadows(false);
	
	mTrap = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTrap->attachObject(ent2);
	mTrap->scale(0.3f,0.02f,0.3f); // cube is 100 x 100
	mTrap->setPosition(grid->getPosition(0,0).x, 20.0f, grid->getPosition(0,0).z);
	mTrap->setVisible(false,true);
	//mTrap->showBoundingBox(true);
}

void // Set up lights, shadows, etc
GameApplication::setupEnv()
{
	using namespace Ogre;

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// disable default camera control so the character can do its own 
	mCameraMan->setStyle(OgreBites::CS_FREELOOK); // CS_FREELOOK, CS_ORBIT, CS_MANUAL

	// use small amount of ambient lighting
	mSceneMgr->setAmbientLight(ColourValue(0.5f, 0.5f, 0.5f));

	// add a bright light above the scene
	Light* light = mSceneMgr->createLight();
	light->setType(Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(ColourValue::White);

}

void // Load other props or objects
GameApplication::loadObjects()
{}

void // Load actors, agents, characters
GameApplication::loadCharacters()
{}

//toggle game into a new game state
//and apply the appropiate gui for that state\
//GAME_LOAD,GAME_MENU,GAME_BUILD,GAME_RUNNING,GAME_END
void
GameApplication::toggleState(GameState s)
{
	State = s;
	if (State == GAME_MENU)	//first state
	{
		mTrayMgr->showAll();
		mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMLEFT)->hide();		//show only the center tray for start menu
	}
	else if (State == GAME_LOAD)
	{
		createBoard();		//generate walls, spheres, and grid
		//createStatusGUI();	//issues with an agent health gui right now, don't use
	}
	else if (State == GAME_BUILD)
	{
		vTranslate = Ogre::Vector3::ZERO;	//stop camera movement
		
		mTrap->setVisible(false);
		mTrapVis=false;
		//turns on those numbers of waypoints
		for (int p = 0; p < mNumList.size(); p++)
			mNumList[p]->setVisible(true);
		cash += 15;
		wAgent = 0;

		//toggle overview with camera with fixed position and showing grid outlines
		mCamera->setPosition(Ogre::Vector3(-150,1000,0));
		mCamera->lookAt(-150,0,-10);

		mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();	//hide the menu, by hiding the center tray
		mTrayMgr->getTrayContainer(OgreBites::TL_TOPLEFT)->show();	//status panel for debugging info
		mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMLEFT)->show();	//reveal top applicable trays
		mStatusPanel->show();
		initCreepPhaseB->show();
		//endGameB->show();	
	}
	else if (State == GAME_RUNNING)
	{
		//turns off the numbers
		for (int p = 0; p < mNumList.size(); p++){
			mNumList[p]->setVisible(false);
		}

		moveAround = grid->buildPath();		//build a path to destination
		nextAgent = agentList.begin();		//point to first agent to walk
		mSelected->setVisible(false,true);
		mTrap->setVisible(false,true);
		initCreepPhaseB->hide();
		//
		//adjusting the camera for after the build
		mCamera->setPosition(0, 600, 300);

		//mCamera->lookAt(grid->getPosition(grid->waypoints.front()->getRow(),
		//								  grid->waypoints.front()->getColumn()));
		mCamera->lookAt(grid->getPosition(7,7));
		
		//spawn agents at spheres
		// set up agent's movement
		//std::list<GridNode*> moveAround = grid->buildPath();
		std::list<Agent*>::iterator iterA;
		for (iterA = agentList.begin(); iterA != agentList.end(); iterA++)
			if (*iterA != NULL)
				(*iterA)->moveAgent(moveAround);
	}
	else if (State == GAME_END)
	{
		//show game end menu 
		//closing the end menu will close the game
		Ogre::String credits;
		credits += "Maze Tower Defense\n";
		credits += "Authors: Brad Segobiano and Brandon Shea\n";
		credits += "This game was made for educational purposes, don't sue us!\n";
		credits += "Tower Art from nazzyc at http://opengameart.org/content/sensorradar-tower \n";
		credits += "Music from Blizzard's Starcraft 1: Brood War\n";
		credits += "Special Thanks to Professor Allbeck! ;D\n";
		credits += "Hit OK to exit.";
		mTrayMgr->showOkDialog("Game Over!", credits);
	}
}

void
GameApplication::addTime(Ogre::Real deltaTime)
{	
	if (State == GAME_BUILD)
	{
		//draw openGL lines showing path
	}
	else if (State == GAME_RUNNING)
	{
		//the case they all died or got to the end
		//complete a round
		if (agentList.size() == 0){
			agentList = invisList;
			invisList.clear();
			round++;
			toggleState(GAME_BUILD);
			//here we check if the player life > 0 or round is 20
		}
		if (nextAgent != agentList.end())
		{
			counter += deltaTime;
			if (counter >= 5.0)	//release agent every period of time
			{
				//activate next agent
				(*nextAgent)->toggleActive(true);
				(*nextAgent)->toggleVisibility(true);
				(*nextAgent)->mHealth = round * 15;		//increment health each round
				//(*nextAgent)->adjustSpeed(1.5);			//increment speed each round
				nextAgent++;
				counter = 0.0;
			}
		}
		int gui_count = 0;
		std::list<Agent*>::iterator iterA = agentList.begin();
		while (iterA != agentList.end())
		{
			if ((*iterA) != NULL)
			{
				if ((*iterA)->isActive())
				{
					//trap is visible and agent is on it
					if (mTrapVis&&(*iterA)->intersects(ent2))
					{
						(*iterA)->halfspeed(); //this doesn't allow increasing the speed of the agent each round :(
					}
					else
					{
						(*iterA)->fullspeed();
					}
					//update agent's status panel (Not working)
					//mStatusPanel->setParamValue(gui_count,  Ogre::StringConverter::toString((*iterA)->mHealth) + "%");

					if ((*iterA)->checkHealth() <= 0)	//killed an agent
					{
						cash += 10;	//earn cash for the kill
						(*iterA)->mWalkList.clear(); 
						(*iterA)->toggleVisibility(false);
						(*iterA)->toggleActive(false);
						(*iterA)->setPosition(	grid->getPosition(restartR,restartC)[0],
												grid->getPosition(restartR,restartC)[1],
												grid->getPosition(restartR,restartC)[2]	);
						invisList.push_back((*iterA));
						agentList.erase(iterA++);		//remove and increment
					}
					else if ((*iterA)->mWalkList.empty())	//agent made it to goal
					{
						mLives--;
						(*iterA)->toggleVisibility(false);
						(*iterA)->toggleActive(false);
						(*iterA)->setPosition(	grid->getPosition(restartR,restartC)[0],
												grid->getPosition(restartR,restartC)[1],
												grid->getPosition(restartR,restartC)[2]	);
						invisList.push_back((*iterA));
						agentList.erase(iterA++);		//remove and increment
					}
					else
					{
						(*iterA)->update(deltaTime);
						iterA++;
					}
				}
				else iterA++;
			}
		gui_count++;
		}	// end agent update

		//update projectiles

		//update traps

		//update towers
		if (!agentList.empty())	//have to check agent list since it is passed in to tower.update()
		{
			std::list<Tower*>::iterator iterT;
			for (iterT = towerList.begin(); iterT != towerList.end(); iterT++)
				if (*iterT != NULL)
					(*iterT)->update(deltaTime, agentList);
		}
		if (mLives <= 0)	//out of lives, end game
		{
			std::cout << "You lose" << std::endl;
			toggleState(GAME_END);
		}
	}
	//update status window
	mStatusPanel->setParamValue(0, Ogre::StringConverter::toString(round));
	mStatusPanel->setParamValue(1, Ogre::StringConverter::toString(mLives));
	mStatusPanel->setParamValue(2, "$" + Ogre::StringConverter::toString(cash));

	//update camera
	mCamera->move(vTranslate);
}

bool 
GameApplication::keyPressed( const OIS::KeyEvent &arg ) // Moved from BaseApplication
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
	/*///this breaks if we allow G press
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }*/
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	else if (arg.key == OIS::KC_B)	//build a test tower
	{
		if (selectedNode && selectedNode->isClear() && grid->blockCheck(selectedR, selectedC))
		{
			int cost = 4;
			if (cash < cost)
				std::cout << "You need more cash!" << std::endl;
			else
			{
				int static count = 0;
				count++;
				Tower* t = new Tower(	mSceneMgr, grid, selectedNode, 
										"Tower" + Ogre::StringConverter::toString(count), 
										"sensor.mesh" , 18.0, 6, this	);
				towerList.push_back(t);
				cash -= cost;
			}
		}
		else
			std::cout << "Invalid Space" << std::endl;
	}
	else if (arg.key == OIS::KC_U) //upgrade tower
	{
		if (selectedNode && (selectedNode->getTower() != NULL))	//upgrade selected tower if there is one
		{
			Tower* selected_tower = selectedNode->getTower();
			int cost = 15 * (selected_tower->getLevel() + 1);
			if (cash < cost) 
				std::cout << "You need more cash!" << std::endl;
			else
			{
				selected_tower->levelUp();
				cash -= cost;
			}
		}
		else
			std::cout << "Invalid Upgrade" << std::endl;
	}
   
    mCameraMan->injectKeyDown(arg);
    return true;
}

bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;

	// use a RTS style camera
	// by updating a translation matrix to use in addTime()
	if (State == GAME_RUNNING)	
	{
		
		Ogre::Vector3 mousePos;
		mousePos.x = arg.state.X.abs;
		mousePos.y = arg.state.Y.abs;
		mousePos.z = arg.state.Z.abs;

		//get dimensions of window, mins are 0
		unsigned int max_x = mWindow->getWidth();
		unsigned int max_y = mWindow->getHeight();

		//move camera x direction
		if (mousePos.x >= max_x - 15)	{ vTranslate[0] =  8; }
		else if (mousePos.x <= 15)		{ vTranslate[0] = -8; }
		else vTranslate[0] = 0;
		//move camera z direction
		if (mousePos.y >= max_y - 15)	{ vTranslate[2] =  8; }
		else if (mousePos.y <= 15)		{ vTranslate[2] = -8; }
		else vTranslate[2] = 0;
		
		//mCamera->move(trans_vec); //move camera
		//move camera in addTime with updated translation matrix
	}
    return true;
}

bool GameApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (mTrayMgr->injectMouseDown(arg, id)) return true;
	//Clicking code
	
	if (State == GAME_BUILD){
		
		Ogre::Vector3 mousePos;
		mousePos.x = arg.state.X.abs;
		mousePos.y = arg.state.Y.abs;
		mousePos.z = arg.state.Z.abs;
 
		//then send a raycast straight out from the camera at the mouse's position
		Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.x/float(arg.state.width), mousePos.y/float(arg.state.height));
		//mRayScnQuery->setRay(mouseRay);
		Ogre::AxisAlignedBox objBox = floor->getWorldBoundingBox();
		std::pair<bool,Ogre::Real> inter = mouseRay.intersects(objBox);
		Ogre::Vector3 point = mouseRay.getPoint(inter.second);
		if (!inter.first){
			return true;
		}
		int halfsize = (NODESIZE*15)/2;
		point[0] += halfsize;
		//point[1]-=halfsize;
		point[2] += halfsize;
		Ogre::Vector3 result=point/NODESIZE;
		
		//select gridNode according to mouse click
		int indexCol = abs((int)result[0]);
		int indexRow = abs((int)result[2]);
		mSelected->setPosition(grid->getPosition(indexRow,indexCol).x, 1.0f, grid->getPosition(indexRow,indexCol).z);
		mSelected->setVisible(true,true);
		selectedNode = grid->getNode(indexRow, indexCol);
		selectedR = indexRow;
		selectedC = indexCol;
		//std::cout<<"row "<<indexRow<< "  " <<"col "<<indexCol<<std::endl;
		return true;
	}
	if (State == GAME_RUNNING&&!mTrapVis){
		mTrapVis=true;
		Ogre::Vector3 mousePos;
		mousePos.x = arg.state.X.abs;
		mousePos.y = arg.state.Y.abs;
		mousePos.z = arg.state.Z.abs;
		
 
		//then send a raycast straight out from the camera at the mouse's position
		Ogre::Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.x/float(arg.state.width), mousePos.y/float(arg.state.height));
		//mRayScnQuery->setRay(mouseRay);
		Ogre::AxisAlignedBox objBox = floor->getWorldBoundingBox();
		std::pair<bool,Ogre::Real> inter = mouseRay.intersects(objBox);
		Ogre::Vector3 point = mouseRay.getPoint(inter.second);
		if (!inter.first)
		{
			return true;
		}
		point[1]=10;
		mTrap->setPosition(point);
		mTrap->setVisible(true,true);
	}
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

// set up most gui's for the game. 
void GameApplication::createGUI(void)
{
	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 16
	if (mTrayMgr == NULL) return;
	using namespace OgreBites;

	Ogre::StringVector items;

	mTrayMgr->showLogo(OgreBites::TL_CENTER);
	//start up panel
	items.push_back("\t\t\t\t\tMaze Tower Defense");
	mStartPanel = mTrayMgr->createParamsPanel(OgreBites::TL_CENTER, "InfoPanel", 400, items);
	
	//status panel for agent health is built in createStatusGUI(),
	//since agents aren't built in until creatBoard() is finished
	//using panel for game status instead for now
	items.clear();
	items.push_back("Round ");
	items.push_back("Lives: ");
	items.push_back("Cash: ");
	items.push_back("Mouse click to select");
	items.push_back("Press B to build");
	items.push_back("Press U to upgrade");
	mStatusPanel = mTrayMgr->createParamsPanel(OgreBites::TL_TOPLEFT, "Status", 250, items);

	//power slider set up
    mSampleSlider = mTrayMgr->createThickSlider(TL_CENTER, "Lives", "Lives", 400, 80, 0, 0, 0);
	mSampleSlider->setRange(1,50,50);
	mSampleSlider->setValue(20);
	mTrayMgr->sliderMoved(mSampleSlider);

	mSampleSlider = mTrayMgr->createThickSlider(TL_CENTER, "Walls", "Walls", 400, 80, 0, 0, 0);
	mSampleSlider->setRange(0,50,51);
	mSampleSlider->setValue(20);
	mTrayMgr->sliderMoved(mSampleSlider);

	mSampleSlider = mTrayMgr->createThickSlider(TL_CENTER, "Waypoints", "Waypoints", 400, 80, 0, 0, 0);
	mSampleSlider->setRange(2,5,4);
	mSampleSlider->setValue(3);
	mTrayMgr->sliderMoved(mSampleSlider);

	initBuildPhaseB = mTrayMgr->createButton(TL_CENTER, "Build", "Build", 120.0);		//this button toggles the build phase

	initCreepPhaseB = mTrayMgr->createButton(TL_BOTTOMLEFT, "Creep", "Creep", 120.0);		//don't show this one yet, its for build phase

	//endGameB = mTrayMgr->createButton(TL_TOPRIGHT, "End", "End", 120.0);				//button to end game prematurely

	toggleState(GAME_MENU);
	//mTrayMgr->showAll();

	//////////////////////////////////////////////////////////////////////////////////
}
////////////////////////////////////////////////////////////////////
// Lecture 16
// Callback method for buttons
void GameApplication::buttonHit(OgreBites::Button* b)
{
	if (b->getName() == "Build")
	{
		//this needs to get included Winmm.lib
		std::string path = __FILE__;
		 path = path.substr(0,1+path.find_last_of('\\')); 
		 path+= "\\Sounds\\SCTheme.wav"; 
		 PlaySound(path.c_str(), NULL, SND_LOOP | SND_ASYNC);   
		 //Credit for the music goes to Blizzard Entertainment  from Starcraft 1 

		toggleState(GAME_LOAD);		//load level
		
		toggleState(GAME_BUILD);	//set game into build mode		
	}
	else if (b->getName() == "Creep")
	{
		//std::cout << "This button will activate creep mode. " << std::endl;
		toggleState(GAME_RUNNING);
	}
}

// Callback method for sliders
void GameApplication::sliderMoved(OgreBites::Slider* s)
{
	if (s->getName() == "Lives")
	{
		mLives = s->getValue();
	}
	if (s->getName() == "Walls")
	{
		mWalls = s->getValue();
	}
	if (s->getName() == "Waypoints")
	{
		mWaypoint = s->getValue();
	}
}

// create a board based on user input from the sliders
// Brandon the refactor is monster was here...
void GameApplication::createBoard(){
	Ogre::StaticGeometry* sgNode;	//used to make walls static objects for faster rendering
	//Ogre::MovableText* msg;			//text at each waypoint
	Ogre::Entity* ent;				//used to create entities at certain points
	Ogre::SceneNode* mNode;			//
	Ogre::SceneNode* mTran;			//
	Ogre::SceneNode* mTree;			//
	wAgent=0;

	// set up waypoints
	for (int i = 0; i < mWaypoint; i++){
		float x, y;
		//keeps looking for empty
		while (true)
		{
			srand(time(NULL));
			x = rand() % grid->getRow();
			y = rand() % grid->getCol();
			if (grid->getNode(x,y)->isClear())
					break;
		}
		if (i == 0)
		{
			restartR = x;
			restartC = y;
		}
		grid->getNode(x,y)->setOccupied(); 
		grid->getNode(x,y)->wp = true;
		ent = mSceneMgr->createEntity("waypoint" + i, "geosphere4500.mesh");
		ent->setMaterialName("Examples/TextureEffect3");
		//Examples/Chrome
		ent->setCastShadows(false);
		mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mNode->attachObject(ent);
		mNode->scale(0.1f, 0.01f, 0.1f); // cube is 100 x 100
		mNode->setPosition(grid->getPosition(x,y).x, 1.0f, grid->getPosition(x,y).z);
		//mNode->setVisible(false,false);
		//mNode->showBoundingBox(true);
		grid->waypoints.push_back(grid->getNode(x,y));

		//All the numbers
		ent = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
		ent->setMaterialName("Examples/floor" + Ogre::StringConverter::toString(i));

		mTran = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mTran->setInheritScale(false);
		mTran->attachObject(ent);
		mTran->scale(0.5f, 0.5f, 0.5f); // cube is 100 x 100
		mTran->setPosition(grid->getPosition(x,y).x, 10.0f, grid->getPosition(x,y).z);
		mNumList.push_back(mTran);

	}

	// set up walls
	mTree = mSceneMgr->getRootSceneNode()->createChildSceneNode("nTree");
	for (int i = 0; i < mWalls; i++)
	{
		float x, y;
		//random grid location on the board not already a waypoint
		while (true)
		{
			x = rand() % grid->getRow();
			y = rand() % grid->getCol();
			
			if (grid->getNode(x,y)->isClear())
			{ 
				//grid->getNode(x,y)->setOccupied(); 
				if (grid->buildCheck(x,y))
					break;	//A break?!?! Whyyyyyyy D:
				//grid->getNode(x,y)->setClear(); 
			}
		}
		ent = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
		ent->setMaterialName("2 - Default");
		//Examples/Rockwall
		//"2 - Default"
		//Examples/BumpyMetal
		//Examples/SceneCubeMap1
		ent->setCastShadows(false);
		mNode = mSceneMgr->getSceneNode("nTree")->createChildSceneNode();
		mNode->attachObject(ent);
		mNode->scale(0.5f, 0.2f, 0.5f); // cube is 100 x 100
		grid->getNode(x,y)->setOccupied();  // indicate that agents can't pass through // cube is 100 x 100
		mNode->setPosition(grid->getPosition(x,y).x, 10.0f, grid->getPosition(x,y).z);
	}

	//Static geomerty
	sgNode = mSceneMgr->createStaticGeometry("StaticTree");
	sgNode->addSceneNode(mSceneMgr->getSceneNode("nTree"));
	sgNode->build();
	mTree->removeAndDestroyAllChildren();

	for (int i = 0; i < (grid->waypoints.size()); i++)
		grid->waypoints[i]->setClear();

	grid->printToFile();

	//build agents
	for (int i = 0; i < 5; i++)
	{
		Agent* a = new Agent(mSceneMgr, "Enemy"+i, "sinbad.mesh", 30, 5, grid,grid->waypoints.front());
		a->toggleVisibility(false);
		agentList.push_back(a);
	}
	
	mTrapVis=false;
}


////////////////////////////////////////////////////////////////////
// Callback for the Game Over menu's OK button to close game
void 
GameApplication::okDialogClosed(const Ogre::DisplayString& message)
{
	mTrayMgr->closeDialog();
	mShutDown = true;
}



// create the agent's status gui, which will display their health
//void
//GameApplication::createStatusGUI()
//{
//	Ogre::StringVector items;			//establishes what is in the parameter gui
//	std::list<Agent*>::iterator iterA;
//	int count = 0;
//
//	for (iterA = agentList.begin(); iterA != agentList.end(); iterA++)
//		items.push_back("Agent " +  Ogre::StringConverter::toString(count++));
//
//	mStatusPanel = mTrayMgr->createParamsPanel(OgreBites::TL_TOPLEFT, "Agent Status", 200, items);
//	count = 0;
//	for (iterA = agentList.begin(); iterA != agentList.end(); iterA++)
//		mStatusPanel->setParamValue(count++, "N/A");
//}

