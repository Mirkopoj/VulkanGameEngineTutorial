#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GloablUbo {
   mat4 projection;
   mat4 view;
   vec4 ambientLightColor;
	vec3 lightPosition;
	uint cols;
}
ubo;

layout(push_constant) uniform Push {
   mat4 modelMatrix;
   mat4 normalMatrix;
}
push;

void main() {
	vec3 lightColor = ubo.ambientLightColor.xyz;
   vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
   vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(ubo.lightPosition)),0.0);

   outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}
