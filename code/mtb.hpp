#if !defined(MTB_HEADER_mtb_platform)
#define MTB_HEADER_mtb_platform

// MTB_On and MTB_Off are used to ensure an option was defined before it was
// used. MTB_IsOn is used with something undefined

//
// Usage:
//   #define MY_SETTING MTB_On
//   #if MTB_IsOn(MY_SETTING)
//     /* Do stuff */
//   #endif
#define MTB_On  +
#define MTB_Off -
#define MTB_IsOn(x)  ((1 x 1) == 2)
#define MTB_IsOff(x) (!MTB_IsOn(x))


#if defined(__cplusplus)
  #define MTB_Cpp MTB_On
#else
  #define MTB_Cpp MTB_Off
#endif

// NOTE: Only c++ enabled for now.
#if MTB_IsOff(MTB_Cpp)
  #error MTB requires C++ (for now).
#endif


//
// Determine the compiler
// MTB_Compiler_*
//

// Usage:
//   #if MTB_IsCompiler(MSVC)
//   /* ... */
//   #endif
#define MTB_IsCompiler(WHAT) MTB_IsOn(MTB_Compiler_##WHAT)

#define MTB_Compiler_MSVC MTB_Off
#define MTB_Compiler_GCC MTB_Off
#define MTB_Compiler_Clang MTB_Off

#if defined(_MSC_VER)
  #undef  MTB_Compiler_MSVC
  #define MTB_Compiler_MSVC MTB_On
#else
  #error "Unknown compiler."
#endif


//
// Determine the operating system
// MTB_OS_*
//

#define MTB_OS_Windows MTB_Off

#if defined(_WIN32) || defined(_WIN64)
  #undef  MTB_OS_Windows
  #define MTB_OS_Windows MTB_On
#else
  // TODO: Emit some kind of warning instead?
  #error "Unknown operating system."
#endif

#define MTB_IsOS(WHAT) MTB_IsOn(MTB_OS_##WHAT)


//
// Determine the architecture and bitness
// MTB_Arch_*
//

// Usage:
// #if MTB_IsArch(x86)
// /* ... */
// #endif
#define MTB_IsArch(WHAT) MTB_IsOn(MTB_Arch_##WHAT)

#define MTB_Arch_x86    MTB_Off /* 32 bit version of the Intel x86 architecture. */
#define MTB_Arch_x86_64 MTB_Off /* 64 bit version of the Intel x86 architecture. */

#if defined(_M_X64) || defined(_M_AMD64)
  #undef  MTB_Arch_x86_64
  #define MTB_Arch_x86_64 MTB_On
#elif defined(_M_IX86) || defined(_M_I86)
  #undef  MTB_Arch_x86
  #define MTB_Arch_x86 MTB_On
#else
  #error "Unknown architecture."
#endif

#define MTB_32BitArch MTB_IsOff
#define MTB_64BitArch MTB_IsOff
#define MTB_LittleEndian MTB_IsOff
#define MTB_BigEndian MTB_IsOff

#if MTB_IsArch(x86_64)
  #undef  MTB_64BitArch
  #define MTB_64BitArch MTB_On
  #undef  MTB_LittleEndian
  #define MTB_LittleEndian MTB_On
#elif MTB_IsArch(x86)
  #undef  MTB_32BitArch
  #define MTB_32BitArch MTB_On
  #undef  MTB_LittleEndian
  #define MTB_LittleEndian MTB_On
#else
  #error "Undefined architecture."
#endif


//
// Utility stuff
//

#define MTB_File __FILE__
#define MTB_Line __LINE__

#if defined(__PRETTY_FUNCTION__)
  #define MTB_Function __PRETTY_FUNCTION__
#elif defined(__func__)
  #define MTB_Function __func__
#elif defined(__FUNCTION__)
  #define MTB_Function __FUNCTION__
#else
  #define MTB_Function ""
#endif

#define MTB_Impl_ConcatLevel2(LEFT, RIGHT) LEFT##RIGHT
#define MTB_Impl_ConcatLevel1(LEFT, RIGHT) MTB_Impl_ConcatLevel2(LEFT, RIGHT)
#define MTB_Concat(LEFT, RIGHT) MTB_Impl_ConcatLevel1(LEFT, RIGHT)

#define MTB_Impl_Stringify(WHAT) #WHAT
#define MTB_Stringify(WHAT) MTB_Impl_Stringify(WHAT)

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
//   MTB_Inline void func() { /* ... */ }
#if !defined(MTB_Inline)
  #if MTB_IsCompiler(MSVC)
    #define MTB_Inline __forceinline
  #else
    // TODO: Emit a warning here?
    // Fallback to `inline`
    #define MTB_Inline inline
  #endif
#endif


// Try to automatically determine a debug build if the user did not specify anything.
#if !defined(MTB_DebugBuild)
  #if defined(_DEBUG)
    #define MTB_DebugBuild MTB_On
  #else
    #define MTB_DebugBuild MTB_Off
  #endif
#endif

// Try to automatically determine a release build if the user did not specify anything.
#if !defined(MTB_ReleaseBuild)
  #if defined(NDEBUG)
    #define MTB_ReleaseBuild MTB_On
  #else
    #define MTB_ReleaseBuild MTB_Off
  #endif
#endif

// Try to determine a development build if the user did not specify anything.
#if !defined(MTB_DevBuild)
  #if defined(MTB_ReleaseBuild)
    #define MTB_DevBuild MTB_Off
  #else
    #define MTB_DevBuild MTB_On
  #endif
#endif

#if !defined(MTB_Internal)
  #if defined(MTB_ReleaseBuild)
    #define MTB_Internal MTB_Off
  #else
    #define MTB_Internal MTB_On
  #endif
#endif

// Macro to enclose code that is only compiled in in debug builds.
//
// Usage:
//   MTB_DebugCode(auto result = ) someFunctionCall();
//   MTB_DebugCode(if(result == 0))
#if !defined(MTB_DebugCode)
  #if MTB_IsOn(MTB_DebugBuild)
    #define MTB_DebugCode(...) __VA_ARGS__
  #else
    #define MTB_DebugCode(...)
  #endif
#endif

#if !defined(MTB_DebugBreak)
  #if MTB_IsCompiler(MSVC)
    #define MTB_DebugBreak do { __debugbreak(); } while(0)
  #else
    #define MTB_DebugBreak MTB_NOP
  #endif
#endif

#if !defined(MTB_Exceptions)
  #define MTB_Exceptions MTB_Off
#endif

// For STL
// NOTE: Don't undef to inform the user that we're overwriting their settings
// if they specified it.
#if MTB_IsOn(MTB_Exceptions)
  #define _HAS_EXCEPTIONS 1
#else
  #define _HAS_EXCEPTIONS 0
#endif


//
// Primitive types.
//
namespace mtb
{
  // NOTE: These are the only types in MTB that are not prefixed.
  using i8  = char;
  using i16 = short;
  using i32 = int;
  using i64 = long long;

  using u8  = unsigned char;
  using u16 = unsigned short;
  using u32 = unsigned int;
  using u64 = unsigned long long;

  using f32 = float;
  using f64 = double;

  using bool32 = i32;

  /// Generic types if no specific size/precision is required.
  using uint = unsigned int;
} // namespace mtb

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
// mtb_platform.hpp
// ==========================================


