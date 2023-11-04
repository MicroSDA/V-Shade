#pragma once
#include <shade/core/scripting/ScriptableEntity.h>

#ifdef SHADE_SCRIPT
#define SHADE_SCRIPT_API __declspec(dllexport)
#else
#define SHADE_SCRIPT_API __declspec(dllimport)
#endif
/* Prototype of function that should be created in client script */

struct ScriptPack
{
	struct ScriptPair
	{
		shade::ecs::ScriptableEntity* Instance;
		char* Name;
	};
	ScriptPair* Scripts;
	std::size_t Count;
};

#ifdef __cplusplus
extern "C" {
#endif
	SHADE_SCRIPT_API ScriptPack InstantiatePack();
#ifdef __cplusplus
}
#endif