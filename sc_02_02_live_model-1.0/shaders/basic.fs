/* \author Farès BELHADJ */
/* Version GLSL 3.30     */
#version 330
in vec4 mp;
in vec3 vsoNormal;
in float alt;
in vec2 vsoTexCoord;

/* sortie du frament shader : une couleur */
out vec4 fragColor;

const vec3 Lp = vec3(0.0, 10.0, 10.0);

uniform sampler2D tex;//NEW
uniform float temps;
uniform mat4  projectionMatrix, viewMatrix, modelMatrix;

void main(void) {
  vec3 n = normalize((transpose(inverse(modelMatrix)) * vec4(vsoNormal, 0.0)).xyz);
  vec3 nLp = Lp + vec3(0.0, 20.0, 0.0 );

  vec3 Ld = normalize(mp.xyz - nLp);
  float intensiteDeLumiereDiffuse = clamp(dot(n, -Ld), 0.0, 1.0);
  
  float blanc = 7.0;
  // vec4 dcolor = intensiteDeLumiereDiffuse * vec4(alt/blanc, alt/blanc, 0.9, -alt + 5);
  vec4 dcolor = intensiteDeLumiereDiffuse * vec4(0, 0, 0.9, -alt + 4);
  // fragColor = dcolor + scolor;
  fragColor = dcolor;

  fragColor = texture(tex, vsoTexCoord);  

  
  
}
