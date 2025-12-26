#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"
#include "shader.h"

#define TARGET_FPS 120

#ifndef ENABLE_VSYNC
#define ENABLE_VSYNC 0
#endif

// A quick debug logging macro
#ifndef NDEBUG
#include <stdio.h>
#define DBG_LOG(fmt, ...) fprintf(stderr, "[DBG] " fmt "\n", ##__VA_ARGS__)
#else
#define DBG_LOG(...) ((void)0)
#endif

void signal_handler(int signum)
{
    core.state.should_close = true;
}

float get_monotonic_time()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (float)ts.tv_sec + (float)ts.tv_nsec / 1e9f;
}

int main(int argc, char **argv)
{
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    const char *config_path = get_config_path();
    if (config_path)
    {
        load_config(&config, config_path);
    }
    else
    {
        printf("Warning: Could not determine config path, using defaults.\n");
    }

    init_platform(&config);

    char *vertex_source = (char *)shaders_snow_vert;
    GLint vertex_len = (GLint)shaders_snow_vert_len;
    char *fragment_source = (char *)shaders_snow_frag;
    GLint fragment_len = (GLint)shaders_snow_frag_len;

    GLuint shader_program = create_shader_program(vertex_source, vertex_len, fragment_source, fragment_len);
    glUseProgram(shader_program);

    float projection_matrix[16];
    construct_projection_matrix(projection_matrix, 0.0f, (float)core.window_size.width, 0.0f,
                                (float)core.window_size.height, -1.0f, 1.0f);

    // Set uniform values
    GLint proj_location = glGetUniformLocation(shader_program, "u_projection");
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, projection_matrix);
    GLint viewport_location = glGetUniformLocation(shader_program, "u_viewport");
    glUniform2f(viewport_location, (float)core.window_size.width, (float)core.window_size.height);
    GLint time_location = glGetUniformLocation(shader_program, "u_time");

    // Set configuration uniforms
    GLint num_layers_location = glGetUniformLocation(shader_program, "c_num_layers");
    glUniform1ui(num_layers_location, (GLuint)config.num_layers);
    GLint depth_location = glGetUniformLocation(shader_program, "c_depth");
    glUniform1f(depth_location, config.depth);
    GLint width_location = glGetUniformLocation(shader_program, "c_width");
    glUniform1f(width_location, config.width);
    GLint speed_location = glGetUniformLocation(shader_program, "c_speed");
    glUniform1f(speed_location, config.speed);
    GLint alpha_location = glGetUniformLocation(shader_program, "c_alpha");
    glUniform1f(alpha_location, config.alpha);

    float quad_vertices[] = {
        0.0f, 0.0f, // Bottom-left
        1.0f, 0.0f, // Bottom-right
        1.0f, 1.0f, // Top-right
        0.0f, 0.0f, // Bottom-left
        1.0f, 1.0f, // Top-right
        0.0f, 1.0f, // Top-left
    };

    GLuint vao, vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const float frame_time = 1.0f / TARGET_FPS;
    const float start_time = get_monotonic_time();
    float last_frame_time = get_monotonic_time();

    core.state.should_close = false;
    while (!core.state.should_close)
    {
        float frame_start = get_monotonic_time();
        float elapsed_time = frame_start - last_frame_time;
        last_frame_time = frame_start;

        begin_drawing();
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1f(time_location, frame_start - start_time);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        end_drawing();

        // NOTE: When VSync is disabled, my system runs at ~512 FPS, which is way too fast :D
        DBG_LOG("Frame Time: %.3f ms (%.1f FPS)", elapsed_time * 1000.0f, 1.0f / elapsed_time);
    }

    glDeleteProgram(shader_program);
    close_platform();

    return 0;
}
