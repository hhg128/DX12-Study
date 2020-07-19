#pragma once

class CTexture
{
public:
	CTexture();
	~CTexture();

	bool LoadFromDDSFile(std::wstring fileName);

	std::wstring	m_TextureFileName;

	std::wstring	m_TextureFileName_copy;


	int first;
	int second;
	int third;
};

