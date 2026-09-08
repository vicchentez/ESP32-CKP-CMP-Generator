# Diagrama de Conexiones - ESP32 CKP & CMP Signal Generator

## 📐 DIAGRAMA ESQUEMÁTICO COMPLETO

```
╔════════════════════════════════════════════════════════════════════════════╗
║                    ESP32 CKP & CMP SIGNAL GENERATOR                        ║
║                         ESQUEMA DE CONEXIONES                              ║
╚════════════════════════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────────────────────────┐
│                          BLOQUE ALIMENTACIÓN                                │
└─────────────────────────────────────────────────────────────────────────────┘

                            ┌──────────┐
                        USB │ MICRO-B  │
                            │ 5V ──────┼─────┐
                            │ GND ─────┼─┐   │
                            └──────────┘ │   │
                                         │   │
                                    ┌────▼──▼────┐
                                    │  Regulador │
                                    │   5V / 1A  │
                                    │ (Optional) │
                                    └────┬───────┘
                                         │
                    ┌────────────────────┼────────────────────┐
                    │                    │                    │
                   5V                   GND                  3.3V
                    │                    │                    │
        ┌───────────┴────────┬───────────┴─────┬──────────────┘
        │                    │                 │
      5V                   GND               3.3V
```

---

## 🖥️ DIAGRAMA DETALLADO POR MÓDULOS

### MÓDULO 1: ESP32 DevKit + LCD I2C

```
┌──────────────────────────────────────────────────────────────┐
│                      ESP32 DevKit V1                          │
│                                                               │
│  USB   GND  D23  D19  D18  D5   D4   D0   D2   D15  D8  D7   │
│   ║     ║                                                      │
│   ║     ║                                              ┌──────┐
│   ║     └──────────────────────────────────────────────┤ GND  │
│   │                                                    └──────┘
│   │                                          ┌────────────────┐
│   │                                          │  LCD 16x2 I2C  │
│   └─────────────────────────────────────────►│ (0x27 / 0x3F)  │
│       5V                                      │                │
│                                               │ VCC ◄─── 5V   │
│                              D21 (SDA) ◄──────┤ SDA            │
│                              D22 (SCL) ◄──────┤ SCL            │
│                                       GND ────┤ GND            │
│                                               │                │
│                                               │ Pantalla:      │
│                                               │ 16 caracteres  │
│                                               │ 2 líneas       │
│                                               └────────────────┘
│
│  D6   D11  D1   GND  5V   A0   A1   A2   A3   A4   A5   D9  D10 │
│                              │                                   │
│                              └──────┐                            │
└────────────────��─────────────────────┼────────────────────────────┘
                                       │
```

---

### MÓDULO 2: Potenciómetro RPM

```
┌────────────────────────────────────────────────────────────────┐
│                    POTENCIÓMETRO 10kΩ                          │
│                    (Control de RPM)                            │
└────────────────────────────────────────────────────────────────┘

    ┌─ Extremo 1 (Patilla Izquierda)
    │
    ├─────────────────────────────────┐
    │                                 │
    │    Resistencia 10kΩ             │
    │    ┌───[10kΩ]────┐            │
    │    │              │            │
    │    ├──────●────────┤            │
    │    │      ▲        │            │
    │    │      │        │            │
    │    │   Cursor      │            │
    │    │   (salida)    │            │
    │    │      │        │            │
    │    └──────┼────────┘            │
    │           │                    │
    │           ├─────► GPIO 34      │
    │           │      (ADC - A2)    │
    │           │                    │
    └─ Extremo 2 (Patilla Centro) ────┴──► GND
    │
    └─ Extremo 3 (Patilla Derecha) ──────► 3.3V

    Rango de voltaje: 0 a 3.3V
    Mapeo: 0V = 0 RPM
           1.65V = 4000 RPM
           3.3V = 8000 RPM

    Opcional: Capacitor 100nF entre OUT y GND (anti-ruido)
    
           GPIO 34 ──────┬──────► ADC
                         │
                      [100nF]
                         │
                        GND
```

---

### MÓDULO 3: Botones de Menú

