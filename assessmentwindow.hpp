#pragma once

#include <QMainWindow>
#include <QList>
#include <QCameraInfo>

#include "assessmentscore.hpp"

class QCamera;
class QCameraImageCapture;
class QComboBox;
class QLabel;
class QPushButton;

struct AssessmentWindow : public QMainWindow
{
Q_OBJECT

public:
	AssessmentWindow(bool skipSetup, AssessmentScoreLayout score_layout, QWidget *parent = 0);
	~AssessmentWindow();

public slots:
	void loginPerformed(QString name);

private slots:
	void about();
	void startAssessment();
	void switchCamera();
	void takeCapture(bool);
	void imageSaved(int id, const QString &fileName);
	void scoreEntered();

private:
	bool skipSetup;

	QString userName;

	QLabel *title = nullptr;
	QLabel *commandLine = nullptr;
	QPushButton *assessmentBtn = nullptr;

	QComboBox *cameraBox = nullptr;
	QList<QCameraInfo> cameraInfo;

	QCameraInfo currentCamera;
	QCamera *camera = nullptr;
	QCameraImageCapture *imageCapture = nullptr;

	QLabel *webcamImage = nullptr;

	AssessmentScore *score_input = nullptr;
};
