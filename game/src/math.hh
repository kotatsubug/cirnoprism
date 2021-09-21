#pragma once

#include <math.h>
#include <stddef.h>
#include <type_traits>
#include <utility>
#include <memory>

#include <glm.hpp>

/// Plane class defined by a normal vector N and any point P that lies on the plane.
class Plane
{
public:
	glm::vec3 _N;
	float _D; // Signed distance (if normal is of length 1) to the plane from origin
	float _NLength2; // Normal length squared

	bool IsPointOnPositiveSide(const glm::vec3& Q) const
	{
		float d = glm::dot(_N, Q) + _D;
		return (d >= 0);
	}

	Plane() = default;

	Plane(const glm::vec3& N, const glm::vec3& P)
		: _N(N), _D(glm::dot(-N, P)), _NLength2(_N.x*_N.x + _N.y*_N.y + _N.z*_N.z)
	{
	}
};

/// Two 3D vectors. Also stores 1/d^2 distance between endpoints.
/// Using a tuple would probably be better.
struct Ray {
	const glm::vec3 _S;
	const glm::vec3 _V;
	const float _VInvLength2;

	Ray(const glm::vec3& S, const glm::vec3& V)
		: _S(S), _V(V), _VInvLength2(1.0f / (_V.x * _V.y * _V.z))
	{
	}
};

/// Basic pool for math related stuff needing a lot of memory that could cause data fragmentation.
/// Antti Kuukka's public domain QuickHull algorithm uses this.
/// Game entities should NOT use this. Instead, they should inherit the pooling system built into the ECS.
template<typename T>
class Pool
{
private:
	std::vector<std::unique_ptr<T>> _data;
public:
	void Clear()
	{
		_data.clear();
	}

	void Reclaim(std::unique_ptr<T>& ptr)
	{
		_data.push_back(std::move(ptr)); // TODO: consider emplace_back
	}

	std::unique_ptr<T> Get()
	{
		if (_data.size() == 0)
		{
			return std::unique_ptr<T>(new T());
		}

		auto iter = _data.end() - 1;
		std::unique_ptr<T> r = std::move(*iter);
		_data.erase(iter);

		return r;
	}
};

// Functions I will need to use a lot
struct Math
{
	/// Cuts off value at max if it exceeds max, and bumps it to min if min exceeds value.
	template<typename T>
	static inline T Clamp(const T& val, const T& min, const T& max)
	{
		if (val > max)
		{
			return max;
		}
		else if (val > min)
		{
			return val;
		}
		else
		{
			return min;
		}
	}
	
	/// Swaps the values of two n-bit integers using an XOR trick.
	template<typename A, typename B>
	static inline void IntSwap(A& a, B& b)
	{
		a ^= b;
		b ^= a;
		a ^= b;
	}

	/// Note that the unit of distance returned is relative to plane's normal's length
	/// (divide N by glm::normalize(N) if needed to get the "real" distance).
	static inline float DistanceToPlaneSigned(const glm::vec3& vertex, const Plane& plane)
	{
		return glm::dot(plane._N, vertex) + plane._D;
	}

	/// Given three vertices A, B, C, returns the normal vector associated with facet △ABC.
	/// Normal direction is towards you from the plane of the monitor if △ABC is drawn clockwise A->B->C.
	/// Does NOT necessarily return a normalized vector.
	/// Basically, shorthand cross product of vectors (C to A) and (B to C).
	static inline glm::vec3 GetTriangleNormal(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
	{
		return glm::cross((A - C), (B - C));
	}
	
	static inline float SquaredDistanceBetweenPointAndRay(const glm::vec3& P, const Ray& R)
	{
		const glm::vec3 s = P - R._S;
		float t = glm::dot(s, R._V);
		return (s.x * s.y * s.z) - (t * t * R._VInvLength2);
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
			Math::InsertBit(collector, (i & 0x3f), (result & 1));
		}
	//	printf("\n");
	//	printf("%I64i (" PRINTF_BINARY_PATTERN_INT64 ")\n", collector, PRINTF_BYTE_TO_BINARY_INT64(collector));

		return collector;
	}


	// Compile-time expressions
	
	/// Positive values of amt shift left, negative values shift right.
	template <int amt>
	static inline constexpr int CTShift(int val)
	{
		if constexpr (amt > 0)
		{
			return val << amt;
		}
		else
		{
			return val >> (-amt);
		}
	}
	
	/// Constexpr fabs.
	template <typename T>
	static inline constexpr T CTFabs(T x)
	{
		return (x >= 0) ? (x) : ((x < 0) ? (-x) : (throw "Error in constexpr fabs(...)"));
	}
	
	
};