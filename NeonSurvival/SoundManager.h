#pragma once

#include "stdafx.h"

class CGameSource;
class SoundManager {
public:
	SoundManager();
	~SoundManager();

public:
	// ������� ����ϴ� �޼���
	void PlayBg(std::string option, std::string path);

	// ȿ������ ����ϴ� �޼���
	void PlayEf(std::string path);

	void SoundUpdate(CGameSource* pGameSource);
};
