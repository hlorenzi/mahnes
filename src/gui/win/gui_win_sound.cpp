#include "gui_win_sound.h"

namespace MahNES
{
	int GUISound::Init()
	{
		Disable();

        for (int i = 0; i < BUFFER_NUMBER; i++)
        {
            ZeroMemory(&bufferHeader[i], sizeof(WAVEHDR));
        }

        ready = false;
        currentWriteBuffer = 0;
        currentWriteIndex = 0;
        currentPlayBuffer = 0;

		playbackThread = new std::thread(DeviceInitFunc, this);
		return 0;
	}

	void GUISound::Enable()
	{
		enabled = true;
	}

	void GUISound::Disable()
	{
		enabled = false;
	}

	void GUISound::DeviceInitFunc(void* ptr)
	{
	    GUISound* obj = (GUISound*)ptr;

        WAVEFORMATEX waveFormat;

        ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 1;
        waveFormat.nSamplesPerSec = 44100;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;

        if (waveOutOpen(&obj->outHandle, WAVE_MAPPER, &waveFormat, (DWORD_PTR)DeviceWaveOutFunc, (DWORD_PTR)obj, CALLBACK_FUNCTION))
            printf("waveOutOpen error\n");

        for (int i = 0; i < BUFFER_NUMBER; i++)
            obj->AdvanceBuffer();

        while (true)
            Sleep(1000);
	}

    void CALLBACK GUISound::DeviceWaveOutFunc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
    {
        (void)hwo;
        (void)dwParam1;
        (void)dwParam2;
	    GUISound* obj = (GUISound*)dwInstance;

        if (uMsg == WOM_OPEN)
        {
            obj->ready = true;
        }
        else if (uMsg == WOM_DONE)
        {
            obj->AdvanceBuffer();
        }
    }

    void GUISound::AdvanceBuffer()
    {
        if (bufferHeader[currentPlayBuffer].dwFlags & WHDR_PREPARED)
            if (waveOutUnprepareHeader(outHandle, &bufferHeader[currentPlayBuffer], sizeof(WAVEHDR)))
                printf("waveOutUnprepareHeader error\n");

        ZeroMemory(&bufferHeader[currentPlayBuffer], sizeof(WAVEHDR));
        bufferHeader[currentPlayBuffer].lpData = (LPSTR)buffer[currentPlayBuffer];
        bufferHeader[currentPlayBuffer].dwBufferLength = (enabled ? (BUFFER_SIZE * sizeof(short)) : 0);
        bufferHeader[currentPlayBuffer].dwFlags = 0;

        if (waveOutPrepareHeader(outHandle, &bufferHeader[currentPlayBuffer], sizeof(WAVEHDR)))
            printf("waveOutPrepareHeader error\n");

        if (waveOutWrite(outHandle, &bufferHeader[currentPlayBuffer], sizeof(WAVEHDR)))
            printf("waveOutWrite error\n");

        currentPlayBuffer = (currentPlayBuffer + 1) % BUFFER_NUMBER;
    }
}
