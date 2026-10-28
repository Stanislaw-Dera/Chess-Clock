#include <TM1637TinyDisplay6.h>
#include <ResponsiveAnalogRead.h> // used for detecting analog input changes

const int8_t CLK1 = 2;
const int8_t DIO1 = 3;

const int8_t CLK2 = 4;
const int8_t DIO2 = 5;

const int8_t INCREMENTPIN = A0; 
const int8_t HOURSPIN = A1;
const int8_t MINUTESPIN = A2;
const int8_t SECONDSPIN = A3;

const int8_t PLAYER1BUTTONPIN = 6;
const int8_t PLAYER2BUTTONPIN = 8;
const int8_t STARTSTOPBUTTONPIN = 10;

const int8_t PLAYER1LED = 7;
const int8_t PLAYER2LED = 9;

const int8_t MODESWITCHPIN = 12;

enum State {
  SETUP,
  WAITING,
  PLAYER1TURN,
  PLAYER2TURN,
};

enum Mode {
  UPWARDS,
  DOWNWARDS
};

enum InputNames {
  HOURS, MINUTES, SECONDS, INCREMENT
};

class Player {
  public:
    //setup variables
    uint8_t hours = 0;
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t increment = 0;

    //functional variables
    uint32_t timeMillis = 0;

    TM1637TinyDisplay6 tmPlayer;

    Player(uint8_t CLK, uint8_t DIO) : tmPlayer(CLK, DIO) {
      tmPlayer.begin();
      tmPlayer.setBrightness(1);
    }

    void display() {
      uint32_t currentHours = timeMillis / 3600000;
      uint32_t currentMinutes = (timeMillis / 60000UL) % 60;
      uint32_t currentSeconds = (timeMillis / 1000UL) % 60;

      int32_t timeDisplayed = (currentHours*10000) + (currentMinutes * 100) + currentSeconds;

      //Serial.print("time displayed: "); Serial.println(timeDisplayed);

      tmPlayer.showNumberDec(timeDisplayed, 0b01010100 , true);
    }

    void set(InputNames inputChosen, uint8_t value) {
      switch (inputChosen) {
        case HOURS:
          timeMillis -= this->hours * 3600000UL;
          timeMillis += value * 3600000UL;
          this->hours = value;
          display();
          break;
        case MINUTES:
          timeMillis -= this->minutes * 60000UL;
          timeMillis += value * 60000UL;
          this->minutes = value;
          display();
          break;
        case SECONDS:
          timeMillis -= this->seconds * 1000UL;
          timeMillis += value * 1000UL;
          this->seconds = value;
          display();
          break;
        case INCREMENT:
          this->increment = value;
          break;
      }
    }

    void resetTime() {
      timeMillis = (hours * 3600000UL) + (minutes * 60000UL) + (seconds * 1000UL);
      display();
    }

    void selfPrint() {
      Serial.print("hours: ");
      Serial.println(hours);
      Serial.print("minutes: ");
      Serial.println(minutes);
      Serial.print("seconds: ");
      Serial.println(seconds);
      Serial.print("increment: ");
      Serial.println(increment);
    }

};


Player player1(CLK1, DIO1);
Player player2(CLK2, DIO2);


ResponsiveAnalogRead incrementAnalog(A0, true);
ResponsiveAnalogRead hoursAnalog(A1, true);
ResponsiveAnalogRead minutesAnalog(A2, true);
ResponsiveAnalogRead secondsAnalog(A3, true);

enum State state = SETUP;
enum Mode mode;

uint32_t playerChangeMillis = 0;

