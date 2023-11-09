#include <vector> //only to be used to contain dispenser states and nothing else
#include <iostream>
#include <string>

using namespace std;

enum state { NO_NPCS, NO_RESOURCES, HAVE_RESOURCES, SPAWN_NPC };
enum stateParameter { NPCS, RESOURCES };

#pragma region StateClasses

class StateContext;

class State
{
protected:
	StateContext* CurrentContext;
public:
	State(StateContext* Context)
	{
		CurrentContext = Context;
	}
	virtual ~State(void)
	{
	}
};

class StateContext
{
protected:

	State* CurrentState = nullptr;
	int stateIndex = 0;
	vector<State*> availableStates;
	vector<int> stateParameters;

public:

	virtual ~StateContext()
	{
		for (int index = 0; index < this->availableStates.size(); index++) delete this->availableStates[index];
		this->availableStates.clear();
		this->stateParameters.clear();
	}

	virtual void setState(state newState)
	{
		this->CurrentState = availableStates[newState];
		this->stateIndex = newState;
	}

	virtual int getStateIndex(void)
	{
		return this->stateIndex;
	}

	virtual void setStateParam(stateParameter SP, int value)
	{
		this->stateParameters[SP] = value;
	}

	virtual int getStateParam(stateParameter SP)
	{
		return this->stateParameters[SP];
	}
};
#pragma endregion

class Transition
{
public:
	virtual bool addResource(int) { cout << "Error!" << endl; return false; }
	virtual bool makeSelection(int) { cout << "Error!" << endl; return false; }
	virtual bool returnResource(void) { cout << "Error!" << endl; return false; }
	virtual bool addNPCs(int) { cout << "Error!" << endl; return false; }
	virtual bool spawn(void) { cout << "Error!" << endl; return false; }
};

class NPCState : public State, public Transition
{
public:
	NPCState(StateContext* Context) : State(Context) {}
};

#pragma region NPC_Related

class Weapon;

class NPC
{
protected:
	string _description = "NPC";
	int _cost = 0;
	int _damage = 0;
public:
	string description(void) { return this->_description; };
	int cost(void) { return this->_cost; }
	int damage(void) { return this->_damage; }
	virtual int bestDamage(int best_weapon_damage = 0) { return damage(); }; //returns the best damage an NPC can do - i.e. base damage + highest damage weapon. Should return 0 if not implemented
	virtual int costAll(void) { return this->cost(); } //returns NPC base cost
	virtual string describeAll(void) { return this->description(); } //returns description of NPC
	virtual Weapon* ReturnHighestCostWeapon(void) { return nullptr; } //returns nullptr
};

class Peon : public NPC					//OPTION CODE == 1
{
public:
	Peon(void)
	{
		this->_description = "Peon";
		this->_cost = 100;
		this->_damage = 10;
	}
};
class Grunt : public NPC				//OPTION CODE == 2
{
public:
	Grunt(void)
	{
		this->_description = "Grunt";
		this->_cost = 150;
		this->_damage = 20;
	}
};
class Raider : public NPC				//OPTION CODE == 4
{
public:
	Raider(void)
	{
		this->_description = "Raider";
		this->_cost = 200;
		this->_damage = 30;
	}
};
class Tauren : public NPC				//OPTION CODE == 8
{
public:
	Tauren(void)
	{
		this->_description = "Tauren";
		this->_cost = 250;
		this->_damage = 40;
	}
};
class Shaman : public NPC				//OPTION CODE == 16
{
public:
	Shaman(void)
	{
		this->_description = "Shaman";
		this->_cost = 300;
		this->_damage = 50;
	}
};

#pragma region NPC_Weapons

