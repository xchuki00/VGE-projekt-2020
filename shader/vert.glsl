#version 440 core
uniform mat4 modelViewProj;
in vec3 position;
in vec2 color;
out vec2 lineColor;
void main() {
   lineColor =color;
   gl_Position = modelViewProj * vec4(position, 1.0);


}
