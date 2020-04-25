#version 440 core
uniform mat4 modelViewProj;

in vec3 position;

out vec3 lineColor;
void main() {
   lineColor = vec3(0,0,1);
   gl_Position = modelViewProj * vec4(position, 1.0);


}
