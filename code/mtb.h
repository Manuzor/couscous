#if !defined(MTB_HEADER_mtb_platform)
#define MTB_HEADER_mtb_platform

// NOTE: Only c++ enabled for now.
#define MTB_CPP __cplusplus


//
// Clean preprocessor flags
//

#define MTB_ON +
#define MTB_OFF -

// Usage:
//   #define MTB_FLAG_FOO MTB_ON
//   #if MTB_FLAG(FOO) /* ... */
#define MTB_FLAG(OP) (!!(1 MTB_FLAG_##OP 1))


//
// MTB_CURRENT_COMPILER
// Determine the compiler
//
#define MTB_COMPILER_TYPE_UNKNOWN  0
#define MTB_COMPILER_TYPE_MSVC    10
#define MTB_COMPILER_TYPE_GCC     20
#define MTB_COMPILER_TYPE_CLANG   30

#if defined(_MSC_VER)
  #define MTB_CURRENT_COMPILER MTB_COMPILER_TYPE_MSVC
#else
  #define MTB_CURRENT_COMPILER MTB_COMPILER_TYPE_UNKNOWN
  #error "Unknown compiler."
#endif


//
// MTB_CURRENT_OS
// Determine the operating system
//
#define MTB_OS_TYPE_UNKNOWN  0
#define MTB_OS_TYPE_WINDOWS 10
#define MTB_OS_TYPE_LINUX   20
#define MTB_OS_TYPE_OSX     30


#if defined(_WIN32) || defined(_WIN64)
  #define MTB_CURRENT_OS MTB_OS_TYPE_WINDOWS
#else
  // TODO: Emit some kind of warning instead?
  #error "Unknown operating system."
  #define MTB_CURRENT_OS MTB_OS_TYPE_UNKNOWN
#endif


//
// MTB_CURRENT_ARCH
// Determine the architecture and bitness
//
#define MTB_ARCH_TYPE_UNKNOWN  0
#define MTB_ARCH_TYPE_x86     10
#define MTB_ARCH_TYPE_x64     20 // aka x86_64, amd64

#if defined(_M_X64) || defined(_M_AMD64)
  #define MTB_CURRENT_ARCH MTB_ARCH_TYPE_x64
#elif defined(_M_IX86) || defined(_M_I86)
  #define MTB_CURRENT_ARCH MTB_ARCH_TYPE_x86
#else
  #define MTB_CURRENT_ARCH MTB_ARCH_TYPE_UNKNOWN
  #error "Unknown architecture."
#endif

#if MTB_CURRENT_ARCH == MTB_ARCH_TYPE_x64
  #define MTB_CURRENT_ARCH_BITNESS 64
  #define MTB_FLAG_LITTLE_ENDIAN MTB_ON
  #define MTB_FLAG_BIG_ENDIAN MTB_OFF
#elif MTB_CURRENT_ARCH == MTB_ARCH_TYPE_x86
  #define MTB_CURRENT_ARCH_BITNESS 32
  #define MTB_FLAG_LITTLE_ENDIAN MTB_ON
  #define MTB_FLAG_BIG_ENDIAN MTB_OFF
#else
  #define MTB_CURRENT_ARCH_BITNESS 0
  #define MTB_FLAG_LITTLE_ENDIAN MTB_OFF
  #define MTB_FLAG_BIG_ENDIAN MTB_OFF
  #error "Undefined architecture."
#endif

#if !defined(MTB_CURRENT_ARCH_BITNESS) || !defined(MTB_FLAG_LITTLE_ENDIAN) || !defined(MTB_FLAG_BIG_ENDIAN)
  #error Invalid settings.
#endif


//
// Utility stuff
//

#define MTB_CURRENT_FILE __FILE__
#define MTB_CURRENT_LINE __LINE__

#if defined(__PRETTY_FUNCTION__)
  #define MTB_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__func__)
  #define MTB_CURRENT_FUNCTION __func__
#elif defined(__FUNCTION__)
  #define MTB_CURRENT_FUNCTION __FUNCTION__
#else
  #define MTB_CURRENT_FUNCTION ""
#endif

#define MTB_IMPL_CONCAT_L2(LEFT, RIGHT) LEFT##RIGHT
#define MTB_IMPL_CONCAT_L1(LEFT, RIGHT) MTB_IMPL_CONCAT_L2(LEFT, RIGHT)
#define MTB_CONCAT(LEFT, RIGHT) MTB_IMPL_CONCAT_L1(LEFT, RIGHT)

#define MTB_IMPL_STRINGIFY_1(WHAT) #WHAT
#define MTB_IMPL_STRINGIFY_0(WHAT) MTB_IMPL_STRINGIFY_1(WHAT)
#define MTB_STRINGIFY(WHAT) MTB_IMPL_STRINGIFY_0(WHAT)

// A no-op that can be safely terminated with a semicolon.
#if !defined(MTB_NOP)
  #define MTB_NOP do{}while(0)
#endif


// Define a macro that has the strongest inline directive for the current
// compiler.
//
// If you need only a hint for the compiler, use the keyword `inline`
// directly.
//
// Usage:
//   MTB_INLINE void func() { /* ... */ }
#if !defined(MTB_INLINE)
  #if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
    #define MTB_INLINE __forceinline
  #else
    // TODO: Emit a warning here?
    // Fallback to `inline`
    #define MTB_INLINE inline
  #endif
#endif

//
// Determine build flags.
//
#if !defined(MTB_FLAG_RELEASE)
  #if defined(NDEBUG)
    #define MTB_FLAG_RELEASE MTB_ON
  #else
    #define MTB_FLAG_RELEASE MTB_OFF
  #endif
#endif

#if !defined(MTB_FLAG_DEBUG)
  #if defined(_DEBUG) || defined(DEBUG) || !MTB_FLAG(RELEASE)
    #define MTB_FLAG_DEBUG MTB_ON
  #else
    #define MTB_FLAG_DEBUG MTB_OFF
  #endif
#endif

#if !defined(MTB_FLAG_INTERNAL)
  #if !MTB_FLAG(RELEASE)
    #define MTB_FLAG_INTERNAL MTB_ON
  #else
    #define MTB_FLAG_INTERNAL MTB_OFF
  #endif
#endif

#if !defined(MTB_FLAG_BOUNDS_CHECKING)
  #if !MTB_FLAG(RELEASE)
    #define MTB_FLAG_BOUNDS_CHECKING MTB_ON
  #else
    #define MTB_FLAG_BOUNDS_CHECKING MTB_OFF
  #endif
#endif

// Validate settings
#if !(MTB_FLAG(DEBUG) + MTB_FLAG(RELEASE) == 1)
  #pragma message("DEBUG: " MTB_STRINGIFY(MTB_FLAG(DEBUG)))
  #pragma message("RELEASE: " MTB_STRINGIFY(MTB_FLAG(RELEASE)))
  #error Only one of these may be set at a time: MTB_FLAG_DEBUG, MTB_FLAG_DEV, MTB_FLAG_RELEASE
#endif

// Macro to enclose code that is only compiled in in the corresponding build type.
//
// Usage:
//   MTB_DEBUG_CODE(auto result = ) someFunctionCall();
//   MTB_DEBUG_CODE(if(result == 0) { /* ... */ })
#if !defined(MTB_DEBUG_CODE)
  #if MTB_FLAG(DEBUG)
    #define MTB_DEBUG_CODE(...) __VA_ARGS__
  #else
    #define MTB_DEBUG_CODE(...)
  #endif
#endif

#if !defined(MTB_INTERNAL_CODE)
  #if MTB_FLAG(INTERNAL)
    #define MTB_INTERNAL_CODE(...) __VA_ARGS__
  #else
    #define MTB_INTERNAL_CODE(...)
  #endif
#endif

#if !defined(MTB_DebugBreak)
  #if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
    #define MTB_DebugBreak() do { __debugbreak(); } while(0)
  #else
    #define MTB_DebugBreak() MTB_NOP
  #endif
#endif

#if !defined(MTB_FLAG_HAS_EXCEPTIONS)
  #define MTB_FLAG_HAS_EXCEPTIONS MTB_OFF
#endif

// For STL
// NOTE: Don't undef to inform the user that we're overwriting their settings
// if they specified it.
#if !defined(_HAS_EXCEPTIONS)
  #if MTB_FLAG(HAS_EXCEPTIONS)
    #define _HAS_EXCEPTIONS 1
  #else
    #define _HAS_EXCEPTIONS 0
  #endif
#endif


//
// Primitive types.
//
// TODO: Multiple platforms.

using mtb_int = signed int;
using mtb_uint = unsigned int;

using mtb_s08 = signed char;
using mtb_s16 = signed short;
using mtb_s32 = signed int;
using mtb_s64 = signed long long;

using mtb_u08 = unsigned char;
using mtb_u16 = unsigned short;
using mtb_u32 = unsigned int;
using mtb_u64 = unsigned long long;

using mtb_f32 = float;
using mtb_f64 = double;

using mtb_byte = mtb_u08;


#endif // !defined(MTB_HEADER_mtb_platform)
#if !defined(MTB_HEADER_mtb_common)
#define MTB_HEADER_mtb_common

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_COMMON_IMPLEMENTATION)
  #define MTB_COMMON_IMPLEMENTATION
#endif

#if defined(MTB_COMMON_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


// ==========================================
// mtb_platform.h
// ==========================================

#if !defined(MTB_HEADER_mtb_platform)
#define MTB_HEADER_mtb_platform

// NOTE: Only c++ enabled for now.
#define MTB_CPP __cplusplus


//
// Clean preprocessor flags
//

#define MTB_ON +
#define MTB_OFF -

// Usage:
//   #define MTB_FLAG_FOO MTB_ON
//   #if MTB_FLAG(FOO) /* ... */
#define MTB_FLAG(OP) (!!(1 MTB_FLAG_##OP 1))


//
// MTB_CURRENT_COMPILER
// Determine the compiler
//
#define MTB_COMPILER_TYPE_UNKNOWN  0
#define MTB_COMPILER_TYPE_MSVC    10
#define MTB_COMPILER_TYPE_GCC     20
#define MTB_COMPILER_TYPE_CLANG   30

#if defined(_MSC_VER)
  #define MTB_CURRENT_COMPILER MTB_COMPILER_TYPE_MSVC
#else
  #define MTB_CURRENT_COMPILER MTB_COMPILER_TYPE_UNKNOWN
  #error "Unknown compiler."
#endif


//
// MTB_CURRENT_OS
// Determine the operating system
//
#define MTB_OS_TYPE_UNKNOWN  0
#define MTB_OS_TYPE_WINDOWS 10
#define MTB_OS_TYPE_LINUX   20
#define MTB_OS_TYPE_OSX     30


#if defined(_WIN32) || defined(_WIN64)
  #define MTB_CURRENT_OS MTB_OS_TYPE_WINDOWS
#else
  // TODO: Emit some kind of warning instead?
  #error "Unknown operating system."
  #define MTB_CURRENT_OS MTB_OS_TYPE_UNKNOWN
#endif


//
// MTB_CURRENT_ARCH
// Determine the architecture and bitness
//
#define MTB_ARCH_TYPE_UNKNOWN  0
#define MTB_ARCH_TYPE_x86     10
#define MTB_ARCH_TYPE_x64     20 // aka x86_64, amd64

#if defined(_M_X64) || defined(_M_AMD64)
  #define MTB_CURRENT_ARCH MTB_ARCH_TYPE_x64
#elif defined(_M_IX86) || defined(_M_I86)
  #define MTB_CURRENT_ARCH MTB_ARCH_TYPE_x86
#else
  #define MTB_CURRENT_ARCH MTB_ARCH_TYPE_UNKNOWN
  #error "Unknown architecture."
#endif

#if MTB_CURRENT_ARCH == MTB_ARCH_TYPE_x64
  #define MTB_CURRENT_ARCH_BITNESS 64
  #define MTB_FLAG_LITTLE_ENDIAN MTB_ON
  #define MTB_FLAG_BIG_ENDIAN MTB_OFF
#elif MTB_CURRENT_ARCH == MTB_ARCH_TYPE_x86
  #define MTB_CURRENT_ARCH_BITNESS 32
  #define MTB_FLAG_LITTLE_ENDIAN MTB_ON
  #define MTB_FLAG_BIG_ENDIAN MTB_OFF
#else
  #define MTB_CURRENT_ARCH_BITNESS 0
  #define MTB_FLAG_LITTLE_ENDIAN MTB_OFF
  #define MTB_FLAG_BIG_ENDIAN MTB_OFF
  #error "Undefined architecture."
#endif

#if !defined(MTB_CURRENT_ARCH_BITNESS) || !defined(MTB_FLAG_LITTLE_ENDIAN) || !defined(MTB_FLAG_BIG_ENDIAN)
  #error Invalid settings.
#endif


//
// Utility stuff
//

#define MTB_CURRENT_FILE __FILE__
#define MTB_CURRENT_LINE __LINE__

#if defined(__PRETTY_FUNCTION__)
  #define MTB_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__func__)
  #define MTB_CURRENT_FUNCTION __func__
#elif defined(__FUNCTION__)
  #define MTB_CURRENT_FUNCTION __FUNCTION__
#else
  #define MTB_CURRENT_FUNCTION ""
#endif

#define MTB_IMPL_CONCAT_L2(LEFT, RIGHT) LEFT##RIGHT
#define MTB_IMPL_CONCAT_L1(LEFT, RIGHT) MTB_IMPL_CONCAT_L2(LEFT, RIGHT)
#define MTB_CONCAT(LEFT, RIGHT) MTB_IMPL_CONCAT_L1(LEFT, RIGHT)

#define MTB_IMPL_STRINGIFY_1(WHAT) #WHAT
#define MTB_IMPL_STRINGIFY_0(WHAT) MTB_IMPL_STRINGIFY_1(WHAT)
#define MTB_STRINGIFY(WHAT) MTB_IMPL_STRINGIFY_0(WHAT)

// A no-op that can be safely terminated with a semicolon.
#if !defined(MTB_NOP)
  #define MTB_NOP do{}while(0)
#endif


// Define a macro that has the strongest inline directive for the current
// compiler.
//
// If you need only a hint for the compiler, use the keyword `inline`
// directly.
//
// Usage:
//   MTB_INLINE void func() { /* ... */ }
#if !defined(MTB_INLINE)
  #if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
    #define MTB_INLINE __forceinline
  #else
    // TODO: Emit a warning here?
    // Fallback to `inline`
    #define MTB_INLINE inline
  #endif
#endif

//
// Determine build flags.
//
#if !defined(MTB_FLAG_RELEASE)
  #if defined(NDEBUG)
    #define MTB_FLAG_RELEASE MTB_ON
  #else
    #define MTB_FLAG_RELEASE MTB_OFF
  #endif
#endif

#if !defined(MTB_FLAG_DEBUG)
  #if defined(_DEBUG) || defined(DEBUG) || !MTB_FLAG(RELEASE)
    #define MTB_FLAG_DEBUG MTB_ON
  #else
    #define MTB_FLAG_DEBUG MTB_OFF
  #endif
#endif

#if !defined(MTB_FLAG_INTERNAL)
  #if !MTB_FLAG(RELEASE)
    #define MTB_FLAG_INTERNAL MTB_ON
  #else
    #define MTB_FLAG_INTERNAL MTB_OFF
  #endif
#endif

