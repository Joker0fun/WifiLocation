#include <android/log.h>
#include "IpsLocate.h"
#include "Config.h"

#define ADD_CLEAR_TIME 16000
#define CACHE_COUNT 3
#define DELAY_CLEAR_TIME 16000
#define SENT_INTERVAL 800
#define INITIAL_PREDICTION 2.0f
#define PROCESSNIOSE_Q 0.0025f
#define MEASURENOISE_R 0.25f
#define SAFE_DELETE(x) if(NULL!=x){delete x; x=NULL;}
#define SCALE 1
#define LIKE_HUMAN 4
#define I_S 1.6
#define A_S 1.6
#define ROAD_DIS 6
#define LOCATION_NUM 8

#define TAG "IPS_LOCATION"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

IpsLocate::IpsLocate() : m_gLoaderSwtich(nullptr), m_gFlowmeterSwtich(true)
{
	Init();
	m_nCount = 0;
	m_UpTime = 0;
}

IpsLocate::~IpsLocate() {}

void IpsLocate::Init() {
	Config c;
	INIReader reader("/storage/emulated/0/maptest/CHIGOO/ips.ini");
	if (reader.ParseError() < 0) {
		LOGE("Can't read config file");
		return;
	}

	//c.Parse("C://Chigoo//Code//C16INI//C11STRIP//ips.ini", m_configs);
	c.Parse("/storage/emulated/0/maptest/CHIGOO/ips.ini", m_configs);
	string loader_swtich;
	char type = 0;
	m_gLoaderSwtich = true;
	if (m_gLoaderSwtich) {
		m_gLoader.InitDatabase(reader);
		m_gLoader.LoadDatabase(0);
		m_gLoader.GetRoadInfo(floors_roads);
	} else {
	}
}


void bubble(APRssiTime v[], int size)
{
	int i;
	APRssiTime temp;
	for (int p = 1; p < size; p++)
	{
		for (i = 0; i < size - p; i++)
			if (v[i].rssi < v[i + 1].rssi)
			{
				temp = v[i];
				v[i] = v[i + 1];
				v[i + 1] = temp;
			}
	}
}

void InsertionSort(APRssiTime *num, int n)
{
	int i = 0;
	int j = 0;
	APRssiTime tmp;
	for (i = 1; i < n; i++)
	{
		tmp = num[i];
		j = i - 1;
		while (j >= 0 && tmp.rssi > num[j].rssi)
		{
			num[j + 1] = num[j];
			j--;
		}
		num[j + 1] = tmp;
	}
}

void InsertionSort(pair<unsigned short, double> *num, int n)
{
	int i = 0;
	int j = 0;
	pair<unsigned short, double> tmp;
	for (i = 1; i < n; i++)
	{
		tmp = num[i];
		j = i - 1;
		while (j >= 0 && tmp.second > num[j].second)
		{
			num[j + 1] = num[j];
			j--;
		}
		num[j + 1] = tmp;
	}
}


pair<double, float> IpsLocate::KalmanFilter(const double ResrcData, double x_last, float p_last, float PN_Q, float MN_R)
{
	float R = MN_R;
	float Q = PN_Q;

	double x_mid = x_last;
	double x_now;

	float p_mid;
	float p_now;
	float kg;

	x_mid = x_last;
	p_mid = p_last + Q;
	kg = p_mid / (p_mid + R);
	x_now = x_mid + kg*(ResrcData - x_mid);

	p_now = (1 - kg)*p_mid;

	p_last = p_now;
	x_last = x_now;

	pair<double, float> result;
	result = make_pair(x_last, p_last);
	return result;
}

bool IpsLocate::TimeOutClear(vector<APRssiTime>& arrayrssi, time_t timestamp, time_t ClearTime)
{
	if (arrayrssi.size() < 1) return true;
	time_t intervaltime = timestamp - arrayrssi[arrayrssi.size() - 1].time;
	if (intervaltime > ClearTime)
	{
		arrayrssi.clear();
		return true;
	}
	return false;
}

