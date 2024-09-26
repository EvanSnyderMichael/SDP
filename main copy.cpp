#include <FEHLCD.h>
#include <FEHImages.h>
#include <FEHUtility.h>
#include <iostream>
using namespace std;
#include <queue>
#include <stack>
#include <string.h>
using namespace std;
#include <FEHRandom.h>
#include <vector>
#include <deque>

#define MagSize 10

#define PlayerMovementCooldownTime .02

#define EnemyShotAnimCooldownTime .06
#define PlayerShotAnimCooldownTime .06
#define PlayerExplosionAnimCooldownTime .03

#define PlayerFireCooldownTime .1
#define PlayerFireCooldownCycle 30 //keep greater than 27 but really 30 for safety in state switch

#define SpeedOfPlayerShots 1

#define HUDCOLOR GHOSTWHITE
#define HUDTEXT STEELBLUE
#define HUDACCENT MAROON

class Game{
  public:
  //instance variables
  bool gameOver;
  int difficulty;
  //1=easy
  //2=medium
  //3=hard
  int score;
  int yForNextBullet;
  double timeOfGameStart;

  //constructor
  Game(int diff)
  {
    gameOver = false;
    difficulty=diff;
    difficulty=3;
    score = 0;
    yForNextBullet = 234;
    //was 194
    timeOfGameStart = TimeNow();
  }

  void initHud()
  {
    //background of hud
    LCD.SetFontColor(HUDCOLOR);
    LCD.FillRectangle(11,178,309,62);

    //write bullets header
    LCD.SetFontColor(HUDTEXT);
    LCD.WriteAt("Shots:",14,183);

    LCD.SetFontColor(HUDTEXT);
    LCD.WriteAt("9",84,183);
    //while technically proper x is 86, 84 just looks better

    //display border accent
    LCD.SetFontColor(HUDACCENT);
    LCD.FillRectangle(0,173,320,5);

  }
  void bulletsDispUpdate(int b)
  {
    LCD.SetFontColor(HUDCOLOR);
    LCD.FillRectangle(84,183,25,24);
    LCD.SetFontColor(HUDTEXT);
    LCD.WriteAt(b,84,183);
  }
};


/**
 * 
*/
class Player{
  public:
  FEHImage playerImage;
  float curX;
  float curY;
  int lives;
  double timeOfLastShot;
  double timeOfLastMovement;
  int lagShotCooldownCleared = 15;
  double timeOfLastShotAnim;
  double timeOfLastExplosionAnim;
  float direction;
  //requires at least 10 cycles before next shot

  //constructor
  Player()
  {
    timeOfLastShot = TimeNow();
    playerImage.Open("PlayerShipFEH.pic");
    timeOfLastMovement = TimeNow();
    timeOfLastShotAnim = TimeNow();
    timeOfLastExplosionAnim = TimeNow();
    curX=14;
    curY=91;
    lives=3;
    direction=0;
  }

  //returns true if movement should occur check for boundary occurs later
  bool tryMovePlayer(float newY)
    {
      if(curY!=newY)
      {
        if(TimeNow()-timeOfLastMovement>PlayerMovementCooldownTime)
        {
          if((curY+1>=newY&&curY-1<=newY))
          {
            direction =newY-curY;
          }
          else if(newY>curY)
          {
            direction = 2;
          }
          else{
            direction = -2;
          }
          timeOfLastMovement=TimeNow();
          return true;
        }
      }
      return false;
    }


  void wipePlayer()
    {
      LCD.SetFontColor(BLACK);
      LCD.FillRectangle(curX+2,curY+1,13,14);
    }


  void drawPlayer()
    {
      playerImage.Draw(curX,curY);
    }

};

