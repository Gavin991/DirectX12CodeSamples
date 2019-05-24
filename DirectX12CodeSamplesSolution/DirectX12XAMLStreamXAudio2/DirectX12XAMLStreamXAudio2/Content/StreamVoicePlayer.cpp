#include "pch.h"
#include "StreamVoicePlayer.h"

namespace DirectX12XAMLStreamXAudio2
{
	//DWORD WINAPI ThreadProc(LPVOID lpParam)
	//{
	//	/*MYDATA *pmd = (MYDATA *)lpParam;
	//	printf("%d\n", pmd->val1);
	//	printf("%d\n", pmd->val2);*/
	//	return 0;
	//}

	//DWORD(WINAPI *pThreadProc)(LPVOID lpParam);




	StreamVoicePlayer::StreamVoicePlayer()
	{
		// Initialize XAudio2 objects
		DX::ThrowIfFailed(XAudio2Create(m_pXAudio2.GetAddressOf(), 0));

#ifdef _DEBUG
		// Enable debugging features
		XAUDIO2_DEBUG_CONFIGURATION debug = {};
		debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
		debug.BreakMask = XAUDIO2_LOG_ERRORS;
		m_pXAudio2->SetDebugConfiguration(&debug, 0);
#endif

		DX::ThrowIfFailed(m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice));

		m_NumberOfBuffersConsumed = 0;
		m_NumberOfBuffersProduced = 0;
		m_currentPosition = 0;
		m_DoneSubmitting = false;

		// Open the file for reading and parse its header	

		DX::ThrowIfFailed(LoadPCMFile(L"Assets/背景音乐.wav"));
		// Start the voice.
		DX::ThrowIfFailed(m_pSourceVoice->Start(0));

