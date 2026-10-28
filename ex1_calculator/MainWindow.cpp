#include "MainWindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QSizePolicy>
#include <QKeyEvent>

// 构造：初始化状态、绑定按钮、应用样式
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , storedValue(0.0)
    , pendingOperator()
    , waitingForOperand(true)
    , justCalculated(false)
{
    ui->setupUi(this);
    setupGrid();      // 让按钮网格等宽等高分布
    connectButtons();
    applyStyle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------
// 信号连接：每个按钮点击后都转发到核心输入函数
// 这样鼠标输入与键盘输入天然复用同一套逻辑
// ---------------------------------------------------------
void MainWindow::setupGrid()
{
    // 4 列按钮等宽、5 行等高分摊，形成规整的手机计算器布局
    for (int c = 0; c < 4; ++c) ui->buttonGrid->setColumnStretch(c, 1);
    for (int r = 0; r < 5; ++r) ui->buttonGrid->setRowStretch(r, 1);

    // 让按钮随窗口伸缩，占满可用空间（避免下方留白）
    const QList<QPushButton *> btns = ui->centralwidget->findChildren<QPushButton *>();
    for (QPushButton *b : btns) {
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        b->setMinimumSize(0, 40);
    }
    // 按钮网格占据垂直布局的伸缩份额，使按钮充满窗口
    ui->verticalLayout->setStretch(1, 1);
}

void MainWindow::connectButtons()
{
    // 数字键 0~9
    QList<QPushButton *> digitButtons = {
        ui->btn0, ui->btn1, ui->btn2, ui->btn3, ui->btn4,
        ui->btn5, ui->btn6, ui->btn7, ui->btn8, ui->btn9
    };
    for (int i = 0; i < digitButtons.size(); ++i) {
        connect(digitButtons.at(i), &QPushButton::clicked,
                this, [this, i]() { inputDigit(QString::number(i)); });
    }

    // 小数点
    connect(ui->btnPoint, &QPushButton::clicked, this, [this]() { inputPoint(); });

    // 操作符（按钮文本即 "+","-","×","÷"）
    connect(ui->btnAdd,   &QPushButton::clicked, this, [this]() { inputOperator(ui->btnAdd->text()); });
    connect(ui->btnSub,   &QPushButton::clicked, this, [this]() { inputOperator(ui->btnSub->text()); });
    connect(ui->btnMul,   &QPushButton::clicked, this, [this]() { inputOperator(ui->btnMul->text()); });
    connect(ui->btnDiv,   &QPushButton::clicked, this, [this]() { inputOperator(ui->btnDiv->text()); });

    // 等号 / 退格 / 清除
    connect(ui->btnEqual, &QPushButton::clicked, this, [this]() { doEquals(); });
    connect(ui->btnBackspace, &QPushButton::clicked, this, [this]() { doBackspace(); });
    connect(ui->btnClear, &QPushButton::clicked, this, [this]() { doClear(); });
}

// ---------------------------------------------------------
// 键盘事件：把按键映射到与鼠标完全相同的一套核心函数
// ---------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    const QString text = event->text();

    // 数字：主键盘与数字小键盘（Qt 中小键盘数字与主键盘共用 Key_0..Key_9）
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        inputDigit(QString::number(key - Qt::Key_0));
        return;
    }

    // 小数点（主键盘 . 或小键盘）
    if (key == Qt::Key_Period || key == Qt::Key_Comma) { inputPoint(); return; }

    // 操作符：+ - * /  以及小键盘运算符
    if (key == Qt::Key_Plus)                    { inputOperator("+"); return; }
    if (key == Qt::Key_Minus)                   { inputOperator("-"); return; }
    if (key == Qt::Key_Asterisk || text == "x" || text == "X")
                                                 { inputOperator("×"); return; }
    if (key == Qt::Key_Slash)                   { inputOperator("÷"); return; }

    // 等号 / 回车
    if (key == Qt::Key_Equal || key == Qt::Key_Return || key == Qt::Key_Enter) { doEquals(); return; }

    // 退格 / 删除
    if (key == Qt::Key_Backspace || key == Qt::Key_Delete) { doBackspace(); return; }

    // 清除：C / Esc
    if (key == Qt::Key_C || key == Qt::Key_Escape) { doClear(); return; }

    QMainWindow::keyPressEvent(event);
}

// ---------------------------------------------------------
// 核心输入逻辑（鼠标与键盘共用）
// ---------------------------------------------------------

