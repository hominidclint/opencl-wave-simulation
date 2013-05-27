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


#include "GLSLProgram.h"

#include <fstream>
#include <sstream>

#include <sys/stat.h>

GLSLProgram::GLSLProgram()
    : m_handle(0),
      m_linked(false)
{}

bool GLSLProgram::compileShaderFromFile(const char* fileName, GLSLShader::GLSLShaderType type)
{
    if(!fileExists(fileName))
    {
        m_logString = "File not found.";
        return false;
    }

    if(m_handle <= 0)
    {
        m_handle = glCreateProgram();
        if(m_handle == 0)
        {
            m_logString = "Unable to create shader program.";
            return false;
        }
    }

    std::ifstream inFile(fileName, std::ios::in);
    if(!inFile)
    {
        return false;
    }

    std::stringstream code;
    while(inFile.good())
    {
        int c = inFile.get();
        if(!inFile.eof()) code << (char) c;
    }
    inFile.close();

    return compileShaderFromString(code.str(), type);
}

bool GLSLProgram::compileShaderFromString(const std::string& source, GLSLShader::GLSLShaderType type)
{
    if(m_handle <= 0)
    {
        m_handle = glCreateProgram();
        if(m_handle == 0)
        {
            m_logString = "Unable to create shader program.";
            return false;
        }
    }

    GLuint shaderHandle = 0;

    switch(type)
    {
    case GLSLShader::VERTEX:
        shaderHandle = glCreateShader(GL_VERTEX_SHADER);
        break;
    case GLSLShader::FRAGMENT:
        shaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case GLSLShader::GEOMETRY:
        shaderHandle = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    case GLSLShader::TESS_CONTROL:
        shaderHandle = glCreateShader(GL_TESS_CONTROL_SHADER);
        break;
    case GLSLShader::TESS_EVALUATION:
        shaderHandle = glCreateShader(GL_TESS_EVALUATION_SHADER);
        break;
    default:
        return false;
    }

    const char* c_code = source.c_str();
    glShaderSource(shaderHandle, 1, &c_code, NULL);

    // Compile the shader
    glCompileShader(shaderHandle);

    // Check for errors
    int result;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
    if(GL_FALSE == result)
    {
        // Compile failed, store log and return false
        int length = 0;
        m_logString = "";
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
        if(length > 0)
        {
            char* c_log = new char[length];
            int written = 0;
            glGetShaderInfoLog(shaderHandle, length, &written, c_log);
            m_logString = c_log;
            delete[] c_log;
        }

        return false;
    }
    else
    {
        // Compile succeeded, attach shader and return true
        glAttachShader(m_handle, shaderHandle);
        return true;
    }
}

bool GLSLProgram::link()
{
    if(m_linked) return true;
    if(m_handle <= 0) return false;

    glLinkProgram(m_handle);

    int status = 0;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &status);
    if(GL_FALSE == status)
    {
        // Store log and return false
        int length = 0;
        m_logString = "";

        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &length);

        if(length > 0)
        {
            char* c_log = new char[length];
            int written = 0;
            glGetProgramInfoLog(m_handle, length, &written, c_log);
            m_logString = c_log;
            delete[] c_log;
        }

        return false;
    }
    else
    {
        m_linked = true;
        return m_linked;
    }
}

void GLSLProgram::use()
{
    if(m_handle <= 0 || (!m_linked)) return;
    glUseProgram(m_handle);
}

std::string GLSLProgram::log()
{
    return m_logString;
}

int GLSLProgram::getHandle()
{
    return m_handle;
}

bool GLSLProgram::isLinked()
{
    return m_linked;
}

void GLSLProgram::bindAttribLocation(GLuint location, const char* name)
{
    glBindAttribLocation(m_handle, location, name);
}

void GLSLProgram::bindFragDataLocation(GLuint location, const char* name)
{
    glBindFragDataLocation(m_handle, location, name);
}

void GLSLProgram::setUniform(const char* name, float x, float y, float z)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniform3f(loc,x,y,z);
    }
}

void GLSLProgram::setUniform(const char* name, const glm::vec3& v)
{
    this->setUniform(name,v.x,v.y,v.z);
}

void GLSLProgram::setUniform(const char* name, const glm::vec4& v)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniform4f(loc,v.x,v.y,v.z,v.w);
    }
}

void GLSLProgram::setUniform(const char* name, const glm::mat4& m)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
    }
}

void GLSLProgram::setUniform(const char* name, const glm::mat3& m)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
    }
}

void GLSLProgram::setUniform(const char* name, float val)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniform1f(loc, val);
    }
}

void GLSLProgram::setUniform(const char* name, int val)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniform1i(loc, val);
    }
}

void GLSLProgram::setUniform(const char* name, bool val)
{
    int loc = getUniformLocation(name);
    if(loc >= 0)
    {
        glUniform1i(loc, val);
    }
}

void GLSLProgram::printActiveUniforms()
{

    GLint nUniforms, size, location, maxLen;
    GLchar* name;
    GLsizei written;
    GLenum type;

    glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
    glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &nUniforms);

    name = (GLchar*)malloc(maxLen);

    printf(" Location | Name\n");
    printf("------------------------------------------------\n");
    for(int i = 0; i < nUniforms; ++i)
    {
        glGetActiveUniform(m_handle, i, maxLen, &written, &size, &type, name);
        location = glGetUniformLocation(m_handle, name);
        printf(" %-8d | %s\n",location, name);
    }

    free(name);
}

void GLSLProgram::printActiveAttribs()
{

    GLint written, size, location, maxLength, nAttribs;
    GLenum type;
    GLchar* name;

    glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
    glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTES, &nAttribs);

    name = (GLchar*)malloc(maxLength);

    printf(" Index | Name\n");
    printf("------------------------------------------------\n");
    for(int i = 0; i < nAttribs; ++i)
    {
        glGetActiveAttrib(m_handle, i, maxLength, &written, &size, &type, name);
        location = glGetAttribLocation(m_handle, name);
        printf(" %-5d | %s\n",location, name);
    }

    free(name);
}

int GLSLProgram::getUniformLocation(const char* name)
{
    return glGetUniformLocation(m_handle, name);
}

bool GLSLProgram::fileExists(const std::string& fileName)
{
    struct stat info;
    int ret = -1;

    ret = stat(fileName.c_str(), &info);
    return 0 == ret;
}