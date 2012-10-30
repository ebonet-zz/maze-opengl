//the texture coordinate attributes and varying output are provided by OpenGL

void main()
{	
	vec4 vertex = gl_ModelViewProjectionMatrix * gl_Vertex;  //use the uploaded matrix data
	gl_Position = vertex;  //output the transformed vertex
	gl_TexCoord[0] = gl_MultiTexCoord0;  //set texture coordinate
}
