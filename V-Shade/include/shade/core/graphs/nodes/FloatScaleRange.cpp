#include "shade_pch.h"
#include "FloatScaleRange.h"

float ScaleToRange(float value, float inputMin, float inputMax, float outputMin, float outputMax) 
{
	float normalizedValue = (value - inputMin) / (inputMax - inputMin);	
	return glm::mix(outputMin, outputMax, value);
}

void shade::graphs::FloatScaleRange::Evaluate(const FrameTimer& deltaTime)
{
	GET_ENDPOINT<Connection::Output, NodeValueType::Float>(0, 
		ScaleToRange(GET_ENDPOINT<Connection::Input, NodeValueType::Float>(0),
		GET_ENDPOINT<Connection::Input, NodeValueType::Float>(1),
		GET_ENDPOINT<Connection::Input, NodeValueType::Float>(2),
		GET_ENDPOINT<Connection::Input, NodeValueType::Float>(3),
		GET_ENDPOINT<Connection::Input, NodeValueType::Float>(4))); // Value
}