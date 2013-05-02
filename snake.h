#ifndef SNAKE_H_
#define SNAKE_H_

class Snake {
 public:
  Snake(int maxLength, int x, int y);
  Snake();
  ~Snake();

  void eat();
  bool full();
  bool hitsItself(int nextX, int nextY);
  bool moveTo(int x, int y);
  void reset();
  int xAt(int i);
  int yAt(int i);
  int x();
  int y();

  int initX;
  int initY;
  int length;
  int maxLength;

  const int minLength = 2;
  double ** tail;

 private:
  void init();
};

#endif // SNAKE_H_
