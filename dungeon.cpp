#include "opengl.h"
#include "window.h"
#include "screenrenderer.h"
#include "scenemanager.h"
#include "transformation.h"
#include "keyboardtransformation.h"
#include "controllablecamera.h"
#include "floor_tile.h"
#include "wall.h"
#include "planet.h"
#include "color.h"
#include "level0_map.h"

#include "ui_dockwidget.h"

bool TileHasNeighbor(char direction)
{
    // TODO(andreas): Tile utilities
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

typedef struct v2i
{
    int x;
    int y;
} TilePos;

Node *InitDungeonScene()
{
    // drawables
    FloorTile *floor_tile = new FloorTile();
    Wall *WallNorthMesh = new Wall({0.7, 0.7, 0.7, 1.0});
    Wall *WallSouthMesh = new Wall({0.65, 0.65, 0.65, 1.0});
    Wall *WallEastMesh  = new Wall({0.6, 0.6, 0.6, 1.0});
    Wall *WallWestMesh  = new Wall({0.55, 0.55, 0.55, 1.0});

    // scene graph root
    Transformation *root_transform = new Transformation();
    Node *root_node = new Node(root_transform);

    // load tilemap
    {
        TilePos p = {0, 0};
        for(char *c = Level0Map; *c != '\0'; ++c)
        {
            if(*c != ' ') // not empty
            {
                // base tile
                Transformation *tile_transform = new Transformation();
                tile_transform->translate(p.x, 0.0, p.y); // TODO(andreas): Make a x, z 2D vector?
                Node *tile_node = new Node(tile_transform);
                root_node->addChild(tile_node);

                // floor
                Transformation *floor_transform = new Transformation();
                floor_transform->rotate(90.0, 1.0, 0.0, 0.0); // make plane parallel to the ground

                Node *floor_node = new Node(floor_transform);
                tile_node->addChild(floor_node);
                floor_node->addChild(new Node(floor_tile));

                // walls
                bool wall_east = false; // left
                bool wall_west = false; // right
                bool wall_north = false; // up
                bool wall_south = false; // down
                // Note: top-left (NE) corner of the world is at 0,0

                if(p.x == 0)
                {
                    wall_east = true;
                }
                else if(*(c - 1) == ' ')
                {
                     wall_east = true;
                }

                if(p.x == (LEVEL_0_WIDTH - 1))
                {
                    wall_west = true;
                }
                else if(*(c + 1) == ' ')
                {
                    wall_west = true;
                }

                if(p.y == 0)
                {
                    wall_north = true;
                }
                else if(*(c - LEVEL_0_WIDTH) == ' ')
                {
                     wall_north = true;
                }

                if(p.y == (LEVEL_0_HEIGHT - 1))
                {
                    wall_south = true;
                }
                else if(*(c + LEVEL_0_WIDTH) == ' ')
                {
                     wall_south = true;
                }

                if(wall_east)
                {
                    Transformation *wall_transform = new Transformation();
                    wall_transform->rotate(90.0, 0.0, 1.0, 0.0); // change from horizontal to vertical (top-down view)
                    wall_transform->translate(0.0, 0.0, -0.5);

                    // TODO(andreas): Remove the following line once assets are online:
                    wall_transform->translate(0.0, 0.5, 0.0); // move up a bit

                    Node *wall_node = new Node(wall_transform);
                    tile_node->addChild(wall_node);
                    wall_node->addChild(new Node(WallEastMesh));
                }

                if(wall_west)
                {
                    Transformation *wall_transform = new Transformation();
                    wall_transform->rotate(90.0, 0.0, 1.0, 0.0); // change from horizontal to vertical (top-down view)
                    wall_transform->translate(0.0, 0.0, 0.5);

                    // TODO(andreas): Remove the following line once assets are online:
                    wall_transform->translate(0.0, 0.5, 0.0); // move up a bit

                    Node *wall_node = new Node(wall_transform);
                    tile_node->addChild(wall_node);
                    wall_node->addChild(new Node(WallWestMesh));
                }

                if(wall_north)
                {
                    Transformation *wall_transform = new Transformation();
                    wall_transform->translate(0.0, 0.0, -0.5);

                    // TODO(andreas): Remove the following line once assets are online:
                    wall_transform->translate(0.0, 0.5, 0.0); // move up a bit

                    Node *wall_node = new Node(wall_transform);
                    tile_node->addChild(wall_node);
                    wall_node->addChild(new Node(WallNorthMesh));
                }

                if(wall_south)
                {
                    Transformation *wall_transform = new Transformation();
                    wall_transform->translate(0.0, 0.0, 0.5);

                    // TODO(andreas): Remove the following line once assets are online:
                    wall_transform->translate(0.0, 0.5, 0.0); // move up a bit

                    Node *wall_node = new Node(wall_transform);
                    tile_node->addChild(wall_node);
                    wall_node->addChild(new Node(WallSouthMesh));
                }
            }
            else
            {
                // empty
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

    return(root_node);
}
