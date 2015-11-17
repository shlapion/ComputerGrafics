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

out vec4 pass_Normal;
out vec3 vertPos;
out vec3 normalInt;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0f);
	pass_Normal = NormalMatrix * vec4(in_Normal, 0.0f);

	vertPos = vec3(pass_Normal) / pass_Normal.w;
	normalInt = vec3(NormalMatrix * vec4(in_Normal, 0.0));
}