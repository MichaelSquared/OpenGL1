#ifndef BOID_H__
#define BOID_H__

#include <ngl/Camera.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/Vec3.h>
#include <ngl/VertexArrayObject.h>

/*! \brief a Boid class */

class Boid
{
public :

	/// @brief ctor
    /// @param _pos the position of the boid
    /// @param _dir the direction of the boid
    /// @param _rad the radius of the boid
    Boid(ngl::Vec3 _pos,  ngl::Vec3 _dir, GLfloat _rad); //ctor

    //---------------------Draw_Method--------------------//
    void draw(const std::string &_shaderName, const ngl::Mat4 &_globalMat, ngl::Camera *_cam )const ;
    void loadMatricesToShader(ngl::Transformation &_tx, const ngl::Mat4 &_globalMat, ngl::Camera *_cam )const;
    inline void reverse(){m_dir=m_dir*-1.0;}

    //---------------------Position-----------------------//
    inline ngl::Vec3 getPos() const {return m_pos;}
    inline ngl::Vec3 getNextPos() const {return m_nextPos;}

    //----------------------Radius------------------------//
	inline GLfloat getRadius() const {return m_radius;}

    //---------------------Direction----------------------//
    inline void setDirection(ngl::Vec3 _d){m_dir=_d;}
    inline ngl::Vec3 getDirection() const { return m_dir;}

    //-----------------------Move-------------------------//
	void move();

    //---------------------Behaviors----------------------//
    void separate(std::vector <Boid>& _boidArray);
    ngl::Vec3 align(std::vector <Boid>& _boidArray);
    ngl::Vec3 cohesion(std::vector <Boid>& _boidArray);

    /// set the boid values
    /// @param[in] _pos the position to set
    /// @param[in] _dir the direction of the boid
    /// @param[in] _rad the radius of the boid
    void set(ngl::Vec3 _pos, ngl::Vec3 _dir, GLfloat _rad );
    //void setPosition(ngl::Vec3 _pos) { m_pos = _pos; }                 //???

    //------------------------Color------------------------//
    inline ngl::Colour getBoidColour() {return m_boidColour;}
    inline bool isDiscoBoid() {return m_discoStyle;}

    void seek(ngl::Vec3 target);
    void applyForce(ngl::Vec3 force);
    void update();
    void arrive(ngl::Vec3 target);
    void setup();

private :

    /// the position of the Boid
    ngl::Vec3 m_pos;
    /// the acceleration of the Boid
    ngl::Vec3 m_acceleration;
    /// Maximum speed
    float m_maxSpeed;
    /// Maximum force
    float m_maxForce;
    /// Colour
    ngl::Colour m_boidColour;
    bool m_discoStyle;

    // the radius of the boid
	GLfloat m_radius;
    // the direction of the boid
    ngl::Vec3 m_dir;
    // the last position of the boid
    ngl::Vec3 m_lastPos;
    // the next position of the boid
    ngl::Vec3 m_nextPos;

};




#endif
