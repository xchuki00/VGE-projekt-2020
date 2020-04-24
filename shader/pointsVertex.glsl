#version 440 core
uniform mat4 modelViewProj;
uniform vec3 nnr;
uniform int nnrIntensity;

in vec3 position;
out vec3 colorIndexVS;
void main() {

   float red = abs(position[0]);
   float green = abs(position[1]);
   float sum = red+green;
   if(nnr == position){
      colorIndexVS =vec3(red/sum,green/sum,1.0)*nnrIntensity;
   }else{
      colorIndexVS =vec3(red/sum,green/sum,1.0);
   }
   gl_Position = modelViewProj * vec4(position, 1.0);

}
