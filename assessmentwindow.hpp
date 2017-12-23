#pragma once

#include <QMainWindow>

struct AssessmentWindow : public QMainWindow
{
Q_OBJECT

public:
	AssessmentWindow(QWidget *parent = 0);
	~AssessmentWindow();

public slots:
	void loginPerformed(QString name);

private:
	QString userName;
};
