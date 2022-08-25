// Generated on 2022-08-21 23:27:38

#if !defined(GUARD_GENERATED_label_array)
#define GUARD_GENERATED_label_array

struct label_array
{
    int NumElements;
    int Capacity;
    label* _Data;

    enum { FixedCapacity = 32 };
    label _Fixed[FixedCapacity];

    label* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(label_array* Array, int RequiredCapacity);

static void
SetNumElements(label_array* Array, int NewNumElements);

inline bool
IsValidIndex(label_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static label*
AddN(label_array* Array, int NumToAdd);

inline label*
Add(label_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(label_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(label_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(label_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(label_array* Array);

static label*
At(label_array* Array, int Index);

static void
Deallocate(label_array* Array);

template<typename predicate>
static label*
Find(label_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        label* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_label_array)
