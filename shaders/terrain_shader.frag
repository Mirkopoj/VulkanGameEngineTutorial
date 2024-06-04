#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GloablUbo {
   mat4 projection;
   mat4 view;
   mat4 invView;
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
	vec3 lightColor = vec3(0.9921568627,0.9843137255,0.8274509804);
   vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
   vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(ubo.lightPosition)),0.0);
   //vec3 specularLight = vec3(0.3);
   //vec3 surfaceNormal = normalize(fragNormalWorld);

   //vec3 cameraPosWorld = ubo.invView[3].xyz;
   //vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

   outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}
