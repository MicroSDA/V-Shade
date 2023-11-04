#include "shade_pch.h"
#include "Timer.h"
#include <GLFW/glfw3.h>

shade::FrameTimer::FrameTimer():
	m_TimeNow(glfwGetTime()), m_TimeLast(0.0), m_DeltaTime(0.0)
{
}

void shade::FrameTimer::Update()
{
	m_TimeLast = m_TimeNow;
	m_TimeNow =	glfwGetTime();
	m_DeltaTime = m_TimeNow - m_TimeLast;
}
