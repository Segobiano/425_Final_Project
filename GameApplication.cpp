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

	Wpnum=0;

	vTranslate = Ogre::Vector3::ZERO;

	mLives = 1;
	mWalls = 0;
	mWaypoint = 0;
	
	cash = 0.0;
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
	row=15;
	col=15;
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), row*NODESIZE, col*NODESIZE, row, col, true, 1, row, col, Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName("Examples/GrassFloor");
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);
	mSceneMgr->setSkyDome(true, "Examples/SpaceSkyPlane", 5, 8);

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
{
	// Brandon Note: Don't think we'll use this method, since we place towers, and spheres are loaded elsewhere
}

void // Load actors, agents, characters
GameApplication::loadCharacters()
{
	// Brandon Note: will need to load an agent at each sphere/waypoint
	// agent = new Agent(this->mSceneMgr, "Sinbad", "Sinbad.mesh");

}

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
		mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->hide();		//show only the center tray for start menu
		mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMLEFT)->hide();
	}
	else if (State == GAME_LOAD)
	{
		createBoard();		//generate walls, spheres, and grid

		mTrayMgr->getTrayContainer(OgreBites::TL_BOTTOMLEFT)->show();	//status panel for debugging info
		mStatusPanel->show();

	}
	else if (State == GAME_BUILD)
	{
	 
		wAgent=0;
		//toggle overview with camera with fixed position and showing grid outlines

		mCamera->setPosition(Ogre::Vector3(0,1000,0));
		
		mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();	//hide the menu, by hiding the center tray
		mTrayMgr->getTrayContainer(OgreBites::TL_TOPRIGHT)->show();	//reveal top applicable trays
		initCreepPhaseB->show();
		endGameB->show();	
		mCamera->lookAt(10,0,0);
	}
	else if (State == GAME_RUNNING)
	{
		
		moveAround=grid->buildPath();
		nextAgent = agentList.begin();

		mSelected->setVisible(false,true);
		initCreepPhaseB->hide();

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
		
		//activate towers

	}
	else if (State == GAME_END)
	{
		//clear agent list
		//clear towers
		//clear projectiles
		//clear spheres
		//show game end menu GUI
	}
}

