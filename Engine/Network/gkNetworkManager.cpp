/*
-------------------------------------------------------------------------------
    This file is part of OgreKit.
    http://gamekit.googlecode.com/

    Copyright (c) 2006-2013 Charlie C.

    Contributor(s): Kai-Ting (Danil) Ko
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

#include "gkCommon.h"
#include "gkNetworkManager.h"
#include "gkNetworkInstance.h"
#include "gkEngine.h"
#include "gkUserDefs.h"
#include "gkDebugger.h"
#include "gkLogger.h"

///CLASS gkNetworkManager
/// New manager that implement two Instances now
gkNetworkManager::gkNetworkManager()
{
	mConnection=NULL;
	
	initialize();
}  // gkNetworkManager::gkNetworkManager

gkNetworkManager::~gkNetworkManager()
{
	close();
}  // gkNetworkManager::~gkNetworkManager

void gkNetworkManager::initialize(void)
{
	if (enet_initialize() != 0)
    {
		gkLogger::write("An error occurred while initializing ENet.\n");
    }  // if
}  // bool gkNetworkManager::initialize

void gkNetworkManager::close( )
{
	   if(mConnection!=NULL)
	   {
    //mServer->exit();
	delete mConnection;
	mConnection = NULL;
	   }
	 enet_deinitialize();
}  // void gkNetworkManager::initialize

void gkNetworkManager::createNetworkInstance(const int & pType, const gkString & pName, const gkString & pAddress, const int & pPort)
{
	 //If there is already an instance, a new one cannot be created until the existing one is removed
	if(mConnection != NULL)
	{
		gkLogger::write("Network Instance cannot be created since there is already an existing instance\n");
		return;
	} 

	// Only create if there is a non - empty name and a non - empty address
	if(pName.length() == 0 || pAddress.length() == 0)
	{
		gkLogger::write("Network Instance cannot be created due to invalid name and/or address\n");
		return;
	}  // if

	// Setup new connection
	if(pType == SERVER)
	{   mConnectionType=SERVER;
		mConnection = new gkNetworkServer(pName);
		mConnection -> setAddress(pAddress);
	    mConnection -> setPort(pPort);
	}  // if
	else if(pType == CLIENT)
	{   mConnectionType=CLIENT;
		mConnection = new gkNetworkClient(pName);
		mConnection -> setAddress(pAddress);
	    mConnection -> setPort(pPort);
	}
	

	
}  // void gkNetworkManager::startNetworkInstance


void gkNetworkManager::startNetworkInstance()
{
	
	 if(mConnection != NULL)
	 {
		mConnection -> start();
	 }
 
}  // void gkNetworkManager::startNetworkInstance


void gkNetworkManager::stopNetworkInstance()
{
	
	
	 if(mConnection != NULL)
	 {
		mConnection -> stop();
	 }
	  
	
}  // void gkNetworkManager::stopNetworkInstance

bool gkNetworkManager::isServer()
{
	if(mConnectionType==SERVER)return true;return false;
}

bool gkNetworkManager::isNetworkInstanceRunning()
{
	
	 if(mConnection != NULL)
	 {
		return mConnection -> isRunning();
	 }

	 return false; 
}  

void gkNetworkManager::deleteNetworkInstance()
{
	close();//todo : separate client and server treatment....
}  

void gkNetworkManager::sendMessage(
	const gkString & pSender, const gkString & pReceiver, 
	const gkString & pSubject, const gkString & pBody)
{
	
	 if(mConnection != NULL)
	 {
		mConnection ->sendMessage(pSender, pReceiver, pSubject, pBody);
	 }

}  

bool gkNetworkManager::isNetworkInstanceExists()
{
	
	 if(mConnection != NULL)
	 {
		return true;
	 }
	
	return false;
}  // bool isNetworkInstanceExists

UT_IMPLEMENT_SINGLETON(gkNetworkManager);
