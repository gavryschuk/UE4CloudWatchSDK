#pragma once

#define CURRENT_CLASS								(FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))) )
#define CURRENT_CLASS_FUNCTION							(FString(__FUNCTION__))
#define CURRENT_FUNCTION							(FString(__FUNCTION__).Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2 ))
#define CURRENT_LINE_NUMBER							(FString::FromInt(__LINE__))
#define CURRENT_CLASS_WITH_LINE							("(" + CURRENT_LINE_NUMBER + ") " + CURRENT_CLASS)
#define CURRENT_FUNCTION_SIGNATURE						(FString(__FUNCSIG__))

#define DEFINE_LOG(LogCategory)							DEFINE_LOG_CATEGORY_STATIC(LogCategory, All, All)

DEFINE_LOG(LogCloudWatchSDK)

#define CREATE_LOG_NORMAL(LogCategory, Param1)	   		UE_LOG(LogCategory, Log, TEXT("[%s::%s] %s"), *CURRENT_CLASS, *CURRENT_FUNCTION, *FString(Param1))
#define CREATE_LOG_VERBOSE(LogCategory, Param1)	   		UE_LOG(LogCategory, Verbose, TEXT("[%s::%s] %s"), *CURRENT_CLASS, *CURRENT_FUNCTION, *FString(Param1))
#define CREATE_LOG_VERYVERBOSE(LogCategory, Param1)		UE_LOG(LogCategory, VeryVerbose, TEXT("[%s::%s] %s"), *CURRENT_CLASS, *CURRENT_FUNCTION, *FString(Param1))
#define CREATE_LOG_WARNING(LogCategory, Param1)	   		UE_LOG(LogCategory, Warning, TEXT("[%s::%s] %s"), *CURRENT_CLASS, *CURRENT_FUNCTION, *FString(Param1))
#define CREATE_LOG_ERROR(LogCategory, Param1)	   		UE_LOG(LogCategory, Error, TEXT("[%s::%s] %s"), *CURRENT_CLASS, *CURRENT_FUNCTION, *FString(Param1))

#define LOG_NORMAL(Param1)	   							CREATE_LOG_NORMAL(LogCloudWatchSDK, Param1)
#define LOG_VERBOSE(Param1)	   							CREATE_LOG_VERBOSE(LogCloudWatchSDK, Param1)
#define LOG_VERYVERBOSE(Param1)							CREATE_LOG_VERYVERBOSE(LogCloudWatchSDK, Param1)
#define LOG_WARNING(Param1)	   							CREATE_LOG_WARNING(LogCloudWatchSDK, Param1)
#define LOG_ERROR(Param1)	   							CREATE_LOG_ERROR(LogCloudWatchSDK, Param1)
