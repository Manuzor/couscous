// Generated on 2021-12-31 12:16:59
#pragma once

struct text
{
  enum
  {
    FullCapacity = 128,
    Capacity = FullCapacity - 1, // We reserve one character for the null terminator.
  };

  int Size;
  char Data[FullCapacity];
};

// "Constructor"
static text CreateText(strc String);

// To str
static str Str(text Text);

// EnsureZeroTerminated
static void EnsureZeroTerminated(text* Text);

// Clear
static void Clear(text* Text);

// Trim
static text Trim(text Text);

// Append
static int Append(text* Text, strc String);
static int Append(text* Text, char Char);

// Comparison
static int Compare(text* A, text* B);
static bool AreEqual(text* A, text* B);

