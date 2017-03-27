#include "stdafx.h"
#include "GameObjectManager.h"
#include "GameObject.h"

int GameObjectIdGenerator::currentObjectId = 0;

GameObjectManager::GameObjectManager()
{
}


GameObjectManager::~GameObjectManager()
{
}

GameObject* GameObjectManager::CreateGameObject()
{
	GameObject* pNewGameObject = new GameObject;
	if (pNewGameObject)
	{
		pNewGameObject->Initialize();

		m_ObjectMap.insert(std::pair<INT, GameObject*>(GameObjectIdGenerator::GetNewId(), pNewGameObject));

		return pNewGameObject;
	}

	return nullptr;
}
