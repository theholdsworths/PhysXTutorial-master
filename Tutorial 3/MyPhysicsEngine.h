#pragma once

#include "BasicActors.h"
#include "CompoundActors.h"
#include <iostream>
#include <iomanip>


namespace PhysicsEngine
{
	using namespace std;

	static int score = 0;
	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] =
	{	PxVec3(0.f / 1.f,	160.f / 255.f, 5.f / 255.f),	// green
		PxVec3(217 / 255.f, 0.f / 255.f, 0.f / 255.f),	// red
		PxVec3(140.f / 255.f, 94.f / 255.f, 88.f / 255.f),	// brown
		PxVec3(255.f / 255.f, 200.f / 255.f, 87.f / 255.f),	// orange

		PxVec3(0.f / 255.f, 50.f / 255.f, 0.f / 255.f),	// greendarker

		PxVec3(72.f / 255.f, 99.f / 255.f, 156.f / 255.f),	// blue
		PxVec3(100.f / 255.f, 100.f / 255.f, 100.f / 255.f), //Dark grey
		PxVec3(0.f / 255.f, 0.f / 255.f, 0.f / 255.f) //black
	};

	struct FilterGroup
	{
		enum Enum
		{
			ACTOR0 = (1 << 0),
			Section1 = (1 << 1),
			Section2 = (1 << 2),
			Section3 = (1 << 3),
			trypost = (1 << 4),
			ACTOR5 = (1 << 5)
		};
	};

	///An example class showing the use of springs (distance joints).
	class Trampoline
	{
	public:
		vector<DistanceJoint*> springs;
		Box *bottom, *top;

		Trampoline(const PxVec3& dimensions = PxVec3(1.f, 1.f, 1.f), PxReal stiffness = 1.f, PxReal damping = 1.f, PxTransform botPos = PxTransform(PxIdentity), PxTransform topPos = PxTransform(PxIdentity))
		{
			PxReal thickness = 0.08f;
			bottom = new Box(PxTransform(PxVec3(botPos.p.x, topPos.p.y + thickness, botPos.p.z), PxQuat(botPos.q)), PxVec3(dimensions.x, thickness, dimensions.z));
			top = new Box(PxTransform(PxVec3(topPos.p.x, topPos.p.y + dimensions.y + thickness, topPos.p.z), PxQuat(topPos.q)), PxVec3(dimensions.x, thickness, dimensions.z));
			springs.resize(4);
			springs[0] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, dimensions.z)));
			springs[1] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, -dimensions.z)));
			springs[2] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, dimensions.z)));
			springs[3] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, -dimensions.z)));

			for (unsigned int i = 0; i < springs.size(); i++)
			{
				springs[i]->Stiffness(stiffness);
				springs[i]->Damping(damping);
			}
			bottom->SetKinematic(true);
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(bottom);
			scene->Add(top);
		}

		~Trampoline()
		{
			for (unsigned int i = 0; i < springs.size(); i++)
				delete springs[i];
		}
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;

		Variables* variables;

		MySimulationEventCallback(Variables* _variables) : trigger(false), variables(_variables) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;

						cerr << pairs[i].triggerActor->getName() << endl;

						//if (std::strcmp(pairs[i].triggerActor->getName(), "plane") == 0)
							//variables->reset = true;

						trigger = true;
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						trigger = false;
					}
				}
			}
		}

		bool contact = true;

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs)
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;
			if (contact) {
				//this checks if the filterfroup has been activated
				for (PxU32 i = 0; i < nbPairs; i++)
				{
					switch (pairs[i].shapes[0]->getSimulationFilterData().word0)
					{
					case FilterGroup::ACTOR0:
						cerr << "Out of bounds!" << endl;
						score -= 5;
						break;
					case FilterGroup::Section1:
						cerr << "Section 1 hit!\n+5 points" << endl;
						score += 5;
						break;
					case FilterGroup::Section2:
						cerr << "Section 2 hit!\n+10 points" << endl;
						score += 10;
						break;
					case FilterGroup::Section3:
						score += 15;
						cerr << "Section 3 hit!\n+15 points" << endl;
						break;
					case FilterGroup::trypost:
						score += 50;
						cerr << "Try post hit!!!\n+50 points well done!!" << endl;
 						break;
					case FilterGroup::ACTOR5:
						cerr << "Out of bounds!" << endl;
						score -= 5;
						break;
					}
				}
				contact = false;
			}
			
		}
		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		//enable continous collision detection
		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		pairFlags = PxPairFlag::eSOLVE_CONTACT;
		pairFlags |= PxPairFlag::eCCD_LINEAR;
		pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;


		//customise collision filtering here
		//e.g.

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{	
		bool pullSpring = false;
		float springStr = 0.0f;
		int spinnerAngle = 1.5708f;

		PxMaterial *Grass = CreateMaterial(.6f, .6f, 0.1f);
		PxMaterial *CourseGreen = CreateMaterial(.0f, .8f, 0.8f);
		PxMaterial *bouncy = CreateMaterial(0.0f, 0.0f, 1.f);
		PxMaterial *bouncyx4 = CreateMaterial(0.0f, 0.0f, 100.f);
		PxMaterial *rugbyBounce = CreateMaterial(0.0f, 0.0f, 1.f);

		MySimulationEventCallback* my_callback;
		Variables* myVariables;

		Plane* plane;
		Sphere* bouncyBall;
		
		// S 1 or 2 - Stands for what section it is at, this helps me know which part of the field they're at.
		// L or R - This stands for which side the battlements are at.
		CastleWall  *S2LWall, *S2RWall;
		RearCastleWall *S1LWall, *S1RWall;
		castle *castle1, *castle2, *castle3, *castle4, *castle5, *castle6;

		Walls *walls;
		Trampoline *canon;
		DistanceJoint *spring;

		Box *box1, *box2, *box3, *box4, *box5, *box6, *box7, *box8, *box9, *base, *section1, *section2, *section3, *box70, *outofbounds;
		tryPost *post1, *post2, *post3;

		//Field flag
		Cloth *cornerFlag;
		PxCloth *Flag;
		PxReal windX, windZ;
		Box *pole;

		//propeller trap
		PxRigidDynamic *Rblades;
		PxRevoluteJoint *jBlades;
		Blades *blades;

	public:

		Capsule* egg;

		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene(CustomFilterShader) {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit()
		{
			SetVisualisation();

			float field_Angle = PxPi / 6;

			myVariables = new Variables();

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback(myVariables);
			px_scene->setSimulationEventCallback(my_callback);
			px_scene->setFlag(PxSceneFlag::eENABLE_CCD, true);

			egg = new Capsule(PxTransform(PxVec3(-125.0f, 2.6f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(0.3f, 0.5f));
			egg->Material(rugbyBounce);
			egg->Get()->setName("egg");
			Add(egg);
			egg->Get()->isRigidBody()->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			egg->SetupFiltering(FilterGroup::ACTOR0, FilterGroup::Section1 | FilterGroup::Section2 | FilterGroup::Section3 | FilterGroup::trypost | FilterGroup::ACTOR5);
	
			// actor 1 plane
			plane = new Plane();
			plane->Color(color_palette[0]);
			plane->Get()->setName("plane");
			plane->SetTrigger(true);
			Add(plane);
			plane->SetupFiltering(FilterGroup::ACTOR5, FilterGroup::ACTOR0);

			// actor 2 base
			base = new Box(PxTransform(PxVec3(80.0f, 0.0f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(20.0f, 0.5f, 10.0f));
			base->Material(Grass);
			base->Color(color_palette[0]);
			base->SetKinematic(true);
			Add(base);
			base->SetupFiltering(FilterGroup::Section3, FilterGroup::ACTOR0);

			//Propeller trap -- Middle of field
			blades = new Blades(PxTransform(PxVec3(40.5f, 9.65f, 0.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(40.5f, 9.65f, 0.8f), PxQuat(1.5708f, PxVec3(.0f, .0f, 1.0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			blades = new Blades(PxTransform(PxVec3(40.5f, 5.65f, 0.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(40.5f, 5.65f, 0.8f), PxQuat(1.5708f, PxVec3(.0f, .0f, 1.0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			canon = new Trampoline(PxVec3(.5f, 0.5f, 0.5f), 100.0f, 10.0f, PxTransform(PxVec3(-129.0f, 0.3f, 0.0f), PxQuat(field_Angle * 10, PxVec3(0.0f, 0.0f, 1.0f))), PxTransform(PxVec3(-129.0f, 0.3f, 0.0f), PxQuat(field_Angle * 10, PxVec3(0.0f, 0.0f, 1.0f))));
			canon->AddToScene(this);

			walls = new Walls(PxTransform(PxVec3(PxIdentity), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(100.0f, 2.0f, 0.5f));
			walls->GetShape(0)->setLocalPose(PxTransform(PxVec3(0.0f, 1.0f, 10.5f), PxQuat(PxIdentity))); 
			walls->GetShape(1)->setLocalPose(PxTransform(PxVec3(0.0f, 1.0f, -10.5f), PxQuat(PxIdentity)));
			walls->Color(color_palette[2]);
			walls->SetKinematic(true);
			Add(walls);

			//blades
			blades = new Blades(PxTransform(PxVec3(40.5f, 1.65f, 0.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(40.5f, 1.65f, 0.8f), PxQuat(1.5708f, PxVec3(.0f, .0f, 1.0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			//Propeller trap -- Left and side
			blades = new Blades(PxTransform(PxVec3(40.5f, 9.65f, -4.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(40.5f, 9.65f, -4.8f), PxQuat(1.5708f, PxVec3(.0f, .0f, 1.0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(-5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			blades = new Blades(PxTransform(PxVec3(40.5f, 5.65f, -4.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(40.5f, 5.65f, -4.8f), PxQuat(1.5708f, PxVec3(.0f, .0f, 1.0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(-5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			blades = new Blades(PxTransform(PxVec3(40.5f, 1.65f, -4.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(40.5f, 1.65f, -4.8f), PxQuat(1.5708f, PxVec3(.0f, .0f, 1.0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(-5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			box70 = new Box(PxTransform(PxVec3(0.f, 10.0f, 40.0f), PxQuat(PxPi, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(1.f, 5.f, 1.1f)); 
			box70->SetKinematic(false);
			box70->Color(color_palette[2]);
			Add(box70);

			// actor 3 ball
			bouncyBall = new Sphere(PxTransform(PxVec3(0.0f, 15.f, 20.0f), PxQuat(PxIdentity)), 0.6f);
			bouncyBall->Color(color_palette[1]);
			bouncyBall->Material(bouncy);
			Add(bouncyBall);
			bouncyBall->Get()->isRigidBody()->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

			bouncyBall = new Sphere(PxTransform(PxVec3(0.0f, 15.f, -20.0f), PxQuat(PxIdentity)), 0.6f);
			bouncyBall->Color(color_palette[1]);
			bouncyBall->Material(bouncy);
			Add(bouncyBall);
			bouncyBall->Get()->isRigidBody()->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

			#pragma region Tryposts
			//try post
			post1 = new tryPost(PxTransform(PxVec3(1.0f, 3.0f, 0.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			post1->Color(color_palette[6]);
			post1->SetKinematic(false);
			Add(post1);
			post1->SetupFiltering(FilterGroup::trypost, FilterGroup::ACTOR0);

			post2 = new tryPost(PxTransform(PxVec3(60.0f, 3.0f, 0.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			post2->Color(color_palette[6]);
			post2->SetKinematic(false);
			Add(post2);
			post2->SetupFiltering(FilterGroup::trypost, FilterGroup::ACTOR0);

			post3 = new tryPost(PxTransform(PxVec3(-81.0f, 3.0f, 0.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			post3->Color(color_palette[6]);
			post3->SetKinematic(false);
			Add(post3);
			post3->SetupFiltering(FilterGroup::trypost, FilterGroup::ACTOR0);
			#pragma endregion Tryposts

			#pragma region CastleBattlements_&_Walls
			//------- Battlements -------
			
			//Battlement on the right hand side middle of the field
			castle1 = new castle(PxTransform(PxVec3(0.0f, 6.0f, 20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			castle1->Color(color_palette[2]);
			castle1->Material(bouncy);
			castle1->SetKinematic(true);
			Add(castle1);

			//Battlement on the left hand side middle of the field
			castle2 = new castle(PxTransform(PxVec3(0.0f, 6.0f, -20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			castle2->Color(color_palette[2]);
			castle2->Material(bouncy);
			castle2->SetKinematic(true);
			Add(castle2);

			castle3 = new castle(PxTransform(PxVec3(60.0f, 5.0f, 20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			castle3->Color(color_palette[2]);
			castle3->SetKinematic(true);
			Add(castle3);

			castle4 = new castle(PxTransform(PxVec3(60.0f, 5.0f, -20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			castle4->Color(color_palette[2]);
			castle4->SetKinematic(true);
			Add(castle4);

			castle5 = new castle(PxTransform(PxVec3(-80.0f, 5.0f, -20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			castle5->Color(color_palette[2]);
			castle5->SetKinematic(true);
			Add(castle5);

			castle6 = new castle(PxTransform(PxVec3(-80.0f, 5.0f, 20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f))));
			castle6->Color(color_palette[2]);
			castle6->SetKinematic(true);
			Add(castle6);

			//---- Castle walls ----
			S2LWall = new CastleWall((PxTransform(PxVec3(30.0f, 3.0f, -20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f)))));
			S2LWall->Color(color_palette[2]);
			S2LWall->SetKinematic(true);
			Add(S2LWall);

			S2RWall = new CastleWall((PxTransform(PxVec3(30.0f, 3.0f, 20.0f), PxQuat(field_Angle * 45 , PxVec3(0.f, 1.f, .0f)))));
			S2RWall->Color(color_palette[2]);
			S2RWall->SetKinematic(true);
			Add(S2RWall);
			
			S1LWall = new RearCastleWall((PxTransform(PxVec3(-40.0f, 3.0f, -20.0f), PxQuat(PxPiDivTwo, PxVec3(0.f, 1.f, .0f)))));
			S1LWall->Color(color_palette[2]);
			S1LWall->SetKinematic(true);
			Add(S1LWall);

			S1RWall = new RearCastleWall((PxTransform(PxVec3(-40.0f, 3.0f, 20.0f), PxQuat(field_Angle * 45, PxVec3(0.f, 1.f, .0f)))));
			S1RWall->Color(color_palette[2]);
			S1RWall->SetKinematic(true);
			Add(S1RWall);
			#pragma endregion CastleBattlements_&_Walls

			#pragma region Canon 
			//canon wall
			box5 = new Box(PxTransform(PxVec3(-127.0f, 1.5f, 1.7f), PxQuat(field_Angle * 25, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(3.f, 1.0f, 0.5f)); //middle right
			box5->Color(color_palette[7]);
			box5->SetKinematic(true);
			Add(box5);
			
			//canon wall
			box6 = new Box(PxTransform(PxVec3(-127.0f, 1.5f, -1.7f), PxQuat(field_Angle * 25, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(3.f, 1.0f, 0.5f)); //middle right
			box6->Color(color_palette[7]);
			box6->SetKinematic(true);
			Add(box6);

			//canon wall
			box7 = new Box(PxTransform(PxVec3(-125.5f, -1.0f, 0.0f), PxQuat(field_Angle * 25, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(3.f, 2.5f, 15.1f)); //(y, z, x)
			box7->SetKinematic(true);
			box7->Color(color_palette[2]);
			Add(box7);
			#pragma endregion Canon
			
			#pragma region Score sections
			//Sections

			outofbounds = new Box(PxTransform(PxVec3(-90.0f, 0.65f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(10.0f, 0.5f, 10.0f));
			outofbounds->Color(color_palette[2]);
			outofbounds->Material(bouncyx4);
			outofbounds->SetKinematic(true);
			Add(outofbounds);
			outofbounds->SetupFiltering(FilterGroup::ACTOR5, FilterGroup::ACTOR0);

			//Start of section1
			box6 = new Box(PxTransform(PxVec3(-81.0f, 0.0f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(1.0f, 3.0f, 10.0f)); 
			box6->Color(color_palette[2]);
			box6->SetKinematic(true);
			Add(box6);
			
			//Section 1 (Score +5) 
			box9 = new Box(PxTransform(PxVec3(-40.0f, 0.65f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(40.0f, 0.5f, 10.0f));
			box9->SetKinematic(true);
			box9->Color(color_palette[4]);
			box9->Material(Grass);
			Add(box9);
			box9->SetupFiltering(FilterGroup::Section1, FilterGroup::ACTOR0);

			//End of section1 (Half way point) - Start of section2
			box7 = new Box(PxTransform(PxVec3(1.0f, 0.0f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(1.0f, 3.0f, 10.0f)); 
			box7->Color(color_palette[1]);
			box7->SetKinematic(true);
			Add(box7);

			//Section 2 (Score +10)
			section2 = new Box(PxTransform(PxVec3(30.0f, 0.45f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(30.0f, 0.1f, 10.0f));
			section2->Color(color_palette[0]);
			section2->Material(bouncy);
			section2->SetKinematic(true);
			Add(section2);
			section2->SetupFiltering(FilterGroup::Section2, FilterGroup::ACTOR0);

			//End of section2 - Start of section3
			box8 = new Box(PxTransform(PxVec3(60.0f, 0.0f, 0.0f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(1.0f, 3.0f, 10.0f)); 
			box8->SetKinematic(true);
			box8->Color(color_palette[0]);
			Add(box8);
			#pragma endregion Score sections
			
			pole = new Box(PxTransform(PxVec3(1.0f, 6.0f, -10.5f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(0.09f, 3.0f, 0.09f));
			pole->SetKinematic(true);
			pole->Color(color_palette[7]);
			pole->GetShape(0)->getActor()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			Add(pole);

			cornerFlag = new Cloth(PxTransform(PxVec3(1.0f, 7.5f, -10.5f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(1.5f, 1.7f), 80, 80);
			cornerFlag->Color(color_palette[1]);
			Add(cornerFlag);
			Flag = (PxCloth*)cornerFlag->Get();
			Flag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			Flag->setSelfCollisionDistance(0.01f);
			Flag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.9f));
			Flag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			Flag->setDragCoefficient(0.05f);

			/*/Commented out as more than one flag makes the frame rate drop - could be used for test cases?
			pole = new Box(PxTransform(PxVec3(1.0f, 6.0f, 10.5f), PxQuat(field_Angle * 180, PxVec3(0.0f, 0.0f, 1.0f))), PxVec3(0.09f, 3.0f, 0.09f));
			pole->SetKinematic(true);
			pole->Color(color_palette[6]);
			pole->GetShape(0)->getActor()->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
			Add(pole);

			cornerFlag = new Cloth(PxTransform(PxVec3(1.0f, 7.5f, 10.5f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(1.5f, 1.7f), 80, 80);
			cornerFlag->Color(color_palette[3]);
			Add(cornerFlag);
			Flag = (PxCloth*)cornerFlag->Get();
			Flag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			Flag->setSelfCollisionDistance(0.01f);
			Flag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.9f));
			Flag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			Flag->setDragCoefficient(0.05f);*/

		}

		//Custom udpate function
		virtual void CustomUpdate()
		{
			this->selected_actor = (PxRigidDynamic*)egg->Get();

			windX = static_cast<float>(rand() % 10);
			windZ = static_cast<float>(rand() % 20);
			Flag->setExternalAcceleration(PxVec3(windX, 1.0f, windZ));

			if (pullSpring)
			{
				springStr += 2.0f;
				if (springStr >= 200.0f) { springStr = 200.0f; }
			}

			if (myVariables->reset)
				this->Reset();
		}

		virtual int GetScore()
		{
			return score;
		}

		void KeyPressR()
		{
			cornerFlag = new Cloth(PxTransform(PxVec3(1.0f, 7.5f, 10.5f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(1.5f, 1.7f), 80, 80);
			cornerFlag->Color(color_palette[3]);
			Add(cornerFlag);
			Flag = (PxCloth*)cornerFlag->Get();
			Flag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			Flag->setSelfCollisionDistance(0.01f);
			Flag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.9f));
			Flag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			Flag->setDragCoefficient(0.05f);

			cornerFlag = new Cloth(PxTransform(PxVec3(1.0f, 7.5f, 10.5f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(1.5f, 1.7f), 80, 80);
			cornerFlag->Color(color_palette[3]);
			Add(cornerFlag);
			Flag = (PxCloth*)cornerFlag->Get();
			Flag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			Flag->setSelfCollisionDistance(0.01f);
			Flag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.9f));
			Flag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			Flag->setDragCoefficient(0.05f);

			cornerFlag = new Cloth(PxTransform(PxVec3(1.0f, 7.5f, 10.5f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(1.5f, 1.7f), 80, 80);
			cornerFlag->Color(color_palette[3]);
			Add(cornerFlag);
			Flag = (PxCloth*)cornerFlag->Get();
			Flag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			Flag->setSelfCollisionDistance(0.01f);
			Flag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.9f));
			Flag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			Flag->setDragCoefficient(0.05f);

			cornerFlag = new Cloth(PxTransform(PxVec3(1.0f, 7.5f, 10.5f), PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(1.5f, 1.7f), 80, 80);
			cornerFlag->Color(color_palette[3]);
			Add(cornerFlag);
			Flag = (PxCloth*)cornerFlag->Get();
			Flag->setExternalAcceleration(PxVec3(10.0f, 0.5f, 0.0f));
			Flag->setSelfCollisionDistance(0.01f);
			Flag->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.9f));
			Flag->setDampingCoefficient(PxVec3(.1f, .1f, .1f));
			Flag->setDragCoefficient(0.05f);
			Flag->setDragCoefficient(0.05f);
		}
		

		void KeyPressB()
		{
			pullSpring = true;
		}

		void KeyReleaseB()
		{
			pullSpring = false;
			this->SelectActor(5);
			this->GetSelectedActor()->addForce(PxVec3(7.5f, 1.0f, 0.0f) * springStr);
			springStr = 0.0f;

		}

		void KeyPressP()
		{
			blades = new Blades(PxTransform(PxVec3(-120.5f, 5.65f, 0.8f)));
			Rblades = (PxRigidDynamic*)blades->Get();
			Add(blades);
			jBlades = PxRevoluteJointCreate(*GetPhysics(), NULL, PxTransform(PxVec3(-120.5f, 5.65f, -0.8f), PxQuat(1.5708f, PxVec3(1.0f, .0f, .0f))), (PxRigidActor*)blades->Get(), PxTransform(PxVec3(0.f, .0f, .0f), PxQuat(1.5708 * 3, PxVec3(.0f, 1.0f, .0f))));
			jBlades->setDriveVelocity(5.5f);
			jBlades->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		}

		/*void KeyPressP()
		{
			springStr + 50.0f;
			this->SelectActor(5);
			this->GetSelectedActor()->addForce(PxVec3(8.0f, 1.0f, 0.0f) * springStr);
			
			cerr << "popppop" << endl;
		}*/
	};
}
