#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;
void main(){
	vec2 texelSize = 1.0 / textureSize(_ColorBuffer,0).xy;
vec3 totalColor = vec3(0);
for(int y = -5; y <= 5; y++){
   for(int x = -5; x <= 5; x++){
      vec2 offset = vec2(x,y) * texelSize;
      totalColor += texture(_ColorBuffer,UV + offset).rgb;
   }
}
totalColor/=(11 * 11);
FragColor = vec4(totalColor,1.0);
}
