#include "quickhull.hh"



ConvexHull QuickHull::GetConvexHull(const std::vector<glm::vec3>& pointCloud, bool CCW, bool useOriginalIndices, float epsilon)
{
	VertexDataSource vertexDataSource(pointCloud);
	return _GetConvexHull(vertexDataSource, CCW, useOriginalIndices, epsilon);
}

HalfEdgeMesh<size_t> QuickHull::GetConvexHullAsMesh(const glm::vec3* vertexData, size_t vertexCount, bool CCW, float epsilon)
{
	VertexDataSource vertexDataSource(vertexData, vertexCount);
	_BuildMesh(vertexDataSource, CCW, false, epsilon);
	return HalfEdgeMesh<size_t>(_mesh, _vertexData);
}

void QuickHull::_BuildMesh(const VertexDataSource& pointCloud, bool CCW, bool useOriginalIndices, float epsilon)
{
	// Unused for now
	(void)CCW;
	(void)useOriginalIndices;

	// Degenerate case: no vertices to wrap!
	if (pointCloud.size() == 0)
	{
		_mesh = MeshBuilder();
		return;
	}
	_vertexData = pointCloud;

	// Very first: find extreme values and use them to compute the scale of the point cloud.
	_extremeValues = _GetExtremeValues();
	_scale = _GetScale(_extremeValues);

	// Epsilon depends on the scale
	_epsilon = epsilon * _scale;
	_epsilonSquared = _epsilon * _epsilon;

	// Reset diagnostic data
	_diagnostics = DiagnosticData();

	_planar = false; // The planar case happens when all the points appear to lie on a two dimensional subspace of R^3.
	_CreateConvexHalfEdgeMesh();
	if (_planar)
	{
		const size_t extraPointIndex = _planarPointCloudTemp.size() - 1;
		for (auto& he : _mesh._halfEdges)
		{
			if (he._endVertex == extraPointIndex)
			{
				he._endVertex = 0;
			}
		}
		_vertexData = pointCloud;
		_planarPointCloudTemp.clear();
	}
}

ConvexHull QuickHull::_GetConvexHull(const VertexDataSource& pointCloud, bool CCW, bool useOriginalIndices, float epsilon)
{
	_BuildMesh(pointCloud, CCW, useOriginalIndices, epsilon);
	return ConvexHull(_mesh, _vertexData, CCW, useOriginalIndices);
}

