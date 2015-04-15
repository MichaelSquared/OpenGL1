#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>

#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/NGLStream.h>

#include <boost/foreach.hpp>
#include "Boid.h"


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.25; // old value was 0.01
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=2;
//----------------------------------------------------------------------------------------------------------------------
/// @brief extents of the bbox
//----------------------------------------------------------------------------------------------------------------------
const static int s_extents=20;

NGLScene::NGLScene(int _numBoids,QWindow *_parent) : OpenGLWindow(_parent), m_flock(_numBoids, s_extents)
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Flocking System");
  m_animate=true;
  m_checkBoidBoid=false;
  // create vectors for the position and direction
  m_flock.resetBoids();
  m_drawFlockCenter = false;
}

//----------------------------------------------------------------------------------------------------------------------
NGLScene::~NGLScene()
{
  ngl::NGLInit *Init = ngl::NGLInit::instance();
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  Init->NGLQuit();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::resizeEvent(QResizeEvent *_event )
{
  if(isExposed())
  {
  int w=_event->size().width();
  int h=_event->size().height();
  // set the viewport for openGL
  glViewport(0,0,w,h);
  // now set the camera size values as the screen size has changed
  m_cam->setShape(45,(float)w/h,0.05,350);
  renderLater();
  }
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::initialize()
{
    // we must call this first before any other GL commands to load and link the
    // gl commands from the lib, if this is not done program will crash
    ngl::NGLInit::instance();

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);
    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);
    // Now we will create a basic Camera from the graphics library
    // This is a static camera so it only needs to be set once
    // First create Values for the camera position
    ngl::Vec3 from(0,10,150);
    ngl::Vec3 to(0,0,0);
    ngl::Vec3 up(0,1,0);
    m_cam= new ngl::Camera(from,to,up);
    // set the shape using FOV 45 Aspect Ratio based on Width and Height
    // The final two are near and far clipping planes of 0.5 and 10
    m_cam->setShape(45,(float)720.0/576.0,0.5,150);
    //create our Bounding Box, needs to be done once we have a gl context as we create VAO for drawing
    m_bbox = new ngl::BBox(ngl::Vec3(0,0,0),80,80,80);

    m_bbox->setDrawMode(GL_LINE);
    // as re-size is not explicitly called we need to do this.
    glViewport(0,0,width(),height());
    m_boidUpdateTimer=startTimer(40);

    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    prim->createSphere("sphere",2,8);

    // Initialise gold shader
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    // we are creating a shader called Phong
    shader->createShaderProgram("Phong");
    // now we are going to create empty shaders for Frag and Vert
    shader->attachShader("PhongVertex",ngl::VERTEX);
    shader->attachShader("PhongFragment",ngl::FRAGMENT);
    // attach the source
    shader->loadShaderSource("PhongVertex","shaders/PhongVertex.glsl");
    shader->loadShaderSource("PhongFragment","shaders/PhongFragment.glsl");
    // compile the shaders
    shader->compileShader("PhongVertex");
    shader->compileShader("PhongFragment");
    // add them to the program
    shader->attachShaderToProgram("Phong","PhongVertex");
    shader->attachShaderToProgram("Phong","PhongFragment");
    // now bind the shader attributes for most NGL primitives we use the following
    // layout attribute 0 is the vertex data (x,y,z)
    shader->bindAttribute("Phong",0,"inVert");
    // attribute 1 is the UV data u,v (if present)
    shader->bindAttribute("Phong",1,"inUV");
    // attribute 2 are the normals x,y,z
    shader->bindAttribute("Phong",2,"inNormal");

    // now we have associated this data we can link the shader
    shader->linkProgramObject("Phong");
    // and make it active ready to load values
    (*shader)["Phong"]->use();
    // the shader will use the currently active material and light0 so set them

    m_light = new ngl::Light(ngl::Vec3(-2,5,2),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::POINTLIGHT );
//    m_light->setTransform(iv);
    // load these values to the shader as well
    m_light->loadToShader("light");

    ngl::Material m(ngl::GOLD);
    // load our material values to the shader into the structure material (see Vertex shader)
    m.loadToShader("material");
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::loadMatricesToColourShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglColourShader"]->use();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;

  MV= m_transform*m_mouseGlobalTX*m_cam->getViewMatrix() ;
  MVP=MV*m_cam->getProjectionMatrix();
  shader->setShaderParamFromMat4("MVP",MVP);

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::render()
{
    // clear the screen and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Rotation based on the mouse position for our global
    // transform
    ngl::Mat4 rotX;
    ngl::Mat4 rotY;
    // create the rotation matrices
    rotX.rotateX(m_spinXFace);
    rotY.rotateY(m_spinYFace);
    // multiply the rotations
    m_mouseGlobalTX=rotY*rotX;
    // add the translations

//    m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
//    m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
//    m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

    // grab an instance of the shader manager
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["nglColourShader"]->use();
    shader->setShaderParam4f("Colour",0,0,0,1); // Set shader colour to black

    m_transform.identity();
    loadMatricesToColourShader();
    m_bbox->draw();

    ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();

    if(m_drawFlockCenter)
    {
      ngl::Vec3 avg = m_flock.getAveragePos();
      shader->setShaderParam4f("Colour",1,0.5,0,1); // Set shader colour to orange
      m_transform.translate(avg.m_x, avg.m_y, avg.m_z);
      loadMatricesToColourShader();
      prim->draw("sphere");
    }

    m_transform.identity();
    m_transform.translate(m_modelPos.m_x, m_modelPos.m_y, m_modelPos.m_z);
    loadMatricesToColourShader();
    prim->draw("sphere");

    m_flock.draw(m_mouseGlobalTX, m_cam, shader);
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::update()
{

/***********************************************************************/
    //m_velocity += m_acceleration;
    //m_velocity.limit(maxSpeed);
    //m_pos += m_velocity;
    //m_accelaration *= 0;
/***********************************************************************/

    std::cout << m_modelPos << std::endl;
    m_flock.update(m_bbox, m_checkBoidBoid, m_modelPos);

}

//----------------------------------------------------------------------------------------------------------------------
//------------------------------------MOUSE_EVENTS----------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent(QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    renderLater();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    renderLater();

   }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
	renderLater();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case  Qt::Key_Space : m_animate^=true; break;
  case Qt::Key_S : m_checkBoidBoid^=true; break;
  case Qt::Key_R : m_flock.resetBoids(); break;

  case Qt::Key_Minus : removeBoid(); break;
  case Qt::Key_Plus : addBoid(); break;

  default : break;
  }
    renderLater();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::timerEvent(QTimerEvent *_event )
{
    if(_event->timerId() == m_boidUpdateTimer)
	{
        if (m_animate != true)
		{
			return;
		}
		update();
		renderNow();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::removeBoid()
{
    m_flock.removeBoid();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::addBoid()
{
    m_flock.addBoid();
}




