
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


## 3. 安装

```bash
# 示例
```

## 4. 构建（如适用）

```bash
# 示例
```

> 文档、模型、数据等非软件仓库可将本节改为“获取 / 导入 / 使用方式”

## 5. 快速开始

```bash
# 最小可运行示例
```

预期现象 / 结果：

- ...

## 6. 配置说明

| 配置项 | 默认值 | 说明 |
| --- | --- | --- |
| `<param>` | `<value>` | ... |

## 7. 目录结构

```text
.
├── README.md
├── docs/
│   └── plan.md
└── ...
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
