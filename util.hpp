#pragma once

#include <QString>
#include <QDir>

class QWidget;

void addTitleToWidget(QWidget *widget, QString title);

struct Settings {
	QDir storageLocation;
	QStringList users;
};

void loadSettings(Settings &);
void saveSettings(Settings const &s);
