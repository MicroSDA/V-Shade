#pragma once

#include <Shade.h>
#include <layer/EditorLayer.h>


class EditorApplication : public shade::Application
{
public:
	EditorApplication(int argc, char* argv[]);
	virtual ~EditorApplication() = default;
	virtual void OnCreate() override;
	virtual void OnDestroy() override;
};