		// Create the producer thread (reads PCM chunks from disk)
		//auto callBackFun = bind(&StreamVoicePlayer::ReadFileThread, this, std::placeholders::_1);
		if (!CreateThread(nullptr, 0, ReadFileThread, this, 0, nullptr))//pThreadProc
		{
			throw DX::com_exception(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Create the consumer thread (submits PCM chunks to XAudio2)
		if (!CreateThread(nullptr, 0, StreamVoicePlayer::SubmitAudioBufferThread, this, 0, nullptr))
		{
			throw DX::com_exception(HRESULT_FROM_WIN32(GetLastError()));
		}

		//周期性执行定时任务
		t.StartTimer(1000, std::bind(&StreamVoicePlayer::EchoFunc, this));
		
	}

	StreamVoicePlayer::~StreamVoicePlayer()
	{
		//std::this_thread::sleep_for(std::chrono::seconds(4));
		//std::cout << "try to expire timer!" << std::endl;
		t.Expire();
	}
	//--------------------------------------------------------------------------------------
	// Name: LoadPCMFile
	// Desc: Opens a PCM file for reading and parses its header
	//--------------------------------------------------------------------------------------
	HRESULT StreamVoicePlayer::LoadPCMFile(const wchar_t* szFilename)
	{
		HRESULT hr = S_OK;
		WAVEFORMATEXTENSIBLE wfx = {};

		//
		// Read the wave file
		//
		DX::ThrowIfFailed(m_WaveFile.Open(szFilename));

		// Read the format header
		DX::ThrowIfFailed(m_WaveFile.GetFormat(reinterpret_cast<WAVEFORMATEX*>(&wfx), sizeof(wfx)));

		// Calculate how many bytes and samples are in the wave
		m_waveSize = m_WaveFile.GetDuration();

		//
		// Create the source voice to playback the PCM content
		//
		DX::ThrowIfFailed(m_pXAudio2->CreateSourceVoice(&m_pSourceVoice, &(wfx.Format), 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_VoiceContext));
	
		nBlockAlign=wfx.Format.nBlockAlign;
		return hr;
	}


	//--------------------------------------------------------------------------------------
	// Name: ReadFileThread()
	// Desc: Reads PCM chunks from disk. Blocks when the buffer queue is full
	//--------------------------------------------------------------------------------------

	DWORD WINAPI StreamVoicePlayer::ReadFileThread(LPVOID lpParam)
	{
		auto sample = static_cast<StreamVoicePlayer*>(lpParam);

		while (sample->m_currentPosition < sample->m_waveSize)
		{
			while (sample->m_NumberOfBuffersProduced - sample->m_NumberOfBuffersConsumed >= MAX_BUFFER_COUNT)
			{
				//
				// We reached our capacity to stream in data - we should wait for XAudio2 to finish
				// processing at least one buffer.
				// At this point we could go to sleep, or do something else.
				// For the purposes of this sample, we'll just yield.
				//
				SwitchToThread();
			}

			uint32_t cbValid = min(STREAMING_BUFFER_SIZE, sample->m_waveSize - sample->m_currentPosition);
			/*	std::min(STREAMING_BUFFER_SIZE, sample->m_waveSize - sample->m_currentPosition);*/
			//
			// Allocate memory to stream in data.
			// In a game you would probably acquire this from a memory pool.
			// For the purposes of this sample, we'll allocate it here and have the XAudio2 callback free it later.
			//
			auto pbBuffer = static_cast<uint8_t*>(malloc(cbValid));
			if (!pbBuffer)
				throw std::bad_alloc();

			//
			// Stream in the PCM data.
			// You could potentially use an async read for this. We are already in another thread so we choose to block.
			//
			DX::ThrowIfFailed(
				sample->m_WaveFile.ReadSample(sample->m_currentPosition, pbBuffer, cbValid, nullptr)
			);

			sample->m_currentPosition += cbValid;

			XAUDIO2_BUFFER buffer = {};
			buffer.AudioBytes = cbValid;
			buffer.pAudioData = pbBuffer;
			if (sample->m_currentPosition >= sample->m_waveSize)
				buffer.Flags = XAUDIO2_END_OF_STREAM;

			//
			// Point pContext at the allocated buffer so that we can free it in the OnBufferEnd() callback
			//			
			buffer.pContext = pbBuffer;
			//
			// Make the buffer available for consumption.
			//
			sample->m_Buffers[sample->m_NumberOfBuffersProduced % MAX_BUFFER_COUNT] = buffer;

			//
			// A buffer is ready.
			//
			sample->m_NumberOfBuffersProduced++;
		}

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Name: SubmitAudioBufferThread()
	// Desc: Submits audio buffers to XAudio2. Blocks when XAudio2's queue is full or our buffer queue is empty
	//--------------------------------------------------------------------------------------

	DWORD WINAPI StreamVoicePlayer::SubmitAudioBufferThread(LPVOID lpParam)
	{
		auto sample = static_cast<StreamVoicePlayer*>(lpParam);

		for (;;)
		{
			while (sample->m_NumberOfBuffersProduced - sample->m_NumberOfBuffersConsumed == 0)
			{
				//
				// There are no buffers ready at this time - we should wait for the ReadFile thread to stream in data.
				// At this point we could go to sleep, or do something else.
				// For the purposes of this sample, we'll just yield.
				//
				SwitchToThread();
			}

			//
			// Wait for XAudio2 to be ready - we need at least one free spot inside XAudio2's queue.
			//
			for (;;)
			{
				XAUDIO2_VOICE_STATE state;

				sample->m_pSourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

				if (state.BuffersQueued < MAX_BUFFER_COUNT - 1)
					break;

				WaitForSingleObject(sample->m_VoiceContext.m_hBufferEndEvent, INFINITE);
				
			}

			//
			// Now we have at least one spot free in our buffer queue, and at least one spot free
			// in XAudio2's queue, so submit the next buffer.
			//
			XAUDIO2_BUFFER buffer = sample->m_Buffers[sample->m_NumberOfBuffersConsumed % MAX_BUFFER_COUNT];			
			DX::ThrowIfFailed(sample->m_pSourceVoice->SubmitSourceBuffer(&buffer));

			//
			// A buffer is free.
			//
			sample->m_NumberOfBuffersConsumed++;

			//
			// Check if this is the last buffer.
			//
			if (buffer.Flags == XAUDIO2_END_OF_STREAM)
			{
				//
				// We are done.
				//
				sample->m_DoneSubmitting = true;
				break;
			}
		}

		return S_OK;
	}

 
	void StreamVoicePlayer::EchoFunc(LPVOID lpParam)
	{
		auto sample = static_cast<StreamVoicePlayer*>(lpParam);
		if (sample!=nullptr)
		{
			XAUDIO2_VOICE_STATE state;
			sample->m_pSourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			bool isRunning = (state.BuffersQueued > 0);
			if (isRunning == false)
			{
				sample->m_pSourceVoice->DestroyVoice();
				sample->m_DoneSubmitting = false;
				sample->m_NumberOfBuffersConsumed = 0;
				sample->m_NumberOfBuffersProduced = 0;
				sample->m_currentPosition = 0;
			
				sample->PlayNextSone();
			}

			//if (sample->m_DoneSubmitting)
			//{				
			//	sample->m_pSourceVoice->DestroyVoice();
			//	sample->m_DoneSubmitting = false;
			//	sample->PlayNextSone();
			//}
		}
		
	}

	void StreamVoicePlayer::PlayNextSone()
	{
		// Open the file for reading and parse its header	

		DX::ThrowIfFailed(LoadPCMFile(L"Assets/BattleLobby.wav"));
		// Start the voice.
		DX::ThrowIfFailed(m_pSourceVoice->Start(0));

		// Create the producer thread (reads PCM chunks from disk)
		//auto callBackFun = bind(&StreamVoicePlayer::ReadFileThread, this, std::placeholders::_1);
		if (!CreateThread(nullptr, 0, ReadFileThread, this, 0, nullptr))//pThreadProc
		{
			throw DX::com_exception(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Create the consumer thread (submits PCM chunks to XAudio2)
		if (!CreateThread(nullptr, 0, StreamVoicePlayer::SubmitAudioBufferThread, this, 0, nullptr))
		{
			throw DX::com_exception(HRESULT_FROM_WIN32(GetLastError()));
		}
		m_DoneSubmitting = false;
	}
}