#include "loginwindow.hpp"
#include "assessmentwindow.hpp"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("lrcb-rpa");
	QApplication::setApplicationVersion("1.0");

	QCommandLineParser parser;
	parser.setApplicationDescription("LRCB Radiologist performance analysis");
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption skipDialogsOption("s", "Skip dialogs and start assessment immediately");
	parser.addOption(skipDialogsOption);

	parser.process(app);

	bool skipDialogs = parser.isSet(skipDialogsOption);

	LoginWindow *login = nullptr;
	AssessmentWindow *main = nullptr;

	if(!skipDialogs) {
		login = new LoginWindow;

		QObject::connect(login, &LoginWindow::loginPerformed,
		[&](QString loginName) {
			login->deleteLater();
			main = new AssessmentWindow(false);
			main->loginPerformed(loginName);
		});

		login->show();
	} else {
		main = new AssessmentWindow(true);
		main->loginPerformed("");
	}

	return app.exec();
}
