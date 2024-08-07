#version 450 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

layout(std140, binding = 0) uniform u_Buf
{
    vec2 Scale;
};

out vec2 v_TexCoords;
out vec4 v_Color;

void main() {
    gl_Position = vec4(Scale * a_Position + vec2(-1.0f, 1.0f), 0.0, 1.0);
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;
}