#include <iostream>

#include "wiringPi.h"
#include "wiringPiSPI.h"

#include "tft_st7735.h"
#include "tft_manager.h"
#include "tft_field.h"

#include "mcp3008Spi.h"

using namespace std;

enum direction {UP, DOWN, LEFT, RIGHT};

struct position {
    int x,y;
};


class field_cls {
    static const unsigned char height;
    static const unsigned char width;
    static const unsigned char mn;
    static const unsigned char dis;
    static const unsigned char rad;            
    char ** field;
public:
    field_cls() {
        field = new char*[field_cls::height];
        for(int c = 0; c < field_cls::height; ++c) {
            field[c] = new char[field_cls::width];
        }
    }
    ~field_cls() {
        for(int c = 0; c < field_cls::height; ++c) {
            delete[] field[c];
        }
        delete[] field;
    }

    void print() {
        for(int c = 0; c < height; ++c) {
            for(int r = 0; r < width; ++r) {
                cout << field[c][r];
            }
            cout << endl;
        }
    }

    void clear() {
        for(int c = 0; c < height; ++c) {
            for(int r = 0; r < width; ++r) {
                if ((c==0)||(r==0)) {field[c][r] = '*';}
                else if ((c==height-1)||(r==width-1)) {field[c][r] = '*';}
                else {field[c][r] = ' ';}
            }
        }
    }

    int get_width() const {return width;}
    int get_height() const {return height;}


    void draw(int y, int x, char what) {
        field[y][x] = what;
    }
    char getcha(int y, int x) {
        return field[y][x];
    }

} field;


class food_cls {
    char symbol;
    position pos;
public:
    food_cls(): symbol('X'), pos() {
        pos.x = pos.y = -1;
    }

    void set_pos(int x, int y) {
        pos.x = x;
        pos.y = y;
    }

    void reposition(const field_cls & field) {
        pos.x = rand() % (field.get_width()-1);
        pos.y = rand() % (field.get_height()-1);
    }

    int get_x() const {return pos.x;}
    int get_y() const {return pos.y;}
    char get_symbol() const {return symbol;}
} food;

class snake_cls {
    direction dir;  
    char symbol, head_symbol;
    position pos[100];
    position & head;
    int speed;
    int size_;
    bool can_turn;
    int end_game;
public:
    snake_cls(int x, int y):
        dir(RIGHT),
        symbol('#'), head_symbol('@'), pos(),
        head(pos[0]),
        speed(1), size_(1),
        can_turn(true),end_game(0)
    {
        pos[0].x = x;
        pos[0].y = y;
    }
    void clear(int x,int y) {
        pos[0].x = x;
        pos[0].y = y;
		size_=1;
		end_game=0;
		dir=RIGHT;
	
	}
    bool check_food(const food_cls & food) {
        if(food.get_x() == head.x && food.get_y() == head.y) {
            size_ += 1;
            return true;
        }
        return false;
    }
    direction get_dir() {
        return dir;                
    }
    void set_dir(direction d) {
        dir=d;                
    }

    void move(const field_cls & field) {
        position next = {0, 0};
        switch(dir) {
        case UP:
            next.y = -speed;
            break;
        case DOWN:
            next.y = speed;
            break;
        case LEFT:
            next.x = -speed;
            break;
        case RIGHT:
            next.x = speed;
        }
        for(int c = size_ - 1; c > 0; --c) {
            pos[c] = pos[c-1];
        }
        head.x += next.x;
        head.y += next.y;

        if(head.x <= 0 || head.y <= 0 || head.x >= (field.get_width()-1) || head.y >= (field.get_height())-1) {
            end_game=1;
        }
        for(int c = size_ - 1; c > 0; --c) {
            if ((pos[0].x == pos[c].x)&&(pos[0].y==pos[c].y)){
                end_game=1;
            }
        }

    }

    void draw(field_cls & field) {
        for(int c = 0; c < size_; ++c) {
            if(c == 0) {
                field.draw(pos[c].y, pos[c].x, head_symbol);
            } else {
                field.draw(pos[c].y, pos[c].x, symbol);
            }
        }
    }

    int get_x() const { return head.x; }
    int get_y() const { return head.y; }
    char get_symbol() const { return symbol; }
    int check_end(){ return end_game; }
    int get_size() { return size_; }
} snake(1, 1);



const unsigned char field_cls::height = 16;
const unsigned char field_cls::width = 20;

unsigned char heightGr = 160;
unsigned char widthGr  = 128;

const unsigned char field_cls::mn=8;
const unsigned char field_cls::dis=7;
const unsigned char field_cls::rad=4;

const unsigned char bd=2;
const unsigned char mn1=8;
const unsigned char dis1=7;

direction dir;  