void QuickHull::_CreateConvexHalfEdgeMesh() {
	_visibleFaces.clear();
	_horizonEdges.clear();
	_possiblyVisibleFaces.clear();

	// Compute base tetrahedron
	_SetupInitialTetrahedron();
	if (_mesh._faces.size() != 4)
	{
		DEBUG_LOG("QuickHull", LOG_ERROR, "Error setting up initial tetrahedron! [%i] faces generated, expected 4.", _mesh._faces.size());
	}

	// Init face stack with those faces that have points assigned to them
	_faceList.clear();
	for (size_t i = 0; i < 4; i++)
	{
		auto& f = _mesh._faces[i];
		if (f._pointsOnPositiveSide && f._pointsOnPositiveSide->size() > 0)
		{
			_faceList.push_back(i);
			f._inFaceStack = 1;
		}
	}

	// Process faces until the face list is empty
	size_t iter = 0;
	while (!_faceList.empty())
	{
		iter++;
		if (iter == std::numeric_limits<size_t>::max())
		{
			// Visible face traversal marks visited faces with iteration counter
			// (to mark that the face has been visited on this iteration) and the
			// max value represents unvisited faces. At this point we have to reset
			// iteration counter. This shouldn't be an issue on 64 bit machines.
			iter = 0;
		}

		const size_t topFaceIndex = _faceList.front();
		_faceList.pop_front();

		auto& tf = _mesh._faces[topFaceIndex];
		tf._inFaceStack = 0;

		if (!(!tf._pointsOnPositiveSide || tf._pointsOnPositiveSide->size() > 0))
		{
			DEBUG_LOG("QuickHull", LOG_ERROR, "Error creating the convex half-edge mesh. No points on positive side of a face or zero size.");
		}

		if (!tf._pointsOnPositiveSide || tf.IsDisabled())
		{
			continue;
		}

		// Pick the most distant point to this triangle plane as the point to which we extrude
		const glm::vec3& activePoint = _vertexData[tf._mostDistantPoint];
		const size_t activePointIndex = tf._mostDistantPoint;

		// Find out the faces that have our active point on their positive side (these are the "visible faces"). The face on top of the stack of course is one of them. At the same time, we create a list of horizon edges.
		_horizonEdges.clear();
		_possiblyVisibleFaces.clear();
		_visibleFaces.clear();
		_possiblyVisibleFaces.emplace_back(topFaceIndex, std::numeric_limits<size_t>::max());

		while (_possiblyVisibleFaces.size())
		{
			const auto faceData = _possiblyVisibleFaces.back();
			_possiblyVisibleFaces.pop_back();
			auto& pvf = _mesh._faces[faceData._faceIndex];
			if (pvf.IsDisabled())
			{
				DEBUG_LOG("QuickHull", LOG_ERROR, "Error creating the convex half-edge mesh. PVF were disabled.");
			}

			if (pvf._visibilityCheckedOnIteration == iter)
			{
				if (pvf._isVisibleFaceOnCurrentIteration)
				{
					continue;
				}
			}
			else
			{
				const Plane& P = pvf._P;
				pvf._visibilityCheckedOnIteration = iter;
				const float d = glm::dot(P._N, activePoint) + P._D;

				if (d > 0)
				{
					pvf._isVisibleFaceOnCurrentIteration = 1;
					pvf._horizonEdgesOnCurrentIteration = 0;
					_visibleFaces.push_back(faceData._faceIndex);
					for (auto heIndex : _mesh.GetHalfEdgeIndicesOfFace(pvf))
					{
						if (_mesh._halfEdges[heIndex]._opp != faceData._enteredFromHalfEdge)
						{
							_possiblyVisibleFaces.emplace_back(_mesh._halfEdges[_mesh._halfEdges[heIndex]._opp]._face, heIndex);
						}
					}
					continue;
				}

				if (faceData._faceIndex == topFaceIndex)
				{
					DEBUG_LOG("QuickHull", LOG_ERROR, "Error creating the convex half-edge mesh. Face index [%u] should not match top face index [%u].", faceData._faceIndex, topFaceIndex);
				}
			}

			// The face is not visible. Therefore, the halfedge we came from is part of the horizon edge.
			pvf._isVisibleFaceOnCurrentIteration = 0;
			_horizonEdges.push_back(faceData._enteredFromHalfEdge);
			// Store which half edge is the horizon edge. The other half edges of the face will not be part of the final mesh so their data slots can by recycled.
			const auto halfEdges = _mesh.GetHalfEdgeIndicesOfFace(_mesh._faces[_mesh._halfEdges[faceData._enteredFromHalfEdge]._face]);
			const int8_t ind = (halfEdges[0] == faceData._enteredFromHalfEdge) ? 0 : (halfEdges[1] == faceData._enteredFromHalfEdge ? 1 : 2);
			_mesh._faces[_mesh._halfEdges[faceData._enteredFromHalfEdge]._face]._horizonEdgesOnCurrentIteration |= (1 << ind);
		}
		const size_t horizonEdgeCount = _horizonEdges.size();

		// Order horizon edges so that they form a loop. This may fail due to numerical instability in which case we give up trying to solve horizon edge for this point and accept a minor degeneration in the convex hull.
		if (!_ReorderHorizonEdges(_horizonEdges))
		{
			_diagnostics._failedHorizonEdges++;
			DEBUG_LOG("QuickHull", LOG_ERROR, "Failed to resolve horizon edge!");
			auto it = std::find(tf._pointsOnPositiveSide->begin(), tf._pointsOnPositiveSide->end(), activePointIndex);
			tf._pointsOnPositiveSide->erase(it);
			if (tf._pointsOnPositiveSide->size() == 0)
			{
				_ReclaimToIndexVectorPool(tf._pointsOnPositiveSide);
			}
			continue;
		}

		// Except for the horizon edges, all half edges of the visible faces can be marked as disabled. Their data slots will be reused.
		// The faces will be disabled as well, but we need to remember the points that were on the positive side of them - therefore
		// we save pointers to them.
		_newFaceIndices.clear();
		_newHalfEdgeIndices.clear();
		_disabledFacePointVectors.clear();
		size_t disableCounter = 0;

		for (auto faceIndex : _visibleFaces)
		{
			auto& disabledFace = _mesh._faces[faceIndex];
			auto halfEdges = _mesh.GetHalfEdgeIndicesOfFace(disabledFace);
			for (size_t j = 0; j < 3; j++)
			{
				if ((disabledFace._horizonEdgesOnCurrentIteration & (1 << j)) == 0)
				{
					if (disableCounter < horizonEdgeCount * 2)
					{
						// Use on this iteration
						_newHalfEdgeIndices.push_back(halfEdges[j]);
						disableCounter++;
					}
					else
					{
						// Mark for reusal on later iteration step
						_mesh.DisableHalfEdge(halfEdges[j]);
					}
				}
			}
			// Disable the face, but retain pointer to the points that were on the positive side of it. We need to assign those points
			// to the new faces we create shortly.
			auto t = std::move(_mesh.DisableFace(faceIndex));
			if (t)
			{
				if (!(t->size()))
				{
					DEBUG_LOG("QuickHull", LOG_WARN, "Possible error creating the convex half-edge mesh. Nonzero pointer to disabled faces. QuickHull should not assign point vectors to faces unless needed.");
				}
				_disabledFacePointVectors.push_back(std::move(t));
			}
		}

		if (disableCounter < horizonEdgeCount * 2)
		{
			const size_t newHalfEdgesNeeded = horizonEdgeCount * 2 - disableCounter;
			for (size_t i = 0; i < newHalfEdgesNeeded; i++)
			{
				_newHalfEdgeIndices.push_back(_mesh.AddHalfEdge());
			}
		}

		// Create new faces using the edgeloop
		for (size_t i = 0; i < horizonEdgeCount; i++)
		{
			const size_t AB = _horizonEdges[i];

			auto horizonEdgeVertexIndices = _mesh.GetVertexIndicesOfHalfEdge(_mesh._halfEdges[AB]);
			size_t A, B, C;
			A = horizonEdgeVertexIndices[0];
			B = horizonEdgeVertexIndices[1];
			C = activePointIndex;

			const size_t newFaceIndex = _mesh.AddFace();
			_newFaceIndices.push_back(newFaceIndex);

			const size_t CA = _newHalfEdgeIndices[2 * i + 0];
			const size_t BC = _newHalfEdgeIndices[2 * i + 1];

			_mesh._halfEdges[AB]._next = BC;
			_mesh._halfEdges[BC]._next = CA;
			_mesh._halfEdges[CA]._next = AB;

			_mesh._halfEdges[BC]._face = newFaceIndex;
			_mesh._halfEdges[CA]._face = newFaceIndex;
			_mesh._halfEdges[AB]._face = newFaceIndex;

			_mesh._halfEdges[CA]._endVertex = A;
			_mesh._halfEdges[BC]._endVertex = C;

			auto& newFace = _mesh._faces[newFaceIndex];

			const glm::vec3 planeNormal = Math::GetTriangleNormal(_vertexData[A], _vertexData[B], activePoint);
			newFace._P = Plane(planeNormal, activePoint);
			newFace._he = AB;

			_mesh._halfEdges[CA]._opp = _newHalfEdgeIndices[i > 0 ? i * 2 - 1 : 2 * horizonEdgeCount - 1];
			_mesh._halfEdges[BC]._opp = _newHalfEdgeIndices[((i + 1) * 2) % (horizonEdgeCount * 2)];
		}

		// Assign points that were on the positive side of the disabled faces to the new faces.
		for (auto& disabledPoints : _disabledFacePointVectors)
		{
			if (!disabledPoints)
			{
				DEBUG_LOG("QuickHull", LOG_ERROR, "Pointer to disable points didn't exist when assigning positive-sided points of disabled faces to new faces!");
			}

			for (const auto& point : *(disabledPoints))
			{
				if (point == activePointIndex)
				{
					continue;
				}
				for (size_t j = 0; j < horizonEdgeCount; j++)
				{
					if (_AddPointToFace(_mesh._faces[_newFaceIndices[j]], point))
					{
						break;
					}
				}
			}

			// The points are no longer needed: we can move them to the vector pool for reuse.
			_ReclaimToIndexVectorPool(disabledPoints);
		}

		// Increase face stack size if needed
		for (const auto newFaceIndex : _newFaceIndices)
		{
			auto& newFace = _mesh._faces[newFaceIndex];
			if (newFace._pointsOnPositiveSide)
			{
				if (!(newFace._pointsOnPositiveSide->size() > 0))
				{
					DEBUG_LOG("QuickHull", LOG_ERROR, "No points on positive side of a new face on the face stack!");
				}

				if (!newFace._inFaceStack)
				{
					_faceList.push_back(newFaceIndex);
					newFace._inFaceStack = 1;
				}
			}
		}
	}

	// Cleanup
	_indexVectorPool.Clear();
}

