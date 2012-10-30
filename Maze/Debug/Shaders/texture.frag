//OpenGL automatically provides the texture coordinate variable

uniform sampler2D texId; //this is the texture unit that has the rendered image
uniform vec2 resolution;  //view resolution in pixels
uniform float elapsedTime;

const float blurSize = 1.0/512.0;

vec2 getWaveOffset()
{
	float waveAmplitude = 10.0;
	float waveSpeed = elapsedTime * 3.0;
	float waveFrequency = gl_FragCoord.y / 100.0;
	vec2 waveOffset = vec2( cos(waveSpeed+waveFrequency), 0.0) * waveAmplitude / resolution.xy;
	
	return waveOffset;
}

vec4 blur(sampler2D texId, vec2 texPos){
	
	float distanceToCenter = distance(texPos,vec2(0.5));
	
	
	vec4 sum = vec4(0.0);
	
	if (distanceToCenter <= 0.1){
	
		return texture2D(texId, vec2(texPos.x, texPos.y))*(1.5-2*distanceToCenter);;
	}
		
	else if (distanceToCenter<=0.2){	
    	sum += texture2D(texId, vec2(texPos.x, texPos.y - blurSize)) * 0.1;
    	sum += texture2D(texId, vec2(texPos.x, texPos.y)) * 0.8;
    	sum += texture2D(texId, vec2(texPos.x, texPos.y + blurSize)) * 0.1;
    }
    
    else if (distanceToCenter <=0.3){
    	
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - 2.0*blurSize)) * 0.03;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - blurSize)) * 0.17;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y)) * 0.6;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + blurSize)) * 0.17;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + 2.0*blurSize)) * 0.03;
    
    }
    
    else if (distanceToCenter <=0.4){
    	
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - 3.0*blurSize)) * 0.03;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - 2.0*blurSize)) * 0.15;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - blurSize)) * 0.20;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y)) * 0.25;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + blurSize)) * 0.20;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + 2.0*blurSize)) * 0.15;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + 3.0*blurSize)) * 0.03;
    
    }
    
    else {
        sum += texture2D(texId, vec2(texPos.x, texPos.y - 4.0*blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - 3.0*blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - 2.0*blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y - blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + 2.0*blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + 3.0*blurSize)) * 0.1;
	    sum += texture2D(texId, vec2(texPos.x, texPos.y + 4.0*blurSize)) * 0.1;
    }
	return sum;
}

void main(void)
{
	vec2 waveOffset = getWaveOffset();
	vec2 computedPos = gl_FragCoord.xy / resolution.xy; //compute pos from frag location
	vec2 interpolatedPos = gl_TexCoord[0].xy;           //from pos from tex coordinates
	
	vec2 texPos;//both position coordinate should be identical
	texPos = computedPos + waveOffset;
	texPos = interpolatedPos + waveOffset;
	
	vec4 texture = blur(texId, texPos);
	gl_FragColor = texture;
}