class Weapon : public NPC
{
protected:
	NPC* FilledNPC = nullptr; //_component
public:
	~Weapon(void)
	{
		delete this->FilledNPC; //Deletion of _component
	}
	void equipNPC(NPC* NewNPC)
	{
		this->FilledNPC = NewNPC; //Wrapping of _component
	}
	int costAll(void) 
	{
		return this->_cost + FilledNPC->costAll(); //Recursive cost()
	}
	int bestDamage(int best_weapon_damage = 0)
	{
		if (best_weapon_damage > this->_damage)
			return best_weapon_damage + FilledNPC->damage();
		if (best_weapon_damage < this->_damage)
			FilledNPC->bestDamage(this->_damage);
		else
			return 0; //returns the best damage an NPC can do - i.e. base damage + highest damage weapon
	}
	string describeAll(void)
	{
		return FilledNPC->describeAll() + ", " + this->_description;
	}
	Weapon* ReturnHighestCostWeapon(void)
	{
		return this;//returns a pointer to the high cost weapon. Should return nullptr if not implemented
	}
};

class Dagger : public Weapon
{
public:
	Dagger(void)
	{
		this->_description = "Dagger";
		this->_cost = 20;
		this->_damage = 20;
	}
};
class Staff : public Weapon
{
public:
	Staff(void)
	{
		this->_description = "Staff";
		this->_cost = 20;
		this->_damage = 20;
	}
};
class Spear : public Weapon
{
public:
	Spear(void)
	{
		this->_description = "Spear";
		this->_cost = 50;
		this->_damage = 30;
	}
};
class Axe : public Weapon
{
public:
	Axe(void)
	{
		this->_description = "Axe";
		this->_cost = 70;
		this->_damage = 40;
	}
};
class Sword : public Weapon
{
public:
	Sword(void)
	{
		this->_description = "Sword";
		this->_cost = 100;
		this->_damage = 50;
	}
};
#pragma endregion

#pragma endregion

class NPCGenerator : public StateContext, public Transition
{
	friend class SpawnNPC;
	friend class HaveResources;
private:
	NPCState* NPCCurrentState = nullptr;
	bool itemDispensed = false; //indicates whether a NPC is there to be retrieved - needed for ownership managemnt
	NPC* SpawnedNPC = nullptr; //
	bool itemRetrieved = false; //indicates whether a NPC has been retrieved - needed for ownership managemnt
public:
	NPCGenerator(int inventory_count);
	~NPCGenerator(void);
	bool addResource(int resource);
	bool makeSelection(int option);
	bool returnResource(void);
	bool addNPCs(int number);
	bool spawn(void);
	NPC* getNPC(void);
	virtual void setStateParam(stateParameter SP, int value);
	virtual int getStateParam(stateParameter SP);
};

#pragma region NPC_State_Related

class NoNPCs : public NPCState
{
public:
	NoNPCs(StateContext* Context) : NPCState(Context) {}
	bool addResource(int resource)
	{
		cout << "Error! No NPC's available." << endl;
		return returnResource();
	}
	bool makeSelection(int option)
	{
		cout << "Error! No NPC's to spawn." << endl;
		return false;
	}
	bool returnResource(void)
	{
		cout << "Destroying resources!" << endl;
		this->CurrentContext->setStateParam(RESOURCES, 0);
		this->CurrentContext->setState(NO_NPCS);
		return true;
	}
	bool addNPCs(int number)
	{
		this->CurrentContext->setStateParam(NPCS, number);
		this->CurrentContext->setState(NO_RESOURCES);
		return true;
	}
	bool spawn(void)
	{
		cout << "Error! No NPC's to spawn." << endl;
		return false;
	}
};

class NoResources : public NPCState
{
public:
	NoResources(StateContext* Context) : NPCState(Context) {}
	bool addResource(int resource)
	{
		cout << "Added: " << resource;
		this->CurrentContext->setStateParam(RESOURCES, resource);
		cout << "\t\tTotal: " << resource << endl;
		this->CurrentContext->setState(HAVE_RESOURCES);
		return true;
	}
	bool makeSelection(int option)
	{
		cout << "Error! No resources available to use." << endl;
		return false;
	}
	bool returnResource(void)
	{
		cout << "Error! No resources available to return." << endl;
		return false;
	}
	bool addNPCs(int number)
	{
		cout << "Error! Already have NPC's." << endl;
		return false;
	}
	bool spawn(void)
	{
		cout << "Error! No resources available to use." << endl;
		return false;
	}
};