namespace mtb
{
//
// Primitive array types.
//

/// Defines some array variants of types for better readability when used as
/// function parameters.
///
/// For example, a function `Foo` that accepts and array of 4 floats by
/// reference-to-const looks like this:
/// \code
/// void Foo(float const (&ParamName)[4]);
/// \endcode
///
/// Using these typedefs, this can be transformed into:
/// \code
/// void Foo(float_4 const& ParamName);
/// \endcode
#define MTB_DefineArrayTypes(TheType)\
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

MTB_DefineArrayTypes(i8);
MTB_DefineArrayTypes(i16);
MTB_DefineArrayTypes(i32);
MTB_DefineArrayTypes(i64);

MTB_DefineArrayTypes(u8);
MTB_DefineArrayTypes(u16);
MTB_DefineArrayTypes(u32);
MTB_DefineArrayTypes(u64);

MTB_DefineArrayTypes(f32);
MTB_DefineArrayTypes(f64);


//
// ============================
//

constexpr u64 KiB(u64 Amount) { return { Amount * 1024 }; }
constexpr u64 MiB(u64 Amount) { return { Amount * 1024 * 1024 }; }
constexpr u64 GiB(u64 Amount) { return { Amount * 1024 * 1024 * 1024 }; }
constexpr u64 TiB(u64 Amount) { return { Amount * 1024 * 1024 * 1024 * 1024 }; }

constexpr u64 KB(u64 Amount) { return { Amount * 1000 }; }
constexpr u64 MB(u64 Amount) { return { Amount * 1000 * 1000 }; }
constexpr u64 GB(u64 Amount) { return { Amount * 1000 * 1000 * 1000 }; }
constexpr u64 TB(u64 Amount) { return { Amount * 1000 * 1000 * 1000 * 1000 }; }

template<typename OutputType = float> constexpr OutputType ToKiB(u64 Size) { return (OutputType)((double)Size / 1024); }
template<typename OutputType = float> constexpr OutputType ToMiB(u64 Size) { return (OutputType)((double)Size / 1024 / 1024); }
template<typename OutputType = float> constexpr OutputType ToGiB(u64 Size) { return (OutputType)((double)Size / 1024 / 1024 / 1024); }
template<typename OutputType = float> constexpr OutputType ToTiB(u64 Size) { return (OutputType)((double)Size / 1024 / 1024 / 1024 / 1024); }

template<typename OutputType = float> constexpr OutputType ToKB(u64 Size) { return (OutputType)((double)Size / 1000); }
template<typename OutputType = float> constexpr OutputType ToMB(u64 Size) { return (OutputType)((double)Size / 1000 / 1000); }
template<typename OutputType = float> constexpr OutputType ToGB(u64 Size) { return (OutputType)((double)Size / 1000 / 1000 / 1000); }
template<typename OutputType = float> constexpr OutputType ToTB(u64 Size) { return (OutputType)((double)Size / 1000 / 1000 / 1000 / 1000); }



//
// ============================
//
constexpr u32  SetBit(u32 Bits, u32 Position)   { return Bits | (u32(1) << Position); }
constexpr u32  UnsetBit(u32 Bits, u32 Position) { return Bits & ~(u32(1) << Position); }
constexpr bool IsBitSet(u32 Bits, u32 Position) { return !!(Bits & (u32(1) << Position)); }

constexpr u64  SetBit(u64 Bits, u64 Position)   { return Bits | (u64(1) << Position); }
constexpr u64  UnsetBit(u64 Bits, u64 Position) { return Bits & ~(u64(1) << Position); }
constexpr bool IsBitSet(u64 Bits, u64 Position) { return !!(Bits & (u64(1) << Position)); }

constexpr bool IsPowerOfTwo(u32 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }
constexpr bool IsPowerOfTwo(u64 Value) { return Value < 1 ? false : (Value & (Value - 1)) == 0; }


//
// ============================
//
template<typename T = float>
constexpr T
Pi() { return (T)3.14159265359; }

template<typename T = float>
constexpr T
E() { return (T)2.71828182845; }


//
// ================
//
template<typename T> struct impl_size_of { enum { SizeInBytes = sizeof(T) }; };
template<>           struct impl_size_of<void>          : impl_size_of<u8>          { };
template<>           struct impl_size_of<void const>    : impl_size_of<u8 const>    { };
template<>           struct impl_size_of<void volatile> : impl_size_of<u8 volatile> { };

/// Get the size of type T in bytes.
///
/// Same as sizeof(T) except it works also with 'void' (possibly cv-qualified)
/// where a size of 1 byte is assumed.
template<typename T>
constexpr size_t
SizeOf() { return (size_t)impl_size_of<T>::SizeInBytes; }

/// Returns the number of elements in this static array.
template<typename T, size_t N>
constexpr size_t
LengthOf(T(&)[N]) { return N; }

/// Same as \c LengthOf but force to 32 bit value.
template<typename T, size_t N>
constexpr u32
LengthOf32(T(&)[N]) { return (u32)N; }

/// Returns the number of elements in this static array.
template<typename T, size_t N>
constexpr size_t
ByteLengthOf(T(&)[N]) { return N * SizeOf<T>(); }

/// Same as \c LengthOf but force to 32 bit value.
template<typename T, size_t N>
constexpr u32
ByteLengthOf32(T(&)[N]) { return (u32)N * SizeOf<T>(); }

/// Reinterpretation of the given pointer in case PointerType is `void`.
template<typename PointerType>
constexpr PointerType*
NonVoidPtr(PointerType* Ptr)
{
  return Ptr;
}

/// Reinterpretation of the given pointer in case PointerType is `void`.
constexpr u8*
NonVoidPtr(void* Ptr)
{
  return reinterpret_cast<u8*>(Ptr);
}

/// Reinterpretation of the given pointer in case PointerType is `void`.
constexpr u8 const*
NonVoidPtr(void const* Ptr)
{
  return reinterpret_cast<u8 const*>(Ptr);
}

/// Reinterpretation of the given pointer in case PointerType is `void`.
constexpr u8 volatile*
NonVoidPtr(void volatile* Ptr)
{
  return reinterpret_cast<u8 volatile*>(Ptr);
}

/// Advance the given pointer value by the given amount of bytes.
template<typename PointerType, typename OffsetType>
constexpr PointerType*
AddByteOffset(PointerType* Pointer, OffsetType Offset)
{
  return (PointerType*)((u8*)Pointer + Offset);
}

/// Advance the given pointer value by the given amount times sizeof(PointerType)
template<typename PointerType, typename OffsetType>
constexpr PointerType*
AddElementOffset(PointerType* Pointer, OffsetType Offset)
{
  return AddByteOffset(Pointer, Offset * SizeOf<PointerType>());
}

template<typename T> struct impl_is_pod
{
  #if MTB_IsCompiler(MSVC)
    static constexpr bool Value = __is_pod(T);
  #else
    #error "IsPOD not implemented on this platform."
  #endif
};
template<> struct impl_is_pod<void>          : public impl_is_pod<u8>          {};
template<> struct impl_is_pod<void const>    : public impl_is_pod<u8 const>    {};
template<> struct impl_is_pod<void volatile> : public impl_is_pod<u8 volatile> {};

/// Whether the given type T is a "plain old data" (POD) type.
///
/// The type 'void' is also considered POD.
template<typename T>
constexpr bool
IsPOD() { return impl_is_pod<T>::Value; }


template<typename NumberType> struct impl_negate { static constexpr NumberType Do(NumberType Value) { return -Value; } };
template<> struct impl_negate<u8>  { static constexpr u8  Do(u8  Value) { return  Value; } };
template<> struct impl_negate<u16> { static constexpr u16 Do(u16 Value) { return  Value; } };
template<> struct impl_negate<u32> { static constexpr u32 Do(u32 Value) { return  Value; } };
template<> struct impl_negate<u64> { static constexpr u64 Do(u64 Value) { return  Value; } };

template<typename NumberType>
NumberType
Negate(NumberType Value)
{
  return impl_negate<NumberType>::Do(Value);
}

template<typename T>
struct impl_is_integer_type { static bool const Value = false; };
template<> struct impl_is_integer_type<i8>  { static bool const Value = true; };
template<> struct impl_is_integer_type<i16> { static bool const Value = true; };
template<> struct impl_is_integer_type<i32> { static bool const Value = true; };
template<> struct impl_is_integer_type<i64> { static bool const Value = true; };
template<> struct impl_is_integer_type<u8>  { static bool const Value = true; };
template<> struct impl_is_integer_type<u16> { static bool const Value = true; };
template<> struct impl_is_integer_type<u32> { static bool const Value = true; };
template<> struct impl_is_integer_type<u64> { static bool const Value = true; };

template<typename T>
constexpr bool
IsIntegerType() { return impl_is_integer_type<T>::Value; }

template<typename T>
struct impl_is_float_type { static bool const Value = false; };
template<> struct impl_is_float_type<float>  { static bool const Value = true; };
template<> struct impl_is_float_type<double> { static bool const Value = true; };

template<typename T>
constexpr bool
IsFloatType() { return impl_is_float_type<T>::Value; }

template<typename T>
constexpr bool
IsNumberType() { return IsFloatType<T>() || IsIntegerType<T>(); }

/// Get the number of bits of a given type.
///
/// Note: The type 'void' is not supported.
template<typename T>
constexpr size_t
NumBits() { return sizeof(T) * 8; }

template<typename T>
constexpr bool
IntIsSigned() { return ((T)-1) < 0; }

template<typename T>
constexpr T
IntMaxValue()
{
  return IntIsSigned<T>() ? (T(1) << (NumBits<T>() - 1)) - T(1)
                              : T(-1);
}

template<typename T>
constexpr T
IntMinValue()
{
  return IntIsSigned<T>() ? Negate(T(T(1) << (NumBits<T>() - 1)))
                              : T(0);
}

template<typename CharType> struct impl_is_digit_helper { static constexpr bool Do(CharType Char) { return Char >= '0' && Char <= '9'; } };
template<typename CharType> struct impl_is_digit;
template<> struct impl_is_digit<char> : public impl_is_digit_helper<char> {};

template<typename CharType>
constexpr bool
IsDigit(CharType Char)
{
  return impl_is_digit<rm_ref_const<CharType>>::Do(Char);
}

template<typename CharType>
struct impl_is_whitespace_helper
{
  static constexpr bool
  Do(CharType Char)
  {
    return Char == ' '  ||
           Char == '\n' ||
           Char == '\r' ||
           Char == '\t' ||
           Char == '\b';
  }
};

template<typename CharType> struct impl_is_whitespace;
template<> struct impl_is_whitespace<char> : public impl_is_whitespace_helper<char> {};

template<typename CharType>
constexpr bool
IsWhitespace(CharType Char)
{
  return impl_is_whitespace<rm_ref_const<CharType>>::Do(Char);
}

template<typename T> struct impl_nan;
template<> struct impl_nan<float>
{
  #if MTB_IsCompiler(MSVC)
    static constexpr float Quiet()     { return __builtin_nanf("0"); }
    static constexpr float Signaling() { return __builtin_nansf("1"); }
  #else
    #error "NaN not implemented on this platform."
  #endif
};
template<typename T> struct impl_nan;
template<> struct impl_nan<double>
{
  #if MTB_IsCompiler(MSVC)
    static constexpr double Quiet()     { return __builtin_nan("0"); }
    static constexpr double Signaling() { return __builtin_nans("1"); }
  #else
    #error "NaN not implemented on this platform."
  #endif
};

/// Returns a quiet Not-A-Number value of the given type.
template<typename T>
constexpr T
NaN()
{
  return impl_nan<T>::Quiet();
}

/// Returns a signaling Not-A-Number value of the given type.
template<typename T>
constexpr T
SignalingNaN()
{
  return impl_nan<T>::Signaling();
}

template<typename T> struct impl_is_nan;
template<> struct impl_is_nan<float>  { static constexpr bool Do(float  Value) { return Value != Value; } };
template<> struct impl_is_nan<double> { static constexpr bool Do(double Value) { return Value != Value; } };

template<typename T>
constexpr bool
IsNaN(T Value)
{
  return impl_is_nan<T>::Do(Value);
}

template<typename T> struct impl_rm_ref     { using Type = T; };
template<typename T> struct impl_rm_ref<T&> { using Type = T; };

template<typename T>
using rm_ref = typename impl_rm_ref<T>::Type;

template<typename T> struct impl_rm_const          { using Type = T; };
template<typename T> struct impl_rm_const<T const> { using Type = T; };

template<typename T>
using rm_const = typename impl_rm_const<T>::Type;

template<typename T>
using rm_ref_const = rm_const<rm_ref<T>>;

template<class T>
constexpr typename rm_ref<T>&&
Move(T&& Argument)
{
  // forward Argument as movable
  return static_cast<typename rm_ref<T>&&>(Argument);
}

template<typename T>
constexpr T&&
Forward(typename rm_ref<T>& Argument)
{
  return static_cast<T&&>(Argument);
}

template<typename T>
constexpr T&&
Forward(rm_ref<T>&& Argument)
{
  return static_cast<T&&>(Argument);
}

template<typename DestType, typename SourceType>
constexpr DestType
Cast(SourceType Value)
{
  return static_cast<DestType>(Value);
}

template<typename DestType, typename SourceType>
constexpr DestType
Reinterpret(SourceType Value)
{
  return reinterpret_cast<DestType>(Value);
}

/// Coerce value of some type to another.
///
/// Basically just a more explicit C-style cast.
template<typename DestType, typename SourceType>
DestType
Coerce(SourceType Value)
{
  DestType Result = (DestType)Value;
  return Result;
}

template<typename T>
T const&
AsConst(T& Value)
{
  return const_cast<T const&>(Value);
}

template<typename T>
T const*
AsPtrToConst(T* Value)
{
  return const_cast<T const*>(Value);
}

template<typename DestT, typename SourceT>
struct impl_convert
{
  static constexpr DestT
  Do(SourceT const& Value)
  {
    return Cast<DestT>(Value);
  }
};

template<typename DestT, typename SourceT, typename... ExtraTypes>
decltype(auto)
Convert(SourceT const& ToConvert, ExtraTypes&&... Extra)
{
  using UnqualifiedDestT   = rm_ref_const<DestT>;
  using UnqualifiedSourceT = rm_ref_const<SourceT>;
  using Impl = impl_convert<UnqualifiedDestT, UnqualifiedSourceT>;
  return Impl::Do(ToConvert, Forward<ExtraTypes>(Extra)...);
}

/// Asserts on overflows and underflows when converting signed or unsigned
/// integers.
///
/// TODO: Implement what is documented below with Convert.
///
/// In case of error in non-asserting builds, this will return the
/// corresponding min / max value, instead of letting the overflow / underflow
/// happen.
template<typename DestIntegerType, typename SrcIntegerType>
inline DestIntegerType
SafeConvertInt(SrcIntegerType Value)
{
  auto Result = (DestIntegerType)Value;
  auto RevertedResult = (SrcIntegerType)Result;
  Assert(RevertedResult == Value); // Otherwise something went wrong in the conversion step (overflow/underflow).
  return Result;
}

/// \return 1 for a positive number, -1 for a negative number, 0 otherwise.
template<typename T>
constexpr T
Sign(T I)
{
  return T(I > 0 ? 1 : I < 0 ? -1 : 0);
}

template<typename T> struct impl_abs { static constexpr T Do(T Value) { return Sign(Value) * Value; } };
template<> struct impl_abs<u8>  { static constexpr u8  Do(u8  Value) { return Value; } };
template<> struct impl_abs<u16> { static constexpr u16 Do(u16 Value) { return Value; } };
template<> struct impl_abs<u32> { static constexpr u32 Do(u32 Value) { return Value; } };
template<> struct impl_abs<u64> { static constexpr u64 Do(u64 Value) { return Value; } };
template<> struct impl_abs<i8>  { static inline i8  Do(i8  Value) { return Value < 0 ? -Value : Value; } };
template<> struct impl_abs<i16> { static inline i16 Do(i16 Value) { return Value < 0 ? -Value : Value; } };
template<> struct impl_abs<i32> { static inline i32 Do(i32 Value) { return Value < 0 ? -Value : Value; } };
template<> struct impl_abs<i64> { static inline i64 Do(i64 Value) { return Value < 0 ? -Value : Value; } };

template<typename T>
constexpr T
Abs(T Value)
{
  return impl_abs<T>::Do(Value);
}

template<typename TypeA, typename TypeB>
constexpr TypeA
Min(TypeA A, TypeB B)
{
  return (B < A) ? Coerce<TypeA>(B) : A;
}

template<typename TypeA, typename TypeB>
constexpr TypeA
Max(TypeA A, TypeB B)
{
  return (B > A) ? Coerce<TypeA>(B) : A;
}

template<typename ValueT, typename LowerBoundT, typename UpperBoundT>
constexpr ValueT
Clamp(ValueT Value, LowerBoundT LowerBound, UpperBoundT UpperBound)
{
  return UpperBound < LowerBound ? Value : Min(UpperBound, Max(LowerBound, Value));
}

// TODO: Make this a constexpr?
template<typename ValueT, typename LowerBoundT, typename UpperBoundT>
ValueT
Wrap(ValueT Value, LowerBoundT LowerBound, UpperBoundT UpperBound)
{
  const auto BoundsDelta = (Coerce<LowerBoundT>(UpperBound) - LowerBound);
  while(Value >= UpperBound) Value -= BoundsDelta;
  while(Value < LowerBound)  Value += BoundsDelta;
  return Value;
  // return Value >= UpperBound ? Value - BoundsDelta :
  //        Value <  LowerBound ? Value + BoundsDelta :
  //                              Value;
}

double
Pow(double Base, double Exponent);

float
Pow(float Base, float Exponent);

template<typename ReturnType = double, typename BaseType, typename ExponentType>
constexpr ReturnType
Pow(BaseType Base, ExponentType Exponent) { return (ReturnType)Pow((double)Base, (double)Exponent); }

double
Mod(double Value, double Divisor);

float
Mod(float Value, float Divisor);

double
Sqrt(double Value);

float
Sqrt(float Value);

template<typename ReturnType = double, typename T>
constexpr ReturnType
Sqrt(T Value) { return (ReturnType)Sqrt((double)Value); }

float
InvSqrt(float Value);

//
// RoundDown
//
template<typename OutputType, typename InputType>
struct impl_round_down;

template<typename OutputType>
struct impl_round_down<OutputType, float>
{
  static inline OutputType
  Do(float Value)
  {
    return Convert<OutputType>(::floorf(Value));
  }
};

template<typename OutputType>
struct impl_round_down<OutputType, double>
{
  static inline OutputType
  Do(double Value)
  {
    return Convert<OutputType>(::floor(Value));
  }
};

template<typename OutputType, typename InputType>
struct impl_round_down : public impl_round_down<OutputType, double> {};

/// Also known as the floor-function.
template<typename OutputType, typename InputType>
inline OutputType
RoundDown(InputType Value)
{
  return impl_round_down<OutputType, InputType>::Do(Value);
}

//
// RoundUp
//
template<typename OutputType, typename InputType>
struct impl_round_up;

template<typename OutputType>
struct impl_round_up<OutputType, float>
{
  static inline OutputType
  Do(float Value)
  {
    return Convert<OutputType>(::ceilf(Value));
  }
};

template<typename OutputType>
struct impl_round_up<OutputType, double>
{
  static inline OutputType
  Do(double Value)
  {
    return Convert<OutputType>(::ceil(Value));
  }
};

template<typename OutputType, typename InputType>
struct impl_round_up : public impl_round_up<OutputType, double> {};

/// Also known as the ceil-function.
template<typename OutputType, typename InputType>
inline OutputType
RoundUp(InputType Value)
{
  return impl_round_up<OutputType, InputType>::Do(Value);
}


//
// RoundTowardsZero
//
template<typename OutputType, typename InputType> struct impl_round_towards_zero
{
  static inline OutputType
  Do(InputType Value)
  {
    return Value > 0 ? RoundDown<OutputType>(Value) : RoundUp<OutputType>(Value);
  }
};

/// Round towards zero.
///
/// Equivalent to \code Value > 0 ? RoundDown(Value) : RoundUp(Value) \endcode
template<typename OutputType, typename InputType>
inline OutputType
RoundTowardsZero(InputType Value)
{
  return impl_round_towards_zero<OutputType, InputType>::Do(Value);
}


//
// RoundAwayFromZero
//
template<typename OutputType, typename InputType> struct impl_round_away_from_zero
{
  static inline OutputType
  Do(InputType Value)
  {
    return Value > 0 ? RoundUp<OutputType>(Value) : RoundDown<OutputType>(Value);
  }
};

/// Round away from zero.
///
/// Equivalent to \code Value > 0 ? RoundUp(Value) : RoundDown(Value) \endcode
template<typename OutputType, typename InputType>
inline OutputType
RoundAwayFromZero(InputType Value)
{
  return impl_round_away_from_zero<OutputType, InputType>::Do(Value);
}


//
// Round
//
template<typename OutputType, typename InputType> struct impl_round
{
  static inline OutputType
  Do(InputType Value)
  {
    return RoundDown<OutputType>(Value + InputType(0.5f));
  }
};

/// Round to the nearest integral value.
template<typename OutputType, typename InputType>
inline OutputType
Round(InputType Value)
{
  return impl_round<OutputType, InputType>::Do(Value);
}


// Project a value from [LowerBound, UpperBound] to [0, 1]
// Example:
//   auto Result = NormalizeValue<float>(15, 10, 30); // == 0.25f
template<typename ResultT, typename ValueT, typename LowerBoundT, typename UpperBoundT>
constexpr ResultT
NormalizeValue(ValueT Value, LowerBoundT LowerBound, UpperBoundT UpperBound)
{
  return UpperBound <= LowerBound ?
         ResultT(0) : // Bogus bounds.
         Cast<ResultT>(Value - LowerBound) / Cast<ResultT>(UpperBound - LowerBound);
}

bool
AreNearlyEqual(double A, double B, double Epsilon = 1e-4);

bool
AreNearlyEqual(float A, float B, float Epsilon = 1e-4f);

inline bool
IsNearlyZero(double A, double Epsilon = 1e-4) { return AreNearlyEqual(A, 0, Epsilon); }

inline bool
IsNearlyZero(float A, float Epsilon = 1e-4f) { return AreNearlyEqual(A, 0, Epsilon); }

template<typename TypeA, typename TypeB>
inline void
Swap(TypeA& A, TypeB& B)
{
  auto Temp{ Move(A) };
  A = Move(B);
  B = Move(Temp);
}

template<typename T, typename U = T>
inline T
Exchange(T& Value, U&& NewValue)
{
  auto OldValue{ Move(Value) };
  Value = Forward<T>(NewValue);
  return OldValue;
}

/// Maps the given float Value from [0, 1] to [0, MaxValueOf(UNormType)]
template<typename UNormType>
UNormType constexpr
FloatToUNorm(float Value)
{
  return Cast<UNormType>(Clamp((Value * IntMaxValue<UNormType>()) + 0.5f, 0.0f, IntMaxValue<UNormType>()));
}

/// Maps the given unsigned byte Value from [0, 255] to [0, 1]
template<typename UNormType>
float constexpr
UNormToFloat(UNormType Value)
{
  return Clamp(Cast<float>(Value) / IntMaxValue<UNormType>(), 0.0f, 1.0f);
}

/// \see InitStruct
template<typename T>
struct impl_init_struct
{
  // Return an initialized instance of T.
  template<typename... ArgTypes>
  static constexpr T
  Create(ArgTypes&&... Args) { return { Forward<ArgTypes>(Args)... }; }
};

/// Utility function to initialize a struct of the given type with a chance
/// for centralized specialization.
///
/// To control the default or non-default construction behavior of a certain
/// struct the template \c impl_init_struct can be specialized and a Create()
/// function must be provided.
///
/// \see impl_init_struct
template<typename T, typename... ArgTypes>
inline auto
InitStruct(ArgTypes&&... Args)
  -> decltype(impl_init_struct<rm_ref<T>>::Create(Forward<ArgTypes>(Args)...))
{
  // Note: specializations for impl_init_struct are found in VulkanHelper.inl
  return impl_init_struct<rm_ref<T>>::Create(Forward<ArgTypes>(Args)...);
}

struct impl_defer
{
  template<typename LambdaType>
  struct defer
  {
    LambdaType Lambda;
    defer(LambdaType InLambda) : Lambda{ Move(InLambda) } {}
    ~defer() { Lambda(); }
  };

