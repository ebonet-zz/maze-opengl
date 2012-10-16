// Computes fragment colors

varying vec4 color;
uniform float elapsedTime;
varying vec3 normal;  //normal that will be interpolated for the fragment shader
varying vec4 vertexPosition;

uniform float currentPositionX;
uniform float currentPositionY;


void main()
{	

	// lightPos[0]=2.0*cos(elapsedTime);
	// lightPos[1]=2.0*cos(elapsedTime);
	// lightPos[2]=1.0;
	
	gl_FragColor = color;
	//lightPos = vec3(0.0, 0.0, 0.0);
	
	float distance = distance(lightPos.xyz, vertexPosition.xyz);
	
	float scale = dot(lightPos, normal);
	
	gl_FragColor = color /(distance);
	
}
