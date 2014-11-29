#include "Player.h";

Player::Player(std::string n)
{
	pLife=100;
	pName=n;
	pMoney=25;
}

Player::~Player()
{
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
}