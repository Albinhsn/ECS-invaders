#include <SDL.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <stdio.h>

SDL_Window *window;
SDL_GLContext gl_context;
GLuint program;

const char* vertexShaderSource =
    "attribute vec4 position;    \n"
    "void main()                 \n"
    "{                           \n"
    "   gl_Position = position;  \n"
    "}                           \n";

const char* fragmentShaderSource =
    "precision mediump float;    \n"
    "void main()                 \n"
    "{                           \n"
    "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
    "}                           \n";

void mainloop() {
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    GLfloat vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(window);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    window = SDL_CreateWindow("SDL2 + WebGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    gl_context = SDL_GL_CreateContext(window);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    emscripten_set_main_loop(mainloop, 0, 1);

    return 0;
}
