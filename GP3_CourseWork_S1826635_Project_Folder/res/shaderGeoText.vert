//Version Number
#version 400

//The layout qualifers
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTexCoords;
layout (location = 2) in vec3 VertexNormal;

//Uniform variable
uniform mat4 transform;

out VS_OUT {
	vec2 texCoords;
} tc_data;

//Passing out the normal and position data
out vec3 v_norm;
out vec4 v_pos; 

void main()
{
	//Assigning the normal and position data
	v_norm = VertexNormal;
	v_pos = vec4(VertexPosition, 1.0);

	tc_data.texCoords = VertexTexCoords;

	// Sets the position of the current vertex
	gl_Position = transform * vec4(VertexPosition, 1.0);
}