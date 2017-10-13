#define MTB_IMPLEMENTATION
#include "mtb.h"

#if MTB_FLAG(INTERNAL)
    // printf, snprintf, freopen
    #include <stdio.h>
#endif

#if !defined(COUSCOUS_ASSEMBLER)
    #define COUSCOUS_ASSEMBLER 1
#endif

#include "mtb.h"
using u8 = mtb_u08;
using u16 = mtb_u16;
using u32 = mtb_u32;
using u64 = mtb_u64;
using s8 = mtb_s08;
using s16 = mtb_s16;
using s32 = mtb_s32;
using s64 = mtb_s64;

using f32 = mtb_f32;
using f64 = mtb_f64;

using uint = unsigned int;
using bool32 = int;

#include "_generated/text.h"
#include "_generated/token.h"
#include "_generated/label_array.h"
#include "_generated/patch_array.h"
#include "_generated/u8_array.h"
#include "couscous.h"

#include "couscous.cpp"
#include "_generated/all_generated.cpp"

#include "charmap.cpp"

#if defined(COUSCOUS_TESTS)
    #include "couscous_tests.cpp"
#endif

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

struct colorRGBA8
{
    union
    {
        struct
        {
            u8 R;
            u8 G;
            u8 B;
            u8 A;
        };

        u8 Data[4];
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
    if (Memory->Current + NumBytes > Memory->Length)
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

static u8_array
Win32LoadRomFromFile(char const* FileName)
{
    u8_array Result{};

    HANDLE FileHandle = CreateFileA(
        FileName,        // _In_     LPCTSTR               lpFileName,
        GENERIC_READ,    // _In_     DWORD                 dwDesiredAccess,
        FILE_SHARE_READ, // _In_     DWORD                 dwShareMode,
        nullptr,         // _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        OPEN_EXISTING,   // _In_     DWORD                 dwCreationDisposition,
        0,               // _In_     DWORD                 dwFlagsAndAttributes,
        nullptr);        // _In_opt_ HANDLE                hTemplateFile

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        GetFileSizeEx(FileHandle, &FileSize);

        Add(&Result, (int)FileSize.QuadPart);

        DWORD NumBytesRead;
        if (ReadFile(
            FileHandle,         // _In_        HANDLE       hFile
            Result.Data,        // _Out_       LPVOID       lpBuffer
            Result.NumElements, // _In_        DWORD        nNumberOfBytesToRead
            &NumBytesRead,      // _Out_opt_   LPDWORD      lpNumberOfBytesRead
            nullptr))           // _Inout_opt_ LPOVERLAPPED lpOverlapped
        {
            CloseHandle(FileHandle);

            if (NumBytesRead != (DWORD)Result.NumElements)
            {
                // TODO: Diagnostics?
                MTB_Fail("Error reading file.");
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
        // NOTE: Failed to open file.
    }

    return Result;
}

bool
LoadRom(machine* M, size_t RomSize, u8* RomPtr)
{
    bool Result = false;

    if (RomSize <= mtb_ArrayLengthOf(M->ProgramMemory))
    {
        mtb_CopyBytes(RomSize, M->ProgramMemory, RomPtr);
        Result = true;
    }

    return Result;
}

struct win32_front_buffer
{
    BITMAPINFO BitmapInfo;

    colorRGBA8* Pixels;
    DWORD Width;
    DWORD Height;
    DWORD Pitch;
    DWORD BytesPerPixel;

    colorRGBA8 PixelColorOn;
    colorRGBA8 PixelColorOff;
};

static void
Win32SwapBuffers(bool32* ScreenPixels, win32_front_buffer* Front)
{
    colorRGBA8* FrontPixel = Front->Pixels;
    bool32* ScreenPixel = ScreenPixels;

    for (size_t Y = 0; Y < SCREEN_HEIGHT; ++Y)
    {
        for (size_t X = 0; X < SCREEN_WIDTH; ++X, ++ScreenPixel, ++FrontPixel)
        {
            colorRGBA8 NewColor;
            if (*ScreenPixel) NewColor = Front->PixelColorOn;
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
    colorRGBA8* Pixels = FrontBuffer->Pixels;

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

    MTB_INTERNAL_CODE(int Result = ) StretchDIBits(
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
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo{ sizeof(MonitorInfo) };
        if (GetWindowPlacement(WindowHandle, &LocalWindowPosition) &&
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

    if (Message == WM_CLOSE || Message == WM_DESTROY)
    {
        PostQuitMessage(0);
    }
    else if (Message >= WM_KEYFIRST && Message <= WM_KEYLAST
        // Message >= WM_MOUSEFIRST && Message <= WM_MOUSELAST ||
        // Message == WM_INPUT
        )
    {
        if (Message == WM_KEYDOWN || Message == WM_SYSKEYUP)
        {
            u32 VKCode = (u32)WParam;
            bool KeyWasDown = mtb_IsBitSet((u64)LParam, 30);
            bool KeyIsDown = !mtb_IsBitSet((u64)LParam, 31);
            bool KeyWasPressed = !KeyWasDown && KeyIsDown;
            //bool KeyWasReleased = KeyWasDown && !KeyIsDown;
            //bool KeyWasToggled = KeyWasDown != KeyIsDown;

            bool AltKeyModifier = false;
            if (Message == WM_SYSKEYDOWN || Message == WM_SYSKEYUP)
                AltKeyModifier = mtb_IsBitSet((u64)LParam, 29);

            if (KeyWasPressed)
            {
                if (VKCode == VK_ESCAPE)
                {
                    PostQuitMessage(0);
                }

                if (AltKeyModifier && VKCode == VK_F4)
                {
                    PostQuitMessage(0);
                }

                if (AltKeyModifier && VKCode == VK_RETURN)
                {
                    Win32ToggleFullscreenWindow(WindowHandle);
                }
            }

            char VKCodeAsChar = (char)VKCode;
            int KeyIndex = -1;
            // Keyboard mapping / key bindings
            switch (VKCodeAsChar)
            {
                case '1': KeyIndex = 0x1; break;
                case '2': KeyIndex = 0x2; break;
                case '3': KeyIndex = 0x3; break;
                case '4': KeyIndex = 0xC; break;
                case 'Q': KeyIndex = 0x4; break;
                case 'W': KeyIndex = 0x5; break;
                case 'E': KeyIndex = 0x6; break;
                case 'R': KeyIndex = 0xD; break;
                case 'A': KeyIndex = 0x7; break;
                case 'S': KeyIndex = 0x8; break;
                case 'D': KeyIndex = 0x9; break;
                case 'F': KeyIndex = 0xE; break;
                case 'Z': KeyIndex = 0xA; break;
                case 'X': KeyIndex = 0x0; break;
                case 'C': KeyIndex = 0xB; break;
                case 'V': KeyIndex = 0xF; break;
                default: break;
            }

            if (KeyIndex >= 0)
            {
                //u16 KeyIndex_ = MapCharToKeyIndex(KeyIndex);
                u16 OldState = Window->InputState;
                u16 NewState = SetKeyDown(OldState, (u16)KeyIndex, KeyIsDown);
                Window->InputState = NewState;
            }
        }
    }
    else if (Message == WM_SIZE)
    {
        // u32 NewWidth = (u32)(LParam & 0xffff);
        // u32 NewHeight = (u32)((LParam & 0xffff0000) >> 16);
        // TODO: Handle resizing.
    }
    else if (Message == WM_PAINT)
    {
        PAINTSTRUCT Paint;

        RECT Rect;
        bool MustBeginAndEndPaint = !!GetUpdateRect(WindowHandle, &Rect, FALSE);

        if (MustBeginAndEndPaint)
            BeginPaint(WindowHandle, &Paint);

        Win32Present(WindowHandle, &Window->FrontBuffer);

        if (MustBeginAndEndPaint)
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
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch (Message.message)
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

    if (RegisterClassA(&WindowClass))
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
        WindowRect.left += WindowWorkArea.left;
        WindowRect.right += WindowWorkArea.left;
        WindowRect.top += WindowWorkArea.top;
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

        if (WindowHandle)
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
Win32Now()
{
    win32_timestamp Result;
    QueryPerformanceCounter(&Result.Timestamp);
    return Result;
}

static f64
Win32DeltaSeconds(win32_clock* Clock, win32_timestamp End, win32_timestamp Start)
{
    LONGLONG EndInt = End.Timestamp.QuadPart;
    LONGLONG StartInt = Start.Timestamp.QuadPart;
    LONGLONG FrequencyInt = Clock->Frequency.QuadPart;

    LONGLONG Delta = EndInt - StartInt;
    f64 Result = (f64)Delta / (f64)FrequencyInt;
    return Result;
}

// See if input is required in order to continue execution.
static bool
Win32CanTick(machine* M, u16 OldInputState, u16 NewInputState)
{
    // See if input is required in order to continue execution.
    bool CanTick = true;
    if (M->RequiredInputRegisterIndexPlusOne)
    {
        CanTick = false;
        for (u16 KeyIndex = 0; KeyIndex < 16; ++KeyIndex)
        {
            if (!mtb_IsBitSet(OldInputState, KeyIndex) && mtb_IsBitSet(NewInputState, KeyIndex))
            {
                u8* Reg = M->V + (M->RequiredInputRegisterIndexPlusOne - 1);
                MTB_AssertDebug((u16)(u8)KeyIndex == KeyIndex);
                *Reg = (u8)KeyIndex;
                M->RequiredInputRegisterIndexPlusOne = 0;
                CanTick = true;
            }
        }
    }

    return CanTick;
}

static bool
StringEndsWith(size_t StringLength, char const* String, size_t EndLength, char const* End)
{
    bool Result = false;
    if (StringLength >= EndLength)
    {
        Result = mtb_StringsAreEqual(EndLength, String + StringLength - EndLength, End);
    }

    return Result;
}

static bool
StringEndsWith(char const* String, char const* End)
{
    size_t StringLength = mtb_StringLengthOf(String);
    size_t EndLength = mtb_StringLengthOf(End);

    return StringEndsWith(StringLength, String, EndLength, End);
}

static bool
StringEndsWith(char const* String, size_t EndLength, char const* End)
{
    size_t StringLength = mtb_StringLengthOf(String);

    return StringEndsWith(StringLength, String, EndLength, End);
}

static bool
StringEndsWith(size_t StringLength, char const* String, char const* End)
{
    size_t EndLength = mtb_StringLengthOf(End);

    return StringEndsWith(StringLength, String, EndLength, End);
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
    MTB_INTERNAL_CODE(BaseAddress = (LPVOID)(size_t)0x2'000'000);
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
        RomLoaded = LoadRom(M, Rom.Length, Rom.Ptr);
#else
        u8_array FileContents = Win32LoadRomFromFile(FileName);

        if (StringEndsWith(FileName, ".couscous"))
        {
            u8_array ByteCode{};
            AssembleCode((char*)FileContents.Data, (char*)FileContents.Data + FileContents.NumElements, &ByteCode);

            mtb_Swap(ByteCode, FileContents);
            Deallocate(&ByteCode);
        }

        RomLoaded = LoadRom(M, FileContents.NumElements, FileContents.Data);
        Deallocate(&FileContents);
#endif
    }

    if (RomLoaded)
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

        if (Window.Handle)
        {
            //
            // Initialize the front buffer.
            //
            win32_front_buffer* FrontBuffer = &Window.FrontBuffer;
            FrontBuffer->BytesPerPixel = 4;
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
            FrontBuffer->PixelColorOff = { 16, 64, 16, 255 };
            FrontBuffer->PixelColorOn = { 8, 16, 8, 255 };

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

            f64 const FrameTargetSeconds = 1.0 / 55.0; // 55Hz
            int const TicksPerFrame = 15;

            win32_timestamp EndOfLastFrame = Win32Now();

            int PendingTicks = 0;

            while (true)
            {
                PendingTicks += TicksPerFrame;

                // Pump messages while also waiting to be in sync with last frame.
                while (true)
                {
                    Win32MessagePump(&Window);

                    // TODO: Replace this spin-lock with something less wasteful?
                    f64 SecondsSinceLastFrameEnd = Win32DeltaSeconds(&Clock, Win32Now(), EndOfLastFrame);
                    if (SecondsSinceLastFrameEnd >= FrameTargetSeconds)
                    {
                        break;
                    }
                }

                // Apply input.
                u16 OldInputState = M->InputState;
                u16 NewInputState = Window.InputState;
                M->InputState = NewInputState;

                // TODO: Does this belong inside the CanTick-brackets?
                if (M->DT > 0)
                    M->DT -= 1;

                bool CanTick = Win32CanTick(M, OldInputState, NewInputState);
                if (CanTick)
                {
                    while (PendingTicks > 0)
                    {
                        tick_result TickResult = Tick(M);
                        --PendingTicks;

                        if (!TickResult.Continue)
                        {
                            PostQuitMessage(0);
                            break;
                        }
                    }
                }

                Win32SwapBuffers(M->Screen, &Window.FrontBuffer);
                Win32Present(Window.Handle, &Window.FrontBuffer);

                EndOfLastFrame = Win32Now();
            }

            while (true)
            {
                Win32MessagePump(&Window);
            }
        }
        else
        {
            // TODO: Logging?
            MTB_Fail("Unable to open window.");
            return 2;
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
