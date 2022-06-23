// #define Wire Wire1
#include <Wire.h>
#include <EEPROM.h>

int channel = 1; // change this number to use a different midi channel
int addr = 41; // change this number to use a different i2c address

int received_value;
int pot[10] = {14, 12, 11, 13, A15, A10, A20, A16, A17, A14};
int potcc[10];
int potsample1[10];
int potsample2[10];
int potsample3[10];
int potsample4[10];
int potsample5[10];
int potsample6[10];
int lastpotcc[10];
int poti2c[10];

int shiftcount; // counter for shift hold

// setup key pins. zero indexed.
int key[12] {0, 1, 15, 16, 17, 18, 19, 33, 32, 25, 23, 22};
int shift = 10;
int butn[8] = {2, 3, 4, 9, 8, 7, 5, 6};

// setup scale arrays

int chromatic[12] {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
int major[12] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19};
int minor[12] = {0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19};
int harmminor[12] = {0, 2, 3, 5, 7, 8, 11, 12, 15, 16, 18, 19};
int majorpent[12] = {0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26};
int minorpent[12] = {0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24, 27};



int scale[12] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19}; // default scale is major
int octave = 60;
int buttonOctave = 48;
int buttonOctavePot;
int lastButtonOctaveValue;
int transpose = 0;
int transposePot;
int lastTransposeValue;


int thresh[12];
int maximum[12] = {2400, 2600, 2500, 2700, 2600, 2600, 2600, 2900, 2600, 2900, 2700, 2600};


