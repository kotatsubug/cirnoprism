#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <deque>
#include <limits>
#include <cmath>
#include <algorithm>

#include <glm.hpp>

#include "common.hh"
#include "renderer/mesh.hh"

struct DiagnosticData
{
	size_t _failedHorizonEdges; // How many times QuickHull failed to solve the horizon edge. Failures lead to degenerated convex hulls.

	DiagnosticData() : _failedHorizonEdges(0)
	{
	}
};

/// Basically a pointer array glm::vec3* with operator overrides that serve as the source of vertices from the mesh to QuickHull.
class VertexDataSource
{
private:
	const glm::vec3* _ptr;
	size_t _numVertices;
public:
	VertexDataSource(const glm::vec3* ptr, size_t count)
		: _ptr(ptr), _numVertices(count)
	{
	}

	VertexDataSource(const std::vector<glm::vec3>& vec)
		: _ptr(&vec[0]), _numVertices(vec.size())
	{
	}

	VertexDataSource()
		: _ptr(nullptr), _numVertices(0)
	{
	}

	VertexDataSource& operator=(const VertexDataSource& other) = default;

	size_t size() const
	{
		return _numVertices;
	}

	const glm::vec3& operator[](size_t index) const
	{
		return _ptr[index];
	}

	const glm::vec3* begin() const
	{
		return _ptr;
	}

	const glm::vec3* end() const
	{
		return _ptr + _numVertices;
	}
};

class MeshBuilder
{
public:
	struct HalfEdge
	{
		size_t _endVertex;
		size_t _opp;
		size_t _face;
		size_t _next;

		void Disable()
		{
			_endVertex = std::numeric_limits<size_t>::max();
		}

		bool IsDisabled() const
		{
			return _endVertex == std::numeric_limits<size_t>::max();
		}
	};

	struct Face {
		size_t _he;
		qt::Plane _P;
		float _mostDistantPointDist;
		size_t _mostDistantPoint;
		size_t _visibilityCheckedOnIteration;
		std::uint8_t _isVisibleFaceOnCurrentIteration : 1;
		std::uint8_t _inFaceStack : 1;
		std::uint8_t _horizonEdgesOnCurrentIteration : 3; // Bit for each half edge assigned to this face, each being 0 or 1 depending on whether the edge belongs to horizon edge
		std::unique_ptr<std::vector<size_t>> _pointsOnPositiveSide;

		Face()
			:
			_he(std::numeric_limits<size_t>::max()),
			_mostDistantPointDist(0),
			_mostDistantPoint(0),
			_visibilityCheckedOnIteration(0),
			_isVisibleFaceOnCurrentIteration(0),
			_inFaceStack(0),
			_horizonEdgesOnCurrentIteration(0)
		{

		}

		void Disable()
		{
			_he = std::numeric_limits<size_t>::max();
		}

		bool IsDisabled() const
		{
			return _he == std::numeric_limits<size_t>::max();
		}
	};

	// Mesh data
	std::vector<Face> _faces;
	std::vector<HalfEdge> _halfEdges;

	// When the mesh is modified and faces and half edges are removed from it, we do not actually remove them from the container vectors.
	// Insted, they are marked as disabled which means that the indices can be reused when we need to add new faces and half edges to the mesh.
	// We store the free indices in the following vectors.
	std::vector<size_t> _disabledFaces, _disabledHalfEdges;

	size_t AddFace()
	{
		if (_disabledFaces.size())
		{
			size_t index = _disabledFaces.back();
			auto& f = _faces[index];
			EXPECT(f.IsDisabled());
			EXPECT(!f._pointsOnPositiveSide);
			f._mostDistantPointDist = 0;
			_disabledFaces.pop_back();
			return index;
		}

		_faces.emplace_back();

		return _faces.size() - 1;
	}

	size_t AddHalfEdge()
	{
		if (_disabledHalfEdges.size())
		{
			const size_t index = _disabledHalfEdges.back();
			_disabledHalfEdges.pop_back();
			return index;
		}

		_halfEdges.emplace_back();
		return _halfEdges.size() - 1;
	}

	// Mark a face as disabled and return a pointer to the points that were on the positive of it.
	std::unique_ptr<std::vector<size_t>> DisableFace(size_t faceIndex)
	{
		auto& f = _faces[faceIndex];
		f.Disable();
		_disabledFaces.push_back(faceIndex);
		return std::move(f._pointsOnPositiveSide);
	}

	void DisableHalfEdge(size_t heIndex)
	{
		auto& he = _halfEdges[heIndex];
		he.Disable();
		_disabledHalfEdges.push_back(heIndex);
	}

