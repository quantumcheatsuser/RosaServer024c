#pragma once

#include <cstring>
#include <string>
#include "sol/sol.hpp"

static constexpr int maxNumberOfAccounts = 32768;
static constexpr int maxNumberOfPlayers = 256;
static constexpr int maxNumberOfHumans = 256;
static constexpr int maxNumberOfItemTypes = 27;
static constexpr int maxNumberOfItems = 1024;
static constexpr int maxNumberOfVehicleTypes = 14;
static constexpr int maxNumberOfVehicles = 1024;
static constexpr int maxNumberOfRigidBodies = 8192;
static constexpr int maxNumberOfParticles = 16384;
static constexpr int maxNumberOfBonds = 16384;
static constexpr int maxNumberOfBuildings = 128;

/*
  Event types:
  0x1		1	bullethit
  0x2		2	message
  0x3		3	createVehicle (vehicle)
  0x4		4	updateobject
  0x5		5	updateitem
  0x7		7	updateplayer
  0x8		8	updateplayer_finance
  0x9		9	sound
  0xA		10	updatedoor
  0x11	17	updatehuman
  0x14	20	explosion
*/

struct Player;
struct Human;
struct Vehicle;
struct Item;
struct Particle;
struct Bond;
struct StreetIntersection;
struct Event;
struct TrafficCar;
struct RotMatrix;



struct Vector {
	float x, y, z;

	const char* getClass() const { return "Vector"; }
	std::string __tostring() const;
	Vector __add(Vector* other) const;
	Vector __sub(Vector* other) const;
	bool __eq(Vector* other) const;
	Vector __mul(float scalar) const;
	Vector __mul_RotMatrix(RotMatrix* rot) const;
	Vector __div(float scalar) const;
	Vector __unm() const;
	void add(Vector* other);
	void mult(float scalar);
	void set(Vector* other);
	void cross(Vector* other);
	Vector clone() const;
	float dist(Vector* other) const;
	float distSquare(Vector* other) const;
	double length() const;
	double lengthSquare() const;
	double dot(Vector* other) const;
	Vector* normalize();
	std::tuple<int, int, int> getBlockPos() const;
};

struct RotMatrix {
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;

	const char* getClass() const { return "RotMatrix"; }
	std::string __tostring() const;
	RotMatrix __mul(RotMatrix* other) const;
	void set(RotMatrix* other);
	RotMatrix clone() const;
	Vector getForward() const;
	Vector getUp() const;
	Vector getRight() const;
	Vector realForward() const;
};

// 32 bytes (20)
struct EarShot {
	int active;
	int playerID;            // 04
	int humanID;             // 08
	int receivingItemID;     // 0c
	int transmittingItemID;  // 10
	int unk0;                // 14
	float distance;          // 20
	float volume;            // 24

	const char* getClass() const { return "EarShot"; }
	bool getIsActive() const { return active; }
	void setIsActive(bool b) { active = b; }

	Player* getPlayer() const;
	void setPlayer(Player* player);
	Human* getHuman() const;
	void setHuman(Human* human);
	Item* getReceivingItem() const;
	void setReceivingItem(Item* item);
	Item* getTransmittingItem() const;
	void setTransmittingItem(Item* item);
};

// 188888 bytes (2E1D8)
struct Connection {
	unsigned int address;
	unsigned int port;      // 04
	int roundNumber;        // 08
	int adminVisible;       // 0c
	int playerID;           // 10
	char PAD_0[8];         // 158
	//int unk1;               // 14
	//int bandwidth;          // 18
	int timeoutTime;        // 1c
	char PAD_1[36];         // 158
	int numReceivedEvents;  // 1c
	char PAD_2[12];         // 158
	EarShot earShots[8];    // 1c
	int spectatingHumanID;  // 1c
	char PAD_3[61440];      // 158
	Vector headPos;         // f158
	Vector cameraPos;       // f164

