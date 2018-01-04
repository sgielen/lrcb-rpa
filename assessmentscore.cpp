#include "assessmentscore.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QTextStream>

AssessmentScore::AssessmentScore(AssessmentScoreLayout l, QWidget *parent)
: QWidget(parent)
, layout(l)
{
	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);
}

bool AssessmentScore::hasScore() const
{
	return have_score;
}

float AssessmentScore::getScore() const
{
	return score;
}

void AssessmentScore::setScore(float s)
{
	score = s;
	have_score = true;
}

void AssessmentScore::unsetScore()
{
	have_score = false;
}

void AssessmentScore::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(palette().windowText());
	painter.drawLine(0, 0, width() - 1, height() - 1);
	painter.drawText(width() / 2, 10, text);

	// TODO: according to the score layout, draw a gradient bar optionally,
	// marked with "unsure", "very sure abnormal", "very sure normal"
}

QSize AssessmentScore::sizeHint() const
{
	return QSize(200, 300);
}

QSize AssessmentScore::minimumSizeHint() const
{
	return sizeHint();
}

void AssessmentScore::mousePressEvent(QMouseEvent *event)
{
	inDrag = true;
	dragStartedPos = event->pos();
}

inline QTextStream &operator<<(QTextStream &s, const QPoint &p)
{
	s << "(" << p.x() << "," << p.y() << ")";
	return s;
}

void AssessmentScore::mouseReleaseEvent(QMouseEvent *event)
{
	static const int MAX_X_OFFSET = 60;
	static const float MIN_Y_FACTION = 0.5;

	if(inDrag) {
		auto endPos = event->pos();
		auto delta = endPos - dragStartedPos;
		bool isDown = abs(delta.x()) <= MAX_X_OFFSET;
		float y_faction = float(delta.y()) / height();
		have_score = isDown && y_faction >= MIN_Y_FACTION;
		score = float(endPos.x()) / width();

		text.clear();
		if(have_score) {
			QTextStream(&text) << "Score entered: " << score << "\n";
			emit scoreEntered(score);
		} else {
			QTextStream(&text) << "No score entered\n";
		}
	}
	inDrag = false;
	repaint();
}
