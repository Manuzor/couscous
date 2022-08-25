// Generated on 2022-08-21 23:27:38

#if !defined(GUARD_GENERATED_str_array)
#define GUARD_GENERATED_str_array

struct str_array
{
    int NumElements;
    int Capacity;
    str* _Data;

    enum { FixedCapacity = 32 };
    str _Fixed[FixedCapacity];

    str* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(str_array* Array, int RequiredCapacity);

static void
SetNumElements(str_array* Array, int NewNumElements);

inline bool
IsValidIndex(str_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static str*
AddN(str_array* Array, int NumToAdd);

inline str*
Add(str_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(str_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(str_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(str_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(str_array* Array);

static str*
At(str_array* Array, int Index);

static void
Deallocate(str_array* Array);

template<typename predicate>
static str*
Find(str_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        str* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_str_array)
