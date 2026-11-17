// Rigs of Rods - OGRE 14 compatibility helpers (modernization fork)
//
// OGRE 13+ removed position/orientation/direction handling from Ogre::Camera and
// Ogre::Light: both are now plain MovableObjects that get their transform from the
// SceneNode they are attached to. RoR (written for OGRE 1.11) creates cameras and lights
// unattached and moves them directly. These helpers keep the old call style working:
// the first call lazily creates a SceneNode under the root node and attaches the object.
//
// Notes:
//  - Camera and Light look along the node's -Z axis, so directions use NEGATIVE_UNIT_Z.
//  - The lazily created nodes are not destroyed together with the camera/light (small leak,
//    acceptable for the migration; clean up later by owning the nodes explicitly).

#pragma once

#include <OgreCamera.h>
#include <OgreLight.h>
#include <OgreMovableObject.h>
#include <OgreNode.h>
#include <OgreQuaternion.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>

namespace RoR {
namespace OgreCompat {

inline Ogre::SceneNode* EnsureNode(Ogre::MovableObject* obj)
{
    Ogre::SceneNode* node = obj->getParentSceneNode();
    if (node == nullptr)
    {
        node = obj->_getManager()->getRootSceneNode()->createChildSceneNode();
        node->attachObject(obj);
    }
    return node;
}

template <class T> inline void setPosition(T* obj, const Ogre::Vector3& pos)
{
    EnsureNode(obj)->setPosition(pos);
}

template <class T> inline void setPosition(T* obj, Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
    EnsureNode(obj)->setPosition(x, y, z);
}

template <class T> inline void setOrientation(T* obj, const Ogre::Quaternion& q)
{
    EnsureNode(obj)->setOrientation(q);
}

template <class T> inline void setDirection(T* obj, const Ogre::Vector3& dir)
{
    EnsureNode(obj)->setDirection(dir, Ogre::Node::TS_WORLD, Ogre::Vector3::NEGATIVE_UNIT_Z);
}

template <class T> inline void setDirection(T* obj, Ogre::Real x, Ogre::Real y, Ogre::Real z)
{
    setDirection(obj, Ogre::Vector3(x, y, z));
}

template <class T> inline void lookAt(T* obj, const Ogre::Vector3& target)
{
    EnsureNode(obj)->lookAt(target, Ogre::Node::TS_WORLD, Ogre::Vector3::NEGATIVE_UNIT_Z);
}

template <class T> inline void roll(T* obj, const Ogre::Radian& angle)
{
    EnsureNode(obj)->roll(angle);
}

template <class T> inline void setFixedYawAxis(T* obj, bool fixed, const Ogre::Vector3& axis = Ogre::Vector3::UNIT_Y)
{
    EnsureNode(obj)->setFixedYawAxis(fixed, axis);
}

template <class T> inline Ogre::Vector3 getPosition(T* obj)
{
    return EnsureNode(obj)->_getDerivedPosition();
}

template <class T> inline Ogre::Quaternion getOrientation(T* obj)
{
    return EnsureNode(obj)->_getDerivedOrientation();
}

template <class T> inline Ogre::Vector3 getDirection(T* obj)
{
    return EnsureNode(obj)->_getDerivedOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z;
}

} // namespace OgreCompat
} // namespace RoR