	const char* getClass() const { return "Connection"; }
	std::string getAddress();
	bool getAdminVisible() const { return adminVisible; }
	void setAdminVisible(bool b) { adminVisible = b; }
	Player* getPlayer() const;
	void setPlayer(Player* player);
	EarShot* getEarShot(unsigned int idx);
	Human* getSpectatingHuman() const;
	Vector getCameraPosition() const { return cameraPos; }
	Vector getHeadPosition() const { return headPos; }
	//bool hasReceivedEvent(Event* event) const;
};

// 112 bytes (70)
struct Account {
	int unk0;
	int unk1;             // 04
	int token;            // 08
	int token2;           // 0c
	char name[32];        // 10
	int unk2;             // 30
	int money;            // 34
	int playTime;        // 38
	int eyeColor;         // 3c
	int skinColor;        // 40
	int hairColor;        // 44
	int unk3;             // 48

	const char* getClass() const { return "Account"; }
	std::string __tostring() const;
	int getIndex() const;
	char* getName() { return name; }
};

struct RayCastResult {
	Vector pos;
	Vector normal;    // 0c
	float fraction;   // 18
	float unk0;       // 1c
	int unk1;         // 20
	int unk2;         // 24
	int unk3;         // 28
	int unk4;         // 2c
	int vehicleFace;  // 30
	int humanBone;    // 34
	int unk6;         // 38
	int unk7;         // 3c
	int unk8;         // 40
	int unk9;         // 44
	int unk10;        // 48
	int unk11;        // 4c
	int unk12;        // 50
	int unk13;        // 54
	int blockX;       // 58
	int blockY;       // 5c
	int blockZ;       // 60
	int unk17;        // 64
	int unk18;        // 68
	int matMaybe;     // 6c
	int unk20;        // 70
	int unk21;        // 74
	int unk22;        // 78
	int unk23;        // 7c
	int unk24;        // 80
};

// Forward decl
struct Human;
struct Vehicle;
struct Item;
struct Particle;
struct Bond;
struct StreetIntersection;

// 84 bytes (54)
struct Action {
	int type;
	int a;
	int b;
	int c;
	int d;
	char text[64];
	char* getText() { return text; }
	void setText(const char* newMessage) { std::strncpy(text, newMessage, 64); }
	const char* getClass() const { return "Action"; }
};

// 72 bytes (48)
struct MenuButton {
	int id;
	char text[64];
	int unk;

	const char* getClass() const { return "MenuButton"; }
	char* getText() { return text; }
	void setText(const char* newText) { std::strncpy(text, newText, 63); }
};

// 9224 bytes (0x2408)
struct Player {
	int active;
	char name[32];                   // 04
	int token;                       // 24
	int token2;                      // 28
	unsigned int isAdmin;            // 2c
	unsigned int adminAttempts;      // 30
	unsigned int accountID;          // 34
	char unk1[4];                    // 38
	int isReady;                     // 3C
	int money;                       // 40
	char unk2[8];                    // 44
	int itemsBought;                 // 4C
	int vehiclesBought;              // 50
	int withdrawnBills;              // 54
	char unk3[12];                   // 44
	unsigned int team;               // 64
	unsigned int teamSwitchTimer;    // 68
	int stocks;                      // 6C
	int unk4[2];					 // 70
	int humanID;                     // 78
	float gearX;                     // 7c
	float leftRightInput;            // 80
	float gearY;                     // 84
	float forwardBackInput;          // 88
	float viewYaw;                   // 8c
	float viewPitch;                 // 90
	float freeLookYaw;               // 94
	float freeLookPitch;             // 98
	int inputFlags;                  // 9c
	int lastInputFlags;              // a0
	int unk5[2];                    // a4
	int zoomLevel;                   // ac
	char unk6[20];                   // b0
	int isCrouching;                 // c4
	int inputType;                   // c8
	int unk7[2];
	// 0 = none, 1-19 = shop, 2X = base || Not sure if accurate to 24.
	int menuTab;  // d4
	int menuTabBuildingID;  // d8
	char unk8_1[8];
	int numActions;      // e4
	int lastNumActions;  // e8
	Action actions[64];  // ec
	int health;          // 15ec || Networked only? Unsure.
	char unk9[1096];     // 15f0
	int isBot;           // 1a38
	int botDriving;      // 1a38
	char unk9_1[2432];   // 1a40
	int suitColor;       // 23c0
	int sunglasses;      // 23c4
	char unk10[64];
	const char* getClass() const { return "Player"; }
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const { return active; }
	void setIsActive(bool b) { active = b; }
	sol::table getDataTable() const;
	char* getName() { return name; }
	void setName(const char* newName) { std::strncpy(name, newName, 31); }
	bool getIsAdmin() const { return isAdmin; }
	void setIsAdmin(bool b) { isAdmin = b; }
	bool getIsBotDriving() const { return botDriving; }
	void setIsBotDriving(bool b) { botDriving = b; }
	bool getIsReady() const { return isReady; }
	void setIsReady(bool b) { isReady = b; }
	bool getIsBot() const { return isBot; }
	void setIsBot(bool b) { isBot = b; }
	Human* getHuman();
	void setHuman(Human* human);
	Connection* getConnection();
	Account* getAccount();
	void setAccount(Account* account);
	const Vector* getBotDestination() const;
	void setBotDestination(Vector* vec);
	Action* getAction(unsigned int idx);
	MenuButton* getMenuButton(unsigned int idx);

