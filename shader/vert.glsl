#version 440 core
layout (location = 0) in vec3 a_position;
out vec2 coord;
uniform vec2 screenRatio;
uniform mat4 mvp;
void main() {
   coord = a_position.xy;
   gl_Position = mvp*vec4(a_position, 1);
}