	MeshBuilder() = default;

	// Create a mesh with initial tetrahedron ABCD. Dot product of AB with the normal of triangle ABC should be negative.
	void Setup(size_t a, size_t b, size_t c, size_t d)
	{
		_faces.clear();
		_halfEdges.clear();
		_disabledFaces.clear();
		_disabledHalfEdges.clear();

		_faces.reserve(4);
		_halfEdges.reserve(12);

		// Create halfedges
		HalfEdge AB;
		AB._endVertex = b;
		AB._opp = 6;
		AB._face = 0;
		AB._next = 1;
		_halfEdges.push_back(AB);

		HalfEdge BC;
		BC._endVertex = c;
		BC._opp = 9;
		BC._face = 0;
		BC._next = 2;
		_halfEdges.push_back(BC);

		HalfEdge CA;
		CA._endVertex = a;
		CA._opp = 3;
		CA._face = 0;
		CA._next = 0;
		_halfEdges.push_back(CA);

		HalfEdge AC;
		AC._endVertex = c;
		AC._opp = 2;
		AC._face = 1;
		AC._next = 4;
		_halfEdges.push_back(AC);

		HalfEdge CD;
		CD._endVertex = d;
		CD._opp = 11;
		CD._face = 1;
		CD._next = 5;
		_halfEdges.push_back(CD);

		HalfEdge DA;
		DA._endVertex = a;
		DA._opp = 7;
		DA._face = 1;
		DA._next = 3;
		_halfEdges.push_back(DA);

		HalfEdge BA;
		BA._endVertex = a;
		BA._opp = 0;
		BA._face = 2;
		BA._next = 7;
		_halfEdges.push_back(BA);

		HalfEdge AD;
		AD._endVertex = d;
		AD._opp = 5;
		AD._face = 2;
		AD._next = 8;
		_halfEdges.push_back(AD);

		HalfEdge DB;
		DB._endVertex = b;
		DB._opp = 10;
		DB._face = 2;
		DB._next = 6;
		_halfEdges.push_back(DB);

		HalfEdge CB;
		CB._endVertex = b;
		CB._opp = 1;
		CB._face = 3;
		CB._next = 10;
		_halfEdges.push_back(CB);

		HalfEdge BD;
		BD._endVertex = d;
		BD._opp = 8;
		BD._face = 3;
		BD._next = 11;
		_halfEdges.push_back(BD);

		HalfEdge DC;
		DC._endVertex = c;
		DC._opp = 4;
		DC._face = 3;
		DC._next = 9;
		_halfEdges.push_back(DC);

		// Create faces
		Face ABC;
		ABC._he = 0;
		_faces.push_back(std::move(ABC));

		Face ACD;
		ACD._he = 3;
		_faces.push_back(std::move(ACD));

		Face BAD;
		BAD._he = 6;
		_faces.push_back(std::move(BAD));

		Face CBD;
		CBD._he = 9;
		_faces.push_back(std::move(CBD));
	}

	std::array<size_t, 3> GetVertexIndicesOfFace(const Face& f) const
	{
		std::array<size_t, 3> v;
		const HalfEdge* he = &_halfEdges[f._he];

		v[0] = he->_endVertex;
		he = &_halfEdges[he->_next];
		v[1] = he->_endVertex;
		he = &_halfEdges[he->_next];
		v[2] = he->_endVertex;

		return v;
	}

	std::array<size_t, 2> GetVertexIndicesOfHalfEdge(const HalfEdge& he) const
	{
		return { _halfEdges[he._opp]._endVertex,he._endVertex };
	}

	std::array<size_t, 3> GetHalfEdgeIndicesOfFace(const Face& f) const
	{
		return { f._he,_halfEdges[f._he]._next,_halfEdges[_halfEdges[f._he]._next]._next };
	}
};

template<typename IndexType>
class HalfEdgeMesh {
public:
	struct HalfEdge
	{
		IndexType _endVertex;
		IndexType _opp;
		IndexType _face;
		IndexType _next;
	};

	struct Face
	{
		IndexType _halfEdgeIndex; // Index of one of the half edges of this face
	};

	std::vector<glm::vec3> _vertices;
	std::vector<Face> _faces;
	std::vector<HalfEdge> _halfEdges;

