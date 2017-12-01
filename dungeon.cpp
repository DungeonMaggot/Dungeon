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

// temp includes
#include "simplesphere.h"
#include "color.h"

// dungeon includes
#include "dungeon.h"
#include "input.h"
#include "level0_map.h"

#include "ui_dockwidget.h"

static game_state GameState;
static game_button *ButtonState[2];

bool TileHasNeighbor(char direction)
{
    // TODO(andreas): Tile utilities
}



bool PlaceColumnAtPos(int X, int Y,
                      wall_directions WallDirection,
                      relative_column_position ColumnPosition,
                      bool *ColumnMap,
                      int ColumnMapWidth, int ColumnMapHeight)
{
    bool Result = false;

    if(    (X >= 0) && (X < (ColumnMapWidth  - 1))
        && (Y >= 0) && (Y < (ColumnMapHeight - 1))
        && (WallDirection  < NUM_WALL_DIRECTIONS)
        && (ColumnPosition < NUM_RELATIVE_COLUMN_POSITIONS) )
    {
        bool *Column = ColumnMap + Y*ColumnMapHeight + X;

        // There are four possible offset values (a to d),
        // depending on the column's corner position:
        //      N
        //    L   R
        //  R a---b L
        // W  |   |  E
        //  L c---d R
        //    R   L
        //      S
        enum offset_types
        {
            OFFSET_NW = 0, // a
            OFFSET_NE,     // b
            OFFSET_SE,     // c
            OFFSET_SW,     // d

            NUM_OFFSETS
        } OffsetType;
        int OffsetValues[NUM_OFFSETS] = {};
        OffsetValues[OFFSET_NW] = 0;
        OffsetValues[OFFSET_NE] = 1;
        OffsetValues[OFFSET_SE] = ColumnMapWidth;
        OffsetValues[OFFSET_SW] = ColumnMapWidth + 1;
        switch(WallDirection)
        {
            case WALL_WEST:
            {
                OffsetType = (ColumnPosition == COL_LEFT) ? OFFSET_SE : OFFSET_NW;
            } break;

            case WALL_EAST:
            {
                OffsetType = (ColumnPosition == COL_LEFT) ? OFFSET_NE : OFFSET_SW;
            } break;

            case WALL_SOUTH:
            {
                OffsetType = (ColumnPosition == COL_LEFT) ? OFFSET_SW : OFFSET_SE;
            } break;

            case WALL_NORTH:
            {
                OffsetType = (ColumnPosition == COL_LEFT) ? OFFSET_NW : OFFSET_NE;
            } break;

            default:
            {} break; // TODO(andreas): Handle error.
        }

        Column += OffsetValues[OffsetType];

        if(*Column == false)
        {
            *Column = true;
            Result = true;
        }
    }

    return Result;
}

Node *InitDungeonScene();

void SceneManager::initScenes()
{
    Ui_FPSWidget *lDock;
    QDockWidget *lDockWidget = new QDockWidget(QString("FPS"), SceneManager::getMainWindow());

    ControllableCamera *cam = new ControllableCamera();
    RenderingContext *myContext=new RenderingContext(cam);
    unsigned int myContextNr = SceneManager::instance()->addContext(myContext);
    unsigned int myScene = SceneManager::instance()->addScene(InitDungeonScene());
    ScreenRenderer *myRenderer = new ScreenRenderer(myContextNr, myScene);

    //Cam offset of the Pirot Point
    //QVector3D Cam_offset_3DQt_Vec(1.0,1.0,1.0);
    //cam->setPosition(Cam_offset_3DQt_Vec);

    //Vorsicht: Die Szene muss initialisiert sein, bevor das Fenster verändert wird (Fullscreen)
    SceneManager::instance()->setActiveScene(myScene);
    SceneManager::instance()->setActiveContext(myContextNr);
//    SceneManager::instance()->setFullScreen();

    lDock = new Ui_FPSWidget();
    lDock->setupUi(lDockWidget);
    lDockWidget->resize(200,100);
    SceneManager::getMainWindow()->addDockWidget(Qt::RightDockWidgetArea, lDockWidget);
    lDockWidget->show();
    QObject::connect(Window::getInstance(), SIGNAL(sigFPS(int)), lDock->lcdNumber, SLOT(display(int)));
}

