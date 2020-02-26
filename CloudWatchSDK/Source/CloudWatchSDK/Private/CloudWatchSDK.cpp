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
#include "CloudWatchSDK.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"
#include "CloudWatchGlobals.h"

#if WITH_CLOUDWATCH
#include <aws/core/utils/Outcome.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/client/ClientConfiguration.h>
#endif

ULogsCustomEventObject* ULogsCustomEventObject::CreateLogsCustomEvent(const FString& GroupName, const FString& StreamName)
{
#if WITH_CLOUDWATCH
	ULogsCustomEventObject* Proxy = new ULogsCustomEventObject();
	Proxy->GroupName = GroupName;
	Proxy->StreamName = StreamName;
	return Proxy;
#endif
	return nullptr;
}

void ULogsCustomEventObject::Call(const FString& Message)
{
#if WITH_CLOUDWATCH
	// add messages to the log
	Aws::CloudWatchLogs::Model::InputLogEvent LogEvent;
	LogEvent.SetTimestamp(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	LogEvent.SetMessage(TCHAR_TO_UTF8(*Message));
	mInputEvents.push_back(LogEvent);

	// if the previous log is in progress => quit
	if (bIsRunning) return;

	if (LogsClient)
	{
		// log is running
		bIsRunning = true;

		//if Sequence Token is absent => Request Sequence Token
		if (mSequenceToken.Len() == 0) {
			GetSequenceToken();
			return;
		}

		// send logs
		PutLogs();
		return;
	}
	// something went wrong!!!!
	LOG_ERROR("LogsClient is null. Did you call CreateLogsObject and CreateLogsCustomEventObject first?");
#endif
}

void ULogsCustomEventObject::GetSequenceToken()
{
#if WITH_CLOUDWATCH
	// generate describeLogStream Request
	Aws::CloudWatchLogs::Model::DescribeLogStreamsRequest StreamsRequest;
	StreamsRequest.SetLogGroupName(TCHAR_TO_UTF8(*GroupName));
	StreamsRequest.SetLogStreamNamePrefix(TCHAR_TO_UTF8(*StreamName));

	// setup handler
	Aws::CloudWatchLogs::DescribeLogStreamsResponseReceivedHandler StreamRequestHandler;
	StreamRequestHandler = std::bind(&ULogsCustomEventObject::OnDescribeLogStreams, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	
	// call DescribeLogStream
	LogsClient->DescribeLogStreamsAsync(StreamsRequest, StreamRequestHandler);
#endif
}

void ULogsCustomEventObject::PutLogs() 
{
#if WITH_CLOUDWATCH
	// If InputEvents STack is empty => stop the process
	if (mInputEvents.size() == 0) {
		bIsRunning = false;
		return;
	}
	
	// setup GroupName and StreamName
	Aws::CloudWatchLogs::Model::PutLogEventsRequest LogEventRequest;
	LogEventRequest.SetLogGroupName(TCHAR_TO_UTF8(*GroupName));
	LogEventRequest.SetLogStreamName(TCHAR_TO_UTF8(*StreamName));
	LogEventRequest.SetLogEvents(Aws::Vector<Aws::CloudWatchLogs::Model::InputLogEvent>(mInputEvents)); // log stack is copied into a new aws::vector

	// clear current log stack
	mInputEvents.clear();

	//Add Sequence Token to the request
	if( mSequenceToken.Len() > 0 ) LogEventRequest.SetSequenceToken(TCHAR_TO_UTF8(*mSequenceToken));

	// setup handler
	Aws::CloudWatchLogs::PutLogEventsResponseReceivedHandler PutLogEventHandler;
	PutLogEventHandler = std::bind(&ULogsCustomEventObject::PutLogEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	// send Custom Log
	LogsClient->PutLogEventsAsync(LogEventRequest, PutLogEventHandler);
#endif
}

void ULogsCustomEventObject::RegisterGroup() 
{
#if WITH_CLOUDWATCH
	// if Log Goup is absent (new Log Group)
	if (!bIsGroupCreated) {
		// generate CreateLogGroup Request
		Aws::CloudWatchLogs::Model::CreateLogGroupRequest LogGroupRequest;
		LogGroupRequest.SetLogGroupName(TCHAR_TO_UTF8(*GroupName));
		// setup handler
		Aws::CloudWatchLogs::CreateLogGroupResponseReceivedHandler LogGroupRequestHandler;
		LogGroupRequestHandler = std::bind(&ULogsCustomEventObject::OnCreateLogGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		// call Generate Log Group
		LogsClient->CreateLogGroupAsync(LogGroupRequest, LogGroupRequestHandler);
	}
#endif
}

void ULogsCustomEventObject::RegisterStream()
{
#if WITH_CLOUDWATCH
	// if Log Stream is absent (new Log Stream)
	if (!bIsStreamCreated) {
		// generate CreateStreamGroup Request
		Aws::CloudWatchLogs::Model::CreateLogStreamRequest LogStreamRequest;
		LogStreamRequest.SetLogGroupName(TCHAR_TO_UTF8(*GroupName));
		LogStreamRequest.SetLogStreamName(TCHAR_TO_UTF8(*StreamName));
		// setup handler
		Aws::CloudWatchLogs::CreateLogStreamResponseReceivedHandler LogStreamRequestHandler;
		LogStreamRequestHandler = std::bind(&ULogsCustomEventObject::OnCreateLogStream, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		// call Generate Stream Group
		LogsClient->CreateLogStreamAsync(LogStreamRequest, LogStreamRequestHandler);
	}
#endif
}

void ULogsCustomEventObject::OnDescribeLogStreams(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::DescribeLogStreamsRequest& Request, const Aws::CloudWatchLogs::Model::DescribeLogStreamsOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context)
{
#if WITH_CLOUDWATCH
	if (!Outcome.IsSuccess()) {
		LOG_ERROR(FString::Printf(TEXT("On Describe Log Streams: %s"), *FString(Outcome.GetError().GetMessage().c_str())));
		if (!bIsGroupCreated) RegisterGroup(); // register a group
		else if (!bIsStreamCreated) RegisterStream(); // register a stream
	} else {
		// goup and stream are present on CLoudWatchLogs
		bIsGroupCreated = true;
		bIsStreamCreated = true;
		// setup sequence token
		mSequenceToken = FString(Outcome.GetResult().GetLogStreams()[0].GetUploadSequenceToken().c_str());
		// send logs
		PutLogs();
	}
#endif
}

void ULogsCustomEventObject::OnCreateLogGroup(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::CreateLogGroupRequest& Request, const Aws::CloudWatchLogs::Model::CreateLogGroupOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context)
{
#if WITH_CLOUDWATCH
	if (!Outcome.IsSuccess()) {
		LOG_ERROR(FString::Printf(TEXT("Log Group Was Not Registered: %s"), *FString(Outcome.GetError().GetMessage().c_str())));
		// stop the log
		bIsRunning = false;
	} else {
		bIsGroupCreated = true;
		// register stream
		RegisterStream();
	}
#endif
}

void ULogsCustomEventObject::OnCreateLogStream(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::CreateLogStreamRequest& Request, const Aws::CloudWatchLogs::Model::CreateLogStreamOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context)
{
#if WITH_CLOUDWATCH
	if (!Outcome.IsSuccess()) {
		LOG_ERROR(FString::Printf(TEXT("Log Stream Was Not Registered: %s"), *FString(Outcome.GetError().GetMessage().c_str())));
		// stop the log
		bIsRunning = false;
	} else {
		bIsStreamCreated = true;
		// send logs
		PutLogs();
	}
#endif
}

void ULogsCustomEventObject::PutLogEvent(const Aws::CloudWatchLogs::CloudWatchLogsClient* Client, const Aws::CloudWatchLogs::Model::PutLogEventsRequest& Request, const Aws::CloudWatchLogs::Model::PutLogEventsOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context)
{
#if WITH_CLOUDWATCH
	if (!Outcome.IsSuccess()) {
		// we are failed!
		LOG_ERROR(FString::Printf(TEXT("PutLogEvent Error: %s"), *FString(Outcome.GetError().GetMessage().c_str())));
	} else {
		// get sequence token for next request
		mSequenceToken = FString(Outcome.GetResult().GetNextSequenceToken().c_str());
	}
	// stop the log
	bIsRunning = false;
#endif
}

UCloudWatchCustomMetricsObject* UCloudWatchCustomMetricsObject::CreateCloudWatchCustomMetrics(const FString& NameSpace, const FString& GroupName)
{
#if WITH_CLOUDWATCH
	UCloudWatchCustomMetricsObject* Proxy = new UCloudWatchCustomMetricsObject();
	Proxy->NameSpace = NameSpace;
	Proxy->GroupName = GroupName;
	return Proxy;
#endif
	return nullptr;
}

void UCloudWatchCustomMetricsObject::Call(const FString& KeyName, const FString& ValueName, const float Value)
{
#if WITH_CLOUDWATCH
	// if the previous custom metrics log is in progress => quit
	if (bIsRunning) {
		LOG_NORMAL("Previous Custom Metrics call is in progress. Make next call after responce is recieved");
		return;
	}

	if (CloudWatchClient)
	{
		bIsRunning = true;

		Aws::CloudWatch::Model::Dimension dimension;
		dimension.SetName(TCHAR_TO_UTF8(*GroupName));
		dimension.SetValue(TCHAR_TO_UTF8(*KeyName));

		Aws::CloudWatch::Model::MetricDatum datum;
		datum.SetMetricName(TCHAR_TO_UTF8(*ValueName));
		datum.SetUnit(Aws::CloudWatch::Model::StandardUnit::None);
		datum.SetValue(static_cast<double>(Value));
		datum.AddDimensions(dimension);

		Aws::CloudWatch::Model::PutMetricDataRequest MetricDataRequest;
		MetricDataRequest.SetNamespace(TCHAR_TO_UTF8(*NameSpace));
		MetricDataRequest.AddMetricData(datum);
		
		// send data without callbacks
		if (OnCloudWatchCustomMetricsSuccess.IsBound() == false || OnCloudWatchCustomMetricsFailed.IsBound() == false) {
			CloudWatchClient->PutMetricData(MetricDataRequest);
			bIsRunning = false;
			return;
		}

		// callbacks functions are present. Send Custom metrics with Callbacks handler
		Aws::CloudWatch::PutMetricDataResponseReceivedHandler Handler;
		Handler = std::bind(&UCloudWatchCustomMetricsObject::OnCustomMetricsCall, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		CloudWatchClient->PutMetricDataAsync(MetricDataRequest, Handler);
	}
	LOG_ERROR("CloudWatchClient is null. Did you call SetupClient and CreateCloudWatchCustomMetricsObject first?");
#endif
}

void UCloudWatchCustomMetricsObject::OnCustomMetricsCall(const Aws::CloudWatch::CloudWatchClient* Client, const Aws::CloudWatch::Model::PutMetricDataRequest& Request, const Aws::CloudWatch::Model::PutMetricDataOutcome& Outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& Context)
{
#if WITH_CLOUDWATCH
	if (Outcome.IsSuccess())
	{
		LOG_NORMAL("Received OnCustomMetricsCall with Success outcome.");
		OnCloudWatchCustomMetricsSuccess.ExecuteIfBound();
	}
	else
	{
		const FString MyErrorMessage = FString(Outcome.GetError().GetMessage().c_str());
		LOG_ERROR("Received Cloud Watch OnCustomMetricsCall with failed outcome. Error: " + MyErrorMessage);
		OnCloudWatchCustomMetricsFailed.ExecuteIfBound(MyErrorMessage);
	}
	bIsRunning = false;
#endif
}



#define LOCTEXT_NAMESPACE "FCloudWatchSDKModule"

void* FCloudWatchSDKModule::AWSCommonLibraryHandle = nullptr;
void* FCloudWatchSDKModule::AWSEventStreamLibraryHandle = nullptr;
void* FCloudWatchSDKModule::AWSChecksumsLibraryHandle = nullptr;
void* FCloudWatchSDKModule::AWSCoreLibraryHandle = nullptr;
void* FCloudWatchSDKModule::AWSCloudWatchLibraryHandle = nullptr;
void* FCloudWatchSDKModule::AWSLogsLibraryHandle = nullptr;

void FCloudWatchSDKModule::StartupModule()
{
#if PLATFORM_WINDOWS
    #if PLATFORM_64BITS
        #if WITH_CLOUDWATCH
			LOG_NORMAL("Starting CloudWatchSDK Module...");
            
			FString BaseDir = IPluginManager::Get().FindPlugin("CloudWatchSDK")->GetBaseDir();
            const FString SDKDir = FPaths::Combine(*BaseDir, TEXT("ThirdParty"), TEXT("CloudWatchSDK"));
			const FString LibDir = FPaths::Combine(*SDKDir, TEXT("Win64"));
			
			// aws-c-common
			const FString CommonLibName = TEXT("aws-c-common");
            if (!LoadDependency(LibDir, CommonLibName, AWSCommonLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-c-common library. Plug-in will not be functional."));
                FreeDependency(AWSCommonLibraryHandle);
            }

			// aws-c-event-stream
			const FString EventLibName = TEXT("aws-c-event-stream");
            if (!LoadDependency(LibDir, EventLibName, AWSEventStreamLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-c-event-stream library. Plug-in will not be functional."));
                FreeDependency(AWSEventStreamLibraryHandle);
            }

			// aws-checksums
			const FString ChecksumsLibName = TEXT("aws-checksums");
            if (!LoadDependency(LibDir, ChecksumsLibName, AWSChecksumsLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-checksums library. Plug-in will not be functional."));
                FreeDependency(AWSChecksumsLibraryHandle);
            }

			// aws-cpp-sdk-core
			const FString CoreLibName = TEXT("aws-cpp-sdk-core");
            if (!LoadDependency(LibDir, CoreLibName, AWSCoreLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-cpp-sdk-core library. Plug-in will not be functional."));
                FreeDependency(AWSCoreLibraryHandle);
            }

			// aws-cpp-sdk-monitoring
            const FString MonitoringLibName = TEXT("aws-cpp-sdk-monitoring");
            if (!LoadDependency(LibDir, MonitoringLibName, AWSCloudWatchLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-cpp-sdk-monitoring library. Plug-in will not be functional."));
                FreeDependency(AWSCloudWatchLibraryHandle);
            }

			// aws-cpp-sdk-logs
            const FString LogsLibName = TEXT("aws-cpp-sdk-logs");
            if (!LoadDependency(LibDir, LogsLibName, AWSLogsLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-cpp-sdk-logs library. Plug-in will not be functional."));
                FreeDependency(AWSLogsLibraryHandle);
            }
			
			Aws::InitAPI(options);
			LOG_NORMAL("Aws::InitAPI called.");
        #endif
    #endif
#endif
}

bool FCloudWatchSDKModule::LoadDependency(const FString& Dir, const FString& Name, void*& Handle)
{
    FString Lib = Name + TEXT(".") + FPlatformProcess::GetModuleExtension();
    FString Path = Dir.IsEmpty() ? *Lib : FPaths::Combine(*Dir, *Lib);

    Handle = FPlatformProcess::GetDllHandle(*Path);

    if (Handle == nullptr)
    {
        return false;
    }

    return true;
}

void FCloudWatchSDKModule::FreeDependency(void*& Handle)
{
#if !PLATFORM_LINUX
    if (Handle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(Handle);
        Handle = nullptr;
    }
#endif
}

void FCloudWatchSDKModule::ShutdownModule()
{
#if PLATFORM_WINDOWS
	#if PLATFORM_64BITS
		#if WITH_CLOUDWATCH
			LOG_NORMAL("Aws::ShutdownAPI called.");
			Aws::ShutdownAPI(options);
		#endif
	#endif
#endif
	FreeDependency(AWSCommonLibraryHandle);
	FreeDependency(AWSEventStreamLibraryHandle);
	FreeDependency(AWSChecksumsLibraryHandle);
    FreeDependency(AWSCoreLibraryHandle);
	FreeDependency(AWSCloudWatchLibraryHandle);
	FreeDependency(AWSLogsLibraryHandle);

	LOG_NORMAL("Shutting down CloudWatchSDK Module...");
}

void FCloudWatchSDKModule::SetupClient(const FString& AccessKey, const FString& Secret, const FString& Region /*= "us-east-1"*/)
{
#if WITH_CLOUDWATCH
	Aws::Client::ClientConfiguration ClientConfig;
	Aws::Auth::AWSCredentials Credentials;

	ClientConfig.connectTimeoutMs = 10000;
	ClientConfig.requestTimeoutMs = 10000;
	ClientConfig.region = TCHAR_TO_UTF8(*Region);

	Credentials = Aws::Auth::AWSCredentials(TCHAR_TO_UTF8(*AccessKey), TCHAR_TO_UTF8(*Secret));
	CloudWatchClient = new Aws::CloudWatch::CloudWatchClient(Credentials, ClientConfig);
	LogsClient = new Aws::CloudWatchLogs::CloudWatchLogsClient(Credentials, ClientConfig);
#endif
}

UCloudWatchCustomMetricsObject* FCloudWatchSDKModule::CreateCloudWatchCustomMetricsObject(const FString& NameSpace, const FString& GroupName)
{
#if WITH_CLOUDWATCH
	UCloudWatchCustomMetricsObject* Proxy = UCloudWatchCustomMetricsObject::CreateCloudWatchCustomMetrics(NameSpace, GroupName);
	Proxy->CloudWatchClient = CloudWatchClient;
	return Proxy;
#endif
	return nullptr;
}

ULogsCustomEventObject* FCloudWatchSDKModule::CreateLogsCustomEventObject(const FString& GroupName, const FString& StreamName)
{
#if WITH_CLOUDWATCH
	ULogsCustomEventObject* Proxy = ULogsCustomEventObject::CreateLogsCustomEvent(GroupName, StreamName);
	Proxy->LogsClient = LogsClient;
	return Proxy;
#endif
	return nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCloudWatchSDKModule, CloudWatchSDK)
