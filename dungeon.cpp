#include "opengl.h"
#include "window.h"
#include "screenrenderer.h"
#include "scenemanager.h"
#include "transformation.h"
#include "keyboardtransformation.h"
#include "controllablecamera.h"
#include "shadermanager.h"
#include "texture.h"
#include "trianglemesh.h"
#include "pointlight.h"
#include <stdlib.h>
#include <time.h>

#include "simplecube.h"
#include "simplesphere.h"
#include "color.h"

#include "audioengine.h"

// dungeon includes
#include "dungeon.h"
#include "debug_camera.h"
#include "input.h"
#include "level0_map.h"

#include "ui_dockwidget.h"

static game_state GameState;
static game_button *ButtonState[2];


Node *InitDungeonScene(Ui_FPSWidget *);

void SceneManager::initScenes()
{
    Ui_FPSWidget *lDock;
    QDockWidget *lDockWidget = new QDockWidget(QString("FPS"), SceneManager::getMainWindow());

    DebugCamera *PlayerCam = new DebugCamera();
    DebugCamera *DebugCam  = new DebugCamera();
    RenderingContext *myContext=new RenderingContext(PlayerCam);
    unsigned int myContextNr = SceneManager::instance()->addContext(myContext);
    unsigned int myScene = SceneManager::instance()->addScene(InitDungeonScene(lDock));
    ScreenRenderer *myRenderer = new ScreenRenderer(myContextNr, myScene);

    //Cam offset of the Pirot Point
    //QVector3D Cam_offset_3DQt_Vec(1.0,1.0,1.0);
    //cam->setPosition(Cam_offset_3DQt_Vec);

    //Vorsicht: Die Szene muss initialisiert sein, bevor das Fenster verändert wird (Fullscreen)
    SceneManager::instance()->setActiveScene(myScene);
    SceneManager::instance()->setActiveContext(myContextNr);
//    SceneManager::instance()->setFullScreen();

    // store pointer to scene manager and "controllable camera" in game state
    GameState.SceneManagerRef = this;
    GameState.PlayerCam = PlayerCam;
    GameState.DebugCam = DebugCam;

#if 0
    lDock = new Ui_FPSWidget();
    lDock->setupUi(lDockWidget);
    lDockWidget->resize(200,100);
    SceneManager::getMainWindow()->addDockWidget(Qt::RightDockWidgetArea, lDockWidget);
    lDockWidget->show();
    QObject::connect(Window::getInstance(), SIGNAL(sigFPS(int)), lDock->lcdNumber, SLOT(display(int)));
#endif
}

