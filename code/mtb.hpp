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

#define MTB_Is32BitArch MTB_Off
#define MTB_Is64BitArch MTB_Off

#if MTB_IsArch(x86_64)
  #undef  MTB_Is64BitArch
  #define MTB_Is64BitArch MTB_On
#elif MTB_IsArch(x86)
  #undef  MTB_Is32BitArch
  #define MTB_Is32BitArch MTB_On
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
  #if MTB_IsOn(MTB_DebugBuild) || MTB_IsOn(MTB_DevBuild)
    #define MTB_Internal MTB_On
  #else
    #define MTB_Internal MTB_Off
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



#if defined(MTB_IMPLEMENTATION)
  #define MTB_COMMON_IMPLEMENTATION
#endif

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

struct memory_size
{
  // In bytes.
  u64 Value;

  constexpr
  operator size_t() const
  {
    // TODO: Ensure safe conversion?
    return (size_t)Value;
  }
};

constexpr bool operator ==(memory_size A, memory_size B) { return A.Value == B.Value; }
constexpr bool operator !=(memory_size A, memory_size B) { return A.Value != B.Value; }
constexpr bool operator < (memory_size A, memory_size B) { return A.Value <  B.Value; }
constexpr bool operator <=(memory_size A, memory_size B) { return A.Value <= B.Value; }
constexpr bool operator > (memory_size A, memory_size B) { return A.Value >  B.Value; }
constexpr bool operator >=(memory_size A, memory_size B) { return A.Value >= B.Value; }

constexpr memory_size operator +(memory_size A, memory_size B) { return { A.Value + B.Value }; }
constexpr memory_size operator -(memory_size A, memory_size B) { return { A.Value - B.Value }; }
constexpr memory_size operator *(memory_size A, u64 Scale) { return { A.Value * Scale };   }
constexpr memory_size operator *(u64 Scale, memory_size A) { return { Scale * A.Value };   }
constexpr memory_size operator /(memory_size A, u64 Scale) { return { A.Value / Scale };   }

void operator +=(memory_size& A, memory_size B);
void operator -=(memory_size& A, memory_size B);
void operator *=(memory_size& A, u64 Scale);
void operator /=(memory_size& A, u64 Scale);

constexpr memory_size Bytes(u64 Amount) { return { Amount }; }
constexpr u64 ToBytes(memory_size Size) { return Size.Value; }

constexpr memory_size KiB(u64 Amount) { return { Amount * 1024 }; }
constexpr memory_size MiB(u64 Amount) { return { Amount * 1024 * 1024 }; }
constexpr memory_size GiB(u64 Amount) { return { Amount * 1024 * 1024 * 1024 }; }
constexpr memory_size TiB(u64 Amount) { return { Amount * 1024 * 1024 * 1024 * 1024 }; }

constexpr memory_size KB(u64 Amount) { return { Amount * 1000 }; }
constexpr memory_size MB(u64 Amount) { return { Amount * 1000 * 1000 }; }
constexpr memory_size GB(u64 Amount) { return { Amount * 1000 * 1000 * 1000 }; }
constexpr memory_size TB(u64 Amount) { return { Amount * 1000 * 1000 * 1000 * 1000 }; }

template<typename OutputType = float> constexpr OutputType ToKiB(memory_size Size) { return OutputType(double(Size.Value) / 1024); }
template<typename OutputType = float> constexpr OutputType ToMiB(memory_size Size) { return OutputType(double(Size.Value) / 1024 / 1024); }
template<typename OutputType = float> constexpr OutputType ToGiB(memory_size Size) { return OutputType(double(Size.Value) / 1024 / 1024 / 1024); }
template<typename OutputType = float> constexpr OutputType ToTiB(memory_size Size) { return OutputType(double(Size.Value) / 1024 / 1024 / 1024 / 1024); }

