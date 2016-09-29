#include <Windows.h>

struct win32_screen
{
  HWND WindowHandle;
  HDC WindowContext;

  int PixelScale = 12;
  int Width;
  int Height;
  colorRGB8* Buffer;
  BITMAPINFO BitmapInfo{ sizeof(BITMAPINFO) };
};

static
win32_screen*
ToWin32Screen(screen_handle Screen)
{
  return Reinterpret<win32_screen*>(Screen);
}

static
screen_handle
FromWin32Screen(win32_screen* Screen)
{
  return Reinterpret<screen_handle>(Screen);
}

static
screen_handle
OpenScreen(mem_stack* Memory, int Width, int Height)
{
  auto Win32Screen = PushStruct(Memory, win32_screen);
  MemConstruct(1, Win32Screen);

  Win32Screen->WindowHandle = GetConsoleWindow();
  Win32Screen->WindowContext = GetDC(Win32Screen->WindowHandle);

  Win32Screen->Width = Width;
  Win32Screen->Height = Height;
  Win32Screen->Buffer = PushArray(Memory, (u64)Win32Screen->Width * Win32Screen->Height, colorRGB8);

  Win32Screen->BitmapInfo.bmiHeader.biWidth = Win32Screen->Width;
  Win32Screen->BitmapInfo.bmiHeader.biHeight = Win32Screen->Height;
  Win32Screen->BitmapInfo.bmiHeader.biPlanes = 1;
  Win32Screen->BitmapInfo.bmiHeader.biBitCount = 24;
  Win32Screen->BitmapInfo.bmiHeader.biCompression = BI_RGB;

  return FromWin32Screen(Win32Screen);
}

static
void
CloseScreen(screen_handle Screen)
{
  win32_screen* Win32Screen = ToWin32Screen(Screen);
  ReleaseDC(Win32Screen->WindowHandle, Win32Screen->WindowContext);
}

static
void
ClearScreen(screen_handle Screen, colorRGB8 ClearColor)
{
  win32_screen* Win32Screen = ToWin32Screen(Screen);
  int const Width = Win32Screen->Width;
  int const Height = Win32Screen->Height;
  MemSet(Width * Height, Win32Screen->Buffer, ClearColor);
}

static
void
PresentScreen(screen_handle Screen)
{
  win32_screen* Win32Screen = ToWin32Screen(Screen);
  HDC DC = Win32Screen->WindowContext;

  const int DestWidth = Win32Screen->Width * Win32Screen->PixelScale;
  const int DestHeight = Win32Screen->Height * Win32Screen->PixelScale;

  // NOTE: Draw with some Y offset so that the cursor won't be in the way.
  MTB_DebugCode(int Result =) StretchDIBits(
    DC,                       // _In_       HDC        hdc
    0,                        // _In_       int        XDest
    16,                       // _In_       int        YDest
    DestWidth,                // _In_       int        nDestWidth
    DestHeight,               // _In_       int        nDestHeight
    0,                        // _In_       int        XSrc
    0,                        // _In_       int        YSrc
    Win32Screen->Width,       // _In_       int        nSrcWidth
    Win32Screen->Height,      // _In_       int        nSrcHeight
    Win32Screen->Buffer,      // _In_ const VOID       *lpBits
    &Win32Screen->BitmapInfo, // _In_ const BITMAPINFO *lpBitsInfo
    DIB_RGB_COLORS,           // _In_       UINT       iUsage
    SRCCOPY);                 // _In_       DWORD      dwRop
}

static
int
GetScreenWidth(screen_handle Screen)
{
  int Result = ToWin32Screen(Screen)->Width;
  return Result;
}

static
int
GetScreenHeight(screen_handle Screen)
{
  int Result = ToWin32Screen(Screen)->Height;
  return Result;
}
