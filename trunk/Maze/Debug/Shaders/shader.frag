// Computes fragment colors

uniform float elapsedTime;
uniform float currentPositionX;
uniform float currentPositionY;

varying vec4 color;
varying vec3 normal;  //normal that will be interpolated for the fragment shader
varying vec4 vertexPosition;
varying vec3 lightVector;

void main()
{	
	float scale = dot(normalize(lightVector), normalize(normal));
	
	if(scale<0) scale = 0;
	gl_FragColor = color * scale;
}
