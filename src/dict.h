
/************************************************************************
* This file is part of UaNavigator					*	
*                                                                      	*
* Copyright 2020 Matias Henttunen					*
*                                                                      	*
* FINAL CUT is free software; you can redistribute it and/or modify    	*
* it under the terms of the GNU Lesser General Public License as       	*
* published by the Free Software Foundation; either version 3 of       	*
* the License, or (at your option) any later version.                  	*
*                                                                      	*
* FINAL CUT is distributed in the hope that it will be useful, but     	*
* WITHOUT ANY WARRANTY; without even the implied warranty of           	*
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
* GNU Lesser General Public License for more details.                  	*
*                                                                      	*
* You should have received a copy of the GNU Lesser General Public     	*
* License along with this program.  If not, see                        	*
* <http://www.gnu.org/licenses/>.                                      	*
*************************************************************************/
#ifndef DICT_H
#define DICT_H
#include <iostream>
#include <regex>
#include <utility> 
#include <unordered_map>
#include <vector>
#include <map>
#include <algorithm>
#include <stdint.h>

using namespace std;
class ListView;
class ServerDialog;
class OpcUa;

namespace uamodel {
	

	class Value {
		public:	
			Value():m_value(nullptr) { };
			Value(void* a_value):m_value(a_value) {};
			Value(std::string  a_value):m_strvalue(a_value) {};
			std::string getStringValue() {return m_strvalue; };
			void* getValue() {return m_value; };
		private:
			std::string m_strvalue;
			void* m_value;
	};

	#include "open62541.h"
	static inline std::string dataType2String( uint32_t a_kind, bool a_abstract ) {
	
		if( a_abstract ) {
			switch(a_kind)
			{
				case UA_DATATYPEKIND_BOOLEAN:return "Boolean";break;
				case UA_DATATYPEKIND_SBYTE: return "sbyte";break;
				case UA_DATATYPEKIND_BYTE: return "byte";break;
				case UA_DATATYPEKIND_INT16: return "int16";break;
				case UA_DATATYPEKIND_UINT16:return "uint16";break;
				case UA_DATATYPEKIND_INT32 :return "int32";break;
				case UA_DATATYPEKIND_UINT32 :return "uint32";break;
				case UA_DATATYPEKIND_INT64 :return "int64";break;
				case UA_DATATYPEKIND_UINT64:return "uint64";break;
				case UA_DATATYPEKIND_FLOAT :return "float";break;
				case UA_DATATYPEKIND_DOUBLE :return "double";break;
				case UA_DATATYPEKIND_STRING :return "string";break;
				case UA_DATATYPEKIND_DATETIME:return "datatime";break;
				case UA_DATATYPEKIND_GUID :return "guid";break;
				case UA_DATATYPEKIND_BYTESTRING:return "bytestring";break;
				case UA_DATATYPEKIND_XMLELEMENT :return "xmlelement";break;
				case UA_DATATYPEKIND_NODEID :return "nodeid";break;
				case UA_DATATYPEKIND_EXPANDEDNODEID :return "expanded node id";break;
				case UA_DATATYPEKIND_STATUSCODE :return "status code";break;
				case UA_DATATYPEKIND_QUALIFIEDNAME :return "quilified name";break;
				case UA_DATATYPEKIND_LOCALIZEDTEXT:return "localized string";break;
				case UA_DATATYPEKIND_EXTENSIONOBJECT :return "obj";break;
				case UA_DATATYPEKIND_DATAVALUE :return "datavalue";break;
				case UA_DATATYPEKIND_VARIANT :return "variant";break;
				case UA_DATATYPEKIND_DIAGNOSTICINFO:return "diag info";break;
				case UA_DATATYPEKIND_DECIMAL :return "decimal";break;
				case UA_DATATYPEKIND_ENUM :return "enum";break;
				case UA_DATATYPEKIND_STRUCTURE :return "structure";break;
				case UA_DATATYPEKIND_OPTSTRUCT :return "optstruct";break;
				case UA_DATATYPEKIND_UNION :return "union";break;
				case UA_DATATYPEKIND_BITFIELDCLUSTER:return "bitfield cluster";break;

			}
		} else {
			return "User defined type";
		}
		return "*";
	}

