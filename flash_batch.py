#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import esptool
import serial.tools.list_ports
import sys
import time

# =============================================================================
# ESP8266 Batch Flash Script
# =============================================================================
# 用法: python flash_batch.py [firmware.bin]
# 預設 firmware: firmware.bin
# =============================================================================

FIRMWARE_DEFAULT = "firmware.bin"
CHIP_TYPE = "esp8266"
BAUD_RATE = 115200
FLASH_ADDRESS = "0x0"


def GetPortList():
    """取得所有可用的 COM Port"""
    ports = serial.tools.list_ports.comports()
    portList = [p.device for p in ports]
    return portList


def FlashDevice(_firmware, _port):
    """燒錄單一裝置"""
    print(f"→ 開始燒錄 {_port}...")
    
    try:
        esptool.main([
            "--chip", CHIP_TYPE,
            "--port", _port,
            "--baud", str(BAUD_RATE),
            "write_flash", FLASH_ADDRESS, _firmware
        ])
        print(f"✓ {_port} 燒錄成功")
        return True
        
    except Exception as e:
        print(f"✗ {_port} 燒錄失敗: {e}")
        return False


def BatchFlash(_firmware):
    """批次燒錄所有已連接的 ESP8266"""
    portList = GetPortList()
    
    if not portList:
        print("✗ 找不到任何 COM Port，請確認 ESP8266 已連接")
        return
    
    print(f"≡ 找到 {len(portList)} 個 COM Port: {portList}")
    print(f"≡ 開始批次燒錄 firmware: {_firmware}")
    print("=" * 50)
    
    successCount = 0
    failCount = 0
    
    for port in portList:
        if FlashDevice(_firmware, port):
            successCount += 1
        else:
            failCount += 1
        time.sleep(1)  # 等待裝置就緒
    
    print("=" * 50)
    print(f"≡ 燒錄完成: 成功 {successCount}, 失敗 {failCount}")


def main():
    """主程式"""
    # 取得 firmware 檔名
    firmware = FIRMWARE_DEFAULT
    if len(sys.argv) > 1:
        firmware = sys.argv[1]
    
    print("╔══════════════════════════════════════╗")
    print("║   ESP8266 Batch Flash Tool v1.0      ║")
    print("╚══════════════════════════════════════╝")
    print("")
    
    BatchFlash(firmware)


if __name__ == "__main__":
    main()