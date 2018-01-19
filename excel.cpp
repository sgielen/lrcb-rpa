#include "excel.hpp"

#include <QDebug>

bool addRowToExcel(QString excelFilePath, QDateTime now, QString userName, float score, QString imageFileName)
{
	qDebug() << "Add row to Excel at " << excelFilePath << ": " << now.toString(Qt::ISODate) << " / " << userName << " / " << qRound(score * 100) << " / " << imageFileName;
	// TODO: addRowToExcel:
	// - load the sheet
	//   - if it doesn't exist, initialize a basic sheet
	// - add a row to it
	// - save the sheet to temporary file
	// - if temporary file is not empty, move temporary file over normal file
	return true;
}

