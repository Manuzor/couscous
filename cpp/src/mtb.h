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
#elif defined(__clang__)
  #define MTB_CURRENT_COMPILER MTB_COMPILER_TYPE_CLANG
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

#if defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__)
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
        if(mtb_OnFailedCheck(MTB_CURRENT_FILE, MTB_CURRENT_LINE, MTB_CURRENT_FUNCTION, #CONDITION, ## __VA_ARGS__)) \
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

constexpr mtb_u64 mtb_KiB(mtb_u64 Amount) { return Amount * 1024; }
constexpr mtb_u64 mtb_MiB(mtb_u64 Amount) { return Amount * 1024 * 1024; }
constexpr mtb_u64 mtb_GiB(mtb_u64 Amount) { return Amount * 1024 * 1024 * 1024; }
constexpr mtb_u64 mtb_TiB(mtb_u64 Amount) { return Amount * 1024 * 1024 * 1024 * 1024; }

constexpr mtb_u64 mtb_KB(mtb_u64 Amount) { return Amount * 1000; }
constexpr mtb_u64 mtb_MB(mtb_u64 Amount) { return Amount * 1000 * 1000; }
constexpr mtb_u64 mtb_GB(mtb_u64 Amount) { return Amount * 1000 * 1000 * 1000; }
constexpr mtb_u64 mtb_TB(mtb_u64 Amount) { return Amount * 1000 * 1000 * 1000 * 1000; }

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
constexpr mtb_u08 mtb_SetBit(mtb_u08 Bits, mtb_u08 Position)   { return Bits | (mtb_u08(1) << Position); }
constexpr mtb_u16 mtb_SetBit(mtb_u16 Bits, mtb_u16 Position)   { return Bits | (mtb_u16(1) << Position); }
constexpr mtb_u32 mtb_SetBit(mtb_u32 Bits, mtb_u32 Position)   { return Bits | (mtb_u32(1) << Position); }
constexpr mtb_u64 mtb_SetBit(mtb_u64 Bits, mtb_u64 Position)   { return Bits | (mtb_u64(1) << Position); }

constexpr mtb_u08 mtb_UnsetBit(mtb_u08 Bits, mtb_u08 Position) { return Bits & ~(mtb_u08(1) << Position); }
constexpr mtb_u16 mtb_UnsetBit(mtb_u16 Bits, mtb_u16 Position) { return Bits & ~(mtb_u16(1) << Position); }
constexpr mtb_u32 mtb_UnsetBit(mtb_u32 Bits, mtb_u32 Position) { return Bits & ~(mtb_u32(1) << Position); }
constexpr mtb_u64 mtb_UnsetBit(mtb_u64 Bits, mtb_u64 Position) { return Bits & ~(mtb_u64(1) << Position); }

constexpr bool mtb_IsBitSet(mtb_u08 Bits, mtb_u08 Position) { return !!(Bits & (mtb_u08(1) << Position)); }
constexpr bool mtb_IsBitSet(mtb_u16 Bits, mtb_u16 Position) { return !!(Bits & (mtb_u16(1) << Position)); }
constexpr bool mtb_IsBitSet(mtb_u32 Bits, mtb_u32 Position) { return !!(Bits & (mtb_u32(1) << Position)); }
constexpr bool mtb_IsBitSet(mtb_u64 Bits, mtb_u64 Position) { return !!(Bits & (mtb_u64(1) << Position)); }

constexpr bool mtb_IsPowerOfTwo(mtb_u08 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u16 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u32 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u64 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }

constexpr mtb_u32 mtb_AlignValue_PowerOfTwo(mtb_u32 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }
constexpr mtb_u64 mtb_AlignValue_PowerOfTwo(mtb_u64 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }

inline void* mtb_AlignPointer(void* Pointer, int Alignment) { return (void*)mtb_AlignValue_PowerOfTwo((size_t)Pointer, Alignment); }

//
// ============================
//
#define MTB_Pi32     3.14159265359f
#define MTB_TwoPi32  6.28318530718f
#define MTB_HalfPi32 1.57079632679f
#define MTB_InvPi32  0.31830988618f

#define MTB_DegreesToRadians32 (MTB_Pi32 / 180.0f)
#define MTB_RadiansToDegrees32 (180.0f / MTB_Pi32)

#define MTB_Pi64     3.14159265359
#define MTB_TwoPi64  6.28318530718
#define MTB_HalfPi64 1.57079632679
#define MTB_InvPi64  0.31830988618

#define MTB_DegreesToRadians64 (MTB_Pi64 / 180.0)
#define MTB_RadiansToDegrees64 (180.0 / MTB_Pi64)

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
#elif MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_CLANG
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
#define MTB_MinValue_s08 0x80
#define MTB_MaxValue_s08 0x7F
#define MTB_MinValue_s16 0x8000
#define MTB_MaxValue_s16 0x7FFF
#define MTB_MinValue_s32 0x80000000
#define MTB_MaxValue_s32 0x7FFFFFFF
#define MTB_MinValue_s64 0x8000000000000000
#define MTB_MaxValue_s64 0x7FFFFFFFFFFFFFFF

#define MTB_MinValue_u08 0
#define MTB_MaxValue_u08 0xFF
#define MTB_MinValue_u16 0
#define MTB_MaxValue_u16 0xFFFF
#define MTB_MinValue_u32 0
#define MTB_MaxValue_u32 0xFFFFFFFF
#define MTB_MinValue_u64 0
#define MTB_MaxValue_u64 0xFFFFFFFFFFFFFFFF

const mtb_s08 mtb_MinValue_s08 = MTB_MinValue_s08;
const mtb_s08 mtb_MaxValue_s08 = MTB_MaxValue_s08;
const mtb_s16 mtb_MinValue_s16 = MTB_MinValue_s16;
const mtb_s16 mtb_MaxValue_s16 = MTB_MaxValue_s16;
const mtb_s32 mtb_MinValue_s32 = MTB_MinValue_s32;
const mtb_s32 mtb_MaxValue_s32 = MTB_MaxValue_s32;
const mtb_s64 mtb_MinValue_s64 = MTB_MinValue_s64;
const mtb_s64 mtb_MaxValue_s64 = MTB_MaxValue_s64;

const mtb_u08 mtb_MinValue_u08 = MTB_MinValue_u08;
const mtb_u08 mtb_MaxValue_u08 = MTB_MaxValue_u08;
const mtb_u16 mtb_MinValue_u16 = MTB_MinValue_u16;
const mtb_u16 mtb_MaxValue_u16 = MTB_MaxValue_u16;
const mtb_u32 mtb_MinValue_u32 = MTB_MinValue_u32;
const mtb_u32 mtb_MaxValue_u32 = MTB_MaxValue_u32;
const mtb_u64 mtb_MinValue_u64 = MTB_MinValue_u64;
const mtb_u64 mtb_MaxValue_u64 = MTB_MaxValue_u64;


#define MTB_GetNumBits(TYPE) (sizeof(TYPE)*8)


constexpr bool mtb_IsDigit(char Value) { return Value >= '0' && Value <= '9'; }
constexpr bool mtb_IsWhitespace(char Value) { return Value == ' '  || Value == '\n' || Value == '\r' || Value == '\t' || Value == '\b'; }

//
// NaN
#if (MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_MSVC) || (MTB_CURRENT_COMPILER == MTB_COMPILER_TYPE_CLANG)
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
constexpr mtb_rm_ref<T>&&
mtb_Move(T&& Argument)
{
  // forward Argument as movable
  return static_cast<mtb_rm_ref<T>&&>(Argument);
}

template<typename T>
constexpr T&&
mtb_Forward(mtb_rm_ref<T>& Argument)
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
    return DestT(Value);
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


// Integer conversion table.
// An 'x' marks whether a conversion is necessary.
//
//    |s08| s16|s32|s64|u08|u16|u32|u64
// ---|---|----|---|---|---|---|---|---
// s08|   |    |   |   | x | x | x | x
// s16| x |    |   |   | x | x | x | x
// s32| x |  x |   |   | x | x | x | x
// s64| x |  x | x |   | x | x | x | x
// u08| x |    |   |   |   |   |   |
// u16| x |  x |   |   | x |   |   |
// u32| x |  x | x |   | x | x |   |
// u64| x |  x | x | x | x | x | x |

MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u08 Value) { MTB_Require(Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u16 Value) { MTB_Require(Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u32 Value) { MTB_Require(Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s16 Value) { MTB_Require(MTB_MinValue_s08 <= Value && Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s32 Value) { MTB_Require(MTB_MinValue_s08 <= Value && Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s64 Value) { MTB_Require(MTB_MinValue_s08 <= Value && Value <= MTB_MaxValue_s08, "Value out of range."); return (mtb_s08)Value; }

MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_u16 Value) { MTB_Require(Value <= MTB_MaxValue_s16, "Value out of range."); return (mtb_s16)Value; }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_u32 Value) { MTB_Require(Value <= MTB_MaxValue_s16, "Value out of range."); return (mtb_s16)Value; }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_s16, "Value out of range."); return (mtb_s16)Value; }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s32 Value) { MTB_Require(MTB_MinValue_s16 <= Value && Value <= MTB_MaxValue_s16, "Value out of range."); return (mtb_s16)Value; }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s64 Value) { MTB_Require(MTB_MinValue_s16 <= Value && Value <= MTB_MaxValue_s16, "Value out of range."); return (mtb_s16)Value; }

MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_u32 Value) { MTB_Require(Value <= MTB_MaxValue_s32, "Value out of range."); return (mtb_s32)Value; }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_s32, "Value out of range."); return (mtb_s32)Value; }
MTB_INLINE mtb_s64 mtb_SafeConvert_s64(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_s64, "Value out of range."); return (mtb_s64)Value; }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_s64 Value) { MTB_Require(MTB_MinValue_s32 <= Value && Value <= MTB_MaxValue_s32, "Value out of range."); return (mtb_s32)Value; }

MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s08 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u08)Value; }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s16 Value) { MTB_Require(Value >= 0 && Value <= MTB_MaxValue_u08, "Value out of range."); return (mtb_u08)Value; }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s32 Value) { MTB_Require(Value >= 0 && Value <= MTB_MaxValue_u08, "Value out of range."); return (mtb_u08)Value; }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s64 Value) { MTB_Require(Value >= 0 && Value <= MTB_MaxValue_u08, "Value out of range."); return (mtb_u08)Value; }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u16 Value) { MTB_Require(Value <= MTB_MaxValue_u08, "Value out of range."); return (mtb_u08)Value; }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u32 Value) { MTB_Require(Value <= MTB_MaxValue_u08, "Value out of range."); return (mtb_u08)Value; }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_u08, "Value out of range."); return (mtb_u08)Value; }

MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s08 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u16)Value; }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s16 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u16)Value; }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s32 Value) { MTB_Require(Value >= 0 && Value <= MTB_MaxValue_u16, "Value out of range."); return (mtb_u16)Value; }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s64 Value) { MTB_Require(Value >= 0 && Value <= MTB_MaxValue_u16, "Value out of range."); return (mtb_u16)Value; }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u32 Value) { MTB_Require(Value <= MTB_MaxValue_u16, "Value out of range."); return (mtb_u16)Value; }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_u16, "Value out of range."); return (mtb_u16)Value; }

MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s08 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u32)Value; }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s16 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u32)Value; }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s32 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u32)Value; }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s64 Value) { MTB_Require(Value >= 0 && Value <= MTB_MaxValue_u32, "Value out of range."); return (mtb_u32)Value; }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_u64 Value) { MTB_Require(Value <= MTB_MaxValue_u32, "Value out of range."); return (mtb_u32)Value; }

MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s08 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u64)Value; }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s16 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u64)Value; }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s32 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u64)Value; }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s64 Value) { MTB_Require(Value >= 0, "Value out of range."); return (mtb_u64)Value; }


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

constexpr mtb_f32
mtb_Lerp(mtb_f32 A, mtb_f32 B, mtb_f32 Alpha)
{
  return (1.0f - Alpha) * A + Alpha * B;
}

