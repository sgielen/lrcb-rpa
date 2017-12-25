#include "loginwindow.hpp"
#include "assessmentwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	LoginWindow *login = new LoginWindow;
	AssessmentWindow *main = nullptr;

	QObject::connect(login, &LoginWindow::loginPerformed,
	[&](QString loginName) {
		login->deleteLater();
		main = new AssessmentWindow();
		main->loginPerformed(loginName);
	});

	login->show();

	return app.exec();
}
