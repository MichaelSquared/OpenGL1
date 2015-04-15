#include "Flock.h"
#include "Boid.h"

#include <ngl/ShaderLib.h>
#include "ngl/Random.h"
#include <ngl/Material.h>
#include <ngl/BBox.h>
#include <boost/foreach.hpp>
#include <iostream>

void Flock::resetBoids()
{
    std::vector<Boid>::iterator begin=m_boidArray.begin();
    std::vector<Boid>::iterator end=m_boidArray.end();
    m_boidArray.erase(begin,end);
    ngl::Vec3 pos(0,0,0);
    ngl::Vec3 dir;
    ngl::Random *rng=ngl::Random::instance();
    // loop and create the initial particle list
    for(int i=0; i<m_numBoids; ++i)
    {
        dir=rng->getRandomVec3();
        // add the boids to the end of the particle list
        m_boidArray.push_back(Boid(rng->getRandomPoint(s_extents,
                                                       s_extents,
                                                       s_extents),
                                   dir,
                                   rng->randomPositiveNumber(2)+0.5));
    }
}
//----------------------------------------------------------------------------------------------------------------------

void Flock::draw(ngl::Mat4 _mouseGlobalTX, ngl::Camera *_cam, ngl::ShaderLib* shader)
{
    BOOST_FOREACH(Boid s, m_boidArray)
    {
        //const bool fullbright = true;
        const bool fullbright = s.isDiscoBoid() ;

        ngl::Random* rand = ngl::Random::instance();
        ngl::Colour boidColour = s.isDiscoBoid() ? rand->getRandomColour() : s.getBoidColour();

        if(fullbright)
        {
            shader->setShaderParam4f("Colour", boidColour.m_r, boidColour.m_g, boidColour.m_b, 1); // Set shader colour
            s.draw("nglColourShader",_mouseGlobalTX, _cam);
        }
        else
        {
            shader->setShaderParam4f("material.diffuse", boidColour.m_r, boidColour.m_g, boidColour.m_b, 1); // Set shader colour
            s.draw("Phong",_mouseGlobalTX, _cam);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

void Flock::update(ngl::BBox *_bbox, bool _checkBoidBoid, ngl::Vec3 _seekLocation)
{
//    static float time = 0;
//    time+= 0.15f;
//    m_seekLocation = 16 * ngl::Vec3(cos(time * 4), sin(time), tan(time * 2));

    m_seekLocation = _seekLocation;

    checkCollisions(_bbox, false);

//    ngl::Random* rand = ngl::Random::instance();
//    if((averagePosition - m_seekLocation).lengthSquared() < 32)
//    {
//        m_seekLocation = rand->getRandomNormalizedVec3() * 32;
//    }

//    const ngl::Vec3 seekLocation(16, 16, 4);
    BOOST_FOREACH(Boid &boid, m_boidArray)
    {
        boid.arrive(m_seekLocation);
        boid.separate(m_boidArray);
        boid.align(m_boidArray);
        boid.cohesion(m_boidArray);
        boid.update();
    }
}
//----------------------------------------------------------------------------------------------------------------------

void Flock::checkCollisions(ngl::BBox *_bbox, bool _checkBoidBoid)
{

    if(_checkBoidBoid == true)
    {
        checkBoidCollisions();
    }
    BBoxCollision(_bbox);
}
//----------------------------------------------------------------------------------------------------------------------

void Flock::checkBoidCollisions()
{
  bool collide;

  unsigned int size=m_boidArray.size();

    for(unsigned int ToCheck=0; ToCheck<size; ++ToCheck)
    {
        for(unsigned int Current=0; Current<size; ++Current)
        {
            // don't check against self
            if(ToCheck == Current)  continue;

//      else
//      {
//        //cout <<"doing check"<<endl;
//        collide = boidBoidCollision(m_boidArray[Current].getPos(),m_boidArray[Current].getRadius(),
//                                       m_boidArray[ToCheck].getPos(),m_boidArray[ToCheck].getRadius()
//                                      );
        if(collide == true)
        {
          m_boidArray[Current].reverse();
        }
      }
    }
  }
//}
//----------------------------------------------------------------------------------------------------------------------

void Flock::BBoxCollision(ngl::BBox *_bbox)
{
  //create an array of the extents of the bounding box
  float ext[6];
  ext[0]=ext[1]=(_bbox->height()/2.0f);
  ext[2]=ext[3]=(_bbox->width()/2.0f);
  ext[4]=ext[5]=(_bbox->depth()/2.0f);
  // Dot product needs a Vector so we convert The Point Temp into a Vector so we can
  // do a dot product on it
  ngl::Vec3 p;
  // D is the distance of the Agent from the Plane. If it is less than ext[i] then there is
  // no collision
  GLfloat D;
  // Loop for each boid in the vector list
  BOOST_FOREACH(Boid &s, m_boidArray)
  {
    p=s.getPos();
    //Now we need to check the Boid agains all 6 planes of the BBOx
    //If a collision is found we change the dir of the Boid then Break
    for(int i=0; i<6; ++i)
    {
      //to calculate the distance we take the dotporduct of the Plane Normal
      //with the new point P
      D=_bbox->getNormalArray()[i].dot(p);
      //Now Add the Radius of the boid to the offsett
      D+=s.getRadius();
      // If this is greater or equal to the BBox extent /2 then there is a collision
      //So we calculate the Boids new direction
      if(D >=ext[i])
      {
        //We use the same calculation as in raytracing to determine
        // the new direction
        GLfloat x= 2*( s.getDirection().dot((_bbox->getNormalArray()[i])));
        ngl::Vec3 d =_bbox->getNormalArray()[i]*x;
        s.setDirection(s.getDirection()-d);
//        s.setHit();
      }//end of hit test
     }//end of each face test

    // Quick fix to reset particles that escape the bounding box
//    if( !((ext[0] > s.getPos().m_x) && (-ext[0] < s.getPos().m_x)) &&
//         ((ext[2] > s.getPos().m_y) && (-ext[2] < s.getPos().m_y)) &&
//         ((ext[4] > s.getPos().m_z) && (-ext[4] < s.getPos().m_z)) )
//    {
//        s.setPosition( ngl::Vec3(0,0,0) );
//    }

    }//end of for
}
//----------------------------------------------------------------------------------------------------------------------

//bool Flock::boidBoidCollision( ngl::Vec3 _pos1, GLfloat _radius1, ngl::Vec3 _pos2, GLfloat _radius2 )
//{
//  // the relative position of the boids
//  ngl::Vec3 relPos;
//  //min an max distances of the boids
//  GLfloat dist;
//  GLfloat minDist;
//  GLfloat len;
//  relPos =_pos1-_pos2;
//  // and the distance
//  len=relPos.length();
//  dist=len*len;
//  minDist =_radius1+_radius2;
//  if(dist <=(minDist * minDist))
//  {
//    return true;
//  }
//  else
//  {
//    return false;
//  }
//}
//----------------------------------------------------------------------------------------------------------------------

void Flock::removeBoid()
{
  std::vector<Boid>::iterator end=m_boidArray.end();
  if( --m_numBoids == 0)
  {
    m_numBoids=1;
  }
  else
  {
    m_boidArray.erase(end-1,end);
  }
}
//----------------------------------------------------------------------------------------------------------------------

void Flock::addBoid()
{
  ngl::Random *rng=ngl::Random::instance();
  ngl::Vec3 dir;
  dir=rng->getRandomVec3();
  // add the boids to the end of the particle list
  m_boidArray.push_back(Boid(rng->getRandomPoint(s_extents,s_extents,s_extents),dir,rng->randomPositiveNumber(2)+0.5));
  ++m_numBoids;
}
//----------------------------------------------------------------------------------------------------------------------
///------------Average_Posititon_of_the_boids-------------//
ngl::Vec3 Flock::getAveragePos()
{

    for(unsigned int i = 0; i < m_boidArray.size(); i++)
    {
        averagePosition += m_boidArray[i].getPos();
    }

    averagePosition = averagePosition / m_boidArray.size();
    return averagePosition;
}
//----------------------------------------------------------------------------------------------------------------------



