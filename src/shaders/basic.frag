#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 1) uniform sampler2D ts_textures[2];

layout(location = 0) in float g_time;
layout(location = 1) in vec2 g_uv;
layout(location = 2) in vec3 g_normal;
layout(location = 3) in vec3 g_frag_pos;
layout(location = 4) in vec3 g_tangent;
layout(location = 5) in vec3 g_bitangent;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_pos;

void main()
{
    vec3 normal = g_normal;
    normal.y = -normal.y;
    mat3 TBN = { g_tangent, g_bitangent, g_normal };
    vec3 obj_normal = normalize(texture(ts_textures[1], g_uv).xyz * TBN);
    vec3 albedo = texture(ts_textures[0], g_uv).xyz;

    out_color = vec4(albedo, 1);
    out_normal = vec4(obj_normal, 1);
    out_pos = vec4(g_frag_pos, 1);
}
