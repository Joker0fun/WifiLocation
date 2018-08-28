#ifndef _IPS_LOCATE_H_
#define _IPS_LOCATE_H_

#include "Loader.h"
#include "INIReader.h"

#define MAX_APLEN 256

struct road
{
	CVector2 start_point;
	CVector2 end_point;
};

class IpsLocate 
{
public:
	IpsLocate();
	~IpsLocate();
	void Init();
	string Locate(string strUser, vector<APRssi>& aprssi, char isMove, char isSys, time_t timestamp, char K);
	void GetIdcoordInfo(vector<IDCoord>& _idcoord) { m_gLoader.GetIdcoordInfo(_idcoord); }
	void GetFloorRoadInfo(unordered_map<char, vector<CVector2>>& _floors_roads) { m_gLoader.GetRoadInfo(_floors_roads); }

	void SetScanning(string user, unsigned char count, APRssiTime* rssies);
	LocateRslt InitLocation(char isSys, char isMove, char K);
	CVector3 Constraint(time_t n_Time, time_t m_Time, CVector3 outFree, char isSys);
	CVector2 RoadMatching(double x, double z, char floor, unordered_map<char, vector<CVector2>> floor_roads);
	IDCoord PointMatching(unsigned short id, CVector3 point3, char floor);

private:
	bool TimeOutClear(vector<APRssiTime>& arraybeacon, time_t nowtime, time_t ClearTime);
	pair<double, float> KalmanFilter(const double ResrcData, double x_last, float p_last, float PN_Q, float MN_R);
	bool WcaSimple(char K, CVector3 *v, double *w, CVector3 &wca, double &erdis);
	static bool VecSort(pair<unsigned short, double> a, pair<unsigned short, double> b) { return (a.second < b.second); } 

private:
	SectionMap m_configs;
	INIReader mReader;
	bool m_gLoaderSwtich;
	Loader m_gLoader;
	bool m_gFlowmeterSwtich;
	unordered_map<string, vector<APRssiTime>> cacheAP;
	unsigned int m_nCount;
	CVector3 m_UpLoc;
	time_t m_UpTime;
	unordered_map<char, vector<CVector2>> floors_roads;
};

#endif
