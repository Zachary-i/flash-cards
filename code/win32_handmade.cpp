#include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow
		     ){
  MessageBoxA(0, "Holato", "Poggae",
	     MB_OK|MB_ICONINFORMATION);
  return (0);
}
