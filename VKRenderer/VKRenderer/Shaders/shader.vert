#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0)uniform UniformBufferObject{
    mat4 Model;
    mat4 View;
    mat4 Proj;
}UBO;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

//vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(0.5, 0.5), vec2(-0.5, 0.5));
//
//vec3 colors[3] = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

void main()
{
    gl_Position = UBO.Proj * UBO.View * UBO.Model * vec4(inPosition, 0.0, 1.0);
    //gl_Position = vec4(inPosition, 0.0f, 1.0f);
    fragColor = inColor;
}
