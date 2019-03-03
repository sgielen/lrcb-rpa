#include "assessmentscore.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QTextStream>
#include <cassert>

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
	if(layout == AssessmentScoreLayout::GreenWhiteRed) {
		// map (0;1) to (-1;1)
		return (score * 2) - 1;
	} else {
		return score;
	}
}

void AssessmentScore::setScore(float s)
{
	score = s;
	have_score = true;
	repaint();
}

void AssessmentScore::unsetScore()
{
	have_score = false;
	repaint();
}

void AssessmentScore::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(palette().windowText());

	QFontMetrics fm(painter.font());

	// all values are in pixels:
	int const marginTop = 50;
	int const margin = fm.width("normal/benign") / 2;
	int const scoreMarkerUp = 3;
	int const scoreMarkerDown = 3;
	int const scoreMarkerWidth = 30;
	int const labelLineDown = 10;
	int const labelMarginTop = 5;
	int const textHeight = fm.height() * 2;
	int const barHeight = height() - textHeight - labelMarginTop - labelLineDown - scoreMarkerUp - marginTop;

	QColor black(0, 0, 0, 255);
	QColor white(255, 255, 255, 255);
	QColor lighter(89, 94, 99, 255);
	QColor red(150, 0, 0, 255);
	QColor green(10, 150, 0, 255);
	QColor yellow(150, 150, 0, 255);

	QPoint barLeftTop(margin, marginTop + scoreMarkerUp);
	QPoint barRightBottom(width() - margin, marginTop + scoreMarkerUp + barHeight);
	bar = QRect(barLeftTop, barRightBottom - QPoint(1, 0));
	QPoint barMiddleTop = bar.topLeft() + QPoint(bar.width() / 2, 0);
	QPoint labelLineLeft = bar.bottomLeft() + QPoint(0, labelLineDown);
	QPoint labelLineMiddle = barMiddleTop + QPoint(0, barHeight + labelLineDown);
	QPoint labelLineRight = barRightBottom + QPoint(0, labelLineDown);

	QSize labelSize(2 * margin, textHeight);
	QRect labelLeft(QPoint(0, height() - textHeight), labelSize);
	QRect labelMiddle(QPoint(width() / 2 - margin, height() - textHeight), labelSize);
	QRect labelRight(QPoint(width() - 2 * margin, height() - textHeight), labelSize);

	QPen textPen = painter.pen();
	QPen pen = painter.pen();
	pen.setWidth(3);
	pen.setColor(lighter);
	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);

	// draw the bar
	if(layout == AssessmentScoreLayout::GreenRed) {
		QLinearGradient fade(barLeftTop, barRightBottom);
		fade.setColorAt(0, green);
		fade.setColorAt(0.5, yellow);
		fade.setColorAt(1, red);
		painter.fillRect(bar, fade);
		painter.drawRect(bar);
	} else if(layout == AssessmentScoreLayout::GreenWhiteRed) {
		QLinearGradient fade(barLeftTop, barRightBottom);
		fade.setColorAt(0, green);
		fade.setColorAt(0.5, yellow);
		fade.setColorAt(1, red);
		painter.fillRect(bar, fade);
		painter.drawRect(bar);
	} else if(layout == AssessmentScoreLayout::Black) {
		painter.drawLine(bar.bottomLeft(), bar.bottomRight());
	}

	// draw the label lines
	{
		painter.drawLine(bar.bottomLeft(), labelLineLeft);
		painter.drawLine(barRightBottom, labelLineRight);

		if(layout == AssessmentScoreLayout::GreenWhiteRed) {
			painter.drawLine(barMiddleTop, labelLineMiddle);
		}
	}

	// draw the score marker
	if(have_score) {
		int scoreMarkerX = score * bar.width();
		QRect scoreMarker;
		if(layout == AssessmentScoreLayout::Black) {
			QSize scoreMarkerSize(scoreMarkerWidth, scoreMarkerUp + scoreMarkerDown + 4);

			scoreMarker = QRect(QPoint(scoreMarkerX + margin - scoreMarkerWidth / 2, bar.bottom() - scoreMarkerUp - 2), scoreMarkerSize);
		} else {
			QSize scoreMarkerSize(scoreMarkerWidth, scoreMarkerUp + scoreMarkerDown + barHeight);

			scoreMarker = QRect(QPoint(scoreMarkerX + margin - scoreMarkerWidth / 2, marginTop), scoreMarkerSize);
		}
		painter.setBrush(lighter);
		painter.drawRoundedRect(scoreMarker, 2, 2);
	}

	// draw the labels
	painter.setPen(textPen);
	if(layout == AssessmentScoreLayout::GreenWhiteRed) {
		painter.drawText(labelLeft, Qt::AlignHCenter | Qt::AlignTop, "normaal");
		painter.drawText(labelMiddle, Qt::AlignHCenter | Qt::AlignTop, "onzeker");
		painter.drawText(labelRight, Qt::AlignHCenter | Qt::AlignTop, "radiologisch\nmaligne");
	} else {
		painter.drawText(labelLeft, Qt::AlignHCenter | Qt::AlignTop, "unsure");
		painter.drawText(labelRight, Qt::AlignHCenter | Qt::AlignTop, "confident");
	}

}

