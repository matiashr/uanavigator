/************************************************************************
* This file is part of UaNavigator					*	
*                                                                      	*
* Copyright 2020 Matias Henttunen					*
*                                                                      	*
* UA Navigator is free software; you can redistribute it and/or modify    	*
* it under the terms of the GNU Lesser General Public License as       	*
* published by the Free Software Foundation; either version 3 of       	*
* the License, or (at your option) any later version.                  	*
*                                                                      	*
* UA Navigator is distributed in the hope that it will be useful, but     	*
* WITHOUT ANY WARRANTY; without even the implied warranty of           	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
* GNU Lesser General Public License for more details.                  	*
*                                                                      	*
* You should have received a copy of the GNU Lesser General Public     	*
* License along with this program.  If not, see                        	*
* <http://www.gnu.org/licenses/>.                                      	*
*************************************************************************/


#include "opcua.h"
#include <pthread.h>
#include <algorithm>
#include <string>

#if UA_MULTITHREADING >= 200
#include <pthread.h>
static pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static FILE* outfile;
static void UA_Log_file_log(void *context, UA_LogLevel level, UA_LogCategory category, const char *msg, va_list args) {

	if ( context != NULL && (UA_LogLevel)(uintptr_t)context > level )
		return;

	UA_Int64 tOffset = UA_DateTime_localTimeUtcOffset();
	UA_DateTimeStruct dts = UA_DateTime_toStruct(UA_DateTime_now() + tOffset);

#if UA_MULTITHREADING >= 200
	pthread_mutex_lock(&printf_mutex);
#endif

	fprintf(outfile,"[%04u-%02u-%02u %02u:%02u:%02u.%03u (UTC%+05d)] %s/%s" "\t",
			dts.year, dts.month, dts.day, dts.hour, dts.min, dts.sec, dts.milliSec,
			(int)(tOffset / UA_DATETIME_SEC / 36), "*", "*");
	vfprintf(outfile,msg, args);
	fprintf(outfile,"\n");

#if UA_MULTITHREADING >= 200
	pthread_mutex_unlock(&printf_mutex);
#endif
}

static UA_Logger UA_Log_tofile(UA_LogLevel minlevel)
{
	UA_Logger logger = {UA_Log_file_log, (void*)minlevel, UA_Log_Stdout_clear};
	outfile = fopen("client.log", "w+");
	return logger;
}



OpcUa::OpcUa()
{
}

OpcUa::~OpcUa()
{
	fclose(outfile);
}


static void stopHandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
}



bool OpcUa::connect(char* a_server, int a_port)
{
//	signal(SIGINT, stopHandler);
//	signal(SIGTERM, stopHandler);
	client = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(client));
	UA_ClientConfig *config = UA_Client_getConfig(client);
	UA_ClientConfig_setDefault(config);
	m_logger = config->logger = UA_Log_tofile( UA_LOGLEVEL_INFO );
	UA_StatusCode retval = UA_Client_connect(client, a_server );
	if(retval != UA_STATUSCODE_GOOD) {
		UA_Client_delete(client);	
		m_server = "Not connected";
		return false;
	}
	m_server = a_server;
	m_port = a_port;
	return true;
}

void OpcUa::execute()
{
	UA_Client_run_iterate(client, 100);
}

bool OpcUa::reconnect()
{
	return connect( (char*)m_server.c_str(), m_port);
}

bool OpcUa::disconnect()
{
	UA_Client_disconnect(client);
	return true;
}


bool OpcUa::isConnected( std::string& r_status )
{
	UA_StatusCode status;
	UA_Client_getState(client, NULL, NULL, &status );
	r_status = std::string( UA_StatusCode_name(status) );
	if( status !=UA_STATUSCODE_GOOD ) {
		return false;
	}
	return true;
}


static uamodel::ObjClass classTranslate(UA_NodeClass a_class )
{
	switch(a_class)
	{
		case UA_NODECLASS_OBJECT: return uamodel::OBJECT;break;
		case UA_NODECLASS_VARIABLE: return uamodel::VARIABLE;break;
		case UA_NODECLASS_METHOD: return uamodel::METHOD;break;
	}
}