// 输入一位数字
void MainWindow::inputDigit(const QString &digit)
{
    QString s = ui->display->text();

    // 处于“等待第二个操作数”或“刚出结果”状态时，开始全新数字
    if (waitingForOperand || justCalculated) {
        s.clear();
        waitingForOperand = false;
        justCalculated = false;
    }

    // 处理前导 0：当前是 "0" 时，再按 0 保持，按其他数字则替换
    if (s == "0") {
        if (digit == "0") return;
        s.clear();
    }

    // 限制显示位数，防止超长溢出
    // 注意：不能用 s.remove('.') 判断，它会原地删除小数点；
    // 改用"总长 - 小数点个数"计算数字位数，不改变显示文本
    if (s.length() - s.count('.') >= 15) return;

    s += digit;
    ui->display->setText(s);
}

// 输入小数点：同一操作数只允许一个小数点
void MainWindow::inputPoint()
{
    QString s = ui->display->text();

    if (waitingForOperand || justCalculated) {
        s.clear();
        waitingForOperand = false;
        justCalculated = false;
    }

    if (s.isEmpty()) s = "0";

    // 若已含小数点则忽略，避免重复小数点
    if (!s.contains('.')) {
        s += ".";
        ui->display->setText(s);
    }
}

// 输入操作符
void MainWindow::inputOperator(const QString &op)
{
    // 连续按操作符：仅替换待执行的操作符，不重复入栈
    if (waitingForOperand && !pendingOperator.isEmpty()) {
        pendingOperator = op;
        return;
    }

    // 已有待执行操作符：先完成一次中间运算（支持连续运算）
    if (!pendingOperator.isEmpty()) {
        calculate();
    }

    // 保存当前显示值为第一个操作数
    storedValue = ui->display->text().toDouble();
    pendingOperator = op;
    waitingForOperand = true;
    justCalculated = false;
}

// 等号
void MainWindow::doEquals()
{
    if (pendingOperator.isEmpty()) {
        justCalculated = true;
        return;
    }
    calculate();
    pendingOperator.clear();
    waitingForOperand = true;
    justCalculated = true;
}

// 退格
void MainWindow::doBackspace()
{
    // 结果、等待输入状态下不允许退格
    if (justCalculated || waitingForOperand) return;

    QString s = ui->display->text();
    if (s.length() > 1) s.chop(1);
    else s = "0";
    ui->display->setText(s);
}

// 清除：重置全部状态
void MainWindow::doClear()
{
    storedValue = 0.0;
    pendingOperator.clear();
    waitingForOperand = true;
    justCalculated = false;
    ui->display->setText("0");
}

// 计算（处理除零等边界情况）
void MainWindow::calculate()
{
    double second = ui->display->text().toDouble();
    double result = 0.0;

    // 除数为 0：给出明确提示并复位状态
    if (pendingOperator == "÷" && second == 0.0) {
        ui->display->setText("错误：除数不能为0");
        pendingOperator.clear();
        storedValue = 0.0;
        waitingForOperand = true;
        justCalculated = true;
        return;
    }

    if (pendingOperator == "+") result = storedValue + second;
    else if (pendingOperator == "-") result = storedValue - second;
    else if (pendingOperator == "×") result = storedValue * second;
    else if (pendingOperator == "÷") result = storedValue / second;

    // 用 'g' 格式自动省略多余小数位，如 5 显示 "5"
    ui->display->setText(QString::number(result, 'g', 12));
}

// 样式表：美化界面
void MainWindow::applyStyle()
{
    // 数字键：深灰圆角
    QString digitStyle = "QPushButton#btn0,#btn1,#btn2,#btn3,#btn4,"
                         "#btn5,#btn6,#btn7,#btn8,#btn9,#btnPoint{"
                         "background:#333333; color:#ffffff; font-size:20px;"
                         "border-radius:6px;}";
    // 操作符与功能键：橙/灰
    QString opStyle = "QPushButton#btnAdd,#btnSub,#btnMul,#btnDiv{"
                      "background:#ff9500; color:#ffffff; font-size:22px;"
                      "border-radius:6px;}";
    QString fnStyle = "QPushButton#btnClear,#btnBackspace{"
                      "background:#a5a5a5; color:#000000; font-size:18px;"
                      "border-radius:6px;}";
    QString eqStyle = "QPushButton#btnEqual{"
                      "background:#ff9500; color:#ffffff; font-size:22px;"
                      "border-radius:6px;}";

    setStyleSheet(digitStyle + opStyle + fnStyle + eqStyle +
                  "QPushButton:pressed{background:#5a5a5a;}");
}
