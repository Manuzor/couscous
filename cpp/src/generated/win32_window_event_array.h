// Generated on 2021-12-31 12:16:59

#if !defined(GUARD_GENERATED_win32_window_event_array)
#define GUARD_GENERATED_win32_window_event_array

struct win32_window_event_array
{
  int NumElements;
  int Capacity;
  win32_window_event* _Data;

enum { FixedCapacity = 32 };
  win32_window_event _Fixed[FixedCapacity];

  win32_window_event* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(win32_window_event_array* Array, int RequiredCapacity);

static void
SetNumElements(win32_window_event_array* Array, int NewNumElements);

inline bool
IsValidIndex(win32_window_event_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static win32_window_event*
AddN(win32_window_event_array* Array, int NumToAdd);

inline win32_window_event*
Add(win32_window_event_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(win32_window_event_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(win32_window_event_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(win32_window_event_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(win32_window_event_array* Array);

static win32_window_event*
At(win32_window_event_array* Array, int Index);

static void
Deallocate(win32_window_event_array* Array);

template<typename predicate>
static win32_window_event*
Find(win32_window_event_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        win32_window_event* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_win32_window_event_array)