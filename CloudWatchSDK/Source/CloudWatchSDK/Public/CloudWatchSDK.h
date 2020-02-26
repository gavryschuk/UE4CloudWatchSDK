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

#include <aws/monitoring/CloudWatchClient.h>
#include <aws/monitoring/model/PutMetricDataRequest.h>

#include <aws/logs/CloudWatchLogsClient.h>
#include <aws/logs/model/InputLogEvent.h>
#include <aws/logs/model/DescribeLogStreamsRequest.h>
#include <aws/logs/model/CreateLogGroupRequest.h>
#include <aws/logs/model/CreateLogStreamRequest.h>
#include <aws/logs/model/PutLogEventsRequest.h>

#if PLATFORM_WINDOWS
	#include "HideWindowsPlatformTypes.h"
#endif

class CLOUDWATCHSDK_API ULogsCustomEventObject
{
	friend class FCloudWatchSDKModule;

private:
	Aws::CloudWatchLogs::CloudWatchLogsClient* LogsClient;
	FString GroupName;
	FString StreamName;
	
	FString mSequenceToken;
	Aws::Vector<Aws::CloudWatchLogs::Model::InputLogEvent> mInputEvents;
	
	bool bIsGroupCreated = false;
	bool bIsStreamCreated = false;
	bool bIsRunning = false;

	static ULogsCustomEventObject* CreateLogsCustomEvent(const FString& GroupName, const FString& StreamName);
public:
	void Call(const FString& Message);

private:
	void PutLogs();
	void GetSequenceToken();
	void RegisterGroup();
	void RegisterStream();

	void OnDescribeLogStreams(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::DescribeLogStreamsRequest& Request, const Aws::CloudWatchLogs::Model::DescribeLogStreamsOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context);
	void OnCreateLogGroup(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::CreateLogGroupRequest& Request, const Aws::CloudWatchLogs::Model::CreateLogGroupOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context);
	void OnCreateLogStream(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::CreateLogStreamRequest& Request, const Aws::CloudWatchLogs::Model::CreateLogStreamOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context);
	void PutLogEvent(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::PutLogEventsRequest& Request, const Aws::CloudWatchLogs::Model::PutLogEventsOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context);
};

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

	/**
	* public FCloudWatchSDKModule::CreateLogsCustomEventObject
	* Creates Cloud Custom Event Object. To Send Custom Logs
	* @param GroupName [const FString&] Group Name for the Log;
	* @param StreamName [const FString&] Stream Name for the Log;
	* @return [ULogsCustomEventObject*] Returns ULogsCustomEventObject*. Use this to Send Custom Logs.
	**/
	ULogsCustomEventObject* CreateLogsCustomEventObject(const FString& GroupName, const FString& StreamName);
private:
	Aws::CloudWatch::CloudWatchClient* CloudWatchClient;
	Aws::CloudWatchLogs::CloudWatchLogsClient* LogsClient;
private:
	Aws::SDKOptions options;
    /** Handle to the dll we will load */
	static void* AWSCoreLibraryHandle;
	static void* AWSCommonLibraryHandle;
	static void* AWSEventStreamLibraryHandle;
	static void* AWSChecksumsLibraryHandle;
    static void* AWSCloudWatchLibraryHandle;
	static void* AWSLogsLibraryHandle;

    static bool LoadDependency(const FString& Dir, const FString& Name, void*& Handle);
    static void FreeDependency(void*& Handle);

};
