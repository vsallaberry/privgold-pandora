(lldb) target create "vegastrike_priv-1.03/build-cmake/vegastrike"
Current executable set to 'vegastrike_priv-1.03/build-cmake/vegastrike' (x86_64).
(lldb) run -D/Users/vincent/perso/prog/ext/privgold-work/data.1.03
Process 6927 launched: '/Users/vincent/perso/prog/ext/privgold-work/vegastrike_priv-1.03/build-cmake/vegastrike' (x86_64)
bt
 In path /Users/vincent/perso/prog/ext/privgold-work/vegastrike_priv-1.03/build-cmake
Vega Strike  
See http://www.gnu.org/copyleft/gpl.html for license details.

ARG #1 = -D/Users/vincent/perso/prog/ext/privgold-work/data.1.03
Using data dir specified on command line : /Users/vincent/perso/prog/ext/privgold-work/data.1.03
GOT SUBDIR ARG = 
Using .privgold100 as the home directory
Found MODDIR = /Users/vincent/perso/prog/ext/privgold-work/data.1.03/mods
USING HOMEDIR : /Users/vincent/.privgold100 As the home directory 
CONFIGFILE - Found a config file in home directory, using : /Users/vincent/.privgold100/vegastrike.config
DATADIR - No datadir specified in config file, using ; /Users/vincent/perso/prog/ext/privgold-work/data.1.03
SIMULATION_ATOM: 0.06
MISSION_NAME is empty using : main_menu.mission
running import sys
print sys.path
sys.path = [r"/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/builtin",r"/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/quests",r"/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/missions",r"/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/ai",r"/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules",r"/Users/vincent/perso/prog/ext/privgold-work/data.1.03/bases"]
['/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python27.zip', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-darwin', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-mac', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-mac/lib-scriptpackages', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-tk', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-old', '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-dynload', '/Library/Python/2.7/site-packages', '/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python', '/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python/PyObjC']
testing VS randomrunning import sys
print sys.path
['/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/builtin', '/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/quests', '/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/missions', '/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/ai', '/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules', '/Users/vincent/perso/prog/ext/privgold-work/data.1.03/bases']
2021-09-04 16:29:53.085234+0200 vegastrike[6927:909104]   no saved enable hardware sample rate converter preference found
Game Mode Params 1024x1024 at depth 32 @ 0 Hz
2021-09-04 16:29:55.389545+0200 vegastrike[6927:909121] MessageTracer: Falling back to default whitelist
Game Mode Params 1024x1024 at depth 32 @ 0 Hz
OpenGL Extensions supported: GL_ARB_color_buffer_float GL_ARB_depth_buffer_float GL_ARB_depth_clamp GL_ARB_depth_texture GL_ARB_draw_buffers GL_ARB_draw_elements_base_vertex GL_ARB_draw_instanced GL_ARB_fragment_program GL_ARB_fragment_program_shadow GL_ARB_fragment_shader GL_ARB_framebuffer_object GL_ARB_framebuffer_sRGB GL_ARB_half_float_pixel GL_ARB_half_float_vertex GL_ARB_imaging GL_ARB_instanced_arrays GL_ARB_multisample GL_ARB_multitexture GL_ARB_occlusion_query GL_ARB_pixel_buffer_object GL_ARB_point_parameters GL_ARB_point_sprite GL_ARB_provoking_vertex GL_ARB_seamless_cube_map GL_ARB_shader_objects GL_ARB_shader_texture_lod GL_ARB_shading_language_100 GL_ARB_shadow GL_ARB_sync GL_ARB_texture_border_clamp GL_ARB_texture_compression GL_ARB_texture_compression_rgtc GL_ARB_texture_cube_map GL_ARB_texture_env_add GL_ARB_texture_env_combine GL_ARB_texture_env_crossbar GL_ARB_texture_env_dot3 GL_ARB_texture_float GL_ARB_texture_mirrored_repeat GL_ARB_texture_non_power_of_two GL_ARB_texture_rectangle GL_ARB_texture_rg GL_ARB_transpose_matrix GL_ARB_vertex_array_bgra GL_ARB_vertex_blend GL_ARB_vertex_buffer_object GL_ARB_vertex_program GL_ARB_vertex_shader GL_ARB_window_pos GL_EXT_abgr GL_EXT_bgra GL_EXT_bindable_uniform GL_EXT_blend_color GL_EXT_blend_equation_separate GL_EXT_blend_func_separate GL_EXT_blend_minmax GL_EXT_blend_subtract GL_EXT_clip_volume_hint GL_EXT_debug_label GL_EXT_debug_marker GL_EXT_depth_bounds_test GL_EXT_draw_buffers2 GL_EXT_draw_range_elements GL_EXT_fog_coord GL_EXT_framebuffer_blit GL_EXT_framebuffer_multisample GL_EXT_framebuffer_multisample_blit_scaled GL_EXT_framebuffer_object GL_EXT_framebuffer_sRGB GL_EXT_geometry_shader4 GL_EXT_gpu_program_parameters GL_EXT_gpu_shader4 GL_EXT_multi_draw_arrays GL_EXT_packed_depth_stencil GL_EXT_packed_float GL_EXT_provoking_vertex GL_EXT_rescale_normal GL_EXT_secondary_color GL_EXT_separate_specular_color GL_EXT_shadow_funcs GL_EXT_stencil_two_side GL_EXT_stencil_wrap GL_EXT_texture_array GL_EXT_texture_compression_dxt1 GL_EXT_texture_compression_s3tc GL_EXT_texture_env_add GL_EXT_texture_filter_anisotropic GL_EXT_texture_integer GL_EXT_texture_lod_bias GL_EXT_texture_mirror_clamp GL_EXT_texture_rectangle GL_EXT_texture_shared_exponent GL_EXT_texture_sRGB GL_EXT_texture_sRGB_decode GL_EXT_timer_query GL_EXT_transform_feedback GL_EXT_vertex_array_bgra GL_APPLE_aux_depth_stencil GL_APPLE_client_storage GL_APPLE_element_array GL_APPLE_fence GL_APPLE_float_pixels GL_APPLE_flush_buffer_range GL_APPLE_flush_render GL_APPLE_object_purgeable GL_APPLE_packed_pixels GL_APPLE_pixel_buffer GL_APPLE_rgb_422 GL_APPLE_row_bytes GL_APPLE_specular_vector GL_APPLE_texture_range GL_APPLE_transform_hint GL_APPLE_vertex_array_object GL_APPLE_vertex_array_range GL_APPLE_vertex_point_size GL_APPLE_vertex_program_evaluators GL_APPLE_ycbcr_422 GL_ATI_separate_stencil GL_ATI_texture_env_combine3 GL_ATI_texture_float GL_ATI_texture_mirror_once GL_IBM_rasterpos_clip GL_NV_blend_square GL_NV_conditional_render GL_NV_depth_clamp GL_NV_fog_distance GL_NV_fragment_program_option GL_NV_fragment_program2 GL_NV_light_max_exponent GL_NV_multisample_filter_hint GL_NV_point_sprite GL_NV_texgen_reflection GL_NV_texture_barrier GL_NV_vertex_program2_option GL_NV_vertex_program3 GL_SGIS_generate_mipmap GL_SGIS_texture_edge_clamp GL_SGIS_texture_lod 
OpenGL::Accurate Fog Distance supported
OpenGL::Generic Texture Compression supported
OpenGL::S3TC Texture Compression supported
OpenGL::Multitexture supported (8 units)
OpenGL::TextureCubeMapExt supported
OpenGL::S3TC Texture Clamp-to-Edge supported
OpenGL::S3TC Texture Clamp-to-Border supported
OpenGL::EXTColorTable unsupported
0 joysticks were found.

The names of the joysticks are:
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1x1 texture, page 0 (eff: 1x1 - limited at 65536)
FactionXML:LoadXML factions.xml
Transferring 2x2 texture, page 0 (eff: 2x2 - limited at 65536)
Transferring 2x2 texture, page 0 (eff: 2x2 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Reshaping 1024 7682021-09-04 16:29:57.200785+0200 vegastrike[6927:909104] SecTaskLoadEntitlements failed error=22 cs_flags=20, pid=6927
2021-09-04 16:29:57.200875+0200 vegastrike[6927:909104] SecTaskCopyDebugDescription: vegastrike[6927]/0#-1 LF=0
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Compiling python module modules/dj.py
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Contents of star system:
<system name="Empty" background="backgrounds/black" nearstars="0" stars="0" starspread="0"  y="0" z="0" x="0">
</system>

Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Min (0.000000, 0.000000, 0.000000) Max(0.000000, 0.000000, 0.000000) MinLumin 1.000000, MaxLumin 1.000000Read In Star Count 0 used: 1000
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Min (0.000000, 0.000000, 0.000000) Max(0.000000, 0.000000, 0.000000) MinLumin 1.000000, MaxLumin 1.000000Read In Star Count 0 used: 104
Loading a starsystem
Loading Star System Special/Empty
No such special key: keypad-enter
No such special key: keypad-minus
No such special key: keypad-plus
No such special key: keypad-divide
No such special key: keypad-multiply
No such special key: keypad-8
No such special key: keypad-2
No such special key: keypad-5
No such special key: keypad-4
No such special key: keypad-6
No such special key: pause
No such special key: keypad-3
No such special key: keypad-9
No such special key: keypad-7
No such special key: keypad-0
No such special key: keypad-1
FOUND MODIFICATION = player FOR PLAYER #0
CREATING A LOCAL SHIP : dumbfire
Hi helper play 0
HereInitializing optimizer
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1x1 texture, page 0 (eff: 1x1 - limited at 65536)
Transferring 1x1 texture, page 0 (eff: 1x1 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
pox 18068.451000 -60.189000 -371.339000
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)

ERROR: NULL Unit used in Python script; returning default value...
ERROR: NULL Unit used in Python script; returning default value...Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
 +++ m/random_encounters.py:279: done loading rand enc
 +++ m/random_encounters.py:45: init random enc
 +++ m/random_encounters.py:61: end random enc
Force feedback support disabled when compiled
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Unit file contraband not found
Error picking comm animation for 25 faction with bas:0 dock:0
Unit file contraband not found
Error picking comm animation for 26 faction with bas:0 dock:0
Unit file contraband not found
Error picking comm animation for 27 faction with bas:0 dock:0
Unit file contraband not found
Error picking comm animation for 28 faction with bas:0 dock:0
Unit file contraband not found
Error picking comm animation for 29 faction with bas:0 dock:0
Loading completed, now network init
Loading active missions True
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
SITUATION IS 1force change 1 bool 1
SITUATION IS RESET TO 1
peaCce
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
 +++ m/random_encounters.py:26: init playerdata
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Unit file upgrading_dummy_unit not found
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 256x64 texture, page 0 (eff: 256x64 - limited at 65536)
Transferring 256x64 texture, page 0 (eff: 256x64 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 32x128 texture, page 0 (eff: 32x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 16x16 texture, page 0 (eff: 16x16 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 16x16 texture, page 0 (eff: 16x16 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 16x16 texture, page 0 (eff: 16x16 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x512 texture, page 0 (eff: 256x512 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 256x128 texture, page 0 (eff: 256x128 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 8x8 texture, page 0 (eff: 8x8 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Unit file cargobrick not found
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
 +++ m/random_encounters.py:42: done playerdat
Launching bases for Special/Empty
conditioning
nonzeroing
finding quest
quest_drone
 +++ m/random_encounters.py:97: resetting sigdist=3000.000000 detdist=5000.000000
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
UNIT HAS DIED: dumbfire dumbfire (file dumbfire)
Found ship named : tarsus.begin
Faction not found assigning default one : privateer !!!
	Exiting ReadSavedPackets
Contents of star system:
<system name="Troy" background="backgrounds/plasma_galaxy" nearstars="0" stars="0" starspread="0"  y="0" z="0" x="0">
<Light>
<ambient red=".25" green=".25" blue=".25"/>
<diffuse red="1" green="1" blue="1"/>
<specular red="1" green="1" blue="1"/>
</Light>

<Planet name="Troy" file="stars/sun.png" radius="4000000" y="-14594814.000000" z="00.00" x="14594814.000000"  Red="0.6" Green="0.6" Blue="1" ReflectNoLight="true" light="0"/>

<Light>
<ambient red="000.0" green="000.0" blue="000.0"/>
<diffuse red="1" green="1" blue="1"/>
<specular red="1" green="1" blue="1"/>
</Light>

<Light>
<ambient red="1" green=".1" blue="0"/>
<!--<diffuse red="1" green=".7" blue="0"/>-->
<!--<attenuated red=".00000001" blue=".00000015"/>-->
<attenuated red="0" blue=".000000002"/>
</Light>	
<Light>
<ambient red="1" green=".1" blue="0"/>
<!--<diffuse red="1" green=".7" blue="0"/>-->
<!--<attenuated red=".00000001" blue=".00000015"/>-->
<attenuated red="0" blue=".0000001"/>
</Light>	
        <planet name="Jump_To_Pyrenees" file="jump.ani" alpha="ONE ONE" radius="256" gravity="0" x="8000" y="14500" day="240" destination="Gemini/Pyrenees"/>
        <unit name= "Achilles"  file="mining_base" faction="mining" x="17500" y="000"/>
        <planet name="Jump_To_Pender's_Star" file="jump.ani" alpha="ONE ONE" radius="256" gravity="0" x="8200" y="-14000" day="240" destination="Gemini/Penders_Star"/>
        <unit name= "Hector"  file="mining_base" faction="mining" x="-14500" y="-22000"/>

        <planet name="Jump_To_Regallis" file="jump.ani" alpha="ONE ONE" radius="256" gravity="0" x="-13000" y="-7500" day="240" destination="Gemini/Regallis"/>

        <planet name="Helen" file="planets/agricultural.png" radius="1200" gravity="20" x="-14000" y="10500" day="300">
		<Atmosphere file="clouds.png" alpha="SRCALPHA INVSRCALPHA" radius="1211"/>
<!--	<Fog>
		<FogElement file="atmXatm.bfxm" ScaleAtmosphereHeight="1.0"  red="0.900000" blue="0.900000" green="1.000000" alpha="1.000000" dired="0.900000" diblue="0.900000" digreen="1.000000" dialpha="1.000000" concavity=".3" focus=".6" minalpha="0" maxalpha="0.7"/>
		<FogElement file="atmXhalo.bfxm" ScaleAtmosphereHeight="1.0"  red="0.900000" blue="0.900000" green="1.000000" alpha="1.000000" dired="0.900000" diblue="0.900000" digreen="1.000000" dialpha="1.000000" concavity="1" focus=".6" minalpha="0" maxalpha="0.7"/>
	</Fog> -->
        </planet>
        <planet name="Jump_To_War" file="jump.ani" alpha="ONE ONE" radius="256" gravity="0" x="10" y="16000" day="240" destination="Gemini/War"/>
        <planet name="Nav_8" file="invisible.png" alpha="ONE ONE" radius="256" gravity="0" x="4000" y="4000" day="240" />
</system>

Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
UNIT HAS DIED: sun sun (file sun)
Unit file .stable not found
Unit file .neutral.stable not found
UNIT HAS DIED: LOAD_FAILED .stable (file .stable)
UNIT HAS DIED: LOAD_FAILED .neutral.stable (file .neutral.stable)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
UNIT HAS DIED: jump Jump (file jump)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 8x8 texture, page 0 (eff: 8x8 - limited at 65536)
Transferring 1x1 texture, page 0 (eff: 1x1 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Transferring 32x32 texture, page 0 (eff: 32x32 - limited at 65536)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Unit file .stable not found
Unit file .neutral.stable not found
UNIT HAS DIED: LOAD_FAILED .stable (file .stable)
UNIT HAS DIED: LOAD_FAILED .neutral.stable (file .neutral.stable)
UNIT HAS DIED: jump Jump (file jump)
Unit file .stable not found
Unit file .neutral.stable not found
UNIT HAS DIED: LOAD_FAILED .stable (file .stable)
UNIT HAS DIED: LOAD_FAILED .neutral.stable (file .neutral.stable)
UNIT HAS DIED: jump Jump (file jump)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
UNIT HAS DIED: agricultural m_class (file agricultural)
Transferring 1024x512 texture, page 0 (eff: 1024x512 - limited at 65536)
Unit file .stable not found
Unit file .neutral.stable not found
UNIT HAS DIED: LOAD_FAILED .stable (file .stable)
UNIT HAS DIED: LOAD_FAILED .neutral.stable (file .neutral.stable)
UNIT HAS DIED: jump Jump (file jump)
Unit file invisible not found
UNIT HAS DIED: LOAD_FAILED invisible (file invisible)
Transferring 512x512 texture, page 0 (eff: 512x512 - limited at 65536)
Min (0.000000, 0.000000, 0.000000) Max(0.000000, 0.000000, 0.000000) MinLumin 1.000000, MaxLumin 1.000000Read In Star Count 0 used: 1000
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Min (0.000000, 0.000000, 0.000000) Max(0.000000, 0.000000, 0.000000) MinLumin 1.000000, MaxLumin 1.000000Read In Star Count 0 used: 104
Loading a starsystem
Loading Star System Gemini/Troy
 Next To: Gemini/Regallis
 Next To: Gemini/Freyja
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 128x128 texture, page 0 (eff: 128x128 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
ToggleWeaponSet: true
0:Laser 
CURRENT: 0:Laser 
ACTIVE: 0:Laser 
ToggleWeapon end...
ToggleWeaponSet: false
0:Laser 
CURRENT: 0:Laser 
ACTIVE: 0:Laser 
ToggleWeapon end...
ToggleWeaponSet: true
0:Laser 
CURRENT: 0:Laser 
ACTIVE: 0:Laser 
ToggleWeapon end...
ToggleWeaponSet: false
0:Laser 
CURRENT: 0:Laser 
ACTIVE: 0:Laser 
ToggleWeapon end...
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 128x64 texture, page 0 (eff: 128x64 - limited at 65536)
Transferring 64x128 texture, page 0 (eff: 64x128 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 64x64 texture, page 0 (eff: 64x64 - limited at 65536)
Transferring 128x16 texture, page 0 (eff: 128x16 - limited at 65536)
Transferring 16x128 texture, page 0 (eff: 16x128 - limited at 65536)
Transferring 32x128 texture, page 0 (eff: 32x128 - limited at 65536)
Transferring 256x32 texture, page 0 (eff: 256x32 - limited at 65536)
Loading active missions True
Transferring 256x256 texture, page 0 (eff: 256x256 - limited at 65536)
 +++ m/generate_dyn_universe.py:245: Purging...
 +++ m/generate_dyn_universe.py:249: StartSystemCount
 +++ m/generate_dyn_universe.py:211: Getting reachable systems...
 +++ m/generate_dyn_universe.py:213: done
 +++ m/generate_dyn_universe.py:251: {'pirates': 8, 'refinery': 0, 'planets': 0, 'upgrades': 0, 'miggs': 0, 'reismann': 0, 'kroiz': 0, 'kilrathi': 338, 'unknown': 18, 'toth': 0, 'retro': 0, 'landreich': 14, 'seelig': 0, 'merchant': 5, 'privateer': 0, 'mining': 0, 'border_worlds': 49, 'AWACS': 0, 'hunter': 1, 'nephilim': 0, 'riordian': 0, 'neutral': 0, 'confed': 350, 'commerce': 0, 'naval': 0, 'garrovick': 0, None: 803, 'pirate': 0, 'militia': 18, 'steltek': 0, 'firekkan': 2}
 +++ m/generate_dyn_universe.py:252: EndSystemCount
 +++ m/generate_dyn_universe.py:260: reflist is ['neutral', 'confed', 'kilrathi', 'nephilim', 'merchant', 'retro', 'pirates', 'hunter', 'militia', 'unknown', 'landreich', 'border_worlds', 'firekkan', 'privateer', 'steltek', 'upgrades', 'planets', 'riordian', 'seelig', 'kroiz', 'miggs', 'toth', 'garrovick', 'reismann', 'AWACS']
 +++ m/generate_dyn_universe.py:261: curfaclist is ['neutral', 'confed', 'kilrathi', 'nephilim', 'merchant', 'retro', 'pirates', 'hunter', 'militia', 'unknown', 'landreich', 'border_worlds', 'firekkan', 'privateer', 'steltek', 'upgrades', 'planets', 'riordian', 'seelig', 'kroiz', 'miggs', 'toth', 'garrovick', 'reismann', 'AWACS', 'mining', 'refinery', 'naval', 'commerce', 'pirate']
 *** Python Warning 1!
Warning Traceback 1:
  File "m/privateer.py", line 30, in Execute
    i.Execute()
  File "m/random_encounters.py", line 249, in Execute
    generate_dyn_universe.KeepUniverseGenerated()
  File "m/generate_dyn_universe.py", line 306, in KeepUniverseGenerated
    ReloadUniverse()
  File "m/generate_dyn_universe.py", line 265, in ReloadUniverse
    debug.warn('save using legacy FG format... resetting universe to reformat')
Message: save using legacy FG format... resetting universe to reformat

 +++ m/generate_dyn_universe.py:270: generating ships... ... ...
 +++ m/fg_util.py:61: reading base names confed
 +++ m/fg_util.py:61: reading base names kilrathi
 +++ m/fg_util.py:61: reading base names nephilim
 +++ m/fg_util.py:61: reading base names merchant
 +++ m/fg_util.py:61: reading base names retro
 +++ m/fg_util.py:61: reading base names pirates
 +++ m/fg_util.py:61: reading base names hunter
 +++ m/fg_util.py:61: reading base names militia
 +++ m/fg_util.py:61: reading base names unknown
 +++ m/fg_util.py:61: reading base names landreich
 +++ m/fg_util.py:61: reading base names border_worlds
 +++ m/fg_util.py:61: reading base names firekkan
 +++ m/fg_util.py:61: reading base names AWACS
 +++ m/generate_dyn_universe.py:272: placing ships... ... ...
 +++ m/generate_dyn_universe.py:211: Getting reachable systems...
 +++ m/generate_dyn_universe.py:213: done
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for border_worlds
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for border_worlds
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for border_worlds
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for border_worlds
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for landreich
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('kamekh', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('kamekh', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('kamekh', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:71: Generating capital (('drayman', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:71: Generating capital (('kamekh', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:71: Generating capital (('paradigm', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for hunter
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for retro
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:71: Generating capital (('kamekh', 1),)
 +++ m/generate_dyn_universe.py:132: Loading FG names for merchant
 +++ m/generate_dyn_universe.py:132: Loading FG names for kilrathi
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for confed
 +++ m/generate_dyn_universe.py:132: Loading FG names for pirates
 +++ m/generate_dyn_universe.py:132: Loading FG names for militia
Launching bases for Gemini/Troy
 +++ m/universe.py:135: Found sig unit: Nav_8 (invisible)
 +++ m/universe.py:135: Found sig unit: Jump_To_War (jump)
 +++ m/universe.py:135: Found sig unit: Helen (agricultural)
 +++ m/universe.py:135: Found sig unit: Jump_To_Regallis (jump)
 +++ m/universe.py:135: Found sig unit: mining_base (Hector)
 +++ m/universe.py:135: Found sig unit: Jump_To_Pender's_Star (jump)
 +++ m/universe.py:135: Found sig unit: mining_base (Achilles)
 +++ m/universe.py:135: Found sig unit: Jump_To_Pyrenees (jump)
conditioning
nonzeroing
finding quest
quest_drone
 +++ m/random_encounters.py:97: resetting sigdist=3000.000000 detdist=5000.000000
dot -0.000490PITTER PATTER
Importing list from: universe/companies.txt
Traceback (most recent call last):
  File "mining_basesunsetmining.py", line 16, in <module>
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/dynamic_mission.py", line 978, in CreateMissions
    contractMissionsFor(basefac,baseship,minsys,maxsys)
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/dynamic_mission.py", line 870, in contractMissionsFor
    generatePatrolMission(j,numPatrolPoints(j[-1]),faction_ships.get_enemy_of(fac))
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/dynamic_mission.py", line 394, in generatePatrolMission
    randCompany = GetRandomCompanyName()
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/dynamic_mission.py", line 113, in GetRandomCompanyName
    return GetRandomFromList(company_names)
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/dynamic_mission.py", line 104, in GetRandomFromList
    idx = vsrandom.randint(0,len(list)-1)
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/vsrandom.py", line 323, in randint
    return self.randrange(a, b+1)
  File "/Users/vincent/perso/prog/ext/privgold-work/data.1.03/modules/vsrandom.py", line 302, in randrange
    raise ValueError, "empty range for randrange()"
ValueError: empty range for randrange()
dot -0.000490ERROR: there are no rooms in basefile "mining_basesunset.py" ...
curmodechange 1
 +++ m/random_encounters.py:270: launch near
 +++ m/random_encounters.py:133: hola!
Transferring 1024x1024 texture, page 0 (eff: 1024x1024 - limited at 65536)
Transferring 2x2 texture, page 0 (eff: 2x2 - limited at 65536)
bt
Process 6927 exited with status = 0 (0x00000000) 
