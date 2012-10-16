// Computes fragment colors

varying vec4 color;
varying vec3 normal;  //normal that will be interpolated for the fragment shader
varying vec4 vertexPosition;

uniform float elapsedTime;
uniform float currentPositionX;
uniform float currentPositionY;

vec3 lightPos;

void main()
{	
	lightPos = vec3(currentPositionX, currentPositionY, 2.6);
	
	float distance = distance(lightPos.xyz, vertexPosition.xyz);
	
	float scale = dot(lightPos, normal);
	
	gl_FragColor = color * (distance*distance/5);
	
}
