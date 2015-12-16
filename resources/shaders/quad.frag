#version 150

in vec2 pass_textureCoordinate;

uniform bool isGreyscale;
uniform bool isFlippedVertical;
uniform bool isFlippedHorizontal;
uniform bool isGaussianblurred;

out vec4 out_Color;

void main() {
   vec4 color =vec4(0.0f); // color.rgba
   if (isGreyscale) {
     color = vec4(dot(vec3(0.2126,0.7152,0.0722),color.rgb),color.a);
   }

   if (isFlippedHorizontal) {
     pass_textureCoordinate = vec2(1.0f-pass_textureCoordinate.x, pass_textureCoordinate.y);
   }
   if (isFlippedVertical) {
     pass_textureCoordinate = vec2(pass_textureCoordinate.x, 1.0f-pass_textureCoordinate.y);
   }

   if (isGaussianblurred) {
     //color =
   }

   vec4 color1 = texture2D(t,pass_textureCoordinate);
   gl_FragColor = color1;

   out_Color = color;

}