/**
 * PANDA3D BSP LIBRARY
 * Copyright (c) CIO Team. All rights reserved.
 *
 * @file shader_unlitgeneric.h
 * @author Brian Lach
 * @date December 30, 2018
 */

#ifndef SHADER_UNLITGENERIC_H
#define SHADER_UNLITGENERIC_H

#include "shader_spec.h"
#include "shader_features.h"

class ULGConfig : public ShaderConfig
{
public:
        virtual void parse_from_material_keyvalues( const BSPMaterial *mat );

        BaseTextureFeature basetexture;
        AlphaFeature alpha;
};

/**
 * Shader that only supports a basetexture, no fancy lighting effects or anything.
 * Could be used for UI elements, emissive materials, particles, etc.
 */
class UnlitGenericSpec : public ShaderSpec
{
PUBLISHED:
        UnlitGenericSpec();

public:
        virtual ShaderPermutations setup_permutations( const BSPMaterial *mat,
                                                       const RenderState *rs,
                                                       const GeomVertexAnimationSpec &anim,
                                                       BSPShaderGenerator *generator );
        virtual PT( ShaderConfig ) make_new_config();
};

#endif