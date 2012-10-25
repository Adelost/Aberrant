#ifndef DXWIDGET_H
#define DXWIDGET_H

#include <QWidget.h>
#include <QResizeEvent>
#include <QMouseEvent> // needed to grabb mouse input

#include <AntTweakBar.h>

//External
#include "Renderer.h"
#include "DXRenderer.h"
#include "GameTimer.h"


class DXWidget : public QWidget, public Renderer
{
	Q_OBJECT

private:
	GameTimer timer;
	DXRenderer *renderer;
	bool hasMouseLock;
	bool* doMSAA;

public:
	DXWidget(QWidget* parent = 0, Qt::WFlags flags = 0) : QWidget(parent, flags)
	{
		//Make widget non-transparant & draw directly onto screen
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAttribute(Qt::WA_PaintOnScreen);

		doMSAA = new bool(false);

		//Init self
		setMouseTracking(true);
		hasMouseLock = false;
		init();

		
	};
	~DXWidget()
	{
		delete renderer;
	};
	void init()
	{
		// Create renderer
		renderer = new DXRenderer();
		renderer->init(this->winId(), doMSAA);

		// Get input
		connect(this, SIGNAL(signal_mouseMove(int, int)), this, SLOT(slot_mouseMove(int, int)));
		connect(this, SIGNAL(signal_mouseScroll(int)), this, SLOT(slot_mouseScroll(int)));
	
		// Start timer
		timer.reset();
	};
	void renderFrame()
	{
		timer.tick();
		calcFPS();
		renderer->update(timer.getDeltaTime());
		renderer->renderFrame();
	};
	QPaintEngine* paintEngine() const {return 0;}; //Overrides Qt paint engine; prevents flicker

protected:
	void paintEvent(QPaintEvent* e){};
	void resizeEvent(QResizeEvent* e)
	{
		QWidget::resizeEvent(e);
		int width = size().width();
		int height = size().height();
		renderer->onResize(width, height);
	};
	void mousePressEvent(QMouseEvent *e)
	{
		if(tweakbar_handleMouseEvent(TW_MOUSE_PRESSED, e))
			return; // event was handled by tweakbar

		// Lock/release mouse to app
		if(e->button() == Qt::LeftButton)
			toggleMouseLock();
	};
	void mouseReleaseEvent(QMouseEvent *e)
	{
		if(tweakbar_handleMouseEvent(TW_MOUSE_RELEASED, e))
			return; // event was handled by tweakbar
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
			emit signal_mouseMove(dx, dy);
		}
		else {
			//Tweakbar
			TwMouseMotion(e->x(), e->y());
		}
	};
	void wheelEvent(QWheelEvent *e)
	{
		int delta = e->delta();
		if(delta > 0)
			emit signal_mouseScroll(1);
		if(delta < 0)
			emit signal_mouseScroll(-1);
	};
	//void mouseDoubleClickEvent(QMouseEvent *e)
	//{
	//	
	//};
	
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
	int tweakbar_handleMouseEvent(TwMouseAction action, QMouseEvent *e)
	{
		TwMouseButtonID button;
		if(e->button() == Qt::LeftButton)
			button = TW_MOUSE_LEFT;
		if(e->button() == Qt::RightButton)
			button = TW_MOUSE_RIGHT;
		if(e->button() == Qt::MiddleButton)
			button = TW_MOUSE_MIDDLE;

		return TwMouseButton(action, button);
	};
	void calcFPS()
	{
		static int nrOfFrames = 0;
		static float timeElapsed = 0.0f;
		nrOfFrames++;

		//Compute averages over one second period.
		if((timer.getTotalTime()-timeElapsed) >= 1.0f)
		{
			float fps = (float)nrOfFrames; // fps = frameCnt / 1
			float ms_pf = 1000.0f/fps;

			std::wostringstream outs;   
			outs.precision(6);
			outs << L"fps: " << fps << L"    " 
				<< L"Frame Time: " << ms_pf << L" (ms)";

			// QString stats = "FPS: ";
			QString stats;
			stats = "FPS:  %1  Frame Time:  %2 (ms)";
			stats = stats.arg(fps).arg(ms_pf);

			//Send signal
			emit signal_fpsChanged(stats);

			// Reset for next average.
			nrOfFrames = 0;
			timeElapsed += 1.0f;
		}
	}

public slots:
	void slot_mouseMove(int dx, int dy)
	{
		// Set 1 pixel = 0.25 degrees
		float x = XMConvertToRadians(0.20f*(float)dx);
		float y = XMConvertToRadians(0.20f*(float)dy);

		// Rotate camera
		renderer->mCam.Pitch(y);
		renderer->mCam.RotateY(x);
	};
	void slot_mouseScroll(int dx)
	{
		float radius = dx * -0.5f;
		renderer->mCam.ModifyHeight(radius);
	};

signals:
	void signal_mouseScroll(int dx);
	void signal_mouseMove(int dx, int dy);
	void signal_fpsChanged(QString value);
};

#endif