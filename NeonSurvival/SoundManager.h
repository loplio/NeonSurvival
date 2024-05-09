#pragma once

#include "stdafx.h"

class CGameSource;
class SoundManager {
public:
	SoundManager();
	~SoundManager();

public:
	void SetSystemVolume(WORD volume);
	// 배경음을 출력하는 메서드
	void PlayBg(std::string option, std::string path);

	// 효과음을 출력하는 메서드
	void PlayEf(std::string path);

	void SoundUpdate(CGameSource* pGameSource);
};