template<typename OutputType = float> constexpr OutputType ToKB(memory_size Size) { return OutputType(double(Size.Value) / 1000); }
template<typename OutputType = float> constexpr OutputType ToMB(memory_size Size) { return OutputType(double(Size.Value) / 1000 / 1000); }
template<typename OutputType = float> constexpr OutputType ToGB(memory_size Size) { return OutputType(double(Size.Value) / 1000 / 1000 / 1000); }
template<typename OutputType = float> constexpr OutputType ToTB(memory_size Size) { return OutputType(double(Size.Value) / 1000 / 1000 / 1000 / 1000); }



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
/// Returns the number of elements in this static array.
template<typename Type, size_t N>
constexpr size_t
Length(Type(&)[N]) { return N; }

/// Same as \c Length but force to 32 bit value.
template<typename Type, size_t N>
constexpr u32
Length32(Type(&)[N]) { return (u32)N; }

template<typename T> struct impl_size_of { enum { SizeInBytes = sizeof(T) }; };
template<>           struct impl_size_of<void>          : impl_size_of<u8>          { };
template<>           struct impl_size_of<void const>    : impl_size_of<u8 const>    { };
template<>           struct impl_size_of<void volatile> : impl_size_of<u8 volatile> { };

/// Get the size of type T in bytes.
///
/// Same as sizeof(T) except it works also with 'void' (possibly cv-qualified)
/// where a size of 1 byte is assumed.
template<typename T>
constexpr memory_size
SizeOf() { return Bytes(impl_size_of<T>::SizeInBytes); }

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
MemAddByteOffset(PointerType* Pointer, OffsetType Offset)
{
  return (PointerType*)((u8*)Pointer + Offset);
}

