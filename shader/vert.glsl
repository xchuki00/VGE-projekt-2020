#version 440 core
uniform mat4 modelViewProj;
uniform vec3 nnr1;
uniform vec3 nnr2;

uniform vec3 nnr3;
uniform vec3 nnr4;

uniform int nnrIntensity;

in vec3 position;
in vec3 color;

out vec3 lineColor;
void main() {
   if(position == nnr1 || position == nnr2||position == nnr3|| position == nnr4){
      lineColor =color*nnrIntensity;
   }else{
      lineColor =color;
   }
   gl_Position = modelViewProj * vec4(position, 1.0);


}
