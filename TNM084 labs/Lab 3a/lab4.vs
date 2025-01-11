#version 410 core

// Mostly pass-through but with many attributes
in vec3 inPosition;
in vec2 inTexCoord;
in vec3 inNormal;
out vec4 vPosition;
out vec2 vTexCoord;
out vec3 vNormal;

void main()
{
// We will not do transformations here since we want the
// tesselation and geometry shaders to work in model coordinates.
    vPosition = vec4(inPosition, 1.0);
    vTexCoord = inTexCoord;
    vNormal = inNormal;
}