/// Advance the given pointer value by the given amount times sizeof(PointerType)
template<typename PointerType, typename OffsetType>
constexpr PointerType*
MemAddOffset(PointerType* Pointer, OffsetType Offset)
{
  return MemAddByteOffset(Pointer, Offset * ToBytes(SizeOf<PointerType>()));
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

template<class Type>
constexpr typename rm_ref<Type>&&
Move(Type&& Argument)
{
  // forward Argument as movable
  return static_cast<typename rm_ref<Type>&&>(Argument);
}

template<typename Type>
constexpr Type&&
Forward(typename rm_ref<Type>& Argument)
{
  return static_cast<Type&&>(Argument);
}

template<typename Type>
constexpr Type&&
Forward(rm_ref<Type>&& Argument)
{
  return static_cast<Type&&>(Argument);
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

template<typename Type>
Type const&
AsConst(Type& Value)
{
  return const_cast<Type const&>(Value);
}

template<typename Type>
Type const*
AsPtrToConst(Type* Value)
{
  return const_cast<Type const*>(Value);
}

template<typename ToType, typename FromType>
struct impl_convert
{
  static constexpr ToType
  Do(FromType const& Value)
  {
    return Cast<ToType>(Value);
  }
};

template<typename ToType, typename FromType, typename... ExtraTypes>
ToType
Convert(FromType const& From, ExtraTypes&&... Extra)
{
  using UnqualifiedToType   = rm_ref_const<ToType>;
  using UnqualifiedFromType = rm_ref_const<FromType>;
  using Impl = impl_convert<UnqualifiedToType, UnqualifiedFromType>;
  return Impl::Do(From, Forward<ExtraTypes>(Extra)...);
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
template<typename Type>
constexpr Type
Sign(Type I)
{
  return Type(I > 0 ? 1 : I < 0 ? -1 : 0);
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

template<typename Type>
constexpr Type
Abs(Type Value)
{
  return impl_abs<Type>::Do(Value);
}

template<typename t_a_type, typename t_b_type>
constexpr t_a_type
Min(t_a_type A, t_b_type B)
{
  return (B < A) ? Coerce<t_a_type>(B) : A;
}

template<typename t_a_type, typename t_b_type>
constexpr t_a_type
Max(t_a_type A, t_b_type B)
{
  return (B > A) ? Coerce<t_a_type>(B) : A;
}

template<typename t_value_type, typename t_lower_bound_type, typename t_upper_bound_type>
constexpr t_value_type
Clamp(t_value_type Value, t_lower_bound_type LowerBound, t_upper_bound_type UpperBound)
{
  return UpperBound < LowerBound ? Value : Min(UpperBound, Max(LowerBound, Value));
}

// TODO: Make this a constexpr?
template<typename t_value_type, typename t_lower_bound_type, typename t_upper_bound_type>
t_value_type
Wrap(t_value_type Value, t_lower_bound_type LowerBound, t_upper_bound_type UpperBound)
{
  const auto BoundsDelta = (Coerce<t_lower_bound_type>(UpperBound) - LowerBound);
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
template<typename t_result, typename t_value_type, typename t_lower_bound_type, typename t_upper_bound_type>
constexpr t_result
NormalizeValue(t_value_type Value, t_lower_bound_type LowerBound, t_upper_bound_type UpperBound)
{
  return UpperBound <= LowerBound ?
         t_result(0) : // Bogus bounds.
         Cast<t_result>(Value - LowerBound) / Cast<t_result>(UpperBound - LowerBound);
}

bool
AreNearlyEqual(double A, double B, double Epsilon = 1e-4);

bool
AreNearlyEqual(float A, float B, float Epsilon = 1e-4f);

inline bool
IsNearlyZero(double A, double Epsilon = 1e-4) { return AreNearlyEqual(A, 0, Epsilon); }

inline bool
IsNearlyZero(float A, float Epsilon = 1e-4f) { return AreNearlyEqual(A, 0, Epsilon); }

template<typename t_a, typename t_b>
inline void
Swap(t_a& A, t_b& B)
{
  auto Temp{ Move(A) };
  A = Move(B);
  B = Move(Temp);
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

  template<typename t_in_func_type>
  defer<t_in_func_type> operator =(t_in_func_type InLambda) { return { Move(InLambda) }; }
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

void mtb::
operator +=(memory_size& A, memory_size B)
{
  A.Value += B.Value;
}

void mtb::
operator -=(memory_size& A, memory_size B)
{
  A.Value -= B.Value;
}

void mtb::
operator *=(memory_size& A, u64 Scale)
{
  A.Value *= Scale;
}

void mtb::
operator /=(memory_size& A, u64 Scale)
{
  A.Value /= Scale;
}

float mtb::
Pow(float Base, float Exponent)
{
  return powf(Base, Exponent);
}

double mtb::
Pow(double Base, double Exponent)
{
  return pow(Base, Exponent);
}

float mtb::
Mod(float Value, float Divisor)
{
  return fmodf(Value, Divisor);
}

double mtb::
Mod(double Value, double Divisor)
{
  return fmod(Value, Divisor);
}

float mtb::
Sqrt(float Value)
{
  return sqrtf(Value);
}

double mtb::
Sqrt(double Value)
{
  return sqrt(Value);
}

float mtb::
InvSqrt(float Value)
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

bool mtb::
AreNearlyEqual(double A, double B, double Epsilon)
{
  return Abs(A - B) <= Epsilon;
}

bool mtb::
AreNearlyEqual(float A, float B, float Epsilon)
{
  return Abs(A - B) <= Epsilon;
}

#endif // !defined(MTB_IMPL_mtb_common)
#endif // defined(MTB_COMMON_IMPLEMENTATION)


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

#define MTB_Is32BitArch MTB_Off
#define MTB_Is64BitArch MTB_Off

#if MTB_IsArch(x86_64)
  #undef  MTB_Is64BitArch
  #define MTB_Is64BitArch MTB_On
#elif MTB_IsArch(x86)
  #undef  MTB_Is32BitArch
  #define MTB_Is32BitArch MTB_On
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
  #if MTB_IsOn(MTB_DebugBuild) || MTB_IsOn(MTB_DevBuild)
    #define MTB_Internal MTB_On
  #else
    #define MTB_Internal MTB_Off
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



#if defined(MTB_IMPLEMENTATION)
  #define MTB_COMMON_IMPLEMENTATION
#endif

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

struct memory_size
{
  // In bytes.
  u64 Value;

  constexpr
  operator size_t() const
  {
    // TODO: Ensure safe conversion?
    return (size_t)Value;
  }
};

constexpr bool operator ==(memory_size A, memory_size B) { return A.Value == B.Value; }
constexpr bool operator !=(memory_size A, memory_size B) { return A.Value != B.Value; }
constexpr bool operator < (memory_size A, memory_size B) { return A.Value <  B.Value; }
constexpr bool operator <=(memory_size A, memory_size B) { return A.Value <= B.Value; }
constexpr bool operator > (memory_size A, memory_size B) { return A.Value >  B.Value; }
constexpr bool operator >=(memory_size A, memory_size B) { return A.Value >= B.Value; }

constexpr memory_size operator +(memory_size A, memory_size B) { return { A.Value + B.Value }; }
constexpr memory_size operator -(memory_size A, memory_size B) { return { A.Value - B.Value }; }
constexpr memory_size operator *(memory_size A, u64 Scale) { return { A.Value * Scale };   }
constexpr memory_size operator *(u64 Scale, memory_size A) { return { Scale * A.Value };   }
constexpr memory_size operator /(memory_size A, u64 Scale) { return { A.Value / Scale };   }

void operator +=(memory_size& A, memory_size B);
void operator -=(memory_size& A, memory_size B);
void operator *=(memory_size& A, u64 Scale);
void operator /=(memory_size& A, u64 Scale);

constexpr memory_size Bytes(u64 Amount) { return { Amount }; }
constexpr u64 ToBytes(memory_size Size) { return Size.Value; }

constexpr memory_size KiB(u64 Amount) { return { Amount * 1024 }; }
constexpr memory_size MiB(u64 Amount) { return { Amount * 1024 * 1024 }; }
constexpr memory_size GiB(u64 Amount) { return { Amount * 1024 * 1024 * 1024 }; }
constexpr memory_size TiB(u64 Amount) { return { Amount * 1024 * 1024 * 1024 * 1024 }; }

constexpr memory_size KB(u64 Amount) { return { Amount * 1000 }; }
constexpr memory_size MB(u64 Amount) { return { Amount * 1000 * 1000 }; }
constexpr memory_size GB(u64 Amount) { return { Amount * 1000 * 1000 * 1000 }; }
constexpr memory_size TB(u64 Amount) { return { Amount * 1000 * 1000 * 1000 * 1000 }; }

template<typename OutputType = float> constexpr OutputType ToKiB(memory_size Size) { return OutputType(double(Size.Value) / 1024); }
template<typename OutputType = float> constexpr OutputType ToMiB(memory_size Size) { return OutputType(double(Size.Value) / 1024 / 1024); }
template<typename OutputType = float> constexpr OutputType ToGiB(memory_size Size) { return OutputType(double(Size.Value) / 1024 / 1024 / 1024); }
template<typename OutputType = float> constexpr OutputType ToTiB(memory_size Size) { return OutputType(double(Size.Value) / 1024 / 1024 / 1024 / 1024); }

template<typename OutputType = float> constexpr OutputType ToKB(memory_size Size) { return OutputType(double(Size.Value) / 1000); }
template<typename OutputType = float> constexpr OutputType ToMB(memory_size Size) { return OutputType(double(Size.Value) / 1000 / 1000); }
template<typename OutputType = float> constexpr OutputType ToGB(memory_size Size) { return OutputType(double(Size.Value) / 1000 / 1000 / 1000); }
template<typename OutputType = float> constexpr OutputType ToTB(memory_size Size) { return OutputType(double(Size.Value) / 1000 / 1000 / 1000 / 1000); }



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
/// Returns the number of elements in this static array.
template<typename Type, size_t N>
constexpr size_t
Length(Type(&)[N]) { return N; }

/// Same as \c Length but force to 32 bit value.
template<typename Type, size_t N>
constexpr u32
Length32(Type(&)[N]) { return (u32)N; }

template<typename T> struct impl_size_of { enum { SizeInBytes = sizeof(T) }; };
template<>           struct impl_size_of<void>          : impl_size_of<u8>          { };
template<>           struct impl_size_of<void const>    : impl_size_of<u8 const>    { };
template<>           struct impl_size_of<void volatile> : impl_size_of<u8 volatile> { };

/// Get the size of type T in bytes.
///
/// Same as sizeof(T) except it works also with 'void' (possibly cv-qualified)
/// where a size of 1 byte is assumed.
template<typename T>
constexpr memory_size
SizeOf() { return Bytes(impl_size_of<T>::SizeInBytes); }

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
MemAddByteOffset(PointerType* Pointer, OffsetType Offset)
{
  return (PointerType*)((u8*)Pointer + Offset);
}

/// Advance the given pointer value by the given amount times sizeof(PointerType)
template<typename PointerType, typename OffsetType>
constexpr PointerType*
MemAddOffset(PointerType* Pointer, OffsetType Offset)
{
  return MemAddByteOffset(Pointer, Offset * ToBytes(SizeOf<PointerType>()));
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

template<class Type>
constexpr typename rm_ref<Type>&&
Move(Type&& Argument)
{
  // forward Argument as movable
  return static_cast<typename rm_ref<Type>&&>(Argument);
}

template<typename Type>
constexpr Type&&
Forward(typename rm_ref<Type>& Argument)
{
  return static_cast<Type&&>(Argument);
}

template<typename Type>
constexpr Type&&
Forward(rm_ref<Type>&& Argument)
{
  return static_cast<Type&&>(Argument);
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

template<typename Type>
Type const&
AsConst(Type& Value)
{
  return const_cast<Type const&>(Value);
}

template<typename Type>
Type const*
AsPtrToConst(Type* Value)
{
  return const_cast<Type const*>(Value);
}

template<typename ToType, typename FromType>
struct impl_convert
{
  static constexpr ToType
  Do(FromType const& Value)
  {
    return Cast<ToType>(Value);
  }
};

template<typename ToType, typename FromType, typename... ExtraTypes>
ToType
Convert(FromType const& From, ExtraTypes&&... Extra)
{
  using UnqualifiedToType   = rm_ref_const<ToType>;
  using UnqualifiedFromType = rm_ref_const<FromType>;
  using Impl = impl_convert<UnqualifiedToType, UnqualifiedFromType>;
  return Impl::Do(From, Forward<ExtraTypes>(Extra)...);
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
template<typename Type>
constexpr Type
Sign(Type I)
{
  return Type(I > 0 ? 1 : I < 0 ? -1 : 0);
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

template<typename Type>
constexpr Type
Abs(Type Value)
{
  return impl_abs<Type>::Do(Value);
}

template<typename t_a_type, typename t_b_type>
constexpr t_a_type
Min(t_a_type A, t_b_type B)
{
  return (B < A) ? Coerce<t_a_type>(B) : A;
}

template<typename t_a_type, typename t_b_type>
constexpr t_a_type
Max(t_a_type A, t_b_type B)
{
  return (B > A) ? Coerce<t_a_type>(B) : A;
}

template<typename t_value_type, typename t_lower_bound_type, typename t_upper_bound_type>
constexpr t_value_type
Clamp(t_value_type Value, t_lower_bound_type LowerBound, t_upper_bound_type UpperBound)
{
  return UpperBound < LowerBound ? Value : Min(UpperBound, Max(LowerBound, Value));
}

// TODO: Make this a constexpr?
template<typename t_value_type, typename t_lower_bound_type, typename t_upper_bound_type>
t_value_type
Wrap(t_value_type Value, t_lower_bound_type LowerBound, t_upper_bound_type UpperBound)
{
  const auto BoundsDelta = (Coerce<t_lower_bound_type>(UpperBound) - LowerBound);
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
template<typename t_result, typename t_value_type, typename t_lower_bound_type, typename t_upper_bound_type>
constexpr t_result
NormalizeValue(t_value_type Value, t_lower_bound_type LowerBound, t_upper_bound_type UpperBound)
{
  return UpperBound <= LowerBound ?
         t_result(0) : // Bogus bounds.
         Cast<t_result>(Value - LowerBound) / Cast<t_result>(UpperBound - LowerBound);
}

bool
AreNearlyEqual(double A, double B, double Epsilon = 1e-4);

bool
AreNearlyEqual(float A, float B, float Epsilon = 1e-4f);

inline bool
IsNearlyZero(double A, double Epsilon = 1e-4) { return AreNearlyEqual(A, 0, Epsilon); }

inline bool
IsNearlyZero(float A, float Epsilon = 1e-4f) { return AreNearlyEqual(A, 0, Epsilon); }

template<typename t_a, typename t_b>
inline void
Swap(t_a& A, t_b& B)
{
  auto Temp{ Move(A) };
  A = Move(B);
  B = Move(Temp);
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

  template<typename t_in_func_type>
  defer<t_in_func_type> operator =(t_in_func_type InLambda) { return { Move(InLambda) }; }
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

void mtb::
operator +=(memory_size& A, memory_size B)
{
  A.Value += B.Value;
}

void mtb::
operator -=(memory_size& A, memory_size B)
{
  A.Value -= B.Value;
}

void mtb::
operator *=(memory_size& A, u64 Scale)
{
  A.Value *= Scale;
}

void mtb::
operator /=(memory_size& A, u64 Scale)
{
  A.Value /= Scale;
}

float mtb::
Pow(float Base, float Exponent)
{
  return powf(Base, Exponent);
}

double mtb::
Pow(double Base, double Exponent)
{
  return pow(Base, Exponent);
}

float mtb::
Mod(float Value, float Divisor)
{
  return fmodf(Value, Divisor);
}

double mtb::
Mod(double Value, double Divisor)
{
  return fmod(Value, Divisor);
}

float mtb::
Sqrt(float Value)
{
  return sqrtf(Value);
}

double mtb::
Sqrt(double Value)
{
  return sqrt(Value);
}

float mtb::
InvSqrt(float Value)
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

bool mtb::
AreNearlyEqual(double A, double B, double Epsilon)
{
  return Abs(A - B) <= Epsilon;
}

bool mtb::
AreNearlyEqual(float A, float B, float Epsilon)
{
  return Abs(A - B) <= Epsilon;
}

#endif // !defined(MTB_IMPL_mtb_common)
#endif // defined(MTB_COMMON_IMPLEMENTATION)


#if !defined(MTB_HEADER_mtb_assert)
#define MTB_HEADER_mtb_assert

#if defined(MTB_IMPLEMENTATION)
  #define MTB_ASSERT_IMPLEMENTATION
#endif

#if defined(MTB_ASSERT_IMPLEMENTATION) && !defined(MTB_COMMON_IMPLEMENTATION)
  #define MTB_COMMON_IMPLEMENTATION
#endif



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



#if !defined(MTB_HEADER_memory)
#define MTB_HEADER_memory

#if defined(MTB_IMPLEMENTATION)
  #define MTB_MEMORY_IMPLEMENTATION
#endif

#if defined(MTB_MEMORY_IMPLEMENTATION) && !defined(MTB_COMMON_IMPLEMENTATION)
  #define MTB_COMMON_IMPLEMENTATION
#endif

#if defined(MTB_MEMORY_IMPLEMENTATION) && !defined(MTB_ASSERT_IMPLEMENTATION)
  #define MTB_ASSERT_IMPLEMENTATION
#endif




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
/// C Standard Function | Untyped/Bytes                           | Typed
/// ------------------- | --------------------------------------- | -----
/// memcopy, memmove    | MemCopyBytes                        | MemCopy, MemCopyConstruct, MemMove, MemMoveConstruct
/// memset              | MemSetBytes                         | MemSet, MemConstruct
/// memcmp              | MemCompareBytes, MemEqualBytes  | -
///
///
/// All functions are optimized for POD types.
///
/// @{

/// Copy Size from Source to Destination.
///
/// Destination and Source may overlap.
void
MemCopyBytes(memory_size Size, void* Destination, void const* Source);

/// Fill NumBytes in Destination with the value
void
MemSetBytes(memory_size Size, void* Destination, int Value);

bool
MemEqualBytes(memory_size Size, void const* A, void const* B);

int
MemCompareBytes(memory_size Size, void const* A, void const* B);

bool
MemAreOverlappingBytes(memory_size SizeA, void const* A, memory_size SizeB, void const* B);


/// Calls the constructor of all elements in Destination with Args.
///
/// Args may be empty in which case all elements get default-initialized.
template<typename T, typename... ArgTypes>
void
MemConstruct(size_t Num, T* Destination, ArgTypes&&... Args);

/// Destructs all elements in Destination.
template<typename T>
void
MemDestruct(size_t Num, T* Destination);

/// Copy all elements from Source to Destination.
///
/// Destination and Source may overlap.
template<typename T>
void
MemCopy(size_t Num, T* Destination, T const* Source);

/// Copy all elements from Source to Destination using T's constructor.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
MemCopyConstruct(size_t Num, T* Destination, T const* Source);

/// Move all elements from Source to Destination using T's constructor.
///
/// Destination and Source may overlap.
template<typename T>
void
MemMove(size_t Num, T* Destination, T* Source);

/// Move all elements from Source to Destination using T's constructor and destruct Source afterwards.
///
/// Destination and Source may NOT overlap. Destination is assumed to be
/// uninitialized.
template<typename T>
void
MemMoveConstruct(size_t Num, T* Destination, T* Source);

/// Assign the default value of T to all elements in Destination.
template<typename T>
void
MemSet(size_t Num, T* Destination);

/// Assign Item to all elements in Destination.
template<typename T>
void
MemSet(size_t Num, T* Destination, T const& Item);

template<typename TA, typename TB>
bool
MemAreOverlapping(size_t NumA, TA const* A, size_t NumB, TB const* B)
{
  return MemAreOverlappingBytes(NumA * SizeOf<TA>(), Reinterpret<void const*>(A),
                                NumB * SizeOf<TB>(), Reinterpret<void const*>(B));
}


//
// Implementation Details
//

// MemConstruct

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_construct
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
struct impl_mem_construct<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination)
  {
    MemSetBytes(Num * SizeOf<T>(), Destination, 0);
  }

  MTB_Inline static void
  Do(size_t Num, T* Destination, T const& Item)
  {
    // Blit Item over each element of Destination.
    for(size_t Index = 0; Index < Num; ++Index)
    {
      MemCopyBytes(SizeOf<T>(), &Destination[Index], &Item);
    }
  }
};

template<typename T, typename... ArgTypes>
MTB_Inline auto
MemConstruct(size_t Num, T* Destination, ArgTypes&&... Args)
  -> void
{
  impl_mem_construct<T, IsPOD<T>()>::Do(Num, Destination, Forward<ArgTypes>(Args)...);
}


// MemDestruct

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_destruct
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
struct impl_mem_destruct<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination)
  {
    // Nothing to do for POD types.
  }
};

template<typename T>
MTB_Inline auto
MemDestruct(size_t Num, T* Destination)
  -> void
{
  impl_mem_destruct<T, IsPOD<T>()>::Do(Num, Destination);
}


// MemCopy

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_copy
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    if(Destination == Source)
      return;

    if(MemAreOverlapping(Num, Destination, Num, Source) && Destination < Source)
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
struct impl_mem_copy<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    MemCopyBytes(SizeOf<T>() * Num, Destination, Source);
  }
};

template<typename T>
MTB_Inline auto
MemCopy(size_t Num, T* Destination, T const* Source)
  -> void
{
  impl_mem_copy<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// MemCopyConstruct

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_copy_construct
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_DebugAssert(!MemAreOverlapping(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Source[Index]);
    }
  }
};

template<typename T>
struct impl_mem_copy_construct<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_DebugAssert(!MemAreOverlapping(Num, Destination, Num, Source));

    MemCopy(Num, Destination, Source);
  }
};

template<typename T>
MTB_Inline auto
MemCopyConstruct(size_t Num, T* Destination, T const* Source)
  -> void
{
  impl_mem_copy_construct<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// MemMove

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_move
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T* Source)
  {
    if(Destination == Source)
      return;

    if(MemAreOverlapping(Num, Destination, Num, Source))
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
        MemDestruct(NumToDestruct, MemAddOffset(Source, Num - NumToDestruct));
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
        MemDestruct(NumToDestruct, Source);
      }
    }
    else
    {
      // Straight forward: Move one by one, then destruct all in Source.
      for(size_t Index = 0; Index < Num; ++Index)
      {
        Destination[Index] = Move(Source[Index]);
      }
      MemDestruct(Num, Source);
    }
  }
};

