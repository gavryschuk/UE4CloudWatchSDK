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
#pragma once

#include "ModuleManager.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DelegateCombinations.h"

#if PLATFORM_WINDOWS
	#include "AllowWindowsPlatformTypes.h"
#endif

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/Outcome.h>

#include <aws/monitoring/CloudWatchClient.h>

#if PLATFORM_WINDOWS
	#include "HideWindowsPlatformTypes.h"
#endif

DECLARE_DELEGATE(FOnCloudWatchCustomMetricsSuccess);
DECLARE_DELEGATE_OneParam(FOnCloudWatchCustomMetricsFailed, const FString&);
class CLOUDWATCHSDK_API UCloudWatchCustomMetricsObject
{
	friend class FCloudWatchSDKModule;
public:
	FOnCloudWatchCustomMetricsSuccess OnCloudWatchCustomMetricsSuccess;
	FOnCloudWatchCustomMetricsFailed OnCloudWatchCustomMetricsFailed;
private:
	Aws::CloudWatch::CloudWatchClient* CloudWatchClient;
	FString NameSpace;
	FString GroupName;

	bool bIsRunning = false;

	static UCloudWatchCustomMetricsObject* CreateCloudWatchCustomMetrics(const FString& NameSpace, const FString& GroupName);

public:
	void Call(const FString& KeyName, const FString& ValueName, const float Value );
private:
	void OnCustomMetricsCall(const Aws::CloudWatch::CloudWatchClient* Client, const Aws::CloudWatch::Model::PutMetricDataRequest& Request, const Aws::CloudWatch::Model::PutMetricDataOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context);
};

class CLOUDWATCHSDK_API FCloudWatchSDKModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
	/**
	* public static FCloudWatchSDKModule::setupClient
	* Creates a CloudWatch Client Qith User credentials.
	* @param AccessKey [const FString&] AccessKey of your AWS user. @See http://docs.aws.amazon.com/general/latest/gr/managing-aws-access-keys.html
	* @param Secret [const FString&] SecretKey of your AWS user. @See http://docs.aws.amazon.com/general/latest/gr/managing-aws-access-keys.html
	* @param Region [const FString&] Default is set to us-east-1 (North Virginia).
	**/
	void SetupClient(const FString& AccessKey, const FString& Secret, const FString& Region = "us-east-1");
	/**
	* public FCloudWatchSDKModule::CreateCloudWatchCustomMetricsObject
	* Creates a Cloud Watch Custom Metrics Object. To Send Custom Metrics
	* @return [UCloudWatchCustomMetricsObject*] Returns UAWSCloudWatchCustomMetricsObject*. Use this to Send Custom Metrics and manage response.
	**/
	UCloudWatchCustomMetricsObject* CreateCloudWatchCustomMetricsObject(const FString& NameSpace, const FString& GroupName);
private:
	Aws::CloudWatch::CloudWatchClient* CloudWatchClient;
private:
    /** Handle to the dll we will load */
	Aws::SDKOptions options;
	static void* AWSCoreLibraryHandle;
	static void* AWSCommonLibraryHandle;
	static void* AWSEventStreamLibraryHandle;
	static void* AWSChecksumsLibraryHandle;
    static void* CloudWatchSDKLibraryHandle;
    static bool LoadDependency(const FString& Dir, const FString& Name, void*& Handle);
    static void FreeDependency(void*& Handle);

};
