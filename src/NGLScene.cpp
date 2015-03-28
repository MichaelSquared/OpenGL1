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

#include <boost/foreach.hpp>
#include "Boid.h"


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;
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
  m_drawFlockCenter = true;
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
  // now to load the shader and set the values
  // grab an instance of shader manager
   ngl::ShaderLib *shader=ngl::ShaderLib::instance();
   (*shader)["nglDiffuseShader"]->use();

   shader->setShaderParam4f("Colour",1,1,0,1);
   shader->setShaderParam3f("lightPos",1,1,1);
   shader->setShaderParam4f("lightDiffuse",1,1,1,1);

   (*shader)["nglColourShader"]->use();
   shader->setShaderParam4f("Colour",1,1,1,1);
   //void draw();
   glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

  //ngl::VAOPrimitives *prim =  ngl::VAOPrimitives::instance();
  //prim->createBoid("boid",1.0,40);
  //create our Bounding Box, needs to be done once we have a gl context as we create VAO for drawing
  m_bbox = new ngl::BBox(ngl::Vec3(0,0,0),80,80,80);

  m_bbox->setDrawMode(GL_LINE);
  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());
  m_boidUpdateTimer=startTimer(40);

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",2,8);

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglDiffuseShader"]->use();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV= m_mouseGlobalTX*m_cam->getViewMatrix() ;
  MVP=MV*m_cam->getProjectionMatrix() ;
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
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
   m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
   m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
   m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglColourShader"]->use();
  shader->setShaderParam4f("Colour",0,0,0,1); // Set shader colour to black

  m_transform.identity();
  loadMatricesToColourShader();
  m_bbox->draw();

  if(m_drawFlockCenter)
  {
      ngl::Vec3 avg = m_flock.getAveragePos();
      shader->setShaderParam4f("Colour",1,0.5,0,1); // Set shader colour to black

      m_transform.translate(avg.m_x, avg.m_y, avg.m_z);
      loadMatricesToColourShader();
      ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();
      prim->draw("sphere");
  }

  m_flock.draw(m_mouseGlobalTX, m_cam, shader);
//    m_vao->bind();
//    m_vao->draw();
//    m_vao->unbind();
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

    m_flock.update(m_bbox, m_checkBoidBoid);

    //checkCollisions();
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
  // finally update the GLWindow and re-draw
  //if (isExposed())
    renderLater();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::timerEvent(QTimerEvent *_event )
{
    if(_event->timerId() == m_boidUpdateTimer)
	{
		if (m_animate !=true)
		{
			return;
		}
		update();
		renderNow();
	}
}
//----------------------------------------------------------------------------------------------------------------------
//-------------------------------COLLISIONS-----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
bool NGLScene::boidBoidCollision( ngl::Vec3 _pos1, GLfloat _radius1, ngl::Vec3 _pos2, GLfloat _radius2 )
{
  // the relative position of the boids
  ngl::Vec3 relPos;
  //min an max distances of the boids
  GLfloat dist;
  GLfloat minDist;
  GLfloat len;
  relPos =_pos1-_pos2;
  // and the distance
  len=relPos.length();
  dist=len*len;
  minDist =_radius1+_radius2;
  // if it is a hit
  if(dist <=(minDist * minDist))
  {
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::removeBoid()
{
//  std::vector<Boid>::iterator end=m_boidArray.end();
//  if(--m_numBoids==0)
//  {
//    m_numBoids=1;
//  }
//  else
//  {
//    m_boidArray.erase(end-1,end);
//  }
    m_flock.removeBoid();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::addBoid()
{
//  ngl::Random *rng=ngl::Random::instance();
//  ngl::Vec3 dir;
//  dir=rng->getRandomVec3();
//  // add the boids to the end of the particle list
//  m_boidArray.push_back(Boid(rng->getRandomPoint(s_extents,s_extents,s_extents),dir,rng->randomPositiveNumber(2)+0.5));
//  ++m_numBoids;
    m_flock.addBoid();
}




