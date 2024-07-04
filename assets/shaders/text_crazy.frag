#version 450 core

layout(location = 0) out vec4 o_Color;
layout(binding = 0) uniform sampler2D u_Tex;

uniform float u_Time;

layout(std140, binding = 0) uniform u_Buf
{
    vec2 Scale;
};

in vec4 v_Color;
in vec2 v_TexCoords;

vec3 hsl2rgb(vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0);
    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

void main() {
    float texAlpha = texture(u_Tex, v_TexCoords).r;
    vec2 frag_uv = gl_FragCoord.xy / Scale;
    vec4 rainbow = vec4(hsl2rgb(vec3((u_Time + v_TexCoords.x + v_TexCoords.y), 0.5, 0.5)), 1.0);
    o_Color = vec4(rainbow.rgb, texAlpha);
}