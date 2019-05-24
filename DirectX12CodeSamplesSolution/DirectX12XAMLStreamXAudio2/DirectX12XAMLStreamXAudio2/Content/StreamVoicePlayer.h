#pragma once
#include "WAVStreamer.h"

#include<iostream>
#include<string>
#include<memory>
#include "Content/MyTimer.h"


namespace DirectX12XAMLStreamXAudio2
{

	static const uint32_t STREAMING_BUFFER_SIZE = 65536;
	static const size_t MAX_BUFFER_COUNT = 3;
	//--------------------------------------------------------------------------------------
	// Name: struct PlaySoundStreamVoiceContext
	// Desc: Frees up the audio buffer after processing
	//--------------------------------------------------------------------------------------
	struct PlaySoundStreamVoiceContext : public IXAudio2VoiceCallback
	{
		virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
		virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
		virtual void STDMETHODCALLTYPE OnStreamEnd() override 
		{ 
			//SetEvent(m_hBufferEndEvent);			
		}
		virtual void STDMETHODCALLTYPE OnBufferStart(void*) override {}
		void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext)
		{
			SetEvent(m_hBufferEndEvent);

			//Free up the memory chunk holding the PCM data that was read from disk earlier.
			//	In a game you would probably return this memory to a pool.
			free(pBufferContext);
		}
		virtual void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
		virtual void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}

		HANDLE m_hBufferEndEvent;

		PlaySoundStreamVoiceContext()
		{
			m_hBufferEndEvent = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		}
		virtual ~PlaySoundStreamVoiceContext()
		{
			CloseHandle(m_hBufferEndEvent);
		}
	};


	class StreamVoicePlayer
	{
	public:
		StreamVoicePlayer();
		~StreamVoicePlayer();
		HRESULT LoadPCMFile(const wchar_t* szFilename);	

		static DWORD WINAPI ReadFileThread(LPVOID lpParam);
		static DWORD WINAPI SubmitAudioBufferThread(LPVOID lpParam);
		static void EchoFunc(LPVOID lpParam);

		void PlayNextSone();
	private:
		// Audio objects.
		Microsoft::WRL::ComPtr<IXAudio2>            m_pXAudio2;
		IXAudio2MasteringVoice*                     m_pMasteringVoice;
		IXAudio2SourceVoice*                        m_pSourceVoice;
		bool				                        m_DoneSubmitting;
		PlaySoundStreamVoiceContext					m_VoiceContext;		
		WaveFile								    m_WaveFile;
		uint32_t                                    m_waveSize;
		uint32_t                                    m_currentPosition;
		XAUDIO2_BUFFER						        m_Buffers[MAX_BUFFER_COUNT];
		size_t                                      m_NumberOfBuffersProduced;
		size_t                                      m_NumberOfBuffersConsumed;
		uint32_t nBlockAlign;
		Timer t;
		
	};

}