// Generated on 2022-08-21 23:27:38

#if defined(GUARD_GENERATED_label_array)

static void
Reserve(label_array* Array, int RequiredCapacity)
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

        void* NewData = malloc(NewCapacity * sizeof(label));

        if (Array->_Data)
        {
            ::mtb::CopyBytes(NewData, Array->_Data, Array->NumElements * sizeof(label));
            free(Array->_Data);
        }
        else
        {
            ::mtb::CopyBytes(NewData, Array->_Fixed, Array->NumElements * sizeof(label));
        }

        Array->_Data = (label*)NewData;
        Array->Capacity = NewCapacity;
    }
}

static void
SetNumElements(label_array* Array, int NewNumElements)
{
    Reserve(Array, NewNumElements);
    Array->NumElements = NewNumElements;
}

static label*
AddN(label_array* Array, int NumToAdd)
{
    Reserve(Array, Array->NumElements + NumToAdd);
    label* Result = Array->Data() + Array->NumElements;
    Array->NumElements += NumToAdd;

    return Result;
}

static bool
RemoveRange(label_array* Array, int FirstIndex, int OnePastLastIndex)
{
    bool Result = false;

    if(OnePastLastIndex > FirstIndex && IsValidIndex(Array, FirstIndex) && IsValidIndex(Array, OnePastLastIndex - 1))
    {
        int NumTrailing = Array->NumElements - OnePastLastIndex;
        ::mtb::CopyBytes(Array->Data() + FirstIndex, Array->Data() + OnePastLastIndex, NumTrailing * sizeof(label));
        Array->NumElements -= OnePastLastIndex - FirstIndex;

        Result = true;
    }

    return Result;
}

static void
Clear(label_array* Array)
{
    Array->NumElements = 0;
}

static label*
At(label_array* Array, int Index)
{
    MTB_ASSERT(Index >= 0);
    MTB_ASSERT(Index < Array->NumElements);
    return Array->Data() + Index;
}

static void
Deallocate(label_array* Array)
{
    if (Array->_Data)
    {
        free(Array->_Data);
    }
    *Array = {};
}

#endif // defined(GUARD_GENERATED_label_array)
