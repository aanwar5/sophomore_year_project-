#include "HX711.h" //including the HX-711 load cell library 

// Ive used the millis() function 
//Student Name: Abdullah Anwar 
//Student ID: 216358214 
//Unit Name: SER202 Programming for Embedded Systems


struct tunes //Contains the tune and tune duration in memory
{
  int tune[6]; 
  int tune_duration[6];
};

struct dogs_type //Contains all the essentials of the dog types and the requirement of the feeder to function. Including the weight range, wait duration during the buzzer, intervals in between the time and feed duration.
{
  int weight_range[2];
  const tunes* tune;
  unsigned long interval;
  int wait_duration_s;
  unsigned long last_time;
  double feed_duration_s;
};


tunes large_dog_tune = { {0, 500, 80, 140, 120, 100}, {50, 70, 90, 110, 130, 150} };  //Tune for the large dog , duration of tune 
tunes small_dog_tune = { {100, 159, 180, 70, 120, 100}, {150, 70, 90, 190, 130, 150} }; //Tune for the small dog , duration of tune

dogs_type SMALL_DOG = { {100,400}, &small_dog_tune, 50000UL, 5, 0UL, 1.00}; // Weight range between 100 and 400 grams, tune, 50 second interval(unsigned long), 5 second wait time, 1 second rotation of the continuous rotation servo
dogs_type LARGE_DOG = { {600,900}, &large_dog_tune, 90000UL, 5, 0UL, 2.00}; // Weight range between 600 and 900 grams, tune, 90 second interval(unsigned long) --> this function works 40 seconds after the first 50 seconds(which have been allocated for the small dog), 5 second wait time, 2 second rotation of the continuous rotation servo) 

dogs_type MY_DOGS[] = {SMALL_DOG, LARGE_DOG}; //Two types of dogs have been defined; small and large

const float CALIBRATION_FACTOR = 1500; //Calibration of the load cell has been set to 1500. 
const int DOUT = 3; //pin output number for DOUT/DT of the load cell, connection on the arduino (PIN3)
const int CLK = 2; //pin output number for SCK/CLK of load cell connection on the arduino (PIN2)
HX711 scale(DOUT, CLK);

void setup() 
{

  DDRB = (1 << DDB7); // pin output for the buzzer(PIN13)
  TCCR1A = (1 << COM1C1)|(1 << WGM11);
  TCCR1B = (1 << WGM13)|(1 << WGM12)|(1 << CS12);

  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  
  DDRE |= (1 << DDE3); //pin output for the continuous rotation servo (PIN5)
  TCCR3A = (1 << COM3A1)|(1 << WGM31);
  TCCR3B = (1 << WGM33)|(1 << WGM32)|(1 << CS32);
  ICR3 = 1250; 
  OCR3A = 93;
}

void loop()
{
  int array_size = sizeof(MY_DOGS) / sizeof(MY_DOGS[0]);
  for (int i = 0; i < array_size; i++)
  {
    if (MY_DOGS[i].last_time + MY_DOGS[i].interval <= millis())
    {
      MY_DOGS[i].last_time = millis();

      buzzer(MY_DOGS[i]);
      feed_dog(MY_DOGS[i]);
    }
  }
}

void feed_dog(dogs_type my_dog)
{
  unsigned long start = millis();
  while(millis() - start < my_dog.wait_duration_s * 1000)
  {
    if(check_dog(my_dog))
    {
      drop_food(my_dog);

      break;
    }
  }
}

bool check_dog(dogs_type my_dog) //a boolean to check the type of dog present, whether it is small or large dog
{
  scale.set_scale(CALIBRATION_FACTOR);

  if (scale.get_units() >= my_dog.weight_range[0] && scale.get_units() <= my_dog.weight_range[1])
  {

    return true;
  }
  return false;
}

void drop_food(dogs_type my_dog) 
{
  OCR3A = 43; // starting position of the servo
    for (int i = 0; i < my_dog.feed_duration_s * 1000; i++)
        _delay_ms(1); //delay
    OCR3A = 93; //rotation has been set for clockwise motion
}

void buzzer(dogs_type my_dog) // function for the tune which is played
{
  int array_size = sizeof(my_dog.tune->tune)/sizeof(my_dog.tune->tune[0]);
  for (int i = 0; i < array_size; i++)
  {
    ICR1 = my_dog.tune->tune[i];
    OCR1C = my_dog.tune->tune[i]/2;
    TCNT1 = 0;
    for (int j = 0; j < my_dog.tune->tune_duration[i]; j++)
      _delay_ms(1);
  }
  ICR1 = 0;
  OCR1C = 0;
  TCNT1 = 0;
}
