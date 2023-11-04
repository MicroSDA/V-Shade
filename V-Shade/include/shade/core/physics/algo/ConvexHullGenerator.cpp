#include "shade_pch.h"
#include "ConvexHullGenerator.h"
#include <glm/glm/gtx/closest_point.hpp>
#include <glm/glm/gtx/intersect.hpp>
#include <glm/glm/gtx/normal.hpp>

namespace shade
{
	namespace physic
	{
		namespace algo
		{
			using vec3 = glm::vec<3, scalar_t>;

			template<typename T>
			class Pool {
				std::vector<std::unique_ptr<T>> m_Data;
			public:
				void Clear() {
					m_Data.clear();
				}

				void Reclaim(std::unique_ptr<T>& ptr) {
					m_Data.push_back(std::move(ptr));
				}

				std::unique_ptr<T> Get() {
					if (m_Data.size() == 0) {
						return std::unique_ptr<T>(new T());
					}
					auto it = m_Data.end() - 1;
					std::unique_ptr<T> r = std::move(*it);
					m_Data.erase(it);
					return r;
				}

			};
			struct Ray
			{
				const vec3 S;
				const vec3 V;
				const scalar_t VInvLengthSquared;

				Ray(const vec3& s, const vec3& v) : S(s), V(v), VInvLengthSquared(1 / glm::length2(v)) {}
			};

			struct Plane
			{
				vec3 Normal = glm::vec3(0.0);
				// Signed distance (if normal is of length 1) to the plane from origin
				scalar_t Distance = 0.0;
				// Normal length squared
				scalar_t SqrNLength = 0.0;

				Plane() = default;
				// Construct a plane using normal N and any point P on the plane
				Plane(const vec3& N, const vec3& P) : Normal(N), Distance(-glm::dot(N, P)), SqrNLength(Normal.x* Normal.x + Normal.y * Normal.y + Normal.z * Normal.z) {}

				bool IsPointOnPositiveSide(const vec3& Q) const
				{
					scalar_t d = glm::dot(Normal, Q) + Distance;
					if (d >= 0) return true;
					return false;
				}
			};

			struct HalfEdge
			{
				std::size_t EndVertex;
				std::size_t Opp;
				std::size_t Face;
				std::size_t Next;

				HalfEdge() = default;
				HalfEdge(std::size_t end, std::size_t opp, std::size_t face, std::size_t next) : EndVertex(end), Opp(opp), Face(face), Next(next) {}

				void Disable() { EndVertex = std::numeric_limits<size_t>::max(); }
				bool IsDisabled() const { return EndVertex == std::numeric_limits<size_t>::max(); }
			};

			struct Face
			{
				std::size_t He;
				Plane Plane;
				scalar_t MostDistantPointDist;
				std::size_t MostDistantPoint, VisibilityCheckedOnIteration;
				std::uint8_t IsVisibleFaceOnCurrentIteration : 1, InFaceStack : 1, HorizonEdgesOnCurrentIteration : 3;
				std::unique_ptr<std::vector<std::size_t>> PointsOnPositiveSide;

				Face() : He(std::numeric_limits<size_t>::max()), MostDistantPointDist(0), MostDistantPoint(0), VisibilityCheckedOnIteration(0), IsVisibleFaceOnCurrentIteration(0), InFaceStack(0), HorizonEdgesOnCurrentIteration(0) {}
				Face(std::size_t he) : He(he), MostDistantPointDist(0), MostDistantPoint(0), VisibilityCheckedOnIteration(0), IsVisibleFaceOnCurrentIteration(0), InFaceStack(0), HorizonEdgesOnCurrentIteration(0) {}

				void Disable() { He = std::numeric_limits<size_t>::max(); };
				bool IsDisabled() const { return (He == std::numeric_limits<size_t>::max()); };
			};
			struct FaceData
			{
				std::size_t FaceIndex;
				std::size_t EnteredFromHalfEdge; // If the face turns out not to be visible, this half edge will be marked as horizon edge
				FaceData(std::size_t fi, std::size_t he) :FaceIndex(fi), EnteredFromHalfEdge(he) {}
			};
			struct Polyhedron
			{
				std::vector<Face> Faces;
				std::vector<HalfEdge> HalfEdges;
				std::vector<std::size_t> DisabledFaces, DisabledHalfEdges;

