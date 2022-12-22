// Custom Sessions plugin by juaxix - 2022-2023 - MIT License

using UnrealBuildTool;
using System.Collections.Generic;

public class MenuSystemEditorTarget : TargetRules
{
	public MenuSystemEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange( new string[] { "MenuSystem" } );
	}
}
