#include <QOpenGLFunctions>
#include "lbwshader.h"
#include <lbwgeometrybuffer.h>

static inline void MatrixAsUniform(QOpenGLFunctions *f, GLuint location, QMatrix4x4 m) { f->glUniformMatrix4fv(location,1,GL_TRUE,m.data()); }

//-------------------------------------------------------------------------------------------------
LbwShader::LbwShader()
{
    mVertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    mVertexShader->compileSourceFile(":/phong.vert");
    Q_ASSERT(mVertexShader->isCompiled());

    mFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);
    mFragmentShader->compileSourceFile(":/phong.frag");
    Q_ASSERT(mFragmentShader->isCompiled());


    mShaderProgram = new QOpenGLShaderProgram();
    mShaderProgram->addShader(mVertexShader);
    mShaderProgram->addShader(mFragmentShader);

    bool done = mShaderProgram->link();
    Q_ASSERT_X(done,"Shader Linker", mShaderProgram->log().toUtf8().data());

    // Get locations of attributes and uniforms used inside.
    vertexPosition = mShaderProgram->attributeLocation("vertex" );
    vertexNormal = mShaderProgram->attributeLocation("vnormal" );
    vertexColor  = mShaderProgram->attributeLocation("vcolor" );

    modelviewMatrix  = mShaderProgram->uniformLocation("modelview" );
    normalviewMatrix = mShaderProgram->uniformLocation("normalview" );
    projectionMatrix = mShaderProgram->uniformLocation("projection" );
}

//-------------------------------------------------------------------------------------------------
void LbwShader::bind(const QMatrix4x4 &perspectiveMatrix)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    // Activate shader.
    // f->glUseProgram( mShaderProgram->programId() );
    mShaderProgram->bind();

    MatrixAsUniform(f,projectionMatrix,perspectiveMatrix);
}

//-------------------------------------------------------------------------------------------------
void LbwShader::drawBuffer(const LbwGeometryBuffer &buffer, const QMatrix4x4 &modelView, int animationState)
{
    if (buffer.mAnimatedBody.isEmpty())
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    const LbwGeometryBuffer::BodyShapes &shapes = buffer.mAnimatedBody.at(0);

    for (int i=0; i<shapes.count(); i++) {

        const LbwGeometryBuffer::Shape &s = shapes[i];

        // Vertices
        f->glBindBuffer( GL_ARRAY_BUFFER, s.vertexBufferId );
        f->glVertexAttribPointer(
                    vertexPosition,
                    4,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(GLfloat) * 10,
                    (void*)0
                    );
        f->glEnableVertexAttribArray( vertexPosition );

        // Normals
        f->glVertexAttribPointer(
                    vertexNormal,
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(GLfloat) * 10,
                    (void*)(sizeof(GLfloat) * 4)
                    );
        f->glEnableVertexAttribArray( vertexNormal );

        // Colors
        f->glVertexAttribPointer(
                    vertexColor,
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    sizeof(GLfloat) * 10,
                    (void*)(sizeof(GLfloat) * 7)
                    );
        f->glEnableVertexAttribArray( vertexColor );

        MatrixAsUniform( f, modelviewMatrix, modelView );

        // Normal view matrix - inverse transpose of modelview.
        QMatrix4x4 normalView = modelView.inverted().transposed();
        MatrixAsUniform(f, normalviewMatrix, normalView );

        // Send element buffer to GPU and draw.
        f->glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, s.elementBufferId );
        f->glDrawElements(
                    s.glType,
                    s.verticesCount,
                    GL_UNSIGNED_SHORT,
                    (void*)0
                    );

        // Clean up
        f->glDisableVertexAttribArray( vertexPosition );
        f->glDisableVertexAttribArray( vertexNormal );
        f->glDisableVertexAttribArray( vertexColor );
    }
}