				std::size_t AddFace()
				{
					if (DisabledFaces.size()) {
						size_t index = DisabledFaces.back();
						auto& f = Faces[index];
						assert(f.IsDisabled());
						assert(!f.PointsOnPositiveSide);
						f.MostDistantPointDist = 0;
						DisabledFaces.pop_back();
						return index;
					}
					Faces.emplace_back();
					return Faces.size() - 1;
				};
				std::size_t AddHalfEdge()
				{
					if (DisabledHalfEdges.size())
					{
						const size_t index = DisabledHalfEdges.back();
						DisabledHalfEdges.pop_back();
						return index;
					}
					HalfEdges.emplace_back();
					return HalfEdges.size() - 1;

				};
				std::unique_ptr<std::vector<std::size_t>> DisableFace(std::size_t faceIndex)
				{
					auto& f = Faces[faceIndex];
					f.Disable();
					DisabledFaces.push_back(faceIndex);
					return std::move(f.PointsOnPositiveSide);
				};
				void DisableHalfEdge(std::size_t halfEdgeIndex)
				{
					auto& he = HalfEdges[halfEdgeIndex];
					he.Disable();
					DisabledHalfEdges.push_back(halfEdgeIndex);
				};
				// Create a mesh with initial tetrahedron ABCD. Dot product of AB with the normal of triangle ABC should be negative.
				void Setup(std::size_t a, std::size_t b, std::size_t c, std::size_t d)
				{
					Faces.clear();
					HalfEdges.clear();
					DisabledFaces.clear();
					DisabledHalfEdges.clear();

					Faces.reserve(4);
					HalfEdges.reserve(12);

					// Create halfedges
					HalfEdges.emplace_back(b, 6, 0, 1);// AB
					HalfEdges.emplace_back(c, 9, 0, 2);// BC
					HalfEdges.emplace_back(a, 3, 0, 0);// CA
					HalfEdges.emplace_back(c, 2, 1, 4);// AC
					HalfEdges.emplace_back(d, 11, 1, 5);// CD
					HalfEdges.emplace_back(a, 7, 1, 3);// DA
					HalfEdges.emplace_back(a, 0, 2, 7);// BA
					HalfEdges.emplace_back(d, 5, 2, 8);// AD
					HalfEdges.emplace_back(b, 10, 2, 6);// DB
					HalfEdges.emplace_back(b, 1, 3, 10);// CB
					HalfEdges.emplace_back(d, 8, 3, 11);// BD
					HalfEdges.emplace_back(c, 4, 3, 9);// DC

					// Create faces
					Faces.emplace_back(0); // ABC
					Faces.emplace_back(3); // ACD
					Faces.emplace_back(6); // BAD
					Faces.emplace_back(9); // CBD
				};

				std::array<size_t, 3> GetVertexIndicesOfFace(const Face& f) const
				{
					std::array<size_t, 3> v;
					const HalfEdge* he = &HalfEdges[f.He];
					v[0] = he->EndVertex;
					he = &HalfEdges[he->Next];
					v[1] = he->EndVertex;
					he = &HalfEdges[he->Next];
					v[2] = he->EndVertex;
					return v;
				}

				std::array<size_t, 2> GetVertexIndicesOfHalfEdge(const HalfEdge& he) const
				{
					return { HalfEdges[he.Opp].EndVertex,he.EndVertex };
				}

				std::array<size_t, 3> GetHalfEdgeIndicesOfFace(const Face& f) const
				{
					return { f.He,HalfEdges[f.He].Next, HalfEdges[HalfEdges[f.He].Next].Next };
				}

				Polyhedron(std::size_t a, std::size_t b, std::size_t c, std::size_t d) { Setup(a, b, c, d); }
			};

			scalar_t GetDistanceBetweenPointAndRay(const vec3& point, const Ray& ray)
			{
				const vec3 s = point - ray.S;
				scalar_t t = glm::dot(s, ray.V);
				return glm::length2(s) - t * t * ray.VInvLengthSquared;
			}