  template<typename FuncT>
  defer<FuncT> operator =(FuncT InLambda) { return { Move(InLambda) }; }
};

/// Defers execution of code until the end of the current scope.
///
/// Usage:
///   int i = 0;
///   MTB_Defer [&](){ i++; printf("Foo %d\n", i); };
///   MTB_Defer [&](){ i++; printf("Bar %d\n", i); };
///   MTB_Defer [=](){      printf("Baz %d\n", i); };
///
/// Output:
///   Baz 0
///   Bar 1
///   Foo 2
///
/// \param CaptureSpec The lambda capture specification.
#define MTB_Defer auto MTB_Concat(_Defer, MTB_Line) = ::mtb::impl_defer() =

} // namespace mtb

#endif // !defined(MTB_HEADER_mtb_common)


//
// Implementation
//

#if defined(MTB_COMMON_IMPLEMENTATION)

// Impl guard.
#if !defined(MTB_IMPL_mtb_common)
#define MTB_IMPL_mtb_common

#include <math.h>

auto mtb::
Pow(float Base, float Exponent)
  -> float
{
  return powf(Base, Exponent);
}

auto mtb::
Pow(double Base, double Exponent)
  -> double
{
  return pow(Base, Exponent);
}

auto mtb::
Mod(float Value, float Divisor)
  -> float
{
  return fmodf(Value, Divisor);
}

