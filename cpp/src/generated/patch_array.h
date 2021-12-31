// Generated on 2021-12-31 12:16:59

#if !defined(GUARD_GENERATED_patch_array)
#define GUARD_GENERATED_patch_array

struct patch_array
{
  int NumElements;
  int Capacity;
  patch* _Data;

enum { FixedCapacity = 32 };
  patch _Fixed[FixedCapacity];

  patch* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(patch_array* Array, int RequiredCapacity);

static void
SetNumElements(patch_array* Array, int NewNumElements);

inline bool
IsValidIndex(patch_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static patch*
AddN(patch_array* Array, int NumToAdd);

inline patch*
Add(patch_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(patch_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(patch_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(patch_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(patch_array* Array);

static patch*
At(patch_array* Array, int Index);

static void
Deallocate(patch_array* Array);

template<typename predicate>
static patch*
Find(patch_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        patch* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_patch_array)
