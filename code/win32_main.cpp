#define MTB_IMPLEMENTATION
#include "mtb.h"

#if MTB_FLAG(INTERNAL)
  // printf, snprintf, freopen
  #include <stdio.h>
#endif

#include "couscous.h"
#include "couscous.cpp"

#include <Windows.h>

#if !defined(COUSCOUS_TESTS)
  #define COUSCOUS_TESTS 0
#endif

#if !defined(USE_TEST_PROGRAM)
  #define USE_TEST_PROGRAM 0
#endif

#if USE_TEST_PROGRAM
  #define INSTRUCTION(Word) (u8)((Word) >> 8), (u8)((Word) & 0x00FF)
  #include "testprogram.cpp"
#endif

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

static void*
PushBytes(mem_stack* Memory, size_t NumBytes)
{
  if(Memory->Current + NumBytes > Memory->Length)
    return nullptr;

  void* Ptr = Memory->Ptr + Memory->Current;
  Memory->Current += NumBytes;
  return Ptr;
}

#define PushStruct(Memory, Struct) ((Struct*)PushBytes((Memory), sizeof(Struct)))
#define PushArray(Memory, Length, Struct) ((Struct*)PushBytes((Memory), (Length) * sizeof(Struct)))


struct win32_loaded_rom
{
  size_t Length;
  u8* Ptr;
};

static win32_loaded_rom
Win32LoadRomFromFile(char const* FileName, size_t RomBufferLen, u8* RomBufferPtr)
{
  win32_loaded_rom Result{};
  Result.Ptr = RomBufferPtr;

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

    DWORD const RomLength = (DWORD)FileSize.QuadPart;
    if(RomLength <= RomBufferLen)
    {
      DWORD NumBytesRead;
      if(ReadFile(
          FileHandle,    // _In_        HANDLE       hFile
          RomBufferPtr,  // _Out_       LPVOID       lpBuffer
          RomLength,     // _In_        DWORD        nNumberOfBytesToRead
          &NumBytesRead, // _Out_opt_   LPDWORD      lpNumberOfBytesRead
          nullptr))      // _Inout_opt_ LPOVERLAPPED lpOverlapped
      {
        CloseHandle(FileHandle);

        if(NumBytesRead == RomLength)
        {
          Result.Length = RomLength;
          Result.Ptr = RomBufferPtr;
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
      MTB_Fail("Rom file too large.");
    }
  }
  else
  {
    // NOTE: Failed to open file.
  }

  return Result;
}

bool
LoadRom(machine* M,  size_t RomSize, u8* RomPtr)
{
  bool Result = false;

  if(RomSize <= mtb_ArrayLengthOf(M->ProgramMemory))
  {
    mtb_CopyBytes(RomSize, M->ProgramMemory, RomPtr);
    Result = true;
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

static void
Win32SwapBuffers(bool32* ScreenPixels, win32_front_buffer* Front)
{
  colorRGB8* FrontPixel = Front->Pixels;
  bool32* ScreenPixel = ScreenPixels;

  for(size_t Y = 0; Y < SCREEN_HEIGHT; ++Y)
  {
    for(size_t X = 0; X < SCREEN_WIDTH; ++X, ++ScreenPixel, ++FrontPixel)
    {
      colorRGB8 NewColor;
      if(*ScreenPixel) NewColor = Front->PixelColorOn;
      else             NewColor = Front->PixelColorOff;
      *FrontPixel = NewColor;
    }
  }
}

static void
Win32GetWindowClientArea(HWND WindowHandle, int* ClientWidth, int* ClientHeight)
{
  RECT ClientRect;
  GetClientRect(WindowHandle, &ClientRect);
  *ClientWidth = (int)(ClientRect.right - ClientRect.left);
  *ClientHeight = (int)(ClientRect.bottom - ClientRect.top);
}

struct rect_s32
{
  s32 X, Y;
  s32 Width, Height;
};

static void
Win32Present(HWND WindowHandle, win32_front_buffer* FrontBuffer)
{
  HDC DC = GetDC(WindowHandle);
  colorRGB8* Pixels = FrontBuffer->Pixels;

  // TODO: Keep aspect ratio of screen.
  rect_s32 SourceRect{};
  SourceRect.Width = (int)FrontBuffer->Width;
  SourceRect.Height = (int)FrontBuffer->Height;

  // float Aspect = (float)SourceRect.Width / (float)SourceRect.Height;

  rect_s32 DestRect{};
  Win32GetWindowClientArea(WindowHandle, &DestRect.Width, &DestRect.Height);

  // Add some margin to possibly catch errors.
  #if MTB_FLAG(DEBUG)
    DestRect.X += 64;
    DestRect.Y += 64;
    DestRect.Width -= DestRect.X + 64;
    DestRect.Height -= DestRect.Y + 64;
  #endif

  MTB_INTERNAL_CODE(int Result =) StretchDIBits(
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

struct win32_window
{
  HWND Handle;

  // The entire inner area of the window.
  int ClientWidth;
  int ClientHeight;

  win32_front_buffer FrontBuffer;
  u16 InputState;
};

static void
Win32ToggleFullscreenWindow(HWND WindowHandle)
{
  // Source:
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

static LRESULT CALLBACK
Win32MainWindowCallback(HWND WindowHandle, UINT Message,
                        WPARAM WParam, LPARAM LParam)
{
  LRESULT Result = 0;

  win32_window* Window = (win32_window*)GetWindowLongPtr(WindowHandle, GWLP_USERDATA);

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
      bool KeyWasDown = mtb_IsBitSet((u64)LParam, 30);
      bool KeyIsDown = !mtb_IsBitSet((u64)LParam, 31);
      bool KeyWasPressed = !KeyWasDown && KeyIsDown;
      // bool KeyWasReleased = KeyWasDown && !KeyIsDown;
      bool IsInitialKeyAction = KeyWasDown != KeyIsDown;

      bool AltKeyModifier = false;
      if(Message == WM_SYSKEYDOWN || Message == WM_SYSKEYUP)
        AltKeyModifier = mtb_IsBitSet((u64)LParam, 29);

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

      }

      if(IsInitialKeyAction)
      {
        char VKCodeAsChar = (char)VKCode;
        char KeyChar = 0;
        // Keyboard mapping / key bindings
        switch(VKCodeAsChar)
        {
          case '1': KeyChar = '1'; break;
          case '2': KeyChar = '2'; break;
          case '3': KeyChar = '3'; break;
          case '4': KeyChar = 'C'; break;
          case 'Q': KeyChar = '4'; break;
          case 'W': KeyChar = '5'; break;
          case 'E': KeyChar = '6'; break;
          case 'R': KeyChar = 'D'; break;
          case 'A': KeyChar = '7'; break;
          case 'S': KeyChar = '8'; break;
          case 'D': KeyChar = '9'; break;
          case 'F': KeyChar = 'E'; break;
          case 'Z': KeyChar = 'A'; break;
          case 'X': KeyChar = '0'; break;
          case 'C': KeyChar = 'B'; break;
          case 'V': KeyChar = 'F'; break;
          default: break;
        }

        if(KeyChar > 0)
        {
          u16 KeyIndex = MapCharToKeyIndex(KeyChar);
          u16 OldState = Window->InputState;
          u16 NewState = SetKeyDown(OldState, KeyIndex, KeyIsDown);
          Window->InputState = NewState;
        }
      }
    }
  }
  else if(Message == WM_SIZE)
  {
    // u32 NewWidth = (u32)(LParam & 0xffff);
    // u32 NewHeight = (u32)((LParam & 0xffff0000) >> 16);
    // TODO: Handle resizing.
  }
  else if(Message == WM_PAINT)
  {
    PAINTSTRUCT Paint;

    RECT Rect;
    bool MustBeginAndEndPaint = !!GetUpdateRect(WindowHandle, &Rect, FALSE);

    if(MustBeginAndEndPaint)
      BeginPaint(WindowHandle, &Paint);

    Win32Present(WindowHandle, &Window->FrontBuffer);

    if(MustBeginAndEndPaint)
      EndPaint(WindowHandle, &Paint);
  }
  else
  {
    Result = DefWindowProc(WindowHandle, Message, WParam, LParam);
  }

  return(Result);
}

static void
Win32MessagePump(win32_window* Window)
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

static win32_window
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
      MTB_Fail("Failed to create window.");
    }
  }
  else
  {
    MTB_Fail("Failed to register window class: %s", WindowClass.lpszClassName);
  }

  return Result;
}

