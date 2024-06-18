#include "SoundManager.h"
#include "GameScene.h"
#include <sstream>

SoundManager::SoundManager()
{

}

SoundManager::~SoundManager()
{
	
}
void SoundManager::SetSystemVolume(WORD volume) {
	DWORD dwVolume;
	BOOL bSuccess;

	// 현재 디바이스의 볼륨을 가져옴
	bSuccess = waveOutGetVolume(0, &dwVolume);

	if (!bSuccess) {
		// 볼륨의 좌측 및 우측 값 설정
		WORD newVolume = volume | (volume << 16);
		// 볼륨 설정
		waveOutSetVolume(0, newVolume);
	}
}
void SoundManager::PlayBg(std::string option,std::string path)
{
	std::string str;
	str += option;
	str += path;
	//str += " repeat";
	//std::wstring wpath(path.begin(), path.end());

	//wchar_t* volumeCommand = (wchar_t*)"setaudio MediaFile volume to 1";
	//std::wstringstream volumeCommandStream;
	//volumeCommandStream << L"setaudio \"" << wpath << L"\" volume to 1000";
	//std::wstring volumeCommand = volumeCommandStream.str();

	mciSendString(L"setaudio MediaFile volume to 1",0,0,0);

	// string to tchar
	int slength = static_cast<int>(str.length()) + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	TCHAR* buf = new TCHAR[len];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
	TCHAR* r(buf);

	mciSendString(r, 0, 0, 0);

	delete[] buf;
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
	//PlaySound(r, 0, SND_ASYNC||SND_NOSTOP);
}

void SoundManager::SoundUpdate(CGameSource* pGameSource)
{
	CPlayer& pPlayer = pGameSource->GetRefPlayer();

	if (pPlayer.GetFire())
	{
		//SetSystemVolume(100000);
		PlayEf("Sound/Laser gun.wav");
	}
	//if (pPlayer.GetDash())
	//{
	//	//PlayBg("play","Sound/faststep.wav");
	//	//PlayEf("Sound/faststep.wav");
	//}
	//if (pPlayer.GetLUP())
	//{
	//	PlayEf("Sound/level up2.wav");
	//}
	
	//else PlayBg("stop", "Sound/faststep.wav");

}
