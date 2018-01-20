#include "loginwindow.hpp"
#include "settingswindow.hpp"
#include "util.hpp"

#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>

LoginWindow::LoginWindow(QWidget *parent)
: QMainWindow(parent)
{
	setWindowTitle("LRCB-RPA Login");
	auto screenGeom = QApplication::desktop()->availableGeometry(this);
	resize(screenGeom.width() * float(0.35), screenGeom.height() * float(0.40));

	auto *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(QIcon(":/info.png"), tr("&About"), this, &LoginWindow::about);
	fileMenu->addAction(QIcon(":/settings.png"), tr("&Settings"), this, &LoginWindow::settings);
	fileMenu->addSeparator();
	fileMenu->addAction(QIcon(":/exit.xpm"), tr("&Quit"), this, &QWidget::close);

	setCentralWidget(new QWidget);
	centralWidget()->setLayout(new QVBoxLayout);

	addTitleToWidget(centralWidget(), "Log in");

	centralWidget()->layout()->addWidget(new QLabel("Select your name from the list below.", this));

	users = new QListWidget(this);
	centralWidget()->layout()->addWidget(users);
	Settings s;
	loadSettings(s);
	for(auto &user : s.users) {
		users->addItem(user);
	}

	QPushButton *login = new QPushButton("Log in", this);
	centralWidget()->layout()->addWidget(login);

	connect(login, &QPushButton::pressed, this, &LoginWindow::performLogin);
}

void LoginWindow::about()
{
	QMessageBox::information(this, "About LRCB-RPA",
		"This is the Radiology Performance Assessment tool, written for "
		"the Dutch expert centre for screening (LRCB) by Sjors Gielen.\n\n"
		"Version: " __DATE__ " " __TIME__ "\n\n"
		"Some icons made by Freepik, licensed by CC BY 3.0.\n"
		"Some icons made by FatCow, licenesd by CC BY 3.0.\n"
	);
}

void LoginWindow::settings()
{
	hide();
	SettingsWindow set;
	set.exec();
	show();

	users->clear();
	Settings s;
	loadSettings(s);
	for(auto &user : s.users) {
		users->addItem(user);
	}
}

void LoginWindow::performLogin()
{
	if(users->selectedItems().count() == 0) {
		return;
	}

	QString user = users->item(users->currentRow())->text();
	emit loginPerformed(user);
}