			vec3 GetTriangleNormal(const vec3& a, const vec3& b, const vec3& c)
			{
				scalar_t x = a.x - c.x;
				scalar_t y = a.y - c.y;
				scalar_t z = a.z - c.z;
				scalar_t rhsx = b.x - c.x;
				scalar_t rhsy = b.y - c.y;
				scalar_t rhsz = b.z - c.z;
				scalar_t px = y * rhsz - z * rhsy;
				scalar_t py = z * rhsx - x * rhsz;
				scalar_t pz = x * rhsy - y * rhsx;
				return vec3(px, py, pz);
			}
			scalar_t GetDistanceToPlane(const vec3& v, const Plane& p)
			{
				return glm::dot(p.Normal, v) + p.Distance;
			}
			std::array<std::size_t, 6> GetExtremeValues(const std::vector<vec3>& vertexData)
			{
				std::array<size_t, 6> outIndices{ 0, 0, 0, 0, 0, 0 };
				std::array<scalar_t, 6> extremeValues
				{   vertexData[0].x,
					vertexData[0].x,
					vertexData[0].y,
					vertexData[0].y,
					vertexData[0].z,
					vertexData[0].z
				};

				for (std::size_t i = 1; i < vertexData.size(); i++)
				{
					const vec3 position = vertexData[i];

					for (int j = 0; j < 3; j++)
					{
						if (position[j] > extremeValues[j * 2])
						{
							extremeValues[j * 2] = position[j];
							outIndices[j * 2] = i;
						}
						else if (position[j] < extremeValues[(j * 2) + 1])
						{
							extremeValues[(j * 2) + 1] = position[j];
							outIndices[(j * 2) + 1] = i;
						}
					}
				}
				return outIndices;
			}

			scalar_t GetScale(const std::array<size_t, 6>& extremeValues, const std::vector<vec3>& vertexData)
			{
				scalar_t s = 0;
				for (std::size_t i = 0; i < 6; i++)
				{
					const glm::vec3& position = vertexData[extremeValues[i]];
					const scalar_t a = std::abs(position[i / 2]);
					if (a > s) s = a;
				}

				return s;
			}

			class QuickHull
			{
			public:
				std::pair<std::vector<vec3>, Indices> CreateConvexHull(const Vertices& pointsCloud, scalar_t epsilon);
			private:
				Polyhedron CreateConvexHalfEdgeMesh();
				Polyhedron SetupInitialTetrahedron();
				bool AddPointToface(Face& face, std::size_t pointIndex);

				std::unique_ptr<std::vector<size_t>> GetIndexVectorFromPool();
				// Given a list of half edges, try to rearrange them so that they form a loop. Return true on success.
				bool ReorderHorizonEdges(std::vector<size_t>& horizonEdges, Polyhedron& tetrahedron);
				void ReclaimToIndexVectorPool(std::unique_ptr<std::vector<size_t>>& ptr);
			private:
				std::vector<vec3> m_VertexData, m_PlanarPointCloudTemp;
				std::array<size_t, 6> m_ExtremeValues;
				scalar_t m_Epsilon, m_EpsilonSquared, m_Scale;
				std::vector<size_t> m_VisibleFaces, m_HorizonEdges, m_NewFaceIndices, m_NewHalfEdgeIndices;
				std::vector<FaceData> m_PossiblyVisibleFaces;
				std::deque<size_t> m_FaceList;
				Pool<std::vector<size_t>> m_IndexPool;
				bool m_IsPlanar = false;
				size_t m_FailedHorizonEdges = 0;
				std::vector< std::unique_ptr<std::vector<size_t>> > m_DisabledFacePointVectors;
			};

			std::pair<std::vector<glm::vec<3, scalar_t>>, Indices> GenerateConvexHull(const Vertices& pointsCloud, scalar_t epsilon)
			{
				QuickHull qh;

				return qh.CreateConvexHull(pointsCloud, epsilon);
			}

