#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>


//1. There's should be some context 
//2. State machine should contain graphs and another state machine ?
//3. State machine shoudl contains some states
//4. State is node with some varibles inside and be able make transition into another state
// There should be event's, Input Event and Exit events at least.
// It has to be able manage animations, maibe physics and so on
// Do we need to make calculation about speed velocity and soo on insede state machine or we should be able just to push events ad varibles
// into state machine and say ok i just requesing state and. I guess no, i need just to push every neseccry varibles inside state machine anf just get response when state is ocurs or
// we have transiton and which state is current 
// Each state has to know about previes state and make good transition or we just need to keep it inside state machine only

template<class T> // Don't know at this moment what is T
class SHADE_API StateMachine // public: Asset<StateMachine> i guess 
{
public:

	StateMachine() = default;
	virtual ~StateMachine() = default;

};

