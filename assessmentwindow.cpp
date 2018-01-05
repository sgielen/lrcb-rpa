#include "assessmentwindow.hpp"
#include "assessmentscore.hpp"
#include "util.hpp"

#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraInfo>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <cassert>

AssessmentWindow::AssessmentWindow(bool s, QWidget *parent)
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

	score_input = new AssessmentScore(AssessmentScoreLayout::GreenRed, this);
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

	webcamImage = new QLabel(this);
	centralWidget()->layout()->addWidget(webcamImage);

	connect(cameraBox, &QComboBox::currentTextChanged, this, &AssessmentWindow::switchCamera);

	dynamic_cast<QVBoxLayout*>(centralWidget()->layout())->addStretch();

	assessmentBtn = new QPushButton("Start assessment", this);
	centralWidget()->layout()->addWidget(assessmentBtn);

	connect(assessmentBtn, &QPushButton::pressed, this, &AssessmentWindow::startAssessment);
	assessmentBtn->setEnabled(false);

	switchCamera();

	if(skipSetup) {
		startAssessment();
	}
}

AssessmentWindow::~AssessmentWindow()
{
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
	webcamImage->hide();
	webcamImage->clear();

	title->setText("Assessment");
	commandLine->setText("Drag your finger over the bar to enter a confidence level:");

	assessmentBtn->disconnect();
	assessmentBtn->setText("Save and quit");
	connect(assessmentBtn, &QPushButton::pressed, this, &AssessmentWindow::close);

	score_input->show();
	connect(score_input, &AssessmentScore::scoreEntered, this, &AssessmentWindow::scoreEntered);
}

void AssessmentWindow::scoreEntered()
{
	assessmentBtn->setEnabled(false);
	webcamImage->show();

	assert(camera);
	assert(imageCapture);

	// TODO: make behaviour of imageSaved() different if we take a capture
	// because the score is entered?
	camera->start();

	// TODO: add Retake button, add Accept button, make the Accept button count down automatically

	// TODO: once a score is accepted:
	// - load the sheet
	// - add a row to it
	// - save the sheet to temporary file
	// - if temporary file is not empty, move temporary file over normal file
	// - hide the webcamImage & buttons again
	// - re-enable the assessmentBtn
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
		pixmap = pixmap.scaledToWidth(webcamImage->width());
		webcamImage->setPixmap(pixmap);
		file.remove();
		assessmentBtn->setEnabled(true);
	}

	camera->unlock();

	QTimer::singleShot(100, [this](){
		takeCapture(imageCapture->isReadyForCapture());
	});
}
