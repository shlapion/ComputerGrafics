#version 150

#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location=0) in vec3 in_Position;
layout(location=1) in vec2 in_TextureCoordinate;

out vec2 pass_TextureCoordinate;


void main() {


   gl_Position = vec4(in_Position, 1.0);
   pass_TextureCoordinate = in_TextureCoordinate;

}