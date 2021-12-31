// Generated on 2021-12-31 12:16:59

#if !defined(GUARD_GENERATED_token_array)
#define GUARD_GENERATED_token_array

struct token_array
{
  int NumElements;
  int Capacity;
  token* _Data;

enum { FixedCapacity = 8 };
  token _Fixed[FixedCapacity];

  token* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(token_array* Array, int RequiredCapacity);

static void
SetNumElements(token_array* Array, int NewNumElements);

inline bool
IsValidIndex(token_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static token*
AddN(token_array* Array, int NumToAdd);

inline token*
Add(token_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(token_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(token_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(token_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(token_array* Array);

static token*
At(token_array* Array, int Index);

static void
Deallocate(token_array* Array);

template<typename predicate>
static token*
Find(token_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        token* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_token_array)
