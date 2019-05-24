//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#include "pch.h"
#include "Audio.h"
#include "..\Common\DirectXHelper.h"

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

Audio::Audio():
    m_audioAvailable(false)
{
}

void Audio::Initialize()
{
}

void Audio::CreateDeviceIndependentResources()
{
    UINT32 flags = 0;

    DX::ThrowIfFailed(
        XAudio2Create(&m_musicXAudio2)
        );

#if defined(_DEBUG)
    XAUDIO2_DEBUG_CONFIGURATION debugConfiguration = {0};
    debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
    debugConfiguration.TraceMask = XAUDIO2_LOG_ERRORS;
    m_musicXAudio2->SetDebugConfiguration(&debugConfiguration);
#endif


    HRESULT hr = m_musicXAudio2->CreateMasteringVoice(&m_musicMasteringVoice);
    if (FAILED(hr))
    {
        // Unable to create an audio device
        m_audioAvailable = false;
        return;
    }

    DX::ThrowIfFailed(
        XAudio2Create(&m_soundEffectXAudio2)
        );

#if defined(_DEBUG)
    m_soundEffectXAudio2->SetDebugConfiguration(&debugConfiguration);
#endif

    DX::ThrowIfFailed(
        m_soundEffectXAudio2->CreateMasteringVoice(&m_soundEffectMasteringVoice)
        );

    m_audioAvailable = true;

}

IXAudio2* Audio::MusicEngine()
{
    return m_musicXAudio2.Get();
}

IXAudio2* Audio::SoundEffectEngine()
{
    return m_soundEffectXAudio2.Get();
}

void Audio::SuspendAudio()
{
    if (m_audioAvailable)
    {
        m_musicXAudio2->StopEngine();
        m_soundEffectXAudio2->StopEngine();
    }
}

void Audio::ResumeAudio()
{
    if (m_audioAvailable)
    {
        DX::ThrowIfFailed(m_musicXAudio2->StartEngine());
        DX::ThrowIfFailed(m_soundEffectXAudio2->StartEngine());
    }
}

void Audio::SetMusicVolume(float vol)
{
	if (m_audioAvailable)
	{
		m_musicMasteringVoice->SetVolume(vol);
	}
		
}