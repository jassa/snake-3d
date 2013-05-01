#include "snake.h"

Snake::Snake(int x, int y, int maxLength) : initX(x), initY(y) {
  this->maxLength = maxLength + minLength;
  init();
}

Snake::Snake() {
  maxLength = 100 + minLength;
  initX = 0;
  initY = 0;
  init();
}

Snake::~Snake() {
  delete[] tail;
}

void Snake::init() {
  tail = new double* [maxLength];

  for(int i = 0; i < maxLength; i++) {
    tail[i] = new double[2];
    tail[i][0] = tail[i][1] = 0;
  }

  reset();
}

void Snake::eat() {
  length++;
}

bool Snake::full() {
  return length == maxLength - 1;
}

void Snake::moveTo(int x, int y) {
  for (int i = length; i > 0; i--) {
    tail[i][0] = tail[i - 1][0];
    tail[i][1] = tail[i - 1][1];
  }

  tail[0][0] = tail[1][0] + x;
  tail[0][1] = tail[1][1] + y;
}

void Snake::reset() {
  length = minLength;

  tail[0][0] = initX;
  tail[0][1] = initY;
  tail[1][0] = initX - 1;
  tail[1][1] = initY;
}

int Snake::xAt(int i) {
  return tail[i][0];
}

int Snake::yAt(int i) {
  return tail[i][1];
}

int Snake::x() {
  return xAt(0);
}

int Snake::y() {
  return yAt(0);
}
