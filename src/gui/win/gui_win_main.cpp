#include "gui_win_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

namespace MahNES
{
	MahNES::Emulator emulation;

	int8 pallete_normal[64][4];

	void PPUOutput(void* ptr, int scanline, int pixel, int8 palette, int8 mask)
	{
	    (void)mask;
		((int8*)ptr)[(scanline * 256 + pixel) * 3 + 0] = pallete_normal[palette][0];
		((int8*)ptr)[(scanline * 256 + pixel) * 3 + 1] = pallete_normal[palette][1];
		((int8*)ptr)[(scanline * 256 + pixel) * 3 + 2] = pallete_normal[palette][2];
	}

	void APUOutput(void* ptr, short* data, int n, int freq)
	{
	    (void)freq;
	    GUISound* obj = (GUISound*)ptr;

		for(int i = 0; i < n; i++)
		{
		    obj->buffer[obj->currentWriteBuffer][obj->currentWriteIndex] = data[i];
		    obj->currentWriteIndex++;
		    if (obj->currentWriteIndex >= GUISound::BUFFER_SIZE)
            {
                obj->currentWriteIndex = 0;
                obj->currentWriteBuffer = (obj->currentWriteBuffer + 1) % GUISound::BUFFER_NUMBER;
            }
		}
	}