/*
 * Private helper functions
 */

std::array<size_t, 6> QuickHull::_GetExtremeValues()
{
	std::array<size_t, 6> outIndices{ 0, 0, 0, 0, 0, 0 };
	float extremeVals[6] = 
	{
		_vertexData[0].x,
		_vertexData[0].x,
		_vertexData[0].y,
		_vertexData[0].y,
		_vertexData[0].z,
		_vertexData[0].z
	};
	const size_t vCount = _vertexData.size();
	for (size_t i = 1; i < vCount; i++)
	{
		const glm::vec3& pos = _vertexData[i];

		if (pos.x > extremeVals[0])
		{
			extremeVals[0] = pos.x;
			outIndices[0] = i;
		}
		else if (pos.x < extremeVals[1])
		{
			extremeVals[1] = pos.x;
			outIndices[1] = i;
		}

		if (pos.y > extremeVals[2])
		{
			extremeVals[2] = pos.y;
			outIndices[2] = i;
		}
		else if (pos.y < extremeVals[3])
		{
			extremeVals[3] = pos.y;
			outIndices[3] = i;
		}

		if (pos.z > extremeVals[4])
		{
			extremeVals[4] = pos.z;
			outIndices[4] = i;
		}
		else if (pos.z < extremeVals[5])
		{
			extremeVals[5] = pos.z;
			outIndices[5] = i;
		}
	}

	return outIndices;
}

