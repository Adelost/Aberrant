#ifndef GLWIDGET_H
#define GLWIDGET_H

//Qt includes
#include <QGLWidget>

//OpenGL includes
#include <QGLBuffer>
#include <QGLShaderProgram>

//Exteral
#include "InputWidget.h"
#include "Renderer.h"
#include "Cube.h" // contains vertex, normal and texture data

class GLWidget : public QGLWidget, public Renderer
{
    Q_OBJECT
private:
	float zoomLevel;
	bool hasMouseLock;

    QGLShaderProgram shader;
    QGLBuffer *vertexBuffer;
    QGLBuffer *normalBuffer;
    QGLBuffer *indexBuffer;

    QMatrix4x4 mat_proj;
    QMatrix4x4 mat_modelView;

	float rotX;
    float rotY;
	float rotZ;

    bool prepareShaderProgram(const QString& vertexPath, const QString& fragmentPath)
    {
        //Load and compile vertex shader
        bool result = shader.addShaderFromSourceFile(QGLShader::Vertex, vertexPath);
        if (!result)
            qWarning() << shader.log();

        //Load and compile fragmentshader shader
        result = shader.addShaderFromSourceFile(QGLShader::Fragment, fragmentPath );
        if (!result)
            qWarning() << shader.log();

        //Linking
        result = shader.link();
        if (!result)
            qWarning() << "Could not link shader program:" << shader.log();
        return result;
    };
    bool prepareBufferObject(QGLBuffer* buffer, QGLBuffer::UsagePattern usagePattern, const void* data, int dataSize)
    {
        buffer->create();
        buffer->setUsagePattern(usagePattern);
        if (!buffer->bind())
        {
            qDebug() << "Could not bind buffer object to the context";
            return false;
        }
        buffer->allocate( data, dataSize );
        return true;
    };
public:
    GLWidget(const QGLFormat& format, QWidget* parent = 0) : QGLWidget(format, parent) 
    {
		//Make widget non-transparant & draw directly onto screen
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAttribute(Qt::WA_PaintOnScreen);

		//Init variables
		rotX = 0.0f; rotY = 0.0f; rotZ = 0.0f; zoomLevel=5.0f;

		//Create buffers
		vertexBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
		normalBuffer = new QGLBuffer(QGLBuffer::VertexBuffer);
		indexBuffer = new QGLBuffer(QGLBuffer::IndexBuffer);

		setMouseTracking(true);
		hasMouseLock = false;
    };
	~GLWidget()
	{
		//Delete buffers
		delete vertexBuffer;
		delete normalBuffer;
		delete indexBuffer;
	};
	void init(){};
	void update(double delta){};
	void renderFrame()
	{
		this->repaint();
	};
	void cleanUp(){};
	float getXRotation()
	{
		return rotX;
	};
	float getYRotation()
	{
		return rotY;
	};
	float getZRotation()
	{
		return rotZ;
	};

protected:
    virtual void initializeGL()
    {
		//Set sample buffrs
        QGLFormat glFormat = QGLWidget::format();
        if (!glFormat.sampleBuffers())
            qWarning() << "Could not enable sample buffers";

        //Set the clear color to black
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

        //Prepare shaders
        if (!prepareShaderProgram( "root/Shaders/GL/vertex.vert", "root/Shaders/GL/fragment.frag"))
            return;

        //Prepare vertex, normal and index buffers
		if (!prepareBufferObject(vertexBuffer, QGLBuffer::StaticDraw, Cube::vertices(), Cube::vertexDataSize() ) )
            return;
        if (!prepareBufferObject(normalBuffer, QGLBuffer::StaticDraw, Cube::normals(), Cube::normalDataSize() ) )
            return;
        if (!prepareBufferObject(indexBuffer, QGLBuffer::StaticDraw, Cube::indices(), Cube::indexDataSize() ) )
            return;

        //Bind the shader program so that we can associate variables from our application to the shaders
        if (!shader.bind())
        {
            qWarning() << "Could not bind shader program to context";
            return;
        }

        //Enable the "vertex" attribute to bind it to our vertex buffer
        vertexBuffer->bind();
        shader.setAttributeBuffer("vertex", GL_FLOAT, 0, 4);
        shader.enableAttributeArray("vertex");

        //Enable the "normal" attribute to bind it to our texture coords buffer
        normalBuffer->bind();
        shader.setAttributeBuffer("normal", GL_FLOAT, 0, 3);
        shader.enableAttributeArray("normal");

        //Bind the index buffer ready for drawing
        indexBuffer->bind();

		//OpenGL settings
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    };
    virtual void resizeGL(int w, int h)
    {
        //Set viewport to window dimensions
        glViewport(0, 0, w, h);

        //Setup an orthogonal projection matrix
        float viewingAngle = 40.0;
        float nearPlane = 1.0;
        float farPlane = 100.0;
        h = qMax( h, 1 );
        float aspectRatio = float(w)/float(h);
        mat_proj.setToIdentity();
        mat_proj.perspective(viewingAngle, aspectRatio, nearPlane, farPlane);
        shader.setUniformValue("projectionMatrix", mat_proj);
    };
    virtual void paintGL()
    {
        //Setup the modelview matrix
        QMatrix4x4 model;
        model.setToIdentity();

		//Rotation
		bool isTracking = QWidget::hasMouseTracking();
		if(!isTracking)
		{
			/*rotX += 0.4f;
			rotY += 0.4f;
			rotZ += 0.4f;*/
		}
        model.rotate(rotY, 0.0f, 1.0f, 0.0f );
        model.rotate(rotX, 1.0f, 0.0f, 0.0f );
		model.rotate(rotZ, 0.0f, 0.0f, 1.0f );

		//Camera
        QVector3D eyePosition(0.0, 0.0, zoomLevel);
        QVector3D targetPosition(0.0, 0.0, 0.0 );
        QVector3D upDirection(0.0, 1.0, 0.0 );
        mat_modelView.setToIdentity();
        mat_modelView.lookAt(eyePosition, targetPosition, upDirection );
        mat_modelView = mat_modelView * model;

        shader.setUniformValue("modelViewMatrix", mat_modelView);

		//Clear the buffer with the current clearing color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw stuff
		glDrawElements(GL_TRIANGLES, Cube::indexCount(), GL_UNSIGNED_INT, 0);
	};
	void mousePressEvent(QMouseEvent *e)
	{
		// Lock/release mouse to app
		if(e->button() == Qt::LeftButton)
			toggleMouseLock();
	};
	void mouseMoveEvent(QMouseEvent *e)
	{
		//Rotate based on mouse movement
		if(hasMouseLock)
		{
			//Calculate change (delta) in mouse pos
			QPoint mouseAnchor = QWidget::mapToGlobal(QPoint(this->width()*0.5f,this->height()*0.5f));
			QCursor::setPos(mouseAnchor.x(), mouseAnchor.y()); // anchor mouse again
			int dx = e->globalX() - mouseAnchor.x();
			int dy = e->globalY() - mouseAnchor.y();


			//Send event
			emit slot_mouseMove(dx, dy);
		}
		else {
			//Tweakbar
			TwMouseMotion(e->x(), e->y());
		}
	};
private:
	void toggleMouseLock()
	{
		// Locking/release mouse cursor to widget
		hasMouseLock = !hasMouseLock;
		if(hasMouseLock)
		{
			//Hide cursor and set new anchor point
			QWidget::setCursor(Qt::BlankCursor);
			QWidget::grabMouse();

			//Move mouse to middle
			QPoint mouseAnchor = QWidget::mapToGlobal(QPoint(this->width()*0.5f,this->height()*0.5f));
			QCursor::setPos(mouseAnchor.x(), mouseAnchor.y()); // anchor mouse again
		}
		else
		{
			//Show cursor again and release mouse cursor
			QWidget::setCursor(Qt::ArrowCursor);	
			QWidget::releaseMouse();
		}
	};
public slots:
	void slot_mouseMove(int dx, int dy)
	{
		float x = 1.0*0.25f*(float)dx;
		float y = 1.0*0.25f*(float)dy;

		//Rotate
		rotX += y;
		rotY += x;
	};
	void slot_mouseScroll(int dx)
	{
		float radius = dx * 0.5f;
		zoomLevel -= radius;
	};
};

#endif // GLWIDGET_H
