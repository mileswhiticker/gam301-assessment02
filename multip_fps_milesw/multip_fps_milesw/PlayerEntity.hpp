#ifndef PLAYER_ENTITY_HPP
#define PLAYER_ENTITY_HPP

#include <OGRE/OgreVector3.h>

namespace Ogre
{
	class Entity;
	class SceneNode;
	class SceneManager;
	class Camera;
};

class PlayerEntity
{
public:
	PlayerEntity(std::string OgreName, int a_NodeUID, Ogre::SceneManager& a_SceneMgr, Ogre::Camera* a_pCamera);
	~PlayerEntity();
	//
	int GetNodeUID();
	void Update(float a_DeltaT);
	//
	void SetMoveForward(float a_Forward = 1);
	void SetMoveSide(float a_Left = 1);
	void SetLookHoriz(float a_Left = 1);
	void SetLookVert(float a_Up = 1);
	//
	void SetPosition(Ogre::Vector3 a_NewPos);
	Ogre::Vector3 GetPosition();
	//
	Ogre::Camera* GetCamera();
	void SetGroundHeight(float a_Height);
	void SetForwardMove(float a_Rate = 1);
	//
private:
	Ogre::Entity* m_pMyEntity;
	Ogre::SceneNode* m_pMyNode;
	Ogre::SceneManager& m_SceneMgr;
    Ogre::Camera* m_pCamera;
	//
	Ogre::Degree m_EulerX;
	Ogre::Degree m_EulerY;
	Ogre::Degree m_EulerZ;
	//
	Ogre::Vector3 m_MoveVector;
	float m_ForwardMove;
	float m_SideMove;
	float m_VertLook;
	float m_HorizLook;
	//
	int& m_NodeUID;
	//
	float m_CurGroundHeight;
	float m_FallVelocity;
};

#endif	//PLAYER_ENTITY_HPP