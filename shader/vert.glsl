#version 440 core
uniform mat4 modelViewProj;
in vec3 position;
void main() {
   gl_Position = modelViewProj * vec4(position, 1.0);
}
