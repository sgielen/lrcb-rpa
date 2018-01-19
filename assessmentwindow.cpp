#include "assessmentwindow.hpp"
#include "assessmentscore.hpp"
#include "excel.hpp"
#include "util.hpp"

#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
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

	// set fullscreen
	setWindowState(windowState() | Qt::WindowFullScreen);

	auto *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(QIcon(":/info.png"), tr("&About"), this, &AssessmentWindow::about);
	fileMenu->addSeparator();
	fileMenu->addAction(QIcon(":/exit.xpm"), tr("&Quit"), this, &QWidget::close);

	setCentralWidget(new QWidget);
	centralWidget()->setLayout(new QVBoxLayout);
	title = addTitleToWidget(centralWidget(), "Assessment configuration");

	commandLine = new QLabel("First, calibrate the webcam so that the study information is clearly readable.", this);
	centralWidget()->layout()->addWidget(commandLine);

	score_input = new AssessmentScore(input_layout, this);
	score_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	auto *score_layout = new QHBoxLayout;
	score_layout->addStretch();
	score_layout->addWidget(score_input);
	score_layout->addStretch();
	centralWidget()->layout()->addItem(score_layout);
	score_input->hide();

	cameraInfo = QCameraInfo::availableCameras();
	if(cameraInfo.count() == 0) {
		QLabel *cameras = new QLabel("Error: No webcameras available -- connect a camera and restart the application.", this);
		centralWidget()->layout()->addWidget(cameras);
		dynamic_cast<QVBoxLayout*>(centralWidget()->layout())->addStretch();
		return;
	}

	cameraBox = new QComboBox(this);
	for(auto camera : cameraInfo) {
		cameraBox->addItem(camera.description());
	}

	centralWidget()->layout()->addWidget(cameraBox);
	if(cameraInfo.count() == 1) {
		// if there's only one camera, don't show the combobox at all
		cameraBox->hide();
	}

	connect(cameraBox, &QComboBox::currentTextChanged, this, &AssessmentWindow::switchCamera);

	webcamImageText = new QLabel("Webcam shot");
	centralWidget()->layout()->addWidget(webcamImageText);
	webcamImageText->hide();

	webcamImage = new QLabel(this);
	webcamImage->setFrameStyle(QFrame::Panel | QFrame::Sunken);

	QFrame *webcamButtonsFrame = new QFrame(this);
	webcamButtonsFrame->setLayout(new QVBoxLayout);

	webcamAcceptButton = new QPushButton(QIcon(":/tick.png"), "Accept", this);
	webcamButtonsFrame->layout()->addWidget(webcamAcceptButton);
	QPushButton *webcamRetakeButton = new QPushButton(QIcon(":/clock_stop.png"), "Stop countdown", this);
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
	webcamRetakeButton->hide();
	connect(webcamAcceptButton, &QPushButton::pressed, this, [this, webcamRetakeButton]() {
		webcamAcceptButton->setText("Accept");
		webcamRetakeButton->show();
		startAssessment();
	});

	webcamFrame = new QFrame(this);
	webcamFrame->setLayout(new QHBoxLayout);
	webcamFrame->layout()->addWidget(webcamImage);
	webcamFrame->layout()->addWidget(webcamButtonsFrame);
	centralWidget()->layout()->addWidget(webcamFrame);

	dynamic_cast<QVBoxLayout*>(centralWidget()->layout())->addStretch();

	savedLabel = new QLabel(this);
	savedLabel->setTextFormat(Qt::RichText);
	savedLabel->setText("<img src=\":/diskette.png\"/> Assessment saved!");
	savedLabel->setStyleSheet("QLabel { background-color: #dff0d8; border: 1px solid #a2d48f; }");
	savedLabel->setAlignment(Qt::AlignHCenter);
	savedLabel->hide();
	centralWidget()->layout()->addWidget(savedLabel);

	finishAssessmentBtn = new QPushButton("Cancel assessment session and exit", this);
	centralWidget()->layout()->addWidget(finishAssessmentBtn);

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

void AssessmentWindow::about()
{
}

void AssessmentWindow::loginPerformed(QString name)
{
	userName = name;
	show();
}

void AssessmentWindow::startAssessment()
{
	// TODO: stop taking webcam pictures automatically
	webcamImageText->hide();
	webcamFrame->hide();
	webcamImage->clear();

	title->setText("Assessment");
	commandLine->setText("Drag your finger over the bar to enter a confidence level:");

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
	webcamImageText->show();
	webcamFrame->show();

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
		camera->start();

		connect(imageCapture, &QCameraImageCapture::readyForCaptureChanged, this, &AssessmentWindow::takeCapture);
		connect(imageCapture, &QCameraImageCapture::imageSaved, this, &AssessmentWindow::imageSaved);

		connect(imageCapture, QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
		    [=](int, QCameraImageCapture::Error, const QString &errorString){
			QMessageBox::critical(this, "Camera error", errorString);
		});
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
		webcamAcceptButton->setEnabled(true);

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
