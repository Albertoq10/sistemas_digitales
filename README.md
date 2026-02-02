Control domótico de persianas IoT (STM32 + Wi-Fi + HTTP Server)
Proyecto de domótica en STM32 que controla 2 persianas mediante servos (PWM) con 3 modos: automático (según luz), manual y vacaciones/aleatorio. Incluye lectura de luz por ADC, pantalla OLED SSD1306 (I2C) y un servidor HTTP embebido para consultar estado y cambiar parámetros desde un navegador.

✅ Funcionalidades
Control de 2 servos (persianas) mediante PWM
Lectura de LDR por ADC para modo automático
Pantalla OLED SSD1306 por I2C (estado/modo/valores)
Conectividad Wi-Fi y servidor HTTP en la placa
Endpoints GET/POST para:
consultar estado (modo, posiciones, lectura LDR)
cambiar modo/posiciones/parámetros

🧩 Arquitectura
Sensores/ADC (LDR) → lógica de control (modo auto/manual/vacaciones) → PWM servos
UI local (OLED I2C) + UI remota (HTTP)

⚙️ Modos de operación
Manual: posiciones controladas por comandos (HTTP)
Automático: ajusta persianas según umbral/lectura de luz (LDR)
Vacaciones/Aleatorio: movimiento en patrones para mver servos


🌐 API HTTP
GET
/status → devuelve estado actual (modo, posición persiana 1/2, LDR)
POST
/mode → cambia modo (manual|auto|vacaciones)
/blind/1 → cambia posición persiana 1 (0–100%)
/blind/2 → cambia posición persiana 2 (0–100%)


Hardware usado
Placa STM32: (B-L475E-IOT01A / STM32L475)
LDR (ADC)
2 servos (PWM)
OLED SSD1306 128x32 (I2C)
Módulo Wi-Fi