	HalfEdgeMesh(const MeshBuilder& builderObject, const VertexDataSource& vertexData)
	{
		std::unordered_map<IndexType, IndexType> faceMapping;
		std::unordered_map<IndexType, IndexType> halfEdgeMapping;
		std::unordered_map<IndexType, IndexType> vertexMapping;
		
		size_t i = 0;
		for (const auto& face : builderObject._faces)
		{
			if (!face.IsDisabled())
			{
				_faces.push_back(
					{
						static_cast<IndexType>(face._he)
					}
				);
				faceMapping[i] = _faces.size() - 1;

				const auto heIndices = builderObject.GetHalfEdgeIndicesOfFace(face);
				for (const auto heIndex : heIndices)
				{
					const IndexType vertexIndex = builderObject._halfEdges[heIndex]._endVertex;
					if (vertexMapping.count(vertexIndex) == 0)
					{
						_vertices.push_back(vertexData[vertexIndex]);
						vertexMapping[vertexIndex] = _vertices.size() - 1;
					}
				}
			}
			i++;
		}

		i = 0;
		for (const auto& halfEdge : builderObject._halfEdges)
		{
			if (!halfEdge.IsDisabled())
			{
				_halfEdges.push_back(
					{
						static_cast<IndexType>(halfEdge._endVertex),
						static_cast<IndexType>(halfEdge._opp),
						static_cast<IndexType>(halfEdge._face),
						static_cast<IndexType>(halfEdge._next)
					}
				);
				halfEdgeMapping[i] = _halfEdges.size() - 1;
			}
			i++;
		}

		for (auto& face : _faces)
		{
			EXPECT(halfEdgeMapping.count(face._halfEdgeIndex) == 1);
			face._halfEdgeIndex = halfEdgeMapping[face._halfEdgeIndex];
		}

		for (auto& he : _halfEdges)
		{
			he._face = faceMapping[he._face];
			he._opp = halfEdgeMapping[he._opp];
			he._next = halfEdgeMapping[he._next];
			he._endVertex = vertexMapping[he._endVertex];
		}
	}
};

class ConvexHull
{
	std::unique_ptr<std::vector<glm::vec3>> _optimizedVertexBuffer;
	VertexDataSource _vertices;
	std::vector<size_t> _indices;
public:
	ConvexHull()
	{
	}

	// Copy constructor
	ConvexHull(const ConvexHull& obj)
	{
		_indices = obj._indices;

		if (obj._optimizedVertexBuffer)
		{
			_optimizedVertexBuffer.reset(new std::vector<glm::vec3>(*obj._optimizedVertexBuffer));
			_vertices = VertexDataSource(*_optimizedVertexBuffer);
		}
		else
		{
			_vertices = obj._vertices;
		}
	}

	ConvexHull& operator=(const ConvexHull& obj)
	{
		if (&obj == this)
		{
			return *this;
		}

		_indices = obj._indices;

		if (obj._optimizedVertexBuffer)
		{
			_optimizedVertexBuffer.reset(new std::vector<glm::vec3>(*obj._optimizedVertexBuffer));
			_vertices = VertexDataSource(*_optimizedVertexBuffer);
		}
		else
		{
			_vertices = obj._vertices;
		}

		return *this;
	}

	ConvexHull(ConvexHull&& obj) noexcept
	{
		_indices = std::move(obj._indices);

		if (obj._optimizedVertexBuffer)
		{
			_optimizedVertexBuffer = std::move(obj._optimizedVertexBuffer);
			obj._vertices = VertexDataSource();
			_vertices = VertexDataSource(*_optimizedVertexBuffer);
		}
		else
		{
			_vertices = obj._vertices;
		}
	}

	ConvexHull& operator=(ConvexHull&& obj) noexcept
	{
		if (&obj == this)
		{
			return *this;
		}

		_indices = std::move(obj._indices);

		if (obj._optimizedVertexBuffer)
		{
			_optimizedVertexBuffer = std::move(obj._optimizedVertexBuffer);
			obj._vertices = VertexDataSource();
			_vertices = VertexDataSource(*_optimizedVertexBuffer);
		}
		else
		{
			_vertices = obj._vertices;
		}

		return *this;
	}

