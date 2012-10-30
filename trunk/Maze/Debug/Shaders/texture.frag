//OpenGL automatically provides the texture coordinate variable

uniform sampler2D texId; //this is the texture unit that has the rendered image
uniform vec2 resolution;  //view resolution in pixels
uniform float elapsedTime;

vec2 getWaveOffset()
{
	float waveAmplitude = 10.0;
	float waveSpeed = elapsedTime * 3.0;
	float waveFrequency = gl_FragCoord.y / 100.0;
	vec2 waveOffset = vec2( cos(waveSpeed+waveFrequency), 0.0) * waveAmplitude / resolution.xy;
	
	return waveOffset;
}

void main(void)
{
	vec2 waveOffset = getWaveOffset();
	vec2 computedPos = gl_FragCoord.xy / resolution.xy; //compute pos from frag location
	vec2 interpolatedPos = gl_TexCoord[0].xy;           //from pos from tex coordinates
	
	vec2 texPos;//both position coordinate should be identical
	texPos = computedPos + waveOffset;
	texPos = interpolatedPos + waveOffset;
	
	vec4 texture = texture2D(texId, texPos);
	gl_FragColor = texture;
}