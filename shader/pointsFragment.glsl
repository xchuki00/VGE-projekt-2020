#version 440 core
out vec4 fragColor;
in vec3 colorIndex;
void main(void) {
    fragColor = vec4(colorIndex, 1.0);

}
