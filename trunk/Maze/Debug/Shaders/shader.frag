// Computes fragment colors

uniform float elapsedTime;

varying vec3 lightVector;
varying vec3 normal;
varying vec4 color;

void main()
{	
	float scale = 1.3*(dot(normalize(lightVector),normalize(normal))+0.2);
	
	if(scale<0.0) scale = 0.0;	
	
	gl_FragColor = color * (1.3*scale+0.2);
}
