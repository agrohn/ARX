// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class ARX : ModuleRules
{
    public string ThirdPartyPath {
			get {
            return Path.Combine(ModuleDirectory, "..", "..", "ThirdParty");
			}
    }
    

    
    public ARX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] { 
				"ARX/Public"
				// ... add public include paths required here ...

			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"ARX/Private",
				// ... add other private include paths required here ...

			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",

				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

				// ... add any modules that your module loads dynamically here ...
			}
		);

		string LibraryPath = Path.Combine(ThirdPartyPath, "lib", Target.Platform.ToString(), Target.Architecture); 
		string IncludePath = Path.Combine(ThirdPartyPath, "include", Target.Platform.ToString(), Target.Architecture);
		Console.WriteLine( "includepath: " + IncludePath);
        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicIncludePaths.AddRange(new string[]{
                    Path.Combine(IncludePath),
            });
            PublicAdditionalLibraries.AddRange(new string[] {
                    Path.Combine( LibraryPath, "ARX"),
            });

            RuntimeDependencies.Add(Path.Combine(LibraryPath, "libARX.so"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32)
        {
            PublicIncludePaths.AddRange(new string[]{
                    Path.Combine(IncludePath),
            });
            
            RuntimeDependencies.Add(Path.Combine(LibraryPath, "ARX.dll"));
            PublicDelayLoadDLLs.Add("ARX.dll");
            Console.WriteLine("Attempting to include lib: " + Path.Combine(LibraryPath, "ARX.lib"));
            PublicAdditionalLibraries.AddRange(new string[] {
                Path.Combine(LibraryPath, "AR.lib"),
                Path.Combine(LibraryPath, "AR2.lib"),
                Path.Combine(LibraryPath, "ARG.lib"),
                Path.Combine(LibraryPath, "ARUtil.lib"),
                Path.Combine(LibraryPath, "ARVideo.lib"),
                Path.Combine(LibraryPath, "OCVT.lib"),
                Path.Combine(LibraryPath, "Eden.lib"),
                Path.Combine(LibraryPath, "libjpeg.lib"),
                Path.Combine(LibraryPath, "zlibstatic.lib"),
                Path.Combine(LibraryPath, "KPM.lib"),
                Path.Combine(LibraryPath, "pthreadVC2static.lib")
            });
            
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string[] archs = new string[] { /*"arm64-v8a",*/ "armeabi-v7a"/*, "x86","x86_64"*/ };

            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));
            foreach (string arch in archs)
            {
                // Target.Architecture does not seem to work in Android
                IncludePath = Path.Combine(ThirdPartyPath, "include", Target.Platform.ToString(), arch);
                LibraryPath = Path.Combine(ThirdPartyPath, "lib", Target.Platform.ToString(), arch);

                PublicIncludePaths.Add(IncludePath);
                PublicLibraryPaths.Add(LibraryPath);

                RuntimeDependencies.Add(Path.Combine(LibraryPath, "libARX.so"));
                PublicAdditionalLibraries.AddRange(new string[] {
                        Path.Combine("ARX")
                    });

            }
            DynamicallyLoadedModuleNames.Add("ARX");
            // this is the preferred way to include shared libraries to APK.
            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "ARX_APL.xml"));

        }
	}
}
