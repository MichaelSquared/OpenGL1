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
    /// @param pos the position of the boid
    /// @param rad the radius of the boid
    Boid(ngl::Vec3 _pos,  ngl::Vec3 _dir, GLfloat _rad); //ctor
    /// the position of the Boid

    Boid();  //default ctor

    //---------------------Draw_Method--------------------//
    void draw(const std::string &_shaderName, const ngl::Mat4 &_globalMat, ngl::Camera *_cam )const ;
    void loadMatricesToShader(ngl::Transformation &_tx, const ngl::Mat4 &_globalMat, ngl::Camera *_cam )const;
    inline void reverse(){m_dir=m_dir*-1.0;}

    //---------------------Wireframe----------------------//
    inline void setHit(){m_hit=true;}
    inline void setNotHit(){m_hit=false;}
    inline bool isHit()const {return m_hit;}

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

    /// set the boid values
	/// @param[in] _pos the position to set
    /// @param[in] _dir the direction of the boid
    /// @param[in] _rad the radius of the boid

    void set(ngl::Vec3 _pos, ngl::Vec3 _dir, GLfloat _rad );

    inline ngl::Colour getBoidColour()
    {
        return m_boidColour;
    }

    inline bool isDiscoBoid()
    {
        return m_discoStyle;
    }

/************************************************************************/
    void seek(ngl::Vec3 target);
    void applyForce(ngl::Vec3 force);
    void display();
    void arrive(ngl::Vec3 target);
    float mag()
    {
      //return sqrt(m_x*m_x + m_y*m_y + m_z*m_z);
    }
    void setup();
/************************************************************************/

private :

/*******************************************************************************************/
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
/*******************************************************************************************/

    /*! the radius of the boid */
	GLfloat m_radius;
    /*! flag to indicate if the boid has been hit by ray */
    bool m_hit;
    // the direction of the boid
    ngl::Vec3 m_dir;
    // the last position of the boid
    ngl::Vec3 m_lastPos;
    // the next position of the boid
    ngl::Vec3 m_nextPos;

};




#endif