void IpsLocate::SetScanning(string user, unsigned char count, APRssiTime *rssies)
{
	if (count == 0) return;
	if (count > 16) count = 16;
	time_t nowtime = 0;

	for (unsigned char i = 0; i < count; i++)
	{
		unordered_map<string, vector<APRssiTime>>::iterator it = cacheAP.find(rssies[i].mac);
		nowtime = rssies[i].time;

		if (it == cacheAP.end())
		{
			vector<APRssiTime> pVecAPRT;
			rssies[i].p = INITIAL_PREDICTION;
			pVecAPRT.push_back(rssies[i]);
			cacheAP[rssies[i].mac] = pVecAPRT;
		}
		else
		{
			if (!TimeOutClear(it->second, nowtime, ADD_CLEAR_TIME))
			{
				//kalman filter
				//pair<double, float> kf_status;
				//APRssiTime last = cacheAP[rssies[i].mac].back();
				//kf_status = KalmanFilter(rssies[i].rssi, last.rssi, last.p, PROCESSNIOSE_Q, MEASURENOISE_R);
				//rssies[i].rssi = kf_status.first;
				//rssies[i].p = kf_status.second;
			}
			else
			{
				rssies[i].p = INITIAL_PREDICTION;
			}
			it->second.push_back(rssies[i]);
		}
	}

	unordered_map<string, vector<APRssiTime>>::iterator it;

	for (it = cacheAP.begin(); it != cacheAP.end(); it++)
	{
		while (it->second.size() > CACHE_COUNT)
			it->second.erase(it->second.begin());
		if (TimeOutClear(it->second, nowtime, DELAY_CLEAR_TIME))
		{
			cacheAP.erase(it);
			it = cacheAP.begin();
		}
	}
};

bool IpsLocate::WcaSimple(char K, CVector3 *v, double *wgt, CVector3 &wca, double &erdis)
{
	double delta = 0.0000001;
	CVector3 zeta = CVector3(0.0, 0.0, 0.0);

	for (char i = 0; i < K; i++)
	{
		if (wgt[i] == 0) wgt[i] = 0.0000001;
		delta += 1.0f / wgt[i];
		zeta += v[i] / wgt[i];
	}
	wca = zeta / delta;

	erdis = (wca - v[0]).length() / 2;
	if (erdis < 0)
	{
		erdis = 0;
	}
	else if (erdis > 10) {
		erdis = 10;
	}
	return true;
}


LocateRslt IpsLocate::InitLocation(char isSys, char isMove, char K)
{
	LocateRslt lr;
	pair<float, float> kf_status;

	float dis = 0.0f;
	int count = 0;
	float w_count = 1.0f;

	vector<pair<unsigned short, double>> iddelta;
	double delta = 0;
	double sigma = 0;
	int findcount = 0;
	unordered_map<unsigned short, vector<APRssi>> IdMacRssi = m_gLoader.GetApInfo();
	IDCoord idcoord;
	for (auto it = IdMacRssi.begin(); it != IdMacRssi.end(); it++)
	{
		int vecnum = (int)it->second.size();
		for (int i = 0; i < vecnum; i++)
		{
			auto itmac = cacheAP.find(it->second[i].mac);
			if (cacheAP.end() != itmac)
			{
				int m = (int)itmac->second.size();
				delta = fabs((double)it->second[i].rssi - itmac->second[m - 1].rssi);
				if (delta < 3000)
				{
					sigma += delta;
					findcount++;
				}

			}
			delta = 0;
		}
		if (findcount > 5 && sigma < 3)
		{
			m_gLoader.GetIdInfo(it->first, idcoord);
			lr.xyz = idcoord.xyz;
			lr.erdis = 0.0;
			lr.floor = idcoord.floor;
			return lr;
		}
		if (findcount != 0)
		{
			iddelta.push_back(make_pair(it->first, sqrt(sigma) * 1 / (findcount*findcount)));
		}
		sigma = 0;
		findcount = 0;
	}

	cacheAP.erase(cacheAP.begin(), cacheAP.end());

	sort(iddelta.begin(), iddelta.end(), VecSort);


	if (iddelta.size() == 0)
	{
		lr.xyz = CVector3(0.0, 0.0, 0.0);
		lr.erdis = 0.0;
		lr.floor = 0;
		return lr;
	}
	else if (iddelta.size() < K)
	{
		K = (char)iddelta.size();
	}

	//K = 1;

	/*if (isMove == 0)
	{
		K = 1;
	}*/

	vector<pair<unsigned short, double>> findID;
	findID.assign(iddelta.begin(), iddelta.begin() + K);
	
	unsigned short *id = new unsigned short[K];
	CVector3 *v = new CVector3[K];
	double *wgt = new double[K];
	char *floor = new char[K];

	int k = 0;
	for (auto itid = findID.begin(); itid != findID.end(); itid++)
	{
		if (m_gLoader.GetIdInfo(itid->first, idcoord))
		{
			id[k] = idcoord.id;
			v[k] = { idcoord.xyz.x,idcoord.xyz.y,idcoord.xyz.z };
			wgt[k] = itid->second*itid->second;
			floor[k] = idcoord.floor;
		}
		k++;
	}

	CVector3 out = CVector3(0.0, 0.0, 0.0);
	double erdis = 1.0;
	WcaSimple(K, v, wgt, out, erdis);

	lr.id = findID.front().first;
	lr.xyz = out;
	lr.erdis = erdis;
	lr.floor = floor[0];

	SAFE_DELETE(id);
	SAFE_DELETE(v);
	SAFE_DELETE(wgt);
	SAFE_DELETE(floor);
	return lr;
}

