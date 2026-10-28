# Qt 计算器（带键盘事件）— 实验一

东莞理工学院《Qt 应用程序开发》实验一：带键盘事件的计算器。

## 源码文件（上传到 GitHub 仓库的 5 个核心文件）
- `Calculator.pro`      — Qt 工程文件
- `main.cpp`            — 程序入口
- `MainWindow.h`        — 主窗口类声明（含状态机与键盘事件）
- `MainWindow.cpp`      — 全部核心逻辑、键盘映射、布局与样式表
- `mainwindow.ui`       — 界面布局（显示器 + 4×5 按钮网格）

## 功能
- 四则运算（+ − × ÷）、小数、退格、清除
- 鼠标与键盘输入复用同一套核心逻辑（signals 与 keyPressEvent 都转发到 inputDigit/inputPoint/inputOperator/doEquals/doBackspace/doClear/calculate）
- 连续运算、重复操作符替换、重复小数点忽略、除零提示
- 深色样式表美化

## 编译（Qt 5.12.12 + MinGW）
```
set PATH=C:\Qt\Qt5.12.12\5.12.12\mingw73_64\bin;C:\Qt\Qt5.12.12\Tools\mingw730_64\bin;%PATH%
qmake Calculator.pro
mingw32-make -j4
windeployqt --release release\Calculator.exe
```
运行：`release\Calculator.exe`

仓库地址：https://github.com/imnotbambi/QtCourse
