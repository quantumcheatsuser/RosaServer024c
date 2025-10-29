#pragma once
#include "structs.h"

namespace Engine {
extern int* gameType;
extern char* mapName;
extern char* loadedMapName;
extern int* gameState;
extern int* gameTimer;
extern int* gameTicksSinceReset;
extern unsigned int* sunTime;

extern RayCastResult* lineIntersectResult;

extern Connection* connections;
extern Account* accounts;
extern Player* players;
extern Human* humans;
extern VehicleType* vehicleTypes;
extern Vehicle* vehicles;
extern ItemType* itemTypes;
extern Item* items;
extern Bullet* bullets;
extern Particle* particles;
extern Bond* bonds;
extern Building* buildings;
extern Street* streets;
extern StreetIntersection* streetIntersections;

extern unsigned int* numConnections;
extern unsigned int* numBullets;
extern unsigned int* numBonds;
extern unsigned int* numParticles;
extern unsigned int* numBuildings;
extern unsigned int* numStreets;
extern unsigned int* numStreetIntersections;

/*
  Misc
*/

typedef int (*subRosaPutsFunc)(const char* str);
extern subRosaPutsFunc subRosaPuts;
typedef int (*subRosa__printf_chkFunc)(int flag, const char* format, ...);
extern subRosa__printf_chkFunc subRosa__printf_chk;

typedef void (*voidFunc)();
typedef void (*voidIndexFunc)(int id);

extern voidFunc resetGame;
typedef void (*createTrafficFunc)(int amount);
extern createTrafficFunc createTraffic;

extern voidFunc logicSimulation;
extern voidFunc logicSimulationRace;
extern voidFunc logicSimulationRound;
extern voidFunc logicSimulationWorld;
extern voidFunc logicSimulationTerminator;
extern voidFunc logicSimulationCoop;
extern voidFunc logicSimulationVersus;
extern voidIndexFunc logicPlayerActions;
extern voidIndexFunc itemWeaponSimulation;
extern voidIndexFunc trainSimulation;
extern voidIndexFunc humanCalculateArmAngles;
extern voidIndexFunc humanCollideHuman;

extern voidFunc physicsSimulation;
typedef int (*serverReceiveFunc)();
extern serverReceiveFunc serverReceive;
extern voidFunc serverSend;
extern voidFunc bulletSimulation;
extern voidFunc bondSimulation;
extern voidFunc vehicleSimulation;
extern voidFunc bulletTimeToLive;
extern voidFunc economyCarMarket;

extern voidFunc saveAccountsServer;

typedef int (*createAccountByJoinTicketFunc)(int identifier,
                                             unsigned int ticket);
extern createAccountByJoinTicketFunc createAccountByJoinTicket;
typedef void (*serverSendConnectResponseFunc)(unsigned int address, unsigned int port, const char* message);
extern serverSendConnectResponseFunc serverSendConnectResponse;
typedef void (*writePacketFunc)(int connectionID, int playerID);
extern writePacketFunc writePacket;
typedef void (*sendPacketFunc)(unsigned int address, unsigned int port);
extern sendPacketFunc sendPacket;

typedef void (*scenarioArmHumanFunc)(int human, int weapon, int magCount);
extern scenarioArmHumanFunc scenarioArmHuman;

typedef int (*linkItemFunc)(int itemID, int childItemID, int parentHumanID,
                            int slot);
extern linkItemFunc linkItem;

typedef int (*itemSetMemoFunc)(int itemID, const char* memo);
extern itemSetMemoFunc itemSetMemo;
typedef int (*itemComputerTransmitLineFunc)(int itemID, unsigned int line);
extern itemComputerTransmitLineFunc itemComputerTransmitLine;
extern voidIndexFunc itemComputerIncrementLine;
typedef int (*itemComputerInputFunc)(int itemID, unsigned int character);
extern itemComputerInputFunc itemComputerInput;

typedef int (*itemCashAddBillFunc)(int itemID, int zero, int amount);
extern itemCashAddBillFunc itemCashAddBill;
typedef void (*itemCashRemoveBillFunc)(int itemID, int amount);
extern itemCashRemoveBillFunc itemCashRemoveBill;
typedef int (*itemCashGetBillValueFunc)(int itemID);
extern itemCashGetBillValueFunc itemCashGetBillValue;

typedef void (*createEventUpdateHumanFunc)(int humanID);
extern voidIndexFunc createEventUpdateHuman;

typedef void (*createEventUpdateItemFunc)(int itemID);
extern voidIndexFunc createEventUpdateItem;

typedef void (*createEventUpdateItemInfoFunc)(int itemID);
extern voidIndexFunc createEventUpdateItemInfo;

typedef void (*humanApplyDamageFunc)(int humanID, int bone, int unk,
                                     int damage);
extern humanApplyDamageFunc humanApplyDamage;

typedef void (*humanCollisionVehicleFunc)(int humanID, int vehicleID);
extern humanCollisionVehicleFunc humanCollisionVehicle;

typedef void (*vehicleApplyDamageFunc)(int vehicleID, int damage);
extern vehicleApplyDamageFunc vehicleApplyDamage;

extern voidIndexFunc humanGrabbing;

extern voidIndexFunc grenadeExplosion;

typedef int (*serverPlayerMessageFunc)(int playerID, char* message);
extern serverPlayerMessageFunc serverPlayerMessage;
extern voidIndexFunc playerAI;
extern voidIndexFunc playerDeathTax;

typedef int (*createBondRigidBodyToRigidBodyFunc)(int aBodyID, int bBodyID,
                                                  Vector* aLocalPos,
                                                  Vector* bLocalPos);
extern createBondRigidBodyToRigidBodyFunc createBondRigidBodyToRigidBody;
typedef int (*createBondRigidBodyRotRigidBodyFunc)(int aBodyID, int bBodyID);
extern createBondRigidBodyRotRigidBodyFunc createBondRigidBodyRotRigidBody;
typedef int (*createBondRigidBodyToLevelFunc)(int bodyID, Vector* localPos,
                                              Vector* globalPos);
extern createBondRigidBodyToLevelFunc createBondRigidBodyToLevel;

//typedef void (*addCollisionRigidBodyOnRigidBodyFunc)(int aBodyID, int bBodyID, Vector* aLocalPos, Vector* bLocalPos, Vector* normal, float, float, float, float);
//extern addCollisionRigidBodyOnRigidBodyFunc addCollisionRigidBodyOnRigidBody;
//
//typedef void (*addCollisionRigidBodyOnLevelFunc)(int bodyID, Vector* localPos, Vector* normal, float, float, float, float);
//extern addCollisionRigidBodyOnLevelFunc addCollisionRigidBodyOnLevel;

/*
  Object Handling
*/

typedef int (*createPlayerFunc)();
extern createPlayerFunc createPlayer;
extern voidIndexFunc deletePlayer;

typedef int (*createHumanFunc)(Vector* pos, RotMatrix* rot, int playerID);
extern createHumanFunc createHuman;
extern voidIndexFunc deleteHuman;

typedef int (*createItemFunc)(int type, Vector* pos, Vector* vel,
                              RotMatrix* rot);
extern createItemFunc createItem;
extern voidIndexFunc deleteItem;

typedef int (*createRopeFunc)(Vector* pos, RotMatrix* rot);
extern createRopeFunc createRope;

typedef int (*createBulletFunc)(int bulletType, Vector* pos, Vector* vel, int playerID);
extern createBulletFunc createBullet;

typedef int (*createVehicleFunc)(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color);
extern createVehicleFunc createVehicle;
extern voidIndexFunc deleteVehicle;

typedef int (*createParticleFunc)(int unk, int type, Vector* pos, Vector* vel, int veh);
extern createParticleFunc createParticle;

/*
  Events
*/

typedef void (*createEventMessageFunc)(int type, char* message, int speakerID,
                                       int distance);
extern createEventMessageFunc createEventMessage;
// Sends team, active, isBot, humanID, skinColor, hair, gender, head, necklace,
// eyeColor, tieColor, suitColor, shirtColor, hairColor, name
extern voidIndexFunc createEventUpdatePlayer;
// Sends money, stocks, phoneNumber
extern voidIndexFunc createEventUpdatePlayerFinance;
// Sends active, type, parentHumanID, parentItemID, parentSlot
extern voidIndexFunc createevent_updateitem;
// Sends type, color
extern voidIndexFunc createEventCreateVehicle;
/*
updateType:
0 = window
1 = tire
2 = body
*/
typedef void (*createEventUpdateVehicleFunc)(int vehicleID, int updateType,
                                             int partID, Vector* pos,
                                             Vector* normal);
extern createEventUpdateVehicleFunc createEventUpdateVehicle;
typedef void (*createEventSoundFunc)(int soundType, Vector* pos, float volume,
                                     float pitch);
extern createEventSoundFunc createEventSound;
typedef void (*createEventExplosionFunc)(int type, Vector* pos);
extern createEventExplosionFunc createEventExplosion;
/*
hitType:
0 = bullet hole
1 = hit body
2 = hit car
3 = blood drip
*/
typedef void (*createEventBulletHitFunc)(int unk, int hitType, Vector* pos, Vector* normal);
extern createEventBulletHitFunc createEventBulletHit;

typedef void (*createEventBulletFunc)(int bulletType, Vector* pos, Vector* vel, int itemID);
extern createEventBulletFunc createEventBullet;

/*
  Math
*/

typedef int (*lineIntersectLevelFunc)(Vector* posA, Vector* posB);
extern lineIntersectLevelFunc lineIntersectLevel;

typedef int (*lineIntersectHumanFunc)(int humanID, Vector* posA, Vector* posB);
extern lineIntersectHumanFunc lineIntersectHuman;

typedef int (*lineIntersectVehicleFunc)(int vehicleID, Vector* posA,
                                        Vector* posB);
extern lineIntersectVehicleFunc lineIntersectVehicle;

typedef int (*lineIntersectTriangleFunc)(Vector* outPos, Vector* normal,
                                         float* outFraction, Vector* posA,
                                         Vector* posB, Vector* triA,
                                         Vector* triB, Vector* triC);
extern lineIntersectTriangleFunc lineIntersectTriangle;
};  // namespace Engine