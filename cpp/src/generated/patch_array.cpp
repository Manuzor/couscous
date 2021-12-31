// Generated on 2021-12-31 12:16:59

#if defined(GUARD_GENERATED_patch_array)

static void
Reserve(patch_array* Array, int RequiredCapacity)
{
  if (Array->_Data == nullptr || Array->Capacity == 0)
  {
    Array->Capacity = Array->FixedCapacity;
    Array->_Data = nullptr;
  }

  if (RequiredCapacity > Array->Capacity)
  {
    int NewCapacity = Array->Capacity > 0 ? Array->Capacity : 32;

    while (NewCapacity < RequiredCapacity)
      NewCapacity *= 2;

    void* NewData = malloc(NewCapacity * sizeof(patch));

    if (Array->_Data)
    {
      mtb_CopyBytes(Array->NumElements * sizeof(patch), NewData, Array->_Data);
      free(Array->_Data);
    }
    else
    {
      mtb_CopyBytes(Array->NumElements * sizeof(patch), NewData, Array->_Fixed);
    }

    Array->_Data = (patch*)NewData;
    Array->Capacity = NewCapacity;
  }
}

static void
SetNumElements(patch_array* Array, int NewNumElements)
{
    Reserve(Array, NewNumElements);
    Array->NumElements = NewNumElements;
}

static patch*
AddN(patch_array* Array, int NumToAdd)
{
  Reserve(Array, Array->NumElements + NumToAdd);
  patch* Result = Array->Data() + Array->NumElements;
  Array->NumElements += NumToAdd;

  return Result;
}

static bool
RemoveRange(patch_array* Array, int FirstIndex, int OnePastLastIndex)
{
  bool Result = false;

  if(OnePastLastIndex > FirstIndex && IsValidIndex(Array, FirstIndex) && IsValidIndex(Array, OnePastLastIndex - 1))
  {
      int NumTrailing = Array->NumElements - OnePastLastIndex;
      mtb_CopyBytes(NumTrailing * sizeof(patch), Array->Data() + FirstIndex, Array->Data() + OnePastLastIndex);
      Array->NumElements -= OnePastLastIndex - FirstIndex;

      Result = true;
  }

  return Result;
}

static void
Clear(patch_array* Array)
{
  Array->NumElements = 0;
}

static patch*
At(patch_array* Array, int Index)
{
  MTB_AssertDebug(Index >= 0);
  MTB_AssertDebug(Index < Array->NumElements);
  return Array->Data() + Index;
}

static void
Deallocate(patch_array* Array)
{
  if (Array->_Data)
  {
    free(Array->_Data);
  }
  *Array = {};
}

#endif // defined(GUARD_GENERATED_patch_array)
