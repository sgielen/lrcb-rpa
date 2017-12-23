#include "loginwindow.hpp"
#include "settingswindow.hpp"
#include "util.hpp"

#include <QLabel>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>

LoginWindow::LoginWindow(QWidget *parent)
: QMainWindow(parent)
{
	setWindowTitle("LRCB-RPA Login");
	resize(500, 350);

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

	login = new QPushButton("Log in", this);
	centralWidget()->layout()->addWidget(login);

	connect(login, &QPushButton::pressed, this, &LoginWindow::performLogin);
}

void LoginWindow::about()
{
	// TODO: show about screen
	// TODO: show "Some icons made by Freepik, licensed by CC BY 3.0"
	// TODO: show "Some icons made by FatCow, licensed by CC BY 3.0"
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
	QString user = users->item(users->currentRow())->text();
	emit loginPerformed(user);
}
