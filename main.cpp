#include <FEHLCD.h>
#include <FEHImages.h>
#include <FEHUtility.h>
#include <iostream>
#include <string.h>
using namespace std;

#define shotCooldownMS .5
#define playerMovementCooldownMS .01

class Game{
  public:
  bool GameOver;
  int lives, difficulty;
  
  

  Game(int diff)
  {
    GameOver = false;
    lives = 3;
    difficulty = diff;
    //difficulty 1 = easy
    //difficulty 2 = medium
    //difficulty 3 = hard
  }

  void EndGame()
  {
    
  }

  void checkGameOver()
  {

  }
};


class Enemy {
  public:
    float curX,curY;
    FEHImage enemyImage;
};

class PlayerShot
{
  public:
    int speed,state,curImage;
    float curX,curY;
    FEHImage playerShotImage;
    //state 0 bullet in inventory
    //state 1 bullet is fired
    //state 2 bullet is collided with enemy
    //state 3 bullet is collided with wall
    //state 4 bullet is awaiting reload
    //state 5 bullet is being reloaded
    PlayerShot()
    {
      state =0;
      curImage =0;
      playerShotImage.Open("PlayerShotFEH.pic");
    }

    void switchImage(int i)
    {
      switch(i){
        case 0:
        playerShotImage.Open("PlayerShotFEH.pic");
        break;
        case 1:
        playerShotImage.Open("tile001FEH.pic");
        break;
        case 2:
        playerShotImage.Open("tile002FEH.pic");
        break;
        case 3:
        playerShotImage.Open("tile003FEH.pic");
        break;
        case 4:
        playerShotImage.Open("tile004FEH.pic");
        break;
        case 5:
        playerShotImage.Open("tile005FEH.pic");
        break;
        case 6: 
        playerShotImage.Open("tile006FEH.pic");
        break;
        case 7:
        playerShotImage.Open("tile007FEH.pic");
        break;
      }
      
    }

    void fireShot(float plrX,float plrY)
    {
      state=1;
      speed=2;
      wipeShot();
      curX=plrX+17;
      curY=plrY+7;
      playerShotImage.Draw(curX,curY);
      
    }
    
    bool reloadBullet(int br)
    {
      if(state==4)
      {
      curImage=0;
      playerShotImage.Close();
      switchImage(curImage);
      cout<<"Bullets Remaining at reload: "<<br<<endl;
      curY=(195-br*6);
      cout<<"Y set as:"<<curY<<endl;
      curX=2;
      playerShotImage.Draw(curX,curY);
      state =0;
      return true;
      }
      return false;
    }

    void wipeShot()
    {
      LCD.SetFontColor(BLACK);
      LCD.FillRectangle(curX,curY,8,4);
    }
     void wipeExplosion()
    {
      LCD.SetFontColor(BLACK);
      LCD.FillRectangle(curX,curY,16,16);
    }

    static void alignShotsInInventory(PlayerShot p[])
    {
      int yMag = 141;
      for(int i = 0; i < 10;i++)
      {
        p[i].curY=yMag;
        p[i].curX=2;
        p[i].playerShotImage.Draw(p[i].curX,p[i].curY);
        yMag+=6;
      }
    }

    //returns -1 if no valid shot or first valid shot in array
    static int findValidShotIndex(PlayerShot p[])
    {
      int bestIndex =-1;
      int bestY=220;
      for(int i =0;i<10;i++)
      {
        if (p[i].state==0)
        {
          if(p[i].curY<bestY)
          {
            bestIndex =i;
            bestY=p[i].curY;
          }
        }
      }
      return bestIndex;
    }