constexpr mtb_f64
mtb_Lerp(mtb_f64 A, mtb_f64 B, mtb_f64 Alpha)
{
  return (1.0 - Alpha) * A + Alpha * B;
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
mtb_u08 constexpr mtb_Normalized_f32_To_u08(mtb_f32 Value) { return (mtb_u08)mtb_Clamp((Value * (mtb_f32)MTB_MaxValue_u08) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u08); }
mtb_u16 constexpr mtb_Normalized_f32_To_u16(mtb_f32 Value) { return (mtb_u16)mtb_Clamp((Value * (mtb_f32)MTB_MaxValue_u16) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u16); }
mtb_u32 constexpr mtb_Normalized_f32_To_u32(mtb_f32 Value) { return (mtb_u32)mtb_Clamp((Value * (mtb_f32)MTB_MaxValue_u32) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u32); }
mtb_u64 constexpr mtb_Normalized_f32_To_u64(mtb_f32 Value) { return (mtb_u64)mtb_Clamp((Value * (mtb_f32)MTB_MaxValue_u64) + 0.5f, 0.0f, (mtb_f32)MTB_MaxValue_u64); }

mtb_f32 constexpr mtb_Normalized_u08_To_f32(mtb_u08 Value) { return mtb_Clamp((mtb_f32)Value / (mtb_f32)MTB_MaxValue_u08, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u16_To_f32(mtb_u16 Value) { return mtb_Clamp((mtb_f32)Value / (mtb_f32)MTB_MaxValue_u16, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u32_To_f32(mtb_u32 Value) { return mtb_Clamp((mtb_f32)Value / (mtb_f32)MTB_MaxValue_u32, 0.0f, 1.0f); }
mtb_f32 constexpr mtb_Normalized_u64_To_f32(mtb_u64 Value) { return mtb_Clamp((mtb_f32)Value / (mtb_f32)MTB_MaxValue_u64, 0.0f, 1.0f); }


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
constexpr mtb_u08 mtb_SetBit(mtb_u08 Bits, mtb_u08 Position)   { return Bits | (mtb_u08(1) << Position); }
constexpr mtb_u16 mtb_SetBit(mtb_u16 Bits, mtb_u16 Position)   { return Bits | (mtb_u16(1) << Position); }
constexpr mtb_u32 mtb_SetBit(mtb_u32 Bits, mtb_u32 Position)   { return Bits | (mtb_u32(1) << Position); }
constexpr mtb_u64 mtb_SetBit(mtb_u64 Bits, mtb_u64 Position)   { return Bits | (mtb_u64(1) << Position); }

constexpr mtb_u08 mtb_UnsetBit(mtb_u08 Bits, mtb_u08 Position) { return Bits & ~(mtb_u08(1) << Position); }
constexpr mtb_u16 mtb_UnsetBit(mtb_u16 Bits, mtb_u16 Position) { return Bits & ~(mtb_u16(1) << Position); }
constexpr mtb_u32 mtb_UnsetBit(mtb_u32 Bits, mtb_u32 Position) { return Bits & ~(mtb_u32(1) << Position); }
constexpr mtb_u64 mtb_UnsetBit(mtb_u64 Bits, mtb_u64 Position) { return Bits & ~(mtb_u64(1) << Position); }

constexpr bool mtb_IsBitSet(mtb_u08 Bits, mtb_u08 Position) { return !!(Bits & (mtb_u08(1) << Position)); }
constexpr bool mtb_IsBitSet(mtb_u16 Bits, mtb_u16 Position) { return !!(Bits & (mtb_u16(1) << Position)); }
constexpr bool mtb_IsBitSet(mtb_u32 Bits, mtb_u32 Position) { return !!(Bits & (mtb_u32(1) << Position)); }
constexpr bool mtb_IsBitSet(mtb_u64 Bits, mtb_u64 Position) { return !!(Bits & (mtb_u64(1) << Position)); }

constexpr bool mtb_IsPowerOfTwo(mtb_u08 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u16 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u32 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool mtb_IsPowerOfTwo(mtb_u64 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }

constexpr mtb_u32 mtb_AlignValue_PowerOfTwo(mtb_u32 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }
constexpr mtb_u64 mtb_AlignValue_PowerOfTwo(mtb_u64 Value, int Alignment) { return ((Value + Alignment - 1) / Alignment) * Alignment; }

inline void* mtb_AlignPointer(void* Pointer, int Alignment) { return (void*)mtb_AlignValue_PowerOfTwo((size_t)Pointer, Alignment); }

//
// ============================
//
#define MTB_Pi32     3.14159265359f
#define MTB_TwoPi32  6.28318530718f
#define MTB_HalfPi32 1.57079632679f
#define MTB_InvPi32  0.31830988618f

#define MTB_DegreesToRadians32 (MTB_Pi32 / 180.0f)
#define MTB_RadiansToDegrees32 (180.0f / MTB_Pi32)

#define MTB_Pi64     3.14159265359
#define MTB_TwoPi64  6.28318530718
#define MTB_HalfPi64 1.57079632679
#define MTB_InvPi64  0.31830988618

#define MTB_DegreesToRadians64 (MTB_Pi64 / 180.0)
#define MTB_RadiansToDegrees64 (180.0 / MTB_Pi64)

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
constexpr mtb_rm_ref<T>&&
mtb_Move(T&& Argument)
{
  // forward Argument as movable
  return static_cast<mtb_rm_ref<T>&&>(Argument);
}

template<typename T>
constexpr T&&
mtb_Forward(mtb_rm_ref<T>& Argument)
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
    return DestT(Value);
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


// Integer conversion table.
// An 'x' marks whether a conversion is necessary.
//
//    |s08| s16|s32|s64|u08|u16|u32|u64
// ---|---|----|---|---|---|---|---|---
// s08|   |    |   |   | x | x | x | x
// s16| x |    |   |   | x | x | x | x
// s32| x |  x |   |   | x | x | x | x
// s64| x |  x | x |   | x | x | x | x
// u08| x |    |   |   |   |   |   |
// u16| x |  x |   |   | x |   |   |
// u32| x |  x | x |   | x | x |   |
// u64| x |  x | x | x | x | x | x |

#define MTB__SAFE_CONVERT_BODY(RESULT_TYPE) if(Value >= MTB_MinValue_##RESULT_TYPE && Value <= MTB_MaxValue_##RESULT_TYPE) \
  MTB_Require(Value >= MTB_MinValue_##RESULT_TYPE && Value <= MTB_MaxValue_##RESULT_TYPE, "Value out of range."); \
  mtb_##RESULT_TYPE Result = (mtb_##RESULT_TYPE)Value; \
  return Result;

MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u08 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u16 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s08 mtb_SafeConvert_s08(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(s08); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_u16 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s16 mtb_SafeConvert_s16(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(s16); }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(s32); }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(s32); }
MTB_INLINE mtb_s32 mtb_SafeConvert_s32(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(s32); }
MTB_INLINE mtb_s64 mtb_SafeConvert_s64(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(s64); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s08 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u16 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u08 mtb_SafeConvert_u08(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u08); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s08 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u32 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u16 mtb_SafeConvert_u16(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u16); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s08 Value) { MTB__SAFE_CONVERT_BODY(u32); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(u32); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(u32); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(u32); }
MTB_INLINE mtb_u32 mtb_SafeConvert_u32(mtb_u64 Value) { MTB__SAFE_CONVERT_BODY(u32); }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s08 Value) { MTB__SAFE_CONVERT_BODY(u64); }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s16 Value) { MTB__SAFE_CONVERT_BODY(u64); }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s32 Value) { MTB__SAFE_CONVERT_BODY(u64); }
MTB_INLINE mtb_u64 mtb_SafeConvert_u64(mtb_s64 Value) { MTB__SAFE_CONVERT_BODY(u64); }


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

constexpr mtb_f32
mtb_Lerp(mtb_f32 A, mtb_f32 B, mtb_f32 Alpha)
{
  return (1.0f - Alpha) * A + Alpha * B;
}

constexpr mtb_f64
mtb_Lerp(mtb_f64 A, mtb_f64 B, mtb_f64 Alpha)
{
  return (1.0 - Alpha) * A + Alpha * B;
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
  return Result_u32;
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_u16, mtb_u16);
MTB_INLINE mtb_parse_string_result_u16
mtb_ParseString_u16(size_t SourceLen, char const* SourcePtr, mtb_u16 Fallback)
{
  mtb_parse_string_result_u64 Result_u64 = mtb_ParseString_u64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_u16 Result_u16{ Result_u64.Success, Result_u64.RemainingSourceLen, Result_u64.RemainingSourcePtr };
  Result_u16.Value = mtb_SafeConvert_u16(Result_u64.Value);
  return Result_u16;
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_u08, mtb_u08);
MTB_INLINE mtb_parse_string_result_u08
mtb_ParseString_u08(size_t SourceLen, char const* SourcePtr, mtb_u08 Fallback)
{
  mtb_parse_string_result_u64 Result_u64 = mtb_ParseString_u64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_u08 Result_u08{ Result_u64.Success, Result_u64.RemainingSourceLen, Result_u64.RemainingSourcePtr };
  Result_u08.Value = mtb_SafeConvert_u08(Result_u64.Value);
  return Result_u08;
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
  return Result_s32;
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_s16, mtb_s16);
MTB_INLINE mtb_parse_string_result_s16
mtb_ParseString_s16(size_t SourceLen, char const* SourcePtr, mtb_s16 Fallback)
{
  mtb_parse_string_result_s64 Result_s64 = mtb_ParseString_s64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_s16 Result_s16{ Result_s64.Success, Result_s64.RemainingSourceLen, Result_s64.RemainingSourcePtr };
  Result_s16.Value = mtb_SafeConvert_s16(Result_s64.Value);
  return Result_s16;
}


MTB_DEFINE_PARSE_STRING_RESULT(mtb_parse_string_result_s08, mtb_s08);
MTB_INLINE mtb_parse_string_result_s08
mtb_ParseString_s08(size_t SourceLen, char const* SourcePtr, mtb_s08 Fallback)
{
  mtb_parse_string_result_s64 Result_s64 = mtb_ParseString_s64(SourceLen, SourcePtr, Fallback);
  mtb_parse_string_result_s08 Result_s08{ Result_s64.Success, Result_s64.RemainingSourceLen, Result_s64.RemainingSourcePtr };
  Result_s08.Value = mtb_SafeConvert_s08(Result_s64.Value);
  return Result_s08;
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
      mtb_parse_string_result_u64 NumericResult = mtb_ParseString_u64(Len, Ptr, 0);
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
#if !defined(MTB_HEADER_mtb_color)
#define MTB_HEADER_mtb_color

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_COLOR_IMPLEMENTATION)
  #define MTB_COLOR_IMPLEMENTATION
#endif

#if defined(MTB_COLOR_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif



union mtb_linear_color
{
  struct
  {
    mtb_f32 R;
    mtb_f32 G;
    mtb_f32 B;
    mtb_f32 A;
  };

  mtb_f32 Data[4];
};

static_assert(sizeof(mtb_linear_color) == 16, "Incorrect size for mtb_linear_color.");

constexpr mtb_linear_color
mtb_LinearColor(mtb_f32 R, mtb_f32 G, mtb_f32 B, mtb_f32 A = 1.0f)
{
  return { R, G, B, A };
}

mtb_linear_color
mtb_LinearColorFromLinearHSV(mtb_f32 Hue, mtb_f32 Saturation, mtb_f32 Value);

mtb_linear_color
mtb_LinearColorFromGammaHSV(mtb_f32 Hue, mtb_f32 Saturation, mtb_f32 Value);

// http://en.wikipedia.org/wiki/Luminance_%28relative%29
constexpr mtb_f32
mtb_GetLuminance(mtb_linear_color Color)
{
  return 0.2126f * Color.R + 0.7152f * Color.G + 0.0722f * Color.B;
}

constexpr bool
mtb_IsColorNormalized(mtb_linear_color Color)
{
  return Color.R <= 1.0f && Color.G <= 1.0f && Color.B <= 1.0f && Color.A <= 1.0f &&
         Color.R >= 0.0f && Color.G >= 0.0f && Color.B >= 0.0f && Color.A >= 0.0f;
}

constexpr mtb_linear_color
mtb_GetInvertedColor(mtb_linear_color Color)
{
  return { 1.0f - Color.R, 1.0f - Color.G, 1.0f - Color.B, 1.0f - Color.A };
}

MTB_INLINE mtb_linear_color
mtb_GetSafeInvertedColor(mtb_linear_color Color)
{
  mtb_linear_color Result{};
  if(mtb_IsColorNormalized(Color))
  {
    Result = mtb_GetInvertedColor(Color);
  }
  return Result;
}

struct mtb_hsv
{
  mtb_f32 Hue;
  mtb_f32 Saturation;
  mtb_f32 Value;
};

mtb_hsv mtb_GetLinearHSV(mtb_linear_color Color);
mtb_hsv mtb_GetGammaHSV(mtb_linear_color Color);


union mtb_color
{
  struct
  {
    mtb_u08 R;
    mtb_u08 G;
    mtb_u08 B;
    mtb_u08 A;
  };

  mtb_u08 Data[4];
};

static_assert(sizeof(mtb_color) == 4, "Incorrect size for mtb_color.");

constexpr mtb_color
mtb_Color(mtb_u08 R, mtb_u08 G, mtb_u08 B, mtb_u08 A = 255)
{
  return { R, G, B, A };
}

MTB_INLINE mtb_f32
mtb_FromGammaToLinearColorSpace(mtb_f32 GammaValue)
{
  mtb_f32 LinearValue = GammaValue <= 0.04045f ? GammaValue / 12.92f : mtb_Pow((GammaValue + 0.055f) / 1.055f, 2.4f);
  return LinearValue;
}

MTB_INLINE mtb_f32
mtb_FromLinearToGammaColorSpace(mtb_f32 LinearValue)
{
  mtb_f32 GammaValue = LinearValue <= 0.0031308f ? 12.92f * LinearValue : 1.055f * mtb_Pow(LinearValue, 1.0f / 2.4f) - 0.055f;
  return GammaValue;
}

MTB_INLINE mtb_linear_color
mtb_ToLinearColor(mtb_color Color)
{
  mtb_linear_color Result {
    mtb_FromGammaToLinearColorSpace(mtb_Normalized_u08_To_f32(Color.R)),
    mtb_FromGammaToLinearColorSpace(mtb_Normalized_u08_To_f32(Color.G)),
    mtb_FromGammaToLinearColorSpace(mtb_Normalized_u08_To_f32(Color.B)),
    mtb_Normalized_u08_To_f32(Color.A)
  };
  return Result;
}

MTB_INLINE mtb_color
mtb_ToGammaColor(mtb_linear_color Color)
{
  mtb_color Result {
    mtb_Normalized_f32_To_u08(mtb_FromLinearToGammaColorSpace(Color.R)),
    mtb_Normalized_f32_To_u08(mtb_FromLinearToGammaColorSpace(Color.G)),
    mtb_Normalized_f32_To_u08(mtb_FromLinearToGammaColorSpace(Color.B)),
    mtb_Normalized_f32_To_u08(Color.A)
  };
  return Result;
}


//
// Pre-defined color values.
//

constexpr mtb_color mtb_Color_AliceBlue            = mtb_Color(0xF0, 0xF8, 0xFF);
constexpr mtb_color mtb_Color_AntiqueWhite         = mtb_Color(0xFA, 0xEB, 0xD7);
constexpr mtb_color mtb_Color_Aqua                 = mtb_Color(0x00, 0xFF, 0xFF);
constexpr mtb_color mtb_Color_Aquamarine           = mtb_Color(0x7F, 0xFF, 0xD4);
constexpr mtb_color mtb_Color_Azure                = mtb_Color(0xF0, 0xFF, 0xFF);
constexpr mtb_color mtb_Color_Beige                = mtb_Color(0xF5, 0xF5, 0xDC);
constexpr mtb_color mtb_Color_Bisque               = mtb_Color(0xFF, 0xE4, 0xC4);
constexpr mtb_color mtb_Color_Black                = mtb_Color(0x00, 0x00, 0x00);
constexpr mtb_color mtb_Color_BlanchedAlmond       = mtb_Color(0xFF, 0xEB, 0xCD);
constexpr mtb_color mtb_Color_Blue                 = mtb_Color(0x00, 0x00, 0xFF);
constexpr mtb_color mtb_Color_BlueViolet           = mtb_Color(0x8A, 0x2B, 0xE2);
constexpr mtb_color mtb_Color_Brown                = mtb_Color(0xA5, 0x2A, 0x2A);
constexpr mtb_color mtb_Color_BurlyWood            = mtb_Color(0xDE, 0xB8, 0x87);
constexpr mtb_color mtb_Color_CadetBlue            = mtb_Color(0x5F, 0x9E, 0xA0);
constexpr mtb_color mtb_Color_Chartreuse           = mtb_Color(0x7F, 0xFF, 0x00);
constexpr mtb_color mtb_Color_Chocolate            = mtb_Color(0xD2, 0x69, 0x1E);
constexpr mtb_color mtb_Color_Coral                = mtb_Color(0xFF, 0x7F, 0x50);
constexpr mtb_color mtb_Color_CornflowerBlue       = mtb_Color(0x64, 0x95, 0xED);
constexpr mtb_color mtb_Color_Cornsilk             = mtb_Color(0xFF, 0xF8, 0xDC);
constexpr mtb_color mtb_Color_Crimson              = mtb_Color(0xDC, 0x14, 0x3C);
constexpr mtb_color mtb_Color_Cyan                 = mtb_Color(0x00, 0xFF, 0xFF);
constexpr mtb_color mtb_Color_DarkBlue             = mtb_Color(0x00, 0x00, 0x8B);
constexpr mtb_color mtb_Color_DarkCyan             = mtb_Color(0x00, 0x8B, 0x8B);
constexpr mtb_color mtb_Color_DarkGoldenRod        = mtb_Color(0xB8, 0x86, 0x0B);
constexpr mtb_color mtb_Color_DarkGray             = mtb_Color(0xA9, 0xA9, 0xA9);
constexpr mtb_color mtb_Color_DarkGreen            = mtb_Color(0x00, 0x64, 0x00);
constexpr mtb_color mtb_Color_DarkKhaki            = mtb_Color(0xBD, 0xB7, 0x6B);
constexpr mtb_color mtb_Color_DarkMagenta          = mtb_Color(0x8B, 0x00, 0x8B);
constexpr mtb_color mtb_Color_DarkOliveGreen       = mtb_Color(0x55, 0x6B, 0x2F);
constexpr mtb_color mtb_Color_DarkOrange           = mtb_Color(0xFF, 0x8C, 0x00);
constexpr mtb_color mtb_Color_DarkOrchid           = mtb_Color(0x99, 0x32, 0xCC);
constexpr mtb_color mtb_Color_DarkRed              = mtb_Color(0x8B, 0x00, 0x00);
constexpr mtb_color mtb_Color_DarkSalmon           = mtb_Color(0xE9, 0x96, 0x7A);
constexpr mtb_color mtb_Color_DarkSeaGreen         = mtb_Color(0x8F, 0xBC, 0x8F);
constexpr mtb_color mtb_Color_DarkSlateBlue        = mtb_Color(0x48, 0x3D, 0x8B);
constexpr mtb_color mtb_Color_DarkSlateGray        = mtb_Color(0x2F, 0x4F, 0x4F);
constexpr mtb_color mtb_Color_DarkTurquoise        = mtb_Color(0x00, 0xCE, 0xD1);
constexpr mtb_color mtb_Color_DarkViolet           = mtb_Color(0x94, 0x00, 0xD3);
constexpr mtb_color mtb_Color_DeepPink             = mtb_Color(0xFF, 0x14, 0x93);
constexpr mtb_color mtb_Color_DeepSkyBlue          = mtb_Color(0x00, 0xBF, 0xFF);
constexpr mtb_color mtb_Color_DimGray              = mtb_Color(0x69, 0x69, 0x69);
constexpr mtb_color mtb_Color_DodgerBlue           = mtb_Color(0x1E, 0x90, 0xFF);
constexpr mtb_color mtb_Color_FireBrick            = mtb_Color(0xB2, 0x22, 0x22);
constexpr mtb_color mtb_Color_FloralWhite          = mtb_Color(0xFF, 0xFA, 0xF0);
constexpr mtb_color mtb_Color_ForestGreen          = mtb_Color(0x22, 0x8B, 0x22);
constexpr mtb_color mtb_Color_Fuchsia              = mtb_Color(0xFF, 0x00, 0xFF);
constexpr mtb_color mtb_Color_Gainsboro            = mtb_Color(0xDC, 0xDC, 0xDC);
constexpr mtb_color mtb_Color_GhostWhite           = mtb_Color(0xF8, 0xF8, 0xFF);
constexpr mtb_color mtb_Color_Gold                 = mtb_Color(0xFF, 0xD7, 0x00);
constexpr mtb_color mtb_Color_GoldenRod            = mtb_Color(0xDA, 0xA5, 0x20);
constexpr mtb_color mtb_Color_Gray                 = mtb_Color(0x80, 0x80, 0x80);
constexpr mtb_color mtb_Color_Green                = mtb_Color(0x00, 0x80, 0x00);
constexpr mtb_color mtb_Color_GreenYellow          = mtb_Color(0xAD, 0xFF, 0x2F);
constexpr mtb_color mtb_Color_HoneyDew             = mtb_Color(0xF0, 0xFF, 0xF0);
constexpr mtb_color mtb_Color_HotPink              = mtb_Color(0xFF, 0x69, 0xB4);
constexpr mtb_color mtb_Color_IndianRed            = mtb_Color(0xCD, 0x5C, 0x5C);
constexpr mtb_color mtb_Color_Indigo               = mtb_Color(0x4B, 0x00, 0x82);
constexpr mtb_color mtb_Color_Ivory                = mtb_Color(0xFF, 0xFF, 0xF0);
constexpr mtb_color mtb_Color_Khaki                = mtb_Color(0xF0, 0xE6, 0x8C);
constexpr mtb_color mtb_Color_Lavender             = mtb_Color(0xE6, 0xE6, 0xFA);
constexpr mtb_color mtb_Color_LavenderBlush        = mtb_Color(0xFF, 0xF0, 0xF5);
constexpr mtb_color mtb_Color_LawnGreen            = mtb_Color(0x7C, 0xFC, 0x00);
constexpr mtb_color mtb_Color_LemonChiffon         = mtb_Color(0xFF, 0xFA, 0xCD);
constexpr mtb_color mtb_Color_LightBlue            = mtb_Color(0xAD, 0xD8, 0xE6);
constexpr mtb_color mtb_Color_LightCoral           = mtb_Color(0xF0, 0x80, 0x80);
constexpr mtb_color mtb_Color_LightCyan            = mtb_Color(0xE0, 0xFF, 0xFF);
constexpr mtb_color mtb_Color_LightGoldenRodYellow = mtb_Color(0xFA, 0xFA, 0xD2);
constexpr mtb_color mtb_Color_LightGray            = mtb_Color(0xD3, 0xD3, 0xD3);
constexpr mtb_color mtb_Color_LightGreen           = mtb_Color(0x90, 0xEE, 0x90);
constexpr mtb_color mtb_Color_LightPink            = mtb_Color(0xFF, 0xB6, 0xC1);
constexpr mtb_color mtb_Color_LightSalmon          = mtb_Color(0xFF, 0xA0, 0x7A);
constexpr mtb_color mtb_Color_LightSeaGreen        = mtb_Color(0x20, 0xB2, 0xAA);
constexpr mtb_color mtb_Color_LightSkyBlue         = mtb_Color(0x87, 0xCE, 0xFA);
constexpr mtb_color mtb_Color_LightSlateGray       = mtb_Color(0x77, 0x88, 0x99);
constexpr mtb_color mtb_Color_LightSteelBlue       = mtb_Color(0xB0, 0xC4, 0xDE);
constexpr mtb_color mtb_Color_LightYellow          = mtb_Color(0xFF, 0xFF, 0xE0);
constexpr mtb_color mtb_Color_Lime                 = mtb_Color(0x00, 0xFF, 0x00);
constexpr mtb_color mtb_Color_LimeGreen            = mtb_Color(0x32, 0xCD, 0x32);
constexpr mtb_color mtb_Color_Linen                = mtb_Color(0xFA, 0xF0, 0xE6);
constexpr mtb_color mtb_Color_Magenta              = mtb_Color(0xFF, 0x00, 0xFF);
constexpr mtb_color mtb_Color_Maroon               = mtb_Color(0x80, 0x00, 0x00);
constexpr mtb_color mtb_Color_MediumAquaMarine     = mtb_Color(0x66, 0xCD, 0xAA);
constexpr mtb_color mtb_Color_MediumBlue           = mtb_Color(0x00, 0x00, 0xCD);
constexpr mtb_color mtb_Color_MediumOrchid         = mtb_Color(0xBA, 0x55, 0xD3);
constexpr mtb_color mtb_Color_MediumPurple         = mtb_Color(0x93, 0x70, 0xDB);
constexpr mtb_color mtb_Color_MediumSeaGreen       = mtb_Color(0x3C, 0xB3, 0x71);
constexpr mtb_color mtb_Color_MediumSlateBlue      = mtb_Color(0x7B, 0x68, 0xEE);
constexpr mtb_color mtb_Color_MediumSpringGreen    = mtb_Color(0x00, 0xFA, 0x9A);
constexpr mtb_color mtb_Color_MediumTurquoise      = mtb_Color(0x48, 0xD1, 0xCC);
constexpr mtb_color mtb_Color_MediumVioletRed      = mtb_Color(0xC7, 0x15, 0x85);
constexpr mtb_color mtb_Color_MidnightBlue         = mtb_Color(0x19, 0x19, 0x70);
constexpr mtb_color mtb_Color_MintCream            = mtb_Color(0xF5, 0xFF, 0xFA);
constexpr mtb_color mtb_Color_MistyRose            = mtb_Color(0xFF, 0xE4, 0xE1);
constexpr mtb_color mtb_Color_Moccasin             = mtb_Color(0xFF, 0xE4, 0xB5);
constexpr mtb_color mtb_Color_NavajoWhite          = mtb_Color(0xFF, 0xDE, 0xAD);
constexpr mtb_color mtb_Color_Navy                 = mtb_Color(0x00, 0x00, 0x80);
constexpr mtb_color mtb_Color_OldLace              = mtb_Color(0xFD, 0xF5, 0xE6);
constexpr mtb_color mtb_Color_Olive                = mtb_Color(0x80, 0x80, 0x00);
constexpr mtb_color mtb_Color_OliveDrab            = mtb_Color(0x6B, 0x8E, 0x23);
constexpr mtb_color mtb_Color_Orange               = mtb_Color(0xFF, 0xA5, 0x00);
constexpr mtb_color mtb_Color_OrangeRed            = mtb_Color(0xFF, 0x45, 0x00);
constexpr mtb_color mtb_Color_Orchid               = mtb_Color(0xDA, 0x70, 0xD6);
constexpr mtb_color mtb_Color_PaleGoldenRod        = mtb_Color(0xEE, 0xE8, 0xAA);
constexpr mtb_color mtb_Color_PaleGreen            = mtb_Color(0x98, 0xFB, 0x98);
constexpr mtb_color mtb_Color_PaleTurquoise        = mtb_Color(0xAF, 0xEE, 0xEE);
constexpr mtb_color mtb_Color_PaleVioletRed        = mtb_Color(0xDB, 0x70, 0x93);
constexpr mtb_color mtb_Color_PapayaWhip           = mtb_Color(0xFF, 0xEF, 0xD5);
constexpr mtb_color mtb_Color_PeachPuff            = mtb_Color(0xFF, 0xDA, 0xB9);
constexpr mtb_color mtb_Color_Peru                 = mtb_Color(0xCD, 0x85, 0x3F);
constexpr mtb_color mtb_Color_Pink                 = mtb_Color(0xFF, 0xC0, 0xCB);
constexpr mtb_color mtb_Color_Plum                 = mtb_Color(0xDD, 0xA0, 0xDD);
constexpr mtb_color mtb_Color_PowderBlue           = mtb_Color(0xB0, 0xE0, 0xE6);
constexpr mtb_color mtb_Color_Purple               = mtb_Color(0x80, 0x00, 0x80);
constexpr mtb_color mtb_Color_RebeccaPurple        = mtb_Color(0x66, 0x33, 0x99);
constexpr mtb_color mtb_Color_Red                  = mtb_Color(0xFF, 0x00, 0x00);
constexpr mtb_color mtb_Color_RosyBrown            = mtb_Color(0xBC, 0x8F, 0x8F);
constexpr mtb_color mtb_Color_RoyalBlue            = mtb_Color(0x41, 0x69, 0xE1);
constexpr mtb_color mtb_Color_SaddleBrown          = mtb_Color(0x8B, 0x45, 0x13);
constexpr mtb_color mtb_Color_Salmon               = mtb_Color(0xFA, 0x80, 0x72);
constexpr mtb_color mtb_Color_SandyBrown           = mtb_Color(0xF4, 0xA4, 0x60);
constexpr mtb_color mtb_Color_SeaGreen             = mtb_Color(0x2E, 0x8B, 0x57);
constexpr mtb_color mtb_Color_SeaShell             = mtb_Color(0xFF, 0xF5, 0xEE);
constexpr mtb_color mtb_Color_Sienna               = mtb_Color(0xA0, 0x52, 0x2D);
constexpr mtb_color mtb_Color_Silver               = mtb_Color(0xC0, 0xC0, 0xC0);
constexpr mtb_color mtb_Color_SkyBlue              = mtb_Color(0x87, 0xCE, 0xEB);
constexpr mtb_color mtb_Color_SlateBlue            = mtb_Color(0x6A, 0x5A, 0xCD);
constexpr mtb_color mtb_Color_SlateGray            = mtb_Color(0x70, 0x80, 0x90);
constexpr mtb_color mtb_Color_Snow                 = mtb_Color(0xFF, 0xFA, 0xFA);
constexpr mtb_color mtb_Color_SpringGreen          = mtb_Color(0x00, 0xFF, 0x7F);
constexpr mtb_color mtb_Color_SteelBlue            = mtb_Color(0x46, 0x82, 0xB4);
constexpr mtb_color mtb_Color_Tan                  = mtb_Color(0xD2, 0xB4, 0x8C);
constexpr mtb_color mtb_Color_Teal                 = mtb_Color(0x00, 0x80, 0x80);
constexpr mtb_color mtb_Color_Thistle              = mtb_Color(0xD8, 0xBF, 0xD8);
constexpr mtb_color mtb_Color_Tomato               = mtb_Color(0xFF, 0x63, 0x47);
constexpr mtb_color mtb_Color_Turquoise            = mtb_Color(0x40, 0xE0, 0xD0);
constexpr mtb_color mtb_Color_Violet               = mtb_Color(0xEE, 0x82, 0xEE);
constexpr mtb_color mtb_Color_Wheat                = mtb_Color(0xF5, 0xDE, 0xB3);
constexpr mtb_color mtb_Color_White                = mtb_Color(0xFF, 0xFF, 0xFF);
constexpr mtb_color mtb_Color_WhiteSmoke           = mtb_Color(0xF5, 0xF5, 0xF5);
constexpr mtb_color mtb_Color_Yellow               = mtb_Color(0xFF, 0xFF, 0x00);
constexpr mtb_color mtb_Color_YellowGreen          = mtb_Color(0x9A, 0xCD, 0x32);

#endif // !defined(MTB_HEADER_mtb_color)


// ==============
// Implementation
// ==============

#if defined(MTB_COLOR_IMPLEMENTATION)
#if !defined(MTB_IMPL_mtb_color)
#define MTB_IMPL_mtb_color

mtb_linear_color
mtb_LinearColorFromLinearHSV(mtb_f32 Hue, mtb_f32 Saturation, mtb_f32 Value)
{
  // http://www.rapidtables.com/convert/color/hsv-to-rgb.htm

  mtb_linear_color Result{};

  if((Hue >= 0 || Hue <= 360) &&(Saturation >= 0 || Saturation <= 1) && (Value >= 0 || Value <= 1))
  {
    mtb_f32 const C = Saturation * Value;
    mtb_f32 const X = C * (1.0f - mtb_Abs(mtb_Mod(Hue / 60.0f, 2) - 1.0f));
    mtb_f32 const M = Value - C;

    Result.A = 1.0f;

    if (Hue < 60)
    {
      Result.R = C + M;
      Result.G = X + M;
      Result.B = 0 + M;
    }
    else if (Hue < 120)
    {
      Result.R = X + M;
      Result.G = C + M;
      Result.B = 0 + M;
    }
    else if (Hue < 180)
    {
      Result.R = 0 + M;
      Result.G = C + M;
      Result.B = X + M;
    }
    else if (Hue < 240)
    {
      Result.R = 0 + M;
      Result.G = X + M;
      Result.B = C + M;
    }
    else if (Hue < 300)
    {
      Result.R = X + M;
      Result.G = 0 + M;
      Result.B = C + M;
    }
    else
    {
      Result.R = C + M;
      Result.G = 0 + M;
      Result.B = X + M;
    }
  }
  else
  {
    // TODO: Values are out of range. Diagnostics?
  }


  return Result;
}

mtb_linear_color
mtb_LinearColorFromGammaHSV(mtb_f32 Hue, mtb_f32 Saturation, mtb_f32 Value)
{
  mtb_linear_color LinearColorWithGammaValues = mtb_LinearColorFromLinearHSV(Hue, Saturation, Value);

  mtb_linear_color Result;
  Result.R = mtb_FromGammaToLinearColorSpace(LinearColorWithGammaValues.R);
  Result.G = mtb_FromGammaToLinearColorSpace(LinearColorWithGammaValues.G);
  Result.B = mtb_FromGammaToLinearColorSpace(LinearColorWithGammaValues.B);
  Result.A = LinearColorWithGammaValues.A;

  return Result;
}

mtb_hsv
mtb_GetLinearHSV(mtb_linear_color Color)
{
  // http://en.literateprograms.org/RGB_to_HSV_color_space_conversion_%28C%29

  mtb_hsv Result{};
  Result.Value = mtb_Max(Color.R, mtb_Max(Color.G, Color.B));

  if(Result.Value != 0)
  {
    mtb_f32 const InvValue = 1.0f / Result.Value;
    mtb_f32 Norm_R = Color.R * InvValue;
    mtb_f32 Norm_G = Color.G * InvValue;
    mtb_f32 Norm_B = Color.B * InvValue;
    mtb_f32 const RGB_Min = mtb_Min(Norm_R, mtb_Min(Norm_G, Norm_B));
    mtb_f32 const RGB_Max = mtb_Max(Norm_R, mtb_Max(Norm_G, Norm_B));

    Result.Saturation = RGB_Max - RGB_Min;

    if(Result.Saturation != 0)
    {
      // Normalize saturation
      mtb_f32 const RGB_Delta_Inverse = 1.0f / Result.Saturation;
      Norm_R = (Norm_R - RGB_Min) * RGB_Delta_Inverse;
      Norm_G = (Norm_G - RGB_Min) * RGB_Delta_Inverse;
      Norm_B = (Norm_B - RGB_Min) * RGB_Delta_Inverse;

      // Hue
      if(RGB_Max == Norm_R)
      {
        Result.Hue = 60.0f * (Norm_G - Norm_B);

        if(Result.Hue < 0)
          Result.Hue += 360.0f;
      }
      else if(RGB_Max == Norm_G)
      {
        Result.Hue = 120.0f + 60.0f * (Norm_B - Norm_R);
      }
      else
      {
        Result.Hue = 240.0f + 60.0f * (Norm_R - Norm_G);
      }
    }
  }

  return Result;
}

mtb_hsv
mtb_GetGammaHSV(mtb_linear_color Color)
{
  mtb_f32 GammaR = mtb_FromLinearToGammaColorSpace(Color.R);
  mtb_f32 GammaG = mtb_FromLinearToGammaColorSpace(Color.G);
  mtb_f32 GammaB = mtb_FromLinearToGammaColorSpace(Color.B);

  mtb_linear_color LinearColorWithGammaValues = mtb_LinearColor(GammaR, GammaG, GammaB, Color.A);
  mtb_hsv Result = mtb_GetLinearHSV(LinearColorWithGammaValues);
  return Result;
}


#endif // !defined(MTB_IMPL_mtb_color)
#endif // defined(MTB_COLOR_IMPLEMENTATION)

/*
 * Random number generation is modeled after the minimal implementation
 * of one member of the PCG family of random number
 * generators by Melissa O'Neill <oneill@pcg-random.org>
 * and adjusted to my codebase (mtb).
 * Hosted at: https://github.com/imneme/pcg-c-basic
 * See below for details.
 */

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 */

/*
 * The general interface for random number generation is heavily inspired by
 * Handmade Hero by Casey Muratori.
 */

#if !defined(MTB_HEADER_mtb_rng)
#define MTB_HEADER_mtb_rng

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_RNG_IMPLEMENTATION)
  #define MTB_RNG_IMPLEMENTATION
#endif

#if defined(MTB_RNG_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


struct mtb_rng
{
  mtb_u64 State;
  mtb_u64 Inc;
};

/// Create a random number generator object with the given seed.
mtb_rng
mtb_RandomSeed(mtb_u64 InitialState, mtb_u64 StreamID = 1);

///
/// Random unsigned 32-bit integer values.
///

/// Generate a uniformly distributed 32-bit random number.
mtb_u32
mtb_Random_u32(mtb_rng* RNG);

/// Generate a uniformly distributed 32-bit random number, Result, where 0 <= Result < Bound.
mtb_u32
mtb_RandomBelow_u32(mtb_rng* RNG, mtb_u32 Bound);

/// Generate a uniformly distributed 32-bit random number, Result, where LowerBound <= Result <= UpperBound.
mtb_u32
mtb_RandomBetween_u32(mtb_rng* RNG, mtb_u32 LowerBound, mtb_u32 UpperBound);

///
/// Random 32-bit floating-point values.
///

/// Generate a uniformly distributed 32-bit random floating point number, Result, where 0 <= Result <= 1.
mtb_f32
mtb_RandomUnilateral_f32(mtb_rng* RNG);

/// Generate a uniformly distributed 32-bit random floating point number, Result, where -1 <= Result <= 1.
mtb_f32
mtb_RandomBilateral_f32(mtb_rng* RNG);

/// Generate a uniformly distributed 32-bit random number, Result, where LowerBound <= Result < UpperBound.
mtb_f32
mtb_RandomBetween_f32(mtb_rng* RNG, mtb_f32 LowerBound, mtb_f32 UpperBound);

#if !defined(MTB_FLAG_ORIGINAL_BOUNDED_PCG)
  #define MTB_FLAG_ORIGINAL_BOUNDED_PCG MTB_OFF
#endif

#endif // !defined(MTB_HEADER_mtb_rng)


// ==============
// Implementation
// ==============

#if defined(MTB_RNG_IMPLEMENTATION)
#if !defined(MTB_IMPL_mtb_rng)
#define MTB_IMPL_mtb_rng

mtb_rng
mtb_RandomSeed(mtb_u64 InitialState, mtb_u64 StreamID)
{
  mtb_rng Result{};
  Result.Inc = (StreamID << 1u) | 1u;
  (void)mtb_Random_u32(&Result);
  Result.State += InitialState;
  (void)mtb_Random_u32(&Result);
  return Result;
}

mtb_u32
mtb_Random_u32(mtb_rng* RNG)
{
  mtb_u64 OldState = RNG->State;
  RNG->State = OldState * 6364136223846793005ULL + RNG->Inc;
  mtb_u32 XOrShifted = (mtb_u32)(((OldState >> 18u) ^ OldState) >> 27u);
  mtb_u32 Rot = (mtb_u32)(OldState >> 59u);
  mtb_u32 Result = (XOrShifted >> Rot) | (XOrShifted << (((mtb_u32)(-(mtb_s32)Rot)) & 31));
  return Result;
}

mtb_u32
mtb_RandomBelow_u32(mtb_rng* RNG, mtb_u32 Bound)
{
  mtb_u32 Result = 0;

  if(Bound > 0)
  {
#if MTB_FLAG(ORIGINAL_BOUNDED_PCG)
    // NOTE(Manuzor): Even though it says that the original implementation
    // should usually be fast, I'm kind of hung up on that "usually" part. I
    // think I'm willing to sacrifice a little performance for deterministic
    // program behavior in this case. By default, at least.

    // To avoid bias, we need to make the range of the RNG a multiple of
    // Bound, which we do by dropping output less than a Threshold.
    // A naive scheme to calculate the Threshold would be to do
    //
    //     mtb_u32 Threshold = 0x100000000ull % Bound;
    //
    // but 64-bit div/mod is slower than 32-bit div/mod (especially on
    // 32-bit platforms).  In essence, we do
    //
    //     mtb_u32 Threshold = (0x100000000ull-Bound) % Bound;
    //
    // because this version will calculate the same modulus, but the LHS
    // value is less than 2^32.
    mtb_u32 Threshold = (mtb_u32)(-(mtb_s32)Bound) % Bound;

    // Uniformity guarantees that this loop will terminate.  In practice, it
    // should usually terminate quickly; on average (assuming all bounds are
    // equally likely), 82.25% of the time, we can expect it to require just
    // one iteration.  In the worst case, someone passes a Bound of 2^31 + 1
    // (i.e., 2147483649), which invalidates almost 50% of the range.  In
    // practice, bounds are typically small and only a tiny amount of the range
    // is eliminated.
    while(true)
    {
      Result = mtb_Random_u32(RNG);
      if(Result >= Threshold)
        break;
    }

    Result %= Bound;
#else
    Result = mtb_Random_u32(RNG) % Bound;
#endif
  }

  return Result;
}

mtb_u32
mtb_RandomBetween_u32(mtb_rng* RNG, mtb_u32 LowerBound, mtb_u32 UpperBound)
{
  mtb_u32 Result = 0;
  if(UpperBound > LowerBound)
  {
    mtb_u32 Rand = mtb_Random_u32(RNG);
    mtb_u32 LocalBound = (UpperBound + 1) - LowerBound;
    Result = LowerBound + (Rand % LocalBound);
  }
  return Result;
}

mtb_f32
mtb_RandomUnilateral_f32(mtb_rng* RNG)
{
  mtb_f32 Divisor = 1.0f / (mtb_f32)MTB_MaxValue_u32;
  mtb_f32 Result = Divisor * (mtb_f32)mtb_Random_u32(RNG);
  return Result;
}

mtb_f32
mtb_RandomBilateral_f32(mtb_rng* RNG)
{
  mtb_f32 Result = 2.0f * mtb_RandomUnilateral_f32(RNG) - 1.0f;
  return Result;
}

mtb_f32
mtb_RandomBetween_f32(mtb_rng* RNG, mtb_f32 LowerBound, mtb_f32 UpperBound)
{
  mtb_f32 Alpha = mtb_RandomUnilateral_f32(RNG);
  mtb_f32 Result = mtb_Lerp(LowerBound, UpperBound, Alpha);
  return Result;
}

#endif // !defined(MTB_IMPL_mtb_rng)
#endif // defined(MTB_RNG_IMPLEMENTATION)