	class DataType{
		public:	
			void dump() {
				cout << "ns:" << NamespaceIndex << " id: " << Identifier << " idtype:" << IdentifierType ;	
			};
			uint32_t NamespaceIndex;
			uint32_t IdentifierType;			// Numeric=4, string= , guid, opaque , this is the type of nodeid
			uint32_t Identifier;				// datatype, string, int, localized text etc
			bool isAbstract;				// if true then type is one of Uint16,32 etc
	};


	enum IdentifierType {Numeric, String};
	enum ObjClass {UNSPECIFIED=0, OBJECT=1, VARIABLE=2, METHOD=4, DATATYPE=64};
	class Object {
		public:
			Object(void) {
				NodeId.Identifier = "RootFolder";
				NodeId.NamespaceIndex = 0;
				NodeId.IdType = String;
				DisplayName = "RootFolder";
				NodeClass= UNSPECIFIED;
				BrowseName=DisplayName;
				Description=DisplayName;
			}
			Object(  uint32_t ns, string identifier, enum ObjClass type) {
				NodeId.Identifier = identifier;
				NodeId.NamespaceIndex = ns;
				NodeId.IdType = String;
				NodeClass= type;
			};
			Object(  uint32_t ns, uint32_t identifier, enum ObjClass type) {
				NodeId.IdentifierNum= identifier;
				NodeId.NamespaceIndex = ns;
				NodeId.IdType = Numeric;
				NodeClass = type;
			};
			void setDisplayName(string a_name ) {
				DisplayName = a_name;
			};
			string getDisplayName() const {
				return DisplayName ;
			};
			string getIdentifier() {
				return NodeId.Identifier;
			};
			void setIdentifier(string name) {
				NodeId.Identifier=name;
				NodeId.IdType =String;
			};
			void setIdentifier(uint32_t num) {
				NodeId.IdentifierNum=num;
				NodeId.IdType =Numeric;
			};

			void setBrowseName(string a_name ) {
				BrowseName= a_name;
			};
			string getBrowseName() {
				return BrowseName;
			};
	
			void setDescription(string a_name ) {
				Description= a_name;
			};
			void setDataType( uamodel::DataType a_type ){
				m_type = a_type;
			};
			uamodel::DataType getDataType() const{
				return m_type;
			};

			std::string toString() const {
				std::string ret;
				ret = "NS:"+std::to_string(NodeId.NamespaceIndex);
				if( NodeId.IdType== Numeric) ret+=" NodeId:"+std::to_string(NodeId.IdentifierNum);
				if( NodeId.IdType== String) ret+=" NodeId:"+NodeId.Identifier;
				return ret;
			};

			void setData( Value a_data, DataType& a_type) { m_value = a_data; m_type = a_type; };
			Value getData() { return m_value;};
			virtual ~Object() {};

			bool add( Object* a_object) {
				if( find(m_children.begin(), m_children.end(), a_object) != m_children.end() ) {
					cerr << a_object->getDisplayName() << "Already exists" <<endl;
					return false;
				} 
				a_object->m_parentNode = this;				//save reference to parent node in each added node
				m_children.push_back(a_object);
				return true;
			};

			inline bool operator==(const Object& lhs  ){ 
				if(  NodeId.Identifier  == lhs.NodeId.Identifier) return true;
				return false;
			};
	