struct win32_clock
{
  LARGE_INTEGER Frequency;
};

static win32_clock
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

static win32_timestamp
Win32Timestamp()
{
  win32_timestamp Result;
  QueryPerformanceCounter(&Result.Timestamp);
  return Result;
}

static f64
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
  #if MTB_FLAG(INTERNAL)
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
    freopen("CON", "w", stdout);
    SetConsoleTitleA("Couscous CHIP-8 Debug Console");
  #endif

  #if COUSCOUS_TESTS
    RunTests();
  #endif

  // MTB_Require(ArgsLength == 2, "Invalid number of arguments.");

  char const* FileName = CommandLine;

  mem_stack MemStack{};
  MemStack.Length = (size_t)mtb_MiB(1);

  LPVOID BaseAddress = nullptr;
  MTB_INTERNAL_CODE( BaseAddress = (LPVOID)(size_t)0x2'000'000 );
  MemStack.Ptr = (u8*)VirtualAlloc(BaseAddress, MemStack.Length,
                                    MEM_RESERVE | MEM_COMMIT,
                                    PAGE_READWRITE);

  machine* M = (machine*)PushStruct(&MemStack, machine);
  mtb_SetBytes(sizeof(machine), M, 0);
  #if defined(COUSCOUS_RANDOM_SEED)
    M->RNG = mtb_RandomSeed(COUSCOUS_RANDOM_SEED);
  #elif MTB_FLAG(DEBUG)
    M->RNG = mtb_RandomSeed(0);
  #else
    // TODO(Manuzor): Find a way to properly initialize the RNG.
    #error Random number generator is not initialized and has no seed!
  #endif

  bool RomLoaded = false;
  {
    #if USE_TEST_PROGRAM
      win32_loaded_rom Rom{ mtb_ArrayLengthOf(GlobalTestProgram), GlobalTestProgram };
      // DWORD RomLength = (DWORD)sizeof(GlobalTestProgram);
      // CopyBytes(RomLength, M->Memory + PROGRAM_START_ADDRESS, (u8*)GlobalTestProgram);
    #else
      u8 RomData[sizeof(decltype(machine::ProgramMemory))];
      win32_loaded_rom Rom = Win32LoadRomFromFile(FileName, mtb_ArrayLengthOf(RomData), RomData);
    #endif

    // Insert ROM data into the machine.
    RomLoaded = LoadRom(M, Rom.Length, Rom.Ptr);
  }

  if(RomLoaded)
  {
    win32_window Window;

    {
      char WindowTitle[512]{};
      mtb_ConcatStrings("Couscous CHIP-8 // ", FileName, mtb_ArrayLengthOf(WindowTitle), WindowTitle);

      const int ScreenWidth = SCREEN_WIDTH;
      const int ScreenHeight = SCREEN_HEIGHT;
      const int SizeOfPixelInWindow = 16;
      Window = Win32CreateWindow(ProcessHandle, WindowTitle,
                                 ScreenWidth * SizeOfPixelInWindow,
                                 ScreenHeight * SizeOfPixelInWindow);
    }

    if(Window.Handle)
    {
      //
      // Initialize the front buffer.
      //
      win32_front_buffer* FrontBuffer = &Window.FrontBuffer;
      FrontBuffer->BytesPerPixel = 3;
      FrontBuffer->Width = SCREEN_WIDTH;
      FrontBuffer->Height = SCREEN_HEIGHT;
      FrontBuffer->Pitch = FrontBuffer->Width * FrontBuffer->BytesPerPixel;
      FrontBuffer->BitmapInfo.bmiHeader.biSize = sizeof(FrontBuffer->BitmapInfo.bmiHeader);
      FrontBuffer->BitmapInfo.bmiHeader.biWidth = (LONG)FrontBuffer->Width;
      FrontBuffer->BitmapInfo.bmiHeader.biHeight = -(LONG)FrontBuffer->Height; // Note: This is negative so pixels go downwards.
      FrontBuffer->BitmapInfo.bmiHeader.biPlanes = 1;
      FrontBuffer->BitmapInfo.bmiHeader.biBitCount = (WORD)(FrontBuffer->BytesPerPixel * 8);
      FrontBuffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;
      FrontBuffer->Pixels = (decltype(FrontBuffer->Pixels))PushBytes(&MemStack, FrontBuffer->Width * FrontBuffer->Height * FrontBuffer->BytesPerPixel);
      FrontBuffer->PixelColorOff = { 16, 64, 16 };
      FrontBuffer->PixelColorOn = { 8, 16, 8 };

      // Init swap to ensure properly cleared buffers.
      Win32SwapBuffers(M->Screen, FrontBuffer);

      // Associate the back buffer with the window for presenting.
      SetWindowLongPtr(Window.Handle, GWLP_USERDATA, (LONG_PTR)&Window);

      //
      // Initialize the machine
      //
      // TODO: Ensure charmap size is ok.
      u8* CharMemory = (u8*)M->Memory + CHAR_MEMORY_OFFSET;
      mtb_CopyBytes(mtb_ArrayByteSizeOf(GlobalCharMap), CharMemory, (u8*)GlobalCharMap);

      M->ProgramCounter = (u16)((u8*)M->ProgramMemory - (u8*)M->InterpreterMemory);
      MTB_AssertDebug(M->ProgramCounter == 0x200);


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

        // Apply input.
        u16 OldInputState = M->InputState;
        u16 NewInputState = Window.InputState;
        M->InputState = NewInputState;

        // See if input is required in order to continue execution.
        bool CanTick = true;
        if(M->RequiredInput)
        {
          CanTick = false;
          for(u16 KeyIndex = 0; KeyIndex < 16; ++KeyIndex)
          {
            if(!mtb_IsBitSet(OldInputState, KeyIndex) && mtb_IsBitSet(NewInputState, KeyIndex))
            {
              MTB_AssertDebug((u16)(u8)KeyIndex == KeyIndex);
              *M->RequiredInput = (u8)KeyIndex;
              M->RequiredInput = nullptr;
              CanTick = true;
            }
          }
        }

        if(CanTick)
        {
          tick_result TickResult = Tick(M);
          if(!TickResult.Continue)
          {
            break;
          }

          Win32SwapBuffers(M->Screen, &Window.FrontBuffer);
          Win32Present(Window.Handle, &Window.FrontBuffer);
        }
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
    MTB_Fail("Unable to load ROM.");
    return 1;
  }

  return 0;
}
