#pragma once

#include <math.h>
#include <stddef.h>
#include <type_traits>
#include <utility>
#include <memory>

#include <glm.hpp>

namespace qt
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