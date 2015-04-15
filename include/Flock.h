#ifndef FLOCK_H
#define FLOCK_H
#include <ngl/Camera.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/Vec3.h>
#include <ngl/VertexArrayObject.h>
#include <ngl/BBox.h>
#include <ngl/Material.h>
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

    ~Flock() {}; //dctor

    ngl::Vec3 getAveragePos();
    std::vector <Boid> m_boidArray;  //boid arrayList
    ngl::Vec3 m_seekLocation;

    ///reset the boid array
    void resetBoids();
    void setup();
    void flock();

    void draw(ngl::Mat4 _mouseGlobalTX, ngl::Camera *_cam, ngl::ShaderLib* shader);
    void update(ngl::BBox *_bbox, bool _checkBoidBoid, ngl::Vec3 _seekLocation);

    /// check the collisions
    void  checkCollisions(ngl::BBox *_bbox, bool _checkBoidBoid);

    /// check the boid collisions
    void  checkBoidCollisions();

    /// @brief check the bounding box collisions
    void  BBoxCollision(ngl::BBox *_bbox);

    void removeBoid();
    void addBoid();
    void move();
    void applyBehaviors(std::vector <Boid> m_boidArray);

private:
    ngl::Vec3 averagePosition;
    float s_extents;
    int m_numBoids;  //number of boids
};



#endif // FLOCK_H


//appent to a list = .push_back
