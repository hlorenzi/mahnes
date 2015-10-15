#ifndef GUI_WIN_SOUND_H
#define GUI_WIN_SOUND_H

#include <windows.h>

typedef unsigned char Uint8;

namespace MahNES
{
	class GUISound
	{
	public:
		int Init();
		void Enable();
		void Disable();

        static long unsigned int __attribute__((__stdcall__)) DeviceInitFunc(void* ptr);
        static void CALLBACK DeviceWaveOutFunc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

        void AdvanceBuffer();
        void ForceAdvanceBuffer();

	public:
		bool enabled;

		HANDLE playbackThread;
		DWORD playbackThreadId;
        HWAVEOUT outHandle;
        bool ready;

        static const int BUFFER_NUMBER = 3;
        static const int BUFFER_SIZE = 2048;
        short buffer[BUFFER_NUMBER][BUFFER_SIZE];
        WAVEHDR bufferHeader[BUFFER_NUMBER];

        int currentWriteBuffer;
        int currentWriteIndex;
        int currentPlayBuffer;
	};
}

#endif
