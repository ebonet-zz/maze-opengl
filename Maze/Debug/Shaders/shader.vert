// All model vertices and normals are passed to this shader.
// It must transform the vertices as appropriate for the view.
// The view and projection matrices are provided, if you need
// the normal matrix, you must construct it in the shader.

uniform float elapsedTime;

vec4 lightPos;
vec3 lightVector;
vec3 normal;
varying vec4 color;


void main()
{	
	color = gl_Color;
	normal = gl_Normal;
	vec4 vertex = gl_ModelViewProjectionMatrix * gl_Vertex;  //use the uploaded matrix data
	
	 //Debug normals
	//if(normal[0]<0) color = vec4(0.0,0.0,1.0,1.0);
	//if(normal[0]>0) color = vec4(1.0,0.0,0.0,1.0);
	//if(normal[1]<0) color = vec4(0.0,1.0,0.0,1.0);
	//if(normal[1]>0) color = vec4(0.0,1.0,1.0,1.0);
	//if(normal[2]<0) color = vec4(1.0,0.0,1.0,1.0);
	//if(normal[2]>0) color = vec4(1.0,1.0,0.0,1.0);
	
	
	lightPos = gl_ModelViewProjectionMatrix * vec4(0.8*cos(elapsedTime*0.8),0.8*sin(elapsedTime*0.8)/2,0.4,1.0);
	
	lightVector = vertex.xyz - lightPos.xyz;
	
	float scale = 1.3*(dot(normalize(lightVector),normalize(normal))+0.2);
	
	
	if(scale<0.0) scale = 0.0;	
	
	color = color * (1.3*scale+0.2);	
	
	gl_Position = vertex;  //output the transformed vertex
}
