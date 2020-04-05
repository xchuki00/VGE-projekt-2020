#version 440 core

#define sqrt2 1.414213562

uniform vec3 LS0;
uniform vec3 LS1;
uniform vec3 LS2;
uniform float lambdas0;
uniform float lambdas1;
uniform float lambdas2;
//uniform float count;

uniform float thick;
uniform float radius;
uniform float diameter;
uniform float piDivAlfa;
in vec2 coord;
out vec4 fragColor;



void main(void) {

    float s = sqrt(pow(coord.x,2) + pow(coord.y,2));
    if(s>1){
        fragColor = vec4(0,0,0,1);
        return;
    }
    float gama = atan(s*diameter,(radius-thick));
    float at = 1 - cos(gama * piDivAlfa);
    at *= thick;
//    float gamma = 2 * asin(s*diameter / (2 * radius));
//    float at = radius * (1-cos(gamma));
    vec3 colorVec = vec3(0,0,0);
    float color =fract((2*at)/(lambdas0/2));
    color -= 0.5f;
    color *= 2.0f;
    colorVec += abs(color) * LS0;

    color =fract((2*at)/(lambdas1/2));
    color -= 0.5f;
    color *= 2.0f;
    colorVec += abs(color) * LS1;

    color =fract((2*at)/(lambdas2/2));
    color -= 0.5f;
    color *= 2.0f;
    colorVec += abs(color) * LS2;


//        fragColor = vec4(gamma*1000,0,0,1);

    fragColor = vec4(colorVec,1);

}
