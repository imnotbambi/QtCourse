#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 键盘事件：将按键翻译为与鼠标一致的输入动作
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;

    // ===== 计算器核心状态 =====
    double storedValue;       // 第一个操作数（或中间结果）
    QString pendingOperator;  // 待执行的操作符："+","-","×","÷"
    bool waitingForOperand;   // 刚按下操作符，等待输入第二个操作数
    bool justCalculated;      // 刚刚计算出结果，输入数字时将开启新运算

    // ===== 核心输入逻辑（鼠标与键盘复用同一套） =====
    void inputDigit(const QString &digit);   // 输入一位数字
    void inputPoint();                        // 输入小数点
    void inputOperator(const QString &op);    // 输入操作符
    void doEquals();                          // 执行等号
    void doBackspace();                       // 退格
    void doClear();                           // 清除
    void calculate();                         // 计算中间/最终结果
    void setupGrid();                         // 设置按钮网格等宽等高
    void connectButtons();                    // 绑定所有按钮信号
    void applyStyle();                        // 应用样式表
};

#endif // MAINWINDOW_H
