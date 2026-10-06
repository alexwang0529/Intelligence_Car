
# <Intelligence_Car>

> 多功能小车

## 最新稳定版本

- **Stable:** `v0.1.0`
- **Release:** <[GitHub Release 链接](https://github.com/alexwang0529/Intelligence_Car)>
- **状态:** 稳定维护

> 本 README 以最新稳定版本为准；开发中功能请查看仓库中的 `docs/plan.md` 与对应 Issue / PR



## 1. 项目简介

说明：做一台多功能的小车，让任何人都可复现的项目

- 项目用途：记录小车迭代过程
- 主要能力：自动纠偏、指定角度转动、记录里程、循线、物体测距与计数、遥控

## 2. 环境要求

### 软件

- OS：Windows 10 / Windows 11
- Compiler：Keil MDK-ARM（µVision、ARM Compiler 5）
- 关键依赖：ARM CMSIS、STM32F10x 标准外设库（SPL）、STM32F103C8T6 启动文件及各硬件模块驱动程序 （仓库均有提供）

### 硬件（仅对V0.1.0版本）

- 主控：STM32F103C8T6 最小系统板、STM32 智能小车扩展板、D24A TB6612 四路电机驱动板、8.4V 锂电池、12V 锂电池
- 传感器：MPU6050、HC-SR04、四路红外循迹传感器、左右轮霍尔编码器（电机自带）
- 执行器：4 台 MG513X 直流减速电机（带编码器）、4 个 65 mm 车轮、0.96 英寸 I²C OLED 显示屏
- 接口：SWD 下载调试接口；软件 I²C 接口；串口；超声波接口； 循迹接口；按键接口； 编码器接口；4 路 PWM 及 8 路 GPIO 电机控制接口

> 主控选择一个stm32扩展板和适配电机的驱动板即可
> 电机必须带编码器
> 如果扩展板模块不完全满足，则利用空余引脚外接即可
> 若需要硬件的链接，可从主页获取邮箱联系本人

## 3. 安装

1. 安装 Keil MDK-ARM 和 ST-Link 驱动。
2. 从 GitHub 获取工程代码。
3. 使用 Keil µVision 打开 `Project.uvprojx`。
4. 检查工程中的源文件路径和目标芯片是否为 `STM32F103C8`。
5. 使用 ST-Link 将主控板连接到电脑，用于下载和调试程序。

```text
git clone https://github.com/alexwang0529/Intelligence_Car.git
```

## 4. 构建（如适用）

本项目使用 Keil µVision 构建，不需要 ROS 或 Python 环境。

```text
打开 Project.uvprojx
选择 Project -> Build Target，或按 F7
检查 Build Output 中是否显示 0 Error(s)
```

编译成功后，通过 `Flash -> Download` 或下载按钮将程序烧录到 STM32F103C8T6。

## 5. 快速开始

```text
1. 将四路电机、编码器、MPU6050、OLED、超声波和循迹模块接好。
2. 确认电机驱动板、主控板和传感器共地。
3. 接通电池，为小车提供外部电源。
4. 使用 ST-Link 下载程序并复位主控板。
5. 通过 K1 切换模式，通过 K2 确认模式。
```

当前程序包含以下模式：自动纠偏、指定角度、里程计、循线行驶和物体测距。

预期现象 / 结果：

- OLED 显示当前模式和运行状态。
- 自动纠偏模式下，小车根据 MPU6050 的航向角修正直行方向。
- 指定角度模式下，小车完成设定角度转向。
- 里程计模式下，小车按照编码器计数行驶目标距离。
- 循线行驶模式下，小车根据四路循迹传感器沿黑线行驶。
- 物体测距模式下，小车循线行驶，并使用右侧超声波统计物体数量和距离。

## 6. 配置说明

### STM32 引脚表

| STM32 引脚 | 外设或模块 | 功能 |
| --- | --- | --- |
| PA1 | D24A PWMC | 电机 C PWM |
| PA2 | D24A CIN1 | 电机 C 方向 1 |
| PA3 | D24A CIN2 | 电机 C 方向 2 |
| PA4 | HC-SR04 TRIG | 超声波触发 |
| PA5 | HC-SR04 ECHO | 超声波回波 |
| PA6 | D24A PWMA | 电机 A PWM |
| PA7 | D24A PWMB | 电机 B PWM |
| PA8 | D24A PWMD | 电机 D PWM |
| PA9 | JDY-31 USART1_TX | 蓝牙发送，当前程序可选启用 |
| PA10 | JDY-31 USART1_RX | 蓝牙接收，当前程序可选启用 |
| PA11 | OLED、MPU6050 | 软件 I²C SCL |
| PA12 | OLED、MPU6050 | 软件 I²C SDA |
| PA15 | D24A DIN1 | 电机 D 方向 1 |
| PB0 | 四路循迹模块 X1 | 左外侧循迹输入 |
| PB1 | 四路循迹模块 X2 | 左内侧循迹输入 |
| PB3 | 按键 K1 | 模式切换 |
| PB4 | 按键 K2 | 模式确认或停止 |
| PB5 | D24A AIN1 | 电机 A 方向 1 |
| PB6 | D24A AIN2 | 电机 A 方向 2 |
| PB7 | D24A BIN1 | 电机 B 方向 1 |
| PB8 | D24A BIN2 | 电机 B 方向 2 |
| PB9 | D24A DIN2 | 电机 D 方向 2 |
| PB10 | 四路循迹模块 X3 | 右内侧循迹输入 |
| PB11 | 四路循迹模块 X4 | 右外侧循迹输入 |
| PB12 | 左后轮编码器 A 相 | 编码器输入 |
| PB13 | 左后轮编码器 B 相 | 编码器输入 |
| PB14 | 右前轮编码器 A 相 | 编码器输入 |
| PB15 | 右前轮编码器 B 相 | 编码器输入 |
| PC13 | 板载 LED | 状态指示，可选使用 |
| GND | 所有模块 | 公共地，必须连接 |

> 由于引脚不足，本版本仅链接了两个编码器

### 主要运行参数

| 配置项 | 默认值 | 说明 |
| --- | --- | --- |
| 电机反向配置 | A=0，B=1，C=1，D=0 | 用于匹配四个电机的实际正转方向 |
| 编码器脉冲数 | 728 脉冲/轮转一圈 | 减速比 28、霍尔线数 13、四倍频 |
| 车轮直径 | 6.5 cm | 用于里程计算 |
| 物体检测距离 | 30 cm | 小于等于该距离时判断发现物体 |
| 物体释放距离 | 35 cm | 连续离开该距离后允许识别下一个物体 |

## 7. 目录结构

```text
.
├── Project.uvprojx       # Keil 主工程文件
├── Hardware/             # 硬件驱动
│   ├── Bluetooth.c/.h      # 蓝牙模块（未经验证）
│   ├── DirectionHold.c/.h  # 自动纠偏模块
│   ├── Encoder.c/.h        # 编码器
│   ├── Heading.c/.h        # 转向角度控制模块
│   ├── Key.c/.h            # 按键
│   ├── LED.c/.h            # LED（未使用）
│   ├── LineSensor.c/.h     # 循线
│   ├── Motor.c/.h          # 电机
│   ├── MPU6050.c/.h        # 陀螺仪
│   ├── MPU6050_Reg.h      
│   ├── MyI2C.c/.h          # I2C通信
│   ├── OLED.c/.h           # OLED
│   ├── OLED_Font.h
│   ├── Turn.c/.h           # 转向
│   └── Ultrasonic.c/.h     # 超声波模块
├── System/                 # 软件驱动
│   ├── Delay.c/.h
│   ├── Menu.c/.h          # 主菜单
│   ├── Mode.c/.h          # 模式选择
│   └── ObjectCounter.c/.h # 物体计数
├── User/                 # main 函数和用户入口代码
├── Start/                # STM32 启动文件和系统初始化文件
├── Library/              # STM32 标准外设库和 CMSIS 文件
├── docs/                 # 项目文档
│   └── plan.md           # 项目方案和开发计划
├── DebugConfig/          # 调试配置
└── 环境要求.md           # 项目环境与使用说明
```

## 8. 文档

- `docs/plan.md`：项目规划（必须）
- `.github/CONTRIBUTING.md`：项目协作流程（使用 Repository Template 时自带）
- `docs/architecture.md`：系统架构（如有）
- `docs/interface.md`：接口说明（如有）
- `docs/deployment.md`：部署说明（如有）
- `docs/calibration.md`：标定说明（如有）
- `docs/troubleshooting.md`：故障排查（如有）

## 9. 常见问题

### 问题 1

现象：

原因：

处理：

## 10. 版本与发布

正式稳定版本使用 Git Tag + GitHub Release 发布

版本历史见：<Releases 链接>

## 11. 维护者

- Maintainer / 项目负责人：`@alexwang0529`
