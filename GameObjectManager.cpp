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

void GameObjectManager::Initialize()
{

}

GameObject* GameObjectManager::CreateGameObject()
{
	GameObject* pNewGameObject = new GameObject;
	if (pNewGameObject)
	{
		int newId = GameObjectIdGenerator::GetNewId();

		pNewGameObject->Initialize(newId);

		m_ObjectMap.insert(std::pair<INT, GameObject*>(newId, pNewGameObject));

		return pNewGameObject;
	}

	return nullptr;
}
