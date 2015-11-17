#version 150


in vec4 v; 

in  vec4 pass_Normal;
out vec4 out_Color;

void main(void)
{
	
	vec3 L = normalize(gl_LightSource[0].position.xyz - v);
	vec3 E = normalize(-v);
	vec3 R = normalize(-reflect(L,pass_Normal));

	vec3 ambient = gl_FrontLightProduct[0].ambient;

	vec4 diffuse = gl_FrontLightProductp[0].diffuse * max(dot(pass_Normal,L), 0.0);
	diffuse = clamp(diffuse, 0.0, 1.0);

	vec4 specular = gl_FrontLightProduct[0].specular * pow(max(dot(R,E),0.0), 0.3 * gl_FrontMaterial.shininess);
	specular = clamp(spec, 0.0, 1.0);

	out_Color = gl_FrontLightModelProduct.sceneColor + ambient + diffuse + specular;

}