#if !defined(MTB_FLAG_BOUNDS_CHECKING)
  #if !MTB_FLAG(RELEASE)
    #define MTB_FLAG_BOUNDS_CHECKING MTB_ON
  #else
    #define MTB_FLAG_BOUNDS_CHECKING MTB_OFF
  #endif
#endif

// Validate settings
#if !(MTB_FLAG(DEBUG) + MTB_FLAG(RELEASE) == 1)
  #pragma message("DEBUG: " MTB_STRINGIFY(MTB_FLAG(DEBUG)))
  #pragma message("RELEASE: " MTB_STRINGIFY(MTB_FLAG(RELEASE)))
  #error Only one of these may be set at a time: MTB_FLAG_DEBUG, MTB_FLAG_DEV, MTB_FLAG_RELEASE
#endif

// Macro to enclose code that is only compiled in in the corresponding build type.
//
// Usage:
//   MTB_DEBUG_CODE(auto result = ) someFunctionCall();
//   MTB_DEBUG_CODE(if(result == 0) { /* ... */ })
#if !defined(MTB_DEBUG_CODE)
  #if MTB_FLAG(DEBUG)
    #define MTB_DEBUG_CODE(...) __VA_ARGS__
  #else
    #define MTB_DEBUG_CODE(...)
  #endif
#endif

#if !defined(MTB_INTERNAL_CODE)
  #if MTB_FLAG(INTERNAL)
    #define MTB_INTERNAL_CODE(...) __VA_ARGS__
  #else
    #define MTB_INTERNAL_CODE(...)
  #endif
#endif

#if !defined(MTB_DebugBreak)
  #if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
    #define MTB_DebugBreak() do { __debugbreak(); } while(0)
  #else
    #define MTB_DebugBreak() MTB_NOP
  #endif
#endif

#if !defined(MTB_FLAG_HAS_EXCEPTIONS)
  #define MTB_FLAG_HAS_EXCEPTIONS MTB_OFF
#endif

// For STL
// NOTE: Don't undef to inform the user that we're overwriting their settings
// if they specified it.
#if !defined(_HAS_EXCEPTIONS)
  #if MTB_FLAG(HAS_EXCEPTIONS)
    #define _HAS_EXCEPTIONS 1
  #else
    #define _HAS_EXCEPTIONS 0
  #endif
#endif


//
// Primitive types.
//
// TODO: Multiple platforms.

using mtb_int = signed int;
using mtb_uint = unsigned int;

using mtb_s08 = signed char;
using mtb_s16 = signed short;
using mtb_s32 = signed int;
using mtb_s64 = signed long long;

using mtb_u08 = unsigned char;
using mtb_u16 = unsigned short;
using mtb_u32 = unsigned int;
using mtb_u64 = unsigned long long;

using mtb_f32 = float;
using mtb_f64 = double;

using mtb_byte = mtb_u08;


#endif // !defined(MTB_HEADER_mtb_platform)

// ==========================================
// mtb_assert.h
// ==========================================

#if !defined(MTB_HEADER_mtb_assert)
#define MTB_HEADER_mtb_assert

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_ASSERT_IMPLEMENTATION)
  #define MTB_ASSERT_IMPLEMENTATION
#endif

#if defined(MTB_ASSERT_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif



/// Returning \c true means that the a debug break is required.
using mtb_assert_handler = bool(*)(
  char const* FileName,     //< The name of the file where the assert is.
  size_t      Line,         //< The line number in the file where the assert is.
  char const* FunctionName, //< The name of the function where the assert is.
  char const* Expression,   //< The condition as captured by the MTB_Assert macro. May be nullptr in case of MTB_Fail.
  char const* Message);     //< The user defined message.

mtb_assert_handler mtb_GetAssertHandler();
void mtb_SetAssertHandler(mtb_assert_handler AssertHandler);

bool mtb_OnFailedCheck(
  char const* FileName,
  size_t      Line,
  char const* FunctionName,
  char const* Expression,
  char const* MessageFormat = nullptr,
  ...);


//
// Define the actual assertion macro.
//
#if !defined(MTB_Require)
  #define MTB_Require(CONDITION, ...) \
    do \
    { \
      if(!(CONDITION)) \
      { \
        if(mtb_OnFailedCheck(MTB_CURRENT_FILE, MTB_CURRENT_LINE, MTB_CURRENT_FUNCTION, #CONDITION, __VA_ARGS__)) \
        { \
          MTB_DebugBreak(); \
        } \
      } \
    } while(0)
#endif


//
// Define assertion macros
//

#if !defined(MTB_AssertDebug)
  #if MTB_FLAG(DEBUG)
    #define MTB_AssertDebug MTB_Require
  #else
    #define MTB_AssertDebug(...) MTB_NOP
  #endif
#endif

#if !defined(MTB_AssertRelease)
  #if MTB_FLAG(RELEASE)
    #define MTB_AssertRelease MTB_Require
  #else
    #define MTB_AssertRelease(...) MTB_NOP
  #endif
#endif

#if !defined(MTB_AssertInternal)
  #if MTB_FLAG(INTERNAL)
    #define MTB_AssertInternal MTB_Require
  #else
    #define MTB_AssertInternal(...) MTB_NOP
  #endif
#endif

#if !defined(MTB_BoundsCheck)
  #if MTB_FLAG(BOUNDS_CHECKING)
    #define MTB_BoundsCheck MTB_Require
  #else
    #define MTB_BoundsCheck(...) MTB_NOP
  #endif
#endif


//
// Macro to mark an invalid code path.
//
#if !defined(MTB_Fail)
  #define MTB_Fail(...) \
    do \
    { \
      if(mtb_OnFailedCheck(MTB_CURRENT_FILE, MTB_CURRENT_LINE, MTB_CURRENT_FUNCTION, nullptr, __VA_ARGS__)) \
      { \
        MTB_DebugBreak(); \
      } \
    } while(0)
#endif

#if !defined(MTB_NOT_IMPLEMENTED)
  #define MTB_NOT_IMPLEMENTED MTB_Fail("Not implemented.")
#endif

#if !defined(MTB_INVALID_CODE_PATH)
  #define MTB_INVALID_CODE_PATH MTB_Fail("Invalid code path.")
#endif


#endif // !defined(MTB_HEADER_mtb_assert)


//
// Implementation
//
#if defined(MTB_ASSERT_IMPLEMENTATION)

#if !defined(MTB_IMPL_mtb_assert)
#define MTB_IMPL_mtb_assert


static bool mtb__DefaultAssertHandler(
  char const* FileName,
  size_t      Line,
  char const* FunctionName,
  char const* Expression,
  char const* Message)
{
  // Don't do anything special, just trigger a debug break by returning true.
  return true;
}

static mtb_assert_handler mtb__GlobalAssertHandler = &mtb__DefaultAssertHandler;

mtb_assert_handler
mtb_GetAssertHandler()
{
  return mtb__GlobalAssertHandler;
}

void
mtb_SetAssertHandler(mtb_assert_handler AssertHandler)
{
  mtb__GlobalAssertHandler = AssertHandler;
}

bool
mtb_OnFailedCheck(
  char const* FileName,
  size_t      Line,
  char const* FunctionName,
  char const* Expression,
  char const* MessageFormat,
  ...)
{
  bool Result = true;

  if(mtb__GlobalAssertHandler)
  {
  // TODO: Format message.
  // NOTE: Ensure message is a valid string, so at least let it be empty.
    char const* Message = MessageFormat ? MessageFormat : "";
    Result = (*mtb__GlobalAssertHandler)(FileName, Line, FunctionName, Expression, Message);
  }

  return Result;
}


#endif // !defined(MTB_IMPL_mtb_assert)
#endif // defined(MTB_ASSERT_IMPLEMENTATION)


//
// Primitive array types.
//

/// Defines some array variants of types for better readability when used as
/// function parameters.
///
/// For example, a function `Foo` that accepts and array of 4 floats by
/// reference-to-const looks like this:
/// \code
/// void Foo(mtb_f32 const (&ParamName)[4]);
/// \endcode
///
/// Using these typedefs, this can be transformed into:
/// \code
/// void Foo(mtb_f32_4 const& ParamName);
/// \endcode
#define MTB_DEFINE_ARRAY_TYPES(TheType)\
  using TheType##_1   = TheType[1];\
  using TheType##_2   = TheType[2];\
  using TheType##_3   = TheType[3];\
  using TheType##_4   = TheType[4];\
  using TheType##_1x1 = TheType##_1[1];\
  using TheType##_1x2 = TheType##_1[2];\
  using TheType##_1x3 = TheType##_1[3];\
  using TheType##_1x4 = TheType##_1[4];\
  using TheType##_2x1 = TheType##_2[1];\
  using TheType##_2x2 = TheType##_2[2];\
  using TheType##_2x3 = TheType##_2[3];\
  using TheType##_2x4 = TheType##_2[4];\
  using TheType##_3x1 = TheType##_3[1];\
  using TheType##_3x2 = TheType##_3[2];\
  using TheType##_3x3 = TheType##_3[3];\
  using TheType##_3x4 = TheType##_3[4];\
  using TheType##_4x1 = TheType##_4[1];\
  using TheType##_4x2 = TheType##_4[2];\
  using TheType##_4x3 = TheType##_4[3];\
  using TheType##_4x4 = TheType##_4[4]

MTB_DEFINE_ARRAY_TYPES(mtb_s08);   // mtb_s08   => mtb_s08_1,   mtb_s08_2,   ..., mtb_s08_4x3,   mtb_s08_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_s16);   // mtb_s16   => mtb_s16_1,   mtb_s16_2,   ..., mtb_s16_4x3,   mtb_s16_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_s32);   // mtb_s32   => mtb_s32_1,   mtb_s32_2,   ..., mtb_s32_4x3,   mtb_s32_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_s64);   // mtb_s64   => mtb_s64_1,   mtb_s64_2,   ..., mtb_s64_4x3,   mtb_s64_4x4

MTB_DEFINE_ARRAY_TYPES(mtb_u08);  // mtb_u08  => mtb_u08_1,  mtb_u08_2,  ..., mtb_u08_4x3,  mtb_u08_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_u16);  // mtb_u16  => mtb_u16_1,  mtb_u16_2,  ..., mtb_u16_4x3,  mtb_u16_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_u32);  // mtb_u32  => mtb_u32_1,  mtb_u32_2,  ..., mtb_u32_4x3,  mtb_u32_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_u64);  // mtb_u64  => mtb_u64_1,  mtb_u64_2,  ..., mtb_u64_4x3,  mtb_u64_4x4

MTB_DEFINE_ARRAY_TYPES(mtb_f32); // mtb_f32 => mtb_f32_1, mtb_f32_2, ..., mtb_f32_4x3, mtb_f32_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_f64); // mtb_f64 => mtb_f64_1, mtb_f64_2, ..., mtb_f64_4x3, mtb_f64_4x4


//
// ============================
//

constexpr mtb_u64 mtb_KiB(mtb_u64 Amount) { return { Amount * 1024 }; }
constexpr mtb_u64 mtb_MiB(mtb_u64 Amount) { return { Amount * 1024 * 1024 }; }
constexpr mtb_u64 mtb_GiB(mtb_u64 Amount) { return { Amount * 1024 * 1024 * 1024 }; }
constexpr mtb_u64 mtb_TiB(mtb_u64 Amount) { return { Amount * 1024 * 1024 * 1024 * 1024 }; }

constexpr mtb_u64 mtb_KB(mtb_u64 Amount) { return { Amount * 1000 }; }
constexpr mtb_u64 mtb_MB(mtb_u64 Amount) { return { Amount * 1000 * 1000 }; }
constexpr mtb_u64 mtb_GB(mtb_u64 Amount) { return { Amount * 1000 * 1000 * 1000 }; }
constexpr mtb_u64 mtb_TB(mtb_u64 Amount) { return { Amount * 1000 * 1000 * 1000 * 1000 }; }

constexpr mtb_f64 mtb_ToKiB(mtb_u64 Size) { return (mtb_f64)Size / 1024; }
constexpr mtb_f64 mtb_ToMiB(mtb_u64 Size) { return (mtb_f64)Size / 1024 / 1024; }
constexpr mtb_f64 mtb_ToGiB(mtb_u64 Size) { return (mtb_f64)Size / 1024 / 1024 / 1024; }
constexpr mtb_f64 mtb_ToTiB(mtb_u64 Size) { return (mtb_f64)Size / 1024 / 1024 / 1024 / 1024; }

constexpr mtb_f64 mtb_ToKB(mtb_u64 Size) { return (mtb_f64)Size / 1000; }
constexpr mtb_f64 mtb_ToMB(mtb_u64 Size) { return (mtb_f64)Size / 1000 / 1000; }
constexpr mtb_f64 mtb_ToGB(mtb_u64 Size) { return (mtb_f64)Size / 1000 / 1000 / 1000; }
constexpr mtb_f64 mtb_ToTB(mtb_u64 Size) { return (mtb_f64)Size / 1000 / 1000 / 1000 / 1000; }



//
// ============================
//
constexpr mtb_u32 mtb_SetBit(mtb_u32 Bits, mtb_u32 Position)   { return Bits | (mtb_u32(1) << Position); }
constexpr mtb_u32 mtb_UnsetBit(mtb_u32 Bits, mtb_u32 Position) { return Bits & ~(mtb_u32(1) << Position); }
constexpr bool mtb_IsBitSet(mtb_u32 Bits, mtb_u32 Position) { return !!(Bits & (mtb_u32(1) << Position)); }

constexpr mtb_u64 mtb_SetBit(mtb_u64 Bits, mtb_u64 Position)   { return Bits | (mtb_u64(1) << Position); }
constexpr mtb_u64 mtb_UnsetBit(mtb_u64 Bits, mtb_u64 Position) { return Bits & ~(mtb_u64(1) << Position); }
constexpr bool mtb_IsBitSet(mtb_u64 Bits, mtb_u64 Position) { return !!(Bits & (mtb_u64(1) << Position)); }

constexpr bool mtb_IsPowerOfTwo(mtb_u32 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u64 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }

constexpr mtb_u32 mtb_AlignValue_PowerOfTwo(mtb_u32 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }
constexpr mtb_u64 mtb_AlignValue_PowerOfTwo(mtb_u64 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }

constexpr void* mtb_AlignPointer(void* Pointer, int Alignment) { return (void*)mtb_AlignValue_PowerOfTwo((size_t)Pointer, Alignment); }

//
// ============================
//
#define MTB_Pi32     3.14159265359f
#define MTB_HalfPi32 1.57079632679f
#define MTB_InvPi32  0.31830988618f

#define MTB_Pi64     3.14159265359
#define MTB_HalfPi64 1.57079632679
#define MTB_InvPi64  0.31830988618

#define MTB_E32     2.71828182845f
#define MTB_HalfE32 1.35914091423f
#define MTB_InvE32  0.36787944117f

#define MTB_E64     2.71828182845
#define MTB_HalfE64 1.35914091423
#define MTB_InvE64  0.36787944117


//
// ================
//
template<typename T> struct mtb_impl_size_of { enum { SizeInBytes = sizeof(T) }; };
template<>           struct mtb_impl_size_of<void>          : mtb_impl_size_of<mtb_byte>          { };
template<>           struct mtb_impl_size_of<void const>    : mtb_impl_size_of<mtb_byte const>    { };
template<>           struct mtb_impl_size_of<void volatile> : mtb_impl_size_of<mtb_byte volatile> { };

/// Get the size of type T in bytes.
///
/// Same as sizeof(T) except it works also with (cv-qualified) 'void' where a
/// size of 1 byte is assumed.
template<typename T>
constexpr size_t
mtb_SafeSizeOf() { return (size_t)mtb_impl_size_of<T>::SizeInBytes; }

/// Returns the number of elements in this static array.
template<typename T, size_t N>
constexpr size_t
mtb_ArrayLengthOf(T(&)[N]) { return N; }

/// Returns the size in bytes that the given array occupies.
template<typename T, size_t N>
constexpr size_t
mtb_ArrayByteSizeOf(T(&)[N]) { return N * sizeof(T); }

/// Get length of the given \a String, not including the trailing zero.
MTB_INLINE size_t
mtb_StringByteSizeOf(char const* String)
{
  size_t Result = 0;
  while(*String++) { ++Result; }
  return Result;
}

/// Get length of the given \a String, not including the trailing zero.
MTB_INLINE size_t
mtb_StringLengthOf(char const* String)
{
  // TODO: UTF-8
  return mtb_StringByteSizeOf(String);
}

MTB_INLINE int
mtb_StringCompare(size_t Len, char const* PtrA, char const* PtrB)
{
  int Result = 0;

  for(size_t ByteIndex = 0; ByteIndex < Len; ++ByteIndex)
  {
    if(PtrA[ByteIndex] != PtrB[ByteIndex])
    {
      Result = PtrA[ByteIndex] < PtrB[ByteIndex] ? -1 : 1;
      break;
    }
  }

  return Result;
}

MTB_INLINE int
mtb_StringCompare(size_t LenA, char const* PtrA, size_t LenB, char const* PtrB)
{
  int Result = 0;

  if(LenA == LenB) { Result = mtb_StringCompare(LenA, PtrA, PtrB); }
  else             { Result = LenA < LenB ? -1 : 1; }

  return Result;
}

MTB_INLINE bool
mtb_StringsAreEqual(size_t LenA, char const* PtrA, size_t LenB, char const* PtrB)
{
  bool Result = mtb_StringCompare(LenA, PtrA, LenB, PtrB) == 0;
  return Result;
}

MTB_INLINE bool
mtb_StringsAreEqual(size_t Len, char const* PtrA, char const* PtrB)
{
  bool Result = mtb_StringCompare(Len, PtrA, PtrB) == 0;
  return Result;
}

MTB_INLINE bool
mtb_StringsAreEqual(char const* PtrA, char const* PtrB)
{
  size_t LenA = mtb_StringLengthOf(PtrA);
  size_t LenB = mtb_StringLengthOf(PtrB);
  bool Result = mtb_StringCompare(LenA, PtrA, LenB, PtrB) == 0;
  return Result;
}

struct mtb_concat_strings_result
{
  size_t Len;
  char* Ptr;
};
/// Concatenate two strings of the given sizes together.
///
/// \return The result is pointing into the given buffer memory.
mtb_concat_strings_result
mtb_ConcatStrings(size_t HeadSize, char const* HeadPtr, size_t TailSize, char const* TailPtr, size_t BufferSize, char* BufferPtr);

/// Concatenates two zero-terminated strings together using the given buffer.
MTB_INLINE mtb_concat_strings_result
mtb_ConcatStrings(char const* HeadPtr, char const* TailPtr, size_t BufferSize, char* BufferPtr)
{
  size_t HeadSize = mtb_StringLengthOf(HeadPtr);
  size_t TailSize = mtb_StringLengthOf(TailPtr);
  mtb_concat_strings_result Result = mtb_ConcatStrings(HeadSize, HeadPtr, TailSize, TailPtr, BufferSize, BufferPtr);
  return Result;
}

//
// MTB_IsStrictPod
#if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
  #define MTB_IsStrictPod(TYPE) __is_pod(TYPE)
#else
  #error "MTB_IsStrictPod() not implemented for this compiler."
#endif


//
// MTB_IsPod
template<typename T>
struct mtb_impl_is_pod { static constexpr bool Value = MTB_IsStrictPod(T); };
template<> struct mtb_impl_is_pod<void>          { static constexpr bool Value = true; };
template<> struct mtb_impl_is_pod<void const>    { static constexpr bool Value = true; };
template<> struct mtb_impl_is_pod<void volatile> { static constexpr bool Value = true; };

#define MTB_IsPod(TYPE) mtb_impl_is_pod<TYPE>::Value

//
// MTB_(Min|Max)Value_*
#define MTB_MinValue_s08 ((mtb_s08)0x7F)
#define MTB_MaxValue_s08 ((mtb_s08)0x80)
#define MTB_MinValue_s16 ((mtb_s16)0x7FFF)
#define MTB_MaxValue_s16 ((mtb_s16)0x8000)
#define MTB_MinValue_s32 ((mtb_s32)0x7FFFFFFF)
#define MTB_MaxValue_s32 ((mtb_s32)0x80000000)
#define MTB_MinValue_s64 ((mtb_s64)0x7FFFFFFFFFFFFFFF)
#define MTB_MaxValue_s64 ((mtb_s64)0x8000000000000000)

#define MTB_MinValue_u08 ((mtb_u08)0)
#define MTB_MaxValue_u08 ((mtb_u08)0xFF)
#define MTB_MinValue_u16 ((mtb_u16)0)
#define MTB_MaxValue_u16 ((mtb_u16)0xFFFF)
#define MTB_MinValue_u32 ((mtb_u32)0)
#define MTB_MaxValue_u32 ((mtb_u32)0xFFFFFFFF)
#define MTB_MinValue_u64 ((mtb_u64)0)
#define MTB_MaxValue_u64 ((mtb_u64)0xFFFFFFFFFFFFFFFF)


#define MTB_GetNumBits(TYPE) (sizeof(TYPE)*8)


constexpr bool mtb_IsDigit(char Value) { return Value >= '0' && Value <= '9'; }
constexpr bool mtb_IsWhitespace(char Value) { return Value == ' '  || Value == '\n' || Value == '\r' || Value == '\t' || Value == '\b'; }

//
// NaN
#if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
  constexpr mtb_f32 mtb_Impl_QNaN_f32() { return __builtin_nanf("0"); }
  constexpr mtb_f64 mtb_Impl_QNaN_f64() { return __builtin_nanf("0"); }

  #define MTB_QuietNaN_f32 mtb_Impl_QNaN_f32()
  #define MTB_QuietNaN_f64 mtb_Impl_QNaN_f64()

  constexpr mtb_f32 mtb_Impl_SNaN_f32() { return __builtin_nanf("0"); }
  constexpr mtb_f64 mtb_Impl_SNaN_f64() { return __builtin_nanf("0"); }

  #define MTB_SignallingNaN_f32 mtb_Impl_SNaN_f32()
  #define MTB_SignallingNaN_f64 mtb_Impl_SNaN_f64()

  #define MTB_NaN_f32 MTB_QuietNaN_f32
  #define MTB_NaN_f64 MTB_QuietNaN_f64
#else
  #error "MTB_*NaN* not implemented on this platform."
#endif

constexpr bool mtb_IsNaN(mtb_f32 Value) { return Value != Value; }
constexpr bool mtb_IsNaN(mtb_f64 Value) { return Value != Value; }


template<typename T> struct mtb_impl_rm_ref     { using Type = T; };
template<typename T> struct mtb_impl_rm_ref<T&> { using Type = T; };

template<typename T>
using mtb_rm_ref = typename mtb_impl_rm_ref<T>::Type;

template<typename T> struct mtb_impl_rm_const          { using Type = T; };
template<typename T> struct mtb_impl_rm_const<T const> { using Type = T; };

template<typename T>
using mtb_rm_const = typename mtb_impl_rm_const<T>::Type;

template<typename T>
using mtb_rm_ref_const = mtb_rm_const<mtb_rm_ref<T>>;

template<class T>
constexpr typename mtb_rm_ref<T>&&
mtb_Move(T&& Argument)
{
  // forward Argument as movable
  return static_cast<typename mtb_rm_ref<T>&&>(Argument);
}

template<typename T>
constexpr T&&
mtb_Forward(typename mtb_rm_ref<T>& Argument)
{
  return static_cast<T&&>(Argument);
}

template<typename T>
constexpr T&&
mtb_Forward(mtb_rm_ref<T>&& Argument)
{
  return static_cast<T&&>(Argument);
}

template<typename DestT, typename SourceT>
struct mtb_impl_convert
{
  static constexpr DestT
  Do(SourceT const& Value)
  {
    return Cast<DestT>(Value);
  }
};

template<typename DestT, typename SourceT, typename... ExtraTypes>
decltype(auto)
mtb_Convert(SourceT const& ToConvert, ExtraTypes&&... Extra)
{
  using UnqualifiedDestT   = mtb_rm_ref_const<DestT>;
  using UnqualifiedSourceT = mtb_rm_ref_const<SourceT>;
  using Impl = mtb_impl_convert<UnqualifiedDestT, UnqualifiedSourceT>;
  return Impl::Do(ToConvert, mtb_Forward<ExtraTypes>(Extra)...);
}

#define MTB__SAFE_CONVERT_BODY(TYPE) if(Value >= MTB_MinValue_##TYPE && Value <= MTB_MaxValue_##TYPE) \
  MTB_Require(Value >= MTB_MinValue_##TYPE && Value <= MTB_MaxValue_##TYPE, "Value out of range."); \
  mtb_##TYPE Result = (mtb_##TYPE)Value; \
  return Result;

MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s32); }

MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u16 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u32); }


