/*
-------------------------------------------------------------------------------
    This file is part of OgreKit.
    http://gamekit.googlecode.com/

    Copyright (c) 2006-2010 Charlie C.

    Contributor(s): Ezee ( bruno C )
-------------------------------------------------------------------------------
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#include "gkGameLevel.h"
#include "gkGamePlayer.h"
#include "gkGameObjectManager.h"
#include "gkFontManager.h"
#include "OgreKit.h"
#include <gkNetworkManager.h>

#ifdef OGREKIT_CPPDEMO_COMPILE_BLEND
#include "Generated/gkInlineBlendFiles.inl"
#endif

#define LEVEL_GROUP_NAME "CppDemo"



gkGameLevel::gkGameLevel()
	:    m_pickup(0),
	     m_killThemAll(0),
	     m_level(GK_LEVEL_EXIT),
	     m_keyboard(0),
	     m_mouse(0),
		 m_player(0)
{
    //	client or server behavior
	NetType = networkType();
	if(NetType>=0)registerToNetwork();

	spawnRequested=false;
	canJoin=false;
	m_keyboard = gkWindowSystem::getSingleton().getKeyboard();
	m_mouse    = gkWindowSystem::getSingleton().getMouse();
	m_joy      = gkWindowSystem::getSingleton().getJoystick(0);

	gkGameObjectManager::getSingleton().addInstanceListener(this);
	gkGameObjectManager::getSingleton().addResourceListener(this);
	gkSceneManager::getSingleton().addInstanceListener(this);
	gkSceneManager::getSingleton().addResourceListener(this);
	gkGroupManager::getSingleton().addResourceListener(this);
	gkTextManager::getSingleton().addResourceListener(this);
	gkFontManager::getSingleton().addResourceListener(this);
}

bool gkGameLevel::isMultiplayer()
{
	return gkNetworkManager::getSingletonPtr()->isNetworkInstanceExists();
}

int gkGameLevel::networkType()
{
	if(gkNetworkManager::getSingletonPtr()->isServer())return 0;
	return 1;
}

void gkGameLevel::configureJoystick()
{
	if(!m_joy)return;
	
}

gkGameLevel::~gkGameLevel()
{
	delete m_player;
	m_player = 0;

	for (UTsize i = 0; i < m_enemies.size(); i++)
		delete m_enemies[i];
	m_enemies.clear();

	gkGameObjectManager::getSingleton().removeResourceListener(this);
	gkGameObjectManager::getSingleton().removeInstanceListener(this);
	gkSceneManager::getSingleton().removeInstanceListener(this);
	gkSceneManager::getSingleton().removeResourceListener(this);
	gkGroupManager::getSingleton().removeResourceListener(this);
	gkTextManager::getSingleton().removeResourceListener(this);
	gkFontManager::getSingleton().removeResourceListener(this);

	if (m_pickup)
		gkSceneManager::getSingleton().destroy(m_pickup->getResourceHandle());
}


void gkGameLevel::loadLevel(const gkLevel& level)
{
	m_level = level;
	if (m_level == GK_LEVEL_PICKUP)loadPickup();
	//client must say to server it's okay
	gkNetworkManager::getSingletonPtr()->sendMessage("gkGameLevel","server","JOIN",DONE_JOIN);
		           
}



void gkGameLevel::loadPickup(void)
{
	if (m_pickup)
	{
		m_pickup->reinstance();
		return;
	}

#ifndef OGREKIT_CPPDEMO_COMPILE_BLEND
	gkBlendFile* playerData = gkBlendLoader::getSingleton().loadFile(gkUtils::getFile(GK_RESOURCE_PLAYER), "", LEVEL_GROUP_NAME);
#else
	gkBlendFile* playerData = gkBlendLoader::getSingleton().loadFromMemory(MOMO,MOMO_SIZE,gkBlendLoader::LO_ALL_SCENES, "", LEVEL_GROUP_NAME);
#endif

	if (!playerData || !playerData->getMainScene())
	{
		gkPrintf("GameLevel: Blend '%s' loading failed", GK_RESOURCE_PLAYER);
		return;
	}


	// log status
	gkPrintf("GameLevel: Blend '%s' loaded", GK_RESOURCE_PLAYER);

#ifndef OGREKIT_CPPDEMO_COMPILE_BLEND
	gkBlendFile* mapData = gkBlendLoader::getSingleton().loadFile(gkUtils::getFile(GK_RESOURCE_MAPS), "Pickup", LEVEL_GROUP_NAME);
#else
	gkBlendFile* mapData = gkBlendLoader::getSingleton().loadFromMemory(MAPS,MAPS_SIZE,gkBlendLoader::LO_ALL_SCENES, "Pickup", LEVEL_GROUP_NAME);
#endif
	if (!mapData || !mapData->getMainScene())
	{
		gkPrintf("GameLevel: Blend '%s'->Pickup loading failed", GK_RESOURCE_MAPS);
		return;
	}

	// log status
	gkPrintf("GameLevel: Blend '%s'->Pickup loaded", GK_RESOURCE_MAPS);

	gkScene* sc = mapData->getMainScene();



	m_pickup = (gkScene*)gkSceneManager::getSingleton().create("PickupLevel");
	m_pickup->setHorizonColor(gkColor(0.2f, 0.2f, 0.2f));
	m_pickup->setAmbientColor(gkColor(0.5f, 0.5f, 0.5f));

	gkSceneManager::getSingleton().copyObjects(sc, m_pickup);

	m_player = new gkGamePlayer(this);
	m_player->load(playerData);
    m_player->registerToNetwork();
	

	m_pickup->createInstance();
/*
	gkGamePlayer* enemy = m_player->clone();
	enemy->registerToNetwork();
	enemy->setPosition(gkVector3(0,1,1));
	m_enemies.push_back(enemy);

	enemy = enemy->clone();
	enemy->registerToNetwork();
	enemy->setPosition(gkVector3(1,1,1));
	m_enemies.push_back(enemy);
	*/
}



