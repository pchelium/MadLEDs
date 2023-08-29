#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SFE_MAX1704X lipo(MAX1704X_MAX17048);

TaskHandle_t checkButtonTaskHandle;
TaskHandle_t ledDiodeTaskHandle;

// some comments in all code is for debugging!(Like: Serial.print(...))

// define LEDs and buttons pins
const int led1Pin = 25;
const int led2Pin = 26;
const int led3Pin = 27;

const int button1 = 34;
const int button2 = 33;
const int button3 = 35;

const int acceptButton = 32;

const int scrollButton = 15;
const int sleepButton = 4; // definition of the button that triggers deepSleep mode and also wakes up the board from this mode

// define and initialize variables
int acceptButtonValue = 0;

int ledDelay = 0;
int randomNumber = 0;
int maxSpeed = 9;
int minSpeed = 5;
int resolution = 100;

int changeButtonValue = 0;

int okPoints = 0;
int noOkPoints = 0;

int buttonTouch = 0;
int randomValue = 0;

int target = 50; // variable to define the maximum number of attempts(target)
int changeDisplay = 0;

String historyList[5];
int historyCount = 0;
int userNumber = 0; // user number, this number is used to identify the user in the history
/**
 * Causes the LEDs to light up at random times and adds points.
 */
void ledDiod(void *parameter)
{
  for (;;)
  {
    randomNumber = random(1, 4);

    buttonTouch = 0;

    if (randomNumber == randomValue)
    {
      buttonTouch = 1;
    }

    ledDelay = random(minSpeed, maxSpeed) * resolution; // define sleep time for LED diode

    //  Serial.print(String(randomNumber)+"--");
    //  Serial.print(String(ledDelay)+"\n");

    switch (randomNumber) // random LED selection
    {
    case 1:
    {
      digitalWrite(led1Pin, HIGH);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, LOW);
      vTaskDelay(ledDelay / portTICK_PERIOD_MS);
      break;
    }
    case 2:
    {
      digitalWrite(led2Pin, HIGH);
      digitalWrite(led1Pin, LOW);
      digitalWrite(led3Pin, LOW);
      vTaskDelay(ledDelay / portTICK_PERIOD_MS);
      break;
    }
    case 3:
    {
      digitalWrite(led3Pin, HIGH);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led1Pin, LOW);
      vTaskDelay(ledDelay / portTICK_PERIOD_MS);
      break;
    }
    }

    if ((buttonTouch == 0 && randomValue != randomNumber) || (buttonTouch == 2 && randomValue != randomNumber)) // adding a point to the variable of badly pressed buttons(noOkPoints)
    {
      noOkPoints++;
    }
    randomValue = randomNumber;
  }
}

/**
 * Function to recognize the pressed button and then evaluate whether the user has hit the LED or not.
 */
void checkButtonTask(void *parameter)
{
  for (;;)
  {
    if (digitalRead(button1) && randomNumber == 1 && buttonTouch == 0 && digitalRead(button2) == LOW && digitalRead(button3) == LOW)
    {
      okPoints++;
      buttonTouch = 1;
    }
    else if (digitalRead(button2) && randomNumber == 2 && buttonTouch == 0 && digitalRead(button1) == LOW && digitalRead(button3) == LOW)
    {
      okPoints++;
      buttonTouch = 1;
    }
    else if (digitalRead(button3) && randomNumber == 3 && buttonTouch == 0 && digitalRead(button1) == LOW && digitalRead(button2) == LOW)
    {
      okPoints++;
      buttonTouch = 1;
    }
    else if (((digitalRead(button1) && randomNumber != 1) || (digitalRead(button2) && randomNumber != 2) || (digitalRead(button3) && randomNumber != 3)) && buttonTouch == 0) // checking whether the user has pressed the wrong button
    {
      buttonTouch = 2;
    }

    // Serial.print("\n"+String(digitalRead(button3))+"/"+String(digitalRead(button2))+"/"+String(digitalRead(button1))+"/"+String(randomNumber));
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/**
 *For turn off all LEDs.
 */
void ledOff()
{
  digitalWrite(led3Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led1Pin, LOW);
}

void setup()
{
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);
  randomSeed(analogRead(0)); // set the unique number for randomSeed function
  Serial.print(analogRead(0));
  Serial.print("\n");
  pinMode(sleepButton, INPUT_PULLDOWN);
  // set the LED diode output pins
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);

  // set the input button with internall pull-down resistore
  pinMode(button1, INPUT_PULLDOWN);
  pinMode(button2, INPUT_PULLDOWN);
  pinMode(button3, INPUT_PULLDOWN);
  pinMode(acceptButton, INPUT_PULLDOWN);
  pinMode(scrollButton, INPUT_PULLDOWN);

  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // check if battery senzor is conected
  if (!lipo.begin())
  {
    Serial.print("\nCould not find a valid battery sensor!");
  }

  lipo.quickStart();

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 26);
  display.print("WELCOME TO");
  display.display();
  delay(1000);

  display.clearDisplay();
  display.setCursor(7, 26);
  display.print(" MadLEDs");
  display.display();
  delay(1500);

  // display.clearDisplay();

  for (int i = 0; i <= 7; i++)
  {
    display.drawLine(random(0, 129), random(0, 65), random(0, 129), random(0, 65), WHITE);
    display.display();
    delay(147 - 10 * i / 3);
  }
}

/**
 * Reset values in some variables for new game
 */
void resetValues()
{
  acceptButtonValue = 0;

  okPoints = 0;
  noOkPoints = 0;
  buttonTouch = 0;
  randomValue = 0;
}

