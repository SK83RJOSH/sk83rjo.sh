#version 300 es
precision lowp float;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;

uniform sampler2D Albedo;
uniform sampler2D Detail;

const vec3 LIGHT_DIR = normalize(vec3(1, 1, 1));

in vec3 VertPosition;
in mat3 VertTBN;
in vec4 VertColor;
in vec2 VertUV;

out vec4 FragColor;

vec3 ReconstructNormal(vec2 xy)
{
	return vec3(xy.x, xy.y, sqrt(1.0 - clamp(dot(xy, xy), 0.0, 1.0)));
}

void main()
{
	vec4 color = texture(Albedo, VertUV) * VertColor;
	vec4 detail = texture(Detail, VertUV);
	vec3 normal = normalize(VertTBN * ReconstructNormal(detail.xy));
	float lambert = max(0.0, dot(normal, LIGHT_DIR));

	vec3 view_direction = normalize((inverse(ProjectionMatrix) * inverse(ViewMatrix) * vec4(0, 0, 1, 0)).xyz);
	vec3 reflect_direction = reflect(-LIGHT_DIR, normal);
	float specular = pow(max(dot(view_direction, reflect_direction), 0.0), 32.0);

	//FragColor = vec4(color.rgb, 1.0);
	//FragColor = vec4((ProjectionMatrix * ViewMatrix * vec4(normal, 0.0)).xyz, 1.0);
	FragColor = min(color * vec4(vec3(lambert + specular), 1.0), 1.0);
}