bool OpcUa::browse( uamodel::Object& a_dict )
{
	UA_BrowseRequest request;
	UA_BrowseRequest_init(&request);
	request.requestedMaxReferencesPerNode = 0;
	request.nodesToBrowse = UA_BrowseDescription_new();
	request.nodesToBrowseSize = 1;
	/* browse objects folder */
	request.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
	/* return everything */
	request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 			
	UA_BrowseResponse resp = UA_Client_Service_browse(client, request);

	UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Browse response");
	for(size_t i = 0; i < resp.resultsSize; ++i)
	{
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"resp:%d", i);
		for(size_t j = 0; j < resp.results[i].referencesSize; ++j)
		{
			UA_ReferenceDescription *ref = &(resp.results[i].references[j]);
			if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC)
			{
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"N) %-9d %-16d %-16.*s %-16.*s\n", 
						ref->nodeId.nodeId.namespaceIndex,
						ref->nodeId.nodeId.identifier.numeric, 
						(int)ref->browseName.name.length,
						ref->browseName.name.data, 
						(int)ref->displayName.text.length,
						ref->displayName.text.data
				);
				uamodel::Object* entry = new uamodel::Object(   (uint32_t)ref->nodeId.nodeId.namespaceIndex , 
										(uint32_t)ref->nodeId.nodeId.identifier.numeric,
										classTranslate( ref->nodeClass)  );
				entry->setIdentifier( ref->nodeId.nodeId.identifier.numeric );
				entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
				a_dict.add( entry);
				if( isBrowsable(ref->nodeClass) ) {
					browseNs( ref->nodeId.nodeId.namespaceIndex, (uint32_t)ref->nodeId.nodeId.identifier.numeric , entry);
				}

			} else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"S) %-9d %-16.*s %-16.*s %-16.*s\n", 
						ref->nodeId.nodeId.namespaceIndex,
						(int)ref->nodeId.nodeId.identifier.string.length,
						ref->nodeId.nodeId.identifier.string.data,
						(int)ref->browseName.name.length, 
						ref->browseName.name.data,
						(int)ref->displayName.text.length, 
						ref->displayName.text.data
				);	
				uamodel::Object* entry=	new uamodel::Object( (uint32_t)ref->nodeId.nodeId.namespaceIndex,
											std::string( (char*)ref->nodeId.nodeId.identifier.string.data ), 
											classTranslate( ref->nodeClass)  );
	
				entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
				a_dict.add( entry );
				if( isBrowsable(ref->nodeClass) ) {
					browseNs( ref->nodeId.nodeId.namespaceIndex, entry->getIdentifier(), entry);
				}

			}
		}
	}
	if( resp.resultsSize > 0 ) {
		return true;
	}
	return false;
}

bool OpcUa::browseNs( uint32_t a_ns, std::string a_id, uamodel::Object* a_dict )
{
	uamodel::Object* top=a_dict;
	UA_BrowseRequest request;
	UA_BrowseRequest_init(&request);
	request.requestedMaxReferencesPerNode = 0;
	request.nodesToBrowse = UA_BrowseDescription_new();
	request.nodesToBrowseSize = 1;
	/* browse objects folder */
	request.nodesToBrowse[0].nodeId = UA_NODEID_STRING(a_ns, (char*)a_id.c_str());
	/* return everything */
	request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 			
	UA_BrowseResponse resp = UA_Client_Service_browse(client, request);

	UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Browse response");
	for(size_t i = 0; i < resp.resultsSize; ++i)
	{
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"resp:%d", i);
		for(size_t j = 0; j < resp.results[i].referencesSize; ++j)
		{
			UA_ReferenceDescription *ref = &(resp.results[i].references[j]);
			if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC)
			{
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"N) %-9d %-16d %-16.*s %-16.*s\n", 
						ref->nodeId.nodeId.namespaceIndex,
						ref->nodeId.nodeId.identifier.numeric, 
						(int)ref->browseName.name.length,
						ref->browseName.name.data, 
						(int)ref->displayName.text.length,
						ref->displayName.text.data
				);
				uamodel::Object* entry = new uamodel::Object(   (uint32_t)ref->nodeId.nodeId.namespaceIndex , 
										(uint32_t)ref->nodeId.nodeId.identifier.numeric,
										classTranslate( ref->nodeClass)  );
				entry->setIdentifier( ref->nodeId.nodeId.identifier.numeric );
				entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
				top->add( entry);
				if( isBrowsable(ref->nodeClass) ) {
					browseNs( ref->nodeId.nodeId.namespaceIndex, (uint32_t)ref->nodeId.nodeId.identifier.numeric , entry);
				}

			} else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"oBjS) %-9d %-16.*s %-16.*s %-16.*s\n", 
						ref->nodeId.nodeId.namespaceIndex,
						(int)ref->nodeId.nodeId.identifier.string.length,
						ref->nodeId.nodeId.identifier.string.data,
						(int)ref->browseName.name.length, 
						ref->browseName.name.data,
						(int)ref->displayName.text.length, 
						ref->displayName.text.data
				);	
				uamodel::Object* entry=	new uamodel::Object( (uint32_t)ref->nodeId.nodeId.namespaceIndex,
											std::string( (char*)ref->nodeId.nodeId.identifier.string.data ), 
											classTranslate( ref->nodeClass)  );
				entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
				top->add( entry );
				if( isBrowsable(ref->nodeClass) ) {
					browseNs( ref->nodeId.nodeId.namespaceIndex, entry->getIdentifier(), entry);
				}

			}
		}
	}
	if( resp.resultsSize > 0 ) {
		return true;
	}
	return false;
}



