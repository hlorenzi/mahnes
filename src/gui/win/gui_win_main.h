#ifndef GUI_WIN_H
#define GUI_WIN_H

#include <windows.h>
#include <gl/gl.h>
#include "../../emul/emul.h"
#include "gui_win_sound.h"

namespace MahNES
{
	extern MahNES::Emulator emulation;

	class GUI
	{
	public:
		int WindowMainInit();
		int WindowMainProcess();
		int WindowMainDestroy();
		const char* GetROMOpen();

		GUISound sound;

	private:
		HWND hwndMain;
		HDC hdcMain;
		HGLRC hrcMain;

		GLuint textureId;
		unsigned char* screen;

		static LRESULT CALLBACK WindowMainProc(HWND, UINT, WPARAM, LPARAM);
		static void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
		static void DisableOpenGL(HWND, HDC, HGLRC);
	};
}

#endif

