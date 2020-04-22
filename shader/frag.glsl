#version 440 core
out vec4 fragColor;
uniform float intensity;
void main(void) {
    fragColor = vec4(vec3(intensity), 1.0);
}