			std::pair<std::vector<vec3>, Indices> QuickHull::CreateConvexHull(const Vertices& pointsCloud, scalar_t epsilon)
			{
				m_VertexData = Vertex::ConvertToArray<scalar_t>(pointsCloud);
				m_ExtremeValues = GetExtremeValues(m_VertexData);

				m_Scale = GetScale(m_ExtremeValues, m_VertexData);
				m_Epsilon = epsilon * m_Scale;
				m_EpsilonSquared = m_Epsilon * m_Epsilon;

				m_IsPlanar = false;

				Polyhedron convexHull = CreateConvexHalfEdgeMesh();

				if (m_IsPlanar) {
					const size_t extraPointIndex = m_PlanarPointCloudTemp.size() - 1;
					for (auto& he : convexHull.HalfEdges) {
						if (he.EndVertex == extraPointIndex)
						{
							he.EndVertex = 0;
						}
					}
					m_VertexData = Vertex::ConvertToArray<scalar_t>(pointsCloud);
					m_PlanarPointCloudTemp.clear();
				}

				std::vector<vec3> outVertices;
				Indices outIndices;

				std::vector<bool> faceProcessed(convexHull.Faces.size(), false);
				std::vector<size_t> faceStack;
				std::unordered_map<size_t, size_t> vertexIndexMapping; // Map vertex indices from original point cloud to the new mesh vertex indices
				for (size_t i = 0; i < convexHull.Faces.size(); i++) {
					if (!convexHull.Faces[i].IsDisabled()) {
						faceStack.push_back(i);
						break;
					}
				}
				if (faceStack.size() == 0) {
					return { outVertices , outIndices };
				}

				const size_t iCCW = false;
				const size_t finalMeshFaceCount = convexHull.Faces.size() - convexHull.DisabledFaces.size();
				outIndices.reserve(finalMeshFaceCount * 3);

				while (faceStack.size()) {
					auto it = faceStack.end() - 1;
					size_t top = *it;
					assert(!convexHull.Faces[top].IsDisabled());
					faceStack.erase(it);
					if (faceProcessed[top]) {
						continue;
					}
					else {
						faceProcessed[top] = true;
						auto halfEdges = convexHull.GetHalfEdgeIndicesOfFace(convexHull.Faces[top]);
						size_t adjacent[] = { convexHull.HalfEdges[convexHull.HalfEdges[halfEdges[0]].Opp].Face, convexHull.HalfEdges[convexHull.HalfEdges[halfEdges[1]].Opp].Face,convexHull.HalfEdges[convexHull.HalfEdges[halfEdges[2]].Opp].Face };
						for (auto a : adjacent) {
							if (!faceProcessed[a] && !convexHull.Faces[a].IsDisabled()) {
								faceStack.push_back(a);
							}
						}
						auto vertices = convexHull.GetVertexIndicesOfFace(convexHull.Faces[top]);

						for (auto& v : vertices) {
							auto itV = vertexIndexMapping.find(v);
							if (itV == vertexIndexMapping.end()) {
								outVertices.push_back({ pointsCloud[v].Position });
								vertexIndexMapping[v] = outVertices.size() - 1;
								v = outVertices.size() - 1;
							}
							else {
								v = itV->second;
							}
						}

						outIndices.push_back(vertices[0]);
						outIndices.push_back(vertices[1 + iCCW]);
						outIndices.push_back(vertices[2 - iCCW]);
					}
				}

				return { outVertices, outIndices };
			}

