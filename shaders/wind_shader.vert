#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

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
   vec4 positionWorld = push.modelMatrix * vec4(position ,1.0);

   gl_Position = ubo.projection * ubo.view * positionWorld;

	float t = mod(ubo.time + gl_VertexIndex, 100) / 100.f;
	vec3 anim_col = vec3(1, 1, 1) * t;
   fragColor = color*anim_col;
}