```
┌─────────────────────────────────────────────────────────────────┐
│                    BOTONES DE CONTROL                           │
│              (Pull-Ups Internos Activados)                      │
└─────────────────────────────────────────────────────────────────┘

BOTÓN UP (GPIO 32)           BOTÓN DOWN (GPIO 33)
─────────────────            ──────────────────
  
  3.3V                         3.3V
   ▲                            ▲
   │                            │
   │                            │
┌──┴──┐                      ┌──┴──┐
│     │ Push                 │     │ Push
│ ○─○ │ ───► Contacto      │ ○─○ │ ───► Contacto
│     │                      │     │
└──┬──┘                      └──┬──┘
   │                            │
   │ (Cuando se presiona)       │ (Cuando se presiona)
   │ Contacto abierto = 3.3V    │ Contacto abierto = 3.3V
   │ Contacto cerrado = 0V      │ Contacto cerrado = 0V
   │                            │
  GND                          GND
   ▲                            ▲
   │ (Pull-Up interno)          │ (Pull-Up interno)
   │ 30-50kΩ                    │ 30-50kΩ
   │                            │
GPIO 32                        GPIO 33


BOTÓN MENU (GPIO 14)         BOTÓN OK (GPIO 27)
─────────────────            ──────────────
  
  3.3V                         3.3V
   ▲                            ▲
   │                            │
   │                            │
┌──┴──┐                      ┌──┴──┐
│     │ Push                 │     │ Push
│ ○─○ │ ───► Contacto      │ ○─○ │ ───► Contacto
│     │                      │     │
└──┬──┘                      └──┬──┘
   │                            │
  GND                          GND
   ▲                            ▲
   │ (Pull-Up interno)          │ (Pull-Up interno)
   │ 30-50kΩ                    │ 30-50kΩ
   │                            │
GPIO 14                        GPIO 27


Comportamiento:
  - Botón sin presionar: GPIO = 3.3V (HIGH = 1)
  - Botón presionado:    GPIO = 0V   (LOW = 0)
```

---

### MÓDULO 4: Salidas de Señales CKP y CMP

```
┌───────────────────────────────────────────────────────────────┐
│              SALIDAS DE SEÑALES (GPIO 25 y 26)               │
│                                                               │
│  OPCIÓN A: Conexión Directa (Simulador/Osciloscopio)        │
│  ────────────────────────────────────────────────────────────│
│                                                               │
│   GPIO 25 (CKP)              GPIO 26 (CMP)                  │
│       │                           │                          │
│       │ 0-3.3V                    │ 0-3.3V                  │
│       │                           │                          │
│       ├──────► [CKP_OUT]          │                          │
│       │        (a osciloscopio)   │                          │
│       │                           ├──────► [CMP_OUT]         │
│       │                           │        (a osciloscopio)  │
│       │                           │                          │
│      GND ─────────────────────────┴──────► GND               │
│                                                               │
│  OPCIÓN B: Con Acondicionador (Para ECU 12V)               │
│  ────────────────────────────────────────────────────────────│
│                                                               │
│   GPIO 25 (CKP)                                              │
│       │                                                       │
│       │ 0-3.3V                    ┌──────────────┐           │
│       ├────────────────────────────┤ Entrada 1    │           │
│       │                            │   ULN2803    │───► PULL-UP ──────► ECU
│      GND ──────────────────────────┤ GND (Pin 8)  │   4.7kΩ a 5V       (CKP_OUT)
│                                    └──────────────┘           │
│                                                               │
│   GPIO 26 (CMP)                                              │
│       │                                                       │
│       │ 0-3.3V                    ┌──────────────┐           │
│       ├────────────────────────────┤ Entrada 2    │           │
│       │                            │   ULN2803    │───► PULL-UP ──────► ECU
│      GND ──────────────────────────┤ GND (Pin 8)  │   4.7kΩ a 5V       (CMP_OUT)
│                                    └──────────────┘           │
│                                                               │
└───────────────────────────────────────────────────────────────┘

Características de las señales:
  • Voltaje: 0-3.3V (TTL)
  • Frecuencia CKP: 0-20 kHz (típico @ 8000 RPM)
  • Frecuencia CMP: 0-300 Hz (típico)
  • Ancho de pulso: 2ms (configurable)
  • Duty Cycle: ~20% (2ms HIGH, 8ms LOW @ típico)
```

---

## 📋 TABLA DE CONEXIONES GENERAL