	void update() const;
	void updateFinance() const;
	void remove() const;
	void sendMessage(const char* message) const;
};

// 224 bytes (e0)
struct Bone {
	Vector pos;      // 04
	Vector pos2;     // 10
	Vector vel;      // 1c
	int unk0;        // 28
	int unk1;        // 2c
	int unk2;        // 30
	RotMatrix rot;   // 34
	RotMatrix rotVel;// 58
	float unk4;      // 78
	float unk5;      // 7c
	Vector unk6;     // 80
	float mass;      // 8c
	Vector size;     // 90
	Vector size2;    // 9c
	RotMatrix unk7;  // a8
	float unk8;      // cc
	Vector unk9;     // d0
	int bondID;      // dc

	const char* getClass() const { return "Bone"; }
};

// 40 bytes (28)
struct InventorySlot {
	int count;
	int primaryItemID;
	int secondaryItemID;
	char PAD[24];

	const char* getClass() const { return "InventorySlot"; }
	Item* getPrimaryItem() const;
	Item* getSecondaryItem() const;
};

// 11108 bytes (2b64)
struct Human {
	int active;
	int physicsSim;                 // 04
	int playerID;                   // 08
	int accountID;                  // 0c
	int unk1;                       // 10
	int vehicleID;                  // 14
	int vehicleSeat;                // 18
	int lastVehicleID;              // 1c
	int lastVehicleCooldown;        // 20
	// counts down after death
	unsigned int despawnTime;       // 24
	int health;                     // 28
	int unk2;                       // 2c
	int vehicleExitTimer;           // 30
	unsigned int spawnProtection;   // 34
	int unk3;                       // 38
	int zoomLevel;                  // 3c
	int unk4;                       // 40
	int unk5;                       // 44
	Vector pos;                     // 48
	char unk6[12];                  // 54
	/*
	0=normal
	1=jumping/falling
	2=sliding
	5=getting up?
	*/
	float viewYaw2;            // 60
	float viewPitch2;          // 64
	char PAD_2[52]; 	       // 68
	float gearX;               // 9c
	float strafeInput;         // a0
	float gearY;               // a4
	float walkInput;           // a8
	float viewYaw;             // ac
	float viewPitch;           // b0
	float freeLookYaw;         // b4
	float freeLookPitch;       // b8
	/*
		mouse1 = 1		1 << 0
		mouse2 = 2		1 << 1
		space = 4		1 << 2
		ctrl = 8		1 << 3
		shift = 16		1 << 4

		Q = 32			1 << 5
		e = 2048		1 << 11
		r = 4096		1 << 12
		f = 8192		1 << 13

		del = 262144	1 << 18
		z = 524288		1 << 19
	*/
	unsigned int inputFlags;          // bc
	unsigned int lastInputFlags;      // c0
	int unk7;                         // c4
	Bone bones[15];                   // c8
	char PAD_4[6964];                 // de8
	InventorySlot inventorySlots[6];  // 291c
	char PAD_5[296];                  // 29f4
	int isBleeding;                   // 2b1c
	char PAD_6[68];                   // 2b20