constexpr mtb_s08 mtb_Sign(mtb_s08 Value) { return (mtb_s08)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_s16 mtb_Sign(mtb_s16 Value) { return (mtb_s16)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_s32 mtb_Sign(mtb_s32 Value) { return (mtb_s32)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_s64 mtb_Sign(mtb_s64 Value) { return (mtb_s64)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_f32 mtb_Sign(mtb_f32 Value) { return (mtb_f32)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_f64 mtb_Sign(mtb_f64 Value) { return (mtb_f64)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }


constexpr mtb_s08 mtb_Abs(mtb_s08 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_s16 mtb_Abs(mtb_s16 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_s32 mtb_Abs(mtb_s32 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_s64 mtb_Abs(mtb_s64 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_f32 mtb_Abs(mtb_f32 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_f64 mtb_Abs(mtb_f64 Value) { return mtb_Sign(Value)*Value; }


constexpr mtb_s08 mtb_Min(mtb_s08 A, mtb_s08 B) { return A < B ? A : B; }
constexpr mtb_s16 mtb_Min(mtb_s16 A, mtb_s16 B) { return A < B ? A : B; }
constexpr mtb_s32 mtb_Min(mtb_s32 A, mtb_s32 B) { return A < B ? A : B; }
constexpr mtb_s64 mtb_Min(mtb_s64 A, mtb_s64 B) { return A < B ? A : B; }
constexpr mtb_u08 mtb_Min(mtb_u08 A, mtb_u08 B) { return A < B ? A : B; }
constexpr mtb_u16 mtb_Min(mtb_u16 A, mtb_u16 B) { return A < B ? A : B; }
constexpr mtb_u32 mtb_Min(mtb_u32 A, mtb_u32 B) { return A < B ? A : B; }
constexpr mtb_u64 mtb_Min(mtb_u64 A, mtb_u64 B) { return A < B ? A : B; }
constexpr mtb_f32 mtb_Min(mtb_f32 A, mtb_f32 B) { return A < B ? A : B; }
constexpr mtb_f64 mtb_Min(mtb_f64 A, mtb_f64 B) { return A < B ? A : B; }

constexpr mtb_s08 mtb_Max(mtb_s08 A, mtb_s08 B) { return A > B ? A : B; }
constexpr mtb_s16 mtb_Max(mtb_s16 A, mtb_s16 B) { return A > B ? A : B; }
constexpr mtb_s32 mtb_Max(mtb_s32 A, mtb_s32 B) { return A > B ? A : B; }
constexpr mtb_s64 mtb_Max(mtb_s64 A, mtb_s64 B) { return A > B ? A : B; }
constexpr mtb_u08 mtb_Max(mtb_u08 A, mtb_u08 B) { return A > B ? A : B; }
constexpr mtb_u16 mtb_Max(mtb_u16 A, mtb_u16 B) { return A > B ? A : B; }
constexpr mtb_u32 mtb_Max(mtb_u32 A, mtb_u32 B) { return A > B ? A : B; }
constexpr mtb_u64 mtb_Max(mtb_u64 A, mtb_u64 B) { return A > B ? A : B; }
constexpr mtb_f32 mtb_Max(mtb_f32 A, mtb_f32 B) { return A > B ? A : B; }
constexpr mtb_f64 mtb_Max(mtb_f64 A, mtb_f64 B) { return A > B ? A : B; }

constexpr mtb_s08 mtb_Clamp(mtb_s08 Value, mtb_s08 Lower, mtb_s08 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_s16 mtb_Clamp(mtb_s16 Value, mtb_s16 Lower, mtb_s16 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_s32 mtb_Clamp(mtb_s32 Value, mtb_s32 Lower, mtb_s32 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_s64 mtb_Clamp(mtb_s64 Value, mtb_s64 Lower, mtb_s64 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u08 mtb_Clamp(mtb_u08 Value, mtb_u08 Lower, mtb_u08 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u16 mtb_Clamp(mtb_u16 Value, mtb_u16 Lower, mtb_u16 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u32 mtb_Clamp(mtb_u32 Value, mtb_u32 Lower, mtb_u32 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u64 mtb_Clamp(mtb_u64 Value, mtb_u64 Lower, mtb_u64 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_f32 mtb_Clamp(mtb_f32 Value, mtb_f32 Lower, mtb_f32 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_f64 mtb_Clamp(mtb_f64 Value, mtb_f64 Lower, mtb_f64 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }


mtb_f64 mtb_Pow(mtb_f64 Base, mtb_f64 Exponent);
mtb_f32 mtb_Pow(mtb_f32 Base, mtb_f32 Exponent);

mtb_f64 mtb_Mod(mtb_f64 Value, mtb_f64 Divisor);
mtb_f32 mtb_Mod(mtb_f32 Value, mtb_f32 Divisor);

mtb_f64 mtb_Sqrt(mtb_f64 Value);
mtb_f32 mtb_Sqrt(mtb_f32 Value);

mtb_f32 mtb_InvSqrt(mtb_f32 Value);

//
// Rounding
//
mtb_f32 mtb_RoundDown(mtb_f32 Value);
mtb_f64 mtb_RoundDown(mtb_f64 Value);

mtb_f32 mtb_RoundUp(mtb_f32 Value);
mtb_f64 mtb_RoundUp(mtb_f64 Value);

MTB_INLINE mtb_f32 mtb_Round(mtb_f32 Value) { return mtb_RoundDown(Value + 0.5f); }
MTB_INLINE mtb_f64 mtb_Round(mtb_f64 Value) { return mtb_RoundDown(Value + 0.5); }

MTB_INLINE mtb_f32 mtb_RoundTowardsZero(mtb_f32 Value) { return Value < 0 ? mtb_RoundUp(Value) : mtb_RoundDown(Value); }
MTB_INLINE mtb_f64 mtb_RoundTowardsZero(mtb_f64 Value) { return Value < 0 ? mtb_RoundUp(Value) : mtb_RoundDown(Value); }

MTB_INLINE mtb_f32 mtb_RoundAwayFromZero(mtb_f32 Value) { return Value < 0 ? mtb_RoundDown(Value) : mtb_RoundUp(Value); }
MTB_INLINE mtb_f64 mtb_RoundAwayFromZero(mtb_f64 Value) { return Value < 0 ? mtb_RoundDown(Value) : mtb_RoundUp(Value); }


// Project a value from [LowerBound, UpperBound] to [0, 1]
// Example:
//   auto Result = mtb_NormalizeValue<mtb_f32>(15, 10, 30); // == 0.25f
// TODO: Un-templatize
template<typename ResultT, typename ValueT, typename LowerBoundT, typename UpperBoundT>
constexpr ResultT
mtb_NormalizeValue(ValueT Value, LowerBoundT LowerBound, UpperBoundT UpperBound)
{
  return UpperBound <= LowerBound ?
         ResultT(0) : // Bogus bounds.
         (ResultT)(Value - LowerBound) / (ResultT)(UpperBound - LowerBound);
}

bool
mtb_AreNearlyEqual(mtb_f32 A, mtb_f32 B, mtb_f32 Epsilon = 1e-4f);

bool
mtb_AreNearlyEqual(mtb_f64 A, mtb_f64 B, mtb_f64 Epsilon = 1e-4);

MTB_INLINE bool
mtb_IsNearlyZero(mtb_f32 A, mtb_f32 Epsilon = 1e-4f) { return mtb_AreNearlyEqual(A, 0, Epsilon); }

MTB_INLINE bool
mtb_IsNearlyZero(mtb_f64 A, mtb_f64 Epsilon = 1e-4) { return mtb_AreNearlyEqual(A, 0, Epsilon); }

template<typename TypeA, typename TypeB>
MTB_INLINE void
mtb_Swap(TypeA& A, TypeB& B)
{
  auto Temp{ mtb_Move(A) };
  A = mtb_Move(B);
  B = mtb_Move(Temp);
}

template<typename T, typename U = T>
MTB_INLINE T
mtb_Exchange(T& Value, U&& NewValue)
{
  auto OldValue{ mtb_Move(Value) };
  Value = mtb_Forward<T>(NewValue);
  return OldValue;
}

/// Maps the given mtb_f32 Value from [0, 1] to [0, MTB_MaxValue_u08]
mtb_u08 constexpr mtb_Normalized_f32_To_u08(mtb_f32 Value) { return (mtb_u08)mtb_Clamp((Value * MTB_MaxValue_u08) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u08); }
mtb_u16 constexpr mtb_Normalized_f32_To_u16(mtb_f32 Value) { return (mtb_u16)mtb_Clamp((Value * MTB_MaxValue_u16) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u16); }
mtb_u32 constexpr mtb_Normalized_f32_To_u32(mtb_f32 Value) { return (mtb_u32)mtb_Clamp((Value * MTB_MaxValue_u32) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u32); }
mtb_u64 constexpr mtb_Normalized_f32_To_u64(mtb_f32 Value) { return (mtb_u64)mtb_Clamp((Value * MTB_MaxValue_u64) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u64); }

mtb_f32 constexpr mtb_Normalized_u08_To_f32(mtb_u08 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u08, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u16_To_f32(mtb_u16 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u16, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u32_To_f32(mtb_u32 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u32, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u64_To_f32(mtb_u64 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u64, 0.0f, 1.0f); }


struct mtb_impl_defer
{
  template<typename LambdaType>
  struct defer
  {
    LambdaType Lambda;
    defer(LambdaType InLambda) : Lambda{ mtb_Move(InLambda) } {}
    ~defer() { Lambda(); }
  };

  template<typename FuncT>
  defer<FuncT> operator =(FuncT InLambda) { return { mtb_Move(InLambda) }; }
};

/// Defers execution of code until the end of the current scope.
///
/// Usage:
///   int i = 0;
///   MTB_DEFER [&](){ i++; printf("Foo %d\n", i); };
///   MTB_DEFER [&](){ i++; printf("Bar %d\n", i); };
///   MTB_DEFER [=](){      printf("Baz %d\n", i); };
///
/// Output:
///   Baz 0
///   Bar 1
///   Foo 2
///
/// \param CaptureSpec The lambda capture specification.
#define MTB_DEFER auto MTB_CONCAT(_Defer, MTB_CURRENT_LINE) = mtb_impl_defer() =

#endif // !defined(MTB_HEADER_mtb_common)


//
// Implementation
//

#if defined(MTB_COMMON_IMPLEMENTATION)

// Impl guard.
#if !defined(MTB_IMPL_mtb_common)
#define MTB_IMPL_mtb_common

#include <math.h>


mtb_concat_strings_result
mtb_ConcatStrings(size_t HeadSize, char const* HeadPtr, size_t TailSize, char const* TailPtr, size_t BufferSize, char* BufferPtr)
{
  size_t BufferIndex{};
  size_t HeadIndex{};
  size_t TailIndex{};

  while(BufferIndex < BufferSize && HeadIndex < HeadSize)
  {
    BufferPtr[BufferIndex++] = HeadPtr[HeadIndex++];
  }

  while(BufferIndex < BufferSize && TailIndex < TailSize)
  {
    BufferPtr[BufferIndex++] = TailPtr[TailIndex++];
  }

  if(BufferIndex < BufferSize)
  {
    // Append the trailing zero-terminator without increasing the BufferIndex.
    BufferPtr[BufferIndex] = '\0';
  }

  mtb_concat_strings_result Result{ BufferIndex, BufferPtr };
  return Result;
}

mtb_f32
mtb_Pow(mtb_f32 Base, mtb_f32 Exponent)
{
  return powf(Base, Exponent);
}

mtb_f64
mtb_Pow(mtb_f64 Base, mtb_f64 Exponent)
{
  return pow(Base, Exponent);
}

mtb_f32
mtb_Mod(mtb_f32 Value, mtb_f32 Divisor)
{
  return fmodf(Value, Divisor);
}

mtb_f64
mtb_Mod(mtb_f64 Value, mtb_f64 Divisor)
{
  return fmod(Value, Divisor);
}

mtb_f32
mtb_Sqrt(mtb_f32 Value)
{
  return sqrtf(Value);
}

mtb_f64
mtb_Sqrt(mtb_f64 Value)
{
  return sqrt(Value);
}

mtb_f32
mtb_InvSqrt(mtb_f32 Value)
{
  union FloatInt
  {
    mtb_f32 Float;
    int Int;
  };
  FloatInt MagicNumber;
  mtb_f32 HalfValue;
  mtb_f32 Result;
  const mtb_f32 ThreeHalfs = 1.5f;

  HalfValue = Value * 0.5f;
  Result = Value;
  MagicNumber.Float = Result;                               // evil floating point bit level hacking
  MagicNumber.Int  = 0x5f3759df - ( MagicNumber.Int >> 1 ); // what the fuck?
  Result = MagicNumber.Float;
  Result = Result * ( ThreeHalfs - ( HalfValue * Result * Result ) ); // 1st iteration

  return Result;
}

mtb_f32
mtb_RoundDown(mtb_f32 Value)
{
  return floorf(Value);
}

mtb_f64
mtb_RoundDown(mtb_f64 Value)
{
  return floor(Value);
}

mtb_f32
mtb_RoundUp(mtb_f32 Value)
{
  return ceilf(Value);
}

mtb_f64
mtb_RoundUp(mtb_f64 Value)
{
  return ceil(Value);
}


bool
mtb_AreNearlyEqual(mtb_f64 A, mtb_f64 B, mtb_f64 Epsilon)
{
  return mtb_Abs(A - B) <= Epsilon;
}

bool
mtb_AreNearlyEqual(mtb_f32 A, mtb_f32 B, mtb_f32 Epsilon)
{
  return mtb_Abs(A - B) <= Epsilon;
}

#endif // !defined(MTB_IMPL_mtb_common)
#endif // defined(MTB_COMMON_IMPLEMENTATION)
#if !defined(MTB_HEADER_mtb_memory)
#define MTB_HEADER_mtb_memory

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_MEMORY_IMPLEMENTATION)
  #define MTB_MEMORY_IMPLEMENTATION
#endif

#if defined(MTB_MEMORY_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


// ==========================================
// mtb_common.h
// ==========================================

#if !defined(MTB_HEADER_mtb_common)
#define MTB_HEADER_mtb_common

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_COMMON_IMPLEMENTATION)
  #define MTB_COMMON_IMPLEMENTATION
#endif

#if defined(MTB_COMMON_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


//
// Primitive array types.
//

/// Defines some array variants of types for better readability when used as
/// function parameters.
///
/// For example, a function `Foo` that accepts and array of 4 floats by
/// reference-to-const looks like this:
/// \code
/// void Foo(mtb_f32 const (&ParamName)[4]);
/// \endcode
///
/// Using these typedefs, this can be transformed into:
/// \code
/// void Foo(mtb_f32_4 const& ParamName);
/// \endcode
#define MTB_DEFINE_ARRAY_TYPES(TheType)\
  using TheType##_1   = TheType[1];\
  using TheType##_2   = TheType[2];\
  using TheType##_3   = TheType[3];\
  using TheType##_4   = TheType[4];\
  using TheType##_1x1 = TheType##_1[1];\
  using TheType##_1x2 = TheType##_1[2];\
  using TheType##_1x3 = TheType##_1[3];\
  using TheType##_1x4 = TheType##_1[4];\
  using TheType##_2x1 = TheType##_2[1];\
  using TheType##_2x2 = TheType##_2[2];\
  using TheType##_2x3 = TheType##_2[3];\
  using TheType##_2x4 = TheType##_2[4];\
  using TheType##_3x1 = TheType##_3[1];\
  using TheType##_3x2 = TheType##_3[2];\
  using TheType##_3x3 = TheType##_3[3];\
  using TheType##_3x4 = TheType##_3[4];\
  using TheType##_4x1 = TheType##_4[1];\
  using TheType##_4x2 = TheType##_4[2];\
  using TheType##_4x3 = TheType##_4[3];\
  using TheType##_4x4 = TheType##_4[4]

MTB_DEFINE_ARRAY_TYPES(mtb_s08);   // mtb_s08   => mtb_s08_1,   mtb_s08_2,   ..., mtb_s08_4x3,   mtb_s08_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_s16);   // mtb_s16   => mtb_s16_1,   mtb_s16_2,   ..., mtb_s16_4x3,   mtb_s16_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_s32);   // mtb_s32   => mtb_s32_1,   mtb_s32_2,   ..., mtb_s32_4x3,   mtb_s32_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_s64);   // mtb_s64   => mtb_s64_1,   mtb_s64_2,   ..., mtb_s64_4x3,   mtb_s64_4x4

MTB_DEFINE_ARRAY_TYPES(mtb_u08);  // mtb_u08  => mtb_u08_1,  mtb_u08_2,  ..., mtb_u08_4x3,  mtb_u08_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_u16);  // mtb_u16  => mtb_u16_1,  mtb_u16_2,  ..., mtb_u16_4x3,  mtb_u16_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_u32);  // mtb_u32  => mtb_u32_1,  mtb_u32_2,  ..., mtb_u32_4x3,  mtb_u32_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_u64);  // mtb_u64  => mtb_u64_1,  mtb_u64_2,  ..., mtb_u64_4x3,  mtb_u64_4x4

MTB_DEFINE_ARRAY_TYPES(mtb_f32); // mtb_f32 => mtb_f32_1, mtb_f32_2, ..., mtb_f32_4x3, mtb_f32_4x4
MTB_DEFINE_ARRAY_TYPES(mtb_f64); // mtb_f64 => mtb_f64_1, mtb_f64_2, ..., mtb_f64_4x3, mtb_f64_4x4


//
// ============================
//

constexpr mtb_u64 mtb_KiB(mtb_u64 Amount) { return { Amount * 1024 }; }
constexpr mtb_u64 mtb_MiB(mtb_u64 Amount) { return { Amount * 1024 * 1024 }; }
constexpr mtb_u64 mtb_GiB(mtb_u64 Amount) { return { Amount * 1024 * 1024 * 1024 }; }
constexpr mtb_u64 mtb_TiB(mtb_u64 Amount) { return { Amount * 1024 * 1024 * 1024 * 1024 }; }

constexpr mtb_u64 mtb_KB(mtb_u64 Amount) { return { Amount * 1000 }; }
constexpr mtb_u64 mtb_MB(mtb_u64 Amount) { return { Amount * 1000 * 1000 }; }
constexpr mtb_u64 mtb_GB(mtb_u64 Amount) { return { Amount * 1000 * 1000 * 1000 }; }
constexpr mtb_u64 mtb_TB(mtb_u64 Amount) { return { Amount * 1000 * 1000 * 1000 * 1000 }; }

constexpr mtb_f64 mtb_ToKiB(mtb_u64 Size) { return (mtb_f64)Size / 1024; }
constexpr mtb_f64 mtb_ToMiB(mtb_u64 Size) { return (mtb_f64)Size / 1024 / 1024; }
constexpr mtb_f64 mtb_ToGiB(mtb_u64 Size) { return (mtb_f64)Size / 1024 / 1024 / 1024; }
constexpr mtb_f64 mtb_ToTiB(mtb_u64 Size) { return (mtb_f64)Size / 1024 / 1024 / 1024 / 1024; }

constexpr mtb_f64 mtb_ToKB(mtb_u64 Size) { return (mtb_f64)Size / 1000; }
constexpr mtb_f64 mtb_ToMB(mtb_u64 Size) { return (mtb_f64)Size / 1000 / 1000; }
constexpr mtb_f64 mtb_ToGB(mtb_u64 Size) { return (mtb_f64)Size / 1000 / 1000 / 1000; }
constexpr mtb_f64 mtb_ToTB(mtb_u64 Size) { return (mtb_f64)Size / 1000 / 1000 / 1000 / 1000; }



//
// ============================
//
constexpr mtb_u32 mtb_SetBit(mtb_u32 Bits, mtb_u32 Position)   { return Bits | (mtb_u32(1) << Position); }
constexpr mtb_u32 mtb_UnsetBit(mtb_u32 Bits, mtb_u32 Position) { return Bits & ~(mtb_u32(1) << Position); }
constexpr bool mtb_IsBitSet(mtb_u32 Bits, mtb_u32 Position) { return !!(Bits & (mtb_u32(1) << Position)); }

constexpr mtb_u64 mtb_SetBit(mtb_u64 Bits, mtb_u64 Position)   { return Bits | (mtb_u64(1) << Position); }
constexpr mtb_u64 mtb_UnsetBit(mtb_u64 Bits, mtb_u64 Position) { return Bits & ~(mtb_u64(1) << Position); }
constexpr bool mtb_IsBitSet(mtb_u64 Bits, mtb_u64 Position) { return !!(Bits & (mtb_u64(1) << Position)); }

constexpr bool mtb_IsPowerOfTwo(mtb_u32 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u64 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }

constexpr mtb_u32 mtb_AlignValue_PowerOfTwo(mtb_u32 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }
constexpr mtb_u64 mtb_AlignValue_PowerOfTwo(mtb_u64 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }

constexpr void* mtb_AlignPointer(void* Pointer, int Alignment) { return (void*)mtb_AlignValue_PowerOfTwo((size_t)Pointer, Alignment); }

//
// ============================
//
#define MTB_Pi32     3.14159265359f
#define MTB_HalfPi32 1.57079632679f
#define MTB_InvPi32  0.31830988618f

#define MTB_Pi64     3.14159265359
#define MTB_HalfPi64 1.57079632679
#define MTB_InvPi64  0.31830988618

#define MTB_E32     2.71828182845f
#define MTB_HalfE32 1.35914091423f
#define MTB_InvE32  0.36787944117f

#define MTB_E64     2.71828182845
#define MTB_HalfE64 1.35914091423
#define MTB_InvE64  0.36787944117


//
// ================
//
template<typename T> struct mtb_impl_size_of { enum { SizeInBytes = sizeof(T) }; };
template<>           struct mtb_impl_size_of<void>          : mtb_impl_size_of<mtb_byte>          { };
template<>           struct mtb_impl_size_of<void const>    : mtb_impl_size_of<mtb_byte const>    { };
template<>           struct mtb_impl_size_of<void volatile> : mtb_impl_size_of<mtb_byte volatile> { };

/// Get the size of type T in bytes.
///
/// Same as sizeof(T) except it works also with (cv-qualified) 'void' where a
/// size of 1 byte is assumed.
template<typename T>
constexpr size_t
mtb_SafeSizeOf() { return (size_t)mtb_impl_size_of<T>::SizeInBytes; }

/// Returns the number of elements in this static array.
template<typename T, size_t N>
constexpr size_t
mtb_ArrayLengthOf(T(&)[N]) { return N; }

/// Returns the size in bytes that the given array occupies.
template<typename T, size_t N>
constexpr size_t
mtb_ArrayByteSizeOf(T(&)[N]) { return N * sizeof(T); }

/// Get length of the given \a String, not including the trailing zero.
MTB_INLINE size_t
mtb_StringByteSizeOf(char const* String)
{
  size_t Result = 0;
  while(*String++) { ++Result; }
  return Result;
}

/// Get length of the given \a String, not including the trailing zero.
MTB_INLINE size_t
mtb_StringLengthOf(char const* String)
{
  // TODO: UTF-8
  return mtb_StringByteSizeOf(String);
}

MTB_INLINE int
mtb_StringCompare(size_t Len, char const* PtrA, char const* PtrB)
{
  int Result = 0;

  for(size_t ByteIndex = 0; ByteIndex < Len; ++ByteIndex)
  {
    if(PtrA[ByteIndex] != PtrB[ByteIndex])
    {
      Result = PtrA[ByteIndex] < PtrB[ByteIndex] ? -1 : 1;
      break;
    }
  }

  return Result;
}

MTB_INLINE int
mtb_StringCompare(size_t LenA, char const* PtrA, size_t LenB, char const* PtrB)
{
  int Result = 0;

  if(LenA == LenB) { Result = mtb_StringCompare(LenA, PtrA, PtrB); }
  else             { Result = LenA < LenB ? -1 : 1; }

  return Result;
}

MTB_INLINE bool
mtb_StringsAreEqual(size_t LenA, char const* PtrA, size_t LenB, char const* PtrB)
{
  bool Result = mtb_StringCompare(LenA, PtrA, LenB, PtrB) == 0;
  return Result;
}

MTB_INLINE bool
mtb_StringsAreEqual(size_t Len, char const* PtrA, char const* PtrB)
{
  bool Result = mtb_StringCompare(Len, PtrA, PtrB) == 0;
  return Result;
}

MTB_INLINE bool
mtb_StringsAreEqual(char const* PtrA, char const* PtrB)
{
  size_t LenA = mtb_StringLengthOf(PtrA);
  size_t LenB = mtb_StringLengthOf(PtrB);
  bool Result = mtb_StringCompare(LenA, PtrA, LenB, PtrB) == 0;
  return Result;
}

struct mtb_concat_strings_result
{
  size_t Len;
  char* Ptr;
};
/// Concatenate two strings of the given sizes together.
///
/// \return The result is pointing into the given buffer memory.
mtb_concat_strings_result
mtb_ConcatStrings(size_t HeadSize, char const* HeadPtr, size_t TailSize, char const* TailPtr, size_t BufferSize, char* BufferPtr);

/// Concatenates two zero-terminated strings together using the given buffer.
MTB_INLINE mtb_concat_strings_result
mtb_ConcatStrings(char const* HeadPtr, char const* TailPtr, size_t BufferSize, char* BufferPtr)
{
  size_t HeadSize = mtb_StringLengthOf(HeadPtr);
  size_t TailSize = mtb_StringLengthOf(TailPtr);
  mtb_concat_strings_result Result = mtb_ConcatStrings(HeadSize, HeadPtr, TailSize, TailPtr, BufferSize, BufferPtr);
  return Result;
}

//
// MTB_IsStrictPod
#if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
  #define MTB_IsStrictPod(TYPE) __is_pod(TYPE)
#else
  #error "MTB_IsStrictPod() not implemented for this compiler."
#endif


//
// MTB_IsPod
template<typename T>
struct mtb_impl_is_pod { static constexpr bool Value = MTB_IsStrictPod(T); };
template<> struct mtb_impl_is_pod<void>          { static constexpr bool Value = true; };
template<> struct mtb_impl_is_pod<void const>    { static constexpr bool Value = true; };
template<> struct mtb_impl_is_pod<void volatile> { static constexpr bool Value = true; };

#define MTB_IsPod(TYPE) mtb_impl_is_pod<TYPE>::Value

//
// MTB_(Min|Max)Value_*
#define MTB_MinValue_s08 ((mtb_s08)0x7F)
#define MTB_MaxValue_s08 ((mtb_s08)0x80)
#define MTB_MinValue_s16 ((mtb_s16)0x7FFF)
#define MTB_MaxValue_s16 ((mtb_s16)0x8000)
#define MTB_MinValue_s32 ((mtb_s32)0x7FFFFFFF)
#define MTB_MaxValue_s32 ((mtb_s32)0x80000000)
#define MTB_MinValue_s64 ((mtb_s64)0x7FFFFFFFFFFFFFFF)
#define MTB_MaxValue_s64 ((mtb_s64)0x8000000000000000)

#define MTB_MinValue_u08 ((mtb_u08)0)
#define MTB_MaxValue_u08 ((mtb_u08)0xFF)
#define MTB_MinValue_u16 ((mtb_u16)0)
#define MTB_MaxValue_u16 ((mtb_u16)0xFFFF)
#define MTB_MinValue_u32 ((mtb_u32)0)
#define MTB_MaxValue_u32 ((mtb_u32)0xFFFFFFFF)
#define MTB_MinValue_u64 ((mtb_u64)0)
#define MTB_MaxValue_u64 ((mtb_u64)0xFFFFFFFFFFFFFFFF)


#define MTB_GetNumBits(TYPE) (sizeof(TYPE)*8)


constexpr bool mtb_IsDigit(char Value) { return Value >= '0' && Value <= '9'; }
constexpr bool mtb_IsWhitespace(char Value) { return Value == ' '  || Value == '\n' || Value == '\r' || Value == '\t' || Value == '\b'; }

//
// NaN
#if MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC
  constexpr mtb_f32 mtb_Impl_QNaN_f32() { return __builtin_nanf("0"); }
  constexpr mtb_f64 mtb_Impl_QNaN_f64() { return __builtin_nanf("0"); }

  #define MTB_QuietNaN_f32 mtb_Impl_QNaN_f32()
  #define MTB_QuietNaN_f64 mtb_Impl_QNaN_f64()

  constexpr mtb_f32 mtb_Impl_SNaN_f32() { return __builtin_nanf("0"); }
  constexpr mtb_f64 mtb_Impl_SNaN_f64() { return __builtin_nanf("0"); }

  #define MTB_SignallingNaN_f32 mtb_Impl_SNaN_f32()
  #define MTB_SignallingNaN_f64 mtb_Impl_SNaN_f64()

  #define MTB_NaN_f32 MTB_QuietNaN_f32
  #define MTB_NaN_f64 MTB_QuietNaN_f64
#else
  #error "MTB_*NaN* not implemented on this platform."
#endif

constexpr bool mtb_IsNaN(mtb_f32 Value) { return Value != Value; }
constexpr bool mtb_IsNaN(mtb_f64 Value) { return Value != Value; }


template<typename T> struct mtb_impl_rm_ref     { using Type = T; };
template<typename T> struct mtb_impl_rm_ref<T&> { using Type = T; };

template<typename T>
using mtb_rm_ref = typename mtb_impl_rm_ref<T>::Type;

template<typename T> struct mtb_impl_rm_const          { using Type = T; };
template<typename T> struct mtb_impl_rm_const<T const> { using Type = T; };

template<typename T>
using mtb_rm_const = typename mtb_impl_rm_const<T>::Type;

template<typename T>
using mtb_rm_ref_const = mtb_rm_const<mtb_rm_ref<T>>;

template<class T>
constexpr typename mtb_rm_ref<T>&&
mtb_Move(T&& Argument)
{
  // forward Argument as movable
  return static_cast<typename mtb_rm_ref<T>&&>(Argument);
}

template<typename T>
constexpr T&&
mtb_Forward(typename mtb_rm_ref<T>& Argument)
{
  return static_cast<T&&>(Argument);
}

template<typename T>
constexpr T&&
mtb_Forward(mtb_rm_ref<T>&& Argument)
{
  return static_cast<T&&>(Argument);
}

template<typename DestT, typename SourceT>
struct mtb_impl_convert
{
  static constexpr DestT
  Do(SourceT const& Value)
  {
    return Cast<DestT>(Value);
  }
};

template<typename DestT, typename SourceT, typename... ExtraTypes>
decltype(auto)
mtb_Convert(SourceT const& ToConvert, ExtraTypes&&... Extra)
{
  using UnqualifiedDestT   = mtb_rm_ref_const<DestT>;
  using UnqualifiedSourceT = mtb_rm_ref_const<SourceT>;
  using Impl = mtb_impl_convert<UnqualifiedDestT, UnqualifiedSourceT>;
  return Impl::Do(ToConvert, mtb_Forward<ExtraTypes>(Extra)...);
}

#define MTB__SAFE_CONVERT_BODY(TYPE) if(Value >= MTB_MinValue_##TYPE && Value <= MTB_MaxValue_##TYPE) \
  MTB_Require(Value >= MTB_MinValue_##TYPE && Value <= MTB_MaxValue_##TYPE, "Value out of range."); \
  mtb_##TYPE Result = (mtb_##TYPE)Value; \
  return Result;

MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s32); }

MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u16 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u32); }


constexpr mtb_s08 mtb_Sign(mtb_s08 Value) { return (mtb_s08)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_s16 mtb_Sign(mtb_s16 Value) { return (mtb_s16)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_s32 mtb_Sign(mtb_s32 Value) { return (mtb_s32)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_s64 mtb_Sign(mtb_s64 Value) { return (mtb_s64)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_f32 mtb_Sign(mtb_f32 Value) { return (mtb_f32)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }
constexpr mtb_f64 mtb_Sign(mtb_f64 Value) { return (mtb_f64)(Value > 0 ? 1 : Value < 0 ? -1 : 0); }


constexpr mtb_s08 mtb_Abs(mtb_s08 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_s16 mtb_Abs(mtb_s16 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_s32 mtb_Abs(mtb_s32 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_s64 mtb_Abs(mtb_s64 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_f32 mtb_Abs(mtb_f32 Value) { return mtb_Sign(Value)*Value; }
constexpr mtb_f64 mtb_Abs(mtb_f64 Value) { return mtb_Sign(Value)*Value; }


constexpr mtb_s08 mtb_Min(mtb_s08 A, mtb_s08 B) { return A < B ? A : B; }
constexpr mtb_s16 mtb_Min(mtb_s16 A, mtb_s16 B) { return A < B ? A : B; }
constexpr mtb_s32 mtb_Min(mtb_s32 A, mtb_s32 B) { return A < B ? A : B; }
constexpr mtb_s64 mtb_Min(mtb_s64 A, mtb_s64 B) { return A < B ? A : B; }
constexpr mtb_u08 mtb_Min(mtb_u08 A, mtb_u08 B) { return A < B ? A : B; }
constexpr mtb_u16 mtb_Min(mtb_u16 A, mtb_u16 B) { return A < B ? A : B; }
constexpr mtb_u32 mtb_Min(mtb_u32 A, mtb_u32 B) { return A < B ? A : B; }
constexpr mtb_u64 mtb_Min(mtb_u64 A, mtb_u64 B) { return A < B ? A : B; }
constexpr mtb_f32 mtb_Min(mtb_f32 A, mtb_f32 B) { return A < B ? A : B; }
constexpr mtb_f64 mtb_Min(mtb_f64 A, mtb_f64 B) { return A < B ? A : B; }

constexpr mtb_s08 mtb_Max(mtb_s08 A, mtb_s08 B) { return A > B ? A : B; }
constexpr mtb_s16 mtb_Max(mtb_s16 A, mtb_s16 B) { return A > B ? A : B; }
constexpr mtb_s32 mtb_Max(mtb_s32 A, mtb_s32 B) { return A > B ? A : B; }
constexpr mtb_s64 mtb_Max(mtb_s64 A, mtb_s64 B) { return A > B ? A : B; }
constexpr mtb_u08 mtb_Max(mtb_u08 A, mtb_u08 B) { return A > B ? A : B; }
constexpr mtb_u16 mtb_Max(mtb_u16 A, mtb_u16 B) { return A > B ? A : B; }
constexpr mtb_u32 mtb_Max(mtb_u32 A, mtb_u32 B) { return A > B ? A : B; }
constexpr mtb_u64 mtb_Max(mtb_u64 A, mtb_u64 B) { return A > B ? A : B; }
constexpr mtb_f32 mtb_Max(mtb_f32 A, mtb_f32 B) { return A > B ? A : B; }
constexpr mtb_f64 mtb_Max(mtb_f64 A, mtb_f64 B) { return A > B ? A : B; }

constexpr mtb_s08 mtb_Clamp(mtb_s08 Value, mtb_s08 Lower, mtb_s08 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_s16 mtb_Clamp(mtb_s16 Value, mtb_s16 Lower, mtb_s16 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_s32 mtb_Clamp(mtb_s32 Value, mtb_s32 Lower, mtb_s32 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_s64 mtb_Clamp(mtb_s64 Value, mtb_s64 Lower, mtb_s64 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u08 mtb_Clamp(mtb_u08 Value, mtb_u08 Lower, mtb_u08 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u16 mtb_Clamp(mtb_u16 Value, mtb_u16 Lower, mtb_u16 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u32 mtb_Clamp(mtb_u32 Value, mtb_u32 Lower, mtb_u32 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_u64 mtb_Clamp(mtb_u64 Value, mtb_u64 Lower, mtb_u64 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_f32 mtb_Clamp(mtb_f32 Value, mtb_f32 Lower, mtb_f32 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }
constexpr mtb_f64 mtb_Clamp(mtb_f64 Value, mtb_f64 Lower, mtb_f64 Upper) { return Upper < Lower ? Value : mtb_Min(Upper, mtb_Max(Lower, Value)); }


mtb_f64 mtb_Pow(mtb_f64 Base, mtb_f64 Exponent);
mtb_f32 mtb_Pow(mtb_f32 Base, mtb_f32 Exponent);

mtb_f64 mtb_Mod(mtb_f64 Value, mtb_f64 Divisor);
mtb_f32 mtb_Mod(mtb_f32 Value, mtb_f32 Divisor);

mtb_f64 mtb_Sqrt(mtb_f64 Value);
mtb_f32 mtb_Sqrt(mtb_f32 Value);

mtb_f32 mtb_InvSqrt(mtb_f32 Value);

//
// Rounding
//
mtb_f32 mtb_RoundDown(mtb_f32 Value);
mtb_f64 mtb_RoundDown(mtb_f64 Value);

mtb_f32 mtb_RoundUp(mtb_f32 Value);
mtb_f64 mtb_RoundUp(mtb_f64 Value);

MTB_INLINE mtb_f32 mtb_Round(mtb_f32 Value) { return mtb_RoundDown(Value + 0.5f); }
MTB_INLINE mtb_f64 mtb_Round(mtb_f64 Value) { return mtb_RoundDown(Value + 0.5); }

MTB_INLINE mtb_f32 mtb_RoundTowardsZero(mtb_f32 Value) { return Value < 0 ? mtb_RoundUp(Value) : mtb_RoundDown(Value); }
MTB_INLINE mtb_f64 mtb_RoundTowardsZero(mtb_f64 Value) { return Value < 0 ? mtb_RoundUp(Value) : mtb_RoundDown(Value); }

MTB_INLINE mtb_f32 mtb_RoundAwayFromZero(mtb_f32 Value) { return Value < 0 ? mtb_RoundDown(Value) : mtb_RoundUp(Value); }
MTB_INLINE mtb_f64 mtb_RoundAwayFromZero(mtb_f64 Value) { return Value < 0 ? mtb_RoundDown(Value) : mtb_RoundUp(Value); }


// Project a value from [LowerBound, UpperBound] to [0, 1]
// Example:
//   auto Result = mtb_NormalizeValue<mtb_f32>(15, 10, 30); // == 0.25f
// TODO: Un-templatize
template<typename ResultT, typename ValueT, typename LowerBoundT, typename UpperBoundT>
constexpr ResultT
mtb_NormalizeValue(ValueT Value, LowerBoundT LowerBound, UpperBoundT UpperBound)
{
  return UpperBound <= LowerBound ?
         ResultT(0) : // Bogus bounds.
         (ResultT)(Value - LowerBound) / (ResultT)(UpperBound - LowerBound);
}

bool
mtb_AreNearlyEqual(mtb_f32 A, mtb_f32 B, mtb_f32 Epsilon = 1e-4f);

bool
mtb_AreNearlyEqual(mtb_f64 A, mtb_f64 B, mtb_f64 Epsilon = 1e-4);

MTB_INLINE bool
mtb_IsNearlyZero(mtb_f32 A, mtb_f32 Epsilon = 1e-4f) { return mtb_AreNearlyEqual(A, 0, Epsilon); }

MTB_INLINE bool
mtb_IsNearlyZero(mtb_f64 A, mtb_f64 Epsilon = 1e-4) { return mtb_AreNearlyEqual(A, 0, Epsilon); }

template<typename TypeA, typename TypeB>
MTB_INLINE void
mtb_Swap(TypeA& A, TypeB& B)
{
  auto Temp{ mtb_Move(A) };
  A = mtb_Move(B);
  B = mtb_Move(Temp);
}

template<typename T, typename U = T>
MTB_INLINE T
mtb_Exchange(T& Value, U&& NewValue)
{
  auto OldValue{ mtb_Move(Value) };
  Value = mtb_Forward<T>(NewValue);
  return OldValue;
}

/// Maps the given mtb_f32 Value from [0, 1] to [0, MTB_MaxValue_u08]
mtb_u08 constexpr mtb_Normalized_f32_To_u08(mtb_f32 Value) { return (mtb_u08)mtb_Clamp((Value * MTB_MaxValue_u08) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u08); }
mtb_u16 constexpr mtb_Normalized_f32_To_u16(mtb_f32 Value) { return (mtb_u16)mtb_Clamp((Value * MTB_MaxValue_u16) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u16); }
mtb_u32 constexpr mtb_Normalized_f32_To_u32(mtb_f32 Value) { return (mtb_u32)mtb_Clamp((Value * MTB_MaxValue_u32) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u32); }
mtb_u64 constexpr mtb_Normalized_f32_To_u64(mtb_f32 Value) { return (mtb_u64)mtb_Clamp((Value * MTB_MaxValue_u64) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u64); }

mtb_f32 constexpr mtb_Normalized_u08_To_f32(mtb_u08 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u08, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u16_To_f32(mtb_u16 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u16, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u32_To_f32(mtb_u32 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u32, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u64_To_f32(mtb_u64 Value) { return mtb_Clamp((mtb_f32)Value / MTB_MaxValue_u64, 0.0f, 1.0f); }


struct mtb_impl_defer
{
  template<typename LambdaType>
  struct defer
  {
    LambdaType Lambda;
    defer(LambdaType InLambda) : Lambda{ mtb_Move(InLambda) } {}
    ~defer() { Lambda(); }
  };

  template<typename FuncT>
  defer<FuncT> operator =(FuncT InLambda) { return { mtb_Move(InLambda) }; }
};

/// Defers execution of code until the end of the current scope.
///
/// Usage:
///   int i = 0;
///   MTB_DEFER [&](){ i++; printf("Foo %d\n", i); };
///   MTB_DEFER [&](){ i++; printf("Bar %d\n", i); };
///   MTB_DEFER [=](){      printf("Baz %d\n", i); };
///
/// Output:
///   Baz 0
///   Bar 1
///   Foo 2
///
/// \param CaptureSpec The lambda capture specification.
#define MTB_DEFER auto MTB_CONCAT(_Defer, MTB_CURRENT_LINE) = mtb_impl_defer() =

#endif // !defined(MTB_HEADER_mtb_common)


//
// Implementation
//

#if defined(MTB_COMMON_IMPLEMENTATION)

// Impl guard.
#if !defined(MTB_IMPL_mtb_common)
#define MTB_IMPL_mtb_common

#include <math.h>


mtb_concat_strings_result
mtb_ConcatStrings(size_t HeadSize, char const* HeadPtr, size_t TailSize, char const* TailPtr, size_t BufferSize, char* BufferPtr)
{
  size_t BufferIndex{};
  size_t HeadIndex{};
  size_t TailIndex{};

  while(BufferIndex < BufferSize && HeadIndex < HeadSize)
  {
    BufferPtr[BufferIndex++] = HeadPtr[HeadIndex++];
  }

  while(BufferIndex < BufferSize && TailIndex < TailSize)
  {
    BufferPtr[BufferIndex++] = TailPtr[TailIndex++];
  }

  if(BufferIndex < BufferSize)
  {
    // Append the trailing zero-terminator without increasing the BufferIndex.
    BufferPtr[BufferIndex] = '\0';
  }

  mtb_concat_strings_result Result{ BufferIndex, BufferPtr };
  return Result;
}

mtb_f32
mtb_Pow(mtb_f32 Base, mtb_f32 Exponent)
{
  return powf(Base, Exponent);
}

mtb_f64
mtb_Pow(mtb_f64 Base, mtb_f64 Exponent)
{
  return pow(Base, Exponent);
}

mtb_f32
mtb_Mod(mtb_f32 Value, mtb_f32 Divisor)
{
  return fmodf(Value, Divisor);
}

mtb_f64
mtb_Mod(mtb_f64 Value, mtb_f64 Divisor)
{
  return fmod(Value, Divisor);
}

mtb_f32
mtb_Sqrt(mtb_f32 Value)
{
  return sqrtf(Value);
}

mtb_f64
mtb_Sqrt(mtb_f64 Value)
{
  return sqrt(Value);
}

mtb_f32
mtb_InvSqrt(mtb_f32 Value)
{
  union FloatInt
  {
    mtb_f32 Float;
    int Int;
  };
  FloatInt MagicNumber;
  mtb_f32 HalfValue;
  mtb_f32 Result;
  const mtb_f32 ThreeHalfs = 1.5f;

  HalfValue = Value * 0.5f;
  Result = Value;
  MagicNumber.Float = Result;                               // evil floating point bit level hacking
  MagicNumber.Int  = 0x5f3759df - ( MagicNumber.Int >> 1 ); // what the fuck?
  Result = MagicNumber.Float;
  Result = Result * ( ThreeHalfs - ( HalfValue * Result * Result ) ); // 1st iteration

  return Result;
}

mtb_f32
mtb_RoundDown(mtb_f32 Value)
{
  return floorf(Value);
}

mtb_f64
mtb_RoundDown(mtb_f64 Value)
{
  return floor(Value);
}

mtb_f32
mtb_RoundUp(mtb_f32 Value)
{
  return ceilf(Value);
}

mtb_f64
mtb_RoundUp(mtb_f64 Value)
{
  return ceil(Value);
}


bool
mtb_AreNearlyEqual(mtb_f64 A, mtb_f64 B, mtb_f64 Epsilon)
{
  return mtb_Abs(A - B) <= Epsilon;
}

bool
mtb_AreNearlyEqual(mtb_f32 A, mtb_f32 B, mtb_f32 Epsilon)
{
  return mtb_Abs(A - B) <= Epsilon;
}

#endif // !defined(MTB_IMPL_mtb_common)
#endif // defined(MTB_COMMON_IMPLEMENTATION)

#include <new>

/// \defgroup Memory manipulation functions
///
/// Provides functions to work on chunks of memory.
///
/// Unlike C standard functions such as memcpy and memset, these functions
/// respect the type of the input objects. Refer to the table below to find
/// which C standard functionality is covered by which of the functions
/// defined here.
///
/// C Standard Function | Untyped/Bytes                        | Typed
/// ------------------- | ------------------------------------ | -----
/// memcopy, memmove    | mtb_CopyBytes                        | mtb_CopyElements, mtb_CopyConstructElements, mtb_MoveElements, mtb_MoveConstructElements
/// memset              | mtb_SetBytes                         | mtb_SetElements, mtb_ConstructElements
/// memcmp              | mtb_CompareBytes, mtb_AreBytesEqual  | -
///
///
/// All functions are optimized for POD types.
///
/// @{

/// Copy Size from Source to Destination.
///
/// Destination and Source may overlap.
void
mtb_CopyBytes(size_t Size, void* Destination, void const* Source);

/// Fill Size in Destination with the Value.
void
mtb_SetBytes(size_t Size, void* Destination, int Value);

bool
mtb_AreBytesEqual(size_t Size, void const* A, void const* B);

int
mtb_CompareBytes(size_t Size, void const* A, void const* B);

void
mtb_ReverseBytesInPlace(size_t Size, void* Ptr);


/// Calls the constructor of all elements in Destination with Args.
///
/// Args may be empty in which case all elements get default-initialized.
template<typename T, typename... ArgTypes>
void
mtb_ConstructElements(size_t Num, T* Destination, ArgTypes&&... Args);

/// Destructs all elements in Destination.
template<typename T>
void
mtb_DestructElements(size_t Num, T* Destination);

/// Copy all elements from Source to Destination.
///
/// Destination and Source may overlap.
template<typename T>
void
mtb_CopyElements(size_t Num, T* Destination, T const* Source);

/// Copy all elements from Source to Destination using T's constructor.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
mtb_CopyConstructElements(size_t Num, T* Destination, T const* Source);

/// Move all elements from Source to Destination using T's constructor.
///
/// Destination and Source may overlap.
template<typename T>
void
mtb_MoveElements(size_t Num, T* Destination, T* Source);

/// Move all elements from Source to Destination using T's constructor and destruct Source afterwards.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
mtb_MoveConstructElements(size_t Num, T* Destination, T* Source);

/// Assign Item to all elements in Destination.
template<typename T>
void
mtb_SetElements(size_t Num, T* Destination, T const& Item);

bool
mtb_ImplTestMemoryOverlap(size_t SizeA, void const* A, size_t SizeB, void const* B);

template<typename TA, typename TB>
bool
mtb_TestMemoryOverlap(size_t NumA, TA const* A, size_t NumB, TB const* B)
{
  return mtb_ImplTestMemoryOverlap(NumA * mtb_SafeSizeOf<TA>(), reinterpret_cast<void const*>(A),
                                   NumB * mtb_SafeSizeOf<TB>(), reinterpret_cast<void const*>(B));
}

template<typename T>
void
mtb_ReverseElementsInPlace(size_t Num, T* Ptr)
{
  size_t const NumSwaps = Num / 2;
  for(size_t FrontIndex = 0; FrontIndex < NumSwaps; ++FrontIndex)
  {
    size_t const BackIndex = Num - FrontIndex - 1;
    Swap(SomeSlice[FrontIndex], SomeSlice[BackIndex]);
  }
}


//
// Implementation Details
//

// mtb_ConstructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_construct_elements
{
  template<typename... ArgTypes>
  MTB_INLINE static void
  Do(size_t Num, T* Destination, ArgTypes&&... Args)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(mtb_Forward<ArgTypes>(Args)...);
    }
  }
};

template<typename T>
struct mtb_impl_construct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    mtb_SetBytes(Num * mtb_SafeSizeOf<T>(), Destination, 0);
  }

  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    // Blit Item over each element of Destination.
    for(size_t Index = 0; Index < Num; ++Index)
    {
      mtb_CopyBytes(mtb_SafeSizeOf<T>(), &Destination[Index], &Item);
    }
  }
};

template<typename T, typename... ArgTypes>
MTB_INLINE auto
mtb_ConstructElements(size_t Num, T* Destination, ArgTypes&&... Args)
  -> void
{
  mtb_impl_construct_elements<T, MTB_IsPod(T)>::Do(Num, Destination, mtb_Forward<ArgTypes>(Args)...);
}


// mtb_DestructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_destruct_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index].~T();
    }
  }
};

template<typename T>
struct mtb_impl_destruct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    // Nothing to do for POD types.
  }
};

template<typename T>
MTB_INLINE auto
mtb_DestructElements(size_t Num, T* Destination)
  -> void
{
  mtb_impl_destruct_elements<T, MTB_IsPod(T)>::Do(Num, Destination);
}


// mtb_CopyElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_copy_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    if(Destination == Source)
      return;

    if(mtb_TestMemoryOverlap(Num, Destination, Num, Source) && Destination < Source)
    {
      // Copy backwards.
      for(size_t Index = Num; Index > 0;)
      {
        --Index;
        Destination[Index] = Source[Index];
      }
    }
    else
    {
      // Copy forwards.
      for(size_t Index = 0; Index < Num; ++Index)
      {
        Destination[Index] = Source[Index];
      }
    }
  }
};

template<typename T>
struct mtb_impl_copy_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    mtb_CopyBytes(mtb_SafeSizeOf<T>() * Num, Destination, Source);
  }
};

template<typename T>
MTB_INLINE auto
mtb_CopyElements(size_t Num, T* Destination, T const* Source)
  -> void
{
  mtb_impl_copy_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_CopyConstructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_copy_construct_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Source[Index]);
    }
  }
};

template<typename T>
struct mtb_impl_copy_construct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    mtb_CopyElements(Num, Destination, Source);
  }
};

template<typename T>
MTB_INLINE auto
mtb_CopyConstructElements(size_t Num, T* Destination, T const* Source)
  -> void
{
  mtb_impl_copy_construct_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_MoveElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_move_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T* Source)
  {
    if(Destination == Source)
      return;

    if(mtb_TestMemoryOverlap(Num, Destination, Num, Source))
    {
      if(Destination < Source)
      {
        // Move forward
        for(size_t Index = 0; Index < Num; ++Index)
        {
          Destination[Index] = mtb_Move(Source[Index]);
        }

        // Destroy the remaining elements in the back.
        size_t const NumToDestruct = Source - Destination;
        mtb_DestructElements(NumToDestruct, Source + (Num - NumToDestruct));
      }
      else
      {
        // Move backward
        for(size_t Index = Num; Index > 0;)
        {
          --Index;
          Destination[Index] = mtb_Move(Source[Index]);
        }

        // Destroy the remaining elements in the front.
        size_t const NumToDestruct = Destination - Source;
        mtb_DestructElements(NumToDestruct, Source);
      }
    }
    else
    {
      // Straight forward: Move one by one, then destruct all in Source.
      for(size_t Index = 0; Index < Num; ++Index)
      {
        Destination[Index] = mtb_Move(Source[Index]);
      }
      mtb_DestructElements(Num, Source);
    }
  }
};

template<typename T>
struct mtb_impl_move_elements<T, true> : public mtb_impl_copy_elements<T, true> {};

template<typename T>
MTB_INLINE auto
mtb_MoveElements(size_t Num, T* Destination, T* Source)
  -> void
{
  mtb_impl_move_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_MoveConstructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_move_construct_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(mtb_Move(Source[Index]));
    }
    mtb_DestructElements(Num, Source);
  }
};

template<typename T>
struct mtb_impl_move_construct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    mtb_CopyElements(Num, Destination, Source);
  }
};

template<typename T>
MTB_INLINE auto
mtb_MoveConstructElements(size_t Num, T* Destination, T* Source)
  -> void
{
  mtb_impl_move_construct_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_SetElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_set_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index] = {};
    }
  }

  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index] = Item;
    }
  }
};

template<typename T>
struct mtb_impl_set_elements<T, true> : public mtb_impl_construct_elements<T, true> {};

template<typename T>
MTB_INLINE auto
mtb_SetElements(size_t Num, T* Destination)
  -> void
{
  mtb_impl_set_elements<T, MTB_IsPod(T)>::Do(Num, Destination);
}

template<typename T>
MTB_INLINE auto
mtb_SetElements(size_t Num, T* Destination, T const& Item)
  -> void
{
  mtb_impl_set_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Item);
}

#endif // !defined(MTB_HEADER_mtb_memory)

#if defined(MTB_MEMORY_IMPLEMENTATION)

#if !defined(MTB_IMPL_mtb_memory)
#define MTB_IMPL_mtb_memory

#include <string.h>

void
mtb_CopyBytes(size_t Size, void* Destination, void const* Source)
{
  // Using memmove so that Destination and Source may overlap.
  memmove(Destination, Source, Size);
}

void
mtb_SetBytes(size_t Size, void* Destination, int Value)
{
  memset(Destination, Value, Size);
}

bool
mtb_AreBytesEqual(size_t Size, void const* A, void const* B)
{
  return mtb_CompareBytes(Size, A, B) == 0;
}

int
mtb_CompareBytes(size_t Size, void const* A, void const* B)
{
  return memcmp(A, B, Size);
}

void
mtb_ReverseBytesInPlace(size_t Size, void* Ptr)
{
  mtb_byte* BytePtr = (mtb_byte*)Ptr;
  size_t const NumSwaps = Size / 2;
  for(size_t FrontIndex = 0; FrontIndex < NumSwaps; ++FrontIndex)
  {
    size_t const BackIndex = Size - FrontIndex - 1;
    mtb_byte ToSwap = BytePtr[FrontIndex];
    BytePtr[FrontIndex] = BytePtr[BackIndex];
    BytePtr[BackIndex] = ToSwap;
  }
}

bool
mtb_ImplTestMemoryOverlap(size_t SizeA, void const* A, size_t SizeB, void const* B)
{
  size_t LeftA = (size_t)A;
  size_t RightA = LeftA + SizeA;

  size_t LeftB = (size_t)B;
  size_t RightB = LeftB + SizeB;

  return LeftB  >= LeftA && LeftB  <= RightA || // Check if LeftB  is in [A, A+SizeA]
         RightB >= LeftA && RightB <= RightA || // Check if RightB is in [A, A+SizeA]
         LeftA  >= LeftB && LeftA  <= RightB || // Check if LeftA  is in [B, B+SizeB]
         RightA >= LeftB && RightA <= RightB;   // Check if RightA is in [B, B+SizeB]
}

#endif // !defiend(MTB_IMPL_mtb_memory)
#endif // defined(MTB_MEMORY_IMPLEMENTATION)
#if !defined(MTB_HEADER_mtb_conv)
#define MTB_HEADER_mtb_conv

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_CONV_IMPLEMENTATION)
  #define MTB_CONV_IMPLEMENTATION
#endif

#if defined(MTB_CONV_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif



#define MTB_DEFINE_PARSE_STRING_RESULT(NAME, VALUE_TYPE) \
struct NAME \
{ \
  bool Success; \
  size_t RemainingSourceLen; \
  char const* RemainingSourcePtr; \
  VALUE_TYPE Value; \
}


//
// Convert: String -> Floating point
//

MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_f64, mtb_f64);
mtb_parse_string_result_f64
mtb_ParseString_f64(size_t SourceLen, char const* SourcePtr, mtb_f64 Fallback = MTB_NaN_f64);


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_f32, mtb_f32);
MTB_INLINE mtb_parse_string_result_f32
mtb_ParseString_f32(size_t SourceLen, char const* SourcePtr, mtb_f32 Fallback = MTB_NaN_f64)
{
  mtb_parse_string_result_f64 Result_f64 = mtb_ParseString_f64(SourceLen, SourcePtr, (mtb_f64)Fallback);
  mtb_parse_string_result_f32 Result_f32{ Result_f64.Success, Result_f64.RemainingSourceLen, Result_f64.RemainingSourcePtr };
  Result_f32.Value = (mtb_f32)Result_f64.Value;
  return Result_f32;
}


//
// Convert: String -> Unsigned integers
//

MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_u64, mtb_u64);
mtb_parse_string_result_u64
mtb_ParseString_u64(size_t SourceLen, char const* SourcePtr, mtb_u64 Fallback);


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_u32, mtb_u32);
MTB_INLINE mtb_parse_string_result_u32
mtb_ParseString_u32(size_t SourceLen, char const* SourcePtr, mtb_u32 Fallback)
{
  mtb_parse_string_result_u64 Result_u64 = mtb_ParseString_u64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_u32 Result_u32{ Result_u64.Success, Result_u64.RemainingSourceLen, Result_u64.RemainingSourcePtr };
  Result_u32.Value = mtb_SafeConvert_u32(Result_u64.Value);
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_u16, mtb_u16);
MTB_INLINE mtb_parse_string_result_u16
mtb_ParseString_u16(size_t SourceLen, char const* SourcePtr, mtb_u16 Fallback)
{
  mtb_parse_string_result_u64 Result_u64 = mtb_ParseString_u64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_u16 Result_u16{ Result_u64.Success, Result_u64.RemainingSourceLen, Result_u64.RemainingSourcePtr };
  Result_u16.Value = mtb_SafeConvert_u16(Result_u64.Value);
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_u08, mtb_u08);
MTB_INLINE mtb_parse_string_result_u08
mtb_ParseString_u08(size_t SourceLen, char const* SourcePtr, mtb_u08 Fallback)
{
  mtb_parse_string_result_u64 Result_u64 = mtb_ParseString_u64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_u08 Result_u08{ Result_u64.Success, Result_u64.RemainingSourceLen, Result_u64.RemainingSourcePtr };
  Result_u08.Value = mtb_SafeConvert_u08(Result_u64.Value);
}