Node *InitDungeonScene()
{
    //
    // game init
    //
    ButtonState[0] = new game_button[PA_NumActions]{};
    ButtonState[1] = new game_button[PA_NumActions]{};
    GameState.NewButtons = ButtonState[0];
    GameState.OldButtons = ButtonState[1];
    InputListener *Input = new InputListener(&GameState);

    bool *Level0ColumnMap = new bool[(LEVEL_0_WIDTH + 1)*(LEVEL_0_HEIGHT + 1)]{};

    //
    // load geometry
    //
    Geometry *FloorGeometry = new TriangleMesh("meshes/floor_v2.obj");
    Geometry *CeilingGeometry = new TriangleMesh("meshes/roof_v2.obj");
    Geometry *WallGeometry = new TriangleMesh("meshes/wall_v2.obj");
    Geometry *ColumnGeometry = new TriangleMesh("meshes/column_v2.obj");

    //
    // light sources
    //
    PointLight *LightSource = new PointLight;
    //marcos test Values
    //LightSource->setDiffuse(0.7, 0.5, 0.5); // RGB
    LightSource->setDiffuse(0.8, 0.8, 0.8); // RGB
    LightSource->setAmbient(0.6, 0.6, 0.6); // RGB
    LightSource->setSpecular(0.6, 0.6, 0.6); // RGB
    LightSource->turnOn();

    //
    // shaders
    //
    //Shader *FlatShader = ShaderManager::getShader("shaders/texture.vert", "shaders/texture.frag");
    //Shader *PhongShaderVL = ShaderManager::getShader("://shaders/PhongVL.vert", "://shaders/PassThrough.frag");
    Shader *PhongTexturedShader = ShaderManager::getShader("://shaders/phong_textured.vert", "://shaders/phong_textured.frag");

    //
    // drawables
    //
    Drawable *FloorModel = new Drawable(FloorGeometry);
    Drawable *CeilingModel = new Drawable(CeilingGeometry);
    Drawable *WallModel = new Drawable(WallGeometry);
    Drawable *ColumnModel = new Drawable(ColumnGeometry);

    //
    // assign textures
    //
    Texture *t = 0;

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

    //
    // assign shaders
    //

    //FloorModel->setShader(FlatShader);
    FloorModel->setShader(PhongTexturedShader);
    CeilingModel->setShader(PhongTexturedShader);
    WallModel->setShader(PhongTexturedShader);
    ColumnModel->setShader(PhongTexturedShader);

    // scene graph root
    Transformation *RootTransform = new Transformation();
    Node *RootNode = new Node(RootTransform);

    // TEMP LIGHT SOURCE TEST
    Transformation *LightTransform = new Transformation();
    Drawable *LightSphere = new Drawable(new SimpleSphere(0.25));
    Color *c = LightSphere->getProperty<Color>();
    c->setValue(1.0, 1.0, 0.0, 1.0);
    LightTransform->translate(4.0, 0.25, 10.0);
    Node *LightNode = new Node(LightTransform);
    //LightNode->addChild(new Node(LightSphere));
    LightNode->addChild(new Node(LightSource));
    RootNode->addChild(LightNode);

    // load tilemap
    {
        // a tile consists of:
        // - a node
        // - a transform
        // - floor geometry (using the tile's base transform)
        // - 0 to 3 walls   (using their own transforms)

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

                        // add columns to the wall, if necessary
                        bool MustPlaceColumn[NUM_RELATIVE_COLUMN_POSITIONS] = {};
                        for(int ColumnIndex = 0; ColumnIndex < NUM_RELATIVE_COLUMN_POSITIONS; ++ColumnIndex)
                        {
                            MustPlaceColumn[ColumnIndex] =
                                PlaceColumnAtPos(p.x, p.y,
                                                 (wall_directions)WallIndex, (relative_column_position) ColumnIndex,
                                                 Level0ColumnMap, (LEVEL_0_WIDTH + 1), (LEVEL_0_HEIGHT + 1));
                            // TODO(andreas): Better function name, function is checking slot and potentially marking
                            //                it as in use.

                            // place column geometry
                            if(MustPlaceColumn[ColumnIndex])
                            {
                                Transformation *ColumnTransform = new Transformation();
                                Node *ColumnNode = new Node(ColumnTransform);
                                WallNode->addChild(ColumnNode);

                                if(ColumnIndex == COL_LEFT)
                                {
                                    ColumnTransform->rotate(90.0, 0.0, 1.0, 0.0);
                                }

                                ColumnNode->addChild(new Node(ColumnModel));
                            }
                        }
                    }
                }
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
