#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/KeyCodes.h>
#include <shade/core/MouseCodes.h>

#define BIT(x) (1 << x)

namespace shade
{

	class SHADE_API Event
	{
	public:
		enum class Type
		{
			_None_ = 0,
			_WindowClose_, _WindowResize_, _WindowFocus_, _WindowLostFocus_, _WindowMoved_,
			_AppTick_, _AppUpdate, _AppRender_,
			_KeyPressed_, _KeyReleased_, _KeyTyped_,
			_MouseButtonPressed_, _MouseButtonReleased_, _MouseMoved_, _MouseScrolled_
		};
		enum Category
		{
			_None_ = 0,
			_ApplicationEvent_ = BIT(0),
			_InputEvent_ = BIT(1),
			_KeyboardEvent_ = BIT(2),
			_MouseEvent_ = BIT(3),
			_MouseButtonEvent_ = BIT(4),
			_UserEvent_ = BIT(5)
		};

		Event() = default;
		virtual ~Event() = default;
		virtual Type GetType() const = 0;
		//virtual Category GetCategory() const = 0;

		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(Category category)
		{
			return GetCategoryFlags() & category;
		}

		std::function<void(Event&)> EventCallback;
	};


#define EVENT_CLASS_TYPE(type) static Event::Type GetStaticType() { return  Event::Type::##type; }\
							   virtual Event::Type GetType() const override { return GetStaticType(); }\
							   virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(std::uint32_t width, std::uint32_t height):
			Width(width), Height(height) {}
		virtual ~WindowResizeEvent() = default;
		EVENT_CLASS_TYPE(_WindowResize_);
		EVENT_CLASS_CATEGORY(_ApplicationEvent_);
		std::uint32_t Width, Height;
	};
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;
		virtual ~WindowCloseEvent() = default;
		EVENT_CLASS_TYPE(_WindowClose_);
		EVENT_CLASS_CATEGORY(_ApplicationEvent_);
	};

	class KeyEvent : public shade::Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(_KeyboardEvent_ | _InputEvent_)
	protected:
		KeyEvent(const KeyCode keycode)
			: m_KeyCode(keycode) {}

		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keycode, const uint16_t repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) {}

		uint16_t GetRepeatCount() const { return m_RepeatCount; }

		EVENT_CLASS_TYPE(_KeyPressed_)
	private:
		uint16_t m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}
		EVENT_CLASS_TYPE(_KeyReleased_)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const KeyCode keycode)
			: KeyEvent(keycode) {}
		EVENT_CLASS_TYPE(_KeyTyped_)
	};

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x, const float y)
			: m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		EVENT_CLASS_TYPE(_MouseMoved_);
		EVENT_CLASS_CATEGORY(_MouseEvent_ | _InputEvent_);
	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }
		EVENT_CLASS_TYPE(_MouseScrolled_);
		EVENT_CLASS_CATEGORY(_MouseEvent_  | _InputEvent_);
	private:
		float m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(_MouseEvent_ | _InputEvent_ | _MouseButtonEvent_);
	protected:
		MouseButtonEvent(const MouseCode button)
			: m_Button(button) {}

		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}
		EVENT_CLASS_TYPE(_MouseButtonPressed_)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const MouseCode button)
			: MouseButtonEvent(button) {}
		EVENT_CLASS_TYPE(_MouseButtonReleased_)
	};
}