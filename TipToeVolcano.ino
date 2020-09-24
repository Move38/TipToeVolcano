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

  byte sendData = (isKiller << 2) | (signalState);
  setValueSentOnAllFaces(sendData);

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

void setupDisplay() {
  byte magmaBrightness;
  byte grassBrightness;

  if (isMagma) {
    magmaBrightness = 255;
    grassBrightness = 100;
  } else {
    magmaBrightness = 100;
    grassBrightness = 255;
  }

  if (isChaos) {
    setColorOnFace(dim(RED, random(155) + 100), 0);
    setColorOnFace(dim(RED, random(155) + 100), 2);
    setColorOnFace(dim(RED, random(155) + 100), 4);
    setColorOnFace(dim(GREEN, random(155) + 100), 1);
    setColorOnFace(dim(GREEN, random(155) + 100), 3);
    setColorOnFace(dim(GREEN, random(155) + 100), 5);
  } else {
    setColorOnFace(dim(RED, magmaBrightness), 0);
    setColorOnFace(dim(RED, magmaBrightness), 2);
    setColorOnFace(dim(RED, magmaBrightness), 4);
    setColorOnFace(dim(GREEN, grassBrightness), 1);
    setColorOnFace(dim(GREEN, grassBrightness), 3);
    setColorOnFace(dim(GREEN, grassBrightness), 5);
  }
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

void deathDisplay() {

  //first figure out which faces should be magma'd
  if (isKiller) {//the center of the volcano
    FOREACH_FACE(f) {
      magmaFaces[f] = true;
    }
  } else {//things that are not the volcano
    //look around to see if you neighbor the volcano
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
        if (getIsKiller(getLastValueReceivedOnFace(f)) == true) {//this neighbor is the volcano itself
          magmaFaces[f] = true;
          magmaFaces[(f + 5) % 6] = true;
          magmaFaces[(f + 1) % 6] = true;
        }
      }
    }
  }

  //now do the graphics
  byte timerProgress = map(eruptionTimer.getRemaining(), 0, ERUPTION_TIME, 0, 255); //goes from 255 at the beginning to 0 at the end
  timerProgress = (timerProgress * 2) - ((timerProgress * timerProgress) / 255);

  byte brightnessRumble = timerProgress / 2;//As the explosion happens, the amount of random reduction in brightness reduces

  FOREACH_FACE(f) {
    if (magmaFaces[f] == true) {
      setColorOnFace(makeColorHSB(random(25), 255 - timerProgress, 255 - random(brightnessRumble)), f);
    } else {
      setColorOnFace(makeColorHSB(grassHues[f], 255 - timerProgress, map(timerProgress, 0, 255, 100, 255 - random(brightnessRumble))), f);
    }
  }
}
