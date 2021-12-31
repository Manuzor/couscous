// Generated on 2021-12-31 12:16:59

#if !defined(GUARD_GENERATED_debug_info_array)
#define GUARD_GENERATED_debug_info_array

struct debug_info_array
{
  int NumElements;
  int Capacity;
  debug_info* _Data;

  debug_info* Data() { return _Data; }
};

static void
Reserve(debug_info_array* Array, int RequiredCapacity);

static void
SetNumElements(debug_info_array* Array, int NewNumElements);

inline bool
IsValidIndex(debug_info_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static debug_info*
AddN(debug_info_array* Array, int NumToAdd);

inline debug_info*
Add(debug_info_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(debug_info_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(debug_info_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(debug_info_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(debug_info_array* Array);

static debug_info*
At(debug_info_array* Array, int Index);

static void
Deallocate(debug_info_array* Array);

template<typename predicate>
static debug_info*
Find(debug_info_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        debug_info* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_debug_info_array)
