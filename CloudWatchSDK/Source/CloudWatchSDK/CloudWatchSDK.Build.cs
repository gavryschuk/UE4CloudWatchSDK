// AMAZON CONFIDENTIAL

/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

using UnrealBuildTool;
using System.IO;

public class CloudWatchSDK : ModuleRules
{
    public CloudWatchSDK(ReadOnlyTargetRules Target) : base (Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Projects", "Engine"});

        PublicDefinitions.Add("USE_IMPORT_EXPORT");
        PublicDefinitions.Add("USE_WINDOWS_DLL_SEMANTICS");

        // This is required to fix a warning for Unreal Engine 4.21 and later
        PrivatePCHHeaderFile = "Private/CloudWatchSDKPrivatePCH.h";

        bEnableExceptions = true;

        string BaseDirectory = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "..", ".."));
        string SDKDirectory = System.IO.Path.Combine(BaseDirectory, "ThirdParty", "CloudWatchSDK", Target.Platform.ToString());

        bool bHasGameLiftSDK = System.IO.Directory.Exists(SDKDirectory);

        if (bHasGameLiftSDK)
        {
           PublicDefinitions.Add("WITH_CLOUDWATCH=1");    
                
           PublicLibraryPaths.Add(SDKDirectory);
                
           // aws-c-common
           PublicAdditionalLibraries.Add(System.IO.Path.Combine(SDKDirectory, "aws-c-common.lib"));
           PublicDelayLoadDLLs.Add("aws-c-common.dll");
           RuntimeDependencies.Add(System.IO.Path.Combine(SDKDirectory, "aws-c-common.dll"));

           // aws-c-event-stream
           PublicAdditionalLibraries.Add(System.IO.Path.Combine(SDKDirectory, "aws-c-event-stream.lib"));
           PublicDelayLoadDLLs.Add("aws-c-event-stream.dll");
           RuntimeDependencies.Add(System.IO.Path.Combine(SDKDirectory, "aws-c-event-stream.dll"));

           // aws-checksums
           PublicAdditionalLibraries.Add(System.IO.Path.Combine(SDKDirectory, "aws-checksums.lib"));
           PublicDelayLoadDLLs.Add("aws-checksums.dll");
           RuntimeDependencies.Add(System.IO.Path.Combine(SDKDirectory, "aws-checksums.dll"));

           // aws-cpp-sdk-core
           PublicAdditionalLibraries.Add(System.IO.Path.Combine(SDKDirectory, "aws-cpp-sdk-core.lib"));
           PublicDelayLoadDLLs.Add("aws-cpp-sdk-core.dll");
           RuntimeDependencies.Add(System.IO.Path.Combine(SDKDirectory, "aws-cpp-sdk-core.dll"));

            // aws-cpp-sdk-monitoring
            PublicAdditionalLibraries.Add(System.IO.Path.Combine(SDKDirectory, "aws-cpp-sdk-monitoring.lib"));
            PublicDelayLoadDLLs.Add("aws-cpp-sdk-monitoring.dll");
            RuntimeDependencies.Add(System.IO.Path.Combine(SDKDirectory, "aws-cpp-sdk-monitoring.dll"));

            // aws-cpp-sdk-logs
            PublicAdditionalLibraries.Add(System.IO.Path.Combine(SDKDirectory, "aws-cpp-sdk-logs.lib"));
            PublicDelayLoadDLLs.Add("aws-cpp-sdk-logs.dll");
            RuntimeDependencies.Add(System.IO.Path.Combine(SDKDirectory, "aws-cpp-sdk-logs.dll"));
        }
        else
        {
            PublicDefinitions.Add("WITH_CLOUDWATCH=0");
        }
    }
}
