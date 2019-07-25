// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

// Silence lots of warnings when building RakNet
#if defined __clang__
#pragma clang diagnostic ignored "-Wshadow"
#elif defined _MSC_VER
#pragma warning( disable : 4456 4457 4458 )
#endif

#include "RakNet.h"

#include "Misc/SecureHash.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.