	const char* getClass() const { return "Human"; }
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const { return active; }
	void setIsActive(bool b) { active = b; }
	sol::table getDataTable() const;
	bool getIsAlive() const { return health > 0; }
	void setIsAlive(bool b) { health = b ? 100 : 0; }
	//bool getIsImmortal() const { return isImmortal; }
	//void setIsImmortal(bool b) { isImmortal = b; }
	//bool getIsOnGround() const { return isOnGround; }
	//bool getIsStanding() const { return isStanding; }
	bool getIsBleeding() const { return isBleeding; }
	void setIsBleeding(bool b) { isBleeding = b; }
	void update() const;
	//bool getIsAppearanceDirty() const { return isAppearanceDirty; }
	//void setIsAppearanceDirty(bool b) { isAppearanceDirty = b; }
	Player* getPlayer() const;
	void setPlayer(Player* player);
	Account* getAccount();
	void setAccount(Account* account);
	Vehicle* getVehicle() const;
	void setVehicle(Vehicle* vcl);
	Bone* getBone(unsigned int idx);
	Bone* getRigidBody(unsigned int idx);
	Item* getRightHandItem() const;
	Item* getLeftHandItem() const;
	Human* getRightHandGrab() const;
	void setRightHandGrab(Human* man);
	Human* getLeftHandGrab() const;
	void setLeftHandGrab(Human* man);
	InventorySlot* getInventorySlot(unsigned int idx);
	void remove() const;
	void teleport(Vector* vec);
	void speak(const char* message, int distance) const;
	void arm(int weapon, int magCount) const;
	void setVelocity(Vector* vel);
	void addVelocity(Vector* vel);
	bool mountItem(Item* childItem, unsigned int slot) const;
	void applyDamage(int bone, int damage) const;
};

// 408 bytes (198)
struct ItemType {
	int price;
	float mass;          // 04
	int isGun;           // 08
	int pistolAim;       // 0c
	// in ticks per bullet
	int fireRate;        // 10
	//?
	int bulletType;        // 14
	int unk0;              // 18
	int unk1;              // 1c
	float bulletVelocity;  // 20
	float bulletSpread;    // 24
	char name[32];         // 28
	char PAD_1[336];
	//char unk2[0x78 - 0x2c - 64];
	//int numHands;         // 78
	//Vector rightHandPos;  // 7c
	//Vector leftHandPos;   // 88
	//char unk3[0x100 - 0x88 - 12];
	//Vector boundsCenter;  // 100
	//char unk4[4912 - 0x100 - 12];

	const char* getClass() const { return "ItemType"; }
	std::string __tostring() const;
	int getIndex() const;
	char* getName() { return name; }
	void setName(const char* newName) { std::strncpy(name, newName, 31); }
	bool getIsGun() const { return isGun; }
	void setIsGun(bool b) { isGun = b; }
	bool getIsPistolAim() const { return isGun; }
	void setIsPistolAim(bool b) { isGun = b; }
};