Node *InitDungeonScene(Ui_FPSWidget *UI)
{
    //
    // game init
    //
    ButtonState[0] = new game_button[PA_NumActions]{};
    ButtonState[1] = new game_button[PA_NumActions]{};
    GameState.NewButtons = ButtonState[0];
    GameState.OldButtons = ButtonState[1];
    InputListener *Input = new InputListener(&GameState);
    GameState.LevelMap = Level0Map;
    GameState.LevelWidth = LEVEL_0_WIDTH;
    GameState.LevelHeight = LEVEL_0_HEIGHT;

    bool *Level0ColumnMap = new bool[(LEVEL_0_WIDTH + 1)*(LEVEL_0_HEIGHT + 1)]{};

    //
    // scene graph root
    //

    Transformation *RootTransform = new Transformation();
    Node *RootNode = new Node(RootTransform);

    //
    // audio init
    //
    AudioListener *AudioListenerInstance = new AudioListener();
    AudioEngine::instance().init(AudioEngineType::OpenAL3D);
    RootNode->addChild(new Node(AudioListenerInstance));
    SoundSource *AmbientSound = new SoundSource(new SoundFile("sounds/atmosphere-cave-loop.wav"));
    AmbientSound->setLooping(true);
    AmbientSound->play();

    GameState.Sound_PlayerStep       = new SoundFile("sounds/player_step_1.wav");
    GameState.Sound_PlayerAttack     = new SoundFile("sounds/attack_slash_1.wav");
    GameState.Sound_PlayerTakeDamage = new SoundFile("sounds/player_take_damage_1.wav");
    GameState.Sound_EnemyStep        = new SoundFile("sounds/enemy_step_1.wav");
    GameState.Sound_EnemyAttack      = new SoundFile("sounds/attack_slash_1.wav");
    GameState.Sound_EnemyTakeDamage  = new SoundFile("sounds/enemy_take_damage_1.wav");

    //
    // shaders
    //
    //Shader *FlatShader = ShaderManager::getShader("shaders/texture.vert", "shaders/texture.frag");
    //Shader *PhongShaderVL = ShaderManager::getShader("://shaders/PhongVL.vert", "://shaders/PassThrough.frag");
    Shader *PhongTexturedShader = ShaderManager::getShader("://shaders/phong_textured.vert", "://shaders/phong_textured.frag");

    //
    // dungeon actors - player
    //
    GameState.Player = new Player(2, 7,  // start position (tile coordinates, negative y is north)
                                  1,     // distance from floor (affects camera)
                                  0, -1, // initial orientation, negative y is "up" on the map
                                  &GameState);
    Node *PlayerNode = new Node(GameState.Player);
    {
        Player *p = dynamic_cast<Player *>(GameState.Player);
        if(p)
        {
            p->ActorModel = new Drawable(new SimpleSphere(0.5));
            Color *c = p->ActorModel->getProperty<Color>();
            c->setValue(1.f, 1.f, 0.f);
            p->ActorModel->setEnabled(false);
            PlayerNode->addChild(new Node(p->ActorModel));

            // player weapon
            p->WeaponPivot = new Transformation;
            p->Weapon = new Transformation;

            p->Weapon->rotate(-20.f, 1.f, 0.f, 0.f);
            p->Weapon->translate(0.5f, 0.5f, -1.f);

            Geometry *WeaponGeometry = new SimpleCube(.1f, 1.f, .1f);
            Drawable *WeaponModel = new Drawable(WeaponGeometry);
            WeaponModel->setEnabled(false);
            c = WeaponModel->getProperty<Color>();
            c->setValue(0.4, 0.4, 0.4, 1.0);
            Node *WeaponPivotNode = new Node(p->WeaponPivot);
            Node *WeaponNode = new Node(p->Weapon);
            WeaponNode->addChild(new Node(WeaponModel));
            WeaponPivotNode->addChild(WeaponNode);
            PlayerNode->addChild(WeaponPivotNode);

            //
            // UI init
            //
            // connect(*p, &Player::TookDamage, UI->hitpointsLabel, &QLabel::setNum);
        }

        // player light source
        PointLight *PlayerLight = new PointLight;
        PlayerLight->setDiffuse(0.8, 0.8, 0.8); // RGB
        PlayerLight->setAmbient(0.6, 0.6, 0.6); // RGB
        PlayerLight->setSpecular(0.6, 0.6, 0.6); // RGB
        PlayerLight->turnOn();
        PlayerLight->setQuadraticAttenuation(0.04); // higher value = darker
        PlayerNode->addChild(new Node(PlayerLight));
    }
    RootNode->addChild(PlayerNode);

    //
    // dungeon actors - enemies
    //
#if 1
    GameState.Enemies[0] = new Megaskull(1, 4, // start position (tile coordinates, negative y is north)
                                         0.25,     // distance from floor (affects model)
                                         0, 1, // intial orientation, negative y is "up" on the map
                                         true, // rotates when idle
                                         &GameState);
    GameState.Enemies[1] = new Megaskull(4, 0, // start position (tile coordinates, negative y is north)
                                         0.25,     // distance from floor (affects model)
                                         0, 1, // intial orientation, negative y is "up" on the map
                                         false, // does not rotate when idle
                                         &GameState);
    GameState.Enemies[2] = new Megaskull(9, 1, // start position (tile coordinates, negative y is north)
                                         0.25,     // distance from floor (affects model)
                                         0, 1, // intial orientation, negative y is "up" on the map
                                         true, // does not rotate when idle
                                         &GameState);
#endif
    // setup enemy graphics and SG nodes
    Geometry *MegaskullGeometry = new TriangleMesh("meshes/skull_base.obj");

    for(unsigned int EnemyIndex = 0; EnemyIndex < ArrayCount(GameState.Enemies); ++EnemyIndex)
    {
        Megaskull *mega = dynamic_cast<Megaskull *>(GameState.Enemies[EnemyIndex]);
        if(mega)
        {
            mega->ActorModel = new Drawable(MegaskullGeometry);
            {
                Texture *t = mega->ActorModel->getProperty<Texture>();
                t->loadPicture("textures/skull_base_tex_2k.png");
                Material *m = mega->ActorModel->getProperty<Material>();
                m->setDiffuse(0.5, 0.5, 0.5, 1.0); // RGBA
                m->setAmbient(0.5, 0.5, 0.5, 1.0); // RGBA
                m->setSpecular(0.5, 0.5, 0.5, 1.0); // RGBA
                m->setShininess(1.0);
                mega->ActorModel->setShader(PhongTexturedShader);
            }

            Node *MegaskullNode = new Node(mega);
            MegaskullNode->addChild(new Node(mega->ActorModel));
            RootNode->addChild(MegaskullNode);
        }
    }

    //
    // load geometry
    //
    Geometry *FloorGeometry = new TriangleMesh("meshes/floor_v2.obj");
    Geometry *CeilingGeometry = new TriangleMesh("meshes/roof_v2.obj");
    Geometry *WallGeometry = new TriangleMesh("meshes/wall_v2.obj");
    Geometry *ColumnGeometry = new TriangleMesh("meshes/column_v2.obj");


    //TODO: (marco) Cleanup the mess i wrote! (everything from the Props)
    // load 10 props_v2 geometry
    // int 48-57 = char 0-9

    unsigned int prop_anz = 10;
    Geometry* Prop_geo_array[10];
    char Prop_Path[200] = "meshes/prop_v2_0.obj";

    for(unsigned int i = 0; i < prop_anz; i++){
        Prop_Path[15] = '0'+i;
        Prop_geo_array[i] = new TriangleMesh(Prop_Path);
    }

    //
    // light sources
    //


    //
    // drawables
    //
    Drawable *FloorModel = new Drawable(FloorGeometry);
    Drawable *CeilingModel = new Drawable(CeilingGeometry);
    Drawable *WallModel = new Drawable(WallGeometry);
    Drawable *ColumnModel = new Drawable(ColumnGeometry);

    //Drawables of the Props
    Drawable* Prop_drawble_array[10];

    for(unsigned int i = 0; i < prop_anz; i++){
        Prop_drawble_array[i] = new Drawable(Prop_geo_array[i]);;
    }

    //
    // assign textures
    //
    Texture *t = 0;
    //18
    char Prop_tex_path[200] = "textures/prop_v2_0_tex.png";


    // floor
    t = FloorModel->getProperty<Texture>();
    t->loadPicture("textures/floor_v2_tex.png");

    // ceiling
    t = CeilingModel->getProperty<Texture>();
    t->loadPicture("textures/roof_v2_tex.png");

    // wall
    t = WallModel->getProperty<Texture>();
    t->loadPicture("textures/wall_v2_tex.png");

    // column
    t = ColumnModel->getProperty<Texture>();
    t->loadPicture("textures/column_v2_tex.png");

    //props Texturen

    for (unsigned int i = 0; i<prop_anz; i++){
        Prop_tex_path[17] = '0'+i;
        t = Prop_drawble_array[i]->getProperty<Texture>();
        t->loadPicture(Prop_tex_path);
    }

    //
    // assign materials
    //
    Material *m = 0;

    // floor
    m = FloorModel->getProperty<Material>();
    m->setDiffuse(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setAmbient(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setSpecular(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setShininess(1.0);

    // ceiling
    m = CeilingModel->getProperty<Material>();
    m->setDiffuse(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setAmbient(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setSpecular(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setShininess(1.0);

    // wall
    m = WallModel->getProperty<Material>();
    m->setDiffuse(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setAmbient(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setSpecular(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setShininess(1.0);

    // column
    m = ColumnModel->getProperty<Material>();
    m->setDiffuse(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setAmbient(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setSpecular(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setShininess(1.0);

    // Props
    m = Prop_drawble_array[5]->getProperty<Material>();
    m->setDiffuse(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setAmbient(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setSpecular(0.5, 0.5, 0.5, 1.0); // RGBA
    m->setShininess(1.0);

    //
    // assign shaders
    //

    //FloorModel->setShader(FlatShader);
    FloorModel->setShader(PhongTexturedShader);
    CeilingModel->setShader(PhongTexturedShader);
    WallModel->setShader(PhongTexturedShader);
    ColumnModel->setShader(PhongTexturedShader);

    // Props
    for(unsigned int i = 0; i<prop_anz; i++){
        Prop_drawble_array[i]->setShader(PhongTexturedShader);
    }

    // load tilemap
    {
        // a tile consists of:
        // - a node
        // - a transform
        // - floor geometry (using the tile's base transform)
        // - 0 to 3 walls   (using their own transforms)

        srand(time(NULL));

        TilePos p = {0, 0};
        for(char *c = Level0Map; *c != '\0'; ++c)
        {
            if(*c != ' ') // not empty
            {
                // base tile
                Transformation *TileTransform = new Transformation();
                TileTransform->translate(p.x*TILE_LENGTH, 0.0, p.y*TILE_LENGTH); // TODO(andreas): Make a x, z 2D vector?
                Node *TileNode = new Node(TileTransform);
                RootNode->addChild(TileNode);

                // floor
                Node *FloorNode = new Node(FloorModel);
                TileNode->addChild(FloorNode);

                // ceiling
                Node *CeilingNode = new Node(CeilingModel);
                TileNode->addChild(CeilingNode);



                // props
                // TODO(marco): Prevent dobble spawning on the same position and rotation
                int anz = rand() % 10+1;
                int rotationTracker[10];
                int TypeTracker[10];

                for (int i = 0; i < anz; i++){
                    int randtype = rand() % 10+1;
                    int randrotation = rand() % 3+1;
                    rotationTracker[i] = randrotation;
                    TypeTracker[i] = randtype;

                    Transformation* prop_trans = new Transformation();

                    for (int j = 0; j < rotationTracker[i]; j++){
                        prop_trans->rotate(90,0.0,1.0,0.0);
                    }


                    Node *Prop_1_Node = new Node(prop_trans);

                    TileNode->addChild(Prop_1_Node);
                    Prop_1_Node->addChild(new Node(Prop_drawble_array[i]));


                }
                // walls


                bool WallInfo[NUM_WALL_DIRECTIONS] = {};

                // Note: top-left (NW) corner of the world is at 0,0

                if(p.x == 0)
                {
                    WallInfo[WALL_WEST] = true;
                }
                else if(*(c - 1) == ' ')
                {
                    WallInfo[WALL_WEST] = true;
                }

                if(p.x == (LEVEL_0_WIDTH - 1))
                {
                    WallInfo[WALL_EAST] = true;
                }
                else if(*(c + 1) == ' ')
                {
                    WallInfo[WALL_EAST] = true;
                }

                if(p.y == 0)
                {
                    WallInfo[WALL_NORTH] = true;
                }
                else if(*(c - LEVEL_0_WIDTH) == ' ')
                {
                     WallInfo[WALL_NORTH] = true;
                }

                if(p.y == (LEVEL_0_HEIGHT - 1))
                {
                    WallInfo[WALL_SOUTH] = true;
                }
                else if(*(c + LEVEL_0_WIDTH) == ' ')
                {
                    WallInfo[WALL_SOUTH] = true;
                }

                // process all four possible walls
                for (int WallIndex = 0; WallIndex < NUM_WALL_DIRECTIONS; ++WallIndex)
                {
                    if(WallInfo[WallIndex] == true)
                    {
                        // add a wall
                        Transformation *WallTransform = new Transformation();

                        switch(WallIndex)
                        {
                            case WALL_WEST:
                            { WallTransform->rotate(90.0, 0.0, 1.0, 0.0); } break;

                            case WALL_EAST:
                            { WallTransform->rotate(-90.0, 0.0, 1.0, 0.0); } break;

                            case WALL_SOUTH:
                            { WallTransform->rotate(180.0, 0.0, 1.0, 0.0); } break;

                            case WALL_NORTH:
                            {} break; // no rotation necessary

                            default:
                            {} break; // TODO(andreas): Handle error.
                        }

                        Node *WallNode = new Node(WallTransform);
                        TileNode->addChild(WallNode);
                        WallNode->addChild(new Node(WallModel));
                        WallNode->addChild(new Node(ColumnModel));
                    }
                }
                //adds props


            }
            else
            {
                // This position is empty: no floor, walls etc. needed.
            }

            if(p.x < (LEVEL_0_WIDTH - 1))
            {
                ++p.x;
            }
            else // new row
            {
                ++p.y;
                p.x = 0;
            }
        }
    }

    return(RootNode);
}