CVector3 IpsLocate::Constraint(time_t n_Time, time_t m_Time, CVector3 outFree, char isSys)
{
	CVector3 out;
	if (n_Time - m_Time < 16000 && m_nCount != 0)
	{
		if ((outFree - m_UpLoc).length() > LIKE_HUMAN * SCALE)
		{
			outFree = (outFree - m_UpLoc)*(LIKE_HUMAN * SCALE / (outFree - m_UpLoc).length()) + m_UpLoc;
		}
		else if (isSys == 1)
		{
			outFree = (outFree - m_UpLoc) / I_S + m_UpLoc;
		}
		else
		{
			outFree = (outFree - m_UpLoc) / A_S + m_UpLoc;
		}
	}

	out = outFree;
	m_UpLoc = outFree;
	m_UpTime = n_Time;
	m_nCount++;
	return out;
};

double calculate_distance(CVector2 point, CVector2 road_start_point, CVector2 road_end_point)
{
	if ((road_start_point.x - point.x)*(road_end_point.x - point.x) < 0 || (road_start_point.y - point.y)*(road_end_point.y - point.y) < 0)
	{
		double slope, D;
		if (road_start_point.x == road_end_point.x)
		{
			D = std::abs(point.x - road_start_point.x);
			return D;
		}
		else
			slope = (road_start_point.y - road_end_point.y) / (road_start_point.x - road_end_point.x);

		double dx = (slope*(point.y - road_start_point.y + slope*road_start_point.x) + point.x) / (slope*slope + 1);
		double dy = road_start_point.y + slope*(dx - road_start_point.x);
		D = sqrt(pow(dx - point.x, 2) + pow(dy - point.y, 2));
		return D;
	}
	else
	{
		double D1, D2;
		D1 = sqrt(pow(road_end_point.x - point.x, 2) + pow(road_end_point.y - point.y, 2));
		D2 = sqrt(pow(road_start_point.x - point.x, 2) + pow(road_start_point.y - point.y, 2));
		return min(D1, D2);
	}
}

CVector2 calculate_foot_point(CVector2 point, CVector2 road_start_point, CVector2 road_end_point)
{
	if ((road_start_point.x - point.x)*(road_end_point.x - point.x) < 0 || (road_start_point.y - point.y)*(road_end_point.y - point.y) < 0)
	{
		double A, B;
		if (road_start_point.x == road_end_point.x)
		{
			return CVector2(road_start_point.x, point.y);
		}
		else
			A = (road_start_point.y - road_end_point.y) / (road_start_point.x - road_end_point.x);
		B = road_start_point.y - A*road_start_point.x;
		double m = point.x + A*point.y;
		double x = (m - A*B) / (A*A + 1);
		double y = A*x + B;
		return CVector2(x, y);
	}
	else
	{
		double D1 = sqrt(pow(point.x - road_start_point.x, 2) + pow(point.y - road_start_point.y, 2));
		double D2 = sqrt(pow(point.x - road_end_point.x, 2) + pow(point.y - road_end_point.y, 2));
		if (D1 < D2)
			return road_start_point;
		else
			return road_end_point;
	}
}

