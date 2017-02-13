#pragma once

class CTexture
{
public:
	CTexture();
	~CTexture();

	bool LoadFromDDSFile(std::wstring fileName);

	std::wstring	m_TextureFileName;
};

