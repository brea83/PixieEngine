#version 330 core

uniform vec4 BaseColor = vec4 (1.0, 1.0, 1.0, 1.0);

out vec4 FragColor;

in VS_OUT
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
} IN;

void main()
{
	FragColor = BaseColor ;
	//FragColor.rgb *= IN.VertexColor;
}