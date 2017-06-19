#include <TimerOne.h> // Biblioteca de interrupção de tempo

/* Associação dos pinos do arduíno */
const byte buttons_interrupt = 2; // Saída AND entre todos os botões: responsável pela interrupção

const byte button_yellow = 3;
const byte button_white = 4;
const byte button_green = 5;
const byte button_blue = 6;
const byte button_red = 7;

const byte led_yellow = 9;
const byte led_white = 10;
const byte led_green = 11;
const byte led_blue = 12;
const byte led_red = 13;

/* Flags e variáveis de controle da pressão dos botões */
bool disable_button = false;
bool button_pressed = false; // Indica para o programa principal se algum botão foi pressionado
bool debounce = false; // Indica se está em período de 'debounce'
int debounce_counter = 0;
byte pressed_button; // Armazena o botão pressionado

int feedback_time = 200;

/* Dados da sequência e controle de cada fase - Fases jogáveis: level 1 a level 99 */
const int max_level = 100;
byte sequence[max_level];
byte level = 0; // Estado padrão do programa (antes do início do jogo)
int index = 0; // Índice da sequência para grantir a ordem de comparação com o botão pressionado

/* Sinais luminosos de comunicação com o usuário - estado inicial, acerto, erro */

// Estado inicial: acende e apaga em sequência
void beginning_sign() {
  digitalWrite(led_yellow, HIGH);
  delay(100);
  digitalWrite(led_white, HIGH);
  delay(100);
  digitalWrite(led_green, HIGH);
  delay(100);
  digitalWrite(led_blue, HIGH);
  delay(100);
  digitalWrite(led_red, HIGH);
  delay(100);
  digitalWrite(led_yellow, LOW);
  delay(100);
  digitalWrite(led_white, LOW);
  delay(100);
  digitalWrite(led_green, LOW);
  delay(100);
  digitalWrite(led_blue, LOW);
  delay(100);
  digitalWrite(led_red, LOW);
  delay(100);
}

// Acerto: acende e apaga todos três vezes
void correct_sign() {
  for (int i=0; i<3; i++) {
    digitalWrite(led_yellow, HIGH);
    digitalWrite(led_white, HIGH);
    digitalWrite(led_green, HIGH);
    digitalWrite(led_blue, HIGH);
    digitalWrite(led_red, HIGH);
    delay(100);
    digitalWrite(led_yellow, LOW);
    digitalWrite(led_white, LOW);
    digitalWrite(led_green, LOW);
    digitalWrite(led_blue, LOW);
    digitalWrite(led_red, LOW);
    delay(100);
  }
}

// Erro: acende e apaga o led vermelho 5 vezes
void wrong_sign() {
  for (int i=0; i<5; i++) {
    digitalWrite(led_red, HIGH);
    delay(400);
    digitalWrite(led_red, LOW);
    delay(100);
  }
}

/* Rotina da interrupção de pressão dos botões */
void buttonInterrupt_routine() {
  if(disable_button == false && debounce == false){
    disable_button = true; // Botões reativados na função principal
    debounce = true; // Consumida na interrupção de tempo
    button_pressed = true; // Consumida na função principal
   }
}

/* Rotina da interrupção de tempo - controle do tempo de debounce */
void timeInterrupt_routine() {
  if (debounce){
    debounce_counter++;
  }
  // Período de 'debounce': 100ms
  if (debounce_counter == 100) {
    // pressed_button - 6 retorna o número do botão pressionado
    // o debounce só termina se o botão não estiver mais pressionado
    if(digitalRead(pressed_button-6)==1){
      debounce = false;
      debounce_counter = 0;
    }
    // Se o usuário manteve o botão pressionado, dobra o período de 'debounce'
    else debounce_counter = 0;
  }
}

/* Período aceso do led na apresentação da sequência */
int high_time(){
  // Inversamente proporcional a fase, com início em 1s e limite em 200ms
  return max(1000/level, 200);
}

/* Gera a sequência do jogo */
void create_sequence(){
    randomSeed(analogRead(0)); // Usa o valor de uma entrada analógica desconectada para alimentar a função que gera um número pseudo aleatório
    sequence[level-1] = random(10)%5 + 9; // Armazena um número pseudo aleatório inteiro de 9 a 13, associados às portas dos leds

    // Apresentação da sequência ao usuário
    for (int i=0; i<level; i++){
        digitalWrite(sequence[i], HIGH);
        delay(high_time());
        digitalWrite(sequence[i], LOW);
        if(i<level-1) delay(500); // Não dá o tempo de de espera após o ultimo led da sequência
    }
}


/* Varredura dos botões - retorna qual botão foi pressionado (ativos em baixo)*/
byte sweep() {
  if(digitalRead(button_yellow)==0) return 9;
  if(digitalRead(button_white)==0) return 10;
  if(digitalRead(button_green)==0) return 11;
  if(digitalRead(button_blue)==0) return 12;
  if(digitalRead(button_red)==0) return 13;
}

void setup() {
  pinMode(led_yellow, OUTPUT);
  pinMode(led_white, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_red, OUTPUT);

  pinMode(button_yellow, INPUT);
  pinMode(button_white, INPUT);
  pinMode(button_green, INPUT);
  pinMode(button_blue, INPUT);
  pinMode(button_red, INPUT);

  // Configuração da interrupção da pressão dos botões
  pinMode(buttons_interrupt, INPUT);
  attachInterrupt( digitalPinToInterrupt(buttons_interrupt), buttonInterrupt_routine, FALLING );

  // Configuração da interrupção de tempo
  Timer1.initialize(1000); // 1 ms
  Timer1.attachInterrupt( timeInterrupt_routine );

}

void loop() {
  // Estado padrão do programa (antes do início do jogo)
  if(level==0){
    beginning_sign();
    // Aguarda qualquer botão ser pressionado e inicia o jogo na fase 1
    if (button_pressed){
      button_pressed = false;
      level++;
      delay(200);
      create_sequence();
      disable_button = false; // Aguarda primeira resposta do usuário
    }
  }

  // Programa controlado pela pressão dos botões
  if (button_pressed){
     button_pressed = false;
     pressed_button = sweep();
     digitalWrite(pressed_button, HIGH); // Feedback do botão pressionado
     delay(300); // Período de feedback: 300ms
     digitalWrite(pressed_button, LOW);
     if (pressed_button == sequence[index]){
      index++; // Verifica se os botões foram apertados na ordem correta
      disable_button = false; // Aguarda próxima pressão de botão
      if (level == 99){
        // Final level: usuário zerou o jogo, retorno ao estado padrão inicial
        level = 0;
        index = 0;
      }
      else if (index == level){ // Next Level: usuário acertou toda a sequência e passou de fase
        disable_button = true;
        delay(100); // Período entre o término da sequência e a indicação de acerto
        correct_sign();
        level++;
        index = 0;
        create_sequence(); // Nova sequência da fase
        disable_button = false; // Aguarda resposta do usuário
      }
     }
     else {
      // Game over: usuário perdeu o jogo, retorno ao estado padrão inicial
      delay(100); // Período entre o término da sequência e a indicação de erro
      wrong_sign();
      level = 0;
      index = 0;
      delay(1000);
      disable_button = false;
     }
  }
}