    void moveShots()
    {
      if(state==1)
        {
          //if collided with wall
          if(curX>224)
          {
            state=3;
            speed=0;
          }
          //if collided with enemy
          else if(curX>180)
          {
            state=2;
            wipeShot();
          }
          else{
            wipeShot();
          }
          curX+=speed;
          playerShotImage.Draw(curX,curY);
        }
      else if(state==2)
        {
          //explosion cleared
            if(curImage==7)
            {
              curImage=0;
              speed =0;
              wipeExplosion();
              state=4;
            }
            else{
              if(curImage==0)
              {
                wipeShot();
                curY-=6;
                curX-=4;
              }
              else{
                wipeExplosion();
              }
              curImage++;
              //if bullet touch wall
              if(curX>224)
              {
                state=4;
                speed=0;
              }
              curX+=speed;
              playerShotImage.Close();
              switchImage(curImage);
              playerShotImage.Draw(curX,curY);
            }
        }
      else if(state ==3)
        {
          wipeExplosion();
          //collision with wall
          state =4;
          speed =0;
        }
    }
};

class Player {       
  public:             
    float curX,curY;
    FEHImage playerImage;
    double timeOfLastShot,timeOfLastMovement;

    //constructor same as initizlize player will be changed later
    Player()
    {
      timeOfLastShot = TimeNow();
      playerImage.Open("PlayerShipFEH.pic");
      timeOfLastMovement = TimeNow();
      curX=10;
      curY=91;
      playerImage.Draw(curX,curY);
    }

    void initializePlayer()
    {
      int timeOfLastShot = TimeNow();
      int timeOfLastMovement = TimeNow();
      playerImage.Open("PlayerShipFEH.pic");
      curX=10;
      curY=92;
      playerImage.Draw(curX,curY);
    }



    //move player returns true if shot should also be taken
    bool tryMovePlayer(float newX, float newY)
    {
      if(!(curY+1>=newY&&curY-1<=newY))
      {
      if(TimeNow()-timeOfLastMovement>playerMovementCooldownMS)
      {
        float yToDraw=curY;
        wipePlayer();
        if(newY>curY)
        {
          yToDraw+=1;
        }
        else{
          yToDraw-=1;
        }
        if(yToDraw<0)
        {
          yToDraw=0;
        }
        if(yToDraw>184)
        {
          yToDraw=184;
        }
        playerImage.Draw(curX,yToDraw);
        LCD.Update();
        timeOfLastMovement=TimeNow();
        curX=curX;
        curY=yToDraw;
      }
      }
      if(newX>80)
      {
        return true;
      }
      return false;

    }

    void wipePlayer()
    {
      LCD.SetFontColor(BLACK);
      LCD.FillRectangle(curX+2,curY+1,13,14);
    }
};

int main(void)
{
    Player plr;
    plr.initializePlayer();


    PlayerShot plrShots[10];
    PlayerShot::alignShotsInInventory(plrShots);
    int bulletsRemaining =10;

    Game currentGame(2);

    LCD.SetFontColor(GRAY);
    LCD.FillRectangle(0,201,319,39);

    float x_position, y_position;
    int loopCount = 0;
    float timeLast = TimeNow();
    double loopAvg =0;
    while(loopCount<2000) 
    {
      cout<<TimeNow()-timeLast<<" Time"<<endl;
      loopAvg+=TimeNow()-timeLast;
      timeLast=TimeNow();
      cout<<loopCount++<<endl;
        if(LCD.Touch(&x_position,&y_position))
        {
          //will return true if on right side which calls the fire shot function
          if(plr.tryMovePlayer(x_position,y_position-8))
          {   
            if((TimeNow()-plr.timeOfLastShot)>shotCooldownMS)
            {
              int validShot=PlayerShot::findValidShotIndex(plrShots);
              if(validShot!=-1)
              {
                plr.timeOfLastShot=TimeNow();
                plrShots[validShot].fireShot(plr.curX,plr.curY);
                bulletsRemaining--;
              }
            }
          }
        }
        for(int i=0;i<10;i++)
        {
          if(plrShots[i].state!=0)
          {
          plrShots[i].moveShots();
          {
        }
        for(int i=0;i<10;i++)
        {
          if(plrShots[i].reloadBullet(bulletsRemaining))
            {
            bulletsRemaining++;
            }
          }
        
        LCD.Update(); //Never quit 
      }
    } 
}
loopAvg/=loopCount;
cout<<loopAvg<<"AVG"<<endl;
}