void AssessmentScore::mousePressEvent(QMouseEvent *event)
{
	inDrag = true;
	dragStartedPos = event->pos();
	event->accept();
}

void AssessmentScore::mouseReleaseEvent(QMouseEvent *event)
{
	// Note, this function can be called from the event filter if the mouse is released
	// outside of our widget after a drag over us.
	static const int MAX_X_OFFSET = 120;
	static const float MIN_Y_FACTION = 0.5;

	if(inDrag) {
		auto endPos = event->pos();
		auto delta = endPos - dragStartedPos;
		bool isDown = abs(delta.x()) <= MAX_X_OFFSET;

		auto posInBar = endPos - bar.topLeft();
		bool isPosInBar = posInBar.x() > 0 && posInBar.x() < bar.width();

		float y_faction = float(delta.y()) / bar.height();

		bool score_entered = isDown && isPosInBar && y_faction >= MIN_Y_FACTION;
		if (score_entered) {
			have_score = true;
			score = float(posInBar.x()) / bar.width();
			emit scoreEntered(score);
		}

		// otherwise, don't remove the score that may have been entered already
	}
	inDrag = false;
	repaint();
	event->accept();
}

void AssessmentScore::mouseMoveEvent(QMouseEvent *event)
{
	// Note, this function can be called from the event filter if the mouse is moving over
	// us, even in a drag which didn't start inside us.
	if (!inDrag && event->buttons() & Qt::LeftButton) {
		// We weren't in a drag before, but the mouse is pressed and is moving over us,
		// so we are in a drag now!
		mousePressEvent(event);
	}
}

bool AssessmentScore::eventFilter(QObject *object, QEvent *e)
{
	if (e->type() == QEvent::MouseMove) {
		QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
		assert(me);

		auto localPos = mapFromGlobal(me->globalPos());
		if (localPos.x() < 0 || localPos.y() < 0 || localPos.x() > width() || localPos.y() > height()) {
			// Not a move over me, ignore it
			return false;
		}

		// Copy to a local event, then give it to ourselves as if the move happened over us
		QMouseEvent event(
			me->type(),
			localPos,
			me->windowPos(),
			me->screenPos(),
			me->button(),
			me->buttons(),
			me->modifiers(),
			me->source());
		mouseMoveEvent(&event);
	} else if (e->type() == QEvent::MouseButtonRelease && inDrag) {
		QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
		assert(me);

		// Here, we do want to allow mouse releases while not over us, as we are
		// currently handling a drag that we know went over us (inDrag is true)
		auto localPos = mapFromGlobal(me->globalPos());

		QMouseEvent event(
			me->type(),
			localPos,
			me->windowPos(),
			me->screenPos(),
			me->button(),
			me->buttons(),
			me->modifiers(),
			me->source());
		mouseReleaseEvent(&event);
	}
	return false;
}
