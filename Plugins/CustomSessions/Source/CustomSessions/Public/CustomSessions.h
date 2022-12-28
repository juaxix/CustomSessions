// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCustomSessionsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
