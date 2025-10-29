#include "engine.h"

namespace Engine {
int* gameType;
char* mapName;
char* loadedMapName;
int* gameState;
int* gameTimer;
int* gameTicksSinceReset;
unsigned int* sunTime;

RayCastResult* lineIntersectResult;

Connection* connections;
Account* accounts;
Player* players;
Human* humans;
VehicleType* vehicleTypes;
Vehicle* vehicles;
ItemType* itemTypes;
Item* items;
Bullet* bullets;
Particle* particles;
Bond* bonds;
Building* buildings;
Street* streets;
StreetIntersection* streetIntersections;

unsigned int* numConnections;
unsigned int* numBonds;
unsigned int* numParticles;
unsigned int* numBuildings;
unsigned int* numBullets;
unsigned int* numStreets;
unsigned int* numStreetIntersections;

subRosaPutsFunc subRosaPuts;
subRosa__printf_chkFunc subRosa__printf_chk;

voidFunc resetGame;

voidFunc logicSimulation;
voidFunc logicSimulationRace;
voidFunc logicSimulationRound;
voidFunc logicSimulationWorld;
voidFunc logicSimulationTerminator;
voidFunc logicSimulationCoop;
voidFunc logicSimulationVersus;
voidIndexFunc logicPlayerActions;
voidIndexFunc itemWeaponSimulation;
voidIndexFunc trainSimulation;
voidIndexFunc humanCalculateArmAngles;
voidIndexFunc humanCollideHuman;

voidFunc physicsSimulation;
serverReceiveFunc serverReceive;
voidFunc serverSend;
writePacketFunc writePacket;
sendPacketFunc sendPacket;
voidFunc bulletSimulation;
voidFunc bondSimulation;
voidFunc vehicleSimulation;
voidFunc bulletTimeToLive;
voidFunc economyCarMarket;

voidFunc saveAccountsServer;

createAccountByJoinTicketFunc createAccountByJoinTicket;
serverSendConnectResponseFunc serverSendConnectResponse;

scenarioArmHumanFunc scenarioArmHuman;
linkItemFunc linkItem;
itemSetMemoFunc itemSetMemo;
itemComputerTransmitLineFunc itemComputerTransmitLine;
voidIndexFunc itemComputerIncrementLine;
itemCashAddBillFunc itemCashAddBill;
itemCashRemoveBillFunc itemCashRemoveBill;
itemCashGetBillValueFunc itemCashGetBillValue;
itemComputerInputFunc itemComputerInput;
humanApplyDamageFunc humanApplyDamage;
createEventUpdateHumanFunc createEventUpdateHuman;
createEventUpdateItemFunc createEventUpdateItem;
createEventUpdateItemInfoFunc createEventUpdateItemInfo;
humanCollisionVehicleFunc humanCollisionVehicle;
vehicleApplyDamageFunc vehicleApplyDamage;
voidIndexFunc humanGrabbing;
voidIndexFunc grenadeExplosion;
serverPlayerMessageFunc serverPlayerMessage;
voidIndexFunc playerAI;
voidIndexFunc playerDeathTax;
//createBondRigidBodyToRigidBodyFunc createBondRigidBodyToRigidBody;
//createBondRigidBodyRotRigidBodyFunc createBondRigidBodyRotRigidBody;
//createBondRigidBodyToLevelFunc createBondRigidBodyToLevel;
//addCollisionRigidBodyOnRigidBodyFunc addCollisionRigidBodyOnRigidBody;
//addCollisionRigidBodyOnLevelFunc addCollisionRigidBodyOnLevel;

createPlayerFunc createPlayer;
voidIndexFunc deletePlayer;
createHumanFunc createHuman;
voidIndexFunc deleteHuman;
createItemFunc createItem;
createBulletFunc createBullet;
voidIndexFunc deleteItem;
createRopeFunc createRope;
createVehicleFunc createVehicle;
voidIndexFunc deleteVehicle;
createTrafficFunc createTraffic;
createParticleFunc createParticle;

createEventMessageFunc createEventMessage;
voidIndexFunc createEventUpdatePlayer;
voidIndexFunc createEventUpdatePlayerFinance;
voidIndexFunc createEventCreateVehicle;
createEventUpdateVehicleFunc createEventUpdateVehicle;
createEventSoundFunc createEventSound;
createEventExplosionFunc createEventExplosion;
createEventBulletHitFunc createEventBulletHit;
createEventBulletFunc createEventBullet;

lineIntersectLevelFunc lineIntersectLevel;
lineIntersectHumanFunc lineIntersectHuman;
lineIntersectVehicleFunc lineIntersectVehicle;
lineIntersectTriangleFunc lineIntersectTriangle;
};  // namespace Engine