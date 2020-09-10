enum signalStates {SETUP, HIDE, DEATH, RESET};
byte signalState = SETUP;

bool isMagma = false;
bool isChaos = false;
bool isRevealed = false;
bool isKiller = false;

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

  setValueSentOnAllFaces(signalState);

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
      if (getLastValueReceivedOnFace(f) == HIDE) {
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
      if (getLastValueReceivedOnFace(f) == DEATH) {
        signalState = DEATH;
      } else if (getLastValueReceivedOnFace(f) == RESET) {
        signalState = RESET;
      }
    }
  }

  if (buttonSingleClicked()) {
    isRevealed = true;
    if (isMagma) {
      signalState = DEATH;
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
      if (getLastValueReceivedOnFace(f) == SETUP) {
        signalState = SETUP;
      } else if (getLastValueReceivedOnFace(f) == RESET) {
        signalState = RESET;
      }
    }
  }
}

void resetLoop() {
  signalState = SETUP;

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//neighbor!
      if (getLastValueReceivedOnFace(f) == HIDE || getLastValueReceivedOnFace(f) == DEATH) {
        signalState = RESET;
      }
    }
  }
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

#define GRASS_HUE_LO 60
#define GRASS_HUE_HI 100
byte grassHues[6];

void setGrass() {
  isRevealed = false;
  isKiller = false;
  FOREACH_FACE(f) {
    grassHues[f] = GRASS_HUE_LO + random(GRASS_HUE_HI - GRASS_HUE_LO);
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

  if (!isKiller) {
    byte fireFace = random(5);
    FOREACH_FACE(f) {
      if (f == fireFace) {
        setColorOnFace(RED, f);
      } else {
        setColorOnFace(makeColorHSB(grassHues[f], 255, 150), f);
      }
    }
  } else {
    setColor(RED);
    setColorOnFace(ORANGE, random(5));
    setColorOnFace(ORANGE, random(5));
  }
}
