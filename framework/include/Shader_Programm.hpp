//
// Created by Martin Heinrich on 04.11.15.
//

#ifndef OPENGL_FRAMEWORK_SHADER_PROGRAMM_HPP
#define OPENGL_FRAMEWORK_SHADER_PROGRAMM_HPP

#include <OpenGL/OpenGL.h>
#include <string>

struct Shader_Programm {
    std::string vertex_path;
    std::string frag_path;
    GLuint* programm;
};

#endif //OPENGL_FRAMEWORK_SHADER_PROGRAMM_HPP
