#include "loginwindow.hpp"
#include "assessmentwindow.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <iostream>

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

	QCommandLineOption layoutStyle("l", "Use this score bar layout", "layout", "greenred");
	parser.addOption(layoutStyle);

	parser.process(app);

	bool skipDialogs = parser.isSet(skipDialogsOption);

	QString layoutstr = parser.value(layoutStyle);
	AssessmentScoreLayout layout = AssessmentScoreLayout::GreenRed;

	if(layoutstr.toLower() == "greenred") {
		layout = AssessmentScoreLayout::GreenRed;
	} else if(layoutstr.toLower() == "black") {
		layout = AssessmentScoreLayout::Black;
	} else if(layoutstr.toLower() == "greenwhitered") {
		layout = AssessmentScoreLayout::GreenWhiteRed;
	} else {
		std::cerr << "Didn't understand option for -l. Supported: greenred, black, greenwhitered.\n";
		exit(1);
	}

	// enable larger point size of fonts
	QFont defaultFont = QApplication::font();
	defaultFont.setPointSize(14);
	qApp->setFont(defaultFont);

	LoginWindow *login = nullptr;
	AssessmentWindow *main = nullptr;

	if(!skipDialogs) {
		login = new LoginWindow;

		QObject::connect(login, &LoginWindow::loginPerformed,
		[&](QString loginName) {
			login->hide();
			login->deleteLater();
			login = nullptr;
			main = new AssessmentWindow(false, layout);
			main->loginPerformed(loginName);
		});

		login->show();
	} else {
		main = new AssessmentWindow(true, layout);
		main->loginPerformed("");
	}

	auto res = app.exec();
	delete login;
	delete main;
	return res;
}
