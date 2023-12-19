#pragma once

#include "tribes/shapeBase.h"
#include "util/struct.h"

struct PlayerMove {
	int forwardAction;
	int backwardAction;
	int leftAction;
	int rightAction;
	bool jetting;
	bool crouching;
	bool jumpAction;
	bool trigger;
	int useItem;
	float turnRot;
	float pitch;
};

class Player : public ShapeBase {
public:
	enum {
		root,
		run,
		runback,
		sideleft1,
		sideleft2,
		jumpstand,
		jumprun,
		crouchroot1,
		crouchroot2,
		crouchroot3,
		crouchforward1,
		crouchforward2,
		crouchsideleft1,
		crouchsideleft2,
		fall,
		landing,
		landing2,
		tumbleloop,
		tumbleend,
		jet,
		pdaaccess,
		Throw,
		flyerroot,
		apcroot,
		apcpilot,
		crouchdie,
		diechest,
		diehead,
		diegrabback,
		dierightside,
		dieleftside,
		dielegleft,
		dielegright,
		dieblownback,
		diespin,
		dieforward,
		dieforwardkneel,
		dieback,
		signoverhere,
		signpoint,
		signretreat,
		signstop,
		signsalut,
		celebration1,
		celebration2,
		celebration3,
		taunt1,
		taunt2,
		posekneel,
		posestand,
		wave,
		NUM_ANIMS
	};

	enum {
		ANIM_PLAYER_FIRST = signoverhere,
		ANIM_PLAYER_LAST = wave,
		ANIM_MOVE_FIRST = run,
		ANIM_MOVE_LAST = sideleft2,
		ANIM_CROUCH_MOVE_FIRST = crouchforward1,
		ANIM_CROUCH_MOVE_LAST = crouchsideleft2,
		ANIM_IDLE = root,
		ANIM_JUMPRUN = jumprun,
		ANIM_CROUCH = crouchroot1,
		ANIM_CROUCH_IDLE = crouchroot2,
		ANIM_FALL = fall,
		ANIM_LAND = landing,
		ANIM_JET = jet,
		ANIM_PDA = pdaaccess,
		ANIM_FLIER = flyerroot,
		ANIM_APC_RIDE = apcroot,
		ANIM_STAND = crouchroot3,
		ANIM_TUMBLE_LOOP = tumbleloop,
		ANIM_DIE_GRAB_BACK = diegrabback,
	};

	FIELD(0x0FD0, int, jumpSurfaceLastContact);
	FIELD(0x0FE4, int, lastContactCount);
	FIELD(0x0FE8, float, viewPitch);
	FIELD(0x0FF0, float, traction);
	FIELD(0x103C, bool, hasFocus);
	FIELD(0x103D, bool, dead);
	FIELD(0x103F, bool, aiControlled);
	FIELD(0x1008, bool, jetting);
	FIELD(0x1010, bool, falling);
	FIELD(0x1012, bool, crouching);
	FIELD(0x1014, PlayerMove, lastPlayerMove);
	FIELD(0x1044, float, interpDoneTime);
	FIELD(0x104C, GameBase*, mount);
	FIELD(0x1050, int, mountPoint);
	FIELD(0x1054, float, forwardAxisMovement);
	FIELD(0x1058, float, sideAxisMovement);

	void setAnimation(uint32_t anim)
	{
		using func_t = to_static_function_t<decltype(&Player::setAnimation)>;
		((func_t)0x4AD1D0)(this, anim);
	}
};
