// CREDENCIAIS BLYNK
#define BLYNK_TEMPLATE_ID "TMPL2pKhns7Ky"
#define BLYNK_TEMPLATE_NAME "Notificar luz"

// Carrega secrets definidos no arquivo .env (mantido fora do controle de versão).
#include ".env"

#ifndef TOKEN_BLYNK
#error "TOKEN_BLYNK não definido no arquivo .env"
#endif
#define BLYNK_AUTH_TOKEN TOKEN_BLYNK

// CREDENCIAIS WIFI
#ifndef NOME_DA_REDE
#error "NOME_DA_REDE não definido no arquivo .env"
#endif

#ifndef SENHA_DA_REDE
#error "SENHA_DA_REDE não definido no arquivo .env"
#endif

char ssid[] = NOME_DA_REDE;
char pass[] = SENHA_DA_REDE;

// BIBLIOTECAS
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// PINOS DO HARDWARE
#define PINO_SENSOR 21
#define PINO_RELE 22

#define VIRTUAL_PIN_STATUS V1
BlynkTimer timer;
bool releEstaLigado = false; // Variável global para saber o estado

// CONTROLAR O RELÉ
void checarSensorERele() {
  int estadoSensor = digitalRead(PINO_SENSOR);

  if (estadoSensor == HIGH) {
    if (!releEstaLigado) {
      Serial.println("Movimento detectado! LIGANDO relé e enviando notificação.");
      digitalWrite(PINO_RELE, LOW);
      Blynk.logEvent("movimento_detectado", "Alerta: Movimento detectado!");
      releEstaLigado = true;
    }
  } else {
    if (releEstaLigado) {
      Serial.println("Sem movimento. Desligando relé.");
      digitalWrite(PINO_RELE, HIGH);
      releEstaLigado = false;
    }
  }
}

// GARANTIR QUE O APP ESTÁ SINCRONIZADO
void sincronizarApp() {
  if (releEstaLigado) {
    // Se o relé está ligado, manda o comando LIGADO para o V1
    Blynk.virtualWrite(VIRTUAL_PIN_STATUS, 255);
  } else {
    // Se o relé está desligado, manda o comando DESLIGADO para o V1
    Blynk.virtualWrite(VIRTUAL_PIN_STATUS, 0);
  }
}

// CONFIGURAÇÃO INICIAL
void setup() {
  Serial.begin(9600);
  Serial.println("\n\nIniciando sistema (Versao Sincronizada)...");

  pinMode(PINO_SENSOR, INPUT_PULLDOWN);
  pinMode(PINO_RELE, OUTPUT);
  digitalWrite(PINO_RELE, HIGH);

  // Conecta ao Wi-Fi e ao Blynk
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");

  Serial.println("Conectando ao Blynk...");
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(10000);
  
  if (Blynk.connected()) {
    Serial.println("Blynk Conectado!");
  } else {
    Serial.println("Falha ao conectar ao Blynk.");
  }

  // 1. Checa o sensor 2 vezes por segundo
  timer.setInterval(500L, checarSensorERele);
  // 2. Sincroniza o app 1 vez por segundo
  timer.setInterval(1000L, sincronizarApp);
}

// LOOP PRINCIPAL
void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
}