	int GUI::WindowMainInit()
	{
		pallete_normal[0x0][0]=0x75; pallete_normal[0x0][1]=0x75; pallete_normal[0x0][2]=0x75;
		pallete_normal[0x1][0]=0x27; pallete_normal[0x1][1]=0x1B; pallete_normal[0x1][2]=0x8F;
		pallete_normal[0x2][0]=0x00; pallete_normal[0x2][1]=0x00; pallete_normal[0x2][2]=0xAB;
		pallete_normal[0x3][0]=0x47; pallete_normal[0x3][1]=0x00; pallete_normal[0x3][2]=0x9F;
		pallete_normal[0x4][0]=0x8F; pallete_normal[0x4][1]=0x00; pallete_normal[0x4][2]=0x77;
		pallete_normal[0x5][0]=0xAB; pallete_normal[0x5][1]=0x00; pallete_normal[0x5][2]=0x13;
		pallete_normal[0x6][0]=0xA7; pallete_normal[0x6][1]=0x00; pallete_normal[0x6][2]=0x00;
		pallete_normal[0x7][0]=0x7F; pallete_normal[0x7][1]=0x0B; pallete_normal[0x7][2]=0x00;
		pallete_normal[0x8][0]=0x43; pallete_normal[0x8][1]=0x2F; pallete_normal[0x8][2]=0x00;
		pallete_normal[0x9][0]=0x00; pallete_normal[0x9][1]=0x47; pallete_normal[0x9][2]=0x00;
		pallete_normal[0xA][0]=0x00; pallete_normal[0xA][1]=0x51; pallete_normal[0xA][2]=0x00;
		pallete_normal[0xB][0]=0x00; pallete_normal[0xB][1]=0x3F; pallete_normal[0xB][2]=0x17;
		pallete_normal[0xC][0]=0x1B; pallete_normal[0xC][1]=0x3F; pallete_normal[0xC][2]=0x5F;
		pallete_normal[0xD][0]=0x00; pallete_normal[0xD][1]=0x00; pallete_normal[0xD][2]=0x00;
		pallete_normal[0xE][0]=0x00; pallete_normal[0xE][1]=0x00; pallete_normal[0xE][2]=0x00;
		pallete_normal[0xF][0]=0x00; pallete_normal[0xF][1]=0x00; pallete_normal[0xF][2]=0x00;

		pallete_normal[0x10][0]=0xBC; pallete_normal[0x10][1]=0xBC; pallete_normal[0x10][2]=0xBC;
		pallete_normal[0x11][0]=0x00; pallete_normal[0x11][1]=0x73; pallete_normal[0x11][2]=0xEF;
		pallete_normal[0x12][0]=0x23; pallete_normal[0x12][1]=0x3B; pallete_normal[0x12][2]=0xEF;
		pallete_normal[0x13][0]=0x83; pallete_normal[0x13][1]=0x00; pallete_normal[0x13][2]=0xF3;
		pallete_normal[0x14][0]=0xBF; pallete_normal[0x14][1]=0x00; pallete_normal[0x14][2]=0xBF;
		pallete_normal[0x15][0]=0xE7; pallete_normal[0x15][1]=0x00; pallete_normal[0x15][2]=0x5B;
		pallete_normal[0x16][0]=0xDB; pallete_normal[0x16][1]=0x2B; pallete_normal[0x16][2]=0x00;
		pallete_normal[0x17][0]=0xCB; pallete_normal[0x17][1]=0x4F; pallete_normal[0x17][2]=0x0F;
		pallete_normal[0x18][0]=0x8b; pallete_normal[0x18][1]=0x73; pallete_normal[0x18][2]=0x00;
		pallete_normal[0x19][0]=0x00; pallete_normal[0x19][1]=0x97; pallete_normal[0x19][2]=0x00;
		pallete_normal[0x1A][0]=0x00; pallete_normal[0x1A][1]=0xAB; pallete_normal[0x1A][2]=0x00;
		pallete_normal[0x1B][0]=0x00; pallete_normal[0x1B][1]=0x93; pallete_normal[0x1B][2]=0x3B;
		pallete_normal[0x1C][0]=0x00; pallete_normal[0x1C][1]=0x83; pallete_normal[0x1C][2]=0x8B;
		pallete_normal[0x1D][0]=0x00; pallete_normal[0x1D][1]=0x00; pallete_normal[0x1D][2]=0x00;
		pallete_normal[0x1E][0]=0x00; pallete_normal[0x1E][1]=0x00; pallete_normal[0x1E][2]=0x00;
		pallete_normal[0x1F][0]=0x00; pallete_normal[0x1F][1]=0x00; pallete_normal[0x1F][2]=0x00;

		pallete_normal[0x20][0]=0xFF; pallete_normal[0x20][1]=0xFF; pallete_normal[0x20][2]=0xFF;
		pallete_normal[0x21][0]=0x3F; pallete_normal[0x21][1]=0xBF; pallete_normal[0x21][2]=0xFF;
		pallete_normal[0x22][0]=0x5F; pallete_normal[0x22][1]=0x97; pallete_normal[0x22][2]=0xFF;
		pallete_normal[0x23][0]=0xA7; pallete_normal[0x23][1]=0x8B; pallete_normal[0x23][2]=0xFD;
		pallete_normal[0x24][0]=0xF7; pallete_normal[0x24][1]=0x7B; pallete_normal[0x24][2]=0xFF;
		pallete_normal[0x25][0]=0xFF; pallete_normal[0x25][1]=0x77; pallete_normal[0x25][2]=0xB7;
		pallete_normal[0x26][0]=0xFF; pallete_normal[0x26][1]=0x77; pallete_normal[0x26][2]=0x63;
		pallete_normal[0x27][0]=0xFF; pallete_normal[0x27][1]=0x9B; pallete_normal[0x27][2]=0x3B;
		pallete_normal[0x28][0]=0xF3; pallete_normal[0x28][1]=0xBF; pallete_normal[0x28][2]=0x3F;
		pallete_normal[0x29][0]=0x83; pallete_normal[0x29][1]=0xD3; pallete_normal[0x29][2]=0x13;
		pallete_normal[0x2A][0]=0x4F; pallete_normal[0x2A][1]=0xDF; pallete_normal[0x2A][2]=0x4B;
		pallete_normal[0x2B][0]=0x58; pallete_normal[0x2B][1]=0xF8; pallete_normal[0x2B][2]=0x98;
		pallete_normal[0x2C][0]=0x00; pallete_normal[0x2C][1]=0xEB; pallete_normal[0x2C][2]=0xDB;
		pallete_normal[0x2D][0]=0x00; pallete_normal[0x2D][1]=0x00; pallete_normal[0x2D][2]=0x00;
		pallete_normal[0x2E][0]=0x00; pallete_normal[0x2E][1]=0x00; pallete_normal[0x2E][2]=0x00;
		pallete_normal[0x2F][0]=0x00; pallete_normal[0x2F][1]=0x00; pallete_normal[0x2F][2]=0x00;

		pallete_normal[0x30][0]=0xFF; pallete_normal[0x30][1]=0xFF; pallete_normal[0x30][2]=0xFF;
		pallete_normal[0x31][0]=0xAB; pallete_normal[0x31][1]=0xE7; pallete_normal[0x31][2]=0xFF;
		pallete_normal[0x32][0]=0xC7; pallete_normal[0x32][1]=0xD7; pallete_normal[0x32][2]=0xFF;
		pallete_normal[0x33][0]=0xD7; pallete_normal[0x33][1]=0xCB; pallete_normal[0x33][2]=0xFF;
		pallete_normal[0x34][0]=0xFF; pallete_normal[0x34][1]=0xC7; pallete_normal[0x34][2]=0xFF;
		pallete_normal[0x35][0]=0xFF; pallete_normal[0x35][1]=0xC7; pallete_normal[0x35][2]=0xDB;
		pallete_normal[0x36][0]=0xFF; pallete_normal[0x36][1]=0xBF; pallete_normal[0x36][2]=0xB3;
		pallete_normal[0x37][0]=0xFF; pallete_normal[0x37][1]=0xDB; pallete_normal[0x37][2]=0xAB;
		pallete_normal[0x38][0]=0xFF; pallete_normal[0x38][1]=0xE7; pallete_normal[0x38][2]=0xA3;
		pallete_normal[0x39][0]=0xE3; pallete_normal[0x39][1]=0xFF; pallete_normal[0x39][2]=0xA3;
		pallete_normal[0x3A][0]=0xAB; pallete_normal[0x3A][1]=0xF3; pallete_normal[0x3A][2]=0xBF;
		pallete_normal[0x3B][0]=0xB3; pallete_normal[0x3B][1]=0xFF; pallete_normal[0x3B][2]=0xCF;
		pallete_normal[0x3C][0]=0x9F; pallete_normal[0x3C][1]=0xFF; pallete_normal[0x3C][2]=0xF3;
		pallete_normal[0x3D][0]=0x00; pallete_normal[0x3D][1]=0x00; pallete_normal[0x3D][2]=0x00;
		pallete_normal[0x3E][0]=0x00; pallete_normal[0x3E][1]=0x00; pallete_normal[0x3E][2]=0x00;
		pallete_normal[0x3F][0]=0x00; pallete_normal[0x3F][1]=0x00; pallete_normal[0x3F][2]=0x00;

		screen = new unsigned char [256 * 256 * 3];

		WNDCLASSEX wcex;
		HINSTANCE hInstance = GetModuleHandle(NULL);

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_OWNDC;
		wcex.lpfnWndProc = GUI::WindowMainProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = "MahNES";
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

		if (!RegisterClassEx(&wcex))
			return 0;

		hwndMain = CreateWindowEx(0, "MahNES", "MahNES", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 512, 512, NULL, NULL, hInstance, NULL);
		ShowWindow(hwndMain, true);
		EnableOpenGL(hwndMain, &hdcMain, &hrcMain);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, screen);

