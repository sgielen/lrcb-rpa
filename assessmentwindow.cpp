#include "assessmentwindow.hpp"
#include "assessmentscore.hpp"
#include "excel.hpp"
#include "util.hpp"

#include <QApplication>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <cassert>

AssessmentWindow::AssessmentWindow(bool s, AssessmentScoreLayout input_layout, QWidget *parent)
: QMainWindow(parent)
, skipSetup(s)
{
	setWindowTitle("LRCB-RPA - Assessment");

	showFullScreen();

	setCentralWidget(new QWidget);
	QVBoxLayout *thisLayout = new QVBoxLayout;
	centralWidget()->setLayout(thisLayout);
	title = addTitleToWidget(centralWidget(), "Assessment configuration");

	//commandLine = new QLabel("First, calibrate the webcam so that the study information is clearly readable.", this);
	//thisLayout->addWidget(commandLine);

	cameraInfo = QCameraInfo::availableCameras();
	if(cameraInfo.count() == 0) {
		QLabel *cameras = new QLabel("Error: No cameras available -- connect a camera and restart the application.", this);
		thisLayout->addWidget(cameras);
		thisLayout->addStretch();
		finishAssessmentBtn = new QPushButton("Cancel assessment session and exit", this);
		thisLayout->addWidget(finishAssessmentBtn);

		connect(finishAssessmentBtn, &QPushButton::pressed, this, &QMainWindow::close);
		return;
	}

	cameraBox = new QComboBox(this);

	Settings set;
	loadSettings(set);
	for(auto camera : cameraInfo) {
		cameraBox->addItem(camera.description());
		if (set.lastUsedCameraName == camera.description()) {
			cameraBox->setCurrentIndex(cameraBox->count() - 1);
		}
	}

	thisLayout->addWidget(cameraBox);
	if(cameraInfo.count() == 1) {
		// if there's only one camera, don't show the combobox at all
		cameraBox->hide();
	}

	connect(cameraBox, &QComboBox::currentTextChanged, this, &AssessmentWindow::switchCamera);

	webcamImage = new QLabel(this);
	webcamImage->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	QFrame *webcamButtonsFrame = new QFrame(this);
	webcamButtonsFrame->setLayout(new QVBoxLayout);

	webcamAcceptButton = new QPushButton(QIcon(":/tick.png"), "Accept", this);
	webcamAcceptButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	webcamButtonsFrame->layout()->addWidget(webcamAcceptButton);
	webcamRetakeButton = new QPushButton(QIcon(":/clock_stop.png"), "Stop countdown", this);
	webcamRetakeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	webcamButtonsFrame->layout()->addWidget(webcamRetakeButton);
	autoAccept = new QTimer(this);
	autoAccept->setInterval(1000);
	autoAccept->setSingleShot(false);
	connect(autoAccept, &QTimer::timeout, this, &AssessmentWindow::countDown);

	connect(webcamRetakeButton, &QPushButton::pressed, this, [this]() {
		autoAccept->stop();
		webcamAcceptButton->setText("Accept");
	});

	// But for now, temporarily re-purpose the accept button to start the assessment session
	webcamAcceptButton->setText("Start assessment");
	webcamAcceptButton->setEnabled(false);
	webcamRetakeButton->setEnabled(false);
	connect(webcamAcceptButton, &QPushButton::pressed, this, &AssessmentWindow::startAssessment);

	webcamFrame = new QFrame(this);
	webcamFrame->setLayout(new QHBoxLayout);
	webcamFrame->layout()->addWidget(webcamImage);
	webcamFrame->layout()->addWidget(webcamButtonsFrame);

	score_input = new AssessmentScore(input_layout, this);
	score_input->hide();

	// Put the webcam frame above the score input, and make them the same size
	score_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	webcamFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	auto screenGeom = QApplication::desktop()->availableGeometry(this);
	score_input->setFixedSize(screenGeom.width() * float(0.9), screenGeom.height() * float(0.1));
	webcamFrame->setFixedWidth(screenGeom.width() * float(0.9));

	thisLayout->addWidget(webcamFrame);
	thisLayout->setAlignment(webcamFrame, Qt::AlignHCenter);

	thisLayout->addWidget(score_input);
	thisLayout->setAlignment(score_input, Qt::AlignHCenter);

	thisLayout->addStretch();

	savedLabel = new QLabel(this);
	savedLabel->setTextFormat(Qt::RichText);
	savedLabel->setText("<img src=\":/diskette.png\"/> Assessment saved!");
	savedLabel->setStyleSheet("QLabel { background-color: #dff0d8; border: 1px solid #a2d48f; }");
	savedLabel->setAlignment(Qt::AlignHCenter);
	savedLabel->hide();
	thisLayout->addWidget(savedLabel);

	finishAssessmentBtn = new QPushButton("Cancel assessment session and exit", this);
	thisLayout->addWidget(finishAssessmentBtn);

	connect(finishAssessmentBtn, &QPushButton::pressed, this, &AssessmentWindow::finishAssessment);

	switchCamera();

	if(skipSetup) {
		startAssessment();
	}
}

AssessmentWindow::~AssessmentWindow()
{
	QFile lastCaptureFile(lastCaptureFilename);
	if(!lastCaptureFilename.isEmpty() && lastCaptureFile.exists()) {
		lastCaptureFile.remove();
	}
}

void AssessmentWindow::loginPerformed(QString name)
{
	userName = name;
	show();
}