void
GameApplication::addTime(Ogre::Real deltaTime)
{	
	if (State == GAME_BUILD)
	{

		//draw lines showing path
		
	}
	else if (State == GAME_RUNNING)
	{
		//the case they all died or not
		if (agentList.size()==0){
			toggleState(GAME_BUILD);
			agentList=invisList;
			invisList.empty();
			//here we check if the player life >0 or round is 20
		}
		if (nextAgent != agentList.end())
		{
			counter += deltaTime;
			if (counter >= 5.0)	//release agent every period of time
			{
				//activate next agent
				(*nextAgent)->toggleActive(true);
				(*nextAgent)->toggleVisibility(true);
				nextAgent++;
				counter = 0.0;
			}
		}
		std::list<Agent*>::iterator iterA = agentList.begin();
		while (iterA != agentList.end())
		{
			if ((*iterA) != NULL)
			{
				if ((*iterA)->isActive()){
					if ((*iterA)->checkHealth() <= 0)	//killed an agent
					{
						cash += 100; //earn cash for the kill
						(*iterA)->toggleVisibility(false);	//hide it
						(*iterA)->toggleActive(false);	//hide it
						invisList.push_back((*iterA));
						(*iterA)->setPosition(grid->getPosition(restartR,restartC)[0],grid->getPosition(restartR,restartC)[1],grid->getPosition(restartR,restartC)[2]);
						agentList.erase(iterA++);		//remove and increment
						
					}
					else if ((*iterA)->mWalkList.empty())	//agent made it to goal
					{
						mLives--;
						(*iterA)->toggleVisibility(false);	//hide it
						(*iterA)->toggleActive(false);	//hide it
						invisList.push_back((*iterA));
						(*iterA)->setPosition(grid->getPosition(restartR,restartC)[0],grid->getPosition(restartR,restartC)[1],grid->getPosition(restartR,restartC)[2]);
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
		}
		//update projectiles

		//update traps

		//update towers
		if (!agentList.empty())	//have to check since it is passed in to tower.update()
		{
			std::list<Tower*>::iterator iterT;
			for (iterT = towerList.begin(); iterT != towerList.end(); iterT++)
				if (*iterT != NULL)
					(*iterT)->update(deltaTime, agentList);
		}
		//NOTE: orb aren't disapearing because of above chunk
		//keeps towers from updating if there are no agents.

		if (mLives <= 0)	//out of lives, end game
		{
			std::cout << "You lose" << std::endl;
			toggleState(GAME_END);
		}

	}
	//update debug status window
	mStatusPanel->setParamValue(0, Ogre::StringConverter::toString(State));
	mStatusPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getPosition()));

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
    }
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
	else if (arg.key == OIS::KC_SPACE)
    {
		//std::list<GridNode*> moveAround=grid->buildPath();

		//std::list<Agent*>::iterator iterA;
		//for (iterA = agentList.begin(); iterA != agentList.end(); iterA++)
		//	if (*iterA != NULL)
		//		(*iterA)->moveAgent(moveAround);
				//(*iterA)->mWalkList=moveAround;
    }
	else if (arg.key == OIS::KC_EQUALS)	//build a test tower
	{
		if (selectedNode && selectedNode->isClear()&&grid->blockCheck(selectedR,selectedC))
		{
			int static count = 0;
			count++;
			Tower* t = new Tower(	mSceneMgr, grid, selectedNode, 
									"Tower" + Ogre::StringConverter::toString(count), 
									"sensor.mesh" , 18.0, 6, this	);
			towerList.push_back(t);
		}
		else
			std::cout << "Invalid Space" << std::endl;
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
	//if (State != GAME_MENU && State != GAME_BUILD)
	//	mCameraMan->injectMouseMove(arg);
	if (State == GAME_RUNNING)	// use a RTS style camera
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
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

// set up ALL gui's for the game. 
void GameApplication::createGUI(void)
{
	//////////////////////////////////////////////////////////////////////////////////
	// Lecture 16
	if (mTrayMgr == NULL) return;
	using namespace OgreBites;

	mTrayMgr->showLogo(OgreBites::TL_CENTER);
	//start up panel
	Ogre::StringVector items;
	items.push_back("\t\t\t\t\tMaze Tower Defense");
	mStartPanel = mTrayMgr->createParamsPanel(OgreBites::TL_CENTER,"InfoPanel",400,items);

	//status panel for debugging
	items.clear();
	items.push_back("Current Mode: ");
	items.push_back("Cam: ");
	items.push_back("Selection: ");
	mStatusPanel = mTrayMgr->createParamsPanel(OgreBites::TL_BOTTOMLEFT,"Status",300,items);
	mStatusPanel->setParamValue(0, "N/A");
	mStatusPanel->setParamValue(1, Ogre::StringConverter::toString(Ogre::Vector3::ZERO));
	mStatusPanel->setParamValue(2, "N/A");

	//power slider set up
    mSampleSlider = mTrayMgr->createThickSlider(TL_CENTER, "Lives", "Lives", 400, 80, 0, 0, 0);
	mSampleSlider->setRange(50,100,51);
	mSampleSlider->setValue(100);
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

	initCreepPhaseB = mTrayMgr->createButton(TL_TOPRIGHT, "Creep", "Creep", 120.0);		//don't show this one yet, its for build phase

	endGameB = mTrayMgr->createButton(TL_TOPRIGHT, "End", "End", 120.0);				//button to end game prematurely

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
		//mTrayMgr->destroyAllWidgetsInTray(OgreBites::TL_CENTER);	//going to remove this, so we don't have to rebuild GUI each time
		toggleState(GAME_LOAD);		//load level
		
		toggleState(GAME_BUILD);	//set game into build mode
		//This moves the camera up
		
	}
	else if (b->getName() == "Creep")
	{
		std::cout << "This button will activate creep mode. " << std::endl;
		toggleState(GAME_RUNNING);
	}
}

// Callback method for sliders
void GameApplication::sliderMoved(OgreBites::Slider* s)
{
	if (s->getName()=="Lives")
	{
		mLives=s->getValue();
	}
	if (s->getName()=="Walls")
	{
		mWalls=s->getValue();
	}
	if (s->getName()=="Waypoints")
	{
		mWaypoint=s->getValue();
	}
}

// create a board based on user input from the sliders
// Brandon the refactor is monster was here...
void GameApplication::createBoard(){
	Ogre::StaticGeometry* sgNode;	//used to make walls static objects for faster rendering
	Ogre::MovableText* msg;			//text at each waypoint
	Ogre::Entity* ent;				//used to create entities at certain points
	Ogre::SceneNode* mNode;			//
	Ogre::SceneNode* mTran;			//
	Ogre::SceneNode* mTree;			//
	wAgent=0;
	for (int i = 0; i < mWaypoint; i++){
		float x, y;
		//keeps looking for empty
		while (true){
			srand(time(NULL));
			x = rand() % grid->getRow();
			y = rand() % grid->getCol();
			if (grid->getNode(x,y)->isClear()){ 
					break;
			}
		}
		if (i==0){
			restartR=x;
			restartC=y;
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

		msg = new Ogre::MovableText("TXT_001", Ogre::StringConverter::toString(i));
		msg->setTextAlignment(Ogre::MovableText::H_CENTER, Ogre::MovableText::V_ABOVE); // Center horizontally and display above the node
		msg->setCharacterHeight(10);
		msg->setSpaceWidth(50);

		mTran = mNode->createChildSceneNode();
		mTran->setInheritScale(false);
		mTran->translate(0, 40, 0);
		mTran->scale(5.0f, 5.0f, 5.0f); // cube is 100 x 100
		//msg->setAdditionalHeight( 2.0f ); //msg->setAdditionalHeight( ei.getRadius() ) // apparently not needed from 1.7*/
		mTran->attachObject(msg);
	}

	//
	mTree = mSceneMgr->getRootSceneNode()->createChildSceneNode("nTree");
	for (int i = 0; i < mWalls; i++){
		float x, y;
		//random grid location on the board not already a waypoint
		while (true){
			
			x = rand() % grid->getRow();
			y = rand() % grid->getCol();
			
			if (grid->getNode(x,y)->isClear()){ 
				//grid->getNode(x,y)->setOccupied(); 
				if (grid->buildCheck(x,y)){
					break;	//A break?!?! Whyyyyyyy D:
				}
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
		//std::cout<<Ogre::Math::RangeRandom(0,100);

	}
	//Static geomerty
	sgNode = mSceneMgr->createStaticGeometry("StaticTree");
	sgNode->addSceneNode(mSceneMgr->getSceneNode("nTree"));
	sgNode->build();
	mTree->removeAndDestroyAllChildren();

	for (int i = 0; i < (grid->waypoints.size()); i++){
		grid->waypoints[i]->setClear();
	}
	grid->printToFile();
	for (int i = 0; i < 5; i++)
	{
		Agent* a = new Agent(mSceneMgr, "Enemy"+i, "sinbad.mesh", 30, 5, grid,grid->waypoints.front());
		a->toggleVisibility(false);
		agentList.push_back(a);
	}


}