bool OpcUa::browseNs( uint32_t a_ns, uint32_t a_id, uamodel::Object* a_dict )
{
	uamodel::Object* top=a_dict;
	UA_BrowseRequest request;
	UA_BrowseRequest_init(&request);
	request.requestedMaxReferencesPerNode = 0;
	request.nodesToBrowse = UA_BrowseDescription_new();
	request.nodesToBrowseSize = 1;
	/* browse objects folder */
	request.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(a_ns, a_id);
	/* return everything */
	request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 			
	UA_BrowseResponse resp = UA_Client_Service_browse(client, request);

	UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Browse Args" );
	for(size_t i = 0; i < resp.resultsSize; ++i)
	{
		for(size_t j = 0; j < resp.results[i].referencesSize; ++j)
		{
			UA_ReferenceDescription *ref = &(resp.results[i].references[j]);
			if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC)
			{
				uamodel::Object* entry = new uamodel::Object(   (uint32_t)ref->nodeId.nodeId.namespaceIndex , 
										(uint32_t)ref->nodeId.nodeId.identifier.numeric,
										classTranslate( ref->nodeClass)  );
				entry->setIdentifier( ref->nodeId.nodeId.identifier.numeric );
				entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
				top->add( entry);
				if( isBrowsable(ref->nodeClass) ) {
					browseNs( ref->nodeId.nodeId.namespaceIndex, (uint32_t)ref->nodeId.nodeId.identifier.numeric , entry);
				}
			} else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
				uamodel::Object* entry=	new uamodel::Object( (uint32_t)ref->nodeId.nodeId.namespaceIndex,
											std::string( (char*)ref->nodeId.nodeId.identifier.string.data ), 
											classTranslate( ref->nodeClass)  );
				entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
				top->add( entry );
				if( isBrowsable(ref->nodeClass) ) {
					browseNs( ref->nodeId.nodeId.namespaceIndex, entry->getIdentifier(), entry);
				}
			}
		}
	}
	if( resp.resultsSize > 0 ) {
		return true;
	}
	return false;
}



bool OpcUa::browseMethod(  uamodel::Object* a_obj, uamodel::Object& inputs, uamodel::Object& outputs)
{
#if 0
	inputs = uamodel::Object(0, "inputs", uamodel::VARIABLE);
	outputs= uamodel::Object(0, "outputs", uamodel::VARIABLE);
	uamodel::Object* arg0 = new uamodel::Object(0, "inarg0", uamodel::VARIABLE);
	uamodel::Object* arg1 = new uamodel::Object(0, "outarg0", uamodel::VARIABLE);

	uamodel::DataType a,b;
	a.Identifier = UA_DATATYPEKIND_BOOLEAN;
	b.Identifier = UA_DATATYPEKIND_INT16;
	arg0->setDisplayName("Arg1 i");
	arg1->setDisplayName("Arg1 q");
	arg0->setDataType(a);
	arg1->setDataType(b);
	inputs.add(arg0);
	outputs.add(arg1);
	return true;
#endif

	UA_BrowseRequest request;
	UA_BrowseRequest_init(&request);
	request.requestedMaxReferencesPerNode = 0;
	request.nodesToBrowse = UA_BrowseDescription_new();
	request.nodesToBrowseSize = 1;
	request.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 			
	if( a_obj->NodeId.IdType == uamodel::String ) {
		request.nodesToBrowse[0].nodeId = UA_NODEID_STRING( a_obj->NodeId.NamespaceIndex, (char*)(a_obj->NodeId.Identifier.c_str()) );
	} else if( a_obj->NodeId.IdType == uamodel::Numeric ) {
		request.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC( a_obj->NodeId.NamespaceIndex, a_obj->NodeId.IdentifierNum );
	} else {
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Invalid node" );
		return false;
	}
	UA_BrowseResponse resp = UA_Client_Service_browse(client, request);
	if( resp.resultsSize  <= 0 ) {
		return false;
	}
	uamodel::Object* entry;
	UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Browse method object" );
	int i=0;		//expecting 1 entry containing all data
	for(size_t j = 0; j < resp.results[i].referencesSize; ++j) {
		UA_ReferenceDescription *ref = &(resp.results[i].references[j]);
		if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
			entry = new uamodel::Object( (uint32_t)ref->nodeId.nodeId.namespaceIndex, std::string( (char*)ref->nodeId.nodeId.identifier.string.data ), classTranslate( ref->nodeClass)  );
			entry->setIdentifier( ref->nodeId.nodeId.identifier.numeric );
			entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
			UA_NodeId nodeId  = UA_NODEID_NUMERIC(ref->nodeId.nodeId.namespaceIndex, ref->nodeId.nodeId.identifier.numeric);
			UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"NS=%d Name= %d\n", 
								ref->nodeId.nodeId.namespaceIndex,
								ref->nodeId.nodeId.identifier.numeric 
							  );
			
			std::vector<uamodel::Object*> args;
			browseArgs( client,nodeId, args );
			//		r_data.add( entry);
		} else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
			entry = new uamodel::Object( (uint32_t)ref->nodeId.nodeId.namespaceIndex, std::string( (char*)ref->nodeId.nodeId.identifier.string.data ), classTranslate( ref->nodeClass)  );
			entry->setBrowseName( std::string( (char*)ref->browseName.name.data, ref->browseName.name.length) );
			//		r_data.add( entry );
			UA_NodeId nodeId  = UA_NODEID_STRING(ref->nodeId.nodeId.namespaceIndex, (char*)ref->nodeId.nodeId.identifier.string.data);

			UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"NS=%d Name= %.*s\n", 
								ref->nodeId.nodeId.namespaceIndex,
								ref->nodeId.nodeId.identifier.string.length, 
							 (char*)ref->nodeId.nodeId.identifier.string.data 
							);

			std::string nodeName = std::string( (char*)ref->nodeId.nodeId.identifier.string.data, ref->nodeId.nodeId.identifier.string.length );
			if ( nodeName.find("OutputArguments") != std::string::npos ) {
				std::vector<uamodel::Object*> output;
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Outputs array"); 
				browseArgs( client, nodeId, output );
				for( auto &i:output ) {
					outputs.add(i);
				}
			} else if ( nodeName.find("InputArguments") != std::string::npos ) {
				std::vector<uamodel::Object*> input;
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Inputs array"); 
				browseArgs( client, nodeId, input );
				for( auto &i:input ) {
					inputs.add( i );
				}
			}
		}
	}
	return true;
}

