// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

using UnrealBuildTool;
using System.Collections.Generic;

public class MenuSystemTarget : TargetRules
{
	public MenuSystemTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.AddRange( new string[] { "MenuSystem" } );
	}
}
