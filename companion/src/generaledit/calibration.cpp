#include <QVBoxLayout>
#include <QHeaderView>
#include <QSpinBox>
#include "calibration.h"


CalibrationPanel::CalibrationPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
  GeneralPanel(parent, generalSettings, firmware)
{

  tableWidget = new QTableWidget();
  QVBoxLayout * layout = new QVBoxLayout();
  layout->addWidget(tableWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(layout);

  tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tableWidget->setColumnCount(3);
  tableWidget->setShowGrid(false);
  tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
  tableWidget->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
  tableWidget->setStyleSheet("QTableWidget {background-color: transparent;}");
  QStringList headerLabels;
  headerLabels << QObject::tr("Negative span") << QObject::tr("Mid value") << QObject::tr("Positive span"); 
  tableWidget->setHorizontalHeaderLabels(headerLabels);

  int rows = NUM_STICKS + GetCurrentFirmware()->getCapability(Pots) + GetCurrentFirmware()->getCapability(Sliders);
  tableWidget->setRowCount(rows);

  for(int i = 0; i < rows; ++i) {

    QTableWidgetItem *newItem = new QTableWidgetItem(AnalogString(i));
    newItem->setTextAlignment(Qt::AlignLeft);
    tableWidget->setVerticalHeaderItem(i, newItem);

    for(int j = 0; j < 3; ++j) {
      QSpinBox * newItem = new QSpinBox();
      newItem->setMinimum(-9999);
      newItem->setMaximum(9999);
      newItem->setSingleStep(1);
      newItem->setValue(getCalibrationValue(i, j));
      newItem->setProperty("row", i);
      newItem->setProperty("column", j);
      tableWidget->setCellWidget(i, j, newItem);
      connect(newItem, SIGNAL(valueChanged(int)), this, SLOT(onCellChanged(int)));
    }
  }
  disableMouseScrolling();
}

void CalibrationPanel::onCellChanged(int value)
{
  QSpinBox * sb = qobject_cast<QSpinBox*>(sender());
  int row = sb->property("row").toInt();
  int column = sb->property("column").toInt();

  if (value != getCalibrationValue(row, column)) {
    setCalibrationValue(row,column, value);
    qDebug() << "CalibrationPanel::onCellChanged" << row << column << "to" << value;
    emit modified();
  }
}
    
int CalibrationPanel::getCalibrationValue(int row, int column)
{
  switch(column) {
    case 0:
      return generalSettings.calibSpanNeg[row];
    case 1:
      return generalSettings.calibMid[row];
    case 2:
      return generalSettings.calibSpanPos[row];
  }
  return 0;
}

void CalibrationPanel::setCalibrationValue(int row, int column, int value)
{
  switch(column) {
    case 0:
      generalSettings.calibSpanNeg[row] = value;
      break;
    case 1:
      generalSettings.calibMid[row] = value;
      break;
    case 2:
      generalSettings.calibSpanPos[row] = value;
      break;
  }
}
