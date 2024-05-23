#version 450

layout(location = 0) in float altittude;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

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
	float x = mod(gl_VertexIndex, ubo.cols);
	float y = floor(gl_VertexIndex/ubo.cols);
	vec3 position = vec3(x, altittude, y);
   vec4 positionWorld = push.modelMatrix * vec4(position ,1.0);

   gl_Position = ubo.projection * ubo.view * positionWorld;

   fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
   fragPosWorld = position;
   fragColor = color;
}
