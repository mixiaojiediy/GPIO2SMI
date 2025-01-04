# GPIO模拟SMI（MDIO）协议软件实现

## 简介
本资源文件提供了一个基于GPIO模拟SMI（MDIO）协议的软件实现方案。该方案具有高度的移植性，可以快速应用于任意MCU/SOC平台，实现稳定可靠的SMI读写功能。通过本方案，可以轻松地在不同硬件平台上实现SMI协议，无需复杂的硬件改动。

## 功能特点
1. **高度可移植性**：支持任意MDC和MDIO端口的指定，移植性强。
2. **简易移植**：只需实现几个底层函数即可完成移植工作，无需复杂的配置。
3. **支持HAL库**：如果平台使用HAL库，无需任何修改，可直接使用stm32文件夹版本。
4. **支持ESP32 Arduino框架**：如果使用ESP32，也可以直接使用esp32文件夹版本。

## 使用说明
1. **配置端口**：根据硬件平台，指定MDC和MDIO的GPIO端口。
2. **实现底层函数**：根据提供的接口，实现底层GPIO操作函数。
3. **集成到项目**：将本方案集成到项目中，调用SMI读写接口即可实现SMI功能。

## 联系方式

微信：zj907638274

哔哩哔哩：https://space.bilibili.com/90511516

CSDN：https://blog.csdn.net/zj907638274

知乎：https://www.zhihu.com/people/fang-yuan-ji-li-46

github：https://github.com/mixiaojiediy

gitee：https://gitee.com/mixiaojiediy

有疑问可以交流；



