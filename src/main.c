#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"
#include "shader.h"

#define TARGET_FPS 30

float get_monotonic_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (float)ts.tv_sec + (float)ts.tv_nsec / 1e9f;
}

int main(int argc, char **argv)
{
    init_platform();

    char *vertex_source = (char *)shaders_snow_vert;
    GLint vertex_len = (GLint)shaders_snow_vert_len;
    char *fragment_source = (char *)shaders_snow_frag;
    GLint fragment_len = (GLint)shaders_snow_frag_len;

    GLuint shader_program = create_shader_program(vertex_source, vertex_len, fragment_source, fragment_len);
    glUseProgram(shader_program);

    float projection_matrix[16];
    construct_projection_matrix(projection_matrix, 0.0f, (float)core.window_size.width, 0.0f,
                                (float)core.window_size.height, -1.0f, 1.0f);
    GLint proj_location = glGetUniformLocation(shader_program, "u_projection");
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, projection_matrix);

    GLint viewport_location = glGetUniformLocation(shader_program, "u_viewport");
    glUniform2f(viewport_location, (float)core.window_size.width, (float)core.window_size.height);

    GLint time_location = glGetUniformLocation(shader_program, "u_time");

    const float frame_time = 1.0f / TARGET_FPS;
    const float start_time = get_monotonic_time();
    float last_iteration_time = 0.0f;
    while (true)
    {
        begin_drawing();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(core.window_size.width, 0.0f);
        glVertex2f(core.window_size.width, core.window_size.height);
        glVertex2f(0.0f, core.window_size.height);
        glEnd();

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Update time uniform
        float current_time = get_monotonic_time() - start_time;
        glUniform1f(time_location, current_time);

        end_drawing();

        // We want to run at TARGET_FPS
        current_time = get_monotonic_time() - start_time;
        float elapsed_time = current_time - last_iteration_time;
        if (elapsed_time < frame_time)
        {
            float time_to_sleep = frame_time - elapsed_time;
            usleep((useconds_t)(time_to_sleep * 1e6f));
        }
        last_iteration_time = current_time;
    }
    close_platform();

    return 0;
}