			Polyhedron QuickHull::CreateConvexHalfEdgeMesh()
			{
				m_VisibleFaces.clear();
				m_HorizonEdges.clear();
				m_PossiblyVisibleFaces.clear();

				Polyhedron tetrahedron = SetupInitialTetrahedron();

				assert(tetrahedron.Faces.size() == 4);
				// Init face stack with those faces that have points assigned to them
				m_FaceList.clear();
				for (size_t i = 0; i < 4; i++)
				{
					auto& f = tetrahedron.Faces[i];
					if (f.PointsOnPositiveSide && f.PointsOnPositiveSide->size() > 0) {
						m_FaceList.push_back(i);
						f.InFaceStack = 1;
					}
				}

				// Process faces until the face list is empty.
				size_t iter = 0;
				while (!m_FaceList.empty())
				{
					iter++;
					if (iter == std::numeric_limits<size_t>::max()) {
						// Visible face traversal marks visited faces with iteration counter (to mark that the face has been visited on this iteration) and the max value represents unvisited faces. At this point we have to reset iteration counter. This shouldn't be an
						// issue on 64 bit machines.
						iter = 0;
					}

					const size_t topFaceIndex = m_FaceList.front();
					m_FaceList.pop_front();

					auto& tf = tetrahedron.Faces[topFaceIndex];
					tf.InFaceStack = 0;

					assert(!tf.PointsOnPositiveSide || tf.PointsOnPositiveSide->size() > 0);
					if (!tf.PointsOnPositiveSide || tf.IsDisabled()) {
						continue;
					}

					// Pick the most distant point to this triangle plane as the point to which we extrude
					const vec3& activePoint = m_VertexData[tf.MostDistantPoint];
					const size_t activePointIndex = tf.MostDistantPoint;

					// Find out the faces that have our active point on their positive side (these are the "visible faces"). The face on top of the stack of course is one of them. At the same time, we create a list of horizon edges.
					m_HorizonEdges.clear();
					m_VisibleFaces.clear();
					m_PossiblyVisibleFaces.clear();
					m_PossiblyVisibleFaces.emplace_back(topFaceIndex, std::numeric_limits<size_t>::max());

					while (m_PossiblyVisibleFaces.size()) {
						const auto faceData = m_PossiblyVisibleFaces.back();
						m_PossiblyVisibleFaces.pop_back();
						auto& pvf = tetrahedron.Faces[faceData.FaceIndex];
						assert(!pvf.IsDisabled());

						if (pvf.VisibilityCheckedOnIteration == iter) {
							if (pvf.IsVisibleFaceOnCurrentIteration) {
								continue;
							}
						}
						else {
							const Plane& P = pvf.Plane;
							pvf.VisibilityCheckedOnIteration = iter;
							const scalar_t d = glm::dot(P.Normal, activePoint) + P.Distance;
							if (d > 0) {
								pvf.IsVisibleFaceOnCurrentIteration = 1;
								pvf.HorizonEdgesOnCurrentIteration = 0;
								m_VisibleFaces.push_back(faceData.FaceIndex);
								for (auto heIndex : tetrahedron.GetHalfEdgeIndicesOfFace(pvf)) {
									if (tetrahedron.HalfEdges[heIndex].Opp != faceData.EnteredFromHalfEdge) {
										m_PossiblyVisibleFaces.emplace_back(tetrahedron.HalfEdges[tetrahedron.HalfEdges[heIndex].Opp].Face, heIndex);
									}
								}
								continue;
							}
							assert(faceData.FaceIndex != topFaceIndex);
						}

						// The face is not visible. Therefore, the halfedge we came from is part of the horizon edge.
						pvf.IsVisibleFaceOnCurrentIteration = 0;
						m_HorizonEdges.push_back(faceData.EnteredFromHalfEdge);
						// Store which half edge is the horizon edge. The other half edges of the face will not be part of the final mesh so their data slots can by recycled.
						const auto halfEdges = tetrahedron.GetHalfEdgeIndicesOfFace(tetrahedron.Faces[tetrahedron.HalfEdges[faceData.EnteredFromHalfEdge].Face]);
						const std::int8_t ind = (halfEdges[0] == faceData.EnteredFromHalfEdge) ? 0 : (halfEdges[1] == faceData.EnteredFromHalfEdge ? 1 : 2);
						tetrahedron.Faces[tetrahedron.HalfEdges[faceData.EnteredFromHalfEdge].Face].HorizonEdgesOnCurrentIteration |= (1 << ind);
					}
					const size_t horizonEdgeCount = m_HorizonEdges.size();
					// Order horizon edges so that they form a loop. This may fail due to numerical instability in which case we give up trying to solve horizon edge for this point and accept a minor degeneration in the convex hull.
					if (!ReorderHorizonEdges(m_HorizonEdges, tetrahedron)) {
						m_FailedHorizonEdges++;
						std::cerr << "Failed to solve horizon edge." << std::endl;
						auto it = std::find(tf.PointsOnPositiveSide->begin(), tf.PointsOnPositiveSide->end(), activePointIndex);
						tf.PointsOnPositiveSide->erase(it);
						if (tf.PointsOnPositiveSide->size() == 0) {
							ReclaimToIndexVectorPool(tf.PointsOnPositiveSide);
						}
						continue;
					}
					// Except for the horizon edges, all half edges of the visible faces can be marked as disabled. Their data slots will be reused.
			// The faces will be disabled as well, but we need to remember the points that were on the positive side of them - therefore
			// we save pointers to them.
					m_NewFaceIndices.clear();
					m_NewHalfEdgeIndices.clear();
					m_DisabledFacePointVectors.clear();
					size_t disableCounter = 0;
					for (auto faceIndex : m_VisibleFaces) {
						auto& disabledFace = tetrahedron.Faces[faceIndex];
						auto halfEdges = tetrahedron.GetHalfEdgeIndicesOfFace(disabledFace);
						for (size_t j = 0; j < 3; j++) {
							if ((disabledFace.HorizonEdgesOnCurrentIteration & (1 << j)) == 0) {
								if (disableCounter < horizonEdgeCount * 2) {
									// Use on this iteration
									m_NewHalfEdgeIndices.push_back(halfEdges[j]);
									disableCounter++;
								}
								else {
									// Mark for reusal on later iteration step
									tetrahedron.DisableHalfEdge(halfEdges[j]);
								}
							}
						}
						// Disable the face, but retain pointer to the points that were on the positive side of it. We need to assign those points
						// to the new faces we create shortly.
						auto t = tetrahedron.DisableFace(faceIndex);
						if (t) {
							assert(t->size()); // Because we should not assign point vectors to faces unless needed...
							m_DisabledFacePointVectors.push_back(std::move(t));
						}
					}
					if (disableCounter < horizonEdgeCount * 2) {
						const size_t newHalfEdgesNeeded = horizonEdgeCount * 2 - disableCounter;
						for (size_t i = 0; i < newHalfEdgesNeeded; i++) {
							m_NewHalfEdgeIndices.push_back(tetrahedron.AddHalfEdge());
						}
					}

					// Create new faces using the edgeloop
					for (size_t i = 0; i < horizonEdgeCount; i++) {
						const size_t AB = m_HorizonEdges[i];

						auto horizonEdgeVertexIndices = tetrahedron.GetVertexIndicesOfHalfEdge(tetrahedron.HalfEdges[AB]);
						size_t A, B, C;
						A = horizonEdgeVertexIndices[0];
						B = horizonEdgeVertexIndices[1];
						C = activePointIndex;

						const size_t newFaceIndex = tetrahedron.AddFace();
						m_NewFaceIndices.push_back(newFaceIndex);

						const size_t CA = m_NewHalfEdgeIndices[2 * i + 0];
						const size_t BC = m_NewHalfEdgeIndices[2 * i + 1];

						tetrahedron.HalfEdges[AB].Next = BC;
						tetrahedron.HalfEdges[BC].Next = CA;
						tetrahedron.HalfEdges[CA].Next = AB;

						tetrahedron.HalfEdges[BC].Face = newFaceIndex;
						tetrahedron.HalfEdges[CA].Face = newFaceIndex;
						tetrahedron.HalfEdges[AB].Face = newFaceIndex;

						tetrahedron.HalfEdges[CA].EndVertex = A;
						tetrahedron.HalfEdges[BC].EndVertex = C;

						auto& newFace = tetrahedron.Faces[newFaceIndex];

						const vec3 planeNormal = GetTriangleNormal(m_VertexData[A], m_VertexData[B], activePoint);
						newFace.Plane = Plane(planeNormal, activePoint);
						newFace.He = AB;

						tetrahedron.HalfEdges[CA].Opp = m_NewHalfEdgeIndices[i > 0 ? i * 2 - 1 : 2 * horizonEdgeCount - 1];
						tetrahedron.HalfEdges[BC].Opp = m_NewHalfEdgeIndices[((i + 1) * 2) % (horizonEdgeCount * 2)];
					}

					// Assign points that were on the positive side of the disabled faces to the new faces.
					for (auto& disabledPoints : m_DisabledFacePointVectors) {
						assert(disabledPoints);
						for (const auto& point : *(disabledPoints)) {
							if (point == activePointIndex) {
								continue;
							}
							for (size_t j = 0; j < horizonEdgeCount; j++) {
								if (AddPointToface(tetrahedron.Faces[m_NewFaceIndices[j]], point)) {
									break;
								}
							}
						}
						// The points are no longer needed: we can move them to the vector pool for reuse.
						ReclaimToIndexVectorPool(disabledPoints);
					}

					// Increase face stack size if needed
					for (const auto newFaceIndex : m_NewFaceIndices) {
						auto& newFace = tetrahedron.Faces[newFaceIndex];
						if (newFace.PointsOnPositiveSide) {
							assert(newFace.PointsOnPositiveSide->size() > 0);
							if (!newFace.InFaceStack) {
								m_FaceList.push_back(newFaceIndex);
								newFace.InFaceStack = 1;
							}
						}
					}
				}

				// Cleanup
				m_IndexPool.Clear();

				return tetrahedron;
			}