//
// Convert: String -> Signed integers
//

MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_s64, mtb_s64);
mtb_parse_string_result_s64
mtb_ParseString_s64(size_t SourceLen, char const* SourcePtr, mtb_s64 Fallback);


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_s32, mtb_s32);
MTB_INLINE mtb_parse_string_result_s32
mtb_ParseString_s32(size_t SourceLen, char const* SourcePtr, mtb_s32 Fallback)
{
  mtb_parse_string_result_s64 Result_s64 = mtb_ParseString_s64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_s32 Result_s32{ Result_s64.Success, Result_s64.RemainingSourceLen, Result_s64.RemainingSourcePtr };
  Result_s32.Value = mtb_SafeConvert_s32(Result_s64.Value);
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_s16, mtb_s16);
MTB_INLINE mtb_parse_string_result_s16
mtb_ParseString_s16(size_t SourceLen, char const* SourcePtr, mtb_s16 Fallback)
{
  mtb_parse_string_result_s64 Result_s64 = mtb_ParseString_s64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_s16 Result_s16{ Result_s64.Success, Result_s64.RemainingSourceLen, Result_s64.RemainingSourcePtr };
  Result_s16.Value = mtb_SafeConvert_s16(Result_s64.Value);
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_s08, mtb_s08);
MTB_INLINE mtb_parse_string_result_s08
mtb_ParseString_s08(size_t SourceLen, char const* SourcePtr, mtb_s08 Fallback)
{
  mtb_parse_string_result_s64 Result_s64 = mtb_ParseString_s64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_s08 Result_s08{ Result_s64.Success, Result_s64.RemainingSourceLen, Result_s64.RemainingSourcePtr };
  Result_s08.Value = mtb_SafeConvert_s08(Result_s64.Value);
}


