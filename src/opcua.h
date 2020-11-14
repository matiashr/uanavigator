#ifndef OPC_UA_CPP_H
#define OPC_UA_CPP_H
#include "open62541.h"
#include "dict.h"
#include <stdint.h>

typedef struct{
	unsigned char* name;
	uint32_t nodeid;
	unsigned char* nodename;		// path if idtype is string, i.e. PLC1.Manual
	char idtype;		// 's' = string id, 'n' = numeric id
	int ns;
}OpcuaNode_t ;

class OpcUa
{
	public:
		OpcUa();
		virtual ~OpcUa();
		bool connect(char* a_server, int a_port);
		bool isConnected( std::string& r_state );
		bool disconnect();
		bool reconnect();
		void execute();
	public:
		bool browse( uamodel::Object& a_dict);
		bool browseNs( uint32_t ns,std::string id, uamodel::Object* a_dict);
		bool browseNs( uint32_t ns, uint32_t id, uamodel::Object* a_dict);
		bool browseMethod(  uamodel::Object* a_obj, uamodel::Object& inputs, uamodel::Object& outputs);
	public:
		bool readVariable( uamodel::Object& a_obj, std::string& r_val );					//read variable described by a_obj, also update a_obj, to contain the type
		bool writeVariable( const uamodel::Object& a_obj, std::string a_val, std::string& r_res );			
	public:
		bool callMethod( uamodel::Object* a_object, uamodel::Object* a_method, uamodel::Object& a_inputs, std::vector<std::string>& r_outputs, std::string& r_status);
		std::string m_server;
		uint32_t m_port;
	private:
		void browseArgs(UA_Client* client, UA_NodeId aid, std::vector<uamodel::Object*>& r_args );
		bool isBrowsable( UA_NodeClass c );
		UA_Client *client;
		UA_Logger m_logger;
};

#endif
