#ifndef LGL_LINE_H
#define LGL_LINE_H

#include "lgl/Shader.h"
#include "lgl/Wrapped_GL.h"
#include "glm/vec3.hpp"
#include <algorithm>

/// LineData
/// Used with Line to render. Allow users to hold Line's geometry data in multiple places
/// without a need to incur for increased size of functionality we don't need at that time.
/// Use LineData feeding into Line class to draw in run-time.
struct LineData
{
    glm::vec3 dir;
    glm::vec3 pos;

    LineData():
        dir(glm::vec3(0.0f)),
        pos(glm::vec3(0.0f))
    {
    }

    LineData(const glm::vec3& dir, const glm::vec3& pos):
        dir(dir),
        pos(pos)
    {
    }

    LineData(const LineData& data):
        dir(data.dir),
        pos(data.pos)
    {
    }

    LineData(const LineData&& data):
        dir(data.dir),
        pos(data.pos)
    {
    }

    LineData& operator=(const LineData& l)
    {
        dir = l.dir;
        pos = l.pos;
        return *this;
    }

    LineData& operator=(const LineData&& l)
    {
        dir = l.dir;
        pos = l.pos;
        return *this;
    }

    inline void setData(const glm::vec3& dir, const glm::vec3& pos)
    {
        this->dir = dir;
        this->pos = pos;
    }
};

/**
 * Line primitive data structure + basic functionality that doesn't add up to size of the class.
 */
class Line
{
public:
    Line()
    {
        // initially set line data that is going to be used mostly
        // anyway Line provides custom LineData drawing
        initialInitialize(LineData(glm::vec3(0.0f), glm::vec3(0.0f)));
    }

    /// Specify line data which is going to be used mostly
    /// then attach to Line for optimized drawing
    Line(const glm::vec3& e0, const glm::vec3& e1)
    {
        initialInitialize(LineData(glm::normalize(e1 - e0), e0));
    }

    Line(const LineData& ld)
    {
        initialInitialize(ld);
    }

    Line(const LineData&& ld)
    {
        initialInitialize(std::move(ld));
    }

    void setLineData(const LineData& ldata);
    void setLineData(const LineData&& ldata);
    const LineData& getLineData() const;

    /// Required: attached shader needs to call Shader::Use() before setting any of the following
    /// functions.
    void updateProjectionMatrix(const glm::mat4& mat);
    void updateViewMatrix(const glm::mat4& mat);
    void updateModelMatrix(const glm::mat4& mat);

    /// build shader 
    void build();

    /// destroy any opengl related objects
    void destroyGLObjects();

    /// singlely draw this object
    /// if you intend to draw multiples, use drawBatchBegin()
    void draw() const;

    /// beginning drawing of multiple of Line instances
    void drawBatchBegin() const;

    /// draw a batched object
    void drawBatchDraw() const;

    /// end batch drawing
    void drawBatchEnd() const;

    /// set line color when draw
    /// Required: shader is active. Call Shader::Use of its Line::shader
    void setLineColor(float r, float g, float b);
    const glm::vec3 getLineColor() const;

    /// set/get t factor
    void setT(float v);
    float getT() const;

    lgl::Shader shader;

private:
    void initialInitialize(const LineData& ldata);
    void initialInitialize(const LineData&& ldata);
    void destroyVertexBuffersIfNeeded();
    void destroyShaderIfNeeded();
    void computeLineDataDraw();

private:
    /// for caching the attached line data for drawing performance
    /// but Line provides ability to draw with custom LineData as well
    LineData lineData;
    glm::vec3 lineDataDraw[2];
    glm::vec3 lineColor;

    /// t factor to draw the line based on line equation
    float t;
    bool isShaderBuilt;

    GLuint spec_vao;
    GLuint spec_vbo;
};

/// inline implementation
inline void Line::setLineData(const LineData& ldata)
{
    lineData = ldata; 
    computeLineDataDraw();

    // update to OpenGL
    glBindVertexArray(spec_vao);
        glBindBuffer(GL_ARRAY_BUFFER, spec_vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, lineDataDraw, GL_STREAM_DRAW);
    glBindVertexArray(0);
}

inline void Line::setLineData(const LineData&& ldata)
{
    lineData = std::move(ldata);
    computeLineDataDraw();

    // update to OpenGL
    glBindVertexArray(spec_vao);
        glBindBuffer(GL_ARRAY_BUFFER, spec_vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, lineDataDraw, GL_STREAM_DRAW);
    glBindVertexArray(0);
}

inline const LineData& Line::getLineData() const
{
    return lineData;
}

inline void Line::setLineColor(float r, float g, float b)
{
    lineColor.r = r;
    lineColor.g = g;
    lineColor.b = b;

    // update to OpenGL
    glUniform3f(shader.GetUniformLocation("color"), lineColor.x, lineColor.y, lineColor.z);
}

inline const glm::vec3 Line::getLineColor() const
{
    return lineColor;
}

inline void Line::setT(float v)
{
    t = v;
}

inline float Line::getT() const
{
    return t;
}

#endif