		sound.Init();

		emulation.SetAPUOutputFunction((void*)this, APUOutput);
		emulation.SetPPUOutputFunction(screen, PPUOutput);

		const char* filename = GetROMOpen();
		if (filename)
		{
			emulation.LoadROM(filename);
			emulation.Reset();
		}
		else
			exit(0);
		sound.Enable();

		return 0;
	}

	int GUI::WindowMainDestroy()
	{
		DisableOpenGL(hwndMain, hdcMain, hrcMain);
		DestroyWindow(hwndMain);
		return 0;
    }

	int GUI::WindowMainProcess()
	{
		MSG msg;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return 1;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (GetKeyState('O') & 0x80)
			{
				sound.Disable();
				const char* filename = GetROMOpen();
				if (filename)
				{
					emulation.LoadROM(filename);
					emulation.Reset();
				}
				sound.Enable();
			}

			LARGE_INTEGER nFreq;
			LARGE_INTEGER nBeginTime;
			LARGE_INTEGER nEndTime;

			QueryPerformanceFrequency(&nFreq);
			QueryPerformanceCounter(&nBeginTime);

			emulation.ExecuteFrame();
			if (GetKeyState('V') & 0x80)
			{
				emulation.ExecuteFrame();
				emulation.ExecuteFrame();
				emulation.ExecuteFrame();
				emulation.ExecuteFrame();
			}

			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, screen);

			glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glBegin(GL_TRIANGLE_FAN);

				glColor3f(1.0f, 1.0f, 1.0f);

				glVertex2f( 1,  1); glTexCoord2f(0, 0);
				glVertex2f(-1,  1); glTexCoord2f(0, 240 / 255.0);
				glVertex2f(-1, -1); glTexCoord2f(1, 240 / 255.0);
				glVertex2f( 1, -1); glTexCoord2f(1, 0);

			glEnd();

			SwapBuffers(hdcMain);

			int executionTime;
			QueryPerformanceCounter(&nEndTime);
			executionTime = (nEndTime.QuadPart - nBeginTime.QuadPart) * 1000 / nFreq.QuadPart;
			if (executionTime < 1000 / 60) Sleep((1000 / 60) - executionTime);
		}
		return 0;
	}

	const char* GUI::GetROMOpen()
	{
		static char result[260];

#ifdef MAHNES_DEBUG
        printf("Open file? >");
        scanf("%s", result);

        return result;
#else
		OPENFILENAME ofn;
		char szFile[260];

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwndMain;
		ofn.lpstrFile = szFile;

		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "NES ROM Images (*.nes)\0*.nes\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn))
		{
			strcpy(result, ofn.lpstrFile);
			return result;
		}

		return nullptr;
#endif
	}

	LRESULT CALLBACK GUI::WindowMainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_CLOSE:
				PostQuitMessage(0);
			break;

			case WM_DESTROY:
				return 0;

            case WM_SIZE:
            {
                RECT r;
                GetClientRect(hwnd, &r);
                glViewport(0, 0, r.right, r.bottom);
            }

			case WM_KEYDOWN:
			{
				switch (wParam)
				{
					case VK_ESCAPE:
						PostQuitMessage(0);
					break;
				}
			}
			break;

			default:
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return 0;
	}

	void GUI::EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
	{
		PIXELFORMATDESCRIPTOR pfd;

		int iFormat;

		/* get the device context (DC) */
		*hDC = GetDC(hwnd);

		/* set the pixel format for the DC */
		ZeroMemory(&pfd, sizeof(pfd));

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW |
					  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;

		iFormat = ChoosePixelFormat(*hDC, &pfd);

		SetPixelFormat(*hDC, iFormat, &pfd);

		/* create and enable the render context (RC) */
		*hRC = wglCreateContext(*hDC);

		wglMakeCurrent(*hDC, *hRC);

		glEnable(GL_TEXTURE_2D);
	}

	void GUI::DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRC);
		ReleaseDC(hwnd, hDC);
	}

}
