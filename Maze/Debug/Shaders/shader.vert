// All model vertices and normals are passed to this shader.
// It must transform the vertices as appropriate for the view.
// The view and projection matrices are provided, if you need
// the normal matrix, you must construct it in the shader.

uniform float elapsedTime;
uniform float currentPositionX;
uniform float currentPositionY;

varying vec4 color;
varying vec3 normal;  //normal that will be interpolated for the fragment shader
varying vec4 vertexPosition;
varying vec3 lightVector;

vec4 lightPos;

void main()
{	
	color = gl_Color;
	
	
}
