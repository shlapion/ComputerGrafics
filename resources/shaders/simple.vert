#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

out vec4 v;

out vec4 pass_Normal;

void main(void)
{	
	v = vec4(ViewMatrix * vec4(in_Position, 1.0f));
	
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0f);
	pass_Normal = normalize(NormalMatrix * vec4(in_Normal, 0.0f));
}
