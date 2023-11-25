#include <windows.h>
#include <stdint.h>

#define internal_function static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint32_t uint32;

struct Win32OffScreenBuffer{
 
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
  int bytesPerPixel;
};

global_variable bool running;
global_variable Win32OffScreenBuffer globalBackBuffer;
  
  internal_function void renderWeirdGradient(Win32OffScreenBuffer buffer, int xOffset, int yOffset){
  
  uint8 *row = (uint8*)buffer.memory;
  for(int y = 0; y < buffer.height; ++y){
    uint32 *pixel = (uint32*)row;
    for(int x = 0; x < buffer.width; ++x){
      /*
	Pixel in memory, hex: BB GG RR xx
      */

      uint8 red = 0;
      uint8 green = (uint8)(y + yOffset);
      uint8 blue = (uint8)(x + xOffset);
      
      *pixel++ = (x%5!=0)?( red << 16 | green << 8 | blue):0;
    }

    row+= buffer.pitch;
  }
}

internal_function void win32ResizeDIBSection(Win32OffScreenBuffer* buffer, int width, int height){

  if(buffer->memory){
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  buffer->width = width;
  buffer->height = height;
  buffer->bytesPerPixel = 4;
  
  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = buffer->width;
  buffer->info.bmiHeader.biHeight = -buffer->height;
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;
  buffer->info.bmiHeader.biSizeImage = 0;
  buffer->info.bmiHeader.biXPelsPerMeter = 0;
  buffer->info.bmiHeader.biYPelsPerMeter = 0;
  buffer->info.bmiHeader.biClrUsed = 0;
  buffer->info.bmiHeader.biClrImportant = 0;

  int bitmapMemorySize = (buffer->width*buffer->height)*buffer->bytesPerPixel;
  buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  buffer->pitch = width*buffer->bytesPerPixel;
  
}

internal_function void win32CopyBufferToWindow(HDC deviceContext, RECT clientRect,
					       Win32OffScreenBuffer buffer,
					       int x, int y, int width, int height){

  int windowWidth = clientRect.right - clientRect.left;
  int windowHeight = clientRect.bottom - clientRect.top;
  //maybe use bitblt for better preformance
  StretchDIBits(deviceContext,
		/*
		x, y, width, height,
		x, y, width, height,
		*/
		0, 0, buffer.width, buffer.height,
		0, 0, windowWidth, windowHeight, 
		buffer.memory,
		&buffer.info,
		DIB_RGB_COLORS,// note here for palatized color
		SRCCOPY);
}

LRESULT CALLBACK mainWindowCallback(HWND hwnd,
		   UINT uMsg,
		   WPARAM wParam,
		   LPARAM lParam){
  LRESULT result = 0;
  
  switch (uMsg){
  case WM_SIZE:
    OutputDebugStringA("WM_SIZE\n");
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    win32ResizeDIBSection(&globalBackBuffer,clientRect.right-clientRect.left,
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
    
    win32CopyBufferToWindow(deviceContext, clientRect, globalBackBuffer, x, y, width, height);
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

  windowClass.style = CS_HREDRAW|CS_VREDRAW;
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

      running = true;
      int xOffset = 0;
      int yOffset = 0;
      
      while(running){
	MSG message;
	while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE)){

	  if(message.message == WM_QUIT){
	    running = false;
	  }
	  
	  TranslateMessage(&message);
	  DispatchMessageA(&message);
	}

	renderWeirdGradient(globalBackBuffer, xOffset++, yOffset++);
	HDC deviceContext =GetDC(windowHandle);
	RECT clientRect;
	GetClientRect(windowHandle,&clientRect);
	int windowWidth = clientRect.right - clientRect.left;
	int windowHeight = clientRect.bottom - clientRect.top;
	win32CopyBufferToWindow(deviceContext, clientRect, globalBackBuffer, 0, 0, windowWidth, windowHeight);
	ReleaseDC(windowHandle, deviceContext);
      }

    } else {
      // log failure
    }
  } else {
    // log failure
  }
  
  return (0);
}
