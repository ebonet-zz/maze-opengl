// Computes fragment colors

uniform float elapsedTime;


varying vec3 lightVector;
varying vec3 normal;
varying float distances;
varying vec3 colorAttribute;


void main()
{	
	float scale = 1.3*(dot(normalize(lightVector),normalize(normal))+0.2);


	if(scale < 0.0) {
		scale = 0.0;
	} else {
		//gl_FragColor = colorAttribute * (1.0*scale+0.2) / (distance/1.7 / 2) ;
	}
	
	gl_FragColor = vec4(colorAttribute * (1.3*scale+0.2),1.0) ;
}