bool OpcUa::readVariable( uamodel::Object& a_obj, std::string& r_val )
{
	switch( a_obj.NodeClass )
	{
		case uamodel::VARIABLE:
			{
				UA_NodeId nodeId;
				UA_StatusCode retval;
				UA_Variant value; 				/* Variants can hold scalar values and arrays of any type */
				UA_Variant_init(&value);
				if( a_obj.NodeId.IdType == uamodel::Numeric ) {
					nodeId  = UA_NODEID_NUMERIC(a_obj.NodeId.NamespaceIndex, a_obj.NodeId.IdentifierNum);
					UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"read %d\n", a_obj.NodeId.IdentifierNum );
				} else if(  a_obj.NodeId.IdType == uamodel::String ) {
					nodeId  = UA_NODEID_STRING(a_obj.NodeId.NamespaceIndex, (char*)a_obj.NodeId.Identifier.c_str() );
					UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"read %s\n", (char*)a_obj.NodeId.Identifier.c_str() );
				} else {
					UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"cannot read %d is invalid node IdType\n", a_obj.NodeId.IdType );	
					r_val = "Bad node Id";
					return false;
				}
				retval = UA_Client_readValueAttribute(client, nodeId, &value);
				if(retval == UA_STATUSCODE_GOOD ) 
				{
					/*
						 UA_DATATYPEKIND_BOOLEAN = 0,
						 UA_DATATYPEKIND_SBYTE = 1,
						 UA_DATATYPEKIND_BYTE = 2,
						 UA_DATATYPEKIND_INT16 = 3,
						 UA_DATATYPEKIND_UINT16 = 4,
						 UA_DATATYPEKIND_INT32 = 5,
						 UA_DATATYPEKIND_UINT32 = 6,
						 UA_DATATYPEKIND_INT64 = 7,
						 UA_DATATYPEKIND_UINT64 = 8,
						 UA_DATATYPEKIND_FLOAT = 9,
						 UA_DATATYPEKIND_DOUBLE = 10,
						 UA_DATATYPEKIND_STRING = 11,
						 UA_DATATYPEKIND_DATETIME = 12,
						 UA_DATATYPEKIND_GUID = 13,
						 UA_DATATYPEKIND_BYTESTRING = 14,
						 UA_DATATYPEKIND_XMLELEMENT = 15,
						 UA_DATATYPEKIND_NODEID = 16,
						 UA_DATATYPEKIND_EXPANDEDNODEID = 17,
						 UA_DATATYPEKIND_STATUSCODE = 18,
						 UA_DATATYPEKIND_QUALIFIEDNAME = 19,
						 UA_DATATYPEKIND_LOCALIZEDTEXT = 20,
						 UA_DATATYPEKIND_EXTENSIONOBJECT = 21,
						 UA_DATATYPEKIND_DATAVALUE = 22,
						 UA_DATATYPEKIND_VARIANT = 23,
						 UA_DATATYPEKIND_DIAGNOSTICINFO = 24,
						 UA_DATATYPEKIND_DECIMAL = 25,
						 UA_DATATYPEKIND_ENUM = 26,
						 UA_DATATYPEKIND_STRUCTURE = 27,
						 UA_DATATYPEKIND_OPTSTRUCT = 28, /* st
						 UA_DATATYPEKIND_UNION = 29,
						 UA_DATATYPEKIND_BITFIELDCLUSTER = 30
					*/
					a_obj.m_type.Identifier = value.type->typeKind;
					switch(value.type->typeKind )
					{
						case UA_DATATYPEKIND_BOOLEAN:
							{
								UA_Boolean b = *(UA_Boolean*) value.data;
								if( b == 1 ) {
									r_val = "true";
								} else {
									r_val = "false";
								}
							} break;
						case UA_DATATYPEKIND_BYTE:
							{
								UA_Byte byte= *(UA_Byte*) value.data;
								std::stringstream ss;
								ss << byte << "/0x" << std::hex << byte ;
								ss >> r_val;
							}break;
						case UA_DATATYPEKIND_INT16:
							{
								UA_Int16 i16= *(UA_Int16*) value.data;
								std::stringstream ss;
								ss << i16 << "/0x" << std::hex << i16;
								ss >> r_val;
							}break;
						case UA_DATATYPEKIND_INT32:
							{
								UA_Int32 i32= *(UA_Int32*) value.data;
								std::stringstream ss;
								ss << i32 << "/0x" << std::hex << i32;
								ss >> r_val;
							}break;
						case UA_DATATYPEKIND_SBYTE:
							{
							}break;
						case UA_DATATYPEKIND_UINT16:
							{
								UA_UInt16 u16= *(UA_UInt16*) value.data;
								std::stringstream ss;
								ss << u16 << "/0x" << std::hex << u16;
								ss >> r_val;
							}break;
						case UA_DATATYPEKIND_UINT32:
							{
								UA_UInt32 u32= *(UA_UInt32*) value.data;
								std::stringstream ss;
								ss << u32 << "/0x" << std::hex << u32;
								ss >> r_val;

							}break;
						case UA_DATATYPEKIND_DOUBLE:
							{
								UA_Double d = *(UA_Double*) value.data;
								std::stringstream ss;
								ss << d;
								ss >> r_val;
							}break;
						case UA_DATATYPEKIND_FLOAT:
							{
								UA_Float d = *(UA_Float*) value.data;
							}break;
						case UA_DATATYPEKIND_STRING:
							{
								UA_String str = *(UA_String*) value.data;
								size_t len = str.length;
								r_val = std::string( (char*)str.data, len );
							} break;
						case UA_DATATYPEKIND_LOCALIZEDTEXT:
							{
								UA_LocalizedText str = *(UA_LocalizedText*) value.data;
								size_t len = str.text.length;
								r_val = std::string( (char*)str.text.data, len );

							}break;
						default:
							//	r_val = "Data type: "+std::to_string(value.type->typeKind) + " is readble, but type is unsupported";
							break;
					}

					UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"RD) read %s ok\n", a_obj.BrowseName.c_str() );
				} else {
					std::stringstream stream;
					stream << "Error: " << (char*)UA_StatusCode_name(retval) << " (" << std::hex << retval << ")";
					r_val = stream.str();
					UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"RD) failed to read %s\n", a_obj.BrowseName.c_str() );
				}

				/* Clean up */
				UA_Variant_clear(&value);
			}break;
		default:
			r_val = "No readable value";
	}
	return true;
}