// 428 bytes (1AC)
struct Item {
	int active;             // 00
	int physicsSim;         // 04
	int physicsSettled;     // 08
	int type;               // 0C
	float mass;             // 10
	int despawnTime;        // 14
	int parentHumanID;      // 18
	int parentItemID;       // 1C
	int parentSlot;         // 20
	int numChildItems;      // 24
	int childItemIDs[4];    // 28
	char PAD_1[16];         // 3c
	Vector pos;             // 48
	Vector pos2;            // 54
	Vector vel2;            // 60
	Vector vel;             // 74 For some reason this one is used more, to avoid confusion I am swapping the names. I know its hacky.
	Vector vel3;            // 78
	RotMatrix rot;          // 84
	RotMatrix rotVel;       // a8
	char PAD_2[12];         // cc
	int cooldown;           // d8
	int unk0;               // dc
	int bullets;            // e0
	int unk1;               // e4
	int triggerTicks;       // e8   | How many ticks you have held LMB with a gun
	int inputFlags;         // ec
	int lastInputFlags;     // f0
	int unk2;               // f4    | This value will count down to 0 if above 0. Nothing ever sets it above 0. Nothing ever uses this.
	int connectedPhoneID;	// f8
	int phoneNumber;		// fc
	int callerRingTimer;	// 100
	int displayPhoneNumber; // 104
	int enteredPhoneNumber; // 108
	int phoneTexture;       // 10c
	int phoneStatus;        // 110
	int vehicleID;          // 114
	Vector unk3;            // 118  | Unsure. Appears to be related to size or bounding box?
	Vector unk4;            // 118  | Unsure. Appears to be related to size or bounding box? Gets divided by 1 to set above Vector
	int unk5;               // 130
	int cashSpread;         // 134
	int cashBillAmount;     // 138
	int cashPureValue;      // 13c
	char PAD_4[60];         // 140
	Vector unk6;            // 17c | Only set when the spawned item is a "Door" (25)
	Vector unk7;            // 188 | Only set when the spawned item is a "Door" (25)
	Vector unk8;            // 194 | Only set when the spawned item is a "Door" (25)
	Vector unk9;            // 1a0 | Only set when the spawned item is a "Door" (25)

	const char* getClass() const { return "Item"; }
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const { return active; }
	void setIsActive(bool b) { active = b; }
	sol::table getDataTable() const;
	bool getHasPhysics() const { return physicsSim; }
	void setHasPhysics(bool b) { physicsSim = b; }
	bool getPhysicsSettled() const { return physicsSettled; }
	void setPhysicsSettled(bool b) { physicsSettled = b; }
	ItemType* getType();
	void setType(ItemType* itemType);
	void remove() const;
	Human* getParentHuman() const;
	void setParentHuman(Human* human);
	Item* getParentItem() const;
	void setParentItem(Item* item);
	Item* getChildItem(unsigned int idx) const;
	Item* getConnectedPhone() const;
	void setConnectedPhone(Item* item);
	Vehicle* getVehicle() const;
	void setVehicle(Vehicle* vehicle);
	bool mountItem(Item* childItem, unsigned int slot) const;
	bool unmount() const;
	void updateInfo() const;
	void update() const;
	void speak(const char* message, int distance) const;
	void cashAddBill(int position, int value) const;
	void cashRemoveBill(int position) const;
	int cashGetBillValue() const;
};

// 152 bytes (98)
struct Wheel {
	int isPopped;     // 04
	int isDestroyed;  // 08
	int health;       // 0c
	char PAD_1[16];   // 10
	float mass;       // 1c
	float size;       // 20
	char PAD_2[116];  // 24

	const char* getClass() const { return "Wheel"; }
};

// 99776 bytes (185C0)
struct VehicleType {
	int controllableState;  // 
	char PAD_0[8];          // 04
	char name[32];          // 0c
	int price;              // 2c
	float mass;             // 30
	char PAD_2[30500];      // 34
	float acceleration;     // 7758
	int numWheels;          // 775c
	char PAD_3[2860];       // 34

	const char* getClass() const { return "VehicleType"; }
	std::string __tostring() const;
	int getIndex() const;
	//bool getUsesExternalModel() const { return usesExternalModel; }
	char* getName() { return name; }
	void setName(const char* newName) {
		std::strncpy(name, newName, sizeof(name) - 1);
	}
};

