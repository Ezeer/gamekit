/*
-------------------------------------------------------------------------------
    This file is part of OgreKit.
    http://gamekit.googlecode.com/

    Copyright (c) 2006-2010 Nestor Silveira.

    Contributor(s): none yet.
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

#include "gkTrackNode.h"
#include "gkEngine.h"
#include "gkGameObject.h"
#include "gkUtils.h"
#include "gkScene.h"
#include "OgreRoot.h"
#include "gkLogger.h"

using namespace Ogre;

gkTrackNode::gkTrackNode(gkLogicTree *parent, size_t id)
: gkLogicNode(parent, id),
m_target(0),
m_track(0),
m_RotationNode(0),
m_oldPosition(0, 0, 0),
m_oldPositionSet(false)
{
	ADD_ISOCK(*getEnable(), this, gkLogicSocket::ST_BOOL);
	ADD_ISOCK(*getOffset(), this, gkLogicSocket::ST_VECTOR);
	ADD_ISOCK(*getStiffness(), this, gkLogicSocket::ST_REAL);
	ADD_ISOCK(*getDamping(), this, gkLogicSocket::ST_REAL);
	ADD_ISOCK(*getTarget(), this, gkLogicSocket::ST_GAME_OBJECT);
	ADD_ISOCK(*getTrack(), this, gkLogicSocket::ST_GAME_OBJECT);

	getOffset()->setValue(gkVector3::ZERO);
	getStiffness()->setValue(0.8f);
	getDamping()->setValue(0.3f);
}

gkTrackNode::~gkTrackNode()
{
	if(m_RotationNode)
		delete m_RotationNode;
}

bool gkTrackNode::evaluate(Real tick)
{
	m_track = getTrack()->getValueGameObject();

	m_target = getTarget()->getValueGameObject();

	return m_track && m_track->isLoaded() && m_target && m_target->isLoaded() && getEnable()->getValueBool();
}

void gkTrackNode::update(Real tick)
{
	if(!m_RotationNode)
	{
		gkScene* pScene = gkEngine::getSingleton().getActiveScene();

		m_RotationNode = new Ogre::SceneNode(pScene->getManager());

		GK_ASSERT(m_RotationNode);

		m_RotationNode->setFixedYawAxis(true, Vector3::UNIT_Z);
	}

	const gkVector3& currPos = m_target->getPosition();

	const gkVector3& oPos = m_track->getPosition();

	m_RotationNode->resetOrientation();

	m_RotationNode->setDirection(oPos - currPos);

	m_target->setOrientation(m_RotationNode->getOrientation());

	gkVector3 targetPos = oPos + m_track->getOrientation() * getOffset()->getValueVector3();

	gkVector3 stretch = (currPos - targetPos) * getStiffness()->getValueReal();

	if(m_target->getAttachedBody())
	{
		gkVector3 dv = m_target->getLinearVelocity() * getDamping()->getValueReal();

		m_target->applyForce(-stretch - dv);

		m_target->setAngularVelocity(gkVector3::ZERO);
	}
	else
	{
		if(!m_oldPositionSet)
		{
			m_oldPosition = currPos;

			m_oldPositionSet = true;
		}

		gkVector3 damp = (currPos - m_oldPosition) * getDamping()->getValueReal();

		m_target->translate(-stretch * tick - damp);

		m_oldPosition = currPos;
	}
}

