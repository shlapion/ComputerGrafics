#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;
layout(location=2) in vec2 in_TextureCoordinate;
/*
    The in modifier is used to qualify inputs into a shader stage.
    Those inputs may be vertex attributes (for vertex shaders),
    or output variables from the preceding shader stage.
    Fragment shaders can further qualify their input values using
    some additional keywords.
*/

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 ColorVec;

uniform vec3 SunPosition;
uniform float ShadingOption;
/*
    The uniform modifier specifies that a variable’s value will be
    specified by the application before the shader’s execution
    and does not change across the primitive being processed.
    Uniform variables are shared between all the shader stages
    enabled in a program and must be declared as global variables.
    Any type of variable, including structures and arrays,
    can be specified as uniform. A shader cannot write to a uniform
    variable and change its value.

For example, you might want to use a color for shading a primitive.
You might declare a uniform variable to pass that information into
your shaders. In the shaders, you would make the declaration:
uniform vec4 BaseColor;
Within your shaders, you can reference BaseColor by name, but
to set its value in your application, you need to do a little
extra work. The GLSL compiler creates a table of all uniform
variables when it links your shader program. To set BaseColor’s
value from your application, you need to obtain the index of
BaseColor in the table, which is done using the
glGetUniformLocation() routine.
*/

// should maybe renamed in pass_*
out vec3 pass_Normal;
out vec3 vertPos;
out vec3 normalInt;
out vec3 pass_Color;
out float pass_shading_optin;
out vec3 lightPos;
out vec2 pass_TextureCoordinate;
/*
    The out modifier is used to qualify outputs from a shader
    stage - for example, the transformed homogeneous coordinates
    from a vertex shader, or the final fragment color from a
    fragment shader.
*/

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0f);

    vertPos = vec3(ViewMatrix  * vec4(in_Position,1.0));


    // normalInt and pass_Normal is the same. And pass_Normal is better name convention
	normalInt = vec3(NormalMatrix * vec4(in_Normal, 0.0));
	pass_Normal = normalize(NormalMatrix * vec4(in_Normal,0.0f)).xyz;

	pass_Color = ColorVec;

    lightPos = SunPosition;

	pass_shading_optin = ShadingOption;

    pass_TextureCoordinate = in_TextureCoordinate;
}