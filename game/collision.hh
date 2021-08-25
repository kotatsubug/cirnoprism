#pragma once

#include "renderer/mesh.hh"



struct AABB
{
	glm::vec3 min;
	glm::vec3 max;
};


class BoundingVolume
{



	// Collision detection is split into two phases: the broad phase and the narrow phase
	// 
	// Broad phase: 1. Use spatial partioning (sort & sweep, bounding volume hierarchies, whatever) to determine which AABB volumes to test
	//				2. Then, determine whether two meshes COULD be colliding with each other by testing AABB intersection
	// 
	// Narrow phase: If the bounding volumes intersect, employ a more complex O(n^2) pairwise algorithm to see exactly IF they intersect.
	//					Thinking of using separating axis tests by constructing planes, need to do more reading


	/// Creates the smallest possible axis-aligned bounding box around a given mesh that encapsulates all vertices
	AABB GenerateAABB(Mesh& mesh);

	/// Creates a polyhedral convex hull using a quickhull algorithm, intended time complexity O(n log n)
//	ConvexHull Quickhull(Mesh& mesh);

};