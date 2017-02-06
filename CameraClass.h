#pragma once

using namespace DirectX;

class CameraClass
{
public:
	CameraClass();
	~CameraClass();

	void Initialize();
	void Update();

	void LookAt(XMFLOAT3 lookAt);
	
	XMMATRIX GetViewMatrix();
	XMMATRIX GetProjMatrix();

	void UpdateViewMatrix();

	void SetPos(XMFLOAT3 camPos);

	void Walk(float d);
	void LiftUp(float d);

	void RotateY(float angle);
	

public:
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	XMMATRIX	m_ViewMatrix;
	XMMATRIX	m_ProjMatrix;

private:
	XMFLOAT3 mPosition	= { 0.0f, 0.0f, -10.0f };
	XMFLOAT3 mRight		= { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 mUp		= { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mLook		= { 0.0f, 0.0f, 1.0f };
};

