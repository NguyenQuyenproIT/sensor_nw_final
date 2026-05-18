STM32 (soil + DHT22) - README

Chức năng:

- Đọc ADC (soil moisture) và DHT22 (temperature, humidity).
- Lọc/trung bình ADC, kiểm tra checksum DHT22.
- Gói dữ liệu thành JSON và gửi qua UART2 (baud 115200, newline terminated).

Build:

- Mở project bằng Keil/STM32CubeIDE/Makefile tùy cấu trúc.
- Kiểm tra `main_code.c` để biết chân kết nối và baudrate.

Chú ý:

- DHT22 yêu cầu pull-up và timing chính xác (bit-banging).
- Đảm bảo GND chung với ESP32.