void setup() {
  Serial.begin(9600);

  Serial.print("Millis (beginning): "); Serial.println(millis());

  // set up reading

  pinMode(A0, INPUT); pinMode(A1, INPUT); pinMode(A2, INPUT); pinMode(A3, INPUT);
  pinMode(PLAYER1BUTTONPIN, INPUT_PULLUP); pinMode(PLAYER2BUTTONPIN, INPUT_PULLUP);
  pinMode(STARTSTOPBUTTONPIN, INPUT_PULLUP); pinMode(MODESWITCHPIN, INPUT_PULLUP);
  pinMode(PLAYER1LED, OUTPUT); pinMode(PLAYER2LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // find the initial players time and increment

  uint8_t hoursValue = map(analogRead(A1), 0, 1024, 0, 8); // max 8hours
  uint8_t minutesValue = map(analogRead(A2), 0, 1023, 0, 59);
  uint8_t secondsValue = map(analogRead(A3), 0, 1023, 0, 59);

  hoursAnalog.setActivityThreshold(30);     // ignore the noise 
  minutesAnalog.setActivityThreshold(30);
  secondsAnalog.setActivityThreshold(30);
  incrementAnalog.setActivityThreshold(30);

  Serial.println("set test");
  Serial.println(hoursValue); Serial.println(minutesValue); Serial.println(secondsValue);

  uint8_t incrementValue = map(analogRead(A0), 0, 1023, 0, 60);

  player1.set(HOURS, hoursValue); player1.set(MINUTES, minutesValue); player1.set(SECONDS, secondsValue); player1.set(INCREMENT, incrementValue);
  player2.set(HOURS, hoursValue); player2.set(MINUTES, minutesValue); player2.set(SECONDS, secondsValue); player2.set(INCREMENT, incrementValue);

  player1.selfPrint();
  digitalWrite(BUZZER, LOW);
  Serial.println("------END OF THE SETUP FUNCTION-----------");
}

void displayIncrement(uint8_t value) {
  player1.tmPlayer.showString("Incr");
  player2.tmPlayer.showNumber(value, true);
}

void displayLoseMessage(TM1637TinyDisplay6 tmPlayer) {
  for (uint8_t i = 0; i < 5; i++) {
    Serial.println("INSIDE!");
    tmPlayer.setBrightness(1, false);
    
    digitalWrite(BUZZER, HIGH);
    delay(400);

    tmPlayer.setBrightness(1, true);
    digitalWrite(BUZZER, LOW);
    delay(400);
  }
  tmPlayer.showString("LOSER");
  delay(1000);
}

void handlePlayerLogic(Player currentPlayer, Player otherPlayer, bool isDownwards){} // to be improved one day...

void loop() {

  uint32_t currentMillis = millis();
  static unsigned long lastDebugPrint = 0;
  static uint32_t lastPressedMillis = 0;
  bool allowPressesMode = (currentMillis - lastPressedMillis > 1000); // variable responsible for debounce on the mode button
  bool allowPressesPlayer = (currentMillis - lastPressedMillis > 100); // variable responsible for debounce on the player buttons


  // handle the buttons logic

  if (digitalRead(MODESWITCHPIN) == LOW) {
    //Serial.println("UPWARDS");
    mode = UPWARDS;
  } else {
    //Serial.println("DOWNWARDS");
    mode = DOWNWARDS;
  }

  if (digitalRead(STARTSTOPBUTTONPIN) == LOW && allowPressesMode) {

    if (state == SETUP) {   // waiting for the first move
      state = WAITING;
    }
    else if (state == PLAYER1TURN || state == PLAYER2TURN) {    // pause
      state = WAITING;
    }
    else if (state == WAITING) {    // reset
      state = SETUP;
      player1.resetTime(); player2.resetTime();
    }
    lastPressedMillis = currentMillis; //debounce
  }

  if (digitalRead(PLAYER1BUTTONPIN) == LOW && allowPressesPlayer) {


    if (state == PLAYER1TURN) {
      player1.timeMillis += player1.increment * 1000UL; // change the player mid-game
      player1.display();

      state = PLAYER2TURN;
      playerChangeMillis = currentMillis;
    }
    else if (state == WAITING) {
      state = PLAYER2TURN;  // start the game
      playerChangeMillis = currentMillis;
    }

    lastPressedMillis = currentMillis; // debounce
  }

  if (digitalRead(PLAYER2BUTTONPIN) == LOW && allowPressesPlayer) {


    if (state == PLAYER2TURN) {
      player2.timeMillis += player2.increment * 1000UL;
      player2.display();

      state = PLAYER1TURN;
      playerChangeMillis = currentMillis;
    }
    else if (state == WAITING) {
      state = PLAYER1TURN;
      playerChangeMillis = currentMillis;
    }

    lastPressedMillis = currentMillis; // debounce
  }

  // handle the clock logic

  if (mode == DOWNWARDS) {
    if (currentMillis - lastDebugPrint >= 2000) {
      Serial.println("downwards!");
    }
    
    if (state == SETUP) {

      digitalWrite(PLAYER1LED, LOW); digitalWrite(PLAYER2LED, LOW);

      static bool showingIncrement = false;
      static uint32_t showingIncrementMillis = 0;

      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("Setup");
      }

      // processing the inputs
      hoursAnalog.update();
      minutesAnalog.update();
      secondsAnalog.update();
      incrementAnalog.update();

      if (hoursAnalog.hasChanged()) {
        uint8_t val = map(hoursAnalog.getValue(), 0, 1023, 0, 7);
        if (val != player1.hours) { // refresh only when value changes
          player1.set(HOURS, val); 
          player2.set(HOURS, val);
        }
      }

      if (minutesAnalog.hasChanged()) {
        uint8_t val = map(minutesAnalog.getValue(), 0, 1023, 0, 59);
        if (val != player1.minutes) { // refresh only when value changes
          player1.set(MINUTES, val); 
          player2.set(MINUTES, val);
        }
      }

      if (secondsAnalog.hasChanged()) {
        uint8_t val = map(secondsAnalog.getValue(), 0, 1023, 0, 59);
        if (val != player1.seconds) { // refresh only when value changes
          player1.set(SECONDS, val); 
          player2.set(SECONDS, val);
        }
      }

      if (incrementAnalog.hasChanged()) {
        uint8_t val = map(incrementAnalog.getValue(), 0, 1023, 0, 60);
        if (val != player1.increment) { // refresh only when value changes
          player1.set(INCREMENT, val); 
          player2.set(INCREMENT, val);
          displayIncrement(val);
          showingIncrement = true;
          showingIncrementMillis = millis();
        }
      }

      if (showingIncrement && (millis() - showingIncrementMillis >= 2000)) { // display increment for 2 seconds
        showingIncrement = false;
        player1.display(); 
        player2.display();
      }
    }

    else if (state == PLAYER1TURN) {
      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("PLAYER1TURN");
      }
      uint32_t elapsed = currentMillis - playerChangeMillis;
      if (elapsed > 0) {
        //if (currentMillis - lastDebugPrint >= 2000) {Serial.println(player1.timeMillis);Serial.print("|");Serial.println(elapsed);}
        if (player1.timeMillis >= elapsed) {
          player1.timeMillis -= elapsed;
        } else {
          player1.timeMillis = 0;
          displayLoseMessage(player1.tmPlayer);
          player1.resetTime();
          player2.resetTime();
          state = SETUP; // loss
        }
        playerChangeMillis = currentMillis;
      }

      digitalWrite(PLAYER1LED, HIGH); digitalWrite(PLAYER2LED, LOW);

      // screen refresh every 200 ms
      static uint32_t lastDisplayP1 = 0;
      if (currentMillis - lastDisplayP1 >= 200) {
        player1.display();
        lastDisplayP1 = currentMillis;
      }
    }

    else if (state == PLAYER2TURN) {

      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("PLAYER1TURN");
      }
      uint32_t elapsed = currentMillis - playerChangeMillis;
      if (elapsed > 0) {

        if (player2.timeMillis >= elapsed) {
          player2.timeMillis -= elapsed;
        } else {
          player2.timeMillis = 0;
          displayLoseMessage(player2.tmPlayer);
          player1.resetTime();
          player2.resetTime();
          state = SETUP; // loss
        }
        playerChangeMillis = currentMillis;
      }

      digitalWrite(PLAYER1LED, LOW); digitalWrite(PLAYER2LED, HIGH);

      // screen refresh every 200 ms
      static uint32_t lastDisplayP2 = 0;
      if (currentMillis - lastDisplayP2 >= 200) {
        player2.display();
        lastDisplayP2 = currentMillis;
      }

    }

    else if (state == WAITING) {
      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("Waiting for players to click the buttons...");
      }
      player1.display(); player2.display();
      digitalWrite(PLAYER1LED, HIGH); digitalWrite(PLAYER2LED, HIGH);
    }
  }
  else if (mode == UPWARDS) {
    if (currentMillis - lastDebugPrint >= 2000) {
      Serial.println("upwards!");
    }

    if (state == SETUP) {
      digitalWrite(PLAYER1LED, LOW); digitalWrite(PLAYER2LED, LOW);
      player1.set(HOURS, 0); player1.set(MINUTES, 0); player1.set(SECONDS, 0); player1.set(INCREMENT, 0);
      player2.set(HOURS, 0); player2.set(MINUTES, 0); player2.set(SECONDS, 0); player2.set(INCREMENT, 0);

    }

    else if (state == PLAYER1TURN) {
      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("PLAYER1TURN");
      }
      uint32_t elapsed = currentMillis - playerChangeMillis;
      if (elapsed > 0) {
        //if (currentMillis - lastDebugPrint >= 2000) {Serial.println(player1.timeMillis);Serial.print("|");Serial.println(elapsed);}

        player1.timeMillis += elapsed; // perhaps I should limit it to something like 8 hours

        playerChangeMillis = currentMillis;
      }

      digitalWrite(PLAYER1LED, HIGH); digitalWrite(PLAYER2LED, LOW);

      // screen refresh every 200 ms
      static uint32_t lastDisplayP1 = 0;
      if (currentMillis - lastDisplayP1 >= 200) {
        player1.display();
        lastDisplayP1 = currentMillis;
      }
    }

    else if (state == PLAYER2TURN) {

      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("PLAYER2TURN");
      }
      uint32_t elapsed = currentMillis - playerChangeMillis;
      if (elapsed > 0) {
        player2.timeMillis += elapsed; //
        playerChangeMillis = currentMillis;
      }

      digitalWrite(PLAYER1LED, LOW); digitalWrite(PLAYER2LED, HIGH);

      // screen refresh every 200 ms
      static uint32_t lastDisplayP2 = 0;
      if (currentMillis - lastDisplayP2 >= 200) {
        player2.display();
        lastDisplayP2 = currentMillis;
      }
    }
    else if (state == WAITING) {
      if (currentMillis - lastDebugPrint >= 2000) {
        Serial.println("Waiting for players to click the buttons...");
      }
      player1.display(); player2.display();
      digitalWrite(PLAYER1LED, HIGH); digitalWrite(PLAYER2LED, HIGH);
    }
  }

  if (currentMillis - lastDebugPrint >= 2000) {
    Serial.print("STATE: "); Serial.println(state);
    Serial.print("P1 ms: "); Serial.println(player1.timeMillis);
    Serial.print("P2 ms: "); Serial.println(player2.timeMillis);
    Serial.println("--------------------");
    lastDebugPrint = currentMillis;
  }
}