void gkGameLevel::notifyInstanceCreated(gkInstancedObject* inst)
{
	gkLogMessage("GameLevel: Instanced -> " << inst->getResourceName().getName());
}

void gkGameLevel::notifyResourceCreated(gkResource* res)
{

	gkLogMessage(res->getManagerType() << ", " << res->getResourceType() <<
	             ":handle " <<  res->getResourceHandle() << ", created " << res->getResourceName().getName());
}


///---------------------------------------------------------------------------------------------------

bool gkGameLevel::spawnRequest()
{
	if(spawnRequested)
	{
		spawnRequested=false;//reinit
		return true;
	}
	return false;
}
///---------------------------------------------------------------------------------------------------

void gkGameLevel::spawn()
{
	gkGamePlayer* enemy = m_player->clone();
	enemy->setPosition(gkVector3(0,1,1));
	m_enemies.push_back(enemy);
}
///---------------------------------------------------------------------------------------------------

void gkGameLevel::tick(gkScalar delta)
{
	// update game states
	if (m_player)
	{	m_player->update(delta);
	
	}
		
    
	for (UTsize i = 0; i < m_enemies.size(); i++)
	m_enemies[i]->update(delta);
	

	if (m_keyboard->key_count > 0)
	{
		if (m_keyboard->isKeyDown(KC_ESCKEY))
			gkEngine::getSingleton().requestExit();
	}
}
///---------------------------------------------------------------------------------------------------

gkScene* gkGameLevel::getLevel(void)
{
	if (m_level == GK_LEVEL_PICKUP)
		return m_pickup;
	else if (m_level == GK_LEVEL_KILL_THEM_ALL)
		return m_killThemAll;
	return 0;
}

///---------------------------------------------------------------------------------------------------

void gkGameLevel::handleMessage(gkMessageManager::Message* message)
{
	
	/// FRAMEWORK :
	///Client ask to join a game first .
	///When he receives clearance and when successfully loaded the level
	// he sends a JOINED status , the server then spawn the client in his scene
	/// whil ethe client do the same for the server player .
	int subject=-1;
    if(message->m_subject.compare("JOIN")==0)
		subject=0;
	else if(message->m_subject.compare("MAP")==0)
		subject=1;
	else if(message->m_subject.compare("CHAT")==0)
		subject=3;
    //debug...
	printf("****** NETWORK MESSAGE RECEIVED   ********* \n");
	printf("FROM ' %s ' : SUBJECT : ' %s ' BODY : ' %s '\n",message->m_from.c_str(),message->m_subject.c_str(),message->m_body.c_str());
			    
	 //	client or server behavior
	switch(NetType)
	{
	 case SERVER:
		 switch(subject)
			{
				//JOIN
	          case 0:
				  //BODY READ
				  //case REQUEST_JOIN
				  if(message->m_body.compare(REQUEST_JOIN)==0)
				  {
				   gkNetworkManager::getSingletonPtr()->sendMessage("server","gkGameLevel","JOIN",CLEARANCE_JOIN);
		           break;
				  }
				   //case DONE_JOIN
				  if(message->m_body.compare(DONE_JOIN)==0)
				  {
					  //add the player
					  spawn();
				   break;
				  }
			}break;

     case CLIENT:switch(subject)
			{
				//JOIN
	         case 0://BODY READ
				  if(message->m_body.compare(CLEARANCE_JOIN)==0)
				  {
					  canJoin=true;
			          break;
				  }
				  break;

			 case 1:break;
			}break;

	}
printf("*************************************** \n");
}
