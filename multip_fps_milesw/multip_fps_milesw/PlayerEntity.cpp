#include "PlayerEntity.hpp"

#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreVector3.h>
//#include <OGRE/SdkTrays.h>
#include <OGRE/OgreCamera.h>

#define ENTITY_Y_OFFSET 50

PlayerEntity::PlayerEntity(std::string OgreName, int a_NodeUID, Ogre::SceneManager& a_SceneMgr, Ogre::Camera* a_pCamera)
:	m_pMyEntity(NULL)
,	m_pMyNode(NULL)
,	m_SceneMgr(a_SceneMgr)
,	m_ForwardMove(0)
,	m_SideMove(0)
,	m_VertLook(0)
,	m_HorizLook(0)
,	m_EulerX(0)
,	m_EulerY(0)
,	m_EulerZ(0)
,	m_pCamera(a_pCamera)
,	m_CurGroundHeight(0)
,	m_FallVelocity(0)
,	m_NodeUID(a_NodeUID)
{
	m_MoveVector.normalise();
	//make an entity
	m_pMyEntity = m_SceneMgr.createEntity(OgreName, "penguin.mesh");

    // Create a SceneNode and attach the Entity to it, matching the camera position
    m_pMyNode = m_SceneMgr.getRootSceneNode()->createChildSceneNode(OgreName);
	m_pMyNode->setScale(0.25f,0.25f,0.25f);
	m_pMyNode->attachObject(m_pMyEntity);
	Ogre::Vector3 startPos = Ogre::Vector3( 1300 * float(rand() / RAND_MAX) + 100, 500,  1300 * float(rand() / RAND_MAX) + 100);
	if(m_pCamera)
	{
		m_pMyNode->setVisible(false);
		m_pCamera->setPosition(startPos);
	}
	m_pMyNode->setPosition(startPos);
}

PlayerEntity::~PlayerEntity()
{
	m_SceneMgr.destroyEntity("Player");
	m_SceneMgr.destroySceneNode(m_pMyNode);
}

void PlayerEntity::Update(float a_DeltaT)
{
	/*float xLookMultiplier = 100000;
	float yLookMultiplier = 100000;
	if( (m_EulerX + Ogre::Degree(m_VertLook * a_DeltaT * xLookMultiplier)).valueDegrees() > 45 || (m_EulerX + Ogre::Degree(m_VertLook * a_DeltaT * xLookMultiplier)).valueDegrees() < -45 )
		m_VertLook = 0;
	m_EulerX += Ogre::Degree(m_VertLook * a_DeltaT * xLookMultiplier);
	m_EulerY += Ogre::Degree(m_HorizLook * a_DeltaT * yLookMultiplier);
	
	//reset camera and entity rotations
	Ogre::Quaternion norm;
	norm.normalise();
	m_pMyNode->setOrientation(norm);

	//rotate entity
	Ogre::Quaternion y1(m_EulerX, Ogre::Vector3::UNIT_X);
	Ogre::Quaternion p1(m_EulerY, Ogre::Vector3::UNIT_Y);
	Ogre::Quaternion newEntityOrientation = y1 * p1;
	m_pMyNode->rotate(newEntityOrientation);

	m_HorizLook = 0;
	m_VertLook = 0;*/

	//walk forward
	Ogre::Vector3 curPos = m_pMyNode->getPosition();
	Ogre::Vector3 curOrientation(0,0,0);
	if(m_pCamera)
	{
		curOrientation = m_pCamera->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
	}
	curPos += curOrientation * m_ForwardMove * a_DeltaT * 100;

	//match the height of the ground as closely as possible
	if(curPos.y < (m_CurGroundHeight + 10 + ENTITY_Y_OFFSET))
	{
		curPos.y = m_CurGroundHeight + 10 + ENTITY_Y_OFFSET;	//climb up a slope, or possibly hover
		m_FallVelocity = 0;
	}
	else if(curPos.y > (m_CurGroundHeight + 10 + ENTITY_Y_OFFSET))
	{
		m_FallVelocity += 9.8f * a_DeltaT;
		curPos.y -= m_FallVelocity;
	}
	m_pMyNode->setPosition(curPos);
	curPos.y -= ENTITY_Y_OFFSET;
	if(m_pCamera)
		m_pCamera->setPosition(curPos);
}

void PlayerEntity::SetPosition(Ogre::Vector3 a_NewPos)
{
	m_pMyNode->setPosition(a_NewPos);
}

Ogre::Vector3 PlayerEntity::GetPosition()
{
	return m_pMyNode->getPosition();
}

void PlayerEntity::SetMoveForward(float a_Forward)
{
	m_ForwardMove = a_Forward;
}

void PlayerEntity::SetMoveSide(float a_Left)
{
	m_SideMove = a_Left;
}

void PlayerEntity::SetLookHoriz(float a_Left)
{
	m_HorizLook = a_Left;
}

void PlayerEntity::SetLookVert(float a_Up)
{
	m_VertLook = a_Up;
}

Ogre::Camera* PlayerEntity::GetCamera()
{
	return m_pCamera;
}

void PlayerEntity::SetGroundHeight(float a_Height)
{
	m_CurGroundHeight = a_Height;
}

void PlayerEntity::SetForwardMove(float a_Rate)
{
	m_ForwardMove = a_Rate;
}

int PlayerEntity::GetNodeUID()
{
	return m_NodeUID;
}
