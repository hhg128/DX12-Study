#pragma once

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	void Initialize(int objectId);

private:
	int m_ojbectId;

	int GameObjectID = 0;

	int First = 0;
	int Second = 0;
};

