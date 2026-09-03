// A "Timber-man" clone with a fantasy RPG-esque aesthetic called "Tower!"

// List of included external libraries:
#include <SFML/Graphics.hpp>        // Graphics library
#include <SFML/Audio.hpp>           // Audio library

// List of included classes:
#include <iostream>                 // for testing
#include <string>                   // for text
#include <sstream>                  // for text

// List of namespaces:
using namespace sf;         // Make code using SFML quicker to type
using namespace std;

// List of prototypes:
void updateTurrets(int seed);       

//List of global objects (outside of main = GLOBAL SCOPE):
const int NUM_TURRETS = 6;              // 6 possible positions for turrets for the player to dodge   
Sprite turrets[NUM_TURRETS];            // Array of turret sprites
enum class side { LEFT, RIGHT, NONE };  // Which side of the tower is the player/turrets? Turrets start at NONE, player starts at LEFT
side turretPositions[NUM_TURRETS];      // Array of turret positions

int main()
{
    // Create a VideoMode object for screen size
    VideoMode vm(1920, 1080);
    // Create a window
    RenderWindow window(vm, "Tower!", Style::Default); 

    // Prepare the background
    Texture textureBackground;
    textureBackground.loadFromFile("assets/image/background.png");
    Sprite spriteBackground;
    spriteBackground.setTexture(textureBackground);
    spriteBackground.setPosition(0,0);

    // Prepare the tower
    Texture textureTower;
    textureTower.loadFromFile("assets/image/tower.png");
    Sprite spriteTower;
    spriteTower.setTexture(textureTower);
    float spriteTowerWidth = spriteTower.getLocalBounds().width;
    const float TOWER_HOR_POS{(1920 / 2) - (spriteTowerWidth / 2)};     // Set the tower to be perfectly central horizontally
    const float TOWER_VER_POS{0.0f};                                    // Tower is drawn from the top of the screen
    spriteTower.setPosition(TOWER_HOR_POS, TOWER_VER_POS);

    // Prepare the flying bird
    Texture textureBird;
    textureBird.loadFromFile("assets/image/bird.png");
    Sprite spriteBird;
    spriteBird.setTexture(textureBird);
    spriteBird.setPosition(-200, 600);          // Bird flies across the screen in close to the player to distract              
    bool birdActive = false;                    // Is the bird currently moving on screen? Initially no
    float birdSpeed = (0.0f);

    // Prepare 3 visually identical clouds 
    Texture textureCloud;
    textureCloud.loadFromFile("assets/image/cloud.png");
    const int NUM_CLOUDS = 3;              // 3 clouds to float by in the background  
    Sprite clouds[NUM_CLOUDS];        
    bool cloudsActive[NUM_CLOUDS];
    float cloudsSpeed[NUM_CLOUDS];
    float cloudsHeight[NUM_CLOUDS];
    for (int i = 0; i <NUM_CLOUDS; i++)
    {
        clouds[i].setTexture(textureCloud);
        clouds[i].setPosition(-300, (i * 150) + 50);
        cloudsActive[i] = false;    
        cloudsSpeed[i] = (0.0f);
    }  

    // Prepare the character
    Texture textureCharacter;
    textureCharacter.loadFromFile("assets/image/character.png");
    Sprite spriteCharacter;
    spriteCharacter.setTexture(textureCharacter);
    side characterSide = side::LEFT;                // Character starts on the left
    spriteCharacter.setPosition(590, 805);          // CHaracter appears near base of tower

    // Prepare the headstone for game over
    Texture textureGrave;
    textureGrave.loadFromFile("assets/image/rip.png");
    Sprite spriteGrave;
    spriteGrave.setTexture(textureGrave);
    spriteGrave.setPosition(575, -500);

    // Prepare the axe weapon
    Texture textureWeapon;
    textureWeapon.loadFromFile("assets/image/axe.png");
    Sprite spriteWeapon;
    spriteWeapon.setTexture(textureWeapon);
    spriteWeapon.setPosition(672, 800);
    float const WEAPON_POS_LEFT = 672.0f;
    float const WEAPON_POS_RIGHT = 1250.0f;   

    // Prepare the destroyed tower section to go flying
    Texture textureSection;
    textureSection.loadFromFile("assets/image/section.png");
    Sprite spriteSection;
    spriteSection.setTexture(textureSection);
    spriteSection.setPosition(745, 795);    // overlap the section with the bottom tier of the tower
    bool sectionActive = false;             // Is the section damaged and flying? Initially no
    float sectionSpeedX = 3000;             // Flying x speed
    float sectionSpeedY = -1000;            // Flying y speed

    // Prepare the sounds and music
    SoundBuffer weaponBuffer;
    SoundBuffer graveBuffer;
    weaponBuffer.loadFromFile("assets/audio/hit.wav");
    graveBuffer.loadFromFile("assets/audio/rip.wav");
    Sound weapon;   
    Sound grave;                                       
    weapon.setBuffer(weaponBuffer);         // Sound for striking the tower
    grave.setBuffer(graveBuffer);           // Sound for being struck by turret/being timed-out
    Music backgroundMusic;                  // Song looping in the background while game is not paused
    backgroundMusic.openFromFile("assets/audio/background.mp3");
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(25.0f);

    // Prepare HUD text:
    int score{0};           // Player's score
    Text messageText;       // Initially instruct player to press Enter. Change to "Game Over" or "KO" when game ends
    Text scoreText;         // Display "Score = [player's score]"
    Font font;              // Font used for all text
    font.loadFromFile("assets/font/BoldPixels.ttf");

    messageText.setFont(font);
    messageText.setString("Press Enter to start!");
    messageText.setCharacterSize(120);
    messageText.setFillColor(Color::White);
    messageText.setOutlineColor(Color::Black);
    messageText.setOutlineThickness(5.0f);
    // Centre the messageText in the middle of the screen:
    FloatRect textRect = messageText.getLocalBounds();  
    messageText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);    
    messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

    scoreText.setFont(font);
    scoreText.setString("Score = 0");
    scoreText.setCharacterSize(120);
    scoreText.setFillColor(Color::White);
    scoreText.setOutlineColor(Color::Black);
    scoreText.setOutlineThickness(5.0f);
    scoreText.setPosition(30, -10);    

    // Prepare the tower's turrets
    Texture textureTurret;
    textureTurret.loadFromFile("assets/image/turret.png");
    for (int i = 0; i < NUM_TURRETS; i++)
    {
        turrets[i].setTexture(textureTurret);
        turrets[i].setPosition(-2000, -2000);
    }
    float turretOriginX = (turrets[0].getLocalBounds().width / 2.0f);
    float turretOriginY = (turrets[0].getLocalBounds().height / 2.0f);
    for (int i = 0; i < NUM_TURRETS; i++)
    {
        turrets[i].setOrigin(turretOriginX, turretOriginY);
    }
    float turretLeftPosX = (TOWER_HOR_POS - turretOriginX + 200.0f);
    float turretRightPosX = (TOWER_HOR_POS + turretOriginX + spriteTowerWidth - 200.0f);

    // Timing:
    Clock clock;                    // Clock for timing (used for framerate/animation, and time-bar)
    float timeRemaining = 6.0f;     // For use on the time bar

    // Prepare the time bar
    RectangleShape timeBar;
    float tbStartWidth = 400;
    float tbHeight = 60;
    timeBar.setSize(Vector2f(tbStartWidth, tbHeight));  
    timeBar.setFillColor(Color::Red);
    timeBar.setPosition((1920 / 2) - (tbStartWidth / 2), 1000);     // Position time bar below the turret
    float tbWidthPerSecond = tbStartWidth / timeRemaining;          // The time bar will shrink as time goes on

    bool paused = true;             // Game starts in paused state

    bool keyboardInput = false;     // Is a key being pressed? Initially, no

    while (window.isOpen())
    {
        /*
        Player's inputs here
        */

        // Event for detecting key presses in game
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyReleased && !paused)
            {
                keyboardInput = false;  // key released = no input
                spriteWeapon.setPosition(2500, spriteWeapon.getPosition().y);   // Hide the axe on key release
            }
        }

        // Unpause the initial paused state and start a new game
        if (paused == true && Keyboard::isKeyPressed(Keyboard::Enter))
        {
            paused = false;

            // Reset time and score
            timeRemaining = 6.0f;
            score = 0;

            // Set all turrets except the top one (position 0) to NONE 
            for (int i = 1; i < NUM_TURRETS; i++)
            {
                turretPositions[i] = side::NONE;
            }
            spriteGrave.setPosition(575, -500);     // Hide the gravestone as the player is not dead (yet)
            spriteCharacter.setPosition(590, 805);  // Move the player into the LEFT position:

            keyboardInput = false;
        }

        if (!keyboardInput)
        {
            if(Keyboard::isKeyPressed(Keyboard::Right))
            {
                characterSide = side::RIGHT;            // Move character to the right
                score++;                                // Increase score
                timeRemaining += (2 / score) + 0.16;    // Increase time remaining
                spriteWeapon.setPosition(
                    WEAPON_POS_RIGHT, 
                    spriteWeapon.getPosition().y);      //Move the axe to the right with the character
                spriteCharacter.setPosition(1330, 805); // CHECK X VALUE LATER
                spriteCharacter.setScale(-1, 1);        // Flip the sprite around
                spriteWeapon.setScale(-1, 1);           // Flip the axe around
                updateTurrets(score);                   // Move the turrets down

                // Prepare the section for flight
                spriteSection.setPosition(745, 795);  
                spriteSection.setRotation(0.0f);  
                sectionSpeedX = -3000;
                sectionActive = true;

                weapon.play();          // play the sound
                keyboardInput = true;   // Is a key being pressed? Yes
            }

            if(Keyboard::isKeyPressed(Keyboard::Left))
            {
                characterSide = side::LEFT;             // Move character to the left
                score++;                                // Increase score
                timeRemaining += (2 / score) + 0.16;    // Increase time remaining
                spriteWeapon.setPosition(WEAPON_POS_LEFT, spriteWeapon.getPosition().y);   //Move the axe to the right with the character
                spriteCharacter.setPosition(590, 805);  // Correct Left position
                spriteCharacter.setScale(1, 1);         // Flip character correct way around
                spriteWeapon.setScale(1, 1);            // Flip axe correct way around
                updateTurrets(score);                   // Move the turrets down

                // Prepare the section for flight
                spriteSection.setPosition(745, 795);  
                spriteSection.setRotation(0.0f);  
                sectionSpeedX = 3000;
                sectionActive = true;

                weapon.play();          // play the sound
                keyboardInput = true;   // Is a key being pressed? Yes
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Escape))   // Close the game
        {
            window.close();
        }

        /*
        Update the scene
        */ 
        
        // Reset the clock every frame to time how long a frame takes
        // Originally, the following 'dt' line was included in the below "if (!paused)" block, but encountered an issue with the timeBar (contd.)
        // not correctly resetting after a Game Over or K.O (i.e. while paused). This was remedied by moving the 'dt' initialisation to its current position.
        Time dt = clock.restart();      // dt = "delta time", the time between two updates

        if (!paused)
        {
            // Guarding "music.play()" with the following status check prevents any  restarts of the music file
            if (backgroundMusic.getStatus() != SoundSource::Status::Playing)
            {
                backgroundMusic.play();     // also ensures music is playing when game not paused
            }

            // Subtract from the amount of time remaining on the time-bar by however long is between each frame update
            timeRemaining -= dt.asSeconds();

            // Adjust size of time-bar
            timeBar.setSize(Vector2f(tbWidthPerSecond * timeRemaining, tbHeight));

            // Pause the game when time runs out
            if (timeRemaining <= 0.0f)
            {
                paused = true;
                backgroundMusic.pause();    // pause the music
                grave.play();               // play the RIP sound effect
                
                // Change game message text
                messageText.setString("Game Over!");
                
                // Update the text box's origin to its new size's centre point
                textRect = messageText.getLocalBounds();
                messageText.setOrigin(textRect.left + (textRect.width / 2.0f), textRect.top + (textRect.height / 2.0f));
                messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);
            }
            
            // Bird Setup
            if (!birdActive)
            {
                // Bird speed
                srand((int)time(0));                // Seed a random number generator using the time
                birdSpeed = (rand() % 250) + 200;   // %remainder after dividing by 250 must be between 1 and 249, + 200 is between 201 and 449
                
                // Bird height
                srand((int)time(0));                        // Seed a random number again 
                float birdHeight = (rand() % 300) + 500;    // assign number between 501 and 799 to bird's height
                spriteBird.setPosition(2000, birdHeight);
                birdActive = true;
            }
            // Bird Animation
            else
            {
                spriteBird.setPosition(
                    spriteBird.getPosition().x - (birdSpeed * dt.asSeconds()), 
                    spriteBird.getPosition().y
                );
                if (spriteBird.getPosition().x < -500)  // Reset the bird if it has gone off-screen
                {
                    birdActive = false;
                }
            }

            // Clouds Management
            for (int i = 0; i < NUM_CLOUDS; i++)
            {
                // Clouds Setup
                if (!cloudsActive[i])
                {
                    srand((int)time(0) * (10 * (i + 1)));               // random number seed (*10 to guarantee a number unique from other animations)
                    cloudsSpeed[i] = (rand() % 100) + 50 + (25 * i);    // guaranteed speeds of between 51-151, 76-176, and 101-201 respectively

                    srand((int)time(0) * (10 * (i + 1)));
                    cloudsHeight[i] = (rand() % 100) + (150 * i);
                    clouds[i].setPosition(-400, cloudsHeight[i]);
                    cloudsActive[i] = true;
                }
                // Clouds Animation
                else
                {
                    clouds[i].setPosition(
                        clouds[i].getPosition().x + (cloudsSpeed[i] * dt.asSeconds()),
                        clouds[i].getPosition().y
                    );
                    if (clouds[i].getPosition().x > (2200 - (100 * i)))
                    {
                        cloudsActive[i] = false;
                    }
                }
            }

            // Update the score display
            stringstream ss;
            ss << "Score = " << score;
            scoreText.setString(ss.str());

            // Update the Turret Sprites
            for (int i = 0; i < NUM_TURRETS; i++)
            {
                float turretPosY = i * ((spriteTower.getLocalBounds().height - 25.0f) / NUM_TURRETS) + 100.0f;
                // the above 25 and 100 are adjustments to look more natural on the textures used. 
                if (turretPositions[i] == side::LEFT)
                {
                    turrets[i].setPosition(turretLeftPosX, turretPosY);     // Move the sprite to the left
                    turrets[i].setRotation(180);    // Flip it around
                }
                else if (turretPositions[i] == side::RIGHT)
                {
                    turrets[i].setPosition(turretRightPosX, turretPosY);    // Move the sprite right
                    turrets[i].setRotation(0);    // Set it to normal
                }
                else
                {
                    turrets[i].setPosition(-2000, turretPosY);      // Hide the turret
                }
            }

            // Handle the tower sections flying off when struck
            if (sectionActive)
            {
                spriteSection.setPosition(spriteSection.getPosition().x + (sectionSpeedX * dt.asSeconds()), 
                spriteSection.getPosition().y + (sectionSpeedY * dt.asSeconds()));
                if (characterSide == side::LEFT)
                {
                    spriteSection.setRotation(spriteSection.getRotation() + (120.0f * dt.asSeconds()));
                }
                if (characterSide == side::RIGHT)
                {
                    spriteSection.setRotation(spriteSection.getRotation() + (-120.0f * dt.asSeconds()));
                }
                // Is the tower section entirely off screen?
                if (spriteSection.getPosition().x < -500 || 
                spriteSection.getPosition().x > 2200)
                {
                    // Prepare the section to be a new section in the next frame
                    sectionActive = false;
                    //spriteSection.setRotation(0.0f);
                    //spriteSection.setPosition(745, 795);
                }
            }

            // The player dies when hit on the head by a turret
            if (turretPositions[5] == characterSide)
            {
                paused = true;
                backgroundMusic.pause();    // pause the music
                grave.play();               // play the RIP sound effect
                keyboardInput = true; // unnecessary? Wait and see
                spriteCharacter.setPosition(-500, 805);     // hide the player
                spriteWeapon.setPosition(-500, 805);        // hide the axe
                // show the headstone on the correct side
                if (characterSide == side::LEFT)
                {
                    spriteGrave.setPosition(575, 810);   
                }
                else if (characterSide == side::RIGHT)
                {
                    spriteGrave.setPosition(1200, 810);    
                }

                // Message for if the player dies rather than times out
                messageText.setString("K.O.!!");

                // Centre and position the message
                FloatRect textRect = messageText.getLocalBounds();
                messageText.setOrigin((textRect.left + textRect.width) / 2.0f, 
                (textRect.top + textRect.height) / 2.0f);
                messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);
            }

        } // End of if !paused
        
        /* 
        Clear, draw, and display here 
        */

        //Clear the previous frame 
        window.clear(); 
        
        // Draw the scene 
        window.draw(spriteBackground);
        for (int i = 0; i < NUM_CLOUDS; i++)
        {
            window.draw(clouds[i]);
        }
        for (int i = 0; i < NUM_TURRETS; i++)
        {
            window.draw(turrets[i]);
        }
        window.draw(spriteTower);
        window.draw(spriteSection);
        window.draw(spriteCharacter);
        window.draw(spriteGrave);
        window.draw(spriteWeapon);
        window.draw(spriteBird);
        window.draw(scoreText);
        window.draw(timeBar);
        if (paused)
        {
            window.draw(messageText);
        }

        // Display the new scene 
        window.display(); 
    }

    return 0;
}

// turret update function definition as the tower gets chopped
 void updateTurrets(int seed)
 {
    for (int j = NUM_TURRETS - 1; j > 0; j--)       // Move all the turrets down one by one from the top
    {
         turretPositions[j] = turretPositions[j - 1];   
    }
        // Spawn a new turret at the top (position 0): LEFT, RIGHT, or NONE
        srand((int)time(0) + seed);     // seed a random number using the parameter "seed"
        int r = (rand() % 3);           // create the random number between 0 and 2 inclusive

        switch (r)
        {
            case 0:
            turretPositions[0] = side::LEFT;
            break;

            case 1:
            turretPositions[0] = side::RIGHT;
            break;

            default:
            turretPositions[0] = side::NONE;
            break;
        }
    return;

                
}
