#version 150

varying vec2 textureCoord;

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

   vec4 color1 = texture2D(t,textureCoord);
   gl_FragColor = color1;

   out_Color = color;

}