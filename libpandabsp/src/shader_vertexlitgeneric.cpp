/**
 * PANDA3D BSP LIBRARY
 * Copyright (c) CIO Team. All rights reserved.
 *
 * @file shader_vertexlitgeneric.cpp
 * @author Brian Lach
 * @date December 26, 2018
 */

#include "shader_vertexlitgeneric.h"

#include <auxBitplaneAttrib.h>
#include <transparencyAttrib.h>
#include <alphaTestAttrib.h>
#include <colorBlendAttrib.h>
#include <colorAttrib.h>
#include <renderState.h>
#include <materialAttrib.h>
#include <lightAttrib.h>
#include <clipPlaneAttrib.h>
#include <config_pgraphnodes.h>
#include <pointLight.h>
#include <directionalLight.h>
#include <spotlight.h>
#include <texturePool.h>

#include "bsploader.h"
#include "pssmCameraRig.h"
#include "bsp_material.h"
#include "shader_generator.h"

void VLGShaderConfig::parse_from_material_keyvalues( const BSPMaterial *mat )
{
        basetexture.parse_from_material_keyvalues( mat, this );
        alpha.parse_from_material_keyvalues( mat, this );
        arme.parse_from_material_keyvalues( mat, this );
        envmap.parse_from_material_keyvalues( mat, this );
        halflambert.parse_from_material_keyvalues( mat, this );
        detail.parse_from_material_keyvalues( mat, this );
        bumpmap.parse_from_material_keyvalues( mat, this );
        lightwarp.parse_from_material_keyvalues( mat, this );
        rimlight.parse_from_material_keyvalues( mat, this );
        selfillum.parse_from_material_keyvalues( mat, this );
}

VertexLitGenericSpec::VertexLitGenericSpec() :
        ShaderSpec( "VertexLitGeneric",
                    Filename( "phase_14/models/shaders/stdshaders/vertexLitGeneric_PBR.vert.glsl" ),
                    Filename( "phase_14/models/shaders/stdshaders/vertexLitGeneric_PBR.frag.glsl" ) )
{
}

PT( ShaderConfig ) VertexLitGenericSpec::make_new_config()
{
        return new VLGShaderConfig;
}

