#pragma once

#include <la.h>


const glm::vec4 unit_look = glm::vec4(0, 0, 1, 0);
const glm::vec4 unit_up = glm::vec4(0, 1, 0, 0);
const glm::vec4 unit_right = glm::vec4(1, 0, 0, 0);

//A perspective projection camera
//Receives its eye position and reference point from the scene XML file
class Camera
{   

public:
    Camera();
    Camera(unsigned int w, unsigned int h);
    Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp);
    Camera(const Camera &c);

     glm::vec4 unit_eye = glm::vec4(0, 0, 0, 1);

    float fovy;
    unsigned int width, height;  // Screen dimensions
    float near_clip;  // Near clip plane distance
    float far_clip;  // Far clip plane distance

    //Computed attributes
    float aspect;

    glm::vec3 eye,      //The position of the camera in world space
              ref,      //The point in world space towards which the camera is pointing
              look,     //The normalized vector from eye to ref. Is also known as the camera's "forward" vector.
              up,       //The normalized vector pointing upwards IN CAMERA SPACE. This vector is perpendicular to LOOK and RIGHT.
              right,    //The normalized vector pointing rightwards IN CAMERA SPACE. It is perpendicular to UP and LOOK.
              world_up, //The normalized vector pointing upwards IN WORLD SPACE. This is primarily used for computing the camera's initial UP vector.
              V,        //Represents the vertical component of the plane of the viewing frustum that passes through the camera's reference point. Used in Camera::Raycast.
              H;        //Represents the horizontal component of the plane of the viewing frustum that passes through the camera's reference point. Used in Camera::Raycast.

    // Polar Camera
    float zoom;
    float theta;
    float phi;

    void rotateHori_P(float deg);
    void rotateVert_P(float deg);
    void zoom_P(float amt);
    void panHori_P(float amt);
    void panVert_P(float amt);


    glm::mat4 getViewProj();

    void RecomputeAttributes();
};