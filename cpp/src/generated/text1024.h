// Generated on 2021-12-31 12:16:59
#pragma once

struct text1024
{
  enum
  {
    FullCapacity = 1024,
    Capacity = FullCapacity - 1, // We reserve one character for the null terminator.
  };

  int Size;
  char Data[FullCapacity];
};

// "Constructor"
static text1024 CreateText1024(strc String);

// To str
static str Str(text1024 Text);

// EnsureZeroTerminated
static void EnsureZeroTerminated(text1024* Text);

// Clear
static void Clear(text1024* Text);

// Trim
static text1024 Trim(text1024 Text);

// Append
static int Append(text1024* Text, strc String);
static int Append(text1024* Text, char Char);

// Comparison
static int Compare(text1024* A, text1024* B);
static bool AreEqual(text1024* A, text1024* B);

