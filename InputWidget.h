#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

#include <QWidget.h>
#include <QResizeEvent>
#include <QMouseEvent> // needed to grabb mouse input

#include <AntTweakBar.h>

class InputWidget : public QWidget
{
	Q_OBJECT

public:
	InputWidget(QWidget* parent = 0, Qt::WFlags flags = 0) : QWidget(parent, flags)
	{
		//Set mouse tracking
		
	};

	~InputWidget(){};

protected:
	void mousePressEvent(QMouseEvent *e)
	{
		TwMouseAction action = TW_MOUSE_PRESSED;
		TwMouseButtonID button = TW_MOUSE_LEFT;
		TwMouseButton(action, button);
	};
	void mouseMoveEvent(QMouseEvent *e)
	{
		TwMouseMotion(e->x(), e->y());

		//Rotate based on mouse movement
		//bool isMouseTracking = QWidget::hasMouseTracking(); // only listen when mouse position is being tracked
		//if(isMouseTracking)
		//{
		//	//Calculate change (delta) in mouse pos
		//	int dx = e->globalX() - mouseAnchor.x();
		//	int dy = e->globalY() - mouseAnchor.y();
		//	QCursor::setPos(mouseAnchor.x(), mouseAnchor.y()); // anchor mouse again

		//	//Send event
		//	emit signal_mouseMove(dx, dy);
		//}
	};
	void mouseReleaseEvent(QMouseEvent *e)
	{
		TwMouseAction action = TW_MOUSE_RELEASED;
		TwMouseButtonID button = TW_MOUSE_LEFT;
		TwMouseButton(action, button);
	};
	void wheelEvent(QWheelEvent *e)
	{
		int delta = e->delta();
		if(delta > 0)
			emit signal_mouseScroll(1);
		if(delta < 0)
			emit signal_mouseScroll(-1);
	};

signals:
	void signal_mouseMove(int dx, int dy);
	void signal_mouseScroll(int dx);
};

#endif