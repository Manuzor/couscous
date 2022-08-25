/*
 * Define MTB_RNG_IMPLEMENTATION in exactly one of your translation units that includes this header.
 */

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
 * Casey Muratori's Handmade Hero.
 */

#if !defined(MTB__RNG_INCLUDED)
#define MTB__RNG_INCLUDED

#if !defined(MTB_RNG_ORIGINAL_BOUNDED_PCG)
#define MTB_RNG_ORIGINAL_BOUNDED_PCG 0
#endif

namespace mtb {

struct tRNG {
    uint64_t state;
    uint64_t inc;

    /// Initialize a random number generator with the given seed.
    static tRNG Seed(uint64_t initial_state, uint64_t stream_id = 1);

    //
    // Random unsigned 32-bit integer values.
    //

    /// Generate a uniformly distributed 32-bit random number.
    uint32_t Random_u32();

    /// Generate a uniformly distributed 32-bit random number, result, where 0 <= result < bound.
    uint32_t RandomBelow_u32(uint32_t bound);

    /// Generate a uniformly distributed 32-bit random number, result, where lower_bound <= result <= upper_bound.
    uint32_t RandomBetween_u32(uint32_t lower_bound, uint32_t upper_bound);

    //
    // Random 32-bit floating-point values.
    //

    /// Generate a uniformly distributed 32-bit random floating point number, result, where 0 <= result <= 1.
    float RandomUnilateral_f32();

    /// Generate a uniformly distributed 32-bit random floating point number, result, where -1 <= result <= 1.
    float RandomBilateral_f32();

    /// Generate a uniformly distributed 32-bit random number, result, where lower_bound <= result < upper_bound.
    float RandomBetween_f32(float lower_bound, float upper_bound);
};

}

#endif // !defined(MTB__RNG_INCLUDED)


// ==============
// Implementation
// ==============

#if defined(MTB_RNG_IMPLEMENTATION)
#if !defined(MTB__RNG_IMPLEMENTED)
#define MTB__RNG_IMPLEMENTED

mtb::tRNG mtb::tRNG::Seed(uint64_t initial_state, uint64_t stream_id) {
    tRNG result{};
    result.inc = (stream_id << 1u) | 1u;
    (void)result.Random_u32();
    result.state += initial_state;
    (void)result.Random_u32();
    return result;
}

uint32_t mtb::tRNG::Random_u32() {
    tRNG& rng = *this;
    uint64_t old_state = rng.state;
    rng.state = old_state * 6364136223846793005ULL + rng.inc;
    uint32_t xor_shifted = (uint32_t)(((old_state >> 18u) ^ old_state) >> 27u);
    uint32_t rot = (uint32_t)(old_state >> 59u);
    uint32_t result = (xor_shifted >> rot) | (xor_shifted << (((uint32_t)(-(int32_t)rot)) & 31));
    return result;
}

uint32_t mtb::tRNG::RandomBelow_u32(uint32_t bound) {
    tRNG& rng = *this;
    uint32_t result = 0;

    if(bound > 0) {
#if MTB_RNG_ORIGINAL_BOUNDED_PCG
        // NOTE(Manuzor): Even though it says that the original implementation
        // should usually be fast, I'm kind of hung up on that "usually" part. I
        // think I'm willing to sacrifice a little performance for deterministic
        // program behavior in this case. By default, at least.

        // To avoid bias, we need to make the range of the RNG a multiple of
        // bound, which we do by dropping output less than a threshold.
        // A naive scheme to calculate the threshold would be to do
        //
        //     uint32_t threshold = 0x100000000ull % bound;
        //
        // but 64-bit div/mod is slower than 32-bit div/mod (especially on
        // 32-bit platforms).  In essence, we do
        //
        //     uint32_t threshold = (0x100000000ull-bound) % bound;
        //
        // because this version will calculate the same modulus, but the LHS
        // value is less than 2^32.
        uint32_t threshold = (uint32_t)(-(int32_t)bound) % bound;

        // Uniformity guarantees that this loop will terminate.  In practice, it
        // should usually terminate quickly; on average (assuming all bounds are
        // equally likely), 82.25% of the time, we can expect it to require just
        // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
        // (i.e., 2147483649), which invalidates almost 50% of the range.  In
        // practice, bounds are typically small and only a tiny amount of the range
        // is eliminated.
        while(true) {
            result = rng.Random_u32();
            if(result >= threshold) {
                break;
            }
        }

        result %= bound;
#else
        result = rng.Random_u32() % bound;
#endif
    }

    return result;
}

uint32_t mtb::tRNG::RandomBetween_u32(uint32_t lower_bound, uint32_t upper_bound) {
    tRNG& rng = *this;
    uint32_t result = 0;
    if (upper_bound > lower_bound) {
        uint32_t rand = rng.Random_u32();
        uint32_t local_bound = (upper_bound + 1) - lower_bound;
        result = lower_bound + (rand % local_bound);
    }
    return result;
}

float mtb::tRNG::RandomUnilateral_f32() {
    tRNG& rng = *this;
    float divisor = 1.0f / (float)(uint32_t)0xFFFFFFFF;
    float result = divisor * (float)rng.Random_u32();
    return result;
}

float mtb::tRNG::RandomBilateral_f32() {
    tRNG& rng = *this;
    float result = 2.0f * rng.RandomUnilateral_f32() - 1.0f;
    return result;
}

float mtb::tRNG::RandomBetween_f32(float lower_bound, float upper_bound) {
    tRNG& rng = *this;
    float alpha = rng.RandomUnilateral_f32();
    float result = (1.0f - alpha) * lower_bound + alpha * upper_bound;
    return result;
}

#endif // !defined(MTB__RNG_IMPLEMENTED)
#endif // defined(MTB_RNG_IMPLEMENTATION)