auto mtb::
Mod(double Value, double Divisor)
  -> double
{
  return fmod(Value, Divisor);
}

auto mtb::
Sqrt(float Value)
  -> float
{
  return sqrtf(Value);
}

auto mtb::
Sqrt(double Value)
  -> double
{
  return sqrt(Value);
}

auto mtb::
InvSqrt(float Value)
  -> float
{
  union FloatInt
  {
    float Float;
    int Int;
  };
  FloatInt MagicNumber;
  float HalfValue;
  float Result;
  const float ThreeHalfs = 1.5f;

  HalfValue = Value * 0.5f;
  Result = Value;
  MagicNumber.Float = Result;                               // evil floating point bit level hacking
  MagicNumber.Int  = 0x5f3759df - ( MagicNumber.Int >> 1 ); // what the fuck?
  Result = MagicNumber.Float;
  Result = Result * ( ThreeHalfs - ( HalfValue * Result * Result ) ); // 1st iteration

  return Result;
}

auto mtb::
AreNearlyEqual(double A, double B, double Epsilon)
  -> bool
{
  return Abs(A - B) <= Epsilon;
}

auto mtb::
AreNearlyEqual(float A, float B, float Epsilon)
  -> bool
{
  return Abs(A - B) <= Epsilon;
}

#endif // !defined(MTB_IMPL_mtb_common)
#endif // defined(MTB_COMMON_IMPLEMENTATION)
#if !defined(MTB_HEADER_mtb_assert)
#define MTB_HEADER_mtb_assert

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_ASSERT_IMPLEMENTATION)
  #define MTB_ASSERT_IMPLEMENTATION
#endif

#if defined(MTB_ASSERT_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


// ==========================================
// mtb_common.hpp
// ==========================================


namespace mtb
{
/// Returning \c true means that the a debug break is required.
using assert_handler = bool(*)(
  char const* FileName,     //< The name of the file where the assert is.
  size_t      Line,         //< The line number in the file where the assert is.
  char const* FunctionName, //< The name of the function where the assert is.
  char const* Expression,   //< The condition as captured by the MTB_Assert macro. May be nullptr in case of MTB_ReportError.
  char const* Message);     //< The user defined message.

assert_handler GetAssertHandler();
void SetAssertHandler(assert_handler AssertHandler);

bool OnFailedCheck(
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
        if(::mtb::OnFailedCheck(MTB_File, MTB_Line, MTB_Function, #CONDITION, __VA_ARGS__)) \
        { \
          MTB_DebugBreak; \
        } \
      } \
    } while(0)
#endif


//
// Find sensible defaults if the user did not set anything.
//
#if !defined(MTB_WantBoundsCheck)
  #define MTB_WantBoundsCheck MTB_On
#endif

#if !defined(MTB_WantDebugAssert)
  #if MTB_IsOn(MTB_DebugBuild)
    #define MTB_WantDebugAssert MTB_On
  #else
    #define MTB_WantDebugAssert MTB_Off
  #endif
#endif

#if !defined(MTB_WantDevAssert)
  #if MTB_IsOn(MTB_DebugBuild) || MTB_IsOn(MTB_DevBuild)
    #define MTB_WantDevAssert MTB_On
  #else
    #define MTB_WantDevAssert MTB_Off
  #endif
#endif

#if !defined(MTB_WantReleaseAssert)
  #if MTB_IsOn(MTB_DebugBuild) || MTB_IsOn(MTB_DevBuild) || MTB_IsOn(MTB_ReleaseBuild)
    #define MTB_WantReleaseAssert MTB_On
  #else
    #define MTB_WantReleaseAssert MTB_Off
  #endif
#endif


//
// Define the actual assert macros
//
#if MTB_IsOn(MTB_WantBoundsCheck)
  #define MTB_BoundsCheck MTB_Require
#endif

#if MTB_IsOn(MTB_WantDebugAssert)
  #define MTB_DebugAssert MTB_Require
#endif

#if MTB_IsOn(MTB_WantDevAssert)
  #define MTB_DevAssert MTB_Require
#endif

#if MTB_IsOn(MTB_WantReleaseAssert)
  #define MTB_ReleaseAssert MTB_Require
#endif

//
// Error reporting macro to mark an invalid code path.
//
#if !defined(MTB_ReportError)
  #define MTB_ReportError(...) \
    do \
    { \
      if(::mtb::OnFailedCheck(MTB_File, MTB_Line, MTB_Function, nullptr, __VA_ARGS__)) \
      { \
        MTB_DebugBreak; \
      } \
    } while(0)
#endif

#if !defined(MTB_NotImplemented)
  #define MTB_NotImplemented MTB_ReportError("Not implemented.")
#endif

#if !defined(MTB_InvalidCodePath)
  #define MTB_InvalidCodePath MTB_ReportError("Invalid code path.")
#endif


//
// Make sure all assert macros are at least defined.
//
#if !defined(MTB_DebugAssert)
  #define MTB_DebugAssert(...) MTB_NOP
#endif

#if !defined(MTB_DevAssert)
  #define MTB_DevAssert(...) MTB_NOP
#endif

#if !defined(MTB_ReleaseAssert)
  #define MTB_ReleaseAssert(...) MTB_NOP
#endif

} // namespace mtb
#endif // !defined(MTB_HEADER_mtb_assert)


// Implementation
//
//
#if defined(MTB_ASSERT_IMPLEMENTATION)

#if !defined(MTB_IMPL_mtb_assert)
#define MTB_IMPL_mtb_assert


static bool DefaultAssertHandler(
  char const* FileName,
  size_t      Line,
  char const* FunctionName,
  char const* Expression,
  char const* Message)
{
  return true;
}

static mtb::assert_handler GlobalAssertHandler = &DefaultAssertHandler;

mtb::assert_handler mtb::
GetAssertHandler()
{
  return GlobalAssertHandler;
}

void mtb::
SetAssertHandler(assert_handler AssertHandler)
{
  GlobalAssertHandler = AssertHandler;
}

bool mtb::
OnFailedCheck(
  char const* FileName,
  size_t      Line,
  char const* FunctionName,
  char const* Expression,
  char const* MessageFormat,
  ...)
{
  if(!GlobalAssertHandler)
  {
    // If there's no assert handler, always trigger a debug break.
    return true;
  }

  // TODO: Format message.
  // NOTE: Ensure message is a valid string, so at least let it be empty.
  char const* Message = MessageFormat ? MessageFormat : "";

  return (*GlobalAssertHandler)(FileName, Line, FunctionName, Expression, Message);
}


#endif // !defined(MTB_IMPL_mtb_assert)
#endif // defined(MTB_ASSERT_IMPLEMENTATION)

#if !defined(MTB_HEADER_mtb_memory)
#define MTB_HEADER_mtb_memory

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_MEMORY_IMPLEMENTATION)
  #define MTB_MEMORY_IMPLEMENTATION
#endif

#if defined(MTB_MEMORY_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


// ==========================================
// mtb_assert.hpp
// ==========================================


#include <new>

