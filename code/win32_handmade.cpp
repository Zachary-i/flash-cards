#include <windows.h>

static bool running;

LRESULT CALLBACK
mainWindowCallback(HWND hwnd,
		   UINT uMsg,
		   WPARAM wParam,
		   LPARAM lParam){
  LRESULT result = 0;
  
  switch (uMsg){
  case WM_SIZE:
    OutputDebugStringA("WM_SIZE\n");
    break;
  case WM_DESTROY:
    running = false;
    OutputDebugStringA("WM_DESTROY\n");
    break;
  case WM_CLOSE:
    running = false;
    OutputDebugStringA("WM_CLOSE\n");
    break;
  case WM_ACTIVATEAPP:
    OutputDebugStringA("WM_ACTIVATEAPP\n");
    break;
  case WM_PAINT:{
    PAINTSTRUCT paint;
    HDC deviceContext = BeginPaint(hwnd, &paint);
    PatBlt(deviceContext,
	   paint.rcPaint.left,
	   paint.rcPaint.top,
	   paint.rcPaint.right - paint.rcPaint.left,
	   paint.rcPaint.bottom - paint.rcPaint.top,
	   BLACKNESS);
    EndPaint(hwnd, &paint);}
    break;
  default:
    result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    break;
  }
  return result;
}

int CALLBACK WinMain(HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR lpCmdLine,
		     int nCmdShow
		     ){
  WNDCLASS windowClass= {};

  windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  windowClass.lpfnWndProc = mainWindowCallback;
  windowClass.hInstance = hInstance;
  //  windowClass.hIcon;
  windowClass.lpszClassName = "flashWindowClass";

  if(RegisterClassA(&windowClass)){
    HWND windowHandle = CreateWindowExA(0,
				       windowClass.lpszClassName,
				       "Flash Cards",
				       WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				       CW_USEDEFAULT,
				       CW_USEDEFAULT,
				       CW_USEDEFAULT,
				       CW_USEDEFAULT,
				       0,
				       0,
				       hInstance,
				       0);
    if (windowHandle){
      MSG message;
      running = true;
      while(running){
	BOOL messageResult = GetMessageA(&message,0,0,0);
	if (messageResult > 0){
	  TranslateMessage(&message);
	  DispatchMessageA(&message);
	} else {
	  break;
	}
      }

    } else {
      // log failure
    }
  } else {
    // log failure
  }
  
  return (0);
}
