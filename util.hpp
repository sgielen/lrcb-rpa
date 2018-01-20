#pragma once

#include <QString>
#include <QDir>

class QLabel;
class QWidget;

QLabel *addTitleToWidget(QWidget *widget, QString title);

struct Settings {
	QDir storageLocation;
	QStringList users;
	QString lastUsedCameraName;
};

void loadSettings(Settings &);
void saveSettings(Settings const &s);
