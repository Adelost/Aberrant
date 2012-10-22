#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//Qt includes
#include <QMainWindow>
#include <QTime>
#include <QTimer> // needed to implement framrate
#include <QMouseEvent> // needed to grabb mouse input

//Form include
#include "ui_MainWindow.h"

//External
#include "GLWidget.h"
#include "DXWidget.h"
#include <AntTweakBar.h>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	// Mainwindow
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0) : QMainWindow(parent, flags)
	{
		// Create UI generated from XML file
		ui.setupUi(this);
		this->setWindowTitle("Aberrant");
		resize(800, 600);

		// Connect functionality with buttons
		connect(ui.actionDirectX, SIGNAL(toggled(bool)), this, SLOT(createDXWidget(bool)));
		connect(ui.actionOpenGL, SIGNAL(toggled(bool)), this, SLOT(createGLWidget(bool)));
		connect(ui.actionCapFPS, SIGNAL(toggled(bool)), this, SLOT(toggleCapFPS(bool)));
		connect(ui.actionFullScreen, SIGNAL(toggled(bool)), this, SLOT(toggleFullScreen(bool)));
		connect(ui.quit, SIGNAL(triggered()), this, SLOT(close()));

		// Create updatetimer
		updateTimer = new QTimer(this);
		connect(updateTimer, SIGNAL(timeout()), this, SLOT(gameLoop()));
	
		// Create dxwidget
		ui.actionDirectX->setChecked(true);
		ui.actionCapFPS->setChecked(true);
	};
	~MainWindow()
	{
	};

private:
	Ui::MainWindowClass ui;
	QPoint mouseAnchor; // used to anchor mouse in one place
	QWidget *renderWidget;
	Renderer *renderer;
	QTimer *updateTimer;

protected:
	 void keyPressEvent(QKeyEvent *e)
	 {
		 if (e->key() == Qt::Key_Escape)
		 {
			 ui.actionFullScreen->setChecked(false);
		 }

		 if(e->key() == Qt::Key_W)
		 {
		 }
		 if(e->key() == Qt::Key_A)
		 {
		 }
		 if(e->key() == Qt::Key_S)
		 {
		 }
		 if(e->key() == Qt::Key_D)
		 {
		 }
	 };

public slots:
	// Puts custom DXWidget in Qt mainwindow
	void createDXWidget(bool isChecked)
	{
		if(isChecked)
		{
			// Disable OpenGL
			ui.actionOpenGL->setEnabled(false);

			// Create DirectX
			renderWidget = new DXWidget(this);
			if(renderer)
			{
				//Connect to mainwindow
				this->setCentralWidget(renderWidget);
				renderer = dynamic_cast<Renderer*>(renderWidget);
				connect(renderWidget, SIGNAL(signal_fpsChanged(QString)), this, SLOT(setTitle(QString)));

				//Start timer
				updateTimer->start();
			}
		}
		else
		{
			// Stop timer
			updateTimer->stop();

			// Delete DirectX
			delete renderWidget;

			// Enable OpenGL
			ui.actionOpenGL->setEnabled(true);
		}
	};
	void createGLWidget(bool isChecked)
	{
		if(isChecked)
		{
			// Disable DirectX
			ui.actionDirectX->setEnabled(false);

			// Specify an OpenGL 3.3 format using the Core profile.
			// That is, no old-school fixed pipeline functionality
			QGLFormat glFormat;
			glFormat.setVersion(3, 3);
			// glFormat.setProfile( QGLFormat::CoreProfile ); // Requires >Qt-4.8.0
			/** \bug Texture upload fails with core profile. See line 2560 in qgl.cpp */
			glFormat.setProfile(QGLFormat::CompatibilityProfile );
			glFormat.setSampleBuffers(true);
			renderWidget = new GLWidget(glFormat, this);
			if(renderer)
			{
				// Connect to mainwindow
				this->setCentralWidget(renderWidget);
				renderer = dynamic_cast<Renderer*>(renderWidget);

				// Connect input with GLWidget
				connect(this, SIGNAL(signal_mouseMove(int, int)), renderWidget, SLOT(slot_mouseMove(int, int)));
				connect(this, SIGNAL(signal_mouseScroll(int)), renderWidget, SLOT(slot_mouseScroll(int)));

				// Start timer
				updateTimer->start();
			}
		}
		else
		{
			// Stop timer
			updateTimer->stop();

			// Remove widget
			delete renderWidget;

			// Enable DX
			ui.actionDirectX->setEnabled(true);
		}
	};

	// Set mainwindow to fullscreen
	void toggleFullScreen(bool isChecked)
	{
		if(isChecked)
		{
			ui.mainToolBar->hide();
			this->showFullScreen();
			
		}
		else
		{
			ui.mainToolBar->show();
			this->showNormal();
		}
	};
	void toggleCapFPS(bool isChecked)
	{
		if(isChecked)
		{
			updateTimer->setInterval(10);
		}
		else
		{
			updateTimer->setInterval(0);
		}
	};
	void gameLoop()
	{
		// Render frame
		renderer->renderFrame();
	};
	void setTitle(QString title)
	{
		this->setWindowTitle("Aberrant " + title);
	};
};

#endif // MAINWINDOW_H