void loop()
{
  // Serial.print("/"+String(minSpeed)+"/"+String(maxSpeed)+"//"+String(digitalRead(button3))+"/"+String(digitalRead(button2))+"/"+String(changeButtonValue)+"\n");

  display.clearDisplay();
  display.setCursor(0, 5);
  display.setTextSize(1);

  if (lipo.begin()) // display battery information if the battery senzor is conected
  {
    display.print(lipo.getVoltage());
    display.print("V ");
    display.print("        ");
    display.print(lipo.getSOC());
    display.print("% ");
  }
  else // display if battery senzor is disconecated
  {
    display.print("!.!!V         !.!!% ");
  }
  display.setCursor(0, 25);
  display.setTextSize(2);

  if (acceptButtonValue == 0) // setting values from the user via buttons
  {
    if (changeDisplay == 0)
    {
      if (digitalRead(button1))
      {
        if (changeButtonValue > 4)
        {
          changeButtonValue = 0;
        }
        else
        {
          changeButtonValue++;
        }
      }

      if (digitalRead(button3))
      {
        if (changeButtonValue == 3)
        {
          if (resolution <= 1000)
          {
            resolution = resolution * 10;
          }
        }
        else if (changeButtonValue == 2)
        {
          maxSpeed++;
        }
        else if (changeButtonValue == 1)
        {
          target++;
        }
        else
        {
          minSpeed++;
        }
      }

      if (digitalRead(button2))
      {
        if (changeButtonValue == 3)
        {
          if (resolution > 1)
          {
            resolution = resolution / 10;
          }
        }
        else if (changeButtonValue == 2 && maxSpeed > 0)
        {
          maxSpeed--;
        }
        else if (changeButtonValue == 1 && target > 1)
        {
          target--;
        }
        else
        {
          minSpeed--;
        }
      }

      display.print(String(minSpeed) + "/" + String(maxSpeed) + "/");
      if (changeButtonValue == 3)
      {
        display.print("RES");
      }
      else if (changeButtonValue == 2)
      {
        display.print("MAX");
      }
      else if (changeButtonValue == 1)
      {
        display.print("TAR");
      }
      else
      {
        display.print("MIN");
      }
      display.setCursor(0, 49);
      display.print(String(resolution) + "/" + String(target));
    }
    else if (changeDisplay == 1)
    {
      display.clearDisplay();
      display.setCursor(0, 5);
      display.setTextSize(1);
      display.print("HISTORY:      ");

      if (lipo.begin()) // display battery information if the battery senzor is conected
      {
        display.print(lipo.getSOC());
        display.print("% ");
      }
      else // display if battery senzor is disconecated
      {
        display.print("!.!!% ");
      }

      display.setCursor(0, 20);

      for (auto &history : historyList) // display history list
      {
        display.print(history);
        display.print("\n");
      }
    }
  }
  else
  {
    display.setCursor(0, 25);
    display.setTextSize(3);

    // display the current game state
    display.print(String(okPoints) + "/" + String(target - okPoints - noOkPoints));
    display.setTextSize(2);
    if (target - okPoints - noOkPoints != 0)
    {
      display.println("/" + String(target));
    }
  }
  display.display();

  if (digitalRead(acceptButton) && acceptButtonValue == 0)
  {
    // countdown to game start
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, HIGH);
    digitalWrite(led3Pin, HIGH);
    delay(1000);
    digitalWrite(led3Pin, LOW);
    delay(1000);
    digitalWrite(led2Pin, LOW);
    delay(1000);
    digitalWrite(led1Pin, LOW);

    xTaskCreatePinnedToCore(ledDiod, "ledDiodeTask", 1000, NULL, 2, &ledDiodeTaskHandle, 1);

    xTaskCreatePinnedToCore(checkButtonTask, "checkButtonTask", 1000, NULL, 3, &checkButtonTaskHandle, 1);
    acceptButtonValue = 1;
  }

  if (digitalRead(acceptButton) && acceptButtonValue == 1)
  {
    vTaskDelete(ledDiodeTaskHandle);
    vTaskDelete(checkButtonTaskHandle);

    ledOff();

    changeButtonValue = 0;

    resetValues();
  }

  if ((okPoints + noOkPoints) == target) // end game
  {
    // overwriting the display with new data to show the final score
    display.setCursor(0, 25);
    display.setTextSize(3);
    display.clearDisplay();
    display.print(String(okPoints) + "/" + String(target));
    display.display();

    vTaskDelete(ledDiodeTaskHandle);
    vTaskDelete(checkButtonTaskHandle);

    for (int i = 0; i <= 15; i++) // LEDs blinking after game end
    {
      digitalWrite(led3Pin, HIGH);
      digitalWrite(led2Pin, HIGH);
      digitalWrite(led1Pin, HIGH);
      vTaskDelay(250 / portTICK_PERIOD_MS);
      ledOff();
      vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    historyList[historyCount].clear(); // delete history record(old record)

    historyList[historyCount] = String(okPoints) + "/" + String(target) + "_" + String(minSpeed) + "/" + String(maxSpeed) + "/" + String(resolution) + "_" + String(userNumber); // safe all setings and users score to history(max 5 records in history, then history is rewritten)

    historyCount++;
    userNumber++;
    if (historyCount == 5) // ensuring history overwriting after 5 records
    {
      historyCount = 0;
    }

    resetValues();
  }

  if (digitalRead(scrollButton) && acceptButtonValue == 0) // switching between history and main screen
  {
    if (changeDisplay == 0)
    {
      changeDisplay = 1;
    }
    else if (changeDisplay == 1)
    {
      changeDisplay = 0;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  if (digitalRead(sleepButton)) // putting the board to sleep and clear all display content
  {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.print("GO sleep..");
    display.display();

    delay(1500);

    display.clearDisplay();
    display.display();

    esp_deep_sleep_start();
  }
  vTaskDelay(200 / portTICK_PERIOD_MS);
}