#define MTB_IMPLEMENTATION
#include "mtb.hpp"

#include "couscous.hpp"
#include "couscous.cpp"

#include <Windows.h>

#if !defined(COUSCOUS_TESTS)
  #define COUSCOUS_TESTS 0
#endif

#define INSTRUCTION(Word) (u8)((Word) >> 8), (u8)((Word) & 0x00FF)

#if !defined(USE_TEST_PROGRAM)
  #define USE_TEST_PROGRAM 1
#endif

#if USE_TEST_PROGRAM
  #include "testprogram.cpp"
#endif

void
Print(char const* String)
{
  OutputDebugStringA(String);
}

struct colorRGB8
{
  union
  {
    struct
    {
      u8 R;
      u8 G;
      u8 B;
    };

    u8 Data[3];
  };
};

struct mem_stack
{
  size_t Current;
  size_t Length;
  u8* Ptr;
};

internal
void*
PushBytes(mem_stack* Memory, size_t NumBytes)
{
  if(Memory->Current + NumBytes > Memory->Length)
    return nullptr;

  void* Ptr = Memory->Ptr + Memory->Current;
  Memory->Current += NumBytes;
  return Ptr;
}

#define PushStruct(Memory, Struct) (Struct*)PushBytes((Memory), SizeOf<Struct>())
#define PushArray(Memory, Length, Struct) (Struct*)PushBytes((Memory), (Length) * SizeOf<Struct>())

struct loaded_rom
{
  size_t Length;
  u8* Ptr;
};

internal
slice<u8>
Win32LoadRomFromFile(char const* FileName, slice<u8> RomBuffer)
{
  slice<u8> Result{};

  HANDLE FileHandle = CreateFileA(
    FileName,        // _In_     LPCTSTR               lpFileName,
    GENERIC_READ,    // _In_     DWORD                 dwDesiredAccess,
    FILE_SHARE_READ, // _In_     DWORD                 dwShareMode,
    nullptr,         // _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OPEN_EXISTING,   // _In_     DWORD                 dwCreationDisposition,
    0,               // _In_     DWORD                 dwFlagsAndAttributes,
    nullptr);        // _In_opt_ HANDLE                hTemplateFile

  if(FileHandle != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER FileSize;
    GetFileSizeEx(FileHandle, &FileSize);

    auto const RomLength = Convert<DWORD>(FileSize.QuadPart);
    if(RomLength <= LengthOf(RomBuffer))
    {
      DWORD NumBytesRead;
      if(ReadFile(
          FileHandle,    // _In_        HANDLE       hFile
          RomBuffer.Ptr, // _Out_       LPVOID       lpBuffer
          RomLength,     // _In_        DWORD        nNumberOfBytesToRead
          &NumBytesRead, // _Out_opt_   LPDWORD      lpNumberOfBytesRead
          nullptr))      // _Inout_opt_ LPOVERLAPPED lpOverlapped
      {
        CloseHandle(FileHandle);

        if(NumBytesRead == RomLength)
        {
          Result = { RomLength, RomBuffer.Ptr };
        }
      }
      else
      {
        // TODO: Handle failure?
        // NOTE: Memory is leaked at this point.
      }
    }
    else
    {
      // TODO: Logging?
      MTB_ReportError("Rom file too large.");
    }
  }
  else
  {
    // NOTE: Failed to open file.
  }

  return Result;
}

struct win32_front_buffer
{
  BITMAPINFO BitmapInfo;

  colorRGB8* Pixels;
  DWORD Width;
  DWORD Height;
  DWORD Pitch;
  DWORD BytesPerPixel;

  colorRGB8 PixelColorOn;
  colorRGB8 PixelColorOff;
};

internal
void
Win32SwapBuffers(screen* Screen, win32_front_buffer* Front)
{
  size_t const Width = Screen->Width;
  size_t const Height = Screen->Height;
  colorRGB8* FrontPixel = Front->Pixels;
  bool32* ScreenPixel = Screen->Pixels;

  for(size_t Y = 0; Y < Height; ++Y)
  {
    for(size_t X = 0; X < Width; ++X, ++ScreenPixel, ++FrontPixel)
    {
      colorRGB8 NewColor;
      if(*ScreenPixel) NewColor = Front->PixelColorOn;
      else             NewColor = Front->PixelColorOff;
      *FrontPixel = NewColor;
    }
  }
}

