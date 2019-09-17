#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(location = 0) out float g_time;
layout(location = 1) out vec2 g_uv;
layout(location = 2) out vec3 g_normal;
layout(location = 3) out vec3 g_frag_pos;
layout(location = 4) out vec3 g_tangent;
layout(location = 5) out vec3 g_bitangent;

// Uniforms
layout(binding = 0) uniform UniformBufferObject {
    float time;
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    mat3 model = mat3(ubo.model);
    g_tangent = normalize(ubo.model * vec4(tangent, 0)).xyz;
    g_bitangent = normalize(ubo.model * vec4(bitangent, 0)).xyz;
    g_normal = normalize(ubo.model * vec4(normal, 0)).xyz;

    g_frag_pos = vec3(ubo.model * vec4(pos, 1.0));
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos, 1.0);
    g_time = ubo.time;
    g_uv = uv;
    g_uv.y *= -1;
}