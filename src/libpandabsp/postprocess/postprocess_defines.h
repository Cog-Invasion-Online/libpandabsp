/**
 * PANDA3D BSP LIBRARY
 * 
 * Copyright (c) Brian Lach <brianlach72@gmail.com>
 * All rights reserved.
 *
 * @file postprocess_defines.h
 * @author Brian Lach
 * @date July 22, 2019
 */

#ifndef POSTPROCESS_DEFINES_H
#define POSTPROCESS_DEFINES_H

#include <dtoolbase.h>

BEGIN_PUBLISH
enum PassTextureBits
{
	bits_PASSTEXTURE_COLOR = 1 << 0,
	bits_PASSTEXTURE_DEPTH = 1 << 1,
	bits_PASSTEXTURE_AUX0 = 1 << 2,
	bits_PASSTEXTURE_AUX1 = 1 << 3,
	bits_PASSTEXTURE_AUX2 = 1 << 4,
	bits_PASSTEXTURE_AUX3 = 1 << 5,
};
END_PUBLISH

#endif // POSTPROCESS_DEFINES_H
