#ifndef VGL_NV_fragment_program2

# ifndef GL_ARB_shader_texture_lod
#  define VGL_ARB_shader_texture_lod 0
# else
#  extension GL_ARB_shader_texture_lod : enable
#  define VGL_ARB_shader_texture_lod GL_ARB_shader_texture_lod
# endif

# if (VGL_ARB_shader_texture_lod == 0)

#  ifndef GL_ATI_shader_texture_lod
#   define VGL_ATI_shader_texture_lod 0
#  else
#   extension GL_ATI_shader_texture_lod : enable
#   define VGL_ATI_shader_texture_lod GL_ATI_shader_texture_lod
#  endif

#  if (VGL_ATI_shader_texture_lod == 0)
#   define NO_TEXTURE_LOD 1
#  endif

# endif

#endif

#ifdef NO_TEXTURE_LOD
vec4 texture2DLod(sampler2D sampler, vec2 P, float lod)
{
    // Turn into bias
    if (lod <= 1.0)
        lod = -10.0;
    return texture2D(sampler, P, lod-1.);
}
#endif
                    
uniform sampler2D diffuseMap;
uniform sampler2D envMap;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;

vec3 ambientMapping(in vec3 normal)
{
   return texture2DLod(envMap, gl_TexCoord[1].zw, 8.0).rgb * 2.0;
}

void main() 
{
  // Sample textures
  vec4 diffusemap  = texture2D(diffuseMap, gl_TexCoord[0].xy);
  vec4 diffuse = gl_Color;
  diffuse.rgb += ambientMapping(gl_TexCoord[2].xyz);
  
  gl_FragColor = diffusemap * diffuse;
  gl_FragColor.rgb += gl_SecondaryColor.rgb;
  gl_FragColor *= cloaking.rrrg;
}