bool QuickHull::_ReorderHorizonEdges(std::vector<size_t>& horizonEdges)
{
	const size_t horizonEdgeCount = horizonEdges.size();
	for (size_t i = 0; i < horizonEdgeCount - 1; i++)
	{
		const size_t endVertex = _mesh._halfEdges[horizonEdges[i]]._endVertex;
		bool foundNext = false;
		for (size_t j = i + 1; j < horizonEdgeCount; j++) {
			const size_t beginVertex = _mesh._halfEdges[_mesh._halfEdges[horizonEdges[j]]._opp]._endVertex;
			if (beginVertex == endVertex) {
				std::swap(horizonEdges[i + 1], horizonEdges[j]);
				foundNext = true;
				break;
			}
		}

		if (!foundNext)
		{
			return false;
		}
	}

	if (!(_mesh._halfEdges[horizonEdges[horizonEdges.size() - 1]]._endVertex == _mesh._halfEdges[_mesh._halfEdges[horizonEdges[0]]._opp]._endVertex))
	{
		DEBUG_LOG("QuickHull", LOG_ERROR, "Error reordering horizon edges!");
	}
	return true;
}

float QuickHull::_GetScale(const std::array<size_t, 6>& extremeValues)
{
	float s = 0;
	for (size_t i = 0; i < 6; i++)
	{
		const float* v = (const float*)(&_vertexData[extremeValues[i]]);
		v += i / 2;
		auto a = std::abs(*v);

		if (a > s)
		{
			s = a;
		}
	}

	return s;
}

