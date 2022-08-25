// Generated on 2022-08-21 23:27:38
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

