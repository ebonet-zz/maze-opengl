// All model vertices and normals are passed to this shader.
// It must transform the vertices as appropriate for the view.
// The view and projection matrices are provided, if you need
// the normal matrix, you must construct it in the shader.

uniform float elapsedTime;

varying vec3 lightVector;
varying vec3 normal;
varying vec4 color;
varying float distance;

vec4 lightPos;

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
	
	distance = distance(vertex.xyz,lightPos.xyz);
	
	gl_Position = vertex;  //output the transformed vertex
}
