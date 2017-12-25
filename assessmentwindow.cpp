#include "assessmentwindow.hpp"
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

AssessmentWindow::AssessmentWindow(QWidget *parent)
: QMainWindow(parent)
{
	setWindowTitle("LRCB-RPA - Assessment");
	resize(500, 350);

	auto *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(QIcon(":/info.png"), tr("&About"), this, &AssessmentWindow::about);
	fileMenu->addSeparator();
	fileMenu->addAction(QIcon(":/exit.xpm"), tr("&Quit"), this, &QWidget::close);

	setCentralWidget(new QWidget);
	centralWidget()->setLayout(new QVBoxLayout);
	addTitleToWidget(centralWidget(), "Assessment configuration");

	centralWidget()->layout()->addWidget(new QLabel("Webcam calibration", this));

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

	QPushButton *start = new QPushButton("Start assessment", this);
	centralWidget()->layout()->addWidget(start);

	connect(start, &QPushButton::pressed, this, &AssessmentWindow::startAssessment);

	switchCamera();
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
	}

	camera->unlock();

	QTimer::singleShot(100, [this](){
		takeCapture(imageCapture->isReadyForCapture());
	});
}
