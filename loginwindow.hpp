#pragma once

#include <QWidget>
#include <QMainWindow>

class QListWidget;
class QPushButton;

struct LoginWindow : public QMainWindow
{
Q_OBJECT

public:
	explicit LoginWindow(QWidget *parent = 0);

signals:
	void loginPerformed(QString name);

private slots:
	void about();
	void settings();
	void performLogin();

private:
	QListWidget *users = nullptr;
};