// setup flags for held notes
int playflagmono = 0;
int playflag[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int buttonFlag[8];
int iibuttonFlag[8];

// key velocity

int velocity[12];

// this array stores values for current keys (key 1 = index 0, etc)

int currentkey[12];

// this array stores touchread values for calibration

int lastcalib[12];
int basecalib[12];
int lowestcalib[12];

// variables for logging which key is used for cc, and its thresholds

int cc1[4];
int cc2[4];
int cc3[4];
int currentcc;
int previouscc;
int ccmin;
int ccmax;   
int cckey;

// i2c key variables

int currentkeyflag1 = 0;
int currentkeyflag2 = 0;
int iikeyflag1[12];
int iikeyflag2[12];
int lastkeyflag1 = 0;
int lastkeyflag2 = 0;

int iikey1 = 0;
int iikey2 = 0;
int iirawkey1 = 50;
int iirawkey2 = 50;
int iiButton;
int iipressure = 0;
int pressurekey = 0;
int pressureindex = 0;

int lastcalibread;

int LED = 20;

int keyGate1 = 12;
int keyGate2 = 11;
int buttonGate = 21;

// midiMode settings
enum buttonMode {
  standard,
  omnichord
};

buttonMode currButtonMode = standard;



void setup() {
  pinMode(LED, OUTPUT);
  pinMode(keyGate1, OUTPUT); //key gate low OUT
  pinMode(keyGate2, OUTPUT); //key gate high OUT
  pinMode(buttonGate, OUTPUT); //button gate OUT
  pinMode(10, INPUT_PULLUP); //shift button IN
  for (int i = 0;  i < 8; i++) { // set up 8 buttons
    pinMode(butn[i], INPUT_PULLUP);
  } 

   
  Wire.begin(addr);               // join i2c bus with address #41
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  Serial.begin(9600);//setup serial connection
  digitalWrite(LED, HIGH);
  delay(1000);     
  digitalWrite(LED, LOW);
  for (int i = 0;  i < 12; i++) {
    thresh[i] = EEPROM.read(i) * 11; //assign saved key calibration to threshold array
  }
}

void loop() {
  ////////////////// counter for shift functions /////////////////////
  if (digitalRead(shift) == LOW) {  // increment shiftcount every 10ms as long as shift is held
    shiftcount++; 
    Serial.println(shiftcount);
    delay(10);
  } else {
    shiftcount = 0;
  }

  if (digitalRead(shift) == LOW && touchRead(key[11]) > thresh[11]) { /// if shift key and key 6 are pressed
    currButtonMode = omnichord;
  }


  if (digitalRead(shift) == LOW && touchRead(key[11]) > thresh[11]) { /// if shift key and key 6 are pressed
    currButtonMode = standard;
  }

  /////////////////////////////////////scale and octave setup////////////////////////////////////////////
  // scales
  if (digitalRead(shift) == LOW && touchRead(key[5]) > thresh[5]) { /// if shift key and key 6 are pressed
    for (int i = 0;  i < 12; i++) { // loop 12 times
      scale[i] = minorpent[i];
        }
        //shiftcount = 0;
    }

  if (digitalRead(shift) == LOW && touchRead(key[4]) > thresh[4]) { /// if shift key and key 5 are pressed
    for (int i = 0;  i < 12; i++) { // loop 12 times
      scale[i] = majorpent[i];
      
        }
        //shiftcount = 0;
    }

  if (digitalRead(shift) == LOW && touchRead(key[3]) > thresh[3]) { /// if shift key and key 4 are pressed
    for (int i = 0;  i < 12; i++) { // loop 12 times
      scale[i] = harmminor[i];
    }
        //shiftcount = 0;
  }

  if (digitalRead(shift) == LOW && touchRead(key[2]) > thresh[2]) { /// if shift key and key 3 are pressed
    for (int i = 0;  i < 12; i++) { // loop 12 times
      scale[i] = minor[i];
      
        }
        //shiftcount = 0;
    }

  if (digitalRead(shift) == LOW && touchRead(key[1]) > thresh[1]) { /// if shift key and key 2 are pressed
    for (int i = 0;  i < 12; i++) { // loop 12 times
      scale[i] = major[i];
        }
        //shiftcount = 0;
    }
    
  if (digitalRead(shift) == LOW && touchRead(key[0]) > thresh[0]) { /// if shift key and key 1 are pressed
    for (int i = 0;  i < 12; i++) { // loop 12 times
      scale[i] = chromatic[i];
    }
  }
    
  //OCTAVES
  for (int i = 0;  i < 8; i++) { //loop 8 times
      if(digitalRead(shift) == LOW && digitalRead(butn[i]) == LOW) { //if shift is held and button is pressed
        octave = (i + 1) * 12; // 
      }
  }

  //TRANSPOSITION
  transposePot = analogRead(pot[0]);
  transposePot = map(transposePot, 1023, 0, 0, 11);
  transposePot = constrain(transposePot, 0, 11);
  if(digitalRead(shift) == LOW && (transposePot != lastTransposeValue)) { //  if transpose value changes while shift is held
    shiftcount = 0;
    Serial.println(transposePot);
    transpose = transposePot;
    usbMIDI.sendNoteOn(scale[0] + transpose + octave, 100, channel); // play new root note
    delay(50);
    usbMIDI.sendNoteOff(scale[0] + transpose + octave, 100, channel); 
  }
  delay(2);
  lastTransposeValue = transposePot;

  //BUTTON OCTAVE
  buttonOctavePot = analogRead(pot[1]);
  buttonOctavePot = map(buttonOctavePot, 1023, 0, 1, 8);
  buttonOctavePot = constrain(buttonOctavePot, 1, 8);
  if(digitalRead(shift) == LOW && (buttonOctavePot != lastButtonOctaveValue)) { //  if transpose value changes while shift is held
    shiftcount = 0;
    buttonOctave = buttonOctavePot * 12;
    usbMIDI.sendNoteOn(scale[0] + transpose + buttonOctave, 100, channel); // play new root note
    delay(50);
    usbMIDI.sendNoteOff(scale[0] + transpose + buttonOctave, 100, channel); 
  }
  delay(2);
  lastButtonOctaveValue = buttonOctavePot;

  //////////////////////////////////// A LENGTHY CALIBRATION DETOUR ///////////////////////////////////////////////////

  if (shiftcount > 700) { // if shift has been held for more than 7 seconds without any other buttons pressed
    for (int i = 0;  i < 12; i++) {
      basecalib[i] = touchRead(key[i]); // set the unpressed key value
    }
  
    digitalWrite(LED, HIGH);
    delay(1500);
    digitalWrite(LED, LOW);
    int brk = 0;
    while (brk == 0) {
      for (int i = 0;  i < 12; i++) {    // loop 12 times
        int currentcalib[12];
        currentcalib[i] = touchRead(key[i]);
        if (currentcalib[i] > basecalib[i] + 200) { // if key value i is over 200 above baseline reading
          digitalWrite(LED, HIGH); //flash led
          if (currentcalib[i] < lastcalib[i]) {    // if touchread is less than the previous reading
            lowestcalib[i] = currentcalib[i]; // set touchread as new lowest value
          }
          Serial.print("key...");
          Serial.print(i);
          Serial.print("...");
          Serial.println(currentcalib[i]);
          delay(5); // add a 5 ms delay between last reading and new reading
          digitalWrite(LED, LOW);        // turn LED off
          lastcalib[i] = currentcalib[i]; //set the last key value for next run through to check againgst
        }
        EEPROM.write(i, lowestcalib[i] / 11);   // write the lowest value from the current key calibration to EEPROM memory.
      }

      if (digitalRead(shift) == LOW) {  // increment shiftcount every 10ms as long as shift is held
        shiftcount++; 
        Serial.println(shiftcount);
        delay(10);
      }
      
      if (shiftcount > 900) { // if shift is held for a few more seconds during this loop, exit calibration mode
        shiftcount = 0; //reset shift counter
        brk = 1; //break the calibration loop
        digitalWrite(LED, LOW);
        Serial.println("exit calibration");
        for (int i = 0;  i < 12; i++) {
          Serial.print("EEPROM Value:");
          Serial.println(EEPROM.read(i) * 11);
          Serial.print("actual calibration Value:");
          Serial.println(lowestcalib[i]);
        }
        digitalWrite(LED, HIGH); //flash the led
        delay(500);
        digitalWrite(LED, LOW);
        for (int i = 0;  i < 12; i++) {
          thresh[i] = EEPROM.read(i) * 11; //assign new saved key calibration to threshold array
        }
      }
    }
  }
    
  //////////////////////////////////// MIDI SECTION ///////////////////////////////////////////////////


  if (digitalRead(shift) == HIGH) { // only move on to detecting musical key presses if shift is released
                                    // this is so pressing shift+key 1 doesnt also play note 1!
    shiftcount = 0; // reset shiftcount

    for (int i = 11;  i > -1; i = i - 1) { // loop 12 times, i increments from 11 to 0, so this one for loop checks all keys.
      currentkey[i] = touchRead(key[i]); 
      if (currentkey[i] > thresh[i] && playflag[i] == 0) {
        playflag[i] = 1;
        if (playflag[i] == 1) { 
          cckey = key[i];
          ccmin = thresh[i];
          ccmax = maximum[i];
        }
        delay(1);
        velocity[i] = touchRead(key[i]); 
        velocity[i] = map(velocity[i], thresh[i], maximum[i], 0, 127); 
        velocity[i] = constrain(velocity[i], 0, 127); 
        usbMIDI.sendNoteOn(scale[i] + transpose + octave, velocity[i], channel); 
      }

      if(currentkey[i] < thresh[i] && playflag[i] == 1) {
        playflag[i] = 0;
        usbMIDI.sendNoteOff(scale[i] + transpose + octave, 0, channel); 
      }
    }

    // assign midi cc 1 to lowest key held and update if changed
    currentcc = touchRead(cckey);
    currentcc = map(currentcc, ccmin, ccmax, 0, 127);
    currentcc = constrain(currentcc, 0, 127);
    if(currentcc != previouscc) {
      previouscc = currentcc;
      usbMIDI.sendControlChange(1, currentcc, channel);
    } 
            
    // button notes
    for (int i = 0;  i < 8; i++) { //loop 8 times
        if(digitalRead(butn[i]) == LOW && buttonFlag[i] == 0) {
          buttonFlag[i] = 1;
          usbMIDI.sendNoteOn(scale[i] + transpose + buttonOctave, 100, channel);
        }
        if(digitalRead(butn[i]) == HIGH && buttonFlag[i] == 1) {
        buttonFlag[i] = 0;
          usbMIDI.sendNoteOff(scale[i] + transpose + buttonOctave, 100, channel);
        }
    }
            
    for (int i = 0;  i < 10; i++) { // loop 10 times to iterate throuch each pot
      potsample1[i] = analogRead(pot[i]);
      potsample1[i] = map(potsample1[i], 1023, 0, 0, 127);
      potsample1[i] = constrain(potsample1[i], 0, 127);
      delay(1);
      potsample2[i] = analogRead(pot[i]);
      potsample2[i] = map(potsample2[i], 1023, 0, 0, 127);
      potsample2[i] = constrain(potsample2[i], 0, 127);
      delay(1);
      potsample3[i] = analogRead(pot[i]);
      potsample3[i] = map(potsample3[i], 1023, 0, 0, 127);
      potsample3[i] = constrain(potsample3[i], 0, 127);
      delay(1);
    //  potsample4[i] = analogRead(pot[i]);
    //  potsample4[i] = map(potsample4[i], 1023, 0, 0, 127);
    //  potsample4[i] = constrain(potsample4[i], 0, 127);
    //  delay(1);
    //  potsample5[i] = analogRead(pot[i]);
    //  potsample5[i] = map(potsample5[i], 1023, 0, 0, 127);
    //  potsample5[i] = constrain(potsample5[i], 0, 127);
    //  delay(1);
    //  potsample6[i] = analogRead(pot[i]);
    //  potsample6[i] = map(potsample6[i], 1023, 0, 0, 127);
    //  potsample6[i] = constrain(potsample6[i], 0, 127);
    //  delay(1);
      potcc[i] = potsample1[i] + potsample2[i] + potsample3[i]; // + potsample4[i] + potsample5[i] + potsample6[i]; // read the pot and map to midi values 6 times, and take an average.
      potcc[i] = potcc[i] / 3;
      
      if ((potcc[i] < lastpotcc[i] - 1) || (potcc[i] > lastpotcc[i] + 1)) { //  if pot value has changed by 2 or more in either direction (for further smoothing), output as cc on channel 2 - 6
        delay(1);
        lastpotcc[i] = potcc[i];
        Serial.println(potcc[0]);
        usbMIDI.sendControlChange(i+2, potcc[i], channel);
      }

    }
          
    //////////////////////////////////// END OF MIDI SECTION ///////////////////////////////////////////////////

    //////////////////////////////////// I2C SECTION ///////////////////////////////////////////////////
    // NOTE CHANNEL 1

    for (int i = 11;  i > -1; i = i - 1) { 
      int currentnote = touchRead(key[i]);
      // channel one key on  
      if (iirawkey2 != i) { // skip if its the loop iteration of the key held by channel 2
        if (iikeyflag1[i] == 0 && currentkeyflag1 == 0 && currentnote > thresh[i] && iikeyflag2 [i] == 0) { // only trigger note on if no keys are pressed.
          currentkeyflag1 = 1; // set keylogger to on
          iikeyflag1[i] = 1;
          iirawkey1 = i; // note the raw key value
          iikey1 = scale[i] + octave + transpose; // save associated note number to variable
          pressurekey = key[i]; // note which key to get pressure value from
          pressureindex = i;  // note the raw index of the pressure key
          Serial.write("key is pressed!");
          digitalWrite(keyGate1, HIGH);
        }
        // channel 1 key off
        if (iikeyflag1[i] == 1 && currentnote < thresh[i]) { // if last we knew key is being held, the value goes below threshold and its not the key from, channel 2
          iirawkey1 = 50; // set raw key value for chanel 2 to ignore out of bounds (so it doesnt ignore any keys)
          iikeyflag1[i] = 0;
          currentkeyflag1 = 0; // set note logger to off
          Serial.write("key is released!");
          digitalWrite(keyGate1, LOW);
        }
      }
      
      // channel 2 ][][[]
      if (iirawkey1 != i) {  // skip if its the loop iteration of the key held by channel 1
        // channel two key on  
        if (iikeyflag2[i] == 0 && currentkeyflag2 == 0 && currentnote > thresh[i]) { // key was not already pressed, value goes above threshold, and the current loop iteration doesnt match channel 2's held key
          currentkeyflag2 = 1; // set keylogger to on
          iikeyflag2[i] = 1;
          iirawkey2 = i; // note the raw key value
          iikey2 = scale[i] + octave + transpose; // save associated note number to variable
          pressurekey = key[i]; // note which key to get pressure value from
          pressureindex = i;  // note the raw index of the pressure key
          Serial.write("key is pressed!");
          digitalWrite(keyGate2, HIGH);
        }
        // channel 2 key off
        if (iikeyflag2[i] == 1 && currentnote < thresh[i]) { // if last we knew key is being held, the value goes below threshold and its not the key from, channel 2
          iirawkey2 = 50; // set raw key value for chanel 2 to ignore out of bounds (so it doesnt ignore any keys)
          iikeyflag2[i] = 0;
          currentkeyflag2 = 0; // set note logger to off
          Serial.write("key is released!");
          digitalWrite(keyGate2, LOW);
        }
      }
    }

    for (int i = 0;  i < 8; i++) { //loop 8 times
      if(digitalRead(butn[i]) == LOW && iibuttonFlag[i] == 0) {
        iibuttonFlag[i] = 1;
        iiButton = i;
        digitalWrite(buttonGate, HIGH);
        delay(5);
        digitalWrite(buttonGate, LOW);
      }
      if(digitalRead(butn[i]) == HIGH && iibuttonFlag[i] == 1) {
        iibuttonFlag[i] = 0;
      }
    }

    // i2c pot reading
    for (int i = 0;  i < 10; i++) {
      // read pot i value, scale for i2c and store in poti2c[i]
      poti2c[i] = analogRead(pot[i]);
      poti2c[i] = map(poti2c[i], 0, 1023, 16384, 0);
      poti2c[i] = constrain(poti2c[i], 0, 16384);
    }
  }
}


///////// function that executes every time data is sent by leader /////////////
void receiveEvent(int howMany) {
  received_value = Wire.read();           // receive byte as an integer and store in received value
 // Serial.println(received_value);         // print the integer
}

//////////// function that executes whenever data is requested by leader////////////
void requestEvent() {

  //i2c pot value request (IIQ 5-9)
  for (int i = 5;  i < 15; i++) { 
    if(received_value == i) { // if teletypes sent command matches i
      Wire.write(poti2c[i - 5] >> 8);  // send first byte of pot i value
      Wire.write(poti2c[i - 5] & 255);  // send second byte of pot i value
    }
  }                
  
  //i2c channel 1 note value (IIQ 1)
  if (received_value == 1) { 
    Wire.write(iikey1 >> 8);  // send first byte of current pressed key value
    Wire.write(iikey1 & 255);  // send second byte of current pressed key value
    //Serial.println(iikey1); 
  }
       
  //i2c channel 2 note value (IIQ 2)
  if (received_value == 2) { 
    Wire.write(iikey2 >> 8);  // send first byte of current pressed key value
    Wire.write(iikey2 & 255);  // send second byte of current pressed key value
    //Serial.println(iikey2); 
  }       

  //i2c button value (IIQ 3)
  if(received_value == 3) { 
    Wire.write(iiButton >> 8);  // send first byte of last pressed button
    Wire.write(iiButton & 255);  // send second byte of last pressed button
    //Serial.println(iibutton); 
  }       

  //i2c pressure output request (IIQ 4)
  if(received_value == 4) {
    iipressure = touchRead(pressurekey);          // read the value of the designated pressure key
    iipressure = map(iipressure, thresh[pressureindex], maximum[pressureindex], 0, 16384); 
    iipressure = constrain(iipressure, 0, 16384);
    Wire.write(iipressure >> 8);  // send first byte of key pressure value
    Wire.write(iipressure & 255);  // send second byte of key pressure value
  }
}
