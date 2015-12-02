#version 150

in vec3 pass_Normal;
in vec3 pass_Color;
in vec3 normalInt;
in vec3 vertPos;

in float pass_shading_optin;
in vec3 lightPos;
in vec2 pass_TextureCoordinate;
out vec4 out_Color;

uniform sampler2D Texture;
uniform sampler2D NormalTexture;

// moved const color into function
const vec3 specColor = vec3(0.5, 0.5, 0.5);
const float shininess = 16.0;
const float screenGamma = 2.2;

void main(void)
{
	if (pass_shading_optin == 1) { // Blinn Phong Shading Model

	vec3 ambientColor = pass_Color;
    vec3 diffuseColor = pass_Color;

	// // Local normal, in tangent space - get 2fps
	//vec3 TextureNormal_tangentspace = normalize(texture( NormalTexture, pass_TextureCoordinate ).rgb*2.0 - 1.0);
	vec3 normal = normalize(normalInt); // normalInt and pass_Normal are both normalized and can be used.
	//normal = TextureNormal_tangentspace;

	vec3 lightDir = normalize(lightPos);

	float lambertian = max(dot(lightDir,normal), 0.0);
	float specular = 0.0;

	if (lambertian > 0.0)
	{
		vec3 viewDirection = normalize(-vertPos);

		vec3 halfDir = normalize(lightDir + viewDirection);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specular = pow(specAngle, shininess);	
	}

	vec3 TextureColor = (texture(Texture, pass_TextureCoordinate)).rgb; // without alpha

	//vec3 colorLinear = ambientColor* vec3(0.1f) * TextureColor + lambertian * diffuseColor * TextureColor+ specular * specColor;
	vec3 colorLinear = vec3(0.1f) * TextureColor + vec3(0.8f)* lambertian * TextureColor+ specular * specColor;

	vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));

    out_Color = vec4(colorGammaCorrected, 1.0);
    } else if (pass_shading_optin == 2 ){
		 float A = 0.1;
         float B = 0.3;
         float C = 0.6;
         float D = 1.0;

        	vec3 ambientColor = pass_Color;
            vec3 diffuseColor = pass_Color;

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
}