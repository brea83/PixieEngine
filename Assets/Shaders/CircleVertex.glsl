#version 450 core
layout (location = 0) in vec3 vertexPosition;

// if bilboarding precalculate the view and model matrices into this one
uniform mat4 Transform;

uniform bool IsBillboard = true;

layout (std140, binding = 0) uniform CameraBlock
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
};

out VS_OUT
{
    vec3 Pos_OS; //Object Space
    vec3 Pos_WS;
    vec3 Pos_CS;
    //vec2 UV;
} OUT;

void main()
{

    if(!IsBillboard)
    {
    	gl_Position = projection * view * Transform * vec4(vertexPosition, 1.0);
        OUT.Pos_OS = vertexPosition;
        OUT.Pos_WS =  vec3(Transform * vec4(vertexPosition, 1.0));
        OUT.Pos_CS = vec3(view * Transform * vec4(vertexPosition, 1.0));
    }
    else
    {
        
        gl_Position = projection * Transform * vec4(vertexPosition, 1.0);
        OUT.Pos_OS = vertexPosition;
        OUT.Pos_WS =  vec3(Transform * vec4(vertexPosition, 1.0));
        OUT.Pos_CS = vec3(view * Transform * vec4(vertexPosition, 1.0));
    }

}