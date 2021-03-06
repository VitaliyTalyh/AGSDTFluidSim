//----------------------------------------------------------------------------------------------------------------------
/// @file cuboidVert.glsl
/// @class cuboidVert
/// @author Declan Russell
/// @date 2/05/15
/// @version 1.0
/// @namepsace GLSL
/// @brief Vertex shader for our cuboid shader.
//----------------------------------------------------------------------------------------------------------------------

# version 400

//----------------------------------------------------------------------------------------------------------------------
/// @brief position buffer in
//----------------------------------------------------------------------------------------------------------------------
layout (location = 0) in vec3 vertexPosition;

//----------------------------------------------------------------------------------------------------------------------
/// @brief vertex shader main. Just passes position data to geometry shader
//----------------------------------------------------------------------------------------------------------------------
void main () {
  gl_Position = vec4(vertexPosition,1.0);
}
