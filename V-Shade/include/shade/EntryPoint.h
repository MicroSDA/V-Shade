#pragma once
#include <shade/core/application/Application.h>

#ifdef SHADE_WINDOWS_PLATFORM

extern shade::Application* shade::CreateApplication(int argc, char* argv[]);

int main(int argc, char* argv[])
{

	shade::Logger::Initialize();

	shade::Application* pApp = nullptr;

	pApp = shade::CreateApplication(argc, argv);
	pApp->Initialize();
	pApp->OnCreate();
	pApp->Launch();
	pApp->Terminate();

	if (pApp != nullptr)
		delete pApp;

	return 0;
}
#endif // SHADE_WINDOW_PLATFORM