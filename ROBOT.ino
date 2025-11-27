#include <Servo.h>

// --- 1. DEFINICIÓN DE PINES ---

// Servomotor MG996R (Dirección)
Servo direccionServo;
const int SERVO_PIN = 3;

// Driver BTS7960 (Propulsión)
// Ambos pines se usarán como PWM para controlar la velocidad (RPM)
// El L_EN y R_EN están cableados a VCC (HIGH) por el hardware.
const int MOTOR_PWM_L = 6; // Pin RPM 
const int MOTOR_PWM_R = 5; // Pin RPM 

// Sensores Ultrasónicos HC-SR04
// Izquierda
const int TRIG_L = 9;
const int ECHO_L = 10;
// Derecha
const int TRIG_R = 11;
const int ECHO_R = 12;
// Delantero
const int TRIG_D = 7;
const int ECHO_D = 8;

// --- 2. CONSTANTES DE CALIBRACIÓN (AJUSTAR EN PRUEBAS) ---

// Control Proporcional
const float Kp = 1.2;         // Ganancia Proporcional (Ajustar: 1.0 a 2.0)

// Servo
const int SERVO_CENTRO = 85;  // Nuevo centro ligeramente ajustado de 90 a 85 grados
const int SERVO_LIMITE_MAX = 130; // Ángulo máximo de giro (Izquierda)
const int SERVO_LIMITE_MIN = 40;  // Ángulo mínimo de giro (Derecha)

// Motor y Velocidad
const int velocidad = 110; // Velocidad de avance (0-255)

// Evitación de Obstáculos
const float DISTANCIA_CHOQUE = 15.0; // Distancia límite en cm para detener el robot
const int TIEMPO_REVERSA = 250;      // Tiempo en ms que retrocede el robot

// --- 3. FUNCIONES DE UTILIDAD ---

// Función para leer la distancia de un sensor HC-SR04
float leerDistancia(int trigPin, int echoPin) {
    // Limpiar el pin TRIG
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Enviar pulso de 10µs
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Leer el pulso de retorno
    long duracion = pulseIn(echoPin, HIGH);
    // Convertir a distancia (cm) 
    // Velocidad del sonido (340 m/s o 0.034 cm/µs). Distancia = (Duración * 0.034) / 2
    float distancia = duracion * 0.034 / 2;
    
    // Opcional: limitar para ignorar lecturas demasiado lejanas
    if (distancia > 200 || distancia < 0) {
        return 200; 
    }
    return distancia;
}

// Función para mover el motor hacia adelante
void avanzar(int velocidad) {
    // En el BTS7960, para avanzar, enviamos la misma señal PWM a ambos RPM
    // ya que el L_EN/R_EN están en VCC y la lógica interna ya está manejada.
    analogWrite(MOTOR_PWM_L, velocidad); 
    analogWrite(MOTOR_PWM_R, 0);
}

// Función para detener el motor (frenado)
void detener() {
    analogWrite(MOTOR_PWM_L, 0); 
    analogWrite(MOTOR_PWM_R, 0);
}

// Función para mover el motor hacia atrás (usando la lógica de H-bridge del BTS7960)
// Esto dependerá de cómo conectaste los pines, ajusta si es necesario.
// Si ambos PWM giran al mismo tiempo, el motor se detiene (como en 'detener')
// Para el 540 necesitas invertir la polaridad, lo cual se hace con los pines de control que 
// tu no estás usando (L_EN/R_EN cableados a VCC).
// Por simplicidad, y dado tu cableado, usaremos solo 'detener' y dependeremos de la lógica de giro.

// --- 4. CONFIGURACIÓN INICIAL ---

void setup() {
    // Configuración de pines de sensores
    delay(2000);
    pinMode(TRIG_L, OUTPUT); pinMode(ECHO_L, INPUT);
    pinMode(TRIG_R, OUTPUT); pinMode(ECHO_R, INPUT);
    pinMode(TRIG_D, OUTPUT); pinMode(ECHO_D, INPUT);
    
    // Configuración de pines de motor
    pinMode(MOTOR_PWM_L, OUTPUT);
    pinMode(MOTOR_PWM_R, OUTPUT);
    
    // Inicialización del servomotor
    direccionServo.attach(SERVO_PIN);
    direccionServo.write(SERVO_CENTRO); // Centrar el servo al inicio
    detener(); // Detener el motor
    
    // Serial para debug (Opcional)
    Serial.begin(9600);
}

// --- 5. BUCLE PRINCIPAL DE NAVEGACIÓN ---

void loop() {
    // a. LECTURA DE SENSORES
    float dist_delantera = leerDistancia(TRIG_D, ECHO_D);
    float dist_izquierda = leerDistancia(TRIG_L, ECHO_L);
    float dist_derecha = leerDistancia(TRIG_R, ECHO_R);

    // --- LÓGICA DE DETECCIÓN FRONTAL (Prioridad Máxima) ---
    if (dist_delantera < DISTANCIA_CHOQUE) {
        // 1. DETENER
        detener();
        // 2. RETROCEDER (Esta función asume que tu cableado permite reversa, si no, solo DETENER)
        // Para simplificar, si no estás usando los pines de dirección, solo frenamos y esperamos.
        // Si no puedes retroceder, el robot quedará atascado. 
        // Si tu driver tiene pines de dirección (ej. DIR_A y DIR_B), DEBERÍAS USARLOS para el 540.
        delay(TIEMPO_REVERSA); 
        
        // 3. GIRAR LA DIRECCIÓN al máximo opuesto para reorientarse (ej. 45 grados a la izquierda si el último giro fue a la derecha)
        // Por defecto, giramos ligeramente a la derecha para intentar salir.
        direccionServo.write(SERVO_LIMITE_MAX); // Intenta girar bruscamente a la izquierda
        avanzar(velocidad/2); 
        delay(500); // Espera medio segundo para que el robot se mueva un poco
        detener();
    }
    
    // --- LÓGICA DE CONTROL LATERAL PROPORCIONAL ---
    else {
        // Error: Si Error > 0, el robot está cerca del muro derecho (DD es menor que DI). Necesita girar IZQUIERDA (ángulo > SERVO_CENTRO)
        // Si Error < 0, el robot está cerca del muro izquierdo (DI es menor que DD). Necesita girar DERECHA (ángulo < SERVO_CENTRO)
        float error = dist_derecha - dist_izquierda; 
        
        // Cálculo de la corrección
        int correccion = (int)(error * Kp);
        
        // Nuevo ángulo de dirección
        int nuevo_angulo = SERVO_CENTRO + correccion;
        
        // Aplicar límites
        if (nuevo_angulo > SERVO_LIMITE_MAX) nuevo_angulo = SERVO_LIMITE_MAX;
        if (nuevo_angulo < SERVO_LIMITE_MIN) nuevo_angulo = SERVO_LIMITE_MIN;
        
        // Ejecutar el movimiento
        direccionServo.write(nuevo_angulo);
        avanzar(velocidad);
    }

    // --- LÓGICA DE DEBUG (Opcional, activa el Serial Monitor) ---
    Serial.print("DI: "); Serial.print(dist_izquierda);
    Serial.print(" | DD: "); Serial.print(dist_derecha);
    Serial.print(" | DF: "); Serial.print(dist_delantera);
    Serial.print(" | Angulo: "); Serial.println(direccionServo.read());
    
    delay(50); // Pequeño retardo para estabilidad de lectura/escritura (ajustar)
}