namespace mtb
{
/// \defgroup Memory manipulation functions
///
/// Provides functions to work on chunks of memory.
///
/// Unlike C standard functions such as memcpy and memset, these functions
/// respect the type of the input objects. Refer to the table below to find
/// which C standard functionality is covered by which of the functions
/// defined here.
///
/// C Standard Function | Untyped/Bytes                | Typed
/// ------------------- | ---------------------------- | -----
/// memcopy, memmove    | CopyBytes                    | CopyElements, CopyConstructElements, MoveElements, MoveConstructElements
/// memset              | SetBytes                     | SetElements, ConstructElements
/// memcmp              | CompareBytes, AreBytesEqual  | -
///
///
/// All functions are optimized for POD types.
///
/// @{

/// Copy Size from Source to Destination.
///
/// Destination and Source may overlap.
void
CopyBytes(size_t Size, void* Destination, void const* Source);

/// Fill NumBytes in Destination with the value
void
SetBytes(size_t Size, void* Destination, int Value);

bool
AreBytesEqual(size_t Size, void const* A, void const* B);

int
CompareBytes(size_t Size, void const* A, void const* B);


/// Calls the constructor of all elements in Destination with Args.
///
/// Args may be empty in which case all elements get default-initialized.
template<typename T, typename... ArgTypes>
void
ConstructElements(size_t Num, T* Destination, ArgTypes&&... Args);

/// Destructs all elements in Destination.
template<typename T>
void
DestructElements(size_t Num, T* Destination);

/// Copy all elements from Source to Destination.
///
/// Destination and Source may overlap.
template<typename T>
void
CopyElements(size_t Num, T* Destination, T const* Source);

/// Copy all elements from Source to Destination using T's constructor.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
CopyConstructElements(size_t Num, T* Destination, T const* Source);

/// Move all elements from Source to Destination using T's constructor.
///
/// Destination and Source may overlap.
template<typename T>
void
MoveElements(size_t Num, T* Destination, T* Source);

/// Move all elements from Source to Destination using T's constructor and destruct Source afterwards.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
MoveConstructElements(size_t Num, T* Destination, T* Source);

/// Assign the default value of T to all elements in Destination.
template<typename T>
void
SetElementsToDefault(size_t Num, T* Destination);

/// Assign Item to all elements in Destination.
template<typename T>
void
SetElements(size_t Num, T* Destination, T const& Item);

bool
ImplTestMemoryOverlap(size_t SizeA, void const* A, size_t SizeB, void const* B);

template<typename TA, typename TB>
bool
TestMemoryOverlap(size_t NumA, TA const* A, size_t NumB, TB const* B)
{
  return ImplTestMemoryOverlap(NumA * SizeOf<TA>(), Reinterpret<void const*>(A),
                               NumB * SizeOf<TB>(), Reinterpret<void const*>(B));
}


//
// Implementation Details
//

// ConstructElements

template<typename T, bool TIsPlainOldData = false>
struct impl_construct_elements
{
  template<typename... ArgTypes>
  MTB_Inline static void
  Do(size_t Num, T* Destination, ArgTypes&&... Args)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Forward<ArgTypes>(Args)...);
    }
  }
};

template<typename T>
struct impl_construct_elements<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination)
  {
    SetBytes(Num * SizeOf<T>(), Destination, 0);
  }

  MTB_Inline static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    // Blit Item over each element of Destination.
    for(size_t Index = 0; Index < Num; ++Index)
    {
      CopyBytes(SizeOf<T>(), &Destination[Index], &Item);
    }
  }
};

template<typename T, typename... ArgTypes>
MTB_Inline auto
ConstructElements(size_t Num, T* Destination, ArgTypes&&... Args)
  -> void
{
  impl_construct_elements<T, IsPOD<T>()>::Do(Num, Destination, Forward<ArgTypes>(Args)...);
}


// DestructElements

template<typename T, bool TIsPlainOldData = false>
struct impl_destruct_elements
{
  MTB_Inline static void
  Do(size_t Num, T* Destination)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index].~T();
    }
  }
};

template<typename T>
struct impl_destruct_elements<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination)
  {
    // Nothing to do for POD types.
  }
};

template<typename T>
MTB_Inline auto
DestructElements(size_t Num, T* Destination)
  -> void
{
  impl_destruct_elements<T, IsPOD<T>()>::Do(Num, Destination);
}


// CopyElements

template<typename T, bool TIsPlainOldData = false>
struct impl_copy_elements
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    if(Destination == Source)
      return;

    if(TestMemoryOverlap(Num, Destination, Num, Source) && Destination < Source)
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
struct impl_copy_elements<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    CopyBytes(SizeOf<T>() * Num, Destination, Source);
  }
};

template<typename T>
MTB_Inline auto
CopyElements(size_t Num, T* Destination, T const* Source)
  -> void
{
  impl_copy_elements<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// CopyConstructElements

template<typename T, bool TIsPlainOldData = false>
struct impl_copy_construct_elements
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_DebugAssert(!TestMemoryOverlap(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Source[Index]);
    }
  }
};

template<typename T>
struct impl_copy_construct_elements<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_DebugAssert(!TestMemoryOverlap(Num, Destination, Num, Source));

    CopyElements(Num, Destination, Source);
  }
};

template<typename T>
MTB_Inline auto
CopyConstructElements(size_t Num, T* Destination, T const* Source)
  -> void
{
  impl_copy_construct_elements<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// MoveElements

template<typename T, bool TIsPlainOldData = false>
struct impl_move_elements
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T* Source)
  {
    if(Destination == Source)
      return;

    if(TestMemoryOverlap(Num, Destination, Num, Source))
    {
      if(Destination < Source)
      {
        // Move forward
        for(size_t Index = 0; Index < Num; ++Index)
        {
          Destination[Index] = Move(Source[Index]);
        }

        // Destroy the remaining elements in the back.
        size_t const NumToDestruct = Source - Destination;
        DestructElements(NumToDestruct, AddElementOffset(Source, Num - NumToDestruct));
      }
      else
      {
        // Move backward
        for(size_t Index = Num; Index > 0;)
        {
          --Index;
          Destination[Index] = Move(Source[Index]);
        }

        // Destroy the remaining elements in the front.
        size_t const NumToDestruct = Destination - Source;
        DestructElements(NumToDestruct, Source);
      }
    }
    else
    {
      // Straight forward: Move one by one, then destruct all in Source.
      for(size_t Index = 0; Index < Num; ++Index)
      {
        Destination[Index] = Move(Source[Index]);
      }
      DestructElements(Num, Source);
    }
  }
};

template<typename T>
struct impl_move_elements<T, true> : public impl_copy_elements<T, true> {};

template<typename T>
MTB_Inline auto
MoveElements(size_t Num, T* Destination, T* Source)
  -> void
{
  impl_move_elements<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// MoveConstructElements

template<typename T, bool TIsPlainOldData = false>
struct impl_move_construct_elements
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_DebugAssert(!TestMemoryOverlap(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Move(Source[Index]));
    }
    DestructElements(Num, Source);
  }
};

template<typename T>
struct impl_move_construct_elements<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_DebugAssert(!TestMemoryOverlap(Num, Destination, Num, Source));

    CopyElements(Num, Destination, Source);
  }
};

template<typename T>
MTB_Inline auto
MoveConstructElements(size_t Num, T* Destination, T* Source)
  -> void
{
  impl_move_construct_elements<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// SetElements

template<typename T, bool TIsPlainOldData = false>
struct impl_set_elements
{
  MTB_Inline static void
  Do(size_t Num, T* Destination)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index] = {};
    }
  }

  MTB_Inline static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    for(size_t Index = 0; Index < Num; ++Index)
    {
      Destination[Index] = Item;
    }
  }
};

template<typename T>
struct impl_set_elements<T, true> : public impl_construct_elements<T, true> {};

template<typename T>
MTB_Inline auto
SetElements(size_t Num, T* Destination)
  -> void
{
  impl_set_elements<T, IsPOD<T>()>::Do(Num, Destination);
}

template<typename T>
MTB_Inline auto
SetElements(size_t Num, T* Destination, T const& Item)
  -> void
{
  impl_set_elements<T, IsPOD<T>()>::Do(Num, Destination, Item);
}

} // namespace mtb
#endif // !defined(MTB_HEADER_mtb_memory)

#if defined(MTB_MEMORY_IMPLEMENTATION)

#if !defined(MTB_IMPL_mtb_memory)
#define MTB_IMPL_mtb_memory

#include <cstring>

auto mtb::
CopyBytes(size_t Size, void* Destination, void const* Source)
  -> void
{
  // Using memmove so that Destination and Source may overlap.
  std::memmove(Destination, Source, Size);
}

auto mtb::
SetBytes(size_t Size, void* Destination, int Value)
  -> void
{
  std::memset(Destination, Value, Size);
}

auto mtb::
AreBytesEqual(size_t Size, void const* A, void const* B)
  -> bool
{
  return CompareBytes(Size, A, B) == 0;
}

auto mtb::
CompareBytes(size_t Size, void const* A, void const* B)
  -> int
{
  return std::memcmp(A, B, Size);
}

auto mtb::
ImplTestMemoryOverlap(size_t SizeA, void const* A, size_t SizeB, void const* B)
  -> bool
{
  auto LeftA = Reinterpret<size_t const>(A);
  auto RightA = LeftA + SizeA;

  auto LeftB = Reinterpret<size_t const>(B);
  auto RightB = LeftB + SizeB;

  return LeftB  >= LeftA && LeftB  <= RightA || // Check if LeftB  is in [A, A+SizeA]
         RightB >= LeftA && RightB <= RightA || // Check if RightB is in [A, A+SizeA]
         LeftA  >= LeftB && LeftA  <= RightB || // Check if LeftA  is in [B, B+SizeB]
         RightA >= LeftB && RightA <= RightB;   // Check if RightA is in [B, B+SizeB]
}

#endif // !defiend(MTB_IMPL_mtb_memory)
#endif // defined(MTB_MEMORY_IMPLEMENTATION)
#if !defined(MTB_HEADER_mtb_slice)
#define MTB_HEADER_mtb_slice

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_SLICE_IMPLEMENTATION)
  #define MTB_SLICE_IMPLEMENTATION
