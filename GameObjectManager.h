#pragma once

class GameObject;


class GameObjectIdGenerator
{
public:
	static int GetNewId() { return currentObjectId++; }

private:
	static int currentObjectId;
};


class GameObjectManager
{
public:
	GameObjectManager();
	~GameObjectManager();

	// Create GameObject and return GameObject's pointer.
	// if failed to create GameObject, return nullptr
	GameObject* CreateGameObject();

private:
	std::unordered_map<INT, GameObject*>	m_ObjectMap;
};

