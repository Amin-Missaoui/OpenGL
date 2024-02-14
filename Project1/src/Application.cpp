#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmentSource;

};

static ShaderProgramSource parseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1,
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }else if (line.find("fragment") != std::string::npos) {
                    type = ShaderType::FRAGMENT;
            }
        }else{
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
}


static unsigned int CompileShader( unsigned int type, const std::string& source) {

    GLCALL(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();

    GLCALL(glShaderSource(id, 1, &src, nullptr));
    GLCALL(glCompileShader(id));
    
    int result;
    GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

    if (result == GL_FALSE) {
        
        int length;
        GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

        char* message = (char*) alloca(length * sizeof(char) );

        GLCALL(glGetShaderInfoLog(id, length, &length, message));

        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        GLCALL(glDeleteShader(id));
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string & fragmentShader) {
    
    GLCALL(unsigned int program = glCreateProgram());

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCALL(glAttachShader(program, vs));
    GLCALL(glAttachShader(program, fs));
    GLCALL(glLinkProgram(program));
    GLCALL(glValidateProgram(program));

    GLCALL(glDeleteShader(vs));
    GLCALL(glDeleteShader(fs));

    return program;

}

int main(void){
    
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Tutorial", NULL, NULL);
    if (!window){
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR!" << std::endl;
    }
    
    std::cout << glGetString(GL_VERSION) << std::endl;
    // start scope 
        // removes the error when closing the program and the index buffer tries to clear the stack allocated objects
        // after glfwTerminate()
    { 
        float positions[] = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.5f,  0.5f,
             -0.5f, 0.5f,
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };
        
        // vertexArray
        VertexArray va;

        // buffer
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));  // already bound

        // vertexBufferLayout
        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        // indices
        IndexBuffer ib(indices, 6);
   
        // shaders
        ShaderProgramSource source = parseShader("res/shaders/Shader.shader");
    
        std::cout << "VERTEX" << std::endl;
        std::cout << source.VertexSource << std::endl;
        std::cout << "FRAGMENT" << std::endl;
        std::cout << source.FragmentSource << std::endl;

        unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
        GLCALL(glUseProgram(shader));

        // change color using global var u_Color
        GLCALL(int location = glGetUniformLocation(shader, "u_Color"));
        ASSERT(location != -1);
        GLCALL(glUniform4f(location, 0.1f, 0.3f, 1.0f, 1.0f));


        // clear gl states
        va.Unbind();
        vb.Unbind();
        GLCALL(glUseProgram(0));
        GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        // color variables
        float r = 0.0f;
        float increment = 0.05f;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)){
            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);

            GLCALL(glUseProgram(shader));
            GLCALL(glUniform4f(location, r, 0.3f, 1.0f, 1.0f));

            va.Bind();
            ib.Bind();

            GLCALL( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr) );

            // color
            if (r > 1.0f) {
                increment = -0.05f;
            }else if (r < 0.0f){
                increment = 0.05f;
            }

            r += increment;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }


        GLCALL(glDeleteProgram(shader));
    } // end scope
    glfwTerminate();

    return 0;
}