#endif

#if defined(MTB_SLICE_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


// ==========================================
// mtb_memory.hpp
// ==========================================


namespace mtb
{

constexpr size_t INVALID_INDEX = (size_t)-1;

template<typename T>
struct slice
{
  using ElementType = T;

  size_t Len;
  ElementType* Ptr;

  /// Test whether this slice is valid or not.
  ///
  /// A slice is considered valid if it does not point to null and contains at
  /// least one element. If `Len` is 0 or `Ptr` is `nullptr`, the slice is
  /// considered invalid (`false`).
  inline operator bool() const { return Len && Ptr; }

  /// Implicit conversion to const version.
  inline operator slice<T const>() const { return { Len, Ptr }; }

  /// Index operator to access elements of the slice.
  template<typename IndexType>
  inline auto
  operator[](IndexType Index) const
    -> decltype(Ptr[Index])
  {
    MTB_BoundsCheck(Index >= 0 && Index < Len);
    return Ptr[Index];
  }
};

template<>
struct slice<void const>
{
  using ElementType = void const;

  size_t Len;
  void const* Ptr;

  /// Test whether this slice is valid or not.
  ///
  /// A slice is considered valid if it does not point to null and contains at
  /// least one element.
  inline operator bool() const { return Len && Ptr; }
};

template<>
struct slice<void>
{
  using ElementType = void;

  size_t Len;
  void* Ptr;

  /// Test whether this slice is valid or not.
  ///
  /// A slice is considered valid if it does not point to null and contains at
  /// least one element.
  inline operator bool() const { return Len && Ptr; }

  /// Implicit conversion to const version.
  inline operator slice<void const>() const { return { Len, Ptr }; }
};

template<typename T>
size_t
LengthOf(slice<T> const& A)
{
  return A.Len;
}

template<typename T>
size_t
SetLength(slice<T>& A, size_t NewLength)
{
  return Exchange(A.Len, NewLength);
}

template<typename T>
typename slice<T>::ElementType*
First(slice<T> A)
{
  return A.Ptr;
}

template<typename T>
typename slice<T>::ElementType*
Last(slice<T> A)
{
  return AddElementOffset(First(A), Max(size_t(1), LengthOf(A)) - 1);
}

template<typename T>
typename slice<T>::ElementType*
OnePastLast(slice<T> A)
{
  return AddElementOffset(First(A), LengthOf(A));
}

/// C++11 range API
template<typename T>
typename slice<T>::ElementType*
begin(slice<T> A)
{
  return First(A);
}

/// C++11 range API
template<typename T>
typename slice<T>::ElementType*
end(slice<T> A)
{
  return OnePastLast(A);
}

template<typename TargetType, typename SourceType>
slice<TargetType>
Reinterpret(slice<SourceType> A)
{
  return Slice(Reinterpret<TargetType*>(First(A)),
               Reinterpret<TargetType*>(OnePastLast(A)));
}

template<typename SourceType>
slice<SourceType const>
AsConst(slice<SourceType> A)
{
  return Slice(AsPtrToConst(First(A)),
               AsPtrToConst(OnePastLast(A)));
}

/// Concatenate two slices together.
///
/// \return The returned slice will be a subset of the given Buffer, which is
/// \used to write the actual result in.
template<typename ElementType>
slice<ElementType>
Concat(slice<ElementType const> Head, slice<ElementType const> Tail, slice<ElementType> Buffer)
{
  MTB_BoundsCheck(LengthOf(Buffer) >= LengthOf(Head) + LengthOf(Tail));
  size_t DestIndex = 0;
  for(auto Element : Head)
  {
    Buffer[DestIndex++] = Element;
  }
  for(auto Element : Tail)
  {
    Buffer[DestIndex++] = Element;
  }

  // DestIndex must now be the combined count of Head and Tail.
  MTB_ReleaseAssert(DestIndex == LengthOf(Head) + LengthOf(Tail));
  auto Result = Slice(Buffer, 0, DestIndex);
  return Result;
}

/// Create a union of both input spans. The resulting slice will contain everything
template<typename ElementType>
constexpr slice<ElementType>
Union(slice<ElementType> SliceA, slice<ElementType> SliceB)
{
  // A union only makes sense when both slices are overlapping.
  return { Min(First(SliceA), First(SliceB)), Max(OnePastLast(SliceA), OnePastLast(SliceB)) };
}

template<typename ElementTypeA, typename ElementTypeB>
constexpr bool
AreDisjoint(slice<ElementTypeA> SliceA, slice<ElementTypeB> SliceB)
{
  return Last(SliceA) < First(SliceB) || First(SliceA) > Last(SliceB);
}

/// Whether SliceA and SliceB overlap.
/// \see Contains
template<typename ElementTypeA, typename ElementTypeB>
bool
AreOverlapping(slice<ElementTypeA> SliceA, slice<ElementTypeB> SliceB)
{
  auto UnionOfAB = Union(SliceA, SliceB);
  return Contains(UnionOfAB, SliceA) || Contains(UnionOfAB, SliceA);
}

/// Whether SliceA completely contains SliceB.
/// \see AreOverlapping
template<typename ElementTypeA, typename ElementTypeB>
constexpr bool
Contains(slice<ElementTypeA> SliceA, slice<ElementTypeB> SliceB)
{
  return First(SliceA) <= First(SliceB) && OnePastLast(SliceA) >= OnePastLast(SliceB);
}

template<typename ElementType>
constexpr slice<ElementType>
Slice(size_t Len, ElementType* Ptr)
{
  return { Len, Ptr };
}


/// FirstPtr and OnePastLastPtr must define valid slice boundaries for the
/// given type.
///
/// Here's an example of an invalid slice boundary:
///   Slice((int*)100, (int*)101);
///
/// The different between the two pointers above is 1 but these differences
/// must always be a multiple of sizeof(int).
template<typename T>
slice<T>
Slice(T* FirstPtr, T* OnePastLastPtr)
{
  auto OnePastLastPtr_ = NonVoidPtr(OnePastLastPtr);
  auto FirstPtr_       = NonVoidPtr(FirstPtr);

  #if MTB_IsOn(MTB_WantBoundsCheck)
  // Check whether FirstPtr and OnePastLastPtr define a valid slice boundary.
  {
    auto A = Reinterpret<size_t>(FirstPtr);
    auto B = Reinterpret<size_t>(OnePastLastPtr);
    // NOTE: Don't use Abs(B - A) here because we're dealing with unsigned values.
    auto Delta = Max(A, B) - Min(A, B);
    MTB_Require(Delta % SizeOf<T>() == 0);
  }
  #endif

  slice<T> Result;
  SetLength(Result, OnePastLastPtr_ <= FirstPtr_ ? 0 : OnePastLastPtr_ - FirstPtr_);
  Result.Ptr = FirstPtr;
  return Result;
}

template<typename T, size_t N>
constexpr slice<T>
Slice(T (&Array)[N])
{
  return { N, &Array[0] };
}

/// Create a char slice from a static char array, excluding '\0'.
template<size_t N>
constexpr slice<char const>
SliceFromString(char const(&StringLiteral)[N])
{
  return { N - 1, &StringLiteral[0] };
}

/// \param StringPtr Must be null-terminated.
slice<char const>
SliceFromString(char const* StringPtr);

/// \param StringPtr Must be null-terminated.
slice<char>
SliceFromString(char* StringPtr);

/// Custom string literal suffix.
/// Usage: slice<char const> Foo = "Foo"_S;
inline slice<char const>
operator "" _S(char const* StringPtr, size_t Num) { return Slice(Num, StringPtr); }

/// Creates a new slice from an existing slice.
///
/// \param InclusiveStartIndex The index to start slicing from.
/// \param ExclusiveEndIndex The index of the first excluded element.
template<typename ElementType, typename StartIndexType, typename EndIndexType>
slice<ElementType>
Slice(slice<ElementType> A, StartIndexType InclusiveStartIndex, EndIndexType ExclusiveEndIndex)
{
  MTB_ReleaseAssert(InclusiveStartIndex <= ExclusiveEndIndex);
  slice<ElementType> Result;
  SetLength(Result, ExclusiveEndIndex - InclusiveStartIndex);
  Result.Ptr = AddElementOffset(A.Ptr, InclusiveStartIndex);
  MTB_BoundsCheck(Contains(A, Result));
  return Result;
}

/// Creates a new slice from an existing one, trimming elements at the beginning.
template<typename ElementType, typename AmountType>
constexpr slice<ElementType>
TrimFront(slice<ElementType> A, AmountType Amount)
{
  return
  {
    Amount > LengthOf(A) ? 0 : LengthOf(A) - Amount,
    AddElementOffset(A.Ptr, Amount)
  };
}

/// Creates a new slice from an existing one, trimming elements at the beginning.
template<typename ElementType, typename AmountType>
constexpr slice<ElementType>
TrimBack(slice<ElementType> A, AmountType Amount)
{
  return
  {
    Amount > LengthOf(A) ? 0 : LengthOf(A) - Amount,
    A.Ptr
  };
}

template<typename T, typename... ArgTypes>
inline void
ConstructElements(slice<T> Destination, ArgTypes&&... Args)
{
  MemConstruct(LengthOf(Destination), Destination.Ptr, Forward<ArgTypes>(Args)...);
}

template<typename T>
inline void
DestructElements(slice<T> Destination)
{
  MemDestruct(LengthOf(Destination), Destination.Ptr);
}

template<typename T>
inline size_t
CopyElements(slice<T> Destination, slice<T const> Source)
{
  size_t const Amount = Min(LengthOf(Destination), LengthOf(Source));
  MemCopy(Amount, Destination.Ptr, Source.Ptr);
  return Amount;
}

template<typename T>
inline size_t
CopyConstructElements(slice<T> Destination, slice<T const> Source)
{
  size_t const Amount = Min(LengthOf(Destination), LengthOf(Source));
  MemCopyConstruct(Amount, Destination.Ptr, Source.Ptr);
  return Amount;
}

template<typename T>
inline size_t
MoveElements(slice<T> Destination, slice<T> Source)
{
  size_t const Amount = Min(LengthOf(Destination), LengthOf(Source));
  MemMove(Amount, Destination.Ptr, Source.Ptr);
  return Amount;
}

template<typename T>
inline size_t
MoveConstructElements(slice<T> Destination, slice<T> Source)
{
  size_t const Amount = Min(LengthOf(Destination), LengthOf(Source));
  MemMoveConstruct(Amount, Destination.Ptr, Source.Ptr);
  return Amount;
}

template<typename T, typename U>
inline void
SetElements(slice<T> Destination, U&& Item)
{
  MemSet(LengthOf(Destination), Destination.Ptr, Forward<U>(Item));
}

template<typename T, typename NeedleType>
size_t
CountUntil(slice<T const> Haystack, NeedleType const& Needle)
{
  size_t Index = 0;

  for(auto& Straw : Haystack)
  {
    if(Straw == Needle)
      return Index;
    ++Index;
  }

  return INVALID_INDEX;
}

/// Counts up until \c Predicate(ElementOfHaystack, Needle) returns \c true.
template<typename T, typename NeedleType, typename PredicateType>
size_t
CountUntil(slice<T const> Haystack, NeedleType const& Needle, PredicateType Predicate)
{
  size_t Index = 0;

  for(auto& Straw : Haystack)
  {
    if(Predicate(Straw, Needle))
      return Index;
    ++Index;
  }

  return INVALID_INDEX;
}

template<typename T, typename U>
bool
StartsWith(slice<T const> Slice, slice<U const> Sequence)
{
  size_t const Amount = Min(LengthOf(Slice), LengthOf(Sequence));

  for(size_t Index = 0; Index < Amount; ++Index)
  {
    if(Slice[Index] != Sequence[Index])
      return false;
  }

  return true;
}

template<typename T, typename NeedleType>
slice<T>
Find(slice<T> Haystack, NeedleType const& Needle)
{
  while(LengthOf(Haystack))
  {
    if(Haystack[0] == Needle)
      return Haystack;
    Haystack = TrimFront(Haystack, 1);
  }

  return Haystack;
}

template<typename T, typename NeedleType, typename PredicateType>
slice<T>
Find(slice<T> Haystack, NeedleType const& Needle, PredicateType Predicate)
{
  while(LengthOf(Haystack))
  {
    if(Predicate(Haystack[0], Needle))
      return Haystack;
    Haystack = TrimFront(Haystack, 1);
  }

  return Haystack;
}

template<typename T, typename NeedleType>
slice<T>
Find(slice<T> Haystack, slice<NeedleType> const& NeedleSequence)
{
  while(LengthOf(Haystack))
  {
    if(StartsWith(Haystack, NeedleSequence))
      return Haystack;
    Haystack = TrimFront(Haystack, 1);
  }

  return Haystack;
}

template<typename T>
void
ReverseElements(slice<T> SomeSlice)
{
  auto const NumSwaps = LengthOf(SomeSlice) / 2;
  for(size_t FrontIndex = 0; FrontIndex < NumSwaps; ++FrontIndex)
  {
    auto const BackIndex = LengthOf(SomeSlice) - FrontIndex - 1;
    Swap(SomeSlice[FrontIndex], SomeSlice[BackIndex]);
  }
}

/// Compares the contents of the two slices for equality.
///
/// Two slices are deemed equal if they have the same number of elements and
/// each individual element in A compares equal to the corresponding element
/// in B in the order they appear in.
template<typename ElementTypeA, typename ElementTypeB>
bool
operator ==(slice<ElementTypeA> A, slice<ElementTypeB> B)
{
  if(LengthOf(A) != LengthOf(B)) return false;

  auto A_ = NonVoidPtr(First(A));
  auto B_ = NonVoidPtr(First(B));
  // if(A_ == B_) return true;
  if(Coerce<size_t>(A_) == Coerce<size_t>(B_)) return true;

  auto NumElements = LengthOf(A);
  while(NumElements)
  {
    if(*A_ != *B_)
      return false;

    ++A_;
    ++B_;
    --NumElements;
  }

  return true;
}

template<typename ElementType>
bool
operator ==(slice<ElementType> Slice, nullptr_t)
{
  return !Cast<bool>(Slice);
}
template<typename ElementType>
bool
operator !=(slice<ElementType> Slice, nullptr_t)
{
  return Cast<bool>(Slice);
}

template<typename ElementType>
bool
operator ==(nullptr_t, slice<ElementType> Slice)
{
  return !(Slice == nullptr);
}
template<typename ElementType>
bool
operator !=(nullptr_t, slice<ElementType> Slice)
{
  return Slice != nullptr;
}

template<typename ElementTypeA, typename ElementTypeB>
bool
operator !=(slice<ElementTypeA> A, slice<ElementTypeB> B)
{
  return !(A == B);
}

template<typename ElementType>
constexpr size_t
ByteLengthOf(slice<ElementType> A)
{
  return LengthOf(A) * SizeOf<ElementType>();
}

} // namespace mtb
#endif // !defined(MTB_HEADER_mtb_slice)


