#pragma once

#include <QDialog>

class QLineEdit;
class QDialogButtonBox;
class QListWidget;

struct SettingsWindow : public QDialog
{
Q_OBJECT

public:
	SettingsWindow(QWidget *parent = 0);

private slots:
	void storageBrowse();
	void addUser();
	void deleteUser();

private:
	void save();

	QLineEdit *storage_location = nullptr;
	QListWidget *users = nullptr;

	QDialogButtonBox *buttonBox = nullptr;
};