	// Construct vertex and index buffers from half edge mesh and pointcloud
	ConvexHull(const MeshBuilder& mesh, const VertexDataSource& pointCloud, bool CCW, bool useOriginalIndices)
	{
		if (!useOriginalIndices) {
			_optimizedVertexBuffer.reset(new std::vector<glm::vec3>());
		}

		std::vector<bool> faceProcessed(mesh._faces.size(), false);
		std::vector<size_t> faceStack;
		std::unordered_map<size_t, size_t> vertexIndexMapping; // Map vertex indices from original point cloud to the new mesh vertex indices
		for (size_t i = 0; i < mesh._faces.size(); i++) {
			if (!mesh._faces[i].IsDisabled())
			{
				faceStack.push_back(i);
				break;
			}
		}
		if (faceStack.size() == 0)
		{
			return;
		}

		const size_t iCCW = CCW ? 1 : 0;
		const size_t finalMeshFaceCount = mesh._faces.size() - mesh._disabledFaces.size();
		_indices.reserve(finalMeshFaceCount * 3);

		while (faceStack.size())
		{
			auto it = faceStack.end() - 1;
			size_t top = *it;
			EXPECT(!mesh._faces[top].IsDisabled());
			faceStack.erase(it);

			if (faceProcessed[top])
			{
				continue;
			}
			else
			{
				faceProcessed[top] = true;
				auto halfEdges = mesh.GetHalfEdgeIndicesOfFace(mesh._faces[top]);
				size_t adjacent[] =
				{
					mesh._halfEdges[mesh._halfEdges[halfEdges[0]]._opp]._face,
					mesh._halfEdges[mesh._halfEdges[halfEdges[1]]._opp]._face,
					mesh._halfEdges[mesh._halfEdges[halfEdges[2]]._opp]._face
				};
				for (auto a : adjacent)
				{
					if (!faceProcessed[a] && !mesh._faces[a].IsDisabled())
					{
						faceStack.push_back(a);
					}
				}
				auto vertices = mesh.GetVertexIndicesOfFace(mesh._faces[top]);
				if (!useOriginalIndices)
				{
					for (auto& v : vertices)
					{
						auto itV = vertexIndexMapping.find(v);
						if (itV == vertexIndexMapping.end())
						{
							_optimizedVertexBuffer->push_back(pointCloud[v]);
							vertexIndexMapping[v] = _optimizedVertexBuffer->size() - 1;
							v = _optimizedVertexBuffer->size() - 1;
						}
						else
						{
							v = itV->second;
						}
					}
				}
				_indices.push_back(vertices[0]);
				_indices.push_back(vertices[1 + iCCW]);
				_indices.push_back(vertices[2 - iCCW]);
			}
		}

		if (!useOriginalIndices)
		{
			_vertices = VertexDataSource(*_optimizedVertexBuffer);
		}
		else {
			_vertices = pointCloud;
		}
	}

	std::vector<size_t>& GetIndexBuffer()
	{
		return _indices;
	}

	const std::vector<size_t>& GetIndexBuffer() const
	{
		return _indices;
	}

	VertexDataSource& GetVertexBuffer()
	{
		return _vertices;
	}

	const VertexDataSource& GetVertexBuffer() const
	{
		return _vertices;
	}

	void WriteWaveformOBJ(const std::string& filename, const std::string& objectName = "quickhull") const
	{
		std::ofstream objFile;
		objFile.open(filename);
		objFile << "o " << objectName << "\n";

		for (const auto& v : GetVertexBuffer())
		{
			objFile << "v " << v.x << " " << v.y << " " << v.z << "\n";
		}

		const auto& indBuf = GetIndexBuffer();
		size_t triangleCount = indBuf.size() / 3;

		for (size_t i = 0; i < triangleCount; i++)
		{
			objFile << "f " << indBuf[i * 3] + 1 << " " << indBuf[i * 3 + 1] + 1 << " " << indBuf[i * 3 + 2] + 1 << "\n";
		}

		objFile.close();
	}
};

class QuickHull
{
private:
	DiagnosticData _diagnostics;

	float _epsilon;
	float _epsilonSquared;
	float _scale;
	bool _planar;

	std::vector<glm::vec3> _planarPointCloudTemp;
	VertexDataSource _vertexData;
	MeshBuilder _mesh;
	std::array<size_t, 6> _extremeValues;

	// Temporary variables used during iteration process
	std::vector<size_t> _newFaceIndices;
	std::vector<size_t> _newHalfEdgeIndices;
	std::vector<std::unique_ptr<std::vector<size_t>>> _disabledFacePointVectors;
	std::vector<size_t> _visibleFaces;
	std::vector<size_t> _horizonEdges;

	struct FaceData {
		size_t _faceIndex;
		size_t _enteredFromHalfEdge; // If the face turns out not to be visible, this half edge will be marked as horizon edge

		FaceData(size_t fi, size_t he)
			: _faceIndex(fi), _enteredFromHalfEdge(he)
		{
		}
	};

	std::vector<FaceData> _possiblyVisibleFaces;
	std::deque<size_t> _faceList;

