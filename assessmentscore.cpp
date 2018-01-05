#include "assessmentscore.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QTextStream>

static inline QTextStream &operator<<(QTextStream &s, const QPoint &p)
{
	s << "(" << p.x() << "," << p.y() << ")";
	return s;
}

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

	QFontMetrics fm(painter.font());

	// all values are in pixels:
	int const marginTop = 30;
	int const margin = 50; // TODO: autodetect from fm.width("...");
	int const scoreMarkerUp = 3;
	int const scoreMarkerDown = 3;
	int const scoreMarkerWidth = 10;
	int const labelLineDown = 10;
	int const labelMarginTop = 5;
	int const textHeight = fm.height();
	int const barHeight = height() - textHeight - labelMarginTop - labelLineDown - scoreMarkerUp - marginTop;

	QColor white(255, 255, 255, 255);
	QColor red(255, 0, 0, 255);
	QColor green(0, 255, 0, 255);

	QPoint barLeftTop(margin, marginTop + scoreMarkerUp);
	QPoint barRightBottom(width() - margin, marginTop + scoreMarkerUp + barHeight);
	bar = QRect(barLeftTop, barRightBottom - QPoint(1, 0));
	QPoint labelLineLeft = bar.bottomLeft() + QPoint(0, labelLineDown);
	QPoint labelLineRight = barRightBottom + QPoint(0, labelLineDown);

	QSize labelSize(2 * margin, textHeight);
	QRect labelLeft(QPoint(0, height() - textHeight), labelSize);
	QRect labelRight(QPoint(width() - 2 * margin, height() - textHeight), labelSize);

	QSize scoreMarkerSize(scoreMarkerWidth, scoreMarkerUp + scoreMarkerDown + barHeight);

	// draw the bar
	{
		QLinearGradient fade(barLeftTop, barRightBottom);
		fade.setColorAt(0, green);
		fade.setColorAt(1, red);
		painter.fillRect(bar, fade);
	}

	// draw the lines
	{
		QPen pen = painter.pen();
		pen.setWidth(2);
		painter.setPen(pen);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(bar);

		painter.drawLine(bar.bottomLeft(), labelLineLeft);
		painter.drawLine(barRightBottom, labelLineRight);
	}

	// draw the labels
	{
		painter.drawText(labelLeft, Qt::AlignHCenter | Qt::AlignTop, "unsure");
		painter.drawText(labelRight, Qt::AlignHCenter | Qt::AlignTop, "confident");
	}

	// draw the score marker
	if(have_score) {
		int scoreMarkerX = score * bar.width();
		QRect scoreMarker(QPoint(scoreMarkerX + margin - scoreMarkerWidth / 2, marginTop), scoreMarkerSize);
		painter.setBrush(white);
		painter.drawRoundedRect(scoreMarker, 2, 2);
	}
}

QSize AssessmentScore::sizeHint() const
{
	return QSize(1200, 100);
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

// TODO: perhaps do below on mouseMove, and only emit scoreEntered on mouseRelease?

void AssessmentScore::mouseReleaseEvent(QMouseEvent *event)
{
	static const int MAX_X_OFFSET = 60;
	static const float MIN_Y_FACTION = 0.5;

	if(inDrag) {
		auto endPos = event->pos();
		auto delta = endPos - dragStartedPos;
		bool isDown = abs(delta.x()) <= MAX_X_OFFSET;

		auto posInBar = endPos - bar.topLeft();
		bool isPosInBar = posInBar.x() > 0 && posInBar.x() < bar.width();

		float y_faction = float(delta.y()) / bar.height();

		have_score = isDown && y_faction >= MIN_Y_FACTION;

		score = float(posInBar.x()) / bar.width();

		if(have_score) {
			emit scoreEntered(score);
		}
	}
	inDrag = false;
	repaint();
}
