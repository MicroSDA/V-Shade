#include "shade_pch.h"
#include "Vertex.h"

namespace shade
{
	namespace algo
	{
		struct Edge
		{
			Index First = 0;
			Index Second = 0;
			glm::mat4 Q = glm::mat4(0.0);
			glm::vec3 P = glm::vec3(0.0);
			float Error = 0.f;

			bool operator == (const Edge& other) const
			{
				return ((First == other.First && Second == other.Second) || (Second == other.First && First == other.Second));
			}
			bool operator != (const Edge& other) const
			{
				return ((First != other.First && Second != other.Second) || (Second != other.First && First != other.Second));
			}
			bool operator < (const Edge& other) const
			{
				return (Error < other.Error);
			}
			bool operator > (const Edge& other) const
			{
				return (Error > other.Error);
			}
		};

		// Create a vector of unique edges from a vector of face indices.
		// This function extracts edges from the face indices, ensuring that only unique edges are added.
		// Parameters:
		// - indices: A vector of face indices where each group of three indices represents a face.
		// Returns:
		// A vector containing unique edges defined by pairs of vertex indices.
		std::vector<Edge> CreateEdges(const Indices& indices)
		{
			std::vector<Edge> edges;
			// Iterate through the face indices, with each group of three indices representing a face.
			for (std::size_t i = 0; i < indices.size(); i += 3)
			{
				for (int j = 0; j < 3; ++j)
				{
					// Extract vertex indices for the current edge.
					Index first = indices[i + j];
					Index second = indices[i + (j + 1) % 3];

					// Ensure that the vertex indices are sorted in ascending order.
					if (first > second)
						std::swap(first, second);

					// Create an Edge struct representing the current edge.
					Edge edge{ .First = first, .Second = second };

					// Add the edge to the vector of edges only if it is unique (not already present).
					if (std::find(edges.begin(), edges.end(), edge) == edges.end())
						edges.push_back(edge);
				}
			}

			return edges;
		}
		// Create a multimap of neighboring vertices for each vertex in a mesh.
		// This function determines which vertices are neighbors to each vertex by examining the edges in the mesh.
		// Parameters:
		// - vertices: A vector of vertex data, including positions.
		// - edges: A vector of edges in the mesh, each defined by two vertex indices.
		// Returns:
		// A multimap where each vertex index is associated with its neighboring vertex indices.
		std::multimap<Index, Index> CreateNeighborVerticesTest(const Vertices& vertices, const std::vector<Edge>& edges)
		{
			std::multimap<Index, Index> neighborVertices;

			for (std::size_t i = 0; i < vertices.size(); i++)
			{
				// Iterate through all edges in the mesh.
				for (const auto& edge : edges)
				{
					// Check if the current vertex index matches the first vertex index of the edge.
					if (i == edge.First)
					{
						// Insert the second vertex index of the edge as a neighbor of the current vertex.
						neighborVertices.insert({ i, edge.Second });
					}
					// Check if the current vertex index matches the second vertex index of the edge.
					if (i == edge.Second)
					{
						// Insert the first vertex index of the edge as a neighbor of the current vertex.
						neighborVertices.insert({ i, edge.First });
					}
				}
			}

			return neighborVertices;
		}
		// Check if two vertices are connected within a single triangle in a mesh.
		// This function examines the neighboring vertices of the 'second' vertex and checks if 'third' is one of its neighbors.
		// If 'third' is a neighboring vertex of 'second', they are considered to be connected within a single triangle.
		// Parameters:
		// - second: The index of the second vertex.
		// - third: The index of the third vertex to check for as a neighbor of 'second'.
		// - neighborVertices: A multimap that maps vertex indices to their neighboring vertex indices.
		// Returns:
		// 'true' if 'third' is a neighbor of 'second' within a single triangle, 'false' otherwise.
		bool WithinOneTriangle(std::size_t second, std::size_t third, const std::multimap<Index, Index>& neighborVertices)
		{
			// Find neighboring vertices of the 'second' vertex in the multimap.
			auto result = neighborVertices.equal_range(second);

			for (auto it = result.first; it != result.second; it++)
			{
				// Check if 'third' is one of the neighboring vertices of 'second'.
				if (third == it->second)
					return true;
			}

			// 'third' is not a neighbor of 'second' within a single triangle.
			return false;
		}
		// Calculate vertex errors for each vertex in the mesh.
		// This function computes an error matrix (glm::mat4) for each vertex based on its neighboring vertices.
		// The error matrix quantifies the geometric error of the vertex in relation to its neighbors.
		// Parameters:
		// - vertices: A vector of vertex data, including positions.
		// - neighborVertices: A multimap that maps each vertex index to its neighboring vertex indices.
		// Returns:
		// A vector of error matrices, where each matrix corresponds to a vertex.
		std::vector<glm::mat4> CreateVertexError(const Vertices& vertices, const std::multimap<Index, Index>& neighborVertices)
		{
			// Initialize a vector of error matrices, one for each vertex, and set them all to zero.
			std::vector<glm::mat4> errors(vertices.size(), glm::mat4(0.0f));

			// Loop through all vertices in the mesh.
			for (std::size_t index = 0; index < vertices.size(); index++)
			{
				// Find neighboring vertices for the current vertex.
				auto result = neighborVertices.equal_range(index);

				// Iterate through neighboring vertex pairs.
				for (auto it = result.first; it != result.second; it++)
				{
					for (auto it2 = it; it2 != result.second; it2++)
					{
						// Avoid processing the same neighbor pair twice.
						if (it2->second != it->second)
						{
							// Check if the neighboring vertices form a triangle with the current vertex.
							if (WithinOneTriangle(it->second, it2->second, neighborVertices))
							{
								// Calculate vectors from the current vertex to its neighbors.
								const glm::vec3 v1 = vertices[it->second].Position - vertices[index].Position;
								const glm::vec3 v2 = vertices[it2->second].Position - vertices[index].Position;

								// Calculate the normal vector of the triangle formed by the current vertex and its neighbors.
								const glm::vec3 normal = glm::normalize(glm::cross(v2, v1));

								// Calculate a temporary vector 't' containing the normal and a distance component.
								const glm::vec4 t = glm::vec4(normal, -glm::dot(vertices[index].Position, normal));

								// Update the error matrix for the current vertex using the outer product of 't'.
								errors[index] += glm::outerProduct(t, t);
							}
						}
					}
				}
			}
			// Return the vector of error matrices for all vertices.
			return errors;
		}

