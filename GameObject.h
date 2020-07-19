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
	int Third = 0;
	int Forth = 0;
	int Fifth = 0;
};