class HaveResources : public NPCState
{
public:
	HaveResources(StateContext* Context) : NPCState(Context) {}
	bool addResource(int resource)
	{
		cout << "Added: " << resource;
		resource = resource + this->CurrentContext->getStateParam(RESOURCES);
		this->CurrentContext->setStateParam(RESOURCES, resource);
		cout << "\t\tTotal: " << resource << endl;
		this->CurrentContext->setState(HAVE_RESOURCES);
		return true;
	}
	bool makeSelection(int option)
	{
		if (!((NPCGenerator*)this->CurrentContext)->itemRetrieved)
		{
			delete ((NPCGenerator*)this->CurrentContext)->SpawnedNPC;
			//Delete NPC if one was spawned but not retrieved
		}
		if (option & 1)								//Option code: 1 [PEON]
		{
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = new Peon;
		}
		if (option & 2)								//Option code: 2 [GRUNT]
		{
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = new Grunt;
		}
		if (option & 4)								//Option code: 4 [RAIDER]
		{
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = new Raider;
		}
		if (option & 8)								//Option code: 8 [TAUREN]
		{
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = new Tauren;
		}
		if (option & 16)							//Option code: 16 [SHAMAN]
		{
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = new Shaman;
		}

		if (option & 32)							//Option code: 32 [DAGGER]
		{
			Weapon* dagger = new Dagger;
			dagger->equipNPC(((NPCGenerator*)this->CurrentContext)->SpawnedNPC);
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = dagger;
		}
		if (option & 64)							//Option code: 64 [STAFF]
		{
			Weapon* staff = new Staff;
			staff->equipNPC(((NPCGenerator*)this->CurrentContext)->SpawnedNPC);
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = staff;
		}
		if (option & 128)							//Option code: 128 [SPEAR]
		{
			Weapon* spear = new Spear;
			spear->equipNPC(((NPCGenerator*)this->CurrentContext)->SpawnedNPC);
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = spear;
		}
		if (option & 256)							//Option code: 256 [AXE]
		{
			Weapon* axe = new Axe;
			axe->equipNPC(((NPCGenerator*)this->CurrentContext)->SpawnedNPC);
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = axe;
		}
		if (option & 512)							//Option code: 512 [SWORD]
		{
			Weapon* sword = new Sword;
			sword->equipNPC(((NPCGenerator*)this->CurrentContext)->SpawnedNPC);
			((NPCGenerator*)this->CurrentContext)->SpawnedNPC = sword;
		}
		((NPCGenerator*)this->CurrentContext)->itemRetrieved = false;
		this->CurrentContext->setState(SPAWN_NPC);
		return true;
	}
	bool returnResource(void)
	{
		cout << "Destroying resources!" << endl;
		this->CurrentContext->setStateParam(RESOURCES, 0);
		this->CurrentContext->setState(NO_RESOURCES);
		return true;
	}
	bool addNPCs(int number)
	{
		cout << "Error! Already have NPC's to spawn." << endl;
		return false;
	}
	bool spawn(void)
	{
		cout << "Error! No selection made." << endl;
		return false;
	}
};