bool OpcUa::writeVariable( const uamodel::Object& a_obj, std::string a_val, std::string& r_result )
{
	UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Write variable %s",  a_obj.getDisplayName());
	UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"value=%s",  a_val.c_str()  );
	UA_NodeId nodeId;
	UA_StatusCode retval;
	UA_Variant *var= UA_Variant_new();
	if( a_obj.NodeId.IdType == uamodel::Numeric ) {
		nodeId  = UA_NODEID_NUMERIC(a_obj.NodeId.NamespaceIndex, a_obj.NodeId.IdentifierNum);
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"read %d\n", a_obj.NodeId.IdentifierNum );
	} else if(  a_obj.NodeId.IdType == uamodel::String ) {
		nodeId  = UA_NODEID_STRING(a_obj.NodeId.NamespaceIndex, (char*)a_obj.NodeId.Identifier.c_str() );
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"read %s\n", (char*)a_obj.NodeId.Identifier.c_str() );
	} else {
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"cannot use %d is invalid node IdType\n", a_obj.NodeId.IdType );	
		r_result = "Bad node Id";
		UA_Variant_delete(var);
		return false;
	}
	uamodel::DataType datatype = a_obj.getDataType();
	switch(datatype.Identifier)
	{
		case UA_DATATYPEKIND_BOOLEAN:
			{
				UA_Boolean value=false;
				transform( a_val.begin(), a_val.end(),a_val.begin(), ::toupper);
				if( a_val == "TRUE" || a_val =="1" )value=true;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
			}break;
		case UA_DATATYPEKIND_SBYTE: 
			{
				UA_SByte value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_SBYTE]);
			}break;
		case UA_DATATYPEKIND_BYTE:
			{
				UA_Byte value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_BYTE]);
			}break;
		case UA_DATATYPEKIND_INT16: 
			{
				UA_Int16 value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_INT16]);
			}break;

		case UA_DATATYPEKIND_UINT16:
			{
				UA_UInt16 value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_UINT16]);
			}break;
		case UA_DATATYPEKIND_INT32 :
			{
				UA_Int32 value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_INT32]);
			}break;
		case UA_DATATYPEKIND_UINT32 :
			{
				UA_UInt32 value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_UINT32]);
			}break;
		case UA_DATATYPEKIND_INT64 :
			{
				UA_Int64 value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_INT64]);
			}break;
		case UA_DATATYPEKIND_UINT64:
			{
				UA_UInt64 value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_INT64]);
			}break;
		case UA_DATATYPEKIND_FLOAT :
			{
				UA_Float value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_FLOAT]);
			}break;

		case UA_DATATYPEKIND_DOUBLE :
			{
				UA_Double value;
				std::istringstream(a_val) >> value;
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
			}break;

		case UA_DATATYPEKIND_LOCALIZEDTEXT:
		case UA_DATATYPEKIND_STRING :
			{
				std::vector<char> char_array(a_val.begin(), a_val.end());
				char_array.push_back(0);
				UA_String value = UA_STRING( &char_array[0] );
				UA_Variant_setScalarCopy(var, &value, &UA_TYPES[UA_TYPES_STRING]);
			}break;
		case UA_DATATYPEKIND_DATETIME:
		case UA_DATATYPEKIND_GUID :
		case UA_DATATYPEKIND_BYTESTRING:
		case UA_DATATYPEKIND_XMLELEMENT :
		case UA_DATATYPEKIND_NODEID :
		case UA_DATATYPEKIND_EXPANDEDNODEID :
		case UA_DATATYPEKIND_STATUSCODE :
		case UA_DATATYPEKIND_QUALIFIEDNAME :
		case UA_DATATYPEKIND_EXTENSIONOBJECT :
		case UA_DATATYPEKIND_DATAVALUE :
		case UA_DATATYPEKIND_VARIANT :
		case UA_DATATYPEKIND_DIAGNOSTICINFO:
		case UA_DATATYPEKIND_DECIMAL :
		case UA_DATATYPEKIND_ENUM :
		case UA_DATATYPEKIND_STRUCTURE :
		case UA_DATATYPEKIND_OPTSTRUCT :
		case UA_DATATYPEKIND_UNION :
		case UA_DATATYPEKIND_BITFIELDCLUSTER:
			r_result = "Unsupported type";
			UA_Variant_delete(var);
			return false;
			break;
		default:
			return false;
	}
	retval = UA_Client_writeValueAttribute(client, nodeId, var);
	UA_Variant_delete(var);
	if(retval != UA_STATUSCODE_GOOD ) {
		std::stringstream stream;
		stream << "Error: " << (char*)UA_StatusCode_name(retval) << " ("<< std::hex << retval << ")";
		r_result = stream.str();
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"RD) failed to write %s\n",  a_obj.toString() );
		return false;
	}

	return true;
}

