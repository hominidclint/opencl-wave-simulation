// Copyright (c) 2013, Hannes Würfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.


#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>

#include <glm/glm.hpp>

namespace GLSLShader
{
    enum GLSLShaderType
    {
        VERTEX, FRAGMENT, GEOMETRY,
        TESS_CONTROL, TESS_EVALUATION
    };
};

class GLSLProgram
{
public:
    GLSLProgram();

    bool compileShaderFromFile(const char* fileName, GLSLShader::GLSLShaderType type);
    bool compileShaderFromString(const std::string& source, GLSLShader::GLSLShaderType type);
    bool link();
    void use();

    std::string log();

    int getHandle();
    bool isLinked();

    // should be called before linking the program
    void bindAttribLocation(GLuint location, const char* name);
    void bindFragDataLocation(GLuint location, const char* name);

    void setUniform(const char* name, float x, float y, float z);
    void setUniform(const char* name, const glm::vec3& v);
    void setUniform(const char* name, const glm::vec4& v);
    void setUniform(const char* name, const glm::mat4& m);
    void setUniform(const char* name, const glm::mat3& m);
    void setUniform(const char* name, float val);
    void setUniform(const char* name, int val);
    void setUniform(const char* name, bool val);

    void printActiveUniforms();
    void printActiveAttribs();

private:
    int  m_handle;
    bool m_linked;
    std::string m_logString;

    int getUniformLocation(const char* name);
    bool fileExists(const std::string& fileName);
};

#endif // GLSLPROGRAM_H
