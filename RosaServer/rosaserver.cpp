#include "rosaserver.h"
#include <cxxabi.h>
#include <execinfo.h>
#include <sys/mman.h>
#include <cerrno>
#include <filesystem>

static unsigned int* version;
//static unsigned int* subVersion;
static char* serverName;
static unsigned int* serverPort;
static unsigned int* serverMaxBytesPerSecond;
static unsigned int* numEvents;
static unsigned int* numConnections;
static unsigned int* numBonds;
static unsigned int* numParticles;
static unsigned int* numBuildings;

//static int* isPassworded;
//static char* password;
static int* maxPlayers;
static char* adminPassword;
static int* doVoiceChat;
static float* gravity;
static float originalGravity;

static void pryMemory(void* address, size_t numPages) {
	size_t pageSize = sysconf(_SC_PAGE_SIZE);

	uintptr_t page = (uintptr_t)address;
	page -= (page % pageSize);

	if (mprotect((void*)page, pageSize * numPages, PROT_WRITE | PROT_READ) == 0) {
		std::ostringstream stream;

		stream << RS_PREFIX "Successfully pried open page at ";
		stream << std::showbase << std::hex;
		stream << static_cast<uintptr_t>(page);
		stream << "\n";

		Console::log(stream.str());
	} else {
		throw std::runtime_error(strerror(errno));
	}
}

struct Server {
	const int TPS = 60;

	const char* getClass() const { return "Server"; }
	int getPort() const { return *serverPort; }
	int getMaxBytesPerSecond() const { return *serverMaxBytesPerSecond; }
	char* getName() const { return serverName; }
	void setName(const char* newName) const { strncpy(serverName, newName, 31); }
	char* getAdminPassword() const { return adminPassword; }
	void setAdminPassword(const char* newPassword) const {
		strncpy(adminPassword, newPassword, 31);
		//*isPassworded = newPassword[0] != 0;
	}
	int getMaxPlayers() const { return *maxPlayers; }
	void setMaxPlayers(int max) const { *maxPlayers = max; }
	int getNumConnections() const { return *numConnections; }
	int getType() const { return *Engine::gameType; }
	void setType(int type) const { *Engine::gameType = type; }
	//char* getLevelName() const { return Engine::mapName; }
	//void setLevelName(const char* newName) const {
	//	strncpy(Engine::mapName, newName, 31);
	//}
	//char* getLoadedLevelName() const { return Engine::loadedMapName; }
	bool getDoVoiceChat() const { return *doVoiceChat; }
	void setDoVoiceChat(bool b) const { *doVoiceChat = b; }
	//float getGravity() const { return *gravity; }
	//void setGravity(float g) const { *gravity = g; }
	float getDefaultGravity() const { return originalGravity; }
	int getState() const { return *Engine::gameState; }
	void setState(int state) const { *Engine::gameState = state; }
	int getTime() const { return *Engine::gameTimer; }
	void setTime(int time) const { *Engine::gameTimer = time; }
	int getTicksSinceReset() const { return *Engine::gameTicksSinceReset; }
	void setTicksSinceReset(int time) const { *Engine::gameTicksSinceReset = time; }
	int getSunTime() const { return *Engine::sunTime; }
	void setSunTime(int time) const { *Engine::sunTime = time % 5184000; }
	//std::string getVersion() const {
	//	std::ostringstream stream;
	//	stream << *version << (char)(*subVersion + 97);
	//	return stream.str();
	//}
	const char* getVersion() const { return "24c"; }
	unsigned int getVersionMajor() const { return *version; }
	//unsigned int getVersionMinor() const { return *subVersion; }
	unsigned int getNumEvents() const { return *numEvents; }
	int getNumBonds() const { return *Engine::numBonds; }
	void setNumBonds(int bonds) const { *Engine::numBonds = bonds; }
	int getNumParticles() const { return *Engine::numParticles; }
	void setNumParticles(int particles) const { *Engine::numParticles = particles; }
	int getNumBuildings() const { return *Engine::numBuildings; }
	void setNumBuildings(int buildings) const { *Engine::numBuildings = buildings; }
	void setConsoleTitle(const char* title) const { Console::setTitle(title); }
	void reset() const { hookAndReset(RESET_REASON_LUACALL); }
	void createTraffic(int count) const { Engine::createTraffic(count); }
};
static Server* server;



// https://github.com/moonjit/moonjit/blob/master/doc/c_api.md#luajit_setmodel-idx-luajit_mode_wrapcfuncflag
static int wrapCExceptions(lua_State* L, lua_CFunction f) {
	try {
		return f(L);
	} catch (const char* s) {
		lua_pushstring(L, s);
	} catch (std::exception& e) {
		lua_pushstring(L, e.what());
	} catch (...) {
	}
	return lua_error(L);
}

static int wrapExceptions(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) {
	Console::log(LUA_PREFIX "Exception caught. Outputting description.\n");
	if (maybe_exception) {
		Console::log("(straight from the exception):\n");
		const std::exception& ex = *maybe_exception;
		Console::log(ex.what());
		std::cout << "\n";
	} else {
		Console::log("(from the description parameter):\n");
		std::cout.write(description.data(),
		                static_cast<std::streamsize>(description.size()));
		std::cout << "\n";
	}

	return sol::stack::push(L, description);
}