			Polyhedron QuickHull::SetupInitialTetrahedron()
			{
				// Find two most distant extreme points.
				scalar_t maxD = m_EpsilonSquared;
				std::pair<size_t, size_t> selectedPoints;

				const std::size_t vertexCount = m_VertexData.size();

				for (std::size_t i = 0; i < 6; i++)
				{
					for (std::size_t j = i + 1; j < 6; j++)
					{
						const scalar_t d = glm::distance2(m_VertexData[m_ExtremeValues[i]], m_VertexData[m_ExtremeValues[j]]);
						if (d > maxD)
						{
							maxD = d;
							selectedPoints = { m_ExtremeValues[i], m_ExtremeValues[j] };
						}
					}
				}

				if (maxD == m_EpsilonSquared)
				{
					// A degenerate case: the point cloud seems to consists of a single point
					return Polyhedron(0, std::min(std::size_t(1), vertexCount - 1), std::min(std::size_t(2), vertexCount - 1), std::min(std::size_t(3), vertexCount - 1));
				}
				assert(selectedPoints.first != selectedPoints.second);
				// Find the most distant point to the line between the two chosen extreme points.
				const Ray ray(m_VertexData[selectedPoints.first], (m_VertexData[selectedPoints.second] - m_VertexData[selectedPoints.first]));

				maxD = m_EpsilonSquared;
				std::size_t maxI = std::numeric_limits<size_t>::max();
				const std::size_t vCount = m_VertexData.size();
				for (std::size_t i = 0; i < vCount; ++i)
				{
					const scalar_t distToRay = GetDistanceBetweenPointAndRay(m_VertexData[i], ray);
					if (distToRay > maxD)
					{
						maxD = distToRay;
						maxI = i;
					}
				}
				if (maxD == m_EpsilonSquared)
				{
					// It appears that the point cloud belongs to a 1 dimensional subspace of R^3: convex hull has no volume => return a thin triangle
					// Pick any point other than selectedPoints.first and selectedPoints.second as the third point of the triangle
					auto it = std::find_if(m_VertexData.begin(), m_VertexData.end(), [&](const vec3& ve) {
						return ve != m_VertexData[selectedPoints.first] && ve != m_VertexData[selectedPoints.second];
						});
					const std::size_t thirdPoint = (it == m_VertexData.end()) ? selectedPoints.first : std::distance(m_VertexData.begin(), it);
					it = std::find_if(m_VertexData.begin(), m_VertexData.end(), [&](const vec3& ve) {
						return ve != m_VertexData[selectedPoints.first] && ve != m_VertexData[selectedPoints.second] && ve != m_VertexData[thirdPoint];
						});
					const size_t fourthPoint = (it == m_VertexData.end()) ? selectedPoints.first : std::distance(m_VertexData.begin(), it);
					return Polyhedron(selectedPoints.first, selectedPoints.second, thirdPoint, fourthPoint);
				}
				// These three points form the base triangle for our tetrahedron.
				assert(selectedPoints.first != maxI && selectedPoints.second != maxI);
				std::array<size_t, 3> baseTriangle{ selectedPoints.first, selectedPoints.second, maxI };
				const vec3 baseTriangleVertices[] = { m_VertexData[baseTriangle[0]], m_VertexData[baseTriangle[1]],  m_VertexData[baseTriangle[2]] };

				// Next step is to find the 4th vertex of the tetrahedron. We naturally choose the point farthest away from the triangle plane.
				maxD = m_Epsilon;
				maxI = 0;
				const vec3 N = GetTriangleNormal(baseTriangleVertices[0], baseTriangleVertices[1], baseTriangleVertices[2]);
				Plane trianglePlane(N, baseTriangleVertices[0]);
				for (size_t i = 0; i < vCount; i++) {
					const scalar_t d = glm::abs(GetDistanceToPlane(m_VertexData[i], trianglePlane));
					if (d > maxD) {
						maxD = d;
						maxI = i;
					}
				}

				if (maxD == m_Epsilon)
				{
					// All the points seem to lie on a 2D subspace of R^3. How to handle this? Well, let's add one extra point to the point cloud so that the convex hull will have volume.
					m_IsPlanar = true;
					const vec3 N1 = GetTriangleNormal(baseTriangleVertices[1], baseTriangleVertices[2], baseTriangleVertices[0]);
					m_PlanarPointCloudTemp.clear();
					m_PlanarPointCloudTemp.insert(m_PlanarPointCloudTemp.begin(), m_VertexData.begin(), m_VertexData.end());
					const vec3 extraPoint = N1 + m_VertexData[0];
					m_PlanarPointCloudTemp.push_back(extraPoint);
					maxI = m_PlanarPointCloudTemp.size() - 1;
					m_VertexData = m_PlanarPointCloudTemp;
				}

				// Enforce CCW orientation (if user prefers clockwise orientation, swap two vertices in each triangle when final mesh is created)
				const Plane triPlane(N, baseTriangleVertices[0]);
				if (triPlane.IsPointOnPositiveSide(m_VertexData[maxI]))
					std::swap(baseTriangle[0], baseTriangle[1]);

				// Create a tetrahedron half edge mesh and compute planes defined by each triangle
				Polyhedron tetrahedron(baseTriangle[0], baseTriangle[1], baseTriangle[2], maxI);

				for (auto& f : tetrahedron.Faces) {
					auto v = tetrahedron.GetVertexIndicesOfFace(f);
					const vec3 va = m_VertexData[v[0]], vb = m_VertexData[v[1]], vc = m_VertexData[v[2]], N1 = GetTriangleNormal(va, vb, vc);
					const Plane plane(N1, va);
					f.Plane = plane;
				}

				// Finally we assign a face for each vertex outside the tetrahedron (vertices inside the tetrahedron have no role anymore)
				for (size_t i = 0; i < vCount; i++) {
					for (auto& face : tetrahedron.Faces) {
						if (AddPointToface(face, i)) {
							break;
						}
					}
				}

				return tetrahedron;
			}
			bool QuickHull::AddPointToface(Face& face, std::size_t pointIndex)
			{
				const scalar_t D = GetDistanceToPlane(m_VertexData[pointIndex], face.Plane);
				if (D > 0 && D * D > m_EpsilonSquared * face.Plane.SqrNLength) {
					if (!face.PointsOnPositiveSide) {
						face.PointsOnPositiveSide = std::move(GetIndexVectorFromPool());
					}
					face.PointsOnPositiveSide->push_back(pointIndex);
					if (D > face.MostDistantPointDist) {
						face.MostDistantPointDist = D;
						face.MostDistantPoint = pointIndex;
					}
					return true;
				}
				return false;
			}
			std::unique_ptr<std::vector<size_t>> shade::physic::algo::QuickHull::GetIndexVectorFromPool()
			{
				auto r = m_IndexPool.Get();
				r->clear();
				return r;
			}
			bool physic::algo::QuickHull::ReorderHorizonEdges(std::vector<size_t>& horizonEdges, Polyhedron& tetrahedron)
			{
				const size_t horizonEdgeCount = horizonEdges.size();
				for (size_t i = 0; i < horizonEdgeCount - 1; i++) {
					const size_t endVertex = tetrahedron.HalfEdges[horizonEdges[i]].EndVertex;
					bool foundNext = false;
					for (size_t j = i + 1; j < horizonEdgeCount; j++) {
						const size_t beginVertex = tetrahedron.HalfEdges[tetrahedron.HalfEdges[horizonEdges[j]].Opp].EndVertex;
						if (beginVertex == endVertex) {
							std::swap(horizonEdges[i + 1], horizonEdges[j]);
							foundNext = true;
							break;
						}
					}
					if (!foundNext) {
						return false;
					}
				}
				assert(tetrahedron.HalfEdges[horizonEdges[horizonEdges.size() - 1]].EndVertex == tetrahedron.HalfEdges[tetrahedron.HalfEdges[horizonEdges[0]].Opp].EndVertex);
				return true;
			}
			void QuickHull::ReclaimToIndexVectorPool(std::unique_ptr<std::vector<size_t>>& ptr)
			{
				const size_t oldSize = ptr->size();
				if ((oldSize + 1) * 128 < ptr->capacity()) {
					// Reduce memory usage! Huge vectors are needed at the beginning of iteration when faces have many points on their positive side. Later on, smaller vectors will suffice.
					ptr.reset(nullptr);
					return;
				}
				m_IndexPool.Reclaim(ptr);
			}
		}
	}
}
