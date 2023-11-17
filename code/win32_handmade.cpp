#include <windows.h>
#include <stdint.h>

#define internal_function static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint32_t uint32;

global_variable bool running;
global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;


internal_function void
win32ResizeDIBSection(int width, int height){

  if(bitmapMemory){
    VirtualFree(bitmapMemory, 0, MEM_RELEASE);
  }

  bitmapWidth = width;
  bitmapHeight = height;
  
  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = bitmapWidth;
  bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;
  bitmapInfo.bmiHeader.biSizeImage = 0;
  bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
  bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
  bitmapInfo.bmiHeader.biClrUsed = 0;
  bitmapInfo.bmiHeader.biClrImportant = 0;

  int bytesPerPixel =4;
  int bitmapMemorySize = (bitmapWidth*bitmapHeight)*bytesPerPixel;
  bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  int pitch = width*bytesPerPixel;
  uint8 *row = (uint8*)bitmapMemory;
  for(int y = 0; y < bitmapHeight; ++y){
    uint8 *pixel = (uint8*)row;
    for(int x = 0; x < bitmapWidth; ++x){
      /*
	Pixel in memory, hex: BB GG RR xx
      */
      *pixel = (uint8)x;
      ++pixel;
      
      *pixel = (uint8)y;
      ++pixel;

      *pixel = 0;
      ++pixel;

      *pixel = 0;
      ++pixel;
    }

    row+= pitch;
  }
}

internal_function void
win32UpdateWindow(HDC deviceContext, RECT *windowRect, int x, int y, int width, int height){

  int windowWidth = windowRect->right - windowRect->left;
  int windowHeight = windowRect->bottom - windowRect->top;
  //maybe use bitblt for better preformance
  StretchDIBits(deviceContext,
		/*
		x, y, width, height,
		x, y, width, height,
		*/
		0, 0, bitmapWidth, bitmapHeight,
		0, 0, windowWidth, windowHeight, 
		bitmapMemory,
		&bitmapInfo,
		DIB_RGB_COLORS,// note here for palatized color
		SRCCOPY);
}

LRESULT CALLBACK
mainWindowCallback(HWND hwnd,
		   UINT uMsg,
		   WPARAM wParam,
		   LPARAM lParam){
  LRESULT result = 0;
  
  switch (uMsg){
  case WM_SIZE:
    OutputDebugStringA("WM_SIZE\n");
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    win32ResizeDIBSection(clientRect.right-clientRect.left,
		     clientRect.bottom-clientRect.top);
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
    int x = paint.rcPaint.left;
    int y = paint.rcPaint.top;
    int width = paint.rcPaint.right - paint.rcPaint.left;
    int height = paint.rcPaint.bottom - paint.rcPaint.top;
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    
    win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
  } break;
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