void AssessmentWindow::startAssessment()
{
	cameraBox->hide();
	webcamAcceptButton->setText("Accept");
	webcamAcceptButton->setEnabled(false);
	webcamRetakeButton->setEnabled(false);
	assessmentStarted = true;

	title->setText("Assessment");
	//commandLine->setText("Drag your finger over the bar to enter a confidence level:");

	score_input->unsetScore();
	score_input->show();
	connect(score_input, &AssessmentScore::scoreEntered, this, &AssessmentWindow::scoreEntered);

	finishAssessmentBtn->setText("Finish assessment session and exit");
	finishAssessmentBtn->setEnabled(true);
	webcamAcceptButton->disconnect(this);
	connect(webcamAcceptButton, &QPushButton::pressed, this, &AssessmentWindow::saveAssessment);
}

void AssessmentWindow::scoreEntered()
{
	finishAssessmentBtn->setEnabled(false);
	webcamAcceptButton->setEnabled(true);
	webcamRetakeButton->setEnabled(true);

	assert(camera);
	assert(imageCapture);

	autoAcceptSecondsRemaining = autoAcceptSeconds + 1;
	autoAccept->start();
	countDown();

	// TODO: make behaviour of imageSaved() different if we take a capture
	// because the score is entered?
	camera->start();
}

void AssessmentWindow::countDown()
{
	autoAcceptSecondsRemaining--;
	if(autoAcceptSecondsRemaining == 0) {
		webcamAcceptButton->setText("Accept");
		webcamAcceptButton->click();
	} else {
		webcamAcceptButton->setText("Accept (" + QString::number(autoAcceptSecondsRemaining) + ")");
	}
}

void AssessmentWindow::switchCamera()
{
	int currentRow = cameraBox->currentIndex();
	if(cameraInfo.at(currentRow) != currentCamera) {
		// the current camera changed, so restart the image fetcher on another camera
		currentCamera = cameraInfo.at(currentRow);

		Settings set;
		loadSettings(set);
		set.lastUsedCameraName = cameraBox->currentText();
		saveSettings(set);

		if(imageCapture) {
			delete imageCapture;
		}

		if(camera) {
			delete camera;
		}

		camera = new QCamera(currentCamera);

		imageCapture = new QCameraImageCapture(camera);
		imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToFile);

		camera->setCaptureMode(QCamera::CaptureStillImage);

		connect(imageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &AssessmentWindow::takeCapture);
		connect(imageCapture, &QCameraImageCapture::imageSaved, this, &AssessmentWindow::imageSaved);

		connect(imageCapture, QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
		    [=](int, QCameraImageCapture::Error, const QString &errorString){
			QMessageBox::critical(this, "Camera error", errorString);
		});

		camera->start();
		camera->searchAndLock();
	}
}

void AssessmentWindow::takeCapture(bool ready)
{
	if(ready && camera && imageCapture) {
		camera->searchAndLock();
		imageCapture->capture();
	}
}

void AssessmentWindow::imageSaved(int id, const QString &filename)
{
	QFile file(filename);
	if(file.exists()) {
		QPixmap pixmap(filename);
		auto const labelWidth = webcamImage->width() - 2 * webcamImage->frameWidth();
		pixmap = pixmap.scaledToWidth(labelWidth);
		webcamImage->setPixmap(pixmap);

		if(!assessmentStarted) {
			webcamAcceptButton->setEnabled(true);
		}

		if(!lastCaptureFilename.isEmpty()) {
			QFile lastCaptureFile(lastCaptureFilename);
			if(lastCaptureFile.exists()) {
				lastCaptureFile.remove();
			}
		}
		lastCaptureFilename = filename;
	}

	camera->unlock();

	QTimer::singleShot(100, [this](){
		takeCapture(imageCapture->isReadyForCapture());
	});
}

void AssessmentWindow::saveAssessment()
{
	autoAccept->stop();

	QFile lastCaptureFile(lastCaptureFilename);
	if(lastCaptureFilename.isEmpty() || !lastCaptureFile.exists()) {
		QMessageBox::critical(this, "Assessment error", "The assessment could not be saved: the capture file does not exist.");
		return;
	} else {
		Settings s;
		loadSettings(s);

		QDateTime now = QDateTime::currentDateTime();

		QString extension = lastCaptureFilename.right(4);
		if(extension.size() != 4 || extension[0] != '.') {
			extension = ".png"; // take a guess
		}

		QString fileName = now.toString("yyyy-MM-dd-HH-mm-ss") + extension;
		QString filePath = s.storageLocation.absoluteFilePath(fileName);
		s.storageLocation.mkpath(".");

		if(!lastCaptureFile.copy(filePath)) {
			QMessageBox::critical(this, "Assessment error", "The assessment could not be saved: the capture file could not be copied.");
			return;
		}

		QString excelFileName = now.toString("yyyy-MM-dd") + ".xls";
		QString excelFilePath = s.storageLocation.absoluteFilePath(excelFileName);
		if(!addRowToExcel(excelFilePath, now, userName, score_input->getScore(), fileName)) {
			QMessageBox::critical(this, "Assessment error", "The assessment could not be saved: the Excel file could not be updated.");
			return;
		}
	}

	savedLabel->show();
	QTimer::singleShot(5000, this, [this]() { savedLabel->hide(); });
	startAssessment();
}

void AssessmentWindow::finishAssessment() {
	auto button = QMessageBox::question(this, "Finish assessment?",
		"Are you sure you want to finish the assessment and exit the application?");
	if(button == QMessageBox::Yes) {
		close();
	}
}
