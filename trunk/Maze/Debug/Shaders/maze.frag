// Computes fragment colors

varying float blueColor;
varying float greenColor;

void main()
{	
	gl_FragColor = vec4(1.0, greenColor, blueColor, 1.0);
	gl_FragColor = gl_FragColor * 0.8;
}