void OpcUa::browseArgs(UA_Client* client, UA_NodeId aid, std::vector<uamodel::Object*>& r_args )
{
	UA_ReadRequest request;
	UA_ReadValueId id;
	UA_ReadRequest_init(&request);
	UA_ReadValueId_init(&id);
	UA_Variant value;
	UA_Variant_init(&value);
	id.attributeId          = UA_ATTRIBUTEID_NODECLASS;
	id.nodeId               = aid;
	request.nodesToReadSize = 1;
	request.nodesToRead     = (UA_ReadValueId*)UA_Array_new(request.nodesToReadSize, &UA_TYPES[UA_TYPES_READVALUEID]);
	request.nodesToRead[0].nodeId = aid; 
	request.nodesToRead[0].attributeId = 0xd;		//What attribute is this exactly, TODO
	UA_ReadResponse response = UA_Client_Service_read(client, request);
	printf("Contains %d entries\n", response.resultsSize );
	for(int i=0; i < response.resultsSize; i++ ) {
		UA_DataValue* dv = (UA_DataValue*)( &response.results[i] );
		UA_Variant* var = &dv->value;
		const UA_DataType* type = var->type;	//var->data
		printf("Variant len=%d\n", var->arrayLength);
		UA_ExtensionObject* ext = (UA_ExtensionObject*)var->data;
		for(int a=0; a<var->arrayLength; a++) {
			UA_Argument* arg  = (UA_Argument*) ext->content.decoded.data;
			UA_NodeId* id = &arg->dataType;
			const UA_DataType* type = ext->content.decoded.type;
			if( type->typeId.identifier.numeric  == UA_NS0ID_ARGUMENT ) {
				//printf("---> TYP:%d\n", type->typeId.identifier.numeric );
				UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Name= %.*s %d (%s) \n", arg->name.length, (char*)arg->name.data,  id->identifier.numeric, UA_TYPES[id->identifier.numeric].typeName );
				uamodel::Object* obj = new uamodel::Object(0, std::string( (char*)arg->name.data ,arg->name.length), uamodel::UNSPECIFIED );
				obj->m_type.Identifier = id->identifier.numeric;
				r_args.push_back(obj);
			}
			ext++;
		}
	}
}

