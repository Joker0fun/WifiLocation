#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_

#include <iostream>
#include <vector>
#include <unordered_map>
#include "CVector.h"

using namespace std;

typedef struct
{
	string mac;
	int rssi;
} APRssi;

typedef struct structAPRssiTime
{
	string mac;
	double rssi;
	time_t time;
	float p;

	structAPRssiTime()
	{
		mac = "m";
		rssi = 0.0;
		time = 0;
		p = 2.0f;
	}
} APRssiTime;


typedef struct structIDDIS
{
	unsigned short id;
	double dis;
} IDDIS;


typedef struct structIDCoord
{
	unsigned short id;
	CVector3 xyz;
	char floor;

	structIDCoord()
	{
		id = 0;
		xyz = { 0.0, 0.0, 0.0 };
		floor = 0;
	}
	bool operator==(const structIDCoord& c) const
	{
		return c.floor == floor;
	}
} IDCoord;


typedef struct structLocateRslt
{
	string user;
	unsigned short id;
	CVector3 xyz;
	double erdis;
	char floor;

	structLocateRslt()
	{
		id = 0;
		xyz = { 0.0,0.0,0.0 };
		erdis = 0.0;
		floor = 0;
	}
} LocateRslt;

typedef struct
{
	char floor;
	vector<CVector2> floor_road;
} FloorRoad;

typedef unordered_map<string, string> ConfigMap;

class SectionMap : public unordered_map<string, ConfigMap> 
{
public:
	bool FindConfig(const char *section, const char *key, string &value) const 
	{
		auto it = find(section);
		if (it == cend())
			return false;

		auto it2 = it->second.find(key);
		if (it2 == it->second.cend())
			return false;

		value = it2->second;
		return true;
	}

	bool FindConfig(const char *section, const char *key, char &value) const 
	{
		auto it = find(section);
		if (it == cend())
			return false;

		auto it2 = it->second.find(key);
		if (it2 == it->second.cend())
			return false;

		value = atoi(it2->second.c_str());
		return true;
	}
};

#endif
