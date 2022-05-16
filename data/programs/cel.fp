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
uniform sampler2D specMap;
uniform sampler2D glowMap;
uniform sampler2D damageMap;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;
uniform vec4 ambientLevel;
uniform vec4 shades;

float lerp(float f, float a, float b){return (1.0-f)*a+f*b; }
vec2  lerp(float f, vec2 a, vec2 b) { return (1.0-f)*a+f*b; }
vec3  lerp(float f, vec3 a, vec3 b) { return (1.0-f)*a+f*b; }
vec4  lerp(float f, vec4 a, vec4 b) { return (1.0-f)*a+f*b; }

vec3  cel(vec3 x, float ambient)
{
    float l = length(x);
    if (l >= 0.25)
        return normalize(x) * ceil(l*shades.x) * shades.y;
    else if (l >= ambientLevel.x)
        return normalize(x) * ambientLevel.y;
    else
        return x;
}

float shininess2Lod(float shininess) { return max(0.0,7.0-log2(shininess+1.0))+3.0*(1.0+envColor.a); }

vec3 envMapping(in vec2 coord, in vec4 specmap)
{
   float envLod = shininess2Lod(gl_FrontMaterial.shininess);
   return texture2DLod(envMap, coord, envLod).rgb * specmap.rgb * envColor.rgb * 2.0;
}

void main() 
{
  // Sample textures
  vec4 damagecolor = texture2D(damageMap , gl_TexCoord[0].xy);
  vec4 diffusecolor= texture2D(diffuseMap, gl_TexCoord[0].xy);
  vec4 speccolor   = texture2D(specMap   , gl_TexCoord[0].xy);
  vec4 glowcolor   = texture2D(glowMap   , gl_TexCoord[0].xy);
  vec4 diffusemap  = lerp(damage.x, diffusecolor, damagecolor);
  vec4 specmap     = speccolor;
  
  vec3 d = diffusemap.rgb * gl_Color.rgb;
  vec3 s = specmap.rgb * gl_SecondaryColor.rgb;
  vec3 e = envMapping(gl_TexCoord[1].xy, specmap);
  vec3 g = glowcolor.rgb;
  
  vec4 rv;
  rv.rgb = cel(d + s + g, ambientLevel) + cel(e, 0.0);
  rv.a = diffusemap.a * gl_Color.a;
  
  gl_FragColor = rv * cloaking.rrrg;
}
