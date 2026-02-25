#include <Arduino.h>
#include "soft_smi.h"

uint8_t phyAddr = 0x00; // 默认 PHY 地址
uint8_t regAddr = 0x00; // 默认寄存器地址
bool readFlag = false;  // 默认不进行读操作

void setup()
{
  // 初始化串口
  Serial.begin(115200);
  while (!Serial)
  {
    ; // 等待串口准备好
  }

  // 初始化 SMI（软 MII 接口）
  soft_smi_init();

  // 打印提示信息
  Serial.println("Configure PHY address: type 'P <phyAddr>'");
  Serial.println("Configure register address and read/write: type 'R <regAddr>' for read or 'W <regAddr> <value>' for write");
  Serial.println("Advanced MMD access: type 'M <devAddr> <regAddr> <R/W> [value]'");
}

void loop()
{
  static String input = ""; // 用于存储用户输入

  if (Serial.available() > 0)
  {
    char c = Serial.read();
    Serial.print(c); // 实时回显输入的字符

    if (c == '\n')
    { // 按回车键时处理命令
      input.trim();

      if (input.startsWith("P "))
      {
        // 配置 PHY 地址
        phyAddr = strtoul(input.substring(2).c_str(), NULL, 16); // 直接读取 16 进制值
        Serial.print("\nConfigured PHY Address: 0x");
        Serial.println(phyAddr, HEX);
      }
      else if (input.startsWith("R "))
      {
        // 配置寄存器地址并读取
        regAddr = strtoul(input.substring(2).c_str(), NULL, 16); // 直接读取 16 进制值
        readFlag = true;

        uint16_t regValue = smi_read_reg(phyAddr, regAddr);

        Serial.print("\nRead PHY Address: 0x");
        Serial.println(phyAddr, HEX);
        Serial.print("Register Address: 0x");
        Serial.println(regAddr, HEX);
        Serial.print("Register Value: 0x");
        Serial.println(regValue, HEX);
      }
      else if (input.startsWith("W "))
      {
        // 配置寄存器地址并写入值
        int firstSpace = input.indexOf(' ');
        int secondSpace = input.indexOf(' ', firstSpace + 1);

        if (secondSpace > 0)
        {
          regAddr = strtoul(input.substring(firstSpace + 1, secondSpace).c_str(), NULL, 16); // 读取 16 进制地址
          uint16_t regValue = strtoul(input.substring(secondSpace + 1).c_str(), NULL, 16);   // 读取 16 进制值

          smi_set_reg(phyAddr, regAddr, regValue);

          Serial.print("\nWrite PHY Address: 0x");
          Serial.println(phyAddr, HEX);
          Serial.print("Register Address: 0x");
          Serial.println(regAddr, HEX);
          Serial.print("Register Value: 0x");
          Serial.println(regValue, HEX);
        }
        else
        {
          Serial.println("\nInvalid write command format. Use 'W <regAddr> <value>'");
        }
      }
      else if (input.startsWith("M "))
      {
        // 高级 MMD 访问命令
        int firstSpace = input.indexOf(' ');
        int secondSpace = input.indexOf(' ', firstSpace + 1);
        int thirdSpace = input.indexOf(' ', secondSpace + 1);

        if (secondSpace > 0 && thirdSpace > 0)
        {
          uint8_t devAddr = strtoul(input.substring(firstSpace + 1, secondSpace).c_str(), NULL, 16);  // 设备地址
          uint16_t regAddr = strtoul(input.substring(secondSpace + 1, thirdSpace).c_str(), NULL, 16); // 寄存器地址
          String operation = input.substring(thirdSpace + 1);

          if (operation.startsWith("R"))
          {
            // 写入设备地址到寄存器 0x0D
            smi_set_reg(phyAddr, 0x0D, (devAddr & 0x1F));
            // 写入目标寄存器地址到寄存器 0x0E
            smi_set_reg(phyAddr, 0x0E, regAddr);
            // 配置访问模式为读（0x4000）
            smi_set_reg(phyAddr, 0x0D, (devAddr & 0x1F) | 0x4000);
            // 读取数据
            uint16_t regValue = smi_read_reg(phyAddr, 0x0E);

            Serial.print("\nMMD Read PHY Address: 0x");
            Serial.println(phyAddr, HEX);
            Serial.print("Device Address: 0x");
            Serial.println(devAddr, HEX);
            Serial.print("Register Address: 0x");
            Serial.println(regAddr, HEX);
            Serial.print("Register Value: 0x");
            Serial.println(regValue, HEX);
          }
          else if (operation.startsWith("W "))
          {
            // 获取写入值
            uint16_t regValue = strtoul(operation.substring(2).c_str(), NULL, 16);
            // 写入设备地址到寄存器 0x0D
            smi_set_reg(phyAddr, 0x0D, (devAddr & 0x1F));
            // 写入目标寄存器地址到寄存器 0x0E
            smi_set_reg(phyAddr, 0x0E, regAddr);
            // 配置访问模式为写（0x4000）
            smi_set_reg(phyAddr, 0x0D, (devAddr & 0x1F) | 0x4000);
            // 写入数据到寄存器 0x0E
            smi_set_reg(phyAddr, 0x0E, regValue);

            Serial.print("\nMMD Write PHY Address: 0x");
            Serial.println(phyAddr, HEX);
            Serial.print("Device Address: 0x");
            Serial.println(devAddr, HEX);
            Serial.print("Register Address: 0x");
            Serial.println(regAddr, HEX);
            Serial.print("Register Value: 0x");
            Serial.println(regValue, HEX);
          }
          else
          {
            Serial.println("\nInvalid MMD operation. Use 'M <devAddr> <regAddr> R' or 'M <devAddr> <regAddr> W <value>'");
          }
        }
        else
        {
          Serial.println("\nInvalid MMD command format. Use 'M <devAddr> <regAddr> R' or 'M <devAddr> <regAddr> W <value>'");
        }
      }
      else
      {
        Serial.println("\nInvalid command. Use 'P <phyAddr>', 'R <regAddr>', 'W <regAddr> <value>', or 'M <devAddr> <regAddr> <R/W> [value]'");
      }

      input = ""; // 清空输入缓冲区
    }
    else
    {
      input += c; // 继续读取输入
    }
  }
}