internal
void
Win32GetWindowClientArea(HWND WindowHandle, int* ClientWidth, int* ClientHeight)
{
  RECT ClientRect;
  GetClientRect(WindowHandle, &ClientRect);
  *ClientWidth = Convert<int>(ClientRect.right - ClientRect.left);
  *ClientHeight = Convert<int>(ClientRect.bottom - ClientRect.top);
}

struct rect_i32
{
  i32 X, Y;
  i32 Width, Height;
};

internal
void
Win32Present(HWND WindowHandle, win32_front_buffer* FrontBuffer)
{
  HDC DC = GetDC(WindowHandle);
  colorRGB8* Pixels = FrontBuffer->Pixels;

  // TODO: Keep aspect ratio of screen.
  rect_i32 SourceRect{};
  SourceRect.Width = (int)FrontBuffer->Width;
  SourceRect.Height = (int)FrontBuffer->Height;

  float Aspect = (float)SourceRect.Width / (float)SourceRect.Height;

  rect_i32 DestRect{};
  Win32GetWindowClientArea(WindowHandle, &DestRect.Width, &DestRect.Height);

  // Add some margin to possibly catch errors.
  #if MTB_IsOn(MTB_Internal)
    DestRect.X += 64;
    DestRect.Y += 64;
    DestRect.Width -= DestRect.X + 64;
    DestRect.Height -= DestRect.Y + 64;
  #endif

  MTB_DebugCode(int Result =) StretchDIBits(
    DC,                       // _In_       HDC        hdc
    DestRect.X,               // _In_       int        XDest
    DestRect.Y,               // _In_       int        YDest
    DestRect.Width,           // _In_       int        nDestWidth
    DestRect.Height,          // _In_       int        nDestHeight
    SourceRect.X,             // _In_       int        XSrc
    SourceRect.Y,             // _In_       int        YSrc
    SourceRect.Width,         // _In_       int        nSrcWidth
    SourceRect.Height,        // _In_       int        nSrcHeight
    Pixels,                   // _In_ const VOID       *lpBits
    &FrontBuffer->BitmapInfo, // _In_ const BITMAPINFO *lpBitsInfo
    DIB_RGB_COLORS,           // _In_       UINT       iUsage
    SRCCOPY);                 // _In_       DWORD      dwRop

  // TODO: Clear regions to black that aren't covered by the code above?

  ReleaseDC(WindowHandle, DC);
}