```
┌───────────────────┬──────────────┬──────────────────┬──────────────────┐
│   COMPONENTE      │  PIN ESP32   │   CONEXIÓN       │   OBSERVACIONES  │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ LCD I2C (SDA)     │ GPIO 21 (D21)│ ← → LCD SDA      │ Pull-ups 4.7kΩ   │
│ LCD I2C (SCL)     │ GPIO 22 (D22)│ ← → LCD SCL      │ Pull-ups 4.7kΩ   │
│ LCD (VCC)         │ 5V           │ ← → LCD VCC      │ Alimentación     │
│ LCD (GND)         │ GND          │ ← → LCD GND      │ Masa común       │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Potenciómetro     │ GPIO 34 (A2) │ ← ← → Patilla 1  │ Salida analógica │
│ Potenciómetro GND │ GND          │ ← → Patilla 2    │ Referencia 0V    │
│ Potenciómetro +   │ 3.3V         │ ← → Patilla 3    │ Alimentación     │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Botón UP          │ GPIO 32 (A4) │ ← ← → Contacto   │ Pull-up interno  │
│ Botón UP GND      │ GND          │ ← → Contacto     │ Tierra           │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Botón DOWN        │ GPIO 33 (A5) │ ← ← → Contacto   │ Pull-up interno  │
│ Botón DOWN GND    │ GND          │ ← → Contacto     │ Tierra           │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Botón MENU        │ GPIO 14      │ ← ← → Contacto   │ Pull-up interno  │
│ Botón MENU GND    │ GND          │ ← → Contacto     │ Tierra           │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Botón OK          │ GPIO 27      │ ← ← → Contacto   │ Pull-up interno  │
│ Botón OK GND      │ GND          │ ← → Contacto     │ Tierra           │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Salida CKP        │ GPIO 25      │ ► CKP_OUT        │ 0-3.3V TTL       │
│ CKP Tierra        │ GND          │ → GND            │ Masa común       │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Salida CMP        │ GPIO 26      │ ► CMP_OUT        │ 0-3.3V TTL       │
│ CMP Tierra        │ GND          │ → GND            │ Masa común       │
├───────────────────┼──────────────┼──────────────────┼──────────────────┤
│ Alimentación      │ 5V (USB)     │ ► Regulador      │ ~1A mínimo       │
│ Tierra Común      │ GND          │ ◄─► Todos GND    │ Tierra común     │
│ 3.3V              │ 3.3V Onboard │ ► Potenciómetro  │ Regulador interno│
└───────────────────┴──────────────┴──────────────────┴──────────────────┘
```

---

## 🎯 VISTA SUPERIOR DEL PROTOBOARD (Conexión 30 cables)

```
LADO IZQUIERDO (Potencia y GND)              LADO DERECHO (Señales)
═════════════════════════════════════════════════════════════════════

    5V GND   3.3V                                LCD GND   LCD SDA LCD SCL
    ║  ║     ║                                   ║     ║      ║      ║
    ║  ║     ║                                   ║     ║      ║      ║
    ╬══╬═════╬═══════════════════════════════════╬═════╬══════╬══════╬════
    
    ESP32
    ┌─────────────────────────────────────────────────────────────┐
    │                                                             │
    │   ARRIBA (Números GPIO)                                    │
    │                                                             │
    │   D21 D22 D27 D14 D25 D26 D32 D33 D34  ...                 │
    │   │   │   │   │   │   │   │   │   │                        │
    │   ├─→ LCD SCL                                              │
    │   ├─→ LCD SDA                                              │
    │   │                                                         │
    │   ├─→ BOTÓN OK                                             │
    │   ├─→ BOTÓN MENU                                           │
    │   │                                                         │
    │   ├─→ CKP OUT (GPIO 25)                                    │
    │   ├─→ CMP OUT (GPIO 26)                                    │
    │   │                                                         │
    │   ├─→ BOTÓN UP (GPIO 32)                                   │
    │   ├─→ BOTÓN DOWN (GPIO 33)                                 │
    │   │                                                         │
    │   ├─→ POTENCIÓMETRO (GPIO 34 / A2)                         │
    │   │                                                         │
    │   ABAJO (GND y 5V)                                         │
    │                                                             │
    └─────────────────────────────────────────────────────────────┘

CABLE GRIS = GND
CABLE ROJO = 5V / 3.3V
CABLE NEGRO = Datos/Señales
```

