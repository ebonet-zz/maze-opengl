// All model vertices and normals are passed to this shader.
// It must transform the vertices as appropriate for the view.
// The view and projection matrices are provided, if you need
// the normal matrix, you must construct it in the shader.

uniform float elapsedTime;

varying vec3 lightVector;
varying vec3 normal;
varying float distances;
varying vec3 colorAttribute;

attribute vec3 attr_color;


vec4 lightPos;

void main()
{	
	colorAttribute = attr_color;
	
	normal = gl_Normal;
	vec4 vertex = gl_Vertex;  //use the uploaded matrix data
	
	lightPos = vec4(1.1*cos(elapsedTime*0.8),1.1*sin(elapsedTime*0.8),1.0,1.0); // Light Position
	
	lightVector = vertex.xyz - lightPos.xyz;
	
	distances = distance(vertex.xyz,lightPos.xyz);
	
	gl_Position = gl_ModelViewProjectionMatrix * vertex;  //output the transformed vertex
}