//
// ==============================================
//

struct mtb_to_string_result
{
  bool Success;
  size_t StrLen;
  char* StrPtr;
};


//
// Conversion: String -> Floating Point
//

// TODO


//
// Conversion: Integer -> String
//

mtb_to_string_result
mtb_ToString(mtb_s64 Value, size_t BufferSize, char* BufferPtr);

MTB_INLINE mtb_to_string_result mtb_ToString(mtb_s32 Value, size_t BufferSize, char* BufferPtr) { return mtb_ToString((mtb_s64)Value, BufferSize, BufferPtr); }
MTB_INLINE mtb_to_string_result mtb_ToString(mtb_s16 Value, size_t BufferSize, char* BufferPtr) { return mtb_ToString((mtb_s64)Value, BufferSize, BufferPtr); }
MTB_INLINE mtb_to_string_result mtb_ToString(mtb_s08 Value, size_t BufferSize, char* BufferPtr) { return mtb_ToString((mtb_s64)Value, BufferSize, BufferPtr); }


mtb_to_string_result
mtb_ToString(mtb_u64 Value, size_t BufferSize, char* BufferPtr);

MTB_INLINE mtb_to_string_result mtb_ToString(mtb_u32 Value, size_t BufferSize, char* BufferPtr) { return mtb_ToString((mtb_u64)Value, BufferSize, BufferPtr); }
MTB_INLINE mtb_to_string_result mtb_ToString(mtb_u16 Value, size_t BufferSize, char* BufferPtr) { return mtb_ToString((mtb_u64)Value, BufferSize, BufferPtr); }
MTB_INLINE mtb_to_string_result mtb_ToString(mtb_u08 Value, size_t BufferSize, char* BufferPtr) { return mtb_ToString((mtb_u64)Value, BufferSize, BufferPtr); }


