visual motion areas localizer experiment stimuli

These scripts use a converted example from PTB-3 to play frames 
from motion area localizer movie frames provided by Koen Nelissen. They depend
on PTB-3 [1]. The original script was PlayMoviesWithoutGapDemoOSX() and,
at least initially, the changes are quite minor. Some masks will be added,
and provision will be made to blank the stimuli using a digital line. The
blanking will depend on the DAQ toolbox from PTB-3. 

So far there will be 4 flow field experiments. 

	AB repeating design with 12s blocks
	static vs. expansion/contraction
	static vs. rotation ( cw and ccw )
	static vs. translation, ( 4 directions: up/down, left/right, and the two diagonals )
	
	ABAC repeating with 8 or 16 second blocks
	static vs. slow vs. fast translation ( 4 directions: up/down, left/right, and the two diagonals )

The main script is PlayMovies(). By default it will play all movies (*.mov) 
in the current directory. Optionally you can give it a directory as the first
argument. In that case it will play all the movies in that directory. The movies
are played in the order they are returned by 'dir'. Basically alpha sorted. 
The basic idea is to put all of your movies in one directory then make directories 
for each experiment and put soft links (ln -s) in the experiment directories. 
for example in the directory 'test':

    dlpfc:test mbolding$ ls -l
    lrwxr-xr-x  1 mbolding  staff  31 Feb 18 16:40 1.mov -> ../movies/flow_static_30fps.mov
    lrwxr-xr-x  1 mbolding  staff  32 Feb 18 16:40 2.mov -> ../movies/flow_staticr_30fps.mov
    lrwxr-xr-x  1 mbolding  staff  28 Feb 18 16:40 3.mov -> ../movies/flow_exp_30fps.mov
    lrwxr-xr-x  1 mbolding  staff  33 Feb 18 16:40 4.mov -> ../movies/flow_contract_30fps.mov

Then run PlayMovies('test') from the parent directory. Hit the spacebar when 
the 'ready' prompt appears. The movies are played forever or until the optional
iteration count is reached.


==================================
	
example output of LoadMovieIntoTexturesDemoOSX on macbook pro 2.4GHz core 2 duo:
    >> PlayMTloc('flow_exp.mov',0,10000,1)
    Loading movie flow_exp.mov ...

    PTB-INFO: This is the OpenGL-Psychtoolbox for Apple OS X, version 3.0.8. (Build date: Jan  8 2009)
    PTB-INFO: Type 'PsychtoolboxVersion' for more detailed version information.
    PTB-INFO: Psychtoolbox is licensed to you under terms of the GNU General Public License (GPL). See file 'License.txt' in the
    PTB-INFO: Psychtoolbox root folder for a copy of the GPL license.

    OpenGL-Extensions are: GL_ARB_transpose_matrix GL_ARB_vertex_program GL_ARB_vertex_blend GL_ARB_window_pos GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_shading_language_100 GL_EXT_multi_draw_arrays GL_EXT_clip_volume_hint GL_EXT_rescale_normal GL_EXT_draw_range_elements GL_EXT_fog_coord GL_EXT_gpu_program_parameters GL_EXT_geometry_shader4 GL_EXT_transform_feedback GL_APPLE_client_storage GL_APPLE_specular_vector GL_APPLE_transform_hint GL_APPLE_packed_pixels GL_APPLE_fence GL_APPLE_vertex_array_object GL_APPLE_vertex_program_evaluators GL_APPLE_element_array GL_APPLE_flush_render GL_APPLE_aux_depth_stencil GL_NV_texgen_reflection GL_NV_light_max_exponent GL_IBM_rasterpos_clip GL_SGIS_generate_mipmap GL_ARB_imaging GL_ARB_point_parameters GL_ARB_texture_env_crossbar GL_ARB_texture_border_clamp GL_ARB_multitexture GL_ARB_texture_env_add GL_ARB_texture_cube_map GL_ARB_texture_env_dot3 GL_ARB_multisample GL_ARB_texture_env_combine GL_ARB_texture_compression GL_ARB_texture_mirrored_repeat GL_ARB_shadow GL_ARB_depth_texture GL_ARB_fragment_program GL_ARB_fragment_program_shadow GL_ARB_fragment_shader GL_ARB_occlusion_query GL_ARB_point_sprite GL_ARB_texture_non_power_of_two GL_ARB_vertex_buffer_object GL_ARB_pixel_buffer_object GL_ARB_draw_buffers GL_ARB_shader_texture_lod GL_EXT_compiled_vertex_array GL_EXT_framebuffer_object GL_EXT_framebuffer_blit GL_EXT_framebuffer_multisample GL_EXT_texture_rectangle GL_ARB_texture_rectangle GL_EXT_texture_env_add GL_EXT_blend_color GL_EXT_blend_minmax GL_EXT_blend_subtract GL_EXT_texture_lod_bias GL_EXT_abgr GL_EXT_bgra GL_EXT_stencil_wrap GL_EXT_texture_filter_anisotropic GL_EXT_secondary_color GL_EXT_blend_func_separate GL_EXT_shadow_funcs GL_EXT_stencil_two_side GL_EXT_depth_bounds_test GL_EXT_texture_compression_s3tc GL_EXT_texture_compression_dxt1 GL_EXT_texture_sRGB GL_EXT_blend_equation_separate GL_EXT_texture_mirror_clamp GL_EXT_packed_depth_stencil GL_EXT_bindable_uniform GL_EXT_texture_integer GL_EXT_gpu_shader4 GL_EXT_draw_buffers2 GL_APPLE_flush_buffer_range GL_APPLE_ycbcr_422 GL_APPLE_vertex_array_range GL_APPLE_texture_range GL_APPLE_float_pixels GL_ATI_texture_float GL_ARB_texture_float GL_ARB_half_float_pixel GL_APPLE_pixel_buffer GL_APPLE_object_purgeable GL_NV_point_sprite GL_NV_register_combiners GL_NV_register_combiners2 GL_NV_blend_square GL_NV_texture_shader GL_NV_texture_shader2 GL_NV_texture_shader3 GL_NV_fog_distance GL_NV_depth_clamp GL_NV_multisample_filter_hint GL_NV_fragment_program_option GL_NV_fragment_program2 GL_NV_vertex_program2_option GL_NV_vertex_program3 GL_ATI_texture_mirror_once GL_ATI_texture_env_combine3 GL_ATI_separate_stencil GL_SGIS_texture_edge_clamp GL_SGIS_texture_lod 

    PTB-INFO: OpenGL-Renderer is NVIDIA Corporation :: NVIDIA GeForce 9400M OpenGL Engine :: 2.0 NVIDIA-1.5.36
    PTB-INFO: Renderer has 256 MB of VRAM and a maximum 236 MB of texture memory.
    PTB-Info: VBL startline = 900 , VBL Endline = 900
    PTB-Info: Measured monitor refresh interval from beamposition = 16.694162 ms [59.901179 Hz].
    PTB-Info: Will use beamposition query for accurate Flip time stamping.
    PTB-Info: Measured monitor refresh interval from VBLsync = 16.695480 ms [59.896453 Hz]. (50 valid samples taken, stddev=0.048354 ms.)
    PTB-Info: Small deviations between reported values are normal and no reason to worry.
    Movie to texture conversion speed is 155.715679 frames per second == 29.804954 Megapixels/second.
    Movietexture playback rate is 416.483849 frames per second == 79.717612 Megapixels/second.
    Done. Bye!

The rates seem to depend on how long you let the movie run (speedup and plateu)
and whether or not this the script has been run before in this matlab session.
(.mex loading, or something) 




1. http://psychtoolbox.org

