#pragma once

#include <QMainWindow>
#include <QList>
#include <QCameraInfo>
#include <QTemporaryDir>
#include <QFile>

#include "assessmentscore.hpp"

class QCamera;
class QCameraImageCapture;
class QComboBox;
class QLabel;
class QPushButton;
class QFrame;
class QCheckBox;
class QSlider;

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
	void focusSettingsChanged();

private:
	bool skipSetup;
	bool assessmentStarted = false;

	QString userName;

	QTemporaryDir captureDir;
	uint64_t imageCounter = 0;

	QFile logfile;

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
	QPushButton *webcamAcceptButton = nullptr;
	QPushButton *webcamStopCountdownButton = nullptr;
	QPushButton *webcamRetakeButton = nullptr;
	QString lastCaptureFilename;

	//QCheckBox *webcamAutoFocus = nullptr;
	QSlider *webcamFocusSetting = nullptr;
	QFrame *webcamFocusFrame = nullptr;
	QFrame *webcamLeftFrame = nullptr;

	bool capturing = false;
	bool captureOne = false;

	const int autoAcceptSeconds = 4;
	int autoAcceptSecondsRemaining = 0;
	QTimer *autoAccept = nullptr;

	QLabel *savedLabel = nullptr;
	QTimer *savedLabelHide = nullptr;

	AssessmentScore *score_input = nullptr;
};
