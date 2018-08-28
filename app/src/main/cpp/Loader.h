#ifndef _IPS_LOADER_H_
#define _IPS_LOADER_H_

#include <algorithm>
#include "typesdef.h"
#include "json/json.h"
#include "INIReader.h"

using namespace Json;

class Loader 
{
public:
	static Loader* GetInstance() 
	{
		return loader_;
	}

	Loader() : default_height_(0), default_tx_power_(0), default_path_loss_(0) 
	{
	}

	virtual ~Loader() 
	{
		Drop();
	}

	void Drop() 
	{
	}

	void InitDatabase(const SectionMap &configs);

	void InitDatabase(INIReader &reader);

	void LoadDatabase(char type);


	int GetMapId() 
	{
		return map_id_;
	}

	char GetBuildingId() 
	{
		return building_id_;
	}

	
	bool GetIdInfo(const unsigned short& id, IDCoord& idcoord)
	{
		bool ret = false;
		auto iter = idcoord_.find(id);
		if (idcoord_.end() != iter)
		{
			idcoord = iter->second;
			idcoord.id = id;
			ret = true;
		}
		return ret;
	}

	unordered_map<unsigned short, vector<APRssi>> GetApInfo()
	{
		return idmacrssi_;
	}

	void GetIdcoordInfo(vector<IDCoord>& _idcoord) 
	{
		for (auto it : idcoord_) {
			_idcoord.push_back(it.second);
		}
	}

	void GetRoadInfo(unordered_map<char, vector<CVector2>>& _floors_roads) 
	{
		for (auto it : floors_roads_) {
			_floors_roads.insert(it);
		}
	}


private:
	bool LoadLocalAConf();
	bool LoadRoadConf();
	static bool BSSort(const APRssi &a, const APRssi &b) { return a.rssi > b.rssi; }

private:
	static Loader* loader_;
	char default_height_;
	char default_tx_power_;
	double default_path_loss_;

	double kfr_;
	double kfq_;
	string local_conf_path_;
	char map_id_;
	char building_id_;
	char road_map_id_;
	char road_building_id_;
	unordered_map<unsigned short, vector<APRssi>> idmacrssi_;
	unordered_map<unsigned short, IDCoord> idcoord_;
	unordered_map<char, vector<CVector2>> floors_roads_;
};

#endif
