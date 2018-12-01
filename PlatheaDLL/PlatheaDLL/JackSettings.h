#pragma once



extern int IDroom;

struct TrackedPerson {
	int ID;
	bool justIdentified;
	char name[256];
	int nameID;
	int X, Y;
	char type[50];
	

	~TrackedPerson();
};
