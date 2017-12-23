#include "settingswindow.hpp"
#include "util.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpacerItem>

#include <iostream>

SettingsWindow::SettingsWindow(QWidget *parent)
: QDialog(parent, Qt::Dialog)
{
	setWindowTitle("LRCB-RPA - Settings");

	resize(500, 350);

	setLayout(new QVBoxLayout);

	addTitleToWidget(this, "Application configuration");

	// Storage location
	layout()->addWidget(new QLabel("Storage location", this));
	QLayout *storage_layout = new QHBoxLayout;
	storage_location = new QLineEdit(this);
	storage_location->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QPushButton *storage_browse = new QPushButton("Browse", this);
	storage_layout->addWidget(storage_location);
	storage_layout->addWidget(storage_browse);
	layout()->addItem(storage_layout);
	connect(storage_browse, &QPushButton::pressed, this, &SettingsWindow::storageBrowse);

	// List of users
	layout()->addWidget(new QLabel("User list", this));

	auto *list_change = new QVBoxLayout;
	auto *add_btn = new QPushButton(QPixmap(":/add.png"), "", this);
	auto *del_btn = new QPushButton(QPixmap(":/delete.png"), "", this);
	list_change->addWidget(add_btn);
	list_change->addWidget(del_btn);

	auto *list_outer = new QHBoxLayout;
	layout()->addItem(list_outer);
	users = new QListWidget(this);
	list_outer->addWidget(users);
	list_outer->addItem(list_change);

	connect(add_btn, &QPushButton::clicked, this, &SettingsWindow::addUser);
	connect(del_btn, &QPushButton::clicked, this, &SettingsWindow::deleteUser);

	// Save button
	dynamic_cast<QVBoxLayout*>(layout())->addStretch();

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	layout()->addWidget(buttonBox);

	connect(this, &QDialog::accepted, this, &SettingsWindow::save);

	// Load values from settings
	Settings s;
	loadSettings(s);
	storage_location->setText(s.storageLocation.absolutePath());
	for(auto &user : s.users) {
		users->addItem(user);
	}
}

void SettingsWindow::storageBrowse() {
	storage_location->setText(
		QFileDialog::getExistingDirectory(this,
			"Select storage location",
			storage_location->text()));
}

void SettingsWindow::save() {
	Settings s;
	loadSettings(s);
	s.storageLocation = storage_location->text();
	s.users.clear();
	for(size_t i = 0; i < users->count(); ++i) {
		s.users.append(users->item(i)->text());
	}
	saveSettings(s);
}

void SettingsWindow::addUser() {
	QString user = QInputDialog::getText(this,
		"Enter the name of the user to add",
		"User name:");
	if(!user.isEmpty()) {
		users->addItem(user);
	}
}

void SettingsWindow::deleteUser() {
	for(auto item : users->selectedItems()) {
		delete item;
	}
}
