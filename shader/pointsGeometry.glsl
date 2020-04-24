#version 440 core
layout(points) in;
layout(line_strip, max_vertices = 4) out;
in vec3 colorIndexVS[1];
out vec3 colorIndex;

void main()
{
    colorIndex = colorIndexVS[0];
    gl_Position = gl_in[0].gl_Position + vec4(-0.01, 0.0, 0.0, 0.0);
    EmitVertex();

    colorIndex = colorIndexVS[0];
    gl_Position = gl_in[0].gl_Position + vec4(0.01, 0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();

    colorIndex = colorIndexVS[0];
    gl_Position = gl_in[0].gl_Position + vec4(0.0, -0.01, 0.0, 0.0);
    EmitVertex();

    colorIndex = colorIndexVS[0];
    gl_Position = gl_in[0].gl_Position + vec4(0.0, 0.01, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}