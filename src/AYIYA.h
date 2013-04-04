#ifndef AYIYA_h
#define AYIYA_h

#include "ayiya_pac.h"

class AYIYA_Analyzer : public analyzer::Analyzer {
public:
	AYIYA_Analyzer(Connection* conn);
	virtual ~AYIYA_Analyzer();

	virtual void Done();
	virtual void DeliverPacket(int len, const u_char* data, bool orig,
					int seq, const IP_Hdr* ip, int caplen);

	static analyzer::Analyzer* InstantiateAnalyzer(Connection* conn)
		{ return new AYIYA_Analyzer(conn); }

protected:
	friend class AnalyzerTimer;
	void ExpireTimer(double t);

	binpac::AYIYA::AYIYA_Conn* interp;
};

#endif
