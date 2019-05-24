//
// MainPage.xaml.h
// MainPage 类的声明。
//

#pragma once

#include "MainPage.g.h"

#include "Content/StreamVoicePlayer.h"



namespace DirectX12XAMLStreamXAudio2
{

	

	/// <summary>
	/// 可用于自身或导航至 Frame 内部的空白页。
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
		

	private:
		//StreamVoicePlayer svp;
		std::unique_ptr<StreamVoicePlayer> svp;
		
		
	};
}