CVector2 IpsLocate::RoadMatching(double x, double z, char floor, unordered_map<char, vector<CVector2>> floors_id_roads)
{
	unordered_map<int, road> id_roads;

	unordered_map<char, vector<CVector2>>::iterator it = floors_id_roads.find(floor);
	if (it == floors_id_roads.end())
	{
		return CVector2(x, z);
	}
	else
	{
		vector<CVector2> vecRoads;
		vecRoads = it->second;
		int roads_size = (int)(vecRoads.size()*0.5);

		for (int i = 0; i < roads_size; i++)
		{
			road temp_road;
			temp_road.start_point = vecRoads[2 * i];
			temp_road.end_point = vecRoads[2 * i + 1];
			id_roads.insert(pair<int, road>(i, temp_road));
		}
	}


	CVector2 inFoot;
	inFoot.x = x; inFoot.y = z;
	double min_dis = 1024;
	int min_ID;

	int id_size = (int)id_roads.size();

	for (int i = 0; i < id_size; i++)
	{
		CVector2 ax = id_roads[i].start_point;
		CVector2 ay = id_roads[i].end_point;
		double temp_dis = calculate_distance(inFoot, ax, ay);
		if (temp_dis < min_dis)
		{
			min_ID = i;
			min_dis = temp_dis;
		}
	}

	if (min_dis > ROAD_DIS)
	{
		return inFoot;
	}
	else
	{
		CVector2 outFoot;
		outFoot = calculate_foot_point(inFoot, id_roads[min_ID].start_point, id_roads[min_ID].end_point);

		return outFoot;
	}
}


IDCoord IpsLocate::PointMatching(unsigned short id, CVector3 point3, char floor)
{
	IDCoord idxyz;
	idxyz.id = id;
	idxyz.xyz = point3;
	idxyz.floor = floor;

	vector<IDCoord> idcoord;
	m_gLoader.GetIdcoordInfo(idcoord);

	vector<IDCoord> floorcoord;
	vector<pair<unsigned short, CVector3>> pair_idpoint;

	IDCoord tmp;
	tmp.id = 0;
	tmp.xyz = { 0.0,0.0,0.0 };
	tmp.floor = floor;

	auto icit = find(idcoord.begin(), idcoord.end(), tmp);

	if (icit == idcoord.end())
	{
		return idxyz;
	}
	else
	{
		pair_idpoint.push_back(make_pair(icit->id, icit->xyz));
	}

	int FPnum = (int)pair_idpoint.size();
	
	pair<unsigned short, double> *pair_iddis = new pair<unsigned short, double>[FPnum];
	for (int i = 0; i < FPnum; i++)
	{
		pair_iddis[i].first = pair_idpoint[i].first;
		pair_iddis[i].second = (point3 - pair_idpoint[i].second).length();
	}
	InsertionSort(pair_iddis, FPnum);

	pair_iddis[FPnum - 1].first;


	m_gLoader.GetIdInfo(pair_iddis[FPnum - 1].first, idxyz);

	SAFE_DELETE(pair_iddis);
	return idxyz;
}



string IpsLocate::Locate(string strUser, vector<APRssi>& aprssi, char isMove, char isSys, time_t timestamp, char K)
{
	APRssiTime inrssi[MAX_APLEN];
	int apcount = 0;

	for (auto it = aprssi.begin(); it != aprssi.end(); it++)
	{
		inrssi[apcount].mac = it->mac;
		inrssi[apcount].rssi = (double)it->rssi;
		inrssi[apcount].time = timestamp;
		apcount++;
	}

	InsertionSort(inrssi, apcount);

	APRssiTime rssies[LOCATION_NUM];
	if (apcount > LOCATION_NUM)
	{
		apcount = LOCATION_NUM;
		for (int i = 0; i < LOCATION_NUM; i++)
		{
			rssies[i] = inrssi[i];
		}
	}
	else
	{
		for (int i = 0; i < apcount; i++)
		{
			rssies[i] = inrssi[i];
		}
	}

	LocateRslt lr;
	Json::Value root;

	SetScanning(strUser, apcount, rssies);
	lr = InitLocation(isSys, isMove, K);
	root["id"] = lr.id;
	lr.xyz = Constraint(timestamp, m_UpTime, lr.xyz, isSys);
	//CVector2 xz = RoadMatching(lr.xyz.x, lr.xyz.z, lr.floor, floors_roads);
	//lr.xyz.x = xz.x;
	//lr.xyz.z = xz.y;
	//IDCoord idxyz = PointMatching(lr.id, lr.xyz, lr.floor);
	//root["id"] = idxyz.id;

	lr.user = strUser;
	root["user"] = lr.user;
	root["x"] = lr.xyz.x;
	root["y"] = lr.xyz.y;
	root["z"] = lr.xyz.z;
	root["floor"] = lr.floor;
	root["erdis"] = lr.erdis;

	string out = root.toStyledString();

	return out;
}
