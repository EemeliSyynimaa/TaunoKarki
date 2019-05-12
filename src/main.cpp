#include "mesh.cpp"
#include "shaderprogram.cpp"
#include "texture.cpp"
#include "aicontroller.cpp"
#include "ammobar.cpp"
#include "boxcollider.cpp"
#include "circlecollider.cpp"
#include "collectible.cpp"
#include "health.cpp"
#include "healthbar.cpp"
#include "menubutton.cpp"
#include "meshrenderer.cpp"
#include "playercontroller.cpp"
#include "rigidbody.cpp"
#include "staticbody.cpp"
#include "transform.cpp"
#include "assetmanager.cpp"
#include "audio.cpp"
#include "camera.cpp"
#include "collisionhandler.cpp"
#include "component.cpp"
#include "game.cpp"
#include "gameobject.cpp"
#include "gameobjectmanager.cpp"
#include "locator.cpp"
#include "lodepng.cpp"
#include "scene.cpp"
#include "scenemanager.cpp"
#include "tilemap.cpp"
#include "gamescene.cpp"
#include "menuscene.cpp"
#include "machinegun.cpp"
#include "pistol.cpp"
#include "shotgun.cpp"

#include "Box2D/Collision/Shapes/b2ChainShape.cpp"
#include "Box2D/Collision/Shapes/b2CircleShape.cpp"
#include "Box2D/Collision/Shapes/b2EdgeShape.cpp"
#include "Box2D/Collision/Shapes/b2PolygonShape.cpp"
#include "Box2D/Collision/b2BroadPhase.cpp"
#include "Box2D/Collision/b2Collision.cpp"
#include "Box2D/Collision/b2CollideCircle.cpp"
#include "Box2D/Collision/b2CollideEdge.cpp"
#include "Box2D/Collision/b2CollidePolygon.cpp"
#include "Box2D/Collision/b2Distance.cpp"
#include "Box2D/Collision/b2DynamicTree.cpp"
#include "Box2D/Collision/b2TimeOfImpact.cpp"
#include "Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
#include "Box2D/Dynamics/Contacts/b2CircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2Contact.cpp"
#include "Box2D/Dynamics/Contacts/b2ContactSolver.cpp"
#include "Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
#include "Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2PolygonContact.cpp"
#include "Box2D/Dynamics/Joints/b2DistanceJoint.cpp"
#include "Box2D/Dynamics/Joints/b2FrictionJoint.cpp"
#include "Box2D/Dynamics/Joints/b2GearJoint.cpp"
#include "Box2D/Dynamics/Joints/b2Joint.cpp"
#include "Box2D/Dynamics/Joints/b2MotorJoint.cpp"
#include "Box2D/Dynamics/Joints/b2MouseJoint.cpp"
#include "Box2D/Dynamics/Joints/b2PrismaticJoint.cpp"
#include "Box2D/Dynamics/Joints/b2PulleyJoint.cpp"
#include "Box2D/Dynamics/Joints/b2RevoluteJoint.cpp"
#include "Box2D/Dynamics/Joints/b2RopeJoint.cpp"
#include "Box2D/Dynamics/Joints/b2WeldJoint.cpp"
#include "Box2D/Dynamics/Joints/b2WheelJoint.cpp"
#include "Box2D/Dynamics/b2Body.cpp"
#include "Box2D/Dynamics/b2ContactManager.cpp"
#include "Box2D/Dynamics/b2Fixture.cpp"
#include "Box2D/Dynamics/b2Island.cpp"
#include "Box2D/Dynamics/b2World.cpp"
#include "Box2D/Dynamics/b2WorldCallbacks.cpp"
#include "Box2D/Rope/b2Rope.cpp"
#include "Box2D/Common/b2BlockAllocator.cpp"
#include "Box2D/Common/b2Draw.cpp"
#include "Box2D/Common/b2Math.cpp"
#include "Box2D/Common/b2Settings.cpp"
#include "Box2D/Common/b2StackAllocator.cpp"
#include "Box2D/Common/b2Timer.cpp"

int main(int argc, char** argv)
{
    Game().run();

    return 0;
}