#endif // !defined(MTB_HEADER_mtb_conv)


// ==============
// Implementation
// ==============

#if defined(MTB_CONV_IMPLEMENTATION)
#if !defined(MTB_IMPL_mtb_conv)
#define MTB_IMPL_mtb_conv


// ==========================================
// mtb_memory.h
// ==========================================

#if !defined(MTB_HEADER_mtb_memory)
#define MTB_HEADER_mtb_memory

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_MEMORY_IMPLEMENTATION)
  #define MTB_MEMORY_IMPLEMENTATION
#endif

#if defined(MTB_MEMORY_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


#include <new>

/// \defgroup Memory manipulation functions
///
/// Provides functions to work on chunks of memory.
///
/// Unlike C standard functions such as memcpy and memset, these functions
/// respect the type of the input objects. Refer to the table below to find
/// which C standard functionality is covered by which of the functions
/// defined here.
///
/// C Standard Function | Untyped/Bytes                        | Typed
/// ------------------- | ------------------------------------ | -----
/// memcopy, memmove    | mtb_CopyBytes                        | mtb_CopyElements, mtb_CopyConstructElements, mtb_MoveElements, mtb_MoveConstructElements
/// memset              | mtb_SetBytes                         | mtb_SetElements, mtb_ConstructElements
/// memcmp              | mtb_CompareBytes, mtb_AreBytesEqual  | -
///
///
/// All functions are optimized for POD types.
///
/// @{