ShaderPermutations VertexLitGenericSpec::setup_permutations( const BSPMaterial *mat,
                                                             const RenderState *rs,
                                                             const GeomVertexAnimationSpec &anim,
                                                             BSPShaderGenerator *generator )
{
        VLGShaderConfig *conf = (VLGShaderConfig *)get_shader_config( mat );

        ShaderPermutations result = ShaderSpec::setup_permutations( mat, rs, anim, generator );

        conf->basetexture.add_permutations( result );
        conf->alpha.add_permutations( result );
        conf->bumpmap.add_permutations( result );
        conf->detail.add_permutations( result );
        conf->arme.add_permutations( result );
        conf->envmap.add_permutations( result );
        conf->halflambert.add_permutations( result );
        conf->lightwarp.add_permutations( result );
        conf->rimlight.add_permutations( result );
        conf->selfillum.add_permutations( result );

        bool disable_alpha_write = false;
        bool calc_primary_alpha = false;

        bool need_tbn = true;
        bool need_world_position = true;
        bool need_world_normal = true;
        bool need_world_vec = true;
        bool need_eye_vec = false;
        bool need_eye_position = false;
        bool need_eye_normal = false;

        // verify_enforce_attrib_lock();
        const AuxBitplaneAttrib *aux_bitplane;
        rs->get_attrib_def( aux_bitplane );
        int outputs = aux_bitplane->get_outputs();

        // Decide whether or not we need alpha testing or alpha blending.
        bool have_alpha_blend = false;
        const ColorBlendAttrib *color_blend;
        rs->get_attrib_def( color_blend );
        if ( color_blend->get_mode() != ColorBlendAttrib::M_none )
        {
                have_alpha_blend = true;
        }
        const TransparencyAttrib *transparency;
        rs->get_attrib_def( transparency );
        if ( transparency->get_mode() == TransparencyAttrib::M_alpha ||
             transparency->get_mode() == TransparencyAttrib::M_premultiplied_alpha ||
             transparency->get_mode() == TransparencyAttrib::M_dual ||
             conf->alpha.has_feature )
        {
                have_alpha_blend = true;
        }

        // Decide what to send to the framebuffer alpha, if anything.
        if ( outputs & AuxBitplaneAttrib::ABO_glow )
        {
                if ( have_alpha_blend )
                {
                        outputs &= ~AuxBitplaneAttrib::ABO_glow;
                        disable_alpha_write = true;
                        result.add_flag( ShaderAttrib::F_disable_alpha_write );
                }
        }

	add_alpha_test( rs, result );

        if ( outputs & AuxBitplaneAttrib::ABO_aux_normal )
        {
                need_eye_normal = true;
                result.add_permutation( "HAVE_AUX_NORMAL" );
        }

        add_color( rs, result );

        BSPLoader *bsploader = BSPLoader::get_global_ptr();

        if ( !bsploader->has_active_level() )
        {
                // Break out the lights by type.
                const LightAttrib *la;
                rs->get_attrib_def( la );
                size_t num_lights = la->get_num_on_lights();
                if ( num_lights > 0 )
                {
			need_world_vec = true;
			need_world_normal = true;

                        result.add_permutation( "LIGHTING" );

                        std::stringstream ss;
                        ss << num_lights;
                        result.permutations["NUM_LIGHTS"] = ss.str();
                }
        }
        else
        {
                const LightAttrib *la;
                rs->get_attrib_def( la );

                // Make sure there is no setLightOff()
                if ( !la->has_all_off() )
                {
			need_world_vec = true;
                        need_world_normal = true; // for ambient cube

                        result.add_permutation( "LIGHTING" );
                        result.add_permutation( "BSP_LIGHTING" );
                        stringstream nlss;
                        nlss << MAX_TOTAL_LIGHTS;
                        result.add_permutation( "NUM_LIGHTS", nlss.str() );
                }

        }

        if ( add_csm( rs, result, generator ) )
        {
                need_world_normal = true;
                need_world_position = true;
        }

        const LightRampAttrib *lra;
        rs->get_attrib_def( lra );
        if ( lra->get_mode() != LightRampAttrib::LRT_default )
        {
                result.add_permutation( "HDR" );
        }

        const TextureAttrib *texture;
        rs->get_attrib_def( texture );

        size_t num_textures = texture->get_num_on_stages();
        for ( size_t i = 0; i < num_textures; ++i )
        {
                TextureStage *stage = texture->get_on_stage( i );
                Texture *tex = texture->get_on_texture( stage );
                nassertd( tex != nullptr ) continue;

                // Mark this TextureStage as having been used by the shader generator, so
                // that the next time its properties change, it will cause the state to be
                // rehashed to ensure that the shader is regenerated if needed.
                stage->mark_used_by_auto_shader();

                if ( stage->get_name() == "cubemap_tex" &&
                     !conf->envmap.envmap_texture &&
                     conf->envmap.has_feature)
                {
                        // env_cubemap (or something else) supplied envmap
                        result.add_input( ShaderInput( "envmapSampler", tex ) );
                }
        }

        if ( add_clip_planes( rs, result ) )
        {
                need_eye_position = true;
        }

	if ( add_fog( rs, result, generator ) )
	{
		need_eye_position = true;
	}

        add_hw_skinning( anim, result );

        if ( need_world_vec )
                need_world_position = true;

        if ( need_tbn )
        {
                result.add_permutation( "NEED_TBN" );
        }

        if ( need_eye_normal )
        {
                result.add_permutation( "NEED_EYE_NORMAL" );
        }
        if ( need_world_normal )
        {
                result.add_permutation( "NEED_WORLD_NORMAL" );
        }
        if ( need_world_position )
        {
                result.add_permutation( "NEED_WORLD_POSITION" );
        }
        if ( need_eye_position )
        {
                result.add_permutation( "NEED_EYE_POSITION" );
        }
        if ( need_eye_vec )
                result.add_permutation( "NEED_EYE_VEC" );
        if ( need_world_vec )
                result.add_permutation( "NEED_WORLD_VEC" );

        // Done!
        return result;
}

void VertexLitGenericSpec::add_precache_combos( ShaderPrecacheCombos &combos )
{
	ShaderSpec::add_precache_combos( combos );

	combos.add_bool( "NEED_WORLD_POSITION", true );
	combos.add_bool( "NEED_EYE_POSITION", true );
	combos.add_bool( "NEED_EYE_VEC", true );
	combos.add_bool( "NEED_WORLD_VEC", true );
	combos.add_bool( "NEED_WORLD_NORMAL", true );
	combos.add_bool( "NEED_EYE_NORMAL", true );
	combos.add_bool( "NEED_TBN", true );
	combos.add_bool( "HDR", true );
	combos.add_bool( "LIGHTING" );
	combos.add_bool( "BSP_LIGHTING", true );
	//combos.add_bool( "CALC_PRIMARY_ALPHA" );
	//combos.add_bool( "HAVE_AUX_NORMAL" );
	combos.add_bool( "BASETEXTURE" );
	combos.add_bool( "BUMPMAP" );
	combos.add_bool( "RIMLIGHT" );
	combos.add_bool( "ARME" );
	combos.add_bool( "LIGHTWARP" );
	combos.add_bool( "HALFLAMBERT" );
	combos.add_bool( "SELFILLUM" );
	combos.add_bool( "ENVMAP" );
	combos.add_bool( "HARDWARE_SKINNING", true );
	combos.add_bool( "INDEXED_TRANSFORMS" );
	combos.add_bool( "NEED_COLOR" );
	combos.add_bool( "COLOR_VERTEX" );
	combos.add_bool( "COLOR_FLAT" );

	combos.add( "NUM_LIGHTS", 8, 8 );
}