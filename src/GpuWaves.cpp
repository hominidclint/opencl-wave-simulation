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

#include "GpuWaves.h"

GPUWaves::GPUWaves()
    : m_nRows(0),
    m_nCols(0),
    m_nVertices(0),
    m_nTriangles(0), 
    m_k1(0.0f),
    m_k2(0.0f),
    m_k3(0.0f),
    m_timeStep(0.0f),
    m_spatialStep(0.0f),
    m_vertices(0),
    m_normals(0),
    m_tangentsX(0),
    m_indices(0)
{
}

GPUWaves::~GPUWaves()
{
    delete[] m_vertices;
    delete[] m_normals;
    delete[] m_tangentsX;
    delete[] m_indices;
}

unsigned int GPUWaves::rowCount() const
{
    return m_nRows;
}

unsigned int GPUWaves::columnCount() const
{
    return m_nCols;
}

unsigned int GPUWaves::vertexCount() const
{
    return m_nVertices;
}

unsigned int GPUWaves::triangleCount() const
{
    return m_nTriangles;
}

float GPUWaves::width() const
{
    return m_nCols*m_spatialStep;
}

float GPUWaves::depth() const
{
    return m_nRows*m_spatialStep;
}


const float* GPUWaves::k1() const
{
    return &m_k1;
}

const float* GPUWaves::k2() const
{
    return &m_k2;
}

const float* GPUWaves::k3() const
{
    return &m_k3;
}

void GPUWaves::init(unsigned int m, unsigned int n, float dx, float dt, float speed, float damping)
{
    m_nRows = m;
    m_nCols = n;

    m_nVertices = m * n;
    m_nTriangles = 2 * (m-1)*(n-1);

    m_timeStep    = dt;
    m_spatialStep = dx;

    float d = damping * dt + 2.0f;
    float e = (speed*speed)*(dt*dt)/(dx*dx);
    m_k1     = (damping*dt-2.0f)/ d;
    m_k2     = (4.0f-8.0f*e) / d;
    m_k3     = (2.0f*e) / d;

    // in case init() is called again
    delete[] m_vertices;
    delete[] m_normals;
    delete[] m_tangentsX;

    m_vertices  = new glm::vec4[m*n];
    m_normals   = new glm::vec3[m*n];
    m_tangentsX = new glm::vec3[m*n];

    float halfWidth = (n-1)*dx*0.5f;
    float halfDepth = (m-1)*dx*0.5f;

    for(unsigned int i = 0; i < m; ++i)
    {
        float z = halfDepth - i*dx;
        for(unsigned int j = 0; j < n; ++j)
        {
            float x = -halfWidth + j * dx;

            m_vertices[i*n+j] = glm::vec4(x, 0.0f, z, 1.0f);
            m_normals[i*n+j]      = glm::vec3(0.0f , 1.0f, 0.0f);
            m_tangentsX[i*n+j]     = glm::vec3(1.0f, 0.0f, 0.0f);
        }
    }

    createIndices();
}

void GPUWaves::createIndices()
{
    delete[] m_indices;
    m_indices = new unsigned int[3 * m_nTriangles];
    unsigned int m = m_nRows;
    unsigned int n = m_nCols;
    unsigned int k = 0;
    for(unsigned int i = 0; i < m-1; ++i)
    {
        for(unsigned int j = 0; j < n-1; ++j)
        {
            m_indices[k]   = i*n+j;
            m_indices[k+1] = i*n+j+1;
            m_indices[k+2] = (i+1)*n+j;

            m_indices[k+3] = (i+1)*n+j;
            m_indices[k+4] = i*n+j+1;
            m_indices[k+5] = (i+1)*n+j+1;

            k += 6; // next quad
        }
    }
}

const unsigned int* GPUWaves::getIndices() const
{
    return m_indices;
}

glm::vec4* GPUWaves::getVertices() const
{
    return m_vertices;
}