	// Begin funcs

	/// Create a half edge mesh representing the base tetrahedron from which the QuickHull iteration proceeds. m_extremeValues must be properly set up when this is called.
	void _SetupInitialTetrahedron();

	/// Given a list of half edges, try to rearrange them so that they form a loop. Return true on success.
	bool _ReorderHorizonEdges(std::vector<size_t>& horizonEdges);

	/// Find indices of extreme values (max x, min x, max y, min y, max z, min z) for the given point cloud
	std::array<size_t, 6> _GetExtremeValues();

	/// Computes scale of the vertex data.
	float _GetScale(const std::array<size_t, 6>& extremeValues);

	// Each face contains a unique pointer to a vector of indices. However, many - often most - faces do not have any points on the positive
	// side of them especially at the the end of the iteration. When a face is removed from the mesh, its associated point vector, if such
	// exists, is moved to the index vector pool, and when we need to add new faces with points on the positive side to the mesh,
	// we reuse these vectors. This reduces the amount of std::vectors we have to deal with, and impact on performance is remarkable.
	qt::Pool<std::vector<size_t>> _indexVectorPool;
	inline std::unique_ptr<std::vector<size_t>> _GetIndexVectorFromPool();
	inline void _ReclaimToIndexVectorPool(std::unique_ptr<std::vector<size_t>>& ptr);

	// Associates a point with a face if the point resides on the positive side of the plane. Returns true if the points was on the positive side.
	inline bool _AddPointToFace(typename MeshBuilder::Face& f, size_t pointIndex);

	// This will update m_mesh from which we create the ConvexHull object that getConvexHull function returns
	void _CreateConvexHalfEdgeMesh();

	// Constructs the convex hull into a MeshBuilder object which can be converted to a ConvexHull or Mesh object
	void _BuildMesh(const VertexDataSource& pointCloud, bool CCW, bool useOriginalIndices, float epsilon);

	// The public GetConvexHull functions will setup a VertexDataSource object and call this
	ConvexHull _GetConvexHull(const VertexDataSource& pointCloud, bool CCW, bool useOriginalIndices, float epsilon);
public:
	/// Computes convex hull for a given point cloud.
	/// This function assumes that the vertex data resides in memory in the following format: x0,y0,z0, x1,y1,z1, ...
	ConvexHull GetConvexHull(
		const std::vector<glm::vec3>& pointCloud,
		bool CCW,
		bool useOriginalIndices = false,
		float epsilon = std::numeric_limits<float>::epsilon()
	);

	/// Computes convex hull for a given point cloud and returns it as a mesh object with a half edge structure.
	/// This function assumes that the vertex data resides in memory in the following format: x0,y0,z0, x1,y1,z1, ...
	HalfEdgeMesh<size_t> GetConvexHullAsMesh(
		const glm::vec3* vertexData,
		size_t vertexCount,
		bool CCW,
		float epsilon = std::numeric_limits<float>::epsilon()
	);

	// Get diagnostics about last generated convex hull
	const DiagnosticData& GetDiagnostics()
	{
		return _diagnostics;
	}
};

inline std::unique_ptr<std::vector<size_t>> QuickHull::_GetIndexVectorFromPool()
{
	auto r = std::move(_indexVectorPool.Get());
	r->clear();
	return r;
}

inline void QuickHull::_ReclaimToIndexVectorPool(std::unique_ptr<std::vector<size_t>>& ptr)
{
	const size_t oldSize = ptr->size();
	if ((oldSize + 1) * 128 < ptr->capacity())
	{
		// Reduce memory usage! Huge vectors are needed at the beginning of iteration when faces have many points on their positive side. Later on, smaller vectors will suffice.
		ptr.reset(nullptr);
		return;
	}
	_indexVectorPool.Reclaim(ptr);
}

inline bool QuickHull::_AddPointToFace(typename MeshBuilder::Face& f, size_t pointIndex)
{
	const float D = qt::DistanceToPlaneSigned(_vertexData[pointIndex], f._P);

	if (D > 0 && D * D > _epsilonSquared * f._P._NLength2)
	{
		if (!f._pointsOnPositiveSide)
		{
			f._pointsOnPositiveSide = std::move(_GetIndexVectorFromPool());
		}

		f._pointsOnPositiveSide->push_back(pointIndex);

		if (D > f._mostDistantPointDist)
		{
			f._mostDistantPointDist = D;
			f._mostDistantPoint = pointIndex;
		}

		return true;
	}
	return false;
}
