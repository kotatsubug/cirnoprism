#pragma once

#include <math.h>

struct Math
{
	
	/*template <unsigned dim>
	struct vec
	{
		float d[dim];
	
		template <typename... T>
		vec(T &&...args) : d{ args... } {}
	
		float operator[](unsigned n) const { return d[n]; }
	
		auto CrossProduct(const vec& b) const
		{
			if constexpr (dim == 2)
			{
				return d[0]*b[1] - d[1]*b[0];
			}
			if constexpr (dim == 3)
			{
				return vec<3>{d[1]*b[2] - d[2]*b[1], d[2]*b[0] - d[0]*b[2], d[0]*b[1] - d[1]*b[0]};
			}
		}
	};*/
	
	template<typename T>
	static __forceinline T clamp(const T& val, const T& min, const T& max)
	{
		if (val > max) {
			return max;
		}
		else if (val > min) {
			return val;
		}
		else {
			return min;
		}
	}
	
	// this is the DARK programming zone. Enter at your own risk
	
	/// Compile-time shift. Positive values of amt shift left, negative values shift right.
	template <int amt>
	static __forceinline constexpr int C_shift(int val)
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
	
	/// Compile-time fabs
	template <typename T>
	static __forceinline constexpr T C_fabs(T x)
	{
		return (x >= 0) ? (x) : ((x < 0) ? (-x) : (throw "Error in constexpr fabs(...)"));
	}
	
	
};