internal void
Win32ToggleFullscreenWindow(HWND WindowHandle)
{
  // Note: This follows  prescription
  // for fullscreen toggling, see:
  // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx

  static WINDOWPLACEMENT LocalWindowPosition{ sizeof(LocalWindowPosition) };

  DWORD Style = GetWindowLong(WindowHandle, GWL_STYLE);
  if(Style & WS_OVERLAPPEDWINDOW)
  {
    MONITORINFO MonitorInfo{ sizeof(MonitorInfo) };
    if(GetWindowPlacement(WindowHandle, &LocalWindowPosition) &&
       GetMonitorInfo(MonitorFromWindow(WindowHandle, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
    {
      SetWindowLong(WindowHandle, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
      SetWindowPos(WindowHandle, HWND_TOP,
                   MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                   MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                   MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                   SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
  else
  {
    SetWindowLong(WindowHandle, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(WindowHandle, &LocalWindowPosition);
    SetWindowPos(WindowHandle, 0, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

internal
LRESULT CALLBACK
Win32MainWindowCallback(HWND WindowHandle, UINT Message,
                        WPARAM WParam, LPARAM LParam)
{
  LRESULT Result = 0;

  auto FrontBuffer = Reinterpret<win32_front_buffer*>(GetWindowLongPtr(WindowHandle, GWLP_USERDATA));

  if(Message == WM_CLOSE || Message == WM_DESTROY)
  {
    PostQuitMessage(0);
  }
  else if(Message >= WM_KEYFIRST && Message <= WM_KEYLAST
          // Message >= WM_MOUSEFIRST && Message <= WM_MOUSELAST ||
          // Message == WM_INPUT
  )
  {
    if(Message != WM_CHAR) // Char messages are ignored (for now).)
    {
      u32 VKCode = (u32)WParam;
      bool KeyWasDown = IsBitSet((u64)LParam, 30);
      bool KeyIsDown = !IsBitSet((u64)LParam, 31);
      bool KeyWasPressed = !KeyWasDown && KeyIsDown;
      bool KeyWasReleased = KeyWasDown && !KeyIsDown;

      bool AltKeyModifier = false;
      if(Message == WM_SYSKEYDOWN || Message == WM_SYSKEYUP)
        AltKeyModifier = IsBitSet((u64)LParam, 29);

      if(KeyWasPressed)
      {
        if(VKCode == VK_ESCAPE)
        {
          PostQuitMessage(0);
        }

        if(AltKeyModifier && VKCode == VK_F4)
        {
          PostQuitMessage(0);
        }

        if(AltKeyModifier && VKCode == VK_RETURN)
        {
          Win32ToggleFullscreenWindow(WindowHandle);
        }

        // TODO: Process input.
      }
    }
  }
  else if(Message == WM_SIZE)
  {
    u32 NewWidth = Convert<u32>(LParam & 0xffff);
    u32 NewHeight = Convert<u32>((LParam & 0xffff0000) >> 16);
    // TODO: Handle resizing.
  }
  else if(Message == WM_PAINT)
  {
    PAINTSTRUCT Paint;

    RECT Rect;
    bool MustBeginAndEndPaint = !!GetUpdateRect(WindowHandle, &Rect, FALSE);

    if(MustBeginAndEndPaint)
      BeginPaint(WindowHandle, &Paint);

    Win32Present(WindowHandle, FrontBuffer);

    if(MustBeginAndEndPaint)
      EndPaint(WindowHandle, &Paint);
  }
  else
  {
    Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
  }

  return(Result);
}

internal
void
Win32MessagePump(struct win32_window* Window)
{
  MSG Message;
  while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
  {
    switch(Message.message)
    {
      case WM_QUIT:
      {
        ::quick_exit(0);
      } break;
      default:
      {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
      }
    }
  }
}

struct win32_window
{
  HWND Handle;

  // The entire inner area of the window.
  int ClientWidth;
  int ClientHeight;
};

internal
win32_window
Win32CreateWindow(HINSTANCE ProcessHandle,
                  char const* WindowTitle,
                  int ClientWidth, int ClientHeight)
{
  win32_window Result{};

  WNDCLASSA WindowClass{};
  WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  WindowClass.lpfnWndProc = &Win32MainWindowCallback;
  WindowClass.hInstance = ProcessHandle;
  WindowClass.lpszClassName = "CouscousWindowClass";
  WindowClass.hCursor = LoadCursor(0, IDC_ARROW);

  if(RegisterClassA(&WindowClass))
  {
    DWORD const WindowStyleEx = 0;
    DWORD const WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    RECT WindowRect{};
    WindowRect.right = ClientWidth;
    WindowRect.bottom = ClientHeight;
    AdjustWindowRectEx(&WindowRect, WindowStyle, FALSE, WindowStyleEx);

    WindowRect.right -= WindowRect.left;
    WindowRect.bottom -= WindowRect.top;

    // Apply user translation
    WindowRect.left = 0;
    WindowRect.right += WindowRect.left;
    WindowRect.top = 0;
    WindowRect.bottom += WindowRect.top;

    RECT WindowWorkArea{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &WindowWorkArea, 0);
    WindowRect.left   += WindowWorkArea.left;
    WindowRect.right  += WindowWorkArea.left;
    WindowRect.top    += WindowWorkArea.top;
    WindowRect.bottom += WindowWorkArea.top;

    HWND WindowHandle{};
    auto const WindowX = CW_USEDEFAULT;
    auto const WindowY = CW_USEDEFAULT;
    auto const WindowWidth = WindowRect.right - WindowRect.left;
    auto const WindowHeight = WindowRect.bottom - WindowRect.top;
    WindowHandle = CreateWindowExA(WindowStyleEx,             // _In_     DWORD     dwExStyle
                                   WindowClass.lpszClassName, // _In_opt_ LPCWSTR   lpClassName
                                   WindowTitle,               // _In_opt_ LPCWSTR   lpWindowName
                                   WindowStyle,               // _In_     DWORD     dwStyle
                                   WindowX, WindowY,          // _In_     int       X, Y
                                   WindowWidth, WindowHeight, // _In_     int       nWidth, nHeight
                                   nullptr,                   // _In_opt_ HWND      hWndParent
                                   nullptr,                   // _In_opt_ HMENU     hMenu
                                   ProcessHandle,             // _In_opt_ HINSTANCE hInstance
                                   nullptr);                  // _In_opt_ LPVOID    lpParam

    if(WindowHandle)
    {
      Result.Handle = WindowHandle;
      Result.ClientWidth = ClientWidth;
      Result.ClientHeight = ClientHeight;
    }
    else
    {
      MTB_ReportError("Failed to create window.");
    }
  }
  else
  {
    MTB_ReportError("Failed to register window class: %s", WindowClass.lpszClassName);
  }

  return Result;
}

struct win32_clock
{
  LARGE_INTEGER Frequency;
};

internal
win32_clock
Win32CreateClock()
{
  win32_clock Result;
  QueryPerformanceFrequency(&Result.Frequency);
  return Result;
}

struct win32_timestamp
{
  LARGE_INTEGER Timestamp;
};

internal
win32_timestamp
Win32Timestamp()
{
  win32_timestamp Result;
  QueryPerformanceCounter(&Result.Timestamp);
  return Result;
}

internal
f64
Win32DeltaTime(win32_clock* Clock, win32_timestamp* End, win32_timestamp* Start)
{
  LONGLONG EndInt = End->Timestamp.QuadPart;
  LONGLONG StartInt = Start->Timestamp.QuadPart;
  LONGLONG FrequencyInt = Clock->Frequency.QuadPart;

  LONGLONG Delta = EndInt - StartInt;
  f64 Result = (f64)Delta / (f64)FrequencyInt;
  return Result;
}

int
WinMain(HINSTANCE ProcessHandle, HINSTANCE PreviousProcessHandle,
        LPSTR CommandLine, int ShowCode)
{
  #if COUSCOUS_TESTS
  {
    machine TestMachine;
    int const TestScreenWidth = 8;
    int const TestScreenHeight = 4;
    bool32 TestScreen[TestScreenWidth * TestScreenHeight];
    for(test* Test = GlobalFirstTest;
        Test;
        Test = Test->Next)
    {
      TestMachine = {};
      TestMachine.Screen.Width = TestScreenWidth;
      TestMachine.Screen.Height = TestScreenHeight;
      TestMachine.Screen.Pixels = TestScreen;
      SetBytes(ByteLengthOf(TestScreen), TestScreen, 0xcd);

      char const* Name = Test->Name;
      Print("Test: ");
      Print(Name);
      Print("\n");
      Test->Procedure(&TestMachine);
    }
  }
  #endif

  // MTB_Require(ArgsLength == 2, "Invalid number of arguments.");

  char const* FileName = CommandLine;

  mem_stack UtilStack{};
  UtilStack.Length = MiB(1);

  #if MTB_IsOn(MTB_Internal)
    LPVOID BaseAddress = (LPVOID)(size_t)0x2'000'000;
  #else
    LPVOID BaseAddress = nullptr;
  #endif
  UtilStack.Ptr = (u8*)VirtualAlloc(BaseAddress, UtilStack.Length,
                                    MEM_RESERVE | MEM_COMMIT,
                                    PAGE_READWRITE);

  auto M = (machine*)PushStruct(&UtilStack, machine);
  ConstructElements(1, M);

  bool RomLoaded{};
  {
    slice<u8> Rom{};
    #if USE_TEST_PROGRAM
      Rom = Slice(GlobalTestProgram);
      // DWORD RomLength = (DWORD)sizeof(GlobalTestProgram);
      // CopyBytes(RomLength, M->Memory + PROGRAM_START_ADDRESS, (u8*)GlobalTestProgram);
    #else

      u8 RomData[MAX_ROM_LENGTH];
      Rom = Win32LoadRomFromFile(FileName, Slice(RomData));
    #endif

    // Insert ROM data into the machine.
    RomLoaded = LoadRom(M, Rom);
  }

  if(RomLoaded)
  {
    const int ScreenWidth = 64;
    const int ScreenHeight = 32;
    const int SizeOfPixelInWindow = 16;
    win32_window Window = Win32CreateWindow(ProcessHandle, "Couscous - CHIP-8",
                                            ScreenWidth * SizeOfPixelInWindow,
                                            ScreenHeight * SizeOfPixelInWindow);
    if(Window.Handle)
    {
      //
      // Create front buffer.
      //
      win32_front_buffer Win32FrontBuffer{};
      Win32FrontBuffer.BytesPerPixel = 3;
      Win32FrontBuffer.Width = ScreenWidth;
      Win32FrontBuffer.Height = ScreenHeight;
      Win32FrontBuffer.Pitch = Win32FrontBuffer.Width * Win32FrontBuffer.BytesPerPixel;
      Win32FrontBuffer.BitmapInfo.bmiHeader.biSize = sizeof(Win32FrontBuffer.BitmapInfo.bmiHeader);
      Win32FrontBuffer.BitmapInfo.bmiHeader.biWidth = (LONG)Win32FrontBuffer.Width;
      Win32FrontBuffer.BitmapInfo.bmiHeader.biHeight = -(LONG)Win32FrontBuffer.Height;
      Win32FrontBuffer.BitmapInfo.bmiHeader.biPlanes = 1;
      Win32FrontBuffer.BitmapInfo.bmiHeader.biBitCount = Win32FrontBuffer.BytesPerPixel * 8;
      Win32FrontBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
      Win32FrontBuffer.Pixels = (decltype(Win32FrontBuffer.Pixels))PushBytes(&UtilStack, Win32FrontBuffer.Width * Win32FrontBuffer.Height * Win32FrontBuffer.BytesPerPixel);
      Win32FrontBuffer.PixelColorOff = { 16, 64, 16 };
      Win32FrontBuffer.PixelColorOn =  {  8, 16,  8 };

      //
      // Create back buffer data.
      //
      screen* Screen = &M->Screen;
      Screen->Width = (int)Win32FrontBuffer.Width;
      Screen->Height = (int)Win32FrontBuffer.Height;
      Screen->Pixels = (decltype(Screen->Pixels))PushBytes(&UtilStack, Screen->Width * Screen->Height);

      // Init clear and swap to ensure properly cleared buffers.
      ClearScreen(M);
      Win32SwapBuffers(Screen, &Win32FrontBuffer);

      // Associate the back buffer with the window for presenting.
      SetWindowLongPtr(Window.Handle, GWLP_USERDATA, Reinterpret<LONG_PTR>(&Win32FrontBuffer));

      InitMachine(M);

      //
      // Clock setup.
      //
      win32_clock Clock = Win32CreateClock();
      win32_timestamp LastTickTime = Win32Timestamp();

      while(true)
      {
        f64 const TickDuration = 1.0 / 60.0;

        f64 DeltaTime{};
        win32_timestamp CurrentTime;
        while(true)
        {
          Win32MessagePump(&Window);
          CurrentTime = Win32Timestamp();
          DeltaTime = Win32DeltaTime(&Clock, &CurrentTime, &LastTickTime);

          if(DeltaTime >= TickDuration)
            break;
        }
        LastTickTime = CurrentTime;

        if(!Tick(M))
        {
          break;
        }

        Win32SwapBuffers(Screen, &Win32FrontBuffer);
        Win32Present(Window.Handle, &Win32FrontBuffer);
      }

      // PostQuitMessage(0);
      while(true)
      {
        Win32MessagePump(&Window);
      }
    }
    else
    {
      // TODO: Logging?
    }
  }
  else
  {
    // TODO: Logging?
    MTB_ReportError("Unable to load ROM.");
    return 1;
  }

  return 0;
}