// ==============
// Implementation
// ==============

#if defined(MTB_SLICE_IMPLEMENTATION)
#if !defined(MTB_IMPL_mtb_slice)
#define MTB_IMPL_mtb_slice

auto mtb::
SliceFromString(char const* StringPtr)
  -> slice<char const>
{
  auto Seek = StringPtr;
  size_t Count = 0;
  while(*Seek++) ++Count;
  return { Count, StringPtr };
}

auto mtb::
SliceFromString(char* StringPtr)
  -> slice<char>
{
  auto Constified = Coerce<char const*>(StringPtr);
  auto ConstResult = SliceFromString(Constified);
  slice<char> Result;
  SetLength(Result, LengthOf(ConstResult));
  Result.Ptr = Coerce<char*>(ConstResult.Ptr);
  return Result;
}

#endif // !defined(MTB_IMPL_mtb_slice)
#endif // defined(MTB_SLICE_IMPLEMENTATION)
#if !defined(MTB_HEADER_mtb_conv)
#define MTB_HEADER_mtb_conv

#if defined(MTB_IMPLEMENTATION) && !defined(MTB_CONV_IMPLEMENTATION)
  #define MTB_CONV_IMPLEMENTATION
#endif

#if defined(MTB_CONV_IMPLEMENTATION) && !defined(MTB_IMPLEMENTATION)
  #define MTB_IMPLEMENTATION
#endif


// ==========================================
// mtb_slice.hpp
// ==========================================


namespace mtb
{

template<typename T>
struct convert_string_to_number_result
{
  // Whether conversion was successful or not.
  bool Success;

  // The actual value.
  T Value;

  // What is left after parsing the value.
  slice<char const> RemainingSource;
};


//
// Conversion: String -> Floating Point
//

convert_string_to_number_result<double>
ImplConvertStringToDouble(slice<char const> Source, double Fallback);

template<typename FloatType>
struct impl_convert_string_to_floating_point_helper
{
  template<typename CharType>
  static inline convert_string_to_number_result<FloatType>
  Do(slice<CharType> Source, FloatType Fallback = NaN<FloatType>())
  {
    auto StringToDoubleResult = ImplConvertStringToDouble(Coerce<slice<char const>>(Source), Cast<double>(Fallback));
    convert_string_to_number_result<FloatType> Result;
    Result.Success = StringToDoubleResult.Success;
    Result.Value = (FloatType)StringToDoubleResult.Value;
    Result.RemainingSource = StringToDoubleResult.RemainingSource;
    return Result;
  }
};

template<> struct impl_convert<double, slice<char>>        : impl_convert_string_to_floating_point_helper<double> {};
template<> struct impl_convert<double, slice<char const>>  : impl_convert_string_to_floating_point_helper<double> {};
template<> struct impl_convert<float,  slice<char>>        : impl_convert_string_to_floating_point_helper<float>  {};
template<> struct impl_convert<float,  slice<char const>>  : impl_convert_string_to_floating_point_helper<float>  {};

//
// Conversion: String -> Integer
//

convert_string_to_number_result<u64>
ImplConvertStringToInteger(slice<char const> Source, u64 Fallback);

convert_string_to_number_result<i64>
ImplConvertStringToInteger(slice<char const> Source, i64 Fallback);


template<typename MaxIntegerType, typename IntegerType>
struct impl_convert_string_to_integer_helper
{
  template<typename CharType>
  static inline convert_string_to_number_result<IntegerType>
  Do(slice<CharType> String, IntegerType Fallback = 0)
  {
    auto StringToIntResult = ImplConvertStringToInteger(Coerce<slice<char const>>(String), Cast<MaxIntegerType>(Fallback));
    convert_string_to_number_result<IntegerType> Result;
    Result.Success = StringToIntResult.Success;
    Result.Value = Convert<IntegerType>(StringToIntResult.Value);
    Result.RemainingSource = StringToIntResult.RemainingSource;
    return Result;
  }
};

template<> struct impl_convert<i8,  slice<char>>       : impl_convert_string_to_integer_helper<i64, i8>  {};
template<> struct impl_convert<i8,  slice<char const>> : impl_convert_string_to_integer_helper<i64, i8>  {};
template<> struct impl_convert<i16, slice<char>>       : impl_convert_string_to_integer_helper<i64, i16> {};
template<> struct impl_convert<i16, slice<char const>> : impl_convert_string_to_integer_helper<i64, i16> {};
template<> struct impl_convert<i32, slice<char>>       : impl_convert_string_to_integer_helper<i64, i32> {};
template<> struct impl_convert<i32, slice<char const>> : impl_convert_string_to_integer_helper<i64, i32> {};
template<> struct impl_convert<i64, slice<char>>       : impl_convert_string_to_integer_helper<i64, i64> {};
template<> struct impl_convert<i64, slice<char const>> : impl_convert_string_to_integer_helper<i64, i64> {};

template<> struct impl_convert<u8,  slice<char>>       : impl_convert_string_to_integer_helper<u64, u8>  {};
template<> struct impl_convert<u8,  slice<char const>> : impl_convert_string_to_integer_helper<u64, u8>  {};
template<> struct impl_convert<u16, slice<char>>       : impl_convert_string_to_integer_helper<u64, u16> {};
template<> struct impl_convert<u16, slice<char const>> : impl_convert_string_to_integer_helper<u64, u16> {};
template<> struct impl_convert<u32, slice<char>>       : impl_convert_string_to_integer_helper<u64, u32> {};
template<> struct impl_convert<u32, slice<char const>> : impl_convert_string_to_integer_helper<u64, u32> {};
template<> struct impl_convert<u64, slice<char>>       : impl_convert_string_to_integer_helper<u64, u64> {};
template<> struct impl_convert<u64, slice<char const>> : impl_convert_string_to_integer_helper<u64, u64> {};


//
// ==============================================
//

struct convert_number_to_string_result
{
  bool Success;
  slice<char> Value;
};


//
// Conversion: String -> Floating Point
//

// TODO


//
// Conversion: Integer -> String
//

convert_number_to_string_result
ImplConvertIntegerToString(i64 Integer, slice<char> Buffer);

convert_number_to_string_result
ImplConvertIntegerToString(u64 Integer, slice<char> Buffer);

template<typename MaxIntegerType, typename IntegerType>
struct impl_convert_integer_to_string_helper
{
  static inline convert_number_to_string_result
  Do(IntegerType Integer, slice<char> Buffer)
  {
    return ImplConvertIntegerToString((MaxIntegerType)Integer, Buffer);
  }
};

template<> struct impl_convert<slice<char>,        i8>  : impl_convert_integer_to_string_helper<i64, i8>   {};
template<> struct impl_convert<slice<char const>,  i8>  : impl_convert_integer_to_string_helper<i64, i8>   {};
template<> struct impl_convert<slice<char>,        i16> : impl_convert_integer_to_string_helper<i64, i16>  {};
template<> struct impl_convert<slice<char const>,  i16> : impl_convert_integer_to_string_helper<i64, i16>  {};
template<> struct impl_convert<slice<char>,        i32> : impl_convert_integer_to_string_helper<i64, i32>  {};
template<> struct impl_convert<slice<char const>,  i32> : impl_convert_integer_to_string_helper<i64, i32>  {};
template<> struct impl_convert<slice<char>,        i64> : impl_convert_integer_to_string_helper<i64, i64>  {};
template<> struct impl_convert<slice<char const>,  i64> : impl_convert_integer_to_string_helper<i64, i64>  {};

template<> struct impl_convert<slice<char>,        u8>  : impl_convert_integer_to_string_helper<u64, u8>  {};
template<> struct impl_convert<slice<char const>,  u8>  : impl_convert_integer_to_string_helper<u64, u8>  {};
template<> struct impl_convert<slice<char>,        u16> : impl_convert_integer_to_string_helper<u64, u16> {};
template<> struct impl_convert<slice<char const>,  u16> : impl_convert_integer_to_string_helper<u64, u16> {};
template<> struct impl_convert<slice<char>,        u32> : impl_convert_integer_to_string_helper<u64, u32> {};
template<> struct impl_convert<slice<char const>,  u32> : impl_convert_integer_to_string_helper<u64, u32> {};
template<> struct impl_convert<slice<char>,        u64> : impl_convert_integer_to_string_helper<u64, u64> {};
template<> struct impl_convert<slice<char const>,  u64> : impl_convert_integer_to_string_helper<u64, u64> {};

} // namespace mtb
#endif // !defined(MTB_HEADER_mtb_conv)


