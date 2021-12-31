// Generated on 2021-12-31 12:16:59

#if defined(GUARD_GENERATED_debug_info_array)

static void
Reserve(debug_info_array* Array, int RequiredCapacity)
{


  if (RequiredCapacity > Array->Capacity)
  {
    int NewCapacity = Array->Capacity > 0 ? Array->Capacity : 32;

    while (NewCapacity < RequiredCapacity)
      NewCapacity *= 2;

    void* NewData = malloc(NewCapacity * sizeof(debug_info));

    if (Array->_Data)
    {
      mtb_CopyBytes(Array->NumElements * sizeof(debug_info), NewData, Array->_Data);
      free(Array->_Data);
    }


    Array->_Data = (debug_info*)NewData;
    Array->Capacity = NewCapacity;
  }
}

static void
SetNumElements(debug_info_array* Array, int NewNumElements)
{
    Reserve(Array, NewNumElements);
    Array->NumElements = NewNumElements;
}

static debug_info*
AddN(debug_info_array* Array, int NumToAdd)
{
  Reserve(Array, Array->NumElements + NumToAdd);
  debug_info* Result = Array->Data() + Array->NumElements;
  Array->NumElements += NumToAdd;

  return Result;
}

static bool
RemoveRange(debug_info_array* Array, int FirstIndex, int OnePastLastIndex)
{
  bool Result = false;

  if(OnePastLastIndex > FirstIndex && IsValidIndex(Array, FirstIndex) && IsValidIndex(Array, OnePastLastIndex - 1))
  {
      int NumTrailing = Array->NumElements - OnePastLastIndex;
      mtb_CopyBytes(NumTrailing * sizeof(debug_info), Array->Data() + FirstIndex, Array->Data() + OnePastLastIndex);
      Array->NumElements -= OnePastLastIndex - FirstIndex;

      Result = true;
  }

  return Result;
}

static void
Clear(debug_info_array* Array)
{
  Array->NumElements = 0;
}

static debug_info*
At(debug_info_array* Array, int Index)
{
  MTB_AssertDebug(Index >= 0);
  MTB_AssertDebug(Index < Array->NumElements);
  return Array->Data() + Index;
}

static void
Deallocate(debug_info_array* Array)
{
  if (Array->_Data)
  {
    free(Array->_Data);
  }
  *Array = {};
}

#endif // defined(GUARD_GENERATED_debug_info_array)