// 20840 bytes (5168)
struct Vehicle {
    int active;
    unsigned int type;          // 04
    int controllableState;      // 08
    // default 100
    int health;                 // 0c
    int unk1;                   // 10
    int lastDriverPlayerID;     // 14
    unsigned int color;         // 18
    //-1 = won't despawn
    short despawnTime;          // 1c
    int isLocked;               // 20
    Vector pos;                 // 24
    Vector pos2;                // 30
    RotMatrix rot;              // 3c
	Vector vel;                 // 60
	Vector vel2;                // 6c
	Vector vel3;                // 78
	Vector boundingBoxCornerA;  // 6c
	Vector boundingBoxCornerB;  // 78
	char UNUSED[12];            // 9c
	int numParticles;           // a8
	int particles[8];           // ac
    char PAD_1[23280];
    //int unk4;            // 68
    //Vector vel;          // 6c
    //PAD(0x27fc - 0x6c - 12);
    //int windowStates[8];  // 27fc
    //PAD(0x3600 - 0x27fc - (4 * 8));
    float gearX;         // 5bbc
    float steerControl;  // 5bc0
    float gearY;         // 5bc4
    float gasControl;    // 5bc8
    char PAD_2[16];      // 5bcc
    int inputFlags;      // 5bdc
    char PAD_3[632];     // 5be0
    float acceleration;  // 5e58
    char PAD_4[132];     // 5e5c
    int engineRPM;       // 5ee0
	int unk2;            // 5ee4
	int numWheels;       // 5ee8
	Wheel wheels[6];     // 5eec
    char PAD_5[8116];    // 5ee8
    //PAD(0x3648 - 0x360c - 4);
    //int trafficCarID;  // 3648
    //PAD(0x38a0 - 0x3648 - 4);
    //PAD(0x3930 - 0x38a0 - 4);
    //PAD(0x3940 - 0x3930 - 4);
    //int numWheels;    // 3940
    //Wheel wheels[6];  // 3944
    //PAD(0x4fa8 - (0x3944 + (sizeof(Wheel) * 6)));
    //int bladeBodyID;  // 4fa8
    //PAD(0x50dc - 0x4fa8 - 4);
    //int numSeats;  // 50dc
    //PAD(0x5168 - 0x50dc - 4);
	Particle* getParticle(unsigned int idx) const;
    const char* getClass() const { return "Vehicle"; }
    std::string __tostring() const;
    int getIndex() const;
    bool getIsActive() const { return active; }
    void setIsActive(bool b) { active = b; }
    VehicleType* getType();
    void setType(VehicleType* vehicleType);
    bool getIsLocked() const { return isLocked; }
    void setIsLocked(bool b) { isLocked = b; }
    sol::table getDataTable() const;
    Player* getLastDriver() const;
    //RigidBody* getRigidBody() const;
    //TrafficCar* getTrafficCar() const;
    //void setTrafficCar(TrafficCar* trafficCar);

    void updateType() const;
    void updateDestruction(int updateType, int partID, Vector* pos, Vector* normal) const;
    void remove() const;
    //bool getIsWindowBroken(unsigned int idx) const;
    //void setIsWindowBroken(unsigned int idx, bool b);
    Wheel* getWheel(unsigned int idx);
	void applyDamage(int damage) const;
};

// 92 bytes (5C)
struct Bullet {
	unsigned int type;
	int time;        // 04
	int playerID;    // 08
	float unk0;      // 0c
	float unk1;      // 10
	Vector lastPos;  // 14
	Vector pos;      // 20
	Vector vel;      // 2c
	char unk2[92 - 56];

	const char* getClass() const { return "Bullet"; }
	Player* getPlayer() const;
};

// 240 bytes (F0)
struct Particle {
	/*
	//Not actually accurate to 24, ONLY cars have particles
	0 = human bone
	1 = car body
	2 = wheel
	3 = item
	*/
	int type;           // 00
	int despawnTime;    // 04
	int unk0;           // 08
	float gravity;      // 0c
	int unk1;           // 10
	Vector pos;         // 14
	Vector pos2;        // 20
	Vector vel;         // 2c
	Vector vel2;        // 38
	//Vector startVel;  // 
	//RotMatrix rot;    // 3c
	//RotMatrix rot2;   // 60
	char unk3[144];     // 44
	int numVehicles;    // d4
	int vehicles[6];    // d8