// ==============
// Implementation
// ==============

#if defined(MTB_CONV_IMPLEMENTATION)
#if !defined(MTB_IMPL_mtb_conv)
#define MTB_IMPL_mtb_conv


static mtb::slice<char const>
mtb_TrimWhitespaceFront(mtb::slice<char const> String)
{
  using namespace mtb;

  while(true)
  {
    if(LengthOf(String) == 0)
      return String;

    if(!IsWhitespace(String[0]))
      return String;

    String = TrimFront(String, 1);
  }
}

/// If a '+' was consumed, `1` is returned and \a OriginalSource is advanced by 1,
/// else, if a '-' was consumed, `-1` is returned and \a OriginalSource is advanced by 1,
/// otherwise 0 is returned and Source will not be modified.
static int
mtb_ConsumeSign(mtb::slice<char const>* Source)
{
  int Result{};

  if(*Source)
  {
    if((*Source)[0] == '+')
    {
      *Source = TrimFront(*Source, 1);
      Result = 1;
    }
    else if((*Source)[0] == '-')
    {
      *Source = TrimFront(*Source, 1);
      Result = -1;
    }
  }

  return Result;
}

auto mtb::
ImplConvertStringToDouble(slice<char const> Source, double Fallback)
  -> convert_string_to_number_result<double>
{
  convert_string_to_number_result<double> Result;
  Result.Success = false;
  Result.Value = Fallback;
  Result.RemainingSource = Source;

  Source = mtb_TrimWhitespaceFront(Source);
  if(Source)
  {
    bool HasSign = mtb_ConsumeSign(&Source) < 0;

    if(Source)
    {
      // Parse all parts of a floating point numbers
      convert_string_to_number_result<u64> NumericResult;
      convert_string_to_number_result<u64> ExponentResult;

      // Numeric part
      NumericResult = Convert<u64>(Source, 0.0f);
      Source = NumericResult.RemainingSource;
      double Value = NumericResult.Value;

      // Decimal part
      bool HasDecimalPart{};
      u64 DecimalValue{};
      u64 DecimalDivider = 1;
      if(Source && Source[0] == '.')
      {
        Source = TrimFront(Source, 1);
        while(Source && IsDigit(Source[0]))
        {
          u64 NewDigit = (u64)(Source[0] - '0');
          DecimalValue = (10 * DecimalValue) + NewDigit;
          HasDecimalPart = true;
          DecimalDivider *= 10;
          Source = TrimFront(Source, 1);
        }

        Value += (double)DecimalValue / (double)DecimalDivider;
      }

      if(NumericResult.Success || HasDecimalPart)
      {
        // Parse exponent, if any.
        if(Source && (Source[0] == 'e' || Source[0] == 'E'))
        {
          auto ExponentSource = TrimFront(Source, 1);
          bool ExponentHasSign = mtb_ConsumeSign(&ExponentSource) < 0;
          ExponentResult = Convert<u64>(ExponentSource);
          if(ExponentResult.Success)
          {
            Source = ExponentResult.RemainingSource;

            u64 ExponentPart = ExponentResult.Value;
            i64 ExponentValue = 1;
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
        Result.RemainingSource = Source;
        Result.Value = HasSign ? -Value : Value;
      }
    }
  }

  return Result;
}

template<typename IntegerType>
mtb::convert_string_to_number_result<IntegerType>
ImplConvertStringToIntegerHelper(mtb::slice<char const> Source, IntegerType Fallback)
{
  using namespace mtb;

  convert_string_to_number_result<IntegerType> Result;
  Result.Success = false;
  Result.Value = Fallback;
  Result.RemainingSource = Source;

  if(Source)
  {
    Source = mtb_TrimWhitespaceFront(Source);

    bool HasSign = false;
    switch(Source[0])
    {
    case '+':
      Source = TrimFront(Source, 1);
      break;
    case '-':
      HasSign = true;
      Source = TrimFront(Source, 1);
      break;
    default:
      break;
    }

    u64 NumericalPart = 0;
    bool HasNumericalPart = false;

    while(LengthOf(Source) > 0 && IsDigit(Source[0]))
    {
      NumericalPart *= 10;
      NumericalPart += Source[0] - '0';
      HasNumericalPart = true;
      Source = TrimFront(Source, 1);
    }

    if(HasNumericalPart)
    {
      auto Value = Convert<IntegerType>(NumericalPart);

      if(HasSign)
      {
        if(IntIsSigned<IntegerType>())
        {
          Result.Value = Negate(Value);
          Result.Success = true;
        }
        else
        {
          // Unsigned types cannot have a '-' sign.
        }
      }
      else
      {
        Result.Value = Value;
        Result.Success = true;
      }
    }
  }

  if(Result.Success)
    Result.RemainingSource = Source;

  return Result;
}

auto mtb::
ImplConvertStringToInteger(slice<char const> Source, u64 Fallback)
  -> convert_string_to_number_result<u64>
{
  return ImplConvertStringToIntegerHelper<u64>(Source, Fallback);
}

auto mtb::
ImplConvertStringToInteger(slice<char const> Source, i64 Fallback)
  -> convert_string_to_number_result<i64>
{
  return ImplConvertStringToIntegerHelper<i64>(Source, Fallback);
}

template<typename IntegerType>
mtb::convert_number_to_string_result
ImplConvertIntegerToStringHelper(IntegerType Integer, mtb::slice<char> Buffer)
{
  using namespace mtb;

  convert_number_to_string_result Result{};

  if(Buffer)
  {
    if(Integer == 0)
    {
      Buffer[0] = '0';
      Result.Success = true;
      Result.Value = Slice(Buffer, 0, 1);
    }
    else
    {
      size_t NumChars = 0;
      if(Integer < 0)
      {
        Buffer[NumChars++] = '-';
        Integer = Negate(Integer);
      }

      while(Integer > 0)
      {
        auto const Digit = (char)(Integer % 10);
        Buffer[NumChars++] = '0' + Digit;
        Integer /= 10;
      }

      Result.Value = Slice(Buffer, 0, NumChars);

      // Result.Value now contains the digits in reverse order, so we swap them around.
      ReverseElements(Result.Value);

      Result.Success = true;
    }
  }

  return Result;
}

auto mtb::
ImplConvertIntegerToString(i64 Integer, slice<char> Buffer)
  -> mtb::convert_number_to_string_result
{
  return ImplConvertIntegerToStringHelper<i64>(Integer, Buffer);
}

auto mtb::
ImplConvertIntegerToString(u64 Integer, slice<char> Buffer)
  -> mtb::convert_number_to_string_result
{
  return ImplConvertIntegerToStringHelper<u64>(Integer, Buffer);
}

#endif // !defined(MTB_IMPL_mtb_conv)
#endif // defined(MTB_CONV_IMPLEMENTATION)
