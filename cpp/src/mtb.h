/**

Basic usage
===========
```
// Most of your .cpp files should include it like this:
#include "mtb.h"

// But in exactly ONE of your .cpp files you additionally need to define MTB_IMPLEMENTATION before you include it.
#define MTB_IMPLEMENTATION
#include "mtb.h"
```

There are some defines (like MTB_USE_LIBC) that you can configure before
including this file. You can find all options by searching for "#Option" in
this file.

Natvis support
==============
Take the contents of the comment at the end of this file and
save it in a `mtb.natvis` file somewhere reachable by your debugger.

*/

#if !defined(MTB__INCLUDED)
#define MTB__INCLUDED

//
// Enable usually disabled code for browsing with Visual Studio's Intellisense.
//
#if defined(__INTELLISENSE__)

#undef MTB_USE_STB_SPRINTF
#define MTB_USE_STB_SPRINTF 1
#include "stb_sprintf.h"

#endif

//
// Detect compiler
//
#define MTB_COMPILER_MSVC 0
#define MTB_COMPILER_CLANG 0

#if defined(_MSC_VER)
#undef MTB_COMPILER_MSVC
#define MTB_COMPILER_MSVC 1
#endif

#if defined(__clang__)
#undef MTB_COMPILER_CLANG
#define MTB_COMPILER_CLANG 1
#endif

// #Option
#if !defined(MTB_USE_LIBC)
#define MTB_USE_LIBC 1
#endif

// #Option
#if !defined(MTB_TESTS)
#if defined(DOCTEST_LIBRARY_INCLUDED)
#define MTB_TESTS 1
#else
#define MTB_TESTS 0
#endif
#endif

#if MTB_TESTS && defined(MTB_IMPLEMENTATION)
#define MTB_TEST_IMPLEMENTATION 1
#else
#define MTB_TEST_IMPLEMENTATION 0
#endif

// #Option
#if !defined(MTB_USE_STB_SPRINTF)
// Auto-detect whether stb_printf is available
#if defined(STB_SPRINTF_H_INCLUDE)
#define MTB_USE_STB_SPRINTF 1
#else
#define MTB_USE_STB_SPRINTF 0
#endif
#endif

#include <float.h>  // FLT_MAX, DBL_MAX, LDBL_MAX
#include <new>      // Placement-new
#include <stdarg.h> // va_list, va_start, va_end
#include <stdint.h> // uint8_t, uint16_t, ..., uintptr_t

#if MTB_USE_LIBC
#include <assert.h> // assert
#include <stdlib.h> // realloc
#include <string.h> // memcpy, memmove
#endif // MTB_USE_LIBC

// #Option
#if !defined(MTB_memcpy)
#define MTB_memcpy ::mtb::CopyBytes
#endif

// #Option
#if !defined(MTB_memmove)
#define MTB_memmove ::mtb::MoveBytes
#endif

// #Option
#if !defined(MTB_memset)
#define MTB_memset ::mtb::SetBytes
#endif

// #Option
#if !defined(MTB_memcmp)
#define MTB_memcmp ::mtb::CompareBytes
#endif

namespace mtb {
    void CopyBytes(void* dest, void const* src, size_t size);
    void MoveBytes(void* dest, void const* src, size_t size);
    void SetBytes(void* dest, int byte_value, size_t size);
    inline void SetZero(void* dest, size_t size) { SetBytes(dest, 0, size); }
    int CompareBytes(void const* a, void const* b, size_t size);
    inline bool BytesAreEqual(void const* a, void const* b, size_t size) { return 0 == CompareBytes(a, b, size); }
} // namespace mtb::impl