---

## 🔌 ORDEN DE CONEXIÓN PASO A PASO

### Paso 1: ALIMENTACIÓN (Cables 1-3)
```
1. Cable 5V (Rojo)     → Pin 5V del ESP32
2. Cable GND (Negro)   → Pin GND del ESP32
3. Cable 3.3V (Gris)   → Pin 3.3V del ESP32
```

### Paso 2: LCD I2C (Cables 4-7)
```
4. Cable GND LCD       → GND (Gris)
5. Cable 5V LCD        → 5V (Rojo)
6. Cable SDA LCD       → GPIO 21 (D21)
7. Cable SCL LCD       → GPIO 22 (D22)
```

### Paso 3: POTENCIÓMETRO (Cables 8-10)
```
8. Patilla 1 (Centro)  → GND
9. Patilla 2 (Derecha) → 3.3V
10. Patilla 3 (Salida) → GPIO 34 (A2)
   [Opcional: Capacitor 100nF entre 10 y GND]
```

### Paso 4: BOTONES (Cables 11-18)
```
11. Botón UP A         → GPIO 32
12. Botón UP B         → GND

13. Botón DOWN A       → GPIO 33
14. Botón DOWN B       → GND

15. Botón MENU A       → GPIO 14
16. Botón MENU B       → GND

17. Botón OK A         → GPIO 27
18. Botón OK B         → GND
```

### Paso 5: SALIDAS DE SEÑALES (Cables 19-22)
```
19. CKP Salida         → GPIO 25
20. CKP Tierra         → GND

21. CMP Salida         → GPIO 26
22. CMP Tierra         → GND
```

---

## 📸 ESQUEMA SIMPLIFICADO EN ASCII

```
                    ┌────────────────┐
                    │   ALIMENTACIÓN │
                    │    5V / GND    │
                    └────────┬───────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
    ┌─────────┐         ┌─────────┐        ┌──────────┐
    │  LCD    │         │  ESP32  │        │  Sensores│
    │  I2C    │         │ DevKit  │        │  (Botón) │
    └────┬────┘         └────┬────┘        └────┬─────┘
         │                   │                  │
         │◄──► I2C ──────────┤                  │
         │   (SDA/SCL)       │                  │
         │                   │◄─ Entrada Anal. ─┤ Potenciómetro
         │                   │                  │
         │                   │◄─ Entrada Dig. ──┤ Botones
         │                   │                  │
         │                   ├─ Salida Dig. ───►├─ CKP
         │                   │                  │
         │                   ├─ Salida Dig. ───►├─ CMP
         │                   │                  │
         └───────────────────┴──────────────────┘
                        ▲        ▲        ▲
                        │        │        │
                       GND      GND      GND
                     (Común)
```

---

## ⚠️ NOTAS IMPORTANTES

### ✅ Verificaciones Antes de Encender

1. **Polaridad de alimentación**
   - 5V de USB → Pin 5V
   - GND → Pin GND
   - ✅ Verificar 2 veces

2. **Conexiones I2C**
   - SDA (GPIO 21) = Línea DATA
   - SCL (GPIO 22) = Línea CLOCK
   - ✅ No invertir

3. **Botones y Potenciómetro**
   - Todos los GND deben estar conectados
   - Pull-ups internos activados en código
   - ✅ No necesitan resistencias externas

4. **Salidas CKP/CMP**
   - GPIO 25 y 26 en bajo consumo (50mA máx)
   - ✅ Seguro para osciloscopio directo
   - ⚠️ Para ECU 12V usar ULN2803

### 🛠️ Herramientas Necesarias

- Cables Dupont hembra-macho (mínimo 22)
- Cable USB Micro-B
- Osciloscopio (recomendado para verificación)
- Multímetro (para verificar voltajes)
- Soldador (opcional, si se desea permanente)

### 📐 Tolerancias

- Voltajes: ±0.2V
- Resistencias: ±10%
- Frecuencias: ±1% (limitado por cristal ESP32)

---

**Versión:** 1.0  
**Fecha:** 2026  
**Autor:** Vicchentez  
**Licencia:** MIT

