#include "loginwindow.hpp"
#include "assessmentwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	LoginWindow *login = new LoginWindow;
	AssessmentWindow main;

	QObject::connect(login, &LoginWindow::loginPerformed,
	                 &main,  &AssessmentWindow::loginPerformed);
	QObject::connect(login, &LoginWindow::loginPerformed,
	                 login, &LoginWindow::deleteLater);

	login->show();

	return app.exec();
}
