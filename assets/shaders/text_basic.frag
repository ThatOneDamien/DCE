#version 450 core

layout(location = 0) out vec4 o_Color;
layout(binding = 0) uniform sampler2D u_Tex;

in vec4 v_Color;
in vec2 v_TexCoords;

void main() {
    if(v_TexCoords.x >= 0.0f) 
    {
        float d = texture(u_Tex, v_TexCoords).r;
        float aaf = 0.5f * fwidth(d);
        aaf = smoothstep(0.5 - aaf, 0.5 + aaf, d);
        o_Color = vec4(v_Color.rgb, aaf * v_Color.a);
    } 
    else
    {
        o_Color = v_Color;
    }
}