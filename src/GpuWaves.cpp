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
      m_prevSolution(0),
      m_currSolution(0),
      m_normals(0),
      m_tangents(0)
{
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

    // buildwavesimulationviews()
}