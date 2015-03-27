#include "Boid.h"
#include <ngl/VAOPrimitives.h>
#include <ngl/Random.h>
#include <QDebug>

Boid::Boid( ngl::Vec3 _pos, ngl::Vec3 _dir,  GLfloat _rad)
{
  //--set values from params--//
  //m_pos=_pos;
  m_pos=0,0,0;
  m_dir=_dir;
  m_radius=_rad;
  m_hit=false;

  //---------------------------------Colour--------------------------------------//
  ngl::Random *rand = ngl::Random::instance();     //Created a pointer
  m_boidColour = rand->getRandomColour();          //Pointing at ->getRandomColour()
  const float probabilityOfDisco = 1.0;            //Value/number of disco boids
  //--Randomly decide if this boid is a disco boid or not--//
  m_discoStyle = (bool)(fmod(rand->randomPositiveNumber(), 1) > probabilityOfDisco);
}

//default ctor
Boid::Boid()
{
  m_hit=false;
}

Boid::Boid(float m_x, float m_y, float m_z)
{
    //m_acceleration = new ngl::Vec3(0.0f,0.0f,0.0f);
    //m_velocity = new ngl::Vec3(0.0f,0.0f,0.0f);
    //m_pos = new ngl::Vec3(m_x, m_y, m_z);

}

void Boid::loadMatricesToShader( ngl::Transformation &_tx, const ngl::Mat4 &_globalMat, ngl::Camera *_cam  ) const
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV=_tx.getMatrix()*_globalMat*_cam->getViewMatrix() ;
  MVP=MV*_cam->getProjectionMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
}


void Boid::draw( const std::string &_shaderName, const ngl::Mat4 &_globalMat,  ngl::Camera *_cam )const
{
//--------------------------------------------//
//-----------Draw wireframe if hit------------//
//--------------------------------------------//
  if(m_hit)
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  }
//----------------------------------------------


//--------------------------------------------//
//-----------------Draw Boid------------------//
//--------------------------------------------//
  static const ngl::Vec3 verts[]=
  {
    ngl::Vec3(0,1,1),
    ngl::Vec3(0,0,-1),
    ngl::Vec3(-0.5,0,1),
    ngl::Vec3(0,1,1),
    ngl::Vec3(0,0,-1),
    ngl::Vec3(0.5,0,1),
    ngl::Vec3(0,1,1),
    ngl::Vec3(0,0,1.5),
    ngl::Vec3(-0.5,0,1),
    ngl::Vec3(0,1,1),
    ngl::Vec3(0,0,1.5),
    ngl::Vec3(0.5,0,1)

  };
  // create a vao as a series of GL_TRIANGLES
   ngl::VertexArrayObject *m_vao = ngl::VertexArrayObject::createVOA(GL_TRIANGLES);
   m_vao->bind();

   // in this case we are going to set our data as the vertices above
   m_vao->setData(sizeof(verts),verts[0].m_x);

   // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)
   m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);

   m_vao->setNumIndices(sizeof(verts)/sizeof(ngl::Vec3));

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(_shaderName);

  ngl::Transformation t;

  t.setPosition(m_pos);
  t.setScale(m_radius, m_radius, m_radius);
  loadMatricesToShader(t,_globalMat,_cam);

  //m_vao->bind();
  m_vao->draw();
  m_vao->unbind();

  delete m_vao;

}



void Boid::set(ngl::Vec3 _pos, ngl::Vec3 _dir, GLfloat _rad)
{
  m_pos=_pos;
  m_dir=_dir;
  m_radius=_rad;
}

void Boid::move()
{
  // store the last position
  m_lastPos=m_pos;
  // update the current position
  m_pos+=m_dir;
  // get the next position
  m_nextPos=m_pos+m_dir;
  m_hit=false;
}

/****************************************************************************/

void Boid::seek(ngl::Vec3 target)
{
    ngl::Vec3 desired = target - m_pos;
    //desired *= 0.05;
    desired.normalize();
    desired *= maxSpeed;
    ngl::Vec3 steer = desired - m_velocity;
    //steer.limit(maxForce);
    //applyForce(steer);

}
/*
void Boid::display()
{
    float theta = velocity.heading() + PI/2;
    fill(175);
    stroke(0);
    pushMatrix();
    translate(location.x,location.y);
    rotate(theta);
    beginShape();
    vertex(0, -_rad*2);
    vertex(-r, _rad*2);
    vertex(r, _rad*2);
    endShape(CLOSE);
    popMatrix();
}*/
/****************************************************************************/




