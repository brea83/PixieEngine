#version 450 core

// OpenGL guarantees there are always at least 16 locations
// for 4-component vertex attributes
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;
//layout (location = 2) in vec3 vertexNormal;
//layout (location = 3) in vec2 uvCoord;
//layout (location = 4) in vec3 vertexTangent;
//layout (location = 5) in vec3 vertexBitangent;

uniform mat4 Transform;
//uniform bool BIsDrawingFrustum = false;
//uniform mat4 FrustumMatrix;


layout (std140, binding = 0) uniform CameraBlock
{
    mat4 view;
    mat4 projection;
    vec4 cameraPosition;
};

out VS_OUT
{
//   vec3 Pos_WS;
//   //vec3 Pos_TS;
//   vec3 Pos_CS;
//   //vec4 Pos_LS;
//   vec3 Normal_WS;
//   vec3 Normal_CS;
//   vec2 UV;
   vec3 VertexColor;
   //vec3 CameraPos_WS;
   //vec3 CameraPos_TS; 
   //vec3 EyeDirection_CS;
   //mat3 TBN;
} OUT;

void main()
{
//    if(BIsDrawingFrustum)
//    {
//         gl_Position = projection * view * Transform * vec4(vertexPosition, 1.0);
//        OUT.Pos_WS =  vec3(Transform * vec4(vertexPosition, 1.0));
//    }
//    else
//    {
        vec4 Pos_WS =  Transform * vec4(vertexPosition, 1.0);
        //Pos_WS = Pos_WS / Pos_WS.w;
        gl_Position = projection * view * Pos_WS;//Transform * vec4(vertexPosition, 1.0);
        gl_Position /= Pos_WS.w;
        //OUT.Pos_CS = vec3(view * transform * vec4(vertexPosition, 1.0));

        //mat3 MV3 = mat3(view * Transform);
        //mat3 M = mat3(Transform);
        //vec3 t = M * normalize(vertexTangent);
        //vec3 b = M * normalize(vertexBitangent);
        //vec3 n = M * normalize(vertexNormal);

        //OUT.TBN = transpose(mat3(t, b, n));

        //OUT.Pos_TS = OUT.TBN * OUT.Pos_WS;

        //OUT.CameraPos_WS = cameraPosition.xyz;
        //OUT.CameraPos_TS = OUT.TBN * cameraPosition.xyz;
        //OUT.EyeDirection_CS = vec3(0,0,0) - OUT.Pos_CS;



//        OUT.Normal_WS = (Transform * vec4(vertexNormal, 0.0)).xyz;
//        OUT.Normal_CS = (view * Transform * vec4(vertexNormal, 0.0)).xyz;
//
//        OUT.UV = uvCoord;

        OUT.VertexColor = vertexColor;
   // }
}