/// Copy Size from Source to Destination.
///
/// Destination and Source may overlap.
void
mtb_CopyBytes(size_t Size, void* Destination, void const* Source);

/// Fill Size in Destination with the Value.
void
mtb_SetBytes(size_t Size, void* Destination, int Value);

bool
mtb_AreBytesEqual(size_t Size, void const* A, void const* B);

int
mtb_CompareBytes(size_t Size, void const* A, void const* B);

void
mtb_ReverseBytesInPlace(size_t Size, void* Ptr);


/// Calls the constructor of all elements in Destination with Args.
///
/// Args may be empty in which case all elements get default-initialized.
template<typename T, typename... ArgTypes>
void
mtb_ConstructElements(size_t Num, T* Destination, ArgTypes&&... Args);

/// Destructs all elements in Destination.
template<typename T>
void
mtb_DestructElements(size_t Num, T* Destination);

/// Copy all elements from Source to Destination.
///
/// Destination and Source may overlap.
template<typename T>
void
mtb_CopyElements(size_t Num, T* Destination, T const* Source);

/// Copy all elements from Source to Destination using T's constructor.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
mtb_CopyConstructElements(size_t Num, T* Destination, T const* Source);

/// Move all elements from Source to Destination using T's constructor.
///
/// Destination and Source may overlap.
template<typename T>
void
mtb_MoveElements(size_t Num, T* Destination, T* Source);

/// Move all elements from Source to Destination using T's constructor and destruct Source afterwards.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
mtb_MoveConstructElements(size_t Num, T* Destination, T* Source);

/// Assign Item to all elements in Destination.
template<typename T>
void
mtb_SetElements(size_t Num, T* Destination, T const& Item);

bool
mtb_ImplTestMemoryOverlap(size_t SizeA, void const* A, size_t SizeB, void const* B);

template<typename TA, typename TB>
bool
mtb_TestMemoryOverlap(size_t NumA, TA const* A, size_t NumB, TB const* B)
{
  return mtb_ImplTestMemoryOverlap(NumA * mtb_SafeSizeOf<TA>(), reinterpret_cast<void const*>(A),
                                   NumB * mtb_SafeSizeOf<TB>(), reinterpret_cast<void const*>(B));
}

template<typename T>
void
mtb_ReverseElementsInPlace(size_t Num, T* Ptr)
{
  size_t const NumSwaps = Num / 2;
  for(size_t FrontIndex = 0; FrontIndex < NumSwaps; ++FrontIndex)
  {
    size_t const BackIndex = Num - FrontIndex - 1;
    Swap(SomeSlice[FrontIndex], SomeSlice[BackIndex]);
  }
}


//
// Implementation Details
//

// mtb_ConstructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_construct_elements
{
  template<typename... ArgTypes>
  MTB_INLINE static void
  Do(size_t Num, T* Destination, ArgTypes&&... Args)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(mtb_Forward<ArgTypes>(Args)...);
    }
  }
};

template<typename T>
struct mtb_impl_construct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    mtb_SetBytes(Num * mtb_SafeSizeOf<T>(), Destination, 0);
  }

  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    // Blit Item over each element of Destination.
    for(size_t Index = 0; Index < Num; ++Index)
    {
      mtb_CopyBytes(mtb_SafeSizeOf<T>(), &Destination[Index], &Item);
    }
  }
};

template<typename T, typename... ArgTypes>
MTB_INLINE auto
mtb_ConstructElements(size_t Num, T* Destination, ArgTypes&&... Args)
  -> void
{
  mtb_impl_construct_elements<T, MTB_IsPod(T)>::Do(Num, Destination, mtb_Forward<ArgTypes>(Args)...);
}