			static void indent(int levels) { for(int i=0;i < levels; i++) cout <<"\t"; };
			void info(int levels) {
				indent(levels);
				cout << "Ns:" << NodeId.NamespaceIndex << endl;
				indent(levels);
				if( NodeId.IdType == String ) {
					cout << "Id (str):" << NodeId.Identifier ;
				} else {
					cout << "Id (num):" << NodeId.IdentifierNum ;
				}
				cout << endl;
				indent(levels);
				cout << "DisplayName:" << DisplayName <<endl;
				indent(levels);
				switch( NodeClass ) 
				{
					case UNSPECIFIED: cout << "Class: unspecified\n";break;
					case OBJECT: cout << "Class: object\n";break;
					case DATATYPE: cout << "Class: datatype\n";break;
					case METHOD: cout << "Class: method\n";break;
					case VARIABLE: 
					{
						cout << "Class: variable\n";
						indent(levels);
						cout << "[ ";
						m_type.dump();
						cout << " ]\n";
					}
					break;
				}
			}

			void dump(int levels){
				info(levels);
				if( m_children.size() > 0 ) {
					indent(levels);
					cout << "|\n";
					indent(levels); cout << "+-------\n";
					for(auto const& value: m_children) {
						value->dump(levels+1);
						cout  << endl;
					}
				} 
				if( levels == 0 ) 
					cout << endl; 
				else {
					indent(levels);
					cout << "|";
				}
			};

			void mkGraph(string name ) {
				FILE * fd = fopen(name.c_str(), "w");
				fprintf(fd, "digraph g {");
				graph(fd);
				fprintf(fd, "}\n");
				fclose(fd);
			}

			void graph(FILE* fd ) {
				for(auto const& child : m_children) {
					fprintf(fd, " %s->%s;\n", NodeId.Identifier.c_str(), child->NodeId.Identifier.c_str()  );
					child->graph(fd);
				}
			}
				// == addToTree() 
			typedef void*(*add2Tree)(void*widget, void*iterator, uamodel::Object&);

			bool forEach( void* parentWidgetOpaq, void* rIterator, Object& obj, add2Tree a_callback ) {
				for( auto item : m_children ) {
					void* leafItem= a_callback(parentWidgetOpaq, rIterator, *item);
					//leafItem is created by callback, where to delete TODO
					item->forEach( parentWidgetOpaq, leafItem, *item, a_callback );
				}
				return true;
			}
	
			vector<Object*> getChildren() { return m_children; };

			Object* Find( string criteria ) {
				if( criteria == "") return nullptr;
				try {
					regex pattern(criteria);
				} catch( exception &e ) {
					cerr <<"Bad search\n";
					return nullptr;
				}

				regex pattern(criteria);
				if( regex_match(NodeId.Identifier, pattern) ) return this;
				
				if( m_children.size() > 0 ) {
					for( auto item : m_children ) {
						//if( item->NodeId.Identifier == criteria ) {	
						if( regex_match(item->NodeId.Identifier, pattern) ) {
							return item;
						} else {
							Object* obj = item->Find(criteria);
							if( obj != nullptr) return obj;
						}

					}
				} 
				return nullptr;
			}
			void setValue(Value a_val) { m_value = a_val; };
			Value getValue() { return m_value;  };

			void* userptr;			//allow userdata to be attached.
			UA_NodeId asUaNodeId() {
				if( NodeId.IdType == uamodel::String ) {
					return UA_NODEID_STRING( NodeId.NamespaceIndex, (char*)(NodeId.Identifier.c_str()) );
				} else if( NodeId.IdType == uamodel::Numeric ) {
					return UA_NODEID_NUMERIC( NodeId.NamespaceIndex, NodeId.IdentifierNum );
				}
			};
			Object* getParentNode() { return m_parentNode; };
		private:
			struct {
				string Identifier;
				uint32_t IdentifierNum;
				uint32_t NamespaceIndex;
				enum IdentifierType IdType;
			}NodeId;
			enum ObjClass NodeClass; 
			string BrowseName;
			string DisplayName{""};
			string Description;

			vector<Object*> m_children;
			Value m_value;
			DataType m_type;

		private:
			uamodel::Object* m_parentNode;
			friend ListView;
			friend ServerDialog;
			friend OpcUa;
	};

	static inline std::string ValueOrEmpty(const char* s)
	{
	    return s == nullptr ? std::string() : s;
	};
};

#endif
