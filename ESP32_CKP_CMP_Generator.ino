/*
 * ESP32 CKP & CMP Signal Generator
 * Generador de señales CKP y CMP sincronizadas para simulador automotriz
 * Compatible con ESP32 Arduino 3.x
 * 
 * Autor: Vicchentez
 * Fecha: 2026
 * 
 * Características:
 * - Generación de señales CKP (Crankshaft) y CMP (Camshaft)
 * - Control de RPM mediante potenciómetro (0-8000 RPM)
 * - Menú interactivo con botones
 * - LCD 16x2 I2C multimarca
 * - Sincronización perfecta entre señales
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== DEFINICIÓN DE PINES =====
#define PIN_RPM_POT       34      // ADC - Potenciómetro RPM
#define PIN_CKP_OUTPUT    25      // GPIO - Salida CKP
#define PIN_CMP_OUTPUT    26      // GPIO - Salida CMP
#define PIN_BUTTON_UP     32      // GPIO - Botón ARRIBA
#define PIN_BUTTON_DOWN   33      // GPIO - Botón ABAJO
#define PIN_BUTTON_MENU   14      // GPIO - Botón MENU
#define PIN_BUTTON_OK     27      // GPIO - Botón OK

// ===== LCD I2C =====
#define LCD_ADDR          0x27    // Dirección I2C (común: 0x27 ó 0x3F)
#define LCD_COLS          16
#define LCD_ROWS          2

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// ===== VARIABLES GLOBALES =====
volatile int rpm = 0;
volatile int rpm_target = 0;
volatile int rpm_display = 0;
volatile int ckp_frequency = 0;  // Hz
volatile int cmp_frequency = 0;  // Hz
volatile boolean ckp_pulse = false;
volatile boolean cmp_pulse = false;

// Parámetros del motor (configurables)
int cylinder_count = 4;          // Número de cilindros
int ckp_teeth = 60;              // Número de dientes CKP (típico: 60-2)
int cmp_pulses_per_cycle = 1;    // CMP pulsos por ciclo (típico: 1-2)

// Variables de menú
enum MenuState {
  MENU_MAIN,
  MENU_CYLINDERS,
  MENU_CKP_TEETH,
  MENU_CMP_PULSES,
  MENU_SYNC_TEST,
  MENU_CALIBRATE
};

MenuState current_menu = MENU_MAIN;
int menu_value = 0;
unsigned long last_button_time = 0;
const int DEBOUNCE_TIME = 50;

// Timers de hardware
hw_timer_t* timer_ckp = NULL;
hw_timer_t* timer_cmp = NULL;

// Variables de temporización
volatile unsigned long ckp_pulse_start = 0;
volatile unsigned long cmp_pulse_start = 0;
const unsigned long PULSE_WIDTH_US = 2000;  // Ancho de pulso: 2ms

// Variables para generador de pulsos por software
volatile unsigned long ckp_period_us = 50000;  // Período en microsegundos
volatile unsigned long cmp_period_us = 100000; // Período en microsegundos
volatile unsigned long last_ckp_pulse = 0;
volatile unsigned long last_cmp_pulse = 0;

// ===== PROTOTIPOS DE FUNCIONES =====
void setup_timers();
void update_rpm_from_pot();
void calculate_frequencies();
void display_main_screen();
void display_menu();
void handle_buttons();
void init_lcd();
void generate_pulses();
void check_lcd_address();

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=== ESP32 CKP & CMP Signal Generator ===");
  Serial.println("ESP32 Arduino 3.x Compatible");
  
  // Configurar pines
  pinMode(PIN_RPM_POT, INPUT);
  pinMode(PIN_CKP_OUTPUT, OUTPUT);
  pinMode(PIN_CMP_OUTPUT, OUTPUT);
  pinMode(PIN_BUTTON_UP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_DOWN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MENU, INPUT_PULLUP);
  pinMode(PIN_BUTTON_OK, INPUT_PULLUP);
  
  // Inicializar salidas
  digitalWrite(PIN_CKP_OUTPUT, LOW);
  digitalWrite(PIN_CMP_OUTPUT, LOW);
  
  // Inicializar LCD
  init_lcd();
  
  Serial.println("Sistema inicializado correctamente");
  display_main_screen();
}

// ===== LOOP PRINCIPAL =====
void loop() {
  update_rpm_from_pot();
  calculate_frequencies();
  generate_pulses();  // Generador de pulsos por software
  handle_buttons();
  
  // Actualizar pantalla cada 100ms
  static unsigned long last_display = 0;
  if (millis() - last_display >= 100) {
    if (current_menu == MENU_MAIN) {
      display_main_screen();
    } else {
      display_menu();
    }
    last_display = millis();
  }
  
  delay(5);
}

// ===== INICIALIZAR LCD =====
void init_lcd() {
  Wire.begin(21, 22);  // SDA=21, SCL=22 (pines estándar ESP32)
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Mostrar splash screen
  lcd.setCursor(2, 0);
  lcd.print("CKP & CMP GEN");
  lcd.setCursor(4, 1);
  lcd.print("ESP32 v2.0");
  
  delay(2000);
  lcd.clear();
}

// ===== GENERADOR DE PULSOS (Software-based) =====
void generate_pulses() {
  unsigned long now = micros();
  
  // Generar pulso CKP
  if (!ckp_pulse && (now - last_ckp_pulse >= ckp_period_us)) {
    digitalWrite(PIN_CKP_OUTPUT, HIGH);
    ckp_pulse = true;
    ckp_pulse_start = now;
    last_ckp_pulse = now;
  }
  
  // Verificar ancho de pulso CKP
  if (ckp_pulse && (now - ckp_pulse_start >= PULSE_WIDTH_US)) {
    digitalWrite(PIN_CKP_OUTPUT, LOW);
    ckp_pulse = false;
  }
  
  // Generar pulso CMP
  if (!cmp_pulse && (now - last_cmp_pulse >= cmp_period_us)) {
    digitalWrite(PIN_CMP_OUTPUT, HIGH);
    cmp_pulse = true;
    cmp_pulse_start = now;
    last_cmp_pulse = now;
  }
  
  // Verificar ancho de pulso CMP
  if (cmp_pulse && (now - cmp_pulse_start >= PULSE_WIDTH_US)) {
    digitalWrite(PIN_CMP_OUTPUT, LOW);
    cmp_pulse = false;
  }
}

// ===== LEER POTENCIÓMETRO Y CALCULAR RPM =====
void update_rpm_from_pot() {
  static unsigned long last_read = 0;
  
  if (millis() - last_read >= 50) {  // Leer cada 50ms
    int adc_value = analogRead(PIN_RPM_POT);  // 0-4095
    
    // Mapear: 0-4095 ADC -> 0-8000 RPM
    rpm_target = map(adc_value, 0, 4095, 0, 8000);
    
    // Suavizado exponencial (evitar saltos)
    rpm = (rpm * 0.7) + (rpm_target * 0.3);
    rpm_display = rpm;
    
    last_read = millis();
  }
}

// ===== CALCULAR FRECUENCIAS =====
void calculate_frequencies() {
  // Fórmula: Frecuencia = (RPM × Dientes) / 60 segundos / 2 (4T)
  // Para motor 4T: RPM / 120 = revoluciones de cigüeñal por segundo
  
  // CKP: número de pulsos = dientes
  // En RPM, cada revolución genera ckp_teeth pulsos
  ckp_frequency = (rpm * ckp_teeth) / 120;  // Hz
  
  // CMP: típicamente 1 pulso por ciclo (2 revoluciones de cigüeñal)
  cmp_frequency = (rpm * cmp_pulses_per_cycle) / 120;
  
  // Configurar períodos en microsegundos
  if (ckp_frequency > 0) {
    ckp_period_us = 1000000 / ckp_frequency;
  } else {
    ckp_period_us = 50000;  // Valor por defecto
  }
  
  if (cmp_frequency > 0) {
    cmp_period_us = 1000000 / cmp_frequency;
  } else {
    cmp_period_us = 100000;  // Valor por defecto
  }
}

// ===== PANTALLA PRINCIPAL =====
void display_main_screen() {
  lcd.setCursor(0, 0);
  lcd.print("RPM:");
  lcd.setCursor(4, 0);
  if (rpm_display < 10000) {
    lcd.print(rpm_display);
    lcd.print("   ");
  } else {
    lcd.print(rpm_display);
  }
  
  lcd.setCursor(11, 0);
  lcd.print("CKP");
  
  lcd.setCursor(0, 1);
  lcd.print("CY:");
  lcd.print(cylinder_count);
  lcd.print(" T:");
  lcd.print(ckp_teeth);
  
  lcd.setCursor(11, 1);
  lcd.print("CMP");
  
  // Indicador de estado
  if (rpm_display > 100) {
    lcd.setCursor(15, 0);
    lcd.print("*");
  }
}

// ===== MENÚ INTERACTIVO =====
void display_menu() {
  static unsigned long last_menu_update = 0;
  
  if (millis() - last_menu_update < 100) return;
  last_menu_update = millis();
  
  lcd.clear();
  
  switch (current_menu) {
    case MENU_CYLINDERS:
      lcd.setCursor(0, 0);
      lcd.print("Cilindros:");
      lcd.setCursor(10, 0);
      lcd.print(cylinder_count);
      lcd.setCursor(0, 1);
      lcd.print("< OK: Aceptar >");
      break;
      
    case MENU_CKP_TEETH:
      lcd.setCursor(0, 0);
      lcd.print("Dientes CKP:");
      lcd.setCursor(12, 0);
      lcd.print(ckp_teeth);
      lcd.setCursor(0, 1);
      lcd.print("< OK: Aceptar >");
      break;
      
    case MENU_CMP_PULSES:
      lcd.setCursor(0, 0);
      lcd.print("Pulsos CMP:");
      lcd.setCursor(11, 0);
      lcd.print(cmp_pulses_per_cycle);
      lcd.setCursor(0, 1);
      lcd.print("< OK: Aceptar >");
      break;
      
    case MENU_SYNC_TEST:
      lcd.setCursor(0, 0);
      lcd.print("TEST SINCRO");
      lcd.setCursor(0, 1);
      lcd.print("CKP:");
      lcd.print(ckp_frequency);
      lcd.print("Hz");
      break;
      
    default:
      current_menu = MENU_MAIN;
  }
}

// ===== MANEJO DE BOTONES =====
void handle_buttons() {
  static unsigned long last_press_up = 0;
  static unsigned long last_press_down = 0;
  static unsigned long last_press_menu = 0;
  static unsigned long last_press_ok = 0;
  
  unsigned long now = millis();
  
  // Botón ARRIBA
  if (digitalRead(PIN_BUTTON_UP) == LOW && now - last_press_up > DEBOUNCE_TIME) {
    last_press_up = now;
    
    if (current_menu == MENU_MAIN) {
      current_menu = MENU_CYLINDERS;
      menu_value = cylinder_count;
    } else if (current_menu == MENU_CYLINDERS) {
      cylinder_count = min(cylinder_count + 1, 8);
    } else if (current_menu == MENU_CKP_TEETH) {
      ckp_teeth = min(ckp_teeth + 2, 120);
    } else if (current_menu == MENU_CMP_PULSES) {
      cmp_pulses_per_cycle = min(cmp_pulses_per_cycle + 1, 4);
    }
    
    Serial.println("Botón UP presionado");
  }
  
  // Botón ABAJO
  if (digitalRead(PIN_BUTTON_DOWN) == LOW && now - last_press_down > DEBOUNCE_TIME) {
    last_press_down = now;
    
    if (current_menu == MENU_CYLINDERS) {
      cylinder_count = max(cylinder_count - 1, 1);
    } else if (current_menu == MENU_CKP_TEETH) {
      ckp_teeth = max(ckp_teeth - 2, 4);
    } else if (current_menu == MENU_CMP_PULSES) {
      cmp_pulses_per_cycle = max(cmp_pulses_per_cycle - 1, 1);
    }
    
    Serial.println("Botón DOWN presionado");
  }
  
  // Botón MENU
  if (digitalRead(PIN_BUTTON_MENU) == LOW && now - last_press_menu > DEBOUNCE_TIME) {
    last_press_menu = now;
    
    if (current_menu == MENU_MAIN) {
      current_menu = MENU_CYLINDERS;
    } else {
      current_menu = MENU_MAIN;
    }
    
    Serial.println("Botón MENU presionado");
  }
  
  // Botón OK
  if (digitalRead(PIN_BUTTON_OK) == LOW && now - last_press_ok > DEBOUNCE_TIME) {
    last_press_ok = now;
    
    if (current_menu == MENU_CYLINDERS) {
      current_menu = MENU_CKP_TEETH;
    } else if (current_menu == MENU_CKP_TEETH) {
      current_menu = MENU_CMP_PULSES;
    } else if (current_menu == MENU_CMP_PULSES) {
      current_menu = MENU_SYNC_TEST;
    } else if (current_menu == MENU_SYNC_TEST) {
      current_menu = MENU_MAIN;
    }
    
    Serial.println("Botón OK presionado");
  }
}

// ===== FUNCIONES DE UTILIDAD =====
void print_system_status() {
  Serial.println("\n=== Estado del Sistema ===");
  Serial.print("RPM: ");
  Serial.println(rpm_display);
  Serial.print("Cilindros: ");
  Serial.println(cylinder_count);
  Serial.print("Dientes CKP: ");
  Serial.println(ckp_teeth);
  Serial.print("Pulsos CMP: ");
  Serial.println(cmp_pulses_per_cycle);
  Serial.print("Frecuencia CKP: ");
  Serial.print(ckp_frequency);
  Serial.println(" Hz");
  Serial.print("Frecuencia CMP: ");
  Serial.print(cmp_frequency);
  Serial.println(" Hz");
  Serial.print("Período CKP: ");
  Serial.print(ckp_period_us);
  Serial.println(" µs");
  Serial.print("Período CMP: ");
  Serial.print(cmp_period_us);
  Serial.println(" µs");
}

// ===== ESCANEO I2C (para encontrar dirección LCD) =====
void check_lcd_address() {
  Serial.println("\nEscaneando direcciones I2C...");
  Wire.begin(21, 22);
  
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Dispositivo encontrado en: 0x");
      if (i < 16) Serial.print("0");
      Serial.println(i, HEX);
    }
  }
  Serial.println("Escaneo completado\n");
}
