#pragma once

#include "common.hh"

namespace qt
{
	/// Swaps the values of two n-bit integers using XOR.
	template<typename A, typename B>
	static inline void IntSwap(A& a, B& b)
	{
		a ^= b;
		b ^= a;
		a ^= b;
	}

	/// Inserts a bit of value newBit into an integer val at a given index (LSB is index = 0).
	template <typename T>
	static void InsertBit(T& val, size_t index, int newBit)
	{
		T x = val;
		T y = x;
		x <<= 1;

		newBit ? (x |= (((T)1) << index)) : (x &= ~(((T)1) << index));

		x &= ((~((T)0)) << index);
		y &= ~((~((T)0)) << index);
		x |= y;

		val = x;
	}

	static uint8_t FindFirstBit(uint32_t val)
	{
		if (val == 0)
		{
			return 0;
		}

		uint8_t pos = 0;

		if ((val & 0x0000ffff) == 0) { val >>= 16; pos += 16; }
		if ((val & 0x000000ff) == 0) { val >>= 8; pos += 8; }
		if ((val & 0x0000000f) == 0) { val >>= 4; pos += 4; }
		if ((val & 0x00000003) == 0) { val >>= 2; pos += 2; }
		if ((val & 0x00000001) == 0) { pos += 1; }

		return pos;
	}

	static uint8_t FindLastBit(uint64_t val)
	{
		if (val == 0)
		{
			return 0;
		}

		uint8_t pos = 0;

		if ((val & 0xffffffff00000000ULL) != 0) { val >>= 32; pos += 32; }
		if ((val & 0x00000000ffff0000ULL) != 0) { val >>= 16; pos += 16; }
		if ((val & 0x000000000000ff00ULL) != 0) { val >>= 8; pos += 8; }
		if ((val & 0x00000000000000f0ULL) != 0) { val >>= 4; pos += 4; }
		if ((val & 0x000000000000000cULL) != 0) { val >>= 2; pos += 2; }
		if ((val & 0x0000000000000002ULL) != 0) { pos += 1; }

		return pos;
	}

	/// Greatest common divisor for integer types.
	template <typename T>
	static T GCD(T a, T b)
	{
		while (b != 0)
		{
			T t = b;
			b = a % b;
			a = t;
		}

		return a;
	}

	/// Least common multiple for integer types.
	template <typename T>
	static T LCM(T a, T b)
	{
		if (a == 0 || b == 0)
		{
			return 0;
		}
		if (a == 1 || a == b)
		{
			return b;
		}
		if (b == 1)
		{
			return a;
		}

		return ((a * b) / (GCD<T>(a, b)));
	}

	/// Returns a pseudorandom bit sequence using a linear feedback shift register starting from some 64-bit seed value.
	static uint64_t LFSR(uint64_t seed, size_t iterations)
	{
		// For n = 64 bits, feedback polynomial gives XOR bit positions [64,63,61,60,0]
		// Calculated tap bits to XOR against should be the number
		// 2^63 + 2^62 + 2^60 + 2^59 + 2^0 = 0xd800000000000001.
		const uint64_t mask = 0xd800000000000001;
		auto result = seed;
		uint64_t collector = 0;

		for (size_t i = 0; i < iterations; i++)
		{
			result <<= 1;
			auto XOR = result >> 63;
			if (XOR != 0)
			{
				result ^= mask;
			}

			//	printf(PRINTF_BINARY_PATTERN_INT64 "\n", PRINTF_BYTE_TO_BINARY_INT64(result));
			//	(result & 1) ? printf("1") : printf("0");

			// Instead of using i % 64, just mask [...]111111
			InsertBit(collector, (i & 0x3f), (result & 1));
		}
		//	printf("\n");
		//	printf("%I64i (" PRINTF_BINARY_PATTERN_INT64 ")\n", collector, PRINTF_BYTE_TO_BINARY_INT64(collector));

		return collector;
	}
}