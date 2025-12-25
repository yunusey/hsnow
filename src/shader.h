#ifndef SHADER_H
#define SHADER_H

#include <GL/gl.h>
#include <GLES3/gl3.h>

// Shader data declared here, included in shader.c, automatically generated from shader files in the shaders/ folder
extern unsigned char shaders_snow_frag[];
extern unsigned int shaders_snow_frag_len;
extern unsigned char shaders_snow_vert[];
extern unsigned int shaders_snow_vert_len;

GLuint compile_shader(GLenum type, const char *source, GLint length);
GLuint create_shader_program(const char *vertex_source, GLint vertex_len, const char *fragment_source,
                             GLint fragment_len);
void construct_projection_matrix(float matrix[16], float left, float right, float bottom, float top, float near,
                                 float far);

#endif // SHADER_H
