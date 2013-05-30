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

#include "CpuWaves.h"
#include <algorithm>
#include <vector>
#include <cassert>

CPUWaves::CPUWaves()
    : m_nRows(0),
      m_nCols(0),
      m_nVertices(0),
      m_nTriangles(0), 
      m_k1(0.0f),
      m_k2(0.0f),
      m_k3(0.0f),
      m_timeStep(0.0f),
      m_spatialStep(0.0f),
      m_prevSolution(0),
      m_currSolution(0),
      m_normals(0),
      m_tangentX(0)
{
}

CPUWaves::~CPUWaves()
{
    delete[] m_prevSolution;
    delete[] m_currSolution;
    delete[] m_normals;
    delete[] m_tangentX;
}

unsigned int CPUWaves::rowCount() const
{
    return m_nRows;
}

unsigned int CPUWaves::columnCount() const
{
    return m_nCols;
}

unsigned int CPUWaves::vertexCount() const
{
    return m_nVertices;
}

unsigned int CPUWaves::triangleCount() const
{
    return m_nTriangles;
}

float CPUWaves::width() const
{
    return m_nCols*m_spatialStep;
}

float CPUWaves::depth() const
{
    return m_nRows*m_spatialStep;
}


void CPUWaves::init(unsigned int m, unsigned int n, float dx, float dt, float speed, float damping)
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

    // In case Init() called again.
    delete[] m_prevSolution;
    delete[] m_currSolution;
    delete[] m_normals;
    delete[] m_tangentX;

    m_prevSolution = new glm::vec3[m*n];
    m_currSolution = new glm::vec3[m*n];
    m_normals      = new glm::vec3[m*n];
    m_tangentX     = new glm::vec3[m*n];

    // create grid vertices in system memory (as the highfield)
    float halfWidth = (n-1)*dx*0.5f;
    float halfDepth = (m-1)*dx*0.5f;

    for(unsigned int i = 0; i < m; ++i)
    {
        float z = halfDepth - i*dx;
        for(unsigned int j = 0; j < n; ++j)
        {
            float x = -halfWidth + j * dx;

            m_prevSolution[i*n+j] = glm::vec3(x, 0.0f, z);
            m_currSolution[i*n+j] = glm::vec3(x, 0.0f, z);
            m_normals[i*n+j]      = glm::vec3(0.0f , 1.0f, 0.0f);
            m_tangentX[i*n+j]     = glm::vec3(1.0f, 0.0f, 0.0f);
        }
    }
}

void CPUWaves::update(float dt)
{
    static float t = 0;

    // accumulate time
    t += dt;

    // Only update the simulation at the specified time step
    if(t >= m_timeStep)
    {
        // only update interior points; we use zero boundary conditions.
        for(unsigned int i = 1; i < m_nRows-1; ++i)
        {
            for(unsigned int j = 1; j < m_nCols-1; ++j)
            {
                // After this update we will be discarding the old previous
                // buffer, so overwrite that buffer with the new update.
                // Note how we can do this inplace (read/write to same element) 
                // because we won't need prev_ij again and the assignment happens last.

                // Note j indexes x and i indexes z: h(x_j, z_i, t_k)
                // Moreover, our +z axis goes "down"; this is just to 
                // keep consistent with our row indices going down.

                m_prevSolution[i*m_nCols+j].y = m_k1*m_prevSolution[i*m_nCols+j].y +
                    m_k2*m_currSolution[i*m_nCols+j].y +
                    m_k3*(m_currSolution[(i+1)*m_nCols+j].y + 
                    m_currSolution[(i-1)*m_nCols+j].y + 
                    m_currSolution[i*m_nCols+j+1].y + 
                    m_currSolution[i*m_nCols+j-1].y);
            }
        }

        // We just overwrote the previous buffer with the new data, so
        // this data needs to become the current solution and the old
        // current solution becomes the new previous solution.
        std::swap(m_prevSolution, m_currSolution);

        t = 0.0f; // reset time

        //
        // Compute normals using finite difference scheme.
        //
        for(unsigned int i = 1; i < m_nRows-1; ++i)
        {
            for(unsigned int j = 1; j < m_nCols-1; ++j)
            {
                float l = m_currSolution[i*m_nCols+j-1].y;
                float r = m_currSolution[i*m_nCols+j+1].y;
                float t = m_currSolution[(i-1)*m_nCols+j].y;
                float b = m_currSolution[(i+1)*m_nCols+j].y;
                m_normals[i*m_nCols+j].x = l-r;
                m_normals[i*m_nCols+j].y = 2.0f*m_spatialStep;
                m_normals[i*m_nCols+j].z = b-t;

                m_normals[i*m_nCols+j] = glm::normalize(m_normals[i*m_nCols+j]);

                m_tangentX[i*m_nCols+j] = glm::vec3(2.0f*m_spatialStep, r-l, 0.0f);
                m_tangentX[i*m_nCols+j] = glm::normalize(m_tangentX[i*m_nCols+j]);
            }
        }
    }
}

void CPUWaves::disturb(unsigned int i, unsigned int j, float magnitude)
{
    // don't disturb boundaries
    assert(i > 1 && i < m_nRows-2);
    assert(j > 1 && j < m_nCols-2);

    float halfMag = 0.5f * magnitude;

    // Disturb the ijth vertex height and its neighbors.
    m_currSolution[i*m_nCols+j].y     += magnitude;
    m_currSolution[i*m_nCols+j+1].y   += halfMag;
    m_currSolution[i*m_nCols+j-1].y   += halfMag;
    m_currSolution[(i+1)*m_nCols+j].y += halfMag;
    m_currSolution[(i-1)*m_nCols+j].y += halfMag;
}