void QuickHull::_SetupInitialTetrahedron()
{
	const size_t vertexCount = _vertexData.size();

	// If we have at most 4 points, just return a degenerate tetrahedron:
	if (vertexCount <= 4)
	{
		size_t v[4] =
		{ 
			0,
			std::min((size_t)1,vertexCount - 1),
			std::min((size_t)2,vertexCount - 1),
			std::min((size_t)3,vertexCount - 1)
		};

		const glm::vec3 N = Math::GetTriangleNormal(_vertexData[v[0]], _vertexData[v[1]], _vertexData[v[2]]);
		const Plane trianglePlane(N, _vertexData[v[0]]);
		if (trianglePlane.IsPointOnPositiveSide(_vertexData[v[3]]))
		{
			std::swap(v[0], v[1]);
		}
		return _mesh.Setup(v[0], v[1], v[2], v[3]);
	}

	// Find two most distant extreme points.
	float maxD = _epsilonSquared;
	std::pair<size_t, size_t> selectedPoints;
	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = i + 1; j < 6; j++)
		{
			// Get the squared distance from _vertexData[_extremeValues[i]] to _vertexData[_extremeValues[j]]
			const float dx = _vertexData[_extremeValues[i]].x - _vertexData[_extremeValues[j]].x;
			const float dy = _vertexData[_extremeValues[i]].y - _vertexData[_extremeValues[j]].y;
			const float dz = _vertexData[_extremeValues[i]].z - _vertexData[_extremeValues[j]].z;
			const float d = (dx*dx) + (dy*dy) + (dz*dz);

			if (d > maxD)
			{
				maxD = d;
				selectedPoints = { _extremeValues[i], _extremeValues[j] };
			}
		}
	}

	if (maxD == _epsilonSquared)
	{
		// A degenerate case: the point cloud seems to consists of a single point
		return _mesh.Setup(0, std::min((size_t)1, vertexCount - 1), std::min((size_t)2, vertexCount - 1), std::min((size_t)3, vertexCount - 1));
	}

	if (selectedPoints.first == selectedPoints.second)
	{
		DEBUG_LOG("QuickHull", LOG_ERROR, "Selected two most distant extreme points (IDs [%i] and [%i]) are identical. Possible degenerate mesh.",
			selectedPoints.first, selectedPoints.second);
	}

	// Find the most distant point to the line between the two chosen extreme points.
	const Ray r(_vertexData[selectedPoints.first], (_vertexData[selectedPoints.second] - _vertexData[selectedPoints.first]));
	maxD = _epsilonSquared;
	size_t maxI = std::numeric_limits<size_t>::max();
	const size_t vCount = _vertexData.size();
	for (size_t i = 0; i < vCount; i++) {
		const float distToRay = Math::SquaredDistanceBetweenPointAndRay(_vertexData[i], r);
		if (distToRay > maxD)
		{
			maxD = distToRay;
			maxI = i;
		}
	}
	if (maxD == _epsilonSquared)
	{
		// It appears that the point cloud belongs to a 1 dimensional subspace of R^3: convex hull has no volume => return a thin triangle
		// Pick any point other than selectedPoints.first and selectedPoints.second as the third point of the triangle
		auto it = std::find_if(_vertexData.begin(), _vertexData.end(), [&](const glm::vec3& ve) {
			return ve != _vertexData[selectedPoints.first] && ve != _vertexData[selectedPoints.second];
			});
		const size_t thirdPoint = (it == _vertexData.end()) ? selectedPoints.first : std::distance(_vertexData.begin(), it);
		it = std::find_if(_vertexData.begin(), _vertexData.end(), [&](const glm::vec3& ve) {
			return ve != _vertexData[selectedPoints.first] && ve != _vertexData[selectedPoints.second] && ve != _vertexData[thirdPoint];
			});
		const size_t fourthPoint = (it == _vertexData.end()) ? selectedPoints.first : std::distance(_vertexData.begin(), it);
		return _mesh.Setup(selectedPoints.first, selectedPoints.second, thirdPoint, fourthPoint);
	}

	// These three points form the base triangle for our tetrahedron.
	if (!(selectedPoints.first != maxI && selectedPoints.second != maxI))
	{
		DEBUG_LOG("QuickHull", LOG_ERROR, "Selected extreme point(s) (IDs [%i] and [%i]) is(are) equal to integer limit. Possible degenerate mesh.",
			selectedPoints.first, selectedPoints.second);
	}
	std::array<size_t, 3> baseTriangle{ selectedPoints.first, selectedPoints.second, maxI };
	const glm::vec3 baseTriangleVertices[] = { _vertexData[baseTriangle[0]], _vertexData[baseTriangle[1]], _vertexData[baseTriangle[2]] };

	// Next step is to find the 4th vertex of the tetrahedron. We naturally choose the point farthest away from the triangle plane.
	maxD = _epsilon;
	maxI = 0;
	const glm::vec3 N = Math::GetTriangleNormal(baseTriangleVertices[0], baseTriangleVertices[1], baseTriangleVertices[2]);
	Plane trianglePlane(N, baseTriangleVertices[0]);
	for (size_t i = 0; i < vCount; i++)
	{
		const float d = std::abs(Math::DistanceToPlaneSigned(_vertexData[i], trianglePlane));
		if (d > maxD)
		{
			maxD = d;
			maxI = i;
		}
	}
	if (maxD == _epsilon)
	{
		// All the points seem to lie on a 2D subspace of R^3. How to handle this? Well, let's add one extra point to the point cloud so that the convex hull will have volume.
		_planar = true;
		const glm::vec3 N1 = Math::GetTriangleNormal(baseTriangleVertices[1], baseTriangleVertices[2], baseTriangleVertices[0]);
		_planarPointCloudTemp.clear();
		_planarPointCloudTemp.insert(_planarPointCloudTemp.begin(), _vertexData.begin(), _vertexData.end());
		const glm::vec3 extraPoint = N1 + _vertexData[0];
		_planarPointCloudTemp.push_back(extraPoint);
		maxI = _planarPointCloudTemp.size() - 1;
		_vertexData = VertexDataSource(_planarPointCloudTemp);
	}

	// Enforce CCW orientation (if user prefers clockwise orientation, swap two vertices in each triangle when final mesh is created)
	const Plane triPlane(N, baseTriangleVertices[0]);
	if (triPlane.IsPointOnPositiveSide(_vertexData[maxI]))
	{
		std::swap(baseTriangle[0], baseTriangle[1]);
	}

	// Create a tetrahedron half edge mesh and compute planes defined by each triangle
	_mesh.Setup(baseTriangle[0], baseTriangle[1], baseTriangle[2], maxI);
	for (auto& f : _mesh._faces)
	{
		auto v = _mesh.GetVertexIndicesOfFace(f);
		const glm::vec3& va = _vertexData[v[0]];
		const glm::vec3& vb = _vertexData[v[1]];
		const glm::vec3& vc = _vertexData[v[2]];
		const glm::vec3 N1 = Math::GetTriangleNormal(va, vb, vc);
		const Plane plane(N1, va);
		f._P = plane;
	}

	// Finally we assign a face for each vertex outside the tetrahedron (vertices inside the tetrahedron have no role anymore)
	for (size_t i = 0; i < vCount; i++)
	{
		for (auto& face : _mesh._faces)
		{
			if (_AddPointToFace(face, i))
			{
				break;
			}
		}
	}
}
