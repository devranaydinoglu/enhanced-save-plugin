// Copyright 2023 devran. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(EssLog, Log, All);

#define ESS_LOG(Verbosity, Format, ...) \
{ \
    if (!UE_BUILD_SHIPPING) \
    { \
        UE_LOG(EssLog, Verbosity, TEXT(Format), ##__VA_ARGS__); \
    } \
}
