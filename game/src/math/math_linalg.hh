/*#pragma once

#include <variant>
#include <math.h>

// This is my vector and matrix math library
// Eventually once this is stable enough to carry out the existing functions of GLM + the stuff I'd like to implement, I'll replace GLM with this

namespace Math
{
	template<unsigned dim>
	struct Vec
	{
		float _d[dim];

		template<typename ...T>
		Vec(T&&... args)
			: _d{ args... }
		{
		}

		float operator[] (unsigned n) const
		{
			return _d[n];
		}





	};

	/// Cross product.
	/// For 2-dimensional vectors, result is a scalar.
	/// For 3-dimensional vectors, result is a vector.
	template<unsigned dim>
	auto Cross(const Vec<dim>& a, const Vec<dim>& b)
	{
		if constexpr (dim == 2)
		{
			return a[0] * b[1] + a[1] * b[0];
		}
		if constexpr (dim == 3)
		{
			return vec<3>{
				a[1] * b[2] - a[2] * b[1],
				a[2] * b[0] - a[0] * b[2],
				a[0] * b[1] - a[1] * b[0]
			};
		}
	}

	/// Squared distance from one vector to another.
	template<unsigned dim>
	float Distance2(const Vec<dim>& p0, const Vec<dim>& p1)
	{
		if constexpr (dim == 2)
		{
			return ((p1.x - p0.x) * (p1.x - p0.x) 
				  + (p1.y - p0.y) * (p1.y - p0.y));
		}
		if constexpr (dim == 3)
		{
			return ((p1.x - p0.x) * (p1.x - p0.x) 
				  + (p1.y - p0.y) * (p1.y - p0.y) 
				  + (p1.z - p0.z) * (p1.z - p0.z));
		}
		if constexpr (dim == 4)
		{
			return ((p1.x - p0.x) * (p1.x - p0.x) 
				  + (p1.y - p0.y) * (p1.y - p0.y) 
				  + (p1.z - p0.z) * (p1.z - p0.z) 
				  + (p1.w - p0.w) * (p1.w - p0.w));
		}
	}

	/// Real distance from one vector to another.
	template<unsigned dim>
	float Distance(const Vec<dim>& p0, const Vec<dim>& p1)
	{
		return std::sqrt(Distance2<dim>(p0, p1));
	}

	// Length of 1 vector

	// Dot

	// Normalize

	// Faceforward

	// Reflect

	// Refract


}
*/