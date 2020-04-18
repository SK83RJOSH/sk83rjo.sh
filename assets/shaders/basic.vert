#version 300 es
precision lowp float;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

in vec3 Position;
in vec3 Normal;
in vec4 Tangent;
in vec3 Color;
in vec2 UV;

out vec3 VertPosition;
out mat3 VertTBN;
out vec4 VertColor;
out vec2 VertUV;

vec4 snap(vec4 position)
{
	vec4 snapped = position;
	snapped.xyz = position.xyz / position.w;
	snapped.x = floor(160.0 * snapped.x) / 160.0;
	snapped.y = floor(120.0 * snapped.y) / 120.0;
	snapped.xyz *= position.w;
	return snapped;
}

void main()
{
	vec3 normal = normalize((ModelMatrix * vec4(Normal, 0.0)).xyz);
	vec3 tangent = normalize((ModelMatrix * vec4(Tangent.xyz, 0.0)).xyz);
	vec3 bitangent = normalize((ModelMatrix * vec4((cross(Normal, Tangent.xyz) * Tangent.w), 0.0)).xyz);

	VertPosition = (ViewMatrix * ModelMatrix * vec4(Position, 1.0)).xyz;
	VertTBN = mat3(tangent, bitangent, normal);
	VertColor = vec4(Color, 1.0);
	VertUV = vec2(UV.x, UV.y);
	gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);
}