		// Calculate the error for a given edge.
		// The error is computed based on the quadric error matrices of its two endpoints.
		// Parameters:
		// - edge: The edge for which to calculate the error.
		// - vertexErrors: A vector of quadric error matrices for all vertices in the mesh.
		// - vertices: A vector of vertex data, including positions.
		void CalculateEdgeError(Edge& edge, const std::vector<glm::mat4>& vertexErrors, const Vertices& vertices)
		{
			// Calculate the combined quadric error matrix for the edge by summing the matrices of its two endpoints.
			edge.Q = vertexErrors[edge.First] + vertexErrors[edge.Second];

			// Calculate the midpoint of the edge.
			edge.P = glm::vec3((vertices[edge.First].Position + vertices[edge.Second].Position) / 2.0f);

			// Calculate the error for the edge using the quadric error matrix and the midpoint.
			// The error is computed as the dot product of a 4D vector and the product of the quadric matrix and the 4D vector.
			// The 4D vector represents the midpoint in 3D space with an additional component (1.0).
			edge.Error = glm::dot(glm::vec4(edge.P, 1.0), edge.Q * glm::vec4(edge.P, 1.0));
		}
		void CalculateEdgesError(std::vector<Edge>& edges, const std::vector<glm::mat4>& vertexErrors, const Vertices& vertices)
		{
			for (auto& edge : edges)
				CalculateEdgeError(edge, vertexErrors, vertices);
		}
		// Calculate and update faces in a mesh, removing specific faces and edges.
		// This function searches for faces in the mesh that contain the specified edge indices (firstEdge and secondEdge).
		// If a face contains these edge indices, it is removed from the mesh along with any corresponding edges.
		// Additionally, the indices that were pointing to the removed edge are updated to point to the newly combined vertex.
		// Parameters:
		// - firstEdge: The index of the first edge to check for in faces.
		// - secondEdge: The index of the second edge to check for in faces.
		// - indices: A vector of indices representing the faces of the mesh.
		// - edges: A vector of edges in the mesh.
		void CalculateFaces(Index firstEdge, Index secondEdge, Vertices& vertices, Indices& indices, std::vector<Edge>& edges)
		{
			Index first, second, third;

			for (size_t i = 0; i < indices.size() || i + 2 < indices.size(); )
			{
				// Extract the vertex indices of the current face.
				first = indices[i]; second = indices[i + 1]; third = indices[i + 2];

				// Check if the current face contains the specified edge indices (firstEdge and secondEdge).
				if (
					(firstEdge == first || firstEdge == second || firstEdge == third) &&
					(secondEdge == first || secondEdge == second || secondEdge == third))
				{
					// Remove matching edges from the 'edges' vector using the remove-erase idiom.
					edges.erase(std::remove_if(edges.begin(), edges.end(), [&](const Edge& edge) {
						return  (edge.First != firstEdge && edge.Second != firstEdge) &&
							(edge.First == first || edge.First == second || edge.First == third) &&
							(edge.Second == first || edge.Second == second || edge.Second == third);
						}), edges.end());

					// Remove the face indices from the 'indices' vector.
					indices.erase(indices.begin() + i, indices.begin() + i + 3);
					// Update the iterator to the previous face if applicable.
					if (i > 2) i -= 3;
				}
				else
				{
					// Move to the next face in the 'indices' vector.
					i += 3;
				}
			}
			// Update indices that were previously pointing to the removed edge to point to the newly combined vertex.
			for (Index& index : indices)
				if (index == secondEdge)
					index = firstEdge;
		}
		// Simplifies a 3D mesh by reducing the number of faces while preserving overall shape.
	    // This function simplifies a 3D mesh represented by its vertices and indices. It iteratively removes faces and edges
		// based on a specified count until the desired level of simplification is achieved.
	    // Parameters:
	    // - vertices: The vector of vertex data representing the mesh.
	    // - indices: The vector of indices defining the mesh's triangles.
	    // - count: The desired number of faces to reduce the mesh to.
		void SimplifyMesh(Vertices& vertices, Indices& indices, std::size_t count)
		{
			// Create edges from the input face indices.
			std::vector<Edge> edges = CreateEdges(indices);

			// Create a multimap to store neighbor vertices for efficient lookup.
			std::multimap<Index, Index> neighborVertices = CreateNeighborVerticesTest(vertices, edges);

			// Calculate vertex errors and initialize them.
			std::vector<glm::mat4> vertexErrors = CreateVertexError(vertices, neighborVertices);
			CalculateEdgesError(edges, vertexErrors, vertices);

			// Create a max heap for edges based on their error values.
			std::make_heap(edges.begin(), edges.end(), [&](const Edge& a, const Edge& b) { return (a.Error > b.Error); });

			while ((indices.size() / 3) > count)
			{
				// Pop the edge with the highest error from the heap.
				std::pop_heap(edges.begin(), edges.end(), [&](const Edge& a, const Edge& b) { return (a.Error > b.Error); });
				if (!edges.size()) break;

				const Edge edgeRemove = edges.back();

				// Update the errors and position of the newly set vertex.
				vertexErrors[edgeRemove.First] = edgeRemove.Q;
				vertices[edgeRemove.First].Position = edgeRemove.P;
				
				bool neighborToBothVertices = false;

				// Find neighbors of the removed edge's second vertex.
				auto ret = neighborVertices.equal_range(edgeRemove.Second);
				for (auto it = ret.first; it != ret.second; ++it)
				{
					neighborToBothVertices = false;
					auto neighbor = neighborVertices.equal_range(it->second);
					for (auto it2 = neighbor.first; it2 != neighbor.second; ++it2)
					{
						// Check if this neighbor contained both firstVertexInd and secondVertexInd.
						if (it2->second == edgeRemove.First)
							neighborToBothVertices = true;
					}

					// Add the neighbor vertex if it is not already connected to the new vertex.
					if (it->second != edgeRemove.First && !neighborToBothVertices)
						neighborVertices.insert(std::pair<Index, Index>(it->second, edgeRemove.First));
				}

				// Remove the edge with the minimal error.
				edges.pop_back();

				// Remove the deleted vertex in the neighbors multimap.
				neighborVertices.erase(edgeRemove.Second);

				// Recalculate faces after vertex simplification.
				CalculateFaces(edgeRemove.First, edgeRemove.Second, vertices, indices, edges);

				// Update connected edges and recalculate errors.
				for (auto& edge : edges)
				{
					if (edge.Second == edgeRemove.Second)
					{
						edge.Second = edgeRemove.First;
					}
					else if (edge.First == edgeRemove.Second)
					{
						edge.First = edgeRemove.First;
					}

					if (edge.First == edgeRemove.First || edge.Second == edgeRemove.First)
						CalculateEdgeError(edge, vertexErrors, vertices);
				}

				// Rebuild the heap with the updated edge errors.
				std::make_heap(edges.begin(), edges.end(), [&](const Edge& a, const Edge& b) { return (a.Error > b.Error); });
			}

			std::unordered_map<Index, Index> indexMapping; // Maps old indices to new indices

			// Step 1: Create a mapping from old indices to new indices
			Indices newIndices;
			for (std::size_t oldIndex : indices) 
			{
				if (indexMapping.find(oldIndex) == indexMapping.end()) 
				{
					// This old index has not been mapped to a new index yet
					indexMapping[oldIndex] = newIndices.size();
					newIndices.push_back(oldIndex);
				}
			}

			// Step 2: Create a new vector of vertices based on new indices
			Vertices newVertices;
			for (std::size_t newIndex : newIndices) 
			{
				newVertices.push_back(vertices[newIndex]);
			}

			// Step 3: Update the vector of indices to use the new indices
			for (Index& oldIndex : indices) {
				oldIndex = indexMapping[oldIndex];
			}

			vertices = newVertices;
		}

		std::vector<std::size_t> CalculateFaceCountLodLevel(std::size_t levelCount, std::size_t maxFaces, std::size_t minFaces, float splitLambda)
		{
			std::vector<std::size_t> intermediateValues;
			// Calculate the step size to evenly divide the range between the upper and lower thresholds.
			float stepSize = (maxFaces - minFaces) / (levelCount - 1);

			// Generate the intermediate values with a variable transition factor.
			for (int i = 0; i < levelCount; i++) {
				float t = static_cast<float>(i) / (levelCount - 1);

				// Calculate the transition factor as a linear interpolation between 1 and maxTransitionFactor.
				float transitionFactor = 1.0 + (splitLambda - 1.0) * t;

				float value = maxFaces - (maxFaces - minFaces) * pow(t, transitionFactor);
				intermediateValues.push_back(value);
			}

			return intermediateValues;
		}
	}
}