class PlayerShot{
  public:
  /**
   * 0 in magazine                                                               
   * 1 in transit simple                           
   * 2 in transit near first enemy set   if > 263 maybe just do 260
   * 3 in transit near second enemy set  if > 287 second zone if >311 wall zone so trigger reload
   * 4 explosion just Triggered                    264 marks zone 1 coord of inside
   * 5 explosion occuring                          288 marks zone 2 coord of inside
   * 6 awaiting reload                             312 marks zone 3 coord of inside
  */
  int state;
  int curImage;
  FEHImage playerShotImage;
  float curX;
  float curY;

  //constructor
  PlayerShot()
  {

    state =0;
    curImage=0;
    curX = 1;
    playerShotImage.Open(shotImages[0]);

  }

  //do not independendantly draw the reload it is handled here
  void shotReloader(int newY)
  {
    state =0;
    curImage =0;
    //do curX 1 because it looks a lttle wonky with the starting image if its 2
    curX = 1;
    playerShotImage.Open(shotImages[0]);
    curY = newY;
    drawShot();
  }



  
  /**
   * wipe functions
  */
  void wipeShot()
  {
    LCD.SetFontColor(BLACK);
    LCD.FillRectangle(curX,curY,8,4);
  }
  /**
   * wipes larger area than wipe shot
  */
  void wipeExplosion()
  {
    LCD.SetFontColor(BLACK);
    LCD.FillRectangle(curX-4,curY-6,16,16);
  }

  //shot fires
  void fireShot(float plrY)
  {
    state=1;
    curX=31;
    curY=plrY+6;
    drawShot();
  }

  void drawShot()
  {
    playerShotImage.Draw(curX,curY);
  }

    void drawExplosion()
  {
    playerShotImage.Draw(curX-4,curY-6);
  }
  

  /**
   * Increments cur image to be one greater for in transit bullets and those near enemies
  */
    void switchImageStateTransit()
  {
    curImage++;
    if(curImage>5)
    {
      curImage=0;
    }
    playerShotImage.Open(shotImages[curImage]);
  }
  
    void switchImageStateExplosionFirst()
  {
    curImage=6;
    playerShotImage.Open(shotImages[curImage]);
  }
    void switchImageStateExplosion()
  {
    curImage++;
    playerShotImage.Open(shotImages[curImage]);
  }

  /**
   * HIGHLY IMPORTANT ARRAY
  */
  const char* shotImages[14]
      //= { "shot000FEH.pic","shot001FEH.pic","shot002FEH.pic","shot003FEH.pic","shot004FEH.pic","shot005FEH.pic",+
       = { "shot000FEH.pic","shot001FEH.pic","shot002FEH.pic","shot002FEH.pic","shot001FEH.pic","shot000FEH.pic",+
       "tile001FEH.pic","tile002FEH.pic","tile003FEH.pic","tile004FEH.pic","tile005FEH.pic","tile006FEH.pic","tile007FEH.pic","tile007FEH.pic"};
  //img 0-5
  //playerShots 6
  //img 6-13
  //playerShots explosions 8

  void moveShot()
  {
    curX+=SpeedOfPlayerShots;
  }
  void moveExplosion()
  {
    curX+=SpeedOfPlayerShots*2;
  }


};


