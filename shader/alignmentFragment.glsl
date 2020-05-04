#version 440 core

in vec3 colorIndex;

out vec4 fragColor;

void main(void) {
    fragColor = vec4(colorIndex, 1);
    //gl_FragDepth = gl_FragDepth + 1;
}