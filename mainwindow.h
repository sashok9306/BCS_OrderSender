#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <BCS_CONNECTOR/BCS_Exc.h>
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
    void OrderUpdate(MoexOrder UpOrder);

    void OrdersRecive(QHash<QString, MoexOrder> orders);

    void on_VolumeType_currentIndexChanged(int index);

    void on_Instrumments_currentIndexChanged(int index);

    void CreateWidgetInOrderList(int index,MoexOrder DataOrder);

private:
    Ui::MainWindow *ui;
    Instrument *CurrentInstrument;
    QHash<QString,Instrument*>ControlledInstrumments;
    QHash<QString,OrderWidget*>WidgetOrderControll;
    QTimer TimForSendPos;
    bool Bay;
    QString GlobalToken;
    BCS_Exc BCS;
};
#endif // MAINWINDOW_H