int main(void)
{
  //-------------------------------------------------------------------------------ADD DIFFICULTY LATER
  Game g(2);
  g.initHud();

  Player plr;

  PlayerShot plrShots[10];
  
  //all shots in this vector are of state 0
  vector<PlayerShot*> mag;

  //magazine reload and player shots creation
  for(int i=0;i<MagSize;i++)
  {
    mag.push_back(&plrShots[i]);
    (*mag.back()).shotReloader(g.yForNextBullet);
    g.yForNextBullet-=6;
  }

  // pointer to justFiredShot
  PlayerShot* justFiredShot;
  //boolean for logic of justFiredShot false means no need to do transfer of justFiredShot
  bool justFiredShotCheck = false;
  
  //deque of simply fired shots
  deque<PlayerShot*> firedShotsSimple;

  //pointer to shot near first squad
  PlayerShot* firedShotNearFirst;
  bool isShotNearFirst = false;

  //pointer to shot near second squad
  PlayerShot* firedShotNearSecond;
  bool isShotNearSecond = false;

  //vector of just occured explosions allowing bypass of anim time checks
  vector<PlayerShot*> explodedShotsAnimBypass;
  bool explosionRequiresBypass;

  //deque of exploding shots
  deque<PlayerShot*> explodingShots;

  //vector of shots in need of reload
  vector<PlayerShot*> reloadingShots;


  


  //draw player
  plr.drawPlayer();



  //Touch variables
  float x_position, y_position;
  bool PlayerMoveGoAhead;

  while(!g.gameOver)
  {
    LCD.ClearBuffer();
    if(LCD.Touch(&x_position,&y_position))
    {
      //check if player will be moved later this function also give direction
      PlayerMoveGoAhead=plr.tryMovePlayer(y_position-8);


      if(x_position>45)
      {
        if(plr.lagShotCooldownCleared>PlayerFireCooldownCycle&&(TimeNow()-plr.timeOfLastShot)>PlayerFireCooldownTime)
        {
          if (!mag.empty())
          {
            justFiredShotCheck=true;
            justFiredShot = (mag.back());
            mag.pop_back();
            g.yForNextBullet+=6;
            plr.lagShotCooldownCleared=0;
            plr.timeOfLastShot=TimeNow();
          }
        }
        else{
          cout<<" lag shot not cleared or time not cooldown";
        }
      }
          
      LCD.SetFontColor(PINK);
      LCD.DrawPixel(x_position,y_position);
      cout<<"X: "<<x_position<<"  Y: "<<y_position<<endl;
    }
    else{
      PlayerMoveGoAhead=false;
    }
    plr.lagShotCooldownCleared++;

    //wipe statements

    //plr wipe always
    plr.wipePlayer();
        
    //wipe from mag
    if(justFiredShotCheck)
    {
      (*justFiredShot).wipeShot();
      cout<<"wiping from mag visual";
      cout<<mag.size();
    }

    //wiple simple shots
    for (PlayerShot* p : firedShotsSimple) {
    (*p).wipeShot();
    }

    //wipe shots near first
    if(isShotNearFirst)
    {
      (*firedShotNearFirst).wipeShot();
    }

    //wipe shots near second
    if(isShotNearSecond)
    {
      (*firedShotNearSecond).wipeShot();
    }

    for (PlayerShot* p : explodingShots) {
    (*p).wipeExplosion();
    }





    //move statements


    if(PlayerMoveGoAhead)
    {
      plr.curY+=plr.direction;
      if(plr.curY>157)
      {
        plr.curY=157;
      }
      else if(plr.curY<0)
      {
        plr.curY=0;
      }
    }

    //move for shots not tied to a constraint
    for (PlayerShot* p : firedShotsSimple) {
    (*p).moveShot();
    }

    //move shots near second
    if(isShotNearSecond)
    {
      (*firedShotNearSecond).moveShot();
    }

    //move shots near first
    if(isShotNearFirst)
    {
      (*firedShotNearFirst).moveShot();
    }
    
    //exploding shots
    //for (PlayerShot* p : explodingShots) {
    //(*p).moveExplosion();
    //}

    for (PlayerShot* p : explodedShotsAnimBypass) {
    (*p).moveExplosion();
    }


    //checks

    //check for fired shots
    if(!firedShotsSimple.empty())
    {
      if((*firedShotsSimple.front()).curX>260)
      {
        firedShotNearFirst=firedShotsSimple.front();
        isShotNearFirst=true;
        firedShotsSimple.pop_front();
        (*firedShotNearFirst).state=2;
    }
    }

  
    //check near firsts
    if(isShotNearFirst)
    {
      if((*firedShotNearFirst).curY<50)
      {
        isShotNearFirst=false;
        (*firedShotNearFirst).state=4;
        explosionRequiresBypass=true;
        explodedShotsAnimBypass.push_back(firedShotNearFirst);
      }
      else if((*firedShotNearFirst).curX>287)
      {
        isShotNearFirst=false;
        isShotNearSecond=true;
        firedShotNearSecond=firedShotNearFirst;
        (*firedShotNearFirst).state=3;
      }
    }
    

    //process leaving second zone indication for reload
    if(isShotNearSecond)
    {
      if(((*firedShotNearSecond).curY>150))
      {
        isShotNearSecond=false;
        (*firedShotNearSecond).state=4;
        explosionRequiresBypass=true;
        explodedShotsAnimBypass.push_back(firedShotNearSecond);
      }
      else if((*firedShotNearSecond).curX>311)
        {
          isShotNearSecond=false;
          reloadingShots.push_back(firedShotNearSecond);
          (*firedShotNearSecond).state=6;
        }
    }

    if(!explodingShots.empty())
    {
      if((*explodingShots.front()).curImage==13)
      {
        reloadingShots.push_back(explodingShots.front());
        explodingShots.pop_front();
        (*reloadingShots.back()).state=6;
        if(!explodingShots.empty())
        {
          if((*explodingShots.front()).curImage==13)
          {
            reloadingShots.push_back(explodingShots.front());
            explodingShots.pop_front();
            (*reloadingShots.back()).state=6;
            cout<<" Double trigger reload explosion";
          }
        }
      }
    }

    //anim statements

    if(TimeNow()-plr.timeOfLastShotAnim>PlayerShotAnimCooldownTime)
    {
      //anim statements
      for (PlayerShot* p : firedShotsSimple) {
      (*p).switchImageStateTransit();
      }
      if(isShotNearFirst)
        {
          (*firedShotNearFirst).switchImageStateTransit();
        }
      if(isShotNearSecond)
        {
          (*firedShotNearSecond).switchImageStateTransit();
        }
      plr.timeOfLastShotAnim=TimeNow();

  
       //no need to anim just fired shot
    }
    for(PlayerShot* p : explodedShotsAnimBypass) {
      (*p).switchImageStateExplosionFirst();
    }
    if(TimeNow()-plr.timeOfLastExplosionAnim>PlayerExplosionAnimCooldownTime)
    {
    for (PlayerShot* p : explodingShots) {
    (*p).switchImageStateExplosion();
    (*p).moveExplosion();
    plr.timeOfLastExplosionAnim=TimeNow();
    }
    }




    //draw statements
    //  enemy>>enemy bullet>>player>>playerbullet
 
    
    //in transit draw
    for (PlayerShot* p : firedShotsSimple) {
    (*p).drawShot();
    }
    
    //just fired add it to in transit
    if(justFiredShotCheck)
    {
      (*justFiredShot).fireShot(plr.curY);
      firedShotsSimple.push_back(justFiredShot);
      justFiredShotCheck=false;
    }

    if(isShotNearFirst)
    {
      (*firedShotNearFirst).drawShot();
    }

    if(isShotNearSecond)
    {
      (*firedShotNearSecond).drawShot();
    }

    //exploding 
    for (PlayerShot* p : explodingShots) {
    (*p).drawExplosion();
    }

    //explodingbypass
    for (PlayerShot* p : explodedShotsAnimBypass) {
    (*p).drawExplosion();
    explosionRequiresBypass=false;
    explodingShots.push_back(p);
    }
    for(int i=0;i<explodedShotsAnimBypass.size();i++)
    {
      explodedShotsAnimBypass.pop_back();
    }
    

    if(!reloadingShots.empty())
    {
      mag.push_back(reloadingShots.back());
      (*mag.back()).shotReloader(g.yForNextBullet);
      g.yForNextBullet-=6;
      reloadingShots.pop_back();
    }

    plr.drawPlayer();

    g.bulletsDispUpdate(mag.size());
    LCD.Update();
    
  }
}
