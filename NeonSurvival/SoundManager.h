#pragma once

#include "stdafx.h"

class CGameSource;
class SoundManager {
public:
	SoundManager();
	~SoundManager();

public:
	void SetSystemVolume(WORD volume);
	// ������� ����ϴ� �޼���
	void PlayBg(std::string option, std::string path);

	// ȿ������ ����ϴ� �޼���
	void PlayEf(std::string path);

	void SoundUpdate(CGameSource* pGameSource);
};