template<typename T>
struct impl_mem_move<T, true> : public impl_mem_copy<T, true> {};

template<typename T>
MTB_Inline auto
MemMove(size_t Num, T* Destination, T* Source)
  -> void
{
  impl_mem_move<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// MemMoveConstruct

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_move_construct
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T* Source)
  {
    // When using the constructor, overlapping is not allowed.
    MTB_DebugAssert(!MemAreOverlapping(Num, Destination, Num, Source));

    for(size_t Index = 0; Index < Num; ++Index)
    {
      new (&Destination[Index]) T(Move(Source[Index]));
    }
    MemDestruct(Num, Source);
  }
};

template<typename T>
struct impl_mem_move_construct<T, true>
{
  MTB_Inline static void
  Do(size_t Num, T* Destination, T const* Source)
  {
    // When using the constructor, overlapping is not allowed. Even though in
    // the POD case here it doesn't make a difference, it might help to catch
    // bugs since this can't be intentional.
    MTB_DebugAssert(!MemAreOverlapping(Num, Destination, Num, Source));

    MemCopy(Num, Destination, Source);
  }
};

template<typename T>
MTB_Inline auto
MemMoveConstruct(size_t Num, T* Destination, T* Source)
  -> void
{
  impl_mem_move_construct<T, IsPOD<T>()>::Do(Num, Destination, Source);
}


