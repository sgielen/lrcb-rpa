#pragma once

#include <QString>
#include <QDateTime>

bool addRowToExcel(QString excelFilePath, QDateTime now, QString userName, float score, QString imageFileName);
