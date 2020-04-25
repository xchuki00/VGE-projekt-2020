#version 440 core
uniform mat4 modelViewProj;

in vec3 position;
out vec3 colorIndexVS;
void main() {

    float red = abs(position[0]);
    float green = abs(position[1]);
    float sum = red+green;
    colorIndexVS =vec3(red/sum, green/sum, 0.0);
    gl_Position = modelViewProj * vec4(position, 1.0);

}
