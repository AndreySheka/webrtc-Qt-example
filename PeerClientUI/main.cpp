#include "peerclientui.h"
#include "talk\base\thread.h"
#include "talk/base/win32socketserver.h"
#include <QtWidgets/QApplication>
//Initialize winsocket


int main(int argc, char *argv[])
{
	talk_base::Win32Thread w32_thread;
	talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);
	QApplication::addLibraryPath("./plugins");
	QApplication a(argc, argv);
	PeerClientUI w;
	w.show();
	return a.exec();
}
