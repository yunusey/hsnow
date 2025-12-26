#version 310 es
precision highp float;

layout (location = 0) in vec2 a_position;

uniform mat4 u_projection;
uniform vec2 u_viewport;

void main()
{
    vec2 world = a_position * vec2(u_viewport);
    gl_Position = u_projection * vec4(world, 0.0, 1.0);
}
