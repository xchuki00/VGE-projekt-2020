#version 440 core
out vec4 fragColor;
in vec3 lineColor;
void main(void) {
    fragColor = vec4(lineColor, 1.0);
}
