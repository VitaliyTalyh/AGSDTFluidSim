//----------------------------------------------------------------------------------------------------------------------
/// @file skyBoxFrag.glsl
/// @author Declan Russell
/// @date 15/03/15
/// @version 1.0
/// @brief Fragment shader for our shy box shader.
//----------------------------------------------------------------------------------------------------------------------


#version 400

//----------------------------------------------------------------------------------------------------------------------
/// @brief texture coordinates of our cube from vertex shader
//----------------------------------------------------------------------------------------------------------------------
in vec3 texcoords;
//----------------------------------------------------------------------------------------------------------------------
/// @brief cube map sampler
//----------------------------------------------------------------------------------------------------------------------
uniform samplerCube cubeMapTex;
//----------------------------------------------------------------------------------------------------------------------
/// @brief our output fragment
//----------------------------------------------------------------------------------------------------------------------
out vec4 fragcolour;


//----------------------------------------------------------------------------------------------------------------------
/// @brief fragment main for shading our cubemap.
//----------------------------------------------------------------------------------------------------------------------
void main () {
  fragcolour = texture (cubeMapTex, texcoords);
  fragcolour.a = 0;
}
