#pragma once

class CLight
{
public:
	CLight();
	~CLight();

	void SetLightPosition(DirectX::XMFLOAT3 pos) { m_vPos = pos; };
	DirectX::XMFLOAT3 GetLightPosition() { return m_vPos; }

	DirectX::XMFLOAT3	m_vPos;
};

