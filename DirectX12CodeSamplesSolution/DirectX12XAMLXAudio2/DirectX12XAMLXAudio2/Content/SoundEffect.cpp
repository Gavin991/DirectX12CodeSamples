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
#include "SoundEffect.h"
#include "..\Common\DirectXHelper.h"

SoundEffect::SoundEffect():
    m_audioAvailable(false)
{
}

//----------------------------------------------------------------------

void SoundEffect::Initialize(
    _In_ IXAudio2 *masteringEngine,
    _In_ WAVEFORMATEX *sourceFormat,
    _In_ Platform::Array<byte>^ soundData)
{
    m_soundData = soundData;

    if (masteringEngine == nullptr)
    {
        // Audio is not available so just return.
        m_audioAvailable = false;
        return;
    }

 //    Create a source voice for this sound effect.
    DX::ThrowIfFailed(
        masteringEngine->CreateSourceVoice(
            &m_sourceVoice,
            sourceFormat
            )
        );

	
	//DX::ThrowIfFailed(
	//	masteringEngine->CreateSourceVoice(
	//		&m_sourceVoice,
	//		sourceFormat,
	//		0, 1.0f, &pCallBack
	//	)
	//);

	nBlockAlign = sourceFormat->nBlockAlign;
	//pEngine->CreateSourceVoice(&pSourceVoice, waveFormat, 0, 1.0f, &pCallBack);//创建源声音，用来提交数据
    m_audioAvailable = true;
}

//----------------------------------------------------------------------

void SoundEffect::PlaySound(_In_ float volume)
{
    XAUDIO2_BUFFER buffer = {0};

    if (!m_audioAvailable)
    {
        // Audio is not available so just return.
        return;
    }

    // Interrupt sound effect if it is currently playing.
    DX::ThrowIfFailed(
        m_sourceVoice->Stop()
        );
    DX::ThrowIfFailed(
        m_sourceVoice->FlushSourceBuffers()
        );

    // Queue the memory buffer for playback and start the voice.
    buffer.AudioBytes = m_soundData->Length;
    buffer.pAudioData = m_soundData->Data;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

	//------------
	//XAUDIO2_BUFFER buffer = { 0 };//将读取的文件数据，赋值XAUDIO2_BUFFER
	//buffer.AudioBytes = m_soundData->Length;
	//buffer.pAudioData = m_soundData->Data;
	buffer.LoopBegin = 0;
	buffer.LoopCount = XAUDIO2_MAX_LOOP_COUNT;
	buffer.LoopLength = (double)m_soundData->Length / nBlockAlign;//块对齐，数据的最小原子单位
	buffer.PlayBegin = 0;
	buffer.PlayLength = (double)m_soundData->Length / nBlockAlign;
	//buffer.pContext = &pCallBack;
	//buffer.Flags = XAUDIO2_END_OF_STREAM;


	//-------------
    DX::ThrowIfFailed(
        m_sourceVoice->SetVolume(volume)
        );
    DX::ThrowIfFailed(
        m_sourceVoice->SubmitSourceBuffer(&buffer)
        );
    DX::ThrowIfFailed(
        m_sourceVoice->Start()
        );


	//XAUDIO2_VOICE_STATE state;
	//m_sourceVoice->GetState(&state);//获取状态
	//while (state.BuffersQueued > /*XAUDIO2_MAX_QUEUED_BUFFERS - 1*/0)
	//{
	//	WaitForSingleObject(pCallBack.hBufferEndEvent, INFINITE);
	//	m_sourceVoice->GetState(&state);
	//}


}

//----------------------------------------------------------------------

void SoundEffect::SetVolume(_In_ float volume)
{
	if (!m_audioAvailable)
	{
		// Audio is not available so just return.
		return;
	}
	DX::ThrowIfFailed(
		m_sourceVoice->SetVolume(volume)
	);
}