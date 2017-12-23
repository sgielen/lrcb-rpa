#include "assessmentwindow.hpp"
#include "util.hpp"

#include <QVBoxLayout>

AssessmentWindow::AssessmentWindow(QWidget *parent)
: QMainWindow(parent)
{
	setWindowTitle("LRCB-RPA - Assessment");

	resize(500, 350);

	setCentralWidget(new QWidget);
	centralWidget()->setLayout(new QVBoxLayout);
	addTitleToWidget(centralWidget(), "Assessment configuration");

	dynamic_cast<QVBoxLayout*>(centralWidget()->layout())->addStretch();
}

AssessmentWindow::~AssessmentWindow()
{
}

void AssessmentWindow::loginPerformed(QString name)
{
	userName = name;
	show();
}
