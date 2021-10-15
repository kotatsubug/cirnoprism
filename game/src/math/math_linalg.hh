#pragma once

#include <variant>
#include <math.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// This is my vector and matrix math library
// Eventually once this is stable enough to carry out the existing functions of GLM + the stuff I'd like to implement, I'll replace GLM with this
// Especially since GLM is a little limited with what it can do with complex numbers and quaternions

namespace qt
{
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
			: _N(N), _D(glm::dot(-N, P)), _NLength2(_N.x* _N.x + _N.y * _N.y + _N.z * _N.z)
		{
		}
	};

	/// Pair of 3D vectors, also stores inverse square distance between endpoints.
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
	static inline glm::vec3 GetTriangleNormalCW(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
	{
		return glm::cross((A - C), (B - C));
	}

	static inline float SquaredDistanceBetweenPointAndRay(const glm::vec3& P, const Ray& R)
	{
		const glm::vec3 s = P - R._S;
		float t = glm::dot(s, R._V);
		return (s.x * s.y * s.z) - (t * t * R._VInvLength2);
	}

	/// Linear interpolation between two 3D positions.
	/// Factor is the interpolation factor in range [0,1].
	static inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float factor)
	{
		return glm::vec3(
			a.x + (b.x - a.x)*factor,
			a.y + (b.y - a.y)*factor,
			a.z + (b.z - a.z)*factor
		);
	}


/*
	class fcomplex
	{
	private:
		float _real, _imag;
	public:
		fcomplex(float r = 0.0f, float i = 0.0f);

		float modulus();
		float argument();

		fcomplex operator+(const fcomplex& c) const;
		fcomplex operator-(const fcomplex& c) const;
	};

	fcomplex::fcomplex(float r, float i)
	{
		_real = r;
		_imag = i;
	}

	float fcomplex::modulus()
	{
		return sqrt(_real*_real + _imag*_imag);
	}

	float fcomplex::argument()
	{
		return atan(_imag / _real);
	}

	fcomplex fcomplex::operator+(const fcomplex& c) const
	{
		fcomplex result;
		result._real = (_real + c._real);
		result._imag = (_imag + c._imag);
		return result;
	}

	fcomplex fcomplex::operator-(const fcomplex& c) const
	{
		fcomplex result;
		result._real = (_real - c._real);
		result._imag = (_imag - c._imag);
		return result;
	}




	struct fvec2
	{
		float x, y;

		fvec2(float x, float y)
			: x(x), y(y)
		{
		}

		inline fvec2 operator+(const fvec2& v) const
		{
			return fvec2(x + v.x, y + v.y);
		}
	};

	struct fvec3
	{
		float x, y, z;

		fvec3(float x, float y, float z)
			: x(x), y(y), z(z)
		{
		}

		inline fvec3 operator+(const fvec3& v) const
		{
			return fvec3(x + v.x, y + v.y, z + v.z);
		}
	};

	auto cross(const fvec2& a, const fvec2& b)
	{
		return a.x*b.y + a.y*b.x;
	}

	auto cross(const fvec3& a, const fvec3& b)
	{
		return fvec3(
			a.y*b.z - a.z*b.y,
			a.z*b.x - a.x*b.z,
			a.x*b.y - a.y*b.x
		);
	}

	auto dot(const fvec2& a, const fvec2& b)
	{

	}

	auto dot(const fvec3& a, const fvec3& b)
	{

	}

	/// Squared distance from one vector to another.
	float dist2(const fvec2& p0, const fvec2& p1)
	{
		return ((p1.x - p0.x) * (p1.x - p0.x) 
			+ (p1.y - p0.y) * (p1.y - p0.y));
	}

	/// Squared distance from one vector to another.
	float dist2(const fvec3& p0, const fvec3& p1)
	{
		return ((p1.x - p0.x) * (p1.x - p0.x)
			+ (p1.y - p0.y) * (p1.y - p0.y)
			+ (p1.z - p0.z) * (p1.z - p0.z));
	}

	// Length of 1 vector

	// Normalize

	// Faceforward

	// Reflect

	// Refract
	*/

}
