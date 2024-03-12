#pragma once
#include <shade/config/ShadeAPI.h>

namespace shade
{
	class SHADE_API FrameTimer
	{
	public:
		FrameTimer();
		FrameTimer(float time);
		~FrameTimer() = default;
		void Update();

		template <typename T, std::enable_if_t<std::is_scalar_v<T>, bool> = true>
		inline T GetInSeconds() const 
		{
			return static_cast<T>(m_DeltaTime); 
		}
		template<typename T, typename = std::enable_if<std::is_scalar<T>::value>>
		inline T GetInMilliseconds() const 
		{
			return static_cast<T>(m_DeltaTime * 1000.0);
		}

		operator double() const { return static_cast<double>(m_DeltaTime * 1000.0); } // milliseconds
	private:
		double m_TimeLast, m_TimeNow, m_DeltaTime;
	};
}