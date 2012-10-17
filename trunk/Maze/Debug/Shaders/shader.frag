// Computes fragment colors

uniform float elapsedTime;

varying vec3 lightVector;
varying vec3 normal;
varying vec4 color;
varying float distance;

void main()
{	
	float scale = 1.3*(dot(normalize(lightVector),normalize(normal))+0.2);
	
	gl_FragColor = color * (1.3*scale+0.2) / (distance/1.7 / 2) ;
	
	//gl_FragColor = color * (1.3*scale+0.2) ;
}
