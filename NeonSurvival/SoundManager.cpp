#include "SoundManager.h"
#include "GameScene.h"

SoundManager::SoundManager()
{

}

SoundManager::~SoundManager()
{
	
}

void SoundManager::PlayBg(std::string option,std::string path)
{
	std::string str;
	str += option;
	str += path;
	//str += " repeat";

	// string to tchar
	int slength = (int)str.length() + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	TCHAR* buf = new TCHAR[len];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
	TCHAR* r(buf);

	mciSendString(r, 0, 0, 0);
}

void SoundManager::PlayEf(std::string path)
{
	// string to tchar
	int slength = (int)path.length() + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, path.c_str(), slength, 0, 0);
	TCHAR* buf = new TCHAR[len];
	MultiByteToWideChar(CP_ACP, 0, path.c_str(), slength, buf, len);
	TCHAR* r(buf);

	// 효과음 파일 재생
	sndPlaySound(r, SND_ASYNC);
}

void SoundManager::SoundUpdate(CGameSource* pGameSource)
{
	CPlayer& pPlayer = pGameSource->GetRefPlayer();

	if (pPlayer.GetFire())
	{
		PlayEf("Sound/Laser gun.wav");
	}
}
