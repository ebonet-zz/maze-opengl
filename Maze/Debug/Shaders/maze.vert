// All model vertices and normals are passed to this shader.
// It must transform the vertices as appropriate for the view.
// The view and projection matrices are provided, if you need
// the normal matrix, you must construct it in the shader.

attribute vec2 localAttr;

varying float blueColor;
varying float greenColor;

void main()
{	
	vec4 vertex = gl_ModelViewProjectionMatrix * gl_Vertex;  //use the uploaded matrix data
	gl_Position = vertex;  //output the transformed vertex
	
	greenColor = localAttr.y;
	blueColor = localAttr.x;
}
