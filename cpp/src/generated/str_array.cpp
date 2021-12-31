// Generated on 2021-12-31 12:16:59

#if defined(GUARD_GENERATED_str_array)

static void
Reserve(str_array* Array, int RequiredCapacity)
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

    void* NewData = malloc(NewCapacity * sizeof(str));

    if (Array->_Data)
    {
      mtb_CopyBytes(Array->NumElements * sizeof(str), NewData, Array->_Data);
      free(Array->_Data);
    }
    else
    {
      mtb_CopyBytes(Array->NumElements * sizeof(str), NewData, Array->_Fixed);
    }

    Array->_Data = (str*)NewData;
    Array->Capacity = NewCapacity;
  }
}

static void
SetNumElements(str_array* Array, int NewNumElements)
{
    Reserve(Array, NewNumElements);
    Array->NumElements = NewNumElements;
}

static str*
AddN(str_array* Array, int NumToAdd)
{
  Reserve(Array, Array->NumElements + NumToAdd);
  str* Result = Array->Data() + Array->NumElements;
  Array->NumElements += NumToAdd;

  return Result;
}

static bool
RemoveRange(str_array* Array, int FirstIndex, int OnePastLastIndex)
{
  bool Result = false;

  if(OnePastLastIndex > FirstIndex && IsValidIndex(Array, FirstIndex) && IsValidIndex(Array, OnePastLastIndex - 1))
  {
      int NumTrailing = Array->NumElements - OnePastLastIndex;
      mtb_CopyBytes(NumTrailing * sizeof(str), Array->Data() + FirstIndex, Array->Data() + OnePastLastIndex);
      Array->NumElements -= OnePastLastIndex - FirstIndex;

      Result = true;
  }

  return Result;
}

static void
Clear(str_array* Array)
{
  Array->NumElements = 0;
}

static str*
At(str_array* Array, int Index)
{
  MTB_AssertDebug(Index >= 0);
  MTB_AssertDebug(Index < Array->NumElements);
  return Array->Data() + Index;
}

static void
Deallocate(str_array* Array)
{
  if (Array->_Data)
  {
    free(Array->_Data);
  }
  *Array = {};
}

#endif // defined(GUARD_GENERATED_str_array)
