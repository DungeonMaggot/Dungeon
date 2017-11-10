#include "opengl.h"
#include "window.h"
#include "screenrenderer.h"
#include "scenemanager.h"
#include "transformation.h"
#include "keyboardtransformation.h"
#include "controllablecamera.h"
#include "floor_tile.h"
#include "planet.h"
#include "color.h"
#include "level0_map.h"

#include "ui_dockwidget.h"

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


    // scene graph root
    Transformation *root_transform = new Transformation();
    Node *root_node = new Node(root_transform);

    // load tilemap
    {
        TilePos p = {0, 0};
        for(char *c = Level0Map; *c != '\0'; ++c)
        {
            if(*c == '\n' || *c == '#') // new row
            {
                ++p.y;
                p.x = 0;
            }
            else
            {
                if(*c != ' ') // not empty
                {
                    Transformation *tile_transform = new Transformation();
                    tile_transform->rotate(90.0, 1.0, 0.0, 0.0); // make plane parallel to the ground
                    tile_transform->translate(p.x, p.y, 0.0);

                    Node *tile_node = new Node(tile_transform);
                    root_node->addChild(tile_node);
                    tile_node->addChild(new Node(floor_tile));
                }
                else
                {
                    // empty
                }

                ++p.x;
            }
        }
    }

    return(root_node);
}
