#version 450 core

layout(location = 0) out vec4 o_Color;
layout(binding = 0) uniform sampler2D u_Tex;

in vec4 v_Color;
in vec2 v_TexCoords;

void main() {
    float texAlpha = texture(u_Tex, v_TexCoords).r;
    o_Color = vec4(v_Color.rgb, texAlpha);
}