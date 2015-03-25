#include "Sphere.h"
#include <ngl/VAOPrimitives.h>


Sphere::Sphere( ngl::Vec3 _pos, ngl::Vec3 _dir,  GLfloat _rad)
{
  // set values from params
  m_pos=_pos;
  m_dir=_dir;
  m_radius=_rad;
  m_hit=false;


}

Sphere::Sphere()
{
  m_hit=false;
}

void Sphere::loadMatricesToShader( ngl::Transformation &_tx, const ngl::Mat4 &_globalMat, ngl::Camera *_cam  ) const
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


void Sphere::draw( const std::string &_shaderName, const ngl::Mat4 &_globalMat,  ngl::Camera *_cam )const
{

  // draw wireframe if hit
  if(m_hit)
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  }

  ngl::Vec3 verts[]=
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
  std::cout<<"sizeof(verts) "<<sizeof(verts)<<" sizeof(ngl::Vec3) "<<sizeof(ngl::Vec3)<<"\n";
  // create a vao as a series of GL_TRIANGLES
   ngl::VertexArrayObject *m_vao= ngl::VertexArrayObject::createVOA(GL_TRIANGLES);
  m_vao->bind();

   // in this case we are going to set our data as the vertices above

   m_vao->setData(sizeof(verts),verts[0].m_x);
   // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)

   m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);


   m_vao->setNumIndices(sizeof(verts)/sizeof(ngl::Vec3));

 // now unbind
  m_vao->unbind();


  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(_shaderName);
  // grab an instance of the primitives for drawing
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Transformation t;

  t.setPosition(m_pos);
  t.setScale(m_radius,m_radius,m_radius);
  loadMatricesToShader(t,_globalMat,_cam);
  //prim->draw("sphere");

  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();

glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}



void Sphere :: set(ngl::Vec3 _pos, ngl::Vec3 _dir, GLfloat _rad)
{
  m_pos=_pos;
  m_dir=_dir;
  m_radius=_rad;
}

void Sphere::move()
{
  // store the last position
  m_lastPos=m_pos;
  // update the current position
  m_pos+=m_dir;
  // get the next position
  m_nextPos=m_pos+m_dir;
  m_hit=false;
}



void Sphere::seek(ngl::Vec3 target)
{
    ngl::Vec3 desired = target - m_pos;
    desired.normalize();
    //desired*maxspeed);
    //ngl::Vec3 steer = desired - velocity;
    //steer.limit(maxForce);
    //applyForce(steer);

}