/*
	a_obj :
		contains a valid nodeid , and Value represented as string from the linedit input in the gui
		

*/
static bool Object2UaType( uamodel::Object* a_obj, UA_Variant &a_var )
{
	uint32_t type = a_obj->getDataType().Identifier;
	switch( type  )
	{
		case UA_DATATYPEKIND_LOCALIZEDTEXT:
		case UA_DATATYPEKIND_BYTESTRING :
		case UA_DATATYPEKIND_STRING:
			{
				UA_String argString = UA_STRING( (char*)a_obj->getValue().getStringValue().c_str()  );
				UA_Variant_setScalarCopy(&a_var, &argString, &UA_TYPES[UA_TYPES_STRING]);
				return true;
			}break;
			case UA_DATATYPEKIND_BOOLEAN:
			{
				UA_Boolean bol= UA_Boolean( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_BOOLEAN]);
				return true;
			}break;
			case UA_DATATYPEKIND_SBYTE: 
			{
				UA_SByte bol= UA_SByte( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_SBYTE]);
				return true;
			}break;
			case UA_DATATYPEKIND_BYTE : 
			{
				UA_Byte bol= UA_Byte( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_BYTE]);
				return true;
			}break;
			case UA_DATATYPEKIND_INT16 :
			{
				UA_Int16 bol= UA_Int16( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_INT16]);
				return true;
			}break;
			case UA_DATATYPEKIND_UINT16 :
			{
				UA_UInt16 bol= UA_UInt16( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_UINT16]);
				return true;
			}break;
			case UA_DATATYPEKIND_INT32 :
			{
				UA_Int32 bol= UA_Int32( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_INT32]);
				return true;
			}break;
			case UA_DATATYPEKIND_UINT32 :
			{
				UA_UInt32 bol= UA_UInt32( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_UINT32]);
				return true;
			}break;
			case UA_DATATYPEKIND_INT64 :
			{
	#warning TODO_ATOI_DOESNT_READ_64BITS
				UA_Int64 bol= UA_Int64( atoi(a_obj->getValue().getStringValue().c_str())  );		
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_INT64]);
				return true;
			}break;
			case UA_DATATYPEKIND_UINT64 :
			{
				UA_UInt64 bol= UA_UInt64( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_UINT64]);
				return true;
			}break;
			case UA_DATATYPEKIND_FLOAT :
			{
				UA_Float bol= UA_Float( std::stof(a_obj->getValue().getStringValue())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_FLOAT]);
				return true;
			}break;
			case UA_DATATYPEKIND_DOUBLE :
			{
				UA_Double bol= UA_Double( std::stof(a_obj->getValue().getStringValue())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_DOUBLE]);
				return true;
			}break;
			case UA_DATATYPEKIND_ENUM :
			{
				UA_UInt32 bol= UA_UInt32( atoi(a_obj->getValue().getStringValue().c_str())  );
				UA_Variant_setScalarCopy(&a_var, &bol, &UA_TYPES[UA_TYPES_UINT32]);
				return true;
			}break;

			case UA_DATATYPEKIND_DATETIME :
			case UA_DATATYPEKIND_GUID :
			case UA_DATATYPEKIND_XMLELEMENT:
			case UA_DATATYPEKIND_NODEID :
			case UA_DATATYPEKIND_EXPANDEDNODEID :
			case UA_DATATYPEKIND_STATUSCODE :
			case UA_DATATYPEKIND_QUALIFIEDNAME:
			case UA_DATATYPEKIND_EXTENSIONOBJECT:
			case UA_DATATYPEKIND_DATAVALUE:
			case UA_DATATYPEKIND_VARIANT:
			case UA_DATATYPEKIND_DIAGNOSTICINFO :
			case UA_DATATYPEKIND_DECIMAL :
			case UA_DATATYPEKIND_STRUCTURE:
			case UA_DATATYPEKIND_OPTSTRUCT:
			case UA_DATATYPEKIND_UNION:
			case UA_DATATYPEKIND_BITFIELDCLUSTER:
				printf("Unsupported variable type\n" );
				return false;
			break;
	}
	return false;
}


