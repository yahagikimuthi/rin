#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>

// シェーダーソース（C++20/26 の Raw String Literal）
constexpr const char* vertexShaderSource = R"(
    #version 450 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec3 aColor;

    layout (location = 0) out vec3 vColor;

    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        vColor = aColor;
    }
)";

constexpr const char* fragmentShaderSource = R"(
    #version 450 core
    layout (location = 0) in vec3 vColor;
    layout (location = 0) out vec4 FragColor;

    void main() {
        FragColor = vec4(vColor, 1.0);
    }
)";

auto main() -> int {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL 4.5 Core Profile を要求
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(800, 600, "C++26 Game Engine - Hello Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // glad の初期化（OpenGL 関数ポインタのロード）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // 頂点データ (位置 x, y + 色 r, g, b)
    float vertices[] = {
        // Position         // Color
        0.0f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        1.0f
    };

    // OpenGL 4.5 (DSA) を用いた VBO / VAO の構築
    GLuint vao = 0, vbo = 0;
    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);

    glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // バッファバインディングと属性設定 (DSA API)
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, 5 * sizeof(float));

    // location 0: Position
    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao, 0, 0);

    // location 1: Color
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
    glVertexArrayAttribBinding(vao, 1, 0);

    // シェーダーのコンパイル
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // メインループ
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    // リソース解放
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}