#include "excel.hpp"

#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>

bool addRowToExcel(QString excelFilePath, QDateTime now, QString userName, float score, QString imageFileName)
{
	QFile excelFile(excelFilePath);
	if(!excelFile.exists()) {
		// Fill the file with some default contents
		if(!excelFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			return false;
		}
		QTextStream out(&excelFile);
		out << R"excelfile(<?xml version="1.0" encoding="UTF-8"?>
<?mso-application progid="Excel.Sheet"?>
<Workbook xmlns="urn:schemas-microsoft-com:office:spreadsheet" xmlns:x="urn:schemas-microsoft-com:office:excel" xmlns:ss="urn:schemas-microsoft-com:office:spreadsheet" xmlns:html="http://www.w3.org/TR/REC-html40">
 <ExcelWorkbook xmlns="urn:schemas-microsoft-com:office:excel">
  <WindowHeight>8500</WindowHeight>
  <WindowWidth>20000</WindowWidth>
 </ExcelWorkbook>
<Worksheet ss:Name="Assessments">
<Table>
<Column ss:Index="1" ss:AutoFitWidth="0" ss:Width="160"/>
<Column ss:Index="2" ss:AutoFitWidth="0" ss:Width="150"/>
<Column ss:Index="3" ss:AutoFitWidth="0" ss:Width="40"/>
<Column ss:Index="4" ss:AutoFitWidth="0" ss:Width="200"/>
<Row>
<Cell><Data ss:Type="String">Date &amp; time</Data></Cell>
<Cell><Data ss:Type="String">User name</Data></Cell>
<Cell><Data ss:Type="String">Score</Data></Cell>
<Cell><Data ss:Type="String">Image filename</Data></Cell>
</Row>
</Table>
</Worksheet>
</Workbook>
)excelfile";
		out.flush();
		excelFile.close();
	}

	if(!excelFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}

	QDomDocument doc("");
	if(!doc.setContent(&excelFile)) {
		return false;
	}

	QDomElement table = doc.documentElement().firstChildElement("Worksheet").firstChildElement("Table");
	if(table.isNull()) {
		return false;
	}

	QDomElement newRow = doc.createElement("Row");
	table.appendChild(newRow);

	auto addCell = [&](QString type, QString value) {
		QDomText text = doc.createTextNode(value);

		QDomElement data = doc.createElement("Data");
		data.setAttributeNS("urn:schemas-microsoft-com:office:spreadsheet", "ss:Type", type);
		data.appendChild(text);

		QDomElement cell = doc.createElement("Cell");
		cell.appendChild(data);

		newRow.appendChild(cell);
	};

	addCell("String", now.toString("yyyy-MM-dd HH:mm:ss"));
	addCell("String", userName);
	addCell("Number", QString::number(qRound(score * 100)));
	addCell("String", imageFileName);

	QString xmlDocument = doc.toString();
	if(xmlDocument.isEmpty()) {
		return false;
	}

	excelFile.close();
	if(!excelFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream out(&excelFile);
	out << xmlDocument;
	out.flush();
	if(!excelFile.flush()) {
		return false;
	}
	excelFile.close();
	return true;
}

