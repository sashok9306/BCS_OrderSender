#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <BCS_Exc.h>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <Order.h>
#include <SetupKey.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void LoadSettings();
    void SaveSettings();
    void on_Buy_clicked();
    void SendOrder();
    void on_Sell_clicked();
    void PrintLog(QString Message);

    void on_OrderType_currentIndexChanged(int index);
    void CancelOrder(MoexOrder ORDER, QListWidgetItem *Item);
    void OrderChange(MoexOrder ORDER, double Price, double Offset, double Spread, double quantity);

    void on_ClassCode_currentIndexChanged(int index);
    void UpdateInstData();

    void OrdersRecived(QList<MoexOrder> orders);

    void on_VolumeType_currentIndexChanged(int index);

    void on_Instrumments_currentIndexChanged(int index);

    void on_tabWidget_currentChanged(int index);

private:
    Ui::MainWindow *ui;
    Instrument CurrentInstrument;
    QTimer TimForSendPos;
    bool Bay;
    QString GlobalToken;
    BCS_Exc BCS;
};
#endif // MAINWINDOW_H