// mtb_DestructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_destruct_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index].~T();
    }
  }
};

template<typename T>
struct mtb_impl_destruct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    // Nothing to do for POD types.
  }
};

template<typename T>
MTB_INLINE auto
mtb_DestructElements(size_t Num, T* Destination)
  -> void
{
  mtb_impl_destruct_elements<T, MTB_IsPod(T)>::Do(Num, Destination);
}


// mtb_CopyElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_copy_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    if(Destination == Source)
      return;

    if(mtb_TestMemoryOverlap(Num, Destination, Num, Source) && Destination < Source)
    {
      // Copy backwards.
      for(size_t Index = Num; Index > 0;)
      {
        --Index;
        Destination[Index] = Source[Index];
      }
    }
    else
    {
      // Copy forwards.
      for(size_t Index = 0; Index < Num; ++Index)
      {
        Destination[Index] = Source[Index];
      }
    }
  }
};

template<typename T>
struct mtb_impl_copy_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    mtb_CopyBytes(mtb_SafeSizeOf<T>() * Num, Destination, Source);
  }
};

template<typename T>
MTB_INLINE auto
mtb_CopyElements(size_t Num, T* Destination, T const* Source)
  -> void
{
  mtb_impl_copy_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_CopyConstructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_copy_construct_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Source[Index]);
    }
  }
};

template<typename T>
struct mtb_impl_copy_construct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    mtb_CopyElements(Num, Destination, Source);
  }
};

template<typename T>
MTB_INLINE auto
mtb_CopyConstructElements(size_t Num, T* Destination, T const* Source)
  -> void
{
  mtb_impl_copy_construct_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_MoveElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_move_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T* Source)
  {
    if(Destination == Source)
      return;

    if(mtb_TestMemoryOverlap(Num, Destination, Num, Source))
    {
      if(Destination < Source)
      {
        // Move forward
        for(size_t Index = 0; Index < Num; ++Index)
        {
          Destination[Index] = mtb_Move(Source[Index]);
        }

        // Destroy the remaining elements in the back.
        size_t const NumToDestruct = Source - Destination;
        mtb_DestructElements(NumToDestruct, Source + (Num - NumToDestruct));
      }
      else
      {
        // Move backward
        for(size_t Index = Num; Index > 0;)
        {
          --Index;
          Destination[Index] = mtb_Move(Source[Index]);
        }

        // Destroy the remaining elements in the front.
        size_t const NumToDestruct = Destination - Source;
        mtb_DestructElements(NumToDestruct, Source);
      }
    }
    else
    {
      // Straight forward: Move one by one, then destruct all in Source.
      for(size_t Index = 0; Index < Num; ++Index)
      {
        Destination[Index] = mtb_Move(Source[Index]);
      }
      mtb_DestructElements(Num, Source);
    }
  }
};

template<typename T>
struct mtb_impl_move_elements<T, true> : public mtb_impl_copy_elements<T, true> {};

template<typename T>
MTB_INLINE auto
mtb_MoveElements(size_t Num, T* Destination, T* Source)
  -> void
{
  mtb_impl_move_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_MoveConstructElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_move_construct_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(mtb_Move(Source[Index]));
    }
    mtb_DestructElements(Num, Source);
  }
};

template<typename T>
struct mtb_impl_move_construct_elements<T, true>
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_AssertDebug(!mtb_TestMemoryOverlap(Num, Destination, Num, Source));

    mtb_CopyElements(Num, Destination, Source);
  }
};

template<typename T>
MTB_INLINE auto
mtb_MoveConstructElements(size_t Num, T* Destination, T* Source)
  -> void
{
  mtb_impl_move_construct_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Source);
}


// mtb_SetElements

template<typename T, bool TIsPlainOldData = false>
struct mtb_impl_set_elements
{
  MTB_INLINE static void
  Do(size_t Num, T* Destination)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index] = {};
    }
  }

  MTB_INLINE static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index] = Item;
    }
  }
};

template<typename T>
struct mtb_impl_set_elements<T, true> : public mtb_impl_construct_elements<T, true> {};

template<typename T>
MTB_INLINE auto
mtb_SetElements(size_t Num, T* Destination)
  -> void
{
  mtb_impl_set_elements<T, MTB_IsPod(T)>::Do(Num, Destination);
}

template<typename T>
MTB_INLINE auto
mtb_SetElements(size_t Num, T* Destination, T const& Item)
  -> void
{
  mtb_impl_set_elements<T, MTB_IsPod(T)>::Do(Num, Destination, Item);
}

#endif // !defined(MTB_HEADER_mtb_memory)

#if defined(MTB_MEMORY_IMPLEMENTATION)

#if !defined(MTB_IMPL_mtb_memory)
#define MTB_IMPL_mtb_memory

#include <string.h>

void
mtb_CopyBytes(size_t Size, void* Destination, void const* Source)
{
  // Using memmove so that Destination and Source may overlap.
  memmove(Destination, Source, Size);
}

void
mtb_SetBytes(size_t Size, void* Destination, int Value)
{
  memset(Destination, Value, Size);
}

bool
mtb_AreBytesEqual(size_t Size, void const* A, void const* B)
{
  return mtb_CompareBytes(Size, A, B) == 0;
}

int
mtb_CompareBytes(size_t Size, void const* A, void const* B)
{
  return memcmp(A, B, Size);
}

void
mtb_ReverseBytesInPlace(size_t Size, void* Ptr)
{
  mtb_byte* BytePtr = (mtb_byte*)Ptr;
  size_t const NumSwaps = Size / 2;
  for(size_t FrontIndex = 0; FrontIndex < NumSwaps; ++FrontIndex)
  {
    size_t const BackIndex = Size - FrontIndex - 1;
    mtb_byte ToSwap = BytePtr[FrontIndex];
    BytePtr[FrontIndex] = BytePtr[BackIndex];
    BytePtr[BackIndex] = ToSwap;
  }
}

bool
mtb_ImplTestMemoryOverlap(size_t SizeA, void const* A, size_t SizeB, void const* B)
{
  size_t LeftA = (size_t)A;
  size_t RightA = LeftA + SizeA;

  size_t LeftB = (size_t)B;
  size_t RightB = LeftB + SizeB;

  return LeftB  >= LeftA && LeftB  <= RightA || // Check if LeftB  is in [A, A+SizeA]
         RightB >= LeftA && RightB <= RightA || // Check if RightB is in [A, A+SizeA]
         LeftA  >= LeftB && LeftA  <= RightB || // Check if LeftA  is in [B, B+SizeB]
         RightA >= LeftB && RightA <= RightB;   // Check if RightA is in [B, B+SizeB]
}

#endif // !defiend(MTB_IMPL_mtb_memory)
#endif // defined(MTB_MEMORY_IMPLEMENTATION)

static size_t
mtb_TrimWhitespaceFront(size_t* SourceLen, char const** SourcePtr)
{
  size_t Len = *SourceLen;
  char const* Ptr = *SourcePtr;

  while(Len && mtb_IsWhitespace(Ptr[0]))
  {
    ++Ptr;
    --Len;
  }

  size_t NumTrimmed = *SourceLen - Len;
  *SourceLen = Len;
  *SourcePtr = Ptr;
  return NumTrimmed;
}

/// If a '+' was consumed, `1` is returned and \a SourcePtr is advanced by 1,
/// else, if a '-' was consumed, `-1` is returned and \a SourcePtr is advanced by 1,
/// otherwise 0 is returned and \a SourcePtr will not be modified.
static int
mtb_ConsumeSign(size_t* SourceLen, char const** SourcePtr)
{
  int Result{};
  size_t Len = *SourceLen;
  char const* Ptr = *SourcePtr;

  if(Len)
  {
    if(Ptr[0] == '+')
    {
      ++Ptr;
      --Len;
      Result = 1;
    }
    else if(Ptr[0] == '-')
    {
      ++Ptr;
      --Len;
      Result = -1;
    }
  }

  *SourceLen = Len;
  *SourcePtr = Ptr;
  return Result;
}

mtb_parse_string_result_f64
mtb_ParseString_f64(size_t SourceLen, char const* SourcePtr, mtb_f64 Fallback)
{
  mtb_parse_string_result_f64 Result{};
  Result.Value = Fallback;
  Result.RemainingSourceLen = SourceLen;
  Result.RemainingSourcePtr = SourcePtr;

  size_t Len = SourceLen;
  char const* Ptr = SourcePtr;

  mtb_TrimWhitespaceFront(&Len, &Ptr);
  if(Len)
  {
    bool HasSign = mtb_ConsumeSign(&Len, &Ptr) < 0;

    // Parse all parts of a floating point number.
    if(Len)
    {
      // Numeric part
      mtb_parse_string_result_u64 NumericResult = mtb_ParseString_u64(Len, Ptr, (mtb_u64)-1);
      Len = NumericResult.RemainingSourceLen;
      Ptr = NumericResult.RemainingSourcePtr;
      double Value = (double)NumericResult.Value;

      // Decimal part
      bool HasDecimalPart = false;
      mtb_u64 DecimalValue = 0;
      mtb_u64 DecimalDivider = 1;
      if(Len && Ptr[0] == '.')
      {
        ++Ptr;
        --Len;
        while(Len && mtb_IsDigit(Ptr[0]))
        {
          mtb_u64 NewDigit = (mtb_u64)(Ptr[0] - '0');
          DecimalValue = (10 * DecimalValue) + NewDigit;
          HasDecimalPart = true;
          DecimalDivider *= 10;
          ++Ptr;
          --Len;
        }

        Value += (double)DecimalValue / (double)DecimalDivider;
      }

      if(NumericResult.Success || HasDecimalPart)
      {
        // Parse exponent, if any.
        if(Len && (Ptr[0] == 'e' || Ptr[0] == 'E'))
        {
          size_t ExponentSourceLen = Len - 1;
          char const* ExponentSourcePtr = Ptr + 1;
          bool ExponentHasSign = mtb_ConsumeSign(&ExponentSourceLen, &ExponentSourcePtr) < 0;

          mtb_parse_string_result_u64 ExponentResult = mtb_ParseString_u64(ExponentSourceLen, ExponentSourcePtr, (mtb_u64)-1);
          if(ExponentResult.Success)
          {
            Len = ExponentResult.RemainingSourceLen;
            Ptr = ExponentResult.RemainingSourcePtr;

            mtb_u64 ExponentPart = ExponentResult.Value;
            mtb_s64 ExponentValue = 1;
            while(ExponentPart > 0)
            {
              ExponentValue *= 10;
              --ExponentPart;
            }

            if(ExponentHasSign) Value /= ExponentValue;
            else                Value *= ExponentValue;
          }
        }

        Result.Success = true;
        Result.RemainingSourceLen = Len;
        Result.RemainingSourcePtr = Ptr;
        Result.Value = HasSign ? -Value : Value;
      }
    }
  }

  return Result;
}

mtb_parse_string_result_u64
mtb_ParseString_u64(size_t SourceLen, char const* SourcePtr, mtb_u64 Fallback)
{
  mtb_parse_string_result_u64 Result{};
  Result.Value = Fallback;
  Result.RemainingSourceLen = SourceLen;
  Result.RemainingSourcePtr = SourcePtr;

  size_t Len = SourceLen;
  char const* Ptr = SourcePtr;

  if(Len)
  {
    mtb_TrimWhitespaceFront(&Len, &Ptr);
    bool HasSign = mtb_ConsumeSign(&Len, &Ptr) < 0;
    if(!HasSign)
    {
      mtb_u64 NumericalPart = 0;
      bool HasNumericalPart = false;

      while(Len && mtb_IsDigit(Ptr[0]))
      {
        NumericalPart = (10 * NumericalPart) + (*Ptr - '0');
        HasNumericalPart = true;
        --Len;
        ++Ptr;
      }

      if(HasNumericalPart)
      {
        Result.Value = NumericalPart;
        Result.Success = true;
      }
    }
  }

  if(Result.Success)
  {
    Result.RemainingSourceLen = Len;
    Result.RemainingSourcePtr = Ptr;
  }

  return Result;
}

mtb_parse_string_result_s64
mtb_ParseString_s64(size_t SourceLen, char const* SourcePtr, mtb_s64 Fallback)
{
  mtb_parse_string_result_s64 Result{};
  Result.Value = Fallback;
  Result.RemainingSourceLen = SourceLen;
  Result.RemainingSourcePtr = SourcePtr;

  size_t Len = SourceLen;
  char const* Ptr = SourcePtr;

  if(Len)
  {
    mtb_TrimWhitespaceFront(&Len, &Ptr);
    bool HasSign = mtb_ConsumeSign(&Len, &Ptr) < 0;

    mtb_u64 NumericalPart = 0;
    bool HasNumericalPart = false;

    while(Len && mtb_IsDigit(Ptr[0]))
    {
      NumericalPart = (10 * NumericalPart) + (*Ptr - '0');
      HasNumericalPart = true;
      --Len;
      ++Ptr;
    }

    if(HasNumericalPart)
    {
      if(HasSign)
      {
        if(NumericalPart <= (mtb_u64)MTB_MaxValue_s64 + 1)
        {
          if(NumericalPart == (mtb_u64)MTB_MaxValue_s64 + 1)
          {
            Result.Value = MTB_MinValue_s64;
          }
          else
          {
            Result.Value = -(mtb_s64)NumericalPart;
          }

          Result.Success = true;
        }
      }
      else
      {
        if(NumericalPart <= MTB_MaxValue_s64)
        {
          Result.Value = NumericalPart;
          Result.Success = true;
        }
      }
    }
  }

  if(Result.Success)
  {
    Result.RemainingSourceLen = Len;
    Result.RemainingSourcePtr = Ptr;
  }

  return Result;
}


mtb_to_string_result
mtb_ToString(mtb_s64 Value, size_t BufferSize, char* BufferPtr)
{
  #if MTB_FLAG(INTERNAL)
    mtb_s64 OriginalValue = Value;
  #endif
  mtb_to_string_result Result{};
  Result.StrPtr = BufferPtr;

  if(BufferSize)
  {
    size_t NumChars = 0;
    if(Value < 0)
    {
      BufferPtr[NumChars++] = '-';
        // TODO: What if Value == MTB_MinValue_s64?
      Value = -Value;
    }

    while(Value > 0)
    {
      char Digit = (char)(Value % 10);
      BufferPtr[NumChars++] = '0' + Digit;
      Value /= 10;
    }

    Result.StrLen = NumChars;

      // Characters are now in reverse order, so we swap them around.
    mtb_ReverseBytesInPlace(Result.StrLen, Result.StrPtr);

    Result.Success = true;
  }

  return Result;
}

mtb_to_string_result
mtb_ToString(mtb_u64 Value, size_t BufferSize, char* BufferPtr)
{
  #if MTB_FLAG(INTERNAL)
    mtb_u64 OriginalValue = Value;
  #endif
  mtb_to_string_result Result{};
  Result.StrPtr = BufferPtr;

  if(BufferSize)
  {
    size_t NumChars = 0;
    while(Value > 0)
    {
      char Digit = (char)(Value % 10);
      BufferPtr[NumChars++] = '0' + Digit;
      Value /= 10;
    }

    Result.StrLen = NumChars;

      // Characters are now in reverse order, so we swap them around.
    mtb_ReverseBytesInPlace(Result.StrLen, Result.StrPtr);

    Result.Success = true;
  }

  return Result;
}


#endif // !defined(MTB_IMPL_mtb_conv)
#endif // defined(MTB_CONV_IMPLEMENTATION)
