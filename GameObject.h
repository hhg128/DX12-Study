#pragma once

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	void Initialize(int objectId);

private:
	int m_ojbectId;
};

