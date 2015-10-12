#include "gui/win/gui_win_main.h"
#include "emul/emul.h"

int main()
{
	MahNES::GUI gui;
	gui.WindowMainInit();
	while (!gui.WindowMainProcess());
	gui.WindowMainDestroy();

	return 0;
}


