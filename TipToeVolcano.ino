/*
 *  Tip-Toe Volcano
 *  by Big Potato Games 2020
 *  Lead development by Daniel King
 *  Original game by Big Potato Games
 *
 *
 *  --------------------
 *  Blinks by Move38
 *  Brought to life via Kickstarter 2018
 *
 *  @madewithblinks
 *  www.move38.com
 *  --------------------
 */

enum signalStates {SETUP, HIDE, DEATH, RESET};
byte signalState = SETUP;

bool isMagma = false;
bool isChaos = false;
bool isRevealed = false;
bool isKiller = false;

Timer eruptionTimer;
#define ERUPTION_TIME 2000
bool magmaFaces[6] = {false, false, false, false, false, false};

void setup() {
  // put your setup code here, to run once:
  randomize();
}

void loop() {
  switch (signalState) {
    case SETUP:
      setupLoop();
      setupDisplay();
      break;
    case HIDE:
      hideLoop();
      hideDisplay();
      break;
    case DEATH:
      deathLoop();
      deathDisplay();
      break;
    case RESET:
      resetLoop();
      setupDisplay();
      break;
  }

  FOREACH_FACE(f) {
    byte sendData = (magmaFaces[f] << 3) | (isKiller << 2) | (signalState);
    setValueSentOnFace(sendData, f);
  }

  buttonSingleClicked();
  buttonDoubleClicked();
  buttonMultiClicked();
}

void setupLoop() {

  if (buttonSingleClicked()) {
    isMagma = !isMagma;
  }

  if (buttonDoubleClicked()) {
    signalState = HIDE;
    setGrass();
    if (isChaos) {
      isMagma = random(1);
    }


  }

  if (buttonMultiClicked() && buttonClickCount() == 4) {
    isChaos = !isChaos;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == HIDE) {
        signalState = HIDE;
        setGrass();
        if (isChaos) {
          isMagma = random(1);
        }
      }
    }
  }
}

void hideLoop() {

  if (buttonDoubleClicked()) {
    signalState = RESET;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == DEATH) {
        signalState = DEATH;
        eruptionTimer.set(ERUPTION_TIME);
      } else if (getSignalState(getLastValueReceivedOnFace(f)) == RESET) {
        signalState = RESET;
      }
    }
  }

  if (buttonSingleClicked()) {
    isRevealed = true;
    if (isMagma) {
      signalState = DEATH;
      eruptionTimer.set(ERUPTION_TIME);
      isKiller = true;
    }
  }
}

void deathLoop() {
  if (buttonDoubleClicked()) {
    signalState = SETUP;
    if (isChaos) {
      isMagma = false;
    }
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == SETUP) {
        signalState = SETUP;
      } else if (getSignalState(getLastValueReceivedOnFace(f)) == RESET) {
        signalState = RESET;
      }
    }
  }
}

void resetLoop() {
  signalState = SETUP;

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == HIDE || getSignalState(getLastValueReceivedOnFace(f)) == DEATH) {
        signalState = RESET;
      }
    }
  }
}

byte getSignalState(byte data) {
  return (data & 3);
}

byte getIsKiller(byte data) {
  return ((data >> 2) & 1);
}

byte getIsMagma(byte data) {
  return ((data >> 3) & 1);
}

#define SETUP_ANIM_INTERVAL 200

void setupDisplay() {

  setColor(OFF);
  byte whichFrame = (millis() % (SETUP_ANIM_INTERVAL * 6)) / SETUP_ANIM_INTERVAL;


  Color color1;
  Color color2;

  if (isChaos) {
    color1 = GREEN;
    color2 = RED;
  } else {
    if (isMagma) {
      color1 = RED;
      color2 = RED;
    } else {
      color1 = GREEN;
      color2 = GREEN;
    }
  }

  setColorOnFace(color1, whichFrame);
  setColorOnFace(color2, (whichFrame + 3) % 6);

  //  byte magmaBrightness;
  //  byte grassBrightness;
  //
  //  if (isMagma) {
  //    magmaBrightness = 255;
  //    grassBrightness = 100;
  //  } else {
  //    magmaBrightness = 100;
  //    grassBrightness = 255;
  //  }
  //
  //  if (isChaos) {
  //    setColorOnFace(dim(RED, random(155) + 100), 0);
  //    setColorOnFace(dim(RED, random(155) + 100), 2);
  //    setColorOnFace(dim(RED, random(155) + 100), 4);
  //    setColorOnFace(dim(GREEN, random(155) + 100), 1);
  //    setColorOnFace(dim(GREEN, random(155) + 100), 3);
  //    setColorOnFace(dim(GREEN, random(155) + 100), 5);
  //  } else {
  //    setColorOnFace(dim(RED, magmaBrightness), 0);
  //    setColorOnFace(dim(RED, magmaBrightness), 2);
  //    setColorOnFace(dim(RED, magmaBrightness), 4);
  //    setColorOnFace(dim(GREEN, grassBrightness), 1);
  //    setColorOnFace(dim(GREEN, grassBrightness), 3);
  //    setColorOnFace(dim(GREEN, grassBrightness), 5);
  //  }
}

