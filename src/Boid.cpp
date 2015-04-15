#include "Boid.h"
#include <ngl/VAOPrimitives.h>
#include <ngl/Random.h>
#include <ngl/NGLStream.h>
#include <QDebug>
#include <boost/foreach.hpp>
#include "mathfunctions.h"
#include <math.h>

Boid::Boid(ngl::Vec3 _pos, ngl::Vec3 _dir,  GLfloat _rad)
{
    //--set values from params--//
    m_pos=0,0,0; //appear in the center of the boundingBox
    m_dir=_dir;
    //ngl::Random *rng=ngl::Random::instance();
    //float angle = rng->randomNumber(M_PI);
    //m_dir=(cos(angle), sin(angle), cos(angle));
    m_radius=0.5;

    ///max speed of the flock
    m_maxSpeed = 0.9;
    ///max force of the flock
    m_maxForce = 0.05;

    //---------------------------------Colour--------------------------------------//
    ngl::Random *rand = ngl::Random::instance();     //Created a pointer
    m_boidColour = rand->getRandomColour();          //Pointing at ->getRandomColour()
    const float probabilityOfDisco = 0.3;            //Value/number of disco boids
    //-------------Randomly decide if this boid is a disco boid or not-------------//
    m_discoStyle = (bool)(fmod(rand->randomPositiveNumber(), 1) < probabilityOfDisco);

    update();
}

//----------------------------------------------------------------------------------------------------------------------

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
//----------------------------------------------------------------------------------------------------------------------

void Boid::draw( const std::string &_shaderName, const ngl::Mat4 &_globalMat,  ngl::Camera *_cam )const
{
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

    ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();
    prim->draw("sphere");

    //m_vao->bind();
    //m_vao->draw();
    //m_vao->unbind();

    delete m_vao;
}
//----------------------------------------------------------------------------------------------------------------------

void Boid::update()
{
    m_dir += m_acceleration;
    m_dir.clamp(-m_maxSpeed, m_maxSpeed);
    m_pos += m_dir;
    m_acceleration *= 0;

    //m_pos.m_z = 0; //move in 2D
}
//----------------------------------------------------------------------------------------------------------------------

void Boid::set(ngl::Vec3 _pos, ngl::Vec3 _dir, GLfloat _rad)
{
  m_pos=_pos;
  m_dir=_dir;
  m_radius=_rad;
}
//----------------------------------------------------------------------------------------------------------------------

void Boid::applyForce(ngl::Vec3 force)
{
    m_acceleration += force;
}
//----------------------------------------------------------------------------------------------------------------------

void Boid::seek(ngl::Vec3 target)
{
    ngl::Vec3 desired = target - m_pos;
    desired.normalize();
    desired = desired*m_maxSpeed;

    ngl::Vec3 steer = desired - m_dir;   //Reynold's formula for steering force
    steer.clamp(m_maxForce);
    applyForce(steer);
}
//----------------------------------------------------------------------------------------------------------------------

void Boid::arrive(ngl::Vec3 target)
{
    ngl::Vec3 desired = target - m_pos;

    float d = desired.length();
    desired.normalize();
    if(d < 100)
    {
        float m = map(d, 0, 10, 0, m_maxSpeed);  //change speed for seeking the target //100 before
        desired *= m;
    }
    else
    {
        desired *= m_maxSpeed;
    }

    ngl::Vec3 steer = desired - m_dir;
    steer.clamp(m_maxForce);
    applyForce(steer);
}
//----------------------------------------------------------------------------------------------------------------------

void Boid::separate(std::vector <Boid>& _boidArray)
{
    float desiredSeparation = 3;
    ngl::Vec3 sum;
    int count = 0;

    BOOST_FOREACH(Boid &other, _boidArray)
    {
        float d = (m_pos - other.m_pos).length();

        if((d > 0) && (d < desiredSeparation))
        {
            ngl::Vec3 diff = m_pos - other.m_pos;
            diff.normalize();
            diff /= d;
            sum += diff;
            count++;
        }
    }

    if(count > 0)
    {
        sum /= count;
        sum.normalize();
        sum *= m_maxSpeed;

        ngl::Vec3 steer = sum - m_dir;
        steer.clamp(m_maxSpeed);
        applyForce(steer);
    }
}
//----------------------------------------------------------------------------------------------------------------------

ngl::Vec3 Boid::align(std::vector <Boid>& _boidArray)
{
    float neighbordist = 50;
    ngl::Vec3 sum;
    int count = 0;

    BOOST_FOREACH(Boid &other, _boidArray)
    {
        float d = (m_pos - other.m_pos).length();

        if ((d > 0) && (d < neighbordist))
        {
            sum += other.m_dir;
            count++;
        }
    }

    if(count > 0)
    {
        sum /= count;
        sum.normalize();
        sum *= m_maxSpeed;

        ngl::Vec3 steer = sum - m_dir;   //Reynold's formula for steering force
        steer.clamp(m_maxSpeed);
        return steer;
    }
    else
    {
        return 0;
    }
}
//----------------------------------------------------------------------------------------------------------------------

ngl::Vec3 Boid::cohesion(std::vector <Boid>& _boidArray)
{
    float neighbordist = 50;
    ngl::Vec3 sum;
    int count = 0;
    BOOST_FOREACH(Boid &other, _boidArray)
    {
        float d = (m_pos - other.m_pos).length();

        if ((d > 0) && (d < neighbordist))
        {
        sum += other.m_dir;
        count++;
        }
    }

    if(count > 0)
    {
        sum /= count;
        seek(sum);
    }
    else
    {
        return 0;
    }

}
//----------------------------------------------------------------------------------------------------------------------

void Boid::move()
{
    // store the last position
  m_lastPos=m_pos;
    // update the current position
  m_pos+=m_dir;
    // get the next position
  m_nextPos=m_pos+m_dir;
}
