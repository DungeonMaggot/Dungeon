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

Node *InitDungeonScene()
{
//    Objekte anlegen

    FloorTile *tile = new FloorTile();
    Planet *sonne = new Planet(1.0);
    Transformation *WorldRoot = new Transformation();
    Transformation *Tile0Transform = new Transformation();
    Transformation *Tile1Transform = new Transformation();
    Transformation *Tile2Transform = new Transformation();

    Color *c;

    //Farben
    c = sonne->getProperty<Color>();
    c->setValue(1.0,1.0,0.0,1.0);
    c = tile->getProperty<Color>();
    c->setValue(0.5, 0.5, 0.5, 1.0);

    //Damit man die Drehungen sieht, Gitternetz aktivieren
    sonne->deactivateFill();

    //Vorsicht beim ändern von Rot. und Trans. derselben Transformation:
    //Die Reihenfolge ist wichtig!
   // WorldRoot->rotate(45.0,1.0,0.0,0.0); //Neigen, damit man die Umlaufbahn besser sieht
    Tile0Transform->rotate(90.0f,1.0,0.0,0.0);
    Tile1Transform->rotate(90.0f,1.0,0.0,0.0);
    Tile2Transform->rotate(90.0f,1.0,0.0,0.0);

    Tile1Transform->translate(3.0,0.0,0.0);
    Tile2Transform->translate(0.0,2.0,0.0);

    //Szenengraph aufbauen
    Node *RootNode = new Node(WorldRoot);
    Node *Tile0Node = new Node(Tile0Transform);
    Node *Tile1Node = new Node(Tile1Transform);
    Node *Tile2Node = new Node(Tile2Transform);

    RootNode->addChild(new Node(sonne));

    RootNode->addChild(Tile0Node);
    Tile0Node->addChild(new Node(tile));

    RootNode->addChild(Tile1Node);
    Tile1Node->addChild(new Node(tile));

    RootNode->addChild(Tile2Node);
    Tile2Node->addChild(new Node(tile));


    return(RootNode);
}