void defineThreadSafeAPIs(sol::state* state) {

	lua_pushlightuserdata(*state, (void*)wrapCExceptions);
	luaJIT_setmode(*state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
	lua_pop(*state, 1);

	state->set_exception_handler(&wrapExceptions);

	state->open_libraries(sol::lib::base);
	state->open_libraries(sol::lib::package);
	state->open_libraries(sol::lib::coroutine);
	state->open_libraries(sol::lib::string);
	state->open_libraries(sol::lib::os);
	state->open_libraries(sol::lib::math);
	state->open_libraries(sol::lib::table);
	state->open_libraries(sol::lib::debug);
	state->open_libraries(sol::lib::bit32);
	state->open_libraries(sol::lib::io);
	state->open_libraries(sol::lib::ffi);
	state->open_libraries(sol::lib::jit);

	{
		auto meta = state->new_usertype<Vector>("new", sol::no_constructor);
		meta["x"] = &Vector::x;
		meta["y"] = &Vector::y;
		meta["z"] = &Vector::z;

		meta["class"] = sol::property(&Vector::getClass);
		meta["__tostring"] = &Vector::__tostring;
		meta["__add"] = &Vector::__add;
		meta["__sub"] = &Vector::__sub;
		meta["__eq"] = &Vector::__eq;
		meta["__mul"] = sol::overload(&Vector::__mul, &Vector::__mul_RotMatrix);
		meta["__div"] = &Vector::__div;
		meta["__unm"] = &Vector::__unm;
		meta["add"] = &Vector::add;
		meta["mult"] = &Vector::mult;
		meta["set"] = &Vector::set;
		meta["cross"] = &Vector::cross;
		meta["clone"] = &Vector::clone;
		meta["dist"] = &Vector::dist;
		meta["distSquare"] = &Vector::distSquare;
		meta["length"] = &Vector::length;
		meta["lengthSquare"] = &Vector::lengthSquare;
		meta["dot"] = &Vector::dot;
		meta["getBlockPos"] = &Vector::getBlockPos;
		meta["normalize"] = &Vector::normalize;

	}

	{
		auto meta = state->new_usertype<RotMatrix>("new", sol::no_constructor);
		meta["x1"] = &RotMatrix::x1;
		meta["y1"] = &RotMatrix::y1;
		meta["z1"] = &RotMatrix::z1;
		meta["x2"] = &RotMatrix::x2;
		meta["y2"] = &RotMatrix::y2;
		meta["z2"] = &RotMatrix::z2;
		meta["x3"] = &RotMatrix::x3;
		meta["y3"] = &RotMatrix::y3;
		meta["z3"] = &RotMatrix::z3;

		meta["class"] = sol::property(&RotMatrix::getClass);
		meta["__tostring"] = &RotMatrix::__tostring;
		meta["__mul"] = &RotMatrix::__mul;
		meta["set"] = &RotMatrix::set;
		meta["clone"] = &RotMatrix::clone;
		meta["getForward"] = &RotMatrix::getForward;
		meta["getUp"] = &RotMatrix::getUp;
		meta["getRight"] = &RotMatrix::getRight;
	}

	{
		auto meta = state->new_usertype<Image>("Image");
		meta["width"] = sol::property(&Image::getWidth);
		meta["height"] = sol::property(&Image::getHeight);
		meta["numChannels"] = sol::property(&Image::getNumChannels);
		meta["free"] = &Image::_free;
		meta["loadFromFile"] = &Image::loadFromFile;
		meta["loadBlank"] = &Image::loadBlank;
		meta["getRGB"] = &Image::getRGB;
		meta["getRGBA"] = &Image::getRGBA;
		meta["setPixel"] = sol::overload(&Image::setRGB, &Image::setRGBA);
		meta["getPNG"] = &Image::getPNG;
	}

	(*state)["print"] = Lua::print;

	(*state)["Vector"] = sol::overload(Lua::Vector_, Lua::Vector_3f);
	(*state)["RotMatrix"] = Lua::RotMatrix_;

	(*state)["os"]["listDirectory"] = Lua::os::listDirectory;
	(*state)["os"]["createDirectory"] = Lua::os::createDirectory;
	(*state)["os"]["realClock"] = Lua::os::realClock;
	(*state)["os"]["exit"] = sol::overload(Lua::os::exit, Lua::os::exitCode);

	{
		auto httpTable = state->create_table();
		(*state)["http"] = httpTable;
		httpTable["getSync"] = Lua::http::getSync;
		httpTable["postSync"] = Lua::http::postSync;
	}
}

void luaInit(bool redo) {
	std::lock_guard<std::mutex> guard(stateResetMutex);

	if (redo) {
		Console::log(LUA_PREFIX "Resetting state...\n");
		delete server;

		for (int i = 0; i < maxNumberOfPlayers; i++) {
			if (playerDataTables[i]) {
				delete playerDataTables[i];
				playerDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfHumans; i++) {
			if (humanDataTables[i]) {
				delete humanDataTables[i];
				humanDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfItems; i++) {
			if (itemDataTables[i]) {
				delete itemDataTables[i];
				itemDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfVehicles; i++) {
			if (vehicleDataTables[i]) {
				delete vehicleDataTables[i];
				vehicleDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfParticles; i++) {
			if (particleDataTables[i]) {
				delete particleDataTables[i];
				particleDataTables[i] = nullptr;
			}
		}

		delete lua;
	} else {
		Console::log(LUA_PREFIX "Initializing state...\n");
	}

	lua = new sol::state();

	Console::log(LUA_PREFIX "Defining...\n");
	defineThreadSafeAPIs(lua);

	{
		auto meta = lua->new_usertype<Server>("new", sol::no_constructor);
		meta["TPS"] = &Server::TPS;

		meta["class"] = sol::property(&Server::getClass);
		meta["port"] = sol::property(&Server::getPort);
		meta["name"] = sol::property(&Server::getName, &Server::setName);
		meta["adminPassword"] = sol::property(&Server::getAdminPassword, &Server::setAdminPassword);
		meta["maxBytesPerSecond"] = sol::property(&Server::getMaxBytesPerSecond);
		meta["maxPlayers"] = sol::property(&Server::getMaxPlayers, &Server::setMaxPlayers);
		meta["type"] = sol::property(&Server::getType, &Server::setType);
		//meta["levelToLoad"] = sol::property(&Server::getLevelName, &Server::setLevelName);
		//meta["loadedLevel"] = sol::property(&Server::getLoadedLevelName);
		meta["doVoiceChat"] = sol::property(&Server::getDoVoiceChat, &Server::setDoVoiceChat);
		//meta["gravity"] = sol::property(&Server::getGravity, &Server::setGravity);
		//meta["defaultGravity"] = sol::property(&Server::getDefaultGravity);
		meta["state"] = sol::property(&Server::getState, &Server::setState);
		meta["time"] = sol::property(&Server::getTime, &Server::setTime);
		meta["ticksSinceReset"] = sol::property(&Server::getTicksSinceReset, &Server::setTicksSinceReset);
		meta["sunTime"] = sol::property(&Server::getSunTime, &Server::setSunTime);
		meta["version"] = sol::property(&Server::getVersion);
		meta["versionMajor"] = sol::property(&Server::getVersionMajor);
		//meta["versionMinor"] = sol::property(&Server::getVersionMinor);
		meta["numEvents"] = sol::property(&Server::getNumEvents);
		meta["numBonds"] = sol::property(&Server::getNumBonds, &Server::setNumBonds);
		meta["numParticles"] = sol::property(&Server::getNumParticles, &Server::setNumParticles);
		meta["numBuildings"] = sol::property(&Server::getNumBuildings, &Server::setNumBuildings);
		meta["numConnections"] = sol::property(&Server::getNumConnections);
		meta["setConsoleTitle"] = &Server::setConsoleTitle;
		meta["reset"] = &Server::reset;
		meta["addTraffic"] = &Server::createTraffic;
	}

	server = new Server();
	(*lua)["server"] = server;

	{
		auto meta = lua->new_usertype<EarShot>("new", sol::no_constructor);
		meta["class"] = sol::property(&EarShot::getClass);
		meta["isActive"] = sol::property(&EarShot::getIsActive, &EarShot::setIsActive);
		meta["player"] = sol::property(&EarShot::getPlayer, &EarShot::setPlayer);
		meta["human"] = sol::property(&EarShot::getHuman, &EarShot::setHuman);
		meta["receivingItem"] = sol::property(&EarShot::getReceivingItem, &EarShot::setReceivingItem);
		meta["transmittingItem"] = sol::property(&EarShot::getTransmittingItem, &EarShot::setTransmittingItem);
		meta["distance"] = &EarShot::distance;
		meta["volume"] = &EarShot::volume;
	}

	{
		auto meta = lua->new_usertype<Connection>("new", sol::no_constructor);
		meta["address"] = sol::property(&Connection::getAddress);
		meta["port"] = &Connection::port;
		meta["roundNumber"] = &Connection::roundNumber;
		meta["adminVisible"] = sol::property(&Connection::getAdminVisible,&Connection::setAdminVisible);
		meta["player"] = sol::property(&Connection::getPlayer, &Connection::setPlayer);
		meta["timeoutTime"] = &Connection::timeoutTime;
		meta["numReceivedEvents"] = &Connection::numReceivedEvents;
		meta["getEarShot"] = &Connection::getEarShot;
		meta["spectatingHuman"] = sol::property(&Connection::getSpectatingHuman);
		meta["headPos"] = sol::property(&Connection::getHeadPosition);
		meta["cameraPos"] = sol::property(&Connection::getCameraPosition);
		meta["class"] = sol::property(&Connection::getClass);
	}

	{
		auto meta = lua->new_usertype<Account>("new", sol::no_constructor);
		meta["token"] = &Account::token;
		meta["token2"] = &Account::token2;
		meta["hairColor"] = &Account::hairColor;
		meta["name"] = sol::property(&Account::getName);
		meta["money"] = &Account::money;
		meta["playTime"] = &Account::playTime;
		meta["eyeColor"] = &Account::eyeColor;
		meta["skinColor"] = &Account::skinColor;
		meta["hairColor"] = &Account::hairColor;

		meta["class"] = sol::property(&Account::getClass);
		meta["__tostring"] = &Account::__tostring;
		meta["index"] = sol::property(&Account::getIndex);
	}

	{
		auto meta = lua->new_usertype<Player>("new", sol::no_constructor);
		meta["isActive"] = sol::property(&Player::getIsActive, &Player::setIsActive);
		meta["name"] = sol::property(&Player::getName, &Player::setName);
		meta["token"] = &Player::token;
		meta["token2"] = &Player::token2;
		meta["isAdmin"] = sol::property(&Player::getIsAdmin, &Player::setIsAdmin);
		meta["adminAttempts"] = &Player::adminAttempts;
		meta["account"] = sol::property(&Player::getAccount, &Player::setAccount);
		meta["isReady"] = sol::property(&Player::getIsReady, &Player::setIsReady);
		meta["money"] = &Player::money;
		meta["itemsBought"] = &Player::itemsBought;
		meta["vehiclesBought"] = &Player::vehiclesBought;
		meta["withdrawnBills"] = &Player::withdrawnBills;
		meta["team"] = &Player::team;
		meta["teamSwitchTimer"] = &Player::teamSwitchTimer;
		meta["stocks"] = &Player::stocks;
		meta["human"] = sol::property(&Player::getHuman, &Player::setHuman);
		meta["gearX"] = &Player::gearX;
		meta["leftRightInput"] = &Player::leftRightInput;
		meta["gearY"] = &Player::gearY;
		meta["forwardBackInput"] = &Player::forwardBackInput;
		meta["viewYaw"] = &Player::viewYaw;
		meta["viewPitch"] = &Player::viewPitch;
		meta["freeLookYaw"] = &Player::freeLookYaw;
		meta["freeLookPitch"] = &Player::freeLookPitch;
		meta["inputFlags"] = &Player::inputFlags;
		meta["lastInputFlags"] = &Player::lastInputFlags;

		meta["zoomLevel"] = &Player::zoomLevel;
		meta["isCrouching"] = &Player::isCrouching;
		meta["inputType"] = &Player::inputType;

		meta["menuTab"] = &Player::menuTab;
		meta["menuTabBuildingID"] = &Player::menuTabBuildingID;
		meta["numActions"] = &Player::numActions;
		meta["lastNumActions"] = &Player::lastNumActions;
		meta["getAction"] = &Player::getAction;
		meta["health"] = &Player::health;
		meta["isBot"] = sol::property(&Player::getIsBot, &Player::setIsBot);
		meta["botDriving"] = sol::property(&Player::getIsBotDriving, &Player::setIsBotDriving);
		meta["suitColor"] = &Player::suitColor;
		meta["sunglasses"] = &Player::sunglasses;

		//meta["numMenuButtons"] = &Player::numMenuButtons;
		//meta["gender"] = &Player::gender;
		//meta["skinColor"] = &Player::skinColor;
		//meta["hairColor"] = &Player::hairColor;
		//meta["hair"] = &Player::hair;
		//meta["eyeColor"] = &Player::eyeColor;
		//meta["model"] = &Player::model;
		//meta["suitColor"] = &Player::suitColor;
		//meta["tieColor"] = &Player::tieColor;
		//meta["head"] = &Player::head;
		//meta["necklace"] = &Player::necklace;

		meta["class"] = sol::property(&Player::getClass);
		meta["__tostring"] = &Player::__tostring;
		meta["index"] = sol::property(&Player::getIndex);
		meta["data"] = sol::property(&Player::getDataTable);
		meta["connection"] = sol::property(&Player::getConnection);
		//meta["botDestination"] = sol::property(&Player::getBotDestination, &Player::setBotDestination);

		meta["update"] = &Player::update;
		meta["updateFinance"] = &Player::updateFinance;
		meta["remove"] = &Player::remove;
	}

	{
		auto meta = lua->new_usertype<Human>("new", sol::no_constructor);
		meta["isActive"] = sol::property(&Human::getIsActive, &Human::setIsActive);
		meta["hasPhysics"] = &Human::physicsSim;
		meta["player"] = sol::property(&Human::getPlayer, &Human::setPlayer);
		meta["account"] = sol::property(&Human::getAccount, &Human::setAccount);
		meta["vehicle"] = sol::property(&Human::getVehicle, &Human::setVehicle);
		meta["vehicleSeat"] = &Human::vehicleSeat;
		meta["lastVehicleID"] = &Human::lastVehicleID;
		meta["lastVehicleCooldown"] = &Human::lastVehicleCooldown;
		meta["despawnTime"] = &Human::despawnTime;
		meta["health"] = &Human::health;
		meta["vehicleExitTimer"] = &Human::vehicleExitTimer;
		meta["spawnProtection"] = &Human::spawnProtection;
		meta["zoomLevel"] = &Human::zoomLevel;
		meta["viewYaw2"] = &Human::viewYaw2;
		meta["viewPitch2"] = &Human::viewPitch2;
		meta["gearX"] = &Human::gearX;
		meta["strafeInput"] = &Human::strafeInput;
		meta["gearY"] = &Human::gearY;
		meta["walkInput"] = &Human::walkInput;
		meta["viewYaw"] = &Human::viewYaw;
		meta["viewPitch"] = &Human::viewPitch;
		meta["freeLookYaw"] = &Human::freeLookYaw;
		meta["freeLookPitch"] = &Human::freeLookPitch;
		meta["inputFlags"] = &Human::inputFlags;
		meta["lastInputFlags"] = &Human::lastInputFlags;
		meta["pos"] = &Human::pos;
		meta["getBone"] = &Human::getBone;
		meta["isBleeding"] = sol::property(&Human::getIsBleeding, &Human::setIsBleeding);

		meta["class"] = sol::property(&Human::getClass);
		meta["__tostring"] = &Human::__tostring;
		meta["index"] = sol::property(&Human::getIndex);
		meta["data"] = sol::property(&Human::getDataTable);
		meta["getInventorySlot"] = &Human::getInventorySlot;

		meta["remove"] = &Human::remove;
		meta["teleport"] = &Human::teleport;
		meta["speak"] = &Human::speak;
		meta["arm"] = &Human::arm;
		//meta["getRigidBody"] = &Human::getRigidBody;
		meta["setVelocity"] = &Human::setVelocity;
		meta["addVelocity"] = &Human::addVelocity;
		meta["mountItem"] = &Human::mountItem;
		meta["update"] = &Human::update;
		meta["applyDamage"] = &Human::applyDamage;
	}

	{
		auto meta = lua->new_usertype<InventorySlot>("new", sol::no_constructor);
		meta["count"] = &InventorySlot::count;

		meta["class"] = sol::property(&InventorySlot::getClass);

		meta["primaryItem"] = sol::property(&InventorySlot::getPrimaryItem);
		meta["secondaryItem"] = sol::property(&InventorySlot::getSecondaryItem);
	}

	{
		auto meta = lua->new_usertype<ItemType>("new", sol::no_constructor);
		meta["price"] = &ItemType::price;
		meta["mass"] = &ItemType::mass;
		meta["fireRate"] = &ItemType::fireRate;
		meta["bulletType"] = &ItemType::bulletType;
		meta["bulletVelocity"] = &ItemType::bulletVelocity;
		meta["bulletSpread"] = &ItemType::bulletSpread;
		//meta["numHands"] = &ItemType::numHands;
		//meta["rightHandPos"] = &ItemType::rightHandPos;
		//meta["leftHandPos"] = &ItemType::leftHandPos;
		//meta["boundsCenter"] = &ItemType::boundsCenter;

		meta["class"] = sol::property(&ItemType::getClass);
		meta["__tostring"] = &ItemType::__tostring;
		meta["index"] = sol::property(&ItemType::getIndex);
		meta["name"] = sol::property(&ItemType::getName, &ItemType::setName);
		meta["isGun"] = sol::property(&ItemType::getIsGun, &ItemType::setIsGun);
		meta["pistolAim"] = sol::property(&ItemType::getIsPistolAim, &ItemType::setIsPistolAim);
	}

	{
		auto meta = lua->new_usertype<Item>("new", sol::no_constructor);
		meta["isActive"] = sol::property(&Item::getIsActive, &Item::setIsActive);
		meta["hasPhysics"] = sol::property(&Item::getHasPhysics, &Item::setHasPhysics);
		meta["physicsSettled"] = sol::property(&Item::getPhysicsSettled, &Item::setPhysicsSettled);
		meta["type"] = sol::property(&Item::getType, &Item::setType);
		meta["mass"] = &Item::mass;
		meta["despawnTime"] = &Item::despawnTime;
		meta["parentHuman"] = sol::property(&Item::getParentHuman, &Item::setParentHuman);
		meta["parentItem"] = sol::property(&Item::getParentItem, &Item::setParentItem);
		meta["parentSlot"] = &Item::parentSlot;
		meta["numChildItems"] = &Item::numChildItems;
		meta["getChildItem"] = &Item::getChildItem;
		meta["pos"] = &Item::pos;
		meta["pos2"] = &Item::pos2;
		meta["vel"] = &Item::vel;
		meta["vel2"] = &Item::vel2;
		meta["vel3"] = &Item::vel3;
		meta["rot"] = &Item::rot;
		meta["rotVel"] = &Item::rotVel;
		meta["cooldown"] = &Item::cooldown;
		meta["cashSpread"] = &Item::cashSpread;
		meta["cashAmount"] = &Item::cashBillAmount;
		meta["cashPureValue"] = &Item::cashPureValue;
		meta["bullets"] = &Item::bullets;
		meta["triggerTicks"] = &Item::triggerTicks;
		meta["inputFlags"] = &Item::inputFlags;
		meta["lastInputFlags"] = &Item::lastInputFlags;
		meta["connectedPhone"] = sol::property(&Item::getConnectedPhone, &Item::setConnectedPhone);
		meta["phoneNumber"] = &Item::phoneNumber;
		meta["callerRingTimer"] = &Item::callerRingTimer;
		meta["displayPhoneNumber"] = &Item::displayPhoneNumber;
		meta["enteredPhoneNumber"] = &Item::enteredPhoneNumber;
		meta["phoneTexture"] = &Item::phoneTexture;
		meta["phoneStatus"] = &Item::phoneStatus;
		meta["vehicle"] = sol::property(&Item::getVehicle, &Item::setVehicle);
		meta["class"] = sol::property(&Item::getClass);
		meta["__tostring"] = &Item::__tostring;
		meta["index"] = sol::property(&Item::getIndex);
		meta["data"] = sol::property(&Item::getDataTable);
		meta["update"] = &Item::update;
		meta["updateInfo"] = &Item::updateInfo;
		meta["remove"] = &Item::remove;
		meta["mountItem"] = &Item::mountItem;
		meta["unmount"] = &Item::unmount;
		meta["speak"] = &Item::speak;
		//meta["setMemo"] = &Item::setMemo;
		meta["cashAddBill"] = &Item::cashAddBill;
		meta["cashRemoveBill"] = &Item::cashRemoveBill;
		meta["cashGetBillValue"] = &Item::cashGetBillValue;
	}

	{
		auto meta = lua->new_usertype<VehicleType>("new", sol::no_constructor);
		meta["controllableState"] = &VehicleType::controllableState;
		meta["price"] = &VehicleType::price;
		meta["mass"] = &VehicleType::mass;
		meta["numWheels"] = &VehicleType::numWheels;
		meta["acceleration"] = &VehicleType::acceleration;
		meta["class"] = sol::property(&VehicleType::getClass);
		meta["__tostring"] = &VehicleType::__tostring;
		meta["index"] = sol::property(&VehicleType::getIndex);
		meta["name"] = sol::property(&VehicleType::getName, &VehicleType::setName);
	}

	{
		auto meta = lua->new_usertype<ShopCar>("new", sol::no_constructor);
		meta["price"] = &ShopCar::price;
		meta["color"] = &ShopCar::color;

		meta["class"] = sol::property(&ShopCar::getClass);
		meta["type"] = sol::property(&ShopCar::getType, &ShopCar::setType);
	}

	{
		auto meta = lua->new_usertype<Building>("new", sol::no_constructor);
		meta["type"] = &Building::type;
		meta["pos"] = &Building::pos;
		//meta["spawnRot"] = &Building::spawnRot;
		meta["interiorCuboidA"] = &Building::interiorCuboidA;
		meta["interiorCuboidB"] = &Building::interiorCuboidB;
		meta["numShopCars"] = &Building::numShopCars;
		meta["getShopCar"] = &Building::getShopCar;
		meta["shopCarSales"] = &Building::shopCarSales;

		meta["class"] = sol::property(&Building::getClass);
		meta["__tostring"] = &Building::__tostring;
		meta["index"] = sol::property(&Building::getIndex);
	}

	{
		auto meta = lua->new_usertype<Wheel>("new", sol::no_constructor);
		meta["isPopped"] = &Wheel::isPopped;
		meta["isDestroyed"] = &Wheel::isDestroyed;
		meta["health"] = &Wheel::health;
		meta["mass"] = &Wheel::mass;
		meta["size"] = &Wheel::size;

		meta["class"] = sol::property(&Wheel::getClass);
		//meta["player"] = sol::property(&Bullet::getPlayer);
	}

	{
        auto meta = lua->new_usertype<Vehicle>("new", sol::no_constructor);
        meta["isActive"] = sol::property(&Vehicle::getIsActive, &Vehicle::setIsActive);
        meta["type"] = sol::property(&Vehicle::getType, &Vehicle::setType);
        meta["controllableState"] = &Vehicle::controllableState;
        meta["health"] = &Vehicle::health;
        meta["lastDriver"] = sol::property(&Vehicle::getLastDriver);
        meta["color"] = &Vehicle::color;
		meta["despawnTime"] = &Vehicle::despawnTime;
        meta["isLocked"] = &Vehicle::isLocked;
        meta["pos"] = &Vehicle::pos;
        meta["pos2"] = &Vehicle::pos2;
        meta["rot"] = &Vehicle::rot;
        meta["vel"] = &Vehicle::vel;
		meta["vel2"] = &Vehicle::vel2;
		meta["vel3"] = &Vehicle::vel3;

		meta["numParticles"] = &Vehicle::numParticles;
		meta["getParticle"] = &Vehicle::getParticle;
        // Messy but faster than using a table or some shit
        //meta["windowState0"] = &Vehicle::windowState0;
        //meta["windowState1"] = &Vehicle::windowState1;
        //meta["windowState2"] = &Vehicle::windowState2;
        //meta["windowState3"] = &Vehicle::windowState3;
        //meta["windowState4"] = &Vehicle::windowState4;
        //meta["windowState5"] = &Vehicle::windowState5;
        //meta["windowState6"] = &Vehicle::windowState6;
        //meta["windowState7"] = &Vehicle::windowState7;
        meta["gearX"] = &Vehicle::gearX;
        meta["steerControl"] = &Vehicle::steerControl;
        meta["gearY"] = &Vehicle::gearY;
        meta["gasControl"] = &Vehicle::gasControl;
        //meta["bladeBodyID"] = &Vehicle::bladeBodyID;
        meta["inputFlags"] = &Vehicle::inputFlags;
        meta["acceleration"] = &Vehicle::acceleration;
        meta["engineRPM"] = &Vehicle::engineRPM;

        meta["class"] = sol::property(&Vehicle::getClass);
        meta["__tostring"] = &Vehicle::__tostring;
        meta["index"] = sol::property(&Vehicle::getIndex);
        meta["data"] = sol::property(&Vehicle::getDataTable);
        //meta["rigidBody"] = sol::property(&Vehicle::getRigidBody);
		meta["getWheel"] = &Vehicle::getWheel;
        meta["updateType"] = &Vehicle::updateType;
        meta["updateDestruction"] = &Vehicle::updateDestruction;
        meta["remove"] = &Vehicle::remove;
		meta["applyDamage"] = &Vehicle::applyDamage;
    }

	{
		auto meta = lua->new_usertype<Bullet>("new", sol::no_constructor);
		meta["type"] = &Bullet::type;
		meta["time"] = &Bullet::time;
		meta["lastPos"] = &Bullet::lastPos;
		meta["pos"] = &Bullet::pos;
		meta["vel"] = &Bullet::vel;

		meta["class"] = sol::property(&Bullet::getClass);
		meta["player"] = sol::property(&Bullet::getPlayer);
	}

	{
		auto meta = lua->new_usertype<Bone>("new", sol::no_constructor);
		meta["pos"] = &Bone::pos;
		meta["pos2"] = &Bone::pos2;
		meta["vel"] = &Bone::vel;
		meta["unk0"] = &Bone::unk0;
		meta["unk1"] = &Bone::unk1;
		meta["unk2"] = &Bone::unk2;
		meta["rot"] = &Bone::rot;
		meta["rotVel"] = &Bone::rotVel;
		meta["unk4"] = &Bone::unk4;
		meta["unk5"] = &Bone::unk5;
		meta["unk6"] = &Bone::unk6;
		meta["mass"] = &Bone::mass;
		meta["size"] = &Bone::size;
		meta["size2"] = &Bone::size2;
		meta["unk7"] = &Bone::unk7;
		meta["unk8"] = &Bone::unk8;
		meta["unk9"] = &Bone::unk9;
		meta["bondID"] = &Bone::bondID;
		meta["class"] = sol::property(&Bone::getClass);
	}

	{
		auto meta = lua->new_usertype<Particle>("new", sol::no_constructor);

		meta["type"] = &Particle::type;
		meta["despawnTime"] = &Particle::despawnTime;
		meta["unk0"] = &Particle::unk0;
		meta["gravity"] = &Particle::gravity;
		meta["unk1"] = &Particle::unk1;
		meta["pos"] = &Particle::pos;
		meta["pos2"] = &Particle::pos2;
		meta["vel"] = &Particle::vel;
		meta["vel2"] = &Particle::vel2;
		meta["numVehicles"] = &Particle::numVehicles;
		meta["getVehicle"] = &Particle::getVehicle;
		meta["class"] = sol::property(&Particle::getClass);
		meta["__tostring"] = &Particle::__tostring;
		meta["index"] = sol::property(&Particle::getIndex);
		meta["data"] = sol::property(&Particle::getDataTable);
	}

	{
		auto meta = lua->new_usertype<Bond>("new", sol::no_constructor);
		meta["type"] = &Bond::type;
		meta["despawnTime"] = &Bond::despawnTime;
		meta["globalPos"] = &Bond::globalPos;
		meta["localPos"] = &Bond::localPos;
		meta["otherLocalPos"] = &Bond::otherLocalPos;

		meta["class"] = sol::property(&Bond::getClass);
		meta["__tostring"] = &Bond::__tostring;
		meta["index"] = sol::property(&Bond::getIndex);
		//meta["isActive"] = sol::property(&Bond::getIsActive, &Bond::setIsActive);
		meta["human"] = sol::property(&Bond::getHuman);
		meta["body"] = sol::property(&Bond::getBody);
		meta["otherBody"] = sol::property(&Bond::getOtherBody);
		meta["unk6"] = &Bond::unk6;
	}

	{
		auto meta = lua->new_usertype<Action>("new", sol::no_constructor);
		meta["type"] = &Action::type;
		meta["a"] = &Action::a;
		meta["b"] = &Action::b;
		meta["c"] = &Action::c;
		meta["d"] = &Action::d;
		meta["message"] = sol::property(&Action::getText, &Action::setText);
		meta["class"] = sol::property(&Action::getClass);
	}

	{
		auto meta = lua->new_usertype<MenuButton>("new", sol::no_constructor);
		meta["id"] = &MenuButton::id;
		meta["text"] = sol::property(&MenuButton::getText, &MenuButton::setText);

		meta["class"] = sol::property(&MenuButton::getClass);
	}

	{
		auto meta = lua->new_usertype<Worker>(
		    "Worker", sol::constructors<ChildProcess(std::string)>());
		meta["stop"] = &Worker::stop;
		meta["sendMessage"] = &Worker::sendMessage;
		meta["receiveMessage"] = &Worker::receiveMessage;
	}

	{
		auto meta = lua->new_usertype<ChildProcess>(
		    "ChildProcess", sol::constructors<ChildProcess(std::string)>());
		meta["isRunning"] = &ChildProcess::isRunning;
		meta["terminate"] = &ChildProcess::terminate;
		meta["getExitCode"] = &ChildProcess::getExitCode;
		meta["receiveMessage"] = &ChildProcess::receiveMessage;
		meta["sendMessage"] = &ChildProcess::sendMessage;
		meta["setCPULimit"] = &ChildProcess::setCPULimit;
		meta["setMemoryLimit"] = &ChildProcess::setMemoryLimit;
		meta["setFileSizeLimit"] = &ChildProcess::setFileSizeLimit;
		meta["getPriority"] = &ChildProcess::getPriority;
		meta["setPriority"] = &ChildProcess::setPriority;
	}

	{
		auto meta = lua->new_usertype<StreetLane>("new", sol::no_constructor);
		meta["direction"] = &StreetLane::direction;
		meta["posA"] = &StreetLane::posA;
		meta["posB"] = &StreetLane::posB;

		meta["class"] = sol::property(&StreetLane::getClass);
	}

	{
		auto meta = lua->new_usertype<Street>("new", sol::no_constructor);
		meta["trafficCuboidA"] = &Street::trafficCuboidA;
		meta["trafficCuboidB"] = &Street::trafficCuboidB;
		meta["numTraffic"] = &Street::numTraffic;

		meta["class"] = sol::property(&Street::getClass);
		meta["__tostring"] = &Street::__tostring;
		meta["index"] = sol::property(&Street::getIndex);
		meta["name"] = sol::property(&Street::getName);
		meta["intersectionA"] = sol::property(&Street::getIntersectionA);
		meta["intersectionB"] = sol::property(&Street::getIntersectionB);
		meta["numLanes"] = sol::property(&Street::getNumLanes);

		meta["getLane"] = &Street::getLane;
	}

	{
		auto meta =
		    lua->new_usertype<StreetIntersection>("new", sol::no_constructor);
		meta["blockPos"] = &StreetIntersection::blockPos;
		meta["pos"] = &StreetIntersection::pos;
		meta["lightsState"] = &StreetIntersection::lightsState;
		meta["lightsTimer"] = &StreetIntersection::lightsTimer;
		meta["lightsTimerMax"] = &StreetIntersection::lightsTimerMax;
		meta["lightEast"] = &StreetIntersection::lightEast;
		meta["lightSouth"] = &StreetIntersection::lightSouth;
		meta["lightWest"] = &StreetIntersection::lightWest;
		meta["lightNorth"] = &StreetIntersection::lightNorth;

		meta["class"] = sol::property(&StreetIntersection::getClass);
		meta["__tostring"] = &StreetIntersection::__tostring;
		meta["index"] = sol::property(&StreetIntersection::getIndex);
		meta["streetEast"] = sol::property(&StreetIntersection::getStreetEast);
		meta["streetSouth"] = sol::property(&StreetIntersection::getStreetSouth);
		meta["streetWest"] = sol::property(&StreetIntersection::getStreetWest);
		meta["streetNorth"] = sol::property(&StreetIntersection::getStreetNorth);
	}

	(*lua)["flagStateForReset"] = Lua::flagStateForReset;

	(*lua)["hook"] = lua->create_table();
	(*lua)["hook"]["persistentMode"] = hookMode;

	{
		auto eventTable = lua->create_table();
		(*lua)["event"] = eventTable;
		eventTable["sound"] = sol::overload(Lua::event::sound, Lua::event::soundSimple);
		//eventTable["explosion"] = Lua::event::explosion;
		eventTable["bulletHit"] = Lua::event::bulletHit;
		eventTable["createBullet"] = Lua::event::createBullet;
	}

	{
		auto physicsTable = lua->create_table();
		(*lua)["physics"] = physicsTable;
		physicsTable["lineIntersectLevel"] = Lua::physics::lineIntersectLevel;
		physicsTable["lineIntersectHuman"] = Lua::physics::lineIntersectHuman;
		physicsTable["lineIntersectVehicle"] = Lua::physics::lineIntersectVehicle;
		physicsTable["lineIntersectTriangle"] = Lua::physics::lineIntersectTriangle;
		physicsTable["garbageCollectBullets"] = Lua::physics::garbageCollectBullets;
	}

	{
		auto chatTable = lua->create_table();
		(*lua)["chat"] = chatTable;
		chatTable["announce"] = Lua::chat::announce;
		chatTable["tellAdmins"] = Lua::chat::tellAdmins;
		chatTable["addRaw"] = Lua::chat::addRaw;
	}

	{
		auto accountsTable = lua->create_table();
		(*lua)["accounts"] = accountsTable;
		accountsTable["save"] = Lua::accounts::save;
		accountsTable["getCount"] = Lua::accounts::getCount;
		accountsTable["getAll"] = Lua::accounts::getAll;
		//accountsTable["getByPhone"] = Lua::accounts::getByPhone;

		sol::table _meta = lua->create_table();
		accountsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::accounts::getCount;
		_meta["__index"] = Lua::accounts::getByIndex;
	}

	{
		auto playersTable = lua->create_table();
		(*lua)["players"] = playersTable;
		playersTable["getCount"] = Lua::players::getCount;
		playersTable["getAll"] = Lua::players::getAll;
		//playersTable["getByPhone"] = Lua::players::getByPhone;
		playersTable["getNonBots"] = Lua::players::getNonBots;
		playersTable["createBot"] = Lua::players::createBot;

		sol::table _meta = lua->create_table();
		playersTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::players::getCount;
		_meta["__index"] = Lua::players::getByIndex;
	}

	{
		auto humansTable = lua->create_table();
		(*lua)["humans"] = humansTable;
		humansTable["getCount"] = Lua::humans::getCount;
		humansTable["getAll"] = Lua::humans::getAll;
		humansTable["create"] = Lua::humans::create;

		sol::table _meta = lua->create_table();
		humansTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::humans::getCount;
		_meta["__index"] = Lua::humans::getByIndex;
	}

	{
		auto itemTypesTable = lua->create_table();
		(*lua)["itemTypes"] = itemTypesTable;
		itemTypesTable["getCount"] = Lua::itemTypes::getCount;
		itemTypesTable["getAll"] = Lua::itemTypes::getAll;

		sol::table _meta = lua->create_table();
		itemTypesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::itemTypes::getCount;
		_meta["__index"] = Lua::itemTypes::getByIndex;
	}

	{
		auto itemsTable = lua->create_table();
		(*lua)["items"] = itemsTable;
		itemsTable["getCount"] = Lua::items::getCount;
		itemsTable["getAll"] = Lua::items::getAll;
		itemsTable["create"] =
		    sol::overload(Lua::items::create, Lua::items::createVel);
		itemsTable["createRope"] = Lua::items::createRope;

		sol::table _meta = lua->create_table();
		itemsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::items::getCount;
		_meta["__index"] = Lua::items::getByIndex;
	}

	{
		auto vehicleTypesTable = lua->create_table();
		(*lua)["vehicleTypes"] = vehicleTypesTable;
		vehicleTypesTable["getCount"] = Lua::vehicleTypes::getCount;
		vehicleTypesTable["getAll"] = Lua::vehicleTypes::getAll;
		vehicleTypesTable["getByName"] = Lua::vehicleTypes::getByName;

		sol::table _meta = lua->create_table();
		vehicleTypesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::vehicleTypes::getCount;
		_meta["__index"] = Lua::vehicleTypes::getByIndex;
	}

	{
		auto vehiclesTable = lua->create_table();
		(*lua)["vehicles"] = vehiclesTable;
		vehiclesTable["getCount"] = Lua::vehicles::getCount;
		vehiclesTable["getAll"] = Lua::vehicles::getAll;
		vehiclesTable["create"] = sol::overload(Lua::vehicles::create, Lua::vehicles::createVel);

		sol::table _meta = lua->create_table();
		vehiclesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::vehicles::getCount;
		_meta["__index"] = Lua::vehicles::getByIndex;
	}

	{
		auto bulletsTable = lua->create_table();
		(*lua)["bullets"] = bulletsTable;
		bulletsTable["getCount"] = Lua::bullets::getCount;
		bulletsTable["getAll"] = Lua::bullets::getAll;
		bulletsTable["create"] = Lua::bullets::create;

	}

	{
		auto particlesTable = lua->create_table();
		(*lua)["particles"] = particlesTable;
		particlesTable["getCount"] = Lua::particles::getCount;
		particlesTable["getAll"] = Lua::particles::getAll;

		sol::table _meta = lua->create_table();
		particlesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::particles::getCount;
		_meta["__index"] = Lua::particles::getByIndex;
	}

	{
		auto buildingsTable = lua->create_table();
		(*lua)["buildings"] = buildingsTable;
		buildingsTable["getCount"] = Lua::buildings::getCount;
		buildingsTable["getAll"] = Lua::buildings::getAll;

		sol::table _meta = lua->create_table();
		buildingsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::buildings::getCount;
		_meta["__index"] = Lua::buildings::getByIndex;
	}

	{
		auto bondsTable = lua->create_table();
		(*lua)["bonds"] = bondsTable;
		bondsTable["getCount"] = Lua::bonds::getCount;
		bondsTable["getAll"] = Lua::bonds::getAll;

		sol::table _meta = lua->create_table();
		bondsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::bonds::getCount;
		_meta["__index"] = Lua::bonds::getByIndex;
	}

	{
		auto streetsTable = lua->create_table();
		(*lua)["streets"] = streetsTable;
		streetsTable["getCount"] = Lua::streets::getCount;
		streetsTable["getAll"] = Lua::streets::getAll;

		sol::table _meta = lua->create_table();
		streetsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::streets::getCount;
		_meta["__index"] = Lua::streets::getByIndex;
	}

	{
		auto intersectionsTable = lua->create_table();
		(*lua)["intersections"] = intersectionsTable;
		intersectionsTable["getCount"] = Lua::intersections::getCount;
		intersectionsTable["getAll"] = Lua::intersections::getAll;

		sol::table _meta = lua->create_table();
		intersectionsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::intersections::getCount;
		_meta["__index"] = Lua::intersections::getByIndex;
	}

	{
		auto memoryTable = lua->create_table();
		(*lua)["memory"] = memoryTable;
		memoryTable["getBaseAddress"] = Lua::memory::getBaseAddress;
		memoryTable["getAddress"] = sol::overload(
		    &Lua::memory::getAddressOfConnection, &Lua::memory::getAddressOfAccount,
		    &Lua::memory::getAddressOfPlayer, &Lua::memory::getAddressOfHuman,
		    &Lua::memory::getAddressOfItemType, &Lua::memory::getAddressOfItem,
		    &Lua::memory::getAddressOfVehicle,&Lua::memory::getAddressOfVehicleType,
			 &Lua::memory::getAddressOfBullet,&Lua::memory::getAddressOfBuilding,
		    &Lua::memory::getAddressOfBone, &Lua::memory::getAddressOfParticle,
		    &Lua::memory::getAddressOfBond, &Lua::memory::getAddressOfAction,
		    &Lua::memory::getAddressOfMenuButton,
		    &Lua::memory::getAddressOfStreetLane, &Lua::memory::getAddressOfStreet,
		    &Lua::memory::getAddressOfStreetIntersection);
		memoryTable["readByte"] = Lua::memory::readByte;
		memoryTable["readUByte"] = Lua::memory::readUByte;
		memoryTable["readShort"] = Lua::memory::readShort;
		memoryTable["readUShort"] = Lua::memory::readUShort;
		memoryTable["readInt"] = Lua::memory::readInt;
		memoryTable["readUInt"] = Lua::memory::readUInt;
		memoryTable["readLong"] = Lua::memory::readLong;
		memoryTable["readULong"] = Lua::memory::readULong;
		memoryTable["readFloat"] = Lua::memory::readFloat;
		memoryTable["readDouble"] = Lua::memory::readDouble;
		memoryTable["readBytes"] = Lua::memory::readBytes;
		memoryTable["writeByte"] = Lua::memory::writeByte;
		memoryTable["writeUByte"] = Lua::memory::writeUByte;
		memoryTable["writeShort"] = Lua::memory::writeShort;
		memoryTable["writeUShort"] = Lua::memory::writeUShort;
		memoryTable["writeInt"] = Lua::memory::writeInt;
		memoryTable["writeUInt"] = Lua::memory::writeUInt;
		memoryTable["writeLong"] = Lua::memory::writeLong;
		memoryTable["writeULong"] = Lua::memory::writeULong;
		memoryTable["writeFloat"] = Lua::memory::writeFloat;
		memoryTable["writeDouble"] = Lua::memory::writeDouble;
		memoryTable["writeBytes"] = Lua::memory::writeBytes;
	}

	(*lua)["RESET_REASON_BOOT"] = RESET_REASON_BOOT;
	(*lua)["RESET_REASON_ENGINECALL"] = RESET_REASON_ENGINECALL;
	(*lua)["RESET_REASON_LUARESET"] = RESET_REASON_LUARESET;
	(*lua)["RESET_REASON_LUACALL"] = RESET_REASON_LUACALL;

	(*lua)["STATE_PREGAME"] = 1;
	(*lua)["STATE_GAME"] = 2;
	(*lua)["STATE_RESTARTING"] = 3;

	(*lua)["TYPE_DRIVING"] = 1;
	(*lua)["TYPE_RACE"] = 2;
	(*lua)["TYPE_ROUND"] = 3;
	(*lua)["TYPE_WORLD"] = 4;
	(*lua)["TYPE_TERMINATOR"] = 5;
	(*lua)["TYPE_COOP"] = 6;
	(*lua)["TYPE_VERSUS"] = 7;

	Console::log(LUA_PREFIX "Running " LUA_ENTRY_FILE "...\n");
	//std::filesystem::path currentPath = std::filesystem::current_path();
	//Console::log(LUA_PREFIX "Current directory " currentPath "...\n");
	std::filesystem::path currentPath = std::filesystem::current_path();
	Console::log(std::string(LUA_PREFIX) + "Current directory: " + currentPath.string() + "...\n");
	std::filesystem::path parentPath = currentPath.parent_path();
	std::filesystem::path dataPath = "/home/container/data";
	if (currentPath == dataPath) { //A weird issue where it wants to be in container/data, I am too stupid to fix it, I LOVE HARDCODING STUFF!
		std::filesystem::current_path(parentPath);
	}
	sol::load_result load = lua->load_file(LUA_ENTRY_FILE);
	if (noLuaCallError(&load)) {
		sol::protected_function_result res = load();
		if (noLuaCallError(&res)) {
			Console::log(LUA_PREFIX "No problems!\n");
		}
	}
}

static inline uintptr_t getBaseAddress() {
	std::ifstream file("/proc/self/maps");
	std::string line;
	// First line
	std::getline(file, line);
	auto pos = line.find("-");
	auto truncated = line.substr(0, pos);

	return std::stoul(truncated, nullptr, 16);
}

static inline void printBaseAddress(uintptr_t base) {
	std::ostringstream stream;

	stream << RS_PREFIX "Base address is ";
	stream << std::showbase << std::hex;
	stream << base;
	stream << "\n";

	Console::log(stream.str());
}

//Anything labeled with 0x18db0ba4 is not real.

static inline void locateMemory(uintptr_t base) {
	version = (unsigned int*)(base + 0x2ab660);
	//subVersion = (unsigned int*)(base + 0x2ab664);
	serverName = (char*)(base + 0x13b6bc94);
	serverPort = (unsigned int*)(base + 0x22AE4FF8);
	serverMaxBytesPerSecond = (unsigned int*)(base + 0xda182e4);
	numEvents = (unsigned int*)(base + 0x24b88188);
	numConnections = (unsigned int*)(base + 0x24b883a8);
	numBonds = (unsigned int*)(base + 0x8444A60);
	numParticles = (unsigned int*)(base + 0x26F8E230);
	numBuildings = (unsigned int*)(base + 0x20134a18);
	//isPassworded = (int*)(base + 0x18db0ba4);
	adminPassword = (char*)(base + 0xDA18368);
	maxPlayers = (int*)(base + 0x13b6bcdc);

	Engine::gameType = (int*)(base + 0x24B62428);
	//Engine::mapName = (char*)(base + 0x18db0ba4); // HARD CODED ONLY ONE MAP PER LOL
	//Engine::loadedMapName = (char*)(base + 0x18db0ba4); // HARD CODED ONLY ONE MAP PER LOL
	Engine::gameState = (int*)(base + 0x24B62440);
	Engine::gameTimer = (int*)(base + 0x24B62444);
	Engine::gameTicksSinceReset = (int*)(base + 0x24B62458);
	Engine::sunTime = (unsigned int*)(base + 0xbe385c0);
	doVoiceChat = (int*)(base + 0xda1840c);
	gravity = (float*)(base + 0xa5cac);
	//pryMemory(gravity, 1);
	originalGravity = *gravity;

	Engine::lineIntersectResult = (RayCastResult*)(base + 0x26104460);

	Engine::connections = (Connection*)(base + 0x2add00);
	Engine::accounts = (Account*)(base + 0x11c4d00);
	Engine::players = (Player*)(base + 0xd68c160);
	Engine::humans = (Human*)(base + 0x8890680);
	Engine::vehicles = (Vehicle*)(base + 0xDC18B40);
	Engine::vehicleTypes = (VehicleType*)(base + 0x3D480A0);
	Engine::itemTypes = (ItemType*)(base + 0x270B47C0);
	Engine::items = (Item*)(base + 0x8444a80);
	Engine::bullets = (Bullet*)(base + 0x24de2280);
	Engine::particles = (Particle*)(base + 0x26A376E0);
	Engine::bonds = (Bond*)(base + 0x136bbc80);
	Engine::buildings = (Building*)(base + 0x23D34A5C);
	Engine::numBuildings = (unsigned int*)(base + 0x23D34A18);
	Engine::streets = (Street*)(base + 0x2411f08c);// COULD NOT FIND// PROBABLY HAS A DIF NAME?
	Engine::streetIntersections = (StreetIntersection*)(base + 0x23D05088);
	Engine::numBonds = (unsigned int*)(base + 0x8444A60);
	Engine::numParticles = (unsigned int*)(base + 0x26F8E230);
	Engine::numConnections = (unsigned int*)(base + 0x24b883a8);
	Engine::numBullets = (unsigned int*)(base + 0x24b88184);
	Engine::numStreets = (unsigned int*)(base + 0x23D1F088); // COULD NOT FIND// PROBABLY HAS A DIF NAME?
	Engine::numStreetIntersections = (unsigned int*)(base + 0x23D05080);

	Engine::subRosaPuts = (Engine::subRosaPutsFunc)(base + 0x12a0);
	Engine::subRosa__printf_chk = (Engine::subRosa__printf_chkFunc)(base + 0x1340);

	Engine::resetGame = (Engine::voidFunc)(base + 0x4097a);
	//Engine::addTraffic = (Engine::voidFunc)(base + 0x87987);
	Engine::createTraffic = (Engine::createTrafficFunc)(base + 0x87987);
	Engine::logicSimulation = (Engine::voidFunc)(base + 0x6434a);
	Engine::logicSimulationRace = (Engine::voidFunc)(base + 0x64e1d);
	Engine::logicSimulationRound = (Engine::voidFunc)(base + 0x69631);
	Engine::logicSimulationWorld = (Engine::voidFunc)(base + 0x6a114);
	//Engine::logicSimulationTerminator = (Engine::voidFunc)(base + 0x18db0ba4); // THIS GAME TYPE DOES NOT EXIST IN 24C
	//Engine::logicSimulationCoop = (Engine::voidFunc)(base + 0x18db0ba4); // THIS GAME TYPE DOES NOT EXIST IN 24C
	//Engine::logicSimulationVersus = (Engine::voidFunc)(base + 0x18db0ba4); // THIS GAME TYPE DOES NOT EXIST IN 24C
	Engine::logicPlayerActions = (Engine::voidIndexFunc)(base + 0x67265);
	Engine::itemWeaponSimulation = (Engine::voidIndexFunc)(base + 0x5aed2);
	Engine::trainSimulation = (Engine::voidIndexFunc)(base + 0x81f42);
	Engine::humanCalculateArmAngles = (Engine::voidIndexFunc)(base + 0x4b445);
	Engine::humanCollideHuman = (Engine::voidIndexFunc)(base + 0x50bd3);

	Engine::physicsSimulation = (Engine::voidFunc)(base + 0x84eb6);
	Engine::serverReceive = (Engine::serverReceiveFunc)(base + 0x8b20d);
	Engine::serverSend = (Engine::voidFunc)(base + 0x8a2a6);
	Engine::writePacket = (Engine::writePacketFunc)(base + 0x6f1ff);
	Engine::sendPacket = (Engine::sendPacketFunc)(base + 0xa4925);
	Engine::bulletSimulation = (Engine::voidFunc)(base + 0x31e3d);
	Engine::bondSimulation = (Engine::voidFunc)(base + 0x1f28a);
	Engine::vehicleSimulation = (Engine::voidFunc)( base + 0x74412);
	Engine::bulletTimeToLive = (Engine::voidFunc)(base + 0x31cef);
	Engine::economyCarMarket = (Engine::voidFunc)(base + 0x3ad71);

	Engine::saveAccountsServer = (Engine::voidFunc)(base + 0x1e8d);

	Engine::createAccountByJoinTicket = (Engine::createAccountByJoinTicketFunc)(base + 0x1734);
	Engine::serverSendConnectResponse = (Engine::serverSendConnectResponseFunc)(base + 0x18db0ba4);// DOESNT EXIST / HANDLED DIFFERENTLY? / COULDNT FIND

	Engine::scenarioArmHuman = (Engine::scenarioArmHumanFunc)(base + 0x88c35);
	Engine::linkItem = (Engine::linkItemFunc)(base + 0x5c2c1);
	//Engine::itemSetMemo = (Engine::itemSetMemoFunc)(base + 0x18db0ba4); // DOES NOT EXIST // MEMOS NOT IN THE GAME AS EXPECTED
	//Engine::itemComputerTransmitLine = (Engine::itemComputerTransmitLineFunc)(base + 0x18db0ba4); // COMPUTERS ARE NOT IN 24C
	//Engine::itemComputerIncrementLine = (Engine::voidIndexFunc)(base + 0x18db0ba4); // COMPUTERS ARE NOT IN 24C
	//Engine::itemComputerInput = (Engine::itemComputerInputFunc)(base + 0x18db0ba4); // COMPUTERS ARE NOT IN 24C
	Engine::itemCashAddBill = (Engine::itemCashAddBillFunc)(base + 0x5bf4b);
	Engine::itemCashRemoveBill = (Engine::itemCashRemoveBillFunc)(base + 0x5c14b);
	Engine::itemCashGetBillValue = (Engine::itemCashGetBillValueFunc)(base + 0x5bed6);
	Engine::humanApplyDamage = (Engine::humanApplyDamageFunc)(base + 0x4a7dc); //NEVER CALLED IN THE BASE GAME
	Engine::vehicleApplyDamage = (Engine::vehicleApplyDamageFunc)(base + 0x77cf2);
	//Engine::humanCollisionVehicle = (Engine::humanCollisionVehicleFunc)(base + 0x18db0ba4); // DOES NOT EXIST
	//Engine::humanGrabbing = (Engine::voidIndexFunc)(base + 0x18db0ba4);// DOES NOT EXIST
	//Engine::grenadeExplosion = (Engine::voidIndexFunc)(base + 0x18db0ba4);// DOES NOT EXIST
	Engine::serverPlayerMessage = (Engine::serverPlayerMessageFunc)(base + 0x391dc);
	Engine::playerAI = (Engine::voidIndexFunc)(base + 0x2cc3);
	Engine::playerDeathTax = (Engine::voidIndexFunc)(base + 0x3bede);
	//Engine::createBondRigidBodyToRigidBody = (Engine::createBondRigidBodyToRigidBodyFunc)(base + 0x18db0ba4); // DOESNT EXIST / HANDLED DIFFERENTLY? / COULDNT FIND
	//Engine::createBondRigidBodyRotRigidBody = (Engine::createBondRigidBodyRotRigidBodyFunc)(base + 0x18db0ba4);// DOESNT EXIST / HANDLED DIFFERENTLY? / COULDNT FIND
	//Engine::createBondRigidBodyToLevel = (Engine::createBondRigidBodyToLevelFunc)(base + 0x18db0ba4);// DOESNT EXIST / HANDLED DIFFERENTLY? / COULDNT FIND
	//Engine::addCollisionRigidBodyOnRigidBody = (Engine::addCollisionRigidBodyOnRigidBodyFunc)(base + 0x18db0ba4);// DOESNT EXIST / HANDLED DIFFERENTLY? / COULDNT FIND
	//Engine::addCollisionRigidBodyOnLevel = (Engine::addCollisionRigidBodyOnLevelFunc)(base + 0x18db0ba4);// DOESNT EXIST / HANDLED DIFFERENTLY? / COULDNT FIND

	Engine::createPlayer = (Engine::createPlayerFunc)(base + 0x85049);
	Engine::deletePlayer = (Engine::voidIndexFunc)(base + 0x85417);
	Engine::createHuman = (Engine::createHumanFunc)(base + 0x4350e);
	Engine::deleteHuman = (Engine::voidIndexFunc)(base + 0x44221);
	Engine::createItem = (Engine::createItemFunc)(base + 0x5800a);
	Engine::deleteItem = (Engine::voidIndexFunc)(base + 0x58728);
	Engine::createBullet = (Engine::createBulletFunc)(base + 0x318ed);
	//Engine::createRope = (Engine::createRopeFunc)(base + 0x18db0ba4); // DOESNT EXIST IN 24C
	Engine::createVehicle = (Engine::createVehicleFunc)(base + 0x70F40);
	Engine::deleteVehicle = (Engine::voidIndexFunc)(base + 0x73746);
	Engine::createParticle = (Engine::createParticleFunc)(base + 0x845b2);// DOESNT EXIST IN 24C, VEHICLES AND RIGIDBODIES ARE THE SAME OR SOME SHIT

	Engine::createEventMessage = (Engine::createEventMessageFunc)(base + 0x3d808);
	Engine::createEventUpdateHuman = (Engine::voidIndexFunc)(base + 0x3e05a);
	Engine::createEventUpdateItem = (Engine::voidIndexFunc)(base + 0x3dbab);
	Engine::createEventUpdateItemInfo = (Engine::voidIndexFunc)(base + 0x3dd00);
	Engine::createEventUpdatePlayer = (Engine::voidIndexFunc)(base + 0x3de21);
	Engine::createEventUpdatePlayerFinance = (Engine::voidIndexFunc)(base + 0x3e136);
	Engine::createEventCreateVehicle = (Engine::voidIndexFunc)(base + 0x3d965);
	Engine::createEventUpdateVehicle = (Engine::createEventUpdateVehicleFunc)(base + 0x3da42);
	Engine::createEventSound = (Engine::createEventSoundFunc)(base + 0x3e212);
	//Engine::createEventExplosion = (Engine::createEventExplosionFunc)(base + 0x18db0ba4); //DOESNT EXIST IN 24C
	Engine::createEventBulletHit = (Engine::createEventBulletHitFunc)(base + 0x3d6e1);
	Engine::createEventBullet = (Engine::createEventBulletFunc)(base + 0x3d5e5);
	Engine::lineIntersectHuman = (Engine::lineIntersectHumanFunc)(base + 0x4a679);
	Engine::lineIntersectLevel = (Engine::lineIntersectLevelFunc)(base + 0x60a2f);
	Engine::lineIntersectVehicle = (Engine::lineIntersectVehicleFunc)(base + 0x7eacb);
	Engine::lineIntersectTriangle = (Engine::lineIntersectTriangleFunc)(base + 0xa2d1d);
}

static inline void installHook(
    const char* name, subhook::Hook& hook, void* source, void* destination,
    subhook::HookFlags flags = subhook::HookFlags::HookFlag64BitOffset) {
	if (!hook.Install(source, destination, flags)) {
		std::ostringstream stream;
		stream << RS_PREFIX "Hook " << name << " failed to install";

		throw std::runtime_error(stream.str());
	}
}

#define INSTALL(name)                                               \
	installHook(#name "Hook", Hooks::name##Hook, (void*)Engine::name, \
	            (void*)Hooks::name);

static inline void installHooks() {
	//  INSTALL(subRosaPuts);
	//  INSTALL(subRosaPuts);
	//  INSTALL(subRosa__printf_chk);
	INSTALL(resetGame);
	//Console::log(RS_PREFIX "Attempting to install logicSimulation.\n");
	INSTALL(logicSimulation);
	//Console::log(RS_PREFIX "Installed logicSimulation.\n");

	INSTALL(logicSimulationRace);
	INSTALL(logicSimulationRound);
	INSTALL(logicSimulationWorld);
	//INSTALL(logicSimulationTerminator);
	//INSTALL(logicSimulationCoop);
	//INSTALL(logicSimulationVersus);
	INSTALL(logicPlayerActions);
	INSTALL(itemWeaponSimulation);
	INSTALL(trainSimulation);
	INSTALL(humanCalculateArmAngles);
	INSTALL(humanCollideHuman);
	//Console::log(RS_PREFIX "Attempting to install physicsSimulation.\n");
	INSTALL(physicsSimulation);
	//Console::log(RS_PREFIX "Installed physicsSimulation.\n");
	INSTALL(serverReceive);
	INSTALL(serverSend);
	INSTALL(writePacket);
	INSTALL(sendPacket);
	INSTALL(bulletSimulation);
	INSTALL(bondSimulation);
	INSTALL(economyCarMarket);
	INSTALL(vehicleSimulation);
	INSTALL(saveAccountsServer);
	//INSTALL(createAccountByJoinTicket);
	//INSTALL(serverSendConnectResponse);
	INSTALL(linkItem);
	//INSTALL(itemComputerInput);
	INSTALL(humanApplyDamage);
	INSTALL(vehicleApplyDamage);
	//INSTALL(humanCollisionVehicle);
	//INSTALL(humanGrabbing);
	//INSTALL(grenadeExplosion);
	INSTALL(serverPlayerMessage);
	INSTALL(playerAI);
	INSTALL(playerDeathTax);
	//INSTALL(addCollisionRigidBodyOnRigidBody);
	INSTALL(createPlayer);
	INSTALL(deletePlayer);
	INSTALL(createHuman);
	INSTALL(deleteHuman);
	INSTALL(createItem);
	INSTALL(deleteItem);
	INSTALL(createBullet);
	INSTALL(createVehicle);
	INSTALL(deleteVehicle);
	INSTALL(createTraffic);
	//  INSTALL(createParticle);
	INSTALL(createEventMessage);
	INSTALL(createEventUpdatePlayer);
	INSTALL(createEventUpdateHuman);
	INSTALL(createEventUpdateItem);
	INSTALL(createEventUpdateItemInfo);
	INSTALL(createEventCreateVehicle);
	INSTALL(createEventUpdateVehicle);
	INSTALL(createEventBulletHit);
	INSTALL(createEventBullet);
	INSTALL(lineIntersectHuman);
}

static inline void attachSignalHandler() {
	struct sigaction action;
	action.sa_handler = Console::handleInterruptSignal;

	if (sigaction(SIGINT, &action, nullptr) == -1) {
		throw std::runtime_error(strerror(errno));
	}
}
// start of signal crap

static void crashSignalHandler(int signal) {
	Console::shouldExit = true;

	std::stringstream sstream;
	std::cerr << std::flush;
	sstream << "\033[41;1m " << strsignal(signal) << " \033[0m\n\033[31m";

	sstream << "Stack traceback:\n";

	void* backtraceEntries[10];

	size_t backtraceSize = backtrace(backtraceEntries, 10);
	auto backtraceSymbols = backtrace_symbols(backtraceEntries, backtraceSize);

	for (int i = 0; i < backtraceSize; i++) {
		sstream << "\t#" << i << ' ' << backtraceSymbols[i] << '\n';
	}

	sstream << std::flush;

	luaL_traceback(*lua, *lua, nullptr, 0);
	sstream << "Lua " << lua_tostring(*lua, -1);

	sstream << "\033[0m" << std::endl;

	std::cerr << sstream.str();

	std::filesystem::remove(std::filesystem::path("rs_crash_report.txt"));
	std::ofstream file("rs_crash_report.txt");
	if (file.is_open()) {
		file << sstream.str();
		file.flush();
		file.close();
	}

	raise(signal);
	kill(0, SIGTERM);
	_exit(EXIT_FAILURE);
}

static const std::array handledSignals = {
    SIGABRT, SIGBUS, SIGFPE,  SIGILL,  SIGQUIT,
    SIGSEGV, SIGSYS, SIGTRAP, SIGXCPU, SIGXFSZ,
};

static inline void attachCrashSignalHandler() {
	struct sigaction signalAction;

	signalAction.sa_handler = crashSignalHandler;
	signalAction.sa_flags = SA_RESTART | SA_SIGINFO;

	for (const auto signal : handledSignals) {
		if (sigaction(signal, &signalAction, nullptr) == -1) {
			std::ostringstream stream;
			stream << RS_PREFIX "Signal handler failed to attach for signal "
			       << signal << " (" << strsignal(signal) << "): " << strerror(errno);

			throw std::runtime_error(stream.str());
		}
	}
}

// end of signal crap


static void attach() {
	// Don't load self into future child processes
	unsetenv("LD_PRELOAD");

	attachSignalHandler();
	attachCrashSignalHandler();
	Console::log(RS_PREFIX "Assuming 24c\n");

	Console::log(RS_PREFIX "Locating memory...\n");
	Lua::memory::baseAddress = getBaseAddress();
	printBaseAddress(Lua::memory::baseAddress);
	locateMemory(Lua::memory::baseAddress);
	Console::log(RS_PREFIX "Base Address located.\n");
	Console::log(RS_PREFIX "Installing hooks...\n");
	installHooks();

	Console::log(RS_PREFIX "Waiting for engine init...\n");

	unsetenv("LD_PRELOAD");
}

int __attribute__((constructor)) Entry() {
	std::thread mainThread(attach);
	mainThread.detach();
	return 0;
}

int __attribute__((destructor)) Destroy() {
	if (lua != nullptr) {
		delete lua;
		lua = nullptr;
	}
	return 0;
}