void print1(TFT_ST7735& tft);
void clearScr(TFT_ST7735 &tft);
direction chk_dir();
int main() {
    field.clear();
    food.set_pos(5, 5);
    int i=0;
    int score=0;
    char ch1, ch2;

    TFT_ST7735 tft = *new TFT_ST7735(0, 24, 25, 32000000);
  
    wiringPiSetupGpio();      // initialize wiringPi and wiringPiGpio

    tft.commonInit();         // initialize SPI and reset display
    tft.initR();              // initialize display
    tft.setRotation(true);
    tft.setBackground(TFT_BLACK);
    tft.clearScreen();        // reset Display

    while(1){
    clearScr(tft);

while(dir != DOWN){
tft.drawString(12,(TFT_height/2)-10,"go RIGHT to Start",TFT_WHITE,1);
dir=chk_dir();
}    

    while(snake.check_end()==0) {
        field.clear();
        clearScr(tft);
        dir=chk_dir();
        snake.set_dir(dir);

        snake.move(field);
        snake.draw(field);
        field.draw(food.get_y(), food.get_x(), food.get_symbol());


        if(snake.check_food(food)) {
            food.reposition(field);
        }

        print1(tft);
        delay(500);
              
        if (snake.check_end()==1) {
	    score = snake.get_size();
	    ch1 = score/10 + 48;
	    ch2 = score%10 + 48;
            cout << "Game Over" <<endl;
	    clearScr(tft);
	    tft.drawString(12,(TFT_height/2)-10,"Game Over",TFT_WHITE,2);

            tft.setCursor(25,110);
            if (ch1 != '0') {
       	       tft.drawChar((ch1), TFT_WHITE, 3);
            }
            tft.drawChar((ch2), TFT_WHITE, 3);
            tft.drawChar(('0'), TFT_WHITE, 3);
            tft.drawChar(('0'), TFT_WHITE, 3);

	    delay(5000); //shibaniq fucking adc i swear
        }
    }

    snake.clear(1,1);
    }
    return 0;
}

void print1(TFT_ST7735& tft){
   int width=field.get_width();
   int height=field.get_height();
   char ch;  
   unsigned char x0, y0;
   for(int c = 0; c < height; ++c) {
      for(int r = 0; r < width; ++r) {
          ch=field.getcha(c,r);
          cout<<ch;
	  x0=(char) mn1*c;
	  y0=(char) mn1*r;

          if (ch=='@'){
             tft.drawRect(x0,y0,7,7,TFT_YELLOW);
          }  
          else if (ch=='#'){
             tft.drawRect(x0,y0,7,7,TFT_GREEN);
          }  
          else if (ch=='X'){
             tft.fillRect(x0,y0,7,7,TFT_BLUE);
	  }  
          
      }
      cout<<endl;
   }
}       
          

void clearScr(TFT_ST7735 &tft){
        
        tft.setBackground(TFT_BLACK);
        tft.clearScreen();        // reset Display

        tft.drawLine(bd,bd,widthGr-bd,bd, TFT_RED);
        tft.drawLine(bd,bd,bd,heightGr-bd, TFT_RED);
        tft.drawLine(bd,heightGr-bd,widthGr-bd,heightGr-bd, TFT_RED);
        tft.drawLine(widthGr-bd,heightGr-bd,widthGr-bd,bd, TFT_RED);
}

direction chk_dir() {

	mcp3008Spi a2d("/dev/spidev0.1", SPI_MODE_1, 1000000, 8);

        int xVal = 0;
        int yVal = 0;
        int Channel = 0;
        unsigned char data[3];

        Channel=2;
        data[0] = 1;  //  first byte transmitted -> start bit
        data[1] = 0b10000000 |( ((Channel & 7) << 4)); // second byte transmitted -> (SGL/DIF = 1, D2=D1=D0=0)
        data[2] = 0; // third byte transmitted....don't care

        a2d.spiWriteRead(data, sizeof(data) );

        yVal = 0;
        yVal = (data[1]<< 8) & 0b1100000000; //merge data[1] & data[2] to get result
        yVal |=  (data[2] & 0xff);
        delay(1);

        Channel=1;
        data[0] = 1;  //  first byte transmitted -> start bit
        data[1] = 0b10000000 |( ((Channel & 7) << 4)); // second byte transmitted -> (SGL/DIF = 1, D2=D1=D0=0)
        data[2] = 0; // third byte transmitted....don't care

        a2d.spiWriteRead(data, sizeof(data) );

        xVal = 0;
        xVal = (data[1]<< 8) & 0b1100000000; //merge data[1] & data[2] to get result
        xVal |=  (data[2] & 0xff);
        delay(1);
          
	//cout << yVal << endl;
	//cout << xVal << endl;

        int tmdir=0;
        if (xVal<100){
           tmdir=3;
        }   
        else if (xVal>900){
           tmdir=4;        
        }          
        else if (yVal<100){
           tmdir=1;
        }   
        else if (yVal>900){
           tmdir=2;        
        }          
        
        direction dir; 
        
        dir=snake.get_dir();
        if (tmdir==0){
            return dir;
        } 
        if((tmdir==3) && dir != DOWN) {
            dir = UP;
        }
        if((tmdir==4)  && dir != UP) {
            dir = DOWN;
        }
        if((tmdir==1)  && dir != RIGHT) {
            dir = LEFT;
        }
        if((tmdir==2)  && dir != LEFT) {
            dir = RIGHT;
        }
        return dir;
}