// Generated on 2022-08-21 23:27:38

#if !defined(GUARD_GENERATED_breakpoint_array)
#define GUARD_GENERATED_breakpoint_array

struct breakpoint_array
{
    int NumElements;
    int Capacity;
    breakpoint* _Data;

    enum { FixedCapacity = 32 };
    breakpoint _Fixed[FixedCapacity];

    breakpoint* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(breakpoint_array* Array, int RequiredCapacity);

static void
SetNumElements(breakpoint_array* Array, int NewNumElements);

inline bool
IsValidIndex(breakpoint_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static breakpoint*
AddN(breakpoint_array* Array, int NumToAdd);

inline breakpoint*
Add(breakpoint_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(breakpoint_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(breakpoint_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(breakpoint_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(breakpoint_array* Array);

static breakpoint*
At(breakpoint_array* Array, int Index);

static void
Deallocate(breakpoint_array* Array);

template<typename predicate>
static breakpoint*
Find(breakpoint_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        breakpoint* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_breakpoint_array)
