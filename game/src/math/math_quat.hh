#pragma once

#include <math.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace qt
{
	/// Quaternion class, representation equivalent to w*1 + x*i + y*j + z*k.
	/// Values can be normalized for 3D rotation.
	class Quaternion
	{
	public:
		float x, y, z, w;

		Quaternion() { }

		/// Pass normalize=0 if this quaternion should NOT be normalized.
		Quaternion(float w, float x, float y, float z, const bool normalize = true)
			: w(w), x(x), y(y), z(z)
		{
			if (normalize)
			{
				Normalize();
			}
		}

		/// Unit quaternion constructor, calculates w based on the fact that the norm of a unit quaternion is 1.
		Quaternion(float x, float y, float z)
			: x(x), y(y), z(z)
		{
			const float t = 1.0f - (x*x) - (y*y) - (z*z);

			if (t < 0.0f)
			{
				w = 0.0f;
			}
			else
			{
				w = -sqrt(t);
			}

			Normalize();
		}

		/// Create a quaternion from a 4x4 transformation matrix, pass normalize=0 if this quaternion should NOT be normalized.
		Quaternion(const glm::mat4& transformationMatrix, const uint32_t& normalize = 1)
		{
			Quaternion(Quaternion::ExtractRotationQuaternionFromTransformMat(transformationMatrix));

			if (normalize)
			{
				Normalize();
			}
		}

		Quaternion(const Quaternion& other)
			: x(other.x), y(other.y), z(other.z), w(other.w)
		{

		}

		~Quaternion()
		{

		}
		
		void Normalize()
		{
			float magnitude = static_cast<float>(sqrt(w*w + x*x + y*y + z*z));
			w /= magnitude;
			x /= magnitude;
			y /= magnitude;
			z /= magnitude;
		}

		/// Returns the quaternion as a transformation matrix with position (0,0,0) and scale 1.
		glm::mat4 GetRotationTransformMat()
		{
			glm::mat4 matrix(1.0f);

			// For quaternion qw + i*qx + j*qy + k*qz, equivalent 3x3 rotation matrix
			// in a 4x4 transformation matrix is as follows:

			// | 1 - 2*qy^2 - 2*qz^2 |  2*qx*qy - 2*qz*qw  |  2*qx*qz + 2*qy*qw  |        0       |
			// |  2*qx*qy + 2*qz*qw  | 1 - 2*qx^2 - 2*qz^2 |  2*qy*qz - 2*qx*qw  |        0       |
			// |  2*qx*qz - 2*qy*qw  |  2*qy*qz + 2*qx*qw  | 1 - 2*qx^2 - 2*qy^2 |        0       |
			// |          0          |          0          |          0          |        1       |

			const float xy = x*y;
			const float xz = x*z;
			const float xw = x*w;
			const float yz = y*z;
			const float yw = y*w;
			const float zw = z*w;
			const float x2 = x*x;
			const float y2 = y*y;
			const float z2 = z*z;

			matrix[0][0] = 1.0f - 2.0f*(y2 + z2);
			matrix[0][1] = 2.0f*(xy - zw);
			matrix[0][2] = 2.0f*(xz + yw);
			matrix[0][3] = 0.0f;

			matrix[1][0] = 2.0f*(xy + zw);
			matrix[1][1] = 1.0f - 2.0f*(x2 + z2);
			matrix[1][2] = 2.0f*(yz - xw);
			matrix[1][3] = 0.0f;

			matrix[2][0] = 2.0f*(xz - yw);
			matrix[2][1] = 2.0f*(yz + xw);
			matrix[2][2] = 1.0f - 2.0f*(x2 + y2);
			matrix[2][3] = 0.0f;

			matrix[3][0] = 0.0f;
			matrix[3][1] = 0.0f;
			matrix[3][2] = 0.0f;
			matrix[3][3] = 1.0f;

			return matrix;
		}

		/// Rotates a point by a unit quaternion.
		static glm::vec3 RotatePoint(const glm::vec3& pos, const Quaternion& nRot)
		{
			// P = [0, px, py, pz]
			// R = [w, x, y, z]
			// R' = R* = [w, -x, -y, -z] because |R| = 1
			
			// P' = RPR*
			const Quaternion p = Quaternion(0.0f, pos.x, pos.y, pos.z, false); // Treat position as a quaternion. Do NOT normalize!
			const Quaternion result = Multiply(Multiply(nRot, p), Conjugate(nRot));

			return glm::vec3(result.x, result.y, result.z);
		}

		/// Computes Hamilton product. Does NOT normalize result.
		/// NOTE: Quaternion multiplication is NOT commutative.
		static inline Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs)
		{
			return Quaternion(
				lhs.w*rhs.w - lhs.x*rhs.x - lhs.y*rhs.y - lhs.z*rhs.z,
				lhs.w*rhs.x + lhs.x*rhs.w + lhs.y*rhs.z - lhs.z*rhs.y, // i
				lhs.w*rhs.y - lhs.x*rhs.z + lhs.y*rhs.w + lhs.z*rhs.x, // j
				lhs.w*rhs.z + lhs.x*rhs.y - lhs.y*rhs.x + lhs.z*rhs.w, // k
				false
			);
		}

		/// |Q|^2
		static inline float NormSquared(const Quaternion& q)
		{
			return (q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
		}

		/// Q^-1
		/// NOTE: For unit quaternions, consider getting the conjugate instead. If |Q|=1, Q^-1 = Q*/1 = Q*
		static inline Quaternion Inverse(const Quaternion& q)
		{
			// Q^-1 = Q*/|Q|^2
			const float invNormSq = 1.0f / NormSquared(q);
			const Quaternion qstar = Conjugate(q);
			return Quaternion(invNormSq*qstar.w, invNormSq*qstar.x, invNormSq*qstar.y, invNormSq*qstar.z);
		}

		/// Q*
		static inline Quaternion Conjugate(const Quaternion& q)
		{
			return Quaternion(q.w, -q.x, -q.y, -q.z);
		}

		/// For a given transformation matrix, returns the quaternion associated with the rotational component of that matrix.
		static Quaternion ExtractRotationQuaternionFromTransformMat(const glm::mat4& matrix, const bool normalize = true)
		{
			float wOut, xOut, yOut, zOut;
			float rotDiagonalSum = matrix[0][0] + matrix[1][1] + matrix[2][2];

			if (rotDiagonalSum > 0.0f)
			{
				float w4 = static_cast<float>(2.0f*sqrt(1.0f + rotDiagonalSum));
				wOut = w4 / 4.0f;
				xOut = (matrix[2][1] - matrix[1][2]) / w4;
				yOut = (matrix[0][2] - matrix[2][0]) / w4;
				zOut = (matrix[1][0] - matrix[0][1]) / w4;
			}
			else if ((matrix[0][0] > matrix[1][1]) && (matrix[0][0] > matrix[2][2]))
			{
				float x4 = static_cast<float>(2.0f*sqrt(1.0f + matrix[0][0] - matrix[1][1] - matrix[2][2]));
				wOut = (matrix[2][1] - matrix[1][2]) / x4;
				xOut = x4 / 4.0f;
				yOut = (matrix[0][1] + matrix[1][0]) / x4;
				zOut = (matrix[0][2] + matrix[2][0]) / x4;
			}
			else if (matrix[1][1] > matrix[2][2])
			{
				float y4 = static_cast<float>(2.0f*sqrt(1.0f + matrix[1][1] - matrix[0][0] - matrix[2][2]));
				wOut = (matrix[0][2] - matrix[2][0]) / y4;
				xOut = (matrix[0][1] + matrix[1][0]) / y4;
				yOut = y4 / 4.0f;
				zOut = (matrix[1][2] + matrix[2][1]) / y4;
			}
			else
			{
				float z4 = static_cast<float>(2.0f*sqrt(1.0f + matrix[2][2] - matrix[0][0] - matrix[1][1]));
				wOut = (matrix[1][0] - matrix[0][1]) / z4;
				xOut = (matrix[0][2] + matrix[2][0]) / z4;
				yOut = (matrix[1][2] + matrix[2][1]) / z4;
				zOut = z4 / 4.0f; 
			}

			return Quaternion(wOut, xOut, yOut, zOut, normalize);
		}

		/// Normalized linear interpolation between two quaternions. Interpolation factor is in range [0,1].
		/// Best suited for animation, NLERP is communative, less computationally expensive, and
		/// provides a normalized quaternion, but does NOT maintain constant velocity like how SLERP does.
		static Quaternion NLerp(const Quaternion& a, const Quaternion& b, float f)
		{
			const float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
			const float fInv = 1.0f - f;

			Quaternion result(1.0f, 0.0f, 0.0f, 0.0f, false);
			if (dot < 0.0f)
			{
				result.w = fInv*a.w + f*(-b.w);
				result.x = fInv*a.x + f*(-b.x);
				result.y = fInv*a.y + f*(-b.y);
				result.z = fInv*a.z + f*(-b.z);
			}
			else
			{
				result.w = fInv*a.w + f*(b.w);
				result.x = fInv*a.x + f*(b.x);
				result.y = fInv*a.y + f*(b.y);
				result.z = fInv*a.z + f*(b.z);
			}

			result.Normalize();
			return result;
		}
	};
}