// MemSet

template<typename T, bool TIsPlainOldData = false>
struct impl_mem_set
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
struct impl_mem_set<T, true> : public impl_mem_construct<T, true> {};

template<typename T>
MTB_Inline auto
MemSet(size_t Num, T* Destination)
  -> void
{
  impl_mem_set<T, IsPOD<T>()>::Do(Num, Destination);
}

template<typename T>
MTB_Inline auto
MemSet(size_t Num, T* Destination, T const& Item)
  -> void
{
  impl_mem_set<T, IsPOD<T>()>::Do(Num, Destination, Item);
}

} // namespace mtb
#endif // !defined(MTB_HEADER_memory)

#if defined(MTB_MEMORY_IMPLEMENTATION)

#if !defined(MTB_IMPL_memory)
#define MTB_IMPL_memory

#include <cstring>

auto mtb::
MemCopyBytes(memory_size Size, void* Destination, void const* Source)
  -> void
{
  // Using memmove so that Destination and Source may overlap.
  std::memmove(Destination, Source, ToBytes(Size));
}

auto mtb::
MemSetBytes(memory_size Size, void* Destination, int Value)
  -> void
{
  std::memset(Destination, Value, ToBytes(Size));
}

auto mtb::
MemEqualBytes(memory_size Size, void const* A, void const* B)
  -> bool
{
  return MemCompareBytes(Size, A, B) == 0;
}

auto mtb::
MemCompareBytes(memory_size Size, void const* A, void const* B)
  -> int
{
  return std::memcmp(A, B, ToBytes(Size));
}

auto mtb::
MemAreOverlappingBytes(memory_size SizeA, void const* A, memory_size SizeB, void const* B)
  -> bool
{
  auto LeftA = Reinterpret<size_t const>(A);
  auto RightA = LeftA + ToBytes(SizeA);

  auto LeftB = Reinterpret<size_t const>(B);
  auto RightB = LeftB + ToBytes(SizeB);

  return LeftB  >= LeftA && LeftB  <= RightA || // Check if LeftB  is in [A, A+SizeA]
         RightB >= LeftA && RightB <= RightA || // Check if RightB is in [A, A+SizeA]
         LeftA  >= LeftB && LeftA  <= RightB || // Check if LeftA  is in [B, B+SizeB]
         RightA >= LeftB && RightA <= RightB;   // Check if RightA is in [B, B+SizeB]
}

#endif // !defiend(MTB_IMPL_memory)
#endif // defined(MTB_MEMORY_IMPLEMENTATION)