class SpawnNPC : public NPCState
{
public:
	SpawnNPC(StateContext* Context) : NPCState(Context) {}
	bool addResource(int resource)
	{
		cout << "Error! Busy Spawning NPC!" << endl;
		return false;
	}
	bool makeSelection(int option)
	{
		cout << "Error! Busy Spawning NPC!" << endl;
		return false;
	}
	bool returnResource(void)
	{
		cout << "Error! Busy Spawning NPC!" << endl;
		return false;
	}
	bool addNPCs(int number)
	{
		cout << "Error! Busy Spawning NPC!" << endl;
		return false;
	}
	bool spawn(void)
	{
		if (this->CurrentContext->getStateParam(RESOURCES) >= ((NPCGenerator*)this->CurrentContext)->SpawnedNPC->costAll())
		{
			cout << "Spawn NPC -\t" << ((NPCGenerator*)this->CurrentContext)->SpawnedNPC->describeAll() << "\n";
			cout << "Spawn Cost -\t" << ((NPCGenerator*)this->CurrentContext)->SpawnedNPC->costAll() << "\n";
			cout << "Best Damage -\t" << ((NPCGenerator*)this->CurrentContext)->SpawnedNPC->bestDamage() << "\n";
			if(((NPCGenerator*)this->CurrentContext)->SpawnedNPC->ReturnHighestCostWeapon() != nullptr)
				cout << "Highest Cost -\t" << ((NPCGenerator*)this->CurrentContext)->SpawnedNPC->ReturnHighestCostWeapon()->cost() << "\n";
			this->CurrentContext->setStateParam(RESOURCES, this->CurrentContext->getStateParam(RESOURCES) - ((NPCGenerator*)this->CurrentContext)->SpawnedNPC->costAll());
			this->CurrentContext->setStateParam(NPCS, this->CurrentContext->getStateParam(NPCS) - 1);
			((NPCGenerator*)this->CurrentContext)->itemDispensed = true;
			cout << "Resources after Spawning = " << this->CurrentContext->getStateParam(RESOURCES) << "\n" << endl;
		}
		else
		{
			cout << "Error! You lack the resources to perform this action." << endl;
		}

		if (this->CurrentContext->getStateParam(RESOURCES) > 0)
		{
			this->CurrentContext->setState(HAVE_RESOURCES);
		}
		else
		{
			this->CurrentContext->setState(NO_RESOURCES);
		}

		if (this->CurrentContext->getStateParam(NPCS) == 0)
		{
			this->CurrentContext->setState(NO_NPCS);
		}
		return true;
	}
};
#pragma endregion

#pragma region NPC_Generator_Function_Definitions

NPCGenerator::NPCGenerator(int inventory_count)
{
	this->availableStates.push_back(new NoNPCs(this));
	this->availableStates.push_back(new NoResources(this));
	this->availableStates.push_back(new HaveResources(this));
	this->availableStates.push_back(new SpawnNPC(this));
	this->stateParameters.push_back(0);//No of NPC's
	this->stateParameters.push_back(0);//Available Resources

	this->setState(NO_NPCS);
	if (inventory_count > 0)
	{
		this->addNPCs(inventory_count);
	}
}

NPCGenerator::~NPCGenerator(void)
{
	if (!this->itemRetrieved)
	{
		delete this->SpawnedNPC;
	}
}

bool NPCGenerator::addResource(int resource)
{
	NPCCurrentState = (NPCState*)this->CurrentState;
	return this->NPCCurrentState->addResource(resource);
}

bool NPCGenerator::makeSelection(int option)
{
	NPCCurrentState = (NPCState*)this->CurrentState;
	return this->NPCCurrentState->makeSelection(option);
}

bool NPCGenerator::returnResource(void)
{
	NPCCurrentState = (NPCState*)this->CurrentState;
	return this->NPCCurrentState->returnResource();
}

bool NPCGenerator::addNPCs(int number)
{
	NPCCurrentState = (NPCState*)this->CurrentState;
	return this->NPCCurrentState->addNPCs(number);
}

bool NPCGenerator::spawn(void)
{
	NPCCurrentState = (NPCState*)this->CurrentState;
	return this->NPCCurrentState->spawn();
}

NPC* NPCGenerator::getNPC(void)
{
	//handles transfer of NPC object ownership
	if (this->itemDispensed)
	{
		this->itemDispensed = false;
		this->itemRetrieved = true;
		return this->SpawnedNPC;
	}

	return nullptr;
}

void NPCGenerator::setStateParam(stateParameter SP, int value)
{
	this->stateParameters[SP] = value;
}

int NPCGenerator::getStateParam(stateParameter SP)
{
	return this->stateParameters[SP];
}

#pragma endregion