// from a variant determine value and return it as a string
static std::string UaVariant2String( UA_Variant* a_val )
{
	std::ostringstream  retval;
	printf(" typeindex=%x\n", a_val->type->typeIndex );
	retval <<"";
	switch(a_val->type->typeIndex)
	{
		case UA_DATATYPEKIND_BOOLEAN:
			{
				UA_Boolean* value=(UA_Boolean*)a_val->data;
				if( *value ) {
					retval <<"true";
				} else {
					retval << "false";
				}
			}break;
		case UA_DATATYPEKIND_SBYTE:  { UA_SByte*  value = (UA_SByte*)a_val->data; retval << (int)*value; }break;
		case UA_DATATYPEKIND_BYTE:   { UA_Byte*   value = (UA_Byte*)a_val->data; retval << (int)*value; }break;
		case UA_DATATYPEKIND_INT16:  { UA_Int16*  value = (UA_Int16*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_UINT16: { UA_UInt16* value = (UA_UInt16*)a_val->data;; retval << *value; }break;
		case UA_DATATYPEKIND_INT32 : { UA_Int32*  value = (UA_Int32*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_UINT32: { UA_UInt32* value = (UA_UInt32*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_INT64:  { UA_Int64*  value = (UA_Int64*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_UINT64: { UA_UInt64* value =(UA_UInt64*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_FLOAT : { UA_Float* value = (UA_Float*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_DOUBLE :{ UA_Double* value = (UA_Double*)a_val->data; retval << *value; }break;
		case UA_DATATYPEKIND_LOCALIZEDTEXT:
		case UA_DATATYPEKIND_STRING :
			{
				UA_String* str = (UA_String*)a_val->data;
				retval << std::string( (char*)str->data, str->length );
			}break;
		case UA_DATATYPEKIND_DATETIME:
		case UA_DATATYPEKIND_GUID :
		case UA_DATATYPEKIND_BYTESTRING:
		case UA_DATATYPEKIND_XMLELEMENT :
		case UA_DATATYPEKIND_NODEID :
		case UA_DATATYPEKIND_EXPANDEDNODEID :
		case UA_DATATYPEKIND_STATUSCODE :
		case UA_DATATYPEKIND_QUALIFIEDNAME :
		case UA_DATATYPEKIND_EXTENSIONOBJECT :
		case UA_DATATYPEKIND_DATAVALUE :
		case UA_DATATYPEKIND_VARIANT :
		case UA_DATATYPEKIND_DIAGNOSTICINFO:
		case UA_DATATYPEKIND_DECIMAL :
		case UA_DATATYPEKIND_ENUM :
		case UA_DATATYPEKIND_STRUCTURE :
		case UA_DATATYPEKIND_OPTSTRUCT :
		case UA_DATATYPEKIND_UNION :
		case UA_DATATYPEKIND_BITFIELDCLUSTER:
		default:
			return "Unsupported type";
			break;

	}
	return retval.str();
}


// Call method defined in a_obj
// take argument values from a_inputs, and return data in r_outputs as strings
// return true on ok else false, r_status with reason
bool OpcUa::callMethod( uamodel::Object* a_object, 
			uamodel::Object* a_method, 
			uamodel::Object& a_inputs, 
			std::vector<std::string>& r_outputs, 
			std::string& r_status )
{
	r_status ="Called";
	bool status;
	UA_StatusCode retval;
	size_t outputSize=r_outputs.size();
	size_t inputSize=a_inputs.getChildren().size();
	UA_Variant input[ inputSize ];
	size_t pos=0;
	for( auto arg: a_inputs.getChildren()) {
		UA_Variant_init( &input[pos] );
		if(! Object2UaType( arg, input[pos]  ) ) {
			//coulnt determine UA type
			return false;
		}
		pos++;
	}
	UA_Variant *output;
	retval = UA_Client_call(client, a_object->asUaNodeId() , a_method->asUaNodeId() , inputSize, input, &outputSize, &output);
	if ( retval == UA_STATUSCODE_GOOD) {
		printf("Method call was successfull, and %lu returned values available.\n", (unsigned long)outputSize);
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Invalid node" );
		status = true;
		for(size_t i=0; i< outputSize; i++) {		
			UA_Variant *oarg= &output[i];
			if( UA_Variant_isScalar( oarg ) ) {
				// builtin type
				std::string value = UaVariant2String( oarg );
				r_outputs.push_back( value );
			} else {
				//TODO: array type
				r_outputs.push_back( "<This type hasnt been implemented>");
			}
	//		const UA_DataType type = &output->type;
	//		output->data is scalar data or array
		}

		UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
	} else {
		UA_LOG_INFO( &m_logger, UA_LOGCATEGORY_USERLAND,"Method call failed %s", (char*)UA_StatusCode_name(retval) );
		status = false;
		r_status = "failed:" + std::string( UA_StatusCode_name(retval) );
	}
	for( pos=0;pos<inputSize; pos++ ) {
		UA_Variant_deleteMembers( &input[pos]);
	}
	return status;
}

bool OpcUa::isBrowsable( UA_NodeClass c )
{
#if SHOW_FULL_DATA
	if( c == UA_NODECLASS_OBJECT ||c == UA_NODECLASS_METHOD) {		// note: c == UA_NODECLASS_METHOD makes browse show all things even in out of methods, this is slow but comprehencive, maybe make this a option ?
		return true;
	}
#else
	if( c == UA_NODECLASS_OBJECT ) {	
		return true;
	}
#endif
	return false;
}

