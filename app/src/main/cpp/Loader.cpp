#include "Loader.h"
#include <android/log.h>

#define DATABASE_NUM 8
#define TAG "LOADER"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

void Loader::InitDatabase(const SectionMap &configs) {
	string host, user, passwd, db, path_loss, kfq, kfr;
	int port = 0;

	if (!configs.FindConfig("loader", "confPath", local_conf_path_))
		cout << "Cannot find confPath in loader section." << endl;

	if (!configs.FindConfig("project", "mapId", map_id_))
	if (!configs.FindConfig("project", "buildingId", building_id_))

	if (!configs.FindConfig("algorithm", "height", default_height_))
	if (!configs.FindConfig("algorithm", "txPower", default_tx_power_))
	if (!configs.FindConfig("algorithm", "pathLoss", path_loss))
	default_path_loss_ = atof(path_loss.c_str());

	if (!configs.FindConfig("algorithm", "KFQ", kfq))
	if (!configs.FindConfig("algorithm", "KFR", kfr))
	kfq_ = atof(kfq.c_str());
	kfr_ = atof(kfr.c_str());
}

void Loader::InitDatabase(INIReader &reader) {
    string path_loss, kfq, kfr;
    local_conf_path_ = reader.Get("loader", "confPath", "UNKOWN");
    map_id_ = (char)reader.GetInteger("project", "mapId", -1);
    building_id_ = (char)reader.GetInteger("project", "buildingId", -1);
    default_height_ = (char)reader.GetInteger("algorithm", "height", -1);
    default_tx_power_ = (char)reader.GetInteger("algorithm", "txPower", -1);
    path_loss = reader.Get("algorithm", "pathLoss", "0");
    default_path_loss_ = atof(path_loss.c_str());

    kfq = reader.Get("algorithm", "KFQ", "0");
    kfr = reader.Get("algorithm", "KFR", "0");
    kfq_ = atof(kfq.c_str());
    kfr_ = atof(kfr.c_str());
}

void Loader::LoadDatabase(char type) 
{
	switch (type) 
	{
	case 0:
	default:
		if (!LoadLocalAConf()) 
		{
		}

		LoadRoadConf();
		break;
	}
}

bool Loader::LoadLocalAConf() 
{
	string data = local_conf_path_;
	//data = local_conf_path_.substr(0, local_conf_path_.length() - 1);

    LOGD("local_conf_path_ is %s", local_conf_path_.c_str());
    LOGD("config file path is %s", data.c_str());
	FILE *fp = fopen(data.c_str(), "r");

	if (fp == NULL) 
	{
		return false;
	}

	char ch[4096];
	memset(ch, 0, sizeof(ch));
	size_t count = 0;
	string str;
	while ((count = fread(ch, sizeof(char), 4096, fp)) > 0) {
		str.append(ch, count);
	}
	fclose(fp);

	Json::Value value;
	Json::Reader reader;

	vector<APRssi> vec_mac_rssi;
	APRssi mac_rssi;
	IDCoord id_coord;

	if (reader.parse(str, value))
	{
		map_id_ = value["mapId"].asInt();
		building_id_ = value["buildintId"].asInt();
		const Json::Value id_coord_ = value["idcoord"];

		for (unsigned int i = 0; i < id_coord_.size(); i++)
		{
			id_coord.id = id_coord_[i]["id"].asUInt();
			id_coord.xyz.x = id_coord_[i]["x"].asDouble();
			id_coord.xyz.y = id_coord_[i]["y"].asDouble();
			id_coord.xyz.z = id_coord_[i]["z"].asDouble();
			id_coord.floor = id_coord_[i]["floor"].asInt();
			const Json::Value mac_rssi_ = id_coord_[i]["macrssi"];
			for (unsigned int k = 0; k < mac_rssi_.size(); k++)
			{
				mac_rssi.mac = mac_rssi_[k]["mac"].asString();
				mac_rssi.rssi = mac_rssi_[k]["rssi"].asInt();
				vec_mac_rssi.push_back(mac_rssi);
			}

			idmacrssi_.insert(make_pair(id_coord.id, vec_mac_rssi));
			idcoord_.insert(make_pair(id_coord.id, id_coord));

		}

	}

	unordered_map<unsigned short, vector<APRssi>>::iterator itsort;

	for (itsort = idmacrssi_.begin(); itsort != idmacrssi_.end(); itsort++)
	{
		vector<APRssi> unmacrssi = itsort->second;
		sort(unmacrssi.begin(), unmacrssi.end(), BSSort);

		vector<APRssi> sortMacRssi;
		if (unmacrssi.size() > DATABASE_NUM)
		{
			sortMacRssi.assign(unmacrssi.begin(), unmacrssi.begin() + DATABASE_NUM);
		}
		itsort->second.clear();
		itsort->second = sortMacRssi;
	}

	return true;
}

bool Loader::LoadRoadConf()
{
	FILE *fp = fopen("C://Chigoo//Code//AData//RoadConf//CHIGOO-ROAD", "r");
	if (fp == NULL) 
	{
		return false;
	}

	char ch[1024];
	memset(ch, 0, sizeof(ch));
	size_t count = 0;
	string str;
	while ((count = fread(ch, sizeof(char), 1024, fp)) > 0) 
	{
		str.append(ch, count);
	}
	fclose(fp);

	Json::Value value;
	Json::Reader reader;

	FloorRoad floor_path;
	CVector2 roadPoint;


	if (reader.parse(str, value))
	{
		road_map_id_ = value["mapId"].asInt();
		road_building_id_ = value["buildintId"].asInt();
		const Json::Value path_ = value["path"];

		for (unsigned int i = 0; i < path_.size(); i++)
		{
			floor_path.floor = path_[i]["floor"].asInt();

			const Json::Value road_ = path_[i]["road"];
			for (unsigned int k = 0; k < road_.size(); k++)
			{
				roadPoint.x = road_[k]["x"].asDouble();
				roadPoint.y = road_[k]["y"].asDouble();
				floor_path.floor_road.push_back(roadPoint);
			}
			floors_roads_.insert(make_pair(floor_path.floor, floor_path.floor_road));
		}
	}

	return true;
}
