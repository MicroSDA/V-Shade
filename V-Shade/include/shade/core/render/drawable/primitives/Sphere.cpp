#include "shade_pch.h"
#include "Sphere.h"

shade::SharedPointer<shade::Sphere> shade::Sphere::Create(float radius, float density, float step)
{
	return SharedPointer<Sphere>::Create(radius, density, step);
}

shade::Sphere::Sphere(std::size_t xDensity, std::size_t yDensity, float radius)
{
	std::size_t indicesCount = 6 * (xDensity + (yDensity - 2) * xDensity), verticesCount = (yDensity + 1) * (xDensity + 1);

	GetVertices().resize(verticesCount);
	GetIndices().resize(indicesCount);

	// Vertices
	float xStep = 2.0 * glm::pi<float>() / xDensity;
	float yStep = glm::pi<float>() / yDensity;

	Vertices::iterator vertex = GetVertices().begin();

	for (std::size_t y = 0; y <= yDensity; y++)
	{
		for (std::size_t x = 0; x <= xDensity; x++)
		{
			float yP = sin(-glm::pi<float>() / 2 + y * yStep);
			float xP = cos(x * xStep) * sin(y * yStep);
			float zP = sin(x * xStep) * sin(y * yStep);

			vertex->Position			= glm::vec3(xP, yP, zP) * radius;
			vertex->Normal				= glm::normalize(glm::vec3(xP, yP, zP));
			vertex->UV_Coordinates		= glm::vec2(static_cast<float>(x) / xDensity , static_cast<float>(y) / yDensity );

			vertex++;
		}
	}
	// Indices
	Indices::iterator index = GetIndices().begin(); std::size_t vert = xDensity + 1;
	for (std::size_t x = 0; x < xDensity; ++x, vert++)
	{
		(*index++) = x; 
		(*index++) = vert; 
		(*index++) = vert + 1;
	}
	for (std::size_t y = 1; y < yDensity - 1; y++)
	{
		vert = y * (xDensity + 1);
		for (std::size_t x = 0; x < xDensity; x++, vert++)
		{
			(*index++) = vert; 
			(*index++) = vert + xDensity + 1;
			(*index++) = vert + 1;
			(*index++) = vert + 1;
			(*index++) = vert + xDensity + 1; 
			(*index++) = vert + xDensity + 2;
		}
	}
	vert = (yDensity - 1) * (xDensity + 1);
	for (std::size_t x = 0; x < xDensity; x++, vert++)
	{
		(*index++) = vert; 
		(*index++) = vert + xDensity + 1; 
		(*index++) = vert + 1;
	}
	

 //   // Polygon mode Lines !
	//const float _step = std::atan(1.0f) * step / density;
	//for (std::uint32_t i = 0; i < density; i++) 
	//{
	//	float a = i * _step;
	//	AddVertex({.Position = glm::vec3(std::cos(a) * radius, std::sin(a) * radius, 0.0f) });
	//	AddIndex(i);
	//}
	//for (std::uint32_t i = 0; i < density; i++)
	//{
	//	float a = i * _step;
	//	AddVertex({ .Position = glm::vec3(0.0f, std::cos(a) * radius, std::sin(a) * radius) });
	//	AddIndex(density + i);
	//}
	//for (std::uint32_t i = 0; i < density; i++)
	//{
	//	float a = i * _step;
	//	AddVertex({ .Position = glm::vec3(std::cos(a) * radius, 0.0f, std::sin(a) * radius) });
	//	AddIndex(density + density + i);
	//}
}
