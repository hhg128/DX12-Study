#pragma once

class CLight
{
public:
	CLight();
	~CLight();

	void SetLightDirection(DirectX::XMFLOAT3 dir) { m_vDir = dir; };
	DirectX::XMFLOAT3 GetLightDirection() { return m_vDir; }

	DirectX::XMFLOAT3	m_vDir;
};

