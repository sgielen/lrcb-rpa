#pragma once

#include <QWidget>

enum class AssessmentScoreLayout {
	GreenRed, // left being 0, right 1
	Black, // left being 0, right 1
	GreenWhiteRed, // left being -1, middle 0, right 1
};

class AssessmentScore : public QWidget {
Q_OBJECT

public:
	AssessmentScore(AssessmentScoreLayout layout, QWidget *parent);

	bool hasScore() const;
	float getScore() const;

public slots:
	void setScore(float score);
	void unsetScore();

signals:
	void scoreEntered(float score);

private:
	void paintEvent(QPaintEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;

	AssessmentScoreLayout layout;
	bool have_score = false;
	float score = 0;

	bool inDrag = false;
	QPoint dragStartedPos;
	QRect bar;
	QString text;
};
