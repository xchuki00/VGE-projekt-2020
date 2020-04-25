#version 440 core
uniform mat4 modelViewProj;
in vec3 position;
in vec3 color;

out vec3 lineColor;
void main() {
    lineColor =color;
    gl_Position = modelViewProj * vec4(position, 1.0);


}
