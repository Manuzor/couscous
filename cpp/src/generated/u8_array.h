// Generated on 2021-12-31 12:16:59

#if !defined(GUARD_GENERATED_u8_array)
#define GUARD_GENERATED_u8_array

struct u8_array
{
  int NumElements;
  int Capacity;
  u8* _Data;

enum { FixedCapacity = 32 };
  u8 _Fixed[FixedCapacity];

  u8* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(u8_array* Array, int RequiredCapacity);

static void
SetNumElements(u8_array* Array, int NewNumElements);

inline bool
IsValidIndex(u8_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static u8*
AddN(u8_array* Array, int NumToAdd);

inline u8*
Add(u8_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(u8_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(u8_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(u8_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(u8_array* Array);

static u8*
At(u8_array* Array, int Index);

static void
Deallocate(u8_array* Array);

template<typename predicate>
static u8*
Find(u8_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        u8* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_u8_array)
