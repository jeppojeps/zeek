#ifndef dhcp_binpac_h
#define dhcp_binpac_h

#include "UDP.h"

#include "dhcp_pac.h"


class DHCP_Analyzer_binpac : public analyzer::Analyzer {
public:
	DHCP_Analyzer_binpac(Connection* conn);
	virtual ~DHCP_Analyzer_binpac();

	virtual void Done();
	virtual void DeliverPacket(int len, const u_char* data, bool orig,
					int seq, const IP_Hdr* ip, int caplen);

	static analyzer::Analyzer* InstantiateAnalyzer(Connection* conn)
		{ return new DHCP_Analyzer_binpac(conn); }

protected:
	binpac::DHCP::DHCP_Conn* interp;
};

#endif
