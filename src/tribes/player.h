#pragma once

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

class Player;
