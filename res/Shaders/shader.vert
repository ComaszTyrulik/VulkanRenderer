#version 450

layout(location = 0) out vec4 fragColor;

vec3 vertexPositions[3] = vec3[](
    vec3(-0.5, -0.5, 0.0),
    vec3(0.5, -0.5, 0.0),
    vec3(0.0, 0.5, 0.0)
);

vec4 vertexColor[3] = vec4[](
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
);

void main()
{
    fragColor = vertexColor[gl_VertexIndex];
    gl_Position = vec4(vertexPositions[gl_VertexIndex], 1.0);
}