#version 440 core
uniform mat4 modelViewProj;

in vec3 position;
out vec3 colorIndex;
void main() {

    colorIndex = vec3(0.1, 0.1, 0.1);
    gl_Position = modelViewProj*vec4(position, 1.0);

}