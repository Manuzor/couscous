// Generated on 2021-12-31 12:16:59
#pragma once

struct token
{
  enum
  {
    FullCapacity = 32,
    Capacity = FullCapacity - 1, // We reserve one character for the null terminator.
  };

  int Size;
  char Data[FullCapacity];
};

// "Constructor"
static token CreateToken(strc String);

// To str
static str Str(token Text);

// EnsureZeroTerminated
static void EnsureZeroTerminated(token* Text);

// Clear
static void Clear(token* Text);

// Trim
static token Trim(token Text);

// Append
static int Append(token* Text, strc String);
static int Append(token* Text, char Char);

// Comparison
static int Compare(token* A, token* B);
static bool AreEqual(token* A, token* B);