#define GRASS_HUE_LO 56
#define GRASS_HUE_HI 88
byte grassHues[6];

void setGrass() {
  isRevealed = false;
  isKiller = false;
  FOREACH_FACE(f) {
    grassHues[f] = GRASS_HUE_LO + random(GRASS_HUE_HI - GRASS_HUE_LO);
    magmaFaces[f] = false;
  }
}

void hideDisplay() {
  //determine brightness
  byte grassBrightness = 255;
  if (isRevealed) {
    grassBrightness = 100;
  }

  FOREACH_FACE(f) {
    setColorOnFace(makeColorHSB(grassHues[f], 255, grassBrightness), f);
  }
}

Timer magmaSpreadTimer;
#define MAGMA_INTERVAL 1000

void deathDisplay() {

  //first figure out which faces should be automatically magma'd
  if (isKiller) {//the center of the volcano
    FOREACH_FACE(f) {
      magmaFaces[f] = true;
    }
  } else {//things that are not the volcano
    //    //look around to see if you neighbor the volcano
    //    FOREACH_FACE(f) {
    //      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
    //        if (getIsKiller(getLastValueReceivedOnFace(f)) == true) {//this neighbor is the volcano itself
    //          magmaFaces[f] = true;
    //          magmaFaces[(f + 5) % 6] = true;
    //          magmaFaces[(f + 1) % 6] = true;
    //        }
    //      }
    //    }
  }

  //do magma spreading stuff!
  if (magmaSpreadTimer.isExpired()) {
    magmaSpreadTimer.set(MAGMA_INTERVAL - random(500));

    //let's evaluate our neighbors!
    byte tempMagmaFaces[6] = {false, false, false, false, false, false};
    FOREACH_FACE(f) {
      byte magmaNeighbors = 0;
      if (magmaFaces[f] == false) {//I am not magma yet!
        if (magmaFaces[(f + 1) % 6] == true) {
          magmaNeighbors++;
        }
        if (magmaFaces[(f + 5) % 6] == true) {
          magmaNeighbors++;
        }
        if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
          if (getIsMagma(getLastValueReceivedOnFace(f)) == true) {//this neighbor is magma
            magmaNeighbors++;
          }
        }

        //now that we've evaluated the neighbors, should we become magma?
        switch (magmaNeighbors) {
          case 0:
            break;
          case 1:
            if (random(2) == 0) {//ok chance of becoming magma
              tempMagmaFaces[f] = true;
            }
            break;
            //          case 2:
            //            if (random(8) == 0) {//low chance of becoming magma
            //              tempMagmaFaces[f] = true;
            //            }
            //            break;
        }
      }
    }//end checking loop

    //now actually apply the new magma faces
    FOREACH_FACE(f) {
      if (tempMagmaFaces[f] == true) {
        magmaFaces[f] = true;
      }
    }
  }

  //now do the graphics
  byte timerProgress = map(eruptionTimer.getRemaining(), 0, ERUPTION_TIME, 0, 255); //goes from 255 at the beginning to 0 at the end
  timerProgress = (timerProgress * 2) - ((timerProgress * timerProgress) / 255);

  byte brightnessRumble = timerProgress / 2;//As the explosion happens, the amount of random reduction in brightness reduces

  FOREACH_FACE(f) {
    if (magmaFaces[f] == true) {
      if (isKiller) {
        setColorOnFace(makeColorHSB(random(31), 255 - timerProgress, 255 - random(brightnessRumble / 2)), f);
      } else {
        setColorOnFace(makeColorHSB(random(23), 255 - timerProgress, 255 - random(brightnessRumble)), f);
      }
    } else {
      setColorOnFace(makeColorHSB(grassHues[f], 255 - timerProgress, map(timerProgress, 0, 255, 100, 255 - random(brightnessRumble))), f);
    }
  }
}
