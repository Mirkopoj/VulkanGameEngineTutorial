#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GloablUbo {
   mat4 projection;
   mat4 view;
   vec4 ambientLightColor;
	vec3 lightPosition;
	uint cols;
	uint time;
}
ubo;

layout(push_constant) uniform Push {
   mat4 modelMatrix;
   mat4 normalMatrix;
}
push;

void main() {
	vec3 lightColor = ubo.ambientLightColor.xyz;

   outColor = vec4(lightColor * fragColor, 1.0);
}
