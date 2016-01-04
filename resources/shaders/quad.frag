#version 150

uniform sampler2D Texture;

uniform bool isGreyscale;
uniform bool isFlippedVertical;
uniform bool isFlippedHorizontal;
uniform bool isGaussianblurred;

in vec2 pass_TextureCoordinate;

out vec4 out_Color;
const vec3 luminance = vec3(0.2126f,0.7152f,0.0722f);

void main() {

   vec2 textureCoordinates = pass_TextureCoordinate;
   vec4 color = vec4(0.0f);

   if (isFlippedHorizontal) {
     textureCoordinates = vec2(1.0f-textureCoordinates.x, textureCoordinates.y);
     color = texture(Texture, textureCoordinates);
   }

   if (isFlippedVertical) {
     textureCoordinates = vec2(textureCoordinates.x, 1.0f-textureCoordinates.y);
     color = texture(Texture, textureCoordinates);
   }

   if (isGreyscale) {
        color = vec4(vec3(dot(luminance,color.rgb)),color.a);
   }
   else {
      color = texture(Texture, textureCoordinates);
   }

   if (isGaussianblurred) {
     //color +=
   }


   out_Color = color;

}