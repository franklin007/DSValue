/*
 * CAutoTest.cpp
 *
 *  Created on: 2014��7��5��
 *      Author: ranger.shi
 */

#include "CAutoTest.h"
#include "bitcoinnet/CChainManager.h"
namespace test {
template<typename T>
void ConvertTo(Value& value, bool fAllowNull = false) {
	if (fAllowNull && value.type() == null_type)
		return;
	if (value.type() == str_type) {
		// reinterpret string as unquoted json value
		Value value2;
		string strJSON = value.get_str();
		if (!read_string(strJSON, value2))
			throw runtime_error(string("Error parsing JSON:") + strJSON);
		ConvertTo<T>(value2, fAllowNull);
		value = value2;
	} else {
		value = value.get_value<T>();
	}
}

int CAutoTest::getBestBlockHigh() const {

	Array param;
	Value result = getinfo(param, false);
	if (result.type() == obj_type) {
		const json_spirit::Object& obj = result.get_obj();
		size_t i = 0;
		for (; i < obj.size(); ++i) {
			const json_spirit::Pair& pair = obj[i];
			const std::string& str_name = pair.name_;
			if (str_name == "blocks") {
				const json_spirit::Value& val_val = pair.value_;
				return val_val.get_int();
			}

		}
	}
	throw "getBestBlockHigh failed";
}


void CAutoTest::CheckAndSendLottoKey(int cycles, int gaptimems, string privateKey, string lottoPrivateKey) const {

	try {
		LogTrace("autotest", "thread start:%s\n", __FUNCTION__);
		while (cycles > 0) {
			int blockhigh = chainActive.Tip()->nHeight;
			static int lastsendedId = 0;
			int newid = (blockhigh / GetArg("-intervallotto", nIntervalLottery));
			if (lastsendedId != newid && SendCheckPoint(privateKey)) {
				lastsendedId = newid; //updata the id
				string tem = "";
				tem += tfm::format("%d", newid);
				string strMethod = "sendlottokey";
				Array strParams;
				strParams.push_back(tem);
				strParams.push_back(privateKey);
				strParams.push_back(lottoPrivateKey);
				ConvertTo<boost::int64_t>(strParams[0]);
				//sendlottokey(strParams, false);
				string id = sendlottokey(strParams, false).get_str();
				LogTrace("autotest", "cycles: %d gaptimems:%d,SendLottoKey ID:%d, ret: %s\n", cycles, gaptimems,
						newid, id);
				cycles--;
			}
			MilliSleep(gaptimems < 1000 ? 1000 : gaptimems);
		}
		LogTrace("autotest", "%s thread exit\n", __FUNCTION__);
	} catch (boost::thread_interrupted) {
		LogTrace("autotest", "%s thread interrupt\n", __FUNCTION__);
	} catch (Object& objError) {
		LogTrace("autotest", "%s error: %s", __FUNCTION__, find_value(objError, "message").get_str());
	} catch (...) {
		LogTrace("autotest", "%s sendlottokey failed \n", __FUNCTION__);
	}

}
bool CAutoTest::SendCheckPoint(string privateKey) const
{
	try {
		LogTrace("autotest", "thread start:%s\n", __FUNCTION__);
		static int lastsendedHeight = 0;
		CBlockIndex *tipBlockIndex = chainActive.Tip();
		////// check point height
		int64_t nInterVal = GetArg("-intervallotto", nIntervalLottery);
		int lowHeight = (chainActive.Height() / nInterVal) * nInterVal + 1;
		LogTrace("autotest", "thread start:%s lowHeight=%d tipHeight=%d\n", __FUNCTION__, lowHeight, tipBlockIndex->nHeight);
		if (lowHeight <= chainActive.Tip()->nHeight) {
			CBlockIndex *lowblockindex = chainActive[lowHeight];
			Bitcoin::CChainManager &chainManager = Bitcoin::CChainManager::CreateChainManagerInstance();

			CBlockIndex * topBblockindex = chainManager.GetBitcoinBlockIndex(
					tipBlockIndex->lottoHeader.mBitcoinHash.rbegin()->second);
			int topBitcoinHeight = topBblockindex->nHeight;

			CBlockIndex * lowBblockindex = chainManager.GetBitcoinBlockIndex(
					lowblockindex->lottoHeader.mBitcoinHash.begin()->second);
			int lowBitcoinHeight = lowBblockindex->nHeight;

			if (((topBitcoinHeight - lowBitcoinHeight) >= 4
					|| (chainActive.Tip()->nHeight - lowHeight) >= (nInterVal - 10))
					&& lastsendedHeight != lowHeight) {
				LogTrace("autotest", "enter\n");
				lastsendedHeight = lowHeight;
				Array strParamsCheck;
				strParamsCheck.push_back(tfm::format("%d", lowHeight).c_str());
				ConvertTo<boost::int64_t>(strParamsCheck[0]);
				strParamsCheck.push_back(privateKey);
				string ret = sendcheckpointchain(strParamsCheck, false).get_str();
				LogTrace("autotest", "sendchekpoint: %s\n", ret);
				return true;
			}
			LogTrace("autotest", "%s thread exit\n", __FUNCTION__);
		}

	} catch (boost::thread_interrupted) {
		LogTrace("autotest", "%s thread interrupt\n", __FUNCTION__);
	} catch (Object& objError) {
		LogTrace("autotest", "%s error: %s", __FUNCTION__, find_value(objError, "message").get_str());
	} catch (...) {
		LogTrace("autotest", "%s SendCheckPoint failed \n", __FUNCTION__);
	}
	return false;

}
void CAutoTest::SendCheckPoint(int cycles, int gaptimems, string privateKey) const {

	try {
		LogTrace("autotest", "thread start:%s\n", __FUNCTION__);
		while (--cycles > 0) {
			int tipHeight = chainActive.Tip()->nHeight-1;
			static int lastsendedHeight = 0;
			int checkpointHeight = (tipHeight / GetArg("-intervallotto", nIntervalLottery) -1) * nIntervalLottery + nLottoStep;
			if (lastsendedHeight != checkpointHeight) {
				lastsendedHeight = checkpointHeight;
				Array strParamsCheck;
				strParamsCheck.push_back(tfm::format("%d", checkpointHeight).c_str());
				ConvertTo<boost::int64_t>(strParamsCheck[0]);
				strParamsCheck.push_back(privateKey);
				string ret = sendcheckpointchain(strParamsCheck, false).get_str();
				LogTrace("autotest", "cycles: %d gaptimems:%d,ret: %s\n", cycles, gaptimems, ret);
			}
			MilliSleep(gaptimems < 1000 ? 1000 : gaptimems);
		}
		LogTrace("autotest", "%s thread exit\n", __FUNCTION__);
	} catch (boost::thread_interrupted) {
		LogTrace("autotest", "%s thread interrupt\n", __FUNCTION__);
	} catch (Object& objError) {
		LogTrace("autotest", "%s error: %s", __FUNCTION__, find_value(objError, "message").get_str());
	} catch (...) {
		LogTrace("autotest", "%s SendCheckPoint failed \n", __FUNCTION__);
	}

}

string CAutoTest::getAddress() const {
	Array param;
	Value result = getnewaddress(param, false);
	return result.get_str();
}

double CAutoTest::getBanlance() const {
	Array param;
	Value nbalance = getbalance(param, false);
	return nbalance.get_real();
}

void CAutoTest::SendRandBet(int cycles, int gaptimems,string reciveaddr) {

	double total = 0;
	try {
		LogTrace("autotest", "thread start:%s\n", __FUNCTION__);
		int height = chainActive.Height();
		while (--cycles > 0) {
			height = chainActive.Height();
			double reth = getBanlance();
			//cout<<"balnce"<<reth<<endl;
			if (reth > 50) {
				int rand = SelectSize(50000, 100);
				total += rand;
				Array strParams;
				strParams.push_back("");
				strParams.push_back(tfm::format("%d", rand));
				int type = SelectSize(1);
				string select = "";
				if (type == 0)
					select = getvchSelect(SelectSize(15, 6), 15);
				if (type == 1) {
					select = getvchSelect(SelectSize(20, 5), 20);
					select += tfm::format("%s", "ff");
					select += tfm::format("%s", getvchSelect(SelectSize(10, 2), 10));
				}
				string betT = "";
				betT += tfm::format("%d", type);
				strParams.push_back(betT);
				strParams.push_back(select);
				strParams.push_back(reciveaddr);
				ConvertTo<int64_t>(strParams[1]);
				ConvertTo<int64_t>(strParams[2]);
				string Id = sendlottobet(strParams, false).get_str();
				LogTrace("autotest", "cycles: %d gaptimems:%d,sended bet :%s hash: %s \n",cycles,gaptimems, select, Id);
			} else
				LogTrace("autotest", "not enough money  only: %f\n", reth);
			MilliSleep(gaptimems < 10 ? 10 : gaptimems);
		}

		LogTrace("autotest", "%s thread exit\n", __FUNCTION__);
	} catch (boost::thread_interrupted) {
		LogTrace("autotest", "%s thread interrupt\n", __FUNCTION__);
	}catch(Object& objError){
		LogTrace("autotest", "%s error: %s", __FUNCTION__ , find_value(objError, "message").get_str());
	}

	catch (...) {
		LogTrace("autotest", "%s unknown error \n", __FUNCTION__);
	}

	LogTrace("autotest", "bettransation spend:%lf\n",total);
}

void CAutoTest::SendRandTransation(int cycles, int gaptimems,string reciveaddr) {

	double total = 0;
	try {
		LogTrace("autotest", "thread start:%s\n", __FUNCTION__);
		while (--cycles > 0) {
			double reth = getBanlance();
			if (reth > 50) {
				Array param;
				int sent = SelectSize(50, 0);
				total +=sent;
				param.push_back(reciveaddr);
				param.push_back(tfm::format("%d", sent));
				ConvertTo<double>(param[1]);
				string id = sendtoaddress(param, false).get_str();
				LogTrace("autotest", "cycles: %d gaptimems:%d,sended transation to :%s money:%d hash: %s\n",cycles,gaptimems, revAddr, sent, id);
			} else {
				LogTrace("autotest", "not enough money  only: %f\n", reth);
			}
			MilliSleep(gaptimems < 1000 ? 1000 : gaptimems);
		}
		LogTrace("autotest", "%s thread exit\n", __FUNCTION__);
	} catch (boost::thread_interrupted) {
		LogTrace("autotest", "%s thread interrupt\n", __FUNCTION__);
	}catch(Object& objError){
		LogTrace("autotest", "%s error: %s", __FUNCTION__ , find_value(objError, "message").get_str());
	}

	catch (...) {
		LogTrace("autotest", "%s unkown err \n", __FUNCTION__);
	}
	LogTrace("autotest", "bettransation spend:%lf\n",total);
}
void CAutoTest::setgengerate(int cycles, int gaptimems) const {

	try {
		LogTrace("autotest", "thread start:%s\n", __FUNCTION__);
		while (--cycles > 0) {
				string strMethod = "setgenerate";
				Array strParams;
				strParams.push_back("true");
				ConvertTo<bool>(strParams[0]);
				string id = setgenerate(strParams, false).get_str();
			MilliSleep(gaptimems < 1000 ? 1000 : gaptimems);
		}
		LogTrace("autotest", "%s thread exit\n", __FUNCTION__);
	}
	catch (boost::thread_interrupted) {
		LogTrace("autotest", "%s thread interrupt\n", __FUNCTION__);
		//throw;
	}catch(Object& objError){
		LogTrace("autotest", "%s error: %s", __FUNCTION__ , find_value(objError, "message").get_str());
	}
	catch (...) {
		LogTrace("autotest", "%s setgengerate failed \n", __FUNCTION__);
		//throw;
	}

}

bool CAutoTest::Inital(std::string& revAddir, int cycles, int gaptimems, std::string &privateKey,
		std::string &lottoPrivateKey) {
	this->revAddr = revAddir;
	this->cycles = cycles;
	this->gaptimems = gaptimems;
	this->privateKey = privateKey;
	this->lottoPrivateKey = lottoPrivateKey;
	return true;
}
void CAutoTest::stopThread()
{

	BOOST_FOREACH(const PAIRTYPE(int,boost::shared_ptr<boost::thread>)& tep1 , threadmap)
	{
		boost::shared_ptr<boost::thread> thead = tep1.second;
		thead->interrupt();
		thead->join();
	}
	threadmap.clear();
}
void CAutoTest::startThread(int type) {

	if (threadmap.count(type)) {
		threadmap[type]->interrupt();
		threadmap[type]->join();
		threadmap.erase(type);
	}
	boost::shared_ptr<boost::thread> node;
	if (type == 1) {
		node.reset(new boost::thread(boost::bind(&CAutoTest::SendRandBet, this, cycles, gaptimems, revAddr)));
	} else if (type == 2) {
		node.reset(new boost::thread(boost::bind(&CAutoTest::SendRandTransation, this, cycles, gaptimems, revAddr)));
	} else if (type == 3) {
		node.reset(new boost::thread(boost::bind(&CAutoTest::CheckAndSendLottoKey, this, cycles, gaptimems, privateKey,
								lottoPrivateKey)));
	} else if (type == 4) {
		node.reset(new boost::thread(boost::bind(&CAutoTest::setgengerate, this, cycles, gaptimems)));
	} else if (type == 5) {
		node.reset(new boost::thread(boost::bind(&CAutoTest::SendCheckPoint, this, cycles, gaptimems, privateKey)));
	}
	else
		Assert(0);
	threadmap[type] = node;
}



} /* namespace test */