// #Option
// #TODO Create better assertion macro that accepts a format text.
#if !defined(MTB_ASSERT)
#if MTB_TESTS
#define MTB_ASSERT(...)                 \
    do                                  \
    {                                   \
        if(!(__VA_ARGS__))              \
        {                               \
            DOCTEST_FAIL(#__VA_ARGS__); \
        }                               \
    } while(false)
#elif MTB_USE_LIBC
#define MTB_ASSERT assert
#else
#define MTB_ASSERT(...)                        \
    do                                         \
    {                                          \
        if (!(__VA_ARGS__))                    \
        {                                      \
            /* Do something obviously stupid*/ \
            *(int*)nullptr = 0;                \
        }                                      \
    } while (false)
#endif
#endif

#if !defined(MTB_CONCAT)
#define MTB_CONCAT(a, b) MTB__CONCAT0(a, b)
#define MTB__CONCAT0(a, b) MTB__CONCAT1(a, b)
#define MTB__CONCAT1(a, b) a##b
#endif

#define MTB_sizeof(TYPE) (::mtb::tSizeOf<TYPE>::value)
namespace mtb
{
    // clang-format off
    template<typename T> struct tSizeOf                      { static const size_t value = sizeof(T); };
    template<>           struct tSizeOf<void>                { static const size_t value = 1; };
    template<>           struct tSizeOf<void const>          { static const size_t value = 1; };
    template<>           struct tSizeOf<void volatile>       { static const size_t value = 1; };
    template<>           struct tSizeOf<void volatile const> { static const size_t value = 1; };
    // clang-format on
} // namespace mtb

#define MTB_alignof(TYPE) (::mtb::tAlignOf<TYPE>::value)
namespace mtb
{
    // clang-format off
    template<typename T> struct tAlignOf                      { static const size_t value = alignof(T); };
    template<>           struct tAlignOf<void>                { static const size_t value = 1; };
    template<>           struct tAlignOf<void const>          { static const size_t value = 1; };
    template<>           struct tAlignOf<void volatile>       { static const size_t value = 1; };
    template<>           struct tAlignOf<void volatile const> { static const size_t value = 1; };
    // clang-format on
} // namespace mtb

// --------------------------------------------------
// -- #Section General Utilities --------------------
// --------------------------------------------------

/// Number of elements in a static array. Can be used in a static context.
///
/// You should prefer mtb::ArrayCount if possible since that one is type-safe.
///
/// Example:
///   struct tFoo {
///       uint32_t numbers[128];
///       uint8_t bytes[MTB_ARRAY_COUNT(numbers)]; // sizeof(bytes) == 128
///   };
///
/// Example:
///   uint32_t stuff[128];
///   for(ptrdiff_t index = 0; index < MTB_ARRAY_COUNT(stuff); ++index) {
///       printf("Stack@0x%x: %u \n", stuff + index, stuff[index]);
///   }
#define MTB_ARRAY_COUNT(ARRAY) (sizeof(::mtb::impl::ArrayCountHelper(ARRAY)))
#define MTB_ARRAY_SIZE(ARRAY) (sizeof(::mtb::impl::ArraySizeHelper(ARRAY)))
namespace mtb {
    namespace impl {
        template<typename T, size_t N>
        char (&ArrayCountHelper(T (&)[N]))[N];

        template<typename T, size_t N>
        T (&ArraySizeHelper(T (&array)[N]))[N];
    }

    template<typename T, size_t N>
    inline constexpr bool IsValidIndex(T (&array)[N], ptrdiff_t index) {
        return 0 <= index && static_cast<size_t>(index) < N;
    }

    template<typename T, size_t N>
    inline constexpr T& ArrayLast(T (&array)[N]) {
        return array[N - 1];
    }

    template<typename T>
    inline constexpr T* PtrOffset(T* ptr, ptrdiff_t offset) {
        return (T*)(((uintptr_t)ptr) + (offset * MTB_sizeof(T)));
    }

    template<typename T>
    inline constexpr ptrdiff_t PtrDistance(T* a, T* b) {
        return ((ptrdiff_t)(uintptr_t)a - (ptrdiff_t)(uintptr_t)b) / MTB_sizeof(T);
    }
} // namespace mtb

#if MTB_TESTS
namespace mtb_test_dump {
    struct tCountSizeThing {
        uint32_t integers[32];
        uint8_t foo[MTB_ARRAY_COUNT(integers)];
        uint8_t bar[MTB_ARRAY_SIZE(integers)];
    };
    static_assert(MTB_ARRAY_COUNT(tCountSizeThing::integers) == 32);
    static_assert(MTB_ARRAY_SIZE(tCountSizeThing::integers) == 128);
    static_assert(MTB_ARRAY_COUNT(tCountSizeThing::foo) == 32);
    static_assert(MTB_ARRAY_SIZE(tCountSizeThing::foo) == 32);
    static_assert(MTB_ARRAY_COUNT(tCountSizeThing::bar) == 128);
    static_assert(MTB_ARRAY_SIZE(tCountSizeThing::bar) == 128);
}
#endif

// --------------------------------------------------
// -- #Section Defer --------------------------------
// --------------------------------------------------
#define MTB_DEFER auto MTB_CONCAT(_mtb_defer, __LINE__) = ::mtb::impl::DeferHelper() = [&]()
namespace mtb::impl {
    struct DeferHelper {
        template<typename TDeferFunc>
        struct Impl {
            TDeferFunc& deferred;

            Impl() = delete;
            Impl(Impl const&) = delete;
            Impl& operator=(Impl const&) = delete;
            Impl& operator=(Impl&&) = delete;

            inline Impl(TDeferFunc&& in_deferred)
            : deferred{in_deferred} {}

            inline ~Impl() { deferred(); }
        };

        template<typename TDeferFunc>
        inline Impl<TDeferFunc> operator=(TDeferFunc&& in_deferred) {
            return Impl<TDeferFunc>{(TDeferFunc&&)in_deferred};
        }
    };
} // namespace mtb::impl

// --------------------------------------------------
// -- #Section POD - plain old data -----------------
// --------------------------------------------------
#if MTB_COMPILER_MSVC || MTB_COMPILER_CLANG
#define MTB_IS_POD_STRICT(...) (__is_pod(__VA_ARGS__))
#else
#include <type_traits>
#define MTB_IS_POD_STRICT(...) (::std::is_pod<__VA_ARGS__>::value)
#endif

#define MTB_IS_POD(...) (::mtb::tIsPOD<__VA_ARGS__>::value)
namespace mtb
{
    // clang-format off
    // #TODO `constexpr bool value`?
    template<typename T> struct tIsPOD                      { static const bool value = MTB_IS_POD_STRICT(T); };
    template<>           struct tIsPOD<void>                { static const bool value = true; };
    template<>           struct tIsPOD<void const>          { static const bool value = true; };
    template<>           struct tIsPOD<void volatile>       { static const bool value = true; };
    template<>           struct tIsPOD<void const volatile> { static const bool value = true; };
    // clang-format on
} // namespace mtb

namespace mtb
{
    namespace impl
    {
        // clang-format off
        template<typename T> struct tRemoveReference      { using tType = T; };
        template<typename T> struct tRemoveReference<T&>  { using tType = T; };
        template<typename T> struct tRemoveReference<T&&> { using tType = T; };
        // clang-format on
    } // namespace impl

    // clang-format off
    template<typename T> using tRemoveReference = typename impl::tRemoveReference<T>::tType;
    // clang-format on

    namespace impl
    {
        // clang-format off
        template<typename T> struct tRemovePointer            { using tType = T; };
        template<typename T> struct tRemovePointer<T*>        { using tType = T; };
        // clang-format on
    } // namespace impl

    // clang-format off
    template<typename T> using tRemovePointer = typename impl::tRemovePointer<T>::tType;
    // clang-format on

    namespace impl
    {
        // clang-format off
        template<typename T> struct tRemoveConst                   { using tType = T; };
        template<typename T> struct tRemoveConst<T const>          { using tType = T; };
        template<typename T> struct tRemoveConst<T const volatile> { using tType = T volatile; };
        // clang-format on
    } // namespace impl

    // clang-format off
    template<typename T> using tRemoveConst = typename impl::tRemoveConst<T>::tType;
    // clang-format on

    namespace impl
    {
        // clang-format on
        template<typename T> struct tDecay                    { using tType = T; };
        template<typename T> struct tDecay<T&>                { using tType = T; };
        template<typename T> struct tDecay<T const>           { using tType = T; };
        template<typename T> struct tDecay<T const&>          { using tType = T; };
        template<typename T> struct tDecay<T volatile>        { using tType = T; };
        template<typename T> struct tDecay<T volatile&>       { using tType = T; };
        template<typename T> struct tDecay<T volatile const>  { using tType = T; };
        template<typename T> struct tDecay<T volatile const&> { using tType = T; };
        // clang-format off
    }
    template<typename T> using tDecay = typename impl::tDecay<T>::tType;

    namespace impl {
        // clang-format on
        template<typename T> struct tIsPointer     { static constexpr bool value = false; };
        template<typename T> struct tIsPointer<T*> { static constexpr bool value = true; };
        // clang-format off
    }
    namespace traits {
        template<typename T> static constexpr bool is_pointer = impl::tIsPointer<tDecay<T>>::value;
    }

} // namespace mtb

namespace mtb
{
    // clang-format off
    template<typename T> constexpr T&& ForwardCast(tRemoveReference<T>&  value) { return static_cast<T&&>(value); }
    template<typename T> constexpr T&& ForwardCast(tRemoveReference<T>&& value) { return static_cast<T&&>(value); }
    template<typename T> constexpr tRemoveReference<T>&& MoveCast(T&& value) { return static_cast<tRemoveReference<T>&&>(value); }
    // clang-format on

    template<typename T>
    void Swap(T& a, T& b)
    {
        T Temp{MoveCast(a)};
        a = MoveCast(b);
        b = MoveCast(Temp);
    }
} // namespace mtb

// --------------------------------------------------
// -- #Section Type properties ----------------------
// --------------------------------------------------
namespace mtb
{
    template<typename T>
    struct tIntProperties
    {
        static constexpr bool is_signed = (((T)-1) < 0);
        static constexpr auto num_bits = sizeof(T) * 8;
        static constexpr T min_value = is_signed ? (T)((uint64_t)1 << (num_bits - 1)) : 0;
        static constexpr T max_value = (T)(~min_value);
    };

    template<typename TOut, typename TIn>
    inline TOut IntCast(TIn value)
    {
        TOut result = (TOut)value;
        MTB_ASSERT((TIn)result == value && "Cast would truncate.");
        return result;
    }

    template<typename T>
    struct tFloatProperties;

    template<>
    struct tFloatProperties<float>
    {
        static constexpr float max_value = FLT_MAX;
        static constexpr float min_value = -FLT_MAX;

        static constexpr float small_number = 0.0001f;
    };

    template<>
    struct tFloatProperties<double>
    {
        static constexpr double max_value = DBL_MAX;
        static constexpr double min_value = -DBL_MAX;
    };

    template<>
    struct tFloatProperties<long double>
    {
        static constexpr long double max_value = LDBL_MAX;
        static constexpr long double min_value = -LDBL_MAX;
    };
} // namespace mtb

// --------------------------------------------------
// -- #Section Item Construction --------------------
// --------------------------------------------------
namespace mtb
{
    enum eInit {
        kNoInit,
        kClearToZero,
    };

    // Optimized ops for POD types
    template<bool Pod = true>
    struct tItemOps {
        template<typename T>
        static void DefaultConstruct(T* items, size_t len) {
            MTB_memset(items, 0, len * MTB_sizeof(T));
        }

        template<typename T, typename U>
        static void CopyConstruct(T* dest, size_t dest_len, U* src, size_t src_len) {
            static_assert(MTB_sizeof(T) == MTB_sizeof(U), "POD types must have the same size.");
            MTB_ASSERT((MTB_sizeof(T) * dest_len) >= (MTB_sizeof(U) * src_len));
            MTB_memmove(dest, src, src_len * MTB_sizeof(U));
        }

        template<typename T, typename U>
        inline static void CopyAssign(T* dest, size_t dest_len, U* src, size_t src_len) {
            CopyConstruct(dest, dest_len, src, src_len);
        }

        inline static void SetConstruct(void* items, size_t len, int repeated_item) { MTB_memset(items, repeated_item, len); }
        inline static void SetConstruct(uint8_t* items, size_t len, int repeated_item) { MTB_memset(items, repeated_item, len); }
        inline static void SetConstruct(int8_t* items, size_t len, int repeated_item) { MTB_memset(items, repeated_item, len); }

        template<typename T, typename U>
        static void SetConstruct(T* items, size_t len, U repeated_item) {
            for(size_t index = 0; index < len; ++index) {
                new(items + index) T(repeated_item);
            }
        }

        template<typename T, typename U>
        static void SetAssign(T* items, size_t len, U repeated_item) {
            for(size_t index = 0; index < len; ++index) {
                items[index] = repeated_item;
            }
        }

        template<typename T>
        inline static void Destruct(T* items, size_t len) {
            // Nothing to do for POD types.
        }

        template<typename T, typename U>
        static void Relocate(T* dest, size_t dest_len, U* src, size_t src_len) {
            static_assert(MTB_sizeof(T) == MTB_sizeof(U), "POD types must have the same size.");
            MTB_ASSERT((MTB_sizeof(T) * dest_len) >= (MTB_sizeof(U) * src_len));
            MTB_memmove(dest, src, src_len * MTB_sizeof(U));
        }
    };

    // Slow ops for non-POD types
    template<>
    struct tItemOps<false> {
        template<typename T>
        static void DefaultConstruct(T* items, size_t len) {
            for (size_t index = 0; index < len; ++index) {
                new (items + index) T();
            }
        }

        template<typename T, typename U>
        static void CopyConstruct(T* dest, size_t dest_len, U* src, size_t src_len) {
            MTB_ASSERT(dest_len >= src_len);
            for(size_t index = 0; index < src_len; ++index) {
                new(dest + index) T(src[index]);
            }
        }

        template<typename T, typename U>
        static void CopyAssign(T* dest, size_t dest_len, U* src, size_t src_len) {
            MTB_ASSERT(dest_len >= src_len);
            for(size_t index = 0; index < src_len; ++index) {
                dest[index] = src[index];
            }
        }

        template<typename T, typename U>
        static void SetConstruct(T* items, size_t len, U repeated_item) {
            for(size_t index = 0; index < len; ++index) {
                new(items + index) T(repeated_item);
            }
        }

        template<typename T, typename U>
        static void SetAssign(T* items, size_t len, U repeated_item) {
            for(size_t index = 0; index < len; ++index) {
                items[index] = repeated_item;
            }
        }

        template<typename T>
        static void Destruct(T* items, size_t len) {
            for (size_t index = 0; index < len; ++index) {
                items[index].~T();
            }
        }

        template<typename T, typename U>
        static void Relocate(T* dest, U* src, size_t src_len) {
            for (size_t index = 0; index < src_len; ++index) {
                new (dest + index) T(src[index]); // #TODO Do we need MoveCast(src[index]) here?
                src[index].~U();
            }
        }
    };

    template<typename T>
    void DefaultConstructItems(T* items, size_t len) {
        tItemOps<MTB_IS_POD(T)>::DefaultConstruct(items, len);
    }

    template<typename T, typename U>
    void CopyConstructItems(T* dest, size_t dest_len, U* src, size_t src_len) {
        tItemOps<MTB_IS_POD(T)>::CopyConstruct(dest, dest_len, src, src_len);
    }

    template<typename T, typename U>
    void CopyAssignItems(T* dest, size_t dest_len, U* src, size_t src_len) {
        tItemOps<MTB_IS_POD(T)>::CopyAssign(dest, dest_len, src, src_len);
    }

    template<typename T, typename U>
    void SetConstructItems(T* items, size_t len, U repeated_item) {
        tItemOps<MTB_IS_POD(T)>::SetConstruct(items, len, repeated_item);
    }

    template<typename T, typename U>
    void SetAssignItems(T* items, size_t len, U repeated_item) {
        tItemOps<MTB_IS_POD(T)>::SetAssign(items, len, repeated_item);
    }

    template<typename T>
    void DestructItems(T* items, size_t len) {
        tItemOps<MTB_IS_POD(T)>::Destruct(items, len);
    }

    template<typename T, typename U>
    void RelocateItems(T* dest, size_t dest_len, U* src, size_t src_len) {
        tItemOps<MTB_IS_POD(T)>::Relocate(dest, dest_len, src, src_len);
    }
} // namespace mtb

// --------------------------------------------------
// -- #Section Slice --------------------------------
// --------------------------------------------------
namespace mtb
{
    // #Note slice specialization could make use of inheritance.
    //       However, some compilers no longer recognize such slices as POD types.
    //

    // #Note Due to conversion operators from mutable to const that we implement, the const versions of tSlice
    //       have to be declared before the mutable version. In other words, declaration order is important here.
    //       Don't rearrange.
    template<typename T>
    struct tSlice;

    // special case - void const
    template<>
    struct tSlice<void const>
    {
        void const* ptr;
        ptrdiff_t len;

        // C++ 11 range-based for interface
        constexpr void const* begin() const { return ptr; }
        // C++ 11 range-based for interface
        constexpr void const* end() const { return PtrOffset(ptr, len); }

        // C-array like pointer offset. slice + 3 is equivalent to slice.ptr + 3, except the former does bounds checking.
        inline void const* operator+(ptrdiff_t index) const {
            MTB_ASSERT(0 <= index && index < len);
            return PtrOffset(ptr, index);
        }

        inline constexpr explicit operator bool() const { return ptr && len > 0; }
    };

    // special case - void (mutable)
    template<>
    struct tSlice<void>
    {
        void* ptr;
        ptrdiff_t len;

        // C++ 11 range-based for interface
        constexpr void* begin() const { return ptr; }
        // C++ 11 range-based for interface
        constexpr void* end() const { return PtrOffset(ptr, len); }

        // C-array like pointer offset. slice + 3 is equivalent to slice.ptr + 3, except the former does bounds checking.
        inline void* operator+(ptrdiff_t index) const {
            MTB_ASSERT(0 <= index && index < len);
            return PtrOffset(ptr, index);
        }

        inline constexpr explicit operator bool() const { return ptr && len > 0; }

        // Implicit conversion to the const version.
        inline constexpr operator tSlice<void const>() const { return {ptr, len}; }
    };

    // special case - T const
    template<typename T>
    struct tSlice<T const>
    {
        T const* ptr;
        ptrdiff_t len;

        // C++ 11 range-based for interface
        constexpr T const* begin() const { return ptr; }
        // C++ 11 range-based for interface
        constexpr T const* end() const { return PtrOffset(ptr, len); }

        // C-array like pointer offset. slice + 3 is equivalent to slice.ptr + 3, except the former does bounds checking.
        inline T const* operator+(ptrdiff_t index) const {
            MTB_ASSERT(0 <= index && index < len);
            return PtrOffset(ptr, index);
        }

        inline constexpr explicit operator bool() const { return ptr && len > 0; }

        inline T const& operator[](ptrdiff_t index) const { return *(*this + index); }

        inline constexpr operator tSlice<void const>() const { return {ptr, (ptrdiff_t)(sizeof(T) * len)}; }
    };

    // general case - T (mutable)
    template<typename T>
    struct tSlice
    {
        T* ptr;
        ptrdiff_t len;

        // C++ 11 range-based for interface
        constexpr T* begin() const { return ptr; }
        // C++ 11 range-based for interface
        constexpr T* end() const { return PtrOffset(ptr, len); }

        // C-array like pointer offset. slice + 3 is equivalent to slice.ptr + 3, except the former does bounds checking.
        inline T* operator+(ptrdiff_t index) const {
            MTB_ASSERT(0 <= index && index < len);
            return PtrOffset(ptr, index);
        }

        inline constexpr explicit operator bool() const { return ptr && len > 0; }

        inline T& operator[](ptrdiff_t index) const { return *(*this + index); }

        // Implicit conversion to the const version.
        constexpr operator tSlice<T const>() const { return {ptr, len}; }
        constexpr operator tSlice<void>() const { return {ptr, (ptrdiff_t)(sizeof(T) * len)}; }
        constexpr operator tSlice<void const>() const { return {ptr, (ptrdiff_t)(sizeof(T) * len)}; }
    };

    static_assert(MTB_IS_POD_STRICT(tSlice<char>), "char slices must be PODs");
    static_assert(MTB_IS_POD_STRICT(tSlice<char const>), "char slices must be PODs");
    static_assert(MTB_IS_POD_STRICT(tSlice<void>), "void slices must be PODs");

    // --------------------------------------------------

    template<typename T>
    inline constexpr tSlice<T> PtrSlice(T* ptr, ptrdiff_t len) {
        return {ptr, len};
    }

    template<typename T>
    inline constexpr tSlice<T> PtrSliceBetween(T* start, T* end) {
        return {start, IntCast<ptrdiff_t>(((uintptr_t)end - (uintptr_t)start) / MTB_sizeof(T))};
    }

    template<typename T, size_t N>
    inline constexpr tSlice<T> ArraySlice(T (&array)[N]) {
        return {&array[0], N};
    }

    template<typename T, size_t N>
    inline constexpr tSlice<T> ArraySliceRange(T (&array)[N], ptrdiff_t offset, ptrdiff_t len)
    {
        return SliceRange(ArraySlice(array), offset, len);
    }

    template<typename T, size_t N>
    inline constexpr tSlice<T> ArraySliceBetween(T (&array)[N], ptrdiff_t first_index, ptrdiff_t one_past_last_index)
    {
        return SliceBetween(ArraySlice(array), first_index, one_past_last_index);
    }

    template<typename T, size_t N>
    inline constexpr tSlice<T> ArraySliceOffset(T (&array)[N], ptrdiff_t offset)
    {
        return SliceBetween(ArraySlice(array), offset, N);
    }

    template<typename T>
    inline constexpr tSlice<T> StructSlice(T& item) {
        static_assert(!traits::is_pointer<T>, "Invalid type. To create a slice from a pointer use PtrSlice()");
        return {&item, 1};
    }

    template<typename T>
    tSlice<T> SliceRange(tSlice<T> slice, ptrdiff_t offset, ptrdiff_t len) {
        // #TODO Should we just do a boundscheck here?
#if 1
        MTB_ASSERT(0 <= len && len <= slice.len + offset);
#else
        if (len < 0)
        {
            len = 0;
        }
        if (len > slice.len + offset)
        {
            len = slice.len - offset;
        }
#endif

        return PtrSlice(PtrOffset(slice.ptr, offset), len);
    }

    template<typename T>
    inline tSlice<T> SliceBetween(tSlice<T> slice, ptrdiff_t first_index, ptrdiff_t one_past_last_index)
    {
        return SliceRange(slice, first_index, one_past_last_index - first_index);
    }

    template<typename T>
    inline tSlice<T> SliceOffset(tSlice<T> slice, ptrdiff_t offset)
    {
        return SliceBetween(slice, offset, slice.len);
    }

    template<typename T>
    inline constexpr bool IsValidIndex(tSlice<T> slice, ptrdiff_t index) {
        return 0 <= index && index < slice.len;
    }

    /// Raw byte-size of the slice's underlying memory.
    template<typename T>
    inline constexpr size_t SliceSize(tSlice<T> slice) {
        return MTB_sizeof(T) * slice.len;
    }

    template<typename T>
    inline constexpr T* SliceLast(tSlice<T> slice) {
        return slice.len == 0 ? nullptr : PtrOffset(slice.ptr, slice.len - 1);
    }

    template<typename U, typename T>
    inline tSlice<U> SliceCast(tSlice<T> slice)
    {
        MTB_ASSERT((MTB_sizeof(T) * slice.len) % MTB_sizeof(U) == 0);
        return PtrSliceBetween(reinterpret_cast<U*>(slice.begin()), reinterpret_cast<U*>(slice.end()));
    }

    template<typename T>
    tSlice<T> SliceRemoveConst(tSlice<T const> slice) { return PtrSlice(const_cast<T*>(slice.ptr), slice.len); }

    template<typename D, typename S>
    void SliceCopyBytes(tSlice<D> dest, tSlice<S> src) {
        MTB_ASSERT(SliceSize(dest) >= SliceSize(src));
        MTB_memcpy(dest.ptr, src.ptr, SliceSize(src));
    }

    template<typename T>
    bool SliceIsZero(tSlice<T> slice) {
        tSlice<uint8_t const> bytes = PtrSlice((uint8_t const*)slice.ptr, SliceSize(slice));
        bool result = true;
        for (uint8_t byte : bytes) {
            if (byte) {
                result = false;
                break;
            }
        }
        return result;
    }

    template<typename T>
    inline void SliceSetZero(tSlice<T> slice)
    {
        SetZero(slice.ptr, MTB_sizeof(T) * slice.len);
    }

    inline void SliceReverseRaw(tSlice<void> slice)
    {
        uint8_t* byte_ptr = (uint8_t*)slice.ptr;
        size_t const num_swaps = slice.len / 2;
        for(size_t front_index = 0; front_index < num_swaps; ++front_index)
        {
            size_t const back_index = slice.len - front_index - 1;
            uint8_t to_swap = byte_ptr[front_index];
            byte_ptr[front_index] = byte_ptr[back_index];
            byte_ptr[back_index] = to_swap;
        }
    }

    template<typename T, typename U>
    bool SliceAreEqual(tSlice<T> a, tSlice<U> b)
    {
        bool result = false;
        if(a.len == b.len)
        {
            result = true;
            for(ptrdiff_t index = 0; index < a.len; ++index)
            {
                if(!(a.ptr[index] == b.ptr[index]))
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    bool SliceBytesAreEqual(tSlice<void const> a, tSlice<void const> b);

    int SliceCompareBytes(tSlice<void const> a, tSlice<void const> b);

    template<typename T, typename U>
    bool SliceStartsWith(tSlice<T> a, tSlice<U> b)
    {
        bool result = false;
        if(a.len >= b.len)
        {
            result = SliceAreEqual(SliceBetween(a, 0, b.len), b);
        }
        return result;
    }

    template<typename T, typename U>
    bool SliceEndsWith(tSlice<T> a, tSlice<U> b)
    {
        bool result = false;
        if(a.len >= b.len)
        {
            result = SliceAreEqual(SliceOffset(a, a.len - b.len), b);
        }
        return result;
    }

    template<typename T, typename TPredicate>
    tSlice<T> SliceTrimStartByPredicate(tSlice<T> slice, TPredicate Predicate)
    {
        ptrdiff_t offset = 0;
        for(T const& item : slice)
        {
            if(!Predicate(item))
            {
                break;
            }
            ++offset;
        }
        return SliceOffset(slice, offset);
    }

    template<typename T, typename TPredicate>
    tSlice<T> SliceTrimEndByPredicate(tSlice<T> slice, TPredicate Predicate)
    {
        ptrdiff_t len = slice.len;
        for(; len > 0; --len)
        {
            if(!Predicate(slice.ptr[len - 1]))
            {
                break;
            }
        }
        return SliceBetween(slice, 0, len);
    }

    template<typename T, typename TPredicate>
    tSlice<T> SliceTrimByPredicate(tSlice<T> slice, TPredicate Predicate)
    {
        return SliceTrimStartByPredicate(SliceTrimEndByPredicate(slice, Predicate), Predicate);
    }

    template<typename T, typename U>
    T* SliceFindItem(tSlice<T> haystack, U const& needle)
    {
        T* result = nullptr;
        for(ptrdiff_t index = 0; index < haystack.len; ++index)
        {
            T* item = haystack.ptr + index;
            if(*item == needle)
            {
                result = item;
                break;
            }
        }
        return result;
    }

    template<typename T, typename TPredicate>
    T* SliceFindByPredicate(tSlice<T> haystack, TPredicate Predicate)
    {
        T* result = nullptr;
        for(ptrdiff_t index = 0; index < haystack.len; ++index)
        {
            T* item = haystack.ptr + index;
            if(Predicate(*item))
            {
                result = item;
                break;
            }
        }
        return result;
    }

    template<typename T, typename U>
    T* SliceFindLastItem(tSlice<T> haystack, U const& needle)
    {
        T* result = nullptr;
        for(ptrdiff_t rIndex = haystack.len; rIndex > 0;)
        {
            T* item = haystack.ptr + --rIndex;
            if(*item == needle)
            {
                result = item;
                break;
            }
        }
        return result;
    }

    template<typename T, typename TPredicate>
    T* SliceFindLastByPredicate(tSlice<T> haystack, TPredicate Predicate)
    {
        T* result = nullptr;
        for(ptrdiff_t rIndex = haystack.len; rIndex > 0;)
        {
            T* item = haystack.ptr + --rIndex;
            if(Predicate(*item))
            {
                result = item;
                break;
            }
        }
        return result;
    }
} // namespace mtb

#if MTB_TEST_IMPLEMENTATION
namespace mtb {
    // static doctest::String toString(mtb::tSlice<char const> str) {
    //     return {static_cast<char const*>(str.ptr), static_cast<unsigned>(str.len)};
    // }
}

DOCTEST_TEST_SUITE("mtb::tSlice") {
    DOCTEST_TEST_CASE("Default-initialized slices") {
        mtb::tSlice<int> foo{};
        CHECK(foo.len == 0);
        CHECK(foo.ptr == nullptr);
    }

    DOCTEST_TEST_CASE("Implicit bool conversion") {
        mtb::tSlice<int> foo{};
        REQUIRE(static_cast<bool>(foo) == false);

        DOCTEST_SUBCASE("Set Num") {
            foo.len = 42;
            REQUIRE(static_cast<bool>(foo) == false);
        }

        DOCTEST_SUBCASE("Set ptr") {
            int dummy;
            foo.ptr = &dummy;
            REQUIRE(static_cast<bool>(foo) == false);
        }
    }

    DOCTEST_TEST_CASE("From pointer and length") {
        int foo = 42;
        auto foo_slice = mtb::PtrSlice(&foo, 1);
        CHECK(foo_slice.len == 1);
        CHECK(foo_slice[0] == 42);
    }

    DOCTEST_TEST_CASE("From begin and end pointer") {
        CHECK(mtb::PtrSliceBetween<int>(nullptr, nullptr).len == 0);
        CHECK(mtb::PtrSliceBetween<int>(nullptr, nullptr).ptr == nullptr);

        int foos[] = {0, 1, 2};

        mtb::tSlice<int> foo1 = mtb::PtrSliceBetween(&foos[0], &foos[0]);
        CHECK(foo1.len == 0);
        CHECK(foo1.ptr == &foos[0]);

        mtb::tSlice<int> foo2 = mtb::PtrSliceBetween(&foos[0], &foos[3]);
        CHECK(foo2.len == 3);
        CHECK(foo2[0] == foos[0]);
        CHECK(foo2[1] == foos[1]);
        CHECK(foo2[2] == foos[2]);
    }

    DOCTEST_TEST_CASE("From two indices") {
        int data_array[]{ 0, 1, 2 };
        mtb::tSlice<int> data = mtb::ArraySlice(data_array);

        mtb::tSlice<int> foo = mtb::SliceBetween(data, 0, 0);
        REQUIRE( foo.len == 0 );
        REQUIRE( foo.ptr == data.ptr );

        mtb::tSlice<int> bar = mtb::SliceBetween(data, 0, 1);
        REQUIRE( bar.len == 1 );
        REQUIRE( bar.ptr == data.ptr );

        mtb::tSlice<int> baz = mtb::SliceBetween(bar, 0, 1);
        REQUIRE( baz.len == 1 );
        REQUIRE( baz.ptr == data.ptr );

        mtb::tSlice<int> baar = mtb::SliceBetween(data, 1, 3);
        REQUIRE( baar.len == 2 );
        REQUIRE( baar.ptr == data.ptr + 1 );
    }

    DOCTEST_TEST_CASE("From static array") {
        int foo[] = {0, 1, 2};
        auto bar = mtb::ArraySlice(foo);
        CHECK(bar.len == MTB_ARRAY_COUNT(foo));
        CHECK(bar.ptr == &foo[0]);
    }

    DOCTEST_TEST_CASE("SliceCast")
    {
        char data[]{'a', 'b', 'c', 'd', 'e', 'f'};
        mtb::tSlice<char> chars = mtb::ArraySlice(data);
        CHECK(chars.len == 6);

        struct tFoo {
            char stuff[3];
        };
        static_assert(sizeof(tFoo) == 3, "Unexpected size of tFoo");

        struct tBar {
            char stuff[2];
        };
        static_assert(sizeof(tBar) == 2, "Unexpected size of tBar");

        tFoo foo_array[4] {
            tFoo{ { 'a', 'b', 'c' } },
            tFoo{ { 'd', 'e', 'f' } },
            tFoo{ { 'g', 'h', 'i' } },
            tFoo{ { 'j', 'k', 'l' } },
        };

        tBar bar_array[6] {
            tBar{ { 'A', 'B' } },
            tBar{ { 'C', 'D' } },
            tBar{ { 'E', 'F' } },
            tBar{ { 'G', 'H' } },
            tBar{ { 'I', 'J' } },
            tBar{ { 'K', 'L' } },
        };

        DOCTEST_SUBCASE("To void and back again") {
            mtb::tSlice<uint16_t> shorts = mtb::SliceCast<uint16_t>(chars);
            CHECK(shorts.len == 3);

            mtb::tSlice<void> raw = shorts;
            CHECK(raw.len == 6);
            uint8_t ref_byte = (uint8_t)'a';
            for (uint8_t byte : mtb::SliceCast<uint8_t>(raw)) {
                CHECK(byte == ref_byte++);
            }
        }

        DOCTEST_SUBCASE("Reinterpretation to smaller type") {
            mtb::tSlice<tFoo> foo = mtb::ArraySlice(foo_array);
            mtb::tSlice<tBar> bar = mtb::SliceCast<tBar>(foo);
            REQUIRE(bar.len == 6);
            REQUIRE(bar[0].stuff[0] == 'a');
            REQUIRE(bar[0].stuff[1] == 'b');
            REQUIRE(bar[1].stuff[0] == 'c');
            REQUIRE(bar[1].stuff[1] == 'd');
            REQUIRE(bar[2].stuff[0] == 'e');
            REQUIRE(bar[2].stuff[1] == 'f');
            REQUIRE(bar[3].stuff[0] == 'g');
            REQUIRE(bar[3].stuff[1] == 'h');
            REQUIRE(bar[4].stuff[0] == 'i');
            REQUIRE(bar[4].stuff[1] == 'j');
            REQUIRE(bar[5].stuff[0] == 'k');
            REQUIRE(bar[5].stuff[1] == 'l');
        }
        DOCTEST_SUBCASE("Reinterpretation to bigger type") {
            mtb::tSlice<tBar> bar = mtb::ArraySlice(bar_array);
            mtb::tSlice<tFoo> foo = mtb::SliceCast<tFoo>(bar);
            REQUIRE(foo[0].stuff[0] == 'A');
            REQUIRE(foo[0].stuff[1] == 'B');
            REQUIRE(foo[0].stuff[2] == 'C');
            REQUIRE(foo[1].stuff[0] == 'D');
            REQUIRE(foo[1].stuff[1] == 'E');
            REQUIRE(foo[1].stuff[2] == 'F');
            REQUIRE(foo[2].stuff[0] == 'G');
            REQUIRE(foo[2].stuff[1] == 'H');
            REQUIRE(foo[2].stuff[2] == 'I');
            REQUIRE(foo[3].stuff[0] == 'J');
            REQUIRE(foo[3].stuff[1] == 'K');
            REQUIRE(foo[3].stuff[2] == 'L');
        }
    }
}
#endif

// --------------------------------------------------
// -- #Section Memory Allocation --------------------
// --------------------------------------------------
#if !defined(MTB_ALLOCATOR_DEFAULT_ALIGNMENT)
#define MTB_ALLOCATOR_DEFAULT_ALIGNMENT 16
#endif

#if MTB_ALLOCATOR_DEFAULT_ALIGNMENT <= 0
#error "MTB_ALLOCATOR_DEFAULT_ALIGNMENT must be non-zero."
#endif

namespace mtb
{
    void AlignAllocation(void** inout_ptr, size_t* inout_size, size_t alignment);

    template<typename T>
    void ItemSetZero(T& item) { SetZero(&item, MTB_sizeof(T)); }

    template<typename T>
    bool ItemIsZero(T const& item) { return SliceIsZero(PtrSlice(&item, 1)); }

    /// Compare the bytes of a and b. Both types must be the same size in memory.
    template<typename T, typename U>
    bool StructEqualsRaw(T a, U b)
    {
        static_assert(MTB_sizeof(T) == MTB_sizeof(U), "Types must be of same size, otherwise an equality test of raw memory will not be useful.");
        return MTB_memcmp(&a, &b, MTB_sizeof(T)) == 0;
    }

    struct tAllocator
    {
        void* user;
        tSlice<void> (*realloc_proc)(void* user, tSlice<void> old_mem, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init);

        inline constexpr bool IsValid() const { return realloc_proc; }
        inline constexpr explicit operator bool() const { return realloc_proc; }

        // --------------------------------------------------
        // -- Raw Allocation --------------------------------
        // --------------------------------------------------
        tSlice<void> AllocRaw(size_t size, size_t alignment, eInit init) const;

        tSlice<void> ReallocRaw(tSlice<void> old_mem, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init) const;

        tSlice<void> DupeRaw(tSlice<void> mem, size_t alignment) const;

        void FreeRaw(tSlice<void> mem, size_t alignment) const;

        // --------------------------------------------------
        // -- Typed Array Allocation ------------------------
        // --------------------------------------------------
        template<typename T>
        tSlice<T> AllocArray(size_t len, eInit init = kClearToZero) const {
            tSlice<void> raw = AllocRaw(len * MTB_sizeof(T), MTB_alignof(T), init);
            return SliceCast<T>(raw);
        }

        template<typename T>
        tSlice<T> ResizeArray(tSlice<T> array, size_t new_len, eInit init = kClearToZero) const {
            tSlice<void> raw = ReallocRaw(array, MTB_alignof(T), new_len * MTB_sizeof(T), MTB_alignof(T), init);
            return SliceCast<T>(raw);
        }

        template<typename T>
        tSlice<T> DupeArray(tSlice<T> array) const {
            tSlice<void> raw = DupeRaw(array, MTB_alignof(T));
            return SliceCast<T>(raw);
        }

        template<typename T>
        void FreeArray(tSlice<T> array) const {
            FreeRaw(array, MTB_alignof(T));
        }

        // --------------------------------------------------
        // -- Typed Single-Item Allocation ------------------
        // --------------------------------------------------
        template<typename T>
        T* CreateOne(eInit init = kClearToZero) const {
            tSlice<void> raw = AllocRaw(MTB_sizeof(T), MTB_alignof(T), init);
            return SliceCast<T>(raw).ptr;
        }

        template<typename T>
        T* DupeOne(T& one) const {
            tSlice<void> raw = DupeRaw(StructSlice(one), MTB_alignof(T));
            return SliceCast<T>(raw).ptr;
        }

        template<typename T>
        void FreeOne(T* one) const {
            if (one) {
                FreeRaw(StructSlice(*one), MTB_alignof(T));
            }
        }
    };

#if MTB_USE_LIBC
    tAllocator GetLibcAllocator();
#endif

#if MTB_USE_STB_SPRINTF
    /// \remark Does not account for the null-terminator.
    /// \return The formatted string EXCLUDING the null-terminator.
    tSlice<char> vprintfAlloc(tAllocator a, char const* format, va_list vargs);

    /// \remark Does not account for the null-terminator.
    /// \return The formatted string EXCLUDING the null-terminator.
    inline tSlice<char> printfAlloc(tAllocator a, char const* format, ...)
    {
        va_list vargs;
        va_start(vargs, format);
        tSlice<char> result = vprintfAlloc(a, format, vargs);
        va_end(vargs);
        return result;
    }

    /// \return The formatted string INCLUDING the null-terminator.
    inline tSlice<char> vprintfAllocZ(tAllocator a, char const* format, va_list vargs)
    {
        tSlice<char> result = vprintfAlloc(a, format, vargs);
        return AppendArray(a, result, PtrSlice("\0", 1));
    }

    /// \return The formatted string INCLUDING the null-terminator.
    inline tSlice<char> printfAllocZ(tAllocator a, char const* format, ...)
    {
        va_list vargs;
        va_start(vargs, format);
        tSlice<char> result = vprintfAllocZ(a, format, vargs);
        va_end(vargs);
        return result;
    }
#endif // MTB_USE_STB_SPRINTF

} // namespace mtb

// --------------------------------------------------
// -- #Section Array --------------------------------
// --------------------------------------------------
namespace mtb
{
    template<typename T>
    struct tArray {
        /// May not be null.
        tAllocator allocator;

        /// May be null. Fallback employs amortized growth.
        ptrdiff_t (*calc_capacity)(tArray<T>& array, ptrdiff_t MinCapacity);

        union
        {
            struct
            {
                /// Pointer to first element of allocated array.
                T* ptr;
                /// Number of elements currently in use.
                ptrdiff_t len;
            };
            /// Convenient access of the slice currently in use.
            tSlice<T> items;
        };
        /// Number of allocated elements.
        ptrdiff_t cap;

        // --------------------------------------------------
        // --------------------------------------------------
        // --------------------------------------------------

        T* operator+(ptrdiff_t index) { return items + index; }
        T& operator[](ptrdiff_t index) { return items[index]; }
        inline constexpr explicit operator bool() const { return (bool)items; }

        inline T* begin() const { return items.begin(); }
        inline T* end() const { return items.end(); }
    };

    template<typename T>
    bool Reserve(tArray<T>& array, ptrdiff_t min_requested_capacity)
    {
        if(array.cap >= min_requested_capacity)
        {
            return true;
        }

        ptrdiff_t new_alloc_len;
        if(array.calc_capacity)
        {
            new_alloc_len = array.calc_capacity(array, min_requested_capacity);
            if(new_alloc_len < min_requested_capacity)
            {
                new_alloc_len = min_requested_capacity;
            }
        }
        else
        {
            new_alloc_len = array.cap > 0 ? array.cap : 16;
            while(new_alloc_len < min_requested_capacity)
            {
                new_alloc_len = (new_alloc_len * 3) / 2;
            }
        }
        tSlice<T> new_alloc = array.allocator.ResizeArray(PtrSlice(array.ptr, array.cap), new_alloc_len, kNoInit);
        if (new_alloc)
        {
            array.ptr = new_alloc.ptr;
            array.cap = new_alloc.len;
        }
        return !!new_alloc;
    }

    template<typename T>
    tSlice<T> ShrinkAllocation(tArray<T>& array)
    {
        tSlice<T> new_alloc = array.allocator.ResizeArray(PtrSlice(array.ptr, array.cap), array.len, kNoInit);
        array.ptr = new_alloc.ptr;
        array.cap = new_alloc.len;
        return array.items;
    }

    template<typename T>
    void Clear(tArray<T>& array)
    {
        array.len = 0;
    }

    template<typename T>
    void ClearAllocation(tArray<T>& array)
    {
        Clear(array);
        ShrinkAllocation(array);
    }

    template<typename T>
    bool SetLength(tArray<T>& array, ptrdiff_t new_count, eInit init = kClearToZero)
    {
        if(array.len < new_count)
        {
            if(Reserve(array, new_count))
            {
                tSlice<T> Fresh = SliceBetween(PtrSlice(array.ptr, array.cap), array.len, new_count);
                switch(init)
                {
                    case kClearToZero: SliceSetZero(SliceCast<void>(Fresh)); break;
                    case kNoInit: break;
                }
                array.len = new_count;
            }
        }
        else
        {
            array.len = new_count;
        }
        return array.len == new_count;
    }

    template<typename T>
    T* GetLast(const tArray<T>& array)
    {
        return array ? (array + (array.len - 1)) : nullptr;
    }

    template<typename T>
    tSlice<T> GetSlack(const tArray<T>& array)
    {
        return SliceOffset(PtrSlice(array.ptr, array.cap), array.len);
    }

    template<typename T>
    size_t ArraySize(const tArray<T>& array)
    {
        return SliceSize(array.items);
    }

    // InsertN      - N fresh items
    // InsertMany   - N copied items
    // InsertOne    - 1 fresh item
    // Insert       - 1 copied item
    // InsertRepeat - N fresh items, copied from a single item

    /// Create \a insert_count new items at the given index and return them as a slice.
    template<typename T>
    tSlice<T> InsertN(tArray<T>& array, ptrdiff_t insert_index, ptrdiff_t insert_count, eInit init = kClearToZero)
    {
        MTB_ASSERT(insert_count > 0);
        tSlice<T> result{};
        if(Reserve(array, array.len + insert_count))
        {
            MTB_ASSERT(IsValidIndex(array, insert_index) || insert_index == array.len);
            tSlice<T> Allocation = PtrSlice(array.ptr, array.cap);
            tSlice<T> result = SliceRange(Allocation, insert_index, insert_count);
            SliceRelocateItems(SliceOffset(Allocation, insert_index + insert_count), result);
            switch(init)
            {
                case kClearToZero: SliceSetZero(SliceCast<void>(result)); break;
                case kNoInit: break;
            }
            array.len += insert_count;
        }
        return result;
    }

    /// Create enough room for slice items to be copied to at the specified index.
    template<typename T, typename U>
    tSlice<T> InsertMany(tArray<T>& array, ptrdiff_t insert_index, tSlice<U> slice)
    {
        tSlice<T> result = InsertN(array, insert_index, slice.len, kNoInit);
        SliceCopyConstructItems(result, slice);
        return result;
    }

    /// Create a new item in the array and return a pointer to it.
    template<typename T>
    T* InsertOne(tArray<T>& array, ptrdiff_t insert_index, eInit init = kClearToZero)
    {
        return InsertN(array, insert_index, 1, init).ptr;
    }

    /// Create a new item in the array and copy \a item there.
    template<typename T, typename TItem>
    T* Insert(tArray<T>& array, ptrdiff_t insert_index, TItem item)
    {
        return new(InsertOne<T>(array, insert_index, kNoInit)) T(item);
    }

    /// Create \a repeat_count items in the array, initializing them all to the value of \a item.
    template<typename T, typename TItem>
    tSlice<T> InsertRepeat(tArray<T>& array, ptrdiff_t insert_index, TItem item, ptrdiff_t repeat_count)
    {
        tSlice<TItem> item_slice = PtrSlice(&item, 1);
        tSlice<T> result = InsertN(array, insert_index, repeat_count, kNoInit);
        for(ptrdiff_t index = 0; index < repeat_count; ++index)
        {
            SliceCopyConstructItems(SliceOffset(result, index), item_slice);
        }
        return result;
    }

    /// Create \a push_count new items at the end and return them as a slice.
    template<typename T>
    tSlice<T> PushN(tArray<T>& array, ptrdiff_t push_count, eInit init = kClearToZero)
    {
        tSlice<T> result{};
        if(Reserve(array, array.len + push_count))
        {
            tSlice<T> Allocation = PtrSlice(array.ptr, array.cap);
            result = SliceRange(Allocation, array.len, push_count);
            switch(init)
            {
                case kClearToZero: SliceSetZero(SliceCast<void>(result)); break;
                case kNoInit: break;
            }
            array.len += push_count;
        }
        return result;
    }

    /// Create enough room for slice items to be copied to.
    template<typename T, typename U>
    tSlice<T> PushMany(tArray<T>& array, tSlice<U> slice)
    {
        tSlice<T> result = PushN(array, slice.len, kNoInit);
        SliceCopyConstructItems(result, slice);
        return result;
    }

    /// Create a new item in the array and return a pointer to it.
    template<typename T>
    T* PushOne(tArray<T>& array, eInit init = kClearToZero)
    {
        return PushN(array, 1, init).ptr;
    }

    /// Create a new item in the array and copy \a item there.
    template<typename T, typename TItem>
    T* Push(tArray<T>& array, TItem item)
    {
        return new(PushOne<T>(array, kNoInit)) T(item);
    }

    /// Create \a repeat_count items in the array, initializing them all to the value of \a item.
    template<typename T, typename TItem>
    tSlice<T> PushRepeat(tArray<T>& array, TItem item, ptrdiff_t repeat_count)
    {
        tSlice<TItem> item_slice = PtrSlice(&item, 1);
        tSlice<T> result = PushN(array, repeat_count, kNoInit);
        for(ptrdiff_t index = 0; index < repeat_count; ++index)
        {
            SliceCopyConstructItems(SliceOffset(result, index), item_slice);
        }
        return result;
    }

#if MTB_USE_STB_SPRINTF
    tSlice<char> PushFormat(tArray<char>& array, char const* format, ...);
    tSlice<char> PushFormatV(tArray<char>& array, char const* format, va_list vargs);
#endif

    template<typename T>
    void RemoveAt(tArray<T>& array, ptrdiff_t remove_index, ptrdiff_t remove_count = 1, bool swap = false)
    {
        MTB_ASSERT(remove_count > 0);
        MTB_ASSERT(IsValidIndex(array.items, remove_index));
        MTB_ASSERT(IsValidIndex(array.items, remove_index + remove_count));
        if(swap)
        {
            SliceRelocateItems(SliceRange(array.items, remove_index, remove_count), SliceRange(array.items, array.len - remove_count, remove_count));
        }
        else
        {
            SliceRelocateItems(SliceOffset(array.items, remove_index), SliceOffset(array.items, remove_index + remove_count));
        }
        array.len -= remove_count;
    }

    template<typename T, typename P>
    void RemoveAll(tArray<T>& array, P Predicate, bool swap = false)
    {
        // #TODO
        MTB_ASSERT(false);
    }

    template<typename T, typename P>
    void RemoveFirst(tArray<T>& array, P Predicate, bool swap = false)
    {
        // #TODO
        MTB_ASSERT(false);
    }

    template<typename T, typename P>
    void RemoveLast(tArray<T>& array, P Predicate, bool swap = false)
    {
        // #TODO
        MTB_ASSERT(false);
    }

    tSlice<char> ToString(tArray<char> array, bool null_terminate = true);

} // namespace mtb

// --------------------------------------------------
// -- #Section Map ----------------------------------
// --------------------------------------------------
namespace mtb
{
    template<typename K, typename V>
    struct tMap;

    struct tMapSlot
    {
        enum eState : uint8_t
        {
            kFree,
            kOccupied,
            kDead,
        };

        eState State;
    };
    static_assert(sizeof(tMapSlot) == sizeof(uint8_t));

    template<typename T>
    struct tMapIterator_KeyOrValue
    {
        ptrdiff_t cap;
        tMapSlot* Slots;
        T* Items;
        ptrdiff_t index;

        bool operator!=(tMapIterator_KeyOrValue const& Other) const
        {
            return index != Other.index;
        }

        T& operator*() const
        {
            MTB_ASSERT(index < cap);
            return Items[index];
        }

        tMapIterator_KeyOrValue& operator++()
        {
            while (++index < cap)
            {
                if (Slots[index].State == tMapSlot::kOccupied)
                {
                    break;
                }
            }
            return *this;
        }

        tMapIterator_KeyOrValue begin()
        {
            tMapIterator_KeyOrValue result = *this;
            result.index = -1;
            return ++result;
        }

        tMapIterator_KeyOrValue end()
        {
            tMapIterator_KeyOrValue result = *this;
            result.index = cap;
            return result;
        }
    };

    // #TODO Return size_t instead? May be easier to leave it up to the user to take care of bitness.
    using tMapHashFunc = uint64_t(*)(void const* Key, size_t KeySize);
    using tMapCompareFunc = int(*)(void const* KeyA, void const* KeyB, size_t KeySize);

    template<typename K, typename V>
    struct tMap
    {
        /// May not be null.
        tAllocator allocator;

        /// May not be null.
        tMapHashFunc HashFunc;

        /// May be null. Fallback is memcpy-style compairson.
        tMapCompareFunc CompareFunc;

        /// Number of elements in the map.
        ptrdiff_t count;

        /// Number of elements the map could theoretically store. The map is resized before this value is reached.
        ptrdiff_t cap;

        /// Internal. array(N=cap) of slots in this map.
        tMapSlot* Slots;

        /// Internal. array(N=cap) of keys in this map.
        // #TODO Compute this based on Slots instead of storing it?
        K* Keys;

        /// Internal. array(N=cap) of values in this map.
        // #TODO Compute this based on Slots instead of storing it?
        V* Values;
    };
}

namespace mtb
{
    template<typename K, typename V>
    tMapIterator_KeyOrValue<K> IterKeys(tMap<K, V>& map);

    template<typename K, typename V>
    tMapIterator_KeyOrValue<V> IterValues(tMap<K, V>& map);

    template<typename K, typename V>
    tMap<K, V> CreateMap(tAllocator allocator, tMapHashFunc HashFunc, tMapCompareFunc CompareFunc = nullptr);

    template<typename K, typename V>
    void Put(tMap<K, V>& map, K const& Key, V const& value);

    template<typename K, typename V>
    V* Find(tMap<K, V>& map, K const& Key);

    template<typename K, typename V>
    V& FindChecked(tMap<K, V>& map, K const& Key);

    template<typename K, typename V>
    bool Remove(tMap<K, V>& map, K const& Key);
}

namespace mtb
{
    template<typename K, typename V>
    void InternalMapPut(tMap<K, V>& map, K const& Key, V const& value);

    template<typename K, typename V>
    void InternalEnsureAdditionalCapacity(tMap<K, V>& map, ptrdiff_t additional_len);
}

template<typename K, typename V>
void mtb::InternalMapPut(tMap<K, V>& map, K const& Key, V const& value)
{
    MTB_ASSERT(map.cap > 0);

    tMapSlot* Slots = map.Slots;
    K* Keys = map.Keys;
    V* Values = map.Values;
    ptrdiff_t StartIndex = (uint64_t)(map.HashFunc(&Key, sizeof(K)) % map.cap);
    ptrdiff_t index = StartIndex;
    bool bFound = false;
    while (true)
    {
        tMapSlot& Slot = Slots[index];
        if (Slot.State == tMapSlot::kFree || Slot.State == tMapSlot::kDead)
        {
            Slot.State = tMapSlot::kOccupied;
            tItemOps<MTB_IS_POD(K)>::CopyConstruct(Keys + index, 1, &Key, 1);
            tItemOps<MTB_IS_POD(K)>::CopyConstruct(Values + index, 1, &value, 1);
            ++map.len;
            bFound = true;
            break;
        }

        if (map.CompareFunc(Keys + index, &Key, sizeof(K)) == 0)
        {
            tItemOps<MTB_IS_POD(K)>::CopyConstruct(Values + index, 1, &value, 1);
            bFound = true;
            break;
        }

        if (++index == map.cap) index = 0;
        if (index == StartIndex) break;
    }

    MTB_ASSERT(bFound);
}

template<typename K, typename V>
void mtb::InternalEnsureAdditionalCapacity(tMap<K, V>& map, ptrdiff_t additional_len)
{
    MTB_ASSERT(map.allocator);

    ptrdiff_t threshold = (ptrdiff_t)(0.7f * map.cap);
    if (map.len + additional_len < threshold)
    {
        // We got enough space.
        return;
    }

    ptrdiff_t NewCapacity = map.cap == 0 ? 64 : map.cap << 1;
    size_t const alignment = MTB_alignof(V) > MTB_alignof(K) ? MTB_alignof(V) : MTB_alignof(K);
    size_t const PayloadSize = sizeof(tMapSlot) + sizeof(K) + sizeof(V);
    tSlice<void> new_alloc = map.allocator.AllocRaw(NewCapacity * PayloadSize, alignment, kClearToZero);

    tMap<K, V> new_map{};
    new_map.allocator = map.allocator;
    new_map.HashFunc = map.HashFunc;
    new_map.CompareFunc = map.CompareFunc;
    new_map.len = 0;
    new_map.cap = NewCapacity;
    new_map.Slots = (tMapSlot*)new_alloc.ptr;
    new_map.Keys = (K*)(new_map.Slots + NewCapacity);
    new_map.Values = (V*)(new_map.Keys + NewCapacity);

    for (ptrdiff_t index = 0; index < map.cap; ++index)
    {
        if (map.Slots[index].State == tMapSlot::kOccupied)
        {
            InternalMapPut(new_map, map.Keys[index], map.Values[index]);
        }
    }

    tSlice<void> OldAllocation = PtrSlice((void*)map.Slots, map.cap * PayloadSize);
    map.allocator.FreeRaw(OldAllocation, alignment);

    map = new_map;
}

template<typename K, typename V>
mtb::tMapIterator_KeyOrValue<K> mtb::IterKeys(tMap<K, V>& map)
{
    tMapIterator_KeyOrValue<K> result;
    result.cap = map.cap;
    result.Slots = map.Slots;
    result.Items = map.Keys;
    return result;
}

template<typename K, typename V>
mtb::tMapIterator_KeyOrValue<V> mtb::IterValues(tMap<K, V>& map)
{
    tMapIterator_KeyOrValue<V> result;
    result.cap = map.cap;
    result.Slots = map.Slots;
    result.Items = map.Values;
    return result;
}

template<typename K, typename V>
mtb::tMap<K, V> mtb::CreateMap(tAllocator allocator, tMapHashFunc HashFunc, tMapCompareFunc CompareFunc)
{
    MTB_ASSERT(allocator);
    MTB_ASSERT(HashFunc);
    if(!CompareFunc)
    {
        CompareFunc = MTB_memcmp;
    }
    return{ allocator, HashFunc, CompareFunc };
}

template<typename K, typename V>
void mtb::Put(tMap<K, V>& map, K const& Key, V const& value)
{
    InternalEnsureAdditionalCapacity(map, 1);
    InternalMapPut(map, Key, value);
}

template<typename K, typename V>
V* mtb::Find(tMap<K, V>& map, K const& Key)
{
    V* result = nullptr;
    if(map.len)
    {
        ptrdiff_t StartIndex = (uint64_t)(map.HashFunc(&Key, sizeof(K)) % map.cap);
        ptrdiff_t index = StartIndex;
        while(map.Slots[index].State != tMapSlot::kFree)
        {
            tMapSlot& Slot = map.Slots[index];
            if(Slot.State == tMapSlot::kFree) break;

            if(Slot.State == tMapSlot::kOccupied && map.CompareFunc(map.Keys + index, &Key, sizeof(K)) == 0)
            {
                result = map.Values + index;
                break;
            }

            if(++index == map.cap) index = 0;
            if(index == StartIndex) break;
        }
    }

    return result;
}

template<typename K, typename V>
V& mtb::FindChecked(tMap<K, V>& map, K const& Key)
{
    V* value = Find(map, Key);
    MTB_ASSERT(value);
    return *value;
}

template<typename K, typename V>
bool mtb::Remove(tMap<K, V>& map, K const& Key)
{
    bool result = false;
    if (map.len)
    {
        ptrdiff_t StartIndex = (uint64_t)(map.HashFunc(&Key, sizeof(K)) % map.cap);
        ptrdiff_t index = StartIndex;
        while(true)
        {
            tMapSlot& Slot = map.Slots[index];
            if (Slot.State == tMapSlot::kFree) break;

            if (map.Slots[index].State == tMapSlot::kOccupied && map.CompareFunc(map.Keys + index, &Key, sizeof(K)) == 0)
            {
                map.Slots[index].State = tMapSlot::kDead;
                tItemOps<MTB_IS_POD(K)>::Destruct(map.Keys + index, 1);
                tItemOps<MTB_IS_POD(K)>::Destruct(map.Values + index, 1);
                result = true;
                break;
            }

            if (++index == map.cap) index = 0;
            if (index == StartIndex) break;
        }
    }

    return result;
}

// --------------------------------------------------
// -- #Section Delegate -----------------------------
// --------------------------------------------------
namespace mtb
{
    template<typename>
    struct tDelegate;

    template<typename R, typename... tArgs>
    struct tDelegate<R(tArgs...)>
    {
        using tDispatchSignature = R(void*, tArgs&&...);
        using tDelegateSignature = R(tArgs&&...);

        void* DispatchTarget{};
        tDispatchSignature* DispatchProc{};

        template<typename tTargetSignature>
        inline static R Dispatcher(void* DispatchTarget, tArgs&&... args)
        {
            return (*(tTargetSignature*)DispatchTarget)(ForwardCast<tArgs>(args)...);
        }

        tDelegate() {}

        tDelegate(tDelegateSignature* PlainProcPtr)
        : DispatchTarget{PlainProcPtr}
        , DispatchProc{Dispatcher<tDelegateSignature>}
        {
            MTB_ASSERT(PlainProcPtr);
        }

        template<typename T>
        tDelegate(T&& CallableTarget)
        : DispatchTarget{&CallableTarget}
        , DispatchProc{Dispatcher<tDecay<T>>}
        {
        }

        inline R operator()(tArgs&&... args)
        {
            MTB_ASSERT(DispatchProc && DispatchTarget);
            return DispatchProc(DispatchTarget, ForwardCast<tArgs>(args)...);
        }

        inline constexpr explicit operator bool() const
        {
            return !!DispatchTarget && !!DispatchProc;
        }
    };
} // namespace mtb

// --------------------------------------------------
// -- #Section Sorting ------------------------------
// --------------------------------------------------
namespace mtb
{
    // Quick sort sorting algorithm. This procedure does not require to know the actual data. It relies on the fact that
    // less_proc and swap_proc are able to produce the desired information/effect required for effectively sorting
    // something. This implementation works on all data structures that work using indices. In other words, you can sort
    // data in linked lists if you can index each node, or sort a discontiguous (chunked) array.
    //
    // \remark This sort is not stable.
    //
    // \param user_ptr will be passed through to \a less_proc and \a swap_proc as-is. Can be null.
    // \param count The number of items to sort. In other words, this is the first invalid index into whatever is being sorted.
    // \param less_proc Whether element at the first index is considered less than the element at the second index.
    // \param threshold Partitions with a count equal to or less than this value will be sorted with insertion sort.
    void QuickSort(void* user_ptr, ptrdiff_t count, bool (*less_proc)(void*, ptrdiff_t, ptrdiff_t), void (*swap_proc)(void*, ptrdiff_t, ptrdiff_t), ptrdiff_t threshold);

    template<typename T, typename tLessProc, typename tSwapProc>
    inline void QuickSortSlice(tSlice<T> slice, tLessProc less_proc, tSwapProc swap_proc, ptrdiff_t threshold = 16)
    {
        struct tContext
        {
            T* ptr;
            tLessProc Less;
            tSwapProc Swap;
        } Context{slice.ptr, less_proc, swap_proc};
        QuickSort(&Context,
                  slice.len,
                  [](void* C, ptrdiff_t I, ptrdiff_t J) {
                      tContext* Context = (tContext*)C;
                      return Context->Less(Context->ptr[I], Context->ptr[J]);
                  },
                  [](void* C, ptrdiff_t I, ptrdiff_t J) {
                      tContext* Context = (tContext*)C;
                      Context->Swap(Context->ptr[I], Context->ptr[J]);
                  },
                  threshold);
    }

    template<typename T, typename tLessProc>
    inline void QuickSortSlice(tSlice<T> slice, tLessProc less_proc, ptrdiff_t threshold = 16)
    {
        struct tContext
        {
            T* ptr;
            tLessProc Less;
        } Context{slice.ptr, less_proc};
        QuickSort(&Context,
                  slice.len,
                  [](void* C, ptrdiff_t I, ptrdiff_t J) {
                      tContext* Context = (tContext*)C;
                      return Context->Less(Context->ptr[I], Context->ptr[J]);
                  },
                  [](void* C, ptrdiff_t I, ptrdiff_t J) {
                      tContext* Context = (tContext*)C;
                      ::mtb::Swap(Context->ptr[I], Context->ptr[J]);
                  },
                  threshold);
    }

    template<typename T>
    inline void QuickSortSlice(tSlice<T> slice, ptrdiff_t threshold = 16)
    {
        QuickSort(slice.ptr,
                  slice.len,
                  [](void* ptr, ptrdiff_t i, ptrdiff_t j) { return ((T*)ptr)[i] < ((T*)ptr)[j]; },
                  [](void* ptr, ptrdiff_t i, ptrdiff_t j) { ::mtb::Swap(((T*)ptr)[i], ((T*)ptr)[j]); },
                  threshold);
    }
} // namespace mtb

// #Note I tried using tOption. In general, I would like something like that
// very much. However, it's so hard to correctly implement in C++ that I don't
// think it's worth the effort. one would have to verify on all compilers that
// it actually behaves as expected with regards to implicit casting and move
// semantics, and that it doesn't allow implicit casts from T to
// tOption<T> etc. There's too much uncertainty on little details that could
// go wrong. So I just disable this here. Maybe at some point in the future
// (C++20 and beyond?) it might be easier to achieve this kind of thing.
#if 1
// --------------------------------------------------
// -- #Section Option -------------------------------
// --------------------------------------------------
namespace mtb
{
    template<typename T>
    struct tOption
    {
        bool has_value;
        union {
            T value;
        };

        inline tOption()
        : has_value{false} {}

        inline tOption(T InValue)
        : has_value{true}, value{MoveCast(InValue)} {}

        inline tOption(T&& InValue)
        : has_value{true}, value{MoveCast(InValue)} {}

        inline tOption(tOption const& to_copy)
        : has_value{to_copy.has_value}
        {
            if (to_copy.has_value)
            {
                new (&value) T(to_copy.value);
            }
        }

        inline tOption(tOption&& ToMove)
        : has_value{ToMove.has_value}
        {
            if (ToMove.has_value)
            {
                new (&value) T(MoveCast(ToMove.value));
            }
        }

        inline ~tOption()
        {
            if (has_value)
            {
                value.~T();
            }
        }

        tOption& operator=(tOption const& to_copy)
        {
            if (&to_copy != this)
            {
                if (!has_value && !to_copy.has_value)
                {
                    // value stays untouched.
                }
                else if (has_value && to_copy.has_value)
                {
                    // we had a value, so copy directly.
                    value = to_copy.value;
                }
                else if (!has_value && to_copy.has_value)
                {
                    // we didn't have a value before, so construct.
                    new (&value) T(to_copy.value);
                    has_value = true;
                }
                else if (has_value && !to_copy.has_value)
                {
                    // we had a value, so destroy.
                    value.~T();
                    has_value = false;
                }
            }

            return *this;
        }

        tOption& operator=(tOption&& ToMove)
        {
            if (&ToMove != this)
            {
                if (!has_value && !ToMove.has_value)
                {
                    // value stays untouched.
                }
                else if (has_value && ToMove.has_value)
                {
                    // we had a value, so copy directly.
                    value = (T &&) ToMove.value;
                }
                else if (!has_value && ToMove.has_value)
                {
                    // we didn't have a value before, so construct.
                    new (&value) T((T &&) ToMove.value);
                    has_value = true;
                }
                else if (has_value && !ToMove.has_value)
                {
                    // we had a value, so destroy.
                    value.~T();
                    has_value = false;
                }
            }

            return *this;
        }

        constexpr operator bool() const { return has_value; }
    };

    template<typename T>
    constexpr bool operator==(tOption<T> const& a, tOption<T> const& b)
    {
        return (!a.has_value && !b.has_value) || (a.has_value && b.has_value && a.value == b.value);
    }

    template<typename T>
    constexpr bool operator!=(tOption<T> const& a, tOption<T> const& b)
    {
        return !(a == b);
    }

    template<typename T>
    T& Unwrap(tOption<T>& Option)
    {
        MTB_ASSERT(Option.has_value);
        return Option.value;
    }

    template<typename T>
    T Unwrap(tOption<T> Option)
    {
        MTB_ASSERT(Option.has_value);
        return Option.value;
    }
} // namespace mtb
#endif

// --------------------------------------------------
// -- #Section Arena --------------------------------
// --------------------------------------------------

// #Option
#if !defined(MTB_ARENA_DEFAULT_BUCKET_SIZE)
#define MTB_ARENA_DEFAULT_BUCKET_SIZE 4096
#endif

namespace mtb::arena
{
    struct tBucket
    {
        tBucket* next;
        tBucket* prev;
        size_t used_size;
        size_t total_size;
        uint8_t data[1]; // trailing data
    };

    struct tArenaMarker
    {
        tBucket* bucket;
        size_t offset;

        inline constexpr uint8_t* ptr()
        {
            return bucket ? bucket->data + offset : nullptr;
        }
    };

    // #TODO struct tArenaStats { size_t largest_bucket_size; size_t NumBucketAllocations; size_t NumBucketFrees; };

    struct tArena
    {
        tAllocator child_allocator;
        size_t min_bucket_size;
        tBucket* current_bucket;
        tBucket* first_free_bucket;

        size_t largest_bucket_size;
    };

    size_t BucketTotalSize(tBucket* bucket);

    size_t BucketUsedSize(tBucket* bucket);

    void Reserve(tArena& arena, size_t total_size);

    void Grow(tArena& arena, size_t required_size);

    void Clear(tArena& arena, bool release_memory = true);

    void* ReallocRaw(tArena& arena, void* old_ptr, size_t old_size, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init);

    void* PushRaw(tArena& arena, size_t size, size_t alignment, eInit init);

    tSlice<void> ReallocRawArray(tArena& arena, tSlice<void> old_array, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init);

    tSlice<void> PushRawArray(tArena& arena, size_t size, size_t alignment, eInit init);

    template<typename T>
    tSlice<T> ReallocArray(tArena& arena, tSlice<T> old_array, size_t new_count, eInit init = kClearToZero)
    {
        size_t new_size = MTB_sizeof(T) * new_count;
        size_t alignment = MTB_alignof(T);
        void* ptr = ReallocRaw(arena, old_array.ptr, SliceSize(old_array), alignment, new_size, alignment, init);
        tSlice<T> result = PtrSlice((T*)ptr, new_count);
        if (init == kClearToZero)
        {
            SliceSetZero(SliceOffset(result, old_array.len));
        }
        return result;
    }

    template<typename T>
    tSlice<T> PushArray(tArena& arena, size_t count, eInit init = kClearToZero)
    {
        return ReallocArray<T>(arena, {}, count, init);
    }

    template<typename T, typename U = tRemoveConst<T>>
    tSlice<U> PushCopyArray(tArena& arena, tSlice<T> to_copy)
    {
        tSlice<U> Copy = PushArray<U>(arena, to_copy.len, kNoInit);
        SliceCopyConstructItems(Copy, to_copy);
        return Copy;
    }

    // The same as PushCopyArray with one additional zero-element at the end.
    template<typename T, typename U = tRemoveConst<T>>
    tSlice<U> PushCopyString(tArena& arena, tSlice<T> to_copy)
    {
        tSlice<U> ZeroTerminatedCopy = PushArray<U>(arena, to_copy.len + 1, kNoInit);
        tSlice<U> result = SliceRange(ZeroTerminatedCopy, 0, ZeroTerminatedCopy.len - 1);
        SliceCopyConstructItems(result, to_copy);
        // set terminator element.
        new (PtrOffset(ZeroTerminatedCopy.ptr, ZeroTerminatedCopy.len - 1)) U{};
        return result;
    }

    template<typename T>
    T* PushOne(tArena& arena, eInit init = kClearToZero)
    {
        return PushArray<T>(arena, 1, init).ptr;
    }

    template<typename T>
    T* PushCopy(tArena& arena, T const& item)
    {
        T* result = PushOne<T>(arena, kNoInit);
        SliceCopyConstructItems(PtrSlice(result, 1), PtrSlice(&item, 1));
        return result;
    }

    /* #TODO
     - NumBuckets?
     - accumulated total_size across all buckets?
     - accumulated used_size across all buckets?
    */

    /// \remark Only valid before free was called.
    tArenaMarker GetMarker(tArena& arena);
    void ResetToMarker(tArena& arena, tArenaMarker marker, bool release_memory = true);

    /// Ensure the memory in the given range is contiguous.
    /// Returns the marker to the beginning of the linearized section.
    tSlice<void> Linearize(tArena& arena, tArenaMarker begin, tArenaMarker end);

    tAllocator MakeAllocator(tArena& arena);

#if MTB_USE_STB_SPRINTF
    /// \brief Produce several fragments of formatted strings within the given arena. Use `*printf_Arena` or `Linearize` to produce an actual string.
    void vprintf_ArenaRaw(tArena& arena, char const* format, va_list vargs);

    // Append a string with the given format inside the given arena.
    inline void printf_ArenaRaw(tArena& arena, char const* format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf_ArenaRaw(arena, format, args);
        va_end(args);
    }

    /// \brief Append a string with the given format inside the given arena. Produces a null-terminated string.
    /// \return The formatted string EXCLUDING the null-terminator.
    tSlice<char> vprintf_Arena(tArena& arena, char const* format, va_list vargs);

    // Append a string with the given format inside the given arena.
    inline tSlice<char> printf_Arena(tArena& arena, char const* format, ...)
    {
        va_list args;
        va_start(args, format);
        tSlice<char> result = vprintf_Arena(arena, format, args);
        va_end(args);

        return result;
    }

#endif // MTB_USE_STB_SPRINTF

} // namespace mtb::arena

// --------------------------------------------------
// -- #Section Strings ------------------------------
// --------------------------------------------------
namespace mtb::string
{
    // #TODO I want UTF-8 support :(

    enum eStringComparison
    {
        kCaseSensitive,
        kIgnoreCase,
    };

    // #TODO
    // struct tString {
    //     union {
    //         struct {
    //             uint8_t const* ptr;
    //             ptrdiff_t len;
    //         };
    //         tSlice<uint8_t const> data;
    //     };
    // };

    // #TODO
    // struct tStringBuilder {
    //     tArray<uint8_t> buf;
    // };
    // tStringBuilder StringBuilder(tAllocator a);

    char ToLowerChar(char c);
    char ToUpperChar(char c);

    bool IsDigitChar(char c);
    bool IsWhiteChar(char c);

    int32_t StringCompare(tSlice<char const> str_a, tSlice<char const> str_b, eStringComparison cmp = kCaseSensitive);

    bool StringEquals(tSlice<char const> str_a, tSlice<char const> str_b, eStringComparison cmp = kCaseSensitive);

    bool StringStartsWith(tSlice<char const> str, tSlice<char const> prefix, eStringComparison cmp = kCaseSensitive);

    bool StringEndsWith(tSlice<char const> str, tSlice<char const> prefix, eStringComparison cmp = kCaseSensitive);

    using tTrimPredicate = bool (*)(char);

    tSlice<char const> StringTrimStartPredicate(tSlice<char const> str, tTrimPredicate Predicate);
    inline tSlice<char> StringTrimStartPredicate(tSlice<char> str, tTrimPredicate Predicate) { return SliceRemoveConst(StringTrimStartPredicate((tSlice<char const>)str, Predicate)); }

    tSlice<char const> StringTrimEndPredicate(tSlice<char const> str, tTrimPredicate Predicate);
    inline tSlice<char> StringTrimEndPredicate(tSlice<char> str, tTrimPredicate Predicate) { return SliceRemoveConst(StringTrimEndPredicate((tSlice<char const>)str, Predicate)); }

    tSlice<char const> StringTrimPredicate(tSlice<char const> str, tTrimPredicate Predicate);
    inline tSlice<char> StringTrimPredicate(tSlice<char> str, tTrimPredicate Predicate) { return SliceRemoveConst(StringTrimPredicate((tSlice<char const>)str, Predicate)); }

    tSlice<char const> StringTrimStart(tSlice<char const> str);
    inline tSlice<char> StringTrimStart(tSlice<char> str) { return SliceRemoveConst(StringTrimStart((tSlice<char const>)str)); }

    tSlice<char const> StringTrimEnd(tSlice<char const> str);
    inline tSlice<char> StringTrimEnd(tSlice<char> str) { return SliceRemoveConst(StringTrimEnd((tSlice<char const>)str)); }

    tSlice<char const> StringTrim(tSlice<char const> str);
    inline tSlice<char> StringTrim(tSlice<char> str) { return SliceRemoveConst(StringTrim((tSlice<char const>)str)); }

    tSlice<char> AllocString(tAllocator a, size_t char_count);
    tSlice<char> DupeString(tAllocator a, tSlice<char const> str);
    void FreeString(tAllocator a, tSlice<char> str);

    //
    // Wide char overloads
    //
    // #TODO Drop support for this and instead require the user code to
    //       convert from wide strings to narrow strings first if they
    //       want to use our functionality.

    wchar_t ToLowerChar(wchar_t c);
    wchar_t ToUpperChar(wchar_t c);

    bool IsDigitChar(wchar_t c);
    bool IsWhiteChar(wchar_t c);

    int32_t StringCompare(tSlice<wchar_t const> str_a, tSlice<wchar_t const> str_b, eStringComparison cmp = kCaseSensitive);

    bool StringEquals(tSlice<wchar_t const> str_a, tSlice<wchar_t const> str_b, eStringComparison cmp = kCaseSensitive);

    bool StringStartsWith(tSlice<wchar_t const> str, tSlice<wchar_t const> prefix, eStringComparison cmp = kCaseSensitive);

    bool StringEndsWith(tSlice<wchar_t const> str, tSlice<wchar_t const> prefix, eStringComparison cmp = kCaseSensitive);

    using tWTrimPredicate = bool (*)(wchar_t);

    tSlice<wchar_t const> StringTrimStartPredicate(tSlice<wchar_t const> str, tWTrimPredicate Predicate);
    inline tSlice<wchar_t> StringTrimStartPredicate(tSlice<wchar_t> str, tWTrimPredicate Predicate) { return SliceRemoveConst(StringTrimStartPredicate((tSlice<wchar_t const>)str, Predicate)); }

    tSlice<wchar_t const> StringTrimEndPredicate(tSlice<wchar_t const> str, tWTrimPredicate Predicate);
    inline tSlice<wchar_t> StringTrimEndPredicate(tSlice<wchar_t> str, tWTrimPredicate Predicate) { return SliceRemoveConst(StringTrimEndPredicate((tSlice<wchar_t const>)str, Predicate)); }

    tSlice<wchar_t const> StringTrimPredicate(tSlice<wchar_t const> str, tWTrimPredicate Predicate);
    inline tSlice<wchar_t> StringTrimPredicate(tSlice<wchar_t> str, tWTrimPredicate Predicate) { return SliceRemoveConst(StringTrimPredicate((tSlice<wchar_t const>)str, Predicate)); }

    tSlice<wchar_t const> StringTrimStart(tSlice<wchar_t const> str);
    inline tSlice<wchar_t> StringTrimStart(tSlice<wchar_t> str) { return SliceRemoveConst(StringTrimStart((tSlice<wchar_t const>)str)); }

    tSlice<wchar_t const> StringTrimEnd(tSlice<wchar_t const> str);
    inline tSlice<wchar_t> StringTrimEnd(tSlice<wchar_t> str) { return SliceRemoveConst(StringTrimEnd((tSlice<wchar_t const>)str)); }

    tSlice<wchar_t const> StringTrim(tSlice<wchar_t const> str);
    inline tSlice<wchar_t> StringTrim(tSlice<wchar_t> str) { return SliceRemoveConst(StringTrim((tSlice<wchar_t const>)str)); }

    tSlice<wchar_t> AllocWString(tAllocator a, size_t char_count);
    tSlice<wchar_t> DupeString(tAllocator a, tSlice<wchar_t const> str);
    void FreeWString(tAllocator a, tSlice<wchar_t> str);

    //
    // C-str interop (char)
    //
    size_t StringLengthZ(char const* str_z);

    inline tSlice<char> WrapZ(char* str_z) { return PtrSlice(str_z, StringLengthZ(str_z)); }

    inline tSlice<char const> ConstZ(char const* str_z) { return PtrSlice(str_z, StringLengthZ(str_z)); }

    //
    // C-str interop (wchar_t)
    //
    size_t StringLengthZ(wchar_t const* str_z);

    inline tSlice<wchar_t> WrapZ(wchar_t* str_z) { return PtrSlice(str_z, StringLengthZ(str_z)); }

    inline tSlice<wchar_t const> ConstZ(wchar_t const* str_z) { return PtrSlice(str_z, StringLengthZ(str_z)); }
} // namespace mtb::string

// --------------------------------------------------
// -- #Section Units --------------------------------
// --------------------------------------------------
namespace mtb
{
    const uint64_t kibibytes_to_bytes = 1024ULL;
    const uint64_t mebibytes_to_bytes = 1024ULL * kibibytes_to_bytes;
    const uint64_t gibibytes_to_bytes = 1024ULL * mebibytes_to_bytes;
    const uint64_t tebibytes_to_bytes = 1024ULL * gibibytes_to_bytes;
    const uint64_t pebibytes_to_bytes = 1024ULL * tebibytes_to_bytes;
    const uint64_t exbibytes_to_bytes = 1024ULL * pebibytes_to_bytes;

    const uint64_t kilobytes_to_bytes = 1000ULL;
    const uint64_t megabytes_to_bytes = 1000ULL * kilobytes_to_bytes;
    const uint64_t gigabytes_to_bytes = 1000ULL * megabytes_to_bytes;
    const uint64_t terabytes_to_bytes = 1000ULL * gigabytes_to_bytes;
    const uint64_t petabytes_to_bytes = 1000ULL * terabytes_to_bytes;
    const uint64_t exabytes_to_bytes = 1000ULL * petabytes_to_bytes;

    // clang-format off
    const uint64_t microseconds_to_nanoseconds = 1000ULL;
    const uint64_t milliseconds_to_nanoseconds = 1000ULL * microseconds_to_nanoseconds;
    const uint64_t seconds_to_nanoseconds      = 1000ULL * milliseconds_to_nanoseconds;
    const uint64_t minutes_to_nanoseconds      =   60ULL * seconds_to_nanoseconds;
    const uint64_t hours_to_nanoseconds        =   60ULL * minutes_to_nanoseconds;
    const uint64_t days_to_nanoseconds         =   24ULL * hours_to_nanoseconds;
    // clang-format on

    /// Break a value into whole unit components.
    /// \example
    ///    uint64_t byte_value = (1337 * mebibytes_to_bytes) + (666 * kibibytes_to_bytes) + 42;
    ///    uint64_t Units[]{mebibytes_to_bytes, kibibytes_to_bytes, 1}; uint32_t Results[3];
    ///    BreakIntoUnits(byte_value, ArraySlice(Units), ArraySlice(Results));
    ///    // Results[0] == 1337; Results[1] == 666; Results[0] == 42;
    void BreakIntoUnits(uint64_t value, tSlice<uint64_t const> units_table, tSlice<uint32_t> out_results);
} // namespace mtb

// --------------------------------------------------
// -- #Section Timespan ClockTime -------------------
// --------------------------------------------------

namespace mtb::time
{
    struct tTimespan
    {
        int64_t nanoseconds;

        inline constexpr operator bool() const { return (bool)nanoseconds; }
    };

    constexpr tTimespan timespan_zero{0};
    constexpr tTimespan timespan_min{tIntProperties<int64_t>::min_value};
    constexpr tTimespan timespan_max{tIntProperties<int64_t>::max_value};

    //
    // timespan + timespan
    //
    inline constexpr tTimespan operator+(tTimespan a, tTimespan b)
    {
        return {a.nanoseconds + b.nanoseconds};
    }
    inline constexpr tTimespan& operator+=(tTimespan& a, tTimespan b)
    {
        a.nanoseconds += b.nanoseconds;
        return a;
    }

    //
    // (timespan) - timespan
    //
    inline constexpr tTimespan operator-(tTimespan a)
    {
        return {-a.nanoseconds};
    }
    inline constexpr tTimespan operator-(tTimespan a, tTimespan b)
    {
        return {a.nanoseconds - b.nanoseconds};
    }
    inline constexpr tTimespan& operator-=(tTimespan& a, tTimespan b)
    {
        a.nanoseconds -= b.nanoseconds;
        return a;
    }

    //
    // timespan comparison
    //
    inline constexpr bool operator==(tTimespan a, tTimespan b)
    {
        return a.nanoseconds == b.nanoseconds;
    }
    inline constexpr bool operator!=(tTimespan a, tTimespan b)
    {
        return a.nanoseconds != b.nanoseconds;
    }
    inline constexpr bool operator<(tTimespan a, tTimespan b)
    {
        return a.nanoseconds < b.nanoseconds;
    }
    inline constexpr bool operator<=(tTimespan a, tTimespan b)
    {
        return a.nanoseconds <= b.nanoseconds;
    }
    inline constexpr bool operator>(tTimespan a, tTimespan b)
    {
        return a.nanoseconds > b.nanoseconds;
    }
    inline constexpr bool operator>=(tTimespan a, tTimespan b)
    {
        return a.nanoseconds >= b.nanoseconds;
    }

    // clang-format off
    inline constexpr int64_t GetNanoseconds(tTimespan timespan) { return timespan.nanoseconds; }
    inline constexpr double GetMicroseconds(tTimespan timespan) { return (double)timespan.nanoseconds / (double)microseconds_to_nanoseconds; }
    inline constexpr double GetMilliseconds(tTimespan timespan) { return (double)timespan.nanoseconds / (double)milliseconds_to_nanoseconds; }
    inline constexpr double GetSeconds(tTimespan timespan)      { return (double)timespan.nanoseconds / (double)seconds_to_nanoseconds; }
    inline constexpr double GetMinutes(tTimespan timespan)      { return (double)timespan.nanoseconds / (double)minutes_to_nanoseconds; }
    inline constexpr double GetHours(tTimespan timespan)        { return (double)timespan.nanoseconds / (double)hours_to_nanoseconds; }
    inline constexpr double GetDays(tTimespan timespan)         { return (double)timespan.nanoseconds / (double)days_to_nanoseconds; }
    // clang-format on

    // clang-format off
    inline constexpr tTimespan FromNanoseconds(int64_t in_nanoseconds)  { return { in_nanoseconds }; }
    inline constexpr tTimespan FromMicroseconds(double in_microseconds) { return { (int64_t)(in_microseconds * (double)microseconds_to_nanoseconds) }; }
    inline constexpr tTimespan FromMilliseconds(double in_milliseconds) { return { (int64_t)(in_milliseconds * (double)milliseconds_to_nanoseconds) }; }
    inline constexpr tTimespan FromSeconds     (double in_seconds)      { return { (int64_t)(in_seconds      * (double)seconds_to_nanoseconds) }; }
    inline constexpr tTimespan FromMinutes     (double in_minutes)      { return { (int64_t)(in_minutes      * (double)minutes_to_nanoseconds) }; }
    inline constexpr tTimespan FromHours       (double in_hours)        { return { (int64_t)(in_hours        * (double)hours_to_nanoseconds) }; }
    inline constexpr tTimespan FromDays        (double in_days)         { return { (int64_t)(in_days         * (double)days_to_nanoseconds) }; }
    // clang-format on

    struct tClockTime
    {
        bool negative;
        uint32_t days;
        uint32_t hours;
        uint32_t minutes;
        uint32_t seconds;
        uint32_t milliseconds;
        uint32_t microseconds;
        uint32_t nanoseconds;
    };

    tClockTime ClockTimeFromTimespan(tTimespan span);
} // namespace mtb::time

#endif // !defined(MTB__INCLUDED)

// --------------------------------------------------
// -- #Section Implementation -----------------------
// --------------------------------------------------
#if defined(MTB_IMPLEMENTATION)

void mtb::CopyBytes(void* dest, void const* src, size_t size) {
#if MTB_USE_LIBC
    ::memcpy(dest, src, size);
#else
    uint8_t* dst = (uint8_t*)dest;
    uint8_t const* src = (uint8_t const*)src;
    for (size_t index = 0; index < size; ++index) {
        dst[index] = src[index];
    }
#endif // MTB_USE_LIBC
}

void mtb::MoveBytes(void* dest, void const* src, size_t size) {
#if MTB_USE_LIBC
    ::memmove(dest, src, size);
#else
    uint8_t* dst = (uint8_t*)dest;
    uint8_t const* src = (uint8_t const*)src;
    if (dst < src) {
        // copy forward
        for (size_t index = 0; index < size; ++index) {
            dst[index] = src[index];
        }
    } else {
        // copy reverse
        for (size_t rIndex = size; rIndex > 0; --rIndex) {
            size_t index = rIndex - 1;
            dst[index] = src[index];
        }
    }
#endif // MTB_USE_LIBC
}

void mtb::SetBytes(void* dest, int byte_value, size_t size)
{
#if MTB_USE_LIBC
    ::memset(dest, byte_value, size);
#else
    uint8_t* dst = (uint8_t*)dest;
    uint8_t byte_value_u8 = (uint8_t)byte_value;
    for (size_t index = 0; index < size; index++)
    {
        dst[index] = byte_value_u8;
    }
#endif
}

int mtb::CompareBytes(void const* a, void const* b, size_t size)
{
#if MTB_USE_LIBC
    return ::memcmp(a, b, size);
#else
    uint8_t* byte_a = (uint8_t*)a;
    uint8_t* byte_b = (uint8_t*)b;
    for(size_t index = 0; index < size; ++index)
    {
        if(byte_a[index] != byte_b[index])
        {
            return byte_a[index] - byte_b[index];
        }
    }
    return 0;
#endif
}

int mtb::SliceCompareBytes(tSlice<void const> a, tSlice<void const> b)
{
    int result = (int)(a.len - b.len);
    if (result == 0)
    {
        result = MTB_memcmp(a.ptr, b.ptr, SliceSize(a));
    }
    return result;
}

bool mtb::SliceBytesAreEqual(tSlice<void const> a, tSlice<void const> b)
{
    return SliceCompareBytes(a, b) == 0;
}

void mtb::AlignAllocation(void** inout_ptr, size_t* inout_size, size_t alignment)
{
    MTB_ASSERT(inout_ptr != nullptr);
    MTB_ASSERT(*inout_ptr != nullptr);
    MTB_ASSERT(alignment > 0);

    uintptr_t unaligned = (uintptr_t)*inout_ptr;

    /* Example
        unaligned =      0b1010'1010 // input pointer
        alignment = 16 = 0b0001'0000

        a = 0b0000'1111
        ~a = 0b1111'0000

        unaligned + a = 0b1011'1001
        (unaligned + a) & ~a = 0b1011'0000 // i.e. the last four bits are cleared.
    */
    size_t a = alignment - 1;
    uintptr_t aligned = (unaligned + a) & ~a;
    *inout_ptr = (void*)aligned;
    if (inout_size)
    {
        *inout_size += aligned - unaligned;
    }
}

// IMPL
namespace mtb::impl
{
    size_t ChooseAlignment(size_t alignment)
    {
        return alignment ? alignment : MTB_ALLOCATOR_DEFAULT_ALIGNMENT;
    }
}

mtb::tSlice<void> mtb::tAllocator::AllocRaw(size_t size, size_t alignment, eInit init) const
{
    MTB_ASSERT(IsValid());
    size_t chosen_alignment = impl::ChooseAlignment(alignment);
    tSlice<void> result = realloc_proc(user, {}, chosen_alignment, size, chosen_alignment, init);
    return result;
}

mtb::tSlice<void> mtb::tAllocator::ReallocRaw(tSlice<void> old_mem, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init) const
{
    MTB_ASSERT(IsValid());
    return realloc_proc(user, old_mem, impl::ChooseAlignment(old_alignment), new_size, impl::ChooseAlignment(new_alignment), init);
}

mtb::tSlice<void> mtb::tAllocator::DupeRaw(tSlice<void> mem, size_t alignment) const
{
    MTB_ASSERT(IsValid());
    size_t chosen_alignment = impl::ChooseAlignment(alignment);
    tSlice<void> result = realloc_proc(user, mem, chosen_alignment, mem.len, chosen_alignment, kNoInit);
    MTB_ASSERT(result.len == mem.len);
    MTB_memcpy(result.ptr, mem.ptr, mem.len);
    return result;
}

void mtb::tAllocator::FreeRaw(tSlice<void> mem, size_t alignment) const
{
    MTB_ASSERT(IsValid());
    size_t chosen_alignment = impl::ChooseAlignment(alignment);
    realloc_proc(user, mem, chosen_alignment, 0, chosen_alignment, kNoInit);
}

#if MTB_USE_LIBC
namespace mtb::impl
{
    tSlice<void> LibcReallocProc(void* user, tSlice<void> old_mem, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init) {
        MTB_ASSERT(old_alignment <= new_alignment && "Changing alignment is not supported");

        tSlice<void> result{};
        if(old_mem || new_size)
        {
            void* new_ptr = ::realloc(old_mem.ptr, new_size);
            if(new_ptr)
            {
                result = PtrSlice(new_ptr, new_size);
                void* AlignedPtr = result.ptr;
                size_t AlignedSize = result.len;
                AlignAllocation(&AlignedPtr, &AlignedSize, new_alignment);
                MTB_ASSERT(AlignedSize == new_size && "Unsupported alignment");

                if(init == kClearToZero && old_mem.len < result.len)
                {
                    SliceSetZero(SliceOffset(result, old_mem.len));
                }
            }
            else
            {
                MTB_ASSERT((old_mem.ptr == nullptr || new_size == 0) && "realloc failed to resize an existing allocation?!");
            }
        }

        return result;
    }
} // namespace mtb::impl

mtb::tAllocator mtb::GetLibcAllocator()
{
    tAllocator result{};
    result.realloc_proc = impl::LibcReallocProc;
    return result;
}
#endif

#if MTB_USE_STB_SPRINTF
namespace mtb::impl
{
    char* InternalPrintfCallback(char const* buf, void* user, int len)
    {
        PushMany(*(tArray<char>*)user, PtrSlice(buf, len));
        return (char*)buf;
    }
} // namespace mtb

mtb::tSlice<char> mtb::vprintfAlloc(tAllocator a, char const* format, va_list vargs)
{
    tArray<char> buffer{a};
    char temp_buffer[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(impl::InternalPrintfCallback, &buffer, temp_buffer, format, vargs);
    return ShrinkAllocation(buffer);
}

mtb::tSlice<char> mtb::PushFormat(tArray<char>& array, char const* format, ...)
{
    va_list vargs;
    va_start(vargs, format);
    tSlice<char> result = PushFormatV(array, format, vargs);
    va_end(vargs);
    return result;
}

mtb::tSlice<char> mtb::PushFormatV(tArray<char>& array, char const* format, va_list vargs)
{
    ptrdiff_t offset = array.len;
    char temp_buffer[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(impl::InternalPrintfCallback, &array, temp_buffer, format, vargs);
    return SliceOffset(array.items, offset);
}
#endif // MTB_USE_STB_SPRINTF

mtb::tSlice<char> mtb::ToString(tArray<char> array, bool null_terminate /*= true*/)
{
    ptrdiff_t end_offset = 0;
    if (null_terminate)
    {
        Push(array, '\0');
        end_offset = 1;
    }
    ShrinkAllocation(array);
    return SliceCast<char>(SliceRange(array.items, 0, array.len - end_offset));
}

void mtb::QuickSort(void* user_ptr, ptrdiff_t count, bool(*less_proc)(void*, ptrdiff_t, ptrdiff_t), void(*swap_proc)(void*, ptrdiff_t, ptrdiff_t), ptrdiff_t threshold)
{
    // Based on https://github.com/svpv/qsort
    if (count <= 1)
    {
        return;
    }

    ptrdiff_t left = 0;
    ptrdiff_t right = count - 1;
    ptrdiff_t stack_pointer = 0; // the number of frames pushed to the stack
    struct
    {
        ptrdiff_t left, right;
    } stack[sizeof(ptrdiff_t) > 4 && sizeof(count) > 4 ? 48 : 32];
    while (true)
    {
        if (right - left + 1 >= threshold)
        {
            // Partition
            ptrdiff_t Middle = left + ((right - left) >> 1);

            // Sort 3 elements
            if (less_proc(user_ptr, Middle, left + 1))
            {
                if (less_proc(user_ptr, right, Middle))
                {
                    swap_proc(user_ptr, left + 1, right);
                }
                else
                {
                    swap_proc(user_ptr, left + 1, Middle);
                    if (less_proc(user_ptr, right, Middle))
                    {
                        swap_proc(user_ptr, Middle, right);
                    }
                }
            }
            else if (less_proc(user_ptr, right, Middle))
            {
                swap_proc(user_ptr, Middle, right);
                if (less_proc(user_ptr, Middle, left + 1))
                {
                    swap_proc(user_ptr, left + 1, Middle);
                }
            }

            // Place the median at the beginning
            swap_proc(user_ptr, left, Middle);

            ptrdiff_t I = left + 1;
            ptrdiff_t J = right;
            while (true)
            {
                do
                {
                    ++I;
                } while (less_proc(user_ptr, I, left));
                do
                {
                    --J;
                } while (less_proc(user_ptr, left, J));
                if (I >= J)
                {
                    break;
                }

                swap_proc(user_ptr, I, J);
            }

            // Compensate for the I==J case.
            I = J + 1;
            swap_proc(user_ptr, left, J);
            // The median is not part of the left subfile.
            --J;

            ptrdiff_t L1, R1, L2, R2;
            if (J - left >= right - I)
            {
                L1 = left;
                R1 = J;
                L2 = I;
                R2 = right;
            }
            else
            {
                L1 = I;
                R1 = right;
                L2 = left;
                R2 = J;
            }

            if (L2 == R2)
            {
                left = L1;
                right = R1;
            }
            else
            {
                stack[stack_pointer].left = L1;
                stack[stack_pointer].right = R1;
                stack_pointer++;
                // Process the smaller subfile on the next iteration.
                left = L2;
                right = R2;
            }
        }
        else
        {
            // Insertion sort
            for (ptrdiff_t I = left + 1; I <= right; ++I)
            {
                for (ptrdiff_t J = I; J > left && less_proc(user_ptr, J, J - 1); --J)
                {
                    swap_proc(user_ptr, J, J - 1);
                }
            }

            if (stack_pointer == 0)
            {
                break;
            }

            --stack_pointer;
            left = stack[stack_pointer].left;
            right = stack[stack_pointer].right;
        }
    }
}

namespace mtb::arena
{
    void InternalInsertNextBucket(tBucket*& current_bucket, tBucket* new_bucket)
    {
        if(current_bucket)
        {
            new_bucket->prev = current_bucket;
            new_bucket->next = current_bucket->next;
            new_bucket->next->prev = new_bucket->prev->next = new_bucket;
        }
        else
        {
            new_bucket->prev = new_bucket->next = new_bucket;
        }
        current_bucket = new_bucket;
    }

    tBucket* InternalUnlinkBucket(tBucket*& current_bucket)
    {
        MTB_ASSERT(current_bucket != nullptr);

        tBucket* result = current_bucket;
        if(current_bucket == current_bucket->prev)
        {
            current_bucket = nullptr;
        }
        else
        {
            current_bucket->prev->next = current_bucket->next;
            current_bucket->next->prev = current_bucket->prev;
            current_bucket = current_bucket->prev;
        }
        result->prev = result->next = nullptr;
        return result;
    }

    uint8_t* InternalBucketAlloc(tBucket* bucket, size_t* inout_size, size_t alignment)
    {
        uint8_t* result = nullptr;
        if (bucket && inout_size)
        {
            result = bucket->data + bucket->used_size;
            size_t EffectiveSize = *inout_size;
            AlignAllocation((void**)&result, &EffectiveSize, alignment);
            if (BucketUsedSize(bucket) + EffectiveSize <= BucketTotalSize(bucket))
            {
                *inout_size = EffectiveSize;
            }
            else
            {
                result = nullptr;
            }
        }
        return result;
    }

    void* InternalArenaAlloc(tArena& arena, size_t size, size_t alignment)
    {
        size_t EffectiveSize = size;
        uint8_t* result = InternalBucketAlloc(arena.current_bucket, &EffectiveSize, alignment);
        if (!result)
        {
            result = InternalBucketAlloc(arena.first_free_bucket, &EffectiveSize, alignment);
            if (result)
            {
                InternalInsertNextBucket(arena.current_bucket, InternalUnlinkBucket(arena.first_free_bucket));
            }
            else
            {
                Grow(arena, size + alignment);
                MTB_ASSERT(arena.current_bucket);

                result = arena.current_bucket->data;
                AlignAllocation((void**)&result, &EffectiveSize, alignment);
                MTB_ASSERT(BucketUsedSize(arena.current_bucket) + EffectiveSize <= BucketTotalSize(arena.current_bucket));
            }
        }

        arena.current_bucket->used_size += EffectiveSize;

        return result;
    }

    tSlice<void> InternalArenaAllocatorProc(void* user, tSlice<void> old_mem, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init)
    {
        tArena* arena = (tArena*)user;
        MTB_ASSERT(arena);
        void* new_ptr = ReallocRaw(*arena, old_mem.ptr, old_mem.len, old_alignment, new_size, new_alignment, init);
        tSlice<void> result = PtrSlice(new_ptr, new_size);
        return result;
    }
} // namespace mtb::arena

size_t mtb::arena::BucketTotalSize(tBucket* bucket)
{
    return bucket ? bucket->total_size : 0;
}

size_t mtb::arena::BucketUsedSize(tBucket* bucket)
{
    return bucket ? bucket->used_size : 0;
}

void mtb::arena::Grow(tArena& arena, size_t required_size)
{
    if(arena.min_bucket_size == 0)
    {
        arena.min_bucket_size = MTB_ARENA_DEFAULT_BUCKET_SIZE;
    }

    size_t new_bucket_size = BucketTotalSize(arena.current_bucket);
    if(new_bucket_size < arena.min_bucket_size)
    {
        new_bucket_size = arena.min_bucket_size;
    }
    while(new_bucket_size < required_size)
    {
        new_bucket_size *= 2;
    }

    tAllocator allocator = arena.child_allocator;
    if(allocator)
    {
        tBucket* new_bucket = (tBucket*)allocator.AllocRaw(sizeof(tBucket) - sizeof(uint8_t) + new_bucket_size, alignof(tBucket), kNoInit).ptr;
        // #TODO Handle out-of-memory properly.
        MTB_ASSERT(new_bucket != nullptr);
        new_bucket->used_size = 0;
        new_bucket->total_size = new_bucket_size;
        InternalInsertNextBucket(arena.current_bucket, new_bucket);

        if(arena.largest_bucket_size < new_bucket_size)
        {
            arena.largest_bucket_size = new_bucket_size;
        }
    }
}

void mtb::arena::Reserve(tArena& arena, size_t total_size)
{
    if (total_size > BucketTotalSize(arena.current_bucket))
    {
        Grow(arena, total_size);
    }
}

void mtb::arena::Clear(tArena& arena, bool release_memory /*= true*/)
{
    ResetToMarker(arena, {}, release_memory);
    if(arena.first_free_bucket && release_memory)
    {
        tAllocator allocator = arena.child_allocator;
        if(allocator)
        {
            while(arena.first_free_bucket)
            {
                tBucket* bucket = InternalUnlinkBucket(arena.first_free_bucket);
                allocator.FreeOne(bucket);
            }
        }
    }
}

void* mtb::arena::ReallocRaw(tArena& arena, void* old_ptr, size_t old_size, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init)
{
    if(old_ptr || old_size)
    {
        MTB_ASSERT(old_ptr && old_size);
    }

    ptrdiff_t DeltaSize = (ptrdiff_t)new_size - (ptrdiff_t)old_size;
    if(DeltaSize < 0)
    {
        DeltaSize = -DeltaSize;
    }

    void* result = nullptr;
    if(!old_ptr)
    {
        if(new_size)
        {
            result = InternalArenaAlloc(arena, new_size, new_alignment);
            if(init == kClearToZero)
            {
                MTB_memset(result, 0, new_size);
            }
        }
    }
    else
    {
        MTB_ASSERT(old_alignment == new_alignment && "Old and new alignment must be the same for now.");

        if(new_size < old_size)
        {
            if((uintptr_t)old_ptr + (uintptr_t)old_size == (uintptr_t)GetMarker(arena).ptr())
            {
                // Shrink the existing allocation.
                MTB_ASSERT(arena.current_bucket);
                MTB_ASSERT(arena.current_bucket->used_size >= (size_t)DeltaSize);
                arena.current_bucket->used_size -= DeltaSize;
            }

            if(new_size)
            {
                result = old_ptr;
            }
        }
        else if(new_size > old_size)
        {
            // Can we grow the existing allocation?
            if((uintptr_t)old_ptr + (uintptr_t)old_size == (uintptr_t)GetMarker(arena).ptr() &&
               BucketUsedSize(arena.current_bucket) + DeltaSize <= BucketTotalSize(arena.current_bucket))
            {
                void* new_ptr = InternalArenaAlloc(arena, DeltaSize, 1);
                MTB_ASSERT((uintptr_t)new_ptr == (uintptr_t)old_ptr + (uintptr_t)old_size);
                (void)new_ptr;
                result = old_ptr;
            }
            else
            {
                result = InternalArenaAlloc(arena, new_size, new_alignment);
                MTB_ASSERT(result != nullptr);
                MTB_memcpy(result, old_ptr, old_size);
            }

            if(init == kClearToZero)
            {
                MTB_memset(PtrOffset(result, old_size), 0, DeltaSize);
            }
        }
        else
        {
            result = old_ptr;
        }
    }

    return result;
}

void* mtb::arena::PushRaw(tArena& arena, size_t size, size_t alignment, eInit init)
{
    return ReallocRaw(arena, nullptr, 0, 0, size, alignment, init);
}

mtb::tSlice<void> mtb::arena::ReallocRawArray(tArena& arena, tSlice<void> old_array, size_t old_alignment, size_t new_size, size_t new_alignment, eInit init)
{
    void* ptr = ReallocRaw(arena, old_array.ptr, old_array.len, old_alignment, new_size, new_alignment, init);
    return PtrSlice(ptr, new_size);
}

mtb::tSlice<void> mtb::arena::PushRawArray(tArena& arena, size_t size, size_t alignment, eInit init)
{
    return ReallocRawArray(arena, {}, 0, size, alignment, init);
}

mtb::arena::tArenaMarker mtb::arena::GetMarker(tArena& arena)
{
    tArenaMarker result{arena.current_bucket, BucketUsedSize(arena.current_bucket)};
    return result;
}

void mtb::arena::ResetToMarker(tArena& arena, tArenaMarker marker, bool release_memory /*= true*/)
{
    if (arena.current_bucket)
    {
        tAllocator allocator = arena.child_allocator;
        if(!allocator)
        {
            release_memory = false;
        }

        if(!marker.bucket)
        {
            // Treat the empty marker as a marker of the oldest bucket.
            marker.bucket = arena.current_bucket->next;
        }

        // search backwards through all buckets, freeing them if they're not
        // the one we're looking for.
        bool looping = true;
        while(looping)
        {
            if(arena.current_bucket == marker.bucket)
            {
                looping = false;
                if(marker.offset > 0)
                {
                    break;
                }
            }

            tBucket* free_bucket = InternalUnlinkBucket(arena.current_bucket);
            if(release_memory)
            {
                allocator.FreeRaw(PtrSlice((void*)free_bucket, sizeof(tBucket) - sizeof(uint8_t) + free_bucket->total_size), alignof(tBucket));
            }
            else
            {
                InternalInsertNextBucket(arena.first_free_bucket, free_bucket);
            }
        }

        if(arena.current_bucket)
        {
            arena.current_bucket->used_size = marker.offset;
        }
    }
}

mtb::tSlice<void> mtb::arena::Linearize(tArena& arena, tArenaMarker begin, tArenaMarker end)
{
    if(!end.bucket)
    {
        MTB_ASSERT(!begin.bucket);
    }
    else if(!begin.bucket)
    {
        MTB_ASSERT(arena.current_bucket);
        // If the begin marker has a null-bucket, use the first/oldest bucket.
        begin.bucket = arena.current_bucket->next;
    }

    tSlice<void> result;
    if(begin.bucket == end.bucket)
    {
        MTB_ASSERT(begin.offset <= end.offset);
        result = PtrSliceBetween(begin.ptr(), end.ptr());
    }
    else
    {
        MTB_ASSERT(end.bucket);

        // determine the size
        size_t required_size = begin.bucket->used_size - begin.offset;
        for(tBucket* bucket = begin.bucket->next; bucket != end.bucket; bucket = bucket->next)
        {
            required_size += bucket->used_size;
        }
        required_size += end.offset;

        // allocate data
        result = PushArray<uint8_t>(arena, required_size, kNoInit);

        // copy the data
        size_t cursor = 0;
        MTB_memcpy(result + cursor, begin.bucket->data + begin.offset, begin.bucket->used_size - begin.offset);
        cursor += begin.bucket->used_size - begin.offset;
        for(tBucket* bucket = begin.bucket->next; bucket != end.bucket; bucket = bucket->next)
        {
            MTB_memcpy(result + cursor, bucket->data, bucket->used_size);
            cursor += bucket->used_size;
        }
        MTB_memcpy(result + cursor, end.bucket->data, end.offset);
    }

    return result;
}

mtb::tAllocator mtb::arena::MakeAllocator(tArena& arena)
{
    tAllocator result{};
    result.user = &arena;
    result.realloc_proc = InternalArenaAllocatorProc;
    return result;
}

#if MTB_USE_STB_SPRINTF
namespace mtb::arena
{
    static char* InternalArenaPrintCallback(char const* buf, void* user, int len)
    {
        tArena* arena = (tArena*)user;
        tSlice<void> dest = PushRawArray(*arena, len, 1, false);
        SliceCopyBytes(dest, PtrSlice(buf, len));
        return (char*)buf;
    }
} // namespace mtb::arena

void mtb::arena::vprintf_ArenaRaw(tArena& arena, char const* format, va_list vargs)
{
    char temp_buffer[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(InternalArenaPrintCallback, &arena, temp_buffer, format, vargs);
}

mtb::tSlice<char> mtb::arena::vprintf_Arena(tArena& arena, char const* format, va_list vargs)
{
    // Remember where we were in this arena.
    tArenaMarker begin_marker = GetMarker(arena);

    // Produce formatted string fragments within the arena.
    vprintf_ArenaRaw(arena, format, vargs);

    // Append the null-terminator.
    *PushOne<char>(arena, kNoInit) = 0;

    // Make sure all formatted string fragments we produced are actually linear in memory.
    tSlice<char> result = SliceCast<char>(Linearize(arena, begin_marker, GetMarker(arena)));
    MTB_ASSERT(result.len > 0);

    // Remove the null-terminator from the result.
    result.len--;

    return result;
}
#endif // MTB_USE_STB_SPRINTF

namespace mtb::string
{
    template<typename C>
    C Impl_ToLowerChar(C c)
    {
        switch (c)
        {
            case 'a': return 'a';
            case 'b': return 'b';
            case 'C': return 'c';
            case 'D': return 'd';
            case 'E': return 'e';
            case 'F': return 'f';
            case 'G': return 'g';
            case 'H': return 'h';
            case 'I': return 'i';
            case 'J': return 'j';
            case 'K': return 'k';
            case 'L': return 'l';
            case 'M': return 'm';
            case 'N': return 'n';
            case 'O': return 'o';
            case 'P': return 'p';
            case 'Q': return 'q';
            case 'R': return 'r';
            case 'S': return 's';
            case 'T': return 't';
            case 'U': return 'u';
            case 'V': return 'v';
            case 'W': return 'w';
            case 'X': return 'x';
            case 'Y': return 'y';
            case 'Z': return 'z';
            default: return c;
        }
    }

    template<typename C>
    C Impl_ToUpperChar(C c)
    {
        switch (c)
        {
            case 'a': return 'a';
            case 'b': return 'b';
            case 'c': return 'C';
            case 'd': return 'D';
            case 'e': return 'E';
            case 'f': return 'F';
            case 'g': return 'G';
            case 'h': return 'H';
            case 'i': return 'I';
            case 'j': return 'J';
            case 'k': return 'K';
            case 'l': return 'L';
            case 'm': return 'M';
            case 'n': return 'N';
            case 'o': return 'O';
            case 'p': return 'P';
            case 'q': return 'Q';
            case 'r': return 'R';
            case 's': return 'S';
            case 't': return 'T';
            case 'u': return 'U';
            case 'v': return 'V';
            case 'w': return 'W';
            case 'x': return 'X';
            case 'y': return 'Y';
            case 'z': return 'Z';
            default: return c;
        }
    }

    template<typename C>
    bool Impl_IsDigitChar(C c)
    {
        switch (c)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return true;
            default:
                return false;
        }
    }

    template<typename C>
    bool Impl_IsWhiteChar(C c)
    {
        switch (c)
        {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
            case '\b':
                return true;
            default:
                return false;
        }
    }

    template<typename C>
    int32_t Impl_StringCompare(tSlice<C const> str_a, tSlice<C const> str_b, eStringComparison cmp)
    {
        int32_t result = 0;

        if (str_a.len != str_b.len)
        {
            result = IntCast<int32_t>(str_a.len - str_b.len);
        }
        else
        {
            size_t count = str_a.len;
            switch (cmp)
            {
                case kCaseSensitive:
                {
                    for (size_t ByteIndex = 0; ByteIndex < count; ++ByteIndex)
                    {
                        if (str_a.ptr[ByteIndex] != str_b.ptr[ByteIndex])
                        {
                            result = str_a.ptr[ByteIndex] < str_b.ptr[ByteIndex] ? -1 : 1;
                            break;
                        }
                    }
                }
                break;

                case eStringComparison::kIgnoreCase:
                {
                    for (size_t ByteIndex = 0; ByteIndex < count; ++ByteIndex)
                    {
                        C CharA = ToLowerChar(str_a.ptr[ByteIndex]);
                        C CharB = ToLowerChar(str_b.ptr[ByteIndex]);
                        if (CharA != CharB)
                        {
                            result = (int32_t)(CharA - CharB);
                            break;
                        }
                    }
                }
                break;
            }
        }

        return result;
    }

    template<typename C>
    bool Impl_StringStartsWith(tSlice<C const> str, tSlice<C const> prefix, eStringComparison cmp)
    {
        bool result = false;
        if (str.len >= prefix.len)
        {
            result = StringEquals(SliceRange(str, 0, prefix.len), prefix, cmp);
        }

        return result;
    }

    template<typename C>
    bool Impl_StringEndsWith(tSlice<C const> str, tSlice<C const> prefix, eStringComparison cmp)
    {
        bool result = false;
        if (str.len >= prefix.len)
        {
            size_t offset = str.len - prefix.len;
            result = StringEquals(SliceBetween(str, offset, str.len), prefix, cmp);
        }

        return result;
    }

    template<typename C, typename P>
    tSlice<C> Impl_StringTrimStartPredicate(tSlice<C> str, P Predicate)
    {
        ptrdiff_t offset = 0;
        if (Predicate)
        {
            for (; offset < str.len; ++offset)
            {
                C c = str.ptr[offset];
                bool bIsValid = Predicate(c);
                if (!bIsValid)
                {
                    break;
                }
            }
        }

        return SliceBetween(str, offset, str.len);
    }

    template<typename C, typename P>
    tSlice<C> Impl_StringTrimEndPredicate(tSlice<C> str, P Predicate)
    {
        ptrdiff_t one_past_last_index = str.len;
        if (Predicate)
        {
            while (one_past_last_index > 0 && Predicate(str.ptr[one_past_last_index - 1]))
            {
                --one_past_last_index;
            }
        }

        return SliceBetween(str, 0, one_past_last_index);
    }

    template<typename C>
    tSlice<C> Impl_AllocString(tAllocator a, size_t char_count)
    {
        tSlice<C> result = a.AllocArray<C>(char_count + 1, kClearToZero);
        return SliceRange(result, 0, result.len - 1);
    }

    template<typename CIn, typename COut = tRemoveConst<CIn>>
    tSlice<COut> Impl_AllocStringCopy(tAllocator a, tSlice<CIn> str)
    {
        tSlice<COut> result = a.AllocArray<COut>(str.len + 1, kNoInit);
        SliceCopyBytes(result, str);
        *SliceLast(result) = 0;
        return SliceRange(result, 0, result.len - 1);
    }

    template<typename C>
    size_t Impl_StringLengthZ(C const* str_z)
    {
        size_t result = 0;
        if (str_z)
        {
            while (str_z[result])
            {
                ++result;
            }
        }

        return result;
    }
} // namespace mtb::string

char mtb::string::ToLowerChar(char c)
{
    return Impl_ToLowerChar(c);
}

char mtb::string::ToUpperChar(char c)
{
    return Impl_ToUpperChar(c);
}

bool mtb::string::IsDigitChar(char c)
{
    return Impl_IsDigitChar(c);
}

bool mtb::string::IsWhiteChar(char c)
{
    return Impl_IsWhiteChar(c);
}

int32_t mtb::string::StringCompare(tSlice<char const> str_a, tSlice<char const> str_b, eStringComparison cmp /*= kCaseSensitive*/)
{
    return Impl_StringCompare(str_a, str_b, cmp);
}

bool mtb::string::StringEquals(tSlice<char const> str_a, tSlice<char const> str_b, eStringComparison cmp)
{
    return Impl_StringCompare(str_a, str_b, cmp) == 0;
}

bool mtb::string::StringStartsWith(tSlice<char const> str, tSlice<char const> prefix, eStringComparison cmp)
{
    return Impl_StringStartsWith(str, prefix, cmp);
}

bool mtb::string::StringEndsWith(tSlice<char const> str, tSlice<char const> prefix, eStringComparison cmp)
{
    return Impl_StringEndsWith(str, prefix, cmp);
}

mtb::tSlice<char const> mtb::string::StringTrimStartPredicate(tSlice<char const> str, tTrimPredicate Predicate)
{
    return Impl_StringTrimStartPredicate<char const>(str, Predicate);
}

mtb::tSlice<char const> mtb::string::StringTrimEndPredicate(tSlice<char const> str, tTrimPredicate Predicate)
{
    return Impl_StringTrimEndPredicate(str, Predicate);
}

mtb::tSlice<char const> mtb::string::StringTrimPredicate(tSlice<char const> str, tTrimPredicate Predicate)
{
    return StringTrimStartPredicate(StringTrimEndPredicate(str, Predicate), Predicate);
}

mtb::tSlice<char const> mtb::string::StringTrimStart(tSlice<char const> str)
{
    return StringTrimStartPredicate(str, IsWhiteChar);
}

mtb::tSlice<char const> mtb::string::StringTrimEnd(tSlice<char const> str)
{
    return StringTrimEndPredicate(str, IsWhiteChar);
}

mtb::tSlice<char const> mtb::string::StringTrim(tSlice<char const> str)
{
    return StringTrimPredicate(str, IsWhiteChar);
}

mtb::tSlice<char> mtb::string::AllocString(tAllocator a, size_t char_count)
{
    return Impl_AllocString<char>(a, char_count);
}

mtb::tSlice<char> mtb::string::DupeString(tAllocator a, tSlice<char const> str)
{
    return Impl_AllocStringCopy(a, str);
}

void mtb::string::FreeString(tAllocator a, tSlice<char> str)
{
    ++str.len;
    a.FreeArray(str);
}

size_t mtb::string::StringLengthZ(char const* str_z)
{
    return Impl_StringLengthZ(str_z);
}

wchar_t mtb::string::ToLowerChar(wchar_t c)
{
    return Impl_ToLowerChar(c);
}

wchar_t mtb::string::ToUpperChar(wchar_t c)
{
    return Impl_ToUpperChar(c);
}

bool mtb::string::IsDigitChar(wchar_t c)
{
    return Impl_IsDigitChar(c);
}

bool mtb::string::IsWhiteChar(wchar_t c)
{
    return Impl_IsWhiteChar(c);
}

int32_t mtb::string::StringCompare(tSlice<wchar_t const> str_a, tSlice<wchar_t const> str_b, eStringComparison cmp /*= kCaseSensitive*/)
{
    return Impl_StringCompare(str_a, str_b, cmp);
}

bool mtb::string::StringEquals(tSlice<wchar_t const> str_a, tSlice<wchar_t const> str_b, eStringComparison cmp)
{
    return Impl_StringCompare(str_a, str_b, cmp) == 0;
}

bool mtb::string::StringStartsWith(tSlice<wchar_t const> str, tSlice<wchar_t const> prefix, eStringComparison cmp)
{
    return Impl_StringStartsWith(str, prefix, cmp);
}

bool mtb::string::StringEndsWith(tSlice<wchar_t const> str, tSlice<wchar_t const> prefix, eStringComparison cmp)
{
    return Impl_StringEndsWith(str, prefix, cmp);
}

mtb::tSlice<wchar_t const> mtb::string::StringTrimStartPredicate(tSlice<wchar_t const> str, tWTrimPredicate Predicate)
{
    return Impl_StringTrimStartPredicate(str, Predicate);
}

mtb::tSlice<wchar_t const> mtb::string::StringTrimEndPredicate(tSlice<wchar_t const> str, tWTrimPredicate Predicate)
{
    return Impl_StringTrimEndPredicate(str, Predicate);
}

mtb::tSlice<wchar_t const> mtb::string::StringTrimPredicate(tSlice<wchar_t const> str, tWTrimPredicate Predicate)
{
    return StringTrimStartPredicate(StringTrimEndPredicate(str, Predicate), Predicate);
}

mtb::tSlice<wchar_t const> mtb::string::StringTrimStart(tSlice<wchar_t const> str)
{
    return StringTrimStartPredicate(str, IsWhiteChar);
}

mtb::tSlice<wchar_t const> mtb::string::StringTrimEnd(tSlice<wchar_t const> str)
{
    return StringTrimEndPredicate(str, IsWhiteChar);
}

mtb::tSlice<wchar_t const> mtb::string::StringTrim(tSlice<wchar_t const> str)
{
    return StringTrimPredicate(str, IsWhiteChar);
}

mtb::tSlice<wchar_t> mtb::string::AllocWString(tAllocator a, size_t char_count)
{
    return Impl_AllocString<wchar_t>(a, char_count);
}

mtb::tSlice<wchar_t> mtb::string::DupeString(tAllocator a, tSlice<wchar_t const> str)
{
    return Impl_AllocStringCopy(a, str);
}

size_t mtb::string::StringLengthZ(wchar_t const* str_z)
{
    return Impl_StringLengthZ(str_z);
}

void mtb::string::FreeWString(tAllocator a, tSlice<wchar_t> str)
{
    ++str.len;
    a.FreeArray(str);
}

void mtb::BreakIntoUnits(uint64_t value, tSlice<uint64_t const> units_table, tSlice<uint32_t> out_results)
{
    uint64_t Remaining = value;
    for (ptrdiff_t index = 0; index < units_table.len; ++index)
    {
        uint64_t Unit = units_table.ptr[index];
        uint32_t result = (uint32_t)(Remaining / Unit);
        Remaining -= (uint64_t)(result * Unit);

        out_results[index] = result;
    }
}

mtb::time::tClockTime mtb::time::ClockTimeFromTimespan(tTimespan timespan)
{
    tClockTime result{};
    uint64_t Total;
    if (timespan.nanoseconds < 0)
    {
        Total = IntCast<uint64_t>(-timespan.nanoseconds);
        result.negative = true;
    }
    else
    {
        Total = IntCast<uint64_t>(timespan.nanoseconds);
    }

    uint64_t Units[7]{
        days_to_nanoseconds,
        hours_to_nanoseconds,
        minutes_to_nanoseconds,
        seconds_to_nanoseconds,
        milliseconds_to_nanoseconds,
        microseconds_to_nanoseconds,
        1,
    };
    BreakIntoUnits(Total, ArraySlice(Units), PtrSlice(&result.days, 7));

    return result;
}

// --------------------------------------------------
// -- #Section Tests --------------------------------
// --------------------------------------------------
#if MTB_TESTS
DOCTEST_TEST_SUITE("mtb::tArena_SKIP")
{
    using namespace mtb;
    using namespace mtb::arena;

    ptrdiff_t BucketCount(tArena& arena)
    {
        ptrdiff_t result = 0;
        if(arena.current_bucket)
        {
            result = 1;
            for(tBucket* bucket = arena.current_bucket->next; bucket != arena.current_bucket; bucket = bucket->next)
            {
                ++result;
            }
        }
        return result;
    }

    DOCTEST_TEST_CASE("General")
    {
        tArena arena{};
        arena.min_bucket_size = 1024;
        DOCTEST_CHECK(BucketCount(arena) == 0);
        uint8_t* p0 = (uint8_t*)PushRaw(arena, 1023, 1, kNoInit);
        DOCTEST_CHECK(BucketCount(arena) == 1);
        memset(p0, 2, 1023);
        uint8_t* p1 = (uint8_t*)PushRaw(arena, 1, 1024, kNoInit);
        DOCTEST_CHECK(BucketCount(arena) == 2);
        DOCTEST_CHECK((uintptr_t)p1 % 1024 == 0);
        *p1 = 3;
        uint8_t* p2 = (uint8_t*)PushRaw(arena, 1, 1024, kNoInit);
        DOCTEST_CHECK(BucketCount(arena) == 2);
        DOCTEST_CHECK((uintptr_t)p2 % 1024 == 0);
        *p2 = 4;

        for (int index = 0; index < 1023; ++index)
        {
            DOCTEST_CHECK(p0[index] == 2);
        }
        DOCTEST_CHECK(*p1 == 3);
        DOCTEST_CHECK(*p2 == 4);

        Clear(arena, true);
        DOCTEST_CHECK(BucketCount(arena) == 0);
    }

    DOCTEST_TEST_CASE("marker")
    {
        tArena arena{};
        arena.min_bucket_size = 1024;
        DOCTEST_CHECK(BucketUsedSize(arena.current_bucket) == 0);

        tArenaMarker zero_marker{};

        PushRaw(arena, 512, 1, kNoInit);
        DOCTEST_CHECK(BucketUsedSize(arena.current_bucket) == 512);

        DOCTEST_SUBCASE("force bucket overflow")
        {
            tArenaMarker marker = GetMarker(arena);

            PushRaw(arena, 1024, 1, kNoInit);
            DOCTEST_CHECK(arena.current_bucket != marker.bucket);

            ResetToMarker(arena, marker, false);
            DOCTEST_CHECK(arena.current_bucket == marker.bucket);
        }

        DOCTEST_SUBCASE("free entire arena via marker")
        {
            ResetToMarker(arena, zero_marker, false);
            DOCTEST_CHECK(arena.current_bucket == nullptr);
            DOCTEST_CHECK(BucketUsedSize(arena.current_bucket) == 0);
        }

        DOCTEST_SUBCASE("linearize")
        {
            tArenaMarker marker0 = GetMarker(arena);

            void* p0 = PushRaw(arena, 1024, 1, kNoInit);
            void* p1 = PushRaw(arena, 1024, 1, kNoInit);
            DOCTEST_CHECK(p0 != p1);

            tSlice<void> Linear = Linearize(arena, marker0, GetMarker(arena));
            tArenaMarker Marker1 = GetMarker(arena);
            DOCTEST_CHECK(Linear.ptr != p0);
            DOCTEST_CHECK(Linear.ptr != p1);
            DOCTEST_CHECK(BucketUsedSize(arena.current_bucket) >= 2 * 1024);
            DOCTEST_CHECK(Marker1.bucket != nullptr);
            DOCTEST_CHECK(BucketTotalSize(Marker1.bucket) >= 2 * 1024);
        }

        Clear(arena, true);
    }

    DOCTEST_TEST_CASE("Linearize")
    {
        tArena arena{};
        arena.min_bucket_size = 1;
        MTB_DEFER { Clear(arena, true); };

        *PushOne<int>(arena) = 1;
        *PushOne<int>(arena) = 2;
        *PushOne<int>(arena) = 3;
        tArenaMarker MidMarker = GetMarker(arena);
        *PushOne<int>(arena) = 4;
        *PushOne<int>(arena) = 5;
        *PushOne<int>(arena) = 6;
        *PushOne<int>(arena) = 7;
        tArenaMarker end_marker = GetMarker(arena);

        tSlice<int> left = SliceCast<int>(Linearize(arena, {}, MidMarker));
        DOCTEST_CHECK(left.len == 3);
        DOCTEST_CHECK(left[0] == 1);
        DOCTEST_CHECK(left[1] == 2);
        DOCTEST_CHECK(left[2] == 3);

        tSlice<int> right = SliceCast<int>(Linearize(arena, MidMarker, end_marker));
        DOCTEST_CHECK(right.len == 4);
        DOCTEST_CHECK(right[0] == 4);
        DOCTEST_CHECK(right[1] == 5);
        DOCTEST_CHECK(right[2] == 6);
        DOCTEST_CHECK(right[3] == 7);

        tSlice<int> All = SliceCast<int>(Linearize(arena, {}, end_marker));
        DOCTEST_CHECK(All.len == 7);
        DOCTEST_CHECK(All[0] == 1);
        DOCTEST_CHECK(All[1] == 2);
        DOCTEST_CHECK(All[2] == 3);
        DOCTEST_CHECK(All[3] == 4);
        DOCTEST_CHECK(All[4] == 5);
        DOCTEST_CHECK(All[5] == 6);
        DOCTEST_CHECK(All[6] == 7);
    }

#if MTB_USE_STB_SPRINTF
    DOCTEST_TEST_CASE("printf")
    {
        tArena arena{};
        MTB_DEFER { Clear(arena, true); };

        tSlice<char> str = printf_Arena(arena, "Hello world!");
        DOCTEST_CHECK(toString(str) == "Hello world!");

        Clear(arena, false);

        DOCTEST_SUBCASE("manual linearization")
        {
            // This produces a formatted non-null-terminated string.
            arena.min_bucket_size = 1;
            tArenaMarker begin_marker = GetMarker(arena);
            printf_ArenaRaw(arena, "%c", 'a');
            printf_ArenaRaw(arena, "%c", 'b');
            printf_ArenaRaw(arena, "%c", 'c');
            printf_ArenaRaw(arena, "%c", 'd');
            printf_ArenaRaw(arena, "%c", 'e');
            printf_ArenaRaw(arena, "%c", 'f');
            tArenaMarker end_marker = GetMarker(arena);
            tSlice<char> linearized = SliceCast<char>(Linearize(arena, begin_marker, end_marker));
            DOCTEST_CHECK(toString(linearized) == "abcdef");
        }

        DOCTEST_SUBCASE("force linearization over multiple buckets")
        {
            arena.min_bucket_size = 8;

            char const* lipsum =
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed cursus "
                "hendrerit pharetra. Cras sit amet blandit arcu."
                "Mauris tristique dapibus vulputate. Maecenas ipsum dui, porta id "
                "felis quis, gravida dictum neque. Suspendisse id"
                "fringilla justo. Duis volutpat lobortis magna et imperdiet. Etiam "
                "vulputate nisl eu lacus pulvinar sodales. Mauris eu"
                "diam nulla. Pellentesque commodo, ligula in varius eleifend, velit "
                "metus fermentum sapien, a accumsan nisl metus vel"
                "lorem. Aliquam ultricies nisl sed ex tincidunt semper. Ut ut turpis "
                "eleifend, sagittis velit eu, lobortis elit. Mauris"
                "pellentesque dignissim elit, quis suscipit ante iaculis vitae. "
                "Aenean non gravida ex. Mauris lobortis mi massa, id"
                "bibendum eros laoreet lobortis. Fusce nisi libero, dictum et dui "
                "iaculis, pellentesque aliquet erat. Cras pharetra metus"
                "nec sapien consectetur, vitae viverra sem sollicitudin. Fusce id "
                "condimentum leo. Ut gravida sed nisi eu convallis."
                "Nulla volutpat metus id nisi ultricies, nec gravida libero laoreet. "
                "Vestibulum bibendum leo lorem, vitae maximus leo"
                "luctus non. Pellentesque nec pretium risus. Integer libero neque, "
                "molestie vel erat sit amet, euismod rutrum enim."
                "Maecenas tincidunt magna sapien, vel iaculis est vehicula a. "
                "Suspendisse sollicitudin purus quis magna interdum, id"
                "bibendum tortor sollicitudin. Phasellus eget lectus feugiat, luctus "
                "diam et, venenatis felis. Cras sit amet tellus non"
                "est aliquam posuere. Curabitur eu aliquet arcu. Suspendisse a "
                "sollicitudin eros. Donec ac egestas ante. Pellentesque id"
                "pellentesque turpis. Pellentesque lobortis vel elit fringilla "
                "scelerisque. Donec eu posuere lacus, vitae iaculis metus."
                "Aliquam eleifend facilisis quam, et fringilla nisl ornare "
                "facilisis. Phasellus in massa ac risus posuere maximus."
                "Vestibulum id velit id quam eleifend fermentum. Interdum et "
                "malesuada fames ac ante ipsum primis in faucibus. Integer eu"
                "rutrum est. Curabitur non risus non felis dictum scelerisque. "
                "Maecenas id rutrum dolor, in sagittis mauris. Donec id"
                "nunc vel nisl volutpat vulputate nec sed leo. Suspendisse potenti. "
                "Integer cursus, ante eu ullamcorper imperdiet, orci"
                "metus porta ligula, at mollis est urna quis massa. Mauris faucibus "
                "placerat purus in volutpat. Etiam lacinia non tellus"
                "at iaculis. Maecenas bibendum odio volutpat varius fermentum. Sed "
                "libero massa, semper eget scelerisque a, sodales vel"
                "felis. Integer sodales augue eu felis hendrerit, ac malesuada felis "
                "auctor. In hac habitasse platea dictumst. Phasellus"
                "maximus eu magna id pellentesque. Fusce quis tempus lacus. "
                "Suspendisse luctus suscipit lacus euismod viverra. Morbi"
                "consequat nec felis id blandit. Morbi nec felis turpis. Cras ornare "
                "enim leo, sed convallis lectus elementum vel. Cras"
                "luctus augue finibus nibh varius consectetur. Quisque elementum, "
                "velit quis egestas tempus, sapien magna faucibus odio,"
                "ac tincidunt odio mauris in erat.";

            tSlice<char> result = printf_Arena(arena, "%s", lipsum);
            DOCTEST_CHECK(toString(result) == lipsum);
        }
    }
#endif // MTB_USE_STB_SPRINTF
}

#endif // MTB_TESTS
#endif // MTB_IMPLEMENTATION

/*
<!-- BEGIN NATVIS -->

<?xml version="1.0" encoding="utf-8"?>
<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010">
    <Type Name="::mtb::arena::tBucket">
        <DisplayString>Used={used_size}/{total_size}</DisplayString>
        <Expand>
            <item Name="Used size">used_size</item>
            <item Name="Unused size">total_size - used_size</item>
            <item Name="Total size">total_size</item>
            <item Name="next bucket">next</item>
            <item Name="prev bucket">prev</item>
            <Synthetic Name="[Used data]">
                <Expand>
                    <ArrayItems>
                        <size>used_size</size>
                        <ValuePointer>data</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <Synthetic Name="[Unused data]">
                <Expand>
                    <ArrayItems>
                        <size>total_size - used_size</size>
                        <ValuePointer>data + used_size</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <Synthetic Name="[All data]">
                <Expand>
                    <ArrayItems>
                        <size>total_size</size>
                        <ValuePointer>data</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
        </Expand>
    </Type>

    <Type Name="::mtb::arena::tArena">
        <DisplayString>Current bucket {*current_bucket}</DisplayString>
    </Type>

    <Type Name="::mtb::tSlice &lt; * &gt;">
        <DisplayString>{{{count} items ({count * sizeof(*ptr)} bytes)}}</DisplayString>
        <Expand>
            <ExpandedItem>ptr,[count]</ExpandedItem>
        </Expand>
    </Type>

    <!-- string slices -->
    <Type Name="::mtb::tSlice &lt; char &gt;">
        <AlternativeType Name="::mtb::tSlice &lt; char const &gt;"/>
        <AlternativeType Name="::mtb::tSlice &lt; const char &gt;"/>
        <DisplayString>({count}) {ptr,[count]int8_t}</DisplayString>
        <StringView>ptr,[count]int8_t</StringView>
        <Expand>
            <item Name="Is zero-terminated">ptr[count] == 0</item>
            <item Name="count">count</item>
            <item Name="data">ptr,[count]</item>
        </Expand>
    </Type>

    <Type Name="::mtb::tSlice &lt; wchar_t &gt;">
        <AlternativeType Name="::mtb::tSlice &lt; wchar_t const &gt;"/>
        <AlternativeType Name="::mtb::tSlice &lt; const wchar_t &gt;"/>
        <DisplayString>({count}) {ptr,[count]su}</DisplayString>
        <StringView>ptr,[count]su</StringView>
        <Expand>
            <item Name="Is zero-terminated">ptr[count] == 0</item>
            <item Name="count">count</item>
            <item Name="data">ptr,[count]</item>
        </Expand>
    </Type>

    <!-- void slices -->
    <Type Name="::mtb::tSlice &lt; void &gt;">
        <AlternativeType Name="::mtb::tSlice &lt; void const &gt;"/>
        <AlternativeType Name="::mtb::tSlice &lt; const void &gt;"/>
        <DisplayString>{count} bytes</DisplayString>
        <Expand>
            <ExpandedItem>(char unsigned*)ptr,[count]</ExpandedItem>
        </Expand>
    </Type>

    <Type Name="::mtb::tArray &lt; * &gt;">
        <DisplayString>{slice}</DisplayString>
        <Expand>
            <Synthetic Name="[Slack]">
                <DisplayString>{{{cap - count} items ({(cap - count) * sizeof(*ptr)} bytes)}}</DisplayString>
                <Expand>
                    <ArrayItems>
                        <size>cap - count</size>
                        <ValuePointer>ptr + count</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <Synthetic Name="[Allocation]">
                <DisplayString>{{{cap} items ({cap * sizeof(*ptr)} bytes)}}</DisplayString>
                <Expand>
                    <ArrayItems>
                        <size>cap</size>
                        <ValuePointer>ptr</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <ExpandedItem>slice</ExpandedItem>
        </Expand>
    </Type>

    <Type Name="::mtb::tMap &lt; * &gt;">
        <DisplayString>count={count}</DisplayString>
        <Expand>
            <item Name="cap">cap</item>
            <Synthetic Name="Fill %">
                <DisplayString>{(100.0 * (double)count / (double)cap),f}%</DisplayString>
            </Synthetic>
            <Synthetic Name="[Slots]">
                <Expand>
                    <ArrayItems>
                        <size>cap</size>
                        <ValuePointer>Slots</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <Synthetic Name="[Keys]">
                <Expand>
                    <ArrayItems>
                        <size>cap</size>
                        <ValuePointer>Keys</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <Synthetic Name="[Values]">
                <Expand>
                    <ArrayItems>
                        <size>cap</size>
                        <ValuePointer>Values</ValuePointer>
                    </ArrayItems>
                </Expand>
            </Synthetic>
            <CustomListItems MaxItemsPerView="512">
                <Variable Name="index" InitialValue="0"/>
                <Loop Condition="index &lt; cap">
                    <If Condition="Slots[index].State == 1">
                        <item Name="{Keys[index]}">Values[index]</item>
                    </If>
                    <Exec>++index</Exec>
                </Loop>
            </CustomListItems>
        </Expand>
    </Type>

    <Type Name="::mtb::tTimespan">
        <DisplayString>{nanoseconds / (1000.0*1000.0*1000.0)} s</DisplayString>
        <Expand>
            <item Name="Seconds">nanoseconds / (1000.0*1000.0*1000.0)</item>
            <item Name="Milliseconds">nanoseconds / (1000.0*1000.0)</item>
            <item Name="nanoseconds">nanoseconds</item>
        </Expand>
    </Type>

</AutoVisualizer>

<!-- END NATVIS -->
*/
