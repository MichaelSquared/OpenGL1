#ifndef FLOCK_H
#define FLOCK_H
#include <ngl/Camera.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/Vec3.h>
#include <ngl/VertexArrayObject.h>
#include <ngl/BBox.h>
#include "Boid.h"


class Flock
{
public:
    Flock( int _numOfBoids, float _extents )
        : m_numBoids(_numOfBoids)
    {
        m_numBoids = _numOfBoids;
        s_extents = _extents;
    }

    ~Flock() {};

    ngl::Vec3 getAveragePos();
    std::vector <Boid> m_boidArray;

    void resetBoids();
    void setup();
    void draw(ngl::Mat4 _mouseGlobalTX, ngl::Camera *_cam, ngl::ShaderLib* shader);
    void update(ngl::BBox *_bbox, bool _checkBoidBoid);

    void  checkCollisions(ngl::BBox *_bbox, bool _checkBoidBoid);
    void  checkBoidCollisions();
    void  BBoxCollision(ngl::BBox *_bbox);
    bool  boidBoidCollision( ngl::Vec3 _pos1, GLfloat _radius1, ngl::Vec3 _pos2, GLfloat _radius2 );

    void removeBoid();
    void addBoid();

private:
    float s_extents;
    int m_numBoids;
};



#endif // FLOCK_H


//appent to a list = .push_back something
