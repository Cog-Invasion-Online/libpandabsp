#pragma once

#include "config_bsp.h"

#if defined(BUILDING_CLIENT_DLL) || defined(BUILDING_SERVER_DLL)
#define EXPORT_GAME_SHARED __declspec(dllexport)
#else
#define EXPORT_GAME_SHARED __declspec(dllimport)
#endif

// Align value |value| by alignment in bytes |alignment_bytes|.
template <typename T>
constexpr const inline T AlignValue( const T value,
				     const size_t alignment_bytes )
{
	return (T)( ( (size_t)(value)+alignment_bytes - 1 ) & ~( alignment_bytes - 1 ) );
}