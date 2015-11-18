#version 150

in  vec4 pass_Normal;
in vec3 pass_Color;
in vec3 normalInt;
in vec3 vertPos;

out vec4 out_Color;

const vec3 lightPos = vec3(0.0,0.0,0.0);
// moved const color into function
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 16.0;
const float screenGamma = 2.2;

void main(void)
{

	
	const float A = 0.1;
    const float B = 0.3;
    const float C = 0.6;
    const float D = 1.0;

	vec3 ambientColor = pass_Color; //vec3(0.1, 0.0, 0.0);
    vec3 diffuseColor = pass_Color; //vec3(0.5, 0.0, 0.0);

	vec3 normal = normalize(normalInt); //N
	vec3 lightDir = normalize(lightPos); //L
	vec3 E = vec3(0,0,1);
	vec3 H = normalize(lightDir + E);

	float lambertian = max(dot(lightDir,normal), 0.0); //df

	if (lambertian < A) lambertian = 0.0;
    else if (lambertian < B) lambertian = B;
    else if (lambertian < C) lambertian = C;
    else lambertian = D;

	float specular = max(dot(H,normal), 0.0); //sf
	specular = pow(specular, shininess);
	specular = step(0.5, specular);


	vec3 color = ambientColor + lambertian * diffuseColor + specular * specColor;

    out_Color = vec4(color, 1.0);
}