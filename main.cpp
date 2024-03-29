
#include "MainWindow.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	//Detect memory leaks
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	//Create mainwindow
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	//Run Qt
	return a.exec();
}
