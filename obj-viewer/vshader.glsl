#version 330 core

layout(location = 0) in vec3 vPosition;

uniform mat4 mModelView;
uniform mat4 mProjection;

void main() { 
	gl_Position = mProjection * mModelView * vec4(vPosition, 1.0);
}
