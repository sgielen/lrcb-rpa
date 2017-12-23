#include "util.hpp"
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QSettings>

void addTitleToWidget(QWidget *widget, QString title) {
	auto *label = new QLabel(title, widget);
	auto font = label->font();
	font.setPointSize(16);
	font.setBold(true);
	label->setFont(font);
	widget->layout()->addWidget(label);

	auto *line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	widget->layout()->addWidget(line);
}

static const char orgName[] = "LRCB";
static const char appName[] = "RPA";

void loadSettings(Settings &settings) {
	QSettings s(orgName, appName);

	auto storage = s.value("storage_location").toString();
	if(storage.isEmpty()) {
		storage = QDir::homePath() + "/Desktop/LRCB/RPA";
	}
	settings.storageLocation = storage;

	settings.users = s.value("users").toStringList();
}

void saveSettings(Settings const &settings) {
	QSettings s(orgName, appName);
	s.setValue("storage_location", settings.storageLocation.absolutePath());
	s.setValue("users", settings.users);
}
