// Generated on 2021-12-31 12:16:59

#if !defined(GUARD_GENERATED_cursor_array)
#define GUARD_GENERATED_cursor_array

struct cursor_array
{
  int NumElements;
  int Capacity;
  parser_cursor* _Data;

enum { FixedCapacity = 8 };
  parser_cursor _Fixed[FixedCapacity];

  parser_cursor* Data() { return _Data ? _Data : _Fixed; }
};

static void
Reserve(cursor_array* Array, int RequiredCapacity);

static void
SetNumElements(cursor_array* Array, int NewNumElements);

inline bool
IsValidIndex(cursor_array* Array, int Index) { return 0 <= Index && Index < Array->NumElements; }

static parser_cursor*
AddN(cursor_array* Array, int NumToAdd);

inline parser_cursor*
Add(cursor_array* Array) { return AddN(Array, 1); }

static bool
RemoveRange(cursor_array* Array, int FirstIndex, int OnePastLastIndex);

inline bool
RemoveN(cursor_array* Array, int FirstIndex, int NumToRemove) { return RemoveRange(Array, FirstIndex, FirstIndex + NumToRemove); }

inline bool
Remove(cursor_array* Array, int Index) { return RemoveRange(Array, Index, Index + 1); }

static void
Clear(cursor_array* Array);

static parser_cursor*
At(cursor_array* Array, int Index);

static void
Deallocate(cursor_array* Array);

template<typename predicate>
static parser_cursor*
Find(cursor_array* Array, predicate Predicate)
{
    for (int Index = 0; Index < Array->NumElements; ++Index)
    {
        parser_cursor* Item = Array->Data() + Index;
        if (Predicate(Item))
        {
            return Item;
        }
    }

    return nullptr;
}

#endif // !defined(GUARD_GENERATED_cursor_array)