	Vehicle* getVehicle(unsigned int idx) const;
	const char* getClass() const { return "Particle"; }
	std::string __tostring() const;
	int getIndex() const;
	sol::table getDataTable() const;
};

// 236 bytes (ec)
struct Bond {
	/*
	 * 4 = rigidbody_level
	 * 7 = rigidbody_rigidbody
	 * 8 = rigidbody_rot_rigidbody
	 */
	int type;
	int unk0;              // 04
	int despawnTime;       // 08
	int localIndex;        // 0c
	char unk1[8];          // 10
	// for level bonds
	Vector localPos;       // 18
	char unk2[4];          // 24
	// for non-level bonds
	Vector otherLocalPos;  // 28
	Vector globalPos;      // 34
	char unk3[52];         // 40
	Vector unk4;           // 74
	char unk5[16];         // 80
	int humanID;           // 90 This can be inaccurate, its only accurate PER type
	int bodyID;            // 94
	int otherBodyID;       // 98
	int unk6;              // 9c
	char unk7[76];         // a0

	const char* getClass() const { return "Bond"; }
	std::string __tostring() const;
	int getIndex() const;
	//bool getIsActive() const { return active; }
	//void setIsActive(bool b) { active = b; }
	Human* getHuman() const;
	int getBody() const;
	int getOtherBody() const;
};

// 12 bytes (C)
struct ShopCar {
	int type;
	int price;
	int color;

	const char* getClass() const { return "ShopCar"; }
	VehicleType* getType();
	void setType(VehicleType* vehicleType);
};

// 2524 bytes (9dc)
struct Building {
	int type;           
	char PAD_1[12];            //  04
	Vector pos;                //  10
	Vector interiorCuboidA;    //  1c
	Vector interiorCuboidB;    //  28
	char PAD_2[4];             //  34
	Vector unk0;               //  38
	char PAD_3[2180];          //  44
	int numShopCars;           // 8c8
	ShopCar shopCars[16];      // 8cc
	int shopCarSales;          // 98c
	char PAD_4[76];            // 990
	const char* getClass() const { return "Building"; }
	std::string __tostring() const;
	int getIndex() const;
	ShopCar* getShopCar(unsigned int idx);
};

// 28 bytes (1C)
struct StreetLane {
	int direction;
	Vector posA;
	Vector posB;

	const char* getClass() const { return "StreetLane"; }
};

// 1580 bytes (62C)
struct Street {
	char name[32];
	int intersectionA;      // 20
	int intersectionB;      // 24
	int unk0[3];            // 28
	int numLanes;           // 34
	StreetLane lanes[16];   // 38
	float unk1[6];          // 1f8
	Vector trafficCuboidA;  // 210
	Vector trafficCuboidB;  // 21c
	int numTraffic;         // 228
	char unk2[1580 - 0x228 - 4];

	const char* getClass() const { return "Street"; }
	std::string __tostring() const;
	int getIndex() const;
	char* getName() { return name; }
	StreetIntersection* getIntersectionA() const;
	StreetIntersection* getIntersectionB() const;
	int getNumLanes() { return numLanes; }

	StreetLane* getLane(unsigned int idx);
};

// 104 bytes (68)
struct StreetIntersection {
	std::tuple<int, int, int> blockPos; //00
	Vector pos;       // 0c
	int streetEast;   // 18
	int streetSouth;  // 2c
	int streetWest;   // 20
	int streetNorth;  // 24
	char unk1[0x44 - 0x24 - 4];
	int lightsState;     // 44
	int lightsTimer;     // 48
	int lightsTimerMax;  // 4c
	int lightEast;       // 50
	int lightSouth;      // 54
	int lightWest;       // 58
	int lightNorth;      // 5c
	char unk2[104 - 0x5c - 4];

	const char* getClass() const { return "StreetIntersection"; }
	std::string __tostring() const;
	int getIndex() const;
	Street* getStreetEast() const;
	Street* getStreetSouth() const;
	Street* getStreetWest() const;
	Street* getStreetNorth() const;
};