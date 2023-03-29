#include "camera.h"

#include <la.h>
#include <iostream>


Camera::Camera():
    Camera(400, 400)
{
    look = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    eye(e),
    ref(r),
    world_up(worldUp),

    zoom(5), theta(0), phi(0)
{
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H),

    zoom(c.zoom), theta(c.theta), phi(c.phi)
{}

void Camera::rotateHori_P(float deg) {
    theta += deg;
    RecomputeAttributes();
}

void Camera::rotateVert_P(float deg) {
    phi += deg;
    RecomputeAttributes();
}

void Camera::zoom_P(float amt) {
    zoom += amt;
    RecomputeAttributes();
}

void Camera::panHori_P(float amt) {
    ref += right * glm::vec3(amt);
    eye += right * glm::vec3(amt);
}

void Camera::panVert_P(float amt) {
    ref += up * glm::vec3(amt);
    eye += up * glm::vec3(amt);
}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

void Camera::RecomputeAttributes()
{
    glm::mat4 horiRot_tMat = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0, 1, 0));
    glm::mat4 vertRot_tMat = glm::rotate(glm::mat4(1.0f), glm::radians(phi), glm::vec3(1, 0, 0));
    glm::mat4 zoom_tMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, zoom));

    eye = glm::vec3(horiRot_tMat * vertRot_tMat * zoom_tMat * unit_eye);
    look = glm::normalize(glm::vec3(horiRot_tMat * vertRot_tMat * zoom_tMat * unit_look));
    up = glm::normalize(glm::vec3(horiRot_tMat * vertRot_tMat * zoom_tMat * unit_up));
    right = glm::normalize(glm::vec3(horiRot_tMat * vertRot_tMat * zoom_tMat * unit_right));

    float tan_fovy = tan(glm::radians(fovy/2));
    float len = glm::length(ref - eye);
    aspect = width / static_cast<float>(height);
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
}
