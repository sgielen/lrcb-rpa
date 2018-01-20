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
class QFrame;

struct AssessmentWindow : public QMainWindow
{
Q_OBJECT

public:
	AssessmentWindow(bool skipSetup, AssessmentScoreLayout score_layout, QWidget *parent = 0);
	~AssessmentWindow();

public slots:
	void loginPerformed(QString name);

private slots:
	void startAssessment();
	void switchCamera();
	void takeCapture(bool);
	void imageSaved(int id, const QString &fileName);
	void scoreEntered();
	void saveAssessment();
	void countDown();
	void finishAssessment();

private:
	bool skipSetup;

	QString userName;

	QLabel *title = nullptr;
	QLabel *commandLine = nullptr;
	QPushButton *finishAssessmentBtn = nullptr;

	QComboBox *cameraBox = nullptr;
	QList<QCameraInfo> cameraInfo;

	QCameraInfo currentCamera;
	QCamera *camera = nullptr;
	QCameraImageCapture *imageCapture = nullptr;

	QFrame *webcamFrame = nullptr;
	QLabel *webcamImage = nullptr;
	QLabel *webcamImageText = nullptr;
	QPushButton *webcamAcceptButton = nullptr;
	QString lastCaptureFilename;

	const int autoAcceptSeconds = 4;
	int autoAcceptSecondsRemaining = 0;
	QTimer *autoAccept = nullptr;

	QLabel *savedLabel = nullptr;
	QTimer *savedLabelHide = nullptr;

	AssessmentScore *score_input = nullptr;
};
